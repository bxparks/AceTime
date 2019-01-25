#ifndef ACE_TIME_UTC_OFFSET_MUTATION_H
#define ACE_TIME_UTC_OFFSET_MUTATION_H

#include <stdint.h>
#include "common/util.h"
#include "UtcOffset.h"

namespace ace_time {
namespace utc_offset_mutation {

/**
 * @file utc_offset_mutation.h
 *
 * Methods that mutate a UtcOffset object.
 *
 * The number of mutation methods of a UtcOffset object is basically unlimited,
 * so including them in the UtcOffset class would make its API too complex and
 * always incomplete. By extracting them into a separate namespace, we limit
 * the complexity of the UtcOffset class and allow additional mutation methods
 * to be added to this namespace by downstream applications.
 *
 * Example:
 *
 * @code{.cpp}
 * UtcOffset offset(...);
 * utc_offset_mutation::incrementHour(offset);
 * @code
 */

/**
 * Increment the UtcOffset by one hour, keeping the minute component
 * unchanged. For usability, limit the hour to [-15, -15].
 * In other words, (UTC+15:45) by one hour wraps to (UTC-15:45).
 */
inline void incrementHour(UtcOffset& offset) {
  int8_t code = offset.code();
  code += 4;
  if (code >= 64) {
    code = -code + 4; // preserve the minute component
  }
  offset.code(code);
}

/**
 * Increment the UtcOffset by one zone (i.e. 15 minutes) keeping the hour
 * component unchanged. If the offsetCode is negative, the cycle looks like:
 * (-01:00, -01:15, -01:30, -01:45, -01:00, ...).
 */
inline void increment15Minutes(UtcOffset& offset) {
  int8_t code = offset.code();
  uint8_t ucode = (code < 0) ? -code : code;
  ucode = (ucode & 0xFC) | (((ucode & 0x03) + 1) & 0x03);
  offset.code((code < 0) ? -ucode : ucode);
}

}
}

#endif
