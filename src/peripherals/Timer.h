#pragma once

#include <cstdint>
#include <wch_device.h>

namespace hw {

/// Timer driver. See docs/timer.md.
/// The header fields are 16 bits wide (CH32V00X, CH32V20X) or 32 bits wide (CH32V30X).
/// Not for the CH32V006 TIM3: that timer has another register layout.
class Timer {
   public:
   TIM_TypeDef &tim;

   enum class Channel : uint8_t { Ch1 = 0, Ch2 = 1, Ch3 = 2, Ch4 = 3 };

   enum class Polarity : bool { High = 0, Low };

   explicit Timer(TIM_TypeDef *base)
      : tim(*base) {
   }

   Timer(const Timer &) = default;
   Timer(Timer &&) = default;
   Timer &operator=(const Timer &) = delete;
   Timer &operator=(Timer &&) = delete;

   void enableCounter() const {
      tim.CTLR1 |= TIM_CEN;
   }
   void disableCounter() const {
      tim.CTLR1 &= ~TIM_CEN;
   }
   bool isCounterEnabled() const {
      return (tim.CTLR1 & TIM_CEN) != 0;
   }
   bool isUpdateFlag() const {
      return getStatusFlag(TIM_UIF);
   }

   // The INTFR flags are write-0-to-clear. A write of the inverted mask clears one flag and
   // keeps a flag that the hardware sets during the write.
   void clearUpdateFlag() const {
      clearStatusFlag(TIM_UIF);
   }
   void clearCaptureCompare1Flag() const {
      clearStatusFlag(TIM_CC1IF);
   }
   void clearCaptureCompare2Flag() const {
      clearStatusFlag(TIM_CC2IF);
   }
   void clearCaptureCompare3Flag() const {
      clearStatusFlag(TIM_CC3IF);
   }
   void clearCaptureCompare4Flag() const {
      clearStatusFlag(TIM_CC4IF);
   }
   void clearOverflowFlag() const {
      clearStatusFlag(TIM_TIF);
   }
   void clearControlUpdateFlag() const {
      clearStatusFlag(TIM_COMIF);
   }
   void clearBreakFlag() const {
      clearStatusFlag(TIM_BIF);
   }

   void setUpdateRequestSourceOnlyOverflow(bool enable) const {
      setBit(tim.CTLR1, TIM_URS, enable);
   }

   /// The counter stops (CEN is cleared) at the next update event.
   void setOnePulseMode(bool single) const {
      setBit(tim.CTLR1, TIM_OPM, single);
   }

   void setAutoReloadPreload(bool enable) const {
      setBit(tim.CTLR1, TIM_ARPE, enable);
   }

   /// TIM6 and TIM7 of CH32V30X have no DIR bit. They count up only.
   void setDirectionDown(bool down) const {
      setBit(tim.CTLR1, TIM_DIR, down);
   }

   void generateUpdate() const {
      tim.SWEVGR = TIM_UG;
   }

   void setPrescaler(uint16_t psc) const {
      tim.PSC = psc;
   }
   uint16_t getPrescaler() const {
      return static_cast<uint16_t>(tim.PSC);
   }

   // The counters are 16 bits wide. The 32-bit TIM5 of CH32V20x_D8 and D8W gets only the low
   // 16 bits, because the CH32V20X header declares ATRLR and CNT as 16-bit fields.
   void setAutoReload(uint32_t arr) const {
      tim.ATRLR = arr;
   }
   uint32_t getAutoReload() const {
      return tim.ATRLR;
   }

   void setCounter(uint32_t cnt) const {
      tim.CNT = cnt;
   }
   uint32_t getCounter() const {
      return tim.CNT;
   }

   void enableUpdateDma() const {
      tim.DMAINTENR |= TIM_UDE;
   }
   void disableUpdateDma() const {
      tim.DMAINTENR &= ~TIM_UDE;
   }

   void enableCcDma(Channel channel) const {
      tim.DMAINTENR |= (TIM_CC1DE << static_cast<uint8_t>(channel));
   }
   void disableCcDma(Channel channel) const {
      tim.DMAINTENR &= ~(TIM_CC1DE << static_cast<uint8_t>(channel));
   }

   void enableUpdateInterrupt() const {
      tim.DMAINTENR |= TIM_UIE;
   }
   void disableUpdateInterrupt() const {
      tim.DMAINTENR &= ~TIM_UIE;
   }
   bool isUpdateInterruptEnabled() const {
      return (tim.DMAINTENR & TIM_UIE) != 0;
   }

   void enableTriggerInterrupt() const {
      tim.DMAINTENR |= TIM_TIE;
   }
   void disableTriggerInterrupt() const {
      tim.DMAINTENR &= ~TIM_TIE;
   }

