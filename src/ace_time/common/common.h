/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_COMMON_H
#define ACE_TIME_COMMON_COMMON_H

#include <stdint.h>

/** Use PROGMEM for BasicZoneSpecifier. */
#define ACE_TIME_USE_PROGMEM_BASIC 0

/** Use PROGMEM for ExtendedZoneSpecifier. */
#define ACE_TIME_USE_PROGMEM_EXTENDED 0

namespace ace_time {

/**
 * Type for the number of seconds from epoch. AceTime epoch is 2000-01-01
 * 00:00:00Z. Unix epoch is 1970-01-01 00:00:00Z.
 */
typedef int32_t acetime_t;

}

#endif
