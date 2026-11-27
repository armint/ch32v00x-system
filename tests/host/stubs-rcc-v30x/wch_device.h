#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the Rcc driver (src/peripherals/Rcc.cpp and
// src/families/ch32v30x/Rcc_V30x.h, Bkp.h, Pwr.h and Crc_V30x.h). It has the RCC, PWR, BKP and
// CRC registers as host variables, the bits of src/families/ch32v30x/ch32v30x.h (the WCH header)
// that these drivers use, and SystemCoreClock.

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
#define RCC_TIM10EN (1ul << 20)
#define RCC_TIM3EN (1ul << 1)
#define RCC_SPI2EN (1ul << 14)
#define RCC_SPI3EN (1ul << 15)
#define RCC_USART2EN (1ul << 17)
#define RCC_USART3EN (1ul << 18)
#define RCC_PPRE1 (7ul << 8)
#define RCC_PPRE2 (7ul << 11)
#define RCC_TIM1RST (1ul << 11)
#define RCC_SPI1RST (1ul << 12)
#define RCC_TIM3RST (1ul << 1)
#define RCC_TIM10RST (1ul << 20)
#define RCC_SPI3RST (1ul << 15)
#define RCC_IOPDEN (1ul << 5)
#define RCC_UART5EN (1ul << 20)
#define RCC_UART7EN (1ul << 7)
#define RCC_TIM5EN (1ul << 3)
#define RCC_TIM5RST (1ul << 3)
#define RCC_CRCEN (1ul << 6)
#define RCC_OTGFSEN (1ul << 12)
#define RCC_OTGFSRST (1ul << 12)
#define RCC_DACEN (1ul << 29)
#define RCC_DACRST (1ul << 29)
#define RCC_PWREN (1ul << 28)
#define RCC_BKPEN (1ul << 27)
#define RCC_SWS (3ul << 2)
#define RCC_SWS_HSI (0ul << 2)
#define RCC_SWS_HSE (1ul << 2)
#define RCC_SWS_PLL (2ul << 2)
#define RCC_PPRE1_DIV2 (4ul << 8)
#define RCC_PPRE1_DIV4 (5ul << 8)
#define RCC_PPRE1_DIV8 (6ul << 8)
#define RCC_PPRE1_DIV16 (7ul << 8)
#define RCC_PPRE2_DIV2 (4ul << 11)

/* PWR, BKP (the first data registers) and CRC as in the WCH header */
typedef struct {
   volatile uint32_t CTLR;
   volatile uint32_t CSR;
} PWR_TypeDef;
extern PWR_TypeDef hostPwr;
#define PWR (&hostPwr)
#define PWR_CTLR_DBP ((uint16_t)0x0100)

typedef struct {
   uint32_t RESERVED0;
   volatile uint16_t DATAR1;
   uint16_t RESERVED1;
   volatile uint16_t DATAR2;
   uint16_t RESERVED2;
} BKP_TypeDef;
extern BKP_TypeDef hostBkp;
#define BKP (&hostBkp)

typedef struct {
   volatile uint32_t DATAR;
   volatile uint8_t IDATAR;
   uint8_t RESERVED0;
   uint16_t RESERVED1;
   volatile uint32_t CTLR;
} CRC_TypeDef;
extern CRC_TypeDef hostCrc;
#define CRC (&hostCrc)
#define CRC_CTLR_RESET ((uint8_t)0x01)
