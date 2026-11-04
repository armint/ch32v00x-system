# USBHS loopback test on CH32V307

This program tests `hw::UsbDevice` on a CH32V307 board. It has two parts:

- Enumeration. The device has one vendor interface with the name
  `CMSIS-DAP v2`, one bulk OUT endpoint (`0x01`) and one bulk IN endpoint
  (`0x81`). This is the interface shape of a CMSIS-DAP v2 probe.
- Bulk loopback. The device sends each bulk OUT packet back unchanged on
  the bulk IN endpoint.

The program uses the test ID 1209:0001 of pid.codes. Do not use this ID in
a product. The program has only the standard requests that Linux needs for
enumeration. It has no Microsoft OS descriptors, thus Windows does not bind
WinUSB to it. The firmware uses the libArT USB stack instead of this code.

## Requirements

- A CH32V307 board with an 8 MHz crystal on HSE.
- The USB connector on PB6 (D-) and PB7 (D+), the USBHS pins.
- A Linux host with Python 3 and pyusb.
- Read and write access to the device, for example through a udev rule.

## Build

Run these commands from the repository root:

```sh
cmake -S tests/ch32v30x-usb-loopback -B /tmp/usb-loopback \
    -D CMAKE_TOOLCHAIN_FILE=cmake/gcc_riscv.cmake \
    -D GCC_TOOLCHAIN_PREFIX=/path/to/riscv-toolchain \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_C_FLAGS="-DCH32V30x_D8C -march=rv32imafc_zicsr_zifencei -mabi=ilp32f -Os -ffunction-sections -fdata-sections" \
    -D CMAKE_CXX_FLAGS="-DCH32V30x_D8C -march=rv32imafc_zicsr_zifencei -mabi=ilp32f -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti"
cmake --build /tmp/usb-loopback
```

The build writes `usb-loopback.elf` and `usb-loopback.hex`.

## Test

1. Write `usb-loopback.hex` to the board, for example with the WCH-LinkE
   adapter and `etc/openocd.cfg`.
2. Connect the USBHS connector to the host.
3. Run `lsusb -v -d 1209:0001`. The output shows the device descriptor and
   the interface string `CMSIS-DAP v2`.
4. Run `python3 tests/ch32v30x-usb-loopback/loopback.py`.

The script shows the speed and the bulk packet size. A high-speed port
gives 512 bytes, a full-speed port gives 64 bytes. The script then sends
200 packets of different sizes. It shows `PASS` when each packet comes
back unchanged.

Do the test on a high-speed port. Then do it again behind a full-speed
hub, if one is available.
