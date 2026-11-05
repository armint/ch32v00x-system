/*
 * Copyright (c) 2015 Liviu Ionescu.
 *
 * was taken from the µOS++ distribution.
 * (https://github.com/micro-os-plus)
 * Modified for use with the CH32 families without µOS++ by Armin van der Togt
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------
#include "vectors_ch32v30x.h"
#if defined(CH32V30x_D8C)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

void __attribute__((weak)) Default_Handler(void);

// Each handler is an alias of Default_Handler until the application defines it.
// Entry n of the table holds IRQ n + 1: the reset jump is entry 0, at the MTVEC base.
// See Reset_Handler below.
void __attribute__((weak, alias("Default_Handler"))) NMI_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) HardFault_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) SysTick_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) SW_Handler(void);
void __attribute__((weak, alias("Default_Handler"))) WWDG_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) PVD_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TAMPER_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) RTC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) FLASH_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) RCC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI0_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel5_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel6_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel7_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) ADC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USB_HP_CAN1_TX_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USB_LP_CAN1_RX0_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) CAN1_RX1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) CAN1_SCE_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI9_5_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_BRK_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_UP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_TRG_COM_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM1_CC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) I2C1_EV_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) I2C1_ER_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) I2C2_EV_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) I2C2_ER_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) SPI1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) SPI2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USART1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USART2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USART3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) EXTI15_10_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) RTCAlarm_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USBWakeUp_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM8_BRK_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM8_UP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM8_TRG_COM_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM8_CC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) RNG_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) SDIO_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM5_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) SPI3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) UART4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) UART5_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM6_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM7_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel2_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel3_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel5_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) ETH_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) ETH_WKUP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) CAN2_TX_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) CAN2_RX0_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) CAN2_RX1_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) CAN2_SCE_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) OTG_FS_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USBHSWakeup_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USBHS_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DVP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) UART6_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) UART7_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) UART8_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM9_BRK_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM9_UP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM9_TRG_COM_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM9_CC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM10_BRK_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM10_UP_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM10_TRG_COM_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) TIM10_CC_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel6_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel7_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel8_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel9_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel10_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA2_Channel11_IRQHandler(void);

__attribute__((section(".init"), used, naked)) void Reset_Handler() {
   asm volatile(".option   norvc;\n"
                "j       _start\n"
                ".option rvc;\n");
}

// The vector table. The linker script places it at .isr_vector.
typedef void (*const pHandler)(void);

__attribute__((section(".isr_vector"), used)) pHandler g_pfnVectors[INTERRUPT_VECTOR_TABLE_SIZE] = {
   [1] = NMI_Handler,
   [2] = HardFault_Handler,
   [11] = SysTick_Handler,
   [13] = SW_Handler,
   [15] = WWDG_IRQHandler,
   [16] = PVD_IRQHandler,
   [17] = TAMPER_IRQHandler,
   [18] = RTC_IRQHandler,
   [19] = FLASH_IRQHandler,
   [20] = RCC_IRQHandler,
   [21] = EXTI0_IRQHandler,
   [22] = EXTI1_IRQHandler,
   [23] = EXTI2_IRQHandler,
   [24] = EXTI3_IRQHandler,
   [25] = EXTI4_IRQHandler,
   [26] = DMA1_Channel1_IRQHandler,
   [27] = DMA1_Channel2_IRQHandler,
   [28] = DMA1_Channel3_IRQHandler,
   [29] = DMA1_Channel4_IRQHandler,
   [30] = DMA1_Channel5_IRQHandler,
   [31] = DMA1_Channel6_IRQHandler,
   [32] = DMA1_Channel7_IRQHandler,
   [33] = ADC_IRQHandler,
   [34] = USB_HP_CAN1_TX_IRQHandler,
   [35] = USB_LP_CAN1_RX0_IRQHandler,
   [36] = CAN1_RX1_IRQHandler,
   [37] = CAN1_SCE_IRQHandler,
   [38] = EXTI9_5_IRQHandler,
   [39] = TIM1_BRK_IRQHandler,
   [40] = TIM1_UP_IRQHandler,
   [41] = TIM1_TRG_COM_IRQHandler,
   [42] = TIM1_CC_IRQHandler,
   [43] = TIM2_IRQHandler,
   [44] = TIM3_IRQHandler,
   [45] = TIM4_IRQHandler,
   [46] = I2C1_EV_IRQHandler,
   [47] = I2C1_ER_IRQHandler,
   [48] = I2C2_EV_IRQHandler,
   [49] = I2C2_ER_IRQHandler,
   [50] = SPI1_IRQHandler,
   [51] = SPI2_IRQHandler,
   [52] = USART1_IRQHandler,
   [53] = USART2_IRQHandler,
   [54] = USART3_IRQHandler,
   [55] = EXTI15_10_IRQHandler,
   [56] = RTCAlarm_IRQHandler,
   [57] = USBWakeUp_IRQHandler,
   [58] = TIM8_BRK_IRQHandler,
   [59] = TIM8_UP_IRQHandler,
   [60] = TIM8_TRG_COM_IRQHandler,
   [61] = TIM8_CC_IRQHandler,
   [62] = RNG_IRQHandler,
   [64] = SDIO_IRQHandler,
   [65] = TIM5_IRQHandler,
   [66] = SPI3_IRQHandler,
   [67] = UART4_IRQHandler,
   [68] = UART5_IRQHandler,
   [69] = TIM6_IRQHandler,
   [70] = TIM7_IRQHandler,
   [71] = DMA2_Channel1_IRQHandler,
   [72] = DMA2_Channel2_IRQHandler,
   [73] = DMA2_Channel3_IRQHandler,
   [74] = DMA2_Channel4_IRQHandler,
   [75] = DMA2_Channel5_IRQHandler,
   [76] = ETH_IRQHandler,
   [77] = ETH_WKUP_IRQHandler,
   [78] = CAN2_TX_IRQHandler,
   [79] = CAN2_RX0_IRQHandler,
   [80] = CAN2_RX1_IRQHandler,
   [81] = CAN2_SCE_IRQHandler,
   [82] = OTG_FS_IRQHandler,
   [83] = USBHSWakeup_IRQHandler,
   [84] = USBHS_IRQHandler,
   [85] = DVP_IRQHandler,
   [86] = UART6_IRQHandler,
   [87] = UART7_IRQHandler,
   [88] = UART8_IRQHandler,
   [89] = TIM9_BRK_IRQHandler,
   [90] = TIM9_UP_IRQHandler,
   [91] = TIM9_TRG_COM_IRQHandler,
   [92] = TIM9_CC_IRQHandler,
   [93] = TIM10_BRK_IRQHandler,
   [94] = TIM10_UP_IRQHandler,
   [95] = TIM10_TRG_COM_IRQHandler,
   [96] = TIM10_CC_IRQHandler,
   [97] = DMA2_Channel6_IRQHandler,
   [98] = DMA2_Channel7_IRQHandler,
   [99] = DMA2_Channel8_IRQHandler,
   [100] = DMA2_Channel9_IRQHandler,
   [101] = DMA2_Channel10_IRQHandler,
   [102] = DMA2_Channel11_IRQHandler,
};

void __attribute__((section(".after_vectors"))) Default_Handler(void) {
   while (1) {
   }
}

#pragma GCC diagnostic pop

#endif
