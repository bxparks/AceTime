/*
 * MIT License
 * Copyright (c) 2024 Brian T. Park
 */

#include "DateConv.h"

namespace ace_time {

void secondsToHms(uint32_t secs, uint16_t* hh, uint16_t* mm, uint16_t* ss) {
  *ss = secs % 60;
  uint32_t minutes = secs / 60;
  *mm = uint16_t(minutes % 60);
  *hh = uint16_t(minutes / 60);
}

}
