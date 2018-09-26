/*
 * A simple command line (i.e. ascii clock using the Serial line. Useful for
 * debugging. Requires no additional hardware in the simple case. Optionally
 * depends on the DS3231 RTC chip, or an NTP client.
 *
 * Depends on:
 *    * Wire
 *    * AceTime
 *    * AceRoutine
 *
 * Supported boards are:
 *    * Arduino Nano
 *    * Arduino Pro Mini
 *    * Arduino Leonardo (Pro Micro clone)
 *    * ESP8266
 *    * ESP32
 */

#include <Wire.h>
#include <AceRoutine.h>
#include <ace_routine/cli/CommandManager.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "config.h"
#include "Controller.h"

using namespace ace_routine;
using namespace ace_routine::cli;
using namespace ace_time;

//---------------------------------------------------------------------------
// Compensate for buggy F() implementation in ESP8266.
//---------------------------------------------------------------------------

#if defined(ESP8266)
  #define FF(x) (x)
#else
  #define FF(x) F(x)
#endif

//---------------------------------------------------------------------------
// Configure RTC and TimeKeeper
//---------------------------------------------------------------------------

#if defined(USE_DS3231)
  DS3231TimeKeeper dsTimeKeeper;
  SystemTimeKeeper systemTimeKeeper(&dsTimeKeeper, &dsTimeKeeper /*backup*/);
#elif defined(USE_NTP)
  NtpTimeProvider ntpTimeProvider(AUNITER_SSID, AUNITER_PASSWORD);
  SystemTimeKeeper systemTimeKeeper(&ntpTimeProvider, nullptr /*backup*/);
#elif defined(USE_SYSTEM)
  SystemTimeKeeper systemTimeKeeper(nullptr /*sync*/, nullptr /*backup*/);
#else
  #error Unknown time keeper option
#endif

#if SYNC_TYPE == SYNC_TYPE_COROUTINE
  SystemTimeSyncCoroutine systemTimeSync(systemTimeKeeper);
  SystemTimeHeartbeatCoroutine systemTimeHeartbeat(systemTimeKeeper);
#else
  SystemTimeLoop systemTimeLoop;
#endif

//---------------------------------------------------------------------------
// Create a controller retrieves or modifies the underlying clock.
//---------------------------------------------------------------------------

hw::CrcEeprom crcEeprom;
Controller controller(crcEeprom, systemTimeKeeper);

//---------------------------------------------------------------------------
// AceRoutine CLI commands
//---------------------------------------------------------------------------

#define SHIFT do { argv++; argc--; } while (false)

/** List the coroutines known by the CoroutineScheduler. */
void listCommand(Print& printer, int /* argc */, const char** /* argv */) {
  CoroutineScheduler::list(printer);
}

/**
 * Date command. Usage:
 *    date - print current date
 *    date -s {iso8601} - set current date
 */
void dateCommand(Print& printer, int argc, const char** argv) {
  // parse the command line arguments
  const char* newDateString = nullptr;
  SHIFT;
  while (argc > 0) {
    if (strcmp(*argv, "-s") == 0) {
      SHIFT;
      if (argc == 0) {
        printer.println(FF("No date after -s flag"));
        return;
      }
      newDateString = *argv;
    } else if (**argv == '-') {
      printer.print(FF("Unknown flag: "));
      printer.println(*argv);
      return;
    } else {
      break;
    }
    SHIFT;
  }

  if (newDateString != nullptr) {
    DateTime newDate = DateTime::forDateString(newDateString);
    if (newDate.isError()) {
      printer.print(FF("Invalid date: "));
      printer.println(newDateString);
      return;
    }
    controller.setDateTime(newDate);
    printer.print(FF("Date set to: "));
    newDate.printTo(printer);
    printer.println();
  } else {
    DateTime now = controller.now();
    now.printTo(printer);
    printer.println();
  }
}

/**
 * Timezone command. Usage:
 *    timezone - print current timezone
 *    timezone -s {code} - set current timezone
 */
void timezoneCommand(Print& printer, int argc, const char** argv) {
  // parse the command line arguments
  const char* newTimeZoneString = nullptr;
  SHIFT;
  while (argc > 0) {
    if (strcmp(*argv, "-s") == 0) {
      SHIFT;
      if (argc == 0) {
        printer.println(FF("No tzCode after -s flag"));
        return;
      }
      newTimeZoneString = *argv;
    } else if (**argv == '-') {
      printer.print(FF("Unknown flag: "));
      printer.println(*argv);
      return;
    } else {
      break;
    }
    SHIFT;
  }

  if (newTimeZoneString != nullptr) {
    TimeZone tz = TimeZone::forOffsetString(newTimeZoneString);
    if (tz.isError()) {
      printer.println(FF("Invalid time zone"));
      return;
    }
    controller.setTimeZone(tz);
    printer.print(FF("Time zone set to: UTC"));
    tz.printTo(printer);
    printer.println();
  } else {
    TimeZone timeZone = controller.timeZone();
    printer.print(FF("UTC"));
    timeZone.printTo(printer);
    printer.println();
  }
}

// Create an instance of the CommandManager.
const uint8_t TABLE_SIZE = 3;
const uint8_t BUF_SIZE = 64;
const uint8_t ARGV_SIZE = 5;
#if defined(AVR) || defined(__arm__) || defined(ESP32)
CommandManager<__FlashStringHelper, BUF_SIZE, ARGV_SIZE>
    commandManager(Serial, TABLE_SIZE, "> ");
#elif defined(ESP8266)
CommandManager<char, BUF_SIZE, ARGV_SIZE>
    commandManager(Serial, TABLE_SIZE, "> ");
#else
  #error Unsupported board
#endif

//---------------------------------------------------------------------------
// Main setup and loop
//---------------------------------------------------------------------------

void setup() {
  // Wait for stability on some boards.
  // 1000ms needed for Serial.
  // 2000ms needed for Wire, I2C or SSD1306 (don't know which one).
  delay(2000);

#if defined(ARDUINO_AVR_LEONARDO)
  RXLED0; // LED off
  TXLED0; // LED off
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(FF("setup(): begin"));

  Wire.begin();
  Wire.setClock(400000L);

#if defined(USE_DS3231)
  dsTimeKeeper.setup();
#elif defined(USE_NTP)
  ntpTimeProvider.setup();
#endif

  systemTimeKeeper.setup();
  crcEeprom.begin(EEPROM_SIZE);
  controller.setup();

  // add commands
  commandManager.add(dateCommand, FF("date"), FF("[-s dateString]"));
  commandManager.add(timezoneCommand, FF("timezone"), FF("[-s utcOffset]"));
  commandManager.add(listCommand, FF("list"), nullptr);

  // insert into the scheduler
#if SYNC_TYPE == SYNC_TYPE_COROUTINE
  systemTimeSync.setupCoroutine("systemTimeSync");
  systemTimeHeartbeat.setupCoroutine("systemTimeHeartbeat");
#endif
  commandManager.setupCoroutine("commandManager");
  CoroutineScheduler::setup();

  Serial.println(FF("setup(): end"));
}

void loop() {
  CoroutineScheduler::loop();
#if SYNC_TYPE == SYNC_TYPE_MANUAL
  systemTimeLoop.loop();
#endif
}
