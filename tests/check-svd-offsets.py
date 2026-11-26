#!/usr/bin/env python3
"""Check the CH32V30x device header (the WCH header) against the vendor SVD.

The script reads the SVD and writes C `_Static_assert` lines. It compiles
them with the RISC-V compiler and `-fsyntax-only`. A mismatch in a base
address, a register offset or a bit field stops the check.

Usage (from the repository root):
  python3 tests/check-svd-offsets.py \
      --cc riscv-none-elf-gcc \
      --cflags "-DCH32V30x_D8C -march=rv32imafc_zicsr_zifencei -mabi=ilp32f"
"""

import argparse
import os
import shlex
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SVD = os.path.join(ROOT, "svd", "vendor", "CH32V307xx.svd")

# Struct members that map to SVD registers of the same name. A tuple
# (member, SVD register) maps a member whose name differs from the SVD.
STRUCTS = {
    "RCC": ("RCC_TypeDef", ["CTLR", "CFGR0", "INTR", "APB2PRSTR", "APB1PRSTR", "AHBPCENR",
                            "APB2PCENR", "APB1PCENR", "BDCTLR", "RSTSCKR", "AHBRSTR", "CFGR2"]),
    "GPIOA": ("GPIO_TypeDef", ["CFGLR", "CFGHR", "INDR", "OUTDR", "BSHR", "BCR", "LCKR"]),
    "TIM1": ("TIM_TypeDef", ["CTLR1", "CTLR2", "SMCFGR", "DMAINTENR", "INTFR", "SWEVGR",
                             ("CHCTLR1", "CHCTLR1_Output"), ("CHCTLR2", "CHCTLR2_Output"), "CCER", "CNT",
                             "PSC", "ATRLR", "RPTCR", "CH1CVR", "CH2CVR", "CH3CVR", "CH4CVR", "BDTR",
                             "DMACFGR", "DMAADR"]),
    "DMA1": ("DMA_TypeDef", ["INTFR", "INTFCR"]),
    "DMA2": ("DMA_TypeDef", ["INTFR", "INTFCR"]),
    "AFIO": ("AFIO_TypeDef", ["ECR", "PCFR1", ("EXTICR[0]", "EXTICR1"), ("EXTICR[1]", "EXTICR2"),
                              ("EXTICR[2]", "EXTICR3"), ("EXTICR[3]", "EXTICR4"), "PCFR2"]),
    "FLASH": ("FLASH_TypeDef", ["KEYR", "OBKEYR", "STATR", "CTLR", "ADDR", "OBR", "WPR", "MODEKEYR"]),
    "EXTI": ("EXTI_TypeDef", ["INTENR", "EVENR", "RTENR", "FTENR", "SWIEVR", "INTFR"]),
    "USART1": ("USART_TypeDef", ["STATR", "DATAR", "BRR", "CTLR1", "CTLR2", "CTLR3", "GPR"]),
    # The SVD has no I2SPR (0x20). The RM gives it for SPI2 and SPI3 (tables 20-2 and 20-3).
    "SPI1": ("SPI_TypeDef", ["CTLR1", "CTLR2", "STATR", "DATAR", "CRCR", "RCRCR", "TCRCR",
                             ("I2SCFGR", "SPI_I2S_CFGR"), "HSCR"]),
    "USBHS": ("USBHSD_TypeDef", [
        ("CONTROL", "USB_CTRL"), ("HOST_CTRL", "UHOST_CTRL"), ("INT_EN", "USB_INT_EN"),
        ("DEV_AD", "USB_DEV_AD"), ("FRAME_NO", "USB_FRAME_NO"), ("SUSPEND", "USB_USB_SUSPEND"),
        ("SPEED_TYPE", "USB_SPEED_TYPE"), ("MIS_ST", "USB_MIS_ST"), ("INT_FG", "USB_INT_FG"),
        ("INT_ST", "USB_INT_ST"), ("RX_LEN", "USB_RX_LEN"), ("ENDP_CONFIG", "UEP_CONFIG__UHOST_CTRL"),
        ("ENDP_TYPE", "UEP_TYPE"), ("BUF_MODE", "UEP_BUF_MOD"), "UEP0_DMA",
        "UEP1_RX_DMA", "UEP15_RX_DMA", "UEP1_TX_DMA", "UEP15_TX_DMA",
        "UEP0_MAX_LEN", "UEP1_MAX_LEN", "UEP15_MAX_LEN",
        ("UEP0_TX_LEN", "UEP0_T_LEN"), "UEP0_TX_CTRL", "UEP0_RX_CTRL",
        ("UEP1_TX_LEN", "UEP1_T_LEN"), "UEP1_TX_CTRL", "UEP1_RX_CTRL",
        ("UEP15_TX_LEN", "UEP15_T_LEN"), "UEP15_TX_CTRL", "UEP15_RX_CTRL",
    ]),
}

