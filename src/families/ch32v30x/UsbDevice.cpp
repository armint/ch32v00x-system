#include "UsbDevice.h"

#include <ch32v30x_usb.h>

#include <cstddef>

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

// The endpoint registers of USBHSD_TypeDef. The official header has one
// field for each endpoint. The fields of one kind are contiguous (see the
// static_asserts), so the driver indexes them from the field of endpoint 0
// or 1.
static_assert(offsetof(USBHSD_TypeDef, UEP15_TX_LEN) - offsetof(USBHSD_TypeDef, UEP0_TX_LEN) == 15 * 4);
static_assert(offsetof(USBHSD_TypeDef, UEP0_TX_CTRL) == offsetof(USBHSD_TypeDef, UEP0_TX_LEN) + 2);
static_assert(offsetof(USBHSD_TypeDef, UEP0_RX_CTRL) == offsetof(USBHSD_TypeDef, UEP0_TX_LEN) + 3);
static_assert(offsetof(USBHSD_TypeDef, UEP15_MAX_LEN) - offsetof(USBHSD_TypeDef, UEP0_MAX_LEN) == 15 * 4);
static_assert(offsetof(USBHSD_TypeDef, UEP15_RX_DMA) - offsetof(USBHSD_TypeDef, UEP1_RX_DMA) == 14 * 4);
static_assert(offsetof(USBHSD_TypeDef, UEP15_TX_DMA) - offsetof(USBHSD_TypeDef, UEP1_TX_DMA) == 14 * 4);

volatile uint16_t &txLen(uint8_t n) {
   return (&USBHSD->UEP0_TX_LEN)[2 * n];
}

volatile uint8_t &txCtrl(uint8_t n) {
   return (&USBHSD->UEP0_TX_CTRL)[4 * n];
}

volatile uint8_t &rxCtrl(uint8_t n) {
   return (&USBHSD->UEP0_RX_CTRL)[4 * n];
}

volatile uint16_t &maxLen(uint8_t n) {
   return (&USBHSD->UEP0_MAX_LEN)[2 * n];
}

// n = 1..15
volatile uint32_t &rxDma(uint8_t n) {
   return (&USBHSD->UEP1_RX_DMA)[n - 1];
}

volatile uint32_t &txDma(uint8_t n) {
   return (&USBHSD->UEP1_TX_DMA)[n - 1];
}

void setTxResponse(uint8_t n, uint8_t response) {
   txCtrl(n) = static_cast<uint8_t>((txCtrl(n) & ~USBHS_UEP_T_RES_MASK) | response);
}

void setRxResponse(uint8_t n, uint8_t response) {
   rxCtrl(n) = static_cast<uint8_t>((rxCtrl(n) & ~USBHS_UEP_R_RES_MASK) | response);
}

// The control byte that the handler writes for endpoints 1..15: the auto toggle, the data toggle
// from the software copy (Direction::toggle), and a response. TX_CTRL and RX_CTRL have the same
// bit positions.
uint8_t controlByte(uint8_t toggle, uint8_t response) {
   return static_cast<uint8_t>(USBHS_UEP_T_TOG_AUTO | (toggle != 0 ? USBHS_UEP_T_TOG_DATA1 : 0) | response);
}

// INT_ST of a waiting transfer, and whether a SETUP completed too. INT_ST is read first: a SETUP that completes
// before that read (and clears TOG_OK, see outToggleOk()) then shows in INT_FG. A SETUP after it cannot change
// the value read. Two statements, because the order of evaluation of function arguments is unspecified.
struct TransferStatus {
   uint8_t status;
   bool setupPending;
};

