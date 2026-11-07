# ADC calibration on CH32V002 and CH32V006

The CH32V002 and the CH32V006 have an internal calibration voltage (Vcal)
on channel 10. ADC (analog-to-digital converter) channel 10 measures this
voltage. Read channel 10 to get the calibration value. Use this value to
correct the other readings in software.

The CH32V003 has its calibration voltage on channel 9 (`Channel::Vcal`).
The steps below apply to CH32V002 and CH32V006 only.

## Steps

1. Read channel 10 (`Channel::Vcal`) at start-up. Store the value.
2. Read your input channel, for example channel 0 on pin PA2.
3. Correct the reading: `corrected = reading x V_ref / reading_Vcal`. `V_ref`
   is the reference voltage of Vcal. This formula is not confirmed. Check it
   against the reference manual before use.

The value depends on VDD (supply voltage). The exact relation is not confirmed.

## Driver functions

- `Adc::calibrate()` runs the calibration of the chip. The exact behavior
  is not described in this library. Check the reference manual.
- `Adc::setCalibrationVoltage()` selects the calibration voltage. Its two
  values are `Half` and `ThreeQuarters`. The meaning of each value is not
  confirmed in this document.

# ADC of the CH32V30X

The CH32V307 has two ADCs, ADC1 and ADC2. Each has 16 external channels
(channel 0 to 15) and a 12-bit result. ADC1 also has two internal channels:
the temperature sensor on channel 16 and Vrefint (internal reference
voltage) on channel 17. Include `<Adc.h>`.

The class has `Channel` (`PA0`, `PB0`, `TempSensor`, `Vrefint`),
`SamplingCycles::Cycles239_5`, the static `enableClock()`, `disableClock()`,
`reset()` and `configureClock()`, and `enable()`, `disable()`, `calibrate()`,
`setSamplingTime()` and `convert()`. For sequences and DMA, it has
`configureRegularChannel()`, `start()`, `value()` and the trigger and
interrupt functions. The names are the names of the CH32V00X class where the
job is the same, for example `Cycles241` and `Vrefint`.

## Differences from CH32V00X

- `enableClock()`, `disableClock()` and `reset()` are static. They take the
  ADC as an optional argument. The default is ADC1.
  Use `Adc::enableClock(ADC2)` for ADC2.
- `configureClock()` sets the smallest PCLK2 divider (2, 4, 6 or 8) that
  keeps the ADC clock at or below 14 MHz. It returns the ADC clock in Hz.
  The start-up code of this library does not divide APB2, so PCLK2 is
  144 MHz. Then even /8 gives 18 MHz, which is too fast. Set the APB2
  prescaler (`RCC_CFGR0` PPRE2) to 2 before you start the USART. Then
  `configureClock()` selects /6, which gives 12 MHz. The `Usart` driver
  reads the prescaler, so the baud rate stays right.
- `calibrate()` returns false when the ADC is off. It does not power the
  ADC on. Call `enable()` first.
- `ClockDivider` and `setClockDivider()` set the divider by hand.
- There is no `readSupplyMillivolts()`.
- There is no `setCalibrationVoltage()` and no injected trigger.

## Steps

1. Set PPRE2 to 2 (`RCC->CFGR0`) before you initialize USART1. Call
   `Adc::enableClock()` and `Adc::configureClock()`.
2. Make an object: `Adc adc(ADC1);`. Call `setSamplingTime()` for each
   channel. Vrefint and the temperature sensor need at least 17.1 us.
3. Call `enable()`, then `calibrate()`. `calibrate()` blocks until the
   calibration ends.
4. Call `enableInternalChannels()` if you use channel 16 or 17. Do it after
   `calibrate()`: with TSVREFE set, the input buffer stays on and the
   calibration offset is wrong for the external channels. Wait for the
   start-up time.
5. For one conversion, call `convert(channel, value)`. It returns false if
   the conversion does not end.
6. For a sequence or for DMA, call `configureRegularChannel(0, channel)`
   and `setRegularConversionTrigger(RegularTrigger::SWSTART)`. Call
   `start()`. Wait for `isConversionComplete()`. Read `value()`.

The `adc-dma` example of ch32v00x-examples uses the driver. The host test
checks the register values, `configureClock()` and the refusal of
`calibrate()` on an ADC that is off.