# Header macro -> (SVD register, SVD field). The macro is a single bit or a mask.
FIELDS = {
    "RCC_HSION": ("CTLR", "HSION"),
    "RCC_HSIRDY": ("CTLR", "HSIRDY"),
    "RCC_HSEON": ("CTLR", "HSEON"),
    "RCC_HSERDY": ("CTLR", "HSERDY"),
    "RCC_PLLON": ("CTLR", "PLLON"),
    "RCC_PLLRDY": ("CTLR", "PLLRDY"),
    "RCC_SW": ("CFGR0", "SW"),
    "RCC_SWS": ("CFGR0", "SWS"),
    "RCC_HPRE": ("CFGR0", "HPRE"),
    "RCC_PPRE1": ("CFGR0", "PPRE1"),
    "RCC_PPRE2": ("CFGR0", "PPRE2"),
    "RCC_PLLSRC": ("CFGR0", "PLLSRC"),
    "RCC_PLLMULL": ("CFGR0", "PLLMUL"),
    "RCC_PREDIV1": ("CFGR2", "PREDIV1"),
    "RCC_PREDIV1SRC": ("CFGR2", "PREDIV1SRC"),
    "RCC_AFIOEN": ("APB2PCENR", "AFIOEN"),
    "RCC_IOPAEN": ("APB2PCENR", "IOPAEN"),
    "RCC_IOPBEN": ("APB2PCENR", "IOPBEN"),
    "RCC_IOPCEN": ("APB2PCENR", "IOPCEN"),
    "RCC_IOPDEN": ("APB2PCENR", "IOPDEN"),
    "RCC_IOPEEN": ("APB2PCENR", "IOPEEN"),
    "RCC_USART1EN": ("APB2PCENR", "USART1EN"),
    "RCC_UART6EN": ("APB1PCENR", "USART6_EN"),
    "RCC_UART7EN": ("APB1PCENR", "USART7_EN"),
    "RCC_UART8EN": ("APB1PCENR", "USART8_EN"),
    "RCC_USART2EN": ("APB1PCENR", "USART2EN"),
    "RCC_USART3EN": ("APB1PCENR", "USART3EN"),
    "RCC_UART4EN": ("APB1PCENR", "UART4EN"),
    "RCC_UART5EN": ("APB1PCENR", "UART5EN"),
    "RCC_USBHSEN": ("AHBPCENR", "USBHS_EN"),
    "RCC_DMA1EN": ("AHBPCENR", "DMA1EN"),
    "RCC_DMA2EN": ("AHBPCENR", "DMA2EN"),
    "RCC_TIM1EN": ("APB2PCENR", "TIM1EN"),
    "RCC_TIM8EN": ("APB2PCENR", "TIM8EN"),
    "RCC_TIM9EN": ("APB2PCENR", "TIM9_EN"),
    "RCC_TIM10EN": ("APB2PCENR", "TIM10_EN"),
    "RCC_TIM2EN": ("APB1PCENR", "TIM2EN"),
    "RCC_TIM3EN": ("APB1PCENR", "TIM3EN"),
    "RCC_TIM4EN": ("APB1PCENR", "TIM4EN"),
    "RCC_TIM5EN": ("APB1PCENR", "TIM5EN"),
    "RCC_TIM6EN": ("APB1PCENR", "TIM6EN"),
    "RCC_TIM7EN": ("APB1PCENR", "TIM7EN"),
    "RCC_SPI1EN": ("APB2PCENR", "SPI1EN"),
    "RCC_SPI2EN": ("APB1PCENR", "SPI2EN"),
    "RCC_SPI3EN": ("APB1PCENR", "SPI3EN"),
    "RCC_TIM1RST": ("APB2PRSTR", "TIM1RST"),
    "RCC_SPI1RST": ("APB2PRSTR", "SPI1RST"),
    "RCC_TIM10RST": ("APB2PRSTR", "TIM10RST"),
    "RCC_SPI3RST": ("APB1PRSTR", "SPI3RST"),
    "RCC_TIM3RST": ("APB1PRSTR", "TIM3RST"),
    "RCC_USBHSDIV": ("CFGR2", "USBHSDIV"),
    "RCC_USBHSPLLSRC": ("CFGR2", "USBHSPLLSRC"),
    "RCC_USBHSCLK": ("CFGR2", "USBHSCLK"),
    "RCC_USBHSPLL": ("CFGR2", "USBHSPLL"),
    "RCC_USBFSSRC": ("CFGR2", "USBFSSRC"),
}

