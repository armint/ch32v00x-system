// Minimal CH32V307 test image. CI links it to prove that the CH32V30X part of
// the library builds, links and fits in memory. It never runs on hardware in CI.
// The start-up code calls SystemInit() before main(), as in the USB loopback test.
#include <peripherals/GpioPin.h>
#include <wch_device.h>

int main() {
   // Blue LED of the CH32V307 board on PB4
   RCC->APB2PCENR |= RCC_IOPBEN;
   const GpioPin led(GPIOB, 4);
   led.configureOutput(GpioPin::OutputMode::PushPull, GpioPin::OutputSpeed::Output2MHz);
   while(true) {
      led.toggle();
      for(uint32_t i = 0; i < 1000000; i = i + 1) {
         asm volatile("");
      }
   }
}
