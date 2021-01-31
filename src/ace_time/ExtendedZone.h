/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_H
#define ACE_TIME_EXTENDED_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/ExtendedBrokers.h"

class Print;

namespace ace_time {

/**
 * A thin wrapper around an extended::ZoneInfo data structure to provide a
 * stable API access to some useful extended::ZoneInfo data. The
 * extended::ZoneInfo data struct is intended to be an opaque type to the users
 * of this library.
 */
class ExtendedZone {
  public:
    ExtendedZone(const extended::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;

    uint32_t zoneId() const {
      return mZoneInfoBroker.zoneId();
    }

  private:
    // disable copy constructor and assignment operator
    ExtendedZone(const ExtendedZone&) = delete;
    ExtendedZone& operator=(const ExtendedZone&) = delete;

    /**
     * Find the short name that begins after the last separator '/', a keyword
     * reference, or at the beginning of the string if no separator or keyword.
     * The last component of the full ZoneName is never compressed, which
     * allows this to be implemented without using ace_common::KString.
     *
     * For example, "America/Los_Angeles" returns a pointer to "Los_Angeles",
     * and "\x01Denver" returns a ponter to "Denver". This method returns
     * either a (const char*) or a (const __FlashStringHelper*) depending on
     * whether PROGMEM is used or not. The caller is responsible for casting to
     * the correct type.
     */
  #if ACE_TIME_USE_PROGMEM
    static const __FlashStringHelper* findShortName(
        const __FlashStringHelper* fname);
  #else
    static const char* findShortName(const char* name);
  #endif

    const extended::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
