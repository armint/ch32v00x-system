
#include "GpioPin.h"

GpioPin::GpioPin(GPIO_TypeDef *gpio, uint8_t pin)
   : _gpio(*gpio)
   , _pin(pin) {
}

void GpioPin::configureOutput(GpioPin::OutputMode mode, GpioPin::OutputSpeed speed, bool alternateFunction) const {
   uint32_t pinCfg =
   static_cast<uint32_t>(speed) | (static_cast<uint32_t>(mode) << 2) | (static_cast<uint32_t>(alternateFunction) << 3);
   _gpio.CFGLR = (_gpio.CFGLR & ~(0b1111 << _pin * 4)) | (pinCfg << _pin * 4);
}

void GpioPin::configureInput(GpioPin::InputMode mode) const {
   uint32_t pinCfg = (mode >= InputMode::PullDown ? 3 : static_cast<uint32_t>(mode)) << 2;
   uint32_t pullCfg = (static_cast<uint32_t>(mode) - 3) << _pin;
   _gpio.CFGLR = (_gpio.CFGLR & ~(0b1111 << _pin * 4)) | (pinCfg << _pin * 4);
   _gpio.OUTDR = (_gpio.OUTDR & ~(1 << _pin)) | pullCfg;
}
