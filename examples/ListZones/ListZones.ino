/*
 * A program to list the zones of a ZoneManager sorted by UTC offset and name,
 * illustrating the ZoneSorterByName and ZoneSorterByOffsetAndName classes.
 *
 * By default, this sorts all zones and links by UTC offset first, then by name.
 * On EpoxyDuino, the --sort command line flag allows sorting by name:
 *
 * $ ./ListZones [--sort (offset | name)]
 * Sorted 594 zones in 8 millis
 * UTC-12:00 Etc/GMT+12
 * UTC-11:00 Etc/GMT+11
 * ...
 * UTC+14:00 Etc/GMT-14
 * UTC+14:00 Pacific/Kiritimati
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
// Sorting option.
//---------------------------------------------------------------------------

enum class SortOrder : uint8_t {
  kNone,
  kOffsetAndName,
  kName
};

// User-defined option parameters.
SortOrder sortOrder = SortOrder::kOffsetAndName;

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
      if (arg_equals(argv[0], "offset")) {
        sortOrder = SortOrder::kOffsetAndName;
      } else if (arg_equals(argv[0], "name")) {
        sortOrder = SortOrder::kName;
      } else {
        fprintf(stderr, "Unknown option '%s'\n", argv[0]);
        usage_and_exit(1);
      }
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

  if (sortOrder == SortOrder::kNone) {
    fprintf(stderr, "Must provide --sort\n");
    usage_and_exit(1);
  }

  return argc_original - argc;
}

#endif

//---------------------------------------------------------------------------
// Print out the zones (and links) after sorting them by UTC offset, then by
// name.
//---------------------------------------------------------------------------

ExtendedZoneProcessorCache<1> zoneProcessorCache;
ExtendedZoneManager zoneManager(
  zonedbx::kZoneAndLinkRegistrySize,
  zonedbx::kZoneAndLinkRegistry,
  zoneProcessorCache
);

void printZones(uint16_t elapsedMillis, uint16_t indexes[], uint16_t size) {
  SERIAL_PORT_MONITOR.print("Sorted ");
  SERIAL_PORT_MONITOR.print(size);
  SERIAL_PORT_MONITOR.print(" zones in ");
  SERIAL_PORT_MONITOR.print(elapsedMillis);
  SERIAL_PORT_MONITOR.println(" millis");

  // Print each zone in the form of:
  // "UTC-08:00 America/Los_Angeles"
  for (uint16_t i = 0; i < size; i++) {
    ExtendedZone zone = zoneManager.getZoneForIndex(indexes[i]);
    TimeOffset stdOffset = TimeOffset::forMinutes(zone.stdOffsetMinutes());

    SERIAL_PORT_MONITOR.print(F("UTC"));
    stdOffset.printTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.print(' ');

    zone.printNameTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.println();
  }
}

void printZonesSortedByOffsetAndName() {
  uint16_t indexes[zonedbx::kZoneAndLinkRegistrySize];
  ZoneSorterByOffsetAndName<ExtendedZoneManager> zoneSorter(zoneManager);
  zoneSorter.fillIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);

  uint16_t startMillis = millis();
  zoneSorter.sortIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);
  uint16_t elapsedMillis = millis() - startMillis;

  printZones(elapsedMillis, indexes, zonedbx::kZoneAndLinkRegistrySize);
}

void printZonesSortedByName() {
  uint16_t indexes[zonedbx::kZoneAndLinkRegistrySize];
  ZoneSorterByName<ExtendedZoneManager> zoneSorter(zoneManager);
  zoneSorter.fillIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);

  uint16_t startMillis = millis();
  zoneSorter.sortIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);
  uint16_t elapsedMillis = millis() - startMillis;

  printZones(elapsedMillis, indexes, zonedbx::kZoneAndLinkRegistrySize);
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

  if (sortOrder == SortOrder::kOffsetAndName) {
    printZonesSortedByOffsetAndName();
  } else if (sortOrder == SortOrder::kName) {
    printZonesSortedByName();
  } else {
#if defined(EPOXY_DUINO)
    fprintf(stderr, "Invalid sortOrder\n");
    usage_and_exit(1);
#endif
  }

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
