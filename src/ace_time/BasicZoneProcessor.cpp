/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Print.h>
#include "BasicZone.h"
#include "BasicZoneProcessor.h"

namespace ace_time {

void BasicZoneProcessor::printTo(Print& printer) const {
  printer.print(BasicZone(mZoneInfo.zoneInfo()).name());
}

void BasicZoneProcessor::printShortTo(Print& printer) const {
  printer.print(BasicZone(mZoneInfo.zoneInfo()).shortName());
}

}

