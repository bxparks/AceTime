/*
 * A program for debugging ExtendedZoneProcessor, similar to the
 * AceTimeTools/zinfo.py script.
 *
 * Requirements:
 *    * Change ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG to 1 in
 *      ExtendedZoneProcessor.h.
 *    * Compile on Unix desktop using EpoxyDuino.
 *
 * Usage:
 *    $ make
 *    $ ./DebugZoneProcessor.out --zone {name} --year {year}
 */

#include <stdio.h> // fprintf()
#include <string.h> // strcmp()
#include <Arduino.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>

using ace_common::PrintStr;
using namespace ace_time;
using namespace ace_time::zonedbx;

#if ! defined(EPOXY_DUINO)
  #error Supported only on a Unix environment using EpoxyDuino
#endif

//---------------------------------------------------------------------------
// Parse command line flags and arguments.
//---------------------------------------------------------------------------

/** Print usage and exit with status code. (0 means success). */
static void usage_and_exit(int status) {
  fprintf(stderr,
    "Usage: DebugZoneProcessor [--help] [--zone name] [--year year]\n"
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

// User-defined option parameters.
const char* zoneName = nullptr;
int year = 0;

/**
 * Parse command line flags.
 * Returns the index of the first argument after the flags.
 */
static int parse_flags(int argc, const char* const* argv) {
  int argc_original = argc;
  shift(argc, argv);

  while (argc > 0) {
    if (arg_equals(argv[0], "--year")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      year = atoi(argv[0]);
    } else if (arg_equals(argv[0], "--zone")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      zoneName = argv[0];
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

  if (year == 0) {
    fprintf(stderr, "Must provide --year\n");
    usage_and_exit(1);
  }
  if (zoneName == nullptr) {
    fprintf(stderr, "Must provide --zone\n");
    usage_and_exit(1);
  }

  return argc_original - argc;
}

//---------------------------------------------------------------------------
// Print out the internal state of ExtendedZoneProcessor for the selected zone
// and year.
//---------------------------------------------------------------------------

ExtendedZoneProcessorCache<2> zoneProcessorCache;
ExtendedZoneManager zoneManager(
  zonedbx::kZoneRegistrySize,
  zonedbx::kZoneRegistry,
  zoneProcessorCache
);

void printZoneInfo(const char* zoneName, int year) {
  ExtendedZoneProcessor* processor = zoneManager.getZoneProcessor(zoneName);
  if (! processor) {
    SERIAL_PORT_MONITOR.print(zoneName);
    SERIAL_PORT_MONITOR.println(" not found");
    return;
  }

  processor->initForYear(year);
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

  /*int args = */parse_flags(epoxy_argc, epoxy_argv);
  printZoneInfo(zoneName, year);

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
