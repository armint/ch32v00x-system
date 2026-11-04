# Agent Instructions for ch32v00x-system

## Purpose
This library provides the start-up code, the interrupt vector tables and the register-level peripheral drivers for WCH CH32V microcontrollers. It supports several chip families (CH32V00X and CH32V20X). It uses `CH32_FAMILY` at configure time to select the family code, the same way `system-stm32` uses `STM32_FAMILY`.

## Key Features
- **Multi-Family Support**: `CH32_FAMILY` selects the family directory `src/families/<family>/`. CMake compiles only the active family.
- **Startup**: Linker scripts and startup code (`startup.cpp`, `vectors_*.c`).
- **Peripherals**: Drivers for ADC, DMA, EXTI, GPIO, I2C, Flash, USART and Timers.
- **System**: Delay functions, the SysTick timer, interrupt handling and hardware initialization.

## Guidelines
- **Peripheral Drivers**: Shared peripheral interfaces are in `src/peripherals/`.
- **Family-Specific Code**: Hardware-dependent code (for example `Adc`) goes into `src/families/<family>/`. Each class keeps the same interface across families, so `#include "Adc.h"` resolves to the file of the active family.
- **Avoid `#ifdef` for Families**: Do not test `CH32V003` or another device macro in a shared `.cpp` file to handle a family difference. Use the family directory instead. A test for a register or a register bit is acceptable, for example `#if defined(<register bit>)`.
- **Documentation**: `README.md` is a short overview. Put driver and family details into `docs/`, and add each new document to the table in `README.md`. Write the prose in the style of `doc-style`.
- **Hardware Interaction**: When you modify or add peripheral support, match the hardware specification of the targeted WCH MCU. Use the `Interrupt` utility in `src/utils/` to connect interrupt handlers at run time.
- **Memory**: Flash and RAM are limited. Heap memory usage is strictly prohibited. Do not use exceptions.
- **Dependencies**: The embedded template library (ETL) is a dependency. Consider it before you write a similar class.
- **Style & Linting**: Format the code with the CMake target `cmake --build <build_dir> --target clangformat`.

## Verification
- Build the library for each family and device pair in [Compiler flags per chip](docs/building.md#compiler-flags-per-chip). Use the matching `CH32_FAMILY` and device macro.
- Build the examples in `ch32v00x-examples` after a change to a shared driver. The examples cover all chips.
- A change to `CMakeLists.txt` or the package configuration needs a build and an `cmake --install` check.
