# Building and using ch32v00x-system

## Requirements

- CMake 3.20 or later.
- A RISC-V GCC toolchain, for example `riscv-none-elf-gcc`. The toolchain
  file `cmake/gcc_riscv.cmake` uses it.
- The `zifencei` extension. The library uses the `fence.i` instruction
  (fence for instruction memory). Add `zifencei` to the `-march` flag.
  Without it, the build fails.

## CMake variables

| Variable | Default | Meaning |
|---|---|---|
| `CH32_FAMILY` | `CH32V00X` | Chip family of the build: `CH32V00X`, `CH32V20X` or `CH32V30X`. See [Chip families](families.md). |
| `GCC_TOOLCHAIN_PREFIX` | `/usr`, or the parent of the `bin/` directory of `CMAKE_C_COMPILER` when that is set | Directory that contains `bin/` with the toolchain tools. |
| `TARGET_TRIPLET` | `riscv-none-elf` | Prefix of the tool names, for example `riscv64-unknown-elf`. |

Your project sets the device macro of the chip, for example `CH32V003` or
`CH32V20x_D8W`. The device macro and `CH32_FAMILY` must match. If they do
not match, the build stops with an error.

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

| Chip | `CH32_FAMILY` | Device macro | `-march` | `-mabi` |
|---|---|---|---|---|
| CH32V003 | `CH32V00X` | `CH32V003` | `rv32ec_zicsr_zifencei` | `ilp32e` |
| CH32V002 | `CH32V00X` | `CH32V002` | `rv32ec_zicsr_zifencei_zmmul` | `ilp32e` |
| CH32V006 | `CH32V00X` | `CH32V006` | `rv32ec_zicsr_zifencei_zmmul` | `ilp32e` |
| CH32V208 | `CH32V20X` | `CH32V20x_D8W` | `rv32imac_zicsr_zifencei` | `ilp32` |
| CH32V307 | `CH32V30X` | `CH32V30x_D8C` | `rv32imafc_zicsr_zifencei` | `ilp32f` |

The examples in [ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples)
use the same device macro, `-march` and `-mabi`. They also add
`-fsingle-precision-constant`. For C++ files only, they add `-fno-exceptions`
and `-fno-rtti`.

## Using the library in a project

Add the library as a subdirectory, then link the target:

```cmake
add_subdirectory(dependencies/ch32v00x-system)
target_link_libraries(${PROJECT_NAME} PRIVATE ch32v00x-system)
```

The target `ch32v00x-system` is an OBJECT library. It compiles the library
sources with the flags of your project. If the superproject defines the
target `art-build-config`, the library links it. The examples use this
target to pass their compiler flags to the library.

## Installed package

`cmake --install` installs the headers of the shared tree and of the selected
family only. The installed package does not contain compiled code.
`find_package(ch32v00x-system)` gives the interface target
`art::ch32v00x-system`. It has the include path only. Add the library sources
with `add_subdirectory()`, as shown above.

To check the install, run `cmake --install` and look for `wch_device.h` and
the headers of the family in the include directory.
