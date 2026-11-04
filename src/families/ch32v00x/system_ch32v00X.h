#pragma once
#include <stdint.h>

#include <wch_device.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock; /* System Clock Frequency (Core Clock) */

void SystemInit(void);
void SystemCoreClockUpdate(void);
uint32_t getAHBClockFrequency();

void SetSysClockTo_24MHz_HSI(void);
void SetSysClockTo_24MHz_HSE(void);
void SetSysClockTo_48MHz_HSI(void);
void SetSysClockTo_48MHz_HSE(void);
void SetSysClockTo_8MHz_HSI(void);
void SetSysClockTo_8MHz_HSE(void);

#ifdef __cplusplus
}
#endif
