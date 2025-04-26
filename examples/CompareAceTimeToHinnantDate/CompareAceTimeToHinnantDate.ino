/*
 * A program to compare the performance of the AceTime library with the
 * Hinnant Date library. According to this, AceTime is about 100-150X faster (!)
 * than Hinnant date library for converting a date component (YYYY-MM-DD
 * hh:mm:ss) into epoch seconds.
 *
 * The zones.txt file comes from
 * AceTimeSuite/validation/tests/HinnantExtendedTest/zonedbxhd/zones.txt.
 *
 * To compile the binary, type:
 *
 * $ make clean
 * $ make
 *
 * To run, type:
 *
 * $ make result.txt
 * $ cat result.txt
 * benchmarkAceTime: start
 * benchmarkAceTime: zones=377
 * benchmarkAceTime: count=12667200
 * benchmarkAceTime: elapsedMillis 3699
 * benchmarkAceTime: micros/iter 0.292
 * benchmarkHinnantDate: start
 * benchmarkHinnantDate: zones=377
 * benchmarkHinnantDate: count=452400
 * benchmarkHinnantDate: elapsedMillis 21531
 * benchmarkHinnantDate: micros/iter 47.593
 *
 * This program uses EpoxyDuino to compile the Arduino AceTime of the code on a
 * Unix platform. It compiles the Hinnant date library  as a normal C++ library
 * on a Unix platform. It requires the libcurl library to be installed, just
 * like AceSuiteSuite/validation/tools/compare_hinnant.
 */

#if ! defined(EPOXY_DUINO)
  #error Must be compiled using EpoxyDuino
#endif

#include <stdio.h> // fprintf()
#include <string.h> // strcmp()
#include <string>
#include <algorithm> // find_if()
#include <vector>
#include <iostream> // getline()

#include <Arduino.h>
#include <AceTime.h>

#include "BenchmarkAceTime.h"
#include "BenchmarkHinnantDate.h"

using namespace std;
using namespace ace_time;
using namespace ace_time::zonedbx;

//---------------------------------------------------------------------------
// Parse command line flags and arguments.
//---------------------------------------------------------------------------

/** Print usage and exit with status code. (0 means success). */
static void usage_and_exit(int status) {
  fprintf(stderr,
    "Usage: CompareAceTimeToHinnantDate "
    "[--install_dir {dir}] [--tz_version {version}] < zones.txt\n"
  );
  exit(status);
}

/** Shift the command line arguments to the left by one position. */
static void shift(int& argc, const char* const*& argv) {
  argc--;
  argv++;
}

/** Compare 2 C-strings. */
static bool argEquals(const char* s, const char* t) {
  return strcmp(s, t) == 0;
}

// Command line arguments
string startYearStr = "";
string untilYearStr = "";
string tzVersion = "";
string installDir = "";

int startYear = 2000;
int untilYear = 2050;

void usageAndExit() {
  fprintf(stderr,
    "Usage: generate_data [--install_dir {dir}] [--tz_version {version}]\n"
    "   --start_year start --until_year until --epoch_year year\n"
    "   < zones.txt\n");
  exit(1);
}

/**
 * Parse command line flags.
 * Returns the index of the first argument after the flags.
 */
static int parseFlags(int argc, const char* const* argv) {
  int argc_original = argc;
  shift(argc, argv);

  while (argc > 0) {
    if (argEquals(argv[0], "--start_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      startYearStr = argv[0];
    } else if (argEquals(argv[0], "--until_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      untilYearStr = argv[0];
    } else if (argEquals(argv[0], "--tz_version")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      tzVersion = argv[0];
    } else if (argEquals(argv[0], "--install_dir")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      installDir = argv[0];
    } else if (argEquals(argv[0], "--")) {
      shift(argc, argv);
      break;
    } else if (argEquals(argv[0], "--help")) {
      usage_and_exit(0);
      break;
    } else if (strncmp(argv[0], "-", 1) == 0) {
      fprintf(stderr, "Unknonwn flag '%s'\n", argv[0]);
      usage_and_exit(1);
    } else {
      break;
    }
    shift(argc, argv);
  }

  return argc_original - argc;
}

/**
 * Trim from start (in place). See https://stackoverflow.com/questions/216823
 */
inline void ltrim(string &s) {
  s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
      return !isspace(ch);
  }));
}

/** Read the 'zones.txt' from the stdin, and process each zone. */
vector<string> readZones() {
  vector<string> zones;
  string line;
  while (getline(cin, line)) {
    ltrim(line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;
    zones.push_back(line);
  }

  return zones;
}

//---------------------------------------------------------------------------

void setup() {
  /*int args = */parseFlags(epoxy_argc, epoxy_argv);
  if (startYearStr.empty()) {
    fprintf(stderr, "Flag required: --start_year\n");
    exit(1);
  }
  if (untilYearStr.empty()) {
    fprintf(stderr, "Flag required: --until_year\n");
    exit(1);
  }
  if (tzVersion.empty()) {
    fprintf(stderr, "Flag required: --tz_version\n");
    exit(1);
  }
  startYear = atoi(startYearStr.c_str());
  untilYear = atoi(untilYearStr.c_str());

  printf("startYear: %d\n", startYear);
  printf("untilYear: %d\n", untilYear);
  printf("tzVersion: %s\n", tzVersion.c_str());
  vector<string> zones = readZones();
  benchmarkAceTime(zones, startYear, untilYear);

  installTzDb(installDir, tzVersion);
  benchmarkHinnantDate(zones, startYear, untilYear);

  exit(0);
}

void loop() {}
