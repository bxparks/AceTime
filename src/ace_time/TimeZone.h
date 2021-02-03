/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "TimeOffset.h"
#include "ZoneProcessor.h"
#include "BasicZoneProcessor.h"
#include "ExtendedZoneProcessor.h"
#include "BasicZone.h"
#include "ExtendedZone.h"
#include "TimeZoneData.h"

class Print;

namespace ace_time {

/**
 * Class that describes a time zone. There are 2 colloquial usages of "time
 * zone". The first refers to a simple fixed offset from UTC. For example, we
 * may say that "we are in -05:00 time zone". The second is a geographical
 * region that obeys a consistent set of rules regarding the value of the UTC
 * offset, and when the transitions to DST happens (if at all). The best known
 * source of these geographical regions is the TZ Database maintained by IANA
 * (https://www.iana.org/time-zones). The TimeZone class supports both meanings.
 *
 * There are 4 types of TimeZone:
 *
 *    * kTypeError: represents an error or unknown time zone
 *    * kTypeManual: holds a base offset and a DST offset, and
 *      allows the user to modify both of these fields
 *    * kTypeBasic: using an underlying BasicZoneProcessor which
 *      supports subset of zones and links defined by `zonedb/zone_infos.h`
 *    * kTypeExtended: a time zone using an underlying ExtendedZoneProcessor
 *      which supports all zones and links defined by `zonedbx/zone_infos.h`,
 *      essentially the entire IANA TZ database
 *
 * The TimeZone class should be treated as a const value type. (Except for
 * kTypeManual which is self-contained and allows the stdOffset and dstOffset
 * to be modified.) It can be passed around by value, but since it is between 5
 * bytes (8-bit processors) and 12 bytes (32-bit processors) big, it may be
 * slightly more efficient to pass by const reference, then save locally
 * by-value when needed. The ZonedDateTime holds the TimeZone object by-value.
 *
 * Semantically, TimeZone really really wants to be a reference type because it
 * needs have a reference to the ZoneProcessor helper class to do its work. In
 * other words, it would be very convenient if the client code could create
 * this object on the heap, and pass it around using a smart pointer to the
 * ZonedDateTime class and shared among multiple ZonedDateTime objects. This
 * would also allow new TimeZones to be created, while allowing older instances
 * of ZonedDateTime to hold on to the previous versions of TimeZone.
 *
 * However, in a small memory embedded environment (like Arduino Nano or Micro
 * with only 2kB of RAM), I want to avoid any use of the heap (new operator or
 * malloc()) inside the AceTime library. I separated out the memory intensive
 * or mutable features of the TimeZone class into the separate ZoneProcessor
 * class. The ZoneProcessor object should be created once at initialization
 * time of the application (either statically allocated or potentially on the
 * heap early in the application start up).
 *
 * An alternative implementation would use an inheritance hierarchy for the
 * TimeZone with subclasses like ManualTimeZone, BasicTimeZone and
 * ExtendedTimeZone. However since different subclasses are of different sizes,
 * the TimeZone object can no longer be passed around by-value, so the
 * ZonedDateTime is forced to hold on to the TimeZone object using a pointer.
 * Then we are forced to deal with difficult memory management and life cycle
 * problems. Using a single TimeZone class and implementing it as a value type
 * simplifies a lot of code.
 *
 * The object can be serialized using the TimeZone::toTimeZoneData() method,
 * and reconstructed using the ZoneManager::createForTimeZoneData() method.
 */
class TimeZone {
  public:
    static const uint8_t kTypeError = 0;
    static const uint8_t kTypeManual = 1;
    static const uint8_t kTypeBasic = ZoneProcessor::kTypeBasic;
    static const uint8_t kTypeExtended = ZoneProcessor::kTypeExtended;

    /** Factory method to create a UTC TimeZone. */
    static TimeZone forUtc() {
      return TimeZone();
    }

    /**
     * Factory method to create from a UTC offset and an optional DST offset.
     * It may be easier to use the following convenience methods:
     *
     * * TimeZone::forHours(stdHours, dstHours = 0)
     * * TimeZone::forMinutes(stdMinutes, dstMinutes = 0)
     * * TimeZone::forHourMinute(stdHour, stdMinute, dstHour = 0, dstMinute = 0)
     *
     * This method may become deprecated in the future.
     *
     * @param stdOffset the base offset
     * @param dstOffset the DST offset, default TimeOffset() (i.e. 0 offset)
     */
    static TimeZone forTimeOffset(
        TimeOffset stdOffset,
        TimeOffset dstOffset = TimeOffset()
    ) {
      return TimeZone(stdOffset, dstOffset);
    }

