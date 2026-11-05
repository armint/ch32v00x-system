// Host tests for GpioPin register selection: pins 0 to 7 use CFGLR, pins 8
// to 15 use CFGHR. One program per family; the stub <wch_device.h> in the
// include path gives the GPIO register layout of that family.
// Define GPIO_PINS_PER_PORT to 8 (CH32V00X) or 16 (CH32V20X, CH32V30X).

#include "check.h"
#include "peripherals/GpioPin.h"

#include <cstddef>

#ifndef GPIO_PINS_PER_PORT
#error "GPIO_PINS_PER_PORT must be set"
#endif

namespace {

constexpr uint8_t PinCount = GPIO_PINS_PER_PORT;
constexpr bool HasCfgHr = PinCount > 8;

// Both words are an arbitrary pattern, so a write to the wrong word or the
// wrong nibble changes a value that the checks see.
constexpr uint32_t PatternLow = 0x89ABCDEF;
constexpr uint32_t PatternHigh = 0x01234567;
constexpr uint32_t Reserved = 0xA5A5A5A5;

struct Port {
   GPIO_TypeDef gpio{ PatternLow, 0, 0, 0, 0, 0, 0 };

   Port() {
      (&gpio.CFGLR)[1] = HasCfgHr ? PatternHigh : Reserved;
   }

   volatile uint32_t &word(uint8_t pin) {
      return (&gpio.CFGLR)[pin / 8];
   }

   uint32_t field(uint8_t pin) {
      return (word(pin) >> ((pin % 8) * 4)) & 0xFU;
   }

   // Pin data before the write: the nibble of the pattern.
   static uint32_t initialField(uint8_t pin) {
      return ((pin < 8 ? PatternLow : PatternHigh) >> ((pin % 8) * 4)) & 0xFU;
   }

   // Every other pin keeps its pattern nibble, and CFGHR on CH32V00X (a
   // reserved word) is unchanged.
   bool othersUnchanged(uint8_t pin) {
      for(uint8_t other = 0; other < PinCount; ++other) {
         if(other != pin && field(other) != initialField(other)) {
            return false;
         }
      }
      return HasCfgHr || (&gpio.CFGLR)[1] == Reserved;
   }
};

using Mode = GpioPin::OutputMode;
using Speed = GpioPin::OutputSpeed;
using In = GpioPin::InputMode;

void testRegisterLayout() {
   static_assert(offsetof(GPIO_TypeDef, INDR) == 0x08);
   static_assert(offsetof(GPIO_TypeDef, BCR) == 0x14);
}

void testConfigureOutput() {
   for(uint8_t pin = 0; pin < PinCount; ++pin) {
      Port port;
      GpioPin gpioPin(&port.gpio, pin);

      gpioPin.configureOutput(Mode::PushPull, Speed::Output50MHz);
      CHECK(port.field(pin) == 0b0011);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configureOutput(Mode::OpenDrain, Speed::Output2MHz);
      CHECK(port.field(pin) == 0b0110);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configureOutput(Mode::PushPull, Speed::Output10MHz, true);
      CHECK(port.field(pin) == 0b1001);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configureOutput(Mode::OpenDrain, Speed::Output50MHz, true);
      CHECK(port.field(pin) == 0b1111);
      CHECK(port.othersUnchanged(pin));
   }
}

void testConfigureInput() {
   for(uint8_t pin = 0; pin < PinCount; ++pin) {
      Port port;
      GpioPin gpioPin(&port.gpio, pin);

      gpioPin.configureInput(In::Analog);
      CHECK(port.field(pin) == 0b0000);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configureInput(In::Floating);
      CHECK(port.field(pin) == 0b0100);
      CHECK(port.othersUnchanged(pin));
      CHECK(port.gpio.BSHR == 0 && port.gpio.BCR == 0); // no pull selected

      gpioPin.configureInput(In::PullDown);
      CHECK(port.field(pin) == 0b1000);
      CHECK(port.gpio.BCR == (1U << pin));
      CHECK(port.othersUnchanged(pin));

      gpioPin.configureInput(In::PullUp);
      CHECK(port.field(pin) == 0b1000);
      CHECK(port.gpio.BSHR == (1U << pin));
      CHECK(port.othersUnchanged(pin));
   }
}

} // namespace

int main() {
   testRegisterLayout();
   testConfigureOutput();
   testConfigureInput();
   if(failures != 0) {
      std::printf("%d check(s) failed\n", failures);
      return 1;
   }
   std::printf("all GpioPin host tests passed (%u pins)\n", PinCount);
   return 0;
}
