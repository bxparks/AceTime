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
    /** pollNow() returned valid result. */
    static const uint8_t kStatusOk = 0;

    /** pollNow() timed out. */
    static const uint8_t kStatusTimedOut = 1;

    /** Setup the time keeper. */
    virtual void setup() = 0;

    /**
     * Return the number of seconds since the DateTime epoch (2000-01-01
     * 00:00:00Z).
     */
    virtual uint32_t getNow() const = 0;

    /**
     * Return the current time by polling. First it fires off a request, then
     * returns false. Subsequent calls to this method returns false until the
     * response is ready. When the method returns true, the status is set to
     * kStatusOk, and the seconds parameter is filled.
     *
     * The method will return true if an internal time out limit is reached.
     * In that case, the status will be set to kStatusTimedOut. If the
     * seconds parameter is 0, that also indicates an error.
     *
     * While waiting for the response (i.e. while this method returns false), it
     * must be called more often than every 65.535 seconds because the time out
     * parameter is stored as a uint16_t (to save memory). After the method
     * returns true, it does not need to be called again until another request
     * is needed.
     *
     * This method is designed to be used in the AceRoutine::COROUTINE_AWAIT()
     * macro, but you can call it directly with a suitable while() loop.
     */
    virtual bool pollNow(uint8_t& /*status*/, uint32_t& /*seconds*/) const {
      return true;
    }

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
