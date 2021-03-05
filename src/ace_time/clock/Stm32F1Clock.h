/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 *
 * A clock that uses the LSE_CLOCK (Low Speed External) on the STM32F1xx which
 * keeps updating the 32-bit RTC counter as long as the battery is attached to
 * VBat. We use STM32F1_RTC to write directly to the RTC_CNTH and RTC_CNTL
 * registers.
 */

#ifndef ACE_TIME_STM32_F1_CLOCK_H
#define ACE_TIME_STM32_F1_CLOCK_H

#if defined(ARDUINO_ARCH_STM32)
#if defined(STM32F1xx)

#include <stdint.h>
#include "../hw/Stm32F1Rtc.h"
#include "Clock.h"

namespace ace_time {
namespace clock {

/**
 * An implementation of Clock that uses the LSE_CLOCK (Low Speed External)
 * clock on the STM32F1 RTC chip.
 */
class Stm32F1Clock: public Clock {
  public:
    explicit Stm32F1Clock() {}

    void setup() {
      mStm32F1Rtc.begin();
    }

    acetime_t getNow() const override {
      return mStm32F1Rtc.getTime();
    }

    void setNow(acetime_t epochSeconds) override {
      if (epochSeconds == kInvalidSeconds) return;
      mStm32F1Rtc.setTime(epochSeconds);
    }

  private:
    mutable hw::Stm32F1Rtc mStm32F1Rtc;
};

} // hw
} // ace_time

#endif // #if defined(STM32F1xx)
#endif // #if defined(ARDUINO_ARCH_STM32)

#endif // #ifndef ACE_TIME_STM32F1_CLOCK_H
