#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the baud rate code of the
// Usart driver (src/peripherals/Usart.cpp). It has the bits of
// src/families/ch32v30x/ch32v30x.h that Usart.h uses, the RCC and USART
// registers as host variables, and SystemCoreClock.

#include <stdint.h>

// RCC registers, RCC bits and SystemCoreClock: the same stub as the Rcc test (Usart.cpp uses Rcc).
#include "../stubs-rcc-v30x/wch_device.h"

typedef struct {
   volatile uint16_t STATR;
   uint16_t RESERVED0;
   volatile uint16_t DATAR;
   uint16_t RESERVED1;
   volatile uint16_t BRR;
   uint16_t RESERVED2;
   volatile uint16_t CTLR1;
   uint16_t RESERVED3;
   volatile uint16_t CTLR2;
   uint16_t RESERVED4;
   volatile uint16_t CTLR3;
   uint16_t RESERVED5;
   volatile uint16_t GPR;
   uint16_t RESERVED6;
} USART_TypeDef;

extern USART_TypeDef hostUsart1;
extern USART_TypeDef hostUsart2;
#define USART1 (&hostUsart1)
#define USART2 (&hostUsart2)

#define USART_STATR_RXNE ((uint16_t)0x0020)
#define USART_STATR_TC ((uint16_t)0x0040)
#define USART_STATR_TXE ((uint16_t)0x0080)
#define USART_CTLR1_SBK ((uint16_t)0x0001)
#define USART_CTLR1_RE ((uint16_t)0x0004)
#define USART_CTLR1_TE ((uint16_t)0x0008)
#define USART_CTLR1_RXNEIE ((uint16_t)0x0020)
#define USART_CTLR1_TCIE ((uint16_t)0x0040)
#define USART_CTLR1_TXEIE ((uint16_t)0x0080)
#define USART_CTLR1_M ((uint16_t)0x1000)
#define USART_CTLR1_PS ((uint16_t)0x0200)
#define USART_CTLR1_PCE ((uint16_t)0x0400)
#define USART_CTLR1_UE ((uint16_t)0x2000)
#define USART_CTLR2_STOP ((uint16_t)0x3000)
#define USART_CTLR2_STOP_0 ((uint16_t)0x1000)
#define USART_CTLR2_STOP_1 ((uint16_t)0x2000)
#define USART_CTLR3_DMAR ((uint16_t)0x0040)
#define USART_CTLR3_DMAT ((uint16_t)0x0080)
