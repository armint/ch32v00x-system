#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the USBHS device driver
// (src/families/ch32v30x/UsbDevice.cpp). The register blocks and the IRQ
// number are copies from src/families/ch32v30x/ch32v30x.h (the WCH header). USBHS and RCC are
// plain host memory that test_usb_v307.cpp defines, so a register holds the
// last written value. The fake does not model write-1-to-clear flags, the
// hardware data toggle or any other side effect.

#include <stddef.h>
#include <stdint.h>

#define HSE_VALUE ((uint32_t)8000000)

typedef enum IRQn {
   TIM2_IRQn = 44,
   USBHS_IRQn = 85,
} IRQn_Type;

// NVIC: the test records the last enabled and disabled IRQ.
extern IRQn_Type hostNvicEnabled;
extern IRQn_Type hostNvicDisabled;
inline void NVIC_EnableIRQ(IRQn_Type irq) {
   hostNvicEnabled = irq;
}
inline void NVIC_DisableIRQ(IRQn_Type irq) {
   hostNvicDisabled = irq;
}
inline void NVIC_ClearPendingIRQ(IRQn_Type) {
}

typedef struct {
   volatile uint32_t CTLR;      /* 0x00 Clock control */
   volatile uint32_t CFGR0;     /* 0x04 Clock configuration 0 */
   volatile uint32_t INTR;      /* 0x08 Clock interrupt */
   volatile uint32_t APB2PRSTR; /* 0x0C APB2 peripheral reset */
   volatile uint32_t APB1PRSTR; /* 0x10 APB1 peripheral reset */
   volatile uint32_t AHBPCENR;  /* 0x14 AHB peripheral clock enable */
   volatile uint32_t APB2PCENR; /* 0x18 APB2 peripheral clock enable */
   volatile uint32_t APB1PCENR; /* 0x1C APB1 peripheral clock enable */
   volatile uint32_t BDCTLR;    /* 0x20 Backup domain control */
   volatile uint32_t RSTSCKR;   /* 0x24 Control and status */
   volatile uint32_t AHBRSTR;   /* 0x28 AHB peripheral reset */
   volatile uint32_t CFGR2;     /* 0x2C Clock configuration 2 */
} RCC_TypeDef;

extern RCC_TypeDef hostRcc;
#define RCC (&hostRcc)

/* AHB peripheral clock enable bits (RCC_AHBPCENR) */
#define RCC_USBHSEN (1ul << 11)

#define RCC_USBHSDIV (7ul << 24) /* USBHS PHY PLL input: source / (USBHSDIV + 1) */
#define RCC_USBHSPLLSRC (1ul << 27)   /* 0: HSE, 1: HSI */
#define RCC_USBHSCLK (3ul << 28) /* Input frequency of the PHY PLL */
#define RCC_USBHSPLL (1ul << 30) /* PHY PLL on */
#define RCC_USBFSSRC (1ul << 31) /* RM: USBHSSRC. 48 MHz USB clock: 0: PLL, 1: USBHS PHY */

