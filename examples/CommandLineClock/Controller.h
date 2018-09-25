#ifndef COMMAND_LINE_CLOCK_CONTROLLER_H
#define COMMAND_LINE_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "config.h"
#include "StoredInfo.h"

using namespace ace_time;

// Set to 1 to set the TimeProvider::pollNow() method.
class Controller {
  public:
    static const uint16_t kStoredInfoEepromAddress = 0;
    static const int8_t kDefaultTzCode = -28; // Pacific Daylight Time, -07:00

    Controller(hw::CrcEeprom& crcEeprom, TimeKeeper& systemTimeKeeper):
        mCrcEeprom(crcEeprom),
        mSystemTimeKeeper(systemTimeKeeper),
        mTimeZone() {}

    void setup() {
      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      if (isValid) {
        mTimeZone = TimeZone(storedInfo.tzCode);
      } else {
        mTimeZone = TimeZone(kDefaultTzCode);
      }
    }

    void preserveInfo() {
      StoredInfo storedInfo;
      storedInfo.tzCode = mTimeZone.tzCode();
      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

    void setTimeZone(const TimeZone& timeZone) {
      mTimeZone = timeZone;
      preserveInfo();
    }

    void setDateTime(const DateTime& newDateTime) {
      uint32_t seconds = newDateTime.toSecondsSinceEpoch();
      mSystemTimeKeeper.setNow(seconds);
    }

    DateTime now() const {
      return DateTime(mSystemTimeKeeper.getNow(), mTimeZone);
    }

    TimeZone timeZone() const {
      return mTimeZone;
    }

  private:
    hw::CrcEeprom& mCrcEeprom;
    TimeKeeper& mSystemTimeKeeper;
    TimeZone mTimeZone;
};

#endif
