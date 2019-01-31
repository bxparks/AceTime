#line 2 "ValidationTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include "ValidationDataType.h"
#include "validation_data.h"

using namespace aunit;
using namespace ace_time;
using namespace ace_time::zonedb;

// --------------------------------------------------------------------------

void setup() {
#if !defined(__linux__) && !defined(__APPLE__)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
