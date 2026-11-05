#include "utils/Timekeeping.h"
#include "utils/Interrupt.h"
#include "wch_device.h"

// SysTick CTLR bits (QingKe V2/V4 processor manuals)
#define SYSTICK_CTRL_STRE (1ul << 3)  /* Auto reload enable */
#define SYSTICK_CTRL_STCLK (1ul << 2) /* HCLK, not HCLK/8 */
#define SYSTICK_CTRL_STIE (1ul << 1)  /* Counter interrupt enable */
#define SYSTICK_CTRL_STE (1ul << 0)   /* Counter enable */

#define SYSTICK_SR_CNTIF (1ul << 0) /* Counting value comparison flag */

// CH32V20X and CH32V30X have a 64-bit counter and compare value (CH32FV2x_V3xRM
// 9.5.5). The high words keep the values of earlier code (e.g. a loader).
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) || defined(CH32V30x_D8C)
#define SYSTICK_64BIT 1
#else
#define SYSTICK_64BIT 0
#endif

static volatile timestamp_t sysTickCount;

extern "C" void __attribute__((section(".after_vectors"), interrupt)) SysTick_Handler(void) {
   SysTick->SR &= ~SYSTICK_SR_CNTIF;
   // Count under the guard: a higher-priority IRQ must not read the count
   // between the two 32-bit stores. The hook runs outside the guard.
   timestamp_t now;
   {
      Interrupt::CriticalSection guard;
      sysTickCount = sysTickCount + 1;
      now = sysTickCount;
   }
   sysTickUpdateHook(now);
}

timestamp_t getUptimeTicks() {
   // The 64-bit count takes two 32-bit loads. Block the interrupt during the read,
   // so the SysTick handler cannot change the count between the two loads.
   Interrupt::CriticalSection guard;
   return sysTickCount;
}

void __attribute__((weak)) sysTickUpdateHook(timestamp_t sysTick) {
   (void)sysTick;
}

uint32_t SysTick_Config(uint32_t ticks) {
   if(ticks == 0) {
      return 1;
   }
   SysTick->CTLR = 0;
   NVIC_EnableIRQ(SysTicK_IRQn);
   SysTick->SR &= ~SYSTICK_SR_CNTIF;
   SysTick->CMP = ticks - 1;
   SysTick->CNT = 0;
#if SYSTICK_64BIT
   // A non-zero high word would delay the first compare match without end.
   SysTick->CMPH = 0;
   SysTick->CNTH = 0;
#endif
   SysTick->CTLR = SYSTICK_CTRL_STRE | SYSTICK_CTRL_STCLK | SYSTICK_CTRL_STIE | SYSTICK_CTRL_STE;
   return 0;
}
