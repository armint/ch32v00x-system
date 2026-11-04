# Building and using ch32v00x-system

## Requirements

- CMake 3.20 or later.
- A RISC-V GCC toolchain, for example `riscv-none-elf-gcc`. The toolchain
  file `cmake/gcc_riscv.cmake` uses it. Give the toolchain directory with
  `GCC_TOOLCHAIN_PREFIX`.
- The `zifencei` extension. The library uses the `fence.i` instruction.
  Add `zifencei` to the `-march` flag. Without it, the build fails.

## CMake variables

| Variable | Default | Meaning |
|---|---|---|
| `CH32_FAMILY` | `CH32V00X` | Chip family of the build: `CH32V00X` or `CH32V20X`. See [Chip families](families.md). |
| `GCC_TOOLCHAIN_PREFIX` | Derived from the compiler | Directory that contains `bin/`, for the toolchain file. |

The consumer also sets the device macro of the chip, for example
`CH32V003` or `CH32V20x_D8W`. The device macro and `CH32_FAMILY` must
match. If they do not match, the build stops with an error.

## Standalone build

This command builds the library for CH32V003:

```sh
cmake -B build \
    -D CMAKE_TOOLCHAIN_FILE=cmake/gcc_riscv.cmake \
    -D GCC_TOOLCHAIN_PREFIX=/path/to/riscv-toolchain \
    -D CMAKE_BUILD_TYPE=Release \
    -D CH32_FAMILY=CH32V00X \
    -D CMAKE_C_FLAGS="-DCH32V003 -march=rv32ec_zicsr_zifencei -mabi=ilp32e" \
    -D CMAKE_CXX_FLAGS="-DCH32V003 -march=rv32ec_zicsr_zifencei -mabi=ilp32e"
cmake --build build --target ch32v00x-system
```

## Compiler flags per chip

| Chip | Device macro | `-march` | `-mabi` |
|---|---|---|---|
| CH32V003 | `CH32V003` | `rv32ec_zicsr_zifencei` | `ilp32e` |
| CH32V002, CH32V006 | `CH32V002` or `CH32V006` | `rv32ec_zicsr_zifencei_zmmul` | `ilp32e` |
| CH32V208 | `CH32V20x_D8W` | `rv32imac_zicsr_zifencei` | `ilp32` |

The examples project in `ch32v00x-examples` uses the same flags.

## Using the library in a project

Add the library as a subdirectory, then link the target:

```cmake
add_subdirectory(dependencies/ch32v00x-system)
target_link_libraries(${PROJECT_NAME} PRIVATE ch32v00x-system)
```

The target `ch32v00x-system` is an OBJECT library. It compiles the library
sources with the flags of your project.

## Installed package

`cmake --install` installs the headers only. The installed package does not
contain compiled code. `find_package(ch32v00x-system)` gives the interface
target `art::ch32v00x-system` with the include path. Add the library sources with
`add_subdirectory()`, as shown above.
