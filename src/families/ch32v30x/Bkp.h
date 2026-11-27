#pragma once

// The backup data registers of CH32V30X (BKP_DATARx, CH32FV2x_V3xRM chapter 4). They keep their content
// over a system reset and, with VBAT supplied, over a power loss. Each register has 16 bits in a
// 32-bit slot. A typical use is a flag that survives NVIC_SystemReset(), e.g. "enter the
// bootloader".

#include <cstdint>
#include <wch_device.h>

#include "Pwr.h"
#include "peripherals/Rcc.h"

namespace hw::Bkp {

/**
 * @brief Enables the PWR and BKP clocks and allows writes to the backup domain. Reads need only
 * the clocks; call this before the first read or write.
 */
inline void enable() {
   Rcc::enablePWRClock();
   Rcc::enableBKPClock();
   hw::Pwr::enableBackupDomainAccess();
}

/**
 * @brief Locks the backup domain again and stops the PWR and BKP clocks. The data registers keep
 * their content. It also stops the PWR clock and locks the backup domain for other users, for
 * example the RTC.
 */
inline void disable() {
   hw::Pwr::disableBackupDomainAccess();
   Rcc::disableBKPClock();
   Rcc::disablePWRClock();
}

inline uint16_t readDR1() {
   return BKP->DATAR1;
}

/**
 * @brief Writes backup data register 1. Needs enable() first.
 */
inline void writeDR1(uint16_t value) {
   BKP->DATAR1 = value;
}

inline uint16_t readDR2() {
   return BKP->DATAR2;
}

/**
 * @brief Writes backup data register 2. Needs enable() first.
 */
inline void writeDR2(uint16_t value) {
   BKP->DATAR2 = value;
}

} // namespace hw::Bkp
