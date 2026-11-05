// Host tests for the CH32V307 USBHS device driver
// (src/families/ch32v30x/UsbDevice): the register values of begin(), end()
// and connect(), the endpoint open rules, the buffer pool allocation and
// alignment, the DMA address, MAX_LEN, TX_CTRL and RX_CTRL values that the
// driver writes, the packet copy of startIn(), and the STALL and NAK
// helpers.
//
// The double-buffered OUT tests call the interrupt handler with the INT_FG,
// INT_ST, RX_LEN and RX_CTRL values that the hardware would leave (the
// test flips RX_TOG[0] as the auto toggle does). Whether the real USBHS
// selects the buffer as the RM table 21-3 says is for the hardware test.
//
// The USBHS and RCC registers are plain host memory (stub <wch_device.h> in
// stubs-usb-v307/), so a register holds the value of the last write. The
// fake does not model the write-1-to-clear INT_FG flags, the automatic data
// toggle of endpoints 1 to 15, or the response change that the hardware
// makes after a transfer. Thus the test does not run the interrupt handler,
// and it checks no toggle state after a transfer. It also does not check
// interrupt timing (INT_BUSY, the INT_ST and RX_LEN race of docs/usb.md).
// The usb-loopback example of ch32v00x-examples is the place for the
// interrupt handler and the toggle; it is not done yet. No test proves the
// INT_ST and RX_LEN race: the window is too small (docs/usb.md, Known limits).
//
// The driver writes 32-bit DMA addresses. On a 64-bit host this is the low
// half of the pointer. The buffers are in the UsbDevice object, so the test
// takes (DMA address - low half of &usb) as the offset in the object.

#include "check.h"
#include "families/ch32v30x/UsbDevice.h"
#include "utils/Interrupt.h"
#include "delaybusywait.h"

#include <array>
#include <cstring>

USBHS_TypeDef hostUsbhs;
RCC_TypeDef hostRcc;
IRQn_Type hostNvicEnabled = TIM2_IRQn;
IRQn_Type hostNvicDisabled = TIM2_IRQn;

namespace {

using hw::UsbDevice;
using hw::usb::EndpointType;

int blockingDepth = 0;
int blockingSections = 0;
IRQn_Type blockedIrq = TIM2_IRQn;
IRQn_Type connectedIrq = TIM2_IRQn;
uint32_t delayUs = 0;
uint8_t ctrlInDelay = 0;

void resetFakes() {
   std::memset(static_cast<void *>(&hostUsbhs), 0, sizeof(hostUsbhs));
   std::memset(static_cast<void *>(&hostRcc), 0, sizeof(hostRcc));
   hostNvicEnabled = TIM2_IRQn;
   hostNvicDisabled = TIM2_IRQn;
   blockingSections = 0;
   blockedIrq = TIM2_IRQn;
   connectedIrq = TIM2_IRQn;
   delayUs = 0;
   ctrlInDelay = 0;
}

using Snapshot = std::array<uint8_t, sizeof(USBHS_TypeDef)>;

Snapshot snapshot() {
   Snapshot s;
   std::memcpy(s.data(), static_cast<const void *>(&hostUsbhs), s.size());
   return s;
}

void onEvent(void *, const hw::usb::Event &) {
}

auto &ep(uint8_t n) {
   return hostUsbhs.UEP[n];
}

// Offset of a DMA buffer in the UsbDevice object. Modulo 2^32, so it is
// correct for the low half of a 64-bit pointer.
uint32_t offsetOf(const UsbDevice &usb, uint32_t dma) {
   return dma - static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&usb));
}

// A DMA address must be 4-byte aligned and in the object.
bool validBuffer(const UsbDevice &usb, uint32_t dma, uint16_t size) {
   uint32_t offset = offsetOf(usb, dma);
   return dma % 4 == 0 && offset + size <= sizeof(UsbDevice);
}

const uint8_t *bufferAt(const UsbDevice &usb, uint32_t dma) {
   return reinterpret_cast<const uint8_t *>(&usb) + offsetOf(usb, dma);
}

