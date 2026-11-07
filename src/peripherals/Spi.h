#pragma once

#include <cstdint>
#include <optional>
#include <wch_device.h>

namespace hw {

/// SPI driver. See docs/spi.md.
/// All families have SPI1. CH32V20X and CH32V30X also have SPI2, CH32V30X also SPI3.
/// The CH32 SPI has no FIFO: a frame is 8 or 16 bits (CTLR1.DFF).
struct Spi {
   enum class Mode : uint32_t { Slave = 0, Master = SPI_CTLR1_MSTR };

   enum class Direction : uint32_t {
      TwoLines = 0,
      TwoLines_RxOnly = SPI_CTLR1_RXONLY,
      OneLine_Rx = SPI_CTLR1_BIDIMODE,
      OneLine_Tx = SPI_CTLR1_BIDIMODE | SPI_CTLR1_BIDIOE
   };

   enum class DataSize : uint32_t { Bits8 = 0, Bits16 = SPI_CTLR1_DFF };

   enum class ClockPolarity : uint32_t { Low = 0, High = SPI_CTLR1_CPOL };

   enum class ClockPhase : uint32_t { OneEdge = 0, TwoEdge = SPI_CTLR1_CPHA };

   /// SCK = PCLK / divider. The high-speed read mode (HSCR) of CH32V30X changes the dividers;
   /// this driver does not set it.
   enum class BaudRatePrescaler : uint32_t {
      Div2 = 0,
      Div4 = SPI_CTLR1_BR_0,
      Div8 = SPI_CTLR1_BR_1,
      Div16 = (SPI_CTLR1_BR_1 | SPI_CTLR1_BR_0),
      Div32 = SPI_CTLR1_BR_2,
      Div64 = (SPI_CTLR1_BR_2 | SPI_CTLR1_BR_0),
      Div128 = (SPI_CTLR1_BR_2 | SPI_CTLR1_BR_1),
      Div256 = (SPI_CTLR1_BR_2 | SPI_CTLR1_BR_1 | SPI_CTLR1_BR_0)
   };

   enum class FirstBit : uint32_t { Msb = 0, Lsb = SPI_CTLR1_LSBFIRST };

   enum class NssMode : uint32_t {
      HardInput = 0,
      HardOutput = 0x10000, // not a register bit: setNssMode() sets CTLR2.SSOE
      Soft = SPI_CTLR1_SSM
   };

   SPI_TypeDef &spi_;

   explicit Spi(SPI_TypeDef *spiBase)
      : spi_(*spiBase) {
   }

   Spi(const Spi &) = default;
   Spi(Spi &&) = default;
   Spi &operator=(const Spi &) = delete;
   Spi &operator=(Spi &&) = delete;

   /// Sets all fields of CTLR1 except SPE, CRCEN and CRCNEXT, and SSOE in CTLR2. An enabled SPI is
   /// disabled for the time of the change and then enabled again.
   void configure(BaudRatePrescaler prescaler,
                  Mode mode = Mode::Master,
                  ClockPolarity cpol = ClockPolarity::Low,
                  ClockPhase cpha = ClockPhase::OneEdge,
                  DataSize dataSize = DataSize::Bits8,
                  FirstBit firstBit = FirstBit::Msb,
                  NssMode nss = NssMode::Soft,
                  Direction direction = Direction::TwoLines) const;

   inline void enable() const {
      spi_.CTLR1 |= SPI_CTLR1_SPE;
   }

   inline void disable() const {
      spi_.CTLR1 &= ~SPI_CTLR1_SPE;
   }

   inline bool isEnabled() const {
      return (spi_.CTLR1 & SPI_CTLR1_SPE) != 0;
   }

   inline void setMode(Mode mode) const {
      setCtlr1(SPI_CTLR1_MSTR, static_cast<uint32_t>(mode));
   }

   inline void setDirection(Direction direction) const {
      setCtlr1(SPI_CTLR1_BIDIMODE | SPI_CTLR1_BIDIOE | SPI_CTLR1_RXONLY, static_cast<uint32_t>(direction));
   }

   inline void setBaudRatePrescaler(BaudRatePrescaler prescaler) const {
      setCtlr1(SPI_CTLR1_BR, static_cast<uint32_t>(prescaler));
   }

   inline void setClockPolarity(ClockPolarity cpol) const {
      setCtlr1(SPI_CTLR1_CPOL, static_cast<uint32_t>(cpol));
   }

   inline void setClockPhase(ClockPhase cpha) const {
      setCtlr1(SPI_CTLR1_CPHA, static_cast<uint32_t>(cpha));
   }

   inline void setDataSize(DataSize dataSize) const {
      setCtlr1(SPI_CTLR1_DFF, static_cast<uint32_t>(dataSize));
   }

   inline void setFirstBit(FirstBit firstBit) const {
      setCtlr1(SPI_CTLR1_LSBFIRST, static_cast<uint32_t>(firstBit));
   }

   /// Soft: SSM set. SSI is set on a master and clear on a slave, so that the SPI is active
   /// without an NSS pin (RM 20.2.3). Call it after setMode(); configure() does.
   /// HardOutput: SSOE set (master drives NSS). HardInput: the NSS pin selects the SPI.
   void setNssMode(NssMode nss) const;

