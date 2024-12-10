/*
 * MIT License
 * Copyright (c) 2024 Brian T. Park
 *
 * Low-level date conversion routines.
 */

#ifndef ACE_TIME_COMMON_DATE_CONV_H
#define ACE_TIME_COMMON_DATE_CONV_H

#include <stdint.h>

namespace ace_time {

/** Convert unsigned secs to hours, minutes, seconds. */
void secondsToHms(uint32_t secs, uint16_t* hh, uint16_t* mm, uint16_t* ss);

}

#endif
