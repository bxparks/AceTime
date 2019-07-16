#include "flash.h"

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
