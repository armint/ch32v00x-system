#pragma once

// Clock enables, peripheral resets and bus clocks of the RCC. The family header
// (Rcc_V00x.h, Rcc_V20x.h or Rcc_V30x.h) gives the bus of each peripheral.

#include <cstdint>
#include <wch_device.h>

namespace Rcc {
namespace detail {
// The read back makes sure that the clock is on before the next access to the peripheral.
inline void enableClock(volatile uint32_t &reg, uint32_t bit) {
   reg |= bit;
   [[maybe_unused]] const uint32_t readBack = reg;
}

// Sets and clears the reset bit: the peripheral registers go back to their reset values.
inline void resetPeripheral(volatile uint32_t &reg, uint32_t bit) {
   reg |= bit;
   reg &= ~bit;
}
} // namespace detail

// HCLK: SystemCoreClock. Call SystemCoreClockUpdate() after a change of the clock tree.
// On CH32V30X, SystemCoreClockUpdate() ignores HPRE: this value is right only for HPRE = /1,
// which the start-up sets.
uint32_t getAHBClock();
// PCLK1, the clock of the APB1 peripherals. CH32V00X has no APB prescaler: HCLK.
uint32_t getAPBClock();
// PCLK2, the clock of the APB2 peripherals. CH32V00X has no APB prescaler: HCLK.
uint32_t getAPB2Clock();
} // namespace Rcc

#if __has_include(<Rcc_V30x.h>)
#include <Rcc_V30x.h>
#elif __has_include(<Rcc_V20x.h>)
#include <Rcc_V20x.h>
#elif __has_include(<Rcc_V00x.h>)
#include <Rcc_V00x.h>
#else
#error "No Rcc family header: check CH32_FAMILY"
#endif
