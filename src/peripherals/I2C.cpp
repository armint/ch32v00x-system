
#include "I2C.h"
#include <wch_device.h>

void I2c::configure(uint32_t busSpeedHz, MasterMode mode, FastModeDutyCycle dutyCycle) const {
   uint32_t pclk1 = SystemCoreClock;
   uint32_t freqMHz = pclk1 / 1000000;

   // Set CTLR2 FREQ[5:0]
   setPeripheralFrequency(static_cast<uint8_t>(freqMHz));

   uint16_t ckcfgr = static_cast<uint16_t>(mode);
   uint16_t trise;

   if(mode == MasterMode::Standard) {
      // Standard mode: CCR = pclk1 / (2 * busSpeedHz)
      uint32_t ccr = pclk1 / (2 * busSpeedHz);
      if(ccr < 4)
         ccr = 4;
      ckcfgr |= (ccr & I2C_CKCFGR_CCR);

      trise = freqMHz + 1;
   } else {
      // Fast mode
      ckcfgr |= static_cast<uint16_t>(dutyCycle);
      uint32_t ccr;
      if(dutyCycle == FastModeDutyCycle::Duty_2_1) {
         // CCR = pclk1 / (3 * busSpeedHz)
         ccr = pclk1 / (3 * busSpeedHz);
      } else {
         // CCR = pclk1 / (25 * busSpeedHz)
         ccr = pclk1 / (25 * busSpeedHz);
      }
      if(ccr < 1)
         ccr = 1;
      ckcfgr |= (ccr & I2C_CKCFGR_CCR);

      trise = (freqMHz * 300 / 1000) + 1;
   }

   i2c.CKCFGR = ckcfgr;
   setRiseTime(trise);
}
