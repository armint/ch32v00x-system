// Host test of the baud rate of the CH32V30x Usart (src/peripherals/Usart.cpp):
// USART1 takes its clock from APB2 (PPRE2), the other USARTs from APB1 (PPRE1).
// Registers are host memory (stub <wch_device.h> in stubs-usart-v30x/).

#include "check.h"
#include "peripherals/Usart.h"

uint32_t SystemCoreClock = 144000000;
RCC_TypeDef hostRcc;
USART_TypeDef hostUsart1;
USART_TypeDef hostUsart2;

int main() {
   Usart u1(USART1);
   Usart u2(USART2);

   // No prescaler: the bus clock is SystemCoreClock.
   hostRcc.CFGR0 = 0;
   u1.setBaudRate(115200);
   CHECK(hostUsart1.BRR == (144000000 + 57600) / 115200);
   CHECK(u1.getBaudRate() == 144000000 / hostUsart1.BRR);

   // PPRE2 = /2 halves the clock of USART1 only. BRR halves too, so the baud rate stays.
   hostRcc.CFGR0 = 4ul << 11;
   u1.setBaudRate(115200);
   CHECK(hostUsart1.BRR == (72000000 + 57600) / 115200);
   CHECK(u1.getBaudRate() == 72000000 / hostUsart1.BRR);
   u2.setBaudRate(115200);
   CHECK(hostUsart2.BRR == (144000000 + 57600) / 115200);

   // PPRE1 = /4 (101) acts on USART2 only.
   hostRcc.CFGR0 = 5ul << 8;
   u2.setBaudRate(115200);
   CHECK(hostUsart2.BRR == (36000000 + 57600) / 115200);
   u1.setBaudRate(115200);
   CHECK(hostUsart1.BRR == (144000000 + 57600) / 115200);

   // PPRE2 = /16 (111) and the other bits of CFGR0 do not matter.
   hostRcc.CFGR0 = 0xFFFFFFFFul;
   u1.setBaudRate(115200);
   CHECK(hostUsart1.BRR == (9000000 + 57600) / 115200);

   if(failures == 0) {
      std::printf("usart-v30x: all checks passed\n");
   }
   return failures;
}
