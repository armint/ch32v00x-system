# USB device driver

The class `hw::UsbDevice` operates the USBHS controller of the CH32V307 in
device mode. USBHS is the USB 2.0 controller with the internal high-speed
PHY (physical layer). The driver is in `src/families/ch32v30x/UsbDevice.h`.
Only the `CH32V30X` family has this driver.

## Layers

The driver works on single packets. It does not process standard requests
or descriptors. A USB stack above the driver does this work, for example
the `art::usb::Device` class of
[libArT](https://gitlab.vandertogt.org/embedded-libraries/libart).

The public interface is the same as the interface of `hw::UsbDevice` in
system-stm32. The shared header `src/peripherals/UsbDeviceTypes.h` has the
same types (`hw::usb::Event`, `hw::usb::EndpointType` and the endpoint
address helpers). Thus a controller port for the libArT stack needs few
changes.

These items are different from the system-stm32 driver:

- The endpoint numbers are 0 to 15.
- A bulk endpoint has packets of up to 512 bytes. An interrupt endpoint
  has packets of up to 1024 bytes.
- The constructor takes the maximum speed, `Speed::High` or `Speed::Full`.
- `speed()` gives the speed that the host selected. Read it after the first
  bus reset. The descriptors must agree with it. At high speed, a bulk
  endpoint must have 512-byte packets.
- The D+ pull-up is internal. The driver has no pull-up callback.
- `openEndpoint()` refuses isochronous endpoints and double-buffered IN
  endpoints. Double-buffered bulk OUT endpoints are supported (see
  [Double-buffered OUT](#double-buffered-out)).

## Hardware use

| Item | Use |
|---|---|
| Clock | `begin()` starts the USB PHY PLL from HSE. The PLL input is 4 MHz, thus `HSE_VALUE` must be a multiple of 4 MHz, from 4 MHz to 32 MHz. HSE must run. |
| Pins | PB6 (D-) and PB7 (D+). They need no GPIO configuration. |
| Interrupt | `USBHS_IRQn`. `begin()` connects the handler with `Interrupt::connectMemberHandler`. |
| RAM | The object holds the endpoint buffers: 64 bytes for EP0 and a pool of 2048 bytes for endpoints 1 to 15. Each buffer takes the packet size, rounded up to 4 bytes. |

## Sequence

1. Make a `hw::UsbDevice` object with static storage.
2. Call `begin()` with the event callback.
3. Call `connect()`. The host then sees the device.
4. Process the events of the callback in the USB stack.

The callback runs in the interrupt. Copy each event into a queue and
process it in the main loop.

## Endpoint behavior

- An OUT endpoint answers NAK until `armOut()`. After each received
  packet, the endpoint answers NAK again. The data stays in the endpoint
  buffer until the next `armOut()`.
- An IN endpoint answers NAK until `startIn()`. After the host
  acknowledges the packet, the endpoint answers NAK again.
- `openEndpoint()` on a direction that is open with the same attributes
  (the open of SET_INTERFACE) sets DATA0 and NAK, and drops a packet that
  waits. EP0 is not touched.
- `setNak()` returns true when the hardware has completed a transfer of that
  direction that the handler has not reported yet. The controller layer uses
  it to decide if a cancelled IN packet was taken.
- After a SETUP packet, both directions of EP0 answer NAK.
- The hardware sets the data toggle of endpoints 1 to 15. EP0 has no
  automatic toggle, thus the driver sets it for EP0.
- An OUT packet with the wrong data toggle is a repeated packet. The
  driver ignores it, and the endpoint stays armed.

## Double-buffered OUT

`openEndpoint(address, Bulk, size, true)` opens a bulk OUT endpoint with two
buffers (RM 22.2.2.3, table 21-3). The hardware receives the next packet into
the second buffer while the software reads the first. The interface is the
same as in system-stm32: one Out event for each `armOut()`, and `readOut()`
reads that packet. A packet that arrives before `armOut()` waits in its
buffer, and `armOut()` reports it.

- The endpoint takes two buffers from the pool. Buffer 0 is `UEPn_RX_DMA`,
  and buffer 1 is `UEPn_TX_DMA`. Thus the IN direction of that endpoint
  number cannot open.
- The data toggle bit `RX_TOG[0]` selects the buffer, and the hardware flips
  it after each packet. The hardware does not NAK when both buffers are full,
  so the interrupt handler sets NAK then, and `armOut()` sets ACK again.
- `setNak()` and `setStall()` do a read-modify-write of the control byte
  of the endpoint. A packet that the hardware completes in between can lose
  its toggle flip. The driver keeps the toggle of each endpoint 1 to 15 in
  software and writes it back in the interrupt handler, so the loss does not
  stay. The software copy is also the buffer that receives the next packet.
- Clearing STALL, and opening an open endpoint again (SET_INTERFACE), drop
  both buffers and select buffer 0 again.
- The bit position of the endpoint in `UEP_BUF_MOD` (bit n for endpoint n)
  follows the layout of `UEP_CONFIG`. The manual does not show the bit
  numbers. The host test checks the driver logic with a model of the
  hardware. The hardware test must confirm the bit position and the buffer
  order.

## Hardware test

The [`usb-loopback`](https://gitlab.vandertogt.org/riscv/ch32v00x-examples/-/tree/main/examples/usb-loopback) example of ch32v00x-examples tests the driver on a
CH32V307 board. It enumerates with a vendor interface named
`CMSIS-DAP v2` and sends each bulk OUT packet back on the bulk IN
endpoint. Its README gives the build and test steps.

## Known limits

- The hardware test is not done. Enumeration on a host and a bulk loopback
  are not confirmed.
- The reference manual does not say if EP0 accepts a SETUP packet while
  EP0 answers NAK or STALL. The driver expects this behavior. The driver
  ends a STALL on EP0 only when the next SETUP packet arrives. The hardware
  test must confirm both cases.
- The `INT_ST` and `RX_LEN` registers are global for all endpoints.
  `INT_ST` holds the token and the endpoint of the last transfer. `RX_LEN`
  holds the length of the last received packet. A SETUP packet can change
  both registers before the interrupt handler reads them for a transfer of
  endpoints 1 to 15. The time window is the interrupt entry time. These are
  the effects:
  - The handler sees a SETUP token, and the IN or OUT event is lost.
  - After a lost IN event, the IN endpoint stays ready. It sends the same
    packet again, and the driver reports no `InComplete` event. A USB stack
    that waits for this event stops.
  - A wrong `RX_LEN` value gives a wrong length for an OUT packet.

  The reference manual does not say if the controller holds these
  registers while a transfer interrupt waits. The hardware test does not
  prove it either, because the window is small.
- The vendor SVD file names bits 5 to 7 of `USB_INT_EN` differently from
  the reference manual. The header follows the reference manual.
