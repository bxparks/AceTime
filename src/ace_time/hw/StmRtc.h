/*
 * MIT License
 * Copyright (c) 2020 Brian T. Park, Anatoli Arkhipenko
 */

#ifndef ACE_TIME_HW_STM_RTC_H
#define ACE_TIME_HW_STM_RTC_H

#if ! defined(EPOXY_DUINO)
#if defined(ARDUINO_ARCH_STM32)

#include <stdint.h>
#include <STM32RTC.h>

namespace ace_time {
namespace hw {

class HardwareDateTime;

/**
 * A class that reads and writes HardwareDateTime from a STM RTC chip. This
 * class is designed to access just enough features of the RTC chip to
 * implement the ace_time::StmRtcClock class. It is not meant to provide access
 * to all the features of the RTC chip.
 *
 * Requires https://github.com/stm32duino/STM32RTC
 */
class StmRtc {
  public:
    /** Constructor. */
    explicit StmRtc();

    /** Read the time into the HardwareDateTime object. */
    void readDateTime(HardwareDateTime* dateTime) const;

    /** Set the STM with the HardwareDateTime values. */
    void setDateTime(const HardwareDateTime& dateTime) const;

    /** Return true if the RTC is available and the time is set. */
    bool isTimeSet() const;

  private:
    STM32RTC* mRtc;
};

} // hw
} // ace_time

#endif //  #if defined(ARDUINO_ARCH_STM32)
#endif //  #if ! defined(EPOXY_DUINO)
#endif //  #ifndef ACE_TIME_HW_STM_RTC_H