   void enableCcInterrupt(Channel channel) const {
      tim.DMAINTENR |= (TIM_CC1IE << static_cast<uint8_t>(channel));
   }
   void disableCcInterrupt(Channel channel) const {
      tim.DMAINTENR &= ~(TIM_CC1IE << static_cast<uint8_t>(channel));
   }

   bool getStatusFlag(uint32_t flag) const {
      return (tim.INTFR & flag) == flag;
   }
   void clearStatusFlag(uint32_t flag) const {
      tim.INTFR = static_cast<uint16_t>(~flag);
   }

   void clearAllStatusFlags() const {
      tim.INTFR = 0;
   }

   enum class OcMode : uint32_t {
      Frozen = 0,
      ActiveOnMatch = TIM_OC1M_0,
      InactiveOnMatch = TIM_OC1M_1,
      Toggle = TIM_OC1M_0 | TIM_OC1M_1,
      ForceInactive = TIM_OC1M_2,
      ForceActive = TIM_OC1M_2 | TIM_OC1M_0,
      Pwm1 = TIM_OC1M_2 | TIM_OC1M_1,
      Pwm2 = TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1M_0,
   };

   /// Channels 1 and 2 are in CHCTLR1, channels 3 and 4 in CHCTLR2. The even channel is 8 bits higher.
   void setOcMode(Channel channel, OcMode mode) const {
      auto &chctlr = chctlrOf(channel);
      const uint32_t shift = ocShift(channel);
      chctlr = (chctlr & ~(TIM_OC1M << shift)) | (static_cast<uint32_t>(mode) << shift);
   }

   void enableOcPreload(Channel channel) const {
      chctlrOf(channel) |= (TIM_OC1PE << ocShift(channel));
   }

   void disableOcPreload(Channel channel) const {
      chctlrOf(channel) &= ~(TIM_OC1PE << ocShift(channel));
   }

   /// `trgo` is the MMS field value at its bit position (bits 6:4 of CTLR2).
   void setTriggerOutput(uint32_t trgo) const {
      tim.CTLR2 = (tim.CTLR2 & ~TIM_MMS) | trgo;
   }

   void disableMasterSlaveMode() const {
      tim.SMCFGR &= ~TIM_MSM;
   }

   void enableChannel(Channel channel) const {
      tim.CCER |= (TIM_CC1E << (static_cast<uint8_t>(channel) * 4));
   }

   void disableChannel(Channel channel) const {
      tim.CCER &= ~(TIM_CC1E << (static_cast<uint8_t>(channel) * 4));
   }

   /// Low: the output is high when OCxREF is low.
   void setCaptureComparePolarity(Channel channel, Polarity polarity) const {
      setBit(tim.CCER, TIM_CC1P << (static_cast<uint8_t>(channel) * 4), polarity == Polarity::Low);
   }

   /// CH1CVR to CH4CVR are 4 bytes apart in every family. The CH32V20X header declares them
   /// as 16-bit fields, so the index uses a 32-bit pointer.
   void setCompare(Channel channel, uint32_t value) const {
      compareRegisters()[static_cast<uint8_t>(channel)] = value;
   }

   uint32_t getCompare(Channel channel) const {
      return compareRegisters()[static_cast<uint8_t>(channel)];
   }

   /// Advanced timers only (TIM1; on CH32V20X and CH32V30X also TIM8, TIM9 and TIM10).
   /// Without MOE an advanced timer drives no output pin.
   void enableMainOutput() const {
      tim.BDTR |= TIM_MOE;
   }
   void disableMainOutput() const {
      tim.BDTR &= ~TIM_MOE;
   }

   /// Advanced timers only. The next update event sets MOE again.
   void enableAutomaticOutput() const {
      tim.BDTR |= TIM_AOE;
   }

   private:
   template <typename Reg>
   static void setBit(Reg &reg, uint32_t bit, bool set) {
      if(set) {
         reg |= bit;
      } else {
         reg &= ~bit;
      }
   }

   // A 32-bit pointer: the CH32V20X header has CH1CVR_R32 for a 32-bit access.
   volatile uint32_t *compareRegisters() const {
      return reinterpret_cast<volatile uint32_t *>(&tim.CH1CVR);
   }

   decltype(TIM_TypeDef::CHCTLR1) &chctlrOf(Channel channel) const {
      return channel < Channel::Ch3 ? tim.CHCTLR1 : tim.CHCTLR2;
   }

   static uint32_t ocShift(Channel channel) {
      return (static_cast<uint8_t>(channel) & 1u) ? 8u : 0u;
   }
};

} // namespace hw
