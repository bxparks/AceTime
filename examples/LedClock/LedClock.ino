/*
A simple digital clock using:
  * a DS3231 RTC chip
  * a 7-segment LED display, OR an SSD1306 OLED display
  * 2 push buttons

Supported boards are:
  * Arduino Nano
  * Arduino Pro Mini
  * Arduino Leonardo (Pro Micro clone)
*/

#include <Wire.h>
#include <AceSegment.h>
#include <AceButton.h>
#include <AceRoutine.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include <SSD1306AsciiWire.h>
#include "config.h"
#include "LedDisplay.h"
#include "Controller.h"

using namespace ace_segment;
using namespace ace_button;
using namespace ace_routine;
using namespace ace_time;
using namespace ace_time::provider;

//------------------------------------------------------------------
// Configure CrcEeprom.
//------------------------------------------------------------------

hw::CrcEeprom crcEeprom;

//------------------------------------------------------------------
// Configure various TimeKeepers and TimeProviders.
//------------------------------------------------------------------

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  DS3231TimeKeeper dsTimeKeeper;
  SystemTimeKeeper systemTimeKeeper(&dsTimeKeeper, &dsTimeKeeper);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  NtpTimeProvider ntpTimeProvider;
  SystemTimeKeeper systemTimeKeeper(&ntpTimeProvider, nullptr);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_BOTH
  DS3231TimeKeeper dsTimeKeeper;
  NtpTimeProvider ntpTimeProvider;
  SystemTimeKeeper systemTimeKeeper(&ntpTimeProvider, &dsTimeKeeper);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NONE
  SystemTimeKeeper systemTimeKeeper(nullptr /*sync*/, nullptr /*backup*/);
#else
  #error Unknown time keeper option
#endif

SystemTimeSyncCoroutine systemTimeSync(systemTimeKeeper);
SystemTimeHeartbeatCoroutine systemTimeHeartbeat(systemTimeKeeper);

//------------------------------------------------------------------
// Configure LED display using AceSegment.
//------------------------------------------------------------------

// Use polling or interrupt for AceSegment
#define USE_INTERRUPT 0

LedDisplay ledDisplay;

#if USE_INTERRUPT == 1
  // interrupt handler for timer 2
  ISR(TIMER2_COMPA_vect) {
    ledDisplay.renderField();
  }
#else
  COROUTINE(renderLed) {
    COROUTINE_LOOP() {
      ledDisplay.renderFieldWhenReady();
      COROUTINE_YIELD();
    }
  }
#endif

void setupLed() {
#if USE_INTERRUPT == 1
  // set up Timer 2
  uint8_t timerCompareValue =
      (long) F_CPU / 1024 / ledDisplay->getFieldsPerSecond() - 1;
  #if ENABLE_SERIAL == 1
    Serial.print(F("Timer 2, Compare A: "));
    Serial.println(timerCompareValue);
  #endif

  noInterrupts();
  TCNT2  = 0;	// Initialize counter value to 0
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= bit(WGM21); // CTC
  TCCR2B |= bit(CS22) | bit(CS21) | bit(CS20); // prescale 1024
  TIMSK2 |= bit(OCIE2A); // interrupt on Compare A Match
  OCR2A =  timerCompareValue;
  interrupts();
#endif
}

//------------------------------------------------------------------
// Create an appropriate controller/presenter pair.
//------------------------------------------------------------------

Presenter presenter(ledDisplay);
Controller controller(systemTimeKeeper, crcEeprom, presenter);

//------------------------------------------------------------------
// Render the Clock periodically.
//------------------------------------------------------------------

// The RTC has a resolution of only 1s, so we need to poll it fast enough to
// make it appear that the display is tracking it correctly. The benchmarking
// code says that controller.display() runs as fast as or faster than 1ms for
// all DISPLAY_TYPEs. So we can set this to 100ms without worrying about too
// much overhead.
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
#endif

  Wire.begin();
  Wire.setClock(400000L);
  crcEeprom.begin(EEPROM_SIZE);

  setupAceButton();

  setupLed();

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_DS3231
  dsTimeKeeper.setup();
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  ntpTimeProvider.setup(AUNITER_SSID, AUNITER_PASSWORD);
#elif TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_BOTH
  dsTimeKeeper.setup();
  ntpTimeProvider.setup(AUNITER_SSID, AUNITER_PASSWORD);
#endif
  systemTimeKeeper.setup();
  controller.setup();

  systemTimeSync.setupCoroutine(F("systemTimeSync"));
  systemTimeHeartbeat.setupCoroutine(F("systemTimeHeartbeat"));
  CoroutineScheduler::setup();

#if ENABLE_SERIAL == 1
  Serial.println(F("setup(): end"));
#endif
}

void loop() {
  CoroutineScheduler::loop();
}
