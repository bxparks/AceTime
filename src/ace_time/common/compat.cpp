#include "compat.h"

// There are many different boards which identify themselves as
// ARDUINO_SAMD_ZERO. The original Arduino Zero using Native USB Port
// does not set SERIAL_PORT_MONITOR correctly, so warn the user.
#if defined(ARDUINO_SAMD_ZERO)
  #warning Arduino Zero may need SERIAL_PORT_MONITOR fixed (see USER_GUIDE.md)
#endif
