#pragma once

// Clock enables, peripheral resets and bus clocks of the RCC. The family header
// (Rcc_V00x.h, Rcc_V20x.h or Rcc_V30x.h) gives the bus of each peripheral.

#include <cstdint>
#include <wch_device.h>

namespace Rcc {
namespace detail {
// The read back makes sure that the clock is on before the next access to the peripheral.
inline void enableClock(volatile uint32_t &reg, uint32_t bit) {
   reg |= bit;
   [[maybe_unused]] const uint32_t readBack = reg;
}

// Sets and clears the reset bit: the peripheral registers go back to their reset values.
inline void resetPeripheral(volatile uint32_t &reg, uint32_t bit) {
   reg |= bit;
   reg &= ~bit;
}
} // namespace detail

// HCLK: SystemCoreClock. Call SystemCoreClockUpdate() after a change of the clock tree.
// On CH32V30X, SystemCoreClockUpdate() ignores HPRE: this value is right only for HPRE = /1,
// which the start-up sets.
uint32_t getAHBClock();
// PCLK1, the clock of the APB1 peripherals. CH32V00X has no APB prescaler: HCLK.
uint32_t getAPBClock();
// PCLK2, the clock of the APB2 peripherals. CH32V00X has no APB prescaler: HCLK.
uint32_t getAPB2Clock();

// The clock of SYSCLK (RCC_CFGR0.SWS). On CH32V30X, SystemInit() feeds the PLL from the crystal and
// keeps HSI when the crystal or the PLL does not start, so PLL also tells that the crystal runs. On
// CH32V00X and CH32V20X, SystemInit() feeds the PLL from HSI.
enum class SystemClockSource : uint32_t {
   HSI = RCC_SWS_HSI,
   HSE = RCC_SWS_HSE,
   PLL = RCC_SWS_PLL,
};

inline SystemClockSource getSystemClockSource() {
   return static_cast<SystemClockSource>(RCC->CFGR0 & RCC_SWS);
}

#if defined(RCC_PPRE2)
// The divider of an APB clock from HCLK. The values are the PPRE1 field of RCC_CFGR0.
enum class APBPrescaler : uint32_t {
   div1 = 0,
   div2 = RCC_PPRE1_DIV2,
   div4 = RCC_PPRE1_DIV4,
   div8 = RCC_PPRE1_DIV8,
   div16 = RCC_PPRE1_DIV16,
};

// PCLK2 = HCLK / prescaler. A peripheral that took its clock at its setup, for example the baud
// rate of a USART on APB2, does not follow a later change.
inline void setAPB2Prescaler(APBPrescaler prescaler) {
   // PPRE2 is PPRE1 moved by 3 bits
   static_assert(RCC_PPRE2_DIV2 == RCC_PPRE1_DIV2 << 3 && RCC_PPRE2 == RCC_PPRE1 << 3);
   RCC->CFGR0 = (RCC->CFGR0 & ~RCC_PPRE2) | (static_cast<uint32_t>(prescaler) << 3);
}
#endif
} // namespace Rcc

#if __has_include(<Rcc_V30x.h>)
#include <Rcc_V30x.h>
#elif __has_include(<Rcc_V20x.h>)
#include <Rcc_V20x.h>
#elif __has_include(<Rcc_V00x.h>)
#include <Rcc_V00x.h>
#else
#error "No Rcc family header: check CH32_FAMILY"
#endif
