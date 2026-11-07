#pragma once
#include <cstddef>
#include <cstdint>
#include "utils/Span.h"

namespace Flash {


/**
   Unlocks the flash controller and its fast mode for the lifetime of the object.
   The destructor locks both again. A new object unlocks again.
*/
class writable_section {
   public:
   static constexpr uint32_t KEY1 = 0x45670123;
   static constexpr uint32_t KEY2 = 0xCDEF89AB;
   writable_section();
   ~writable_section();
};

/**
   Erase a single flash page identified by the pointer to its base address.
   The page is 64 bytes on CH32V00x and 256 bytes on CH32V20x and CH32V30x (fast erase).
   Preconditions: Flash must be unlocked (e.g., via Flash::writable_section lifetime)
*/
void erase_page(void *address);

/**
   Program a buffer into Flash starting at address.
   Uses fast page programming (64-byte pages on CH32V00x, 256-byte pages on CH32V20x and CH32V30x).
   Data will be padded to the page boundary if needed.
   Preconditions:
   - Flash must be unlocked (e.g., via Flash::writable_section lifetime)
   - address must be 16-byte aligned
*/
bool write_buffer(void *address, hw::span<uint8_t> data);

/**
   Program one half-word in standard mode. Defined when the chip header defines
   FLASH_CTLR_PG (CH32V003, CH32V20x, CH32V30x).
   The half-word must be erased. Returns false for an odd address, a write
   protection error, or when the value does not read back.
   Preconditions: Flash must be unlocked (e.g., via Flash::writable_section lifetime)
*/
bool program_halfword(void *address, uint16_t value);
} // namespace Flash
