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
inline void enableTIM10Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_TIM10EN);
}
inline void enableDMA1Clock() {
   detail::enableClock(RCC->AHBPCENR, RCC_DMA1EN);
}
inline void enableDMA2Clock() {
   detail::enableClock(RCC->AHBPCENR, RCC_DMA2EN);
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
inline void resetTIM10() {
   detail::resetPeripheral(RCC->APB2PRSTR, RCC_TIM10RST);
}
} // namespace Rcc