# Same, for the timer bits. TIM1 has all registers of the other timers. A
# third tuple item selects one bit of a wider field (0 is its lowest bit).
TIM_FIELDS = {
    "TIM_CEN": ("CTLR1", "CEN"), "TIM_URS": ("CTLR1", "URS"), "TIM_OPM": ("CTLR1", "OPM"),
    "TIM_DIR": ("CTLR1", "DIR"), "TIM_ARPE": ("CTLR1", "ARPE"),
    "TIM_MMS": ("CTLR2", "MMS"), "TIM_MSM": ("SMCFGR", "MSM"),
    "TIM_UIE": ("DMAINTENR", "UIE"), "TIM_CC1IE": ("DMAINTENR", "CC1IE"), "TIM_TIE": ("DMAINTENR", "TIE"),
    "TIM_UDE": ("DMAINTENR", "UDE"), "TIM_CC1DE": ("DMAINTENR", "CC1DE"),
    "TIM_UIF": ("INTFR", "UIF"), "TIM_CC1IF": ("INTFR", "CC1IF"),
    "TIM_CC2IF": ("INTFR", "CC2IF"), "TIM_CC3IF": ("INTFR", "CC3IF"), "TIM_CC4IF": ("INTFR", "CC4IF"),
    "TIM_COMIF": ("INTFR", "COMIF"), "TIM_TIF": ("INTFR", "TIF"), "TIM_BIF": ("INTFR", "BIF"),
    "TIM_UG": ("SWEVGR", "UG"),
    "TIM_OC1PE": ("CHCTLR1_Output", "OC1PE"), "TIM_OC1M": ("CHCTLR1_Output", "OC1M"),
    "TIM_OC1M_0": ("CHCTLR1_Output", "OC1M", 0), "TIM_OC1M_1": ("CHCTLR1_Output", "OC1M", 1),
    "TIM_OC1M_2": ("CHCTLR1_Output", "OC1M", 2),
    "TIM_CC1E": ("CCER", "CC1E"), "TIM_CC1P": ("CCER", "CC1P"),
    "TIM_CC1NE": ("CCER", "CC1NE"), "TIM_CC1NP": ("CCER", "CC1NP"),
    "TIM_AOE": ("BDTR", "AOE"), "TIM_MOE": ("BDTR", "MOE"),
}

