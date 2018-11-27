#include "common/Util.h"
#include "AutoTimeZone.h"

namespace ace_time {

using common::printPad2;

void AutoTimeZone::printTo(Print& printer) const {
  printer.print('[');
  printer.print(mZoneManager.getZoneInfo()->name);
  printer.print(']');
}

}
