#line 2 "AceTimeUtilsTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------

test(AceTimeUtilsTest, daysUntil) {
  LocalDate today = LocalDate::forComponents(2000, 12, 25);
  assertEqual(0, daysUntil(today, 12, 25));

  today = LocalDate::forComponents(2000, 12, 24);
  assertEqual(1, daysUntil(today, 12, 25));

  // 2001 is a normal year, so 364 days until next Christmas
  today = LocalDate::forComponents(2000, 12, 26);
  assertEqual(364, daysUntil(today, 12, 25));

  // 2004 is a leap year so 365 days until next Christmas
  today = LocalDate::forComponents(2003, 12, 26);
  assertEqual(365, daysUntil(today, 12, 25));
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
