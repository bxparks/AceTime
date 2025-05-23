// This file was copied from src/zonedb/zone_registry.cpp, then reduced
// to 85 zones so that AutoBenchmark can fit inside the 32kB limit of an
// Arduino Nano.

#ifndef AUTO_BENCHMARK_EXTENDED_REGISTRY_H
#define AUTO_BENCHMARK_EXTENDED_REGISTRY_H

#include <AceTime.h>

extern const uint16_t kExtendedRegistrySize;
extern const ace_time::extended::Info::ZoneInfo* const kExtendedRegistry[];

#endif
