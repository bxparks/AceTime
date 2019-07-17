/*
 * A simple digital clock using:
 *   * a DS3231 RTC chip and/or NTP server
 *   * an SSD1306 OLED display
 *   * 2 push buttons
 *
 * Tested using:
 *   * Arduino Nano
 *   * Arduino Pro Mini
 *   * Arduino Pro Micro
 *   * ESP8266
 *
 * Dependencies:
 *   * [AceTime](https://github.com/bxparks/AceTime)
 *   * [AceRoutine](https://github.com/bxparks/AceRoutine)
 *   * [AceButton](https://github.com/bxparks/AceButton)
 *   * [FastCRC](https://github.com/FrankBoesing/FastCRC)
 *   * [SSD1306Ascii](https://github.com/greiman/SSD1306Ascii)
 *
 * If ENABLE_SERIAL is set to 1, it prints diagnostics like this:
 *  * sizeof(ClockInfo): 32
 *  * sizeof(StoredInfo): 4
 *  * sizeof(RenderingInfo): 28
 */

#include <Wire.h>
#include <SSD1306AsciiWire.h>
#include <AceButton.h>
#include <AceRoutine.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "config.h"
#include "Presenter.h"
#include "Controller.h"

using namespace ace_button;
using namespace ace_routine;
using namespace ace_time;
using namespace ace_time::clock;

//------------------------------------------------------------------
// Configure CrcEeprom.
//------------------------------------------------------------------

hw::CrcEeprom crcEeprom;

//------------------------------------------------------------------
// Configure various TimeKeepers and TimeProviders.
//------------------------------------------------------------------

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  DS3231TimeKeeper dsTimeKeeper;
  SystemClock systemClock(&dsTimeKeeper, &dsTimeKeeper);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  NtpTimeProvider ntpTimeProvider;
  SystemClock systemClock(&ntpTimeProvider, nullptr);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_BOTH
  DS3231TimeKeeper dsTimeKeeper;
  NtpTimeProvider ntpTimeProvider;
  SystemClock systemClock(&ntpTimeProvider, &dsTimeKeeper);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NONE
  SystemClock systemClock(nullptr /*sync*/, nullptr /*backup*/);
#else
  #error Unknown time keeper option
#endif

SystemClockSyncCoroutine systemClockSync(systemClock);

//------------------------------------------------------------------
// Configure OLED display using SSD1306Ascii.
//------------------------------------------------------------------

// OLED address: 0X3C+SA0 - 0x3C or 0x3D
#define OLED_I2C_ADDRESS 0x3C

SSD1306AsciiWire oled;

void setupOled() {
  oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS);
  oled.displayRemap(OLED_REMAP);
  oled.clear();
}

//------------------------------------------------------------------
// Create controller/presenter pair.
//------------------------------------------------------------------

Presenter presenter(oled);
Controller controller(systemClock, crcEeprom, presenter);

//------------------------------------------------------------------
// Render the Clock periodically.
//------------------------------------------------------------------

// The RTC has a resolution of only 1s, so we need to poll it fast enough to
// make it appear that the display is tracking it correctly. The benchmarking
// code says that controller.display() runs as fast as or faster than 1ms, so
// we can set this to 100ms without worrying about too much overhead.
COROUTINE(displayClock) {
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
    COROUTINE_DELAY(10); // check button 100/sec
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
  Serial.print(F("sizeof(ClockInfo): "));
  Serial.println(sizeof(ClockInfo));
  Serial.print(F("sizeof(StoredInfo): "));
  Serial.println(sizeof(StoredInfo));
  Serial.print(F("sizeof(RenderingInfo): "));
  Serial.println(sizeof(RenderingInfo));
#endif

  Wire.begin();
  Wire.setClock(400000L);
  crcEeprom.begin(EEPROM_SIZE);

  setupAceButton();
  setupOled();

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  dsTimeKeeper.setup();
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  ntpTimeProvider.setup(AUNITER_SSID, AUNITER_PASSWORD);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_BOTH
  dsTimeKeeper.setup();
  ntpTimeProvider.setup(AUNITER_SSID, AUNITER_PASSWORD);
#endif
  systemClock.setup();
  controller.setup();

  systemClockSync.setupCoroutine(F("systemClockSync"));
  CoroutineScheduler::setup();

#if ENABLE_SERIAL == 1
  Serial.println(F("setup(): end"));
#endif
}

void loop() {
  CoroutineScheduler::loop();
}
