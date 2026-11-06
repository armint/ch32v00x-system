#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the Rcc driver (src/peripherals/Rcc.cpp and
// src/families/ch32v30x/Rcc_V30x.h). It has the RCC registers as a host variable, the bits of
// src/families/ch32v30x/ch32v30x.h that Rcc_V30x.h uses, and SystemCoreClock.

#include <stdint.h>

extern uint32_t SystemCoreClock;

typedef struct {
   volatile uint32_t CTLR;
   volatile uint32_t CFGR0;
   volatile uint32_t INTR;
   volatile uint32_t APB2PRSTR;
   volatile uint32_t APB1PRSTR;
   volatile uint32_t AHBPCENR;
   volatile uint32_t APB2PCENR;
   volatile uint32_t APB1PCENR;
   volatile uint32_t BDCTLR;
   volatile uint32_t RSTSCKR;
   volatile uint32_t AHBRSTR;
   volatile uint32_t CFGR2;
} RCC_TypeDef;

extern RCC_TypeDef hostRcc;
#define RCC (&hostRcc)

#define RCC_DMA1EN (1ul << 0)
#define RCC_DMA2EN (1ul << 1)
#define RCC_AFIOEN (1ul << 0)
#define RCC_IOPAEN (1ul << 2)
#define RCC_IOPBEN (1ul << 3)
#define RCC_IOPCEN (1ul << 4)
#define RCC_TIM1EN (1ul << 11)
#define RCC_SPI1EN (1ul << 12)
#define RCC_USART1EN (1ul << 14)
#define RCC_TIM3EN (1ul << 1)
#define RCC_SPI2EN (1ul << 14)
#define RCC_USART2EN (1ul << 17)
#define RCC_USART3EN (1ul << 18)
#define RCC_CFGR0_PPRE1_MASK (7ul << 8)
#define RCC_CFGR0_PPRE2_MASK (7ul << 11)
#define RCC_TIM1RST (1ul << 11)
#define RCC_SPI1RST (1ul << 12)
#define RCC_TIM3RST (1ul << 1)
