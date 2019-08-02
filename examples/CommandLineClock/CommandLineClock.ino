/*
 * A simple command line (i.e. ascii clock using the SERIAL_PORT_MONITOR line. Useful for
 * debugging. Requires no additional hardware in the simple case. Optionally
 * depends on the DS3231 RTC chip, or an NTP client.
 *
 * Depends on the following libaries:
 *    * Wire  (built-in to Arduino IDE package)
 *    * AceTime (https://github.com/bxparks/AceTime)
 *    * AceRoutine (https://github.com/bxparks/AceRoutine)
 *
 * Supported boards are:
 *    * Arduino Nano
 *    * Arduino Pro Mini
 *    * Arduino Leonardo or Pro Micro
 *    * SAMD21
 *    * ESP8266
 *    * ESP32
 *
 * The following commands on the serial monitor are supported:
 *
 *    help [command]
 *        Print the list of supported commands.
 *    list
 *        List the AceRoutine coroutines.
 *    date [dateString]
 *        Print or set the date.
 *    timezone [manual {offset} | dst (on | off)] |
 *        Print or set the current TimeZone.
 *    basic [list] | extended [list] ]
 *        Print or set the currently active TimeZone.
 *    sync [status]
 *        Sync the SystemClock from its external source, or print its sync
 *        status.
 *		wifi (status | config [{ssid} {password}] | connect)
 *        Print the ESP8266 or ESP32 wifi connection info.
 *        Connect to the wifi network.
 *        Print or set the wifi ssid and password.
 */

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <Wire.h>
#include <AceRoutine.h>
#include <ace_routine/cli/CommandManager.h>
#include <AceTime.h>
#include "config.h"
#include "Controller.h"
#include "PersistentStore.h"

using namespace ace_routine;
using namespace ace_routine::cli;
using namespace ace_time;
using namespace ace_time::clock;

//---------------------------------------------------------------------------
// Configure RTC and Clock
//---------------------------------------------------------------------------

#if SYNC_TYPE == SYNC_TYPE_COROUTINE
  #define SYSTEM_CLOCK SystemClockCoroutine
#else
  #define SYSTEM_CLOCK SystemClockLoop
#endif

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  DS3231Clock dsClock;
  SYSTEM_CLOCK systemClock(&dsClock, &dsClock /*backup*/);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  NtpClock ntpClock;
  SYSTEM_CLOCK systemClock(&ntpClock, nullptr /*backup*/);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NONE
  SYSTEM_CLOCK systemClock(nullptr /*sync*/, nullptr /*backup*/);
#else
  #error Unknown clock option
#endif

//---------------------------------------------------------------------------
// Create a controller retrieves or modifies the underlying clock.
//---------------------------------------------------------------------------

PersistentStore persistentStore;
Controller controller(persistentStore, systemClock);

//---------------------------------------------------------------------------
// AceRoutine CLI commands
//---------------------------------------------------------------------------

/** List the coroutines known by the CoroutineScheduler. */
class ListCommand: public CommandHandler {
  public:
    ListCommand():
        CommandHandler(F("list"), nullptr) {}

    void run(Print& printer, int /*argc*/, const char* const* /*argv*/)
            const override {
      CoroutineScheduler::list(printer);
    }
};

/**
 * Date command.
 * Usage:
 *    date - print current date
 *    date {iso8601} - set current date
 */
class DateCommand: public CommandHandler {
  public:
    DateCommand():
        CommandHandler(F("date"), F("[dateString]")) {}

    void run(Print& printer, int argc, const char* const* argv) const override {
      if (argc == 1) {
        ZonedDateTime nowDate = controller.getCurrentDateTime();
        nowDate.printTo(printer);
        printer.println();
      } else {
        SHIFT_ARGC_ARGV(argc, argv);
        ZonedDateTime newDate = ZonedDateTime::forDateString(argv[0]);
        if (newDate.isError()) {
          printer.println(F("Invalid date"));
          return;
        }
        controller.setNow(newDate.toEpochSeconds());
        printer.print(F("Date set to: "));
        newDate.printTo(printer);
        printer.println();
      }
    }
};

/**
 * Timezone command.
 * Usage:
 *    timezone - print current timezone
 *    timezone list - print support time zones
 *    timezone manual {timeOffset} - set Manual TimeZone with given offset
 *    timezone dst {on | off} - set Manual TimeZone DST flag to on or off
 *    timezone basic - set timezone to BasicZoneProcessor (if supported)
 *    timezone extended - set timezone to ExtendedZoneProcessor (if supported)
 */
