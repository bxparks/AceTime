#include "Controller.h"

void Controller::setup() {
  // Create the 3 time zones.
  strncpy(mClockInfo0.name, "SFO", ClockInfo::kNameSize);
  mClockInfo0.name[ClockInfo::kNameSize - 1] = '\0';
  mClockInfo0.timeZone = TimeZone::forZoneOffset(
      ZoneOffset::forHour(-8), true, "PST", "PDT");

  strncpy(mClockInfo1.name, "PHL", ClockInfo::kNameSize);
  mClockInfo1.name[ClockInfo::kNameSize - 1] = '\0';
  mClockInfo1.timeZone = TimeZone::forZoneOffset(
      ZoneOffset::forHour(-5), true, "EST", "EDT");

  strncpy(mClockInfo2.name, "LHR", ClockInfo::kNameSize);
  mClockInfo2.name[ClockInfo::kNameSize - 1] = '\0';
  mClockInfo2.timeZone = TimeZone::forZoneOffset(
      ZoneOffset::forHour(0), true, "GMT", "BST");

  // Restore from EEPROM to other settings
  StoredInfo storedInfo;
  bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
      &storedInfo, sizeof(StoredInfo));

  if (isValid) {
    mClockInfo0.timeZone.setBaseDst(storedInfo.isDst);
    mClockInfo0.hourMode = storedInfo.hourMode;
    mClockInfo0.blinkingColon = storedInfo.blinkingColon;

    mClockInfo1.timeZone.setBaseDst(storedInfo.isDst);
    mClockInfo1.hourMode = storedInfo.hourMode;
    mClockInfo1.blinkingColon = storedInfo.blinkingColon;

    mClockInfo2.timeZone.setBaseDst(storedInfo.isDst);
    mClockInfo2.hourMode = storedInfo.hourMode;
    mClockInfo2.blinkingColon = storedInfo.blinkingColon;
  } else {
    preserveInfo();
  }
}
