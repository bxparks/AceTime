#include "flash.h"

// There are many different boards which identify themselves as
// ARDUINO_SAMD_ZERO. The original Arduino Zero is bit broken with regards to
// the definition of SERIAL_PORT_MONITOR, so warn the user about that.
#if defined(ARDUINO_SAMD_ZERO)
  #warning See USER_GUIDE.md about SERIAL_PORT_MONITOR if using an Arduino Zero (ignore if using a dev board from SparkFun or others)
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
