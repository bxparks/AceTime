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
      return mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
    }

    void writeStoredInfo(const StoredInfo& storedInfo) {
      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

  private:
    static const uint16_t kStoredInfoEepromAddress = 0;

    // Must be greater than (sizeof(StoredInfo) + 4).
    static const uint8_t kEepromSize = 32;

    hw::CrcEeprom mCrcEeprom;
};

#endif
