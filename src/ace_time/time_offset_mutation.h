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
 * always incomplete. By extracting them into a separate namespace, we limit
 * the complexity of the TimeOffset class and allow additional mutation methods
 * to be added to this namespace by downstream applications.
 *
 * Example:
 *
 * @code{.cpp}
 * TimeOffset offset(...);
 * time_offset_mutation::incrementHour(offset);
 * @code
 */

/**
 * Increment the TimeOffset by one hour, keeping the minute component
 * unchanged. For usability, limit the hour to [-15, -15].
 * In other words, (UTC+15:45) by one hour wraps to (UTC-15:45).
 */
inline void incrementHour(TimeOffset& offset) {
  int8_t code = offset.toOffsetCode();
  code += 4;
  if (code >= 64) {
    code = -code + 4; // preserve the minute component
  }
  offset.setOffsetCode(code);
}

/**
 * Increment the TimeOffset by one zone (i.e. 15 minutes) keeping the hour
 * component unchanged. If the offsetCode is negative, the cycle looks like:
 * (-01:00, -01:15, -01:30, -01:45, -01:00, ...).
 */
inline void increment15Minutes(TimeOffset& offset) {
  int8_t code = offset.toOffsetCode();
  uint8_t ucode = (code < 0) ? -code : code;
  ucode = (ucode & 0xFC) | (((ucode & 0x03) + 1) & 0x03);
  offset.setOffsetCode((code < 0) ? -ucode : ucode);
}

}
}

#endif
