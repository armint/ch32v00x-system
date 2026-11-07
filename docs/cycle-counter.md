# Cycle counter

`hw::CycleCounter` in `peripherals/CycleCounter.h` gives a 32-bit counter of
core clock cycles. Use it to measure short times, for example in bit-bang
timing loops.

| Function | Effect |
|---|---|
| `start()` | Starts the counter if it is off. It changes nothing when the counter runs. |
| `count()` | Returns the core clock cycles. The counter runs free and wraps at 2^32. |

Both functions are inline and always inline, because bit-bang timing loops
call them.

## Use

```cpp
#include <peripherals/CycleCounter.h>

hw::CycleCounter::start();
const uint32_t begin = hw::CycleCounter::count();
// ... work ...
const uint32_t cycles = hw::CycleCounter::count() - begin;
```

Use only differences of two counts. The unsigned difference is correct across
the wrap, if the time is less than 2^32 cycles (29.8 s at 144 MHz).

## Families

| Family | Counter |
|---|---|
| CH32V20X, CH32V30X | `SysTick->CNT`, the low word of the 64-bit SysTick counter |
| CH32V00X | None. The header stops the build with `#error`. |

The QingKe V4 core of CH32V20X and CH32V30X has no `mcycle` CSR. Thus the
counter is the SysTick counter. `SysTick_Config()` runs this counter free at
the core clock and moves the compare value for the tick
(`utils/Timekeeping.h`). The family header defines `SYSTICK_64BIT` for these
families.

`start()` reads the STE bit of `SysTick->CTLR`. When STE is 0, `start()`
writes STCLK and STE: the counter then runs at the core clock (HCLK),
without the interrupt and without auto reload. A `SysTick_Config()` after
that sets the counter to 0 again. Do not compare a count from before
`SysTick_Config()` with a count from after it.

CH32V00X has no cycle counter. The QingKe V2 core has no `mcycle` CSR. The
SysTick counter has 32 bits and starts again from 0 at each tick (auto
reload, STRE). A free-running SysTick on CH32V00X would change the tick of
all applications, so the library does not do this.

## Example

The `cycle-counter` example of
[ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples)
compares `count()` with `delaymS()`. It prints the measured core clock.
