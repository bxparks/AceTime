/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h> // uintptr_t
#include "TimeOffset.h"
#include "ZoneProcessor.h"
#include "BasicZoneProcessor.h"
#include "ExtendedZoneProcessor.h"
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
 * The TimeZone::getType() is designed to be extensible, so only some of its
 * values are defined by this class:
 *
 *    * TimeZone::kTypeError
 *        * represents an error or unknown time zone
 *    * TimeZone::kTypeManual
 *        * holds a base offset and a DST offset, and allows the user to modify
 *          both of these fields
 *    * TimeZone::kTypeReserved
 *        * reserved for future extension, currently same as kTypeError
 *    * Additional values are provided by specific subclasses of the
 *      ZoneProcessor class. For example:
 *        * BasicZoneProcessor::kTypeBasic
 *            * uses the BasicZoneProcessor to support the zones and links
 *              defined by `zonedb/zone_infos.h`.
 *        * ExtendedZoneProcessor::kTypeExtended
 *            * uses the ExtendedZoneProcessor to support all zones and links
 *              defined by `zonedbx/zone_infos.h`
 *        * Other ZoneProcessors can provide additional values, as long as
 *          they are unique.
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
    /** A TimeZone that represents an invalid condition. */
    static const uint8_t kTypeError = 0;

    /** Manual STD offset and DST offset. */
    static const uint8_t kTypeManual = 1;

    /** Reserved for future use. */
    static const uint8_t kTypeReserved = 2;

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
     * Convenience factory method to create from a zoneInfo and an associated
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
      return TimeZone(
          zoneProcessor->getType(),
          (uintptr_t) zoneInfo,
          zoneProcessor
      );
    }

    /**
     * Convenience factory method to create from a zoneInfo and an associated
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
      return TimeZone(
          zoneProcessor->getType(),
          (uintptr_t) zoneInfo,
          zoneProcessor
      );
    }

    /**
     * Factory method to create from a generic zoneKey and a generic
     * zoneProcessor. The 'type' of the TimeZone is extracted from
     * ZoneProcessor::getType().
     */
    static TimeZone forZoneKey(uintptr_t zoneKey, ZoneProcessor* processor) {
      return TimeZone(processor->getType(), zoneKey, processor);
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
     * Return the type of TimeZone, used to determine the behavior of certain
     * methods at runtime. The exact value returned by this method is designed
     * to be extensible and is an internal implementation detail. It  will
     * probably not be stable across multiple versions of this library. For
     * stable serialization, use the toTimeZoneData() method instead.
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

    /** Return true if timezone is a Link entry pointing to a Zone entry. */
    bool isLink() const {
      switch (mType) {
        case kTypeError:
        case kTypeReserved:
        case kTypeManual:
          return false;

        default:
          return getBoundZoneProcessor()->isLink();
      }
    }

    /**
     * Return the zoneId for kTypeBasic, kTypeExtended. Returns 0 for
     * kTypeManual. (It is not entirely clear that a valid zoneId is always >
     * 0, but there is little I can do without C++ exceptions.)
     */
    uint32_t getZoneId() const {
      switch (mType) {
        case kTypeError:
        case kTypeReserved:
        case kTypeManual:
          return 0;

        default:
          return getBoundZoneProcessor()->getZoneId();
      }
    }

    /** Return true if TimeZone is an error. */
    bool isError() const { return mType == kTypeError; }

    /**
     * Return the total UTC offset at epochSeconds, including DST offset.
     */
    TimeOffset getUtcOffset(acetime_t epochSeconds) const {
      switch (mType) {
        case kTypeError:
        case kTypeReserved:
          return TimeOffset::forError();

        case kTypeManual:
          return TimeOffset::forMinutes(mStdOffsetMinutes + mDstOffsetMinutes);

        default:
          return getBoundZoneProcessor()->getUtcOffset(epochSeconds);
      }
    }

    /**
     * Return the DST offset from standard UTC offset at epochSeconds. This is
     * an experimental method that has not been tested thoroughly. Use with
     * caution.
     */
    TimeOffset getDeltaOffset(acetime_t epochSeconds) const {
      switch (mType) {
        case kTypeError:
        case kTypeReserved:
          return TimeOffset::forError();

        case kTypeManual:
          return TimeOffset::forMinutes(mDstOffsetMinutes);

        default:
          return getBoundZoneProcessor()->getDeltaOffset(epochSeconds);
      }
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
        case kTypeError:
        case kTypeReserved:
          return "";

        case kTypeManual:
          if (isUtc()) {
            return "UTC";
          } else {
            return (mDstOffsetMinutes != 0) ? "DST" : "STD";
          }

        default:
          return getBoundZoneProcessor()->getAbbrev(epochSeconds);
      }
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
        case kTypeError:
        case kTypeReserved:
          break;

        case kTypeManual:
          odt = OffsetDateTime::forLocalDateTimeAndOffset(
              ldt,
              TimeOffset::forMinutes(mStdOffsetMinutes + mDstOffsetMinutes));
          break;

        default:
          odt = getBoundZoneProcessor()->getOffsetDateTime(ldt);
          break;
      }
      return odt;
    }

    /**
     * Return the best estimate of the OffsetDateTime at the given epochSeconds.
     * Used by ZonedDateTime::forEpochSeconds(), so exposed publically for
     * testing and debugging.
     */
    OffsetDateTime getOffsetDateTime(acetime_t epochSeconds) const {
      OffsetDateTime odt = OffsetDateTime::forError();
      switch (mType) {
        case kTypeError:
        case kTypeReserved:
          break;

        case kTypeManual:
          odt = OffsetDateTime::forEpochSeconds(
              epochSeconds,
              TimeOffset::forMinutes(mStdOffsetMinutes + mDstOffsetMinutes));
          break;

        default:
          odt = getBoundZoneProcessor()->getOffsetDateTime(epochSeconds);
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
        case kTypeError:
        case kTypeReserved:
          d.type = TimeZoneData::kTypeError;
          break;

        case TimeZone::kTypeManual:
          d.stdOffsetMinutes = mStdOffsetMinutes;
          d.dstOffsetMinutes = mDstOffsetMinutes;
          d.type = TimeZoneData::kTypeManual;
          break;

        default:
          d.zoneId = getZoneId();
          d.type = TimeZoneData::kTypeZoneId;
          break;
      }
      return d;
    }

    /**
     * Print the text representation of the time zone using the full canonical
     * time zone name or UTC offset shift.
     *
     *   * kTypeManual is printed as "+/-hh:mm+/-hh:mm" (e.g. "-08:00+00:00")
     *   * kTypeBasic is printed as "{zoneName}" (e.g. "America/Los_Angeles")
     *   * kTypeExtended is printed as "{zoneName}" (e.g.
     *     "America/Los_Angeles")
     */
    void printTo(Print& printer) const;

    /**
     * Print the *short* human readable representation of the time zone. This
     * method uses some rough heuristics for determine the reasonable human
     * readable form. For basic and extended time zones, the last component of
     * the canonical zone name is printed, with the underscore character
     * replaced with just a space, for example "Los Angeles". For manual time
     * zones, it prints the total UTC offset with a (D) if the DST flag is
     * active and an (S) if not, for example, "-08:00(S)".
     *
     * If you need better control over how the time zone is displayed, you need
     * to write that code yourself using the getType() and the getZoneId()
     * identifiers.
     *
     *   * kTypeManual is printed as "+/-hh:mm(S|D)" depending on DST or STD
     *     flag (e.g. "-07:00(D)")
     *   * kTypeBasic is printed as "{zoneShortName}" (e.g. "Los Angeles")
     *   * kTypeExtended is printed as "{zoneShortName}" (e.g. "Los Angeles")
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

    /** Constructor using ZoneProcessor. Exposed for library extensions. */
    explicit TimeZone(
        uint8_t type,
        uintptr_t zoneKey,
        ZoneProcessor* zoneProcessor
    ):
        mType(type),
        mZoneKey(zoneKey),
        mZoneProcessor(zoneProcessor)
    {}

    /**
     * Return the ZoneProcessor associated with this TimeZone after forcibly
     * rebinding it to the current zoneKey. This is necessary because the
     * ZoneProcessorCache could have bound the zoneProcessor to another
     * TimeZone if it had run out of available ZoneProcessors. Cache
     * invalidation is hard!
     */
    ZoneProcessor* getBoundZoneProcessor() const {
      mZoneProcessor->setZoneKey(mZoneKey);
      return mZoneProcessor;
    }

    uint8_t mType;

    // 3 combinations:
    //   (kTypeError)
    //   (kTypeManual, mStdOffsetMinutes, mDstOffsetMinutes)
    //   (type, mZoneKey, mZoneProcessor)
    union {
      /** Used by kTypeManual. */
      struct {
        int16_t mStdOffsetMinutes;
        int16_t mDstOffsetMinutes;
      };

      /* Used by kTypeBasic and kTypeExtended. */
      struct {
        /**
         * An opaque zone key.
         *
         *  * For kTypeBasic, this is a (const basic::ZoneInfo*).
         *  * For kTypeExtended, this is a (const extended::ZoneInfo*).
         *
         * Internally, the TimeZone class does not care how this is
         * implemented. The factory methods expose these types for the
         * convenience of the end users.
         */
        uintptr_t mZoneKey;

        /**
         * An instance of a ZoneProcessor, for example, BasicZoneProcessor or
         * ExtendedZoneProcessor.
         */
        ZoneProcessor* mZoneProcessor;
      };
    };
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mType != b.mType) return false;
  switch (a.mType) {
    case TimeZone::kTypeError:
    case TimeZone::kTypeReserved:
      return true;

    case TimeZone::kTypeManual:
      return a.mStdOffsetMinutes == b.mStdOffsetMinutes
          && a.mDstOffsetMinutes == b.mDstOffsetMinutes;

    default:
      return (a.mZoneKey == b.mZoneKey);
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
