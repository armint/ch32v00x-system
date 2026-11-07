// Host tests for the CH32V30x ADC driver (src/families/ch32v30x/Adc):
// the sequence registers, the sample time registers, the prescaler, the clock
// and reset bits of ADC1 and ADC2, configureClock() and the trigger. The registers are host
// memory (stub <wch_device.h> in stubs-adc-v30x/). Only the refusal of
// calibrate() on an ADC that is off is tested: the fake has no hardware that
// clears RSTCAL and CAL, and no conversion.

#include "check.h"
#include "Adc.h"

ADC_TypeDef hostAdc1;
ADC_TypeDef hostAdc2;
RCC_TypeDef hostRcc;
uint32_t SystemCoreClock = 144000000;

int main() {
   using C = Adc::Channel;
   Adc adc(ADC1);

   // Position 0 is in RSQR3 bits 4:0, position 5 in bits 29:25, position 6 in RSQR2,
   // position 12 in RSQR1. Neighbors stay as they are.
   adc.configureRegularChannel(0, C::Vrefint);
   adc.configureRegularChannel(1, C::PB0);
   adc.configureRegularChannel(5, C::TempSensor);
   adc.configureRegularChannel(6, C::PA3);
   adc.configureRegularChannel(12, C::PC5);
   CHECK(hostAdc1.RSQR3 == (17u | (8u << 5) | (16u << 25)));
   CHECK(hostAdc1.RSQR2 == 3u);
   CHECK(hostAdc1.RSQR1 == 15u);
   adc.configureRegularChannel(1, C::PA0);
   CHECK(hostAdc1.RSQR3 == (17u | (16u << 25)));

   // The length is L = count - 1 in bits 23:20 and keeps positions 13 to 16.
   adc.setRegularConversionCount(2);
   CHECK(hostAdc1.RSQR1 == (15u | (1u << 20)));
   adc.setRegularConversionCount(16);
   CHECK(hostAdc1.RSQR1 == (15u | (15u << 20)));

   // Channels 0 to 9 are in SAMPTR2, 10 to 17 in SAMPTR1, 3 bits each.
   using S = Adc::SamplingCycles;
   adc.setSamplingTime(C::PB1, S::Cycles239_5);
   adc.setSamplingTime(C::PA0, S::Cycles7_5);
   CHECK(hostAdc1.SAMPTR2 == ((7u << 27) | 1u));
   CHECK(hostAdc1.SAMPTR1 == 0u);
   adc.setSamplingTime(C::Vrefint, S::Cycles239_5);
   adc.setSamplingTime(C::PC0, S::Cycles71_5);
   CHECK(hostAdc1.SAMPTR1 == ((7u << 21) | 6u));
   adc.setSamplingTime(C::Vrefint, S::Cycles1_5);
   CHECK(hostAdc1.SAMPTR1 == 6u);

   // Prescaler of PCLK2 in CFGR0 bits 15:14, other bits stay.
   hostRcc.CFGR0 = 0xFFFFFFFFu;
   Adc::setClockDivider(Adc::ClockDivider::div2);
   CHECK(hostRcc.CFGR0 == ~(3u << 14));
   Adc::setClockDivider(Adc::ClockDivider::div8);
   CHECK(hostRcc.CFGR0 == 0xFFFFFFFFu);
   hostRcc.CFGR0 = 0;
   Adc::setClockDivider(Adc::ClockDivider::div6);
   CHECK(hostRcc.CFGR0 == (2u << 14));

   // ADC1 and ADC2 have their own clock and reset bit.
   // The default argument is ADC1.
   Adc::enableClock();
   CHECK(hostRcc.APB2PCENR == RCC_ADC1EN);
   Adc::enableClock(ADC2);
   CHECK(hostRcc.APB2PCENR == (RCC_ADC1EN | RCC_ADC2EN));
   Adc::disableClock(ADC1);
   CHECK(hostRcc.APB2PCENR == RCC_ADC2EN);
   Adc::reset(ADC2); // The fake records no pulse. This checks only that the bit is released.
   CHECK((hostRcc.APB2PRSTR & (RCC_ADC1RST | RCC_ADC2RST)) == 0);

   // configureClock() takes the smallest divider that keeps ADCCLK at or below 14 MHz.
   // PCLK2 is HCLK >> PPRE2: PPRE2 = /2 (100) is CFGR0 bits 13:11 = 4.
   const struct { uint32_t hclk; uint32_t cfgr0; uint32_t pre; uint32_t hz; } clocks[] = {
      { 144000000, 4ul << 11, 2ul << 14, 12000000 }, // 72 MHz: /6 is 12 MHz (/4 is 18 MHz)
      { 144000000, 0, 3ul << 14, 18000000 },         // 144 MHz: even /8 is too fast (18 MHz), so /8
      { 28000000, 0, 0, 14000000 },                  // 28 MHz: /2 is exactly 14 MHz
      { 56000000, 0, 1ul << 14, 14000000 },          // 56 MHz: /4
   };
   for(const auto &c : clocks) {
      SystemCoreClock = c.hclk;
      hostRcc.CFGR0 = c.cfgr0;
      CHECK(Adc::configureClock() == c.hz);
      CHECK((hostRcc.CFGR0 & RCC_ADCPRE) == c.pre);
      CHECK((hostRcc.CFGR0 & ~RCC_ADCPRE) == c.cfgr0);
   }

   // Power, internal channels, trigger.
   CHECK(!adc.calibrate()); // not enabled
   adc.enable();
   CHECK(hostAdc1.CTLR2 == ADC_ADON);
   adc.enable(); // no second write
   CHECK(hostAdc1.CTLR2 == ADC_ADON);
   adc.disable();
   CHECK(hostAdc1.CTLR2 == 0);
   adc.enable();
   adc.enableInternalChannels();
   CHECK(hostAdc1.CTLR2 == (ADC_ADON | ADC_TSVREFE));
   adc.setRegularConversionTrigger(Adc::RegularTrigger::SWSTART);
   CHECK(hostAdc1.CTLR2 == (ADC_ADON | ADC_TSVREFE | ADC_EXTSEL | ADC_EXTTRIG));
   adc.setRegularConversionTrigger(Adc::RegularTrigger::TIM1_CC2);
   CHECK(hostAdc1.CTLR2 == (ADC_ADON | ADC_TSVREFE | (1u << 17) | ADC_EXTTRIG));
   adc.disableInternalChannels();
   CHECK((hostAdc1.CTLR2 & ADC_TSVREFE) == 0);
   CHECK(hostAdc2.CTLR2 == 0);

   // convert(): the fake never sets EOC, so a conversion that starts ends as a timeout (false).
   Adc adc2(ADC2);
   uint16_t raw = 0;
   // (a) ADC off: no register changes.
   CHECK(!adc2.convert(Adc::PA0, raw));
   CHECK(hostAdc2.CTLR2 == 0);
   // (b) ADC on, trigger already SWSTART: only SWSTART is added, no rewrite of the same value.
   adc2.enable();
   adc2.setRegularConversionTrigger(Adc::RegularTrigger::SWSTART);
   const uint32_t before = hostAdc2.CTLR2;
   CHECK(!adc2.convert(Adc::PA0, raw));
   CHECK(hostAdc2.CTLR2 == (before | ADC_SWSTART));
   // (c) A stale EOC is cleared before the start: the wait does not end at once.
   hostAdc2.STATR = ADC_EOC;
   hostAdc2.RDATAR = 1234;
   CHECK(!adc2.convert(Adc::PA1, raw));
   CHECK((hostAdc2.STATR & ADC_EOC) == 0);
   CHECK(raw == 0);

   if(failures == 0) {
      std::printf("adc-v30x: all checks passed\n");
   }
   return failures;
}
