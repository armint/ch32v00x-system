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
- `openEndpoint()` refuses double-buffered endpoints and isochronous
  endpoints.

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
- After a SETUP packet, both directions of EP0 answer NAK.
- The hardware sets the data toggle of endpoints 1 to 15. EP0 has no
  automatic toggle, thus the driver sets it for EP0.
- An OUT packet with the wrong data toggle is a repeated packet. The
  driver ignores it, and the endpoint stays armed.

## Known limits

- The hardware test is not done. Enumeration on a host and a bulk loopback
  are not confirmed.
- The reference manual does not say if EP0 accepts a SETUP packet while
  EP0 answers NAK. The driver expects this behavior. The hardware test must
  confirm it.
- The `RX_LEN` register holds the length of the last received packet for
  all endpoints. A SETUP packet can change it before the interrupt handler
  reads the length of a bulk OUT packet. The time window is the interrupt
  entry time.
- The vendor SVD file names bits 5 to 7 of `USB_INT_EN` differently from
  the reference manual. The header follows the reference manual.
