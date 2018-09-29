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
    /** Virtual destructor. Unused except in unit tests. */
    virtual ~TimeProvider() {}

    /**
     * Return the number of seconds since the DateTime epoch
     * (2000-01-01T00:00:00Z). Some TimeProviders will return a 0 to indicate an
     * error has occured.
     */
    virtual uint32_t getNow() const = 0;

    /** Send a time request asynchronously. Used by SystemTimeSyncCoroutine. */
    virtual void sendRequest() const {}

    /** Return true if a response is ready. Used by SystemTimeSyncCoroutine. */
    virtual bool isResponseReady() const { return true; }

    /**
     * Returns number of seconds since AceTime epoch (2000-01-01). Return 0 if
     * there is an error. Valid only if isResponseReady() returns true. Used by
     * SystemTimeSyncCoroutine.
     */
    virtual uint32_t readResponse() const { return getNow(); }
};

}

#endif
