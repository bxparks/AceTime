#ifndef OLED_CLOCK_RENDERING_INFO_H
#define OLED_CLOCK_RENDERING_INFO_H

#include <AceTime.h>
#include "config.h"

/**
 * Data used by the Presenter (the "View") to determine what has changed and
 * what needs to be displayed.
 */ 
struct RenderingInfo {
  uint8_t mode = 0; // display mode, see MODE_xxx above
  bool suppressBlink = false; // true if blinking should be suppressed
  bool blinkShowState = true; // true if blinking info should be shown
  uint8_t hourMode = 0; // 12/24 mode
  ace_time::ManualZoneSpecifier manualZspec;
  ace_time::BasicZoneSpecifier basicZspec;
  ace_time::ZonedDateTime dateTime; // seconds from AceTime epoch

  /** Custom assignment operator to make a deep copy of TimeZone. */
  RenderingInfo& operator=(const RenderingInfo& that) {
    mode = that.mode;
    suppressBlink = that.suppressBlink;
    blinkShowState = that.blinkShowState;
    hourMode = that.hourMode;
    manualZspec = that.manualZspec;
    basicZspec = that.basicZspec;
    dateTime = that.dateTime;

    // Make a deep copy of the TimeZone
    dateTime.timeZone(ace_time::TimeZone::forZoneSpecifier(
        (dateTime.timeZone().getType() == ace_time::TimeZone::kTypeManual)
            ? (ace_time::ZoneSpecifier*) &manualZspec
            : (ace_time::ZoneSpecifier*) &basicZspec));
    return *this;
  }
};

#endif
