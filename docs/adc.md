# ADC calibration on CH32V002 and CH32V006

The CH32V002 and the CH32V006 have an internal calibration voltage (Vcal)
on channel 10. ADC (analog-to-digital converter) channel 10 measures this
voltage. Read channel 10 to get the calibration value. Use this value to
correct the other readings in software.

The CH32V003 has its calibration voltage on channel 9 (`Channel::CH9_Vcal`).
The steps below apply to CH32V002 and CH32V006 only.

## Steps

1. Read channel 10 (`Channel::CH10_Vcal`) at start-up. Store the value.
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
voltage) on channel 17. The class `Adc` has the same method names as the
CH32V00X class. The channel and sample time names are different, because
the pins and the cycle counts are different.

## Differences from CH32V00X

- `Channel` names the pin of each channel, for example `CH8_PB0`.
  `CH16_Temp` and `CH17_Vref` are the internal channels.
- `SamplingCycles` gives 1.5, 7.5, 13.5, 28.5, 41.5, 55.5, 71.5 and 239.5 cycles.
- `ClockDivider` divides PCLK2 by 2, 4, 6 or 8. The ADC clock must not
  exceed 14 MHz. The start-up code of this library does not divide APB2,
  so PCLK2 is 144 MHz. Then even `div8` gives 18 MHz, which is too fast.
  Set the APB2 prescaler (`RCC_CFGR0` PPRE2) to 2 before you start the
  USART. Then `div6` gives 12 MHz and `div8` gives 9 MHz. The `Usart`
  driver must know the prescaler, or the baud rate of USART1 is wrong.
  The `Usart` change in
  [PR 5](https://github.com/armint/ch32v00x-system/pull/5) does this. This
  ADC driver does not change PPRE2 itself.
- There is no `setCalibrationVoltage()` and no injected trigger.
- `enableClock()` and `reset()` act on the ADC of the object.
- `enableInternalChannels()` switches channels 16 and 17 on. Wait for the
  start-up time before you convert.

## Steps

1. Set PPRE2 to 2 (`RCC->CFGR0`) before you initialize USART1. Call
   `enableClock()`. Then call `Adc::setClockDivider(Adc::ClockDivider::div6)`
   (12 MHz) or `div8` (9 MHz).
2. Call `setSamplingTime()` for each channel. Vrefint and the temperature
   sensor need at least 17.1 us.
3. Call `enable()`, then `calibrate()`. `calibrate()` blocks until the
   calibration ends.
4. Call `enableInternalChannels()` if you use channel 16 or 17. Do it after
   `calibrate()`: with TSVREFE set, the input buffer stays on and the
   calibration offset is wrong for the external channels. Wait for the
   start-up time.
5. Call `configureRegularChannel(0, channel)`. Call
   `setRegularConversionTrigger(RegularTrigger::SWSTART)`.
6. Call `start()`. Wait for `isConversionComplete()`. Read `value()`.

The CH32V30X ADC is not tested on hardware yet. Only the register values
are checked by a host test.
