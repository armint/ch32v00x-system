#include "systick.h"
#include "ch32v00x.h"
#include "core_riscv.h"

static volatile timestamp_t sysTickCount;

extern "C" void __attribute__((section(".after_vectors"), interrupt)) SysTick_Handler(void) {
   SysTick->SR &= ~SYSTICK_SR_CNTIF;
   sysTickUpdateHook(++sysTickCount);
}

timestamp_t getSysTick() {
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
