/**
 * CH32V30x (QingKe V4F) device definitions for the CH32V307 (CH32V30x_D8C).
 *
 * Hand-written from CH32FV2x_V3xRM.PDF (reference manual) for the peripherals
 * of the start-up, clock, GPIO, USART and USBHS code. The IRQ numbers come from
 * svd/vendor/CH32V307xx.svd. Check the register offsets with
 * tests/check-svd-offsets.py after a change.
 */
#pragma once

#include <stdint.h>

#if !defined(CH32V30x_D8C)
#error "ch32v30x.h is for CH32V30x_D8C. Define CH32V30x_D8C."
#endif

#ifndef HSE_VALUE
#define HSE_VALUE ((uint32_t)8000000) /* External crystal of the board, in Hz */
#endif
#define HSI_VALUE ((uint32_t)8000000) /* Internal RC oscillator, in Hz */

#ifdef __cplusplus
extern "C" {
#endif

/* Interrupt numbers. The numbers match the vector table positions. */
typedef enum IRQn {
   /* Core interrupts, then the peripheral interrupts in IRQ number order */
   NonMaskableInt_IRQn = 2,
   EXC_IRQn = 3,
   Ecall_M_Mode_IRQn = 5,
   Ecall_U_Mode_IRQn = 8,
   Break_Point_IRQn = 9,
   SysTick_IRQn = 12,
   Software_IRQn = 14,
   WWDG_IRQn = 16,
   PVD_IRQn = 17,
   TAMPER_IRQn = 18,
   RTC_IRQn = 19,
   FLASH_IRQn = 20,
   RCC_IRQn = 21,
   EXTI0_IRQn = 22,
   EXTI1_IRQn = 23,
   EXTI2_IRQn = 24,
   EXTI3_IRQn = 25,
   EXTI4_IRQn = 26,
   DMA1_Channel1_IRQn = 27,
   DMA1_Channel2_IRQn = 28,
   DMA1_Channel3_IRQn = 29,
   DMA1_Channel4_IRQn = 30,
   DMA1_Channel5_IRQn = 31,
   DMA1_Channel6_IRQn = 32,
   DMA1_Channel7_IRQn = 33,
   ADC_IRQn = 34,
   USB_HP_CAN1_TX_IRQn = 35,
   USB_LP_CAN1_RX0_IRQn = 36,
   CAN1_RX1_IRQn = 37,
   CAN1_SCE_IRQn = 38,
   EXTI9_5_IRQn = 39,
   TIM1_BRK_IRQn = 40,
   TIM1_UP_IRQn = 41,
   TIM1_TRG_COM_IRQn = 42,
   TIM1_CC_IRQn = 43,
   TIM2_IRQn = 44,
   TIM3_IRQn = 45,
   TIM4_IRQn = 46,
   I2C1_EV_IRQn = 47,
   I2C1_ER_IRQn = 48,
   I2C2_EV_IRQn = 49,
   I2C2_ER_IRQn = 50,
   SPI1_IRQn = 51,
   SPI2_IRQn = 52,
   USART1_IRQn = 53,
   USART2_IRQn = 54,
   USART3_IRQn = 55,
   EXTI15_10_IRQn = 56,
   RTCAlarm_IRQn = 57,
   USBWakeUp_IRQn = 58,
   TIM8_BRK_IRQn = 59,
   TIM8_UP_IRQn = 60,
   TIM8_TRG_COM_IRQn = 61,
   TIM8_CC_IRQn = 62,
   RNG_IRQn = 63,
   SDIO_IRQn = 65,
   TIM5_IRQn = 66,
   SPI3_IRQn = 67,
   UART4_IRQn = 68,
   UART5_IRQn = 69,
   TIM6_IRQn = 70,
   TIM7_IRQn = 71,
   DMA2_Channel1_IRQn = 72,
   DMA2_Channel2_IRQn = 73,
   DMA2_Channel3_IRQn = 74,
   DMA2_Channel4_IRQn = 75,
   DMA2_Channel5_IRQn = 76,
   ETH_IRQn = 77,
   ETH_WKUP_IRQn = 78,
   CAN2_TX_IRQn = 79,
   CAN2_RX0_IRQn = 80,
   CAN2_RX1_IRQn = 81,
   CAN2_SCE_IRQn = 82,
   OTG_FS_IRQn = 83,
   USBHSWakeup_IRQn = 84,
   USBHS_IRQn = 85,
   DVP_IRQn = 86,
   UART6_IRQn = 87,
   UART7_IRQn = 88,
   UART8_IRQn = 89,
   TIM9_BRK_IRQn = 90,
   TIM9_UP_IRQn = 91,
   TIM9_TRG_COM_IRQn = 92,
   TIM9_CC_IRQn = 93,
   TIM10_BRK_IRQn = 94,
   TIM10_UP_IRQn = 95,
   TIM10_TRG_COM_IRQn = 96,
   TIM10_CC_IRQn = 97,
   DMA2_Channel6_IRQn = 98,
   DMA2_Channel7_IRQn = 99,
   DMA2_Channel8_IRQn = 100,
   DMA2_Channel9_IRQn = 101,
   DMA2_Channel10_IRQn = 102,
   DMA2_Channel11_IRQn = 103,
} IRQn_Type;

#include "core_riscv.h" /* needs IRQn_Type, so it follows the enum */

#define HardFault_IRQn EXC_IRQn
#define SysTicK_IRQn SysTick_IRQn

/* Base addresses */
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define GPIOB_BASE 0x40010C00UL
#define GPIOC_BASE 0x40011000UL
#define GPIOD_BASE 0x40011400UL
#define GPIOE_BASE 0x40011800UL
#define USART1_BASE 0x40013800UL
#define USART2_BASE 0x40004400UL
#define USART3_BASE 0x40004800UL
#define UART4_BASE 0x40004C00UL
#define UART5_BASE 0x40005000UL
#define UART6_BASE 0x40001800UL
#define UART7_BASE 0x40001C00UL
#define UART8_BASE 0x40002000UL
#define USBHS_BASE 0x40023400UL
#define ADC1_BASE 0x40012400UL
#define ADC2_BASE 0x40012800UL
#define TIM1_BASE 0x40012C00UL
#define TIM2_BASE 0x40000000UL
#define TIM3_BASE 0x40000400UL
#define TIM4_BASE 0x40000800UL
#define TIM5_BASE 0x40000C00UL
#define TIM6_BASE 0x40001000UL
#define TIM7_BASE 0x40001400UL
#define TIM8_BASE 0x40013400UL
#define TIM9_BASE 0x40014C00UL
#define TIM10_BASE 0x40015000UL
#define DMA1_BASE 0x40020000UL

typedef struct {
   volatile uint32_t CTLR;     /* 0x00 Clock control */
   volatile uint32_t CFGR0;    /* 0x04 Clock configuration 0 */
   volatile uint32_t INTR;     /* 0x08 Clock interrupt */
   volatile uint32_t APB2PRSTR; /* 0x0C APB2 peripheral reset */
   volatile uint32_t APB1PRSTR; /* 0x10 APB1 peripheral reset */
   volatile uint32_t AHBPCENR; /* 0x14 AHB peripheral clock enable */
   volatile uint32_t APB2PCENR; /* 0x18 APB2 peripheral clock enable */
   volatile uint32_t APB1PCENR; /* 0x1C APB1 peripheral clock enable */
   volatile uint32_t BDCTLR;   /* 0x20 Backup domain control */
   volatile uint32_t RSTSCKR;  /* 0x24 Control and status */
   volatile uint32_t AHBRSTR;  /* 0x28 AHB peripheral reset */
   volatile uint32_t CFGR2;    /* 0x2C Clock configuration 2 */
} RCC_TypeDef;

#define RCC ((RCC_TypeDef *)RCC_BASE)

/* RCC_CTLR bits */
#define RCC_HSION (1ul << 0)
#define RCC_HSIRDY (1ul << 1)
#define RCC_HSEON (1ul << 16)
#define RCC_HSERDY (1ul << 17)
#define RCC_PLLON (1ul << 24)
#define RCC_PLLRDY (1ul << 25)

/* AHB peripheral clock enable bits (RCC_AHBPCENR) */
#define RCC_USBHSEN (1ul << 11)
#define RCC_DMA1EN (1ul << 0)

/* APB2 peripheral clock enable bits (RCC_APB2PCENR) */
#define RCC_AFIOEN (1ul << 0)
#define RCC_IOPAEN (1ul << 2)
#define RCC_IOPBEN (1ul << 3)
#define RCC_IOPCEN (1ul << 4)
#define RCC_IOPDEN (1ul << 5)
#define RCC_IOPEEN (1ul << 6)
#define RCC_ADC1EN (1ul << 9)
#define RCC_ADC2EN (1ul << 10)
#define RCC_USART1EN (1ul << 14)
#define RCC_TIM1EN (1ul << 11)
#define RCC_TIM8EN (1ul << 13)
#define RCC_TIM9EN (1ul << 19)
#define RCC_TIM10EN (1ul << 20)

/* APB1 peripheral clock enable bits (RCC_APB1PCENR) */
#define RCC_TIM2EN (1ul << 0)
#define RCC_TIM3EN (1ul << 1)
#define RCC_TIM4EN (1ul << 2)
#define RCC_TIM5EN (1ul << 3)
#define RCC_TIM6EN (1ul << 4)
#define RCC_TIM7EN (1ul << 5)
#define RCC_UART6EN (1ul << 6)
#define RCC_UART7EN (1ul << 7)
#define RCC_UART8EN (1ul << 8)
#define RCC_USART2EN (1ul << 17)
#define RCC_USART3EN (1ul << 18)
#define RCC_UART4EN (1ul << 19)
#define RCC_UART5EN (1ul << 20)

/* RCC_CFGR0 fields */
#define RCC_CFGR0_SW_MASK (3ul << 0)
#define RCC_CFGR0_SW_PLL (2ul << 0)
#define RCC_CFGR0_SWS_MASK (3ul << 2)
#define RCC_CFGR0_SWS_PLL (2ul << 2)
#define RCC_CFGR0_HPRE_MASK (0xFul << 4)
#define RCC_CFGR0_PPRE1_MASK (7ul << 8)
#define RCC_CFGR0_PPRE2_MASK (7ul << 11)
#define RCC_ADCPRE (3ul << 14)
#define RCC_CFGR0_PLLSRC (1ul << 16) /* 1: PREDIV1 output feeds the PLL */
#define RCC_CFGR0_PLLMUL_MASK (0xFul << 18)
#define RCC_CFGR0_PLLMUL_X18 (0ul << 18) /* CH32V30x_D8C: 0000 is x18 */

/* APB2 peripheral reset bits (RCC_APB2PRSTR) */
#define RCC_ADC1RST (1ul << 9)
#define RCC_ADC2RST (1ul << 10)

/* RCC_CFGR2 fields */
#define RCC_CFGR2_PREDIV1_MASK (0xFul << 0)
#define RCC_CFGR2_PREDIV1SRC (1ul << 16) /* 0: HSE, 1: PLL2 */
#define RCC_CFGR2_USBHSDIV_MASK (7ul << 24) /* USBHS PHY PLL input: source / (USBHSDIV + 1) */
#define RCC_CFGR2_USBHSPLLSRC (1ul << 27)   /* 0: HSE, 1: HSI */
#define RCC_CFGR2_USBHSCLK_MASK (3ul << 28) /* Input frequency of the PHY PLL */
#define RCC_CFGR2_USBHSCLK_4MHZ (1ul << 28)
#define RCC_CFGR2_USBHSPLL (1ul << 30) /* PHY PLL on */
#define RCC_CFGR2_USBFSSRC (1ul << 31) /* RM: USBHSSRC. 48 MHz USB clock: 0: PLL, 1: USBHS PHY */

typedef struct {
   volatile uint32_t CFGLR; /* 0x00 Port configuration low */
   volatile uint32_t CFGHR; /* 0x04 Port configuration high */
   volatile uint32_t INDR;  /* 0x08 Input data */
   volatile uint32_t OUTDR; /* 0x0C Output data */
   volatile uint32_t BSHR;  /* 0x10 Set and reset */
   volatile uint32_t BCR;   /* 0x14 Reset */
   volatile uint32_t LCKR;  /* 0x18 Configuration lock */
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE ((GPIO_TypeDef *)GPIOE_BASE)

/* ADC1 and ADC2 have the same layout. Offsets from the SVD (ADC1 and ADC2 peripherals). */
typedef struct {
   volatile uint32_t STATR;    /* 0x00 Status */
   volatile uint32_t CTLR1;    /* 0x04 Control 1 */
   volatile uint32_t CTLR2;    /* 0x08 Control 2 */
   volatile uint32_t SAMPTR1;  /* 0x0C Sample time, channels 10 to 17 */
   volatile uint32_t SAMPTR2;  /* 0x10 Sample time, channels 0 to 9 */
   volatile uint32_t IOFR[4];  /* 0x14 Injected channel data offset */
   volatile uint32_t WDHTR;    /* 0x24 Watchdog high threshold */
   volatile uint32_t WDLTR;    /* 0x28 Watchdog low threshold */
   volatile uint32_t RSQR1;    /* 0x2C Regular sequence 1 (length, positions 13 to 16) */
   volatile uint32_t RSQR2;    /* 0x30 Regular sequence 2 (positions 7 to 12) */
   volatile uint32_t RSQR3;    /* 0x34 Regular sequence 3 (positions 1 to 6) */
   volatile uint32_t ISQR;     /* 0x38 Injected sequence */
   volatile uint32_t IDATAR[4]; /* 0x3C Injected data */
   volatile uint32_t RDATAR;   /* 0x4C Regular data */
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *)ADC1_BASE)
#define ADC2 ((ADC_TypeDef *)ADC2_BASE)

/* ADC_STATR bits */
#define ADC_EOC (1ul << 1)

/* ADC_CTLR1 bits */
#define ADC_EOCIE (1ul << 5)
#define ADC_AWDIE (1ul << 6)
#define ADC_SCAN (1ul << 8)

/* ADC_CTLR2 bits */
#define ADC_ADON (1ul << 0)
#define ADC_CONT (1ul << 1)
#define ADC_CAL (1ul << 2)
#define ADC_RSTCAL (1ul << 3)
#define ADC_DMA (1ul << 8)
#define ADC_EXTSEL (7ul << 17)
#define ADC_EXTTRIG (1ul << 20)
#define ADC_SWSTART (1ul << 22)
#define ADC_TSVREFE (1ul << 23)

/* ADC_RSQR1 */
#define ADC_L (0xFul << 20)

/* Same layout as the USART of CH32V20x. The SVD also has CTLR4 at 0x1C. */
typedef struct {
   volatile uint16_t STATR; /* 0x00 Status */
   uint16_t RESERVED0;
   volatile uint16_t DATAR; /* 0x04 Data */
   uint16_t RESERVED1;
   volatile uint16_t BRR; /* 0x08 Baud rate */
   uint16_t RESERVED2;
   volatile uint16_t CTLR1; /* 0x0C Control 1 */
   uint16_t RESERVED3;
   volatile uint16_t CTLR2; /* 0x10 Control 2 */
   uint16_t RESERVED4;
   volatile uint16_t CTLR3; /* 0x14 Control 3 */
   uint16_t RESERVED5;
   volatile uint16_t GPR; /* 0x18 Guard time and prescaler */
   uint16_t RESERVED6;
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)USART1_BASE)
#define USART2 ((USART_TypeDef *)USART2_BASE)
#define USART3 ((USART_TypeDef *)USART3_BASE)
#define UART4 ((USART_TypeDef *)UART4_BASE)
#define UART5 ((USART_TypeDef *)UART5_BASE)
#define UART6 ((USART_TypeDef *)UART6_BASE)
#define UART7 ((USART_TypeDef *)UART7_BASE)
#define UART8 ((USART_TypeDef *)UART8_BASE)