# Same, for the DMA bits. DMA1 channel 1 stands for all channels.
DMA_FIELDS = {
    "DMA_GIF1": ("INTFR", "GIF1"), "DMA_TCIF1": ("INTFR", "TCIF1"),
    "DMA_CHTIF1": ("INTFCR", "CHTIF1"), "DMA_CTEIF1": ("INTFCR", "CTEIF1"),
    "DMA_CFGR1_EN": ("CFGR1", "EN"), "DMA_CFGR1_TCIE": ("CFGR1", "TCIE"),
    "DMA_CFGR1_HTIE": ("CFGR1", "HTIE"), "DMA_CFGR1_TEIE": ("CFGR1", "TEIE"),
    "DMA_CFGR1_DIR": ("CFGR1", "DIR"), "DMA_CFGR1_CIRC": ("CFGR1", "CIRC"),
    "DMA_CFGR1_PINC": ("CFGR1", "PINC"), "DMA_CFGR1_MINC": ("CFGR1", "MINC"),
    "DMA_CFGR1_PSIZE": ("CFGR1", "PSIZE"), "DMA_CFGR1_MSIZE": ("CFGR1", "MSIZE"),
    "DMA_CFGR1_PL": ("CFGR1", "PL"), "DMA_CFGR1_PL_0": ("CFGR1", "PL", 0),
    "DMA_CFGR1_PL_1": ("CFGR1", "PL", 1), "DMA_CFGR1_MEM2MEM": ("CFGR1", "MEM2MEM"),
}

# Same, for the bits of the AFIO_PCFR1 and AFIO_PCFR2 registers. The SW_CFG
# value of the header (100) is a value of the field, not a mask, so it is not
# checked.
AFIO_FIELDS = {
    "AFIO_PCFR1_SPI1_REMAP": ("PCFR1", "SPI1_RM"), "AFIO_PCFR1_I2C1_REMAP": ("PCFR1", "I2C1_RM"),
    "AFIO_PCFR1_USART1_REMAP": ("PCFR1", "USART1_RM"), "AFIO_PCFR1_USART2_REMAP": ("PCFR1", "USART2_RM"),
    "AFIO_PCFR1_USART3_REMAP": ("PCFR1", "USART3_RM"), "AFIO_PCFR1_SWJ_CFG": ("PCFR1", "SW_CFG"),
    "AFIO_PCFR1_SPI3_REMAP": ("PCFR1", "SPI3_RM"),
    "AFIO_PCFR1_TIM2_REMAP": ("PCFR1", "TIM2_RM"), "AFIO_PCFR1_TIM3_REMAP": ("PCFR1", "TIM3_RM"),
    "AFIO_PCFR2_USART7_REMAP": ("PCFR2", "UART7_RM"),
}

# Same, for the bits of the FLASH registers.
FLASH_FIELDS = {
    "FLASH_STATR_BSY": ("STATR", "BSY"), "FLASH_STATR_WRBSY": ("STATR", "WRBSY"),
    "FLASH_STATR_WRPRTERR": ("STATR", "WRPRTERR"), "FLASH_STATR_EOP": ("STATR", "EOP"),
    "FLASH_CTLR_PG": ("CTLR", "PG"), "FLASH_CTLR_STRT": ("CTLR", "STRT"), "FLASH_CTLR_LOCK": ("CTLR", "LOCK"),
    "FLASH_CTLR_FAST_LOCK": ("CTLR", "FLOCK"), "FLASH_CTLR_PAGE_PG": ("CTLR", "FTPG"),
    "FLASH_CTLR_PAGE_ER": ("CTLR", "FTER"), "FLASH_CTLR_PG_STRT": ("CTLR", "PGSTRT"),
}

