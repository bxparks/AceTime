/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include "../common/compat.h"
#include "NtpClock.h"

#if defined(ESP8266) || defined(ESP32)

namespace ace_time {
namespace clock {

const char NtpClock::kNtpServerName[] = "us.pool.ntp.org";

}
}

#endif
