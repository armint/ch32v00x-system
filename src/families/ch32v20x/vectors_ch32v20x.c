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
#include "vectors_ch32v20x.h"
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6)

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
/* External Interrupts */
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
void __attribute__((weak, alias("Default_Handler"))) ADC1_2_IRQHandler(void);
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
void __attribute__((weak, alias("Default_Handler"))) USBFS_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) USBFSWakeUp_IRQHandler(void);
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8)
void __attribute__((weak, alias("Default_Handler"))) ETH_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) ETHWakeUp_IRQHandler(void);
#if defined(CH32V20x_D8W)
void __attribute__((weak, alias("Default_Handler"))) BB_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) LLE_IRQHandler(void);
#endif
void __attribute__((weak, alias("Default_Handler"))) TIM5_IRQHandler(void);
#endif
void __attribute__((weak, alias("Default_Handler"))) UART4_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) DMA1_Channel8_IRQHandler(void);
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8)
void __attribute__((weak, alias("Default_Handler"))) OSC32KCal_IRQHandler(void);
void __attribute__((weak, alias("Default_Handler"))) OSCWakeUp_IRQHandler(void);
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
   WWDG_IRQHandler,            /* Window Watchdog */
   PVD_IRQHandler,             /* PVD through EXTI Line detect */
   TAMPER_IRQHandler,          /* TAMPER */
   RTC_IRQHandler,             /* RTC */
   FLASH_IRQHandler,           /* Flash */
   RCC_IRQHandler,             /* RCC */
   EXTI0_IRQHandler,           /* EXTI Line 0 */
   EXTI1_IRQHandler,           /* EXTI Line 1 */
   EXTI2_IRQHandler,           /* EXTI Line 2 */
   EXTI3_IRQHandler,           /* EXTI Line 3 */
   EXTI4_IRQHandler,           /* EXTI Line 4 */
   DMA1_Channel1_IRQHandler,   /* DMA1 Channel 1 */
   DMA1_Channel2_IRQHandler,   /* DMA1 Channel 2 */
   DMA1_Channel3_IRQHandler,   /* DMA1 Channel 3 */
   DMA1_Channel4_IRQHandler,   /* DMA1 Channel 4 */
   DMA1_Channel5_IRQHandler,   /* DMA1 Channel 5 */
   DMA1_Channel6_IRQHandler,   /* DMA1 Channel 6 */
   DMA1_Channel7_IRQHandler,   /* DMA1 Channel 7 */
   ADC1_2_IRQHandler,          /* ADC1_2 */
   USB_HP_CAN1_TX_IRQHandler,  /* USB HP and CAN1 TX */
   USB_LP_CAN1_RX0_IRQHandler, /* USB LP and CAN1RX0 */
   CAN1_RX1_IRQHandler,        /* CAN1 RX1 */
   CAN1_SCE_IRQHandler,        /* CAN1 SCE */
   EXTI9_5_IRQHandler,         /* EXTI Line 9..5 */
   TIM1_BRK_IRQHandler,        /* TIM1 Break */
   TIM1_UP_IRQHandler,         /* TIM1 Update */
   TIM1_TRG_COM_IRQHandler,    /* TIM1 Trigger and Commutation */
   TIM1_CC_IRQHandler,         /* TIM1 Capture Compare */
   TIM2_IRQHandler,            /* TIM2 */
   TIM3_IRQHandler,            /* TIM3 */
   TIM4_IRQHandler,            /* TIM4 */
   I2C1_EV_IRQHandler,         /* I2C1 Event */
   I2C1_ER_IRQHandler,         /* I2C1 Error */
   I2C2_EV_IRQHandler,         /* I2C2 Event */
   I2C2_ER_IRQHandler,         /* I2C2 Error */
   SPI1_IRQHandler,            /* SPI1 */
   SPI2_IRQHandler,            /* SPI2 */
   USART1_IRQHandler,          /* USART1 */
   USART2_IRQHandler,          /* USART2 */
   USART3_IRQHandler,          /* USART3 */
   EXTI15_10_IRQHandler,       /* EXTI Line 15..10 */
   RTCAlarm_IRQHandler,        /* RTC Alarm through EXTI Line */
   USBWakeUp_IRQHandler,       /* USB Wake up from suspend */
   USBFS_IRQHandler,           /* USBFS Break */
   USBFSWakeUp_IRQHandler,     /* USBFS Wake up from suspend */
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8)
   ETH_IRQHandler,       /* ETH global */
   ETHWakeUp_IRQHandler, /* ETH Wake up */
#if defined(CH32V20x_D8W)
   BB_IRQHandler,  /* BLE BB */
   LLE_IRQHandler, /* BLE LLE */
#else
   0,
   0,
#endif
   TIM5_IRQHandler, /* TIM5 */
#endif
   UART4_IRQHandler,         /* UART4 */
   DMA1_Channel8_IRQHandler, /* DMA1 Channel8 */
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8)
   OSC32KCal_IRQHandler, /* OSC32KCal */
   OSCWakeUp_IRQHandler, /* OSC Wake Up */
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
