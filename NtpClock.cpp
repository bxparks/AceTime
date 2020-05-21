/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

// We include <WString.h> here for the sole reason to avoid a compiler warning
// about '"FPSTR" redefined' on an ESP32. That's because the ESP32 had an
// incorrect definition of FPSTR() before v1.0.3 (see
// https://github.com/espressif/arduino-esp32/issues/1371). so compat.h
// clobbers it with the correct definition. If we don't include <WString.h>
// here, "compath.h" gets included first, then something else eventually brings
// in <WString.h> which tries to redefine it, generating the compiler warning.
// At some point, if everyone migrates to v1.0.3 and above, I can remove that
// FPSTR() def in "compath.h".
#include <WString.h>
#include "../common/compat.h"
#include "NtpClock.h"

//#if defined(ESP8266) || defined(ESP32)

namespace ace_time {
namespace clock {

const char NtpClock::kNtpServerName[] = "us.pool.ntp.org";

}
}

//#endif
