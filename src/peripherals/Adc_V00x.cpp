#if defined(CH32V006) || defined(CH32V002)
#include "Adc_V00x.h"

void Adc::configureRegularChannel(uint8_t sequence, Channel channel) const {
   uint32_t regnum = 2 - (sequence / 6);
   uint32_t bitIndex = sequence % 6 * 5;
   auto SQR = &(adc.RSQR1);
   uint32_t tmp = SQR[regnum];
   tmp &= ~(0x1F << bitIndex);
   tmp |= static_cast<uint32_t>(channel) << bitIndex;
   SQR[regnum] = tmp;
}

void Adc::setSamplingTime(Channel channel, SamplingCycles cycles) const {
   uint32_t bitIndex = static_cast<uint32_t>(channel) % 10 * 3;
   adc.SAMPTR2 = ((adc.SAMPTR2 & ~(0x7 << bitIndex)) | static_cast<uint32_t>(cycles) << bitIndex);
}

void Adc::calibrate() {
   /* Ensure that the ADC is on */
   if((adc.CTLR2 & ADC_ADON) == 0) {
      adc.CTLR2 |= ADC_ADON;
   }
   // CH32V006 doesn't have manual calibration bits in CTLR2.
   // It might be automatic or handled via different registers.

}

#endif
