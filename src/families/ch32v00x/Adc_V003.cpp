#if defined(CH32V003)
#include "Adc_V003.h"

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
   uint32_t regnum = 1 - static_cast<uint32_t>(channel) / 10;
   uint32_t bitIndex = static_cast<uint32_t>(channel) % 10 * 3;
   volatile uint32_t *SMPR = &(adc.SAMPTR1);
   SMPR[regnum] = ((SMPR[regnum] & ~(0x7 << bitIndex)) | static_cast<uint32_t>(cycles) << bitIndex);
}

void Adc::calibrate() {
   /* Ensure that the ADC is on */
   if((adc.CTLR2 & ADC_ADON) == 0) {
      adc.CTLR2 |= ADC_ADON;
   }
   /* Reset the calibration */
   adc.CTLR2 |= ADC_RSTCAL;
   while(adc.CTLR2 & ADC_RSTCAL) {
   }
   /* Start calibration */
   adc.CTLR2 |= ADC_CAL;
   /* Wait until calibration finished */
   while((adc.CTLR2 & ADC_CAL) != 0) {
   }
}

#endif
