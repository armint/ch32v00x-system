
#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace Flash {


class writable_section {
   public:
   static constexpr uint32_t KEY1 = 0x45670123;
   static constexpr uint32_t KEY2 = 0xCDEF89AB;
   writable_section();
   ~writable_section();
};

/**
   Erase a single flash page identified by the pointer to its base address
   Preconditions: Flash must be unlocked (e.g., via Flash::writable_section lifetime)
*/
void erase_page(void *address);

/**
   Program a buffer into Flash starting at address.
   Uses fast page programming (64-byte pages on CH32V00x, 256-byte pages on CH32V20x).
   Data will be padded to the page boundary if needed.
   Preconditions:
   - Flash must be unlocked (e.g., via Flash::writable_section lifetime)
   - address must be 16-byte aligned
*/
bool write_buffer(void *address, std::span<uint8_t> data);
} // namespace Flash