#pragma once

// Host stub of <wch_device.h> for the SysTick time base (src/Timekeeping.cpp): the SysTick
// registers of the CH32V30x as one host variable. The layout is the one of SysTick_Type in
// src/core_riscv.h.
#include <cstdint>

#define __IO volatile

typedef enum { SysTick_IRQn = 12 } IRQn_Type;
#define SysTicK_IRQn SysTick_IRQn
#define SYSTICK_64BIT 1 // as in src/families/ch32v30x/ch32v30x.h

typedef struct {
   __IO uint32_t CTLR;
   __IO uint32_t SR;
   __IO uint32_t CNT;
   __IO uint32_t CNTH;
   __IO uint32_t CMP;
   __IO uint32_t CMPH;
} SysTick_Type;

extern SysTick_Type hostSysTick;
#define SysTick (&hostSysTick)

inline void NVIC_EnableIRQ(IRQn_Type) {
}

// As in src/core_riscv.h
extern "C" uint32_t SysTick_Config(uint32_t ticks);
