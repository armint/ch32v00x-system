# GPIO, AFIO and EXTI

This page lists the GPIO APIs of the library and the families that have them.
Each API has the name and the signature of the same API in `system-stm32`.

## API per family

| API | Header | CH32V00X | CH32V20X | CH32V30X |
|---|---|---|---|---|
| `hw::GpioPin` | `peripherals/GpioPin.h` | yes | yes | yes |
| `hw::FastPin` | `peripherals/FastPin.h` | yes (pins 0 to 7) | yes | yes |
| `Exti` (short API: `enableInterrupt()`, `setRisingEdge()`, ...) | `peripherals/Exti.h` | yes | yes | no |
| `Exti` (API of `system-stm32`: `TriggerEdge`, `Port`, `setPortForExtiLine()`, `configureExtiInterrupt()`, `isInterruptPending()`, ...) | `peripherals/Exti.h` | no | no | yes |
| `GpioInterrupt` | `utils/GpioInterrupt.h` | no | no | yes |
| `hw::Afio` | `Afio.h` | no | no | yes |

`system-stm32` has `hw::Afio` for STM32F1 only. It has `Exti` and
`GpioInterrupt` for the other STM32 families, not for STM32F1.

## GpioPin and FastPin

The CH32 GPIO is the STM32F1 GPIO. Each pin has one 4-bit field in `CFGLR`
(pins 0 to 7) or `CFGHR` (pins 8 to 15). The encoding is in
`peripherals/GpioConfig.h`. CH32V00X ports have pins 0 to 7 only.

## Afio (CH32V30X)

`hw::Afio` selects the remapped pin sets in `AFIO_PCFR1`. All functions turn on
the AFIO clock. The functions are `remapSpi1()`, `remapSpi3()`,
`remapI2c1()`, `remapUsart1()`, `remapUsart2()` and `remapUsart3()`. Use
`setRemap()` for other fields.

`setSwjConfig()` sets the debug port. `SwjConfig::Disabled` turns SWD off and
frees PA13 and PA14. A debugger cannot connect then. The chip has no JTAG port:
PA15, PB3 and PB4 are GPIO after reset.

`remapUsart1()` keeps `AFIO_PCFR2.USART1_RM1` at 0. The chip has more
USART1 pin sets than the two that this function selects.

## Exti and GpioInterrupt (CH32V30X)

`Exti` has the GPIO lines 0 to 15. The lines above 15 (PVD, RTC alarm, USB
wake-up and Ethernet wake-up) have no name yet.

`GpioInterrupt::connectHandler()` does these steps for a pin:

1. It routes the line to the port in `AFIO_EXTICRx`.
2. It sets the trigger edge.
3. It connects the handler to the interrupt of the line.
4. It turns on the interrupt of the line in the EXTI and the PFIC.

Your code must configure the pin as an input and turn on the GPIO port clock.
The function does not do this.

Two pins with the same number on different ports share one EXTI line. The
second `connectHandler()` call returns `false`. The lines 5 to 9 share one
interrupt, and so do the lines 10 to 15. The dispatch code calls the handler
of each line that has a flag.

The dispatch code clears the EXTI flag before it calls the handler. An edge
during the handler stays pending. The dispatch code does not clear the flag
of a masked line, so code can poll that line with `isInterruptPending()`.
`GpioInterrupt` does not clear the PFIC pending bit. On a shared vector
(lines 5 to 9 and 10 to 15), an edge on another line can set that bit at
any time. A clear can lose that request.

The handler runs in the interrupt. Keep it short.
