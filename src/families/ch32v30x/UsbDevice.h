#pragma once

#include "peripherals/UsbDeviceTypes.h"
#include "utils/Span.h"
#include <cstdint>
#include <wch_device.h>

namespace hw {

// USB device on the USBHS controller of the CH32V307 (RM chapter 22),
// high speed (480 Mbit/s) or full speed (12 Mbit/s). The public interface
// is the one of hw::UsbDevice in system-stm32, so a controller port above it
// (for example libart-stm32 on the libArT USB stack) needs few changes.
// Differences: 16 endpoint numbers, packets up to 512 bytes (bulk) or 1024
// bytes (interrupt), a maximum speed in the constructor, speed(), and no
// pull-up callback (the D+ pull-up is internal). See docs/usb.md.
//
// The hardware is not the STM32 packet-memory block: each endpoint has a
// DMA buffer in RAM and its own TX_CTRL/RX_CTRL register (response and data
// toggle). This driver keeps the buffers in the object: EP0 has one 64-byte
// buffer for both directions, endpoints 1..15 take theirs from a pool of
// PacketMemorySize bytes.
//
// Packet level only: one call moves at most one packet. Control transfers,
// multi-packet transfers and zero-length packets are the job of the layer
// above. The interrupt handler decodes the hardware events, copies SETUP
// packets and EP0 OUT packets into RAM, and reports each event through the
// EventCallback.
//
// Flow control: an OUT endpoint answers NAK until armOut(), and the
// interrupt handler sets it back to NAK after each received packet, so the
// data stays in the endpoint buffer until the next armOut(). An IN endpoint
// answers NAK until startIn(), and again after the host acknowledged the
// packet. CTRL.INT_BUSY makes the controller NAK all tokens while a
// transfer interrupt is pending, so no packet can arrive between the event
// and the switch to NAK.
//
// Data toggle: endpoints 1..15 use the automatic toggle of the hardware.
// EP0 has none, so the driver sets DATA1 for both directions after each
// SETUP and flips the toggle after each IN or OUT packet of EP0. The status
// stage then gets DATA1, as USB 2.0 section 8.5.3 requires. An OUT packet
// with the wrong toggle (a repeated packet after a lost ACK) is dropped and
// the endpoint stays armed.
//
// After a SETUP packet, both directions of EP0 answer NAK. The layer above
// then calls startIn(), armOut() or setStall() for the data and status
// stages.
//
// Requirements:
//  - A crystal on HSE (HSE_VALUE, a multiple of 4 MHz, 4..32 MHz) that
//    runs. begin() starts the USB PHY PLL from HSE / (HSE_VALUE / 4 MHz).
//  - PB6 (D-) and PB7 (D+) need no GPIO setup. The controller takes them
//    when begin() releases the SIE reset.
//
// Concurrency: every public function may run in the event callback (USB
// interrupt) or in the main loop. The functions that change endpoint state
// mask the USBHS interrupt while they run (Interrupt::BlockingSection).
// begin() and end() are for the main loop only.
//
// Double-buffered bulk OUT (RM 22.2.2.3, table 21-3): openEndpoint() with
// doubleBuffered true gives an OUT endpoint two buffers. The hardware
// receives the next packet into the second buffer while the software still
// reads the first. The interface stays the same as in system-stm32: one Out
// event per armOut(), and readOut() reads that packet. A packet that arrives
// before armOut() waits in its buffer, and armOut() reports it. The
// endpoint number then has no IN direction (the hardware uses the IN buffer
// address register as the second OUT buffer). The hardware does not NAK when
// both buffers are full, so the interrupt handler sets NAK then.
// setNak() on a double-buffered endpoint that answers ACK is not race-free:
// a packet that completes during the read-modify-write of RX_CTRL can lose
// its toggle flip (docs/usb.md).
//
// Limits: no isochronous endpoints, no double-buffered IN endpoints, no
// remote wake-up, no test modes. INT_ST (token and endpoint of the last
// transfer) and RX_LEN (length of the last received packet) are global. A
// SETUP that completes while a TRANSFER event of endpoint 1..15 waits for
// the handler can overwrite both. Then the handler sees a SETUP token, and
// the IN or OUT event is lost: an IN endpoint stays ACK and sends the same
// packet again, with no InComplete. A wrong RX_LEN gives a wrong OUT length.
// The window is the interrupt entry time. The hardware test must show if
// the controller holds INT_ST and RX_LEN for a pending TRANSFER.
class UsbDevice {
   public:
   static constexpr uint8_t EndpointCount = 16;
   static constexpr uint16_t PacketMemorySize = 2048;
   static constexpr uint16_t Ep0MaxPacketSize = 64;
   static constexpr uint16_t BulkMaxPacketSize = 512;
   static constexpr uint16_t InterruptMaxPacketSize = 1024;

   enum class Speed : uint8_t {
      Full,
      High,
   };

   /**
    * @param endpointCount The highest endpoint number that the device uses,
    * plus 1 (1..EndpointCount). openEndpoint() refuses higher numbers.
    * @param maxSpeed High: the controller does the high-speed handshake
    * and falls back to full speed on a full-speed host. Full: full speed
    * only.
    */
   explicit UsbDevice(uint8_t endpointCount = EndpointCount, Speed maxSpeed = Speed::High);
   UsbDevice(const UsbDevice &) = delete;
   UsbDevice &operator=(const UsbDevice &) = delete;

   /**
    * @brief Starts the USB PHY PLL and the USBHS clock, resets the
    * controller, connects the interrupt and enables it in the PFIC. The
    * device stays disconnected (no D+ pull-up) until connect().
    */
   void begin(usb::EventCallback callback, void *context);

