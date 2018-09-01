#include "DateStrings.h"

namespace ace_time {

static const char kError[] PROGMEM = "Error";
static const char kJanuary[] PROGMEM = "January";
static const char kFebruary[] PROGMEM = "February";
static const char kMarch[] PROGMEM = "March";
static const char kApril[] PROGMEM = "April";
static const char kMay[] PROGMEM = "May";
static const char kJune[] PROGMEM = "June";
static const char kJuly[] PROGMEM = "July";
static const char kAugust[] PROGMEM = "August";
static const char kSeptember[] PROGMEM = "September";
static const char kOctober[] PROGMEM = "October";
static const char kNovember[] PROGMEM = "November";
static const char kDecember[] PROGMEM = "December";

const char* const DateStrings::kMonthNames[] = {
    kError, kJanuary, kFebruary, kMarch, kApril, kMay, kJune,
    kJuly, kAugust, kSeptember, kOctober, kNovember, kDecember
};

const uint8_t DateStrings::kNumMonthNames =
    sizeof(kMonthNames) / sizeof(const char *);

static const char kSunday[] PROGMEM = "Sunday";
static const char kMonday[] PROGMEM = "Monday";
static const char kTuesday[] PROGMEM = "Tuesday";
static const char kWednesday[] PROGMEM = "Wednesday";
static const char kThursday[] PROGMEM = "Thursday";
static const char kFriday[] PROGMEM = "Friday";
static const char kSaturday[] PROGMEM = "Saturday";

const char* const DateStrings::kWeekDayNames[] = {
  kError, kSunday, kMonday, kTuesday, kWednesday, kThursday, kFriday, kSaturday
};

const uint8_t DateStrings::kNumWeekDayNames =
    sizeof(kWeekDayNames) / sizeof(const char *);

}
