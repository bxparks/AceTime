#ifndef ACE_TIME_TIME_KEEPER_H
#define ACE_TIME_TIME_KEEPER_H

#include <stdint.h>

namespace ace_time {

/**
 * Base class for objects that provide a source of time. For example, a DS3231
 * RTC chip, an NTP client, or a GPS module.
 */
class TimeKeeper {
  public:
    /**
     * Return the number of seconds since the DateTime epoch (2000-01-01
     * 00:00:00Z).
     */
    virtual uint32_t now() const = 0;

    /**
     * Return true if the time keeper is settable. For example, an RTC chip is
     * settable, but an NTP client is not.
     */
    virtual bool isSettable() const = 0;

    /**
     * Set the time to the indicated seconds. Works only if isSettable()
     * returns true, otherwise does nothing.,
     */
    virtual void setNow(uint32_t secondsSinceEpoch) = 0;
};

}

#endif
