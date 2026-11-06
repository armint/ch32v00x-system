#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the Spi driver (src/peripherals/Spi.cpp). It has
// the SPI bits of src/families/ch32v30x/ch32v30x.h, the RCC and SPI registers as host variables,
// and SystemCoreClock.

#include <stdint.h>

// RCC registers, RCC bits and SystemCoreClock: the same stub as the Rcc test (Spi.cpp uses Rcc).
#include "../stubs-rcc-v30x/wch_device.h"

typedef struct {
   volatile uint16_t CTLR1;
   uint16_t RESERVED0;
   volatile uint16_t CTLR2;
   uint16_t RESERVED1;
   volatile uint16_t STATR;
   uint16_t RESERVED2;
   volatile uint16_t DATAR;
   uint16_t RESERVED3;
} SPI_TypeDef;

extern SPI_TypeDef hostSpi1;
extern SPI_TypeDef hostSpi3;
#define SPI1 (&hostSpi1)
#define SPI3 (&hostSpi3)

#define SPI_CTLR1_CPHA ((uint16_t)0x0001)
#define SPI_CTLR1_CPOL ((uint16_t)0x0002)
#define SPI_CTLR1_MSTR ((uint16_t)0x0004)
#define SPI_CTLR1_BR ((uint16_t)0x0038)
#define SPI_CTLR1_BR_0 ((uint16_t)0x0008)
#define SPI_CTLR1_BR_1 ((uint16_t)0x0010)
#define SPI_CTLR1_BR_2 ((uint16_t)0x0020)
#define SPI_CTLR1_SPE ((uint16_t)0x0040)
#define SPI_CTLR1_LSBFIRST ((uint16_t)0x0080)
#define SPI_CTLR1_SSI ((uint16_t)0x0100)
#define SPI_CTLR1_SSM ((uint16_t)0x0200)
#define SPI_CTLR1_RXONLY ((uint16_t)0x0400)
#define SPI_CTLR1_DFF ((uint16_t)0x0800)
#define SPI_CTLR1_BIDIOE ((uint16_t)0x4000)
#define SPI_CTLR1_BIDIMODE ((uint16_t)0x8000)

#define SPI_CTLR2_RXDMAEN ((uint16_t)0x0001)
#define SPI_CTLR2_TXDMAEN ((uint16_t)0x0002)
#define SPI_CTLR2_SSOE ((uint16_t)0x0004)
#define SPI_CTLR2_ERRIE ((uint16_t)0x0020)
#define SPI_CTLR2_RXNEIE ((uint16_t)0x0040)
#define SPI_CTLR2_TXEIE ((uint16_t)0x0080)

#define SPI_STATR_RXNE ((uint16_t)0x0001)
#define SPI_STATR_TXE ((uint16_t)0x0002)
#define SPI_STATR_MODF ((uint16_t)0x0020)
#define SPI_STATR_OVR ((uint16_t)0x0040)
#define SPI_STATR_BSY ((uint16_t)0x0080)
