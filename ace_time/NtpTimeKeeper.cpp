#include "NtpTimeKeeper.h"

#if defined(ESP8266)

namespace ace_time {

const char NtpTimeKeeper::kNtpServerName[] = "us.pool.ntp.org";

//const char kNtpServerName[] = "time.nist.gov";
//const char kNtpServerName[] = "time-a.timefreq.bldrdoc.gov";
//const char kNtpServerName[] = "time-b.timefreq.bldrdoc.gov";
//const char kNtpServerName[] = "time-c.timefreq.bldrdoc.gov";

}

#endif