/* USART_STATR bits */
#define USART_STATR_RXNE ((uint16_t)0x0020)
#define USART_STATR_TC ((uint16_t)0x0040)
#define USART_STATR_TXE ((uint16_t)0x0080)

/* USART_CTLR1 bits */
#define USART_CTLR1_SBK ((uint16_t)0x0001)
#define USART_CTLR1_RE ((uint16_t)0x0004)
#define USART_CTLR1_TE ((uint16_t)0x0008)
#define USART_CTLR1_RXNEIE ((uint16_t)0x0020)
#define USART_CTLR1_TCIE ((uint16_t)0x0040)
#define USART_CTLR1_TXEIE ((uint16_t)0x0080)
#define USART_CTLR1_PS ((uint16_t)0x0200)
#define USART_CTLR1_PCE ((uint16_t)0x0400)
#define USART_CTLR1_UE ((uint16_t)0x2000)

/* USART_CTLR2 bits */
#define USART_CTLR2_STOP ((uint16_t)0x3000)
#define USART_CTLR2_STOP_0 ((uint16_t)0x1000)
#define USART_CTLR2_STOP_1 ((uint16_t)0x2000)

/* USART_CTLR3 bits */
#define USART_CTLR3_DMAR ((uint16_t)0x0040)
#define USART_CTLR3_DMAT ((uint16_t)0x0080)

