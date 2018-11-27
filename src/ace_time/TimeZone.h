#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "UtcOffset.h"

class Print;

namespace ace_time {

/**
 * Base class that describes a time zone. There are 2 subclasses:
 *
 *    - ManualTimeZone represents a fixed offset from UTC, with an optional DST
 *    flag.
 *    - AutoTimeZone represents a time zone described by the TZ Database which
 *    contains rules about when the transition occurs from standard to DST
 *    modes.
 *
 * This class is designed to be created once then shared with various DateTime
 * instances as necessary. The ManualTimeZone class is mutable to allow the
 * user to define a particular UTC offset. The AutoTimeZone is not mutable
 * after a given ZoneInfo is selected in the constructor.
 */
class TimeZone {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeAuto = 1;

    /** Return the type of TimeZone. */
    uint8_t getType() const { return mType; }

    /** Return the effective zone offset. */
    virtual UtcOffset getUtcOffset(uint32_t epochSeconds) const = 0;

    /** Return the abbreviation of the time zone. */
    virtual const char* getAbbrev(uint32_t epochSeconds) const = 0;

    /** Return true if the time zone observes DST at epochSeconds. */
    virtual bool getDst(uint32_t epochSeconds) const = 0;

    /** Print the human readable representation of the time zone. */
    virtual void printTo(Print& printer) const = 0;

  protected:
    friend bool operator==(const TimeZone& a, const TimeZone& b);

    TimeZone(uint8_t type):
        mType(type) {}

    /** Returns true if this object is equal to that object. */
    virtual bool equals(const TimeZone& that) const = 0;

    /** Type of time zone. */
    uint8_t mType;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  return (a.getType() == b.getType()) && a.equals(b);
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
