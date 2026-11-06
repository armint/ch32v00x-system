#include "Rcc.h"

namespace Rcc {
namespace {
#if defined(RCC_CFGR0_PPRE1_MASK) || defined(RCC_PPRE1)
// CH32V20X and CH32V30X: PPRE1 is CFGR0 bits 10:8, PPRE2 is bits 13:11.
constexpr uint32_t ppre1Shift = 8;
constexpr uint32_t ppre2Shift = 11;

// A PPRE field of 0xx divides by 1. The values 100 to 111 divide by 2, 4, 8 and 16.
uint32_t apbClock(uint32_t shift) {
   const uint32_t field = (RCC->CFGR0 >> shift) & 7u;
   return getAHBClock() >> ((field & 4u) ? (field & 3u) + 1 : 0);
}
#endif
} // namespace

uint32_t getAHBClock() {
   return SystemCoreClock;
}

#if defined(RCC_CFGR0_PPRE1_MASK) || defined(RCC_PPRE1)
uint32_t getAPBClock() {
   return apbClock(ppre1Shift);
}

uint32_t getAPB2Clock() {
   return apbClock(ppre2Shift);
}
#else
uint32_t getAPBClock() {
   return getAHBClock();
}

uint32_t getAPB2Clock() {
   return getAHBClock();
}
#endif
} // namespace Rcc
