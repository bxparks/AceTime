#ifndef COMMAND_LINE_CLOCK_PERSISTENT_STORE_H
#define COMMAND_LINE_CLOCK_PERSISTENT_STORE_H

#if defined(ARDUINO)

#include <AceTime.h>
#if ENABLE_EEPROM
  #include <ace_time/hw/CrcEeprom.h>
#endif
#include "config.h"
#include "StoredInfo.h"

using namespace ace_time;


class PersistentStore {
  public:
    void setup() {
    #if ENABLE_EEPROM
      // Needed for ESP32
      mCrcEeprom.begin(kEepromSize);
    #endif
    }

  #if ENABLE_EEPROM
    bool readStoredInfo(StoredInfo& storedInfo) const {
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
    #if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
      storedInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      storedInfo.password[StoredInfo::kPasswordMaxLength - 1] = '\0';
    #endif
      return isValid;
    }
  #else
    bool readStoredInfo(StoredInfo& /*storedInfo*/) const {
      return false;
    }
  #endif

  #if ENABLE_EEPROM
    uint16_t writeStoredInfo(const StoredInfo& storedInfo) const {
      return mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }
  #else
    uint16_t writeStoredInfo(const StoredInfo& /*storedInfo*/) const {
      return 0;
    }
  #endif

  private:
  #if ENABLE_EEPROM
    static const uint16_t kStoredInfoEepromAddress = 0;

    // Must be greater than or equal to (sizeof(StoredInfo) + 4).
    static const uint8_t kEepromSize = sizeof(StoredInfo) + 4;

    hw::CrcEeprom mCrcEeprom;
  #endif
};

#else

#include <stdio.h>
#include <zlib.h>
#include <AceTime.h>
#include "config.h"
#include "StoredInfo.h"

class PersistentStore {
  public:
    void setup(const char* file) {
      mFile = file;
    }

    bool readStoredInfo(StoredInfo& storedInfo) const {
      FILE* file = fopen(mFile, "rb");
      uint32_t expectedCrc;
      fread((void*) &storedInfo, sizeof(storedInfo), 1, file);
      fread((void*) &expectedCrc, sizeof(expectedCrc), 1, file);
      fclose(file);
      uint32_t actualCrc = crc32(0, (const Bytef*) &storedInfo,
          sizeof(storedInfo));
      bool isValid = (expectedCrc == actualCrc);
#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
      storedInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      storedInfo.password[StoredInfo::kPasswordMaxLength - 1] = '\0';
#endif
      return isValid;
    }

    uint16_t writeStoredInfo(const StoredInfo& storedInfo) const {
      uint32_t actualCrc = crc32(0, (const Bytef*) &storedInfo,
          sizeof(storedInfo));
      FILE* file = fopen(mFile, "wb");
      fwrite((const void*) &storedInfo, sizeof(storedInfo), 1, file);
      fwrite((const void*) &actualCrc, sizeof(actualCrc), 1, file);
      fclose(file);
      return sizeof(storedInfo) + sizeof(actualCrc);
    }

  private:
    const char* mFile;
};
#endif

#endif
