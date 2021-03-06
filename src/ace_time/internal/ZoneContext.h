/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_ZONE_CONTEXT_H
#define ACE_TIME_COMMON_ZONE_CONTEXT_H

namespace ace_time {
namespace internal {

/**
 * Metadata about the zone database. A ZoneInfo struct will contain a pointer
 * to this.
 */
struct ZoneContext {
  /** Represents 'w' or wall time. */
  static const uint8_t kSuffixW = 0x00;

  /** Represents 's' or standard time. */
  static const uint8_t kSuffixS = 0x10;

  /** Represents 'u' or UTC time. */
  static const uint8_t kSuffixU = 0x20;

  /*
   * Epoch year. Currently always 2000 but could change in the future. We're
   * leaving this out for now because it's not clear how or if the various
   * AceTime classes can use this information since the value '2000' is often
   * a compile-time constant instead of a runtime constant.
   */
  //int16_t epoch_year;

  /** Start year of the zone files. */
  int16_t startYear;

  /** Until year of the zone files. */
  int16_t untilYear;

  /** TZ Database version which generated the zone info. */
  const char* tzVersion;

  /** Number of fragments. */
  uint8_t numFragments;

  /** Zone Name fragment list. */
  const char* const* fragments;
};

} // internal
} // ace_time

#endif
