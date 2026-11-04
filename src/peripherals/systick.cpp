#include "systick.h"
#include "utils/Interrupt.h"
#include "wch_device.h"

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

timestamp_t getSysTick() {
   // The 64-bit count takes two 32-bit loads. Block the interrupt during the read,
   // so the SysTick handler cannot change the count between the two loads.
   Interrupt::CriticalSection guard;
   return sysTickCount;
}

void __attribute__((weak)) sysTickUpdateHook(timestamp_t sysTick) {
   (void)sysTick;
}

void initSysTick(uint32_t cycles) {
   sysTickCount = 0;
   SysTick->CTLR = 0;
   /* enable the SysTick IRQ */
   NVIC_EnableIRQ(SysTicK_IRQn);
   /* Clear the interrupt flag */
   SysTick->SR &= ~SYSTICK_SR_CNTIF;
   SysTick->CMP = cycles - 1;
   SysTick->CNT = 0;
   SysTick->CTLR = SYSTICK_CTRL_STRE | SYSTICK_CTRL_STCLK | SYSTICK_CTRL_STIE | SYSTICK_CTRL_STE;
}
