
#include "Flash.h"

#include <wch_device.h>

namespace Flash {
writable_section::writable_section() {
   while(FLASH->STATR & FLASH_STATR_BSY) {
      // Wait while the flash is busy
   }


   FLASH->KEYR = KEY1;
   FLASH->KEYR = KEY2;
   // For option bytes.
   //	FLASH->OBKEYR = FLASH_KEY1;
   //	FLASH->OBKEYR = FLASH_KEY2;

   // For unlocking programming, in general.
   FLASH->MODEKEYR = FLASH_MODEKEYR_MODEKEYR1;
   FLASH->MODEKEYR = FLASH_MODEKEYR_MODEKEYR2;
}

writable_section::~writable_section() {
   while(FLASH->STATR & FLASH_STATR_BSY) {
      // Wait while the flash is busy
   }
   FLASH->KEYR = 0;
}


void erase_page(void *address) {

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

bool write_buffer(void *address, std::span<uint8_t> data) {
   // Preconditions: address must be 16-byte aligned.
   // For 64-byte page programming, we should ideally be on a 64-byte boundary.
   if((reinterpret_cast<uintptr_t>(address) & 0xFu) != 0u) {
      return false;
   }

   uintptr_t addr = reinterpret_cast<uintptr_t>(address);
   const uint8_t *data_ptr = data.data();
   size_t remaining = data.size();

   // 1. Ensure no flash operation ongoing
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }

   // Initialize the first page
   FLASH->CTLR = FLASH_CTLR_PAGE_PG;
   FLASH->CTLR |= FLASH_CTLR_BUF_RST;
   FLASH->ADDR = addr;
   while(FLASH->STATR & FLASH_STATR_BSY) {
   }

   while(remaining > 0 || (addr & 0x3F) != 0) {
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
      FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_BUF_LOAD;
      while(FLASH->STATR & FLASH_STATR_BSY) {
      }

      addr += 4;

      // 4. If we reached the end of a 64-byte page, or no more data (and we want to finish the page)
      if((addr & 0x3F) == 0) {
         FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_STRT;
         while(FLASH->STATR & FLASH_STATR_BSY) {
         }

         if(remaining > 0) {
            // Prepare for next page
            FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_BUF_RST;
            FLASH->ADDR = addr;
            while(FLASH->STATR & FLASH_STATR_BSY) {
            }
         } else {
            break;
         }
      }
   }

   // 5. If we finished all data but didn't reach a 64-byte boundary, we must finish the page
   if((addr & 0x3F) != 0) {
      while((addr & 0x3F) != 0) {
         *reinterpret_cast<volatile uint32_t *>(addr) = 0xFFFFFFFF;
         FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_BUF_LOAD;
         while(FLASH->STATR & FLASH_STATR_BSY) {
         }
         addr += 4;
      }
      FLASH->CTLR = FLASH_CTLR_PAGE_PG | FLASH_CTLR_STRT;
      while(FLASH->STATR & FLASH_STATR_BSY) {
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