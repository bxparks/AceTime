/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

/*
 * Implement logging::printf() that accept formatting strings like printf(). I
 * finally got tired of writing multiple lines of SERIAL_PORT_MONITOR.print()
 * for debugging.
 *
 * NOTE: These *must* be implemented as inline function to allow the compiler
 * to remove unused functions from the binary. For some reason, on AVR, ESP8266
 * and ESP32 compilers, link-time-optimization does not seem to work well.
 * These functions are defined in a .cpp file, they are included in the binary,
 * even if they are not reference at all by anything. This causes
 * the binary to be about 700 (AVR) to 1000 (ESP32) bytes larger in flash
 * memory. Being inlined here means that <Arduino.h> must be included here,
 * which can cause some problems in files that try to clobber macros defined in
 * <Ardhino.h>.
 */

#ifndef ACE_TIME_COMMON_LOGGING_H
#define ACE_TIME_COMMON_LOGGING_H

#include <stdarg.h>
#include <Arduino.h>

namespace ace_time {
namespace logging {

inline void vprintf(const char *fmt, va_list args) {
	char buf[192];
	vsnprintf(buf, 192, fmt, args);
	SERIAL_PORT_MONITOR.print(buf);
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
  SERIAL_PORT_MONITOR.println();
}

/** Print a newline. */
inline void println() {
  SERIAL_PORT_MONITOR.println();
}

}
}

#endif
