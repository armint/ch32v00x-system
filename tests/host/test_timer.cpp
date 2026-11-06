// Host test for hw::Timer with the TIM layout of each family. A block of host memory plays
// the register file. The test checks the register offsets and bits that each function
// writes, and three old bugs: the compare register stride on CH32V20X (issue #7), the
// preload bit of channels 2 and 4 (issue #8) and the flag clear without a read (issue #10).

#include "check.h"
#include "peripherals/Timer.h"
#include <cstddef>
#include <cstring>

alignas(8) static uint8_t timMemory[0x54];

static uint32_t word(uint32_t offset) {
   uint32_t value;
   std::memcpy(&value, timMemory + offset, sizeof(value));
   return value;
}

static void clear() {
   std::memset(timMemory, 0, sizeof(timMemory));
}

int main() {
   static_assert(offsetof(TIM_TypeDef, CH1CVR) == 0x34);
   static_assert(offsetof(TIM_TypeDef, BDTR) == 0x44);
   auto *tim = reinterpret_cast<TIM_TypeDef *>(timMemory);
   const hw::Timer t(tim);
   using Ch = hw::Timer::Channel;

   // Compare registers: CH1CVR to CH4CVR at 0x34, 0x38, 0x3C and 0x40.
   clear();
   t.setCompare(Ch::Ch1, 0x1111);
   t.setCompare(Ch::Ch2, 0x2222);
   t.setCompare(Ch::Ch3, 0x3333);
   t.setCompare(Ch::Ch4, 0x4444);
   CHECK(word(0x34) == 0x1111);
   CHECK(word(0x38) == 0x2222);
   CHECK(word(0x3C) == 0x3333);
   CHECK(word(0x40) == 0x4444);
   CHECK(t.getCompare(Ch::Ch3) == 0x3333);

   // Output compare mode and preload: channel 1 at bit 0 and channel 2 at bit 8 of CHCTLR1
   // (0x18), channels 3 and 4 the same in CHCTLR2 (0x1C).
   clear();
   t.setOcMode(Ch::Ch1, hw::Timer::OcMode::Pwm1);
   t.setOcMode(Ch::Ch2, hw::Timer::OcMode::Pwm2);
   t.setOcMode(Ch::Ch4, hw::Timer::OcMode::Toggle);
   CHECK(word(0x18) == (0x60u | 0x7000u));
   CHECK(word(0x1C) == 0x3000u);
   t.setOcMode(Ch::Ch2, hw::Timer::OcMode::Frozen);
   CHECK(word(0x18) == 0x60u);
   t.enableOcPreload(Ch::Ch2);
   t.enableOcPreload(Ch::Ch4);
   CHECK(word(0x18) == (0x60u | 0x0800u));
   CHECK(word(0x1C) == (0x3000u | 0x0800u));
   t.disableOcPreload(Ch::Ch4);
   CHECK(word(0x1C) == 0x3000u);
   t.enableOcPreload(Ch::Ch3);
   CHECK(word(0x1C) == (0x3000u | 0x0008u));

   // CCER (0x20): 4 bits for each channel.
   clear();
   t.enableChannel(Ch::Ch4);
   t.setCaptureComparePolarity(Ch::Ch4, hw::Timer::Polarity::Low);
   CHECK(word(0x20) == 0x3000u);
   t.setCaptureComparePolarity(Ch::Ch4, hw::Timer::Polarity::High);
   t.disableChannel(Ch::Ch4);
   CHECK(word(0x20) == 0);

   // DMAINTENR (0x0C): UDE bit 8, CC1DE to CC4DE bits 9 to 12, CC1IE to CC4IE bits 1 to 4.
   clear();
   t.enableUpdateDma();
   t.enableCcDma(Ch::Ch4);
   t.enableCcInterrupt(Ch::Ch2);
   t.enableUpdateInterrupt();
   CHECK(word(0x0C) == (0x0100u | 0x1000u | 0x0004u | 0x0001u));
   CHECK(t.isUpdateInterruptEnabled());
   t.disableCcDma(Ch::Ch4);
   t.disableUpdateDma();
   t.disableCcInterrupt(Ch::Ch2);
   t.disableUpdateInterrupt();
   CHECK(word(0x0C) == 0);

   // CTLR1 (0x00): CEN, URS, OPM, DIR and ARPE.
   clear();
   t.setOnePulseMode(true);
   t.setUpdateRequestSourceOnlyOverflow(true);
   t.setAutoReloadPreload(true);
   t.setDirectionDown(true);
   t.enableCounter();
   CHECK(word(0x00) == (0x01u | 0x04u | 0x08u | 0x10u | 0x80u));
   CHECK(t.isCounterEnabled());
   t.setOnePulseMode(false);
   t.disableCounter();
   CHECK(word(0x00) == (0x04u | 0x10u | 0x80u));

   // Counter, prescaler, reload and update event.
   clear();
   t.setCounter(0x1234);
   t.setPrescaler(0x5678);
   t.setAutoReload(0x9ABC);
   t.generateUpdate();
   CHECK(word(0x24) == 0x1234);
   CHECK(t.getCounter() == 0x1234);
   CHECK(word(0x28) == 0x5678);
   CHECK(t.getPrescaler() == 0x5678);
   CHECK(word(0x2C) == 0x9ABC);
   CHECK(t.getAutoReload() == 0x9ABC);
   CHECK(word(0x14) == 0x0001);

   // INTFR (0x10) is write-0-to-clear: a clear writes the inverted mask and does not read.
   clear();
   tim->INTFR = 0x0003;
   CHECK(t.isUpdateFlag());
   CHECK(t.getStatusFlag(TIM_CC1IF));
   t.clearUpdateFlag();
   CHECK((word(0x10) & 0xFFFF) == 0xFFFE);
   t.clearCaptureCompare3Flag();
   CHECK((word(0x10) & 0xFFFF) == 0xFFF7);
   t.clearAllStatusFlags();
   CHECK(word(0x10) == 0);

   // BDTR (0x44) of the advanced timers.
   clear();
   t.enableMainOutput();
   t.enableAutomaticOutput();
   CHECK(word(0x44) == 0xC000u);
   t.disableMainOutput();
   CHECK(word(0x44) == 0x4000u);

   // Trigger output (MMS in CTLR2 at 0x04) and master/slave mode (MSM in SMCFGR at 0x08).
   clear();
   tim->SMCFGR = 0x0080;
   t.setTriggerOutput(0x0020);
   t.disableMasterSlaveMode();
   CHECK(word(0x04) == 0x0020);
   CHECK(word(0x08) == 0);

   if(failures == 0) {
      std::printf("timer: all checks pass\n");
   }
   return failures;
}
