#pragma once

#include <stdint.h>
#include <stddef.h>

#define INTERRUPT_VECTOR_TABLE_SIZE 40

#ifdef __cplusplus
extern "C" {
#endif


void __attribute__((interrupt)) NMI_Handler(void);
void __attribute__((interrupt)) HardFault_Handler(void);
void __attribute__((interrupt)) SysTick_Handler(void);
void __attribute__((interrupt)) SW_Handler(void);
void __attribute__((interrupt)) WWDG_IRQHandler(void);
void __attribute__((interrupt)) PVD_IRQHandler(void);
void __attribute__((interrupt)) FLASH_IRQHandler(void);
void __attribute__((interrupt)) RCC_IRQHandler(void);
void __attribute__((interrupt)) EXTI7_0_IRQHandler(void);
void __attribute__((interrupt)) AWU_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel1_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel2_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel3_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel4_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel5_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel6_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel7_IRQHandler(void);
void __attribute__((interrupt)) ADC1_IRQHandler(void);
void __attribute__((interrupt)) I2C1_EV_IRQHandler(void);
void __attribute__((interrupt)) I2C1_ER_IRQHandler(void);
void __attribute__((interrupt)) USART1_IRQHandler(void);
void __attribute__((interrupt)) SPI1_IRQHandler(void);
void __attribute__((interrupt)) TIM1_BRK_IRQHandler(void);
void __attribute__((interrupt)) TIM1_UP_IRQHandler(void);
void __attribute__((interrupt)) TIM1_TRG_COM_IRQHandler(void);
void __attribute__((interrupt)) TIM1_CC_IRQHandler(void);
void __attribute__((interrupt)) TIM2_IRQHandler(void);
void __attribute__((interrupt)) USART2_IRQHandler(void);
void __attribute__((interrupt)) OPCM_IRQHandler(void);

#ifdef __cplusplus
}
#endif
