#pragma once

// Clock enables and resets of CH32V00X. Include <peripherals/Rcc.h>, not this file.
// CH32V003 has no port B, no USART2 and no TIM3: the device header has no bits for them.
// On CH32V002 and CH32V006, USART2 is on APB2 (PB2), not on APB1.

namespace Rcc {
inline void enableGPIOAClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPAEN);
}
#if defined(RCC_IOPBEN)
inline void enableGPIOBClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPBEN);
}
#endif
inline void enableGPIOCClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_IOPCEN);
}
inline void enableAFIOClock() {
   detail::enableClock(RCC->APB2PCENR, RCC_AFIOEN);
}
inline void enableUSART1Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_USART1EN);
}
#if defined(RCC_USART2EN)
inline void enableUSART2Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_USART2EN);
}
#endif
inline void enableSPI1Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_SPI1EN);
}
inline void enableTIM1Clock() {
   detail::enableClock(RCC->APB2PCENR, RCC_TIM1EN);
}
#if defined(RCC_TIM3EN)
inline void enableTIM3Clock() {
   detail::enableClock(RCC->APB1PCENR, RCC_TIM3EN);
}
#endif
inline void enableDMA1Clock() {
   detail::enableClock(RCC->AHBPCENR, RCC_DMA1EN);
}

inline void resetSPI1() {
   detail::resetPeripheral(RCC->APB2PRSTR, RCC_SPI1RST);
}
inline void resetTIM1() {
   detail::resetPeripheral(RCC->APB2PRSTR, RCC_TIM1RST);
}
#if defined(RCC_TIM3RST)
inline void resetTIM3() {
   detail::resetPeripheral(RCC->APB1PRSTR, RCC_TIM3RST);
}
#endif
} // namespace Rcc
