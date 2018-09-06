#include <stdint.h>
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;

#if defined(AVR)
const uint32_t COUNT = 10000;
#elif defined(ESP8266)
const uint32_t COUNT = 50000;
#else
const uint32_t COUNT = 200000;
#endif

const uint32_t TO_NANO = ( 1000000 / COUNT);

const char TOP[] = 
  "----------------------------+---------+";
const char HEADER[] = 
  "Method                      |  micros |";
const char DIVIDER[] = 
  "----------------------------|---------|";
const char BOTTOM[] = 
  "----------------------------+---------+";
const char EMPTY_LOOP_LABEL[] =
  "Empty loop                  | ";
const char CONSTRUCTOR1_LABEL[] =
  "DateTime(y,m,d,h,m,s)       | ";
const char CONSTRUCTOR2_LABEL[] =
  "DateTime(seconds)           | ";
const char DAYS_SINCE_EPOCH_LABEL[] =
  "toDaysSinceEpochMillis()    | ";
const char SECOND_SINCE_EPOCH_LABEL[] =
  "toSecondsSinceEpochMillis() | ";
const char ENDING[] = " |";

// The compiler is extremelly good about removing code that does nothing. This
// variable is used to ensure user-visible side-effects, preventing the compiler
// optimization.
uint32_t guard;

void disableOptimization(const DateTime& dt) {
  guard ^= ((uint32_t) dt.year() << 24)
         ^ ((uint32_t) dt.month() << 16)
         ^ ((uint32_t) dt.day() << 8)
         ^ ((uint32_t) dt.hour() << 16)
         ^ ((uint32_t) dt.minute() << 8)
         ^ ((uint32_t) dt.second())
         ^ dt.timeZone().tzCode();
}

void printPad3(uint16_t val, char padChar) {
  if (val < 100) Serial.print(padChar);
  if (val < 10) Serial.print(padChar);
  Serial.print(val);
}

void printNanosAsMicros(unsigned long nanos) {
  uint16_t whole = nanos / 1000;
  uint16_t frac = nanos % 1000;
  printPad3(whole, ' ');
  Serial.print('.');
  printPad3(frac, '0');
}

void runBenchmark() {
  Serial.println(TOP);
  Serial.println(HEADER);
  Serial.println(DIVIDER);

  // Empty loop
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long tickMillis = millis();
    guard ^= tickMillis;
    digitalWrite(LED_BENCHMARK, guard);
  });
  yield();
  Serial.print(EMPTY_LOOP_LABEL);
  printNanosAsMicros((emptyLoopMillis) * TO_NANO);
  Serial.println(ENDING);
  Serial.println(DIVIDER);

  // DateTime(seconds) constructor
  unsigned long constructorFromSecondsMillis =
      runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    DateTime dateTime = DateTime(tickMillis);

    disableOptimization(dateTime);
    digitalWrite(LED_BENCHMARK, guard);
  });
  yield();
  Serial.print(CONSTRUCTOR2_LABEL);
  printNanosAsMicros((constructorFromSecondsMillis - emptyLoopMillis)
      * TO_NANO);
  Serial.println(ENDING);

  // DateTime::toDaysSinceEpoch()
  unsigned long toDaysSinceEpochMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    DateTime dateTime = DateTime(tickMillis);
    uint32_t daysSinceEpoch = dateTime.toDaysSinceEpoch();

    guard ^= daysSinceEpoch;
    disableOptimization(dateTime);
    digitalWrite(LED_BENCHMARK, guard);
  });
  yield();
  Serial.print(DAYS_SINCE_EPOCH_LABEL);
  printNanosAsMicros((toDaysSinceEpochMillis - constructorFromSecondsMillis)
      * TO_NANO);
  Serial.println(ENDING);

  // DateTime::toSecondsSinceEpoch()
  unsigned long toSecondsSinceEpochMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    DateTime dateTime = DateTime(tickMillis);
    uint32_t secondsSinceEpoch = dateTime.toSecondsSinceEpoch();

    guard ^= secondsSinceEpoch;
    disableOptimization(dateTime);
    digitalWrite(LED_BENCHMARK, guard);
  });
  yield();
  Serial.print(SECOND_SINCE_EPOCH_LABEL);
  printNanosAsMicros(
      (toSecondsSinceEpochMillis - constructorFromSecondsMillis) * TO_NANO);
  Serial.println(ENDING);

  Serial.println(BOTTOM);

  // Print some stats
  Serial.print("Number of iterations per run: ");
  Serial.println(COUNT);
}
