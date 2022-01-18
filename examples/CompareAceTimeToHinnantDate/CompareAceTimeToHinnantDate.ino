/*
 * A program to compare the performance of the AceTime library with the
 * Hinnant Date library. According to this, AceTime is about a 90X faster (!)
 * than Hinnant date library for converting a date component (YYYY-MM-DD
 * hh:mm:ss) into epoch seconds.
 *
 * The zones.txt file comes from
 * AceTimeValidation/ExtendedHinnantDate/zonedbxhd/zones.txt.
 *
 * To compile the binary, type:
 *
 * $ make clean
 * $ make
 *
 * To run, type:
 *
 * $ ./CompareAceTimeToHinnantDate < zones.txt
 * benchmarkAceTime: start
 * benchmarkAceTime: zones=377
 * benchmarkAceTime: count=6333600
 * benchmarkAceTime: elapsedMillis 3011
 * benchmarkAceTime: micros/iter 0.475
 * Downloading the tzdb...
 * Installing the tzdb...
 * Reloading the tzdb...
 * Loaded TZ Version 2021e
 * benchmarkHinnantDate: start
 * benchmarkHinnantDate: zones=377
 * benchmarkHinnantDate: count=226200
 * benchmarkHinnantDate: elapsedMillis 9260
 * benchmarkHinnantDate: micros/iter 40.937
 *
 * This program uses EpoxyDuino to compile the Arduino AceTime of the code on a
 * Unix platform. It compiles the Hinnant date library  as a normal C++ library
 * on a Unix platform. It requires the libcurl library to be installed, just
 * like AceTimeTools/compare_cpp.
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
static bool arg_equals(const char* s, const char* t) {
  return strcmp(s, t) == 0;
}

// Command line arguments
int startYear = 2000;
int untilYear = 2050;
string tzVersion = "2021e";
string installDir = "";

/**
 * Parse command line flags.
 * Returns the index of the first argument after the flags.
 */
static int parse_flags(int argc, const char* const* argv) {
  int argc_original = argc;
  shift(argc, argv);

  while (argc > 0) {
    if (arg_equals(argv[0], "--tz_version")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      tzVersion = argv[0];
    } else if (arg_equals(argv[0], "--install_dir")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      installDir = argv[0];
    } else if (arg_equals(argv[0], "--")) {
      shift(argc, argv);
      break;
    } else if (arg_equals(argv[0], "--help")) {
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
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

#if defined(EPOXY_DUINO)
  /*int args = */parse_flags(epoxy_argc, epoxy_argv);
#endif

  vector<string> zones = readZones();
  benchmarkAceTime(zones, startYear, untilYear);

  installTzDb(installDir, tzVersion);
  benchmarkHinnantDate(zones, startYear, untilYear);

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