/* Timers. All timers have this layout. The SVD gives each register 32 bits,
 * so CH1CVR to CH4CVR are consecutive 32-bit words (Timer indexes them).
 * TIM1, TIM8, TIM9 and TIM10 are advanced timers (RPTCR, BDTR). TIM6 and TIM7
 * are basic timers without channels. */
typedef struct {
   volatile uint32_t CTLR1;     /* 0x00 Control 1 */
   volatile uint32_t CTLR2;     /* 0x04 Control 2 */
   volatile uint32_t SMCFGR;    /* 0x08 Slave mode control */
   volatile uint32_t DMAINTENR; /* 0x0C DMA and interrupt enable */
   volatile uint32_t INTFR;     /* 0x10 Interrupt flags, write 0 to clear */
   volatile uint32_t SWEVGR;    /* 0x14 Software event generation */
   volatile uint32_t CHCTLR1;   /* 0x18 Compare/capture control, channels 1 and 2 */
   volatile uint32_t CHCTLR2;   /* 0x1C Compare/capture control, channels 3 and 4 */
   volatile uint32_t CCER;      /* 0x20 Compare/capture enable */
   volatile uint32_t CNT;       /* 0x24 Counter */
   volatile uint32_t PSC;       /* 0x28 Prescaler */
   volatile uint32_t ATRLR;     /* 0x2C Auto-reload */
   volatile uint32_t RPTCR;     /* 0x30 Repetition counter */
   volatile uint32_t CH1CVR;    /* 0x34 Compare/capture value, channel 1 */
   volatile uint32_t CH2CVR;    /* 0x38 Compare/capture value, channel 2 */
   volatile uint32_t CH3CVR;    /* 0x3C Compare/capture value, channel 3 */
   volatile uint32_t CH4CVR;    /* 0x40 Compare/capture value, channel 4 */
   volatile uint32_t BDTR;      /* 0x44 Break and dead time */
   volatile uint32_t DMACFGR;   /* 0x48 DMA control */
   volatile uint32_t DMAADR;    /* 0x4C DMA address for burst mode */
} TIM_TypeDef;

