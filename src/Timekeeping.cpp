#include "utils/Timekeeping.h"
#include "utils/Interrupt.h"
#include <wch_device.h>

// SysTick CTLR bits (QingKe V2/V4 processor manuals)
#define SYSTICK_CTRL_STRE (1ul << 3)  /* Auto reload enable */
#define SYSTICK_CTRL_STCLK (1ul << 2) /* HCLK, not HCLK/8 */
#define SYSTICK_CTRL_STIE (1ul << 1)  /* Counter interrupt enable */
#define SYSTICK_CTRL_STE (1ul << 0)   /* Counter enable */

#define SYSTICK_SR_CNTIF (1ul << 0) /* Counting value comparison flag */

// CH32V20X and CH32V30X have a 64-bit counter and compare value (CH32FV2x_V3xRM
// 9.5.5). The high words keep the values of earlier code (e.g. a loader).
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) || defined(CH32V30x_D8C)
#define SYSTICK_64BIT 1
#else
#define SYSTICK_64BIT 0
#endif

#if defined(__riscv)
#define SYSTICK_HANDLER_ATTRS __attribute__((section(".after_vectors"), interrupt))
#else
#define SYSTICK_HANDLER_ATTRS // host tests (tests/host/test_systick_v30x.cpp)
#endif

static volatile timestamp_t sysTickCount;

#if SYSTICK_64BIT
// The counter runs free at the core clock (no auto reload), so its low word,
// SysTick->CNT, is a 32-bit core-clock cycle counter that the application can
// read (it wraps every 2^32 cycles, 29.8 s at 144 MHz). The tick comes from
// the compare value, which the handler moves forward by one period each tick.
static uint32_t sysTickPeriod;
static uint64_t sysTickNextCompare;

static uint64_t readSysTickCounter() {
   // CNTH, CNT, CNTH again: a carry from the low word between the two reads shows as a changed high word.
   uint32_t high;
   uint32_t low;
   do {
      high = SysTick->CNTH;
      low = SysTick->CNT;
   } while(high != SysTick->CNTH);
   return (static_cast<uint64_t>(high) << 32) | low;
}

// High word first: the in-between value (new high, old low) is later than both the old and the new compare value,
// so it cannot match early or in the past, whatever the compare rule (equal or greater or equal) is.
static void writeSysTickCompare(uint64_t value) {
   SysTick->CMPH = static_cast<uint32_t>(value >> 32);
   SysTick->CMP = static_cast<uint32_t>(value);
}

// A compare value that is less than this many cycles ahead of the counter may be passed before it is written,
// and the compare would never match (equal rule). The handler counts the tick and goes on to the next period.
static constexpr uint32_t sysTickMinAheadCycles = 64;
#endif

extern "C" void SYSTICK_HANDLER_ATTRS SysTick_Handler(void) {
   SysTick->SR &= ~SYSTICK_SR_CNTIF;
#if SYSTICK_64BIT
   // Count the periods that have passed. It is one tick, unless the interrupt was late by a whole period or more.
   // A second interrupt for the same match (compare rule "greater or equal") finds none and counts nothing.
   const uint64_t counter = readSysTickCounter();
   uint32_t elapsed = 0;
   while(sysTickNextCompare <= counter + sysTickMinAheadCycles) {
      sysTickNextCompare += sysTickPeriod;
      ++elapsed;
   }
   if(elapsed != 0) {
      writeSysTickCompare(sysTickNextCompare);
   }
   timestamp_t now;
   {
      Interrupt::CriticalSection guard;
      sysTickCount = sysTickCount + elapsed;
      now = sysTickCount;
   }
   if(elapsed != 0) {
      sysTickUpdateHook(now);
   }
#else
   // Count under the guard: a higher-priority IRQ must not read the count
   // between the two 32-bit stores. The hook runs outside the guard.
   timestamp_t now;
   {
      Interrupt::CriticalSection guard;
      sysTickCount = sysTickCount + 1;
      now = sysTickCount;
   }
   sysTickUpdateHook(now);
#endif
}

timestamp_t getUptimeTicks() {
   // The 64-bit count takes two 32-bit loads. Block the interrupt during the read,
   // so the SysTick handler cannot change the count between the two loads.
   Interrupt::CriticalSection guard;
   return sysTickCount;
}

void __attribute__((weak)) sysTickUpdateHook(timestamp_t sysTick) {
   (void)sysTick;
}

uint32_t SysTick_Config(uint32_t ticks) {
   if(ticks == 0) {
      return 1;
   }
   SysTick->CTLR = 0;
   NVIC_EnableIRQ(SysTicK_IRQn);
   SysTick->SR &= ~SYSTICK_SR_CNTIF;
#if SYSTICK_64BIT
   // Free-running up counter from 0 (STRE off), first match after one period.
   sysTickPeriod = ticks;
   sysTickNextCompare = ticks;
   SysTick->CNT = 0;
   SysTick->CNTH = 0;
   writeSysTickCompare(sysTickNextCompare);
   SysTick->CTLR = SYSTICK_CTRL_STCLK | SYSTICK_CTRL_STIE | SYSTICK_CTRL_STE;
#else
   SysTick->CMP = ticks - 1;
   SysTick->CNT = 0;
   SysTick->CTLR = SYSTICK_CTRL_STRE | SYSTICK_CTRL_STCLK | SYSTICK_CTRL_STIE | SYSTICK_CTRL_STE;
#endif
   return 0;
}