# Same, for the bits of the USART registers. USART1 stands for all USART blocks.
USART_FIELDS = {
    "USART_STATR_RXNE": ("STATR", "RXNE"), "USART_STATR_TC": ("STATR", "TC"),
    "USART_STATR_TXE": ("STATR", "TXE"),
    "USART_CTLR1_SBK": ("CTLR1", "SBK"), "USART_CTLR1_RE": ("CTLR1", "RE"),
    "USART_CTLR1_TE": ("CTLR1", "TE"), "USART_CTLR1_RXNEIE": ("CTLR1", "RXNEIE"),
    "USART_CTLR1_TCIE": ("CTLR1", "TCIE"), "USART_CTLR1_TXEIE": ("CTLR1", "TXEIE"),
    "USART_CTLR1_PS": ("CTLR1", "PS"), "USART_CTLR1_PCE": ("CTLR1", "PCE"),
    "USART_CTLR1_UE": ("CTLR1", "UE"),
    "USART_CTLR2_STOP": ("CTLR2", "STOP"),
    "USART_CTLR3_DMAR": ("CTLR3", "DMAR"), "USART_CTLR3_DMAT": ("CTLR3", "DMAT"),
}

# Same, for the bits of the SPI registers. SPI1 stands for all SPI blocks.
SPI_FIELDS = {
    "SPI_CTLR1_CPHA": ("CTLR1", "CPHA"), "SPI_CTLR1_CPOL": ("CTLR1", "CPOL"),
    "SPI_CTLR1_MSTR": ("CTLR1", "MSTR"), "SPI_CTLR1_BR": ("CTLR1", "BR"),
    "SPI_CTLR1_BR_0": ("CTLR1", "BR", 0), "SPI_CTLR1_BR_1": ("CTLR1", "BR", 1),
    "SPI_CTLR1_BR_2": ("CTLR1", "BR", 2),
    "SPI_CTLR1_SPE": ("CTLR1", "SPE"), "SPI_CTLR1_LSBFIRST": ("CTLR1", "LSBFIRST"),
    "SPI_CTLR1_SSI": ("CTLR1", "SSI"), "SPI_CTLR1_SSM": ("CTLR1", "SSM"),
    "SPI_CTLR1_RXONLY": ("CTLR1", "RXONLY"), "SPI_CTLR1_DFF": ("CTLR1", "DFF"),
    "SPI_CTLR1_CRCNEXT": ("CTLR1", "CRCNEXT"), "SPI_CTLR1_CRCEN": ("CTLR1", "CRCEN"),
    "SPI_CTLR1_BIDIOE": ("CTLR1", "BIDIOE"), "SPI_CTLR1_BIDIMODE": ("CTLR1", "BIDIMODE"),
    "SPI_CTLR2_RXDMAEN": ("CTLR2", "RXDMAEN"), "SPI_CTLR2_TXDMAEN": ("CTLR2", "TXDMAEN"),
    "SPI_CTLR2_SSOE": ("CTLR2", "SSOE"), "SPI_CTLR2_ERRIE": ("CTLR2", "ERRIE"),
    "SPI_CTLR2_RXNEIE": ("CTLR2", "RXNEIE"), "SPI_CTLR2_TXEIE": ("CTLR2", "TXEIE"),
    "SPI_STATR_RXNE": ("STATR", "RXNE"), "SPI_STATR_TXE": ("STATR", "TXE"),
    "SPI_STATR_CRCERR": ("STATR", "CRCERR"), "SPI_STATR_MODF": ("STATR", "MODF"),
    "SPI_STATR_OVR": ("STATR", "OVR"), "SPI_STATR_BSY": ("STATR", "BSY"),
}

