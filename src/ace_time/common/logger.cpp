#include <Arduino.h>
#include "flash.h"
#include "logger.h"

namespace ace_time {
namespace logging {

void vprintf(const char *fmt, va_list args) {
	char buf[192];
	vsnprintf(buf, 192, fmt, args);
	SERIAL_PORT_MONITOR.print(buf);
}

/** A print() that works for Arduino using the built-in vsnprintf(). */
void print(const char* fmt, ...) {
  va_list args;
	va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

/**
 * Log the lower 16-bits of millis(), then print the log message using its fmt
 * and arguments. Automatically prints a newline.
 */
void println(const char *fmt, ... ) {
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
