#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the USBHS device driver
// (src/families/ch32v30x/UsbDevice.cpp). The register blocks, bits and IRQ
// number are copies from src/families/ch32v30x/ch32v30x.h. USBHS and RCC are
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

#define RCC_CFGR2_USBHSDIV_MASK (7ul << 24) /* USBHS PHY PLL input: source / (USBHSDIV + 1) */
#define RCC_CFGR2_USBHSPLLSRC (1ul << 27)   /* 0: HSE, 1: HSI */
#define RCC_CFGR2_USBHSCLK_MASK (3ul << 28) /* Input frequency of the PHY PLL */
#define RCC_CFGR2_USBHSCLK_4MHZ (1ul << 28)
#define RCC_CFGR2_USBHSPLL (1ul << 30) /* PHY PLL on */
#define RCC_CFGR2_USBFSSRC (1ul << 31) /* RM: USBHSSRC. 48 MHz USB clock: 0: PLL, 1: USBHS PHY */

/* USBHS controller (USB 2.0 high-speed, RM chapter 22). Device-mode names.
 * The host-mode registers share these addresses. */
typedef struct {
   volatile uint16_t MAX_LEN; /* Largest OUT packet that the endpoint accepts */
   uint16_t RESERVED;
} USBHS_MaxLen_TypeDef;

typedef struct {
   volatile uint16_t T_LEN;  /* IN packet length. EP0: bits 6:0 only */
   volatile uint8_t TX_CTRL; /* IN response and data toggle */
   volatile uint8_t RX_CTRL; /* OUT response and data toggle */
} USBHS_EpCtrl_TypeDef;

typedef struct {
   volatile uint8_t CTRL;      /* 0x00 Control */
   volatile uint8_t HOST_CTRL; /* 0x01 Host control. PHY_SUSPENDM applies in device mode too */
   volatile uint8_t INT_EN;    /* 0x02 Interrupt enable */
   volatile uint8_t DEV_AD;    /* 0x03 Device address */
   volatile uint16_t FRAME_NO; /* 0x04 Frame number */
   volatile uint8_t SUSPEND;   /* 0x06 Suspend */
   uint8_t RESERVED0;
   volatile uint8_t SPEED_TYPE; /* 0x08 Current speed */
   volatile uint8_t MIS_ST;     /* 0x09 Miscellaneous status */
   volatile uint8_t INT_FG;     /* 0x0A Interrupt flags, write 1 to clear */
   volatile uint8_t INT_ST;     /* 0x0B Interrupt status */
   volatile uint16_t RX_LEN;    /* 0x0C Length of the last received packet */
   uint16_t RESERVED1;
   volatile uint32_t UEP_CONFIG;         /* 0x10 Endpoint 1-15 IN/OUT enable */
   volatile uint32_t UEP_TYPE;           /* 0x14 Endpoint 1-15 isochronous type */
   volatile uint32_t UEP_BUF_MOD;        /* 0x18 Endpoint buffer mode */
   volatile uint32_t UEP0_DMA;           /* 0x1C EP0 buffer, IN and OUT shared, 4-byte aligned */
   volatile uint32_t UEP_RX_DMA[15];     /* 0x20 OUT buffer of endpoints 1-15 ([0] is EP1) */
   volatile uint32_t UEP_TX_DMA[15];     /* 0x5C IN buffer of endpoints 1-15 ([0] is EP1) */
   USBHS_MaxLen_TypeDef UEP_MAX_LEN[16]; /* 0x98 Endpoints 0-15 */
   USBHS_EpCtrl_TypeDef UEP[16];         /* 0xD8 Endpoints 0-15 */
} USBHS_TypeDef;

// The offsets of the device header (RM chapter 22).
static_assert(offsetof(USBHS_TypeDef, INT_FG) == 0x0A, "USBHS layout");
static_assert(offsetof(USBHS_TypeDef, RX_LEN) == 0x0C, "USBHS layout");
static_assert(offsetof(USBHS_TypeDef, UEP0_DMA) == 0x1C, "USBHS layout");
static_assert(offsetof(USBHS_TypeDef, UEP_TX_DMA) == 0x5C, "USBHS layout");
static_assert(offsetof(USBHS_TypeDef, UEP_MAX_LEN) == 0x98, "USBHS layout");
static_assert(offsetof(USBHS_TypeDef, UEP) == 0xD8, "USBHS layout");
static_assert(offsetof(RCC_TypeDef, CFGR2) == 0x2C, "RCC layout");

extern USBHS_TypeDef hostUsbhs;
#define USBHS (&hostUsbhs)
/* USBHS_CTRL bits */
#define USBHS_UC_DMA_EN ((uint8_t)0x01)
#define USBHS_UC_CLR_ALL ((uint8_t)0x02)
#define USBHS_UC_RST_SIE ((uint8_t)0x04)
#define USBHS_UC_INT_BUSY ((uint8_t)0x08) /* NAK all tokens while INT_FG.TRANSFER is set */
#define USBHS_UC_DEV_PU_EN ((uint8_t)0x10)
#define USBHS_UC_SPEED_MASK ((uint8_t)0x60)
#define USBHS_UC_SPEED_FULL ((uint8_t)0x00)
#define USBHS_UC_SPEED_HIGH ((uint8_t)0x20)
#define USBHS_UC_HOST_MODE ((uint8_t)0x80)

