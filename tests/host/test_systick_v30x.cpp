// Host test of the SysTick time base of CH32V30x (src/Timekeeping.cpp): the free-running 64-bit
// counter, the compare value that the handler moves forward one period per tick, and the tick
// count. The SysTick registers are host variables (stubs-systick-v30x/), and the test plays the
// hardware: it sets the counter to the value at the match and calls SysTick_Handler().
//
// Not tested: that the hardware counts up past the compare value without STRE, and the compare
// rule (equal or greater or equal). The clock-time example of ch32v00x-examples checks them on
// the board: it compares the tick count with the cycle count over 60 s.

#include "check.h"
#include "utils/Interrupt.h"
#include "utils/Timekeeping.h"

#include <cstring>
#include <unistd.h>

SysTick_Type hostSysTick;

extern "C" void SysTick_Handler(void);

namespace Interrupt {
CriticalSection::CriticalSection()
   : _state(0) {
}
CriticalSection::~CriticalSection() {
}
} // namespace Interrupt

namespace {

constexpr uint32_t ctlrSte = 1u << 0;
constexpr uint32_t ctlrStie = 1u << 1;
constexpr uint32_t ctlrStclk = 1u << 2;
constexpr uint32_t ctlrStre = 1u << 3;
constexpr uint32_t period = 144000;

timestamp_t hookValue = 0;
int hookCalls = 0;

uint64_t compare() {
   return (static_cast<uint64_t>(hostSysTick.CMPH) << 32) | hostSysTick.CMP;
}

void setCounter(uint64_t value) {
   hostSysTick.CNT = static_cast<uint32_t>(value);
   hostSysTick.CNTH = static_cast<uint32_t>(value >> 32);
}

// The hardware raises the flag at the match; the handler clears it.
void match(uint64_t counterAtInterrupt) {
   setCounter(counterAtInterrupt);
   hostSysTick.SR = 1;
   SysTick_Handler();
   CHECK(hostSysTick.SR == 0);
}

void start() {
   std::memset(static_cast<void *>(&hostSysTick), 0xAA, sizeof(hostSysTick)); // a loader left values
   hookCalls = 0;
   CHECK(SysTick_Config(period) == 0);
}

void testConfig() {
   start();
   // Free running: no auto reload, so CNT counts cycles.
   CHECK(hostSysTick.CTLR == (ctlrStclk | ctlrStie | ctlrSte));
   CHECK((hostSysTick.CTLR & ctlrStre) == 0);
   CHECK(hostSysTick.CNT == 0 && hostSysTick.CNTH == 0);
   CHECK(compare() == period);
   CHECK(SysTick_Config(0) == 1);
}

void testTickEveryPeriod() {
   start();
   const timestamp_t before = getUptimeTicks();
   for(uint32_t i = 1; i <= 5; ++i) {
      match(static_cast<uint64_t>(i) * period);
      CHECK(getUptimeTicks() == before + i);
      CHECK(compare() == static_cast<uint64_t>(i + 1) * period);
   }
   CHECK(hookCalls == 5 && hookValue == before + 5);
}

void testLateInterrupt() {
   start();
   const timestamp_t before = getUptimeTicks();
   // The interrupt came 2.5 periods late: three periods have passed, and the compare is ahead again.
   const uint64_t counter = 3ull * period + period / 2 - 1;
   match(counter);
   CHECK(getUptimeTicks() == before + 3);
   CHECK(compare() == 4ull * period);
   CHECK(compare() > counter);
   CHECK(hookCalls == 1 && hookValue == before + 3);
}

void testSecondInterruptForOneMatch() {
   start();
   const timestamp_t before = getUptimeTicks();
   match(period); // with a compare rule "greater or equal", a second interrupt can follow
   match(period + 20);
   CHECK(getUptimeTicks() == before + 1);
   CHECK(compare() == 2ull * period);
   CHECK(hookCalls == 1);
}

void testMatchBeforeTheCompare() {
   start();
   const timestamp_t before = getUptimeTicks();
   // A few cycles before the compare value (the margin of the handler): the match is counted.
   match(period - 10);
   CHECK(getUptimeTicks() == before + 1);
   CHECK(compare() == 2ull * period);
}

void testLowWordCarry() {
   start();
   const timestamp_t before = getUptimeTicks();
   // 2^32 cycles are 29826.16 periods: the compare value passes the low word carry at period 29827.
   uint64_t counter = 0;
   uint32_t ticks = 0;
   // The bound makes a missing CMPH write fail the test and not hang it.
   while(compare() < (1ull << 32) + 2ull * period && ticks < 40000) {
      counter = compare();
      match(counter);
      ++ticks;
      CHECK(compare() == counter + period);
      CHECK(getUptimeTicks() == before + ticks);
   }
   CHECK(ticks == 29828);
   CHECK(hostSysTick.CMPH == 1);
   CHECK(hostSysTick.CMP == static_cast<uint32_t>(compare()));
   // The 64-bit counter read with a carry in the high word works the same way.
   match(compare());
   CHECK(getUptimeTicks() == before + ticks + 1);
}

} // namespace

extern "C" void sysTickUpdateHook(timestamp_t sysTick) {
   hookValue = sysTick;
   ++hookCalls;
}

int main() {
   alarm(20); // a handler loop that does not end (a broken compare write) fails the test and does not hang it
   testConfig();
   testTickEveryPeriod();
   testLateInterrupt();
   testSecondInterruptForOneMatch();
   testMatchBeforeTheCompare();
   testLowWordCarry();
   if(failures == 0) {
      std::printf("systick-v30x: all checks passed\n");
   }
   return failures;
}