    /**
     * Factory method to create from UTC hour offset and optional DST hour
     * offset. This is a convenience alternative to
     * `forTimeOffset(TimeOffset::forHours(stdHour),
     * TimeOffset::forHours(stdHour))`.
     */
    static TimeZone forHours(int8_t stdHours, int8_t dstHours = 0) {
      return TimeZone::forTimeOffset(
          TimeOffset::forHours(stdHours),
          TimeOffset::forHours(dstHours)
      );
    }

    /**
     * Factory method to create from UTC minute offset and optional DST minute
     * offset. This is a convenience alternative to
     * `forTimeOffset(TimeOffset::forMinutes(stdMinutes),
     * TimeOffset::forMinutes(dstMinutes))`.
     */
    static TimeZone forMinutes(int8_t stdMinutes, int8_t dstMinutes = 0) {
      return TimeZone::forTimeOffset(
          TimeOffset::forMinutes(stdMinutes),
          TimeOffset::forMinutes(dstMinutes)
      );
    }

    /**
     * Factory method to create from UTC (hour, minute) pair and optional DST
     * (hour, minute) pair. This is a convenience alternative to
     * `forTimeOffset(TimeOffset::forHour(stdHour),
     * TimeOffset::forHour(stdHour))`.
     */
    static TimeZone forHourMinute(
        int8_t stdHour,
        int8_t stdMinute,
        int8_t dstHour = 0,
        int8_t dstMinute = 0
    ) {
      return TimeZone::forTimeOffset(
          TimeOffset::forHourMinute(stdHour, stdMinute),
          TimeOffset::forHourMinute(dstHour, dstMinute)
      );
    }

    /**
     * Factory method to create from a zoneInfo and an associated
     * BasicZoneProcessor. The ZoneInfo previously associated with the
     * given zoneProcessor is overridden.
     *
     * @param zoneInfo a basic::ZoneInfo that identifies the zone
     * @param zoneProcessor a pointer to a ZoneProcessor, cannot be nullptr
     */
    static TimeZone forZoneInfo(
        const basic::ZoneInfo* zoneInfo,
        BasicZoneProcessor* zoneProcessor
    ) {
      return TimeZone(kTypeBasic, zoneInfo, zoneProcessor);
    }

    /**
     * Factory method to create from a zoneInfo and an associated
     * ExtendedZoneProcessor. The ZoneInfo previously associated with the
     * given zoneProcessor is overridden.
     *
     * @param zoneInfo an extended::ZoneInfo that identifies the zone
     * @param zoneProcessor a pointer to a ZoneProcessor, cannot be nullptr
     */
    static TimeZone forZoneInfo(
        const extended::ZoneInfo* zoneInfo,
        ExtendedZoneProcessor* zoneProcessor
    ) {
      return TimeZone(kTypeExtended, zoneInfo, zoneProcessor);
    }

    /**
     * Return a TimeZone representing an error condition. isError() returns
     * true for this instance.
     */
    static TimeZone forError() {
      return TimeZone(kTypeError);
    }

    /** Default constructor creates a UTC TimeZone. */
    TimeZone():
        mType(kTypeManual),
        mStdOffsetMinutes(0),
        mDstOffsetMinutes(0) {}

    /**
     * Return the type of TimeZone. This value is useful for serializing and
     * deserializing (or storing and restoring) the TimeZone object.
     */
    uint8_t getType() const { return mType; }

    /** Return the Standard TimeOffset. Valid only for kTypeManual. */
    TimeOffset getStdOffset() const {
      return TimeOffset::forMinutes(mStdOffsetMinutes);
    }

    /** Return the DST TimeOffset. Valid only for kTypeManual. */
    TimeOffset getDstOffset() const {
      return TimeOffset::forMinutes(mDstOffsetMinutes);
    }

    /**
     * Return the zoneId for kTypeBasic, kTypeExtended. Returns 0 for
     * kTypeManual. (It is not entirely clear that a valid zoneId is always >
     * 0, but there is little I can do without C++ exceptions.)
     */
    uint32_t getZoneId() const {
      switch (mType) {
        case kTypeManual:
          return 0;
        case kTypeBasic:
          return BasicZone((const basic::ZoneInfo*) mZoneInfo).zoneId();
        case kTypeExtended:
          return ExtendedZone((const extended::ZoneInfo*) mZoneInfo).zoneId();
      }
      return 0;
    }

