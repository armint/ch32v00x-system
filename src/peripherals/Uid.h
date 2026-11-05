#pragma once

#include "utils/Span.h"

#include <cstddef>
#include <cstdint>
#include <wch_device.h>

// The 96-bit unique ID of the chip, with the interface of hw::Uid in system-stm32. The ID is in the
// electronic signature area (RM 31.2). A family without UID_BASE (CH32V00X) reads zeros.
namespace hw::Uid {

constexpr size_t SizeBytes = 12u;
constexpr size_t SizeWords = 3u;

/**
 * @brief Returns a 32-bit word from the 96-bit UID (index 0, 1, or 2).
 */
inline uint32_t word(size_t index) {
#if defined(UID_BASE)
   if(index < SizeWords) {
      // volatile: one 32-bit load for each word
      const auto *uid = reinterpret_cast<const volatile uint32_t *>(UID_BASE);
      return uid[index];
   }
#else
   (void)index;
#endif
   return 0u;
}

/**
 * @brief Reads the full 96-bit UID into a caller-supplied 12-byte span.
 * The bytes are in memory order (little endian).
 */
inline void read(hw::span<uint8_t, SizeBytes> dest) {
   for(size_t w = 0u; w < SizeWords; ++w) {
      const uint32_t value = word(w);
      for(size_t b = 0u; b < 4u; ++b) {
         dest[4u * w + b] = static_cast<uint8_t>(value >> (8u * b));
      }
   }
}

/**
 * @brief XOR of the 3 words of the 96-bit UID, for a pseudo-random seed or a hash.
 */
inline uint32_t hash() {
   return word(0) ^ word(1) ^ word(2);
}

} // namespace hw::Uid
