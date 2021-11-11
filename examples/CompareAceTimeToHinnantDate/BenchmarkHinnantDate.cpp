/*
 * Test the performance of HowardHinnant date library.
 */

#include <iostream> // getline()
#include <map> // map<>
#include <vector> // vector<>
#include <algorithm> // sort()
#include <string.h> // strcmp(), strncmp()
#include <stdio.h> // printf(), fprintf()
#include <chrono>
#include <date/date.h>
#include <date/tz.h> // time_zone
#include <time.h> // clock_gettime()

#include "BenchmarkHinnantDate.h"

using namespace date;
using namespace std::chrono;
using namespace std;

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
static const long SECONDS_SINCE_UNIX_EPOCH = 946684800L;

static volatile uint32_t epochSeconds;

static unsigned long millis() {
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);
  unsigned long ms = spec.tv_sec * 1000U + spec.tv_nsec / 1000000UL;
  return ms;
}

void installTzDb(const string& installDir, const string& tzVersion) {
  // Set the install directory if specified. Otherwise the default is
  // ~/Downloads/tzdata on a Linux/MacOS machine. See
  // https://howardhinnant.github.io/date/tz.html#Installation.
  if (! installDir.empty()) {
    set_install(installDir);
  }

  // Explicitly download load the TZ Database at the specified version if
  // --tz_version is given. This works even if AUTO_DOWNLOAD=0. See
  // https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#thoughts-on-reloading-the-iana-tzdb-for-long-running-programs
  // and https://howardhinnant.github.io/date/tz.html#database.
  if (! tzVersion.empty()) {
    fprintf(stderr, "Downloading the tzdb...\n");
    if (! remote_download(tzVersion)) {
      fprintf(stderr, "Failed to download TZ Version %s\n", tzVersion.c_str());
      exit(1);
    }

    fprintf(stderr, "Installing the tzdb...\n");
    if (! remote_install(tzVersion)) {
      fprintf(stderr, "Failed to install TZ Version %s\n", tzVersion.c_str());
      exit(1);
    }
  }

  // Install the TZ database. Caution: If the source directory is pointed to
  // the raw https://github.com/eggert/tz/ repo, it is not in the form that is
  // expected (I think the 'version' file is missing), so the version returned
  // by get_tzdb() will be in correct.
  fprintf(stderr, "Reloading the tzdb...\n");
  reload_tzdb();
  if (tzVersion.empty()) {
    fprintf(stderr, "Loaded existing TZ Version %s\n",
        date::get_tzdb().version.c_str());
  } else {
    fprintf(stderr, "Loaded TZ Version %s\n", tzVersion.c_str());
  }
}

static int processZone(const string& zoneName, int startYear, int untilYear) {
  //fprintf(stderr, "BenchmarkHinnantDate: Processing %s\n", zoneName.c_str());
  auto* tzp = locate_zone(zoneName);
  if (tzp == nullptr) {
    fprintf(stderr, "BenchmarkHinnantDate: Zone %s not found\n",
        zoneName.c_str());
    return 0;
  }

  // Hinnant date seems to be about 100X slower than AceTime. So don't iterate
  // over days of month, to decrease the iteration count by 30X, allowing the
  // loop to finish in a reasonable time.
  int count = 0;
  for (int y = startYear; y < untilYear; y++) {
    for (int m = 1; m <= 12; m++) {
      //for (int d = 1; d <= 28; d++) {
        int d = 2;
        local_days ld = local_days{month(m)/d/year(y)};
        try {
          count++;
          zoned_time<seconds> zdt = make_zoned(tzp, ld + seconds(0));
          sys_seconds ss = zdt.get_sys_time();
          seconds unixSeconds = floor<seconds>(ss.time_since_epoch());
          epochSeconds = unixSeconds.count() + SECONDS_SINCE_UNIX_EPOCH;
        } catch (...) {
          //fprintf(stderr, "BenchmarkHinnantDate: Error with zone %s\n",
          //    zoneName.c_str());
        }
      //}
    }
  }
  return count;
}

/** Process each zoneName in zones and insert into testData map. */
void benchmarkHinnantDate(
    const vector<string>& zones, int startYear, int untilYear) {

  printf("benchmarkHinnantDate: start\n");
  unsigned long startMillis = millis();

  int totalCount = 0;
  for (string zoneName : zones) {
    int count = processZone(zoneName, startYear, untilYear);
    totalCount += count;
  }

  unsigned long elapsedMillis = millis() - startMillis;
  printf("benchmarkHinnantDate: zones=%d\n", (int) zones.size());
  printf("benchmarkHinnantDate: count=%d\n", totalCount);
  printf("benchmarkHinnantDate: elapsedMillis %ld\n", elapsedMillis);
  double duration = (totalCount == 0)
      ? 0.0
      : (double) elapsedMillis * 1000 / (double) totalCount;
  printf("benchmarkHinnantDate: micros/iter %.3f\n", duration);
}
