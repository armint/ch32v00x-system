#pragma once

#include "../peripherals/Exti.h"
#include "Interrupt.h"
#include <wch_device.h>

// Pin interrupts. Only
// CH32V30X has the implementation (src/families/ch32v30x/GpioInterrupt.cpp).

namespace GpioInterrupt {

// Connects handler to the EXTI line of pin and enables the NVIC interrupt.
// Pins with the same number on two ports share one line: the call fails for
// the second port. Returns false for a pin above 15.
bool connectHandler(Interrupt::AdvancedHandler_t handler,
                    void *handler_arg,
                    GPIO_TypeDef *port,
                    uint8_t pin,
                    Exti::TriggerEdge edge,
                    bool enableInterruptLine = true);

template <typename T, void (T::*Method)()>
static bool connectHandler(T &instance, GPIO_TypeDef *port, uint8_t pin, Exti::TriggerEdge edge, bool enableInterruptLine = true) {
   return connectHandler(&Interrupt::_memberThunk<T, Method>, static_cast<void *>(&instance), port, pin, edge, enableInterruptLine);
}

bool disconnectHandler(GPIO_TypeDef *port, uint8_t pin);

} // namespace GpioInterrupt
