# Timer driver

`hw::Timer` in `peripherals/Timer.h` controls one timer (TIM1 to TIM10). All
families use the same driver.

Do not use `hw::Timer` with TIM3 of CH32V006. That timer has another
register layout. The driver then writes the wrong registers, and no error
occurs.

## Use

1. Enable the timer clock in RCC. On CH32V30X, TIM1 and TIM8 to TIM10 are on
   APB2. TIM2 to TIM7 are on APB1.
2. Make the object: `hw::Timer timer(TIM10);`.
3. Set the time base: `setPrescaler()`, `setAutoReload()`, then
   `generateUpdate()` to load the values.
4. For a PWM output, call `setOcMode(channel, OcMode::Pwm1)`,
   `setCompare()` and `enableChannel()`. Set the pin to the alternate
   function.
5. On an advanced timer, call `enableMainOutput()`. Without the `MOE` bit,
   an advanced timer drives no output pin.
6. Call `enableCounter()`.

For a DMA request on a compare event, call `enableCcDma(channel)`. For a
DMA request on an update event, call `enableUpdateDma()`. The table below
gives the DMA channel of each request.

## Names

| Function | Meaning |
|---|---|
| `Channel::Ch1` to `Channel::Ch4` | The compare channel |
| `enableCounter()`, `disableCounter()`, `isCounterEnabled()` | The `CEN` bit |
| `setCounter()`, `getCounter()` | The counter value |
| `setPrescaler()`, `getPrescaler()`, `setAutoReload()`, `getAutoReload()` | The time base |
| `setAutoReloadPreload()`, `setOnePulseMode()`, `setDirectionDown()`, `setUpdateRequestSourceOnlyOverflow()` | The `ARPE`, `OPM`, `DIR` and `URS` bits |
| `generateUpdate()` | Writes `UG` |
| `setOcMode()`, `enableOcPreload()`, `disableOcPreload()` | The output compare mode and the compare preload |
| `setCompare()`, `getCompare()` | `CH1CVR` to `CH4CVR` |
| `enableChannel()`, `disableChannel()`, `setCaptureComparePolarity()` | The `CCxE` and `CCxP` bits |
| `enableUpdateDma()`, `enableCcDma()` and the `disable` functions | The DMA requests |
| `enableUpdateInterrupt()`, `enableCcInterrupt()`, `enableTriggerInterrupt()` and the `disable` functions | The interrupts |
| `isUpdateFlag()`, `getStatusFlag()`, `clearUpdateFlag()`, `clearCaptureCompare1Flag()` to `clearCaptureCompare4Flag()`, `clearStatusFlag()`, `clearAllStatusFlags()` | The `INTFR` flags |
| `setTriggerOutput()`, `disableMasterSlaveMode()` | `MMS` and `MSM` |
| `enableMainOutput()`, `disableMainOutput()`, `enableAutomaticOutput()` | `MOE` and `AOE` (advanced timers only) |

The old class `Timer` (not in a namespace) no longer exists. The old names
and the new names:

| Old | New |
|---|---|
| `OCChannel::Channel1` | `Channel::Ch1` |
| `OutputCompareMode::PWM1` | `OcMode::Pwm1` |
| `enable()`, `disable()` | `enableCounter()`, `disableCounter()` |
| `getValue()`, `setValue()` | `getCounter()`, `setCounter()` |
| `setReloadValue()`, `getReloadValue()` | `setAutoReload()`, `getAutoReload()` |
| `enableAutoReloadPreload()`, `disableAutoReloadPreload()` | `setAutoReloadPreload(bool)` |
| `setDirection(Direction)` | `setDirectionDown(bool)` |
| `setMasterMode(MasterMode)` | `setTriggerOutput()` with the `MMS` bits |
| `generateUpdateEvent()` | `generateUpdate()` |
| `clearUpdateInterruptFlag()` | `clearUpdateFlag()` |
| `clearCaptureCompareInterruptFlag()` | `clearCaptureCompare1Flag()` to `clearCaptureCompare4Flag()` |
| `enableCaptureCompareInterrupt()` | `enableCcInterrupt()` |
| `setCaptureCompareValue()`, `getCaptureCompareValue()` | `setCompare()`, `getCompare()` |
| `enableCaptureCompare()`, `disableCaptureCompare()` | `enableChannel()`, `disableChannel()` |
| `enableOutputComparePreload()` | `enableOcPreload()` |
| `setOutputCompareMode()` | `setOcMode()` |

## Fixed bugs

- `enableOcPreload()` of channels 2 and 4 set the bit of channels 1 and 3
  (issue #8).
- The flag clear functions write the inverted mask. They do not read
  `INTFR` first, so a flag that the hardware sets at that time stays set
  (issue #10).
- `setCompare()` uses a 4-byte step from `CH1CVR`. The CH32V20X header
  declares `CH1CVR` to `CH4CVR` as 16-bit fields, so the old 2-byte step
  wrote the wrong register (issue #7).

## Check against the CH32V30x reference manual

Source: CH32FV2x_V3xRM V2.5, chapters 11, 14, 15 and 16. The register
layout and the bits that the driver uses are the same as on CH32V00X and
CH32V20X. `tests/check-svd-offsets.py` checks the bits against the SVD.

- **Register width.** The manual gives 16-bit registers (`R16`). The
  exceptions are `CNT`, `ATRLR` and `CH1CVR` to `CH4CVR` of TIM2 to TIM4
  (`R32`). Each register is 4 bytes from the next. The CH32V30X header
  declares 32-bit fields. A 32-bit access works.
- **Counter width.** All counters are 16 bits. The 32-bit TIM5 is only on
  CH32V20x_D8 and D8W. `setAutoReload()` and `setCounter()` write 16 bits
  there, because the CH32V20X header declares 16-bit fields.
- **Advanced timers.** TIM1, TIM8, TIM9 and TIM10. They have `RPTCR`,
  `BDTR` and the complementary outputs (`TIM_CC1NE`, `TIM_CC1NP`). Call
  `enableMainOutput()` before the output pins change.
- **General timers.** TIM2 to TIM5. They have no `BDTR`. Do not call
  `enableMainOutput()` with them.
- **Basic timers.** TIM6 and TIM7 have no channels, no `DIR` bit and no
  `SMCFGR`. Use only the counter, the prescaler, the auto-reload, the
  one-pulse mode, the update interrupt and the update DMA request.

## DMA requests of the timers on CH32V30X

Source: the manual, tables 11-2 to 11-4.

| Timer | DMA1 | DMA2 |
|---|---|---|
| TIM1 | CH1: 2, CH2: 3, CH4, TRIG, COM: 4, UP: 5, CH3: 6 | |
| TIM2 | CH3: 1, UP: 2, CH1: 5, CH2, CH4: 7 | |
| TIM3 | CH3: 2, CH4, UP: 3, CH1, TRIG: 6 | |
| TIM4 | CH1: 1, CH2: 4, CH3: 5, UP: 7 | |
| TIM5 | | CH4, TRIG: 1, CH3, UP: 2, CH2: 4, CH1: 5 |
| TIM6 | | UP: 3 |
| TIM7 | | UP: 4 |
| TIM8 | | CH3, UP: 1, CH4, TRIG, COM: 2, CH1: 3, CH2: 5 |
| TIM9 | | UP: 6, CH1: 7, CH4: 8, CH2: 9, TRIG, COM: 10, CH3: 11 |
| TIM10 | | CH4: 6, TRIG, COM: 7, CH1: 8, CH3: 9, CH2: 10, UP: 11 |

Each DMA channel has more than one request. Enable only one request for
each channel at a time.
