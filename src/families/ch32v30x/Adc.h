#pragma once

#include <cstdint>
#include <wch_device.h>

/**
 * ADC of the CH32V30x. ADC1 and ADC2 have the same registers. The internal
 * channels (temperature sensor on 16, Vrefint on 17) belong to ADC1.
 * Include <Adc.h>.
 *
 * Usage:
 *   Adc::enableClock();
 *   Adc::configureClock();   // ADCPRE so that ADCCLK <= 14 MHz
 *   Adc adc(ADC1);
 *   adc.enable();
 *   if(!adc.calibrate()) { ... }
 *   adc.setSamplingTime(Adc::PB0, Adc::SamplingCycles::Cycles239_5);
 *   uint16_t raw;
 *   if(adc.convert(Adc::PB0, raw)) { ... }
 */
class Adc {
   public:
   /// Sample time in ADC clock cycles (RM: ADCx_SAMPTR1 and SAMPTR2).
   enum class SamplingCycles {
      Cycles1_5 = 0,
      Cycles7_5,
      Cycles13_5,
      Cycles28_5,
      Cycles41_5,
      Cycles55_5,
      Cycles71_5,
      Cycles239_5
   };

   /// Input channels. Channels 16 and 17 belong to ADC1.
   enum Channel : uint8_t {
      PA0 = 0,
      PA1,
      PA2,
      PA3,
      PA4,
      PA5,
      PA6,
      PA7,
      PB0,
      PB1,
      PC0,
      PC1,
      PC2,
      PC3,
      PC4,
      PC5,
      TempSensor,
      Vrefint
   };

   /// Maximum ADC clock.
   static constexpr uint32_t maxClockHz = 14000000;

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

   /// Enable the bus clock of an ADC (APB2). The default is ADC1.
   static void enableClock(const ADC_TypeDef *adc = ADC1) {
      RCC->APB2PCENR |= (adc == ADC2) ? RCC_ADC2EN : RCC_ADC1EN;
   }

   static void disableClock(const ADC_TypeDef *adc = ADC1) {
      RCC->APB2PCENR &= ~((adc == ADC2) ? RCC_ADC2EN : RCC_ADC1EN);
   }

   /// Reset an ADC through RCC. The default is ADC1.
   static void reset(const ADC_TypeDef *adc = ADC1) {
      const uint32_t bit = (adc == ADC2) ? RCC_ADC2RST : RCC_ADC1RST;
      RCC->APB2PRSTR |= bit;
      RCC->APB2PRSTR &= ~bit;
   }

   /**
    * Set ADCPRE to the smallest divider (2, 4, 6 or 8) of PCLK2 that keeps
    * ADCCLK at or below 14 MHz. If /8 is not enough, it uses /8 anyway. No
    * conversion may run while it changes.
    *
    * @return The resulting ADCCLK in Hz.
    */
   static uint32_t configureClock();

   /// Switch the temperature sensor and Vrefint on. Calibrate first. Wait for the start-up time before you convert.
   void enableInternalChannels() const {
      adc.CTLR2 |= ADC_TSVREFE;
   }

   void disableInternalChannels() const {
      adc.CTLR2 &= ~ADC_TSVREFE;
   }

   /// Power the ADC on and wait for the start-up time. Does nothing if it is on: a write while ADON is 1 starts a conversion.
   void enable() const;

   /// Power the ADC down.
   void disable() const {
      adc.CTLR2 &= ~ADC_ADON;
   }

   /**
    * Reset and run the calibration. The ADC must be enabled and must not convert.
    * Call it before enableInternalChannels(): with TSVREFE set the input buffer
    * stays on, and the calibration offset is then wrong for the external channels.
    *
    * @return false if the ADC is not enabled or the calibration did not end in a bounded time.
    */
   bool calibrate() const;

   /**
    * One blocking, software-triggered conversion of a single channel, right aligned.
    *
    * @param channel The channel to convert.
    * @param value Receives the 12-bit result when the function returns true.
    * @return false if the conversion did not finish in a bounded time.
    */
   bool convert(Channel channel, uint16_t &value) const;

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

   inline void enableDMA() const {
      adc.CTLR2 |= ADC_DMA;
   }

   inline void setRegularConversionTrigger(RegularTrigger trigger) const {
      adc.CTLR2 = (adc.CTLR2 & ~ADC_EXTSEL) | static_cast<uint32_t>(trigger) | ADC_EXTTRIG;
   }
};
