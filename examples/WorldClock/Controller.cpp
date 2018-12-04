#include "Controller.h"

void Controller::setup() {
  // Create the timezones
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  mClockInfo0.timeZone = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false /*isDst*/, "PST", "PDT");
  mClockInfo1.timeZone = TimeZone::forUtcOffset(
      UtcOffset::forHour(-5), false /*isDst*/, "EST", "EDT");
  mClockInfo2.timeZone = TimeZone::forUtcOffset(
      UtcOffset::forHour(0), false /*isDst*/, "GMT", "BST");
#else
  mClockInfo0.zoneAgent = ZoneAgent(&zonedb::kZoneLos_Angeles);
  mClockInfo0.timeZone = TimeZone::forZone(&mClockInfo0.zoneAgent);

  mClockInfo1.zoneAgent = ZoneAgent(&zonedb::kZoneNew_York);
  mClockInfo1.timeZone = TimeZone::forZone(&mClockInfo1.zoneAgent);

  mClockInfo2.zoneAgent = ZoneAgent(&zonedb::kZoneLondon);
  mClockInfo2.timeZone = TimeZone::forZone(&mClockInfo2.zoneAgent);
#endif

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
