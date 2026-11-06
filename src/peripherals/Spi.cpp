#include "Spi.h"
#include "Rcc.h"

#include <cstdint>

namespace hw {

void Spi::configure(BaudRatePrescaler prescaler,
                    Mode mode,
                    ClockPolarity cpol,
                    ClockPhase cpha,
                    DataSize dataSize,
                    FirstBit firstBit,
                    NssMode nss,
                    Direction direction) const {
   const bool wasEnabled = isEnabled();
   if(wasEnabled) {
      disable();
   }

   setBaudRatePrescaler(prescaler);
   setMode(mode);
   setDirection(direction);
   setClockPolarity(cpol);
   setClockPhase(cpha);
   setDataSize(dataSize);
   setFirstBit(firstBit);
   setNssMode(nss);

   if(wasEnabled) {
      enable();
   }
}

void Spi::setNssMode(NssMode nss) const {
   if(nss == NssMode::Soft) {
      // SSI is the level of the internal NSS: high deselects a slave, low is a mode fault on a master.
      const uint32_t ssi = (spi_.CTLR1 & SPI_CTLR1_MSTR) ? SPI_CTLR1_SSI : 0u;
      setCtlr1(SPI_CTLR1_SSM | SPI_CTLR1_SSI, SPI_CTLR1_SSM | ssi);
      spi_.CTLR2 &= ~SPI_CTLR2_SSOE;
   } else if(nss == NssMode::HardOutput) {
      setCtlr1(SPI_CTLR1_SSM, 0);
      spi_.CTLR2 |= SPI_CTLR2_SSOE;
   } else {
      setCtlr1(SPI_CTLR1_SSM, 0);
      spi_.CTLR2 &= ~SPI_CTLR2_SSOE;
   }
}

uint32_t Spi::getClockFrequency() const {
   if(reinterpret_cast<uintptr_t>(&spi_) == reinterpret_cast<uintptr_t>(SPI1)) {
      return Rcc::getAPB2Clock();
   }
   return Rcc::getAPBClock();
}

} // namespace hw
