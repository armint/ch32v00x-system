#pragma once

#include "utils/Interrupt.h"

#include <stdint.h>
#include <wch_device.h>

// EXTI of CH32V30X with the API of Exti in system-stm32, so code such as
// GpioInterrupt builds for both libraries. Exti.h includes this file.
// Only the GPIO lines 0 to 15 are here. Lines 16 and up (PVD, RTC alarm,
// USB wake-up, Ethernet wake-up) have no port routing and no name yet.

namespace Exti {

enum class TriggerEdge : uint8_t { Disabled = 0, Rising = 1, Falling = 2, Both = 3 };

// The value is the port number in AFIO_EXTICRx.
enum class Port : uint8_t { A = 0, B = 1, C = 2, D = 3, E = 4, Undefined = 255 };

enum Line : uint8_t {
   GPIO0,
   GPIO1,
   GPIO2,
   GPIO3,
   GPIO4,
   GPIO5,
   GPIO6,
   GPIO7,
   GPIO8,
   GPIO9,
   GPIO10,
   GPIO11,
   GPIO12,
   GPIO13,
   GPIO14,
   GPIO15,
};

constexpr IRQn_Type gpioIRQs[16] = {
   EXTI0_IRQn,     EXTI1_IRQn,     EXTI2_IRQn,     EXTI3_IRQn,     EXTI4_IRQn,     EXTI9_5_IRQn,
   EXTI9_5_IRQn,   EXTI9_5_IRQn,   EXTI9_5_IRQn,   EXTI9_5_IRQn,   EXTI15_10_IRQn, EXTI15_10_IRQn,
   EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn,
};

// Connects line (0-15) to the pin of that number on port. Enables the AFIO
// clock. A line of 16 or more does nothing.
void setPortForExtiLine(uint8_t line, Port port);

// One bit per line in mask.
void setTriggerEdge(uint32_t mask, TriggerEdge trigger);
TriggerEdge getTriggerEdge(uint32_t mask);

inline void setTriggerEdgeForLine(uint8_t line, TriggerEdge trigger) {
   setTriggerEdge(1UL << line, trigger);
}

inline TriggerEdge getTriggerEdgeForLine(uint8_t line) {
   return getTriggerEdge(1UL << line);
}

void configureExtiInterrupt(uint8_t line, Port port, TriggerEdge edge);

// INTENR and EVENR are shared by all lines, so each change is a
// read-modify-write in a critical section.
inline void enableInterruptMask(uint32_t mask) {
   Interrupt::CriticalSection cs;
   EXTI->INTENR |= mask;
}

inline void disableInterruptMask(uint32_t mask) {
   Interrupt::CriticalSection cs;
   EXTI->INTENR &= ~mask;
}

inline void enableEventMask(uint32_t mask) {
   Interrupt::CriticalSection cs;
   EXTI->EVENR |= mask;
}

inline void disableEventMask(uint32_t mask) {
   Interrupt::CriticalSection cs;
   EXTI->EVENR &= ~mask;
}

inline void enableInterrupt(uint8_t line) {
   enableInterruptMask(1UL << line);
}

inline void disableInterrupt(uint8_t line) {
   disableInterruptMask(1UL << line);
}

inline void enableEvent(uint8_t line) {
   enableEventMask(1UL << line);
}

inline void disableEvent(uint8_t line) {
   disableEventMask(1UL << line);
}

inline bool isInterruptMasked(uint8_t line) {
   return !(EXTI->INTENR & (1UL << line));
}

inline bool isInterruptPending(uint8_t line) {
   return EXTI->INTFR & (1UL << line);
}

// INTFR bits are write-1-to-clear: no read-modify-write.
inline void clearInterruptPending(uint8_t line) {
   EXTI->INTFR = (1UL << line);
}

inline Port getPortFromGPIO(GPIO_TypeDef *const gpio) {
   if(gpio == GPIOA) {
      return Port::A;
   }
   if(gpio == GPIOB) {
      return Port::B;
   }
   if(gpio == GPIOC) {
      return Port::C;
   }
   if(gpio == GPIOD) {
      return Port::D;
   }
   if(gpio == GPIOE) {
      return Port::E;
   }
   return Port::Undefined;
}

constexpr IRQn_Type getExtiIRQnForGPIOLine(uint8_t line) {
   if(line > 15) {
      return static_cast<IRQn_Type>(-15); // undefined IRQn, as in system-stm32
   }
   return gpioIRQs[line];
}

} // namespace Exti
