#ifndef ACE_TIME_TIME_PROVIDER_H
#define ACE_TIME_TIME_PROVIDER_H

#include <stdint.h>
#include "../common/common.h"
#include "../LocalTime.h"

namespace ace_time {
namespace provider {

/**
 * Base class for objects that provide a source of time whose time cannot be
 * changed by the end-user. For example, an NTP client, or a GPS module.
 */
class TimeProvider {
  public:
    static const acetime_t kInvalidSeconds = LocalTime::kInvalidSeconds;

    /** Virtual destructor. Unused except in unit tests. */
    virtual ~TimeProvider() {}

    /**
     * Return the number of seconds since the AceTime epoch
     * (2000-01-01T00:00:00Z). Returns kInvalidSeconds if an error has occured.
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
};

}
}

#endif
