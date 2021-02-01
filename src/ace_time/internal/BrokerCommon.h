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

/** Convert (timeCode, timeModifier) fields in ZoneInfo to minutes. */
inline uint16_t timeCodeToMinutes(uint8_t code, uint8_t modifier) {
  return code * (uint16_t) 15 + (modifier & 0x0f);
}

/**
 * Extract the 'w', 's' 'u' suffix from the 'modifier' field, so that they can
 * be compared against kSuffixW, kSuffixS and kSuffixU.
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
