#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the ADC driver
// (src/families/ch32v30x/Adc_V30x.h). The ADC and RCC register blocks and the
// bits are copies from src/families/ch32v30x/ch32v30x.h. The registers are plain
// host memory that test_adc_v30x.cpp defines, so a register holds the last
// written value. The fake models no conversion and no calibration.

#include <stddef.h>
#include <stdint.h>

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

#define RCC_ADC1EN (1ul << 9)
#define RCC_ADC2EN (1ul << 10)
#define RCC_ADCPRE (3ul << 14)
#define RCC_ADC1RST (1ul << 9)
#define RCC_ADC2RST (1ul << 10)

typedef struct {
   volatile uint32_t STATR;
   volatile uint32_t CTLR1;
   volatile uint32_t CTLR2;
   volatile uint32_t SAMPTR1;
   volatile uint32_t SAMPTR2;
   volatile uint32_t IOFR[4];
   volatile uint32_t WDHTR;
   volatile uint32_t WDLTR;
   volatile uint32_t RSQR1;
   volatile uint32_t RSQR2;
   volatile uint32_t RSQR3;
   volatile uint32_t ISQR;
   volatile uint32_t IDATAR[4];
   volatile uint32_t RDATAR;
} ADC_TypeDef;

static_assert(offsetof(ADC_TypeDef, SAMPTR1) == 0x0C, "ADC layout");
static_assert(offsetof(ADC_TypeDef, RSQR1) == 0x2C, "ADC layout");
static_assert(offsetof(ADC_TypeDef, RSQR3) == 0x34, "ADC layout");
static_assert(offsetof(ADC_TypeDef, RDATAR) == 0x4C, "ADC layout");

extern ADC_TypeDef hostAdc1;
extern ADC_TypeDef hostAdc2;
#define ADC1 (&hostAdc1)
#define ADC2 (&hostAdc2)

#define ADC_EOC (1ul << 1)
#define ADC_EOCIE (1ul << 5)
#define ADC_AWDIE (1ul << 6)
#define ADC_SCAN (1ul << 8)
#define ADC_ADON (1ul << 0)
#define ADC_CONT (1ul << 1)
#define ADC_CAL (1ul << 2)
#define ADC_RSTCAL (1ul << 3)
#define ADC_DMA (1ul << 8)
#define ADC_EXTSEL (7ul << 17)
#define ADC_EXTTRIG (1ul << 20)
#define ADC_SWSTART (1ul << 22)
#define ADC_TSVREFE (1ul << 23)
#define ADC_L (0xFul << 20)
