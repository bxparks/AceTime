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
 *      basic [list] | extended [list] ]
 *        Print or set the currently active TimeZone.
 *    sync_status
 *        Print the status of the SystemClockSyncLoop helper.
 *		wifi (status | connect | config [ssid password])
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
// Configure RTC and TimeKeeper
//---------------------------------------------------------------------------

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  DS3231TimeKeeper dsTimeKeeper;
  SystemClock systemClock(&dsTimeKeeper, &dsTimeKeeper /*backup*/);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  NtpTimeProvider ntpTimeProvider;
  SystemClock systemClock(&ntpTimeProvider, nullptr /*backup*/);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NONE
  SystemClock systemClock(nullptr /*sync*/, nullptr /*backup*/);
#else
  #error Unknown time keeper option
#endif

#if SYNC_TYPE == SYNC_TYPE_COROUTINE
  SystemClockSyncCoroutine systemClockSync(systemClock);
#else
  SystemClockSyncLoop systemClockSyncLoop(systemClock);
#endif

//---------------------------------------------------------------------------
// Create a controller retrieves or modifies the underlying clock.
//---------------------------------------------------------------------------

PersistentStore persistentStore;
Controller controller(persistentStore, systemClock);

//---------------------------------------------------------------------------
// AceRoutine CLI commands
//---------------------------------------------------------------------------

/** Shift command arguments to the left by one token. */
#define SHIFT do { argv++; argc--; } while (false)

/** List the coroutines known by the CoroutineScheduler. */
class ListCommand: public CommandHandler {
  public:
    ListCommand():
        CommandHandler("list", nullptr) {}

    void run(Print& printer, int /*argc*/, const char** /*argv*/)
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
        CommandHandler("date", "[dateString]") {}

