# ADC calibration on CH32V00X
he CH32V006 performs calibration by sampling an internal Calibration Voltage (Vcal) connected to Channel 10. You must read this channel to obtain an offset/calibration value and then apply it to your other readings in software.

## How to use the Calibration
The value returned from Channel 10 represents the internal 1.2V reference or a fixed calibration point depending on your specific VDD. To get accurate results:Read the Vcal (CH10) value at startup.Read your target pin (e.g., CH0/PA2).Compensate using the formula:$Value_{Corrected} = \frac{Reading_{Pin} \times V_{Ref\_Value}}{Reading_{Vcal}}$