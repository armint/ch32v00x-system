#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for Afio, Exti and
// GpioInterrupt (src/families/ch32v30x). The register blocks and bits are
// copies from src/families/ch32v30x/ch32v30x.h. The registers are plain host
// memory that test_exti_afio_v30x.cpp defines, so a register holds the last
// written value. The fake does not model write-1-to-clear flags.

#include <stddef.h>
#include <stdint.h>

typedef enum IRQn {
   EXTI0_IRQn = 22,
   EXTI1_IRQn = 23,
   EXTI2_IRQn = 24,
   EXTI3_IRQn = 25,
   EXTI4_IRQn = 26,
   EXTI9_5_IRQn = 39,
   EXTI15_10_IRQn = 56,
} IRQn_Type;

// NVIC: the test records the enable state of the IRQs it uses.
extern bool hostNvicEnabled[64];
inline void NVIC_EnableIRQ(IRQn_Type irq) {
   hostNvicEnabled[irq] = true;
}
inline void NVIC_DisableIRQ(IRQn_Type irq) {
   hostNvicEnabled[irq] = false;
}
// GpioInterrupt must not clear the pending bit of a shared vector: the test counts the calls.
extern int hostNvicClearPendingCalls;
inline void NVIC_ClearPendingIRQ(IRQn_Type) {
   ++hostNvicClearPendingCalls;
}

typedef struct {
   volatile uint32_t CFGLR;
   volatile uint32_t CFGHR;
   volatile uint32_t INDR;
   volatile uint32_t OUTDR;
   volatile uint32_t BSHR;
   volatile uint32_t BCR;
   volatile uint32_t LCKR;
} GPIO_TypeDef;

extern GPIO_TypeDef hostGpio[5];
#define GPIOA (&hostGpio[0])
#define GPIOB (&hostGpio[1])
#define GPIOC (&hostGpio[2])
#define GPIOD (&hostGpio[3])
#define GPIOE (&hostGpio[4])

typedef struct {
   volatile uint32_t APB2PCENR;
} RCC_TypeDef;

extern RCC_TypeDef hostRcc;
#define RCC (&hostRcc)
#define RCC_AFIOEN (1ul << 0)

typedef struct {
   volatile uint32_t ECR;
   volatile uint32_t PCFR1;
   volatile uint32_t EXTICR[4];
   uint32_t RESERVED0;
   volatile uint32_t PCFR2;
} AFIO_TypeDef;

extern AFIO_TypeDef hostAfio;
#define AFIO (&hostAfio)

#define AFIO_PCFR1_SPI1_RM (1ul << 0)
#define AFIO_PCFR1_I2C1_RM (1ul << 1)
#define AFIO_PCFR1_USART1_RM (1ul << 2)
#define AFIO_PCFR1_USART2_RM (1ul << 3)
#define AFIO_PCFR1_USART3_RM_MASK (3ul << 4)
#define AFIO_PCFR1_SW_CFG_MASK (7ul << 24)
#define AFIO_PCFR1_SW_CFG_DISABLE (4ul << 24)
#define AFIO_PCFR1_SPI3_RM (1ul << 28)

typedef struct {
   volatile uint32_t INTENR;
   volatile uint32_t EVENR;
   volatile uint32_t RTENR;
   volatile uint32_t FTENR;
   volatile uint32_t SWIEVR;
   volatile uint32_t INTFR;
} EXTI_TypeDef;

extern EXTI_TypeDef hostExti;
#define EXTI (&hostExti)
