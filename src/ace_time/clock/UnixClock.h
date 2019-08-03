/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_UNIX_CLOCK_H
#define ACE_TIME_UNIX_CLOCK_H

#if defined(UNIX_HOST_DUINO)

#include <time.h> // time()
#include "Clock.h"
#include "../LocalDate.h"

namespace ace_time {
namespace clock {

/**
 * An implementation of Clock that works on Unix using UnixHostDuino.
 */
class UnixClock: public Clock {
  public:
    explicit UnixClock() {}

    void setup() {}

    acetime_t getNow() const override {
      return time(nullptr) - LocalDate::kSecondsSinceUnixEpoch;
    }
};

}
}

#endif

#endif
