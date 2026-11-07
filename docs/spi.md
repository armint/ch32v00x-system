# SPI driver

`hw::Spi` in `peripherals/Spi.h` controls one SPI (serial peripheral
interface). All families use the same driver. The source is the
reference manual CH32FV2x_V3xRM, chapter 20 ("RM" below).

| Family | SPI blocks | Bus clock |
|---|---|---|
| CH32V00X | SPI1 | HCLK |
| CH32V20X | SPI1, SPI2 | SPI1: PCLK2. SPI2: PCLK1 |
| CH32V30X | SPI1, SPI2, SPI3 | SPI1: PCLK2. SPI2 and SPI3: PCLK1 |

The CH32V20X device header has no clock enable bit for SPI2. Thus `Rcc`
has no `enableSPI2Clock()` on CH32V20X (see [RCC driver](rcc.md)).

## Use

1. Enable the SPI clock: `Rcc::enableSPI1Clock()`, `enableSPI2Clock()` or
   `enableSPI3Clock()`.
2. Set the pins. A master drives SCK and MOSI: set them to the alternate
   function, push-pull. A slave drives MISO: set it to the alternate
   function, push-pull. Set the input pins to input. On CH32V30X, the
   remap functions of `hw::Afio` move the pins (see [GPIO](gpio.md)).
3. Make the object: `hw::Spi spi(SPI2);`.
4. Call `configure()` with the prescaler, the mode, the clock polarity and
   phase, the frame size, the bit order and the NSS mode.
5. Call `enable()`.
6. Use `transfer()` for one frame, or the DMA requests for a block.

For a DMA transfer, call `enableTxDma()` and `enableRxDma()`. The DMA
channel takes the data from `DATAR` or writes it to `DATAR`. On CH32V30X,
SPI1 uses DMA1 channels 2 (RX) and 3 (TX), SPI2 uses DMA1 channels 4 (RX)
and 5 (TX), and SPI3 uses DMA2 channels 1 (RX) and 2 (TX) (RM tables 11-2
and 11-3).

## Names

| Function or type | Meaning |
|---|---|
| `Mode::Master`, `Mode::Slave` | The `MSTR` bit |
| `BaudRatePrescaler::Div2` to `Div256` | The `BR` field: SCK is the bus clock divided by 2 to 256 |
| `ClockPolarity::Low`, `High` | The `CPOL` bit: the level of SCK when the bus is idle |
| `ClockPhase::OneEdge`, `TwoEdge` | The `CPHA` bit: the SCK edge that samples the data |
| `DataSize::Bits8`, `Bits16` | The `DFF` bit |
| `FirstBit::Msb`, `Lsb` | The `LSBFIRST` bit |
| `NssMode::Soft`, `HardInput`, `HardOutput` | The `SSM`, `SSI` and `SSOE` bits (see below) |
| `Direction::TwoLines`, `TwoLines_RxOnly`, `OneLine_Rx`, `OneLine_Tx` | The `BIDIMODE`, `BIDIOE` and `RXONLY` bits |
| `configure()` | Sets all these fields. An enabled SPI is disabled for the change and then enabled again |
| `enable()`, `disable()`, `isEnabled()` | The `SPE` bit |
| `write()`, `read()` | One frame to or from `DATAR` |
| `transfer()` | Sends one frame and returns the received frame, or `std::nullopt` after a timeout |
| `waitUntilIdle()` | Waits until `BSY` is clear, with a timeout |
| `isTxEmpty()`, `isRxNotEmpty()`, `isBusy()`, `isOverrun()`, `isModeFault()` | The `STATR` flags |
| `clearOverrun()`, `clearModeFault()` | Clear `OVR` and `MODF` with the sequences of RM 20.2.7 |
| `enableTxDma()`, `enableRxDma()` and the `disable` functions | The DMA requests in `CTLR2` |
| `enableRxNotEmptyInterrupt()`, `enableTxEmptyInterrupt()`, `enableErrorInterrupt()` and the `disable` functions | The interrupt enables in `CTLR2` |
| `getClockFrequency()` | The bus clock of the SPI (see the table above) |

The timeouts of `transfer()` and `waitUntilIdle()` count loop iterations,
not time. The default is `defaultTimeoutLoops` (100000).

## NSS modes

| Mode | Master | Slave |
|---|---|---|
| `Soft` | `SSM` = 1, `SSI` = 1. The NSS pin is free. | `SSM` = 1, `SSI` = 0. The slave is always selected. The NSS pin is free. |
| `HardInput` | `SSM` = 0. A low NSS pin causes a mode fault. | `SSM` = 0. A low NSS pin selects the slave. |
| `HardOutput` | `SSM` = 0, `SSOE` = 1. The master drives NSS low while it is enabled. | Do not use. |

**`SSI` with `Soft`.** A slave with `SSI` = 1 is not selected. Thus `Soft`
sets `SSI` from the `MSTR` bit (RM 20.2.3). Call `setNssMode()` after `setMode()`.
`configure()` uses this order.

## Limits

- The driver does not set the high-speed read mode (`HSCR`) of CH32V30X.
  With `HSRXEN` = 1, the `BR` field has other dividers (RM 20.4.1).
- `configure()` does not change `CRCEN` and `CRCNEXT`. The driver has no
  CRC functions.
- The driver has no I2S functions.

## Tests

The host test `test-spi-v30x` checks the register values of `configure()`,
the NSS modes, the timeouts of `transfer()` and the bus clock of each SPI.
Refer to [Testing](testing.md). The `spi` example of
[ch32v00x-examples](https://gitlab.vandertogt.org/riscv/ch32v00x-examples)
uses the driver.
