#pragma once

// The backup domain write protection of CH32V30X (PWR_CTLR.DBP, CH32FV2x_V3xRM chapter 2).

#include <wch_device.h>

namespace hw::Pwr {

/**
 * @brief Allows writes to the backup domain: the BKP data registers, RCC_BDCTLR and the RTC.
 * Needs the PWR clock (Rcc::enablePWRClock()).
 */
inline void enableBackupDomainAccess() {
   PWR->CTLR |= PWR_CTLR_DBP;
}

inline void disableBackupDomainAccess() {
   PWR->CTLR &= ~PWR_CTLR_DBP;
}

inline bool isBackupDomainAccessEnabled() {
   return (PWR->CTLR & PWR_CTLR_DBP) != 0;
}

} // namespace hw::Pwr
