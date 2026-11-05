// Host tests for the CH32V30X Afio, Exti and GpioInterrupt: the register
// writes, and the dispatch from the interrupt to the handler of a pin.

#include "check.h"
#include "peripherals/Exti.h"
#include "utils/GpioInterrupt.h"

#include <Afio.h>

bool hostNvicEnabled[64];
GPIO_TypeDef hostGpio[5];
RCC_TypeDef hostRcc;
AFIO_TypeDef hostAfio;
EXTI_TypeDef hostExti;

// The test side of utils/Interrupt.h: the connected handler of each IRQ.
namespace {
Interrupt::AdvancedHandler_t connectedHandler[64];
void *connectedArg[64];

void resetRegisters() {
   hostRcc = {};
   hostAfio = {};
   hostExti = {};
   for(bool &enabled : hostNvicEnabled) {
      enabled = false;
   }
}

// Runs the interrupt entry of the IRQ, like the vector table does.
void fire(IRQn_Type irq) {
   connectedHandler[irq](connectedArg[irq]);
}

int callsA;
int callsB;
void handlerA(void *) {
   ++callsA;
}
void handlerB(void *) {
   ++callsB;
}
} // namespace

namespace Interrupt {
void connectHandler(IRQn_Type irq, AdvancedHandler_t handler, void *arg) {
   connectedHandler[irq] = handler;
   connectedArg[irq] = arg;
}
CriticalSection::CriticalSection()
   : _state(0) {
}
CriticalSection::~CriticalSection() {
}
} // namespace Interrupt

