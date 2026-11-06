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

DMA_Channel_TypeDef &Channel::getChannelTypedef() const {
   // Channels 1 to 7 are 0x14 apart from 0x08. Channels 8 and up (CH32V30X DMA2) are 0x10 apart from 0x90.
   const uintptr_t offset = _channelPosition < 7 ? 0x08 + _channelPosition * 0x14 : 0x90 + (_channelPosition - 7) * 0x10;
   return *reinterpret_cast<DMA_Channel_TypeDef *>(reinterpret_cast<uintptr_t>(&_dma) + offset);
}

} // namespace Dma