uint32_t txDma(uint8_t n) {
   return hostUsbhs.UEP_TX_DMA[n - 1];
}

uint32_t rxDma(uint8_t n) {
   return hostUsbhs.UEP_RX_DMA[n - 1];
}

constexpr uint8_t TxOpen = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_RES_NAK;
constexpr uint8_t RxOpen = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_RES_NAK;

void begin(UsbDevice &usb) {
   resetFakes();
   usb.begin(&onEvent, nullptr);
}

void testBegin() {
   resetFakes();
   // Values from before begin() that resetEndpoints() must overwrite.
   ep(0).TX_CTRL = USBHS_UEP_T_RES_STALL;
   ep(3).RX_CTRL = USBHS_UEP_R_RES_ACK;
   hostUsbhs.UEP_CONFIG = 0xFFFFFFFF;
   hostUsbhs.DEV_AD = 5;
   hostRcc.CFGR2 = RCC_CFGR2_USBHSPLLSRC | 0x7; // HSI source bit set, PREDIV1 bits
   UsbDevice usb;
   usb.begin(&onEvent, nullptr);

   // HSE_VALUE 8 MHz: USBHSDIV 1 gives the 4 MHz PHY reference. The
   // PREDIV1 bits stay the same.
   CHECK(hostRcc.CFGR2 == ((1ul << 24) | RCC_CFGR2_USBHSCLK_4MHZ | RCC_CFGR2_USBFSSRC | RCC_CFGR2_USBHSPLL | 0x7));
   CHECK(hostRcc.AHBPCENR == RCC_USBHSEN);
   // The SIE reset is on during the delay and off after it.
   CHECK(delayUs >= 10);
   CHECK(ctrlInDelay == (USBHS_UC_CLR_ALL | USBHS_UC_RST_SIE));
   CHECK(hostUsbhs.CTRL == (USBHS_UC_DMA_EN | USBHS_UC_INT_BUSY | USBHS_UC_SPEED_HIGH));
   CHECK(hostUsbhs.HOST_CTRL == USBHS_UH_PHY_SUSPENDM);
   CHECK(hostUsbhs.INT_EN == (USBHS_UIE_BUS_RST | USBHS_UIE_TRANSFER | USBHS_UIE_SUSPEND | USBHS_UIE_SETUP_ACT));
   CHECK(hostUsbhs.INT_FG == 0xFF);

   // EP0: one 64-byte buffer for both directions, OUT ready, IN NAK.
   CHECK(validBuffer(usb, hostUsbhs.UEP0_DMA, UsbDevice::Ep0MaxPacketSize));
   CHECK(hostUsbhs.UEP_MAX_LEN[0].MAX_LEN == UsbDevice::Ep0MaxPacketSize);
   CHECK(ep(0).T_LEN == 0);
   CHECK(ep(0).TX_CTRL == USBHS_UEP_T_RES_NAK);
   CHECK(ep(0).RX_CTRL == USBHS_UEP_R_RES_ACK);
   CHECK(hostUsbhs.UEP_CONFIG == 0);
   CHECK(hostUsbhs.UEP_TYPE == 0);
   CHECK(hostUsbhs.UEP_BUF_MOD == 0);
   CHECK(hostUsbhs.DEV_AD == 0);
   for(uint8_t n = 1; n < UsbDevice::EndpointCount; ++n) {
      CHECK(ep(n).TX_CTRL == USBHS_UEP_T_RES_NAK);
      CHECK(ep(n).RX_CTRL == USBHS_UEP_R_RES_NAK);
   }

   CHECK(connectedIrq == USBHS_IRQn);
   CHECK(hostNvicEnabled == USBHS_IRQn);
   CHECK(!usb.isConnected());
}

void testBeginFullSpeed() {
   resetFakes();
   UsbDevice usb(UsbDevice::EndpointCount, UsbDevice::Speed::Full);
   usb.begin(&onEvent, nullptr);
   CHECK(hostUsbhs.CTRL == (USBHS_UC_DMA_EN | USBHS_UC_INT_BUSY | USBHS_UC_SPEED_FULL));
}

