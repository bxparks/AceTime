/*
 * A program to list the zones of a ZoneManager sorted by UTC offset and name,
 * illustrating the ZoneSorter class.
 */

#if defined(EPOXY_DUINO)
#include <stdio.h> // fprintf()
#include <string.h> // strcmp()
#endif

#include <Arduino.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::zonedbx;

//---------------------------------------------------------------------------
// Parse command line flags and arguments.
//---------------------------------------------------------------------------

#if defined(EPOXY_DUINO)

/** Print usage and exit with status code. (0 means success). */
static void usage_and_exit(int status) {
  fprintf(stderr,
    "Usage: ListZones [--sort offset|name]\n"
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
const char* sortOrder = "offset";

/**
 * Parse command line flags.
 * Returns the index of the first argument after the flags.
 */
static int parse_flags(int argc, const char* const* argv) {
  int argc_original = argc;
  shift(argc, argv);

  while (argc > 0) {
    if (arg_equals(argv[0], "--sort")) {
      shift(argc, argv);
      if (argc == 0) usage_and_exit(1);
      sortOrder = argv[0];
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

  if (sortOrder == nullptr) {
    fprintf(stderr, "Must provide --sortOrder\n");
    usage_and_exit(1);
  }

  return argc_original - argc;
}

#endif

//---------------------------------------------------------------------------
// Print out the zones (and links) after sorting them by UTC offset, then by
// name.
//---------------------------------------------------------------------------

ExtendedZoneManager<1> zoneManager(
  zonedbx::kZoneAndLinkRegistrySize,
  zonedbx::kZoneAndLinkRegistry
);

void printZonesSortedByOffsetAndName() {
  uint16_t indexes[zonedbx::kZoneAndLinkRegistrySize];
  ZoneSorter<ExtendedZoneManager<1>> zoneSorter(zoneManager);
  zoneSorter.fillIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);

  uint16_t startMillis = millis();
  zoneSorter.sortIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);
  uint16_t elapsedMillis = millis() - startMillis;

  SERIAL_PORT_MONITOR.print("Sorted ");
  SERIAL_PORT_MONITOR.print(zonedbx::kZoneAndLinkRegistrySize);
  SERIAL_PORT_MONITOR.print(" zones in ");
  SERIAL_PORT_MONITOR.print(elapsedMillis);
  SERIAL_PORT_MONITOR.println(" millis");

  // Print each zone in the form of:
  // "UTC-08:00 America/Los_Angeles"
  for (uint16_t i = 0; i < zonedbx::kZoneAndLinkRegistrySize; i++) {
    ExtendedZone zone = zoneManager.getZoneForIndex(indexes[i]);
    TimeOffset stdOffset = TimeOffset::forMinutes(zone.stdOffsetMinutes());

    SERIAL_PORT_MONITOR.print(F("UTC"));
    stdOffset.printTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.print(' ');

    zone.printNameTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.println();
  }
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

#if defined(EPOXY_DUINO)
  /*int args = */parse_flags(epoxy_argc, epoxy_argv);
#endif

  printZonesSortedByOffsetAndName();

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
