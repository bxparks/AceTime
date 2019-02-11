#include <Print.h>
#include "AutoZoneSpecifier.h"

namespace ace_time {

void AutoZoneSpecifier::printTo(Print& printer) const {
  printer.print('[');
  printer.print(getZoneInfo()->name);
  printer.print(']');
}

}