void testEnd() {
   UsbDevice usb;
   begin(usb);
   usb.connect();
   hostRcc.CFGR2 |= 0x7;
   usb.end();
   CHECK(hostNvicDisabled == USBHS_IRQn);
   CHECK(hostUsbhs.INT_EN == 0);
   CHECK(hostUsbhs.CTRL == (USBHS_UC_CLR_ALL | USBHS_UC_RST_SIE));
   CHECK(hostUsbhs.HOST_CTRL == 0);
   CHECK((hostRcc.AHBPCENR & RCC_USBHSEN) == 0);
   CHECK((hostRcc.CFGR2 & RCC_CFGR2_USBHSPLL) == 0);
   CHECK((hostRcc.CFGR2 & 0x7) == 0x7);
}

void testConnectAddressSpeed() {
   UsbDevice usb;
   begin(usb);
   const uint8_t ctrl = hostUsbhs.CTRL;
   usb.connect();
   CHECK(hostUsbhs.CTRL == (ctrl | USBHS_UC_DEV_PU_EN));
   CHECK(usb.isConnected());
   usb.disconnect();
   CHECK(hostUsbhs.CTRL == ctrl);
   CHECK(!usb.isConnected());

   usb.setAddress(42);
   CHECK(hostUsbhs.DEV_AD == 42);
   usb.setAddress(0xFF); // 7-bit address
   CHECK(hostUsbhs.DEV_AD == 0x7F);

   hostUsbhs.SPEED_TYPE = USBHS_SPEED_TYPE_HIGH;
   CHECK(usb.speed() == UsbDevice::Speed::High);
   hostUsbhs.SPEED_TYPE = USBHS_SPEED_TYPE_FULL;
   CHECK(usb.speed() == UsbDevice::Speed::Full);
   hostUsbhs.SPEED_TYPE = static_cast<uint8_t>(0xFC | USBHS_SPEED_TYPE_HIGH); // upper bits ignored
   CHECK(usb.speed() == UsbDevice::Speed::High);
}

void testOpenEndpointRegisters() {
   UsbDevice usb;
   begin(usb);
   blockingSections = 0;

   CHECK(usb.openEndpoint(0x81, EndpointType::Bulk, 512));
   CHECK(validBuffer(usb, txDma(1), 512));
   CHECK(ep(1).T_LEN == 0);
   CHECK(ep(1).TX_CTRL == TxOpen);
   CHECK(hostUsbhs.UEP_CONFIG == USBHS_UEP_T_EN(1));

   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 512));
   CHECK(validBuffer(usb, rxDma(1), 512));
   CHECK(hostUsbhs.UEP_MAX_LEN[1].MAX_LEN == 512);
   CHECK(ep(1).RX_CTRL == RxOpen);
   CHECK(ep(1).TX_CTRL == TxOpen); // the IN direction stays the same
   CHECK(hostUsbhs.UEP_CONFIG == (USBHS_UEP_T_EN(1) | USBHS_UEP_R_EN(1)));

   // Endpoint 15 uses the last DMA register and the bits 15 and 31.
   CHECK(usb.openEndpoint(0x8F, EndpointType::Interrupt, 8));
   CHECK(usb.openEndpoint(0x0F, EndpointType::Interrupt, 8));
   CHECK(validBuffer(usb, txDma(15), 8));
   CHECK(validBuffer(usb, rxDma(15), 8));
   CHECK(hostUsbhs.UEP_MAX_LEN[15].MAX_LEN == 8);
   CHECK(hostUsbhs.UEP_CONFIG == (USBHS_UEP_T_EN(1) | USBHS_UEP_R_EN(1) | 0x80008000u));

   // The two directions of one number can have different types.
   CHECK(usb.openEndpoint(0x82, EndpointType::Interrupt, 900));
   CHECK(usb.openEndpoint(0x02, EndpointType::Bulk, 64));
   CHECK(hostUsbhs.UEP_MAX_LEN[2].MAX_LEN == 64);

   CHECK(blockingSections == 6);
   CHECK(blockedIrq == USBHS_IRQn);
   CHECK(blockingDepth == 0);
}

