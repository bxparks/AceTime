/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

/*
 * Implement logger::print() and logger::println() that accept formatting
 * strings like printf(). I finally got tired of writing multiple lines of
 * SERIAL_PORT_MONITOR.print() for debugging.
 */

#ifndef ACE_TIME_COMMON_LOGGING_H
#define ACE_TIME_COMMON_LOGGING_H

#include <stdarg.h>

namespace ace_time {
namespace logging {

void vprintf(const char *fmt, va_list args);

/** A print() that works for Arduino using the built-in vsnprintf(). */
void print(const char* fmt, ...);

/**
 * Log the lower 16-bits of millis(), then print the log message using its fmt
 * and arguments. Automatically prints a newline.
 */
void println(const char *fmt, ... );

/** Print a newline. */
void println();

}
}

#endif
