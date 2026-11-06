#pragma once

// Host stand-in for <wch_device.h> of CH32V30X for the Flash driver
// (src/peripherals/Flash.cpp). It has the FLASH and RCC bits of
// src/families/ch32v30x/ch32v30x.h that the driver uses, and SystemCoreClock.
// The FLASH registers are small models of the hardware (RM 32.4 and 32.5):
// - KEYR and MODEKEYR clear LOCK and FLOCK after KEY1 then KEY2. Any other
//   key write, and a key write while unlocked, sets keyError.
// - LOCK and FLOCK in CTLR are write-1 bits: a write of 0 does not clear them.
// - Each read of STATR records the HPRE field of RCC_CFGR0 while an operation
//   bit (PG, PAGE_ER, PAGE_PG) is set, and can raise WRPRTERR (injectWrprtErr).
//   A write of 1 clears a STATR bit.

#include <cstdint>

extern uint32_t SystemCoreClock;

typedef struct {
   volatile uint32_t CFGR0;
} RCC_TypeDef;

extern RCC_TypeDef hostRcc;
#define RCC (&hostRcc)

#define RCC_CFGR0_HPRE_MASK (0xFu << 4)
#define RCC_CFGR0_HPRE_DIV2 (8u << 4)

#define FLASH_STATR_BSY (1u << 0)
#define FLASH_STATR_WRBSY (1u << 1)
#define FLASH_STATR_WRPRTERR (1u << 4)
#define FLASH_STATR_EOP (1u << 5)

#define FLASH_CTLR_PG (1u << 0)
#define FLASH_CTLR_STRT (1u << 6)
#define FLASH_CTLR_LOCK (1u << 7)
#define FLASH_CTLR_FLOCK (1u << 15)
#define FLASH_CTLR_PAGE_PG (1u << 16)
#define FLASH_CTLR_PAGE_ER (1u << 17)
#define FLASH_CTLR_PG_STRT (1u << 21)

struct HostFlash;
extern HostFlash hostFlash;

struct ControlRegister {
   uint32_t value = FLASH_CTLR_LOCK | FLASH_CTLR_FLOCK; // reset value 0x8080
   static constexpr uint32_t setOnly = FLASH_CTLR_LOCK | FLASH_CTLR_FLOCK;
   operator uint32_t() const {
      return value;
   }
   ControlRegister &operator=(uint32_t v) {
      value = (value & setOnly) | v;
      return *this;
   }
   ControlRegister &operator|=(uint32_t v) {
      return *this = value | v;
   }
   ControlRegister &operator&=(uint32_t v) {
      return *this = value & v;
   }
};

struct StatusRegister {
   uint32_t value = 0;
   operator uint32_t();
   StatusRegister &operator=(uint32_t v) {
      value &= ~v;
      return *this;
   }
};

struct KeyRegister {
   uint32_t lockBit;
   int step = 0;
   KeyRegister &operator=(uint32_t key);
};

struct HostFlash {
   uint32_t RESERVED0;
   KeyRegister KEYR{ FLASH_CTLR_LOCK };
   uint32_t OBKEYR;
   StatusRegister STATR;
   ControlRegister CTLR;
   uint32_t ADDR;
   uint32_t RESERVED1;
   uint32_t OBR;
   uint32_t WPR;
   KeyRegister MODEKEYR{ FLASH_CTLR_FLOCK };

   // Test state
   bool keyError = false;
   bool injectWrprtErr = false;
   uint32_t hpreDuringOperation = 0xFFFFFFFFu; // the HPRE field at the last read in an operation
};

#define FLASH (&hostFlash)

inline StatusRegister::operator uint32_t() {
   constexpr uint32_t operations = FLASH_CTLR_PG | FLASH_CTLR_PAGE_ER | FLASH_CTLR_PAGE_PG;
   if((hostFlash.CTLR.value & operations) != 0u) {
      hostFlash.hpreDuringOperation = hostRcc.CFGR0 & RCC_CFGR0_HPRE_MASK;
      if(hostFlash.injectWrprtErr) {
         value |= FLASH_STATR_WRPRTERR;
      }
   }
   return value;
}

inline KeyRegister &KeyRegister::operator=(uint32_t key) {
   if((hostFlash.CTLR.value & lockBit) == 0u) {
      hostFlash.keyError = true; // the driver must not write a key when unlocked
   } else if(step == 0 && key == 0x45670123u) {
      step = 1;
   } else if(step == 1 && key == 0xCDEF89ABu) {
      step = 0;
      hostFlash.CTLR.value &= ~lockBit;
   } else {
      step = 0;
      hostFlash.keyError = true;
   }
   return *this;
}
