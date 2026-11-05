#include "Usart.h"
#include <wch_device.h>

#if defined(RCC_CFGR0_PPRE2_MASK)
// CH32V30x: USART1 is on APB2, the other USARTs are on APB1. SystemCoreClock is
// SYSCLK, so this assumes the AHB prescaler (HPRE) is /1, as the start-up sets it.
// The BRR value holds the bus clock of the moment: call setBaudRate() again after
// you change PPRE1 or PPRE2.
static uint32_t busClock(const USART_TypeDef &usart) {
   const uint32_t cfgr = RCC->CFGR0;
   // Field 0xx is /1. 100, 101, 110 and 111 are /2, /4, /8 and /16.
   const uint32_t field = (&usart == USART1) ? (cfgr & RCC_CFGR0_PPRE2_MASK) >> 11 : (cfgr & RCC_CFGR0_PPRE1_MASK) >> 8;
   return SystemCoreClock >> ((field & 4) ? (field & 3) + 1 : 0);
}
#else
static uint32_t busClock(const USART_TypeDef &) {
   return SystemCoreClock;
}
#endif

void Usart::setBaudRate(uint32_t baudRate) const {
   usart.BRR = calculateBRR(baudRate, busClock(usart));
}

uint32_t Usart::getBaudRate() const {
   uint32_t clk = busClock(usart);
   uint32_t brr = usart.BRR;
   if(brr == 0)
      return 0;

   return clk / brr;
}
