#pragma once

#include <wch_device.h>

enum class UsartStopBits {
   StopBits_1 = 0,
   StopBits_0_5 = USART_CTLR2_STOP_0,
   StopBits_2 = USART_CTLR2_STOP_1,
   StopBits_1_5 = USART_CTLR2_STOP_0 | USART_CTLR2_STOP_1,

};

enum class USARTParity {
   None = 0,
   Even = USART_CTLR1_PCE,
   Odd = (USART_CTLR1_PCE | USART_CTLR1_PS),
};
class Usart {
   public:
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

   inline void enableTxDma() const {
      usart.CTLR3 |= USART_CTLR3_DMAT;
   }

   inline void disableTxDma() const {
      usart.CTLR3 &= ~USART_CTLR3_DMAT;
   }

   inline void enableRxDma() const {
      usart.CTLR3 |= USART_CTLR3_DMAR;
   }

   inline void disableRxDma() const {
      usart.CTLR3 &= ~USART_CTLR3_DMAR;
   }

   inline uint8_t getData() const {
      return usart.DATAR;
   }

   inline void setData(uint8_t data) const {
      usart.DATAR = data;
   }

   inline bool isRxNotEmpty() const {
      return (usart.STATR & USART_STATR_RXNE) != 0;
   }

   inline bool isTxEmpty() const {
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

   inline void setParity(USARTParity parity) const {
      usart.CTLR1 = (usart.CTLR1 & ~(USART_CTLR1_PS | USART_CTLR1_PCE)) | static_cast<uint32_t>(parity);
   }

   inline void setStopBits(UsartStopBits bits) const {
      usart.CTLR2 = (usart.CTLR2 & ~USART_CTLR2_STOP) | static_cast<uint32_t>(bits);
   }

   inline void enableTransferCompleteInterrupt() const {
      usart.CTLR1 |= USART_CTLR1_TCIE;
   }

   inline void disableTransferCompleteInterrupt() const {
      usart.CTLR1 &= ~USART_CTLR1_TCIE;
   }

   inline bool isTransferCompleteInterruptEnabled() const {
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

   inline void clearStatusFlag(uint32_t flag) const {
      // On CH32V003, some bits are cleared by writing 0, some by software sequence.
      // TC bit can be cleared by writing 0.
      usart.STATR = static_cast<uint16_t>(~flag);
   }


   inline void sendBreak() const {
      usart.CTLR1 |= USART_CTLR1_SBK;
   }

   void configure(uint32_t baudRate = 115200, USARTParity parity = USARTParity::None, UsartStopBits stopBits = UsartStopBits::StopBits_1) const {
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
