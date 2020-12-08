/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park, Anatoli Arkhipenko
 */

#ifndef ACE_TIME_HW_STMRTC_H
#define ACE_TIME_HW_STMRTC_H

#if ! defined(UNIX_HOST_DUINO)
#if defined(ARDUINO_ARCH_STM32)

#include <stdint.h>
#include <STM32RTC.h>

namespace ace_time {
namespace hw {

class HardwareDateTime;

/**
 * A class that reads and writes HardwareDateTime and HardwareTemperature from a
 * STM RTC chip. This class is designed to access just enough features of the
 * RTC chip to implement the ace_time::StmRtcClock class. It is not
 * meant to provide access to all the features of the RTC chip.
 
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

    bool isTimeSet() const;
  private:
    STM32RTC* mRtc;
};

}
}
#endif  //  #if defined(ARDUINO_ARCH_STM32)

#endif  //  #if ! defined(UNIX_HOST_DUINO)

#endif  //  #ifndef ACE_TIME_HW_STMRTC_H
