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
import java.time.zone.ZoneOffsetTransition;
import java.time.zone.ZoneRules;
import java.time.zone.ZoneRulesException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;

/**
 * Generate 'validation_data.cpp', 'validation_data.h' and 'validation_tests.cpp' files for AceTime
 * from the list of zone names in System.in, normally supplied by a 'zones.txt' file.
 *
 * <pre>
 * {@code
 * $ javac TestDataGenerator.java
 * $ java TestDataGenerator --scope (basic | extended) [--start_year start] [--until_year until]
 *      [--format (cpp|json)
 *      < zones.txt
 * }
 * </pre>
 *
 * The zones.txt file is a list of fully qualified zone names (e.g. "America/Los_Angeles") listed
 * one zone per line. It will normally be generated programmatically using:
 *
 * <pre>
 * {@code
 * $ ../../tools/tzcompiler.sh --tag 2019a --scope extended --action zonedb --language java
 * }
 * </pre>
 */
public class TestDataGenerator {
  // Number of seconds from Unix epoch (1970-01-01T00:00:00Z) to AceTime epoch
  // (2000-01-01T00:00:00Z).
  private static final int SECONDS_SINCE_UNIX_EPOCH = 946684800;

  public static void main(String[] argv) throws IOException {
    String invocation = "java TestDataGenerator " + String.join(" ", argv);

    // Parse command line flags
    int argc = argv.length;
    int argi = 0;
    if (argc == 0) {
      usageAndExit();
    }
    String scope = null;
    String start = "2000";
    String until = "2050";
    String dbNamespace = null;
    String format = "cpp";
    while (argc > 0) {
      String arg0 = argv[argi];
      if ("--scope".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        if (argc == 0) usageAndExit();
        scope = arg0;
      } else if ("--start_year".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        start = arg0;
      } else if ("--until_year".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        until = arg0;
      } else if ("--db_namespace".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        dbNamespace = arg0;
      } else if ("--format".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        format = arg0;
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

    // Validate --scope.
    if (!"basic".equals(scope) && !"extended".equals(scope)) {
      System.err.printf("Unknown scope '%s'%n", scope);
      usageAndExit();
    }

    // Validate --start_year and --end_year.
    // Should check for NumberFormatException but too much overhead for this simple tool.
    int startYear = Integer.parseInt(start);
    int untilYear = Integer.parseInt(until);

    // Validate --db_namespace. Assume defaults if flag not given.
    if (dbNamespace == null) {
      if ("basic".equals(scope)) {
        dbNamespace = "zonedb";
      } else {
        dbNamespace = "zonedbx";
      }
    }

    // Validate --format (json|cpp)
    if (!"json".equals(format) && !"cpp".equals(format)) {
      System.err.printf("Unknown format '%s'%n", format);
      usageAndExit();
    }

    List<String> zones = readZones();
    TestDataGenerator generator = new TestDataGenerator(invocation, scope, dbNamespace,
        startYear, untilYear);
    Map<String, List<TestItem>> testData = generator.createTestData(zones);
    if ("cpp".equals(format)) {
      generator.printCpp(testData);
    } else {
      generator.printJson(testData);
    }
  }

  private static void usageAndExit() {
    System.err.println("Usage: java TestDataGenerator --scope (basic|extended)");
    System.err.println("       [--db_namespace ns] [--start_year {start}] ");
    System.err.println("       [--untilYear {until}] [--format (cpp|json)]");
    System.err.println("       < zones.txt");
    System.exit(1);
  }

  // Print out the list of all ZoneIds
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
  private TestDataGenerator(String invocation, String scope, String dbNamespace,
      int startYear, int untilYear) {
    this.invocation = invocation;
    this.scope = scope;
    this.dbNamespace = dbNamespace;
    this.startYear = startYear;
    this.untilYear = untilYear;
    this.isDefaultNamespace = ("zonedb".equals(dbNamespace) || "zonedbx".equals(dbNamespace));

    this.cppFile = "validation_data.cpp";
    this.headerFile = "validation_data.h";
    this.testsFile = "validation_tests.cpp";
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
    item.type = type;

    return item;
  }

  /** Print the testData to C++ validation files. */
  private void printCpp(Map<String, List<TestItem>> testData) throws IOException {
    printDataCpp(testData);
    printDataHeader(testData);
    printTestsCpp(testData);
  }

  /** Generate the validation_data.cpp file. */
  private void printDataCpp(Map<String, List<TestItem>> testData) throws IOException {
    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(cppFile)))) {
      writer.println("// This file was auto-generated by the following script:");
      writer.println("//");
      writer.printf ("// $ %s%n", invocation);
      writer.println("//");
      writer.println("// DO NOT EDIT");
      writer.println();
      writer.println("#include <ace_time/testing/ValidationDataType.h>");
      writer.println("#include \"validation_data.h\"");
      writer.println();
      writer.println("namespace ace_time {");
      writer.printf ("namespace %s {%n", dbNamespace);

      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String zoneName = entry.getKey();
        List<TestItem> testItems = entry.getValue();
        if (testItems == null) continue;
        printTestItemsCpp(writer, zoneName, testItems);
      }

