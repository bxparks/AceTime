#if defined(LED_BUILTIN)
  #define LED_BENCHMARK LED_BUILTIN
#else
  // define a random pin number for those boards that have multiple BUILTIN
  #define LED_BENCHMARK 5
#endif

extern void runBenchmarks();
