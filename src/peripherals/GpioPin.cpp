
#include "GpioPin.h"

GpioPin::GpioPin(GPIO_TypeDef *gpio, uint8_t pin)
   : _gpio(*gpio)
   , _pin(pin) {
}

void GpioPin::configureOutput(GpioPin::OutputMode mode, GpioPin::OutputSpeed speed, bool alternateFunction) const {
   uint32_t pinCfg =
   static_cast<uint32_t>(speed) | (static_cast<uint32_t>(mode) << 2) | (static_cast<uint32_t>(alternateFunction) << 3);
   _gpio.CFGLR = (_gpio.CFGLR & ~(0b1111 << (_pin * 4))) | (pinCfg << (_pin * 4));
}

void GpioPin::configureInput(GpioPin::InputMode mode) const {
   uint32_t pinCfg = (mode >= InputMode::PullDown ? 0b10 : static_cast<uint32_t>(mode)) << 2;
   _gpio.CFGLR = (_gpio.CFGLR & ~(0b1111 << (_pin * 4))) | (pinCfg << (_pin * 4));
   if(mode == InputMode::PullDown) {
      _gpio.BCR = (1 << _pin);
   } else if(mode == InputMode::PullUp) {
      _gpio.BSHR = (1 << _pin);
   }
}
