#pragma once

#include "GpioConfig.h"
#include "GpioPin.h"

#include <stdint.h>

namespace hw {

/**
 * @brief GPIO pin with the port and pin fixed at compile time, with the API
 * of hw::FastPin in system-stm32 (STM32F1 branch).
 *
 * For tight bit-bang loops (e.g. JTAG/SWD). Each access is one store to
 * BSHR or one load from INDR, with a constant address and mask, and no
 * read-modify-write. Use pin() to get a GpioPin for configure().
 *
 * setOutput(), setInput() and setConfig<>() switch the pin direction (e.g.
 * SWDIO at an SWD turnaround). RISC-V has no bit-band alias, so they write
 * the 4-bit CNF/MODE field with one read-modify-write of CFGLR or CFGHR,
 * with interrupts off. The pin goes from the old to the new field in one
 * store.
 *
 * Example: using Tck = hw::FastPin<GPIOA_BASE, 5>;
 *
 * @tparam PortBase the GPIO port base address, e.g. GPIOA_BASE.
 * @tparam Pin the pin number (0-15).
 */
template <uintptr_t PortBase, uint8_t Pin>
struct FastPin {
   static_assert(Pin < 16, "GPIO pin number must be 0-15");

   static constexpr uint32_t mask = 1UL << Pin;

   [[gnu::always_inline]] static inline GPIO_TypeDef *port() {
      return reinterpret_cast<GPIO_TypeDef *>(PortBase);
   }

   [[gnu::always_inline]] static inline void set() {
      port()->BSHR = mask;
   }

   // The upper half of BSHR does the same as BCR.
   [[gnu::always_inline]] static inline void clear() {
      port()->BSHR = mask << 16;
   }

   // Branch-free: when both the set and the reset bit are 1, set wins.
   [[gnu::always_inline]] static inline void write(bool state) {
      port()->BSHR = (mask << 16) | (static_cast<uint32_t>(state) << Pin);
   }

   [[gnu::always_inline]] [[nodiscard]] static inline bool read() {
      return (port()->INDR & mask) != 0;
   }

   static GpioPin pin() {
      return { port(), Pin };
   }

   // Push-pull output, 50 MHz.
   static constexpr uint8_t OutputConfig =
   gpio_f1::pinConfig(GpioPin::Mode::Output, GpioPin::OutputType::PushPull, GpioPin::Pull::None, GpioPin::Speed::VeryHigh);
   // Floating input.
   static constexpr uint8_t InputConfig = gpio_f1::pinConfig(GpioPin::Mode::Input);

   /**
    * @brief Writes the 4-bit CNF/MODE field of this pin. OUTDR (the output
    * level or the pull direction) does not change.
    * @tparam Config a field value, e.g. gpio_f1::pinConfig(...).
    */
   template <uint8_t Config>
   [[gnu::always_inline]] static inline void setConfig() {
      static_assert(Config <= 0xF, "CNF/MODE field is 4 bits");
      static_assert(Config != 0b1100, "CNF 11 with MODE 00 is reserved");
      constexpr uint32_t shift = gpio_f1::configShift(Pin);
      constexpr uint32_t field = 0xFUL << shift;
      constexpr uint32_t value = static_cast<uint32_t>(Config) << shift;
      volatile uint32_t &reg = (&port()->CFGLR)[gpio_f1::configIndex(Pin)];
      // Clear mstatus.MIE and keep its old value; restore only that bit.
      uint32_t mie;
      asm volatile("csrrci %0, mstatus, 8" : "=r"(mie) : : "memory");
      reg = (reg & ~field) | value;
      asm volatile("csrs mstatus, %0" : : "r"(mie & 8U) : "memory");
   }

   [[gnu::always_inline]] static inline void setOutput() {
      setConfig<OutputConfig>();
   }

   [[gnu::always_inline]] static inline void setInput() {
      setConfig<InputConfig>();
   }
};

} // namespace hw
