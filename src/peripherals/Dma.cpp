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
   , _channel(getChannelTypedef()) {
}

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
   uintptr_t chAddress = DMA1_Channel1_BASE + _channelPosition * (DMA1_Channel2_BASE - DMA1_Channel1_BASE);
   return *reinterpret_cast<DMA_Channel_TypeDef *>(chAddress);
}


} // namespace Dma
