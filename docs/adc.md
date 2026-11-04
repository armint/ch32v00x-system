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
3. Correct the reading with the stored value. The formula is not confirmed
   in this document. Check it against the reference manual before use.

The value depends on VDD (supply voltage). The exact relation is not confirmed.

## Driver functions

- `Adc::calibrate()` runs the calibration of the chip. The exact behavior
  is not described in this library. Check the reference manual.
- `Adc::setCalibrationVoltage()` selects the calibration voltage. Its two
  values are `Half` and `ThreeQuarters`. The meaning of each value is not
  confirmed in this document.
