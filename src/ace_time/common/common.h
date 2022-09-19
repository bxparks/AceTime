/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_COMMON_H
#define ACE_TIME_COMMON_COMMON_H

#include <stdint.h>

namespace ace_time {

/**
 * Type for the number of seconds from epoch. AceTime epoch is 2000-01-01
 * 00:00:00 UTC by default but can be changed using
 * `LocalDate::localEpochYear()`. Unix epoch is 1970-01-01 00:00:00 UTC.
 */
typedef int32_t acetime_t;

}

#endif
