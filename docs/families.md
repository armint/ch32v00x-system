# Chip families

A chip family is a group of WCH chips with the same core, the same start-up
code and almost the same peripherals. The library keeps the code of each
family in its own directory.

## Family selection

Set `CH32_FAMILY` at configure time. The value is one of:

- `CH32V00X`: CH32V002, CH32V003 and CH32V006. This is the default.
- `CH32V20X`: CH32V208.
- `CH32V30X`: CH32V307. Only the start-up, clock, SysTick, delay, interrupt,
  GPIO (with AFIO and EXTI), USART, ADC, DMA, timer and USB device code is ported. See the exclusion list below.
  The USB device driver is in [USB device driver](usb.md).

CMake stops with an error for any other value. It also stops when the
family directory is missing.

Set `CH32_DEVICE`, or let your project define the device macro of the chip,
for example `CH32V003` or `CH32V20x_D8W` or `CH32V30x_D8C`. The header `src/wch_device.h` checks that the device macro
and `CH32_FAMILY` agree. If they do not agree, the build stops with an error
that names the value to set.

## Directory layout

| Path | Contents |
|---|---|
| `src/` | Shared code. CMake compiles it for every family, except the files on the exclusion list of the family. |
| `src/families/ch32v00x/` | Code and headers of the CH32V00X family. |
| `src/families/ch32v00x/ch32v003/` | The device header of CH32V003. It differs from the header of the other CH32V00X chips. |
| `src/families/ch32v20x/` | Code and headers of the CH32V20X family. |
| `src/families/ch32v30x/` | Code and headers of the CH32V30X family. |

CMake compiles the shared sources and the sources of the selected family
only. The other family directories are not compiled.

The CH32V30X family does not support the shared `I2C` driver yet.
`CMakeLists.txt` excludes this file for that family. Its header is still
installed, but it does not compile for CH32V30X.

The `Dma` driver addresses DMA1 and, on CH32V30X, DMA2 (see [DMA driver](dma.md)). The `hw::Timer` driver
works for TIM1 to TIM10. Only the advanced timers TIM1, TIM8, TIM9 and TIM10
have the `BDTR` register for `enableMainOutput()` and `enableAutomaticOutput()`.
TIM6 and TIM7 are basic timers. They have no channels. Use only the counter,
prescaler, reload and update functions with them (see [Timer driver](timer.md)).

## Include paths

CMake adds the directory of the selected family to the include path. A
family header has a name that is unique across the families, for example
`Adc_V00x.h`. A header can select the family header with a preprocessor
test. For example, `src/families/ch32v00x/Adc.h` includes `Adc_V003.h` when
`CH32V003` is defined, and `Adc_V00x.h` when `CH32V006` or `CH32V002` is
defined. Include `<Adc.h>`. On CH32V30X, `Adc.h` in
`src/families/ch32v30x/` is the driver itself.

The installed package has the same include layout. Its include directory
holds the shared headers and the headers of the selected family.

## Device-specific code

Some shared files test a device macro. These files are:

- `src/peripherals/Flash.cpp`: selects the flash page size. On CH32V30X, it also halves HCLK during a flash operation (see [Flash driver](flash.md)).
- `src/peripherals/I2C.h`: selects the register set of the I2C block.

Do not add new device-macro tests to shared files. Put the difference into
a family header instead.

A device folder holds a header or a source file for one chip in a family.
CMake searches the family directory recursively for source files. A `.c` or
`.cpp` file in a device folder therefore compiles for every chip of the
family. Before you add the first source file for one chip, add a device
filter to `CMakeLists.txt`.

## Adding a family

1. Create `src/families/<family>/` with the register header, the system
   file and the vector table of the family.
2. Create the linker script `ldscripts/mem-<chip>.ld` for each chip.
3. Add the family name to the cache help string, to the `STRINGS` list and
   to the regular expression in `CMakeLists.txt`.
4. Add the device macro check to `src/wch_device.h`.
5. Check the two shared files that test device macros: `Flash.cpp` and `I2C.h`.
   Exclude the shared drivers that the new family does not support in the `foreach` list of `CMakeLists.txt`.
6. Update the family list in this document, in [Building and using ch32v00x-system](building.md) and in [the README](../README.md).
