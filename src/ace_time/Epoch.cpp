#include "Epoch.h"

namespace ace_time {

int16_t Epoch::sCurrentEpochYear = 2050;

// Number of days from 2000-01-01 to 2050-01-01: 50*365 + 13 leap days = 18263.
int32_t Epoch::sDaysToCurrentEpochFromConverterEpoch = 18263;

}
