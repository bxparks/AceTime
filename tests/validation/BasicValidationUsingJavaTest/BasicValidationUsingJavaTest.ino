#line 2 "BasicValidationUsingJavaTest.ino"

/*
 * This unit test depends on 'validation_data.cpp' which is so large that it
 * will likely not compile on an Arduino environment. It can however be run on
 * a Linux or MacOS environment using the provided Makefile.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only

#if 0
  TestRunner::exclude("*");
  //TestRunner::include("TransitionTest", "America_Los_Angeles");
  TestRunner::include("TransitionTest", "America_Moncton");
#endif
}

void loop() {
  TestRunner::run();
}
