/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_CLOCK_H
#define ACE_TIME_CLOCK_H

#include <stdint.h>
#include "../common/common.h"
#include "../LocalTime.h"

namespace ace_time {
namespace clock {

/**
 * Base class for objects that provide and store time. For example, a DS3231
 * RTC chip, an NTP client, or a GPS module.
 */
class Clock {
  public:
    static const acetime_t kInvalidSeconds = LocalTime::kInvalidSeconds;

    /** Virtual destructor. Unused except in unit tests. */
    virtual ~Clock() {}

    /**
     * Return the number of seconds since the AceTime epoch
     * (2000-01-01T00:00:00Z). Returns kInvalidSeconds if an error has occured.
     *
     * This is a blocking call. Some clocks (e.g. NTP client) this may take
     * many seconds. On those clocks, use the asynchronous methods
     * (sendRequest(), isResponseReady(), and readResponse()) instead.
     */
    virtual acetime_t getNow() const = 0;

    /** Send a time request asynchronously. Used by SystemClockSyncCoroutine. */
    virtual void sendRequest() const {}

    /** Return true if a response is ready. Used by SystemClockSyncCoroutine. */
    virtual bool isResponseReady() const { return true; }

    /**
     * Returns number of seconds since AceTime epoch (2000-01-01). Return
     * kInvalidSeconds if there is an error. Valid only if isResponseReady()
     * returns true. Used by SystemClockSyncCoroutine.
     */
    virtual acetime_t readResponse() const { return getNow(); }

    /**
     * Set the time to the indicated seconds. Calling with a value of
     * kInvalidSeconds indicates an error condition, so the method should do
     * nothing. Some clocks do not support this feature, for example, NTP or
     * GPS clocks and this method will be a no-op.
     */
    virtual void setNow(acetime_t /*epochSeconds*/) {}
};

}
}

#endif
