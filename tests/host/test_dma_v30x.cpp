// Host test for the Dma driver with the CH32V30x registers: the channel
// register addresses (channels 1 to 7 are 0x14 apart, channels 8 to 11 are 0x10
// apart from 0x90) and the interrupt flags (channels 8 to 11 use the extend
// registers). A block of host memory plays the DMA2 register file.

#include "check.h"
#include "Dma.h"
#include <cstring>

alignas(8) static uint8_t dmaMemory[0x100];

int main() {
   auto *dma = reinterpret_cast<DMA_TypeDef *>(dmaMemory);
   auto at = [](uint32_t offset) { return reinterpret_cast<volatile uint32_t *>(dmaMemory + offset); };

   // Register addresses of the channels.
   const uint32_t cfgrOffset[] = { 0x08, 0x1C, 0x30, 0x44, 0x58, 0x6C, 0x80, 0x90, 0xA0, 0xB0, 0xC0 };
   for(uint8_t n = 1; n <= 11; ++n) {
      std::memset(dmaMemory, 0, sizeof(dmaMemory));
      Dma::Channel channel(dma, n);
      channel.setPeripheralAddress(0x11111111);
      channel.setMemoryAddress(0x22222222);
      channel.setTransferCount(0x1234);
      channel.enable();
      CHECK(*at(cfgrOffset[n - 1]) == DMA_CFGR1_EN);
      CHECK(*at(cfgrOffset[n - 1] + 4) == 0x1234);
      CHECK(*at(cfgrOffset[n - 1] + 8) == 0x11111111);
      CHECK(*at(cfgrOffset[n - 1] + 12) == 0x22222222);
      CHECK(channel.getDataCount() == 0x1234);
   }

   // Flags of channels 1 to 7 are in INTFR, 4 bits each.
   std::memset(dmaMemory, 0, sizeof(dmaMemory));
   Dma::Channel ch3(dma, 3);
   dma->INTFR = DMA_TCIF1 << 8;
   CHECK(ch3.isTransferCompleteInterruptFlagSet());
   CHECK(!ch3.isHalfTransferInterruptFlagSet());
   CHECK(!ch3.isGlobalInterruptFlagSet());
   CHECK(!ch3.isTransferErrorInterruptFlagSet());
   CHECK(!Dma::Channel(dma, 2).isTransferCompleteInterruptFlagSet());
   ch3.clearGlobalInterruptFlag();
   CHECK(dma->INTFCR == (DMA_GIF1 << 8));
   ch3.clearTransferErrorInterruptFlag();
   CHECK(dma->INTFCR == (DMA_CTEIF1 << 8));

   // Flags of channels 8 to 11 are in the extend registers, 4 bits each from bit 0.
   Dma::Channel ch8(dma, 8);
   Dma::Channel ch11(dma, 11);
   dma->INTFR = 0;
   dma->EXTEM_INTFR = (DMA_TCIF1 << 0) | (DMA_CHTIF1 << 12);
   CHECK(ch8.isTransferCompleteInterruptFlagSet());
   CHECK(!ch8.isHalfTransferInterruptFlagSet());
   CHECK(ch11.isHalfTransferInterruptFlagSet());
   CHECK(!ch11.isTransferCompleteInterruptFlagSet());
   dma->INTFCR = 0;
   ch11.clearGlobalInterruptFlag();
   CHECK(dma->EXTEM_INTFCR == (DMA_GIF1 << 12));
   CHECK(dma->INTFCR == 0);
   ch8.clearTransferCompleteInterruptFlag();
   CHECK(dma->EXTEM_INTFCR == DMA_TCIF1);
   return 0;
}
