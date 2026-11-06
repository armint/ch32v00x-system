#include "UsbDevice.h"

#include "utils/Interrupt.h"
#include "delaybusywait.h"

namespace hw {

using usb::EndpointType;
using usb::EventType;

namespace {

constexpr IRQn_Type UsbIrq = USBHS_IRQn;

// The USB PHY PLL takes a 4 MHz reference: HSE / USBHSDIV (RM 3.4.12).
constexpr uint32_t PhyReferenceHz = 4000000;
static_assert(HSE_VALUE % PhyReferenceHz == 0 && HSE_VALUE / PhyReferenceHz >= 1 && HSE_VALUE / PhyReferenceHz <= 8,
              "USBHS needs HSE_VALUE = 4..32 MHz in steps of 4 MHz");
constexpr uint32_t UsbHsDiv = (HSE_VALUE / PhyReferenceHz - 1) << 24;

constexpr uint8_t InterruptMask = USBHS_UIE_BUS_RST | USBHS_UIE_TRANSFER | USBHS_UIE_SUSPEND | USBHS_UIE_SETUP_ACT;

USBHS_EpCtrl_TypeDef &ctrl(uint8_t n) {
   return USBHS->UEP[n];
}

void setTxResponse(uint8_t n, uint8_t response) {
   ctrl(n).TX_CTRL = static_cast<uint8_t>((ctrl(n).TX_CTRL & ~USBHS_UEP_T_RES_MASK) | response);
}

void setRxResponse(uint8_t n, uint8_t response) {
   ctrl(n).RX_CTRL = static_cast<uint8_t>((ctrl(n).RX_CTRL & ~USBHS_UEP_R_RES_MASK) | response);
}

// The control byte that the handler writes for endpoints 1..15: the auto toggle, the data toggle
// from the software copy (Direction::toggle), and a response. TX_CTRL and RX_CTRL have the same
// bit positions.
uint8_t controlByte(uint8_t toggle, uint8_t response) {
   return static_cast<uint8_t>(USBHS_UEP_T_TOG_AUTO | (toggle != 0 ? USBHS_UEP_T_TOG_DATA1 : 0) | response);
}

// DMA addresses are 32-bit RAM addresses.
uint32_t dmaAddress(const uint8_t *buffer) {
   return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(buffer));
}

uint16_t maxPacketSizeOf(EndpointType type) {
   switch(type) {
   case EndpointType::Control:
      return UsbDevice::Ep0MaxPacketSize;
   case EndpointType::Bulk:
      return UsbDevice::BulkMaxPacketSize;
   case EndpointType::Interrupt:
      return UsbDevice::InterruptMaxPacketSize;
   default:
      return 0;
   }
}

} // namespace

UsbDevice::UsbDevice(uint8_t endpointCount, Speed maxSpeed)
   : _endpointCount(endpointCount == 0 || endpointCount > EndpointCount ? EndpointCount : endpointCount),
     _maxSpeed(maxSpeed) {}

void UsbDevice::begin(usb::EventCallback callback, void *context) {
   _callback = callback;
   _context = context;

   // Clock sequence of the WCH USBHS examples: PHY PLL from HSE at 4 MHz,
   // 48 MHz USB clock from the PHY, PLL on, then the AHB clock.
   RCC->CFGR2 = (RCC->CFGR2 & ~(RCC_CFGR2_USBHSDIV_MASK | RCC_CFGR2_USBHSPLLSRC | RCC_CFGR2_USBHSCLK_MASK)) |
                UsbHsDiv | RCC_CFGR2_USBHSCLK_4MHZ | RCC_CFGR2_USBFSSRC;
   RCC->CFGR2 |= RCC_CFGR2_USBHSPLL;
   RCC->AHBPCENR |= RCC_USBHSEN;

   // RST_SIE must be cleared by software (RM 22.2.1.1). delayuS() counts
   // 6 cycles per loop; the V4F core needs fewer, so ask for more than the
   // 10 us of the WCH examples.
   USBHS->CTRL = USBHS_UC_CLR_ALL | USBHS_UC_RST_SIE;
   delayuS(50);
   USBHS->CTRL = 0;
   USBHS->HOST_CTRL = USBHS_UH_PHY_SUSPENDM;
   USBHS->CTRL = USBHS_UC_DMA_EN | USBHS_UC_INT_BUSY |
                 (_maxSpeed == Speed::High ? USBHS_UC_SPEED_HIGH : USBHS_UC_SPEED_FULL);
   USBHS->INT_EN = 0;
   USBHS->INT_FG = 0xFF;
   resetEndpoints();
   USBHS->INT_EN = InterruptMask;

   Interrupt::connectMemberHandler<UsbDevice, &UsbDevice::handleInterrupt>(UsbIrq, *this);
   NVIC_ClearPendingIRQ(UsbIrq);
   NVIC_EnableIRQ(UsbIrq);
}

