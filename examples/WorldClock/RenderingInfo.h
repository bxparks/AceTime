#ifndef WORLD_CLOCK_RENDERING_INFO_H
#define WORLD_CLOCK_RENDERING_INFO_H

#include <AceTime.h>
#include "config.h"
#include "ClockInfo.h"

using namespace ace_time;

/**
 * Data used by the Presenter (the "View") to determine what has changed and
 * what needs to be displayed.
 */ 
struct RenderingInfo {
  uint8_t mode = MODE_UNKNOWN; // display mode
  uint32_t now; // seconds from AceTime epoch
  bool suppressBlink = false; // true if blinking should be suppressed
  bool blinkShowState = true; // true if blinking info should be shown

  // Almost identical to ClockInfo, except that timeZone is a pointer, which
  // saves memory when using AutoTimeZone. The ManualTimeZone::isDst() is
  // denormalized and copied out, so that we can detect changes to that field
  // using a previously saved copy of this object.
  const char* name;
  uint8_t hourMode;
  bool blinkingColon;
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  const ace_time::ManualTimeZone* timeZone;
  bool isDst; // denormalized copy of timeZone.isDst() to detect changes
#else
  const ace_time::AutoTimeZone* timeZone;
#endif
};

#endif
