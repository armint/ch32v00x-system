#pragma once

// Host stand-in for <wch_device.h> of CH32V00X: only the GPIO register block,
// with the layout of the family device header.

#include <stdint.h>

typedef struct {
   volatile uint32_t CFGLR; /* 0x00 Port configuration low */
   uint32_t RESERVED;       /* 0x04 No CFGHR: 8 pins per port */
   volatile uint32_t INDR;  /* 0x08 Input data */
   volatile uint32_t OUTDR; /* 0x0C Output data */
   volatile uint32_t BSHR;  /* 0x10 Set and reset */
   volatile uint32_t BCR;   /* 0x14 Reset */
   volatile uint32_t LCKR;  /* 0x18 Configuration lock */
} GPIO_TypeDef;
