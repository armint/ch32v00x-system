#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the Dma driver
// (src/peripherals/Dma.h). The DMA types and bits are copies from
// src/families/ch32v30x/ch32v30x.h (the WCH header). test_dma_v30x.cpp points a
// DMA_TypeDef at a block of host memory, so the channel registers have their
// real offsets. DMA2_EXTEN (the flags of the channels 8 to 11) is at offset
// 0xD0 of that block.

#include <stddef.h>
#include <stdint.h>

#define DMA1_BASE 0x40020000UL
#define DMA2_BASE 0x40020400UL
#define DMA2_Channel8_BASE (DMA2_BASE + 0x90UL)
#define DMA2_EXTEN_BASE (DMA2_BASE + 0xD0UL)

typedef struct {
   volatile uint32_t CFGR;
   volatile uint32_t CNTR;
   volatile uint32_t PADDR;
   volatile uint32_t MADDR;
} DMA_Channel_TypeDef;

typedef struct {
   volatile uint32_t INTFR;
   volatile uint32_t INTFCR;
} DMA_TypeDef;

extern DMA_TypeDef *hostDma2Exten;
#define DMA2_EXTEN hostDma2Exten

#define DMA_GIF1 (1ul << 0)
#define DMA_TCIF1 (1ul << 1)
#define DMA_CHTIF1 (1ul << 2)
#define DMA_CTEIF1 (1ul << 3)

#define DMA_CFGR1_EN (1ul << 0)
#define DMA_CFGR1_TCIE (1ul << 1)
#define DMA_CFGR1_HTIE (1ul << 2)
#define DMA_CFGR1_TEIE (1ul << 3)
#define DMA_CFGR1_DIR (1ul << 4)
#define DMA_CFGR1_CIRC (1ul << 5)
#define DMA_CFGR1_PINC (1ul << 6)
#define DMA_CFGR1_MINC (1ul << 7)
#define DMA_CFGR1_PSIZE (3ul << 8)
#define DMA_CFGR1_MSIZE (3ul << 10)
#define DMA_CFGR1_PL (3ul << 12)
#define DMA_CFGR1_PL_0 (1ul << 12)
#define DMA_CFGR1_PL_1 (1ul << 13)
#define DMA_CFGR1_MEM2MEM (1ul << 14)
