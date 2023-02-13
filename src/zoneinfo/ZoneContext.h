/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_CONTEXT_H
#define ACE_TIME_ZONE_CONTEXT_H

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

  /** Start year of the zone files. */
  int16_t startYear;

  /** Until year of the zone files. */
  int16_t untilYear;

  /** TZ Database version which generated the zone info. */
  const char* tzVersion;

  /** Number of fragments. */
  uint8_t numFragments;

  /** Number of fragments. */
  uint8_t numLetters;;

  /** Zone Name fragment list. */
  const char* const* fragments;

  /** Zone Rule letters list. */
  const char* const* letters;
};

} // internal
} // ace_time

#endif
