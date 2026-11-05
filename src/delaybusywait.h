#pragma once

#include <stdint.h>
#include <utils/Timekeeping.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

// Needs a 1 ms tick (SysTick_Config(SystemCoreClock / 1000)).
void delaymS(duration_t msec);
void delayuS(duration_t usec);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