    void run(Print& printer, int argc, const char** argv) const override {
      if (argc == 1) {
        ZonedDateTime nowDate = controller.getCurrentDateTime();
        nowDate.printTo(printer);
        printer.println();
      } else {
        SHIFT;
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
 * Sync command - force SystemClock to sync with its time source
 * Usage:
 *    sync
 */
class SyncCommand: public CommandHandler {
  public:
    SyncCommand():
        CommandHandler("sync", nullptr) {}

    void run(Print& printer, int /*argc*/, const char** /*argv*/)
        const override {
      controller.sync();
      printer.print(F("Date set to: "));
      ZonedDateTime currentDateTime = controller.getCurrentDateTime();
      currentDateTime.printTo(printer);
      printer.println();
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
      CommandHandler("timezone",
        "manual {offset} | "
      #if ENABLE_TIME_ZONE_TYPE_BASIC
        "basic [list | {index}] | "
      #endif
      #if ENABLE_TIME_ZONE_TYPE_EXTENDED
        "extended [list | {index}] | "
      #endif
        "dst {on | off}]") {}

    void run(Print& printer, int argc, const char** argv) const override {
      if (argc == 1) {
        const TimeZone& timeZone = controller.getTimeZone();
        timeZone.printTo(printer);
        printer.println();
        return;
      }

      SHIFT;
      if (strcmp(argv[0], "manual") == 0) {
        SHIFT;
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
      } else if (strcmp(argv[0], "basic") == 0) {
        SHIFT;
        if (argc != 0 && strcmp(argv[0], "list") == 0) {
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
      } else if (strcmp(argv[0], "extended") == 0) {
        SHIFT;
        if (argc != 0 && strcmp(argv[0], "list") == 0) {
          controller.printExtendedZonesTo(printer);
        } else {
          int16_t zoneIndex = (argc == 0) ? 0 : atoi(argv[0]);
          controller.setExtendedTimeZoneForIndex(zoneIndex);
          printer.print(F("Time zone set to: "));
          controller.getTimeZone().printTo(printer);
          printer.println();
        }
      #endif
      } else if (strcmp(argv[0], "dst") == 0) {
        SHIFT;
        if (argc == 0) {
          printer.print(F("DST: "));
          printer.println(controller.isDst() ? F("on") : F("off"));
          return;
        }

        if (strcmp(argv[0], "on") == 0) {
          controller.setDst(true);
        } else if (strcmp(argv[0], "off") == 0) {
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

#if SYNC_TYPE == SYNC_TYPE_MANUAL

/**
 * Sync status command.
 * Usage:
 *    sync_status - print the sync status
 */
class SyncStatusCommand: public CommandHandler {
  public:
    SyncStatusCommand(SystemClockSyncLoop& systemClockSyncLoop):
          CommandHandler("sync_status", nullptr),
      mSystemClockSyncLoop(systemClockSyncLoop) {}

    void run(Print& printer, int /*argc*/, const char** /*argv*/)
        const override {
      printer.print(F("Seconds since last sync: "));
      printer.println(mSystemClockSyncLoop.getSecondsSinceLastSync());
    }

  private:
    SystemClockSyncLoop& mSystemClockSyncLoop;
};

#endif

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
        NtpTimeProvider& ntpTimeProvider):
      CommandHandler(
          "wifi", "status | (config [ssid password]) | connect" ),
      mController(controller),
      mNtpTimeProvider(ntpTimeProvider)
      {}

    void run(Print& printer, int argc, const char** argv) const override {
      SHIFT;
      if (argc == 0) {
        printer.println(F("Must give either 'status' or 'config' command"));
      } else if (strcmp(argv[0], "config") == 0) {
        SHIFT;
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
      } else if (strcmp(argv[0], "status") == 0) {
        printer.print(F("NtpTimeProvider::isSetup(): "));
        printer.println(mNtpTimeProvider.isSetup() ? F("true") : F("false"));
        printer.print(F("NTP Server: "));
        printer.println(mNtpTimeProvider.getServer());
        printer.print(F("WiFi IP address: "));
        printer.println(WiFi.localIP());
      } else if (strcmp(argv[0], "connect") == 0) {
        connect(printer);
      } else {
        printer.print(F("Unknown wifi command: "));
        printer.println(argv[0]);
      }
    }

    void connect(Print& printer) const {
      const StoredInfo& storedInfo = mController.getStoredInfo();
      const char* ssid = storedInfo.ssid;
      const char* password = storedInfo.password;
      mNtpTimeProvider.setup(ssid, password);
      if (mNtpTimeProvider.isSetup()) {
        printer.println(F("Connection succeeded."));
      } else {
        printer.println(F("Connection failed... run 'wifi connect' again"));
      }
    }

  private:
    Controller& mController;
    NtpTimeProvider& mNtpTimeProvider;
};

#endif

// Create a list of CommandHandlers.
ListCommand listCommand;
DateCommand dateCommand;
SyncCommand syncCommand;
TimezoneCommand timezoneCommand;
#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
WifiCommand wifiCommand(controller, ntpTimeProvider);
#endif
#if SYNC_TYPE == SYNC_TYPE_MANUAL
SyncStatusCommand syncStatusCommand(systemClockSyncLoop);
#endif

const CommandHandler* const COMMANDS[] = {
  &listCommand,
  &dateCommand,
  &syncCommand,
#if SYNC_TYPE == SYNC_TYPE_MANUAL
  &syncStatusCommand,
#endif
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
  // Wait for stability on some boards.
  // 1000ms needed for SERIAL_PORT_MONITOR.
  // 2000ms needed for Wire, I2C or SSD1306 (don't know which one).
  delay(2000);

#if defined(ARDUINO_AVR_LEONARDO)
  RXLED0; // LED off
  TXLED0; // LED off
#endif

  SERIAL_PORT_MONITOR.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!SERIAL_PORT_MONITOR); // Wait until SERIAL_PORT_MONITOR is ready - Leonardo/Micro
  SERIAL_PORT_MONITOR.println(F("setup(): begin"));

  SERIAL_PORT_MONITOR.print(F("sizeof(StoredInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(StoredInfo));

  Wire.begin();
  Wire.setClock(400000L);

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  dsTimeKeeper.setup();
#endif

  persistentStore.setup();
  systemClock.setup();
  controller.setup();

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
/*
  if (controller.isStoredInfoValid()) {
    const StoredInfo& storedInfo = controller.getStoredInfo();
    ntpTimeProvider.setup(storedInfo.ssid, storedInfo.password);
  }
*/
#endif

  // insert coroutines into the scheduler
#if SYNC_TYPE == SYNC_TYPE_COROUTINE
  systemClockSync.setupCoroutine(F("systemClockSync"));
#endif
  commandManager.setupCoroutine(F("commandManager"));
  CoroutineScheduler::setup();

  SERIAL_PORT_MONITOR.println(F("setup(): end"));
}

void loop() {
  systemClock.keepAlive();
#if SYNC_TYPE == SYNC_TYPE_MANUAL
  systemClockSyncLoop.loop();
#endif

  CoroutineScheduler::loop();
}
