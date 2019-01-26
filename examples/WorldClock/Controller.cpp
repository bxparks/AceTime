#include "Controller.h"

void Controller::setup() {
  // Create the timezones
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  mClockInfo0.zoneSpecifier = ManualZoneSpecifier(
      UtcOffset::forHour(-8), "PST", UtcOffset::forHour(1), "PDT");
  mClockInfo1.zoneSpecifier = ManualZoneSpecifier(
      UtcOffset::forHour(-5), "EST", UtcOffset::forHour(1), "EDT");
  mClockInfo2.zoneSpecifier = ManualZoneSpecifier(
      UtcOffset::forHour(0), "GMT", UtcOffset::forHour(1), "BST");
#else
  mClockInfo0.zoneSpecifier = AutoZoneSpecifier(&zonedb::kZoneLos_Angeles);
  mClockInfo1.zoneSpecifier = AutoZoneSpecifier(&zonedb::kZoneNew_York);
  mClockInfo2.zoneSpecifier = AutoZoneSpecifier(&zonedb::kZoneLondon);
#endif

  mClockInfo0.timeZone = TimeZone(&mClockInfo0.zoneSpecifier);
  mClockInfo1.timeZone = TimeZone(&mClockInfo1.zoneSpecifier);
  mClockInfo2.timeZone = TimeZone(&mClockInfo2.zoneSpecifier);

  // Name the 3 clocks using the airport codes.
  mClockInfo0.name = "SFO";
  mClockInfo1.name = "PHL";
  mClockInfo2.name = "LHR";

  // Restore from EEPROM to other settings
  StoredInfo storedInfo;
  bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
      &storedInfo, sizeof(StoredInfo));

  if (isValid) {
    mClockInfo0.hourMode = storedInfo.hourMode;
    mClockInfo0.blinkingColon = storedInfo.blinkingColon;

    mClockInfo1.hourMode = storedInfo.hourMode;
    mClockInfo1.blinkingColon = storedInfo.blinkingColon;

    mClockInfo2.hourMode = storedInfo.hourMode;
    mClockInfo2.blinkingColon = storedInfo.blinkingColon;

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
    mClockInfo0.timeZone.isDst(storedInfo.isDst0);
    mClockInfo1.timeZone.isDst(storedInfo.isDst1);
    mClockInfo2.timeZone.isDst(storedInfo.isDst2);
#endif
  } else {
    preserveInfo();
  }

  mMode = MODE_DATE_TIME;
}
