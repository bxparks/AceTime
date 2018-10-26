#ifndef ACE_TIME_LOCAL_DATE_H
#define ACE_TIME_LOCAL_DATE_H

#include <stdint.h>

namespace ace_time {

class LocalDate {
  public:
    /** Base year of epoch. */
    static const uint16_t kEpochYear = 2000;

    /** Monday ISO 8601 number. */
    static const uint8_t kMonday = 1;

    /** Tuesday ISO 8601 number. */
    static const uint8_t kTuesday = 2;

    /** Wednesday ISO 8601 number. */
    static const uint8_t kWednesday = 3;

    /** Thursday ISO 8601 number. */
    static const uint8_t kThursday = 4;

    /** Friday ISO 8601 number. */
    static const uint8_t kFriday = 5;

    /** Saturday ISO 8601 number. */
    static const uint8_t kSaturday = 6;

    /** Sunday ISO 8601 number. */
    static const uint8_t kSunday = 7;

    static LocalDate forComponents(uint8_t year, uint8_t month, uint8_t day) {
      return LocalDate(year, month, day);
    }

    static LocalDate forDateString(const char* /*dateString*/) {
      return LocalDate();
    }

    static LocalDate forEpochDays(uint32_t /*epochDays*/) {
      return LocalDate();
    }

    explicit LocalDate() {}

    uint32_t toEpochDays() const {
      return 0;
    }

    /**
     * Calculate the day of week given the (year, month, day).
     * Idea borrowed from https://github.com/evq/utz.
     */
    uint8_t dayOfWeek() const {
      // The "year" starts in March to shift leap year calculation to end.
      uint16_t y = kEpochYear + mYear - (mMonth < 3);
      uint16_t d = y + y/4 - y/100 + y/400 + sDayOfWeek[mMonth-1] + mDay;
      // 2000-1-1 was a Saturday=6
      return (d + 1) % 7 + 1;
    }

  private:
    /**
     * Day of week table for each month, with 0=Jan to 11=Dec. The table
     * offsets actualy start with March to make the leap year calculation
     * easier.
     */
    static const uint8_t sDayOfWeek[12];

    explicit LocalDate(uint8_t year, uint8_t month, uint8_t day):
        mYear(year),
        mMonth(month),
        mDay(day),
        mDayOfWeek(0) {}

    uint8_t mYear; // [00, 99], year - 2000
    uint8_t mMonth; // [1, 12]
    uint8_t mDay; // [1, 31]
    mutable uint8_t mDayOfWeek; // (1=Monday, 7=Sunday)
};

}

#endif
