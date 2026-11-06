// Host test of hw::Spi (src/peripherals/Spi.cpp) with the CH32V30X bits: the CTLR1 and CTLR2
// values of configure(), the NSS modes, the transfer() timeouts and the bus clock of each SPI.
// Registers are host memory (stub <wch_device.h> in stubs-spi-v30x/).

#include "check.h"
#include "peripherals/Spi.h"

#include <cstring>

uint32_t SystemCoreClock = 144000000;
RCC_TypeDef hostRcc;
SPI_TypeDef hostSpi1;
SPI_TypeDef hostSpi3;

namespace {
void clear() {
   std::memset(const_cast<SPI_TypeDef *>(&hostSpi1), 0, sizeof(hostSpi1));
   std::memset(const_cast<SPI_TypeDef *>(&hostSpi3), 0, sizeof(hostSpi3));
}
} // namespace

int main() {
   using S = hw::Spi;
   const S master(SPI1);
   const S slave(SPI3);

   // The settings of the JTAG fast path: master, SCK idles high, sample on the second edge,
   // LSB first. Soft NSS on a master sets SSM and SSI (SSI low would be a mode fault).
   clear();
   master.configure(S::BaudRatePrescaler::Div8, S::Mode::Master, S::ClockPolarity::High, S::ClockPhase::TwoEdge,
                    S::DataSize::Bits8, S::FirstBit::Lsb, S::NssMode::Soft);
   CHECK(hostSpi1.CTLR1 == (0x0010 | 0x0004 | 0x0002 | 0x0001 | 0x0080 | 0x0200 | 0x0100));
   CHECK(hostSpi1.CTLR2 == 0);
   CHECK(!master.isEnabled());

   // Soft NSS on a slave clears SSI: the slave is selected without an NSS pin (RM 20.2.3).
   hostSpi3.CTLR1 = SPI_CTLR1_SSI;
   slave.configure(S::BaudRatePrescaler::Div2, S::Mode::Slave, S::ClockPolarity::High, S::ClockPhase::TwoEdge,
                   S::DataSize::Bits8, S::FirstBit::Lsb, S::NssMode::Soft);
   CHECK(hostSpi3.CTLR1 == (0x0002 | 0x0001 | 0x0080 | 0x0200));

   // configure() of an enabled SPI keeps SPE, and sets all other fields except the CRC bits.
   clear();
   hostSpi1.CTLR1 = 0xFFFF & ~SPI_CTLR1_MSTR;
   master.configure(S::BaudRatePrescaler::Div256, S::Mode::Master);
   CHECK(hostSpi1.CTLR1 == (0x0038 | 0x0004 | 0x0040 | 0x0200 | 0x0100 | 0x3000));

   // The other NSS modes clear SSM. HardOutput sets SSOE, HardInput clears it.
   clear();
   hostSpi1.CTLR1 = SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI;
   master.setNssMode(S::NssMode::HardOutput);
   CHECK(hostSpi1.CTLR1 == (SPI_CTLR1_MSTR | SPI_CTLR1_SSI));
   CHECK(hostSpi1.CTLR2 == SPI_CTLR2_SSOE);
   master.setNssMode(S::NssMode::HardInput);
   CHECK(hostSpi1.CTLR2 == 0);
   master.setNssMode(S::NssMode::Soft);
   CHECK(hostSpi1.CTLR1 == (SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI));

   // Prescaler field: BR = 0 to 7 in bits 5:3.
   clear();
   master.setBaudRatePrescaler(S::BaudRatePrescaler::Div4);
   CHECK(hostSpi1.CTLR1 == 0x0008);
   master.setBaudRatePrescaler(S::BaudRatePrescaler::Div128);
   CHECK(hostSpi1.CTLR1 == 0x0030);

   // Data size, direction and the CTLR2 enables.
   clear();
   master.setDataSize(S::DataSize::Bits16);
   CHECK(master.hasWordAccess());
   master.setDirection(S::Direction::OneLine_Tx);
   CHECK(hostSpi1.CTLR1 == (0x0800 | 0xC000));
   master.setDirection(S::Direction::TwoLines_RxOnly);
   CHECK(hostSpi1.CTLR1 == (0x0800 | 0x0400));
   master.enableRxDma();
   master.enableTxDma();
   master.enableErrorInterrupt();
   CHECK(hostSpi1.CTLR2 == (0x0001 | 0x0002 | 0x0020));
   master.disableRxDma();
   CHECK(hostSpi1.CTLR2 == (0x0002 | 0x0020));

   // transfer(): host memory has no shift register, so the test sets the flags.
   clear();
   hostSpi1.STATR = SPI_STATR_TXE | SPI_STATR_RXNE;
   const auto received = master.transfer(0xA5);
   CHECK(received.has_value());
   CHECK(hostSpi1.DATAR == 0xA5); // the write; the read sees the same host memory
   CHECK(received.value_or(0) == 0xA5);

   // TXE never set: no write, nullopt.
   clear();
   hostSpi1.DATAR = 0x11;
   CHECK(!master.transfer(0x22, 10).has_value());
   CHECK(hostSpi1.DATAR == 0x11);

   // RXNE never set: the write took place, nullopt.
   hostSpi1.STATR = SPI_STATR_TXE;
   CHECK(!master.transfer(0x33, 10).has_value());
   CHECK(hostSpi1.DATAR == 0x33);

   // A zero bound fails at once unless the flag is set.
   hostSpi1.STATR = SPI_STATR_BSY;
   CHECK(!master.waitUntilIdle(0));
   hostSpi1.STATR = 0;
   CHECK(master.waitUntilIdle(0));

   // Bus clock: SPI1 on APB2 (PPRE2), the other SPIs on APB1 (PPRE1).
   hostRcc.CFGR0 = (4ul << 11) | (5ul << 8); // PCLK2 = HCLK / 2, PCLK1 = HCLK / 4
   CHECK(master.getClockFrequency() == 72000000);
   CHECK(slave.getClockFrequency() == 36000000);

   return failures;
}
