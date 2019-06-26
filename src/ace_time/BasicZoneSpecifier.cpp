#include <Print.h>
#include "BasicZoneSpecifier.h"

namespace ace_time {

void BasicZoneSpecifier::printTo(Print& printer) const {
  printer.print(getZoneInfo()->name);
}

}

