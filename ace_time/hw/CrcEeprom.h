#ifndef ACE_TIME_HW_CRC_EEPROM_H
#define ACE_TIME_HW_CRC_EEPROM_H

#include <EEPROM.h>
#include <FastCRC.h>

#if !defined(AVR) && !defined(ESP8266) && !defined(ESP32)
  #error Unsupported board type
#endif

namespace ace_time {
namespace hw {

/**
 * Thin wrapper around the EEPROM object (from the the built-in EEPROM library)
 * to read and write using CRC check.
 *
 * Depends on the FastCRC (https://github.com/FrankBoesing/FastCRC) library.
 *
 * Currently in the AceTime library because it's used to store backup time.
 * Might be moved to another library later.
 */
class CrcEeprom {
  public:

    /**
     * Call from global setup() function. Needed for ESP8266 and ESP32,
     * does nothing for AVR and others.
     */
#if defined(ESP8266) || defined(ESP32)
    void begin(uint16_t size) {
      EEPROM.begin(size);
    }
#else
    void begin(uint16_t /*size*/) {
    }
#endif

    /**
     * Write the data with its CRC8. Returns the number of bytes written.
     */
    uint16_t writeWithCrc(int address, const void* const data,
        const uint16_t dataSize) {
      uint16_t byteCount = dataSize;
      const uint8_t* d = (const uint8_t*) data;
      uint8_t crc = FastCRC8().smbus(d, dataSize);
      write(address++, crc);
      while (byteCount-- > 0) {
        write(address++, *d++);
      }
      bool success = commit();
      return (success) ? dataSize + 1 : 0;
    }

    /**
     * Read the data from EEPROM along with its CRC8. Return true if the CRC of
     * the data retrieved matches the CRC of the data when it was written.
     */
    bool readWithCrc(int address, void* const data,
        const uint16_t dataSize) const {
      uint16_t byteCount = dataSize;
      uint8_t* d = (uint8_t*) data;
      uint8_t crc = read(address++);
      while (byteCount-- > 0) {
        *d++ = read(address++);
      }
      uint8_t dataCrc = FastCRC8().smbus((const uint8_t*)data, dataSize);
      return crc == dataCrc;
    }

  private:
    void write(int address, uint8_t val) {
#if defined(ESP8266) || defined(ESP32)
      EEPROM.write(address++, val);
#else
      EEPROM.update(address++, val);
#endif
    }

    uint8_t read(int address) const {
      return EEPROM.read(address);
    }

    bool commit() {
#if defined(ESP8266) || defined(ESP32)
      return EEPROM.commit();
#else
      return true;
#endif
    }
};

}
}

#endif
