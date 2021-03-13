/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
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
 * An implementation of `Clock` that is specialized for the LSE_CLOCK (Low Speed
 * External clock) on the STM32F1 RTC chip. Normally, the LSE_CLOCK requires an
 * additional external 32.768 kHz crystal, but the popular "Blue Pill" dev board
 * already includes this extenal crystal on pins C14 and C15. **Warning**: For
 * the highest clock accuracy, those pins should not be attached anything else,
 * not even the male header pins. The header pins will add too much stray
 * capacitance to the oscillator circuit, and cause the clock to run too slow. I
 * have seen the clock run as much as 10% too slow with the male header pins
 * attached. If you hold a finger to those pins, it adds so much capacitance
 * that the LSE_CLOCK will appear to just stop.
 *
 * There are 3 possible RTC clocks on the STM32F1 (HSI_CLOCK, LSI_CLOCK, and
 * LSE_CLOCK). But the LSE_CLOCK is special because it keeps updating the RTC on
 * the STM32F1 through a reset or power loss, as long as a battery is attached
 * to VBat. The battery could be a 3V CR2032 coin battery, or 2AA rechargeable
 * or non-rechargeable battery, or it could be a super capacitor.
 *
 * This class uses the `Stm32F1Rtc` helper class to write directly to the RTC
 * registers on the STM32F1, bypassing the generic `STM32RTC` library
 * (https://github.com/stm32duino/STM32RTC) which would normally be used for
 * other STM32 microcontrollers. The generic `STM32RTC` library
 * has a bug on the STM32F1 where it preserves only the time fields in the RTC
 * registers, saving the date fields on SRAM which is lost upon reset (See
 * https://github.com/stm32duino/STM32RTC/issues/29 and
 * https://github.com/stm32duino/STM32RTC/issues/32). The problem is caused in
 * the low-level HAL (hardware abstraction layer) of the STM32F1 chip, because
 * unlike other STM32 processors which stores the time and date fields as
 * separate fields, the RTC on the STM32F1 is just a simple 32-bit counter
 * (split across 2 registeres, RTC_CNTH and RTC_CNTL) that increments once a
 * second.
 *
 * It turns out that for the purposes of AceTime and the `SystemClock`, a 32-bit
 * counter is sufficient to support all of its functionality. In particular, the
 * 32-bit counter is sufficient to allow AceTime to retain both date and time
 * fields through a power reset. So the `Stm32F1Rtc` class is a narrowly
 * targeted HAL whose only purpose is to read from and write to the 32-bit RTC
 * counter on the STM32F1 chip. It bypasses the entire generic RTC and HAL
 * layers provided by the STM32duino framework.
 *
 * The `Stm32F1Rtc` class uses one additional register, the Backup DR1 register,
 * which holds a single status bit to indicate whether or not the underlying RTC
 * counter has been initialized to a valid time. The selection of the `DR1`
 * register, instead of any of the other 9-10 backup registers, is currently
 * hardcoded in `Stm32F1Rtc.h`. If that causes a conflict with something else,
 * let me know, because this is fixable. We can make that a configurable
 * parameter in the Stm32F1Rtc::begin() method.
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

} // clock
} // ace_time

#endif // #if defined(STM32F1xx)
#endif // #if defined(ARDUINO_ARCH_STM32)

#endif // #ifndef ACE_TIME_STM32F1_CLOCK_H
