#pragma once

// Clock enables and resets of CH32V30X (RM chapter 3). Include <peripherals/Rcc.h>, not this file.

namespace Rcc {
inline void enableGPIOAClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPAEN);
}
inline void enableGPIOBClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPBEN);
}
inline void enableGPIOCClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPCEN);
}
inline void enableGPIODClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPDEN);
}
inline void enableAFIOClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_AFIOEN);
}
inline void enableUSART1Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_USART1EN);
}
inline void enableUSART2Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_USART2EN);
}
inline void enableUSART3Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_USART3EN);
}
inline bool isUSART2ClockEnabled() {
   return (RCC->APB1PCENR & RCC_USART2EN) != 0;
}
inline bool isUSART3ClockEnabled() {
   return (RCC->APB1PCENR & RCC_USART3EN) != 0;
}
inline void enableUART5Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_UART5EN);
}
inline void enableUART7Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_UART7EN);
}
inline void enableSPI1Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_SPI1EN);
}
inline void enableSPI2Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_SPI2EN);
}
inline void enableSPI3Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_SPI3EN);
}
inline void enableTIM1Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_TIM1EN);
}
inline void enableTIM3Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_TIM3EN);
}
inline void enableTIM5Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_TIM5EN);
}
inline void disableTIM5Clock() {
   RCC->APB1PCENR &= ~RCC_TIM5EN;
}
inline void enableTIM10Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_TIM10EN);
}
inline void enableDMA1Clock() {
   detail::enableClock(RCC->AHBPCENR, RCC_DMA1EN);
}
inline void enableDMA2Clock() {
   detail::enableClock(RCC->AHBPCENR, RCC_DMA2EN);
}
inline void enableCRCClock() {
   detail::enableClock(RCC->AHBPCENR, RCC_CRCEN);
}
inline void disableCRCClock() {
   RCC->AHBPCENR &= ~RCC_CRCEN;
}
// USBFS (OTG_FS). The clock is off after a reset.
inline void disableOTGFSClock() {
   RCC->AHBPCENR &= ~RCC_OTGFSEN;
}
inline void enableDACClock() {
   detail::enableClock(RCC->APB1PCENR, RCC_DACEN);
}
inline void disableDACClock() {
   RCC->APB1PCENR &= ~RCC_DACEN;
}
inline void enablePWRClock() {
   detail::enableClock(RCC->APB1PCENR, RCC_PWREN);
}
inline void disablePWRClock() {
   RCC->APB1PCENR &= ~RCC_PWREN;
}
inline void enableBKPClock() {
   detail::enableClock(RCC->APB1PCENR, RCC_BKPEN);
}
inline void disableBKPClock() {
   RCC->APB1PCENR &= ~RCC_BKPEN;
}

inline void resetSPI1() {
   detail::resetPeripheral(RCC->APB2PRSTR, RCC_SPI1RST);
}
inline void resetSPI3() {
   detail::resetPeripheral(RCC->APB1PRSTR, RCC_SPI3RST);
}
inline void resetTIM1() {
   detail::resetPeripheral(RCC->APB2PRSTR, RCC_TIM1RST);
}
inline void resetTIM3() {
   detail::resetPeripheral(RCC->APB1PRSTR, RCC_TIM3RST);
}
inline void resetTIM5() {
   detail::resetPeripheral(RCC->APB1PRSTR, RCC_TIM5RST);
}
inline void resetTIM10() {
   detail::resetPeripheral(RCC->APB2PRSTR, RCC_TIM10RST);
}
inline void resetDAC() {
   detail::resetPeripheral(RCC->APB1PRSTR, RCC_DACRST);
}
inline void resetOTGFS() {
   detail::resetPeripheral(RCC->AHBRSTR, RCC_OTGFSRST);
}
} // namespace Rcc