# Same, for the USBHS registers. The SVD names INT_EN bits 5 to 7 wrongly
# (RB_U_1WIRE_MODE, RB_UIE_DEV_NAK at bit 6). The header follows the RM
# (SETUP_ACT, ISO_ACT, DEV_NAK), so these bits are not checked.
USBHS_FIELDS = {
    "USBHS_UC_DMA_EN": ("USB_CTRL", "RB_UC_DMA_EN"), "USBHS_UC_CLR_ALL": ("USB_CTRL", "RB_UC_CLR_ALL"),
    "USBHS_UC_RESET_SIE": ("USB_CTRL", "RB_UC_RST_SIE"), "USBHS_UC_INT_BUSY": ("USB_CTRL", "RB_UC_INT_BUSY"),
    "USBHS_UC_DEV_PU_EN": ("USB_CTRL", "RB_UC_DEV_PU_EN"),
    "USBHS_UC_SPEED_TYPE": ("USB_CTRL", "RB_UC_SPEED_TYPE"),
    "USBHS_UC_HOST_MODE": ("USB_CTRL", "RB_UC_HOST_MODE"),
    "USBHS_UH_PHY_SUSPENDM": ("UHOST_CTRL", "RB_UH_PHY_SUSPENDM"),
    "USBHS_UIE_BUS_RST": ("USB_INT_EN", "RB_UIE_BUS_RST__RB_UIE_DETECT"),
    "USBHS_UIE_TRANSFER": ("USB_INT_EN", "RB_UIE_TRANSFER"),
    "USBHS_UIE_SUSPEND": ("USB_INT_EN", "RB_UIE_SUSPEND"),
    "USBHS_UIE_SOF_ACT": ("USB_INT_EN", "RB_UIE_HST_SOF"),
    "USBHS_UIE_FIFO_OV": ("USB_INT_EN", "RB_UIE_FIFO_OV"),
    "USBHS_MASK_USB_ADDR": ("USB_DEV_AD", "MASK_USB_ADDR"),
    "USBHS_USB_SPEED_TYPE": ("USB_SPEED_TYPE", "USB_SPEED_TYPE"),
    "USBHS_UMS_SUSPEND": ("USB_MIS_ST", "RB_UMS_SUSPEND"),
    "USBHS_UMS_BUS_RESET": ("USB_MIS_ST", "RB_UMS_BUS_RESET"),
    "USBHS_UMS_SIE_FREE": ("USB_MIS_ST", "RB_UMS_SIE_FREE"),
    "USBHS_UIF_BUS_RST": ("USB_INT_FG", "RB_UIF_BUS_RST__RB_UIF_DETECT"),
    "USBHS_UIF_TRANSFER": ("USB_INT_FG", "RB_UIF_TRANSFER"),
    "USBHS_UIF_SUSPEND": ("USB_INT_FG", "RB_UIF_SUSPEND"),
    "USBHS_UIF_HST_SOF": ("USB_INT_FG", "RB_UIF_HST_SOF"),
    "USBHS_UIF_FIFO_OV": ("USB_INT_FG", "RB_UIF_FIFO_OV"),
    "USBHS_UIF_SETUP_ACT": ("USB_INT_FG", "RB_UIF_SETUP_ACT"),
    "USBHS_UIF_ISO_ACT": ("USB_INT_FG", "RB_UIF_ISO_ACT"),
    "USBHS_UIS_ENDP_MASK": ("USB_INT_ST", "MASK_UIS_H_RES__MASK_UIS_ENDP"),
    "USBHS_UIS_TOKEN_MASK": ("USB_INT_ST", "MASK_UIS_TOKEN"),
    "USBHS_UIS_TOG_OK": ("USB_INT_ST", "RB_UIS_TOG_OK"),
    "USBHS_UIS_IS_NAK": ("USB_INT_ST", "RB_UIS_IS_NAK"),
    "USBHS_UEP_T_RES_MASK": ("UEP1_TX_CTRL", "MASK_UEP_T_RES"),
    "USBHS_UEP_T_TOG_MASK": ("UEP1_TX_CTRL", "MASK_UEP_T_TOG"),
    "USBHS_UEP_T_TOG_AUTO": ("UEP1_TX_CTRL", "RB_UEP_T_TOG_AUTO"),
    "USBHS_UEP_R_RES_MASK": ("UEP1_RX_CTRL", "MASK_UEP_R_RES"),
    "USBHS_UEP_R_TOG_MASK": ("UEP1_RX_CTRL", "MASK_UEP_R_TOG"),
    "USBHS_UEP_R_TOG_AUTO": ("UEP1_RX_CTRL", "RB_UEP_R_TOG_AUTO"),
}

