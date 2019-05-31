/*
 * Quick test of CrcEeprom class.
 */

#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>

using namespace ace_time::hw;

struct Info {
  int startTime = 100;
  int interval = 200;
};

CrcEeprom crcEeprom;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial);

  crcEeprom.begin(32);

  Info info;

  Serial.print("Original info: ");
  Serial.print("startTime: "); Serial.print(info.startTime);
  Serial.print("; interval: "); Serial.println(info.interval);

  Serial.println("Writing Info struct");
  uint16_t writtenSize = crcEeprom.writeWithCrc(0, &info, sizeof(info));
  Serial.print("Written size: "); Serial.println(writtenSize);

  Serial.println("Clearing info struct");
  info.startTime = 0;
  info.interval = 0;
  Serial.println("Reading back Info struct");
  bool isValid = crcEeprom.readWithCrc(0, &info, sizeof(info));
  Serial.print("isValid: "); Serial.println(isValid);

  Serial.print("info: ");
  Serial.print("startTime: "); Serial.print(info.startTime);
  Serial.print("; interval: "); Serial.println(info.interval);
}

void loop() {}
