# RCC driver

The `Rcc` namespace in `src/peripherals/Rcc.h` enables peripheral clocks,
resets peripherals and gives the bus clocks. RCC is the reset and clock
control unit. The function names are the names of `system-stm32` for the
same job (name parity, see `AGENTS.md`). The source is the reference manual
CH32FV2x_V3xRM, chapter 3 ("RM" below), and the WCH device headers.

## Structure

- `src/peripherals/Rcc.h` and `Rcc.cpp` are shared. They contain the bus
  clock functions.
- The family header gives the bus of each peripheral: `Rcc_V00x.h`,
  `Rcc_V20x.h` or `Rcc_V30x.h`. `Rcc.h` includes the header of the selected
  family. Include `<peripherals/Rcc.h>`, not the family header.

## Functions

| Function | CH32V00X | CH32V20X | CH32V30X |
|---|---|---|---|
| `enableGPIOAClock()`, `enableGPIOCClock()` | yes | yes | yes |
| `enableGPIOBClock()` | CH32V002, CH32V006 | yes | yes |
| `enableAFIOClock()` | yes | yes | yes |
| `enableUSART1Clock()` | yes | yes | yes |
| `enableUSART2Clock()` | CH32V002, CH32V006 (on APB2) | yes | yes |
| `enableUSART3Clock()` | no | no | yes |
| `enableSPI1Clock()` | yes | yes | yes |
| `enableSPI2Clock()` | no | no | yes |
| `enableSPI3Clock()`, `resetSPI3()` | no | no | yes |
| `enableTIM1Clock()`, `resetTIM1()` | yes | yes | yes |
| `enableTIM3Clock()`, `resetTIM3()` | CH32V002, CH32V006 | yes | yes |
| `enableTIM10Clock()`, `resetTIM10()` | no | no | yes |
| `enableDMA1Clock()` | yes | yes | yes |
| `enableDMA2Clock()` | no | no | yes |
| `resetSPI1()` | yes | yes | yes |
| `getAHBClock()`, `getAPBClock()`, `getAPB2Clock()` | yes | yes | yes |

A function is available when the device header has its bit. The CH32V20X
device header has no enable bit for USART3 and SPI2. Thus these two
functions are not available on CH32V20X, although the chip has the
peripherals.

The driver has only the functions that a user needs now. Add other
peripherals with the same pattern: one line in each family header that has
the bit.

### Clock enable

An enable function sets one bit in `AHBPCENR`, `APB2PCENR` or `APB1PCENR`.
It does not change the other bits. Then it reads the register back. The
read makes sure that the clock is on before the next access to the
peripheral.

### Peripheral reset

The reset functions (`resetSPI1()`, `resetSPI3()`, `resetTIM1()`,
`resetTIM3()` and `resetTIM10()`) set the reset bit of the peripheral in
`APB2PRSTR` or `APB1PRSTR` and then clear it. After the
call, all registers of the peripheral have their reset values. The clock
enable bit does not change. `system-stm32` has no reset function yet.
These names are the proposal for it.

### Bus clocks

| Function | Clock | Value |
|---|---|---|
| `getAHBClock()` | HCLK | `SystemCoreClock` |
| `getAPBClock()` | PCLK1 (APB1) | HCLK divided by the PPRE1 prescaler |
| `getAPB2Clock()` | PCLK2 (APB2) | HCLK divided by the PPRE2 prescaler |

PPRE1 is `CFGR0` bits 10:8. PPRE2 is `CFGR0` bits 13:11. A field value of
`0xx` divides by 1. The values `100` to `111` divide by 2, 4, 8 and 16.
CH32V00X has no APB prescaler. On CH32V00X, all three functions return HCLK.

The functions read `CFGR0` at each call. Thus they show a prescaler change
at once. `getAHBClock()` shows a change of the system clock only after a
call to `SystemCoreClockUpdate()`.

**Limit on CH32V30X.** `SystemCoreClockUpdate()` of CH32V30X ignores the
AHB prescaler (HPRE). Thus `getAHBClock()` is correct only for HPRE = /1.
The start-up sets HPRE = /1. The `Flash` driver changes HPRE to /2 only for
the time of an erase or a program operation (see [Flash driver](flash.md)).

## Tests

The host test `test-rcc-v30x` checks the CH32V30X family header and the bus
clocks. Refer to [Testing](testing.md). The hardware test is the `rcc`
example of [ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples).
