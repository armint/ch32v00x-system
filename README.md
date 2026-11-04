# ch32v00x-system

ch32v00x-system is a C++ hardware layer for WCH CH32V microcontrollers. It
contains the start-up code, the interrupt vector tables and register-level
peripheral drivers. It contains no vendor HAL code. It uses only the vendor
register header files.

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

Higher layers are not in this library. For example, the `libArT` utilities
and the stream classes are in `ch32v00x-art`.

## Supported chips

| `CH32_FAMILY` | Device macro | Chips | Status |
|---|---|---|---|
| `CH32V00X` | `CH32V003` | CH32V003 | Builds. |
| `CH32V00X` | `CH32V006` | CH32V002, CH32V006 | Builds. |
| `CH32V20X` | `CH32V20x_D8W` | CH32V208 | Builds. No ADC driver. |

CH32V30x is not supported yet. The family list is in
[Chip families](docs/families.md).

## Drivers

| Area | Drivers |
|---|---|
| Start-up | Reset handler, weak exception handlers, vector table per family |
| Clocks and timing | `SysTick` (the application sets the tick rate), delay functions |
| GPIO and EXTI | `GpioPin`, `Exti` |
| Communication | `Usart`, `I2C` |
| Analog and timing | `Adc` (CH32V00X only), `Timer`, `Dma` |
| Other | `Flash`, `Interrupt` (C++ member functions as interrupt handlers) |

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
| `src/families/<family>/` | Family-specific code. CMake adds only the active family to the include path. |
| `src/utils/` | `Interrupt` (interrupt binding) and the delay functions. |
| `ldscripts/` | Linker scripts, one memory script per chip. |
| `cmake/` | The RISC-V toolchain file and the package configuration. |
| `etc/svd/` | SVD register descriptions, for reference and debug tools. |
| `etc/openocd.cfg` | OpenOCD configuration. |

## Documentation

| Document | Contents |
|---|---|
| [Building and using ch32v00x-system](docs/building.md) | Requirements, CMake variables, compiler flags, installed package |
| [Chip families](docs/families.md) | `CH32_FAMILY`, family directories, device folders, adding a family |
| [ADC calibration on CH32V006](docs/adc.md) | Calibration of the ADC on CH32V00X |

## Contributing

All work goes to the `main` branch through merge requests, bug fixes too.
Every merge request needs a review by someone other than its author. The
project has no releases yet. Use a commit of `main` in your submodule.
