/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "TimeOffset.h"
#include "ZoneSpecifier.h"
#include "ZoneSpecifierCache.h"
#include "BasicZone.h"
#include "ExtendedZone.h"

class Print;

namespace ace_time {

template<typename ZI, typename ZR, typename ZSC> class ZoneManager;

/**
 * Class that describes a time zone. There are 2 colloquial usages of "time
 * zone". The first refers to a simple fixed offset from UTC. For example, we
 * may say that "we are in -05:00 time zone". The second is a geographical
 * region that obeys a consistent set of rules regarding the value of the UTC
 * offset, and when the transitions to DST happens (if at all). The best known
 * source of these geographical regions is the TZ Database maintained by IANA
 * (https://www.iana.org/time-zones). The TimeZone class supports both meanings.
 *
 * There are 5 types of TimeZone:
 *
 *    * kTypeManual: a time zone that holds a base offset and a DST offset, and
 *      allows the user to modify both of these fields
 *    * kTypeBasic: a time zone using an underlying BasicZoneSpecifier which
 *      supports 231 geographical zones in the TZ Database.
 *    * kTypeExtended: a time zone using an underlying ExtendedZoneSpecifier
 *      which supports 348 geographical zones in the TZ Database (essentially
 *      the entire database).
 *    * kTypeBasicManaged: uses the ZoneSpecifierCache to manage a cache of
 *      BasicZoneSpecifiers and provide mapping from zoneName or zoneId to the
 *      ZoneInfo.
 *    * kTypeExtendedManaged: uses the ZoneSpecifierCache to manage a cache of
 *      ExtendedZoneSpecifiers and provide mapping from zoneName or zoneId to
 *      the ZoneInfo.
 *
 * The TimeZone class really really wants to be a reference type. In other
 * words, it would be very convenient if the client code could create this
 * object on the heap, and pass it around using a pointer (or smart pointer) to
 * the ZonedDateTime class and shared among multiple ZonedDateTime objects.
 * This would also allow new TimeZones to be created, while allowing older
 * instances of ZonedDateTime to hold on to the previous versions of TimeZone.
 *
 * However, in a small memory embedded environment (like Arduino Nano or Micro
 * with only 2kB of RAM), I want to avoid any use of the heap (new operator or
 * malloc()) inside the AceTime library. I separated out the memory intensive
 * or mutable features of the TimeZone class into the separate ZoneSpecifier
 * class. The ZoneSpecifier object should be created once at initialization
 * time of the application (either statically allocated or potentially on the
 * heap early in the application start up).
 *
 * The TimeZone class becomes a thin wrapper around a ZoneSpecifier object
 * (essentially acting like a smart pointer in some sense). It should be
 * treated as a value type and passed around by value or by const reference.
 *
 * An alternative implementation would use an inheritance hierarchy for the
 * TimeZone, with 2 subclasses (ManualTimeZone and AutoTimeZone). However this
 * means that the TimeZone object can no longer be passed around by value, and
 * the ZonedDateTime is forced to hold on to the TimeZone object using a
 * pointer. It then becomes very difficult to change the offset and DST fields
 * of the ManualTimeZone. Using a single TimeZone class and implementing it as
 * a value type simplifies a lot of code.
 *
 * Serialization and Deserialization:
 *
 * Serializing and deserializing the TimeZone object is difficult because we
 * need to save information from both the TimeZone object and (potentially) the
 * ZoneSpecifier object. The TimeZone object can be identified by its
 * `getType()` parameter. If this type is kTypeManual, the time zone is
 * fullly identified stdOffset and dstOffset.
 *
 * If the type is kTypeBasic or kTypeExtended, the underlying
 * BasicZoneSpecifier and ExtendedZoneSpecifier can both be uniquely identified
 * by the fully-qualified zone identifier (e.g. "America/Los_Angeles").
 * However, due to memory limitations, most Arduino environments cannot contain
 * the entire TZ Database with all 348 zones supported by AceTime. Therefore,
 * there is currently no ability to create a BasicZoneSpecifier or
 * ExtendedZoneSpecifier from its fully-qualified zone name.
 *
 * Since most Arduino environments will support only a handful of hardcoded
 * time zones, the recommended procedure is to associate each of these zones
 * with a small numerical identifier, and use that to recreate the appropriate
 * instance of BasicZoneSpecifier or ExtendedZoneSpecifier.
 *
 * On larger Arduino environments (e.g. ESP8266 or ESP32) with enough memory,
 * it may be possible to allow the creation of a BasicZoneSpecifier or
 * ExtendedZoneSpecifier from the fully-qualified zone name. It would then be
 * sufficient to just store the fully-qualified zone name, instead of creating
 * a customized mapping table. However, we still would need to worry about TZ
 * Database version skew. In other words, the code needs to handle situations
 * where the serialized zone name using one version of the TZ Database is not
 * recognized by a new version of TZ Database.
 */
class TimeZone {
  public:
    static const uint8_t kTypeError = 0;
    static const uint8_t kTypeManual = 1;
    static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
    static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;
    static const uint8_t kTypeBasicManaged =
        ZoneSpecifierCache::kTypeBasicManaged;
    static const uint8_t kTypeExtendedManaged =
        ZoneSpecifierCache::kTypeExtendedManaged;

