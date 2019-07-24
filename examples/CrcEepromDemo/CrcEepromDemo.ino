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
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR);

  crcEeprom.begin(32);

  Info info;

  SERIAL_PORT_MONITOR.print("Original info: ");
  SERIAL_PORT_MONITOR.print("startTime: "); SERIAL_PORT_MONITOR.print(info.startTime);
  SERIAL_PORT_MONITOR.print("; interval: "); SERIAL_PORT_MONITOR.println(info.interval);

  SERIAL_PORT_MONITOR.println("Writing Info struct");
  uint16_t writtenSize = crcEeprom.writeWithCrc(0, &info, sizeof(info));
  SERIAL_PORT_MONITOR.print("Written size: "); SERIAL_PORT_MONITOR.println(writtenSize);

  SERIAL_PORT_MONITOR.println("Clearing info struct");
  info.startTime = 0;
  info.interval = 0;
  SERIAL_PORT_MONITOR.println("Reading back Info struct");
  bool isValid = crcEeprom.readWithCrc(0, &info, sizeof(info));
  SERIAL_PORT_MONITOR.print("isValid: "); SERIAL_PORT_MONITOR.println(isValid);

  SERIAL_PORT_MONITOR.print("info: ");
  SERIAL_PORT_MONITOR.print("startTime: "); SERIAL_PORT_MONITOR.print(info.startTime);
  SERIAL_PORT_MONITOR.print("; interval: "); SERIAL_PORT_MONITOR.println(info.interval);
}

void loop() {}