#define TIM1 ((TIM_TypeDef *)TIM1_BASE)
#define TIM2 ((TIM_TypeDef *)TIM2_BASE)
#define TIM3 ((TIM_TypeDef *)TIM3_BASE)
#define TIM4 ((TIM_TypeDef *)TIM4_BASE)
#define TIM5 ((TIM_TypeDef *)TIM5_BASE)
#define TIM6 ((TIM_TypeDef *)TIM6_BASE)
#define TIM7 ((TIM_TypeDef *)TIM7_BASE)
#define TIM8 ((TIM_TypeDef *)TIM8_BASE)
#define TIM9 ((TIM_TypeDef *)TIM9_BASE)
#define TIM10 ((TIM_TypeDef *)TIM10_BASE)

/* TIM_CTLR1 bits */
#define TIM_CEN (1ul << 0)
#define TIM_DIR (1ul << 4)
#define TIM_ARPE (1ul << 7)

/* TIM_CTLR2 */
#define TIM_MMS (7ul << 4)

/* TIM_DMAINTENR bits */
#define TIM_UIE (1ul << 0)
#define TIM_CC1IE (1ul << 1)
#define TIM_TIE (1ul << 6)

/* TIM_INTFR bits */
#define TIM_UIF (1ul << 0)
#define TIM_CC1IF (1ul << 1)