void UsbDevice::end() {
   disconnect();
   NVIC_DisableIRQ(UsbIrq);
   USBHS->INT_EN = 0;
   USBHS->CTRL = USBHS_UC_CLR_ALL | USBHS_UC_RST_SIE;
   USBHS->HOST_CTRL = 0;
   RCC->AHBPCENR &= ~RCC_USBHSEN;
   RCC->CFGR2 &= ~RCC_CFGR2_USBHSPLL;
}

void UsbDevice::connect() {
   USBHS->CTRL |= USBHS_UC_DEV_PU_EN;
}

void UsbDevice::disconnect() {
   USBHS->CTRL &= static_cast<uint8_t>(~USBHS_UC_DEV_PU_EN);
}

bool UsbDevice::isConnected() const {
   return (USBHS->CTRL & USBHS_UC_DEV_PU_EN) != 0;
}

UsbDevice::Speed UsbDevice::speed() const {
   return (USBHS->SPEED_TYPE & USBHS_SPEED_TYPE_MASK) == USBHS_SPEED_TYPE_HIGH ? Speed::High : Speed::Full;
}

void UsbDevice::setAddress(uint8_t address) {
   USBHS->DEV_AD = address & USBHS_DEV_AD_MASK;
}

UsbDevice::Direction *UsbDevice::direction(uint8_t address) {
   uint8_t n = usb::endpointNumber(address);
   if(n >= _endpointCount) {
      return nullptr;
   }
   return usb::isInEndpoint(address) ? &_endpoints[n].in : &_endpoints[n].out;
}

const UsbDevice::Direction *UsbDevice::direction(uint8_t address) const {
   return const_cast<UsbDevice *>(this)->direction(address);
}

