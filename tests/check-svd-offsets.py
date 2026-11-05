#!/usr/bin/env python3
"""Check the hand-written CH32V30x device header against the vendor SVD.

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
    "AFIO": ("AFIO_TypeDef", ["ECR", "PCFR1", ("EXTICR[0]", "EXTICR1"), ("EXTICR[1]", "EXTICR2"),
                              ("EXTICR[2]", "EXTICR3"), ("EXTICR[3]", "EXTICR4"), "PCFR2"]),
    "EXTI": ("EXTI_TypeDef", ["INTENR", "EVENR", "RTENR", "FTENR", "SWIEVR", "INTFR"]),
    "USART1": ("USART_TypeDef", ["STATR", "DATAR", "BRR", "CTLR1", "CTLR2", "CTLR3", "GPR"]),
    "USBHS": ("USBHS_TypeDef", [
        ("CTRL", "USB_CTRL"), ("HOST_CTRL", "UHOST_CTRL"), ("INT_EN", "USB_INT_EN"),
        ("DEV_AD", "USB_DEV_AD"), ("FRAME_NO", "USB_FRAME_NO"), ("SUSPEND", "USB_USB_SUSPEND"),
        ("SPEED_TYPE", "USB_SPEED_TYPE"), ("MIS_ST", "USB_MIS_ST"), ("INT_FG", "USB_INT_FG"),
        ("INT_ST", "USB_INT_ST"), ("RX_LEN", "USB_RX_LEN"), ("UEP_CONFIG", "UEP_CONFIG__UHOST_CTRL"),
        ("UEP_TYPE", "UEP_TYPE"), ("UEP_BUF_MOD", "UEP_BUF_MOD"), ("UEP0_DMA", "UEP0_DMA"),
        ("UEP_RX_DMA[0]", "UEP1_RX_DMA"), ("UEP_RX_DMA[14]", "UEP15_RX_DMA"),
        ("UEP_TX_DMA[0]", "UEP1_TX_DMA"), ("UEP_TX_DMA[14]", "UEP15_TX_DMA"),
        ("UEP_MAX_LEN[0].MAX_LEN", "UEP0_MAX_LEN"), ("UEP_MAX_LEN[1].MAX_LEN", "UEP1_MAX_LEN"),
        ("UEP_MAX_LEN[15].MAX_LEN", "UEP15_MAX_LEN"),
        ("UEP[0].T_LEN", "UEP0_T_LEN"), ("UEP[0].TX_CTRL", "UEP0_TX_CTRL"), ("UEP[0].RX_CTRL", "UEP0_RX_CTRL"),
        ("UEP[1].T_LEN", "UEP1_T_LEN"), ("UEP[1].TX_CTRL", "UEP1_TX_CTRL"), ("UEP[1].RX_CTRL", "UEP1_RX_CTRL"),
        ("UEP[15].T_LEN", "UEP15_T_LEN"), ("UEP[15].TX_CTRL", "UEP15_TX_CTRL"),
        ("UEP[15].RX_CTRL", "UEP15_RX_CTRL"),
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
    "RCC_CFGR0_SW_MASK": ("CFGR0", "SW"),
    "RCC_CFGR0_SWS_MASK": ("CFGR0", "SWS"),
    "RCC_CFGR0_HPRE_MASK": ("CFGR0", "HPRE"),
    "RCC_CFGR0_PPRE1_MASK": ("CFGR0", "PPRE1"),
    "RCC_CFGR0_PPRE2_MASK": ("CFGR0", "PPRE2"),
    "RCC_CFGR0_PLLSRC": ("CFGR0", "PLLSRC"),
    "RCC_CFGR0_PLLMUL_MASK": ("CFGR0", "PLLMUL"),
    "RCC_CFGR2_PREDIV1_MASK": ("CFGR2", "PREDIV1"),
    "RCC_CFGR2_PREDIV1SRC": ("CFGR2", "PREDIV1SRC"),
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
    "RCC_CFGR2_USBHSDIV_MASK": ("CFGR2", "USBHSDIV"),
    "RCC_CFGR2_USBHSPLLSRC": ("CFGR2", "USBHSPLLSRC"),
    "RCC_CFGR2_USBHSCLK_MASK": ("CFGR2", "USBHSCLK"),
    "RCC_CFGR2_USBHSPLL": ("CFGR2", "USBHSPLL"),
    "RCC_CFGR2_USBFSSRC": ("CFGR2", "USBFSSRC"),
}

# Same, for the bits of the AFIO_PCFR1 register. The SW_CFG value of the
# header (100) is a value of the field, not a mask, so it is not checked.
AFIO_FIELDS = {
    "AFIO_PCFR1_SPI1_RM": ("PCFR1", "SPI1_RM"), "AFIO_PCFR1_I2C1_RM": ("PCFR1", "I2C1_RM"),
    "AFIO_PCFR1_USART1_RM": ("PCFR1", "USART1_RM"), "AFIO_PCFR1_USART2_RM": ("PCFR1", "USART2_RM"),
    "AFIO_PCFR1_USART3_RM_MASK": ("PCFR1", "USART3_RM"), "AFIO_PCFR1_SW_CFG_MASK": ("PCFR1", "SW_CFG"),
    "AFIO_PCFR1_SPI3_RM": ("PCFR1", "SPI3_RM"),
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

# Same, for the USBHS registers. The SVD names INT_EN bits 5 to 7 wrongly
# (RB_U_1WIRE_MODE, RB_UIE_DEV_NAK at bit 6). The header follows the RM
# (SETUP_ACT, ISO_ACT, DEV_NAK), so these bits are not checked.
USBHS_FIELDS = {
    "USBHS_UC_DMA_EN": ("USB_CTRL", "RB_UC_DMA_EN"), "USBHS_UC_CLR_ALL": ("USB_CTRL", "RB_UC_CLR_ALL"),
    "USBHS_UC_RST_SIE": ("USB_CTRL", "RB_UC_RST_SIE"), "USBHS_UC_INT_BUSY": ("USB_CTRL", "RB_UC_INT_BUSY"),
    "USBHS_UC_DEV_PU_EN": ("USB_CTRL", "RB_UC_DEV_PU_EN"),
    "USBHS_UC_SPEED_MASK": ("USB_CTRL", "RB_UC_SPEED_TYPE"),
    "USBHS_UC_HOST_MODE": ("USB_CTRL", "RB_UC_HOST_MODE"),
    "USBHS_UH_PHY_SUSPENDM": ("UHOST_CTRL", "RB_UH_PHY_SUSPENDM"),
    "USBHS_UIE_BUS_RST": ("USB_INT_EN", "RB_UIE_BUS_RST__RB_UIE_DETECT"),
    "USBHS_UIE_TRANSFER": ("USB_INT_EN", "RB_UIE_TRANSFER"),
    "USBHS_UIE_SUSPEND": ("USB_INT_EN", "RB_UIE_SUSPEND"),
    "USBHS_UIE_SOF_ACT": ("USB_INT_EN", "RB_UIE_HST_SOF"),
    "USBHS_UIE_FIFO_OV": ("USB_INT_EN", "RB_UIE_FIFO_OV"),
    "USBHS_DEV_AD_MASK": ("USB_DEV_AD", "MASK_USB_ADDR"),
    "USBHS_SPEED_TYPE_MASK": ("USB_SPEED_TYPE", "USB_SPEED_TYPE"),
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
         "UART7_BASE": "UART7", "UART8_BASE": "UART8", "USBHS_BASE": "USBHS"}


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
    lines = ['#include "wch_device.h"', '#include <stddef.h>', '']
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
    checks = [(m, "RCC", rf) for m, rf in FIELDS.items()]
    checks += [(m, "AFIO", rf) for m, rf in AFIO_FIELDS.items()]
    checks += [(m, "USART1", rf) for m, rf in USART_FIELDS.items()]
    checks += [(m, "USBHS", rf) for m, rf in USBHS_FIELDS.items()]
    for macro, svd_name, (reg, field) in checks:
        lsb, width = field_position(periphs[svd_name], reg, field)
        mask = ((1 << width) - 1) << lsb
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