/* TIM_SWEVGR bits */
#define TIM_UG (1ul << 0)

/* TIM_CHCTLR1 output compare fields of channel 1. Channel 2 is 8 bits higher. */
#define TIM_OC1PE (1ul << 3)
#define TIM_OC1M (7ul << 4)
#define TIM_OC1M_0 (1ul << 4)
#define TIM_OC1M_1 (1ul << 5)
#define TIM_OC1M_2 (1ul << 6)

/* TIM_CCER bits of channel 1. Each next channel is 4 bits higher. */
#define TIM_CC1E (1ul << 0)
#define TIM_CC1P (1ul << 1)

/* TIM_BDTR bits (advanced timers only) */
#define TIM_AOE (1ul << 14)
#define TIM_MOE (1ul << 15)

/* DMA1 has 7 channels. DMA2 is not in this header: the Dma driver
 * addresses the channels of DMA1 only. */
typedef struct {
   volatile uint32_t CFGR;  /* 0x00 Configuration */
   volatile uint32_t CNTR;  /* 0x04 Number of data to transfer */
   volatile uint32_t PADDR; /* 0x08 Peripheral address */
   volatile uint32_t MADDR; /* 0x0C Memory address */
} DMA_Channel_TypeDef;

typedef struct {
   volatile uint32_t INTFR;  /* 0x00 Interrupt flags */
   volatile uint32_t INTFCR; /* 0x04 Interrupt flag clear, write 1 to clear */
} DMA_TypeDef;

