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

using hw::GpioPin;
using Mode = GpioPin::Mode;
using Type = GpioPin::OutputType;
using Pull = GpioPin::Pull;
using Speed = GpioPin::Speed;

void testRegisterLayout() {
   static_assert(offsetof(GPIO_TypeDef, INDR) == 0x08);
   static_assert(offsetof(GPIO_TypeDef, BCR) == 0x14);
}

void testConfigure() {
   for(uint8_t pin = 0; pin < PinCount; ++pin) {
      Port port;
      GpioPin gpioPin(&port.gpio, pin);

      gpioPin.configure(Mode::Output, Type::PushPull, Pull::None, Speed::VeryHigh);
      CHECK(port.field(pin) == 0b0011);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configure(Mode::Output, Type::OpenDrain);
      CHECK(port.field(pin) == 0b0110);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configure(Mode::Alternate, Type::PushPull, Pull::None, Speed::High);
      CHECK(port.field(pin) == 0b1001);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configure(Mode::Alternate, Type::OpenDrain, Pull::None, Speed::VeryHigh);
      CHECK(port.field(pin) == 0b1111);
      CHECK(port.othersUnchanged(pin));
   }
}

void testConfigureInput() {
   for(uint8_t pin = 0; pin < PinCount; ++pin) {
      Port port;
      GpioPin gpioPin(&port.gpio, pin);

      gpioPin.configure(Mode::Analog);
      CHECK(port.field(pin) == 0b0000);
      CHECK(port.othersUnchanged(pin));

      gpioPin.configure(Mode::Input);
      CHECK(port.field(pin) == 0b0100);
      CHECK(port.othersUnchanged(pin));
      CHECK(port.gpio.BSHR == 0 && port.gpio.BCR == 0); // no pull selected

      gpioPin.configure(Mode::Input, Type::PushPull, Pull::Down);
      CHECK(port.field(pin) == 0b1000);
      CHECK(port.gpio.BCR == (1U << pin));
      CHECK(port.othersUnchanged(pin));

      gpioPin.configure(Mode::Input, Type::PushPull, Pull::Up);
      CHECK(port.field(pin) == 0b1000);
      CHECK(port.gpio.BSHR == (1U << pin));
      CHECK(port.othersUnchanged(pin));
   }
}

// The setters change only their part of the field (call setMode() first).
void testSetters() {
   for(uint8_t pin = 0; pin < PinCount; ++pin) {
      Port port;
      GpioPin gpioPin(&port.gpio, pin);

      gpioPin.configure(Mode::Output, Type::OpenDrain, Pull::None, Speed::VeryHigh);
      gpioPin.setMode(Mode::Alternate); // keeps the speed and open drain
      CHECK(port.field(pin) == 0b1111);
      gpioPin.setOutputType(Type::PushPull);
      CHECK(port.field(pin) == 0b1011);
      gpioPin.setSpeed(Speed::High);
      CHECK(port.field(pin) == 0b1001);
      gpioPin.setMode(Mode::Output);
      CHECK(port.field(pin) == 0b0001);

      gpioPin.setMode(Mode::Input);
      CHECK(port.field(pin) == 0b0100);
      gpioPin.setSpeed(Speed::VeryHigh); // inputs have no speed
      CHECK(port.field(pin) == 0b0100);
      gpioPin.setPull(Pull::Up);
      CHECK(port.field(pin) == 0b1000);
      CHECK(port.gpio.BSHR == (1U << pin));
      gpioPin.setMode(Mode::Input); // keeps the pull
      CHECK(port.field(pin) == 0b1000);
      gpioPin.setPull(Pull::None);
      CHECK(port.field(pin) == 0b0100);
      CHECK(port.othersUnchanged(pin));
   }
}

// write(), toggle() and read() use BSHR, BCR, OUTDR and INDR only.
void testData() {
   Port port;
   const uint8_t pin = PinCount - 1;
   GpioPin gpioPin(&port.gpio, pin);

   gpioPin.write(true);
   CHECK(port.gpio.BSHR == (1U << pin));
   gpioPin.write(false);
   CHECK(port.gpio.BCR == (1U << pin));

   port.gpio.OUTDR = 0;
   gpioPin.toggle();
   CHECK(port.gpio.BSHR == (1U << pin)); // set
   port.gpio.OUTDR = 1U << pin;
   gpioPin.toggle();
   CHECK(port.gpio.BSHR == (1U << (pin + 16))); // reset
   CHECK(port.gpio.OUTDR == (1U << pin));       // OUTDR is not written

   port.gpio.INDR = 1U << pin;
   CHECK(gpioPin.read());
   port.gpio.INDR = ~(1U << pin);
   CHECK(!gpioPin.read());
}

} // namespace

int main() {
   testRegisterLayout();
   testConfigure();
   testConfigureInput();
   testSetters();
   testData();
   if(failures != 0) {
      std::printf("%d check(s) failed\n", failures);
      return 1;
   }
   std::printf("all GpioPin host tests passed (%u pins)\n", PinCount);
   return 0;
}
