/*
 * Test the performance of the AceTime library.
 */

#include <stdio.h>
#include <string>
#include <vector> // vector<>
#include <Arduino.h>
#include <AceTime.h>
#include "BenchmarkAceTime.h"

using namespace std;
using namespace ace_time;

static ExtendedZoneProcessorCache<1> zoneProcesorCache;

static ExtendedZoneManager zoneManager(
  zonedbx::kZoneAndLinkRegistrySize,
  zonedbx::kZoneAndLinkRegistry,
  zoneProcesorCache
);

static volatile uint32_t epochSeconds;

static int processZone(const TimeZone& tz, int startYear, int untilYear) {
  //tz.printTo(SERIAL_PORT_MONITOR);
  //SERIAL_PORT_MONITOR.println();

  int count = 0;
  for (int16_t y = startYear; y < untilYear; y++) {
    for (uint8_t m = 1; m <= 12; m++) {
      for (uint8_t d = 1; d <= 28; d++) {
        count++;
        ZonedDateTime zdt = ZonedDateTime::forComponents(
            y, m, d, 1, 2, 3, tz);
        epochSeconds = zdt.toEpochSeconds();
      }
    }
  }
  return count;
}

void benchmarkAceTime(
    const vector<string>& zones,
    int startYear,
    int untilYear) {

  printf("benchmarkAceTime: start\n");
  unsigned long startMillis = millis();

  int totalCount = 0;
  for (string zoneName : zones) {
    TimeZone tz = zoneManager.createForZoneName(zoneName.c_str());
    int count = processZone(tz, startYear, untilYear);
    totalCount += count;
  }

  unsigned long elapsedMillis = millis() - startMillis;
  printf("benchmarkAceTime: zones=%d\n", (int) zones.size());
  printf("benchmarkAceTime: count=%d\n", totalCount);
  printf("benchmarkAceTime: elapsedMillis %ld\n", elapsedMillis);
  double duration = (totalCount == 0)
      ? 0.0
      : (double) elapsedMillis * 1000 / (double) totalCount;
  printf("benchmarkAceTime: micros/iter %.3f\n", duration);

  /*
  for (int16_t index = 0; index < zonedbx::kZoneAndLinkRegistrySize; index++) {
    TimeZone tz = zoneManager.createForIndex(index);
    processZone(tz);
  }
  */
}

