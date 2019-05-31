#include <Print.h>
#include "BasicZoneSpecifier.h"

namespace ace_time {

void BasicZoneSpecifier::printTo(Print& printer) const {
  printer.print('[');
  printer.print(getZoneInfo()->name);
  printer.print(']');
}

}

