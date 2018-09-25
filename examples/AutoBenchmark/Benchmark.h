#if defined(LED_BUILTIN)
#define LED_BENCHMARK LED_BUILTIN
#else
#define LED_BENCHMARK 5
#endif

extern void runBenchmarks();
