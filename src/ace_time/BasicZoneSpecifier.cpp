/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Print.h>
#include "BasicZoneSpecifier.h"

namespace ace_time {

void BasicZoneSpecifier::printTo(Print& printer) const {
#if ACE_TIME_USE_BASIC_PROGMEM
  printer.print(FPSTR(mZoneInfo.name()));
#else
  printer.print(mZoneInfo.name());
#endif
}

}

