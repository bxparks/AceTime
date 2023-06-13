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
namespace zoneinfo {

/**
 * Size of the c-string buffer needed to hold a time zone abbreviation.
 * Longest abbreviation currently seems to be 5 characters
 * (https://www.timeanddate.com/time/zones/) but the TZ database spec says
 * that abbreviations are 3 to 6 characters
 * (https://data.iana.org/time-zones/theory.html#abbreviations), so use 6 as
 * the maximum. Plus one for the terminating NUL character.
 */
const int kAbbrevSize = 6 + 1;

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

} // zoneinfo
} // ace_time

#endif
