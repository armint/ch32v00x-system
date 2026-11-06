#if defined(CH32V30x_D8C)
#include "Adc.h"
#include <peripherals/Rcc.h>

void Adc::configureRegularChannel(uint8_t sequence, Channel channel) const {
   // RSQR3 holds positions 0 to 5, RSQR2 positions 6 to 11, RSQR1 positions 12 to 15.
   uint32_t regnum = 2 - (sequence / 6);
   uint32_t bitIndex = sequence % 6 * 5;
   auto SQR = &(adc.RSQR1);
   uint32_t tmp = SQR[regnum];
   tmp &= ~(0x1Ful << bitIndex);
   tmp |= static_cast<uint32_t>(channel) << bitIndex;
   SQR[regnum] = tmp;
}

void Adc::setSamplingTime(Channel channel, SamplingCycles cycles) const {
   const uint32_t ch = static_cast<uint32_t>(channel);
   // SAMPTR2 has channels 0 to 9, SAMPTR1 has channels 10 to 17. 3 bits each.
   volatile uint32_t &reg = (ch < 10) ? adc.SAMPTR2 : adc.SAMPTR1;
   const uint32_t bitIndex = (ch % 10) * 3;
   reg = (reg & ~(0x7ul << bitIndex)) | (static_cast<uint32_t>(cycles) << bitIndex);
}

// Upper bound of a busy wait, in loop passes. The longest calibration is 83 ADC clocks of
// 128 core cycles each (about 11k core cycles). A pass takes at least 3 core cycles.
constexpr uint32_t waitPasses = 100000;

uint32_t Adc::configureClock() {
   const uint32_t pclk2 = Rcc::getAPB2Clock();
   // ClockDivider div2 to div8 are the values 0 to 3 and divide by 2 x (value + 1).
   uint32_t field = 0;
   while(field < 3 && pclk2 / (2 * (field + 1)) > maxClockHz) {
      ++field;
   }
   setClockDivider(static_cast<ClockDivider>(field));
   return pclk2 / (2 * (field + 1));
}

void Adc::enable() const {
   if(adc.CTLR2 & ADC_ADON) {
      return;
   }
   adc.CTLR2 |= ADC_ADON;
   // The RM asks for at least 2 ADC clock cycles with ADON set before the calibration.
   // One ADC cycle is PPRE2 (max 16) x ADCPRE (max 8) core cycles, so 2 cycles are at most
   // 2 x 16 x 8 = 256 core cycles. Each loop pass takes at least 2 core cycles.
   for(int i = 0; i < 256; i++) {
      __asm volatile("");
   }
}

bool Adc::calibrate() const {
   if(!(adc.CTLR2 & ADC_ADON)) {
      return false;
   }
   adc.CTLR2 |= ADC_RSTCAL;
   for(uint32_t i = 0; adc.CTLR2 & ADC_RSTCAL; ++i) {
      if(i == waitPasses) {
         return false;
      }
   }
   adc.CTLR2 |= ADC_CAL;
   for(uint32_t i = 0; adc.CTLR2 & ADC_CAL; ++i) {
      if(i == waitPasses) {
         return false;
      }
   }
   return true;
}

bool Adc::convert(Channel channel, uint16_t &value) const {
   if(!(adc.CTLR2 & ADC_ADON)) {
      return false;
   }
   configureRegularChannel(0, channel);
   setRegularConversionCount(1);
   // A CTLR2 write that changes no bit while ADON is 1 starts a conversion (RM). Thus write the
   // trigger only on a change, and let start() make the only conversion.
   constexpr uint32_t software = static_cast<uint32_t>(RegularTrigger::SWSTART) | ADC_EXTTRIG;
   if((adc.CTLR2 & (ADC_EXTSEL | ADC_EXTTRIG)) != software) {
      setRegularConversionTrigger(RegularTrigger::SWSTART);
   }
   // A stale end-of-conversion flag (an earlier timeout) would end the wait at once. EOC is rc_w0.
   adc.STATR = ~static_cast<uint32_t>(ADC_EOC);
   start();
   // 239.5 + 12.5 cycles at 9 MHz are 28 us. The wait is far longer, and ends in a few ms at 144 MHz.
   for(uint32_t i = 0; i < waitPasses; ++i) {
      if(isConversionComplete()) {
         value = this->value();
         return true;
      }
   }
   return false;
}

#endif
