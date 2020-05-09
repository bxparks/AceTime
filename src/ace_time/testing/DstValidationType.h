/*
 * MIT License
 * Copyright (c) 2020 Brian T. Park
 */

#ifndef ACE_TIME_DST_VALIDATION_TYPE
#define ACE_TIME_DST_VALIDATION_TYPE

namespace ace_time {
namespace testing {

enum class DstValidationType {
  /** Disable validaiton of DST offset. */
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
