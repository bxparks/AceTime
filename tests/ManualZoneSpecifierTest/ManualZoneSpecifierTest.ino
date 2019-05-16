#line 2 "ManualZoneSpecifier.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ManualZoneSpecifier
// --------------------------------------------------------------------------

test(ManualZoneSpecifierTest, operatorEqualEqual) {
  ManualZoneSpecifier a(UtcOffset::forHour(-8), false);

  ManualZoneSpecifier b(UtcOffset::forHour(-8), false);
  assertTrue(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-7), false);
  assertFalse(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), true);
  assertFalse(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), false, "PST");
  assertFalse(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), false, "", "PDT");
  assertFalse(a == b);

  b = ManualZoneSpecifier(UtcOffset::forHour(-8), false, "", "",
      UtcOffset::forHour(2));
  assertFalse(a == b);
}

test(ManualZoneSpecifierTest, copyConstructor) {
  ManualZoneSpecifier a(UtcOffset::forHour(-8), false);
  ManualZoneSpecifier b(a);
  assertTrue(a == b);
}

test(ManualZoneSpecifierTest, getters) {
  ManualZoneSpecifier spec(UtcOffset::forHour(-8), true, "PST", "PDT",
      UtcOffset::forHour(2));

  assertEqual(UtcOffset::forHour(-8).code(), spec.stdOffset().code());
  assertTrue(spec.isDst());
  assertEqual("PST", spec.stdAbbrev());
  assertEqual("PDT", spec.dstAbbrev());
  assertEqual(UtcOffset::forHour(2).code(), spec.deltaOffset().code());
}

test(ManualZoneSpecifierTest, setters) {
  ManualZoneSpecifier spec(UtcOffset::forHour(-8), false, "PST", "PDT");

  // test stdOffset(offset)
  spec.stdOffset(UtcOffset::forHour(12));
  assertEqual(UtcOffset::forHour(12).code(), spec.stdOffset().code());

  // test isDst(flag)
  spec.isDst(true);
  assertTrue(spec.isDst());
}

test(ManualZoneSpecifierTest, overrides) {
  ManualZoneSpecifier spec(UtcOffset::forHour(-8), false, "PST", "PDT");

  assertFalse(spec.isDst());
  assertEqual(UtcOffset::forHour(-8).code(), spec.getUtcOffset(0).code());
  assertEqual(UtcOffset::forHour(0).code(), spec.getDeltaOffset(0).code());
  assertEqual("PST", spec.getAbbrev(0));

  spec.isDst(true);
  assertEqual(UtcOffset::forHour(-7).code(), spec.getUtcOffset(0).code());
  assertEqual(UtcOffset::forHour(1).code(), spec.getDeltaOffset(0).code());
  assertEqual("PDT", spec.getAbbrev(0));
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
