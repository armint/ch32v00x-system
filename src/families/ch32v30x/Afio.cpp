#include "Afio.h"

#include "utils/Interrupt.h"

#include <assert.h>

namespace hw::Afio {

namespace {

void enableClock() {
   RCC->APB2PCENR |= RCC_AFIOEN;
   // Read back, so the clock is on before the first AFIO access.
   (void)RCC->APB2PCENR;
}

// PCFR1 holds the fields of many peripherals, so each change is a
// read-modify-write in a critical section.
void update(uint32_t mask, uint32_t value) {
   enableClock();
   Interrupt::CriticalSection cs;
   AFIO->PCFR1 = (AFIO->PCFR1 & ~mask) | (value & mask);
}

} // namespace

void setSwjConfig(SwjConfig config) {
   update(AFIO_PCFR1_SW_CFG_MASK, static_cast<uint32_t>(config));
}

void setRemap(uint32_t mask, uint32_t value) {
   assert((mask & AFIO_PCFR1_SW_CFG_MASK) == 0);
   update(mask & ~AFIO_PCFR1_SW_CFG_MASK, value);
}

} // namespace hw::Afio
