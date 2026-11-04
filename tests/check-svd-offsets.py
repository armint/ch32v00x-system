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
SVD = os.path.join(ROOT, "etc", "svd", "vendor", "CH32V307xx.svd")

# Struct members that map to SVD registers of the same name.
STRUCTS = {
    "RCC": ("RCC_TypeDef", ["CTLR", "CFGR0", "INTR", "APB2PRSTR", "APB1PRSTR", "AHBPCENR",
                            "APB2PCENR", "APB1PCENR", "BDCTLR", "RSTSCKR", "AHBRSTR", "CFGR2"]),
    "GPIOA": ("GPIO_TypeDef", ["CFGLR", "CFGHR", "INDR", "OUTDR", "BSHR", "BCR", "LCKR"]),
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
}

# Base addresses in the header -> SVD peripheral.
BASES = {"RCC_BASE": "RCC", "GPIOA_BASE": "GPIOA", "GPIOB_BASE": "GPIOB",
         "GPIOC_BASE": "GPIOC", "GPIOD_BASE": "GPIOD", "GPIOE_BASE": "GPIOE"}


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
        for member in members:
            off = register_offset(periph, member)
            lines.append(f'_Static_assert(offsetof({struct}, {member}) == 0x{off:02X}, '
                         f'"{struct}.{member} differs from SVD");')
        lines.append("")
    for macro, (reg, field) in FIELDS.items():
        lsb, width = field_position(periphs["RCC"], reg, field)
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
