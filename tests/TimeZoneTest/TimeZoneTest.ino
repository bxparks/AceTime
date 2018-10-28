#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// ZoneManager
// --------------------------------------------------------------------------

test(ZoneManager, effectiveZoneOffset) {
  TimeZone tz = TimeZone::forZone(&ZoneInfo::kLosAngeles);
  ZoneOffset offset = tz.effectiveZoneOffset(0);
  assertEqual(-32, offset.offsetCode());
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
