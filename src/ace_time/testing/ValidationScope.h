/*
 * MIT License
 * Copyright (c) 2020 Brian T. Park
 */

#ifndef ACE_TIME_VALIDATION_SCOPE
#define ACE_TIME_VALIDATION_SCOPE

namespace ace_time {
namespace testing {

/**
 * Enum that controls when a given entry from validation_data.cpp should
 * trigger the DST offset or Abbreviation to be compared against AceTime's
 * version. There are roughly 3 types of entries in validation_data.cpp,
 * given by the 'type' field:
 *
 *    * A, B: transitions caused by a externally-visible change in UTC offset
 *    * a, b: transitions caused by a change in DST offset which isn't normally
 *      visible to the end-user (called "internal")
 *    * S, Y: sample points (S), usually the first of the month) and year-end
 *      (Y) point at the end of the year.
 */
enum class ValidationScope {
  /** Disable validation of DST offset. */
  kNone,

  /**
    * Validate only the externally visible transitions where the UTC offset
    * changes. These transitions are the most reliable from various datetime
    * packages because these are the ones visible to the users. These are
    * marked as type='A' and type='B' in validation_data.json.
    */
  kExternal,

  /**
    * Validate external AND internal transitions. Internal transitions are
    * those where only the DST offset changes, but not the UTC offset. These
    * are not normally visible to the end users, and many datetime libraries
    * seem to have bugs in these. The internal only transitions are marked as
    * type='a' and type='b' in validation_data.json.
    */
  kAll
};

}
}

#endif
