#pragma once

// Host stand-in for <wch_device.h> of CH32V00X: the GPIO and TIM register blocks,
// with the layout of the family device header.

#include <stdint.h>

typedef struct {
   volatile uint32_t CFGLR; /* 0x00 Port configuration low */
   uint32_t RESERVED;       /* 0x04 No CFGHR: 8 pins per port */
   volatile uint32_t INDR;  /* 0x08 Input data */
   volatile uint32_t OUTDR; /* 0x0C Output data */
   volatile uint32_t BSHR;  /* 0x10 Set and reset */
   volatile uint32_t BCR;   /* 0x14 Reset */
   volatile uint32_t LCKR;  /* 0x18 Configuration lock */
} GPIO_TypeDef;

/* TIM: the layout of the CH32V00X device header (16-bit fields, 32-bit CHxCVR). */
typedef struct {
   volatile uint16_t CTLR1;
   uint16_t RESERVED0;
   volatile uint16_t CTLR2;
   uint16_t RESERVED1;
   volatile uint16_t SMCFGR;
   uint16_t RESERVED2;
   volatile uint16_t DMAINTENR;
   uint16_t RESERVED3;
   volatile uint16_t INTFR;
   uint16_t RESERVED4;
   volatile uint16_t SWEVGR;
   uint16_t RESERVED5;
   volatile uint16_t CHCTLR1;
   uint16_t RESERVED6;
   volatile uint16_t CHCTLR2;
   uint16_t RESERVED7;
   volatile uint16_t CCER;
   uint16_t RESERVED8;
   volatile uint16_t CNT;
   uint16_t RESERVED9;
   volatile uint16_t PSC;
   uint16_t RESERVED10;
   volatile uint16_t ATRLR;
   uint16_t RESERVED11;
   volatile uint16_t RPTCR;
   uint16_t RESERVED12;
   volatile uint32_t CH1CVR;
   volatile uint32_t CH2CVR;
   volatile uint32_t CH3CVR;
   volatile uint32_t CH4CVR;
   volatile uint16_t BDTR;
   uint16_t RESERVED13;
   volatile uint16_t DMACFGR;
   uint16_t RESERVED14;
   volatile uint16_t DMAADR;
   uint16_t RESERVED15;
} TIM_TypeDef;

#define TIM_BIT(value) ((uint16_t)(value))
#include "tim_bits.h"