TransferStatus readTransferStatus() {
   const uint8_t status = USBHSD->INT_ST;
   const bool setupPending = (USBHSD->INT_FG & USBHS_UIF_SETUP_ACT) != 0;
   return { status, setupPending };
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
   RCC->CFGR2 = (RCC->CFGR2 & ~(RCC_USBHSDIV | RCC_USBHSPLLSRC | RCC_USBHSCLK)) |
                UsbHsDiv | USBHS_PLL_CKREF_4M | RCC_USBFSSRC;
   RCC->CFGR2 |= RCC_USBHSPLL;
   RCC->AHBPCENR |= RCC_USBHSEN;

   // RST_SIE must be cleared by software (RM 22.2.1.1). delayuS() counts
   // 6 cycles per loop; the V4F core needs fewer, so ask for more than the
   // 10 us of the WCH examples.
   USBHSD->CONTROL = USBHS_UC_CLR_ALL | USBHS_UC_RESET_SIE;
   delayuS(50);
   USBHSD->CONTROL = 0;
   USBHSD->HOST_CTRL = USBHS_UH_PHY_SUSPENDM;
   USBHSD->CONTROL = USBHS_UC_DMA_EN | USBHS_UC_INT_BUSY |
                 (_maxSpeed == Speed::High ? USBHS_UC_SPEED_HIGH : USBHS_UC_SPEED_FULL);
   USBHSD->INT_EN = 0;
   USBHSD->INT_FG = 0xFF;
   resetEndpoints();
   USBHSD->INT_EN = InterruptMask;

   Interrupt::connectMemberHandler<UsbDevice, &UsbDevice::handleInterrupt>(UsbIrq, *this);
   NVIC_ClearPendingIRQ(UsbIrq);
   NVIC_EnableIRQ(UsbIrq);
}

void UsbDevice::end() {
   disconnect();
   NVIC_DisableIRQ(UsbIrq);
   USBHSD->INT_EN = 0;
   USBHSD->CONTROL = USBHS_UC_CLR_ALL | USBHS_UC_RESET_SIE;
   USBHSD->HOST_CTRL = 0;
   RCC->AHBPCENR &= ~RCC_USBHSEN;
   RCC->CFGR2 &= ~RCC_USBHSPLL;
}

void UsbDevice::connect() {
   USBHSD->CONTROL |= USBHS_UC_DEV_PU_EN;
}

void UsbDevice::disconnect() {
   USBHSD->CONTROL &= static_cast<uint8_t>(~USBHS_UC_DEV_PU_EN);
}

bool UsbDevice::isConnected() const {
   return (USBHSD->CONTROL & USBHS_UC_DEV_PU_EN) != 0;
}

UsbDevice::Speed UsbDevice::speed() const {
   return (USBHSD->SPEED_TYPE & USBHS_USB_SPEED_TYPE) == USBHS_USB_SPEED_HIGH ? Speed::High : Speed::Full;
}

void UsbDevice::setAddress(uint8_t address) {
   USBHSD->DEV_AD = address & USBHS_MASK_USB_ADDR;
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
         // DATA0 and NAK, and no packet that was received before (SET_INTERFACE
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
      txDma(n) = dmaAddress(dir->buffer);
      txLen(n) = 0;
      txCtrl(n) = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_RES_NAK;
      USBHSD->ENDP_CONFIG |= (USBHS_UEP0_T_EN << (n));
   } else {
      rxDma(n) = dmaAddress(dir->buffer);
      maxLen(n) = maxPacketSize;
      rxCtrl(n) = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_RES_NAK;
      if(doubleBuffered) {
         // Buffer 0 is UEPn_RX_DMA, buffer 1 is UEPn_TX_DMA (table 21-3). The
         // toggle bit RX_TOG[0] selects the buffer and flips after each packet.
         txDma(n) = dmaAddress(dir->buffer1);
         USBHSD->BUF_MODE |= (USBHS_UEP0_BUF_MOD << (n));
         resetDoubleBuffer(n);
      }
      USBHSD->ENDP_CONFIG |= (USBHS_UEP0_R_EN << (n));
   }
   return true;
}