# Base addresses in the header -> SVD peripheral.
BASES = {"RCC_BASE": "RCC", "AFIO_BASE": "AFIO", "EXTI_BASE": "EXTI", "GPIOA_BASE": "GPIOA", "GPIOB_BASE": "GPIOB",
         "GPIOC_BASE": "GPIOC", "GPIOD_BASE": "GPIOD", "GPIOE_BASE": "GPIOE",
         "USART1_BASE": "USART1", "USART2_BASE": "USART2", "USART3_BASE": "USART3",
         "UART4_BASE": "UART4", "UART5_BASE": "UART5", "UART6_BASE": "UART6",
         "UART7_BASE": "UART7", "UART8_BASE": "UART8", "USBHS_BASE": "USBHS",
         "TIM1_BASE": "TIM1", "TIM2_BASE": "TIM2", "TIM3_BASE": "TIM3", "TIM4_BASE": "TIM4",
         "TIM5_BASE": "TIM5", "TIM6_BASE": "TIM6", "TIM7_BASE": "TIM7", "TIM8_BASE": "TIM8",
         "TIM9_BASE": "TIM9", "TIM10_BASE": "TIM10", "DMA1_BASE": "DMA1", "DMA2_BASE": "DMA2",
         "FLASH_R_BASE": "FLASH", "SPI1_BASE": "SPI1", "SPI2_BASE": "SPI2", "SPI3_BASE": "SPI3"}

# The SVD lists the DMA channel registers flat (CFGR1, CNTR1, ... CFGR7).
# DMAx_Channel<n>_BASE must point at CFGR<n>, and DMA_Channel_TypeDef must
# match the offsets of channel 1. DMA2 has 11 channels. The SVD has no
# extend flag registers of the channels 8 to 11. The header gives them as
# DMA2_EXTEN, a DMA_TypeDef at DMA2 + 0xD0 (RM, table 11-8).
DMA_CHANNELS = {"DMA1": 7, "DMA2": 11}
DMA_CHANNEL_MEMBERS = [("CNTR", "CNTR1"), ("PADDR", "PADDR1"), ("MADDR", "MADDR1")]


def load_svd():
    root = ET.parse(SVD).getroot()
    periphs = {}
    for p in root.iter("peripheral"):
        periphs[p.findtext("name")] = p
    return periphs


def register_offset(periph, reg_name):
    for reg in periph.iter("register"):
        if reg.findtext("name") == reg_name:
            return int(reg.findtext("addressOffset"), 0)
    raise SystemExit(f"SVD: register {reg_name} not found")


def field_position(periph, reg_name, field_name):
    for reg in periph.iter("register"):
        if reg.findtext("name") != reg_name:
            continue
        for f in reg.iter("field"):
            if f.findtext("name") == field_name:
                bit_range = f.findtext("bitRange")
                if bit_range:  # "[msb:lsb]", used by the USBHS block
                    msb, lsb = (int(v, 0) for v in bit_range.strip("[]").split(":"))
                    return lsb, msb - lsb + 1
                lsb = int(f.findtext("bitOffset") or f.findtext("lsb"), 0)
                width = int(f.findtext("bitWidth"), 0)
                return lsb, width
    raise SystemExit(f"SVD: field {reg_name}.{field_name} not found")


