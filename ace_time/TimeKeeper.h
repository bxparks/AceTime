#ifndef ACE_TIME_TIME_KEEPER_H
#define ACE_TIME_TIME_KEEPER_H

#include <stdint.h>
#include "TimeProvider.h"

namespace ace_time {

/**
 * A TimeProvider whose time can be set by the end-user. For example, an RTC
 * chip.
 */
class TimeKeeper: public TimeProvider {
  public:
    /** Set the time to the indicated seconds. */
    virtual void setNow(uint32_t secondsSinceEpoch) = 0;
};

}

#endif
