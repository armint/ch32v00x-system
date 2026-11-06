#pragma once

#include <wch_device.h>

namespace hw {

// Same names as hw::Usart in system-stm32 (enums, setFrameFormat(), getClockFrequency(), returnStatusFlag()).
// Not all of its functions exist here: the half-duplex, CTS/RTS, idle-line and error-interrupt functions are
// missing, because no code of this project uses them.
class Usart {
   public:
   enum class StopBits {
      StopBits_1 = 0,
      StopBits_0_5 = USART_CTLR2_STOP_0,
      StopBits_2 = USART_CTLR2_STOP_1,
      StopBits_1_5 = USART_CTLR2_STOP_0 | USART_CTLR2_STOP_1,
   };

   enum class Parity {
      None = 0,
      Even = USART_CTLR1_PCE,
      Odd = (USART_CTLR1_PCE | USART_CTLR1_PS),
   };

   // Data bits without the parity bit
   enum class DataBits {
      Seven = 7,
      Eight = 8,
   };

   USART_TypeDef &usart;

   Usart(USART_TypeDef *uartBase)
      : usart(*uartBase) {
   }

   inline void enableTx() const {
      usart.CTLR1 |= USART_CTLR1_TE;
   }

   inline void disableTx() const {
      usart.CTLR1 &= ~USART_CTLR1_TE;
   }

   inline void enableRx() const {
      usart.CTLR1 |= USART_CTLR1_RE;
   }

   inline void disableRx() const {
      usart.CTLR1 &= ~USART_CTLR1_RE;
   }

   inline void enableDmaTx() const {
      usart.CTLR3 |= USART_CTLR3_DMAT;
   }

   inline void disableDmaTx() const {
      usart.CTLR3 &= ~USART_CTLR3_DMAT;
   }

   inline void enableDmaRx() const {
      usart.CTLR3 |= USART_CTLR3_DMAR;
   }

   inline void disableDmaRx() const {
      usart.CTLR3 &= ~USART_CTLR3_DMAR;
   }

   inline uint8_t getData() const {
      return usart.DATAR;
   }

   inline void setData(uint8_t data) const {
      usart.DATAR = data;
   }

   inline bool isRxRegisterNotEmpty() const {
      return (usart.STATR & USART_STATR_RXNE) != 0;
   }

   inline bool isTxRegisterEmpty() const {
      return (usart.STATR & USART_STATR_TXE) != 0;
   }

   inline bool isTransferComplete() const {
      return (usart.STATR & USART_STATR_TC) != 0;
   }

   inline void disable() const {
      usart.CTLR1 &= ~USART_CTLR1_UE;
   }
   inline void enable() const {
      usart.CTLR1 |= USART_CTLR1_UE;
   }

   inline bool isEnabled() const {
      return (usart.CTLR1 & USART_CTLR1_UE) != 0;
   }

   inline void setParity(Parity parity) const {
      usart.CTLR1 = (usart.CTLR1 & ~(USART_CTLR1_PS | USART_CTLR1_PCE)) | static_cast<uint32_t>(parity);
   }

   inline void setStopBits(StopBits bits) const {
      usart.CTLR2 = (usart.CTLR2 & ~USART_CTLR2_STOP) | static_cast<uint32_t>(bits);
   }

   inline void enableTxCompleteInterrupt() const {
      usart.CTLR1 |= USART_CTLR1_TCIE;
   }

   inline void disableTxCompleteInterrupt() const {
      usart.CTLR1 &= ~USART_CTLR1_TCIE;
   }

   inline bool isTxCompleteInterruptEnabled() const {
      return (usart.CTLR1 & USART_CTLR1_TCIE) != 0;
   }

   inline void enableTxEmptyInterrupt() const {
      usart.CTLR1 |= USART_CTLR1_TXEIE;
   }

   inline void disableTxEmptyInterrupt() const {
      usart.CTLR1 &= ~USART_CTLR1_TXEIE;
   }

   inline bool isTxEmptyInterruptEnabled() const {
      return (usart.CTLR1 & USART_CTLR1_TXEIE) != 0;
   }

   inline void enableRxNotEmptyInterrupt() const {
      usart.CTLR1 |= USART_CTLR1_RXNEIE;
   }

   inline void disableRxNotEmptyInterrupt() const {
      usart.CTLR1 &= ~USART_CTLR1_RXNEIE;
   }

   inline bool isRxNotEmptyInterruptEnabled() const {
      return (usart.CTLR1 & USART_CTLR1_RXNEIE) != 0;
   }

   // The clock of the bus of this USART: Rcc::getAPB2Clock() for USART1, Rcc::getAPBClock() for the others.
   // The baud rate is derived from it.
   uint32_t getClockFrequency() const;

   // Data bits without the parity bit, parity and stop bits. Seven data bits need a parity bit: the hardware
   // has 8 or 9 bits in a word. Returns false then, and changes nothing. The USART is off while the frame
   // changes, and on again afterward if it was on.
   [[nodiscard]] bool setFrameFormat(DataBits dataBits, Parity parity, StopBits stopBits) const;

   void setBaudRate(uint32_t baudRate) const;

   uint32_t getBaudRate() const;

   inline void setBRR(uint32_t BRR) const {
      usart.BRR = BRR;
   }

   inline volatile USART_TypeDef *getUsart() const {
      return &usart;
   }

   inline bool getStatusFlag(uint32_t flag) const {
      return (usart.STATR & flag) != 0;
   }

   inline uint32_t getStatusFlags() const {
      return usart.STATR;
   }

   // The whole status register, as returnStatusFlag() of system-stm32.
   inline uint32_t returnStatusFlag() const {
      return usart.STATR;
   }

   inline void clearStatusFlag(uint32_t flag) const {
      // On CH32V003, some bits are cleared by writing 0, some by software sequence.
      // TC bit can be cleared by writing 0.
      usart.STATR = static_cast<uint16_t>(~flag);
   }


   inline void sendBreak() const {
      usart.CTLR1 |= USART_CTLR1_SBK;
   }

   void configure(uint32_t baudRate = 115200, Parity parity = Parity::None, StopBits stopBits = StopBits::StopBits_1) const {
      setBaudRate(baudRate);
      setStopBits(stopBits);
      setParity(parity);
   }

   static constexpr uint32_t calculateBRR(uint32_t baudRate, uint32_t HCLK) {
      // For CH32V003: USARTDIV = HCLK / (16 * baudrate)
      // BRR[15:4] = Mantissa
      // BRR[3:0] = Fraction * 16

      // A common way to calculate this with rounding:
      // USARTDIV_fixed = (HCLK * 2 + baudRate) / (baudRate * 2) // for 8x oversampling
      // But CH32V003 typically uses 16x oversampling by default.

      // Integer arithmetic for (HCLK + baudRate/2) / baudRate
      uint32_t divider = (HCLK + (baudRate / 2)) / baudRate;
      return divider;
   }
};

} // namespace hw
