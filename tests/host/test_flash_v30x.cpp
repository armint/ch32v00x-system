// Host test of the Flash driver on CH32V30x (src/peripherals/Flash.cpp): the unlock and lock
// sequence, the HCLK halving above 120 MHz, and the half-word and page operations. The FLASH
// registers are small models of the hardware (stubs-flash-v30x/), and the flash is host memory.
//
// Not tested: the timing of the flash, the erased value (0xE339 per RM), and if a read after a
// program operation sees the new value. The flash example of ch32v00x-examples checks them on
// the board.

#include "check.h"
#include "peripherals/Flash.h"

#include <wch_device.h>

#include <cstring>

uint32_t SystemCoreClock = 144000000;
RCC_TypeDef hostRcc;
HostFlash hostFlash;

namespace {

constexpr uint32_t locks = FLASH_CTLR_LOCK | FLASH_CTLR_FLOCK;
constexpr uint32_t otherCfgr0Bits = 0x0028040Au; // PLLMUL, PPRE1 and SW bits: must stay

void resetFlash() {
   hostFlash = HostFlash{};
   hostRcc.CFGR0 = otherCfgr0Bits;
}

void testUnlockAndLock() {
   resetFlash();
   {
      Flash::writable_section unlock;
      CHECK((hostFlash.CTLR & locks) == 0u);
   }
   CHECK((hostFlash.CTLR & locks) == locks);
   CHECK(!hostFlash.keyError);

   // A second section unlocks again: the destructor did not lock the controller until reset.
   {
      Flash::writable_section unlock;
      CHECK((hostFlash.CTLR & locks) == 0u);
   }
   CHECK(!hostFlash.keyError);

   // Unlocked already (for example by a debugger): no key write.
   hostFlash.CTLR.value = 0;
   {
      Flash::writable_section unlock;
   }
   CHECK(!hostFlash.keyError);
   CHECK((hostFlash.CTLR & locks) == locks);
}

void testProgramHalfword() {
   resetFlash();
   alignas(4) uint16_t page[4] = { 0xE339, 0xE339, 0xE339, 0xE339 };
   Flash::writable_section unlock;

   CHECK(Flash::program_halfword(&page[1], 0x1234));
   CHECK(page[1] == 0x1234);
   CHECK(page[0] == 0xE339 && page[2] == 0xE339);
   CHECK((hostFlash.CTLR & FLASH_CTLR_PG) == 0u);
   // HCLK = SYSCLK / 2 during the operation, then the old CFGR0 again.
   CHECK(hostFlash.hpreDuringOperation == RCC_CFGR0_HPRE_DIV2);
   CHECK(hostRcc.CFGR0 == otherCfgr0Bits);

   // Odd address: no write.
   auto *const odd = reinterpret_cast<uint8_t *>(&page[2]) + 1;
   CHECK(!Flash::program_halfword(odd, 0x5678));
   CHECK(page[2] == 0xE339);

   // Write protection error: false, and the flag is cleared.
   hostFlash.injectWrprtErr = true;
   CHECK(!Flash::program_halfword(&page[3], 0x5678));
   CHECK((hostFlash.STATR.value & FLASH_STATR_WRPRTERR) == 0u);
   hostFlash.injectWrprtErr = false;
}

void testClockAt120MHz() {
   resetFlash();
   SystemCoreClock = 120000000;
   alignas(4) uint16_t slot = 0xE339;
   {
      Flash::writable_section unlock;
      CHECK(Flash::program_halfword(&slot, 0x00FF));
   }
   CHECK(hostFlash.hpreDuringOperation == 0u);
   CHECK(hostRcc.CFGR0 == otherCfgr0Bits);
   SystemCoreClock = 144000000;
}

void testErasePage() {
   resetFlash();
   alignas(256) static uint8_t page[256];
   {
      Flash::writable_section unlock;
      Flash::erase_page(page);
   }
   CHECK(hostFlash.ADDR == static_cast<uint32_t>(reinterpret_cast<uintptr_t>(page)));
   CHECK((hostFlash.CTLR & FLASH_CTLR_PAGE_ER) == 0u);
   CHECK(hostFlash.hpreDuringOperation == RCC_CFGR0_HPRE_DIV2);
   CHECK(hostRcc.CFGR0 == otherCfgr0Bits);
}

void testWriteBuffer() {
   resetFlash();
   alignas(256) static uint8_t page[256];
   std::memset(page, 0x39, sizeof(page));
   uint8_t data[6] = { 1, 2, 3, 4, 5, 6 };
   {
      Flash::writable_section unlock;
      CHECK(Flash::write_buffer(page, hw::span<uint8_t>(data)));
   }
   CHECK(std::memcmp(page, data, sizeof(data)) == 0);
   // The rest of the 256-byte page is padded with 0xFF.
   bool padded = true;
   for(std::size_t i = sizeof(data); i < sizeof(page); ++i) {
      padded = padded && page[i] == 0xFF;
   }
   CHECK(padded);
   CHECK((hostFlash.CTLR & FLASH_CTLR_PAGE_PG) == 0u);
   CHECK(hostFlash.hpreDuringOperation == RCC_CFGR0_HPRE_DIV2);
   CHECK(hostRcc.CFGR0 == otherCfgr0Bits);
}

} // namespace

int main() {
   testUnlockAndLock();
   testProgramHalfword();
   testClockAt120MHz();
   testErasePage();
   testWriteBuffer();
   return failures;
}
