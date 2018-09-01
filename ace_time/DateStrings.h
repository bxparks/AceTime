#ifndef ACE_TIME_DATE_STRINGS_H
#define ACE_TIME_DATE_STRINGS_H

#include <stdint.h>
#include <string.h>

#if defined(__AVR__) || defined(__arm__)
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#else
  #error Unsupported platform
#endif

namespace ace_time {

/**
 * Class that translates a numeric month (1-12) or weekDay (1-7) into a human
 * readable string. Both long and short versions can be retrieved. The object
 * uses an internal char[] buffer to store the result strings, so this is not
 * thread-safe but Arduino boards are single-threaded currently so we don't have
 * to worry about this. Inspired by the DateStrings.cpp file in
 * https://github.com/PaulStoffregen/Time/blob/master/DateStrings.cpp.
 */
class DateStrings {
  public:
    static const uint8_t kBufferSize = 10;
    static const uint8_t kShortNameLength = 3;

    /** Return the long month name. 0=Error, 1=January, 12=December. */
    const char* monthLongString(uint8_t month) const {
      uint8_t index = (month < kNumMonthNames) ? month : 0;
      strcpy_P(mBuffer, kMonthNames[index]);
      return mBuffer;
    }

    /** Return the short month name. 0=Err, 1=Jan, 12=Dec. */
    const char* monthShortString(uint8_t month) const {
      uint8_t index = (month < kNumMonthNames) ? month : 0;
      strncpy_P(mBuffer, kMonthNames[index], kShortNameLength);
      mBuffer[kShortNameLength] = '\0';
      return mBuffer;
    }

    /** Return the short weekDay name. 0=Error, 1=Sunday, 7=Saturday. */
    const char* weekDayLongString(uint8_t weekDay) const {
      uint8_t index = (weekDay < kNumWeekDayNames) ? weekDay : 0;
      strcpy_P(mBuffer, kWeekDayNames[index]);
      return mBuffer;
    }

    /** Return the short weekDay name. 0=Err, 1=Sun, 7=Sat. */
    const char* weekDayShortString(uint8_t weekDay) const {
      uint8_t index = (weekDay < kNumWeekDayNames) ? weekDay : 0;
      strncpy_P(mBuffer, kWeekDayNames[index], kShortNameLength);
      mBuffer[kShortNameLength] = '\0';
      return mBuffer;
    }

  private:
    static const char * const kWeekDayNames[];
    static const char * const kMonthNames[];
    static const uint8_t kNumWeekDayNames;
    static const uint8_t kNumMonthNames;

    mutable char mBuffer[kBufferSize];
};

}

#endif
