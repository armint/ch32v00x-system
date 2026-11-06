#pragma once

// Host stand-in for <wch_device.h> of CH32V30X: the GPIO and TIM register blocks,
// with the layout of the family device header.

#include <stdint.h>

typedef struct {
   volatile uint32_t CFGLR; /* 0x00 Port configuration low */
   volatile uint32_t CFGHR; /* 0x04 Port configuration high */
   volatile uint32_t INDR;  /* 0x08 Input data */
   volatile uint32_t OUTDR; /* 0x0C Output data */
   volatile uint32_t BSHR;  /* 0x10 Set and reset */
   volatile uint32_t BCR;   /* 0x14 Reset */
   volatile uint32_t LCKR;  /* 0x18 Configuration lock */
} GPIO_TypeDef;

/* TIM: the layout of the CH32V30X device header (32-bit fields). */
typedef struct {
   volatile uint32_t CTLR1;
   volatile uint32_t CTLR2;
   volatile uint32_t SMCFGR;
   volatile uint32_t DMAINTENR;
   volatile uint32_t INTFR;
   volatile uint32_t SWEVGR;
   volatile uint32_t CHCTLR1;
   volatile uint32_t CHCTLR2;
   volatile uint32_t CCER;
   volatile uint32_t CNT;
   volatile uint32_t PSC;
   volatile uint32_t ATRLR;
   volatile uint32_t RPTCR;
   volatile uint32_t CH1CVR;
   volatile uint32_t CH2CVR;
   volatile uint32_t CH3CVR;
   volatile uint32_t CH4CVR;
   volatile uint32_t BDTR;
   volatile uint32_t DMACFGR;
   volatile uint32_t DMAADR;
} TIM_TypeDef;

#define TIM_BIT(value) ((value) * 1ul)
#include "tim_bits.h"
