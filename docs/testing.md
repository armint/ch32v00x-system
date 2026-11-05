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

## Continuous integration

The file `.gitlab-ci.yml` defines the GitLab pipeline. All jobs run in the
`build` stage. They use the builder image `ghcr.io/armint/firmware-build-image:v1.15.0`, which has the
RISC-V toolchain. Each build job looks for `riscv-none-elf-gcc` with
`command -v`. The job fails if the tool is missing.

| Job | What it does |
|---|---|
| `build` | Builds the `ch32v00x-system` library for CH32V00X, CH32V20X and CH32V30X at C++23. The job fails if CMake fetches span-compat. |
| `build-cpp17` | Builds the same three families at C++17. The job fails if CMake does not use `span_compat::span`. |
| `host-tests` | Runs the host tests from `tests/host` with the native compiler. |

The build jobs use the device macro, `-march` and `-mabi` values of
[Compiler flags per chip](building.md#compiler-flags-per-chip). The CH32V00X
jobs use CH32V003. The CH32V20X jobs use CH32V20x_D8W. The CH32V30X jobs use
CH32V30x_D8C.

No job builds a test image yet.