#define DMA1_Channel1_BASE (DMA1_BASE + 0x08UL)
#define DMA1_Channel2_BASE (DMA1_BASE + 0x1CUL)
#define DMA1_Channel3_BASE (DMA1_BASE + 0x30UL)
#define DMA1_Channel4_BASE (DMA1_BASE + 0x44UL)
#define DMA1_Channel5_BASE (DMA1_BASE + 0x58UL)
#define DMA1_Channel6_BASE (DMA1_BASE + 0x6CUL)
#define DMA1_Channel7_BASE (DMA1_BASE + 0x80UL)

#define DMA1 ((DMA_TypeDef *)DMA1_BASE)
#define DMA1_Channel1 ((DMA_Channel_TypeDef *)DMA1_Channel1_BASE)
#define DMA1_Channel2 ((DMA_Channel_TypeDef *)DMA1_Channel2_BASE)
#define DMA1_Channel3 ((DMA_Channel_TypeDef *)DMA1_Channel3_BASE)
#define DMA1_Channel4 ((DMA_Channel_TypeDef *)DMA1_Channel4_BASE)
#define DMA1_Channel5 ((DMA_Channel_TypeDef *)DMA1_Channel5_BASE)
#define DMA1_Channel6 ((DMA_Channel_TypeDef *)DMA1_Channel6_BASE)
#define DMA1_Channel7 ((DMA_Channel_TypeDef *)DMA1_Channel7_BASE)

/* DMA_INTFR and DMA_INTFCR bits of channel 1. Each next channel is 4 bits higher. */
#define DMA_GIF1 (1ul << 0)
#define DMA_TCIF1 (1ul << 1)
#define DMA_CHTIF1 (1ul << 2)
#define DMA_CTEIF1 (1ul << 3)

/* DMA_CFGRx bits. The names follow the other families (CFGR1 for all channels). */
#define DMA_CFGR1_EN (1ul << 0)
#define DMA_CFGR1_TCIE (1ul << 1)
#define DMA_CFGR1_HTIE (1ul << 2)
#define DMA_CFGR1_TEIE (1ul << 3)
#define DMA_CFGR1_DIR (1ul << 4)
#define DMA_CFGR1_CIRC (1ul << 5)
#define DMA_CFGR1_PINC (1ul << 6)
#define DMA_CFGR1_MINC (1ul << 7)
#define DMA_CFGR1_PSIZE (3ul << 8)
#define DMA_CFGR1_MSIZE (3ul << 10)
#define DMA_CFGR1_PL (3ul << 12)
#define DMA_CFGR1_PL_0 (1ul << 12)
#define DMA_CFGR1_PL_1 (1ul << 13)
#define DMA_CFGR1_MEM2MEM (1ul << 14)