void testPoolAllocation() {
   UsbDevice usb;
   begin(usb);
   // Each buffer is the packet size rounded up to 4 bytes, in open order.
   CHECK(usb.openEndpoint(0x81, EndpointType::Interrupt, 9));
   CHECK(usb.openEndpoint(0x01, EndpointType::Interrupt, 1));
   CHECK(usb.openEndpoint(0x82, EndpointType::Bulk, 64));
   const uint32_t first = offsetOf(usb, txDma(1));
   CHECK(offsetOf(usb, rxDma(1)) == first + 12);
   CHECK(offsetOf(usb, txDma(2)) == first + 16);
   CHECK(txDma(1) % 4 == 0 && rxDma(1) % 4 == 0 && txDma(2) % 4 == 0);
   // The pool and the EP0 buffer do not overlap.
   const uint32_t ep0 = offsetOf(usb, hostUsbhs.UEP0_DMA);
   CHECK(ep0 + UsbDevice::Ep0MaxPacketSize <= first || first + UsbDevice::PacketMemorySize <= ep0);
}

void testPoolFull() {
   UsbDevice usb;
   begin(usb);
   // 2048 bytes: four 512-byte buffers fill the pool.
   CHECK(usb.openEndpoint(0x81, EndpointType::Bulk, 512));
   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 512));
   CHECK(usb.openEndpoint(0x82, EndpointType::Bulk, 512));
   CHECK(usb.openEndpoint(0x02, EndpointType::Bulk, 509)); // takes 512
   const uint32_t first = offsetOf(usb, txDma(1));
   CHECK(offsetOf(usb, rxDma(2)) == first + 3 * 512);
   auto before = snapshot();
   CHECK(!usb.openEndpoint(0x83, EndpointType::Interrupt, 1));
   CHECK(snapshot() == before);

   // closeAllEndpoints() frees the pool and disables endpoints 1 to 15.
   usb.closeAllEndpoints();
   CHECK(hostUsbhs.UEP_CONFIG == 0);
   for(uint8_t n = 1; n < UsbDevice::EndpointCount; ++n) {
      CHECK(ep(n).TX_CTRL == USBHS_UEP_T_RES_NAK);
      CHECK(ep(n).RX_CTRL == USBHS_UEP_R_RES_NAK);
      CHECK(ep(n).T_LEN == 0);
   }
   CHECK(ep(0).RX_CTRL == USBHS_UEP_R_RES_ACK); // EP0 stays the same
   CHECK(!usb.startIn(0x81, {}));               // closed
   CHECK(usb.openEndpoint(0x83, EndpointType::Interrupt, 1024));
   CHECK(offsetOf(usb, txDma(3)) == first);
   CHECK(usb.openEndpoint(0x03, EndpointType::Interrupt, 1024));
   CHECK(!usb.openEndpoint(0x84, EndpointType::Interrupt, 1));
}

