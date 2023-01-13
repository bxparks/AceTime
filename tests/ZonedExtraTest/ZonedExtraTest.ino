#line 2 "ZonedExtraTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------

test(ZonedExtra, isError) {
  ZonedExtra ze;
  assertTrue(ze.isError());
}

test(ZonedExtra, type) {
  assertEqual(ZonedExtra::kTypeNotFound, FindResult::kTypeNotFound);
  assertEqual(ZonedExtra::kTypeExact, FindResult::kTypeExact);
  assertEqual(ZonedExtra::kTypeGap, FindResult::kTypeGap);
  assertEqual(ZonedExtra::kTypeOverlap, FindResult::kTypeOverlap);
}

test(ZonedExtra, accessors) {
  const char s[] = "test";
  ZonedExtra ze(1, 2, 3, 4, 5, s);

  assertEqual(ze.type(), 1);
  assertEqual(ze.stdOffset().toMinutes(), 2);
  assertEqual(ze.dstOffset().toMinutes(), 3);
  assertEqual(ze.timeOffset().toMinutes(), 2+3);
  assertEqual(ze.reqStdOffset().toMinutes(), 4);
  assertEqual(ze.reqDstOffset().toMinutes(), 5);
  assertEqual(ze.reqTimeOffset().toMinutes(), 4+5);
  assertEqual(ze.abbrev(), "test");
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
