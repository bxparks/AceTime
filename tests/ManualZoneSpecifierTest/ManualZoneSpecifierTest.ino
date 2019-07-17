#line 2 "ManualZoneSpecifier.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ManualZoneSpecifier
// --------------------------------------------------------------------------

test(ManualZoneSpecifierTest, operatorEqualEqual) {
  ManualZoneSpecifier a(TimeOffset::forHour(-8), false);

  ManualZoneSpecifier b(TimeOffset::forHour(-8), false);
  assertTrue(a == b);

  b = ManualZoneSpecifier(TimeOffset::forHour(-7), false);
  assertFalse(a == b);

  b = ManualZoneSpecifier(TimeOffset::forHour(-8), true);
  assertFalse(a == b);

  b = ManualZoneSpecifier(TimeOffset::forHour(-8), false, "PST");
  assertFalse(a == b);

  b = ManualZoneSpecifier(TimeOffset::forHour(-8), false, "", "PDT");
  assertFalse(a == b);

  b = ManualZoneSpecifier(TimeOffset::forHour(-8), false, "", "",
      TimeOffset::forHour(2));
  assertFalse(a == b);
}

test(ManualZoneSpecifierTest, copyConstructor) {
  ManualZoneSpecifier a(TimeOffset::forHour(-8), false);
  ManualZoneSpecifier b(a);
  assertTrue(a == b);
}

test(ManualZoneSpecifierTest, getters) {
  ManualZoneSpecifier spec(TimeOffset::forHour(-8), true, "PST", "PDT",
      TimeOffset::forHour(2));

  assertEqual(TimeOffset::forHour(-8).toMinutes(),
      spec.stdOffset().toMinutes());
  assertTrue(spec.isDst());
  assertEqual("PST", spec.stdAbbrev());
  assertEqual("PDT", spec.dstAbbrev());
  assertEqual(TimeOffset::forHour(2).toMinutes(),
      spec.deltaOffset().toMinutes());
}

test(ManualZoneSpecifierTest, setters) {
  ManualZoneSpecifier spec(TimeOffset::forHour(-8), false, "PST", "PDT");

  // test setStdOffset(offset)
  spec.setStdOffset(TimeOffset::forHour(12));
  assertEqual(TimeOffset::forHour(12).toMinutes(),
      spec.stdOffset().toMinutes());

  // test setDst(flag)
  spec.setDst(true);
  assertTrue(spec.isDst());
}

test(ManualZoneSpecifierTest, overrides) {
  ManualZoneSpecifier spec(TimeOffset::forHour(-8), false, "PST", "PDT");

  assertFalse(spec.isDst());
  assertEqual(TimeOffset::forHour(-8).toMinutes(),
      spec.getUtcOffset(0).toMinutes());
  assertEqual(TimeOffset::forHour(0).toMinutes(),
      spec.getDeltaOffset(0).toMinutes());
  assertEqual("PST", spec.getAbbrev(0));

  spec.setDst(true);
  assertEqual(TimeOffset::forHour(-7).toMinutes(),
      spec.getUtcOffset(0).toMinutes());
  assertEqual(TimeOffset::forHour(1).toMinutes(),
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
