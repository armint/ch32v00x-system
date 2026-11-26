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

// PCFR1 and PCFR2 hold the fields of many peripherals, so each change is a
// read-modify-write in a critical section.
void update(volatile uint32_t &reg, uint32_t mask, uint32_t value) {
   enableClock();
   Interrupt::CriticalSection cs;
   reg = (reg & ~mask) | (value & mask);
}

} // namespace

void setSwjConfig(SwjConfig config) {
   update(AFIO->PCFR1, AFIO_PCFR1_SWJ_CFG, static_cast<uint32_t>(config));
}

void setRemap(uint32_t mask, uint32_t value) {
   assert((mask & AFIO_PCFR1_SWJ_CFG) == 0);
   update(AFIO->PCFR1, mask & ~AFIO_PCFR1_SWJ_CFG, value);
}

void setRemap2(uint32_t mask, uint32_t value) {
   update(AFIO->PCFR2, mask, value);
}

} // namespace hw::Afio
