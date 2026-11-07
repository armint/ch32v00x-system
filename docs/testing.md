# Testing

## Host tests

Host tests run on the build machine. They do not need a board or the RISC-V toolchain.
They compile library sources with the native compiler (C++23).
A stub `wch_device.h` replaces the device header.
The stub gives the registers of one family as plain variables in memory.
The GPIO tests use `stubs-<family>/`. The USB test uses `stubs-usb-v307/`. The SysTick test uses `stubs-systick-v30x/`. The Usart test uses `stubs-usart-v30x/`. The ADC test uses `stubs-adc-v30x/`. The AFIO and EXTI test uses `stubs-exti-v30x/`. The Flash test uses `stubs-flash-v30x/`. The Rcc test uses `stubs-rcc-v30x/`. The Dma test uses `stubs-dma-v30x/`. The Spi test uses `stubs-spi-v30x/`.

Run the tests:

```
cmake -S tests/host -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The build uses `-Wall -Wextra -Werror`. Each test program returns a non-zero exit status when a check fails.

### What the tests check

| Program | Checks |
|---|---|
| `test-gpio-v00x` | `hw::GpioPin` on CH32V00X, pins 0 to 7: the 4-bit field in `CFGLR` that `configure()` writes for each mode, the pull-up and pull-down writes to `BSHR` and `BCR`, the setters (`setMode()`, `setOutputType()`, `setSpeed()`, `setPull()`), and `write()`, `toggle()` and `read()`. |
| `test-gpio-v20x` | The same checks on CH32V20X, pins 0 to 15. Pins 0 to 7 use `CFGLR`. Pins 8 to 15 use `CFGHR`. |
| `test-gpio-v30x` | The same checks on CH32V30X, pins 0 to 15. |
| `test-timer-v00x`, `test-timer-v20x`, `test-timer-v30x` | `hw::Timer` with the TIM layout of each family: the register and the bit that each function writes, `CH1CVR` to `CH4CVR` 4 bytes apart, the preload bit of channels 2 and 4, and a flag clear that writes the inverted mask and does not read `INTFR`. It does not test the counter or a DMA request. |
| `test-usart-v30x` | The baud rate of `hw::Usart` on CH32V30X: USART1 uses PPRE2 and the other USARTs use PPRE1. It also checks `getClockFrequency()` and `setFrameFormat()` (the M, PCE and PS bits, the stop bits, the refusal of 7 data bits without parity, and that a USART that is off stays off). |
| `test-flash-v30x` | The shared `Flash` driver with the CH32V30X registers. Models of `FLASH_KEYR`, `FLASH_MODEKEYR`, `FLASH_CTLR` and `FLASH_STATR` check the unlock and lock sequence (no key write while unlocked, a second unlock after a lock), HCLK = SYSCLK / 2 during an operation above 120 MHz and the old `RCC_CFGR0` after it, `program_halfword()` (odd address, write protection error), `erase_page()` and the padding of `write_buffer()`. It does not check the timing, the erased value or the read-back. |
| `test-dma-v30x` | `Dma::Channel` with the CH32V30X registers. The channel registers of channels 1 to 11 have the offsets of the RM. The interrupt flags of channels 1 to 7 are in `INTFR` and `INTFCR`. The flags of channels 8 to 11 are in the extend registers. It does not test a transfer. |
| `test-adc-v30x` | The CH32V30X `Adc` driver: the sequence registers, the sample time registers, the prescaler, the clock and reset bits of ADC1 and ADC2, and the trigger. It does not test `calibrate()`. |
| `test-rcc-v30x` | The CH32V30X `Rcc` driver: each clock enable sets its bit in the register of the correct bus and changes no other bit. Each reset function leaves its reset bit clear and changes no other bit. `getAPBClock()` and `getAPB2Clock()` follow PPRE1 and PPRE2. The test sees only the end state of a register: it does not check that a reset function sets the bit before it clears it. |
| `test-spi-v30x` | `hw::Spi` with the CH32V30X bits: the `CTLR1` and `CTLR2` values of `configure()` (the settings of the JTAG fast path of the firmware), `SSI` for a master and a slave with `NssMode::Soft`, the other NSS modes, the prescaler field, the timeouts of `transfer()` and `waitUntilIdle()`, and the bus clock of SPI1 (PCLK2) and SPI3 (PCLK1). It does not test a transfer. |

Each test starts with a known pattern in the configuration registers.
It checks the field of the selected pin.
It also checks that the fields of all other pins stay the same.
On CH32V00X, it checks that the reserved word after `CFGLR` stays the same.

`test-exti-afio-v30x` tests `hw::Afio`, `Exti` and `GpioInterrupt` of CH32V30X
(see [GPIO, AFIO and EXTI](gpio.md)). It checks these items:

- The `AFIO_PCFR1` writes of the remap functions and of `setSwjConfig()`.
  Other fields stay the same.
- The `AFIO_EXTICRx` register and the 4-bit field of each line.
  A line of 16 or more writes nothing.
- The `RTENR` and `FTENR` bits of each trigger edge, the `INTENR` and `EVENR` bits, and the `INTFR` write that clears one flag.
- `GpioInterrupt`: the routing, the edge and the enable bits of a connected pin.
  A second port on the same line number is refused.
  The dispatch code calls the handler of the pending line only, and not the handler of a masked line.
  The dispatch code does not clear the flag of a masked line. `GpioInterrupt` does not clear a PFIC pending bit.
  A shared interrupt stays on until the last line disconnects.

The test does not model the write-1-to-clear behavior of `INTFR`: it checks the value that the code writes.
The test also does not check the PFIC priority or the vector entry.

`test-systick-v30x` tests the time base of CH32V30X (`src/Timekeeping.cpp`). The test plays the hardware: it sets the counter to the value at the match and calls `SysTick_Handler()`. It checks these items:

- `SysTick_Config()` starts a free-running counter: STCLK, STIE and STE are set, STRE (auto reload) is not, the counter is 0 and the compare value is one period.
- Each match counts one tick and moves the compare value forward by one period.
- An interrupt that is late by more than one period counts the periods that passed, and the compare value is ahead of the counter again.
- A second interrupt for one match, and a match a few cycles before the compare value, count correctly.
- The tick count and the compare value stay right across the carry of the low word at 2^32 cycles.
- `hw::CycleCounter`: `count()` returns the low word of the counter, and a difference of two counts is correct across the wrap. `start()` does not change a counter that `SysTick_Config()` started. `start()` sets STCLK and STE on a stopped counter.

The test does not check that the hardware counts past the compare value without auto reload, or the compare rule (equal, or greater or equal). The `clock-time` example of `ch32v00x-examples` checks them.

`test-usb-v307` tests the USBHS device driver of CH32V307
(`src/families/ch32v30x/UsbDevice.cpp`, see [USB device driver](usb.md)).
It checks these items:

- The `begin()`, `end()`, `connect()` and `disconnect()` writes to the USBHS and RCC registers.
  These include the PHY PLL divider for `HSE_VALUE` and the SIE reset during the delay.
- The device address mask of `setAddress()` and the decode of `SPEED_TYPE`.
- The open rules of `openEndpoint()`. The test opens endpoints at the limits of the endpoint count and the packet size.
  Only EP0 accepts the control type. A second open with the same packet size succeeds, and a second open with a different size fails.
- The rejection of isochronous and double-buffered endpoints.
  A rejected call does not change a register.
- The buffer pool: each buffer is 4-byte aligned and starts after the previous buffer.
  A full pool refuses a new endpoint. `closeAllEndpoints()` frees the pool.
- The register values that `openEndpoint()` writes: the DMA address, `MAX_LEN`, `TX_CTRL`, `RX_CTRL` and `UEP_CONFIG`.
- The packet copy and the `T_LEN` and `TX_CTRL` values of `startIn()`, and the `RX_CTRL` value of `armOut()`.
- The reopen of an open endpoint: DATA0, NAK, and no waiting packet.
- The software copy of the data toggle: a flip that the main loop undid is written back by the handler, and a STALL that the main loop set stays.
- A transfer that waits for the handler is reported inside `setNak()`, `setStall()` and the reopen.
- The `TX_CTRL` and `RX_CTRL` values of `setStall()` and `setNak()`.
  When STALL stops, the toggle bits are DATA0. On endpoints 1 to 15, the auto-toggle bit stays set.

The driver writes 32-bit DMA addresses.
On a 64-bit host, the address is the low half of the pointer.
The test uses the offset of the buffer in the `UsbDevice` object.

### What the tests do not check

- Real hardware behavior. A fake register does not model reset values, write-protect or side effects.
- The device header itself. The stub copies the register layout from the family header. A change to the header layout does not change the stub.
- The critical section of `GpioPin`. The host stub does nothing.
- The USB interrupt handler. A fake USBHS register holds the last written value.
  It does not model the write-1-to-clear bits of `INT_FG`.
  It does not model the response change after a transfer.
  It does not model the automatic data toggle of endpoints 1 to 15.
  Thus the test does not check the `Setup`, `Out` and `InComplete` events, or the EP0 toggle after a transfer.
  It also does not check the data that `readOut()` copies after a received packet.
- USB interrupt timing: `INT_BUSY`, and the shared `INT_ST` and `RX_LEN` registers (see [USB known limits](usb.md#known-limits)).
  The [`usb-loopback`](https://gitlab.vandertogt.org/riscv/ch32v00x-examples/-/tree/main/examples/usb-loopback) example of ch32v00x-examples uses the interrupt handler.
  No test proves the `INT_ST` and `RX_LEN` race, because the time window is small.

### Add a test

1. Add the test source to `tests/host/`. Include `check.h` and use the `CHECK` macro.
2. Add a stub directory `tests/host/stubs-<family>/` if the test needs registers of a new family.
3. Add the program and an `add_test` entry to `tests/host/CMakeLists.txt`.

## Test images

A test image is a small program that links the library and a minimal `main()`.
It shows that the code builds, links and fits in memory.
It does not run in CI.

| Image | Family | Content |
|---|---|---|
| `tests/ch32v30x/` | CH32V30X | `main()` sets up PB4 as output (blue LED) and toggles it in a loop. It also connects a pin interrupt and sets the debug port, so that CI compiles these. |

Build the image:

```
cmake -B build \
    -D CMAKE_TOOLCHAIN_FILE=cmake/gcc_riscv.cmake \
    -D GCC_TOOLCHAIN_PREFIX=/path/to/riscv-toolchain \
    -D CH32_FAMILY=CH32V30X \
    -D CH32V00X_SYSTEM_BUILD_TEST_IMAGE=ON \
    -D "CMAKE_C_FLAGS=-DCH32V30x_D8C -march=rv32imafc_zicsr_zifencei -mabi=ilp32f" \
    -D "CMAKE_CXX_FLAGS=-DCH32V30x_D8C -march=rv32imafc_zicsr_zifencei -mabi=ilp32f"
