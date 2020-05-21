#ifndef OLED_CLOCK_PERSISTENT_STORE_H
#define OLED_CLOCK_PERSISTENT_STORE_H

#include <AceTime.h>
#if ! defined(ARDUINO_ARCH_SAMD)
  #include <ace_time/hw/CrcEeprom.h>
#endif
#include "config.h"
#include "StoredInfo.h"
#include "scGlobals.h"
#include "Arduino_DebugUtils.h"

using namespace ace_time;

class PersistentStore {
  public:
    void setup() {
    #if ! defined(ARDUINO_ARCH_SAMD)
      // Needed for ESP32
      mCrcEeprom.begin(kEepromSize);
    #endif
    }

  #if defined(ARDUINO_ARCH_SAMD)
    bool readStoredInfo(StoredInfo& storedInfo) const {
      if (boolstore.info_OK){
        Debug.print(DBG_INFO, "readStoredInfo SAMD in Persistent Store");
          readEvent(storedInfo);  //callback 
          return true;
      } else {
      return false;
      }
    }
  #else
    bool readStoredInfo(StoredInfo& storedInfo) const {
      Debug.print(DBG_INFO, "readStoredInfo notSAMD in Persistent Store");
      return mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
    }
  #endif

  #if defined(ARDUINO_ARCH_SAMD)
    uint16_t writeStoredInfo(StoredInfo& storedInfo) const {
      boolstore.info_OK = true;
      writeEvent(storedInfo);
      return 4;
    }
  #else
    uint16_t writeStoredInfo(StoredInfo& storedInfo) const {
      return mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }
  #endif
  
     using SomeEvent = void (*)(StoredInfo& storedInfo); //type aliasing
    //C++ version of: typedef void (*InputEvent)(const char*)

    void RegisterReadCallback(SomeEvent InEvent)
    {
      readEvent = InEvent;
    }
   void RegisterWriteCallback(SomeEvent OutEvent)
    {
      writeEvent = OutEvent;
    }
  private:
  #if ! defined(ARDUINO_ARCH_SAMD)
    static const uint16_t kStoredInfoEepromAddress = 0;

    // Must be >= (sizeof(StoredInfo) + 4).
    static const uint8_t kEepromSize = sizeof(StoredInfo) + 4;

    hw::CrcEeprom mCrcEeprom;
  #endif
      SomeEvent readEvent;
      SomeEvent writeEvent;
};

#endif
