#ifndef COMMAND_LINE_CLOCK_PERSISTENT_STORE_H
#define COMMAND_LINE_CLOCK_PERSISTENT_STORE_H

#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "StoredInfo.h"

using namespace ace_time;

class PersistentStore {
  public:
    void setup() {
      // Needed for ESP32
      mCrcEeprom.begin(kEepromSize);
    }

    bool readStoredInfo(StoredInfo& storedInfo) const {
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      storedInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      storedInfo.ssid[StoredInfo::kPasswordMaxLength - 1] = '\0';
      return isValid;
    }

    void writeStoredInfo(const StoredInfo& storedInfo) const {
      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

  private:
    static const uint16_t kStoredInfoEepromAddress = 0;

    // Must be greater than (sizeof(StoredInfo) + 4).
    static const uint8_t kEepromSize = sizeof(StoredInfo) + 4;

    hw::CrcEeprom mCrcEeprom;
};

#endif
