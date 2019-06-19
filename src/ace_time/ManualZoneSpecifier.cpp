#include <Print.h>
#include "ManualZoneSpecifier.h"

namespace ace_time {

void ManualZoneSpecifier::printTo(Print& printer) const {
  mStdOffset.printTo(printer);
  printer.print(mIsDst ? F("(DST)") : F("(STD)"));
}

}
