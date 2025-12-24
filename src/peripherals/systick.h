#pragma once

#include <stdint.h>


#define SYSTICK_CTRL_SWIE (1ul << 31) /* Software interrupt enable */
#define SYSTICK_CTRL_STRE (1ul << 3)  /* Auto reload enable */
#define SYSTICK_CTRL_STCLK (1ul << 2) /* Clock source divide */
#define SYSTICK_CTRL_STIE (1ul << 1)  /* Counter interrupt enable */
#define SYSTICK_CTRL_STE (1ul << 0)   /* Counter enable */

#define SYSTICK_SR_CNTIF (1ul << 0) /* Counting value comparison flag */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
typedef int64_t timestamp_t;
typedef uint32_t duration_t;


void initSysTick(uint32_t cycles);
timestamp_t getSysTick();

void sysTickUpdateHook(timestamp_t sysTick);
#if defined(__cplusplus)
}
#endif /* __cplusplus */