cmake --build build --target test-image-v307
```

The option `CH32V00X_SYSTEM_BUILD_TEST_IMAGE` is `OFF` by default.
CMake stops with an error when you set it for a family other than CH32V30X.

The `build-v307-test-image` CI job checks the vector table.
The flash starts at address `0x0`. `Reset_Handler` must be at `0x0`
and `g_pfnVectors` at `0x4`. The job fails if an address is different.
The job also fails if the image has a separate `.sbss` section, because the start-up code clears only the `.bss` range.

The image does not test the USB driver.

## Continuous integration

The file `.gitlab-ci.yml` defines the GitLab pipeline. All jobs run in the
`build` stage. The build jobs use the builder image that `.gitlab-ci.yml`
names. The image has the RISC-V toolchain. Each build job looks for
`riscv-none-elf-gcc` with `command -v`. The job fails if the tool is missing.

| Job | What it does |
|---|---|
| `build` | Builds the `ch32v00x-system` library at C++23 for CH32V003, CH32V006, CH32V20X and CH32V30X. The job fails if CMake fetches span-compat. |
| `build-cpp17` | Builds the same four devices at C++17. The job fails if CMake does not use `span_compat::span`. |
| `host-tests` | Runs the host tests from `tests/host` with the native compiler. |
| `build-v307-test-image` | Links the [CH32V307 test image](#test-images) and checks the address of the vector table and that `.sbss` is inside `.bss`. |

The build jobs use the device macro, `-march` and `-mabi` values of
[Compiler flags per chip](building.md#compiler-flags-per-chip). The CH32V00X
jobs use CH32V003 and CH32V006. Only CH32V006 compiles the ADC driver of
CH32V006 and CH32V002. The CH32V20X jobs use CH32V20x_D8W. The CH32V30X jobs
use CH32V30x_D8C.
