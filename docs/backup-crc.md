# Backup registers and CRC unit

These drivers are for CH32V30X. They are in the family directory. Include
`<Bkp.h>`, `<Pwr.h>` and `<Crc_V30x.h>`.

## Backup registers

`hw::Bkp` in `Bkp.h` reads and writes the backup data registers (BKP,
CH32FV2x_V3xRM chapter 4). These registers keep their value over a system
reset. With VBAT supplied, they also keep it over a power loss. A typical
use is a flag that survives `NVIC_SystemReset()`, for example "start the
bootloader".

| Function | Effect |
|---|---|
| `enable()` | Starts the PWR and BKP clocks and allows writes to the backup domain. Call it before the first read or write. |
| `disable()` | Locks the backup domain and stops the PWR and BKP clocks. The registers keep their values. It also stops the PWR clock and locks the backup domain for other users, for example the RTC. |
| `readDR1()`, `readDR2()` | Read data register 1 or 2 (16 bits). |
| `writeDR1()`, `writeDR2()` | Write data register 1 or 2. They need `enable()`. |

`hw::Pwr` in `Pwr.h` controls the write protection of the backup domain
(`PWR_CTLR.DBP`, CH32FV2x_V3xRM chapter 2): `enableBackupDomainAccess()`,
`disableBackupDomainAccess()` and `isBackupDomainAccessEnabled()`. These
functions need the PWR clock (`Rcc::enablePWRClock()`).

## CRC unit

`hw::Crc` in `Crc_V30x.h` operates the CRC unit (CH32FV2x_V3xRM chapter
5). The unit calculates CRC-32 with the polynomial 0x04C11DB7 on 32-bit
words. The start value is 0xFFFFFFFF, the most significant bit comes first,
and there is no final XOR (CRC-32/MPEG-2 on words). The functions need the
CRC clock (`Rcc::enableCRCClock()`).

| Function | Effect |
|---|---|
| `reset()` | Starts a new calculation. The value is 0xFFFFFFFF again. |
| `add(word)` | Adds one 32-bit word. |
| `value()` | Returns the CRC of the words since the last `reset()`. |

## Tests

The host test `test-rcc-v30x` checks these drivers on fake registers. Refer
to [Testing](testing.md).
