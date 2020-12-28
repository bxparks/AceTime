/*
 * Copyright 2019 Brian T. Park
 *
 * MIT License
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.format.TextStyle;
import java.time.zone.ZoneOffsetTransition;
import java.time.zone.ZoneRules;
import java.time.zone.ZoneRulesException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;

/**
 * Generate 'validation_data.json' from list of zones in the 'zones.txt' file.
 *
 * <pre>
 * {@code
 * $ javac GenerateData.java
 * $ java GenerateData [--start_year start] [--until_year until] [--validate_dst]
 *      < zones.txt
 * }
 * </pre>
 *
 * The zones.txt file is a list of fully qualified zone names (e.g. "America/Los_Angeles") listed
 * one zone per line. It will normally be generated programmatically using:
 *
 * <pre>
 * {@code
 * $ ../../tools/tzcompiler.sh --tag 2019a --action zonedb --language java
 * }
 * </pre>
 */
public class GenerateData {
  // Number of seconds from Unix epoch (1970-01-01T00:00:00Z) to AceTime epoch
  // (2000-01-01T00:00:00Z).
  private static final int SECONDS_SINCE_UNIX_EPOCH = 946684800;

  public static void main(String[] argv) throws IOException {
    String invocation = "java GenerateData " + String.join(" ", argv);

    // Parse command line flags
    int argc = argv.length;
    int argi = 0;
    if (argc == 0) {
      usageAndExit();
    }
    String start = "2000";
    String until = "2050";
    String format = "cpp";
    while (argc > 0) {
      String arg0 = argv[argi];
      if ("--start_year".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        start = arg0;
      } else if ("--until_year".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        until = arg0;
      } else if ("--".equals(arg0)) {
        break;
      } else if (arg0.startsWith("-")) {
        System.err.printf("Unknown flag '%s'%n", arg0);
        usageAndExit();
      } else if (!arg0.startsWith("-")) {
        break;
      }
      {argc--; argi++;} // shift-left
    }

    // Validate --start_year and --end_year.
    // Should check for NumberFormatException but too much overhead for this simple tool.
    int startYear = Integer.parseInt(start);
    int untilYear = Integer.parseInt(until);

    List<String> zones = readZones();
    GenerateData generator = new GenerateData(
        invocation, startYear, untilYear);
    Map<String, List<TestItem>> testData = generator.createTestData(zones);
    generator.printJson(testData);
  }

  private static void usageAndExit() {
    System.err.println("Usage: java GenerateData [--start_year {start}]");
    System.err.println("  [--until_year {until}] [--validate_dst] < zones.txt");
    System.exit(1);
  }

  /** Print out the list of ZoneIds in the java.time database. */
  static void printAvailableZoneIds() {
    Set<String> allZones = ZoneId.getAvailableZoneIds();
    System.out.printf("Found %s ids total:\n", allZones.size());
    SortedSet<String> selectedIds = new TreeSet<>();
    int numZones = 0;
    for (String id : allZones) {
      if (id.startsWith("Etc")) continue;
      if (id.startsWith("SystemV")) continue;
      if (id.startsWith("US")) continue;
      if (id.startsWith("Canada")) continue;
      if (id.startsWith("Brazil")) continue;
      if (!id.contains("/")) continue;
      selectedIds.add(id);
    }
    System.out.printf("Selected %s ids:\n", selectedIds.size());
    for (String id : selectedIds) {
      System.out.println("  " + id);
    }
  }

