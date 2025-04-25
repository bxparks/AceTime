// This file was copied from src/zonedb/zone_registry.cpp, then reduced
// to 85 zones so that AutoBenchmark can fit inside the 32kB limit of an
// Arduino Nano.

#ifndef AUTO_BENCHMARK_COMPLETE_REGISTRY_H
#define AUTO_BENCHMARK_COMPLETE_REGISTRY_H

#include <AceTime.h>

const uint16_t kCompleteRegistrySize = 83;
extern const ace_time::complete::Info::ZoneInfo* const
    kCompleteRegistry[kCompleteRegistrySize];

#endif
