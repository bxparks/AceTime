#ifndef ACE_TIME_COMMON_DATE_STRINGS_H
#define ACE_TIME_COMMON_DATE_STRINGS_H

#include <stdint.h>
#include <string.h>
#include "flash.h"

namespace ace_time {
namespace common {

/**
 * Class that translates a numeric month (1-12) or dayOfWeek (1-7) into a human
 * readable string. Both long and short versions can be retrieved. The object
 * uses an internal char[] buffer to store the result strings, so the strings
 * must be used before DateStrings object is destroyed. This also means that
 * the object is not thread-safe but Arduino boards are single-threaded
 * currently so we don't have to worry about this.
 *
 * Inspired by the DateStrings.cpp file in
 * https://github.com/PaulStoffregen/Time/blob/master/DateStrings.cpp.
 */
class DateStrings {
  public:
    /**
     * Length of the longest month or week name, including the '\0' terminator.
     */
    static const uint8_t kBufferSize = 10;

    /** Number of prefix characters to use to create a short name. */
    static const uint8_t kShortNameLength = 3;

    /** Return the long month name. 0=Error, 1=January, 12=December. */
    const char* monthLongString(uint8_t month) {
      uint8_t index = (month < kNumMonthNames) ? month : 0;
      strcpy_P(mBuffer, kMonthNames[index]);
      return mBuffer;
    }

    /** Return the short month name. 0=Err, 1=Jan, 12=Dec. */
    const char* monthShortString(uint8_t month) {
      uint8_t index = (month < kNumMonthNames) ? month : 0;
      strncpy_P(mBuffer, kMonthNames[index], kShortNameLength);
      mBuffer[kShortNameLength] = '\0';
      return mBuffer;
    }

    /** Return the short dayOfWeek name. 0=Error, 1=Monday, 7=Sunday. */
    const char* dayOfWeekLongString(uint8_t dayOfWeek) {
      uint8_t index = (dayOfWeek < kNumDayOfWeekNames) ? dayOfWeek : 0;
      strcpy_P(mBuffer, kDayOfWeekNames[index]);
      return mBuffer;
    }

    /** Return the short dayOfWeek name. 0=Err, 1=Mon, 7=Sun. */
    const char* dayOfWeekShortString(uint8_t dayOfWeek) {
      uint8_t index = (dayOfWeek < kNumDayOfWeekNames) ? dayOfWeek : 0;
      strncpy_P(mBuffer, kDayOfWeekNames[index], kShortNameLength);
      mBuffer[kShortNameLength] = '\0';
      return mBuffer;
    }

  private:
    static const char * const kDayOfWeekNames[];
    static const char * const kMonthNames[];
    static const uint8_t kNumDayOfWeekNames;
    static const uint8_t kNumMonthNames;

    char mBuffer[kBufferSize];
};

}
}

#endif
