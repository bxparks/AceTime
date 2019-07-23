#include "flash.h"

// Print out a warning about SERIAL_PORT_MONITOR, just once
#if defined(ARDUINO_SAMD_ZERO)
  #warning Setting SERIAL_PORT_MONITOR to SerialUSB
#endif

#if defined(ESP8266) || defined(ESP32)

const char* strchr_P(const char* s, int c) {
  char cc = c;
  while (true) {
    char d = pgm_read_byte(s);
    if (cc == d) return s;
    if (!d) return nullptr;
    s++;
  }
}

const char* strrchr_P(const char* s, int c) {
  char cc = c;
  const char* found = nullptr;
  while (true) {
    char d = pgm_read_byte(s);
    if (cc == d) found = s;
    if (!d) break;
    s++;
  }
  return found;
}

#endif

int acetime_strcmp_PP(const char* a, const char* b) {
  if (a == b) { return 0; }
  if (a == nullptr) { return -1; }
  if (b == nullptr) { return 1; }

  while (true) {
    uint8_t ca = pgm_read_byte(a);
    uint8_t cb = pgm_read_byte(b);
    if (ca != cb) return (int) ca - (int) cb;
    if (ca == '\0') return 0;
    a++;
    b++;
  }
}