    /** Return true if TimeZone is an error. */
    bool isError() const { return mType == kTypeError; }

    /**
     * Return the total UTC offset at epochSeconds, including DST offset.
     */
    TimeOffset getUtcOffset(acetime_t epochSeconds) const {
      switch (mType) {
        case kTypeManual:
          return TimeOffset::forMinutes(mStdOffsetMinutes + mDstOffsetMinutes);
        case kTypeBasic:
        case kTypeExtended:
          mZoneProcessor->setZoneInfo(mZoneInfo);
          return mZoneProcessor->getUtcOffset(epochSeconds);
      }
      return TimeOffset::forError();
    }

    /**
     * Return the DST offset from standard UTC offset at epochSeconds. This is
     * an experimental method that has not been tested thoroughly. Use with
     * caution.
     */
    TimeOffset getDeltaOffset(acetime_t epochSeconds) const {
      switch (mType) {
        case kTypeManual:
          return TimeOffset::forMinutes(mDstOffsetMinutes);
        case kTypeBasic:
        case kTypeExtended:
          mZoneProcessor->setZoneInfo(mZoneInfo);
          return mZoneProcessor->getDeltaOffset(epochSeconds);
      }
      return TimeOffset::forError();
    }

    /**
     * Return the time zone abbreviation at the given epochSeconds.
     *
     *   * kTypeManual: returns "STD" or "DST",
     *   * kTypeBasic, kTypeBasic: returns the "{abbrev}" (e.g. "PDT"),
     *   * error condition: returns the empty string ""
     *
     * The IANA spec
     * (https://data.iana.org/time-zones/theory.html#abbreviations) says that
     * abbreviations are limited to 6 characters. It is unclear if this spec is
     * guaranteed to be followed. Client programs should handle abbreviations
     * longer than 6 characters to be safe.
     *
     * Warning: The returned pointer points to a char buffer that could get
     * overriden by subsequent method calls to this object. The pointer must
     * not be stored permanently, it should be used as soon as possible (e.g.
     * printed out). If you need to store the abbreviation for a long period
     * of time, copy it to anther char buffer.
     */
    const char* getAbbrev(acetime_t epochSeconds) const {
      switch (mType) {
        case kTypeManual:
          if (isUtc()) {
            return "UTC";
          } else {
            return (mDstOffsetMinutes != 0) ? "DST" : "STD";
          }
        case kTypeBasic:
        case kTypeExtended:
          return mZoneProcessor->getAbbrev(epochSeconds);
      }
      return "";
    }

    /**
     * Return the best estimate of the OffsetDateTime at the given
     * LocalDateTime for the current TimeZone. Used by
     * ZonedDateTime::forComponents(), so intended to be used mostly for
     * testing and debugging.
     */
    OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt) const {
      OffsetDateTime odt = OffsetDateTime::forError();
      switch (mType) {
        case kTypeManual:
          odt = OffsetDateTime::forLocalDateTimeAndOffset(ldt,
              TimeOffset::forMinutes(mStdOffsetMinutes + mDstOffsetMinutes));
          break;
        case kTypeBasic:
        case kTypeExtended:
          mZoneProcessor->setZoneInfo(mZoneInfo);
          odt = mZoneProcessor->getOffsetDateTime(ldt);
          break;
      }
      return odt;
    }

    /** Return true if UTC (+00:00+00:00). */
    bool isUtc() const {
      if (mType != kTypeManual) return false;
      return mStdOffsetMinutes == 0 && mDstOffsetMinutes == 0;
    }

    /**
     * Return if mDstOffsetMinutes is not zero. This is a convenience method
     * that is valid only if the TimeZone is a kTypeManual. Returns false for
     * all other type of TimeZone. This is intended to be used by applications
     * which allows the user to set the UTC offset and DST flag manually (e.g.
     * examples/WorldClock.ino).
     */
    bool isDst() const {
      if (mType != kTypeManual) return false;
      return mDstOffsetMinutes != 0;
    }

    /**
     * Sets the stdOffset of the TimeZone. Works only for kTypeManual, does
     * nothing for any other type of TimeZone.
     */
    void setStdOffset(TimeOffset stdOffset) {
      if (mType != kTypeManual) return;
      mStdOffsetMinutes = stdOffset.toMinutes();
    }

