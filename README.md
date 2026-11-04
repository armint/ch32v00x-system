# CH32V00x System Library

The **CH32V00x System Library** is not really a Hardware Abstraction Layer but more a peripheral access layer and
startup framework for the WCH CH32V series of RISC-V microcontrollers, curretly supporting the CH32V00x and CH32V20x
families. The goal is to have a reusable, easy to read and efficient HAL-like layer for these microcontrollers.

## Features

- **Low-level Hardware Support**: Linker scripts (`.ld`) and startup code (`startup.cpp`) tailored for WCH RISC-V MCUs.
- **Peripheral Drivers**:
    - **ADC**: Analog-to-Digital Converter with calibration support.
    - **DMA**: Direct Memory Access for efficient data transfer.
    - **EXTI**: External interrupt management.
    - **GPIO**: Simplified pin configuration and I/O operations.
    - **I2C**: Support for I2C communication.
    - **USART**: Serial communication drivers.
    - **Timers/PWM**: Timer configuration and PWM generation.
    - **Flash**: Internal flash memory programming.
- **System Utilities**:
    - **Interrupt Handling**: A dynamic interrupt connection system.
    - **Delays**: Precise microsecond (`delay_us`) and millisecond (`delay_ms`) delay functions.
- **Debugging**: Includes SVD (System View Description) files for register-level debugging in IDEs like CLion or VS
  Code.

## Supported Devices

| Family       | Supported MCUs               |
|:-------------|:-----------------------------|
| **CH32V00x** | CH32V002, CH32V003, CH32V006 |
| **CH32V20x** | CH32V208_D8W                 |

## Directory Structure

- `src/`: Shared source code for the startup, the peripheral drivers and the utilities.
- `src/families/<family>/`: Source code and headers of one chip family, for example `src/families/ch32v00x/`. The build uses only the family set by `CH32_FAMILY`.
- `ldscripts/`: Linker scripts for memory mapping.
- `etc/svd/`: SVD files for peripheral debugging.
- `cmake/`: CMake build scripts and toolchain configurations.

## Usage in CMake

This library is intended to be used as a submodule or dependency. In your `CMakeLists.txt`:

```cmake
add_subdirectory(dependencies/ch32v00x-system)
target_link_libraries(${PROJECT_NAME} PRIVATE ch32v00x-system)
```

Ensure you are using a compatible RISC-V toolchain. A sample toolchain file is provided in `cmake/gcc_riscv.cmake`.

The installed package (`cmake --install`) provides the headers only. `find_package(ch32v00x-system)` gives an interface target with the include path, but no compiled code. Add the sources with `add_subdirectory()`, as above.

Set `CH32_FAMILY` to the family of your chip. The values are `CH32V00X` and `CH32V20X`. The default is `CH32V00X`.

Also define the device macro of your chip, for example `CH32V003` or `CH32V20x_D8W`. The family and the device macro must match.

The code uses the `fence.i` instruction. Its extension is `zifencei`. Add it to the `-march` flag of your build, for example `-march=rv32ec_zicsr_zifencei -mabi=ilp32e` for CH32V003.

## Development

When contributing to this library:

- Peripheral implementations are located in `src/peripherals/`.
- Ensure new drivers follow the existing pattern of separating header and implementation.