void testOpenEndpointRejects() {
   UsbDevice usb(4);
   begin(usb);
   auto before = snapshot();
   CHECK(!usb.openEndpoint(0x84, EndpointType::Bulk, 64)); // number >= endpointCount
   CHECK(!usb.openEndpoint(0x04, EndpointType::Bulk, 64));
   CHECK(!usb.openEndpoint(0x81, EndpointType::Isochronous, 64));
   CHECK(!usb.openEndpoint(0x81, EndpointType::Control, 64)); // control only on EP0
   CHECK(!usb.openEndpoint(0x80, EndpointType::Bulk, 64));    // EP0 is control only
   CHECK(!usb.openEndpoint(0x81, EndpointType::Bulk, 0));
   CHECK(!usb.openEndpoint(0x81, EndpointType::Bulk, 513));
   CHECK(!usb.openEndpoint(0x81, EndpointType::Interrupt, 1025));
   CHECK(!usb.openEndpoint(0x00, EndpointType::Control, 65));
   CHECK(!usb.openEndpoint(0x81, EndpointType::Bulk, 64, true)); // double-buffered IN
   CHECK(!usb.openEndpoint(0x01, EndpointType::Interrupt, 64, true)); // double-buffered, not bulk
   CHECK(!usb.openEndpoint(0x00, EndpointType::Control, 64, true));
   CHECK(snapshot() == before);

   // Reopening EP0 with 64 bytes is allowed and writes nothing.
   CHECK(usb.openEndpoint(0x00, EndpointType::Control, 64));
   CHECK(usb.openEndpoint(0x80, EndpointType::Control, 64));
   CHECK(snapshot() == before);

   CHECK(usb.openEndpoint(0x83, EndpointType::Bulk, 64));
   const uint32_t dma = txDma(3);
   ep(3).TX_CTRL = USBHS_UEP_T_RES_ACK; // a packet waits: reopen must not reset it
   before = snapshot();
   CHECK(usb.openEndpoint(0x83, EndpointType::Bulk, 64));  // same size: keeps the buffer
   CHECK(!usb.openEndpoint(0x83, EndpointType::Bulk, 32)); // other size
   CHECK(snapshot() == before);
   CHECK(txDma(3) == dma);
}

