#include <stdint.h>
#include <Arduino.h>

#if defined(LED_BUILTIN)
#define LED_BENCHMARK LED_BUILTIN
#else
#define LED_BENCHMARK 5
#endif

template <typename F>
unsigned long runLambda(uint32_t count, F&& lambda) {
  unsigned long startMillis = millis();
  while (count--) {
    lambda();
  }
  return millis() - startMillis;
}

extern void runBenchmark();
