/*
 * A quick implementation of printf() and logf() for Arduino. I finally got
 * very tired of writing multiple lines of Serial.print() for debugging.
 */

#ifndef ACE_TIME_COMMON_PRINTF
#define ACE_TIME_COMMON_PRINTF

namespace ace_time {
namespace common {

#include <stdarg.h>

inline void vprintf(const char *fmt, va_list args) {
	char buf[192];
	vsnprintf(buf, 192, fmt, args);
	Serial.print(buf);
}

/** A printf() that works for Arduino using the built-in vsnprintf(). */
inline void printf(const char* fmt, ...) {
  va_list args;
	va_start(args, fmt);
  common::vprintf(fmt, args);
  va_end(args);
}

/**
 * Similar to printf() but prints the last 2 bytes of millis() before printing
 * out the fmt and arguments.
 */
inline void logf(const char *fmt, ... ) {
  uint16_t now = millis();
  Serial.print(now);
  Serial.print(": ");

	va_list args;
	va_start(args, fmt);
  common::vprintf(fmt, args);
  va_end(args);
}

}
}

#endif
