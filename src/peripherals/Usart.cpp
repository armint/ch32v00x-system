#include "Usart.h"
#include <system_ch32v00X.h>

void Usart::setBaudRate(uint32_t baudRate) const {
   usart.BRR = calculateBRR(baudRate, SystemCoreClock);
}

uint32_t Usart::getBaudRate() const {
   uint32_t clk = SystemCoreClock;
   uint32_t brr = usart.BRR;
   if (brr == 0) return 0;
   
   return clk / brr;
}