   /// True for 16-bit frames.
   inline bool hasWordAccess() const {
      return (spi_.CTLR1 & SPI_CTLR1_DFF) != 0;
   }

   /// Writes one frame (8 or 16 bits) to DATAR.
   inline void write(uint16_t data) const {
      spi_.DATAR = data;
   }

   /// Reads one frame (8 or 16 bits) from DATAR. The read clears RXNE.
   inline uint16_t read() const {
      return static_cast<uint16_t>(spi_.DATAR);
   }

   /// Default bound on the poll loops of transfer() and waitUntilIdle(), in loop iterations.
   /// Loops, not time: it works before SysTick runs.
   static constexpr uint32_t defaultTimeoutLoops = 100'000u;

   /// Blocking full-duplex transfer of one frame. Waits for TXE, writes, then waits for RXNE.
   /// @param timeoutLoops bound of each of the two waits. 0 fails at once unless the flag is set.
   /// @return the received frame, or std::nullopt after a timeout. After a timeout DATAR is read and
   ///         OVR is clear, so that the next call starts in a known state.
   [[nodiscard]] inline std::optional<uint16_t> transfer(uint16_t data, uint32_t timeoutLoops = defaultTimeoutLoops) const {
      uint32_t remaining = timeoutLoops;
      while(!isTxEmpty()) {
         if(remaining == 0u) {
            clearOverrun();
            return std::nullopt;
         }
         --remaining;
      }
      write(data);
      remaining = timeoutLoops;
      while(!isRxNotEmpty()) {
         if(remaining == 0u) {
            // A late frame in DATAR would otherwise be the result of the next transfer().
            clearOverrun();
            return std::nullopt;
         }
         --remaining;
      }
      return read();
   }

   /// Waits until BSY is clear. @return false when timeoutLoops ran out first.
   [[nodiscard]] inline bool waitUntilIdle(uint32_t timeoutLoops = defaultTimeoutLoops) const {
      while(isBusy()) {
         if(timeoutLoops == 0u) {
            return false;
         }
         --timeoutLoops;
      }
      return true;
   }

   inline bool isTxEmpty() const {
      return (spi_.STATR & SPI_STATR_TXE) != 0;
   }

   inline bool isRxNotEmpty() const {
      return (spi_.STATR & SPI_STATR_RXNE) != 0;
   }

   inline bool isBusy() const {
      return (spi_.STATR & SPI_STATR_BSY) != 0;
   }

   inline bool isOverrun() const {
      return (spi_.STATR & SPI_STATR_OVR) != 0;
   }

   inline bool isModeFault() const {
      return (spi_.STATR & SPI_STATR_MODF) != 0;
   }

   /// Clears OVR: a read of DATAR, then a read of STATR (RM 20.2.7).
   inline void clearOverrun() const {
      [[maybe_unused]] uint32_t tmp = spi_.DATAR;
      tmp = spi_.STATR;
   }

   /// Clears MODF: a read of STATR, then a write of CTLR1 (RM 20.2.7). The SPI is enabled after the call.
   inline void clearModeFault() const {
      [[maybe_unused]] const uint32_t tmp = spi_.STATR;
      spi_.CTLR1 &= ~SPI_CTLR1_SPE;
      spi_.CTLR1 |= SPI_CTLR1_SPE;
   }

   inline void enableTxDma() const {
      spi_.CTLR2 |= SPI_CTLR2_TXDMAEN;
   }
   inline void disableTxDma() const {
      spi_.CTLR2 &= ~SPI_CTLR2_TXDMAEN;
   }
   inline void enableRxDma() const {
      spi_.CTLR2 |= SPI_CTLR2_RXDMAEN;
   }
   inline void disableRxDma() const {
      spi_.CTLR2 &= ~SPI_CTLR2_RXDMAEN;
   }

   inline void enableRxNotEmptyInterrupt() const {
      spi_.CTLR2 |= SPI_CTLR2_RXNEIE;
   }
   inline void disableRxNotEmptyInterrupt() const {
      spi_.CTLR2 &= ~SPI_CTLR2_RXNEIE;
   }
   inline void enableTxEmptyInterrupt() const {
      spi_.CTLR2 |= SPI_CTLR2_TXEIE;
   }
   inline void disableTxEmptyInterrupt() const {
      spi_.CTLR2 &= ~SPI_CTLR2_TXEIE;
   }
   inline void enableErrorInterrupt() const {
      spi_.CTLR2 |= SPI_CTLR2_ERRIE;
   }
   inline void disableErrorInterrupt() const {
      spi_.CTLR2 &= ~SPI_CTLR2_ERRIE;
   }

   /// The clock of the APB bus of this SPI: PCLK2 for SPI1, PCLK1 for SPI2 and SPI3 (HCLK on CH32V00X).
   uint32_t getClockFrequency() const;

   private:
   inline void setCtlr1(uint32_t mask, uint32_t value) const {
      spi_.CTLR1 = static_cast<uint16_t>((spi_.CTLR1 & ~mask) | value);
   }
};

} // namespace hw