void testStartIn() {
   UsbDevice usb;
   begin(usb);
   CHECK(usb.openEndpoint(0x81, EndpointType::Bulk, 16));
   const std::array<uint8_t, 16> packet{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

   CHECK(usb.startIn(0x81, packet));
   CHECK(std::memcmp(bufferAt(usb, txDma(1)), packet.data(), packet.size()) == 0);
   CHECK(ep(1).T_LEN == 16);
   CHECK(ep(1).TX_CTRL == (USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_RES_ACK));

   // The previous packet waits for its IN token.
   auto before = snapshot();
   CHECK(!usb.startIn(0x81, hw::span<const uint8_t>(packet.data(), 4)));
   CHECK(snapshot() == before);

   // After the IN transfer the endpoint answers NAK again (the interrupt
   // handler does this on the hardware). The toggle bits stay the same.
   ep(1).TX_CTRL = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_NAK;
   CHECK(usb.startIn(0x81, {})); // zero-length packet
   CHECK(ep(1).T_LEN == 0);
   CHECK(ep(1).TX_CTRL == (USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK));

   ep(1).TX_CTRL = TxOpen;
   const std::array<uint8_t, 17> tooLong{};
   before = snapshot();
   CHECK(!usb.startIn(0x81, tooLong));
   CHECK(!usb.startIn(0x01, packet)); // OUT address
   CHECK(!usb.startIn(0x82, packet)); // not open
   CHECK(snapshot() == before);

   // EP0 sends from the shared EP0 buffer. startIn() ends a STALL.
   ep(0).TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_STALL;
   CHECK(usb.startIn(0x80, hw::span<const uint8_t>(packet.data(), 8)));
   CHECK(std::memcmp(bufferAt(usb, hostUsbhs.UEP0_DMA), packet.data(), 8) == 0);
   CHECK(ep(0).T_LEN == 8);
   CHECK(ep(0).TX_CTRL == (USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK));
   CHECK(blockingDepth == 0);
}

void testArmOutAndReadOut() {
   UsbDevice usb;
   begin(usb);
   CHECK(usb.openEndpoint(0x02, EndpointType::Bulk, 64));
   ep(2).RX_CTRL = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_NAK;
   CHECK(usb.armOut(0x02));
   CHECK(ep(2).RX_CTRL == (USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK));

   // armOut() on EP0 ends a STALL.
   ep(0).RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_STALL;
   CHECK(usb.armOut(0x00));
   CHECK(ep(0).RX_CTRL == (USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK));

   auto before = snapshot();
   CHECK(!usb.armOut(0x82)); // IN address
   CHECK(!usb.armOut(0x03)); // not open
   CHECK(snapshot() == before);

   // No packet received yet: nothing to copy.
   std::array<uint8_t, 64> buffer{};
   CHECK(usb.readOut(0x02, buffer) == 0);
   CHECK(usb.readOut(0x00, buffer) == 0);
   CHECK(usb.readOut(0x82, buffer) == 0);
   CHECK(blockingDepth == 0);
}


// Events that the callback got, in order: 'O' for Out with the size.
std::array<std::pair<uint8_t, uint16_t>, 16> events{};
size_t eventCount = 0;

void recordEvent(void *, const hw::usb::Event &event) {
   if(event.type == hw::usb::EventType::Out && eventCount < events.size()) {
      events[eventCount++] = { event.endpoint, event.size };
   }
}

// What the hardware does for an OUT packet of endpoint n that it accepts: it
// writes the length, flips RX_TOG[0] (auto toggle) and raises TRANSFER.
void receivePacket(UsbDevice &usb, uint8_t n, uint16_t size) {
   hostUsbhs.RX_LEN = size;
   ep(n).RX_CTRL ^= USBHS_UEP_R_TOG_DATA1;
   hostUsbhs.INT_ST = static_cast<uint8_t>(USBHS_UIS_TOKEN_OUT | USBHS_UIS_TOG_OK | n);
   hostUsbhs.INT_FG = USBHS_UIF_TRANSFER;
   usb.handleInterrupt();
}

uint8_t rxResponse(uint8_t n) {
   return ep(n).RX_CTRL & USBHS_UEP_R_RES_MASK;
}

void testDoubleBufferedOpen() {
   UsbDevice usb;
   begin(usb);
   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 512, true));
   // Two buffers, in the pool one after the other. The IN register is buffer 1.
   CHECK(validBuffer(usb, rxDma(1), 512) && validBuffer(usb, txDma(1), 512));
   CHECK(txDma(1) == rxDma(1) + 512);
   CHECK(hostUsbhs.UEP_BUF_MOD == USBHS_UEP_BUF_MOD(1));
   CHECK((hostUsbhs.UEP_CONFIG & USBHS_UEP_R_EN(1)) != 0);
   CHECK((hostUsbhs.UEP_CONFIG & USBHS_UEP_T_EN(1)) == 0);
   CHECK(ep(1).RX_CTRL == RxOpen);
   CHECK(hostUsbhs.UEP_MAX_LEN[1].MAX_LEN == 512);

   // The IN direction of that number cannot open, and a double-buffered
   // endpoint cannot open on a number with an open IN direction.
   CHECK(!usb.openEndpoint(0x81, EndpointType::Bulk, 512));
   CHECK(usb.openEndpoint(0x82, EndpointType::Bulk, 64));
   CHECK(!usb.openEndpoint(0x02, EndpointType::Bulk, 64, true));
   // Reopen with the same attributes keeps the buffers. Another mode fails.
   const uint32_t dma = rxDma(1);
   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 512, true));
   CHECK(!usb.openEndpoint(0x01, EndpointType::Bulk, 512, false));
   CHECK(rxDma(1) == dma);

   // The pool (2048 bytes) has 1024 + 64 bytes in use. Two 256-byte buffers
   // fit, two 512-byte buffers do not.
   CHECK(usb.openEndpoint(0x03, EndpointType::Bulk, 256, true));
   CHECK(!usb.openEndpoint(0x04, EndpointType::Bulk, 512, true));
   usb.closeAllEndpoints();
   CHECK(hostUsbhs.UEP_BUF_MOD == 0);
}

