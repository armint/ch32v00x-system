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
| `CH32V30X` | `CH32V30x_D8C` | CH32V307 | Start-up, clock and GPIO build. Hardware test not recorded. Other drivers not ported yet. |

On CH32V30X, the start-up, the clock, SysTick, the delays, `Interrupt` and
`GpioPin` for pins 0 to 7 build. `GpioPin` pins 8 to 15 do not work yet
([issue #6](https://gitlab.vandertogt.org/riscv/ch32v00x-system/-/work_items/6)).
The USART, DMA, timer, EXTI, I2C and flash drivers are not ported yet. The family list is
in [Chip families](docs/families.md).

## Drivers

| Area | Drivers |
|---|---|
| Start-up | Reset handler, weak exception handlers, vector table per family |
| System timer and delays | `initSysTick()` and `getSysTick()` (your project sets the tick rate), `delaymS()` and `delayuS()` |
| GPIO and EXTI | `GpioPin`, `Exti` |
| Communication | `Usart`, `I2c` |
| Analog and timing | `Adc` (CH32V00X only), `Timer`, `Dma` |
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
| `src/` | Shared start-up code, shared drivers and utilities. |
| `src/peripherals/` | Drivers that more than one family shares. |
| `src/families/<family>/` | Family-specific code. CMake adds only the selected family to the include path. |
| `src/utils/` | `Interrupt` (interrupt binding) and the delay functions. |
| `ldscripts/` | Linker scripts, one memory script per chip. |
| `cmake/` | The RISC-V toolchain file, the package configuration and helper modules for the examples build. |
| `etc/svd/` | SVD register descriptions, for reference and debug tools. |
| `etc/openocd.cfg` | OpenOCD configuration for the WCH-LinkE debug adapter. It needs the WCH fork of OpenOCD. |

## Documentation

| Document | Contents |
|---|---|
| [Building and using ch32v00x-system](docs/building.md) | Requirements, CMake variables, compiler flags, installed package |
| [Chip families](docs/families.md) | `CH32_FAMILY`, family directories, device-specific code, adding a family |
| [ADC calibration on CH32V002 and CH32V006](docs/adc.md) | Calibration of the ADC on these chips |

## Contributing

All changes go to the `main` branch through merge requests. This includes
bug fixes. Every merge request needs a review by someone other than its
author. The project has no releases yet. Only the tag `v0.0.0` exists.
Use a commit of `main` in your submodule.
