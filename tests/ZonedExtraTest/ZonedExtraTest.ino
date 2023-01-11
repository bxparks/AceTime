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
