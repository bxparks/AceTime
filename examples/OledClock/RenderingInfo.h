#ifndef OLED_CLOCK_RENDERING_INFO_H
#define OLED_CLOCK_RENDERING_INFO_H

#include <AceTime.h>
#include "config.h"

/**
 * Data used by the Presenter (the "View") to determine what has changed and
 * what needs to be displayed.
 */ 
struct RenderingInfo {
  uint8_t mode; // display mode, see MODE_xxx in config.h
  bool suppressBlink; // true if blinking should be suppressed
  bool blinkShowState; // true if blinking info should be shown

  uint8_t hourMode; // ClockInfo::kTwelve or kTwentyFour
  ace_time::TimeZone timeZone;
  ace_time::ZonedDateTime dateTime;
};

#endif
