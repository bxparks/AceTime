#ifndef WORLD_CLOCK_RENDERING_INFO_H
#define WORLD_CLOCK_RENDERING_INFO_H

#include <AceTime.h>
#include "config.h"
#include "ClockInfo.h"

using namespace ace_time;

/**
 * Data used by the Presenter (the "View") to determine what has changed and
 * what needs to be displayed. Contains similar information as ClockInfo but in
 * addition to other information related to the presentation of the clock.
 */ 
struct RenderingInfo {
  uint8_t mode = MODE_UNKNOWN; // display mode
  acetime_t now; // seconds from AceTime epoch
  bool suppressBlink = false; // true if blinking should be suppressed
  bool blinkShowState = true; // true if blinking info should be shown
  const char* name;
  uint8_t hourMode;
  bool blinkingColon;
  TimeZone timeZone;
};

#endif