bool UsbDevice::openEndpoint(uint8_t address, EndpointType type, uint16_t maxPacketSize, bool doubleBuffered) {
   Interrupt::BlockingSection block(UsbIrq);
   uint8_t n = usb::endpointNumber(address);
   Direction *dir = direction(address);
   if(dir == nullptr || type == EndpointType::Isochronous || (type == EndpointType::Control) != (n == 0) ||
      maxPacketSize == 0 || maxPacketSize > maxPacketSizeOf(type)) {
      return false;
   }
   const Direction &other = usb::isInEndpoint(address) ? _endpoints[n].out : _endpoints[n].in;
   // The IN buffer register is the second OUT buffer of a double-buffered endpoint.
   if(doubleBuffered && (usb::isInEndpoint(address) || type != EndpointType::Bulk || other.open)) {
      return false;
   }
   if(other.open && other.doubleBuffered) {
      return false;
   }
   if(dir->open) {
      // Reopening with the same size keeps the buffer. A new size would
      // leak the old buffer until closeAllEndpoints().
      if(dir->maxPacketSize != maxPacketSize || dir->doubleBuffered != doubleBuffered) {
         return false;
      }
      if(n != 0) {
         // As the system-stm32 driver: DATA0 and NAK, and no packet that was received before (SET_INTERFACE
         // resets the data toggle of its endpoints).
         drainCompleted(address);
         resetDirection(address);
      }
      return true;
   }
   // EP0 is open from resetEndpoints() on, so n is 1..15 here.
   uint16_t bytes = static_cast<uint16_t>((maxPacketSize + 3u) & ~3u); // DMA addresses are 4-byte aligned
   const uint16_t total = doubleBuffered ? static_cast<uint16_t>(2u * bytes) : bytes;
   if(total > PacketMemorySize - _poolNext) {
      return false;
   }
   dir->buffer = &_pool[_poolNext];
   dir->buffer1 = doubleBuffered ? &_pool[_poolNext + bytes] : nullptr;
   _poolNext = static_cast<uint16_t>(_poolNext + total);
   dir->maxPacketSize = maxPacketSize;
   dir->received = 0;
   dir->received1 = 0;
   dir->doubleBuffered = doubleBuffered;
   dir->open = true;

   if(usb::isInEndpoint(address)) {
      USBHS->UEP_TX_DMA[n - 1] = dmaAddress(dir->buffer);
      ctrl(n).T_LEN = 0;
      ctrl(n).TX_CTRL = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_RES_NAK;
      USBHS->UEP_CONFIG |= USBHS_UEP_T_EN(n);
   } else {
      USBHS->UEP_RX_DMA[n - 1] = dmaAddress(dir->buffer);
      USBHS->UEP_MAX_LEN[n].MAX_LEN = maxPacketSize;
      ctrl(n).RX_CTRL = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_RES_NAK;
      if(doubleBuffered) {
         // Buffer 0 is UEPn_RX_DMA, buffer 1 is UEPn_TX_DMA (table 21-3). The
         // toggle bit RX_TOG[0] selects the buffer and flips after each packet.
         USBHS->UEP_TX_DMA[n - 1] = dmaAddress(dir->buffer1);
         USBHS->UEP_BUF_MOD |= USBHS_UEP_BUF_MOD(n);
         resetDoubleBuffer(n);
      }
      USBHS->UEP_CONFIG |= USBHS_UEP_R_EN(n);
   }
   return true;
}

void UsbDevice::closeAllEndpoints() {
   Interrupt::BlockingSection block(UsbIrq);
   USBHS->UEP_CONFIG = 0;
   USBHS->UEP_BUF_MOD = 0;
   for(uint8_t n = 1; n < EndpointCount; ++n) {
      ctrl(n).TX_CTRL = USBHS_UEP_T_RES_NAK;
      ctrl(n).RX_CTRL = USBHS_UEP_R_RES_NAK;
      ctrl(n).T_LEN = 0;
      _endpoints[n] = {};
   }
   _poolNext = 0;
}

void UsbDevice::resetEndpoints() {
   closeAllEndpoints();
   USBHS->UEP_TYPE = 0;
   USBHS->UEP_BUF_MOD = 0;

   // EP0: one buffer for both directions. OUT answers ACK, so the first
   // control transfer after the reset finds EP0 ready, as on the STM32
   // driver. IN answers NAK until startIn().
   USBHS->UEP0_DMA = dmaAddress(_ep0Buffer);
   USBHS->UEP_MAX_LEN[0].MAX_LEN = Ep0MaxPacketSize;
   ctrl(0).T_LEN = 0;
   ctrl(0).TX_CTRL = USBHS_UEP_T_RES_NAK;
   ctrl(0).RX_CTRL = USBHS_UEP_R_RES_ACK;
   for(Direction *d : { &_endpoints[0].in, &_endpoints[0].out }) {
      *d = {};
      d->buffer = _ep0Buffer;
      d->maxPacketSize = Ep0MaxPacketSize;
      d->open = true;
   }
   USBHS->DEV_AD = 0;
}

