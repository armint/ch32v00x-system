#include "utils/GpioInterrupt.h"

namespace GpioInterrupt {

namespace {

struct LineConnection {
   Interrupt::AdvancedHandler_t handler;
   void *handlerArg;
   Exti::Port port;
};

LineConnection lineConnections[16] = {
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
   { nullptr, nullptr, Exti::Port::Undefined }, { nullptr, nullptr, Exti::Port::Undefined },
};

// One NVIC interrupt serves the lines first to last.
struct Vector {
   uint8_t first;
   uint8_t last;
};

constexpr Vector vectors[] = { { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 9 }, { 10, 15 } };

const Vector &vectorOfLine(uint8_t line) {
   for(const Vector &vector : vectors) {
      if(line <= vector.last) {
         return vector;
      }
   }
   return vectors[0]; // not reached: line is 15 or less
}

// The interrupt entry is Interrupt::handlerWithArgument, which calls this.
void dispatch(void *arg) {
   const Vector &vector = *static_cast<const Vector *>(arg);
   for(uint8_t line = vector.first; line <= vector.last; ++line) {
      if(!Exti::isInterruptPending(line)) {
         continue;
      }
      // Clear before the call: an edge during the handler stays pending.
      Exti::clearInterruptPending(line);
      const LineConnection &connection = lineConnections[line];
      if(!Exti::isInterruptMasked(line) && connection.handler != nullptr) {
         connection.handler(connection.handlerArg);
      }
   }
   NVIC_ClearPendingIRQ(Exti::getExtiIRQnForGPIOLine(vector.first));
}

} // namespace

bool connectHandler(Interrupt::AdvancedHandler_t handler, void *handler_arg, GPIO_TypeDef *port, uint8_t pin, Exti::TriggerEdge edge, bool enableInterruptLine) {
   if(pin > 15) {
      return false;
   }
   const Exti::Port extiPort = Exti::getPortFromGPIO(port);
   LineConnection &connection = lineConnections[pin];
   if(extiPort == Exti::Port::Undefined || (connection.handler != nullptr && connection.port != extiPort)) {
      return false;
   }
   const IRQn_Type irq = Exti::getExtiIRQnForGPIOLine(pin);
   NVIC_DisableIRQ(irq);
   connection = { handler, handler_arg, extiPort };
   Exti::configureExtiInterrupt(pin, extiPort, edge);
   if(enableInterruptLine) {
      Exti::enableInterrupt(pin);
   }
   Exti::clearInterruptPending(pin);
   NVIC_ClearPendingIRQ(irq);
   Interrupt::connectHandler(irq, &dispatch, const_cast<Vector *>(&vectorOfLine(pin)));
   NVIC_EnableIRQ(irq);
   return true;
}

bool disconnectHandler(GPIO_TypeDef *port, uint8_t pin) {
   if(pin > 15 || Exti::getPortFromGPIO(port) != lineConnections[pin].port) {
      return false;
   }
   const IRQn_Type irq = Exti::getExtiIRQnForGPIOLine(pin);
   bool otherLineConnected = false;
   for(uint8_t line = 0; line < 16; ++line) {
      if(line != pin && lineConnections[line].handler != nullptr && Exti::gpioIRQs[line] == irq) {
         otherLineConnected = true;
      }
   }
   NVIC_DisableIRQ(irq);
   lineConnections[pin] = { nullptr, nullptr, Exti::Port::Undefined };
   Exti::disableInterrupt(pin);
   Exti::clearInterruptPending(pin);
   NVIC_ClearPendingIRQ(irq);
   if(otherLineConnected) {
      NVIC_EnableIRQ(irq);
   }
   return true;
}

} // namespace GpioInterrupt
