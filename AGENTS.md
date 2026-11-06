# Agent Instructions for ch32v00x-system

## Purpose
This library provides the start-up code, the interrupt vector tables and the register-level peripheral drivers for WCH CH32V microcontrollers. It supports several chip families (CH32V00X, CH32V20X and CH32V30X). It uses `CH32_FAMILY` at configure time to select the family code.

## Key Features
- **Multi-Family Support**: `CH32_FAMILY` selects the family directory `src/families/<family>/`. CMake compiles only the selected family.
- **Start-up**: Linker scripts and start-up code (`src/bare-metal/startup.cpp`, `vectors_*.c`).
- **Peripherals**: Drivers for ADC (analog-to-digital converter), DMA (direct memory access), EXTI (external interrupt), GPIO (general-purpose input/output), I2C (Inter-Integrated Circuit), Flash, RCC (reset and clock control), SPI (serial peripheral interface), USART (universal synchronous/asynchronous receiver-transmitter), timers and the USB device on CH32V30X (`docs/usb.md`).
- **System**: Delay functions, the SysTick timer, the cycle counter (`docs/cycle-counter.md`), interrupt handling and hardware initialization.

## Guidelines
- **Peripheral Drivers**: Shared peripheral interfaces are in `src/peripherals/`.
- **Name Parity**: A function that does the same job as a function in `system-stm32` has the same public name, for example `enableTxEmptyInterrupt`. Check the `system-stm32` names before you add or rename a public function. The same rule applies to CMake variables: a variable that does the same job as one in `system-stm32` uses the same suffix, for example `CH32_DEVICE` for `STM32_DEVICE`. Keep the register differences inside the driver. Glue code such as `ch32v00x-art` uses these names and does not add its own.
- **Driver Files**: Put each driver in a header and a source file with the same name, for example `Usart.h` and `Usart.cpp`. Put only short register accesses inline in the header.
- **Family-Specific Code**: Put hardware-dependent code into `src/families/<family>/`. A header that `system-stm32` also has, for example `Adc.h`, keeps that name in each family directory, and you include it as `<Adc.h>`. Give each other family header a name that no other family uses, for example `Adc_V00x.h`. `Adc.h` of CH32V00X selects `Adc_V003.h` or `Adc_V00x.h`. `Adc.h` of CH32V30X is the driver.
- **Device Macros in Shared Code**: Do not add a new test of a device macro, for example `CH32V003`, to a shared file. Put the difference into the family directory. Two shared files already test device macros: `src/peripherals/Flash.cpp` and `src/peripherals/I2C.h`. Keep those tests in these two files. Do not add device-macro tests to other shared files.
- **Register or Bit Tests**: A test for a register or a register bit is allowed in shared code, for example `#if defined(<register bit>)`.
- **Documentation**: `README.md` is a short overview. Put driver and family details into `docs/`, and add each new document to the table in `README.md`. Write the prose in ASD-STE100 Simplified Technical English (STE): instructions of 20 words or fewer, descriptions of 25 words or fewer, active voice, simple tenses, and one term for each concept.
- **Hardware Interaction**: Match the hardware specification of the WCH microcontroller when you change or add a driver. Use the `Interrupt` utility in `src/utils/` to connect interrupt handlers at run time.
- **Memory**: Flash and RAM are limited. Do not use heap memory. Do not use exceptions.
- **Dependencies**: The library does not use the embedded template library (ETL). Do not add it as a dependency. ETL is a dependency of [ch32v00x-art](https://gitlab.vandertogt.org/riscv/ch32v00x-art) and of [ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples).
- **Style & Linting**: The format rules are in the `.clang-format` file of `ch32v00x-examples`. Format the code in the `ch32v00x-examples` build with the target `clangformat`. A standalone build of this library has no `clangformat` target and no `.clang-format` file.

## Verification
- Build the library once for each row of the table in [Compiler flags per chip](docs/building.md#compiler-flags-per-chip). Set the `CH32_FAMILY` value of the row and the device macro.
- After a change to `src/families/ch32v30x/ch32v30x.h`, run `python3 tests/check-svd-offsets.py` with the cross compiler. It compares the header with the SVD.
- Build `ch32v00x-examples` once for each `MCU` value: `CH32V002`, `CH32V003`, `CH32V006` and `CH32V208_D8W`. The examples do not yet build for `CH32V307`. On `CH32V208_D8W`, the ADC examples are not built.
- After a change to `CMakeLists.txt` or to the package configuration, run `cmake --install`. Check that the include directory contains `wch_device.h` and the headers of the selected family.
