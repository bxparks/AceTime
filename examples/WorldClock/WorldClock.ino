/*
 * A digital clock with 3 OLED displays to show 3 different time zones. The
 * hardware consists of:
 *
 *   * 1 x DS3231 RTC chip (I2C)
 *   * 3 x SSD1306 OLED displays using SPI interface (not I2C)
 *   * 2 x push buttons
 *
 * Tested on Arduino Pro Micro, but should work for Arduino Nano, ESP8266 and
 * ESP32.
 *
 * Dependencies:
 *
 *  * [AceTime](https://github.com/bxparks/AceTime)
 *  * [AceRoutine](https://github.com/bxparks/AceRoutine)
 *  * [AceButton](https://github.com/bxparks/AceButton)
 *  * [FastCRC](https://github.com/FrankBoesing/FastCRC)
 *  * [SSD1306Ascii](https://github.com/greiman/SSD1306Ascii)
 */

#include <Wire.h>
#include <AceButton.h>
#include <AceRoutine.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include <SSD1306AsciiSpi.h>
#include "config.h"
#include "ClockInfo.h"
#include "Controller.h"

using namespace ace_button;
using namespace ace_routine;
using namespace ace_time;

//------------------------------------------------------------------
// Configure CrcEeprom.
//------------------------------------------------------------------

// Needed by ESP32 chips. Has no effect on other chips.
// Should be bigger than (sizeof(crc32) + sizeof(StoredInfo)).
static const uint16_t EEPROM_SIZE = sizeof(StoredInfo) + sizeof(acetime_t);

hw::CrcEeprom crcEeprom;

//------------------------------------------------------------------
// Configure various TimeKeepers and TimeProviders.
//------------------------------------------------------------------

DS3231TimeKeeper dsTimeKeeper;
SystemClock systemClock(&dsTimeKeeper, &dsTimeKeeper);
SystemClockSyncCoroutine systemClockSync(systemClock);

//------------------------------------------------------------------
// Configure OLED display using SSD1306Ascii.
//------------------------------------------------------------------

SSD1306AsciiSpi oled0;
SSD1306AsciiSpi oled1;
SSD1306AsciiSpi oled2;

void setupOled() {
  pinMode(OLED_CS0_PIN, OUTPUT);
  digitalWrite(OLED_CS0_PIN, HIGH);

  pinMode(OLED_CS1_PIN, OUTPUT);
  digitalWrite(OLED_CS1_PIN, HIGH);

  pinMode(OLED_CS2_PIN, OUTPUT);
  digitalWrite(OLED_CS2_PIN, HIGH);

  oled0.begin(&Adafruit128x64, OLED_CS0_PIN, OLED_DC_PIN, OLED_RST_PIN);
  oled1.begin(&Adafruit128x64, OLED_CS1_PIN, OLED_DC_PIN);
  oled2.begin(&Adafruit128x64, OLED_CS2_PIN, OLED_DC_PIN);

  digitalWrite(OLED_CS0_PIN, HIGH);
  digitalWrite(OLED_CS1_PIN, HIGH);
  digitalWrite(OLED_CS2_PIN, HIGH);
}

//------------------------------------------------------------------
// Create controller with 3 presenters for the 3 OLED displays.
//------------------------------------------------------------------

Presenter presenter0(oled0);
Presenter presenter1(oled1);
Presenter presenter2(oled2);
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
TimeZone tz0 = TimeZone::forTimeOffset(TimeOffset::forHour(-8));
TimeZone tz1 = TimeZone::forTimeOffset(TimeOffset::forHour(-5));
TimeZone tz2 = TimeZone::forTimeOffset(TimeOffset::forHour(0));
#elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
BasicZoneProcessor zoneProcessor0;
BasicZoneProcessor zoneProcessor1;
BasicZoneProcessor zoneProcessor2;
TimeZone tz0 = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
    &zoneProcessor0);
