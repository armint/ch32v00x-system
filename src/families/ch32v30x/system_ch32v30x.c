#include "system_ch32v30x.h"
#include "ch32v30x.h"

#if defined(CH32V30x_D8C)

#define HSE_STARTUP_TIMEOUT 0x5000u
#define SYSCLK_PLL_HZ 144000000u /* HSE x 18 */

uint32_t SystemCoreClock = HSI_VALUE;

/* Start HSE and the PLL, then switch SYSCLK to the PLL. The core keeps HSI
 * when HSE or the PLL does not become ready. The time-out is a bounded loop. */
void SystemInit(void) {
   uint32_t timeout = 0;

   RCC->CTLR |= RCC_HSEON;
   while (!(RCC->CTLR & RCC_HSERDY)) {
      if (++timeout > HSE_STARTUP_TIMEOUT) {
         return; /* SYSCLK stays on HSI */
      }
   }

   /* The PLL input must not change while the PLL runs, so set it first:
    * HSE with PREDIV1 undivided, PLL fed by PREDIV1, PLLMUL x18, no AHB or APB division. */
   RCC->CFGR2 &= ~(RCC_CFGR2_PREDIV1_MASK | RCC_CFGR2_PREDIV1SRC);
   RCC->CFGR0 = (RCC->CFGR0 & ~(RCC_CFGR0_PLLMUL_MASK | RCC_CFGR0_HPRE_MASK | RCC_CFGR0_PPRE1_MASK |
                                RCC_CFGR0_PPRE2_MASK)) |
                RCC_CFGR0_PLLSRC | RCC_CFGR0_PLLMUL_X18;

   RCC->CTLR |= RCC_PLLON;
   timeout = 0;
   while (!(RCC->CTLR & RCC_PLLRDY)) {
      if (++timeout > HSE_STARTUP_TIMEOUT) {
         return; /* SYSCLK stays on HSI */
      }
   }

   RCC->CFGR0 = (RCC->CFGR0 & ~RCC_CFGR0_SW_MASK) | RCC_CFGR0_SW_PLL;
   while ((RCC->CFGR0 & RCC_CFGR0_SWS_MASK) != RCC_CFGR0_SWS_PLL) {
   }

   SystemCoreClock = SYSCLK_PLL_HZ;
}

/* Read the clock source of SYSCLK. The PLL value is the one set by SystemInit(). */
void SystemCoreClockUpdate(void) {
   switch (RCC->CFGR0 & RCC_CFGR0_SWS_MASK) {
   case 0:
      SystemCoreClock = HSI_VALUE;
      break;
   case RCC_CFGR0_SWS_PLL:
      SystemCoreClock = SYSCLK_PLL_HZ;
      break;
   default:
      SystemCoreClock = HSE_VALUE;
      break;
   }
}

#endif /* CH32V30x_D8C */