def build_asserts(periphs):
    # wch_device.h first: the checks then see the same macros as a consumer.
    lines = ['#include "wch_device.h"', '#include <ch32v30x_usb.h>', '#include <stddef.h>', '']
    for base_macro, svd_name in BASES.items():
        base = int(periphs[svd_name].findtext("baseAddress"), 0)
        lines.append(f'_Static_assert({base_macro} == 0x{base:08X}UL, "{base_macro} differs from SVD");')
    lines.append("")
    for svd_name, (struct, members) in STRUCTS.items():
        periph = periphs[svd_name]
        for entry in members:
            member, reg = entry if isinstance(entry, tuple) else (entry, entry)
            off = register_offset(periph, reg)
            lines.append(f'_Static_assert(offsetof({struct}, {member}) == 0x{off:02X}, '
                         f'"{struct}.{member} differs from SVD");')
        lines.append("")
    dma1 = periphs["DMA1"]
    for dma_name, channels in DMA_CHANNELS.items():
        dma = periphs[dma_name]
        base = int(dma.findtext("baseAddress"), 0)
        for n in range(1, channels + 1):
            addr = base + register_offset(dma, f"CFGR{n}")
            lines.append(f'_Static_assert({dma_name}_Channel{n}_BASE == 0x{addr:08X}UL, '
                         f'"{dma_name}_Channel{n}_BASE differs from SVD");')
    dma2 = int(periphs["DMA2"].findtext("baseAddress"), 0)
    lines.append(f'_Static_assert(DMA2_EXTEN_BASE == 0x{dma2 + 0xD0:08X}UL, "DMA2_EXTEN_BASE differs from RM");')
    lines.append('_Static_assert(offsetof(SPI_TypeDef, I2SPR) == 0x20, "SPI_TypeDef.I2SPR differs from RM");')
    cfgr1 = register_offset(dma1, "CFGR1")
    for member, reg in DMA_CHANNEL_MEMBERS:
        off = register_offset(dma1, reg) - cfgr1
        lines.append(f'_Static_assert(offsetof(DMA_Channel_TypeDef, {member}) == 0x{off:02X}, '
                     f'"DMA_Channel_TypeDef.{member} differs from SVD");')
    lines.append("")
    checks = [(m, "RCC", rf) for m, rf in FIELDS.items()]
    checks += [(m, "TIM1", rf) for m, rf in TIM_FIELDS.items()]
    checks += [(m, "DMA1", rf) for m, rf in DMA_FIELDS.items()]
    checks += [(m, "AFIO", rf) for m, rf in AFIO_FIELDS.items()]
    checks += [(m, "USART1", rf) for m, rf in USART_FIELDS.items()]
    checks += [(m, "FLASH", rf) for m, rf in FLASH_FIELDS.items()]
    checks += [(m, "SPI1", rf) for m, rf in SPI_FIELDS.items()]
    checks += [(m, "USBHS", rf) for m, rf in USBHS_FIELDS.items()]
    for macro, svd_name, (reg, field, *bit) in checks:
        lsb, width = field_position(periphs[svd_name], reg, field)
        mask = ((1 << width) - 1) << lsb
        if bit:  # one bit of the field
            assert bit[0] < width, f"{macro}: bit {bit[0]} outside {reg}.{field}"
            mask = 1 << (lsb + bit[0])
        lines.append(f'_Static_assert(({macro}) == 0x{mask:08X}UL, "{macro} differs from SVD {reg}.{field}");')
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cc", default="riscv-none-elf-gcc")
    ap.add_argument("--cflags", default="-DCH32V30x_D8C -march=rv32imafc_zicsr_zifencei -mabi=ilp32f")
    args = ap.parse_args()

    periphs = load_svd()
    text = build_asserts(periphs)
    include_dir = os.path.join(ROOT, "src", "families", "ch32v30x")
    with tempfile.TemporaryDirectory() as tmp:
        src = os.path.join(tmp, "check.c")
        with open(src, "w") as f:
            f.write(text)
        cmd = [args.cc, "-std=gnu11", "-fsyntax-only", f"-I{include_dir}",
               f"-I{os.path.join(ROOT, 'src')}"] + shlex.split(args.cflags) + [src]
        result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        print("FAIL: the header differs from the SVD")
        return 1
    print(f"PASS: {text.count('_Static_assert')} checks against {os.path.basename(SVD)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
