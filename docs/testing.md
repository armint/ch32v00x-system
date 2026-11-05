# Testing

## Host tests

Host tests run on the build machine. They do not need a board or the RISC-V toolchain.
They compile library sources with the native compiler (C++23).
A stub `wch_device.h` replaces the device header.
The stub gives the GPIO registers of one family as plain variables in memory.

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
| `test-gpio-v00x` | `GpioPin` on CH32V00X, pins 0 to 7: the 4-bit field in `CFGLR` for output and input modes, the pull-up and pull-down writes to `BSHR` and `BCR`. |
| `test-gpio-v20x` | The same checks on CH32V20X, pins 0 to 15. Pins 0 to 7 use `CFGLR`. Pins 8 to 15 use `CFGHR`. |
| `test-gpio-v30x` | The same checks on CH32V30X, pins 0 to 15. |

Each test starts with a known pattern in the configuration registers.
It checks the field of the selected pin.
It also checks that the fields of all other pins stay the same.
On CH32V00X, it checks that the reserved word after `CFGLR` stays the same.

### What the tests do not check

- Real hardware behavior. A fake register does not model reset values, write-protect or side effects.
- The device header itself. The stub copies the GPIO layout from the family header. A change to the header layout does not change the stub.
- The `GpioPin` inline functions `set()`, `clear()` and `toggle()`.

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
| `tests/ch32v30x/` | CH32V30X | `main()` sets up PB4 as output (blue LED) and toggles it in a loop. |

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

The image does not test the USB driver.
The USB hardware test is in `tests/ch32v30x-usb-loopback/`.

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
| `build-v307-test-image` | Links the [CH32V307 test image](#test-images) and checks the address of the vector table. |

The build jobs use the device macro, `-march` and `-mabi` values of
[Compiler flags per chip](building.md#compiler-flags-per-chip). The CH32V00X
jobs use CH32V003 and CH32V006. Only CH32V006 compiles the ADC driver of
CH32V006 and CH32V002. The CH32V20X jobs use CH32V20x_D8W. The CH32V30X jobs
use CH32V30x_D8C.
