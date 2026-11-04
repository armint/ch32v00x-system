# Chip families

A chip family is a group of WCH chips that share a register layout and a
startup design. The library keeps the code of each family in its own
directory.

## Family selection

Set `CH32_FAMILY` at configure time. The value is one of:

- `CH32V00X`: CH32V002, CH32V003 and CH32V006. This is the default.
- `CH32V20X`: CH32V208.

CMake stops with an error for any other value. It also stops when the
family directory is missing.

The device macro of the consumer selects the chip inside the family, for
example `CH32V003` or `CH32V20x_D8W`. The header `src/wch_device.h` checks
that the device macro and `CH32_FAMILY` agree. A mismatch stops the build
with an error that names the value to set.

## Directory layout

| Path | Contents |
|---|---|
| `src/` | Shared code. It compiles for every family. |
| `src/families/ch32v00x/` | Code and headers of the CH32V00X family. |
| `src/families/ch32v00x/ch32v003/` | The device header of CH32V003, which differs from the other CH32V00X chips. |
| `src/families/ch32v20x/` | Code and headers of the CH32V20X family. |

CMake compiles the shared sources and the sources of the selected family
only. The other family directories are not compiled.

## Include paths

CMake adds the directory of the selected family to the include path. The
installed package keeps the headers flat, in one include directory, in the
same way. A shared file therefore includes a family header with its plain
name, for example `#include "Adc.h"`. The compiler finds the file of the
selected family.

## Device folders

A device folder holds a header or a source file for one chip in a family.
The source file search of CMake is recursive. A `.c` or `.cpp` file in a
device folder therefore compiles for every chip of the family. Add a device
filter to `CMakeLists.txt` before the first device-specific source file.

## Adding a family

1. Create `src/families/<family>/` with the register header, the system
   file and the vector table of the family.
2. Add the family name to the `CH32_FAMILY` list in `CMakeLists.txt`, and
   to the regular expression that checks the value.
3. Add the device macro checks to `src/wch_device.h`.
4. Document the family in the table of [the README](../README.md).
