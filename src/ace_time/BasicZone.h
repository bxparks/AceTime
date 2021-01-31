/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_H
#define ACE_TIME_BASIC_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/BasicBrokers.h"

class Print;

namespace ace_time {

/**
 * A thin wrapper around a basic::ZoneInfo data structure to provide a stable
 * API access to some useful basic::ZoneInfo data. The basic::ZoneInfo data
 * struct is intended to be an opaque type to the users of this library.
 */
class BasicZone {
  public:
    BasicZone(const basic::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;

    uint32_t zoneId() const {
      return mZoneInfoBroker.zoneId();
    }

  private:
    // disable default copy constructor and assignment operator
    BasicZone(const BasicZone&) = delete;
    BasicZone& operator=(const BasicZone&) = delete;

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
    static const char* findShortName(const char* name);

    const basic::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
