/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park, Anatoli Arkhipenko
 
 Requires https://github.com/stm32duino/STM32RTC
 */

#ifndef ACE_TIME_STMRTC_CLOCK_H
#define ACE_TIME_STMRTC_CLOCK_H

#if ! defined(UNIX_HOST_DUINO)
#if defined(ARDUINO_ARCH_STM32)

#include <stdint.h>
#include "../hw/StmRtc.h"
#include "../hw/HardwareDateTime.h"
#include "../LocalDateTime.h"
#include "Clock.h"

namespace ace_time {
namespace clock {

/**
 * An implementation of Clock that uses a STM32 RTC chip.
 */
class StmRtcClock: public Clock {
  public:
    explicit StmRtcClock() {}

    void setup(const sourceClock_t clockSource = LSI_CLOCK, const hourFormat_t hourFormat = HOUR_FORMAT_24) {
      mStmRtc.begin(clockSource, hourFormat);
    }

    acetime_t getNow() const override {
      hw::HardwareDateTime hardwareDateTime;
      mStmRtc.readDateTime(&hardwareDateTime);
      return toDateTime(hardwareDateTime).toEpochSeconds();
    }
    
    void setNow(acetime_t epochSeconds) override {
      if (epochSeconds == kInvalidSeconds) return;

//Serial.println("setNow: epochSeconds is valid");
      LocalDateTime now = LocalDateTime::forEpochSeconds(epochSeconds);
//Serial.print("now is valid: "); Serial.println(now.isError());
      mStmRtc.setDateTime(toHardwareDateTime(now));
    }

    bool isTimeSet() const { 
      return mStmRtc.isTimeSet(); 
    }
    
  private:
    /**
     * Convert the HardwareDateTime returned by the STM RTC chip to
     * the LocalDateTime object using UTC time zone.
     */
    static LocalDateTime toDateTime(const hw::HardwareDateTime& dt) {
      return LocalDateTime::forComponents(
          dt.year + LocalDate::kEpochYear, dt.month, dt.day,
          dt.hour, dt.minute, dt.second);
    }

    /**
     * Convert a LocalDateTime object to a HardwareDateTime object, ignoring
     * time zone. In practice, it will often be most convenient to store
     * as UTC time. Only 2 digits are supported by the year field.
     */
    static hw::HardwareDateTime toHardwareDateTime(const LocalDateTime& dt) {
//Serial.printf("%d-%d-%d %d:%d\n", dt.yearTiny(), dt.month(), dt.day(), dt.hour(), dt.minute());
      return hw::HardwareDateTime{(uint8_t) dt.yearTiny(), dt.month(),
          dt.day(), dt.hour(), dt.minute(), dt.second(), dt.dayOfWeek()};
    }

    hw::StmRtc mStmRtc;
};

}
}
#endif  //  #if defined(ARDUINO_ARCH_STM32)

#endif

#endif  //  ACE_TIME_STMRTC_CLOCK_H

