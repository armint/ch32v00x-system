
#pragma once
#include <wch_device.h>

// CH32V30X has the full Exti API (TriggerEdge, Port, ...). Its
// family directory holds Exti_V30x.h. The other families have the short
// API below.
#if __has_include("Exti_V30x.h")
#include "Exti_V30x.h"
#else

namespace Exti {
inline void enableInterrupt(unsigned int line) {
   EXTI->INTENR |= (1 << line);
}
inline void disableInterrupt(unsigned int line) {
   EXTI->INTENR &= ~(1 << line);
}
inline bool isInterruptEnabled(unsigned int line) {
   return EXTI->INTENR & (1 << line);
}
inline void enableEvent(unsigned int line) {
   EXTI->EVENR |= (1 << line);
}
inline void disableEvent(unsigned int line) {
   EXTI->EVENR &= ~(1 << line);
}
inline void setRisingEdge(unsigned int line, bool rising) {
   EXTI->RTENR = (EXTI->RTENR & ~(1 << line)) | (rising << line);
}
inline void setFallingEdge(unsigned int line, bool falling) {
   EXTI->FTENR = (EXTI->FTENR & ~(1 << line)) | (falling << line);
}
inline bool isInterruptTriggered(unsigned int line) {
   return EXTI->INTFR & (1 << line);
}
}; // namespace Exti
#endif
