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

  assertEqual(UtcOffset::forHour(-8).toMinutes(),
      spec.stdOffset().toMinutes());
  assertTrue(spec.isDst());
  assertEqual("PST", spec.stdAbbrev());
  assertEqual("PDT", spec.dstAbbrev());
  assertEqual(UtcOffset::forHour(2).toMinutes(),
      spec.deltaOffset().toMinutes());
}

test(ManualZoneSpecifierTest, setters) {
  ManualZoneSpecifier spec(UtcOffset::forHour(-8), false, "PST", "PDT");

  // test stdOffset(offset)
  spec.stdOffset(UtcOffset::forHour(12));
  assertEqual(UtcOffset::forHour(12).toMinutes(),
      spec.stdOffset().toMinutes());

  // test isDst(flag)
  spec.isDst(true);
  assertTrue(spec.isDst());
}

test(ManualZoneSpecifierTest, overrides) {
  ManualZoneSpecifier spec(UtcOffset::forHour(-8), false, "PST", "PDT");

  assertFalse(spec.isDst());
  assertEqual(UtcOffset::forHour(-8).toMinutes(),
      spec.getUtcOffset(0).toMinutes());
  assertEqual(UtcOffset::forHour(0).toMinutes(),
      spec.getDeltaOffset(0).toMinutes());
  assertEqual("PST", spec.getAbbrev(0));

  spec.isDst(true);
  assertEqual(UtcOffset::forHour(-7).toMinutes(),
      spec.getUtcOffset(0).toMinutes());
  assertEqual(UtcOffset::forHour(1).toMinutes(),
      spec.getDeltaOffset(0).toMinutes());
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
