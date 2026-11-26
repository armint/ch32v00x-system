#pragma once

// The CRC unit of CH32V30X (RM chapter 12): CRC-32 with the polynomial 0x04C11DB7 on 32-bit
// words, from the start value 0xFFFFFFFF, most significant bit first, with no final XOR
// (CRC-32/MPEG-2 on words). Needs the CRC clock (Rcc::enableCRCClock()).

#include <cstdint>
#include <wch_device.h>

namespace hw::Crc {

/**
 * @brief Starts a new calculation: the value is 0xFFFFFFFF again.
 */
inline void reset() {
   CRC->CTLR = CRC_CTLR_RESET;
}

/**
 * @brief Adds one 32-bit word to the calculation.
 */
inline void add(uint32_t word) {
   CRC->DATAR = word;
}

/**
 * @brief The CRC of the words since the last reset().
 */
inline uint32_t value() {
   return CRC->DATAR;
}

} // namespace hw::Crc
