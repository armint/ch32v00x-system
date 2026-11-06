#pragma once
#include <wch_device.h>

namespace Dma {

enum class Direction { PeripheralToMemory = 0, MemoryToPeripheral = DMA_CFGR1_DIR };

enum class DataSize { Size8Bits = 0, Size16Bits = 1, Size32Bits = 2 };

enum class ChannelPriority {
   low = 0,
   medium = DMA_CFGR1_PL_0,
   high = DMA_CFGR1_PL_1,
   very_high = (DMA_CFGR1_PL_0 | DMA_CFGR1_PL_1)
};

struct Channel {
   const uint8_t _channelPosition;
   DMA_TypeDef &_dma;
   DMA_Channel_TypeDef &_channel;
   DMA_Channel_TypeDef &getChannelTypedef() const;

   Channel(DMA_TypeDef *dma, uint8_t channelNumber);

   Channel(const Channel &) = default;
   Channel(Channel &&) = default;
   Channel &operator=(const Channel &) = delete;
   Channel &operator=(Channel &&) = delete;

   void configure(void *peripheralAddress,
                  void *memoryAddress,
                  DataSize peripheralSize,
                  DataSize memorySize,
                  Direction direction,
                  bool increaseMemAddress,
                  bool increasePeriphAddress) const;

   inline void
   configure(void *peripheralAddress, void *memoryAddress, DataSize size, Direction direction, bool increaseMemAddress, bool increasePeriphAddress) const {
      configure(peripheralAddress, memoryAddress, size, size, direction, increaseMemAddress, increasePeriphAddress);
   }

   inline void enable() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_EN;
   }

   inline void disable() const {
      _channel.CFGR &= ~(uint32_t)DMA_CFGR1_EN;
   }

   inline void enableIncreaseMemoryAddress() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_MINC;
   }

   inline void disableIncreaseMemoryAddress() const {
      _channel.CFGR &= ~(uint32_t)DMA_CFGR1_MINC;
   }

   inline void enableIncreasePeripheralAddress() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_PINC;
   }

   inline void disableIncreasePeripheralAddress() const {
      _channel.CFGR &= ~(uint32_t)DMA_CFGR1_PINC;
   }

   inline void setPeripheralAddress(uint32_t address) const {
      _channel.PADDR = address;
   }

   inline void setMemoryAddress(uint32_t address) const {
      _channel.MADDR = address;
   }

   inline void setDirection(Direction dir) const {
      _channel.CFGR = (_channel.CFGR & ~((uint32_t)DMA_CFGR1_DIR)) | static_cast<uint32_t>(dir);
   }

   inline void setDataSize(DataSize size) const {
      setMemoryDataSize(size);
      setPeripheralDataSize(size);
   }

   inline void setTransferCount(uint16_t count) const {
      _channel.CNTR = count;
   }

   inline uint16_t getDataCount() const {
      return _channel.CNTR;
   }

   inline void setMemoryDataSize(DataSize size) const {
      _channel.CFGR = (_channel.CFGR & ~(uint32_t)DMA_CFGR1_MSIZE) | static_cast<uint32_t>(size) << 10;
   }

   inline void setPeripheralDataSize(DataSize size) const {
      _channel.CFGR = (_channel.CFGR & ~(uint32_t)DMA_CFGR1_PSIZE) | static_cast<uint32_t>(size) << 8;
   }

   inline void setTransferDirection(Direction direction) const {
      _channel.CFGR = (_channel.CFGR & ~(uint32_t)DMA_CFGR1_DIR) | static_cast<uint32_t>(direction);
   }

   inline void enableMemoryToMemory() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_MEM2MEM;
   }

   inline void disableMemoryToMemory() const {
      _channel.CFGR &= ~(uint32_t)DMA_CFGR1_MEM2MEM;
   }

   inline void setPriority(ChannelPriority prio) const {
      _channel.CFGR = (_channel.CFGR & ~(uint32_t)DMA_CFGR1_PL) | static_cast<uint32_t>(prio);
   }

   inline void setCircular(bool circular) const {
      if(circular) {
         _channel.CFGR |= DMA_CFGR1_CIRC;

      } else {
         _channel.CFGR &= ~DMA_CFGR1_CIRC;
      }
   }

   inline void enableTransferCompleteInterrupt() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_TCIE;
   }

   inline void disableAllInterrupts() const {
      _channel.CFGR &= ~(DMA_CFGR1_TCIE | DMA_CFGR1_HTIE | DMA_CFGR1_TEIE);
   }

   inline void disableTransferCompleteInterrupt() const {
      _channel.CFGR &= ~(uint32_t)DMA_CFGR1_TCIE;
   }

   inline void enableHalfTransferInterrupt() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_HTIE;
   }

   inline void enableTransferErrorInterrupt() const {
      _channel.CFGR |= (uint32_t)DMA_CFGR1_TEIE;
   }

   // Interrupt flag registers and the first flag bit of this channel.
   // On CH32V30X the flags of the DMA2 channels 8 to 11 are in the extend registers.
   inline volatile uint32_t &flagRegister() const {
#if defined(DMA2_Channel8_BASE)
      return _channelPosition < 7 ? _dma.INTFR : _dma.EXTEM_INTFR;
#else
      return _dma.INTFR;
#endif
   }

   inline volatile uint32_t &flagClearRegister() const {
#if defined(DMA2_Channel8_BASE)
      return _channelPosition < 7 ? _dma.INTFCR : _dma.EXTEM_INTFCR;
#else
      return _dma.INTFCR;
#endif
   }

   inline uint32_t flagShift() const {
#if defined(DMA2_Channel8_BASE)
      return (_channelPosition < 7 ? _channelPosition : _channelPosition - 7) * 4;
#else
      return _channelPosition * 4;
#endif
   }

   inline void clearGlobalInterruptFlag() const {
      flagClearRegister() = (uint32_t)DMA_GIF1 << flagShift();
   }

   inline void clearTransferCompleteInterruptFlag() const {
      flagClearRegister() = (uint32_t)DMA_TCIF1 << flagShift();
   }

   inline void clearHalfTransferInterruptFlag() const {
      flagClearRegister() = (uint32_t)DMA_CHTIF1 << flagShift();
   }

   inline void clearTransferErrorInterruptFlag() const {
      flagClearRegister() = (uint32_t)DMA_CTEIF1 << flagShift();
   }

   inline bool isGlobalInterruptFlagSet() const {
      return flagRegister() & ((uint32_t)DMA_GIF1 << flagShift());
   }

   inline bool isTransferCompleteInterruptFlagSet() const {
      return flagRegister() & ((uint32_t)DMA_TCIF1 << flagShift());
   }

   inline bool isHalfTransferInterruptFlagSet() const {
      return flagRegister() & ((uint32_t)DMA_CHTIF1 << flagShift());
   }

   inline bool isTransferErrorInterruptFlagSet() const {
      return flagRegister() & ((uint32_t)DMA_CTEIF1 << flagShift());
   }
};
} // namespace Dma
