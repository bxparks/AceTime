/*
 * A program for debugging ExtendedZoneProcessor or CompleteZoneProcessor,
 * similar to the acetimepy/zinfo.py script.
 *
 * Requirements:
 *    * Change ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG to 1 in
 *      ExtendedZoneProcessor.h.
 *    * Compile on Unix desktop using EpoxyDuino.
 *
 * Usage:
 *    $ make
 *    $ ./DebugZoneProcessor.out --scope {scope} --zone {name} --year {year}
 */

#include <stdio.h> // fprintf()
#include <string.h> // strcmp()
#include <Arduino.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>

using ace_common::PrintStr;
using namespace ace_time;

#if ! defined(EPOXY_DUINO)
  #error Supported only on a Unix environment using EpoxyDuino
#endif

//---------------------------------------------------------------------------
// Parse command line flags and arguments.
//---------------------------------------------------------------------------

/** Print usage and exit with status code. (0 means success). */
static void usage_and_exit(int status) {
  fprintf(stderr,
    "Usage: DebugZoneProcessor [--help] [--scope {extended|complete}] "
    " [--zone name] [--year year]\n"
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
const char* scope = nullptr;
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
    if (arg_equals(argv[0], "--scope")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      scope = argv[0];
    } else if (arg_equals(argv[0], "--zone")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      zoneName = argv[0];
    } else if (arg_equals(argv[0], "--year")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      year = atoi(argv[0]);
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

  if (scope == nullptr) {
    fprintf(stderr, "Must provide --scope\n");
    usage_and_exit(1);
  }
  if (zoneName == nullptr) {
    fprintf(stderr, "Must provide --zone\n");
    usage_and_exit(1);
  }
  if (year == 0) {
    fprintf(stderr, "Must provide --year\n");
    usage_and_exit(1);
  }

  return argc_original - argc;
}

//---------------------------------------------------------------------------
// Print out the internal state of ExtendedZoneProcessor for the selected zone
// and year.
//---------------------------------------------------------------------------

ExtendedZoneProcessorCache<2> extendedZoneProcessorCache;

CompleteZoneProcessorCache<2> completeZoneProcessorCache;

void printExtendedZoneInfo(const char* zoneName, int year) {
  ExtendedZoneManager zoneManager(
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
    extendedZoneProcessorCache
  );
  ExtendedZoneProcessor* processor = zoneManager.getZoneProcessor(zoneName);
  if (! processor) {
    SERIAL_PORT_MONITOR.print(zoneName);
    SERIAL_PORT_MONITOR.println(" not found");
    return;
  }

  processor->initForYear(year);
  SERIAL_PORT_MONITOR.print("TransitionStorage alloc size: ");
  SERIAL_PORT_MONITOR.println(processor->getTransitionAllocSize());
}

void printCompleteZoneInfo(const char* zoneName, int year) {
  CompleteZoneManager zoneManager(
    zonedbc::kZoneRegistrySize,
    zonedbc::kZoneRegistry,
    completeZoneProcessorCache
  );
  CompleteZoneProcessor* processor = zoneManager.getZoneProcessor(zoneName);
  if (! processor) {
    SERIAL_PORT_MONITOR.print(zoneName);
    SERIAL_PORT_MONITOR.println(" not found");
    return;
  }

  processor->initForYear(year);
  SERIAL_PORT_MONITOR.print("TransitionStorage alloc size: ");
  SERIAL_PORT_MONITOR.println(processor->getTransitionAllocSize());
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
  if (strcmp(scope, "extended") == 0) {
    printExtendedZoneInfo(zoneName, year);
  } else if (strcmp(scope, "complete") == 0) {
    printCompleteZoneInfo(zoneName, year);
  } else {
    fprintf(stderr, "Uknonwn --scope flag '%s'\n", scope);
    usage_and_exit(1);
  }

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
