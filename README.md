# ch32v00x-system

ch32v00x-system is a C++ hardware layer for WCH CH32V microcontrollers. It
contains the start-up code, the interrupt vector tables and register-level
peripheral drivers. It contains no code from the WCH peripheral library.
It uses the WCH register header files and the WCH system files
(`system_*.c` and `core_riscv.c`).

## Purpose

The library is the lowest layer of a set of bare-metal firmware projects.
It gives these projects one C++ interface to the hardware:

- Small, direct drivers. Many functions are inline register accesses. The
  library uses no heap and no exceptions.
- One code base for several chip families. CMake selects the family code at
  configure time with `CH32_FAMILY`. Family-specific code goes into
  `src/families/`.
- Drivers that follow the hardware. When a family has a different
  peripheral, its driver exposes the functions of that peripheral.

Higher layers are not in this library. The stream classes that connect
[libArT](https://gitlab.vandertogt.org/embedded-libraries/libart) to this
library are in [ch32v00x-art](https://gitlab.vandertogt.org/riscv/ch32v00x-art).

## Supported chips

| `CH32_FAMILY` | Device macro | Chips | Status |
|---|---|---|---|
| `CH32V00X` | `CH32V003` | CH32V003 | Builds. Hardware test not recorded. |
| `CH32V00X` | `CH32V002` | CH32V002 | Builds. Hardware test not recorded. |
| `CH32V00X` | `CH32V006` | CH32V006 | Builds. Hardware test not recorded. |
| `CH32V20X` | `CH32V20x_D8W` | CH32V208 | Builds. Hardware test not recorded. No ADC driver. |
| `CH32V30X` | `CH32V30x_D8C` | CH32V307 | Start-up, clock, GPIO (with AFIO and EXTI), USART, ADC, DMA, timer and USB device build. Hardware test of ADC, DMA and timer recorded (see below). Other drivers not ported yet. |

On CH32V30X, the start-up, the clock, SysTick, the delays, `Interrupt` and
`GpioPin` build. `GpioPin` sets pins 0 to 7 in `CFGLR` and pins 8 to 15 in `CFGHR`.
The `hw::Usart` driver builds for USART1 to USART3 and UART4 to UART8. A loopback test on hardware is not done yet.
The baud rate uses the APB prescaler: USART1 follows PPRE2, the other USARTs follow PPRE1. Call `setBaudRate()` again after a prescaler change.
The USB device driver `hw::UsbDevice` builds for the USBHS controller. A test on a host is not done yet.
The `Adc` driver builds for ADC1 and ADC2.
The `Dma` driver builds for DMA1 and the `Timer` driver for TIM1 to TIM10.
On the YD-CH32V307VCT6 board, the `adc-dma` and `timer` examples of [ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples) work.
`adc-dma` uses ADC1 with DMA1 channel 1. `timer` uses TIM2 channel 1 on PA15 and TIM3 channel 1 on PB4, through the AFIO remaps.
The I2C and flash drivers are not ported yet. The family list is
in [Chip families](docs/families.md).

## Drivers

| Area | Drivers |
|---|---|
| Start-up | Reset handler, weak exception handlers, vector table per family, the weak hooks `os_startup_initialize_hardware_early()` (calls `SystemInit()`) and `os_startup_initialize_hardware()`, and the device header `cmsis_device.h` |
| System timer and delays | `SysTick_Config()` and `getUptimeTicks()` in `utils/Timekeeping.h` (your project sets the tick rate), `delaymS()` and `delayuS()` in `delaybusywait.h`. On CH32V20X and CH32V30X, `SysTick->CNT` is also a free-running 32-bit core-clock cycle counter (see `utils/Timekeeping.h`) |
| GPIO and EXTI | `hw::GpioPin` and `hw::FastPin` (the API of `system-stm32`), `Exti`, and on CH32V30X also `hw::Afio` and `GpioInterrupt`. See [GPIO](docs/gpio.md). |
| Communication | `hw::Usart`, `I2c`, `hw::UsbDevice` (CH32V30X only, USB device on USBHS) |
| Chip ID | `hw::Uid` in `peripherals/Uid.h`: the 96-bit unique ID (CH32V00X, CH32V20X, CH32V30X) |
| Analog and timing | `Adc` (CH32V00X and CH32V30X), `Timer`, `Dma` |
| Other | `Flash`, `Interrupt` (connects C++ member functions as interrupt handlers) |

## Quick start

Add the repository as a git submodule. Then add it to your CMake project:

```cmake
add_subdirectory(dependencies/ch32v00x-system)
target_link_libraries(${PROJECT_NAME} PRIVATE ch32v00x-system)
```

Set `CH32_FAMILY` and the device macro of your chip. Use the RISC-V
toolchain file `cmake/gcc_riscv.cmake`. For all options, see
[Building and using ch32v00x-system](docs/building.md).

## Repository layout

| Path | Contents |
|---|---|
| `src/` | Shared drivers, the start-up hooks, `syscalls.c` and the hardware initialization. |
| `src/bare-metal/` | Start-up code (`startup.cpp`) and the `assert` handler. |
| `src/peripherals/` | Drivers that more than one family shares. |
| `src/families/<family>/` | Family-specific code. CMake adds only the selected family to the include path. |
| `src/utils/` | `Interrupt` (interrupt binding), `hw::span` and the delay functions. |
| `ldscripts/` | Linker scripts, one memory script per chip. |
| `cmake/` | The RISC-V toolchain file, the package configuration and helper modules for the examples build. |
| `svd/` | SVD register descriptions, for reference and debug tools. |
| `tests/host/` | Host tests with fake registers. They run with `ctest` and need no board. See [Testing](docs/testing.md). |
| `tests/` | Checks of the library. `check-svd-offsets.py` compares the CH32V30x device header with the SVD. `ch32v30x/` is a test image that CI links, see [Testing](docs/testing.md#test-images). The hardware test of the USB device driver is the [`usb-loopback`](https://gitlab.vandertogt.org/riscv/ch32v00x-examples/-/tree/main/examples/usb-loopback) example of ch32v00x-examples. |
| `etc/openocd.cfg` | OpenOCD configuration for the WCH-LinkE debug adapter. It needs the WCH fork of OpenOCD. |

## Documentation

| Document | Contents |
|---|---|
| [Building and using ch32v00x-system](docs/building.md) | Requirements, CMake variables, C++ standard, `hw::span`, compiler flags, installed package |
| [Chip families](docs/families.md) | `CH32_FAMILY`, family directories, device-specific code, adding a family |
| [ADC](docs/adc.md) | Calibration on CH32V002 and CH32V006. The ADC of CH32V30X |
| [GPIO, AFIO and EXTI](docs/gpio.md) | Which GPIO API each family has, `hw::Afio`, `Exti`, `GpioInterrupt` |
| [Testing](docs/testing.md) | Host tests: how to run them, what they check and what they do not check |
| [USB device driver](docs/usb.md) | `hw::UsbDevice` on the USBHS controller of CH32V307: layers, clock, endpoint behavior, known limits |

## Contributing

All changes go to the `main` branch through merge requests. This includes
bug fixes. Every merge request needs a review by someone other than its
author. The project has no releases yet. Only the tag `v0.0.0` exists.
Use a commit of `main` in your submodule.

## License

ch32v00x-system uses the MIT license. See [LICENSE](LICENSE).
Third-party files keep the copyright and the terms in their file headers:

- The WCH device files, for example `src/core_riscv.h` and the CH32V00x and
  CH32V20x headers in `src/families/`, come from the WCH example packages.
  They keep the WCH copyright and terms.
- The start-up code and the vector tables come from µOS++ by Liviu Ionescu
  (MIT), for example `src/bare-metal/startup.cpp` and
  `src/families/*/vectors_*.c`.
- `svd/CMSIS-SVD.xsd` is from Arm (Apache-2.0).

The files in `svd/vendor/` come from WCH (MounRiver Studio V182). The files in
`svd/fixed/` are changed copies of them. These files have no license header.
The MIT license of this repository does not apply to them.