void testDoubleBufferedReceive() {
   UsbDevice usb;
   resetFakes();
   eventCount = 0;
   usb.begin(&recordEvent, nullptr);
   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 64, true));
   auto *buf0 = const_cast<uint8_t *>(bufferAt(usb, rxDma(1)));
   auto *buf1 = const_cast<uint8_t *>(bufferAt(usb, txDma(1)));
   std::array<uint8_t, 64> data{};
   CHECK(usb.readOut(0x01, data) == 0); // nothing reported yet

   // Not armed yet: the endpoint NAKs.
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_NAK);
   CHECK(usb.armOut(0x01));
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_ACK);

   // First packet goes to buffer 0 (toggle 0 before it). It is reported at
   // once, and the endpoint stays ACK for the second buffer.
   std::memset(buf0, 0xA0, 64);
   receivePacket(usb, 1, 10);
   CHECK(eventCount == 1 && events[0].first == 1 && events[0].second == 10);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_ACK);
   CHECK(usb.readOut(0x01, data) == 10 && data[0] == 0xA0);

   // Second packet arrives while the software still reads buffer 0: it goes
   // to buffer 1, waits, and the endpoint NAKs (both buffers busy).
   std::memset(buf1, 0xB1, 64);
   receivePacket(usb, 1, 20);
   CHECK(eventCount == 1);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_NAK);
   CHECK(usb.readOut(0x01, data) == 10 && data[0] == 0xA0); // still the first one

   // armOut() frees buffer 0, reports the waiting packet and arms the endpoint.
   CHECK(usb.armOut(0x01));
   CHECK(eventCount == 2 && events[1].second == 20);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_ACK);
   CHECK(usb.readOut(0x01, data) == 20 && data[0] == 0xB1);

   // Third packet: buffer 0 again (the toggle is back at 0). It waits.
   receivePacket(usb, 1, 30);
   CHECK(eventCount == 2);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_NAK);
   CHECK(usb.armOut(0x01)); // frees buffer 1, reports the third packet from buffer 0
   CHECK(eventCount == 3 && events[2].second == 30);
   CHECK(usb.readOut(0x01, data) == 30 && data[0] == 0xA0);

   // armOut() with nothing waiting frees the buffer and keeps ACK.
   CHECK(usb.armOut(0x01));
   CHECK(eventCount == 3 && rxResponse(1) == USBHS_UEP_R_RES_ACK);
   CHECK(usb.readOut(0x01, data) == 0);

   // A repeated packet (toggle mismatch) changes nothing.
   hostUsbhs.RX_LEN = 5;
   hostUsbhs.INT_ST = static_cast<uint8_t>(USBHS_UIS_TOKEN_OUT | 1); // no TOG_OK
   hostUsbhs.INT_FG = USBHS_UIF_TRANSFER;
   usb.handleInterrupt();
   CHECK(eventCount == 3 && rxResponse(1) == USBHS_UEP_R_RES_ACK);
}

void testDoubleBufferedStallAndNak() {
   UsbDevice usb;
   resetFakes();
   eventCount = 0;
   usb.begin(&recordEvent, nullptr);
   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 64, true));
   CHECK(usb.armOut(0x01));
   receivePacket(usb, 1, 7); // reported, buffer 0 held
   receivePacket(usb, 1, 8); // pending in buffer 1
   CHECK(eventCount == 1);

   // Clearing STALL drops both packets and returns to buffer 0 (DATA0).
   usb.setStall(0x01, true);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_STALL);
   usb.setStall(0x01, false);
   CHECK(ep(1).RX_CTRL == RxOpen);
   std::array<uint8_t, 64> data{};
   CHECK(usb.readOut(0x01, data) == 0);
   CHECK(usb.armOut(0x01)); // nothing pending: no event
   CHECK(eventCount == 1);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_ACK);

   // setNak() disarms. The next armOut() arms again.
   usb.setNak(0x01);
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_NAK);
   CHECK(usb.armOut(0x01));
   CHECK(rxResponse(1) == USBHS_UEP_R_RES_ACK);
}

