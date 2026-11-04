#pragma once

#include <wch_device.h>

class Adc {
   public:
   enum class SamplingCycles {
      ADCSampling3 = 0,
      ADCSampling9,
      ADCSampling15,
      ADCSampling30,
      ADCSampling43,
      ADCSampling57,
      ADCSampling73,
      ADCSampling241
   };

   enum class Channel {
      CH0_PA2 = 0,
      CH1_PA1,
      CH2_PC4,
      CH3_PD2,
      CH4_PD3,
      CH5_PD5,
      CH6_PD6,
      CH7_PD4,
      CH8_Vref,
      CH9_Vcal,
   };

   enum class ClockDivider {
      div2 = 0b00000,
      div4 = 0b01000,
      div6 = 0b10000,
      div8 = 0b11000,
      div12 = 0b10100,
      div16 = 0b11100,
      div24 = 0b10101,
      div32 = 0b11101,
      div48 = 0b10110,
      div64 = 0b11110,
      div96 = 0b10111,
      div128 = 0b11111,
   };

   enum class RegularTrigger {
      TIM1_TRGO = 0,
      TIM1_CC1 = ADC_EXTSEL_0,
      TIM1_CC2 = ADC_EXTSEL_1,
      TIM2_TRGO = ADC_EXTSEL_1 | ADC_EXTSEL_0,
      TIM2_CC1 = ADC_EXTSEL_2,
      TIM2_CC2 = ADC_EXTSEL_2 | ADC_EXTSEL_0,
      PD3_PC2 = ADC_EXTSEL_2 | ADC_EXTSEL_1,
      SWSTART = ADC_EXTSEL_2 | ADC_EXTSEL_1 | ADC_EXTSEL_0

   };

   enum class InjectedTrigger {
      TIM1_CC3 = 0,
      TIM1_CC4 = ADC_JEXTSEL_0,
      TIM2_CC3 = ADC_JEXTSEL_1,
      TIM2_CC4 = ADC_JEXTSEL_0 | ADC_JEXTSEL_1,
      PD1_PA2 = ADC_JEXTSEL_2 | ADC_JEXTSEL_1,
      JSWSTART = ADC_JEXTSEL_2 | ADC_JEXTSEL_1 | ADC_JEXTSEL_0
   };

   enum class CalibrationVoltage { Half = 1 << 25, ThreeQuarters = 2 << 25 };

   private:
   ADC_TypeDef &adc;

   public:
   Adc(ADC_TypeDef *adc)
      : adc(*adc) {
   }

   inline static void setClockDivider(ClockDivider div) {
      RCC->CFGR0 = (RCC->CFGR0 & ~RCC_ADCPRE) | (static_cast<uint32_t>(div) << 11);
   }

   inline void reset() const {
      RCC->APB2PRSTR |= RCC_ADC1RST;
      RCC->APB2PRSTR &= ~RCC_ADC1RST;
   }

   void calibrate();

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

   inline void setCalibrationVoltage(CalibrationVoltage v) {
      adc.CTLR1 = (adc.CTLR1 & ~(3ul << 25)) | static_cast<uint32_t>(v);
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

   inline void enable() const {
      adc.CTLR2 |= ADC_ADON;
   }

   inline void enableDMA() const {
      adc.CTLR2 |= ADC_DMA;
   }

   inline void setRegularConversionTrigger(RegularTrigger trigger) const {
      adc.CTLR2 = (adc.CTLR2 & ~ADC_EXTSEL) | static_cast<uint32_t>(trigger) | ADC_EXTTRIG;
   }
};
