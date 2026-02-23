
#pragma once

#include <wch_device.h>

class I2c {
   I2C_TypeDef &i2c;

   public:
   enum class MasterMode { Standard = 0, Fast = I2C_CKCFGR_FS };

   enum class FastModeDutyCycle { Duty_2_1 = 0, Duty_16_9 = I2C_CKCFGR_DUTY };

   enum class AddressingMode { addressingMode7Bit = 0, addressingMode10Bit = I2C_STAR1_ADD10 };

   I2c(I2C_TypeDef *i2c)
      : i2c(*i2c) {}
   void setMode(MasterMode mode) const {
      i2c.CKCFGR = (i2c.CKCFGR & ~I2C_CKCFGR_FS) | static_cast<uint16_t>(mode);
   }

   void setData(uint8_t data) const {
      i2c.DATAR = data;
   }

   inline uint8_t getData() const {
      return i2c.DATAR;
   }

   inline void setAddress(uint16_t address) const {
      i2c.OADDR1 = (address & 0x7F) << 1;
      if(getAddressingMode() == AddressingMode::addressingMode10Bit) {
         i2c.OADDR2 = (address & 0x310) >> 6;
      }
   }

   inline void enable() const {
      i2c.CTLR1 |= I2C_CTLR1_PE;
   }

   inline bool isEnabled() const {
      return (i2c.CTLR1 & I2C_CTLR1_PE);
   }

   inline void disable() const {
      i2c.CTLR1 &= ~I2C_CTLR1_PE;
   }

   inline void setAddressingMode(AddressingMode mode) const {
      i2c.STAR1 = (i2c.STAR1 & ~I2C_STAR1_ADD10) | static_cast<uint16_t>(mode);
   }

   inline AddressingMode getAddressingMode() const {
      return (AddressingMode)(i2c.STAR1 & I2C_STAR1_ADD10);
   }

   inline void enableDualAddressing() const {
      i2c.OADDR2 |= I2C_OADDR2_ENDUAL;
   }

   inline void disableDualAddress() const {
      i2c.OADDR2 &= ~I2C_OADDR2_ENDUAL;
   }

   inline void enableBroadcastAddressing() const {
      i2c.CTLR1 |= I2C_CTLR1_ENGC;
   }

   inline void disableBroadcastAddressing() const {
      i2c.CTLR1 &= ~I2C_CTLR1_ENGC;
   }

   inline void stopCondition() const {
      // Generate the STOP condition
      i2c.CTLR1 |= I2C_CTLR1_STOP;
   }

   inline void startCondition() const {
      // Generate the START condition
      i2c.CTLR1 |= I2C_CTLR1_START;
   }

   inline void enableAck() const {
      i2c.CTLR1 |= I2C_CTLR1_ACK;
   }

   inline void disableAck() const {
      i2c.CTLR1 &= ~I2C_CTLR1_ACK;
   }

   inline void enableErrorInterrupt() const {
      i2c.CTLR2 |= I2C_CTLR2_ITERREN;
   }

   inline void enableEventInterrupt() const {
      i2c.CTLR2 |= I2C_CTLR2_ITEVTEN;
   }

   inline void enableBufferInterrupt() const {
      i2c.CTLR2 |= I2C_CTLR2_ITBUFEN;
   }

   inline void disableErrorInterrupt() const {
      i2c.CTLR2 &= ~I2C_CTLR2_ITERREN;
   }

   inline void disableEventInterrupt() const {
      i2c.CTLR2 &= ~I2C_CTLR2_ITEVTEN;
   }

   inline void disableBufferInterrupt() const {
      i2c.CTLR2 &= ~I2C_CTLR2_ITBUFEN;
   }

   I2C_TypeDef &getI2c() const {
      return i2c;
   }

   inline void setPeripheralFrequency(uint8_t frequencyInMHz) const {
      i2c.CTLR2 = (i2c.CTLR2 & ~I2C_CTLR2_FREQ) | (frequencyInMHz & I2C_CTLR2_FREQ);
   }

   void configure(uint32_t busSpeedHz, MasterMode mode = MasterMode::Standard, FastModeDutyCycle dutyCycle = FastModeDutyCycle::Duty_2_1) const;

   bool startSent() const {
      return i2c.STAR1 & I2C_STAR1_SB;
   }

   bool addressMatched() const {
      return i2c.STAR1 & I2C_STAR1_ADDR;
   }

   bool byteTransferFinished() const {
      return i2c.STAR1 & I2C_STAR1_BTF;
   }

   bool stopDetected() const {
      return i2c.STAR1 & I2C_STAR1_STOPF;
   }

   void clearStopFlag() const {
      (void)i2c.STAR1;
      i2c.CTLR1 |= I2C_CTLR1_PE;
   }

   bool rxNotEmpty() const {
      return i2c.STAR1 & I2C_STAR1_RXNE;
   }

   bool txEmpty() const {
      return i2c.STAR1 & I2C_STAR1_TXE;
   }

   bool busError() const {
      return i2c.STAR1 & I2C_STAR1_BERR;
   }

   bool arbitrationLost() const {
      return i2c.STAR1 & I2C_STAR1_ARLO;
   }

   bool acknowledgeFailure() const {
      return i2c.STAR1 & I2C_STAR1_AF;
   }

   void clearErrorFlags() const {
      i2c.STAR1 &= ~(I2C_STAR1_BERR | I2C_STAR1_ARLO | I2C_STAR1_AF);
   }

   bool isTransmitter() const {
      return i2c.STAR2 & I2C_STAR2_TRA;
   }

   uint16_t getStatus1() const {
      return i2c.STAR1;
   }

   inline uint16_t getStatus2() const {
      return i2c.STAR2;
   }

   inline void setRiseTime(uint16_t trise) const {
#if defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6)
      i2c.RTR = trise;
#else
      *(__IO uint16_t*)((uintptr_t)&i2c + 0x20) = trise;
#endif
   }
};
