#include "Controller.h"

void Controller::setup() {
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
    mClockInfo0.timeZone.setDst(storedInfo.isDst0);
    mClockInfo1.timeZone.setDst(storedInfo.isDst1);
    mClockInfo2.timeZone.setDst(storedInfo.isDst2);
#endif
  } else {
    preserveInfo();
  }

  mMode = MODE_DATE_TIME;
}
