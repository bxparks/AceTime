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
#include "../hw/stmrtc.h"
#include "../hw/HardwareDateTime.h"
#include "../LocalDateTime.h"
#include "Clock.h"

namespace ace_time {
namespace clock {

/**
 * An implementation of Clock that uses a STM32 RTC chip.
 */
class StmRTCClock: public Clock {
  public:
    explicit StmRTCClock() {}

    void setup() {}

    acetime_t getNow() const override {
      hw::HardwareDateTime hardwareDateTime;
      mSTMRTC.readDateTime(&hardwareDateTime);
      return toDateTime(hardwareDateTime).toEpochSeconds();
    }
    
    void setNow(acetime_t epochSeconds) override {
      if (epochSeconds == kInvalidSeconds) return;

      LocalDateTime now = LocalDateTime::forEpochSeconds(epochSeconds);
      mSTMRTC.setDateTime(toHardwareDateTime(now));
    }

    bool isTimeSet() const { 
      return mSTMRTC.isTimeSet(); 
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
     * time zone. In practice, it will often be most convenient to store the
     * DS3231 as UTC time. Only 2 digits are supported by the year field in the
     * DS3231 so the year is assumed to be between 2000 and 2099.
     */
    static hw::HardwareDateTime toHardwareDateTime(const LocalDateTime& dt) {
      return hw::HardwareDateTime{(uint8_t) dt.yearTiny(), dt.month(),
          dt.day(), dt.hour(), dt.minute(), dt.second(), dt.dayOfWeek()};
    }

    const hw::STMRTC mSTMRTC;
};

}
}
#endif  //  #if defined(ARDUINO_ARCH_STM32)

#endif

#endif  //  ACE_TIME_STMRTC_CLOCK_H
