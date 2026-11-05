#pragma once

#include <stdint.h>

// System tick count, with the names of system-stm32. SysTick_Config()
// (core_riscv.h) starts the tick; the application chooses the tick rate,
// e.g. SysTick_Config(SystemCoreClock / 1000) for 1 ms.

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef int64_t timestamp_t;
typedef uint32_t duration_t;

// Ticks since SysTick_Config()
timestamp_t getUptimeTicks();

// Called from the SysTick interrupt with the new tick count. Weak: the
// application can replace it.
void sysTickUpdateHook(timestamp_t sysTick);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
