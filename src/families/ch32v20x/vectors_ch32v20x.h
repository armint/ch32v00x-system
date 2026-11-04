#pragma once

#include <stddef.h>
#include <stdint.h>

#define INTERRUPT_VECTOR_TABLE_SIZE 100

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
void __attribute__((interrupt)) ADC1_2_IRQHandler(void);
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
void __attribute__((interrupt)) USBFS_IRQHandler(void);
void __attribute__((interrupt)) USBFSWakeUp_IRQHandler(void);
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8)
void __attribute__((interrupt)) ETH_IRQHandler(void);
void __attribute__((interrupt)) ETHWakeUp_IRQHandler(void);
#if defined(CH32V20x_D8W)
void __attribute__((interrupt)) BB_IRQHandler(void);
void __attribute__((interrupt)) LLE_IRQHandler(void);
#endif
void __attribute__((interrupt)) TIM5_IRQHandler(void);
#endif
void __attribute__((interrupt)) UART4_IRQHandler(void);
void __attribute__((interrupt)) DMA1_Channel8_IRQHandler(void);
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8)
void __attribute__((interrupt)) OSC32KCal_IRQHandler(void);
void __attribute__((interrupt)) OSCWakeUp_IRQHandler(void);
#endif
#ifdef __cplusplus
}
#endif