void UsbDevice::closeAllEndpoints() {
   Interrupt::BlockingSection block(UsbIrq);
   USBHSD->ENDP_CONFIG = 0;
   USBHSD->BUF_MODE = 0;
   for(uint8_t n = 1; n < EndpointCount; ++n) {
      txCtrl(n) = USBHS_UEP_T_RES_NAK;
      rxCtrl(n) = USBHS_UEP_R_RES_NAK;
      txLen(n) = 0;
      _endpoints[n] = {};
   }
   _poolNext = 0;
}

void UsbDevice::resetEndpoints() {
   closeAllEndpoints();
   USBHSD->ENDP_TYPE = 0;
   USBHSD->BUF_MODE = 0;

   // EP0: one buffer for both directions. OUT answers ACK, so the first
   // control transfer after the reset finds EP0 ready. IN answers NAK
   // until startIn().
   USBHSD->UEP0_DMA = dmaAddress(_ep0Buffer);
   maxLen(0) = Ep0MaxPacketSize;
   txLen(0) = 0;
   txCtrl(0) = USBHS_UEP_T_RES_NAK;
   rxCtrl(0) = USBHS_UEP_R_RES_ACK;
   for(Direction *d : { &_endpoints[0].in, &_endpoints[0].out }) {
      *d = {};
      d->buffer = _ep0Buffer;
      d->maxPacketSize = Ep0MaxPacketSize;
      d->open = true;
   }
   USBHSD->DEV_AD = 0;
}

