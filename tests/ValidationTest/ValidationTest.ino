#line 2 "ValidationTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
  //TestRunner::exclude("*");
  //TestRunner::include("BasicTransitionTest", "Kosrae");
}

void loop() {
  TestRunner::run();
}