void testStallAndNak() {
   UsbDevice usb;
   begin(usb);
   CHECK(usb.openEndpoint(0x81, EndpointType::Bulk, 64));
   CHECK(usb.openEndpoint(0x01, EndpointType::Bulk, 64));

   // STALL keeps the toggle bits.
   ep(1).TX_CTRL = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
   usb.setStall(0x81, true);
   CHECK(ep(1).TX_CTRL == (USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_STALL));
   CHECK(usb.isStalled(0x81));
   CHECK(!usb.isStalled(0x01));
   // Clearing STALL sets NAK and DATA0, and keeps the auto toggle.
   usb.setStall(0x81, false);
   CHECK(ep(1).TX_CTRL == TxOpen);
   CHECK(!usb.isStalled(0x81));

   ep(1).RX_CTRL = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
   usb.setStall(0x01, true);
   CHECK(ep(1).RX_CTRL == (USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_STALL));
   CHECK(usb.isStalled(0x01));
   CHECK(!usb.isStalled(0x81));
   usb.setStall(0x01, false);
   CHECK(ep(1).RX_CTRL == RxOpen);

   // EP0 has no auto toggle: clearing STALL gives DATA0 and NAK only.
   ep(0).TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
   ep(0).RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
   usb.setStall(0x80, true);
   usb.setStall(0x00, true);
   CHECK(ep(0).TX_CTRL == (USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_STALL));
   CHECK(ep(0).RX_CTRL == (USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_STALL));
   usb.setStall(0x80, false);
   usb.setStall(0x00, false);
   CHECK(ep(0).TX_CTRL == USBHS_UEP_T_RES_NAK);
   CHECK(ep(0).RX_CTRL == USBHS_UEP_R_RES_NAK);

   // setNak() changes the response only.
   ep(1).TX_CTRL = USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
   ep(1).RX_CTRL = USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
   usb.setNak(0x81);
   CHECK(ep(1).TX_CTRL == (USBHS_UEP_T_TOG_AUTO | USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_NAK));
   CHECK(ep(1).RX_CTRL == (USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK));
   usb.setNak(0x01);
   CHECK(ep(1).RX_CTRL == (USBHS_UEP_R_TOG_AUTO | USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_NAK));

   // Endpoints that are not open: no write.
   auto before = snapshot();
   usb.setStall(0x82, true);
   usb.setStall(0x02, true);
   usb.setNak(0x82);
   usb.setNak(0x02);
   CHECK(snapshot() == before);
   CHECK(!usb.isStalled(0x82));
   CHECK(blockingDepth == 0);
}

void testEndpointCountLimit() {
   UsbDevice usb(2);
   begin(usb);
   CHECK(usb.openEndpoint(0x81, EndpointType::Bulk, 64));
   auto before = snapshot();
   CHECK(!usb.openEndpoint(0x82, EndpointType::Bulk, 64));
   CHECK(!usb.startIn(0x82, {}));
   CHECK(!usb.armOut(0x02));
   usb.setStall(0x82, true);
   usb.setNak(0x02);
   CHECK(snapshot() == before);

   // 0 and values above 16 give all 16 endpoint numbers.
   UsbDevice all(0);
   begin(all);
   CHECK(all.openEndpoint(0x8F, EndpointType::Bulk, 64));
   UsbDevice tooMany(17);
   begin(tooMany);
   CHECK(tooMany.openEndpoint(0x8F, EndpointType::Bulk, 64));
}

} // namespace

// Host versions of the interrupt and delay functions that the driver uses.
namespace Interrupt {

void connectHandler(IRQn_Type IRQn, AdvancedHandler_t, void *) {
   connectedIrq = IRQn;
}

BlockingSection::BlockingSection(IRQn_Type IRQn)
   : irqn(static_cast<int8_t>(IRQn)) {
   blockedIrq = IRQn;
   ++blockingDepth;
   ++blockingSections;
}

BlockingSection::~BlockingSection() {
   --blockingDepth;
}

} // namespace Interrupt

extern "C" void delayuS(uint32_t usec) {
   delayUs += usec;
   ctrlInDelay = hostUsbhs.CTRL;
}

int main() {
   testBegin();
   testBeginFullSpeed();
   testEnd();
   testConnectAddressSpeed();
   testOpenEndpointRegisters();
   testPoolAllocation();
   testPoolFull();
   testOpenEndpointRejects();
   testStartIn();
   testArmOutAndReadOut();
   testStallAndNak();
   testDoubleBufferedOpen();
   testDoubleBufferedReceive();
   testDoubleBufferedStallAndNak();
   testEndpointCountLimit();
   if(failures == 0) {
      std::printf("usb-v307: all checks passed\n");
   }
   return failures;
}
