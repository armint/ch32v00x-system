#include "peripherals/Exti.h"

#include "utils/Interrupt.h"

namespace Exti {

void setTriggerEdge(uint32_t mask, TriggerEdge trigger) {
   const bool rising = trigger == TriggerEdge::Rising || trigger == TriggerEdge::Both;
   const bool falling = trigger == TriggerEdge::Falling || trigger == TriggerEdge::Both;
   Interrupt::CriticalSection cs;
   EXTI->RTENR = rising ? (EXTI->RTENR | mask) : (EXTI->RTENR & ~mask);
   EXTI->FTENR = falling ? (EXTI->FTENR | mask) : (EXTI->FTENR & ~mask);
}

TriggerEdge getTriggerEdge(uint32_t mask) {
   uint8_t edge = 0;
   if(EXTI->RTENR & mask) {
      edge |= static_cast<uint8_t>(TriggerEdge::Rising);
   }
   if(EXTI->FTENR & mask) {
      edge |= static_cast<uint8_t>(TriggerEdge::Falling);
   }
   return static_cast<TriggerEdge>(edge);
}

void setPortForExtiLine(uint8_t line, Port port) {
   // Without this guard the register index below runs past EXTICR[3].
   if(line >= 16 || port == Port::Undefined) {
      return;
   }
   // The AFIO clock must run before the first EXTICR access.
   RCC->APB2PCENR |= RCC_AFIOEN;
   (void)RCC->APB2PCENR;
   const uint8_t shift = (line & 0x3U) * 4U; // 4 bits per line
   volatile uint32_t &reg = AFIO->EXTICR[line >> 2];
   Interrupt::CriticalSection cs;
   reg = (reg & ~(0xFUL << shift)) | (static_cast<uint32_t>(port) << shift);
}

void configureExtiInterrupt(uint8_t line, Port port, TriggerEdge edge) {
   setPortForExtiLine(line, port);
   setTriggerEdgeForLine(line, edge);
}

} // namespace Exti
