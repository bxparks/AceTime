#line 2 "ManualZoneSpecifier.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ManualZoneSpecifier
// --------------------------------------------------------------------------

test(ManualZoneSpecifierTest, operatorEqualEqual) {
  ManualZoneSpecifier a(UtcOffset::forHour(-8), UtcOffset::forHour(1));

  ManualZoneSpecifier b(UtcOffset::forHour(-8), UtcOffset::forHour(1));
  assertTrue(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), UtcOffset::forHour(1), "",
      "");
  assertTrue(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), UtcOffset::forHour(2));
  assertTrue(a != b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), UtcOffset::forHour(1), "a");
  assertTrue(a != b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), UtcOffset::forHour(1), "",
      "a");
  assertTrue(a != b);
}

test(ManualZoneSpecifierTest, copyConstructor) {
  ManualZoneSpecifier a(UtcOffset::forHour(-8), UtcOffset::forHour(1));
  ManualZoneSpecifier b(a);
  assertTrue(a == b);
}

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
