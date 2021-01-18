#line 2 "ExtendedJavaTest.ino"

/*
 * This unit test depends on 'validation_data.cpp' which is so large that
 * it will likely not compile on an Arduino environment. It can however be run
 * on a Linux or MacOS environment using the provided Makefile.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

//---------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

#if 0
  TestRunner::exclude("*");
  TestRunner::include("TransitionTest", "Gaza");
  TestRunner::include("TransitionTest", "Goose_Bay");
  TestRunner::include("TransitionTest", "Hebron");
  TestRunner::include("TransitionTest", "Moncton");
  TestRunner::include("TransitionTest", "St_Johns");
#endif
}

void loop() {
  TestRunner::run();
}
