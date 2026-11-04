
#include "GpioPin.h"

#include <stddef.h>

// Pins 0 to 7 use CFGLR, pins 8 to 15 use CFGHR. CFGHR is the word directly
// after CFGLR on every family (a reserved word on CH32V00X, which has 8 pins
// per port), so the register is selected by index and no family test is needed.
static_assert(offsetof(GPIO_TypeDef, INDR) == offsetof(GPIO_TypeDef, CFGLR) + 2 * sizeof(uint32_t),
              "CFGHR must directly follow CFGLR");

static constexpr uint8_t cfgIndex(uint8_t pin) {
   return pin / 8;
}

static constexpr uint8_t cfgShift(uint8_t pin) {
   return (pin % 8) * 4;
}

static_assert(cfgIndex(0) == 0 && cfgShift(0) == 0, "pin 0: CFGLR bits 0..3");
static_assert(cfgIndex(7) == 0 && cfgShift(7) == 28, "pin 7: CFGLR bits 28..31");
static_assert(cfgIndex(8) == 1 && cfgShift(8) == 0, "pin 8: CFGHR bits 0..3");
static_assert(cfgIndex(15) == 1 && cfgShift(15) == 28, "pin 15: CFGHR bits 28..31");

static void writePinConfig(GPIO_TypeDef &gpio, uint8_t pin, uint32_t pinCfg) {
   volatile uint32_t &cfgr = (&gpio.CFGLR)[cfgIndex(pin)];
   const uint8_t shift = cfgShift(pin);
   cfgr = (cfgr & ~(0b1111u << shift)) | (pinCfg << shift);
}

GpioPin::GpioPin(GPIO_TypeDef *gpio, uint8_t pin)
   : _gpio(*gpio)
   , _pin(pin) {}

void GpioPin::configureOutput(GpioPin::OutputMode mode, GpioPin::OutputSpeed speed, bool alternateFunction) const {
   uint32_t pinCfg =
   static_cast<uint32_t>(speed) | (static_cast<uint32_t>(mode) << 2) | (static_cast<uint32_t>(alternateFunction) << 3);
   writePinConfig(_gpio, _pin, pinCfg);
}

void GpioPin::configureInput(GpioPin::InputMode mode) const {
   uint32_t pinCfg = (mode >= InputMode::PullDown ? 0b10 : static_cast<uint32_t>(mode)) << 2;
   writePinConfig(_gpio, _pin, pinCfg);
   if(mode == InputMode::PullDown) {
      _gpio.BCR = (1 << _pin);
   } else if(mode == InputMode::PullUp) {
      _gpio.BSHR = (1 << _pin);
   }
}
