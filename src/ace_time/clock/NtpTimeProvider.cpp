/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include "../common/flash.h"
#include "NtpTimeProvider.h"

#if defined(ESP8266) || defined(ESP32)

namespace ace_time {
namespace clock {

const char NtpTimeProvider::kNtpServerName[] = "us.pool.ntp.org";

}
}

#endif
