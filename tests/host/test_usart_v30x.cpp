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
   hw::Usart u1(USART1);
   hw::Usart u2(USART2);

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

   // getClockFrequency() is the bus clock of the USART.
   hostRcc.CFGR0 = 4ul << 11; // PPRE2 /2
   CHECK(u1.getClockFrequency() == 72000000);
   CHECK(u2.getClockFrequency() == 144000000);

   // setFrameFormat(): 8 data bits with parity are a 9-bit word (M). Other bits of CTLR1 stay.
   constexpr uint16_t te = 0x0008;
   hostUsart1.CTLR1 = USART_CTLR1_UE | te;
   hostUsart1.CTLR2 = 0; // no stop bits yet
   CHECK(u1.setFrameFormat(hw::Usart::DataBits::Eight, hw::Usart::Parity::Even, hw::Usart::StopBits::StopBits_2));
   CHECK(hostUsart1.CTLR1 == (USART_CTLR1_UE | te | USART_CTLR1_PCE | USART_CTLR1_M));
   CHECK((hostUsart1.CTLR2 & USART_CTLR2_STOP) == USART_CTLR2_STOP_1);
   // 7 data bits with odd parity are an 8-bit word: M is clear.
   CHECK(u1.setFrameFormat(hw::Usart::DataBits::Seven, hw::Usart::Parity::Odd, hw::Usart::StopBits::StopBits_1));
   CHECK(hostUsart1.CTLR1 == (USART_CTLR1_UE | te | USART_CTLR1_PCE | USART_CTLR1_PS));
   CHECK((hostUsart1.CTLR2 & USART_CTLR2_STOP) == 0);
   // 8 data bits without parity: no parity, no M.
   CHECK(u1.setFrameFormat(hw::Usart::DataBits::Eight, hw::Usart::Parity::None, hw::Usart::StopBits::StopBits_1));
   CHECK(hostUsart1.CTLR1 == (USART_CTLR1_UE | te));
   // 7 data bits without parity cannot be made: false, and nothing changes.
   hostUsart1.CTLR2 = USART_CTLR2_STOP_0;
   CHECK(!u1.setFrameFormat(hw::Usart::DataBits::Seven, hw::Usart::Parity::None, hw::Usart::StopBits::StopBits_2));
   CHECK(hostUsart1.CTLR1 == (USART_CTLR1_UE | te));
   CHECK(hostUsart1.CTLR2 == USART_CTLR2_STOP_0);
   // A USART that is off stays off.
   hostUsart1.CTLR1 = 0;
   CHECK(u1.setFrameFormat(hw::Usart::DataBits::Eight, hw::Usart::Parity::Even, hw::Usart::StopBits::StopBits_1));
   CHECK((hostUsart1.CTLR1 & USART_CTLR1_UE) == 0);

   if(failures == 0) {
      std::printf("usart-v30x: all checks passed\n");
   }
   return failures;
}
