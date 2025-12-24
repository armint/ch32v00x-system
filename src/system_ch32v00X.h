#pragma once
#include <stdint.h>

#include <ch32v00X.h>

#ifdef __cplusplus
extern "C" {
#endif

 extern uint32_t SystemCoreClock; /* System Clock Frequency (Core Clock) */

 void SystemInit(void);
 void SystemCoreClockUpdate(void);
 uint32_t getAHBClockFrequency();

 void SetSysClockTo_24MHz_HSI(void);
 void SetSysClockTo_24MHz_HSE(void);
 void SetSysClockTo_48MHZ_HSI(void);
 void SetSysClockTo_48MHz_HSE(void);
 void SetSysClockTo_8MHz_HSI(void);
 void SetSysClockTo_8MHz_HSE(void);

 inline void __attribute__((always_inline)) cpuInit(void) {
  extern void Reset_Handler();
  // Set the GP register to allow relative memory access
  asm volatile(".option push\n"
               ".option norelax\n"
               "la gp, __global_pointer$\n"
               ".option pop\n"
               "la sp, __stack\n"
               "li t0, 0x2\n"
               "csrw 0x804, t0\n");
  __set_MSTATUS(0x1880);
  // Configure the interrupt vector table
  __set_MTVEC(((uint32_t)&Reset_Handler | 0b11));
 }

#ifdef __cplusplus
}
#endif

