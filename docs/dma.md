# DMA driver

`Dma::Channel` in `peripherals/Dma.h` controls one DMA channel.

## Use

1. Enable the clock: `Rcc::enableDMA1Clock()`. On CH32V30X, DMA2 has
   `Rcc::enableDMA2Clock()`.
2. Make the channel: `Dma::Channel channel(DMA1, 1);`. The second argument
   is the channel number, counted from 1.
3. Call `configure()` with the addresses, the data sizes, the direction and
   the address increment.
4. Call `setTransferCount()`, then `enable()`.
5. Poll `isTransferCompleteInterruptFlagSet()`, then call
   `clearGlobalInterruptFlag()`.

For a memory-to-memory transfer, call `enableMemoryToMemory()`. The
"peripheral" address is then the source and the "memory" address is the
destination. Set the direction `PeripheralToMemory`.

## Names

| Function | Meaning |
|---|---|
| `setTransferCount()`, `getDataCount()` | Set and read the number of data items |
| `is...InterruptFlagSet()` | Read the global, transfer complete, half transfer and transfer error flags |
| `clear...InterruptFlag()` | Clear the same four flags |

The old names `setDataCount()`, `hasTransferCompleteInterruptFlag()` and
`hasHalfTransferInterruptFlag()` no longer exist.

## DMA2 on CH32V30X

DMA2 has 11 channels. The TIM9 and TIM10 requests are on DMA2 channels 6 to
11. The registers of channels 8 to 11 are 0x10 apart from 0x90. The flags of
channels 8 to 11 are not in `INTFR` and `INTFCR`: they are in the extend
registers `EXTEM_INTFR` and `EXTEM_INTFCR` (0xD0 and 0xD4 from the DMA2
base). `Dma::Channel` selects the register. The code that uses the channel
does not change.

DMA1 has 7 channels. On some lots, an address range limit applies to DMA1.
See section 11.2.3 of the reference manual.
