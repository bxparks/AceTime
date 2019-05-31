#ifndef FULL_OLED_CLOCK_RENDERING_INFO_H
#define FULL_OLED_CLOCK_RENDERING_INFO_H

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
  ace_time::ZonedDateTime dateTime; // seconds from AceTime epoch
  ace_time::ManualZoneSpecifier zoneSpecifier;
};

#endif