/* USBHSD_TypeDef of src/families/ch32v30x/ch32v30x.h (the WCH header) */
typedef struct {
   volatile uint8_t CONTROL;
   volatile uint8_t HOST_CTRL;
   volatile uint8_t INT_EN;
   volatile uint8_t DEV_AD;
   volatile uint16_t FRAME_NO;
   volatile uint8_t SUSPEND;
   volatile uint8_t RESERVED0;
   volatile uint8_t SPEED_TYPE;
   volatile uint8_t MIS_ST;
   volatile uint8_t INT_FG;
   volatile uint8_t INT_ST;
   volatile uint16_t RX_LEN;
   volatile uint16_t RESERVED1;
   volatile uint32_t ENDP_CONFIG;
   volatile uint32_t ENDP_TYPE;
   volatile uint32_t BUF_MODE;
   volatile uint32_t UEP0_DMA;
   volatile uint32_t UEP1_RX_DMA;
   volatile uint32_t UEP2_RX_DMA;
   volatile uint32_t UEP3_RX_DMA;
   volatile uint32_t UEP4_RX_DMA;
   volatile uint32_t UEP5_RX_DMA;
   volatile uint32_t UEP6_RX_DMA;
   volatile uint32_t UEP7_RX_DMA;
   volatile uint32_t UEP8_RX_DMA;
   volatile uint32_t UEP9_RX_DMA;
   volatile uint32_t UEP10_RX_DMA;
   volatile uint32_t UEP11_RX_DMA;
   volatile uint32_t UEP12_RX_DMA;
   volatile uint32_t UEP13_RX_DMA;
   volatile uint32_t UEP14_RX_DMA;
   volatile uint32_t UEP15_RX_DMA;
   volatile uint32_t UEP1_TX_DMA;
   volatile uint32_t UEP2_TX_DMA;
   volatile uint32_t UEP3_TX_DMA;
   volatile uint32_t UEP4_TX_DMA;
   volatile uint32_t UEP5_TX_DMA;
   volatile uint32_t UEP6_TX_DMA;
   volatile uint32_t UEP7_TX_DMA;
   volatile uint32_t UEP8_TX_DMA;
   volatile uint32_t UEP9_TX_DMA;
   volatile uint32_t UEP10_TX_DMA;
   volatile uint32_t UEP11_TX_DMA;
   volatile uint32_t UEP12_TX_DMA;
   volatile uint32_t UEP13_TX_DMA;
   volatile uint32_t UEP14_TX_DMA;
   volatile uint32_t UEP15_TX_DMA;
   volatile uint16_t UEP0_MAX_LEN;
   volatile uint16_t RESERVED2;
   volatile uint16_t UEP1_MAX_LEN;
   volatile uint16_t RESERVED3;
   volatile uint16_t UEP2_MAX_LEN;
   volatile uint16_t RESERVED4;
   volatile uint16_t UEP3_MAX_LEN;
   volatile uint16_t RESERVED5;
   volatile uint16_t UEP4_MAX_LEN;
   volatile uint16_t RESERVED6;
   volatile uint16_t UEP5_MAX_LEN;
   volatile uint16_t RESERVED7;
   volatile uint16_t UEP6_MAX_LEN;
   volatile uint16_t RESERVED8;
   volatile uint16_t UEP7_MAX_LEN;
   volatile uint16_t RESERVED9;
   volatile uint16_t UEP8_MAX_LEN;
   volatile uint16_t RESERVED10;
   volatile uint16_t UEP9_MAX_LEN;
   volatile uint16_t RESERVED11;
   volatile uint16_t UEP10_MAX_LEN;
   volatile uint16_t RESERVED12;
   volatile uint16_t UEP11_MAX_LEN;
   volatile uint16_t RESERVED13;
   volatile uint16_t UEP12_MAX_LEN;
   volatile uint16_t RESERVED14;
   volatile uint16_t UEP13_MAX_LEN;
   volatile uint16_t RESERVED15;
   volatile uint16_t UEP14_MAX_LEN;
   volatile uint16_t RESERVED16;
   volatile uint16_t UEP15_MAX_LEN;
   volatile uint16_t RESERVED17;
   volatile uint16_t UEP0_TX_LEN;
   volatile uint8_t UEP0_TX_CTRL;
   volatile uint8_t UEP0_RX_CTRL;
   volatile uint16_t UEP1_TX_LEN;
   volatile uint8_t UEP1_TX_CTRL;
   volatile uint8_t UEP1_RX_CTRL;
   volatile uint16_t UEP2_TX_LEN;
   volatile uint8_t UEP2_TX_CTRL;
   volatile uint8_t UEP2_RX_CTRL;
   volatile uint16_t UEP3_TX_LEN;
   volatile uint8_t UEP3_TX_CTRL;
   volatile uint8_t UEP3_RX_CTRL;
   volatile uint16_t UEP4_TX_LEN;
   volatile uint8_t UEP4_TX_CTRL;
   volatile uint8_t UEP4_RX_CTRL;
   volatile uint16_t UEP5_TX_LEN;
   volatile uint8_t UEP5_TX_CTRL;
   volatile uint8_t UEP5_RX_CTRL;
   volatile uint16_t UEP6_TX_LEN;
   volatile uint8_t UEP6_TX_CTRL;
   volatile uint8_t UEP6_RX_CTRL;
   volatile uint16_t UEP7_TX_LEN;
   volatile uint8_t UEP7_TX_CTRL;
   volatile uint8_t UEP7_RX_CTRL;
   volatile uint16_t UEP8_TX_LEN;
   volatile uint8_t UEP8_TX_CTRL;
   volatile uint8_t UEP8_RX_CTRL;
   volatile uint16_t UEP9_TX_LEN;
   volatile uint8_t UEP9_TX_CTRL;
   volatile uint8_t UEP9_RX_CTRL;
   volatile uint16_t UEP10_TX_LEN;
   volatile uint8_t UEP10_TX_CTRL;
   volatile uint8_t UEP10_RX_CTRL;
   volatile uint16_t UEP11_TX_LEN;
   volatile uint8_t UEP11_TX_CTRL;
   volatile uint8_t UEP11_RX_CTRL;
   volatile uint16_t UEP12_TX_LEN;
   volatile uint8_t UEP12_TX_CTRL;
   volatile uint8_t UEP12_RX_CTRL;
   volatile uint16_t UEP13_TX_LEN;
   volatile uint8_t UEP13_TX_CTRL;
   volatile uint8_t UEP13_RX_CTRL;
   volatile uint16_t UEP14_TX_LEN;
   volatile uint8_t UEP14_TX_CTRL;
   volatile uint8_t UEP14_RX_CTRL;
   volatile uint16_t UEP15_TX_LEN;
   volatile uint8_t UEP15_TX_CTRL;
   volatile uint8_t UEP15_RX_CTRL;
} USBHSD_TypeDef;

// The offsets of the WCH header (RM chapter 22).
static_assert(offsetof(USBHSD_TypeDef, INT_FG) == 0x0A, "USBHS layout");
static_assert(offsetof(USBHSD_TypeDef, RX_LEN) == 0x0C, "USBHS layout");
static_assert(offsetof(USBHSD_TypeDef, UEP0_DMA) == 0x1C, "USBHS layout");
static_assert(offsetof(USBHSD_TypeDef, UEP1_TX_DMA) == 0x5C, "USBHS layout");
static_assert(offsetof(USBHSD_TypeDef, UEP0_MAX_LEN) == 0x98, "USBHS layout");
static_assert(offsetof(USBHSD_TypeDef, UEP0_TX_LEN) == 0xD8, "USBHS layout");
static_assert(offsetof(RCC_TypeDef, CFGR2) == 0x2C, "RCC layout");

extern USBHSD_TypeDef hostUsbhs;
#define USBHSD (&hostUsbhs)
/* The USBHS bits come from the WCH header <ch32v30x_usb.h>: it has only macros. */