class TimezoneCommand: public CommandHandler {
  public:
    TimezoneCommand():
      CommandHandler(F("timezone"),
        F("manual {offset} | "
      #if ENABLE_TIME_ZONE_TYPE_BASIC
        "basic [list | {index}] | "
      #endif
      #if ENABLE_TIME_ZONE_TYPE_EXTENDED
        "extended [list | {index}] | "
      #endif
        "dst {on | off}]")) {}

    void run(Print& printer, int argc, const char* const* argv) const override {
      if (argc == 1) {
        const TimeZone& timeZone = controller.getTimeZone();
        timeZone.printTo(printer);
        printer.println();
        return;
      }

      SHIFT_ARGC_ARGV(argc, argv);
      if (isArgEqual(argv[0], F("manual"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        if (argc == 0) {
          printer.print(F("'timezone manual' requires 'offset'"));
          return;
        }
        TimeOffset offset = TimeOffset::forOffsetString(argv[0]);
        if (offset.isError()) {
          printer.println(F("Invalid time zone offset"));
          return;
        }
        controller.setManualTimeZone(offset, TimeOffset());
        printer.print(F("Time zone set to: "));
        controller.getTimeZone().printTo(printer);
        printer.println();
      #if ENABLE_TIME_ZONE_TYPE_BASIC
      } else if (isArgEqual(argv[0], F("basic"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        if (argc != 0 && isArgEqual(argv[0], F("list"))) {
          controller.printBasicZonesTo(printer);
        } else {
          int16_t zoneIndex = (argc == 0) ? 0 : atoi(argv[0]);
          controller.setBasicTimeZoneForIndex(zoneIndex);
          printer.print(F("Time zone set to: "));
          controller.getTimeZone().printTo(printer);
          printer.println();
        }
      #endif
      #if ENABLE_TIME_ZONE_TYPE_EXTENDED
      } else if (isArgEqual(argv[0], F("extended"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        if (argc != 0 && isArgEqual(argv[0], F("list"))) {
          controller.printExtendedZonesTo(printer);
        } else {
          int16_t zoneIndex = (argc == 0) ? 0 : atoi(argv[0]);
          controller.setExtendedTimeZoneForIndex(zoneIndex);
          printer.print(F("Time zone set to: "));
          controller.getTimeZone().printTo(printer);
          printer.println();
        }
      #endif
      } else if (isArgEqual(argv[0], F("dst"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        if (argc == 0) {
          printer.print(F("DST: "));
          printer.println(controller.isDst() ? F("on") : F("off"));
          return;
        }

        if (isArgEqual(argv[0], F("on"))) {
          controller.setDst(true);
        } else if (isArgEqual(argv[0], F("off"))) {
          controller.setDst(false);
        } else {
          printer.print(F("'timezone dst' must be either 'on' or 'off'"));
        }
      } else {
        // If we get here, we don't recognize the subcommand.
        printer.print(F("Unknown option ("));
        printer.print(argv[0]);
        printer.println(")");
      }
    }
};

/**
 * Sync command - force SystemClock to sync with its time source
 * Usage:
 *    sync
 */
class SyncCommand: public CommandHandler {
  public:
    SyncCommand(SystemClock& systemClock):
        CommandHandler(F("sync"), F("[status]")),
        mSystemClock(systemClock) {}

    void run(Print& printer, int argc, const char* const* argv)
        const override {
      if (argc == 1) {
        controller.forceSync();
        printer.print(F("Date set to: "));
        ZonedDateTime currentDateTime = controller.getCurrentDateTime();
        currentDateTime.printTo(printer);
        printer.println();
        return;
      }

      SHIFT_ARGC_ARGV(argc, argv);
      if (isArgEqual(argv[0], F("status"))) {
        printer.print(F("Last synced: "));
        if (mSystemClock.isInit()) {
          acetime_t ago = mSystemClock.getNow()
              - mSystemClock.getLastSyncTime();
          printer.print(ago);
          printer.println(F("s ago"));
        } else {
          printer.println(F("<Never>"));
        }
        return;
      }

      printer.print(F("Unknown argument: "));
      printer.println(argv[0]);
    }

  private:
    SystemClock& mSystemClock;
};

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP

/**
 * WiFi manager command.
 * Usage:
 *    wifi status - print current wifi parameters
 *    wifi config - print current ssid and password
 *    wifi config {ssid} {password} - set new ssid and password
 */
class WifiCommand: public CommandHandler {
  public:
    WifiCommand(
        Controller& controller,
        NtpClock& ntpClock):
      CommandHandler(F("wifi"),
          F("status | (config [{ssid} {password}]) | connect") ),
      mController(controller),
      mNtpClock(ntpClock)
      {}

    void run(Print& printer, int argc, const char* const* argv) const override {
      SHIFT_ARGC_ARGV(argc, argv);
      if (argc == 0) {
        printer.println(F("Must give either 'status' or 'config' command"));
      } else if (isArgEqual(argv[0], F("config"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        if (argc == 0) {
          if (mController.isStoredInfoValid()) {
            // print ssid and password
            const StoredInfo& storedInfo = mController.getStoredInfo();
            printer.print(F("ssid: "));
            printer.println(storedInfo.ssid);
            printer.print(F("password: "));
            printer.println(storedInfo.password);
          } else {
            printer.println(
              F("Invalid ssid and password in persistent storage"));
          }
        } else if (argc == 2) {
          const char* ssid = argv[0];
          const char* password = argv[1];
          mController.setWiFi(ssid, password);
          connect(printer);
        } else {
          printer.println(F("Wifi config command requires 2 arguments"));
        }
      } else if (isArgEqual(argv[0], F("status"))) {
        printer.print(F("NtpClock::isSetup(): "));
        printer.println(mNtpClock.isSetup() ? F("true") : F("false"));
        printer.print(F("NTP Server: "));
        printer.println(mNtpClock.getServer());
        printer.print(F("WiFi IP address: "));
        printer.println(WiFi.localIP());
      } else if (isArgEqual(argv[0], F("connect"))) {
        connect(printer);
      } else {
        printer.print(F("Unknown wifi command: "));
        printer.println(argv[0]);
      }
    }

    void connect(Print& printer) const {
      if (! controller.isStoredInfoValid()) {
        printer.println(F("Invalid ssid and password"));
        return;
      }

      const StoredInfo& storedInfo = mController.getStoredInfo();
      const char* ssid = storedInfo.ssid;
      const char* password = storedInfo.password;
      mNtpClock.setup(ssid, password);
      if (mNtpClock.isSetup()) {
        printer.println(F("Connection succeeded."));
      } else {
        printer.println(F("Connection failed... run 'wifi connect' again"));
      }
    }

  private:
    Controller& mController;
    NtpClock& mNtpClock;
};

#endif

// Create a list of CommandHandlers.
ListCommand listCommand;
DateCommand dateCommand;
SyncCommand syncCommand(systemClock);
TimezoneCommand timezoneCommand;
#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
WifiCommand wifiCommand(controller, ntpClock);
#endif

const CommandHandler* const COMMANDS[] = {
  &listCommand,
  &dateCommand,
  &syncCommand,
  &timezoneCommand,
#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  &wifiCommand,
#endif
};
uint8_t const NUM_COMMANDS = sizeof(COMMANDS) / sizeof(CommandHandler*);

// Create an instance of the CommandManager.
uint8_t const BUF_SIZE = 64;
uint8_t const ARGV_SIZE = 5;
CommandManager<BUF_SIZE, ARGV_SIZE> commandManager(
    COMMANDS, NUM_COMMANDS, SERIAL_PORT_MONITOR, "> ");

//---------------------------------------------------------------------------
// Main setup and loop
//---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  // Wait for stability on some boards.
  // 1000ms needed for SERIAL_PORT_MONITOR.
  // 2000ms needed for Wire, I2C or SSD1306 (don't know which one).
  delay(2000);
#endif

#if defined(ARDUINO_AVR_LEONARDO)
  RXLED0; // LED off
  TXLED0; // LED off
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
  SERIAL_PORT_MONITOR.println(F("setup(): begin"));

  SERIAL_PORT_MONITOR.print(F("sizeof(StoredInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(StoredInfo));

#if defined(ARDUINO)
  Serial.println(F("Setting up Wire"));
  Wire.begin();
  Wire.setClock(400000L);
#endif

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  Serial.println(F("Setting up DS3231Clock"));
  dsClock.setup();
#endif

  Serial.println(F("Setting up PersistentStore"));
#if defined(ARDUINO)
  persistentStore.setup();
#else
  persistentStore.setup("commandline.dat");
#endif

  Serial.println(F("Setting up SystemClock"));
  systemClock.setup();

  Serial.println(F("Setting up Controller"));
  controller.setup();

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  Serial.println(F("Automatically connecting to Wifi"));
  wifiCommand.connect(SERIAL_PORT_MONITOR);
#endif

  // insert coroutines into the scheduler
#if SYNC_TYPE == SYNC_TYPE_COROUTINE
  Serial.println(F("Setting up SystemClock coroutine"));
  systemClock.setupCoroutine(F("systemClock"));
#endif

  Serial.println(F("Setting up CommandManager"));
  commandManager.setupCoroutine(F("commandManager"));

  Serial.println(F("Setting up CoroutineScheduler"));
  CoroutineScheduler::setup();

  SERIAL_PORT_MONITOR.println(F("setup(): end"));
}

void loop() {
  systemClock.keepAlive();
#if SYNC_TYPE == SYNC_TYPE_MANUAL
  systemClock.loop();
#endif

  CoroutineScheduler::loop();
}
