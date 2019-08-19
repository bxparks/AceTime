/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_HW_CRC_EEPROM_H
#define ACE_TIME_HW_CRC_EEPROM_H

#if ! defined(UNIX_HOST_DUINO)

// EEPROM is supported only on certain Arduino boards. In particular, many
// (most?) Arduino Zero compatible boards cannot support EEPROM even on Flash
// emulation because the version of the SAMD21 chip on the board doesn't
// support RWW (read-while-write).
#if !defined(AVR) && !defined(ESP8266) && !defined(ESP32) && \
    !defined(TEENSYDUINO)
  #error Unsupported architecture
#endif

#include <EEPROM.h>
#include <FastCRC.h>

namespace ace_time {
namespace hw {

/**
 * Thin wrapper around the EEPROM object (from the the built-in EEPROM library)
 * to read and write a given block of data along with its CRC check. The CRC is
 * written *after* the data block, instead of at the beginning of the data
 * block to reduce flash write wear of the bytes corresonding to the CRC. Over
 * time, it is expected that the size of the data block will change, as fields
 * are added or deleted. Therefore, the location of the CRC bytes will also
 * change, which helps wear leveling. If the CRC bytes were at the beginning,
 * those CRC byes would experience the highest level of writes, even when the
 * actua data block size changes.
 *
 * Depends on the FastCRC (https://github.com/FrankBoesing/FastCRC) library.
 *
 * Currently located in the AceTime library because it's used to store backup
 * time. Might be moved to another library later.
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
     * Write the data with its CRC. Returns the number of bytes written.
     */
    uint16_t writeWithCrc(int address, const void* const data,
        const uint16_t dataSize) const {
      uint16_t byteCount = dataSize;
      const uint8_t* d = (const uint8_t*) data;

      // write data blcok
      while (byteCount-- > 0) {
        write(address++, *d++);
      }

      // write CRC at the end of the data block
      uint32_t crc = FastCRC32().crc32((const uint8_t*) data, dataSize);
      uint8_t buf[4];
      memcpy(buf, &crc, 4);
      write(address++, buf[0]);
      write(address++, buf[1]);
      write(address++, buf[2]);
      write(address++, buf[3]);

      bool success = commit();
      return (success) ? dataSize + sizeof(crc) : 0;
    }

    /**
     * Read the data from EEPROM along with its CRC. Return true if the CRC of
     * the data retrieved matches the CRC of the data when it was written.
     */
    bool readWithCrc(int address, void* const data,
        const uint16_t dataSize) const {
      uint16_t byteCount = dataSize;
      uint8_t* d = (uint8_t*) data;

      // read data block
      while (byteCount-- > 0) {
        *d++ = read(address++);
      }

      // read CRC at the end of the data block
      uint8_t buf[4];
      buf[0] = read(address++);
      buf[1] = read(address++);
      buf[2] = read(address++);
      buf[3] = read(address++);
      uint32_t crc;
      memcpy(&crc, buf, 4);

      uint32_t dataCrc = FastCRC32().crc32((const uint8_t*) data, dataSize);
      return crc == dataCrc;
    }

  private:
    void write(int address, uint8_t val) const {
#if defined(ESP8266) || defined(ESP32)
      EEPROM.write(address, val);
#else
      EEPROM.update(address, val);
#endif
    }

    uint8_t read(int address) const {
      return EEPROM.read(address);
    }

    bool commit() const {
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

#endif
