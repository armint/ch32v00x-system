// ----------------------------------------------------------------------------
#include "vectors_ch32v00X.h"
#if defined(CH32V006) || defined(CH32V002) || defined(CH32V003)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

void __attribute__((weak)) Default_Handler(void);

// Forward declaration of the specific IRQ handlers. These are aliased
// to the Default_Handler, which is a 'forever' loop. When the application
// defines a handler (with the same name), this will automatically take
// precedence over these weak definitions
void __attribute__((weak, alias("Default_Handler"))) NMI_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) HardFault_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) SysTick_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) SW_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) WWDG_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) PVD_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) FLASH_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) RCC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI7_0_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) AWU_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel5_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel6_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel7_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) ADC1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) I2C1_EV_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) I2C1_ER_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USART1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) SPI1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_BRK_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_UP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_TRG_COM_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_CC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM2_IRQHandler(void);
// Same condition as the vector table below: every chip except CH32V003.
#if !defined(CH32V003)
void __attribute__((weak, alias("Default_Handler"))) USART2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) OPCM_IRQHandler(void);
#endif
// ----------------------------------------------------------------------------

extern unsigned int _estack;

typedef void (*const pHandler)(void);

// ----------------------------------------------------------------------------

__attribute__((section(".init"), used, naked)) void Reset_Handler() {
   asm volatile(".option   norvc;\n"
                "j       _start\n"
                ".option rvc;\n");
}

// The vector table.
// This relies on the linker script to place at correct location in memory.

__attribute__((section(".isr_vector"), used)) pHandler g_pfnVectors[INTERRUPT_VECTOR_TABLE_SIZE] = {
   0,
   NMI_Handler,       /* NMI Handler */
   HardFault_Handler, /* Hard Fault Handler */
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   SysTick_Handler, /* SysTick Handler */
   0,
   SW_Handler, /* SW Handler */
   0,
   /* External Interrupts */
   WWDG_IRQHandler,          /* Window Watchdog */
   PVD_IRQHandler,           /* PVD through EXTI Line detect */
   FLASH_IRQHandler,         /* Flash */
   RCC_IRQHandler,           /* RCC */
   EXTI7_0_IRQHandler,       /* EXTI Line 7..0 */
   AWU_IRQHandler,           /* AWU */
   DMA1_Channel1_IRQHandler, /* DMA1 Channel 1 */
   DMA1_Channel2_IRQHandler, /* DMA1 Channel 2 */
   DMA1_Channel3_IRQHandler, /* DMA1 Channel 3 */
   DMA1_Channel4_IRQHandler, /* DMA1 Channel 4 */
   DMA1_Channel5_IRQHandler, /* DMA1 Channel 5 */
   DMA1_Channel6_IRQHandler, /* DMA1 Channel 6 */
   DMA1_Channel7_IRQHandler, /* DMA1 Channel 7 */
   ADC1_IRQHandler,          /* ADC1 */
   I2C1_EV_IRQHandler,       /* I2C1 Event */
   I2C1_ER_IRQHandler,       /* I2C1 Error */
   USART1_IRQHandler,        /* USART1 */
   SPI1_IRQHandler,          /* SPI1 */
   TIM1_BRK_IRQHandler,      /* TIM1 Break */
   TIM1_UP_IRQHandler,       /* TIM1 Update */
   TIM1_TRG_COM_IRQHandler,  /* TIM1 Trigger and Commutation */
   TIM1_CC_IRQHandler,       /* TIM1 Capture Compare */
   TIM2_IRQHandler,          /* TIM2 */
#if !defined(CH32V003)
   USART2_IRQHandler, /* USART2 */
   OPCM_IRQHandler,   /* Opanp and comparator */
#endif
};

// ----------------------------------------------------------------------------

// Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.

void __attribute__((section(".after_vectors"))) Default_Handler(void) {
   while(1) {
   }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
#endif
