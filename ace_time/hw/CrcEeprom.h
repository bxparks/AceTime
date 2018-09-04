#ifndef ACE_TIME_HW_CRC_EEPROM_H
#define ACE_TIME_HW_CRC_EEPROM_H

#include <EEPROM.h>
#include <FastCRC.h>

namespace ace_time {
namespace hw {

#if defined(AVR)

/**
 * Thin wrapper around the EEPROM object (from the the built-in EEPROM library)
 * to read and write using CRC check.
 *
 * Currently works only with AVR because the ESP8266 (and ESP32?) API is
 * slightly different than the AVR library.
 *
 * Depends on the FastCRC (https://github.com/FrankBoesing/FastCRC) library.
 *
 * Currently in the AceTime library because it's used to store backup time.
 * Might be moved to another library later.
 */
class CrcEeprom {
  public:
    /**
     * Write the data with its CRC8. Returns the number of bytes written.
     */
    uint16_t writeWithCrc(uint16_t address, const void* const data,
        const uint16_t dataSize) {
      uint16_t byteCount = dataSize;
      const uint8_t* d = (const uint8_t*) data;
      uint8_t crc = FastCRC8().smbus(d, dataSize);
      EEPROM.update(address++, crc);
      while (byteCount > 0) {
        EEPROM.update(address, *d);
        d++;
        address++;
        byteCount--;
      }
      return dataSize + 1;
    }

    /**
     * Read the data from EEPROM along with its CRC8. Return true if the CRC of
     * the data retrieved matches the CRC of the data when it was written.
     */
    bool readWithCrc(uint16_t address, void* const data,
        const uint16_t dataSize) const {
      uint16_t byteCount = dataSize;
      uint8_t* d = (uint8_t*) data;
      uint8_t crc = EEPROM[address++];
      while (byteCount > 0) {
        *d = EEPROM.read(address);
        d++;
        address++;
        byteCount--;
      }

      uint8_t dataCrc = FastCRC8().smbus((const uint8_t*)data, dataSize);
      return crc == dataCrc;
    }
};

}
}

#endif

#endif
