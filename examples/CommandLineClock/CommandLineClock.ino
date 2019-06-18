/*
 * A simple command line (i.e. ascii clock using the Serial line. Useful for
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
 *    timezone [fixed offset | manual offset | basic | extended | dst (on | off)]
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
  SystemClockHeartbeatCoroutine systemClockHeartbeat(systemClock);
#else
  SystemClockSyncLoop systemClockSyncLoop(systemClock);
  SystemClockHeartbeatLoop systemClockHeartbeatLoop(systemClock);
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
        ZonedDateTime now = controller.getNow();
        now.printTo(printer);
        printer.println();
      } else {
        SHIFT;
        ZonedDateTime newDate = ZonedDateTime::forDateString(argv[0]);
        if (newDate.isError()) {
          printer.println(FF("Invalid date"));
          return;
        }
        controller.setNow(newDate.toEpochSeconds());
        printer.print(FF("Date set to: "));
        newDate.printTo(printer);
        printer.println();
      }
    }
};

/**
 * Timezone command.
 * Usage:
 *    timezone - print current timezone
 *    timezone fixed timeOffset - set timezone to fixed mode with given offset
 *    timezone manual timeOffset - set timezone to ManualZoneSpecifier with given offset
 *    timezone basic - set timezone to BasicZoneSpecifier
 *    timezone extended - set timezone to ExtendedZoneSpecifier
 *    timezone dst (on | off) - set ManualZoneSpecifier DST flag to on or off
 */
class TimezoneCommand: public CommandHandler {
  public:
    TimezoneCommand():
      CommandHandler("timezone",
        "[fixed offset | manual offset | basic | extended | dst (on | off)]") {}

    void run(Print& printer, int argc, const char** argv) const override {
      if (argc == 1) {
        const TimeZone& timeZone = controller.getTimeZone();
        timeZone.printTo(printer);
        printer.println();
        return;
      }

      SHIFT;
      if (strcmp(argv[0], "basic") == 0) {
        controller.setBasicTimeZone();
        printer.print(FF("Time zone using BasicZoneSpecifier: "));
        controller.getTimeZone().printTo(printer);
        printer.println();
        return;
      }

      if (strcmp(argv[0], "extended") == 0) {
        controller.setExtendedTimeZone();
        printer.print(FF("Time zone using ExtendedZoneSpecifier: "));
        controller.getTimeZone().printTo(printer);
        printer.println();
        return;
      }

      if (strcmp(argv[0], "fixed") == 0) {
        SHIFT;
        if (argc == 0) {
          printer.print(FF("'timezone fixed' requires 'offset'"));
          return;
        }
        TimeOffset offset = TimeOffset::forOffsetString(argv[0]);
        if (offset.isError()) {
          printer.println(FF("Invalid time zone offset"));
          return;
        }
        controller.setFixedTimeZone(offset);
        printer.print(FF("Time zone set to: "));
        controller.getTimeZone().printTo(printer);
        printer.println();
        return;
      }

      if (strcmp(argv[0], "manual") == 0) {
        SHIFT;
        if (argc == 0) {
          printer.print(FF("'timezone manual' requires 'offset'"));
          return;
        }
        TimeOffset offset = TimeOffset::forOffsetString(argv[0]);
        if (offset.isError()) {
          printer.println(FF("Invalid time zone offset"));
          return;
        }
        controller.setManualTimeZone(offset, false /*isDst*/);
        printer.print(FF("Time zone set to: "));
        controller.getTimeZone().printTo(printer);
        printer.println();
        return;
      }

      if (strcmp(argv[0], "dst") == 0) {
        SHIFT;
        if (argc == 0) {
          printer.print(FF("DST: "));
          printer.println(controller.isDst() ? FF("on") : FF("off"));
          return;
        }

        if (strcmp(argv[0], "on") == 0) {
          controller.setDst(true);
        } else if (strcmp(argv[0], "off") == 0) {
          controller.setDst(false);
        } else {
          printer.print(FF("'timezone dst' must be either 'on' or 'off'"));
        }
        return;
      }

      // If we get here, we don't recognize the subcommand.
      printer.print(FF("Unknown option ("));
      printer.print(argv[0]);
      printer.println(")");
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
      printer.print(FF("Seconds since last sync: "));
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
        printer.println(FF("Must give either 'status' or 'config' command"));
      } else if (strcmp(argv[0], "config") == 0) {
        SHIFT;
        if (argc == 0) {
          if (mController.isStoredInfoValid()) {
            // print ssid and password
            const StoredInfo& storedInfo = mController.getStoredInfo();
            printer.print(FF("ssid: "));
            printer.println(storedInfo.ssid);
            printer.print(FF("password: "));
            printer.println(storedInfo.password);
          } else {
            printer.println(
              FF("Invalid ssid and password in persistent storage"));
          }
        } else if (argc == 2) {
          const char* ssid = argv[0];
          const char* password = argv[1];
          mController.setWiFi(ssid, password);
          connect(printer);
        } else {
          printer.println(FF("Wifi config command requires 2 arguments"));
        }
      } else if (strcmp(argv[0], "status") == 0) {
        printer.print(FF("NtpTimeProvider::isSetup(): "));
        printer.println(mNtpTimeProvider.isSetup() ? FF("true") : FF("false"));
        printer.print(FF("NTP Server: "));
        printer.println(mNtpTimeProvider.getServer());
        printer.print(FF("WiFi IP address: "));
        printer.println(WiFi.localIP());
      } else if (strcmp(argv[0], "connect") == 0) {
        connect(printer);
      } else {
        printer.print(FF("Unknown wifi command: "));
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
  &timezoneCommand,
#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  &wifiCommand,
#endif
#if SYNC_TYPE == SYNC_TYPE_MANUAL
  &syncStatusCommand,
#endif
};
uint8_t const NUM_COMMANDS = sizeof(COMMANDS) / sizeof(CommandHandler*);

// Create an instance of the CommandManager.
uint8_t const BUF_SIZE = 64;
uint8_t const ARGV_SIZE = 5;
CommandManager<BUF_SIZE, ARGV_SIZE> commandManager(
    COMMANDS, NUM_COMMANDS, Serial, "> ");

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

  Serial.print(FF("sizeof(StoredInfo): "));
  Serial.println(sizeof(StoredInfo));

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
  systemClockSync.setupCoroutine(FF("systemClockSync"));
  systemClockHeartbeat.setupCoroutine(FF("systemClockHeartbeat"));
#endif
  commandManager.setupCoroutine(FF("commandManager"));
  CoroutineScheduler::setup();

  Serial.println(FF("setup(): end"));
}

void loop() {
  CoroutineScheduler::loop();
#if SYNC_TYPE == SYNC_TYPE_MANUAL
  systemClockSyncLoop.loop();
  systemClockHeartbeatLoop.loop();
#endif
}
