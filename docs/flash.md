# Flash driver

The `Flash` namespace in `src/peripherals/Flash.h` erases and programs the
internal code flash. All families share `src/peripherals/Flash.cpp`. This
document gives the CH32V30X details. The source is the reference manual
CH32FV2x_V3xRM, chapter 32 ("RM" below). Chapter 32 applies to CH32V20X
and CH32V30X.

## Functions

| Function | Operation | Size on CH32V20X and CH32V30X |
|---|---|---|
| `writable_section` | Unlocks the controller and the fast mode. The destructor locks both again. | - |
| `erase_page()` | Fast page erase (RM 32.5.7) | 256 bytes |
| `write_buffer()` | Fast page programming (RM 32.5.6). Pads the page with `0xFF`. | 256 bytes |
| `program_halfword()` | Standard programming (RM 32.5.3). Returns `false` for an odd address, a write protection error, or a value that does not read back. | 2 bytes |

Call the erase and program functions only while a `writable_section`
object exists. A new `writable_section` object unlocks the controller
again. The constructor writes the keys only when the lock bit is set.
The RM says that a wrong key sequence locks the controller until the next
reset.

## Answers from the RM (CH32V30X)

**Erase size.** The standard mode erases 4 KB (`PER`, 16 pages of 256
bytes, RM 32.5.4). The fast mode erases 256 bytes (`FTER`) or 32 KB
(`BER32`). `erase_page()` uses the 256-byte fast erase. The driver has no
function for the 4 KB erase.

**Half-word programming.** The standard mode programs one half-word for
each write while `PG` is set (RM 32.5.3). A write of another size causes
a bus error. The fast mode programs only full 256-byte pages, in 64 words
of 32 bits. Thus a log of 16-bit slots (for example the settings page of
the firmware) uses `program_halfword()` for each slot, and `erase_page()`
for the page.

**Erased value.** After an erase, the flash does not read as `0xFF`. The RM
gives these values: a word reads `0xE339E339`, a half-word reads `0xE339`,
a byte at an even address reads `0x39`, and a byte at an odd address reads
`0xE3`. Code that looks for an erased slot must compare with `0xE339`, not
with `0xFFFF`. The `flash` example of
[ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples)
prints the value that the board reads.

## Clock limit

The flash access clock must not be more than 60 MHz (RM 32.1, note 2).
With `SCKMOD` = 0 (reset value), the access clock is half the bus clock.
Thus, when `SystemCoreClock` is more than 120 MHz, the driver sets HCLK to
SYSCLK / 2 for each erase or program operation. After the operation, it
writes the old `RCC_CFGR0` value again.

This has effects on the rest of the chip:

- The APB clocks are also half speed during the operation. A USART frame
  that is on the line at that time has the wrong baud rate.
- SysTick also counts at half speed during the operation.
- The CPU stops while the flash is busy. Interrupts wait for the end of
  the operation.

The CH32V20X has the same RM rule. The driver does not halve the clock on
CH32V20X yet.

## Limits

- Use addresses in the `0x08000000` range, not the alias at `0x00000000`.
- The RM says that the code that erases the flash must be in the
  zero-wait area. The code of an image in the default memory split
  (256 KB code) is in that area.
- The RM gives only full-page operations for the fast mode. Give
  `write_buffer()` the start address of a page.
- The flash enhanced read mode (`EHMOD`) must be off for an erase or a
  program operation. The driver does not set `EHMOD`, and a reset clears it.
- `program_halfword()` compiles for each chip header that defines
  `FLASH_CTLR_PG` (CH32V003, CH32V20X and CH32V30X). It is tested only on
  CH32V30X.

## Tests

- The host test `test-flash-v30x` checks the register sequence. See
  [Testing](testing.md).
- The `flash` example of ch32v00x-examples runs on the YD-CH32V307VCT6
  board. It checks the erase, the two program modes, the read-back and the
  values after a reset.