bool UsbDevice::startIn(uint8_t address, hw::span<const uint8_t> data) {
   Interrupt::BlockingSection block(UsbIrq);
   const Direction *dir = direction(address);
   if(!usb::isInEndpoint(address) || dir == nullptr || !dir->open || data.size() > dir->maxPacketSize) {
      return false;
   }
   uint8_t n = usb::endpointNumber(address);
   if((txCtrl(n) & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_ACK) {
      return false; // the previous packet waits for its IN token
   }
   for(size_t i = 0; i < data.size(); ++i) {
      dir->buffer[i] = data[i];
   }
   txLen(n) = static_cast<uint16_t>(data.size());
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
      out.nakHeld = false;
      out.reported = NoBuffer;
      if(out.pending != NoBuffer) {
         const uint8_t buffer = out.pending;
         out.pending = NoBuffer;
         reportDoubleBuffered(n, buffer);
      }
      // The endpoint stays ACK after a packet while the buffer that the hardware fills next is free (the
      // handler sets NAK when it is not). Write only when it is NAK: no packet is in flight then.
      if((rxCtrl(n) & USBHS_UEP_R_RES_MASK) != USBHS_UEP_R_RES_ACK) {
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
         txCtrl(n) = static_cast<uint8_t>((txCtrl(n) & USBHS_UEP_T_TOG_AUTO) | USBHS_UEP_T_RES_NAK);
         _endpoints[n].in.toggle = 0;
      }
   } else {
      if(stall) {
         setRxResponse(n, USBHS_UEP_R_RES_STALL);
      } else {
         rxCtrl(n) = static_cast<uint8_t>((rxCtrl(n) & USBHS_UEP_R_TOG_AUTO) | USBHS_UEP_R_RES_NAK);
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
      return (txCtrl(n) & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_STALL;
   }
   return (rxCtrl(n) & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL;
}

bool UsbDevice::transferPending(uint8_t address, uint8_t status, bool setupPending) const {
   if((status & USBHS_UIS_ENDP_MASK) != usb::endpointNumber(address)) { // IS_NAK: see handleTransfer()
      return false;
   }
   // An OUT packet with the wrong toggle is dropped by the handler: it makes no event.
   return usb::isInEndpoint(address) ? (status & USBHS_UIS_TOKEN_MASK) == USBHS_UIS_TOKEN_IN
                                     : (status & USBHS_UIS_TOKEN_MASK) == USBHS_UIS_TOKEN_OUT &&
                                          outToggleOk(status, setupPending);
}

// A SETUP that completes while an OUT transfer waits clears TOG_OK in INT_ST (board measurement, issue #25:
// INT_FG 0x22, INT_ST 0x02 for a good EP2 packet). Then the RX toggle of endpoints 1..15 tells it: the auto
// toggle moved on from the software copy only for a packet with the right toggle. Not used without a SETUP,
// because a read-modify-write of setNak() or setStall() can undo the hardware flip (see handleTransfer()).
// Known limit: a SETUP and an undone flip on the same packet drop it (not seen on the board, docs/usb.md).
bool UsbDevice::outToggleOk(uint8_t status, bool setupPending) const {
   const uint8_t n = status & USBHS_UIS_ENDP_MASK;
   if(!setupPending || n == 0 || n >= _endpointCount) {
      return (status & USBHS_UIS_TOG_OK) != 0;
   }
   const uint8_t hardware = (rxCtrl(n) & USBHS_UEP_R_TOG_DATA1) != 0 ? 1 : 0;
   return hardware != _endpoints[n].out.toggle;
}

// Handles a transfer that the hardware has completed and the handler has not reported yet, with the
// interrupt masked (the caller holds the BlockingSection). Then no completion of this direction comes later
// and undoes a reset or a NAK of the main loop.
void UsbDevice::drainCompleted(uint8_t address) {
   if((USBHSD->INT_FG & (USBHS_UIF_BUS_RST | USBHS_UIF_TRANSFER)) != USBHS_UIF_TRANSFER) {
      return;
   }
   const TransferStatus t = readTransferStatus(); // one read for the check and the handler
   if(transferPending(address, t.status, t.setupPending)) {
      handleTransfer(t.status, t.setupPending);
      USBHSD->INT_FG = USBHS_UIF_TRANSFER;
   }
}

void UsbDevice::setNak(uint8_t address) {
   Interrupt::BlockingSection block(UsbIrq);
   Direction *dir = direction(address);
   if(dir == nullptr || !dir->open) {
      return;
   }
   if(!usb::isInEndpoint(address)) {
      dir->nakHeld = true; // before the drain: its handler run must not write ACK
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
   dir.nakHeld = false;
   if(usb::isInEndpoint(address)) {
      txLen(n) = 0;
      txCtrl(n) = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_RES_NAK;
   } else {
      rxCtrl(n) = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_RES_NAK;
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
   // IS_NAK is not checked: DEV_NAK is off, so TRANSFER is set only by a completed transaction. A NAKed retry
   // while TRANSFER waits (INT_BUSY) sets IS_NAK in INT_ST, but the transaction is still to be handled.
   uint8_t n = status & USBHS_UIS_ENDP_MASK;
   switch(status & USBHS_UIS_TOKEN_MASK) {
   case USBHS_UIS_TOKEN_IN:
      // NAK first: the buffer is free for the next startIn(), and a second
      // IN token must not send the same packet again.
      if(n == 0) {
         setTxResponse(n, USBHS_UEP_T_RES_NAK);
         txCtrl(0) ^= USBHS_UEP_T_TOG_DATA1;
      } else if(n < _endpointCount) {
         // The data toggle comes from the software copy, and the write puts it back in the hardware: a
         // read-modify-write of the main loop (setNak(), setStall()) can have undone the flip of the
         // hardware. A STALL that the main loop set after the packet stays.
         Direction &in = _endpoints[n].in;
         in.toggle ^= 1;
         const uint8_t response = (txCtrl(n) & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_STALL ? USBHS_UEP_T_RES_STALL : USBHS_UEP_T_RES_NAK;
         txCtrl(n) = controlByte(in.toggle, response);
      }
      report(EventType::InComplete, static_cast<uint8_t>(n | usb::EndpointDirectionIn));
      break;
   case USBHS_UIS_TOKEN_OUT: {
      uint16_t size = USBHSD->RX_LEN;
      if(!outToggleOk(status, setupPending)) {
         break; // repeated packet: the host lost the ACK. Stay armed.
      }
      if(n != 0 && n < _endpointCount) {
         // The packet is in the buffer that the software copy of the toggle selects (double-buffered), and the
         // toggle goes on. The write below puts the toggle back in the hardware, see the IN case.
         Direction &out = _endpoints[n].out;
         const uint8_t buffer = out.toggle;
         out.toggle ^= 1;
         uint8_t response = (rxCtrl(n) & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL ? USBHS_UEP_R_RES_STALL : USBHS_UEP_R_RES_NAK;
         if(out.doubleBuffered) {
            (buffer == 0 ? out.received : out.received1) = size;
            if(out.reported == NoBuffer) {
               // The software has no packet: report this one. The hardware fills the other buffer next, so the
               // endpoint stays ACK.
               if(response == USBHS_UEP_R_RES_NAK && !out.nakHeld) {
                  response = USBHS_UEP_R_RES_ACK;
               }
               rxCtrl(n) = controlByte(out.toggle, response);
               reportDoubleBuffered(n, buffer);
            } else {
               // The software reads the other buffer, which the hardware would fill next: NAK until armOut()
               // reports this packet.
               out.pending = buffer;
               rxCtrl(n) = controlByte(out.toggle, response);
            }
            break;
         }
         rxCtrl(n) = controlByte(out.toggle, response);
      }
      if(n == 0) {
         if(setupPending) {
            // The SETUP overwrote the shared EP0 buffer and ends this
            // control transfer anyway, so drop the packet.
            break;
         }
         rxCtrl(0) ^= USBHS_UEP_R_TOG_DATA1;
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
   uint8_t flags = USBHSD->INT_FG;

   if(flags & USBHS_UIF_BUS_RST) {
      // A transfer flag from before the reset is stale.
      USBHSD->INT_FG = flags & (USBHS_UIF_BUS_RST | USBHS_UIF_TRANSFER);
      flags &= static_cast<uint8_t>(~USBHS_UIF_TRANSFER);
      resetEndpoints();
      report(EventType::Reset, 0);
   }

   // The transfer before the SETUP: with INT_BUSY no other token completes
   // while TRANSFER is set, but a SETUP does. The layer above sees
   // InComplete or Out first, then Setup. NAKed retries in this time set
   // IS_NAK in INT_ST (see handleTransfer()). A SETUP can also complete
   // after the INT_FG read at entry: readTransferStatus() reads INT_FG again.
   // That SETUP is handled in the next interrupt.
   if(flags & USBHS_UIF_TRANSFER) {
      const TransferStatus t = readTransferStatus();
      handleTransfer(t.status, t.setupPending);
      USBHSD->INT_FG = USBHS_UIF_TRANSFER;
   }

   if(flags & USBHS_UIF_SETUP_ACT) {
      txCtrl(0) = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_NAK;
      rxCtrl(0) = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_NAK;
      usb::Event event{ EventType::Setup, 0, 8, {} };
      for(uint8_t i = 0; i < 8; ++i) {
         event.setup[i] = _ep0Buffer[i];
      }
      // Clear after the copy: the next SETUP overwrites the buffer.
      USBHSD->INT_FG = USBHS_UIF_SETUP_ACT;
      if(_callback != nullptr) {
         _callback(_context, event);
      }
   }

   if(flags & USBHS_UIF_SUSPEND) {
      USBHSD->INT_FG = USBHS_UIF_SUSPEND;
      report((USBHSD->MIS_ST & USBHS_UMS_SUSPEND) ? EventType::Suspend : EventType::Resume, 0);
   }

   // Sources that are not enabled can still be flagged; clear them.
   USBHSD->INT_FG = flags & (USBHS_UIF_HST_SOF | USBHS_UIF_FIFO_OV | USBHS_UIF_ISO_ACT);
}

} // namespace hw
