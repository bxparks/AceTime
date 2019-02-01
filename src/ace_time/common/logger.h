/*
 * A quick implementation of printf() and loggerf() for Arduino. I finally got
 * very tired of writing multiple lines of Serial.print() for debugging.
 */

#ifndef ACE_TIME_COMMON_LOGGER_H
#define ACE_TIME_COMMON_LOGGER_H

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
 * Log the lower 16-bits of millis(), then print the log message using its fmt
 * and arguments. Automatically prints a newline.
 */
inline void logger(const char *fmt, ... ) {
  uint16_t now = millis();
  Serial.print(now);
  Serial.print(": ");

	va_list args;
	va_start(args, fmt);
  common::vprintf(fmt, args);
  va_end(args);
  Serial.println();
}

}
}

#endif
