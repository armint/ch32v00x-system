#pragma once

#include <wch_device.h>

#if defined(CH32V003)
#include "Adc_V003.h"
#elif defined(CH32V006) || defined(CH32V002)
#include "Adc_V00x.h"
#endif
