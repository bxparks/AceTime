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
 * and ESP32 compilers, link-time-optimization does not seem to work well. If
 * these functions are defined in a .cpp file, they are included in the binary,
 * even if they are not reference at all by anything. This causes the binary to
 * be about 700 (AVR) to 1000 (ESP32) bytes larger in flash memory. Being
 * inlined here means that <Arduino.h> must be included here, which can cause
 * some problems in files that try to clobber macros defined in <Ardhino.h>.
 */

#ifndef ACE_TIME_COMMON_LOGGING_H
#define ACE_TIME_COMMON_LOGGING_H

#include <stdio.h> // vsnprintf(), vprintf()
#include <stdarg.h> // va_list, va_start(), va_end()
#include <Arduino.h> // SERIAL_PORT_MONITOR
#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial
#endif

namespace ace_time {
namespace logging {

static const int BUF_SIZE = 192;

inline void vprintf(const char *fmt, va_list args) {
  char buf[BUF_SIZE];
  vsnprintf(buf, BUF_SIZE, fmt, args);
  SERIAL_PORT_MONITOR.print(buf);
}

/**
 * A print() that works for Arduino using the built-in vsnprintf().
 * Append a '\\n' at the end of the string to print a newline.
 */
inline void printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

}
}

#endif
