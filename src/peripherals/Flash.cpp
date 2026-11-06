
#include "Flash.h"

#include <algorithm>
#include <wch_device.h>

namespace Flash {

#if defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) || defined(CH32V30x_D8C)
constexpr uint32_t PAGE_SIZE = 256;
#else
constexpr uint32_t PAGE_SIZE = 64;
#endif
constexpr uint32_t PAGE_MASK = PAGE_SIZE - 1;

#if defined(FLASH_CTLR_FLOCK)
constexpr uint32_t FAST_LOCK = FLASH_CTLR_FLOCK;
#else
constexpr uint32_t FAST_LOCK = FLASH_CTLR_FAST_LOCK;
#endif

namespace {
#if defined(CH32V30x_D8C)
// RM 32.1, note 2: the flash access clock must not exceed 60 MHz. It is HCLK / 2, so above
// 120 MHz HCLK runs at SYSCLK / 2 for the time of one erase or program operation. The
// peripheral clocks are also halved for that time. The CH32V20x has the same rule: not done yet.
class SlowClock {
   public:
   SlowClock()
      : _saved(RCC->CFGR0) {
      if(SystemCoreClock > 120000000u) {
         RCC->CFGR0 = (_saved & ~RCC_CFGR0_HPRE_MASK) | RCC_CFGR0_HPRE_DIV2;
      }
   }
   ~SlowClock() {
      RCC->CFGR0 = _saved;
   }

   private:
   uint32_t _saved;
};
#else
struct SlowClock {};
#endif

void wait_ready() {
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }
}
} // namespace

writable_section::writable_section() {
   wait_ready();
   // A key write while the controller is unlocked is a wrong sequence: the RM says that it
   // locks the controller until the next reset. Thus write the keys only when locked.
   if(FLASH->CTLR & FLASH_CTLR_LOCK) {
      FLASH->KEYR = KEY1;
      FLASH->KEYR = KEY2;
   }
   // For option bytes.
   //	FLASH->OBKEYR = FLASH_KEY1;
   //	FLASH->OBKEYR = FLASH_KEY2;

   // Fast mode: page erase and page programming.
   if(FLASH->CTLR & FAST_LOCK) {
      FLASH->MODEKEYR = KEY1;
      FLASH->MODEKEYR = KEY2;
   }
}

writable_section::~writable_section() {
   wait_ready();
   FLASH->CTLR |= FAST_LOCK | FLASH_CTLR_LOCK;
}


void erase_page(void *address) {
   [[maybe_unused]] SlowClock slow_clock;

   // 1. Ensure no flash operation ongoing (BSY cleared)
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }

   // 2. Erase Page
   FLASH->CTLR = FLASH_CTLR_PAGE_ER;
   FLASH->ADDR = reinterpret_cast<uintptr_t>(address);
   FLASH->CTLR |= FLASH_CTLR_STRT;

   // 3. Wait for operation to complete
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }

   // 4. Clear PAGE_ER bit
   FLASH->CTLR &= ~FLASH_CTLR_PAGE_ER;
}

bool write_buffer(void *address, hw::span<uint8_t> data) {
   // Preconditions: address must be 16-byte aligned.
   // For fast page programming, we should ideally be on a page boundary.
   if((reinterpret_cast<uintptr_t>(address) & 0xFu) != 0u) {
      return false;
   }

   [[maybe_unused]] SlowClock slow_clock;
   uintptr_t addr = reinterpret_cast<uintptr_t>(address);
   const uint8_t *data_ptr = data.data();
   size_t remaining = data.size();

   // 1. Ensure no flash operation ongoing
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }

   // Initialize the first page
   FLASH->CTLR = FLASH_CTLR_PAGE_PG;
#ifdef FLASH_CTLR_BUF_RST
   FLASH->CTLR |= FLASH_CTLR_BUF_RST;
#endif
   FLASH->ADDR = addr;
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }

   while(remaining > 0 || (addr & PAGE_MASK) != 0) {
      // 2. Prepare 4 bytes to write
      uint32_t word = 0xFFFFFFFF;
      size_t to_copy = std::min(remaining, (size_t)4);
      if(to_copy > 0) {
         uint8_t *word_bytes = reinterpret_cast<uint8_t *>(&word);
         for(size_t i = 0; i < to_copy; ++i) {
            word_bytes[i] = data_ptr[i];
         }
         data_ptr += to_copy;
         remaining -= to_copy;
      }

      // 3. Load word into buffer
      *reinterpret_cast<volatile uint32_t *>(addr) = word;
#ifdef FLASH_STATR_WRBSY
      while(FLASH->STATR & FLASH_STATR_WRBSY) {
      }
#endif
#ifdef FLASH_CTLR_BUF_LOAD
      FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_BUF_LOAD;
      while(FLASH->STATR & FLASH_STATR_BSY) {
      }
#endif

      addr += 4;

      // 4. If we reached the end of a page, or no more data (and we want to finish the page)
      if((addr & PAGE_MASK) == 0) {
#ifdef FLASH_CTLR_PG_STRT
         FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_PG_STRT;
#else
         FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_STRT;
#endif
         while(FLASH->STATR & FLASH_STATR_BSY) {
         }

         if(remaining > 0) {
            // Prepare for next page
            FLASH->CTLR = FLASH_CTLR_PAGE_PG;
#ifdef FLASH_CTLR_BUF_RST
            FLASH->CTLR |= FLASH_CTLR_BUF_RST;
#endif
            FLASH->ADDR = addr;
            while(FLASH->STATR & FLASH_STATR_BSY) {
            }
         } else {
            break;
         }
      }
   }

   // Check EOP and clear it by software
   if(FLASH->STATR & FLASH_STATR_EOP) {
      FLASH->STATR = FLASH_STATR_EOP;
   }

   // 8. Clear PG when finished
   FLASH->CTLR &= ~FLASH_CTLR_PAGE_PG;
   return true;
}
} // namespace Flash
