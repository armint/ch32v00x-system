#pragma once

#include <stdint.h>
#include <wch_device.h>

// Alternate-function remap and debug port of CH32V30X. A peripheral has a default pin set
// and one or more remapped pin sets, selected in AFIO_PCFR1. All functions
// enable the AFIO clock themselves.
//
// Some remaps need a larger package, and some remaps exist on some chips of
// the family only (see the notes of AFIO_PCFR1 in the reference manual).

namespace hw::Afio {

// Debug port (AFIO_PCFR1.SW_CFG). The chip has no JTAG port: PA15, PB3 and
// PB4 are GPIO after reset.
enum class SwjConfig : uint32_t {
   // SWD on PA13 and PA14 (reset state).
   SwdOnly = 0,
   // SWD off. PA13 and PA14 are GPIO. A debugger cannot connect any more.
   Disabled = AFIO_PCFR1_SW_CFG_DISABLE,
};

void setSwjConfig(SwjConfig config);

/**
 * @brief Writes remap bits in AFIO_PCFR1.
 * @param mask the remap field(s) to change, e.g. AFIO_PCFR1_SPI3_RM.
 *        Must not include AFIO_PCFR1_SW_CFG_MASK; use setSwjConfig().
 * @param value the new field value(s), within mask.
 */
void setRemap(uint32_t mask, uint32_t value);

// SPI1: NSS/SCK/MISO/MOSI on PA4/PA5/PA6/PA7 (default) or PA15/PB3/PB4/PB5.
inline void remapSpi1(bool remap) {
   setRemap(AFIO_PCFR1_SPI1_RM, remap ? AFIO_PCFR1_SPI1_RM : 0);
}

// SPI3: NSS/SCK/MISO/MOSI on PA15/PB3/PB4/PB5 (default) or PA4/PC10/PC11/PC12.
inline void remapSpi3(bool remap) {
   setRemap(AFIO_PCFR1_SPI3_RM, remap ? AFIO_PCFR1_SPI3_RM : 0);
}

// I2C1: SCL/SDA on PB6/PB7 (default) or PB8/PB9.
inline void remapI2c1(bool remap) {
   setRemap(AFIO_PCFR1_I2C1_RM, remap ? AFIO_PCFR1_I2C1_RM : 0);
}

// USART1: TX/RX on PA9/PA10 (default) or PB6/PB7. The chip has more USART1
// maps (AFIO_PCFR2.USART1_RM1). This function keeps that bit at 0.
inline void remapUsart1(bool remap) {
   setRemap(AFIO_PCFR1_USART1_RM, remap ? AFIO_PCFR1_USART1_RM : 0);
}

// USART2: TX/RX on PA2/PA3 (default) or PD5/PD6 (100-pin package only).
inline void remapUsart2(bool remap) {
   setRemap(AFIO_PCFR1_USART2_RM, remap ? AFIO_PCFR1_USART2_RM : 0);
}

enum class Usart3Remap : uint32_t {
   // TX/RX on PB10/PB11
   None = 0,
   // TX/RX on PC10/PC11
   Partial = 1ul << 4,
   // TX/RX on PD8/PD9 (100-pin package only)
   Full = 3ul << 4,
};

inline void remapUsart3(Usart3Remap remap) {
   setRemap(AFIO_PCFR1_USART3_RM_MASK, static_cast<uint32_t>(remap));
}

enum class Tim2Remap : uint32_t {
   // ETR/CH1/CH2/CH3/CH4 on PA0/PA0/PA1/PA2/PA3
   None = 0,
   // ETR/CH1/CH2/CH3/CH4 on PA15/PA15/PB3/PA2/PA3
   Partial1 = 1ul << 8,
   // ETR/CH1/CH2/CH3/CH4 on PA0/PA0/PA1/PB10/PB11
   Partial2 = 2ul << 8,
   // ETR/CH1/CH2/CH3/CH4 on PA15/PA15/PB3/PB10/PB11
   Full = 3ul << 8,
};

inline void remapTim2(Tim2Remap remap) {
   setRemap(AFIO_PCFR1_TIM2_RM_MASK, static_cast<uint32_t>(remap));
}

// The value 01 is reserved. The remap does not move TIM3_ETR (PD2).
enum class Tim3Remap : uint32_t {
   // CH1/CH2/CH3/CH4 on PA6/PA7/PB0/PB1
   None = 0,
   // CH1/CH2/CH3/CH4 on PB4/PB5/PB0/PB1
   Partial = 2ul << 10,
   // CH1/CH2/CH3/CH4 on PC6/PC7/PC8/PC9
   Full = 3ul << 10,
};

inline void remapTim3(Tim3Remap remap) {
   setRemap(AFIO_PCFR1_TIM3_RM_MASK, static_cast<uint32_t>(remap));
}

} // namespace hw::Afio
