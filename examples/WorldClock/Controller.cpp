#include "Controller.h"

void Controller::setup() {
  // Create the timezones
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  mClockInfo0.zoneAgent = ManualZoneAgent(
      UtcOffset::forHour(-8), "PST", UtcOffset::forHour(1), "PDT");
  mClockInfo1.zoneAgent = ManualZoneAgent(
      UtcOffset::forHour(-5), "EST", UtcOffset::forHour(1), "EDT");
  mClockInfo2.zoneAgent = ManualZoneAgent(
      UtcOffset::forHour(0), "GMT", UtcOffset::forHour(1), "BST");
#else
  mClockInfo0.zoneAgent = AutoZoneAgent(&zonedb::kZoneLos_Angeles);
  mClockInfo1.zoneAgent = AutoZoneAgent(&zonedb::kZoneNew_York);
  mClockInfo2.zoneAgent = AutoZoneAgent(&zonedb::kZoneLondon);
#endif

  mClockInfo0.timeZone = TimeZone(&mClockInfo0.zoneAgent);
  mClockInfo1.timeZone = TimeZone(&mClockInfo1.zoneAgent);
  mClockInfo2.timeZone = TimeZone(&mClockInfo2.zoneAgent);

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