bool UsbDevice::startIn(uint8_t address, hw::span<const uint8_t> data) {
   Interrupt::BlockingSection block(UsbIrq);
   const Direction *dir = direction(address);
   if(!usb::isInEndpoint(address) || dir == nullptr || !dir->open || data.size() > dir->maxPacketSize) {
      return false;
   }
   uint8_t n = usb::endpointNumber(address);
   if((ctrl(n).TX_CTRL & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_ACK) {
      return false; // the previous packet waits for its IN token
   }
   for(size_t i = 0; i < data.size(); ++i) {
      dir->buffer[i] = data[i];
   }
   ctrl(n).T_LEN = static_cast<uint16_t>(data.size());
   setTxResponse(n, USBHS_UEP_T_RES_ACK);
   return true;
}

bool UsbDevice::armOut(uint8_t address) {
   Interrupt::BlockingSection block(UsbIrq);
   const Direction *dir = direction(address);
   if(usb::isInEndpoint(address) || dir == nullptr || !dir->open) {
      return false;
   }
   const uint8_t n = usb::endpointNumber(address);
   if(dir->doubleBuffered) {
      Direction &out = _endpoints[n].out;
      out.reported = NoBuffer;
      if(out.pending != NoBuffer) {
         // The software holds this packet now: the endpoint stays NAK until the next armOut().
         const uint8_t buffer = out.pending;
         out.pending = NoBuffer;
         reportDoubleBuffered(n, buffer);
         return true;
      }
      // Write only when it is not ACK: no packet is in flight then.
      if((ctrl(n).RX_CTRL & USBHS_UEP_R_RES_MASK) != USBHS_UEP_R_RES_ACK) {
         setRxResponse(n, USBHS_UEP_R_RES_ACK);
      }
      return true;
   }
   setRxResponse(n, USBHS_UEP_R_RES_ACK);
   return true;
}

uint16_t UsbDevice::readOut(uint8_t address, hw::span<uint8_t> buffer) const {
   Interrupt::BlockingSection block(UsbIrq);
   const Direction *dir = direction(address);
   if(usb::isInEndpoint(address) || dir == nullptr || !dir->open) {
      return 0;
   }
   const uint8_t *from = usb::endpointNumber(address) == 0 ? _ep0Out : dir->buffer;
   uint16_t size = dir->received;
   if(dir->doubleBuffered) {
      if(dir->reported == NoBuffer) {
         return 0;
      }
      if(dir->reported == 1) {
         from = dir->buffer1;
         size = dir->received1;
      }
   }
   if(size > buffer.size()) {
      size = static_cast<uint16_t>(buffer.size());
   }
   for(uint16_t i = 0; i < size; ++i) {
      buffer[i] = from[i];
   }
   return size;
}

void UsbDevice::setStall(uint8_t address, bool stall) {
   Interrupt::BlockingSection block(UsbIrq);
   const Direction *dir = direction(address);
   if(dir == nullptr || !dir->open) {
      return;
   }
   drainCompleted(address);
   uint8_t n = usb::endpointNumber(address);
   if(usb::isInEndpoint(address)) {
      if(stall) {
         setTxResponse(n, USBHS_UEP_T_RES_STALL);
      } else {
         // DATA0 and NAK; keep the auto toggle of endpoints 1..15.
         ctrl(n).TX_CTRL = static_cast<uint8_t>((ctrl(n).TX_CTRL & USBHS_UEP_T_TOG_AUTO) | USBHS_UEP_T_RES_NAK);
         _endpoints[n].in.toggle = 0;
      }
   } else {
      if(stall) {
         setRxResponse(n, USBHS_UEP_R_RES_STALL);
      } else {
         ctrl(n).RX_CTRL = static_cast<uint8_t>((ctrl(n).RX_CTRL & USBHS_UEP_R_TOG_AUTO) | USBHS_UEP_R_RES_NAK);
         _endpoints[n].out.toggle = 0;
         if(dir->doubleBuffered) {
            resetDoubleBuffer(n); // DATA0 selects buffer 0 again
         }
      }
   }
}

bool UsbDevice::isStalled(uint8_t address) const {
   uint8_t n = usb::endpointNumber(address);
   if(n >= EndpointCount) {
      return false;
   }
   if(usb::isInEndpoint(address)) {
      return (ctrl(n).TX_CTRL & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_STALL;
   }
   return (ctrl(n).RX_CTRL & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL;
}

bool UsbDevice::transferPending(uint8_t address) const {
   if((USBHS->INT_FG & USBHS_UIF_TRANSFER) == 0) {
      return false;
   }
   const uint8_t status = USBHS->INT_ST;
   if((status & USBHS_UIS_IS_NAK) != 0 || (status & USBHS_UIS_ENDP_MASK) != usb::endpointNumber(address)) {
      return false;
   }
   // An OUT packet with the wrong toggle is dropped by the handler: it makes no event.
   return usb::isInEndpoint(address) ? (status & USBHS_UIS_TOKEN_MASK) == USBHS_UIS_TOKEN_IN
                                     : (status & USBHS_UIS_TOKEN_MASK) == USBHS_UIS_TOKEN_OUT && (status & USBHS_UIS_TOG_OK) != 0;
}

// Handles a transfer that the hardware has completed and the handler has not reported yet, with the
// interrupt masked (the caller holds the BlockingSection). Then no completion of this direction comes later
// and undoes a reset or a NAK of the main loop.
void UsbDevice::drainCompleted(uint8_t address) {
   if((USBHS->INT_FG & USBHS_UIF_BUS_RST) == 0 && transferPending(address)) {
      handleTransfer(USBHS->INT_ST, (USBHS->INT_FG & USBHS_UIF_SETUP_ACT) != 0);
      USBHS->INT_FG = USBHS_UIF_TRANSFER;
   }
}

void UsbDevice::setNak(uint8_t address) {
   Interrupt::BlockingSection block(UsbIrq);
   Direction *dir = direction(address);
   if(dir == nullptr || !dir->open) {
      return;
   }
   drainCompleted(address);
   uint8_t n = usb::endpointNumber(address);
   if(usb::isInEndpoint(address)) {
      setTxResponse(n, USBHS_UEP_T_RES_NAK);
   } else {
      setRxResponse(n, USBHS_UEP_R_RES_NAK);
   }
}

// A reopen of an open direction (n = 1..15): DATA0, NAK, and no earlier packet.
void UsbDevice::resetDirection(uint8_t address) {
   const uint8_t n = usb::endpointNumber(address);
   Direction &dir = usb::isInEndpoint(address) ? _endpoints[n].in : _endpoints[n].out;
   dir.toggle = 0;
   if(usb::isInEndpoint(address)) {
      ctrl(n).T_LEN = 0;
      ctrl(n).TX_CTRL = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_RES_NAK;
   } else {
      ctrl(n).RX_CTRL = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_RES_NAK;
      if(dir.doubleBuffered) {
         resetDoubleBuffer(n);
      }
   }
}

void UsbDevice::resetDoubleBuffer(uint8_t n) {
   Direction &out = _endpoints[n].out;
   out.reported = NoBuffer;
   out.pending = NoBuffer;
}

void UsbDevice::reportDoubleBuffered(uint8_t n, uint8_t buffer) {
   Direction &out = _endpoints[n].out;
   out.reported = buffer;
   report(EventType::Out, n, buffer == 0 ? out.received : out.received1);
}

void UsbDevice::report(EventType type, uint8_t endpoint, uint16_t size) {
   if(_callback != nullptr) {
      usb::Event event{ type, endpoint, size, {} };
      _callback(_context, event);
   }
}

void UsbDevice::handleTransfer(uint8_t status, bool setupPending) {
   uint8_t n = status & USBHS_UIS_ENDP_MASK;
   if(status & USBHS_UIS_IS_NAK) {
      return; // the endpoint answered NAK: no data moved (DEV_NAK is off, so this is a guard only)
   }
   switch(status & USBHS_UIS_TOKEN_MASK) {
   case USBHS_UIS_TOKEN_IN:
      // NAK first: the buffer is free for the next startIn(), and a second
      // IN token must not send the same packet again.
      if(n == 0) {
         setTxResponse(n, USBHS_UEP_T_RES_NAK);
         ctrl(0).TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
      } else if(n < _endpointCount) {
         // The data toggle comes from the software copy, and the write puts it back in the hardware: a
         // read-modify-write of the main loop (setNak(), setStall()) can have undone the flip of the
         // hardware. A STALL that the main loop set after the packet stays.
         Direction &in = _endpoints[n].in;
         in.toggle ^= 1;
         const uint8_t response = (ctrl(n).TX_CTRL & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_STALL ? USBHS_UEP_T_RES_STALL : USBHS_UEP_T_RES_NAK;
         ctrl(n).TX_CTRL = controlByte(in.toggle, response);
      }
      report(EventType::InComplete, static_cast<uint8_t>(n | usb::EndpointDirectionIn));
      break;
   case USBHS_UIS_TOKEN_OUT: {
      uint16_t size = USBHS->RX_LEN;
      if((status & USBHS_UIS_TOG_OK) == 0) {
         break; // repeated packet: the host lost the ACK. Stay armed.
      }
      if(n != 0 && n < _endpointCount) {
         // The packet is in the buffer that the software copy of the toggle selects (double-buffered), and the
         // toggle goes on. The write below puts the toggle back in the hardware, see the IN case.
         Direction &out = _endpoints[n].out;
         const uint8_t buffer = out.toggle;
         out.toggle ^= 1;
         uint8_t response = (ctrl(n).RX_CTRL & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL ? USBHS_UEP_R_RES_STALL : USBHS_UEP_R_RES_NAK;
         if(out.doubleBuffered) {
            // NAK after each packet, as single-buffered. The controller seems to accept packets while TRANSFER
            // waits (not confirmed, issue #25), so an ACK while the software holds a packet can overwrite it.
            (buffer == 0 ? out.received : out.received1) = size;
            ctrl(n).RX_CTRL = controlByte(out.toggle, response);
            if(out.reported == NoBuffer) {
               reportDoubleBuffered(n, buffer);
            } else {
               out.pending = buffer; // armOut() reports it. Needs an event queue that the RM does not describe.
            }
            break;
         }
         ctrl(n).RX_CTRL = controlByte(out.toggle, response);
      }
      if(n == 0) {
         if(setupPending) {
            // The SETUP overwrote the shared EP0 buffer and ends this
            // control transfer anyway, so drop the packet.
            break;
         }
         ctrl(0).RX_CTRL ^= USBHS_UEP_R_TOG_DATA1;
         if(size > Ep0MaxPacketSize) {
            size = Ep0MaxPacketSize;
         }
         for(uint16_t i = 0; i < size; ++i) {
            _ep0Out[i] = _ep0Buffer[i];
         }
      }
      if(n < _endpointCount) {
         _endpoints[n].out.received = size;
      }
      report(EventType::Out, n, size);
      break;
   }
   default:
      break; // SOF tokens: SOF_ACT is off. SETUP: reported through SETUP_ACT.
   }
}

void UsbDevice::handleInterrupt() {
   uint8_t flags = USBHS->INT_FG;

   if(flags & USBHS_UIF_BUS_RST) {
      // A transfer flag from before the reset is stale.
      USBHS->INT_FG = flags & (USBHS_UIF_BUS_RST | USBHS_UIF_TRANSFER);
      flags &= static_cast<uint8_t>(~USBHS_UIF_TRANSFER);
      resetEndpoints();
      report(EventType::Reset, 0);
   }

   // The transfer before the SETUP: a SETUP can complete while TRANSFER is
   // set (and, per issue #25, other tokens too, not confirmed). The layer
   // above sees InComplete or Out first, then Setup.
   if(flags & USBHS_UIF_TRANSFER) {
      handleTransfer(USBHS->INT_ST, (flags & USBHS_UIF_SETUP_ACT) != 0);
      USBHS->INT_FG = USBHS_UIF_TRANSFER;
   }

   if(flags & USBHS_UIF_SETUP_ACT) {
      ctrl(0).TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_NAK;
      ctrl(0).RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_NAK;
      usb::Event event{ EventType::Setup, 0, 8, {} };
      for(uint8_t i = 0; i < 8; ++i) {
         event.setup[i] = _ep0Buffer[i];
      }
      // Clear after the copy: the next SETUP overwrites the buffer.
      USBHS->INT_FG = USBHS_UIF_SETUP_ACT;
      if(_callback != nullptr) {
         _callback(_context, event);
      }
   }

   if(flags & USBHS_UIF_SUSPEND) {
      USBHS->INT_FG = USBHS_UIF_SUSPEND;
      report((USBHS->MIS_ST & USBHS_UMS_SUSPEND) ? EventType::Suspend : EventType::Resume, 0);
   }

   // Sources that are not enabled can still be flagged; clear them.
   USBHS->INT_FG = flags & (USBHS_UIF_HST_SOF | USBHS_UIF_FIFO_OV | USBHS_UIF_ISO_ACT);
}

} // namespace hw