/* USBHS controller (USB 2.0 high-speed, RM chapter 22). Device-mode names.
 * The host-mode registers share these addresses. */
typedef struct {
   volatile uint16_t MAX_LEN; /* Largest OUT packet that the endpoint accepts */
   uint16_t RESERVED;
} USBHS_MaxLen_TypeDef;

typedef struct {
   volatile uint16_t T_LEN;  /* IN packet length. EP0: bits 6:0 only */
   volatile uint8_t TX_CTRL; /* IN response and data toggle */
   volatile uint8_t RX_CTRL; /* OUT response and data toggle */
} USBHS_EpCtrl_TypeDef;

typedef struct {
   volatile uint8_t CTRL;      /* 0x00 Control */
   volatile uint8_t HOST_CTRL; /* 0x01 Host control. PHY_SUSPENDM applies in device mode too */
   volatile uint8_t INT_EN;    /* 0x02 Interrupt enable */
   volatile uint8_t DEV_AD;    /* 0x03 Device address */
   volatile uint16_t FRAME_NO; /* 0x04 Frame number */
   volatile uint8_t SUSPEND;   /* 0x06 Suspend */
   uint8_t RESERVED0;
   volatile uint8_t SPEED_TYPE; /* 0x08 Current speed */
   volatile uint8_t MIS_ST;     /* 0x09 Miscellaneous status */
   volatile uint8_t INT_FG;     /* 0x0A Interrupt flags, write 1 to clear */
   volatile uint8_t INT_ST;     /* 0x0B Interrupt status */
   volatile uint16_t RX_LEN;    /* 0x0C Length of the last received packet */
   uint16_t RESERVED1;
   volatile uint32_t UEP_CONFIG;  /* 0x10 Endpoint 1-15 IN/OUT enable */
   volatile uint32_t UEP_TYPE;    /* 0x14 Endpoint 1-15 isochronous type */
   volatile uint32_t UEP_BUF_MOD; /* 0x18 Endpoint buffer mode */
   volatile uint32_t UEP0_DMA;    /* 0x1C EP0 buffer, IN and OUT shared, 4-byte aligned */
   volatile uint32_t UEP_RX_DMA[15]; /* 0x20 OUT buffer of endpoints 1-15 ([0] is EP1) */
   volatile uint32_t UEP_TX_DMA[15]; /* 0x5C IN buffer of endpoints 1-15 ([0] is EP1) */
   USBHS_MaxLen_TypeDef UEP_MAX_LEN[16]; /* 0x98 Endpoints 0-15 */
   USBHS_EpCtrl_TypeDef UEP[16];         /* 0xD8 Endpoints 0-15 */
} USBHS_TypeDef;

#define USBHS ((USBHS_TypeDef *)USBHS_BASE)

/* USBHS_CTRL bits */
#define USBHS_UC_DMA_EN ((uint8_t)0x01)
#define USBHS_UC_CLR_ALL ((uint8_t)0x02)
#define USBHS_UC_RST_SIE ((uint8_t)0x04)
#define USBHS_UC_INT_BUSY ((uint8_t)0x08) /* NAK all tokens while INT_FG.TRANSFER is set */
#define USBHS_UC_DEV_PU_EN ((uint8_t)0x10)
#define USBHS_UC_SPEED_MASK ((uint8_t)0x60)
#define USBHS_UC_SPEED_FULL ((uint8_t)0x00)
#define USBHS_UC_SPEED_HIGH ((uint8_t)0x20)
#define USBHS_UC_HOST_MODE ((uint8_t)0x80)

/* USBHS_HOST_CTRL bits */
#define USBHS_UH_PHY_SUSPENDM ((uint8_t)0x10) /* 1: PHY runs */

/* USBHS_INT_EN bits. Bits 5 to 7 follow the RM; the vendor SVD names them differently. */
#define USBHS_UIE_BUS_RST ((uint8_t)0x01)
#define USBHS_UIE_TRANSFER ((uint8_t)0x02)
#define USBHS_UIE_SUSPEND ((uint8_t)0x04)
#define USBHS_UIE_SOF_ACT ((uint8_t)0x08)
#define USBHS_UIE_FIFO_OV ((uint8_t)0x10)
#define USBHS_UIE_SETUP_ACT ((uint8_t)0x20)
#define USBHS_UIE_ISO_ACT ((uint8_t)0x40)
#define USBHS_UIE_DEV_NAK ((uint8_t)0x80)

