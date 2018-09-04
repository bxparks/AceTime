#line 2 "HardwareTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time::hw;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// HardwareTemperature
// --------------------------------------------------------------------------

test(temperatureCompareAndEquals) {
  HardwareTemperature a;
  HardwareTemperature b;

  a = {1, 2};
  b = {1, 2};
  assertTrue(a == b);
  assertFalse(a != b);

  a = {1, 2};
  b = {2, 2};
  assertFalse(a == b);
  assertTrue(a != b);

  a = {1, 2};
  b = {1, 3};
  assertFalse(a == b);
  assertTrue(a != b);
}

test(temperatureToTemperature256) {
  HardwareTemperature a = {1, 2};
  assertEqual(258, a.toTemperature256());
}

// --------------------------------------------------------------------------
// HardwareTemperature
// --------------------------------------------------------------------------

test(dateTimeEqual) {
  HardwareDateTime a;
  HardwareDateTime b;

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 1, 1, 12, 0, 0, 1};
  assertTrue(a == b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {19, 1, 1, 12, 0, 0, 1};
  assertTrue(a != b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 2, 1, 12, 0, 0, 1};
  assertTrue(a != b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 1, 2, 12, 0, 0, 1};
  assertTrue(a != b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 1, 1, 13, 0, 0, 1};
  assertTrue(a != b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 1, 1, 12, 1, 0, 1};
  assertTrue(a != b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 1, 1, 12, 0, 1, 1};
  assertTrue(a != b);

  a = {18, 1, 1, 12, 0, 0, 1};
  b = {18, 1, 1, 12, 0, 0, 2};
  assertTrue(a != b);
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
