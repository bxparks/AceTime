/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_OFFSET_MUTATION_H
#define ACE_TIME_TIME_OFFSET_MUTATION_H

#include <stdint.h>
#include "common/util.h"
#include "TimeOffset.h"

namespace ace_time {
namespace time_offset_mutation {

/**
 * @file time_offset_mutation.h
 *
 * Methods that mutate a TimeOffset object.
 *
 * The number of mutation methods of a TimeOffset object is basically unlimited,
 * so including them in the TimeOffset class would make its API too complex and
 * incomplete. Instead, they are split off into this separate namespace.
 *
 * Example:
 *
 * @code{.cpp}
 * TimeOffset offset = TimeOffset::forXxx(...);
 * time_offset_mutation::increment15Minute(offset);
 * @code
 */

/**
 * Increment the TimeOffset by 15 minute interval. For usability, the range is
 * limited from -16:00 to +16:00, inclusive, with +16:00 wrapping to -16:00.
 */
inline void increment15Minutes(TimeOffset& offset) {
  int16_t minutes = offset.toMinutes() + 15;
  if (minutes > 960) minutes = -960; // FIXME: This truncates to 15-minutes
  offset.setMinutes(minutes);
}

}
}

#endif
