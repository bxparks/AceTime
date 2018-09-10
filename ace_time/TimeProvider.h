#ifndef ACE_TIME_TIME_PROVIDER_H
#define ACE_TIME_TIME_PROVIDER_H

#include <stdint.h>

namespace ace_time {

/**
 * Base class for objects that provide a source of time whose time cannot be
 * changed by the end-user. For example, an NTP client, or a GPS module.
 */
class TimeProvider {
  public:
    /** pollNow() returned valid result. */
    static const uint8_t kStatusOk = 0;

    /** pollNow() timed out. */
    static const uint8_t kStatusTimedOut = 1;

    /** Setup the time keeper. */
    virtual void setup() = 0;

    /**
     * Return the number of seconds since the DateTime epoch
     * (2000-01-01T00:00:00Z). Some TimeProviders will return a 0 to indicate an
     * error has occured.
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
     *
     * The default implementation simply calls the blocking getNow() method.
     */
    virtual bool pollNow(uint8_t& status, uint32_t& seconds) const {
      seconds = getNow();
      status = kStatusOk;
      return true;
    }
};

}

#endif