      writer.println();
      writer.println("}");
      writer.println("}");
    }

    System.out.printf("Created %s%n", cppFile);
  }

  private static void printTestItemsCpp(PrintWriter writer, String zoneName,
      List<TestItem> testItems) {
    String normalizedName = normalizeName(zoneName);

    writer.println();
    writer.println("//---------------------------------------------------------------------------");
    writer.printf ("// Zone name: %s%n", zoneName);
    writer.println("//---------------------------------------------------------------------------");
    writer.println();
    writer.printf ("static const testing::ValidationItem kValidationItems%s[] = {%n", normalizedName);
    writer.printf ("  //     epoch,  utc,  dst,    y,  m,  d,  h,  m,  s, abbrev%n");

    for (TestItem item : testItems) {
      writer.printf("  { %10d, %4d, %4d, %4d, %2d, %2d, %2d, %2d, %2d, nullptr }, // type=%c%n",
          item.epochSeconds,
          item.utcOffset / 60,
          item.dstOffset / 60,
          item.year,
          item.month,
          item.day,
          item.hour,
          item.minute,
          item.second,
          item.type);
    }
    writer.println(); // extra blank just to match format generated by arvalgenerator.py
    writer.println("};");

    writer.println();
    writer.printf ("const testing::ValidationData kValidationData%s = {%n", normalizedName);
    writer.printf ("  %d /*numItems*/,%n", testItems.size());
    writer.printf ("  kValidationItems%s /*items*/,%n", normalizedName);
    writer.println("};");
  }

  /** Generate the validation_data.h file. */
  private void printDataHeader(Map<String, List<TestItem>> testData) throws IOException {
    int activeCount = getActiveCount(testData);
    int inactiveCount = testData.size() - activeCount;
    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(headerFile)))) {
      writer.println("// This file was auto-generated by the following script:");
      writer.println("//");
      writer.printf ("// $ %s%n", invocation);
      writer.println("//");
      writer.println("// DO NOT EDIT");
      writer.println();
      writer.println("#ifndef ACE_TIME_VALIDATION_TEST_VALIDATION_DATA_H");
      writer.println("#define ACE_TIME_VALIDATION_TEST_VALIDATION_DATA_H");
      writer.println();
      writer.println("#include <ace_time/testing/ValidationDataType.h>");
      writer.println();
      writer.println("namespace ace_time {");
      writer.printf ("namespace %s {%n", dbNamespace);
      writer.println();

      // Print list of active zones
      writer.printf("// numZones: %d%n", activeCount);
      if (inactiveCount > 0) {
        writer.printf("// missingZones: %d%n", inactiveCount);
      }
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String zoneName = entry.getKey();
        String normalizedName = normalizeName(zoneName);
        if (entry.getValue() == null) continue;
        writer.printf("extern const testing::ValidationData kValidationData%s;%n", normalizedName);
      }

      // Print list of missing zones
      if (inactiveCount > 0) {
        writer.println();
        writer.println("// Zones missing from java.time");
        writer.printf ("// missingZones: %d%n", testData.size() - activeCount);
        for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
          String zoneName = entry.getKey();
          String normalizedName = normalizeName(zoneName);
          if (entry.getValue() != null) continue;
          writer.printf("// extern const ValidationData kValidationData%s;%n", normalizedName);
        }
      }

      writer.println();
      writer.println("}");
      writer.println("}");
      writer.println();
      writer.println("#endif");
    }

    System.out.printf("Created %s%n", headerFile);
  }

  private static int getActiveCount(Map<String, List<TestItem>> testData) {
		int activeCount = 0;
    for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
      if (entry.getValue() != null) {
        activeCount++;
      }
    }
    return activeCount;
  }

  private static String normalizeName(String name) {
    return name.replaceAll("\\+", "_PLUS_").replaceAll("[^a-zA-Z0-9_]", "_");
  }

  /** Generate the validation_tests.cpp file. */
  private void printTestsCpp(Map<String, List<TestItem>> testData) throws IOException {
    String testClass;
    if ("basic".equals(scope)) {
      testClass = "BasicTransitionTest";
    } else {
      testClass = "ExtendedTransitionTest";
    }
    int activeCount = getActiveCount(testData);
    int inactiveCount = testData.size() - activeCount;

    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(testsFile)))) {
      writer.println("// This file was auto-generated by the following script:");
      writer.println("//");
      writer.printf ("// $ %s%n", invocation);
      writer.println("//");
      writer.println("// DO NOT EDIT");
      writer.println();
			writer.println("#include <AUnit.h>");
			writer.printf ("#include <ace_time/testing/%s.h>%n", testClass);
			writer.println("#include \"validation_data.h\"");
			writer.printf ("#include \"%s/zone_infos.h\"%n", dbNamespace);
      writer.println();
      writer.println("using namespace ace_time::testing;");
      writer.printf ("using namespace ace_time::%s;%n", dbNamespace);
      writer.println();
      writer.printf ("// numZones: %d%n", activeCount);
      writer.printf ("// missingZones: %d%n", inactiveCount);

      // Create a testF(TransitionTest, {zoneName}) entry for each zone, commenting out missing
      // zones.
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String comment = (entry.getValue() == null) ? "// " : "";
        String zoneName = entry.getKey();
        String normalizedName = normalizeName(zoneName);
        writer.printf("%stestF(%s, %s) {%n", comment, testClass, normalizedName);
        writer.printf("%s  assertValid(&kZone%s, &kValidationData%s);%n",
            comment, normalizedName, normalizedName);
        writer.printf("%s}%n", comment);
      }
    }

    System.out.printf("Created %s%n", testsFile);
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
      writer.printf("%s\"has_abbrev\": false,\n", indent0);
      // TODO(bpark): Check if has_valid_dst can be set to true for java.time.
      writer.printf("%s\"has_valid_dst\": false,\n", indent0);
      writer.printf("%s\"test_data\": {\n", indent0);

      // Print each zone
      int zoneCount = 1;
      int numZones = testData.size();
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String indent1 = indent0 + indentUnit;
        writer.printf("%s\"%s\": [\n", indent1, entry.getKey());

        List<TestItem> items = entry.getValue();
        if (items == null) continue;

        // Print each testItem
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
            writer.printf("%s\"abbrev\": null,\n", indent3);
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
  private final String scope;
  private final int startYear;
  private final int untilYear;
  private final String dbNamespace;

  // derived parameters
  private final boolean isDefaultNamespace;
  private final String cppFile;
  private final String headerFile;
  private final String testsFile;;
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
  // TODO(bpark): Add abbreviation from java.time.
  char type;
}
