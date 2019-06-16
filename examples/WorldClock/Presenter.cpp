#include <AceButton.h>
#include <AceRoutine.h>
#include "Presenter.h"

void Presenter::displayAbout() const {
  mOled.setFont(SystemFont5x7);

  // For smallest flash memory size, use F() macros for these longer
  // strings, but no F() for shorter version strings.
  mOled.print(F("WorldClock: "));
  mOled.println(CLOCK_VERSION_STRING);
  mOled.print(F("TZ: "));
  mOled.println(zonedb::kTzDatabaseVersion);
}
