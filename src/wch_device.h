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

#if defined(CH32V006) || defined(CH32V002)
#include <ch32v00X.h>
#include <system_ch32v00X.h>
#include <vectors_ch32v00X.h>
#elif defined(CH32V003)
#include <ch32v003/ch32v00x.h>
#include <system_ch32v00X.h>
#include <vectors_ch32v00X.h>
#elif defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6)
#include <ch32v20x.h>
#include <system_ch32v20x.h>
#include <vectors_ch32v20x.h>
#else
#error No device defined
#endif
