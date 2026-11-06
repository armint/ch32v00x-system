// Minimal CH32V307 test image. CI links it to prove that the CH32V30X part of
// the library builds, links and fits in memory. It never runs on hardware in CI.
// The start-up code calls SystemInit() before main(), as in the USB loopback test.
#include <Afio.h>
#include <cmsis_device.h>
#include <peripherals/CycleCounter.h>
#include <peripherals/FastPin.h>
#include <peripherals/GpioPin.h>
#include <utils/GpioInterrupt.h>

static void onKey(void *) {
}

int main() {
   // Blue LED of the CH32V307 board on PB4
   RCC->APB2PCENR |= RCC_IOPBEN;
   // FastPin sets the direction, GpioPin toggles: CI compiles both.
   using Led = hw::FastPin<GPIOB_BASE, 4>;
   Led::setOutput();
   // CI compiles the pin interrupt and the remap: key on PB3, SWD stays on.
   GpioInterrupt::connectHandler(&onKey, nullptr, GPIOB, 3, Exti::TriggerEdge::Falling);
   hw::Afio::setSwjConfig(hw::Afio::SwjConfig::SwdOnly);
   const hw::GpioPin led = Led::pin();
   // CI compiles the cycle counter: the LED toggles every 1000000 core clock cycles.
   hw::CycleCounter::start();
   while(true) {
      led.toggle();
      const uint32_t start = hw::CycleCounter::count();
      while(hw::CycleCounter::count() - start < 1000000u) {
      }
   }
}