    /** Factory method to create a UTC TimeZone. */
    static TimeZone forUtc() {
      return TimeZone();
    }

    /**
     * Factory method to create from a UTC offset and an optional DST offset.
     *
     * @param stdOffset the base offset
     * @param dstOffset the DST offset, default TimeOffset() (i.e. 0 offset)
     */
    static TimeZone forTimeOffset(TimeOffset stdOffset,
        TimeOffset dstOffset = TimeOffset()) {
      return TimeZone(stdOffset, dstOffset);
    }

    /**
     * Factory method to create from a BasicZoneSpecifier. A ZoneSpecifierCache
     * is not required.
     * @param zoneSpecifier a pointer to a ZoneSpecifier, cannot be nullptr
     */
    static TimeZone forZoneSpecifier(BasicZoneSpecifier* zoneSpecifier) {
      auto zoneInfo = (const basic::ZoneInfo*) zoneSpecifier->getZoneInfo();
      return TimeZone(kTypeBasic, zoneInfo, zoneSpecifier);
    }

    /**
     * Factory method to create from a ExtendedZoneSpecifier. A
     * ZoneSpecifierCache is not required.
     * @param zoneSpecifier a pointer to a ZoneSpecifier, cannot be nullptr
     */
    static TimeZone forZoneSpecifier(ExtendedZoneSpecifier* zoneSpecifier) {
      auto zoneInfo = (const extended::ZoneInfo*) zoneSpecifier->getZoneInfo();
      return TimeZone(kTypeExtended, zoneInfo, zoneSpecifier);
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
        mStdOffsetCode(0),
        mDstOffsetCode(0) {}

    /**
     * Return the type of TimeZone. This value is useful for serializing and
     * deserializing (or storing and restoring) the TimeZone object.
     */
    uint8_t getType() const { return mType; }

    TimeOffset getStdOffset() const {
      return TimeOffset::forOffsetCode(mStdOffsetCode);
    }

    TimeOffset getDstOffset() const {
      return TimeOffset::forOffsetCode(mDstOffsetCode);
    }

    /**
     * Return the zoneId. Returns 0 if not valid. (It is not entirely clear
     * that the zoneId is always > 0, but there is little I can do without
     * C++ exceptions.)
     */
    uint32_t getZoneId() const {
      switch (mType) {
        case kTypeManual:
          return 0;
        case kTypeBasic:
        case kTypeBasicManaged:
          return BasicZone((const basic::ZoneInfo*) mZoneInfo).zoneId();
        case kTypeExtended:
        case kTypeExtendedManaged:
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
          return TimeOffset::forOffsetCode(mStdOffsetCode + mDstOffsetCode);
        case kTypeBasic:
        case kTypeExtended:
          mZoneSpecifier->setZoneInfo(mZoneInfo);
          return mZoneSpecifier->getUtcOffset(epochSeconds);
        case kTypeBasicManaged:
        case kTypeExtendedManaged:
        {
          ZoneSpecifier* specifier =
              mZoneSpecifierCache->getZoneSpecifier(mZoneInfo);
          if (! specifier) break;
          return specifier->getUtcOffset(epochSeconds);
        }
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
          return TimeOffset::forOffsetCode(mDstOffsetCode);
        case kTypeBasic:
        case kTypeExtended:
          mZoneSpecifier->setZoneInfo(mZoneInfo);
          return mZoneSpecifier->getDeltaOffset(epochSeconds);
        case kTypeBasicManaged:
        case kTypeExtendedManaged:
        {
          ZoneSpecifier* specifier =
              mZoneSpecifierCache->getZoneSpecifier(mZoneInfo);
          if (! specifier) break;
          return specifier->getDeltaOffset(epochSeconds);
        }
      }
      return TimeOffset::forError();
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
              TimeOffset::forOffsetCode(mStdOffsetCode + mDstOffsetCode));
          break;
        case kTypeBasic:
        case kTypeExtended:
          mZoneSpecifier->setZoneInfo(mZoneInfo);
          odt = mZoneSpecifier->getOffsetDateTime(ldt);
          break;
        case kTypeBasicManaged:
        case kTypeExtendedManaged:
        {
          ZoneSpecifier* specifier =
              mZoneSpecifierCache->getZoneSpecifier(mZoneInfo);
          if (! specifier) break;
          odt = specifier->getOffsetDateTime(ldt);
          break;
        }
      }
      return odt;
    }