TimeZone tz1 = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_New_York,
    &zoneProcessor1);
TimeZone tz2 = TimeZone::forZoneInfo(&zonedb::kZoneEurope_London,
    &zoneProcessor2);
#elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
ExtendedZoneProcessor zoneProcessor0;
ExtendedZoneProcessor zoneProcessor1;
ExtendedZoneProcessor zoneProcessor2;
TimeZone tz0 = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
    &zoneProcessor0);
TimeZone tz1 = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_New_York,
    &zoneProcessor1);
TimeZone tz2 = TimeZone::forZoneInfo(&zonedbx::kZoneEurope_London,
    &zoneProcessor2);
#else
  #error Unknown TIME_ZONE_TYPE
#endif
Controller controller(systemClock, crcEeprom,
    presenter0, presenter1, presenter2,
    tz0, tz1, tz2, "SFO", "PHL", "LHR");

// The RTC has a resolution of only 1s, so we need to poll it fast enough to
// make it appear that the display is tracking it correctly. The benchmarking
// code says that controller.update() runs faster than 1ms so we can set this
// to 100ms without worrying about too much overhead.
COROUTINE(updateController) {
  COROUTINE_LOOP() {
    controller.update();
    COROUTINE_DELAY(100);
  }
}

//------------------------------------------------------------------
// Configure AceButton.
//------------------------------------------------------------------

ButtonConfig modeButtonConfig;
AceButton modeButton(&modeButtonConfig);

ButtonConfig changeButtonConfig;
AceButton changeButton(&changeButtonConfig);

void handleModeButton(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventReleased:
      controller.modeButtonPress();
      break;
    case AceButton::kEventLongPressed:
      controller.modeButtonLongPress();
      break;
  }
}

void handleChangeButton(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventPressed:
      controller.changeButtonPress();
      break;
    case AceButton::kEventReleased:
      controller.changeButtonRelease();
      break;
    case AceButton::kEventRepeatPressed:
      controller.changeButtonRepeatPress();
      break;
  }
}

void setupAceButton() {
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);

  modeButton.init(MODE_BUTTON_PIN);
  changeButton.init(CHANGE_BUTTON_PIN);

  modeButtonConfig.setEventHandler(handleModeButton);
  modeButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  modeButtonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  changeButtonConfig.setEventHandler(handleChangeButton);
  changeButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  changeButtonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
  changeButtonConfig.setRepeatPressInterval(150);
}

COROUTINE(checkButton) {
  COROUTINE_LOOP() {
    modeButton.check();
    changeButton.check();
    COROUTINE_DELAY(5); // check buttons 200/sec
  }
}

//------------------------------------------------------------------
// Main setup and loop
//------------------------------------------------------------------

void setup() {
  // Wait for stability on some boards.
  // 1000ms needed for Serial.
  // 1500ms needed for Wire, I2C or SSD1306 (don't know which one).
  delay(2000);

  // Turn off the RX and TX LEDs on Leonardos
#if defined(ARDUINO_AVR_LEONARDO)
  RXLED0; // LED off
  TXLED0; // LED off
#endif

#if ENABLE_SERIAL == 1
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));
#endif

  Wire.begin();
  Wire.setClock(400000L);
  crcEeprom.begin(EEPROM_SIZE);

  setupAceButton();
  setupOled();

  dsTimeKeeper.setup();
  systemClock.setup();

  controller.setup();

  systemClockSync.setupCoroutine("s");
  CoroutineScheduler::setup();

#if ENABLE_SERIAL == 1
  Serial.println(F("setup(): end"));
#endif
}

void loop() {
  systemClock.keepAlive();

  // Using the CoroutineScheduler is conceptually cleaner, but consumes 159
  // bytes of extra flash memory. So run the coroutines manually instead of
  // call CoroutineScheduler::loop();
  updateController.runCoroutine();
  checkButton.runCoroutine();
  systemClockSync.runCoroutine();
}