/* USBHS_DEV_AD */
#define USBHS_DEV_AD_MASK ((uint8_t)0x7F)

/* USBHS_SPEED_TYPE */
#define USBHS_SPEED_TYPE_MASK ((uint8_t)0x03)
#define USBHS_SPEED_TYPE_FULL ((uint8_t)0x00)
#define USBHS_SPEED_TYPE_HIGH ((uint8_t)0x01)
#define USBHS_SPEED_TYPE_LOW ((uint8_t)0x02)

/* USBHS_MIS_ST bits */
#define USBHS_UMS_SUSPEND ((uint8_t)0x04)
#define USBHS_UMS_BUS_RESET ((uint8_t)0x08)
#define USBHS_UMS_SIE_FREE ((uint8_t)0x20)

/* USBHS_INT_FG bits */
#define USBHS_UIF_BUS_RST ((uint8_t)0x01)
#define USBHS_UIF_TRANSFER ((uint8_t)0x02) /* Any transfer except SETUP */
#define USBHS_UIF_SUSPEND ((uint8_t)0x04) /* Suspend or resume, see MIS_ST.SUSPEND */
#define USBHS_UIF_HST_SOF ((uint8_t)0x08)
#define USBHS_UIF_FIFO_OV ((uint8_t)0x10)
#define USBHS_UIF_SETUP_ACT ((uint8_t)0x20)
#define USBHS_UIF_ISO_ACT ((uint8_t)0x40)

/* USBHS_INT_ST fields */
#define USBHS_UIS_ENDP_MASK ((uint8_t)0x0F)
#define USBHS_UIS_TOKEN_MASK ((uint8_t)0x30)
#define USBHS_UIS_TOKEN_OUT ((uint8_t)0x00)
#define USBHS_UIS_TOKEN_SOF ((uint8_t)0x10)
#define USBHS_UIS_TOKEN_IN ((uint8_t)0x20)
#define USBHS_UIS_TOKEN_SETUP ((uint8_t)0x30)
#define USBHS_UIS_TOG_OK ((uint8_t)0x40)
#define USBHS_UIS_IS_NAK ((uint8_t)0x80)

/* USBHS_UEP_CONFIG bits, n = 1..15. EP0 is always on. */
#define USBHS_UEP_T_EN(n) (1ul << (n))
#define USBHS_UEP_R_EN(n) (1ul << ((n) + 16))

/* USBHS_UEPn_TX_CTRL fields */
#define USBHS_UEP_T_RES_MASK ((uint8_t)0x03)
#define USBHS_UEP_T_RES_ACK ((uint8_t)0x00)
#define USBHS_UEP_T_RES_NAK ((uint8_t)0x02)
#define USBHS_UEP_T_RES_STALL ((uint8_t)0x03)
#define USBHS_UEP_T_TOG_MASK ((uint8_t)0x18)
#define USBHS_UEP_T_TOG_DATA1 ((uint8_t)0x08)
#define USBHS_UEP_T_TOG_AUTO ((uint8_t)0x20) /* Reserved on EP0 */

/* USBHS_UEPn_RX_CTRL fields */
#define USBHS_UEP_R_RES_MASK ((uint8_t)0x03)
#define USBHS_UEP_R_RES_ACK ((uint8_t)0x00)
#define USBHS_UEP_R_RES_NYET ((uint8_t)0x01)
#define USBHS_UEP_R_RES_NAK ((uint8_t)0x02)
#define USBHS_UEP_R_RES_STALL ((uint8_t)0x03)
#define USBHS_UEP_R_TOG_MASK ((uint8_t)0x18)
#define USBHS_UEP_R_TOG_DATA1 ((uint8_t)0x08)
#define USBHS_UEP_R_TOG_AUTO ((uint8_t)0x20) /* Reserved on EP0 */

#ifdef __cplusplus
}
#endif
