#include <AceButton.h>
#include <AceRoutine.h>
#include "Presenter.h"

void Presenter::displayAbout() const {
  mOled.set1X();

  // For smallest flash memory size, use F() macros for these longer
  // strings, but no F() for shorter version strings.
  mOled.print(F("AT: "));
  mOled.println(F(ACE_TIME_VERSION_STRING));
  mOled.print(F("TZ: "));
  mOled.println(zonedb::kTzDatabaseVersion);
}