    /**
     * Sets the dstOffset of the TimeZone. Works only for kTypeManual, does
     * nothing for any other type of TimeZone.
     */
    void setDstOffset(TimeOffset dstOffset) {
      if (mType != kTypeManual) return;
      mDstOffsetMinutes = dstOffset.toMinutes();
    }

    /**
     * Convert to a TimeZoneData object, which can be fed back into
     * ZoneManager::createForTimeZoneData() to recreate the TimeZone. Both
     * TimeZone::kTypeBasic and TimeZone::kTypeExtended are mapped to
     * TimeZoneData::kTypeZoneId.
     */
    TimeZoneData toTimeZoneData() const {
      TimeZoneData d;
      switch (mType) {
        case TimeZone::kTypeManual:
          d.stdOffsetMinutes = mStdOffsetMinutes;
          d.dstOffsetMinutes = mDstOffsetMinutes;
          d.type = TimeZoneData::kTypeManual;
          break;
        case TimeZone::kTypeBasic:
        case TimeZone::kTypeExtended:
          d.zoneId = getZoneId();
          d.type = TimeZoneData::kTypeZoneId;
          break;
        default:
          d.type = TimeZoneData::kTypeError;
          break;
      }
      return d;
    }

    /**
     * Print the human readable representation of the time zone.
     *   * kTypeManual is printed as "+/-hh:mm+/-hh:mm" (e.g. "-08:00+00:00")
     *   * kTypeBasic is printed as "{zonename}" (e.g. "America/Los_Angeles")
     *   * kTypeExtended is printed as "{zonename}" (e.g.
     *     "America/Los_Angeles")
     */
    void printTo(Print& printer) const;

    /**
     * Print the *short* human readable representation of the time zone.
     *   * kTypeManual is printed as "+/-hh:mm(STD|DST)" (e.g. "-07:00(DST)")
     *   * kTypeBasic is printed as "{zoneShortName}" (e.g. "Los_Angeles")
     *   * kTypeExtended is printed as "{zoneShortName}" (e.g. "Los_Angeles")
     */
    void printShortTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    TimeZone(const TimeZone&) = default;
    TimeZone& operator=(const TimeZone&) = default;

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);

    /**
     * Constructor for a kTypeManual TimeZone.
     *
     * @param stdOffset the base UTC offset
     * @param dstOffset the DST delta offset (can be negative)
     */
    explicit TimeZone(TimeOffset stdOffset, TimeOffset dstOffset):
      mType(kTypeManual),
      mStdOffsetMinutes(stdOffset.toMinutes()),
      mDstOffsetMinutes(dstOffset.toMinutes()) {}

    /** Constructor needed to create a ::forError(). */
    explicit TimeZone(uint8_t type):
      mType(type) {}

    /** Constructor for kTypeBasic or kTypeExtended. */
    explicit TimeZone(uint8_t type, const void* zoneInfo,
        ZoneProcessor* mZoneProcessor):
        mType(type),
        mZoneInfo(zoneInfo),
        mZoneProcessor(mZoneProcessor) {}

    uint8_t mType;

    // 3 combinations:
    //   (type) (kTypeError)
    //   (type, mStdOffsetMinutes, mDstOffsetMinutes)
    //   (type, mZoneInfo, mZoneProcessor)
    union {
      /** Used by kTypeManual. */
      struct {
        int16_t mStdOffsetMinutes;
        int16_t mDstOffsetMinutes;
      };

      /* Used by kTypeBasic and kTypeExtended. */
      struct {
        /** Either a basic::ZoneInfo or an extended::ZoneInfo. */
        const void* mZoneInfo;

        /** Either BasicZoneProcessor or ExtendedZoneProcessor. */
        ZoneProcessor* mZoneProcessor;
      };
    };
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mType != b.mType) return false;
  switch (a.mType) {
    case TimeZone::kTypeError:
      return true;
    case TimeZone::kTypeManual:
      return a.mStdOffsetMinutes == b.mStdOffsetMinutes
          && a.mDstOffsetMinutes == b.mDstOffsetMinutes;
    case TimeZone::kTypeBasic:
    case TimeZone::kTypeExtended:
      return (a.mZoneInfo == b.mZoneInfo);
    default:
      return false;
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
