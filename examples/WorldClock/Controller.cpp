#include "Controller.h"

void Controller::setup() {
  // Create the timezones
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  mClockInfo0.timeZone = ManualTimeZone::forUtcOffset(
      UtcOffset::forHour(-8), true /*isDst*/, "PST", "PDT");
  mClockInfo1.timeZone = ManualTimeZone::forUtcOffset(
      UtcOffset::forHour(-5), true /*isDst*/, "EST", "EDT");
  mClockInfo2.timeZone = ManualTimeZone::forUtcOffset(
      UtcOffset::forHour(0), true /*isDst*/, "GMT", "BST");
#else
  mClockInfo0.timeZone = AutoTimeZone::forZone(&zonedb::kZoneLos_Angeles);
  mClockInfo1.timeZone = AutoTimeZone::forZone(&zonedb::kZoneNew_York);
  mClockInfo2.timeZone = AutoTimeZone::forZone(&zonedb::kZoneLondon);
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
