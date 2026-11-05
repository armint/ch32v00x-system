// Host tests for the CH32V30x ADC driver (src/families/ch32v30x/Adc_V30x):
// the sequence registers, the sample time registers, the prescaler, the clock
// and reset bits of ADC1 and ADC2, and the trigger. The registers are host
// memory (stub <wch_device.h> in stubs-adc-v30x/). calibrate() is not tested:
// the fake has no hardware that clears RSTCAL and CAL.

#include "check.h"
#include "families/ch32v30x/Adc_V30x.h"

ADC_TypeDef hostAdc1;
ADC_TypeDef hostAdc2;
RCC_TypeDef hostRcc;

int main() {
   using C = Adc::Channel;
   Adc adc(ADC1);

   // Position 0 is in RSQR3 bits 4:0, position 5 in bits 29:25, position 6 in RSQR2,
   // position 12 in RSQR1. Neighbors stay as they are.
   adc.configureRegularChannel(0, C::CH17_Vref);
   adc.configureRegularChannel(1, C::CH8_PB0);
   adc.configureRegularChannel(5, C::CH16_Temp);
   adc.configureRegularChannel(6, C::CH3_PA3);
   adc.configureRegularChannel(12, C::CH15_PC5);
   CHECK(hostAdc1.RSQR3 == (17u | (8u << 5) | (16u << 25)));
   CHECK(hostAdc1.RSQR2 == 3u);
   CHECK(hostAdc1.RSQR1 == 15u);
   adc.configureRegularChannel(1, C::CH0_PA0);
   CHECK(hostAdc1.RSQR3 == (17u | (16u << 25)));

   // The length is L = count - 1 in bits 23:20 and keeps positions 13 to 16.
   adc.setRegularConversionCount(2);
   CHECK(hostAdc1.RSQR1 == (15u | (1u << 20)));
   adc.setRegularConversionCount(16);
   CHECK(hostAdc1.RSQR1 == (15u | (15u << 20)));

   // Channels 0 to 9 are in SAMPTR2, 10 to 17 in SAMPTR1, 3 bits each.
   using S = Adc::SamplingCycles;
   adc.setSamplingTime(C::CH9_PB1, S::ADCSampling239_5);
   adc.setSamplingTime(C::CH0_PA0, S::ADCSampling7_5);
   CHECK(hostAdc1.SAMPTR2 == ((7u << 27) | 1u));
   CHECK(hostAdc1.SAMPTR1 == 0u);
   adc.setSamplingTime(C::CH17_Vref, S::ADCSampling239_5);
   adc.setSamplingTime(C::CH10_PC0, S::ADCSampling71_5);
   CHECK(hostAdc1.SAMPTR1 == ((7u << 21) | 6u));
   adc.setSamplingTime(C::CH17_Vref, S::ADCSampling1_5);
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
   Adc adc2(ADC2);
   adc.enableClock();
   CHECK(hostRcc.APB2PCENR == RCC_ADC1EN);
   adc2.enableClock();
   CHECK(hostRcc.APB2PCENR == (RCC_ADC1EN | RCC_ADC2EN));
   adc2.reset();
   CHECK((hostRcc.APB2PRSTR & (RCC_ADC1RST | RCC_ADC2RST)) == 0);

   // Power, internal channels, trigger.
   adc.enable();
   CHECK(hostAdc1.CTLR2 == ADC_ADON);
   adc.enableInternalChannels();
   CHECK(hostAdc1.CTLR2 == (ADC_ADON | ADC_TSVREFE));
   adc.setRegularConversionTrigger(Adc::RegularTrigger::SWSTART);
   CHECK(hostAdc1.CTLR2 == (ADC_ADON | ADC_TSVREFE | ADC_EXTSEL | ADC_EXTTRIG));
   adc.setRegularConversionTrigger(Adc::RegularTrigger::TIM1_CC2);
   CHECK(hostAdc1.CTLR2 == (ADC_ADON | ADC_TSVREFE | (1u << 17) | ADC_EXTTRIG));
   adc.disableInternalChannels();
   CHECK((hostAdc1.CTLR2 & ADC_TSVREFE) == 0);
   CHECK(hostAdc2.CTLR2 == 0);

   if(failures == 0) {
      std::printf("adc-v30x: all checks passed\n");
   }
   return failures;
}
