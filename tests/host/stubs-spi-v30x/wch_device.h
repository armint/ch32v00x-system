#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the Spi driver (src/peripherals/Spi.cpp). It has
// the SPI bits of src/families/ch32v30x/ch32v30x.h, the RCC and SPI registers as host variables,
// and SystemCoreClock. CTLR1 and STATR model the mode fault (see below).

#include <stdint.h>

// RCC registers, RCC bits and SystemCoreClock: the same stub as the Rcc test (Spi.cpp uses Rcc).
#include "../stubs-rcc-v30x/wch_device.h"

struct SPI_TypeDef;

// CTLR1 and STATR are classes, so that the stub has the mode fault of a master (RM 20.2.7):
// - MSTR set while the internal NSS is low sets MODF and clears MSTR and SPE. The internal NSS is
//   SSI with SSM = 1, else the NSS pin (nssPinLow). The CH32V307 does this also with SPE = 0.
// - While MODF is set, a CTLR1 write cannot set MSTR or SPE.
// - A STATR read and then a CTLR1 write clear MODF.
class HostSpiCtlr1 {
   public:
   SPI_TypeDef *spi;
   uint16_t bits;

   operator uint16_t() const {
      return bits;
   }
   HostSpiCtlr1 &operator=(uint16_t value);
   HostSpiCtlr1 &operator&=(uint16_t mask) {
      return *this = static_cast<uint16_t>(bits & mask);
   }
   HostSpiCtlr1 &operator|=(uint16_t mask) {
      return *this = static_cast<uint16_t>(bits | mask);
   }
};

class HostSpiStatr {
   public:
   SPI_TypeDef *spi;
   uint16_t bits;

   operator uint16_t() const;
   HostSpiStatr &operator=(uint16_t value) {
      bits = value;
      return *this;
   }
};

struct SPI_TypeDef {
   HostSpiCtlr1 CTLR1;
   uint16_t RESERVED0;
   volatile uint16_t CTLR2;
   uint16_t RESERVED1;
   HostSpiStatr STATR;
   uint16_t RESERVED2;
   volatile uint16_t DATAR;
   uint16_t RESERVED3;

   bool nssPinLow;        // level of the NSS pin; a floating pin can read low
   mutable bool modfRead; // STATR was read while MODF was set

   SPI_TypeDef()
      : CTLR1{ this, 0 }
      , STATR{ this, 0 } {
      reset();
   }
   /// All registers 0, NSS pin high.
   void reset() {
      CTLR1.bits = 0;
      CTLR2 = 0;
      STATR.bits = 0;
      DATAR = 0;
      nssPinLow = false;
      modfRead = false;
   }
};

extern SPI_TypeDef hostSpi1;
extern SPI_TypeDef hostSpi3;
#define SPI1 (&hostSpi1)
#define SPI3 (&hostSpi3)

#define SPI_CTLR1_CPHA ((uint16_t)0x0001)
#define SPI_CTLR1_CPOL ((uint16_t)0x0002)
#define SPI_CTLR1_MSTR ((uint16_t)0x0004)
#define SPI_CTLR1_BR ((uint16_t)0x0038)
#define SPI_CTLR1_BR_0 ((uint16_t)0x0008)
#define SPI_CTLR1_BR_1 ((uint16_t)0x0010)
#define SPI_CTLR1_BR_2 ((uint16_t)0x0020)
#define SPI_CTLR1_SPE ((uint16_t)0x0040)
#define SPI_CTLR1_LSBFIRST ((uint16_t)0x0080)
#define SPI_CTLR1_SSI ((uint16_t)0x0100)
#define SPI_CTLR1_SSM ((uint16_t)0x0200)
#define SPI_CTLR1_RXONLY ((uint16_t)0x0400)
#define SPI_CTLR1_DFF ((uint16_t)0x0800)
#define SPI_CTLR1_BIDIOE ((uint16_t)0x4000)
#define SPI_CTLR1_BIDIMODE ((uint16_t)0x8000)

#define SPI_CTLR2_RXDMAEN ((uint16_t)0x0001)
#define SPI_CTLR2_TXDMAEN ((uint16_t)0x0002)
#define SPI_CTLR2_SSOE ((uint16_t)0x0004)
#define SPI_CTLR2_ERRIE ((uint16_t)0x0020)
#define SPI_CTLR2_RXNEIE ((uint16_t)0x0040)
#define SPI_CTLR2_TXEIE ((uint16_t)0x0080)

#define SPI_STATR_RXNE ((uint16_t)0x0001)
#define SPI_STATR_TXE ((uint16_t)0x0002)
#define SPI_STATR_MODF ((uint16_t)0x0020)
#define SPI_STATR_OVR ((uint16_t)0x0040)
#define SPI_STATR_BSY ((uint16_t)0x0080)

inline HostSpiStatr::operator uint16_t() const {
   if(bits & SPI_STATR_MODF) {
      spi->modfRead = true;
   }
   return bits;
}

inline HostSpiCtlr1 &HostSpiCtlr1::operator=(uint16_t value) {
   SPI_TypeDef &s = *spi;
   if(s.modfRead) {
      s.STATR.bits = static_cast<uint16_t>(s.STATR.bits & ~SPI_STATR_MODF);
      s.modfRead = false;
   }
   if(s.STATR.bits & SPI_STATR_MODF) {
      value = static_cast<uint16_t>(value & ~(SPI_CTLR1_MSTR | SPI_CTLR1_SPE));
   }
   const bool nssLow = (value & SPI_CTLR1_SSM) ? (value & SPI_CTLR1_SSI) == 0 : s.nssPinLow;
   if((value & SPI_CTLR1_MSTR) && nssLow) {
      s.STATR.bits = static_cast<uint16_t>(s.STATR.bits | SPI_STATR_MODF);
      value = static_cast<uint16_t>(value & ~(SPI_CTLR1_MSTR | SPI_CTLR1_SPE));
   }
   bits = value;
   return *this;
}
