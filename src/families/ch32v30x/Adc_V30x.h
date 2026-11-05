#pragma once

#include <wch_device.h>

/**
 * ADC of the CH32V30x. ADC1 and ADC2 have the same registers. The internal
 * channels (temperature sensor on 16, Vrefint on 17) belong to ADC1.
 * The method names are the same as in the CH32V00x class.
 */
class Adc {
   public:
   /// Sample time in ADC clock cycles (RM: ADCx_SAMPTR1 and SAMPTR2).
   enum class SamplingCycles {
      ADCSampling1_5 = 0,
      ADCSampling7_5,
      ADCSampling13_5,
      ADCSampling28_5,
      ADCSampling41_5,
      ADCSampling55_5,
      ADCSampling71_5,
      ADCSampling239_5
   };

   enum class Channel {
      CH0_PA0 = 0,
      CH1_PA1,
      CH2_PA2,
      CH3_PA3,
      CH4_PA4,
      CH5_PA5,
      CH6_PA6,
      CH7_PA7,
      CH8_PB0,
      CH9_PB1,
      CH10_PC0,
      CH11_PC1,
      CH12_PC2,
      CH13_PC3,
      CH14_PC4,
      CH15_PC5,
      CH16_Temp,
      CH17_Vref
   };

   /// Prescaler of PCLK2. The ADC clock must not exceed 14 MHz.
   enum class ClockDivider { div2 = 0, div4, div6, div8 };

   enum class RegularTrigger {
      TIM1_CC1 = 0,
      TIM1_CC2 = 1ul << 17,
      TIM1_CC3 = 2ul << 17,
      TIM2_CC2 = 3ul << 17,
      TIM3_TRGO = 4ul << 17,
      TIM4_CC4 = 5ul << 17,
      EXTI11_TIM8_TRGO = 6ul << 17,
      SWSTART = 7ul << 17
   };

   private:
   ADC_TypeDef &adc;

   public:
   Adc(ADC_TypeDef *adc)
      : adc(*adc) {
   }

   inline static void setClockDivider(ClockDivider div) {
      RCC->CFGR0 = (RCC->CFGR0 & ~RCC_ADCPRE) | (static_cast<uint32_t>(div) << 14);
   }

   /// Enable the bus clock of this ADC (ADC1 or ADC2).
   inline void enableClock() const {
      RCC->APB2PCENR |= (&adc == ADC1) ? RCC_ADC1EN : RCC_ADC2EN;
   }

   inline void reset() const {
      const uint32_t bit = (&adc == ADC1) ? RCC_ADC1RST : RCC_ADC2RST;
      RCC->APB2PRSTR |= bit;
      RCC->APB2PRSTR &= ~bit;
   }

   /// Switch the temperature sensor and Vrefint on. Wait for their start-up time before you convert.
   inline void enableInternalChannels() const {
      adc.CTLR2 |= ADC_TSVREFE;
   }

   inline void disableInternalChannels() const {
      adc.CTLR2 &= ~ADC_TSVREFE;
   }

   /// Powers the ADC on if needed, then runs the calibration. Blocks until it ends.
   void calibrate();

   /// Put a channel at a position of the regular sequence. The position counts from 0 to 15.
   void configureRegularChannel(uint8_t sequence, Channel channel) const;

   void setSamplingTime(Channel channel, SamplingCycles cycles) const;

   inline void setRegularConversionCount(uint8_t count) const {
      adc.RSQR1 = (adc.RSQR1 & ~ADC_L) | (static_cast<uint32_t>(count - 1) << 20);
   }

   inline void enableScanConversion() const {
      adc.CTLR1 |= ADC_SCAN;
   }

   inline void disableScanConversion() const {
      adc.CTLR1 &= ~ADC_SCAN;
   }

   inline void enableContinuousConversion() const {
      adc.CTLR2 |= ADC_CONT;
   }

   inline void disableContinuousConversion() const {
      adc.CTLR2 &= ~ADC_CONT;
   }

   inline uint16_t value() const {
      return adc.RDATAR;
   }

   inline void start() const {
      adc.CTLR2 |= ADC_SWSTART;
   }

   inline void enableEndOfRegularConversionInterrupt() const {
      adc.CTLR1 |= ADC_EOCIE;
   }

   inline void disableEndOfRegularConversionInterrupt() const {
      adc.CTLR1 &= ~ADC_EOCIE;
   }

   inline void enableWatchdogInterrupt() const {
      adc.CTLR1 |= ADC_AWDIE;
   }

   inline void disableWatchdogInterrupt() const {
      adc.CTLR1 &= ~ADC_AWDIE;
   }

   inline bool isConversionComplete() const {
      return adc.STATR & ADC_EOC;
   }

   /// Power the ADC on. A write while ADON is already 1 starts a conversion, so write only on a change.
   inline void enable() const {
      if(!(adc.CTLR2 & ADC_ADON)) {
         adc.CTLR2 |= ADC_ADON;
      }
   }

   inline void enableDMA() const {
      adc.CTLR2 |= ADC_DMA;
   }

   inline void setRegularConversionTrigger(RegularTrigger trigger) const {
      adc.CTLR2 = (adc.CTLR2 & ~ADC_EXTSEL) | static_cast<uint32_t>(trigger) | ADC_EXTTRIG;
   }
};
