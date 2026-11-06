#if defined(CH32V30x_D8C)
#include "Adc.h"

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

void Adc::calibrate() {
   enable();
   // The RM asks for at least 2 ADC clock cycles with ADON set before the calibration.
   // One ADC cycle is PPRE2 (max 16) x ADCPRE (max 8) core cycles, so 2 cycles are at most
   // 2 x 16 x 8 = 256 core cycles. Each loop pass takes at least 2 core cycles.
   for(int i = 0; i < 256; i++) {
      __asm volatile("");
   }
   adc.CTLR2 |= ADC_RSTCAL;
   while(adc.CTLR2 & ADC_RSTCAL) {
   }
   adc.CTLR2 |= ADC_CAL;
   while(adc.CTLR2 & ADC_CAL) {
   }
}

#endif
