#pragma once

#include "GpioPin.h"

#include <stdint.h>

// CH32 pin configuration encoding, shared by GpioPin and the FastPin
// direction switch. It is the STM32F1 encoding (RM0008 9.2.1), so the
// namespace has the name that system-stm32 uses: hw::gpio_f1.
//
// Each pin has one 4-bit field in CFGLR (pins 0-7) or CFGHR (pins 8-15):
// CNF[1:0] in the upper two bits and MODE[1:0] in the lower two bits.
//
//   MODE = 00            input; CNF: 00 analog, 01 floating, 10 pull-up/down
//   MODE = 01, 10, 11    output at 10, 2 or 50 MHz (CH32V00X: 30 MHz);
//                        CNF: 00 push-pull, 01 open-drain,
//                             10 alternate push-pull, 11 alternate open-drain
//
// In input pull mode, the OUTDR bit selects pull-up (1) or pull-down (0).
// Outputs have no internal pull resistors.

namespace hw::gpio_f1 {

constexpr uint8_t MODE_INPUT = 0b00;
constexpr uint8_t MODE_10MHZ = 0b01;
constexpr uint8_t MODE_2MHZ = 0b10;
constexpr uint8_t MODE_50MHZ = 0b11;

constexpr uint8_t CNF_INPUT_ANALOG = 0b00;
constexpr uint8_t CNF_INPUT_FLOATING = 0b01;
constexpr uint8_t CNF_INPUT_PULL = 0b10;
constexpr uint8_t CNF_OUTPUT_OPEN_DRAIN = 0b01;
constexpr uint8_t CNF_OUTPUT_ALTERNATE = 0b10;

// The nearest CH32 rate that is not slower than the GpioPin speed.
constexpr uint8_t speedToMode(GpioPin::Speed speed) {
   switch(speed) {
   case GpioPin::Speed::Low:
   case GpioPin::Speed::Medium:
      return MODE_2MHZ;
   case GpioPin::Speed::High:
      return MODE_10MHZ;
   case GpioPin::Speed::VeryHigh:
      return MODE_50MHZ;
   }
   return MODE_2MHZ;
}

constexpr uint8_t field(uint8_t cnf, uint8_t mode) {
   return static_cast<uint8_t>((cnf << 2) | mode);
}

// The 4-bit CNF/MODE field for a GpioPin configuration. Pull::Up and
// Pull::Down give the input pull mode; the OUTDR bit selects the direction
// and is not part of the field. Outputs and analog inputs ignore pull.
constexpr uint8_t pinConfig(GpioPin::Mode mode,
                            GpioPin::OutputType type = GpioPin::OutputType::PushPull,
                            GpioPin::Pull pull = GpioPin::Pull::None,
                            GpioPin::Speed speed = GpioPin::Speed::Low) {
   switch(mode) {
   case GpioPin::Mode::Analog:
      return field(CNF_INPUT_ANALOG, MODE_INPUT);
   case GpioPin::Mode::Input:
      return field(pull == GpioPin::Pull::None ? CNF_INPUT_FLOATING : CNF_INPUT_PULL, MODE_INPUT);
   case GpioPin::Mode::Output:
   case GpioPin::Mode::Alternate: {
      uint8_t cnf = type == GpioPin::OutputType::OpenDrain ? CNF_OUTPUT_OPEN_DRAIN : 0;
      if(mode == GpioPin::Mode::Alternate) {
         cnf |= CNF_OUTPUT_ALTERNATE;
      }
      return field(cnf, speedToMode(speed));
   }
   }
   return field(CNF_INPUT_FLOATING, MODE_INPUT);
}

// Index of CFGLR (0) or CFGHR (1) from CFGLR.
constexpr uint8_t configIndex(uint8_t pin) {
   return pin / 8;
}

constexpr uint32_t configShift(uint8_t pin) {
   return (pin % 8U) * 4U;
}

static_assert(configIndex(7) == 0 && configShift(7) == 28, "pin 7: CFGLR bits 28..31");
static_assert(configIndex(8) == 1 && configShift(8) == 0, "pin 8: CFGHR bits 0..3");
static_assert(configIndex(15) == 1 && configShift(15) == 28, "pin 15: CFGHR bits 28..31");

} // namespace hw::gpio_f1
