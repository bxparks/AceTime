#include "compat.h"

// There are many different boards which identify themselves as
// ARDUINO_SAMD_ZERO. The original Arduino Zero using Native USB Port
// does not set SERIAL_PORT_MONITOR correctly, so warn the user.
#if defined(ARDUINO_SAMD_ZERO)
  #warning Arduino Zero may need SERIAL_PORT_MONITOR fixed (see USER_GUIDE.md)
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