    /** Return true if UTC (+00:00+00:00). */
    bool isUtc() const {
      if (mType != kTypeManual) return false;
      return mStdOffsetCode == 0 && mDstOffsetCode == 0;
    }

    /**
     * Return if mDstOffsetCode is not zero. This is a convenience method that
     * is valid only if the TimeZone is a kTypeManual. Returns false for all
     * other type of TimeZone. This is intended to be used by applications
     * which allows the user to set the UTC offset and DST flag manually (e.g.
     * examples/WorldClock.ino).
     */
    bool isDst() const {
      if (mType != kTypeManual) return false;
      return mDstOffsetCode != 0;
    }

    /**
     * Sets the stdOffset of the TimeZone. Works only for kTypeManual, does
     * nothing for any other type of TimeZone.
     */
    void setStdOffset(TimeOffset stdOffset) {
      if (mType != kTypeManual) return;
      mStdOffsetCode = stdOffset.toOffsetCode();
    }

    /**
     * Sets the dstOffset of the TimeZone. Works only for kTypeManual, does
     * nothing for any other type of TimeZone.
     */
    void setDstOffset(TimeOffset dstOffset) {
      if (mType != kTypeManual) return;
      mDstOffsetCode = dstOffset.toOffsetCode();
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

    /**
     * Print the time zone abbreviation for the given epochSeconds.
     *   * kTypeManual is printed as "STD" or "DST"
     *   * kTypeBasic is printed as "{abbrev}" (e.g. "PDT")
     *   * kTypeExtended is printed as "{abbrev}" (e.g. "PDT")
     */
    void printAbbrevTo(Print& printer, acetime_t epochSeconds) const;

    // Use default copy constructor and assignment operator.
    TimeZone(const TimeZone&) = default;
    TimeZone& operator=(const TimeZone&) = default;

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);

    // Allow ZoneManager to access the TimeZone() constructor that accepts
    // a ZoneSpecifierCache.
    template<typename ZI, typename ZR, typename ZSC> friend class ZoneManager;

    /**
     * Constructor for kType*Managed. Intended to be used ONLY by
     * BasicZoneManager and ExtendedZoneManager.
     *
     * @param zoneInfo a pointer to a basic::ZoneInfo. Cannot be nullptr.
     * @param zoneSpecifierCache a cache of ZoneInfo -> ZoneSpecifier
     */
    explicit TimeZone(const void* zoneInfo,
        ZoneSpecifierCache* zoneSpecifierCache):
        mType(zoneSpecifierCache->getType()),
        mZoneInfo(zoneInfo),
        mZoneSpecifierCache(zoneSpecifierCache) {}

    /**
     * Constructor for a manual Time zone.
     *
     * @param stdOffset the base UTC offset
     * @param dstOffset the DST delta offset (can be negative)
     */
    explicit TimeZone(TimeOffset stdOffset, TimeOffset dstOffset):
      mType(kTypeManual),
      mStdOffsetCode(stdOffset.toOffsetCode()),
      mDstOffsetCode(dstOffset.toOffsetCode()) {}

    /** Constructor needed to create a ::forError(). */
    explicit TimeZone(uint8_t type):
      mType(type) {}

    /** Constructor for kTypeBasic or kTypeExtended. */
    explicit TimeZone(uint8_t type, const void* zoneInfo,
        ZoneSpecifier* mZoneSpecifier):
        mType(type),
        mZoneInfo(zoneInfo),
        mZoneSpecifier(mZoneSpecifier) {}

    uint8_t mType;

    // 4 combinations:
    //   (type) (kTypeError)
    //   (type, mStdOffsetCode, mDstOffsetCode)
    //   (type, mZoneInfo, mZoneSpecifier)
    //   (type, mZoneInfo, mZoneSpecifierCache)
    union {
      /** Used by kTypeManual. */
      struct {
        int8_t mStdOffsetCode;
        int8_t mDstOffsetCode;
      };

      struct {
        /**
         * Used by kTypeBasic, kTypeExtended, kTypeBasicManaged,
         * kTypeExtendedManaged.
         */
        const void* mZoneInfo;

        union {
          /** Used by kTypeBasic, kTypeExtended. */
          ZoneSpecifier* mZoneSpecifier;

          /** Used by kTypeBasicManaged, kTypeExtendedManaged. */
          ZoneSpecifierCache* mZoneSpecifierCache;
        };
      };
    };
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mType != b.mType) return false;
  switch (a.mType) {
    case TimeZone::kTypeError:
      return true;
    case TimeZone::kTypeManual:
      return a.mStdOffsetCode == b.mStdOffsetCode
          && a.mDstOffsetCode == b.mDstOffsetCode;
    case TimeZone::kTypeBasic:
    case TimeZone::kTypeExtended:
    case TimeZone::kTypeBasicManaged:
    case TimeZone::kTypeExtendedManaged:
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
