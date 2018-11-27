#include "Controller.h"

void Controller::setup() {
  // Create the timezones
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  mClockInfo0.timeZone = ManualTimeZone::forUtcOffset(
      UtcOffset::forHour(-8), true, "PST", "PDT");
  mClockInfo1.timeZone = ManualTimeZone::forUtcOffset(
      UtcOffset::forHour(-5), true, "EST", "EDT");
  mClockInfo2.timeZone = ManualTimeZone::forUtcOffset(
      UtcOffset::forHour(0), true, "GMT", "BST");
#else
  mClockInfo0.timeZone = AutoTimeZone::forZone(&zonedb::kZoneLos_Angeles);
  mClockInfo1.timeZone = AutoTimeZone::forZone(&zonedb::kZoneNew_York);
  mClockInfo2.timeZone = AutoTimeZone::forZone(&zonedb::kZoneLondon);
#endif

  // Create the 3 time zones.
  mClockInfo0.name[ClockInfo::kNameSize - 1] = '\0';
  strncpy(mClockInfo0.name, "SFO", ClockInfo::kNameSize);

  strncpy(mClockInfo1.name, "PHL", ClockInfo::kNameSize);
  mClockInfo1.name[ClockInfo::kNameSize - 1] = '\0';

  strncpy(mClockInfo2.name, "LHR", ClockInfo::kNameSize);
  mClockInfo2.name[ClockInfo::kNameSize - 1] = '\0';

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
    mClockInfo0.timeZone.isDst(storedInfo.isDst);
    mClockInfo1.timeZone.isDst(storedInfo.isDst);
    mClockInfo2.timeZone.isDst(storedInfo.isDst);
#endif
  } else {
    preserveInfo();
  }
}