/* USBHS_HOST_CTRL bits */
#define USBHS_UH_PHY_SUSPENDM ((uint8_t)0x10) /* 1: PHY runs */

/* USBHS_INT_EN bits. Bits 5 to 7 follow the RM; the vendor SVD names them differently. */
#define USBHS_UIE_BUS_RST ((uint8_t)0x01)
#define USBHS_UIE_TRANSFER ((uint8_t)0x02)
#define USBHS_UIE_SUSPEND ((uint8_t)0x04)
#define USBHS_UIE_SOF_ACT ((uint8_t)0x08)
#define USBHS_UIE_FIFO_OV ((uint8_t)0x10)
#define USBHS_UIE_SETUP_ACT ((uint8_t)0x20)
#define USBHS_UIE_ISO_ACT ((uint8_t)0x40)
#define USBHS_UIE_DEV_NAK ((uint8_t)0x80)

/* USBHS_DEV_AD */
#define USBHS_DEV_AD_MASK ((uint8_t)0x7F)

/* USBHS_SPEED_TYPE */
#define USBHS_SPEED_TYPE_MASK ((uint8_t)0x03)
#define USBHS_SPEED_TYPE_FULL ((uint8_t)0x00)
#define USBHS_SPEED_TYPE_HIGH ((uint8_t)0x01)
#define USBHS_SPEED_TYPE_LOW ((uint8_t)0x02)

/* USBHS_MIS_ST bits */
#define USBHS_UMS_SUSPEND ((uint8_t)0x04)
#define USBHS_UMS_BUS_RESET ((uint8_t)0x08)
#define USBHS_UMS_SIE_FREE ((uint8_t)0x20)

/* USBHS_INT_FG bits */
#define USBHS_UIF_BUS_RST ((uint8_t)0x01)
#define USBHS_UIF_TRANSFER ((uint8_t)0x02) /* Any transfer except SETUP */
#define USBHS_UIF_SUSPEND ((uint8_t)0x04)  /* Suspend or resume, see MIS_ST.SUSPEND */
#define USBHS_UIF_HST_SOF ((uint8_t)0x08)
#define USBHS_UIF_FIFO_OV ((uint8_t)0x10)
#define USBHS_UIF_SETUP_ACT ((uint8_t)0x20)
#define USBHS_UIF_ISO_ACT ((uint8_t)0x40)

/* USBHS_INT_ST fields */
#define USBHS_UIS_ENDP_MASK ((uint8_t)0x0F)
#define USBHS_UIS_TOKEN_MASK ((uint8_t)0x30)
#define USBHS_UIS_TOKEN_OUT ((uint8_t)0x00)
#define USBHS_UIS_TOKEN_SOF ((uint8_t)0x10)
#define USBHS_UIS_TOKEN_IN ((uint8_t)0x20)
#define USBHS_UIS_TOKEN_SETUP ((uint8_t)0x30)
#define USBHS_UIS_TOG_OK ((uint8_t)0x40)
#define USBHS_UIS_IS_NAK ((uint8_t)0x80)

/* USBHS_UEP_CONFIG bits, n = 1..15. EP0 is always on. */
#define USBHS_UEP_T_EN(n) (1ul << (n))
#define USBHS_UEP_R_EN(n) (1ul << ((n) + 16))

/* USBHS_UEPn_TX_CTRL fields */
#define USBHS_UEP_T_RES_MASK ((uint8_t)0x03)
#define USBHS_UEP_T_RES_ACK ((uint8_t)0x00)
#define USBHS_UEP_T_RES_NAK ((uint8_t)0x02)
#define USBHS_UEP_T_RES_STALL ((uint8_t)0x03)
#define USBHS_UEP_T_TOG_MASK ((uint8_t)0x18)
#define USBHS_UEP_T_TOG_DATA1 ((uint8_t)0x08)
#define USBHS_UEP_T_TOG_AUTO ((uint8_t)0x20) /* Reserved on EP0 */

/* USBHS_UEPn_RX_CTRL fields */
#define USBHS_UEP_R_RES_MASK ((uint8_t)0x03)
#define USBHS_UEP_R_RES_ACK ((uint8_t)0x00)
#define USBHS_UEP_R_RES_NYET ((uint8_t)0x01)
#define USBHS_UEP_R_RES_NAK ((uint8_t)0x02)
#define USBHS_UEP_R_RES_STALL ((uint8_t)0x03)
#define USBHS_UEP_R_TOG_MASK ((uint8_t)0x18)
#define USBHS_UEP_R_TOG_DATA1 ((uint8_t)0x08)
#define USBHS_UEP_R_TOG_AUTO ((uint8_t)0x20) /* Reserved on EP0 */
