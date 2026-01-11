#pragma once

#include <cstdint>
#include <wch_device.h>

class Timer {
   protected:
   TIM_TypeDef &tim;

   public:
   enum class MasterMode : uint8_t {
      Reset = 0,
      Enable,
      Update,
      ComparePulse,
      CompareOC1,
      CompareOC2,
      CompareOC3,
      CompareOC4
   };
   enum class Direction : uint16_t { Up = 0, Down = TIM_DIR };

   enum class OCChannel { Channel1 = 0, Channel2, Channel3, Channel4 };

   enum class Polarity : bool { High = 0, Low };

   enum class OutputCompareMode {
      Frozen = 0,
      ActiveOnMatch = TIM_OC1M_0,
      InactiveOnMatch = TIM_OC1M_1,
      Toggle = TIM_OC1M_1 | TIM_OC1M_0,
      ForceInactive = TIM_OC1M_2,
      ForceActive = TIM_OC1M_2 | TIM_OC1M_0,
      PWM1 = TIM_OC1M_2 | TIM_OC1M_1,
      PWM2 = TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1M_0,
   };

   Timer(TIM_TypeDef *timerType)
      : tim(*timerType) {}

   inline uint16_t getValue() const {
      return tim.CNT;
   }

   inline void setValue(uint16_t value) const {
      tim.CNT = value;
   }

   inline void setMasterMode(MasterMode mms) const {
      tim.CTLR2 = (tim.CTLR2 & ~TIM_MMS) | (static_cast<uint16_t>(mms) << 4);
   }

   inline void setPrescaler(uint16_t prescaleValue) const {
      tim.PSC = prescaleValue;
   }

   inline void setReloadValue(uint16_t reload) const {
      tim.ATRLR = reload;
   }

   inline uint16_t getReloadValue() const {
      return tim.ATRLR;
   }

   inline void enableAutoReloadPreload() const {
      tim.CTLR1 |= TIM_ARPE;
   }

   inline void disableAutoReloadPreload() const {
      tim.CTLR1 &= ~TIM_ARPE;
   }

   inline void enable() const {
      tim.CTLR1 |= TIM_CEN;
   }

   inline void disable() const {
      tim.CTLR1 &= ~TIM_CEN;
   }

   inline void enableUpdateInterrupt() const {
      tim.DMAINTENR |= TIM_UIE;
   }

   inline void disableUpdateInterrupt() const {
      tim.DMAINTENR &= ~TIM_UIE;
   }

   inline void enableTriggerInterrupt() const {
      tim.DMAINTENR |= TIM_TIE;
   }

   inline void disableTriggerInterrupt() const {
      tim.DMAINTENR &= ~TIM_TIE;
   }

   inline void enableCaptureCompareInterrupt(OCChannel channel) const {
      tim.DMAINTENR |= (TIM_CC1IE << static_cast<uint16_t>(channel));
   }

   inline void disableCaptureCompareInterrupt(OCChannel channel) const {
      tim.DMAINTENR &= ~(TIM_CC1IE << static_cast<uint16_t>(channel));
   }

   inline void generateUpdateEvent() const {
      tim.SWEVGR |= TIM_UG;
   }

   inline void clearUpdateInterruptFlag() const {
      tim.INTFR &= ~TIM_UIF;
   }

   inline void clearCaptureCompareInterruptFlag(OCChannel channel) const {
      tim.INTFR &= ~(TIM_CC1IF << static_cast<uint16_t>(channel));
   }

   inline void setDirection(Direction dir) const {
      tim.CTLR1 = (tim.CTLR1 & ~TIM_DIR) | static_cast<uint16_t>(dir);
   }

   inline void setCaptureCompareValue(OCChannel channel, uint16_t value) const {
      auto ptr = &(tim.CH1CVR);
      ptr[static_cast<uint8_t>(channel)] = value;
   }

   inline uint16_t getCaptureCompareValue(OCChannel channel) const {
      auto ptr = &(tim.CH1CVR);
      return ptr[static_cast<uint8_t>(channel)];
   }

   void setCaptureComparePolarity(OCChannel channel, Polarity polarity) const {
      if(polarity == Polarity::High) {
         tim.CCER &= ~(TIM_CC1P << (static_cast<uint8_t>(channel) * 4));
      } else {
         tim.CCER |= TIM_CC1P << (static_cast<uint8_t>(channel) * 4);
      }
   }

   inline void enableCaptureCompare(OCChannel channel) const {
      tim.CCER |= TIM_CC1E << (static_cast<uint8_t>(channel) * 4);
   }

   inline void disableCaptureCompare(OCChannel channel) const {
      tim.CCER &= ~(TIM_CC1E << (static_cast<uint8_t>(channel) * 4));
   }

   inline void enableOutputComparePreload(OCChannel channel) const {
      auto &chctlr = channel < OCChannel::Channel3 ? tim.CHCTLR1 : tim.CHCTLR2;
      chctlr |= TIM_OC1PE << (static_cast<uint8_t>(channel) & 0b1 * 8);
   }

   inline void disableOutputComparePreload(OCChannel channel) const {
      auto &chctlr = channel < OCChannel::Channel3 ? tim.CHCTLR1 : tim.CHCTLR2;
      chctlr &= ~(TIM_OC1PE << (static_cast<uint8_t>(channel) & 0b1 * 8));
   }

   inline void enableMainOutput() const {
      tim.BDTR |= TIM_MOE;
   }

   inline void enableAutomaticOutput() const {
      tim.BDTR |= TIM_AOE;
   }

   void setOutputCompareMode(OCChannel channel, OutputCompareMode mode) const {
      auto &chctlr = channel < OCChannel::Channel3 ? tim.CHCTLR1 : tim.CHCTLR2;
      uint8_t pos = static_cast<int>(channel) & 1 ? 8 : 0;
      chctlr = (chctlr & ~(TIM_OC1M << pos)) | (static_cast<uint32_t>(mode) << pos);
   }
};