  /**
   * Return the list of zone names from the System.in.
   * Ignore empty lines and comment lines starting with '#'.
   */
  private static List<String> readZones() throws IOException {
    List<String> zones = new ArrayList<>();
    try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in))) {
      String line;
      while ((line = reader.readLine()) != null) {
        line = line.trim();
        if (line.isEmpty()) continue;
        if (line.startsWith("#")) continue;
        zones.add(line);
      }
    }
    return zones;
  }

  /** Constructor. */
  private GenerateData(String invocation, int startYear, int untilYear) {
    this.invocation = invocation;
    this.startYear = startYear;
    this.untilYear = untilYear;

    this.jsonFile = "validation_data.json";
  }

  /**
   * Create list of TestItems for each zone in this.zones. If the zone is missing from java.time,
   * create an entry with a null value to indicate that the zone is missing. E.g "Asia/Qostanay"
   * exists in 2019a but is missing from (openjdk version "11.0.3" 2019-04-16).
   */
  private Map<String, List<TestItem>> createTestData(List<String> zones) {
    Map<String, List<TestItem>> testData = new TreeMap<>();
    for (String zoneName : zones) {
      List<TestItem> testItems;
      try {
        ZoneId zoneId = ZoneId.of(zoneName);
        testItems = createValidationData(zoneId);
      } catch (ZoneRulesException e) {
        System.out.printf("Zone '%s' not found%n", zoneName);
        testItems = null;
      }
      testData.put(zoneName, testItems);
    }
    return testData;
  }

  /** Return a list of TestItems for zoneId sorted by increasing epochSeconds. */
  private List<TestItem> createValidationData(ZoneId zoneId) {
    Instant startInstant = ZonedDateTime.of(startYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();
    Instant untilInstant = ZonedDateTime.of(untilYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();

    // Map of (testItem.epochSeconds -> TestItem).
    Map<Integer, TestItem> testItems = new TreeMap<>();

    addTestItemsFromTransitions(testItems, zoneId, startInstant, untilInstant);
    addTestItemsFromSampling(testItems, zoneId, startInstant, untilInstant);

    List<TestItem> sortedItems = new ArrayList<>();
    for (TestItem item : testItems.values()) {
      sortedItems.add(item);
    }

    return sortedItems;
  }

  private static void addTestItemsFromTransitions(Map<Integer, TestItem> testItems, ZoneId zoneId,
      Instant startInstant, Instant untilInstant) {
    ZonedDateTime untilDateTime = ZonedDateTime.ofInstant(untilInstant, zoneId);
    ZoneRules rules = zoneId.getRules();
    Instant prevInstant = startInstant;
    int untilYear = untilDateTime.getYear();
    while (true) {
      // Exit if no more transitions
      ZoneOffsetTransition transition = rules.nextTransition(prevInstant);
      if (transition == null) {
        break;
      }
      // Exit if we get to untilYear.
      LocalDateTime transitionDateTime = transition.getDateTimeBefore();
      if (transitionDateTime.getYear() >= untilYear) {
        break;
      }

      // Get transition time
      Instant currentInstant = transition.getInstant();

      // One second before the transition, and at the transition
      addTestItem(testItems, currentInstant.minusSeconds(1), zoneId, 'A');
      addTestItem(testItems, currentInstant, zoneId, 'B');

      prevInstant = currentInstant;
    }
  }

  /** Add intervening sample test items from startInstant to untilInstant for zoneId. */
  private static void addTestItemsFromSampling(Map<Integer, TestItem> testItems, ZoneId zoneId,
      Instant startInstant, Instant untilInstant) {
    ZonedDateTime startDateTime = ZonedDateTime.ofInstant(startInstant, zoneId);
    ZonedDateTime untilDateTime = ZonedDateTime.ofInstant(untilInstant, zoneId);

    for (int year = startDateTime.getYear(); year < untilDateTime.getYear(); year++) {
      // Add the 1st of every month of every year.
      for (int month = 1; month <= 12; month++) {
        LocalDateTime localDateTime = LocalDateTime.of(year, month, 1, 0, 0, 0);
        ZonedDateTime zonedDateTime = ZonedDateTime.of(localDateTime, zoneId);
        addTestItem(testItems, zonedDateTime.toInstant(), zoneId, 'S');
      }
      // Add last day and hour of the year ({year}-12-31-23:00:00)
      LocalDateTime localDateTime = LocalDateTime.of(year, 12, 31, 23, 0, 0);
      ZonedDateTime zonedDateTime = ZonedDateTime.of(localDateTime, zoneId);
      addTestItem(testItems, zonedDateTime.toInstant(), zoneId, 'Y');
    }
  }

  /**
   * Add the TestItem at instant, with the epoch_seconds, UTC offset, and DST shift.
   */
  private static void addTestItem(Map<Integer, TestItem> testItems, Instant instant,
      ZoneId zoneId, char type) {
    TestItem testItem = createTestItem(instant, zoneId, type);
    if (testItems.containsKey(testItem.epochSeconds)) return;
    testItems.put(testItem.epochSeconds, testItem);
  }

  /** Create a test item using the instant to determine the offsets. */
  private static TestItem createTestItem(Instant instant, ZoneId zoneId, char type) {
    // Calculate the offsets using the instant
    ZoneRules rules = zoneId.getRules();
    Duration dst = rules.getDaylightSavings(instant);
    ZoneOffset offset = rules.getOffset(instant);

    // Convert instant to dateTime components.
    ZonedDateTime dateTime = ZonedDateTime.ofInstant(instant, zoneId);

    // Get abbreviation. See https://stackoverflow.com/questions/56167361. It looks like Java's
    // abbreviations are completely different than the abbreviations used in the TZ Database files.
    // For example, PST or PDT for America/Los_Angeles is returned as "PT", which seems brain-dead
    // since no one in America uses the abbreviation "PT.
    String abbrev = zoneId.getDisplayName(TextStyle.SHORT_STANDALONE, Locale.US);

    TestItem item = new TestItem();
    item.epochSeconds = (int) (instant.getEpochSecond() - SECONDS_SINCE_UNIX_EPOCH);
    item.utcOffset = offset.getTotalSeconds();
    item.dstOffset = (int) dst.getSeconds();
    item.year = dateTime.getYear();
    item.month = dateTime.getMonthValue();
    item.day = dateTime.getDayOfMonth();
    item.hour = dateTime.getHour();
    item.minute = dateTime.getMinute();
    item.second = dateTime.getSecond();
    item.abbrev = abbrev;
    item.type = type;

    return item;
  }

  /**
   * Print the JSON representation of the testData. We serialize JSON manually to avoid pulling in
   * any external dependencies, The TestData format is pretty simple.
   */
  /** Print the testData to a JSON file. */
  private void printJson(Map<String, List<TestItem>> testData) throws IOException {
    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(jsonFile)))) {
      String indentUnit = "  ";
      writer.println("{");
      String indent0 = indentUnit;
      writer.printf("%s\"start_year\": %s,\n", indent0, startYear);
      writer.printf("%s\"until_year\": %s,\n", indent0, untilYear);
      writer.printf("%s\"source\": \"Java11/java.time\",\n", indent0);
      writer.printf("%s\"version\": \"%s\",\n", indent0, System.getProperty("java.version"));
      // Set 'has_valid_abbrev' to false because java.time abbreviations seem completely different
      // than the ones provided by the TZ Database files.
      writer.printf("%s\"has_valid_abbrev\": false,\n", indent0);
      writer.printf("%s\"has_valid_dst\": true,\n", indent0);
      writer.printf("%s\"test_data\": {\n", indent0);

      // Print each zone
      int zoneCount = 1;
      int numZones = testData.size();
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        List<TestItem> items = entry.getValue();
        if (items == null) {
          zoneCount++;
          continue;
        }

        // Print the zone name
        String indent1 = indent0 + indentUnit;
        writer.printf("%s\"%s\": [\n", indent1, entry.getKey());

        // Print the testItems of the zone
        int itemCount = 1;
        for (TestItem item : items) {
          String indent2 = indent1 + indentUnit;
          writer.printf("%s{\n", indent2);
          {
            String indent3 = indent2 + indentUnit;
            writer.printf("%s\"epoch\": %d,\n", indent3, item.epochSeconds);
            writer.printf("%s\"total_offset\": %d,\n", indent3, item.utcOffset);
            writer.printf("%s\"dst_offset\": %d,\n", indent3, item.dstOffset);
            writer.printf("%s\"y\": %d,\n", indent3, item.year);
            writer.printf("%s\"M\": %d,\n", indent3, item.month);
            writer.printf("%s\"d\": %d,\n", indent3, item.day);
            writer.printf("%s\"h\": %d,\n", indent3, item.hour);
            writer.printf("%s\"m\": %d,\n", indent3, item.minute);
            writer.printf("%s\"s\": %d,\n", indent3, item.second);
            writer.printf("%s\"abbrev\": \"%s\",\n", indent3, item.abbrev);
            writer.printf("%s\"type\": \"%s\"\n", indent3, item.type);
          }
          writer.printf("%s}%s\n", indent2, (itemCount < items.size()) ? "," : "");
          itemCount++;
        }

        writer.printf("%s]%s\n", indent1, (zoneCount < numZones) ? "," : "");
        zoneCount++;
      }
      writer.printf("%s}\n", indent0);

      writer.printf("}\n");
    }

    System.out.printf("Created %s%n", jsonFile);
  }

  // constructor parameters
  private final String invocation;
  private final int startYear;
  private final int untilYear;

  // derived parameters
  private final String jsonFile;;
}

class TestItem {
  int epochSeconds; // seconds from AceTime epoch (2000-01-01T00:00:00Z)
  int utcOffset; // total UTC offset in seconds
  int dstOffset; // DST shift from standard offset in seconds
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  String abbrev;
  char type;
}
