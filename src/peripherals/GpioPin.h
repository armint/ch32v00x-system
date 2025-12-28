
#pragma once

#include <wch_device.h>

class GpioPin {
   GPIO_TypeDef &_gpio;
   const uint8_t _pin;

   public:
   enum class OutputSpeed : uint8_t {
      Output10MHz = 0b01,
      Output2MHz = 0b10,
      Output50MHz = 0b11,
   };

   enum class OutputMode : bool { PushPull = 0, OpenDrain };

   enum class InputMode : uint8_t { Analog, Floating, PullDown, PullUp };

   GpioPin(GPIO_TypeDef *gpio, uint8_t pin);
   void configureOutput(OutputMode mode, OutputSpeed speed, bool alternateFunction = false) const;
   void configureInput(InputMode mode) const;

   inline uint8_t getPinNumber() const {
      return _pin;
   }

   inline void set() const {
      _gpio.BSHR = 1 << _pin;
   }

   inline void clear() const {
      _gpio.BCR = 1 << _pin;
   }

   inline void toggle() const {
      _gpio.BSHR = (1 << (_pin + (_gpio.OUTDR & (1 << _pin) ? 16 : 0)));
   }

   inline bool getOutputValue() const {
      return _gpio.OUTDR & (1 << _pin);
   }

   inline bool getInputValue() const {
      return _gpio.INDR & (1 << _pin);
   }
};
