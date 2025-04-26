/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_COMPLETE_ZONE_PROCESSOR_H
#define ACE_TIME_COMPLETE_ZONE_PROCESSOR_H

#include <stdint.h> // uintptr_t
#include "ExtendedZoneProcessor.h"

namespace ace_time {

/**
 * A specific implementation of ExtendedZoneProcessorTemplate that uses the
 * complete::ZoneXxxBrokers classes which read from 'zonedbc' files in PROGMEM
 * flash memory using the high-resolution zoneinfo data structures.
 */
class CompleteZoneProcessor: public
  ExtendedZoneProcessorTemplate<complete::Info> {

  public:
    /** Unique TimeZone type identifier for CompleteZoneProcessor. */
    static const uint8_t kTypeComplete = 5;

    explicit CompleteZoneProcessor(
        const complete::Info::ZoneInfo* zoneInfo = nullptr)
      : ExtendedZoneProcessorTemplate<complete::Info>(
          kTypeComplete, &mZoneInfoStore, (uintptr_t) zoneInfo)
    {}

  private:
    complete::Info::ZoneInfoStore mZoneInfoStore;
};

}

#endif
