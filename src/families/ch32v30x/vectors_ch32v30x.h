#pragma once

#include <stddef.h>
#include <stdint.h>

#define INTERRUPT_VECTOR_TABLE_SIZE 103

#ifdef __cplusplus
extern "C" {
#endif

void __attribute__((interrupt)) NMI_Handler(void);
void __attribute__((interrupt)) HardFault_Handler(void);
void __attribute__((interrupt)) SysTick_Handler(void);
void __attribute__((interrupt)) SW_Handler(void);
void __attribute__((interrupt)) WWDG_IRQHandler(void);
void __attribute__((interrupt)) PVD_IRQHandler(void);
void __attribute__((interrupt)) TAMPER_IRQHandler(void);
void __attribute__((interrupt)) RTC_IRQHandler(void);
void __attribute__((interrupt)) FLASH_IRQHandler(void);
void __attribute__((interrupt)) RCC_IRQHandler(void);
void __attribute__((interrupt)) EXTI0_IRQHandler(void);
void __attribute__((interrupt)) EXTI1_IRQHandler(void);
void __attribute__((interrupt)) EXTI2_IRQHandler(void);
void __attribute__((interrupt)) EXTI3_IRQHandler(void);
void __attribute__((interrupt)) EXTI4_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel1_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel2_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel3_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel4_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel5_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel6_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel7_IRQHandler(void);
void __attribute__((interrupt)) ADC_IRQHandler(void);
void __attribute__((interrupt)) USB_HP_CAN1_TX_IRQHandler(void);
void __attribute__((interrupt)) USB_LP_CAN1_RX0_IRQHandler(void);
void __attribute__((interrupt)) CAN1_RX1_IRQHandler(void);
void __attribute__((interrupt)) CAN1_SCE_IRQHandler(void);
void __attribute__((interrupt)) EXTI9_5_IRQHandler(void);
void __attribute__((interrupt)) TIM1_BRK_IRQHandler(void);
void __attribute__((interrupt)) TIM1_UP_IRQHandler(void);
void __attribute__((interrupt)) TIM1_TRG_COM_IRQHandler(void);
void __attribute__((interrupt)) TIM1_CC_IRQHandler(void);
void __attribute__((interrupt)) TIM2_IRQHandler(void);
void __attribute__((interrupt)) TIM3_IRQHandler(void);
void __attribute__((interrupt)) TIM4_IRQHandler(void);
void __attribute__((interrupt)) I2C1_EV_IRQHandler(void);
void __attribute__((interrupt)) I2C1_ER_IRQHandler(void);
void __attribute__((interrupt)) I2C2_EV_IRQHandler(void);
void __attribute__((interrupt)) I2C2_ER_IRQHandler(void);
void __attribute__((interrupt)) SPI1_IRQHandler(void);
void __attribute__((interrupt)) SPI2_IRQHandler(void);
void __attribute__((interrupt)) USART1_IRQHandler(void);
void __attribute__((interrupt)) USART2_IRQHandler(void);
void __attribute__((interrupt)) USART3_IRQHandler(void);
void __attribute__((interrupt)) EXTI15_10_IRQHandler(void);
void __attribute__((interrupt)) RTCAlarm_IRQHandler(void);
void __attribute__((interrupt)) USBWakeUp_IRQHandler(void);
void __attribute__((interrupt)) TIM8_BRK_IRQHandler(void);
void __attribute__((interrupt)) TIM8_UP_IRQHandler(void);
void __attribute__((interrupt)) TIM8_TRG_COM_IRQHandler(void);
void __attribute__((interrupt)) TIM8_CC_IRQHandler(void);
void __attribute__((interrupt)) RNG_IRQHandler(void);
void __attribute__((interrupt)) SDIO_IRQHandler(void);
void __attribute__((interrupt)) TIM5_IRQHandler(void);
void __attribute__((interrupt)) SPI3_IRQHandler(void);
void __attribute__((interrupt)) UART4_IRQHandler(void);
void __attribute__((interrupt)) UART5_IRQHandler(void);
void __attribute__((interrupt)) TIM6_IRQHandler(void);
void __attribute__((interrupt)) TIM7_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel1_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel2_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel3_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel4_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel5_IRQHandler(void);
void __attribute__((interrupt)) ETH_IRQHandler(void);
void __attribute__((interrupt)) ETH_WKUP_IRQHandler(void);
void __attribute__((interrupt)) CAN2_TX_IRQHandler(void);
void __attribute__((interrupt)) CAN2_RX0_IRQHandler(void);
void __attribute__((interrupt)) CAN2_RX1_IRQHandler(void);
void __attribute__((interrupt)) CAN2_SCE_IRQHandler(void);
void __attribute__((interrupt)) OTG_FS_IRQHandler(void);
void __attribute__((interrupt)) USBHSWakeup_IRQHandler(void);
void __attribute__((interrupt)) USBHS_IRQHandler(void);
void __attribute__((interrupt)) DVP_IRQHandler(void);
void __attribute__((interrupt)) UART6_IRQHandler(void);
void __attribute__((interrupt)) UART7_IRQHandler(void);
void __attribute__((interrupt)) UART8_IRQHandler(void);
void __attribute__((interrupt)) TIM9_BRK_IRQHandler(void);
void __attribute__((interrupt)) TIM9_UP_IRQHandler(void);
void __attribute__((interrupt)) TIM9_TRG_COM_IRQHandler(void);
void __attribute__((interrupt)) TIM9_CC_IRQHandler(void);
void __attribute__((interrupt)) TIM10_BRK_IRQHandler(void);
void __attribute__((interrupt)) TIM10_UP_IRQHandler(void);
void __attribute__((interrupt)) TIM10_TRG_COM_IRQHandler(void);
void __attribute__((interrupt)) TIM10_CC_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel6_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel7_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel8_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel9_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel10_IRQHandler(void);
void __attribute__((interrupt)) DMA2_Channel11_IRQHandler(void);

#ifdef __cplusplus
}
#endif
