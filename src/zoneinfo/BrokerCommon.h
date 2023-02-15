/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKER_COMMON_H
#define ACE_TIME_BROKER_COMMON_H

/**
 * @file BrokerCommon.h
 *
 * Helper functions are used in both Basic brokers and Extended brokers.
 */

#include <stdint.h>

class __FlashStringHelper;

namespace ace_time {
namespace internal {
/**
 * Convert the `deltaCode` in the ZoneInfo or the ZoneRule struct to the actual
 * deltaMinutes. The lower 4-bits stores minutes in units of 15-minutes, shifted
 * by 1h, so can represent the interval [-01:00 to 02:45].
 *
 * @code
 * deltaMinutes = deltaCode * 15m - 1h
 * @endcode
 */
inline int16_t toDeltaMinutes(uint8_t deltaCode) {
  return ((int16_t)(deltaCode & 0x0f) - 4) * 15;
}

/**
 * Convert the `offsetCode` and `deltaCode` into a signed 16-bit integer that
 * represents the UTCOFF of the ZoneEra in minutes. The `offsetCode` is rounded
 * towards -infinity in 15-minute multiples. The upper 4-bits of `deltaCode`
 * holds the (unsigned) remainder in one-minute increments.
 */
inline int16_t toOffsetMinutes(int8_t offsetCode, uint8_t deltaCode) {
  return (offsetCode * 15) + (((uint8_t)deltaCode & 0xf0) >> 4);
}


/**
 * Convert (code, modifier) fields representing the UNTIL time in ZoneInfo or AT
 * time in ZoneRule in one minute resolution. The `code` parameter holds the AT
 * or UNTIL time in minutes component in units of 15 minutes. The lower 4-bits
 * of `modifier` holds the remainder minutes.
 */
inline uint16_t timeCodeToMinutes(uint8_t code, uint8_t modifier) {
  return code * (uint16_t) 15 + (modifier & 0x0f);
}

/**
 * Extract the 'w', 's' 'u' suffix from the 'modifier' field, so that they can
 * be compared against kSuffixW, kSuffixS and kSuffixU. Used for Zone.UNTIL and
 * Rule.AT  fields.
 */
inline uint8_t toSuffix(uint8_t modifier) {
  return modifier & 0xf0;
}

/**
 * Return a pointer to the short name of a full ZoneName. The short name is the
 * last component, which usually begins after the last separator '/'. If the
 * string has been compressed to be compatible with ace_common::KString, then
 * the last component begins just after the last keyword reference (i.e. a
 * non-printable character < ASCII 32). If the fully qualified name has no '/'
 * or a keyword reference, then the short name is the entire string. The last
 * component of the full ZoneName is never compressed, so we do not need to
 * decompress it using ace_common::KString.
 *
 * For example:
 *
 *    * "America/Los_Angeles" returns a pointer to "Los_Angeles",
 *    * "\x01Denver" returns a pointer to "Denver", and
 *    * "UTC" returns "UTC".
 */
const char* findShortName(const char* name);

/**
 * Same as fineShortName(const char*) but for flash strings `(const
 * __FlashStringHelper*)`.
 */
const __FlashStringHelper* findShortName(const __FlashStringHelper* fname);

} // internal
} // ace_time

#endif
