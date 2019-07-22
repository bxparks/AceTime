#include <Arduino.h> // millis()
#include "SystemClock.h"

namespace ace_time {
namespace clock {

unsigned long SystemClock::millis() const { return ::millis(); }

}
}
