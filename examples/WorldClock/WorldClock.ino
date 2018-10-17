/*
 * A digital clock with multiple displays for keeping track of multiple time
 * zones.
 *
 *   * a DS3231 RTC chip (I2C)
 *   * 2 or more SSD1306 OLED displays (SPI)
 *   * 2 push buttons
 * 
 * Supported boards are:
 *   * Arduino Pro Micro
 */

#include <Wire.h>
#include <AceButton.h>
#include <AceRoutine.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "SSD1306AsciiSpi.h"
#include "config.h"
#include "FullOledClock.h"

using namespace ace_button;
using namespace ace_routine;
using namespace ace_time;

//------------------------------------------------------------------
// Configure CrcEeprom.
//------------------------------------------------------------------

hw::CrcEeprom crcEeprom;

//------------------------------------------------------------------
// Configure various TimeKeepers and TimeProviders.
//------------------------------------------------------------------

DS3231TimeKeeper dsTimeKeeper;
SystemTimeKeeper systemTimeKeeper(&dsTimeKeeper, &dsTimeKeeper);

SystemTimeSyncCoroutine systemTimeSync(systemTimeKeeper);
SystemTimeHeartbeatCoroutine systemTimeHeartbeat(systemTimeKeeper);

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
// Create an appropriate controller/presenter pair depending on
// the DISPLAY_TYPE.
//------------------------------------------------------------------

FullOledPresenter presenter(oled0);
FullOledClock clock(systemTimeKeeper, crcEeprom, presenter);

// The RTC has a resolution of only 1s, so we need to poll it fast enough to
// make it appear that the display is tracking it correctly. The benchmarking
// code says that clock.display() runs as fast as or faster than 1ms for all
// DISPLAY_TYPEs. So we can set this to 100ms without worrying about too
// much overhead.
COROUTINE(displayClock) {
  COROUTINE_LOOP() {
    clock.update();
    COROUTINE_DELAY(100);
  }
}

//------------------------------------------------------------------
// Configure AceButton.
//------------------------------------------------------------------

AdjustableButtonConfig modeButtonConfig;
AceButton modeButton(&modeButtonConfig);

AdjustableButtonConfig changeButtonConfig;
AceButton changeButton(&changeButtonConfig);

void handleModeButton(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventReleased:
      clock.modeButtonPress();
      break;
    case AceButton::kEventLongPressed:
      clock.modeButtonLongPress();
      break;
  }
}

void handleChangeButton(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventPressed:
      clock.changeButtonPress();
      break;
    case AceButton::kEventReleased:
      clock.changeButtonRelease();
      break;
    case AceButton::kEventRepeatPressed:
      clock.changeButtonRepeatPress();
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
  setupOled();

  dsTimeKeeper.setup();
  systemTimeKeeper.setup();
  clock.setup();

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
