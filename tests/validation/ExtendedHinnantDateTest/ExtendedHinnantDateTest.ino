#line 2 "ExtendedHinnantDateTest.ino"

/*
 * This unit test depends on 'validation_data.cpp' which is so large that it
 * will likely not compile on an Arduino environment. It can however be run on
 * a Linux or MacOS environment using the provided Makefile.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
