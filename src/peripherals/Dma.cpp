/*
 * Dma.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: armin
 */

#include "Dma.h"
#include <cassert>

namespace Dma {
Channel::Channel(DMA_TypeDef *dma, uint8_t channelNumber)
   : _channelPosition(channelNumber - 1)
   , _dma(*dma)
   , _channel(getChannelTypedef()) {}

void Channel::configure(void *peripheralAddress,
                        void *memoryAddress,
                        DataSize peripheralSize,
                        DataSize memorySize,
                        Direction direction,
                        bool increaseMemAddress,
                        bool increasePeriphAddress) const {
   setPeripheralAddress((uintptr_t)peripheralAddress);
   setMemoryAddress((uintptr_t)memoryAddress);
   setMemoryDataSize(memorySize);
   setPeripheralDataSize(peripheralSize);
   _channel.CFGR = (_channel.CFGR & ~((uint32_t)DMA_CFGR1_MINC | DMA_CFGR1_PINC | DMA_CFGR1_DIR)) |
                   (increaseMemAddress ? DMA_CFGR1_MINC : 0) | (increasePeriphAddress ? DMA_CFGR1_PINC : 0) |
                   static_cast<uint32_t>(direction);
}

// Offset of the registers of a channel from the DMA base.
constexpr uintptr_t channelOffset(uint8_t position) {
#if defined(DMA2_Channel8_BASE)
   // CH32V30X: channels 8 and up (DMA2 only) are 0x10 apart from 0x90.
   if(position >= 7) {
      return 0x90 + (position - 7) * 0x10;
   }
#endif
   return 0x08 + position * 0x14;
}

#if defined(DMA1_Channel8_BASE)
// CH32V20X has channel 8 on DMA1 at 0x94. Fails when the formula and the device header differ.
static_assert(channelOffset(7) == DMA1_Channel8_BASE - DMA1_BASE, "channel 8 offset differs from the device header");
#endif
#if defined(DMA2_Channel11_BASE)
static_assert(channelOffset(10) == DMA2_Channel11_BASE - DMA2_BASE, "channel 11 offset differs from the device header");
static_assert(channelOffset(6) == DMA2_Channel7_BASE - DMA2_BASE, "channel 7 offset differs from the device header");
#endif

DMA_Channel_TypeDef &Channel::getChannelTypedef() const {
   return *reinterpret_cast<DMA_Channel_TypeDef *>(reinterpret_cast<uintptr_t>(&_dma) + channelOffset(_channelPosition));
}

} // namespace Dma
