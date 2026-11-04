#pragma once
#include <stdint.h>
#if !defined(APB1PRSTR)
#define APB1PRSTR PB1PRSTR
#endif

#if !defined(APB2PRSTR)
#define APB2PRSTR PB2PRSTR
#endif

#if !defined(AHBPCENR)
#define AHBPCENR HBPCENR
#endif

#if !defined(APB2PCENR)
#define APB2PCENR PB2PCENR
#endif

#if !defined(APB1PCENR)
#define APB1PCENR PB1PCENR
#endif

// The device macro and CH32_FAMILY must name the same family. The guards
// give a clear error when they do not.
#if defined(CH32V006) || defined(CH32V002)
#if !__has_include(<ch32v00X.h>)
#error "CH32V002/CH32V006 needs -DCH32_FAMILY=CH32V00X"
#endif
#include <ch32v00X.h>
#include <system_ch32v00X.h>
#include <vectors_ch32v00X.h>
#elif defined(CH32V003)
#if !__has_include(<ch32v003/ch32v00x.h>)
#error "CH32V003 needs -DCH32_FAMILY=CH32V00X"
#endif
#include <ch32v003/ch32v00x.h>
#include <system_ch32v00X.h>
#include <vectors_ch32v00X.h>
#elif defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6)
#if !__has_include(<ch32v20x.h>)
#error "CH32V20x devices need -DCH32_FAMILY=CH32V20X"
#endif
#include <ch32v20x.h>
#include <system_ch32v20x.h>
#include <vectors_ch32v20x.h>
#else
#error No device defined
#endif
