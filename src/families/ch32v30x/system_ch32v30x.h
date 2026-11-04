#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Clock of the CPU core, in Hz. SystemInit() sets it. */
extern uint32_t SystemCoreClock;

/* Set the system clock to 144 MHz from the external crystal (HSE). */
void SystemInit(void);

/* Update SystemCoreClock from the clock registers. */
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif
