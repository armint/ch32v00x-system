// Host test of the Rcc driver of CH32V30X (src/peripherals/Rcc.cpp, src/families/ch32v30x/Rcc_V30x.h):
// each clock enable sets one bit in the register of its bus, each reset leaves its bit clear, and the
// bus clocks follow PPRE1 and PPRE2. Also setAPB2Prescaler(), getSystemClockSource(), hw::Bkp,
// hw::Pwr and hw::Crc. Registers are host memory (stub <wch_device.h> in stubs-rcc-v30x/).

#include "Bkp.h"
#include "Crc_V30x.h"
#include "check.h"
#include "peripherals/Rcc.h"

#include <cstring>

uint32_t SystemCoreClock = 144000000;
RCC_TypeDef hostRcc;
PWR_TypeDef hostPwr;
BKP_TypeDef hostBkp;
CRC_TypeDef hostCrc;

namespace {
// Clears all registers, runs the function and checks that only `bit` of `reg` changed.
bool setsOnly(void (*function)(), volatile uint32_t RCC_TypeDef::*reg, uint32_t bit) {
   std::memset(const_cast<RCC_TypeDef *>(&hostRcc), 0, sizeof(hostRcc));
   function();
   RCC_TypeDef expected{};
   expected.*reg = bit;
   return std::memcmp(const_cast<RCC_TypeDef *>(&hostRcc), &expected, sizeof(hostRcc)) == 0;
}

// Sets all bits, runs the reset function and checks that only `bit` of `reg` is clear after it.
bool clearsOnly(void (*function)(), volatile uint32_t RCC_TypeDef::*reg, uint32_t bit) {
   std::memset(const_cast<RCC_TypeDef *>(&hostRcc), 0xFF, sizeof(hostRcc));
   function();
   RCC_TypeDef expected;
   std::memset(&expected, 0xFF, sizeof(expected));
   expected.*reg = ~bit;
   return std::memcmp(const_cast<RCC_TypeDef *>(&hostRcc), &expected, sizeof(hostRcc)) == 0;
}
} // namespace

