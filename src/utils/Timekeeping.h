#pragma once

#include <stdint.h>

// System tick count, with the names of system-stm32. SysTick_Config()
// (core_riscv.h) starts the tick; the application chooses the tick rate,
// e.g. SysTick_Config(SystemCoreClock / 1000) for 1 ms.
//
// On CH32V20X and CH32V30X the SysTick counter is 64 bits wide and runs free at the core clock
// (STCLK, no auto reload). SysTick->CNT, its low word, is then a 32-bit cycle counter: it wraps
// every 2^32 cycles (29.8 s at 144 MHz), so use it for differences only. The interrupt comes from
// the compare value, which the handler moves forward by the tick period. On CH32V00X the counter
// reloads at each tick, so SysTick->CNT is not a cycle counter there. hw::CycleCounter
// (peripherals/CycleCounter.h) reads SysTick->CNT.

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef int64_t timestamp_t;
typedef uint32_t duration_t;

// Ticks since the start. SysTick_Config() does not reset the count.
timestamp_t getUptimeTicks();

// Called from the SysTick interrupt with the new tick count. Weak: the
// application can replace it.
void sysTickUpdateHook(timestamp_t sysTick);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
