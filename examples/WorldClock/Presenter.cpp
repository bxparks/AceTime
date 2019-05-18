#include <AceButton.h>
#include <AceRoutine.h>
#include "Presenter.h"

void Presenter::displayAbout() const {
  mOled.setFont(SystemFont5x7);

  // For smallest flash memory size, use F() macros for these longer
  // strings, but no F() for shorter version strings.
  mOled.print(F("WorldClock: "));
  mOled.println(CLOCK_VERSION_STRING);
  mOled.print(F("Tzdata: "));
  mOled.println(zonedb::kTzDatabaseVersion);
  mOled.print(F("AceTime: "));
  mOled.println(ACE_TIME_VERSION_STRING);
  mOled.print(F("AceButton: "));
  mOled.println(ACE_BUTTON_VERSION_STRING);
  mOled.print(F("AceRoutine: "));
  mOled.println(ACE_ROUTINE_VERSION_STRING);
}
