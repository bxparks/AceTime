#line 2 "ExtendedDateUtilTest.ino"

/*
 * This unit test depends on 'validation_data.cpp' which is so large that it
 * will likely not compile on an Arduino environment. It can however be run on
 * a Linux or MacOS environment using the provided Makefile.
 */

#include <AUnitVerbose.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

#if 0
  TestRunner::exclude("*");
  TestRunner::include("TransitionTest", "Adak");
#endif
}

void loop() {
  TestRunner::run();
}
