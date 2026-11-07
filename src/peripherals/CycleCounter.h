#pragma once

#include <cstdint>
#include <wch_device.h>

// A 32-bit core clock cycle counter. The QingKe V4 core has no mcycle CSR, so the counter is
// SysTick->CNT, the low word of the 64-bit SysTick counter. SysTick_Config() runs that counter free
// at the core clock (utils/Timekeeping.h). The count wraps every 2^32 cycles (29.8 s at 144 MHz):
// use it for differences only. See docs/cycle-counter.md.
//
// CH32V00X has no cycle counter: the QingKe V2 core has no mcycle CSR, and its 32-bit SysTick
// counter starts again from 0 at each tick.
#if !defined(SYSTICK_64BIT)
#error "hw::CycleCounter: this family has no free-running SysTick counter (CH32V20X and CH32V30X only)"
#endif

namespace hw::CycleCounter {

/**
 * @brief Starts the counter if it does not run. Call it before a series of count() calls.
 *
 * When SysTick_Config() has run, the counter runs and the function changes nothing. When the
 * counter is off, it starts at the core clock, without interrupt and without auto reload. A
 * SysTick_Config() after that sets the counter to 0 again.
 */
[[gnu::always_inline]] inline void start() {
   constexpr uint32_t ctlrSte = 1u << 0;   // counter enable
   constexpr uint32_t ctlrStclk = 1u << 2; // HCLK, not HCLK/8
   if((SysTick->CTLR & ctlrSte) == 0u) {
      SysTick->CTLR = ctlrStclk | ctlrSte;
   }
}

/**
 * @brief Returns the core clock cycles. The counter is free-running and wraps at 2^32.
 */
[[gnu::always_inline]] inline uint32_t count() {
   return SysTick->CNT;
}

} // namespace hw::CycleCounter
