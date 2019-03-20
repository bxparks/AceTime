/*
 * Implement logger::print() and logger::println() that accept formatting
 * strings like printf(). I finally got tired of writing multiple lines of
 * Serial.print() for debugging.
 */

#ifndef ACE_TIME_COMMON_LOGGING_H
#define ACE_TIME_COMMON_LOGGING_H

namespace ace_time {
namespace logging {

#include <stdarg.h>

inline void vprintf(const char *fmt, va_list args) {
	char buf[192];
	vsnprintf(buf, 192, fmt, args);
	Serial.print(buf);
}

/** A print() that works for Arduino using the built-in vsnprintf(). */
inline void print(const char* fmt, ...) {
  va_list args;
	va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

/**
 * Log the lower 16-bits of millis(), then print the log message using its fmt
 * and arguments. Automatically prints a newline.
 */
inline void println(const char *fmt, ... ) {
	va_list args;
	va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  Serial.println();
}

/** Print a newline. */
inline void println() {
  Serial.println();
}

}
}

#endif
