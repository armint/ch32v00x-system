#pragma once

#include <stdint.h>
#include <wch_device.h>

// GPIO pin with the API of hw::GpioPin in system-stm32, so code that uses
// a GPIO pin builds for both libraries. The CH32 GPIO is the STM32F1 GPIO:
// one 4-bit CNF/MODE field per pin, in CFGLR (pins 0-7) or CFGHR (pins
// 8-15). The encoding is in GpioConfig.h.

namespace hw {

class GpioPin {
   public:
   enum class Mode : uint8_t { Input = 0, Output = 1, Alternate = 2, Analog = 3 };

   enum class OutputType : bool { PushPull = false, OpenDrain = true };

   // The names of system-stm32. GpioConfig.h maps each one to the nearest
   // CH32 rate that is not slower (2, 10 or 50 MHz).
   enum class Speed : uint8_t {
      Low = 0,     // ~400 kHz
      Medium = 1,  // ~2 MHz
      High = 2,    // ~10 MHz
      VeryHigh = 3 // ~40 MHz
   };

   enum class Pull : uint8_t { None = 0, Up = 1, Down = 2 };

   // The CH32 has no AF number per pin: the AFIO remap selects the
   // peripheral. The enum is here for the API of system-stm32.
   enum class AlternateFunction : uint8_t {
      AF0 = 0,
      AF1 = 1,
      AF2 = 2,
      AF3 = 3,
      AF4 = 4,
      AF5 = 5,
      AF6 = 6,
      AF7 = 7,
      AF8 = 8,
      AF9 = 9,
      AF10 = 10,
      AF11 = 11,
      AF12 = 12,
      AF13 = 13,
      AF14 = 14,
      AF15 = 15
   };

   GPIO_TypeDef *const port;
   const uint8_t pin;

   GpioPin(GPIO_TypeDef *port, uint8_t pin)
      : port(port)
      , pin(pin) {
   }

   GpioPin(const GpioPin &) = default;
   GpioPin(GpioPin &&) = default;
   GpioPin &operator=(const GpioPin &) = delete;
   GpioPin &operator=(GpioPin &&) = delete;

   void setMode(Mode mode) const;
   void setOutputType(OutputType type) const;
   void setSpeed(Speed speed) const;
   void setPull(Pull pull) const;

   // Only relevant if the mode is Alternate. See AlternateFunction.
   void setAlternateFunction(AlternateFunction af) const;

   // Writes the whole field once: the pin passes through no other state.
   void configure(Mode mode, OutputType type = OutputType::PushPull, Pull pull = Pull::None, Speed speed = Speed::Low) const;

   // BSHR and BCR: no read-modify-write
   void set() const {
      port->BSHR = (1UL << pin);
   }

   void clear() const {
      port->BCR = (1UL << pin);
   }

   void write(bool state) const {
      if(state) {
         set();
      } else {
         clear();
      }
   }

   // Reads OUTDR, then sets or resets the pin through BSHR. OUTDR itself is
   // not written, so a change to another pin of the port in an interrupt
   // between the read and the write is not lost.
   void toggle() const {
      uint32_t mask = 1UL << pin;
      uint32_t odr = port->OUTDR;
      port->BSHR = ((odr & mask) << 16) | (~odr & mask);
   }

   [[nodiscard]] bool read() const {
      return (port->INDR & (1UL << pin)) != 0;
   }
};

} // namespace hw
