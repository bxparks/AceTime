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
  /**
   * The maximum value of untilYear. This value is used to represent the
   * sentinel value "-" in the UNTIL column of the TZDB files which means
   * "infinity". Must be greater than ZoneRule::kMaxYear which represents the
   * value "max" in the TO and FROM columns of the TZDB files.
   */
  static const int16_t kMaxUntilYear = 32767;

  /**
   * The maximum value fromYear and toYear. This value is used to represent the
   * sentinel value "max" in the TZDB database files. Must be less than
   * ZoneEra::kMaxUntilYear which is used to represent the entry "-" in the
   * UNTIL column of the TZDB files.
   */
  static const int16_t kMaxYear = kMaxUntilYear - 1;

  /**
   * The minimum value of fromYear and toYear. This value is used for ZoneRule
   * entries which are synthetically generated for certain time zones which do
   * not naturally generate a transition for the database year interval
   * specified by the ZoneContext. This value is guaranteed to be earlier than
   * any explicit year in the TZDB database, which guarantees that all time
   * zones have at least one transition.
   */
  static const int16_t kMinYear = -32767;

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

  /** Max number of transitions required in TransitionStorage. */
  int16_t maxTransitions;

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