   /**
    * @brief Disconnects, disables the interrupt, resets the controller and
    * stops its clock and the PHY PLL.
    */
   void end();

   // Internal D+ pull-up on/off (CTRL.DEV_PU_EN): the host sees the device
   // attach/detach.
   void connect();
   void disconnect();
   bool isConnected() const;

   /**
    * @brief The speed that the host selected. Valid after the first bus
    * reset. The layer above uses it for the descriptors: a high-speed bulk
    * endpoint has 512-byte packets, a full-speed one 64 or fewer.
    */
   Speed speed() const;

   /**
    * @brief Writes the device address. Call it after the status stage of
    * SET_ADDRESS, not before (USB 2.0 section 9.4.6).
    */
   void setAddress(uint8_t address);

   /**
    * @brief Opens one direction of an endpoint and takes its buffer from
    * the pool. The endpoint starts with data toggle DATA0 and answers NAK.
    * The two directions of one endpoint number can have different types.
    * @return false for endpoint number >= the endpointCount of the
    * constructor, isochronous type, control type on an endpoint other than
    * 0, maxPacketSize 0 or above the limit of the type (control 64, bulk
    * 512, interrupt 1024), doubleBuffered true on an IN or non-bulk
    * endpoint, doubleBuffered true while the IN direction of that number
    * is open (or the reverse: IN while the OUT direction is double-buffered),
    * a direction that is already open with another maxPacketSize or
    * doubleBuffered value (call closeAllEndpoints() first), or not enough
    * pool memory. A double-buffered endpoint takes two buffers from the
    * pool. Reopening EP0 with 64 bytes returns true.
    */
   [[nodiscard]] bool openEndpoint(uint8_t address, usb::EndpointType type, uint16_t maxPacketSize, bool doubleBuffered = false);

   /**
    * @brief Disables every endpoint except EP0 and frees the pool. For
    * SET_CONFIGURATION: close all, then open the new ones.
    */
   void closeAllEndpoints();

   /**
    * @brief Copies one packet into the IN buffer and lets the endpoint
    * answer the next IN token with it. data.size() 0 sends a zero-length
    * packet. On EP0 this ends a STALL.
    * @return false if the endpoint is not open, data is larger than
    * maxPacketSize, or the previous packet is not sent yet.
    */
   [[nodiscard]] bool startIn(uint8_t address, hw::span<const uint8_t> data);

   /**
    * @brief Lets an OUT endpoint accept the next packet. On EP0 this ends
    * a STALL. Double-buffered: frees the buffer of the reported packet. If
    * the next packet is already in the other buffer, it reports that packet
    * (Out event) before it returns.
    * @return false if the endpoint is not open.
    */
   bool armOut(uint8_t address);

   /**
    * @brief Copies the last received OUT packet (EP0: from the RAM copy).
    * @return the number of bytes copied: min(received size, buffer.size()).
    */
   uint16_t readOut(uint8_t address, hw::span<uint8_t> buffer) const;

   /**
    * @brief Sets or clears STALL on one direction of an endpoint. Clearing
    * STALL sets NAK and resets the data toggle to DATA0 (CLEAR_FEATURE
    * ENDPOINT_HALT, USB 2.0 section 9.4.5). Both drop a pending startIn()
    * or armOut(). A STALL on EP0 ends with the next SETUP packet.
    */
   void setStall(uint8_t address, bool stall);
   bool isStalled(uint8_t address) const;

   /**
    * @brief Sets NAK on one direction of an endpoint. This drops a pending
    * startIn() or armOut().
    */
   void setNak(uint8_t address);

   /**
    * @brief The interrupt handler. begin() connects it to USBHS_IRQn
    * through Interrupt::connectMemberHandler; do not call it directly.
    */
   void handleInterrupt();

   private:
   static constexpr uint8_t NoBuffer = 0xFF;

   struct Direction {
      uint8_t *buffer = nullptr;
      uint16_t maxPacketSize = 0;
      uint16_t received = 0; // OUT: size of the last reported packet
      bool open = false;
      // Double-buffered OUT only. buffer is buffer 0, buffer1 buffer 1.
      uint8_t *buffer1 = nullptr;
      uint16_t received1 = 0; // size of the packet in buffer 1
      bool doubleBuffered = false;
      bool armed = false;
      uint8_t reported = NoBuffer; // the buffer that the software reads (readOut())
      uint8_t pending = NoBuffer;  // a received packet that waits for armOut()
   };

   struct Endpoint {
      Direction in;
      Direction out;
   };

   usb::EventCallback _callback = nullptr;
   void *_context = nullptr;
   Endpoint _endpoints[EndpointCount] = {};
   uint8_t _endpointCount;
   Speed _maxSpeed;
   uint16_t _poolNext = 0;
   alignas(4) uint8_t _ep0Buffer[Ep0MaxPacketSize] = {}; // DMA buffer of EP0, IN and OUT
   alignas(4) uint8_t _pool[PacketMemorySize] = {};      // DMA buffers of endpoints 1..15
   uint8_t _ep0Out[Ep0MaxPacketSize] = {};               // Copy of the last EP0 OUT packet

   void resetEndpoints();
   void report(usb::EventType type, uint8_t endpoint, uint16_t size = 0);
   void reportDoubleBuffered(uint8_t n, uint8_t buffer);
   void resetDoubleBuffer(uint8_t n);
   void handleTransfer(uint8_t status, bool setupPending);
   Direction *direction(uint8_t address);
   const Direction *direction(uint8_t address) const;
};

} // namespace hw