int main() {
   // RM 3.4.6 to 3.4.8: AHBPCENR, APB2PCENR, APB1PCENR.
   CHECK(setsOnly(Rcc::enableGPIOAClock, &RCC_TypeDef::APB2PCENR, 1ul << 2));
   CHECK(setsOnly(Rcc::enableGPIOBClock, &RCC_TypeDef::APB2PCENR, 1ul << 3));
   CHECK(setsOnly(Rcc::enableGPIOCClock, &RCC_TypeDef::APB2PCENR, 1ul << 4));
   CHECK(setsOnly(Rcc::enableAFIOClock, &RCC_TypeDef::APB2PCENR, 1ul << 0));
   CHECK(setsOnly(Rcc::enableUSART1Clock, &RCC_TypeDef::APB2PCENR, 1ul << 14));
   CHECK(setsOnly(Rcc::enableUSART2Clock, &RCC_TypeDef::APB1PCENR, 1ul << 17));
   CHECK(setsOnly(Rcc::enableUSART3Clock, &RCC_TypeDef::APB1PCENR, 1ul << 18));
   CHECK(setsOnly(Rcc::enableSPI1Clock, &RCC_TypeDef::APB2PCENR, 1ul << 12));
   CHECK(setsOnly(Rcc::enableSPI2Clock, &RCC_TypeDef::APB1PCENR, 1ul << 14));
   CHECK(setsOnly(Rcc::enableSPI3Clock, &RCC_TypeDef::APB1PCENR, 1ul << 15));
   CHECK(setsOnly(Rcc::enableTIM1Clock, &RCC_TypeDef::APB2PCENR, 1ul << 11));
   CHECK(setsOnly(Rcc::enableTIM3Clock, &RCC_TypeDef::APB1PCENR, 1ul << 1));
   CHECK(setsOnly(Rcc::enableTIM10Clock, &RCC_TypeDef::APB2PCENR, 1ul << 20));
   CHECK(setsOnly(Rcc::enableDMA1Clock, &RCC_TypeDef::AHBPCENR, 1ul << 0));
   CHECK(setsOnly(Rcc::enableDMA2Clock, &RCC_TypeDef::AHBPCENR, 1ul << 1));
   CHECK(setsOnly(Rcc::enableGPIODClock, &RCC_TypeDef::APB2PCENR, 1ul << 5));
   CHECK(setsOnly(Rcc::enableUART5Clock, &RCC_TypeDef::APB1PCENR, 1ul << 20));
   CHECK(setsOnly(Rcc::enableUART7Clock, &RCC_TypeDef::APB1PCENR, 1ul << 7));
   CHECK(setsOnly(Rcc::enableTIM5Clock, &RCC_TypeDef::APB1PCENR, 1ul << 3));
   CHECK(setsOnly(Rcc::enableDACClock, &RCC_TypeDef::APB1PCENR, 1ul << 29));
   CHECK(setsOnly(Rcc::enablePWRClock, &RCC_TypeDef::APB1PCENR, 1ul << 28));
   CHECK(setsOnly(Rcc::enableBKPClock, &RCC_TypeDef::APB1PCENR, 1ul << 27));
   CHECK(setsOnly(Rcc::enableCRCClock, &RCC_TypeDef::AHBPCENR, 1ul << 6));

   // A disable clears only its own bit.
   CHECK(clearsOnly(Rcc::disableTIM5Clock, &RCC_TypeDef::APB1PCENR, 1ul << 3));
   CHECK(clearsOnly(Rcc::disableDACClock, &RCC_TypeDef::APB1PCENR, 1ul << 29));
   CHECK(clearsOnly(Rcc::disablePWRClock, &RCC_TypeDef::APB1PCENR, 1ul << 28));
   CHECK(clearsOnly(Rcc::disableBKPClock, &RCC_TypeDef::APB1PCENR, 1ul << 27));
   CHECK(clearsOnly(Rcc::disableCRCClock, &RCC_TypeDef::AHBPCENR, 1ul << 6));
   CHECK(clearsOnly(Rcc::disableOTGFSClock, &RCC_TypeDef::AHBPCENR, 1ul << 12));

   // The clock tests read one bit each.
   hostRcc.APB1PCENR = 1ul << 17;
   CHECK(Rcc::isUSART2ClockEnabled());
   CHECK(!Rcc::isUSART3ClockEnabled());
   hostRcc.APB1PCENR = ~uint32_t{ 1u << 17 };
   CHECK(!Rcc::isUSART2ClockEnabled());
   CHECK(Rcc::isUSART3ClockEnabled());

   // An enable keeps the other clocks on.
   hostRcc.APB2PCENR = 0x00F00000;
   Rcc::enableSPI1Clock();
   CHECK(hostRcc.APB2PCENR == (0x00F00000 | (1ul << 12)));

   // RM 3.4.4 and 3.4.5: APB2PRSTR, APB1PRSTR. Host memory shows only the end state: the bit is clear.
   CHECK(clearsOnly(Rcc::resetSPI1, &RCC_TypeDef::APB2PRSTR, 1ul << 12));
   CHECK(clearsOnly(Rcc::resetTIM1, &RCC_TypeDef::APB2PRSTR, 1ul << 11));
   CHECK(clearsOnly(Rcc::resetTIM3, &RCC_TypeDef::APB1PRSTR, 1ul << 1));
   CHECK(clearsOnly(Rcc::resetSPI3, &RCC_TypeDef::APB1PRSTR, 1ul << 15));
   CHECK(clearsOnly(Rcc::resetTIM10, &RCC_TypeDef::APB2PRSTR, 1ul << 20));
   CHECK(clearsOnly(Rcc::resetTIM5, &RCC_TypeDef::APB1PRSTR, 1ul << 3));
   CHECK(clearsOnly(Rcc::resetDAC, &RCC_TypeDef::APB1PRSTR, 1ul << 29));
   CHECK(clearsOnly(Rcc::resetOTGFS, &RCC_TypeDef::AHBRSTR, 1ul << 12));

   // Bus clocks. HCLK is SystemCoreClock.
   hostRcc.CFGR0 = 0;
   CHECK(Rcc::getAHBClock() == 144000000);
   CHECK(Rcc::getAPBClock() == 144000000);
   CHECK(Rcc::getAPB2Clock() == 144000000);

   // PPRE2 = /2 (100) changes PCLK2 only. This is the setting of the firmware.
   hostRcc.CFGR0 = 4ul << 11;
   CHECK(Rcc::getAPBClock() == 144000000);
   CHECK(Rcc::getAPB2Clock() == 72000000);

   // PPRE1 = /4 (101) changes PCLK1 only. A field of 0xx divides by 1.
   hostRcc.CFGR0 = (5ul << 8) | (3ul << 11);
   CHECK(Rcc::getAPBClock() == 36000000);
   CHECK(Rcc::getAPB2Clock() == 144000000);

   // /16 (111) on both. The other CFGR0 fields do not change the result.
   hostRcc.CFGR0 = 0xFFFFFFFFul;
   CHECK(Rcc::getAPBClock() == 9000000);
   CHECK(Rcc::getAPB2Clock() == 9000000);

   // setAPB2Prescaler() changes PPRE2 (bits 13:11) only.
   SystemCoreClock = 144000000;
   hostRcc.CFGR0 = 0xFFFFFFFFul;
   Rcc::setAPB2Prescaler(Rcc::APBPrescaler::div1);
   CHECK(hostRcc.CFGR0 == ~uint32_t{ 7u << 11 });
   Rcc::setAPB2Prescaler(Rcc::APBPrescaler::div2);
   CHECK(hostRcc.CFGR0 == (~uint32_t{ 7u << 11 } | (4u << 11)));
   hostRcc.CFGR0 = 0;
   Rcc::setAPB2Prescaler(Rcc::APBPrescaler::div16);
   CHECK(hostRcc.CFGR0 == (7ul << 11));
   Rcc::setAPB2Prescaler(Rcc::APBPrescaler::div2);
   CHECK(Rcc::getAPB2Clock() == 72000000);
   CHECK(Rcc::getAPBClock() == 144000000);

   // getSystemClockSource() reads SWS (bits 3:2) only.
   hostRcc.CFGR0 = ~uint32_t{ 3u << 2 } | (2u << 2);
   CHECK(Rcc::getSystemClockSource() == Rcc::SystemClockSource::PLL);
   hostRcc.CFGR0 = 1ul << 2;
   CHECK(Rcc::getSystemClockSource() == Rcc::SystemClockSource::HSE);
   hostRcc.CFGR0 = 3ul; // SW = 11, SWS = 00
   CHECK(Rcc::getSystemClockSource() == Rcc::SystemClockSource::HSI);

   // hw::Bkp: enable() turns on the PWR and BKP clocks and sets DBP. disable() undoes it, the
   // data stay.
   std::memset(const_cast<RCC_TypeDef *>(&hostRcc), 0, sizeof(hostRcc));
   hostPwr = {};
   hostBkp = {};
   hw::Bkp::enable();
   CHECK(hostRcc.APB1PCENR == ((1ul << 28) | (1ul << 27)));
   CHECK(hostPwr.CTLR == (1ul << 8));
   CHECK(hw::Pwr::isBackupDomainAccessEnabled());
   hw::Bkp::writeDR1(0xB007);
   hw::Bkp::writeDR2(0x60A7);
   CHECK(hostBkp.DATAR1 == 0xB007);
   CHECK(hostBkp.DATAR2 == 0x60A7);
   CHECK(hw::Bkp::readDR1() == 0xB007);
   CHECK(hw::Bkp::readDR2() == 0x60A7);
   hw::Bkp::disable();
   CHECK(hostPwr.CTLR == 0);
   CHECK(hostRcc.APB1PCENR == 0);
   CHECK(hw::Bkp::readDR1() == 0xB007);

   // hw::Crc: reset() writes the RESET bit, add() and value() use DATAR.
   hostCrc = {};
   hw::Crc::reset();
   CHECK(hostCrc.CTLR == 1);
   hw::Crc::add(0x12345678);
   CHECK(hostCrc.DATAR == 0x12345678);
   CHECK(hw::Crc::value() == 0x12345678);

   SystemCoreClock = 8000000; // HSI
   hostRcc.CFGR0 = 6ul << 8;  // PPRE1 = /8
   CHECK(Rcc::getAHBClock() == 8000000);
   CHECK(Rcc::getAPBClock() == 1000000);

   return failures;
}
