/**
 * CH32V30x (QingKe V4F) device definitions for the CH32V307 (CH32V30x_D8C).
 *
 * Hand-written from CH32FV2x_V3xRM.PDF (reference manual) for the peripherals
 * of the start-up, clock and GPIO code. The IRQ numbers come from
 * etc/svd/vendor/CH32V307xx.svd. Check the register offsets with
 * tests/check-svd-offsets.py after a change.
 */
#pragma once

#include <stdint.h>

#if !defined(CH32V30x_D8C)
#error "ch32v30x.h is for CH32V30x_D8C. Define CH32V30x_D8C."
#endif

#define HSE_VALUE ((uint32_t)8000000) /* External crystal of the board, in Hz */
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

/* APB2 peripheral clock enable bits (RCC_APB2PCENR) */
#define RCC_AFIOEN (1ul << 0)
#define RCC_IOPAEN (1ul << 2)
#define RCC_IOPBEN (1ul << 3)
#define RCC_IOPCEN (1ul << 4)
#define RCC_IOPDEN (1ul << 5)
#define RCC_IOPEEN (1ul << 6)

/* RCC_CFGR0 fields */
#define RCC_CFGR0_SW_MASK (3ul << 0)
#define RCC_CFGR0_SW_PLL (2ul << 0)
#define RCC_CFGR0_SWS_MASK (3ul << 2)
#define RCC_CFGR0_SWS_PLL (2ul << 2)
#define RCC_CFGR0_HPRE_MASK (0xFul << 4)
#define RCC_CFGR0_PPRE1_MASK (7ul << 8)
#define RCC_CFGR0_PPRE2_MASK (7ul << 11)
#define RCC_CFGR0_PLLSRC (1ul << 16) /* 1: PREDIV1 output feeds the PLL */
#define RCC_CFGR0_PLLMUL_MASK (0xFul << 18)
#define RCC_CFGR0_PLLMUL_X18 (0ul << 18) /* CH32V30x_D8C: 0000 is x18 */

/* RCC_CFGR2 fields */
#define RCC_CFGR2_PREDIV1_MASK (0xFul << 0)
#define RCC_CFGR2_PREDIV1SRC (1ul << 16) /* 0: HSE, 1: PLL2 */

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

#ifdef __cplusplus
}
#endif
