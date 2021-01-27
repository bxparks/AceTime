/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Print.h>
#include "BasicZone.h"
#include "BasicZoneProcessor.h"

namespace ace_time {

void BasicZoneProcessor::printTo(Print& printer) const {
  BasicZone(mZoneInfo.zoneInfo()).printNameTo(printer);
}

void BasicZoneProcessor::printShortTo(Print& printer) const {
  BasicZone(mZoneInfo.zoneInfo()).printShortNameTo(printer);
}

}