namespace {

void testAfio() {
   resetRegisters();
   hostAfio.PCFR1 = 0x00FF00FF; // pattern: no field must change by accident
   hw::Afio::setSwjConfig(hw::Afio::SwjConfig::Disabled);
   CHECK(hostAfio.PCFR1 == (0x00FF00FFu | (4u << 24)));
   CHECK((hostRcc.APB2PCENR & RCC_AFIOEN) != 0);
   hw::Afio::setSwjConfig(hw::Afio::SwjConfig::SwdOnly);
   CHECK(hostAfio.PCFR1 == 0x00FF00FFu);

   hostAfio.PCFR1 = 0;
   hw::Afio::remapSpi3(true);
   CHECK(hostAfio.PCFR1 == (1u << 28));
   hw::Afio::remapSpi1(true);
   hw::Afio::remapI2c1(true);
   hw::Afio::remapUsart1(true);
   hw::Afio::remapUsart2(true);
   CHECK(hostAfio.PCFR1 == ((1u << 28) | 0xFu));
   hw::Afio::remapSpi3(false);
   CHECK(hostAfio.PCFR1 == 0xFu);
   hw::Afio::remapUsart3(hw::Afio::Usart3Remap::Full);
   CHECK(hostAfio.PCFR1 == (0xFu | (3u << 4)));
   hw::Afio::remapUsart3(hw::Afio::Usart3Remap::Partial);
   CHECK(hostAfio.PCFR1 == (0xFu | (1u << 4)));
   hw::Afio::remapUsart3(hw::Afio::Usart3Remap::None);
   CHECK(hostAfio.PCFR1 == 0xFu);
   CHECK(hostAfio.PCFR2 == 0);
}

void testExtiPort() {
   resetRegisters();
   for(auto &reg : hostAfio.EXTICR) {
      reg = 0xFFFFFFFF;
   }
   Exti::setPortForExtiLine(5, Exti::Port::C); // EXTICR[1], bits 7:4
   CHECK(hostAfio.EXTICR[1] == 0xFFFFFF2F);
   CHECK(hostAfio.EXTICR[0] == 0xFFFFFFFF && hostAfio.EXTICR[2] == 0xFFFFFFFF && hostAfio.EXTICR[3] == 0xFFFFFFFF);
   CHECK((hostRcc.APB2PCENR & RCC_AFIOEN) != 0);
   Exti::setPortForExtiLine(15, Exti::Port::E); // EXTICR[3], bits 15:12
   CHECK(hostAfio.EXTICR[3] == 0xFFFF4FFF);
   Exti::setPortForExtiLine(16, Exti::Port::A); // no routing: no write
   Exti::setPortForExtiLine(0, Exti::Port::Undefined);
   CHECK(hostAfio.EXTICR[0] == 0xFFFFFFFF && hostAfio.EXTICR[3] == 0xFFFF4FFF);
}

void testExtiRegisters() {
   resetRegisters();
   Exti::setTriggerEdge(0x28, Exti::TriggerEdge::Rising);
   CHECK(hostExti.RTENR == 0x28 && hostExti.FTENR == 0);
   Exti::setTriggerEdge(0x08, Exti::TriggerEdge::Both);
   CHECK(hostExti.RTENR == 0x28 && hostExti.FTENR == 0x08);
   Exti::setTriggerEdge(0x20, Exti::TriggerEdge::Falling);
   CHECK(hostExti.RTENR == 0x08 && hostExti.FTENR == 0x28);
   CHECK(Exti::getTriggerEdgeForLine(3) == Exti::TriggerEdge::Both);
   CHECK(Exti::getTriggerEdgeForLine(5) == Exti::TriggerEdge::Falling);
   CHECK(Exti::getTriggerEdgeForLine(4) == Exti::TriggerEdge::Disabled);
   Exti::setTriggerEdgeForLine(3, Exti::TriggerEdge::Disabled);
   CHECK(hostExti.RTENR == 0 && hostExti.FTENR == 0x20);

   Exti::enableInterrupt(2);
   Exti::enableInterrupt(9);
   Exti::enableEvent(1);
   CHECK(hostExti.INTENR == 0x204 && hostExti.EVENR == 0x2);
   CHECK(!Exti::isInterruptMasked(9) && Exti::isInterruptMasked(3));
   Exti::disableInterrupt(2);
   Exti::disableEvent(1);
   CHECK(hostExti.INTENR == 0x200 && hostExti.EVENR == 0);

   hostExti.INTFR = 0xFF08;
   CHECK(Exti::isInterruptPending(3) && !Exti::isInterruptPending(4));
   Exti::clearInterruptPending(3); // a write of only this bit: no read-modify-write
   CHECK(hostExti.INTFR == 0x8);

   CHECK(Exti::getPortFromGPIO(GPIOA) == Exti::Port::A);
   CHECK(Exti::getPortFromGPIO(GPIOE) == Exti::Port::E);
   CHECK(Exti::getPortFromGPIO(reinterpret_cast<GPIO_TypeDef *>(0x1234)) == Exti::Port::Undefined);
   CHECK(Exti::getExtiIRQnForGPIOLine(2) == EXTI2_IRQn);
   CHECK(Exti::getExtiIRQnForGPIOLine(7) == EXTI9_5_IRQn);
   CHECK(Exti::getExtiIRQnForGPIOLine(12) == EXTI15_10_IRQn);
   CHECK(static_cast<int>(Exti::getExtiIRQnForGPIOLine(16)) == -15);
}

void testGpioInterrupt() {
   resetRegisters();
   callsA = callsB = 0;
   CHECK(!GpioInterrupt::connectHandler(&handlerA, nullptr, GPIOB, 16, Exti::TriggerEdge::Falling));

   // PB3, falling edge: line 3 routes to port B, only FTENR, INTENR and the IRQ are on.
   CHECK(GpioInterrupt::connectHandler(&handlerA, nullptr, GPIOB, 3, Exti::TriggerEdge::Falling));
   CHECK(((hostAfio.EXTICR[0] >> 12) & 0xF) == 1);
   CHECK(hostExti.RTENR == 0 && hostExti.FTENR == 0x8 && hostExti.INTENR == 0x8);
   CHECK(hostNvicEnabled[EXTI3_IRQn]);
   // Another port on the same line number is refused.
   CHECK(!GpioInterrupt::connectHandler(&handlerB, nullptr, GPIOA, 3, Exti::TriggerEdge::Both));

   hostExti.INTFR = 0x8;
   fire(EXTI3_IRQn);
   CHECK(callsA == 1);
   hostExti.INTFR = 0; // the hardware clears the flag when the code writes the bit
   fire(EXTI3_IRQn);   // nothing pending: no call
   CHECK(callsA == 1);

   // Lines 6 and 8 share the vector of EXTI9_5: only the pending line calls.
   CHECK(GpioInterrupt::connectHandler(&handlerA, nullptr, GPIOC, 6, Exti::TriggerEdge::Rising));
   CHECK(GpioInterrupt::connectHandler(&handlerB, nullptr, GPIOD, 8, Exti::TriggerEdge::Both));
   hostExti.INTFR = 1u << 6;
   fire(EXTI9_5_IRQn);
   CHECK(callsA == 2 && callsB == 0);
   hostExti.INTFR = 1u << 8;
   fire(EXTI9_5_IRQn);
   CHECK(callsA == 2 && callsB == 1);
   // A masked line does not call.
   Exti::disableInterrupt(8);
   hostExti.INTFR = 1u << 8;
   fire(EXTI9_5_IRQn);
   CHECK(callsB == 1);

   // Disconnect keeps the shared IRQ on while line 6 stays connected.
   CHECK(!GpioInterrupt::disconnectHandler(GPIOA, 6)); // wrong port
   CHECK(GpioInterrupt::disconnectHandler(GPIOD, 8));
   CHECK(hostNvicEnabled[EXTI9_5_IRQn]);
   CHECK(GpioInterrupt::disconnectHandler(GPIOC, 6));
   CHECK(!hostNvicEnabled[EXTI9_5_IRQn]);
   CHECK((hostExti.INTENR & ((1u << 6) | (1u << 8))) == 0);
   // After disconnect, another port can use the line.
   CHECK(GpioInterrupt::connectHandler(&handlerB, nullptr, GPIOA, 6, Exti::TriggerEdge::Both));
}

} // namespace

int main() {
   testAfio();
   testExtiPort();
   testExtiRegisters();
   testGpioInterrupt();
   if(failures == 0) {
      std::puts("PASS");
   }
   return failures;
}
