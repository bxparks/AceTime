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
 * Generate the validation_data.* files for AceTime from the list of zone names in System.in.
 *
 * {@code
 * $ javac TestDataGenerator.java
 * $ java TestDataGenerator --scope (basic | extended) [--startYear start] [--untilYear until]
 *      < zones.txt
 * }
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
    while (argc > 0) {
      String arg0 = argv[argi];
      if ("--scope".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        if (argc == 0) usageAndExit();
        scope = arg0;
      } else if ("--startYear".equals(arg0)) {
        {argc--; argi++; arg0 = argv[argi];} // shift-left
        start = arg0;
      } else if ("--untilYear".equals(arg0)) {
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
    if (!"basic".equals(scope) && !"extended".equals(scope)) {
      System.err.printf("Unknown scope '%s'%n", scope);
      usageAndExit();
    }
    // Should check for NumberFormatException but too much overhead for this simple tool
    int startYear = Integer.parseInt(start);
    int untilYear = Integer.parseInt(until);

    List<String> zones = readZones();
    TestDataGenerator generator = new TestDataGenerator(invocation, scope, startYear, untilYear);
    generator.process(zones);
  }

  private static void usageAndExit() {
    System.err.println("Usage: java TestDataGenerator --scope (basic|extended)");
    System.err.println("       [--startYear {start}] [--untilYear {until}] < zones.txt");
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

  private TestDataGenerator(String invocation, String scope, int startYear, int untilYear) {
    this.invocation = invocation;
    this.scope = scope;
    this.startYear = startYear;
    this.untilYear = untilYear;

    this.cppFile = "validation_data.cpp";
    this.headerFile = "validation_data.h";
    this.testsFile = "validation_tests.cpp";
    if ("basic".equals(scope)) {
      this.dbNamespace = "zonedb";
    } else {
      this.dbNamespace = "zonedbx";
    }
  }

  private void process(List<String> zones) throws IOException {
    Map<String, List<TestItem>> testData = createTestData(zones);
    printDataCpp(testData);
    printDataHeader(testData);
    printTestsCpp(testData);
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

      // Get transition time correction, if any.
      int correctionOffset = getCorrectionOffset(zoneId, transitionDateTime);
      Instant currentInstant = transition.getInstant();

      // One second before the transition, and at the transition
      addTestItem(testItems, currentInstant.minusSeconds(1), correctionOffset, zoneId,
          (correctionOffset == 0) ? 'A' : 'a');
      addTestItem(testItems, currentInstant, correctionOffset, zoneId,
          (correctionOffset == 0) ? 'B' : 'b');

      prevInstant = currentInstant;
    }
  }

  /**
   * Get correction offset (if any) at the given transitionDateTime for the zoneId. The
   * transitionDateTime comes from java.time.ZoneId. The correction offset is the number of seconds
   * that we must *subtract* from the transitionDateTime to get the dateTime used by AceTime
   * library.
   */
  private static int getCorrectionOffset(ZoneId zoneId, LocalDateTime transitionDateTime) {
    String name = zoneId.getId();
    Map<LocalDateTime, Integer> corrections = CORRECTIONS.get(name);
    if (corrections == null) return 0;

    Integer correction = corrections.get(transitionDateTime);
    if (correction == null) return 0;

    return correction;
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
        int correctionOffset = getCorrectionOffset(zoneId, localDateTime);
        addTestItem(testItems, zonedDateTime.toInstant(), correctionOffset, zoneId, 'S');
      }
      // Add last day and hour of the year ({year}-12-31-23:00:00)
      LocalDateTime localDateTime = LocalDateTime.of(year, 12, 31, 23, 0, 0);
      ZonedDateTime zonedDateTime = ZonedDateTime.of(localDateTime, zoneId);
      int correctionOffset = getCorrectionOffset(zoneId, localDateTime);
      addTestItem(testItems, zonedDateTime.toInstant(), correctionOffset, zoneId, 'Y');
    }
  }

  /**
   * Add the TestItem at actualInstant, with the epoch_seconds, UTC offset, and DST shift calculated
   * at actualInstant, but the date and time components are calculated from the
   * actualDateTime.minus(truncationCorrection).
   */
  private static void addTestItem(Map<Integer, TestItem> testItems, Instant actualInstant,
      int truncationCorrection, ZoneId zoneId, char type) {
    TestItem testItem = createTestItem(actualInstant, truncationCorrection, zoneId, type);
    if (testItems.containsKey(testItem.epochSeconds)) return;
    testItems.put(testItem.epochSeconds, testItem);
  }

  /**
   * Create a test item using the actualInstant to determine the offsets, but using
   * truncatedDateTime when creating the TestItem object. This supports zones whose transition
   * occurs at 00:01, which AceTime truncates to 00:00.
   */
  private static TestItem createTestItem(Instant actualInstant, int truncationCorrection,
      ZoneId zoneId, char type) {
    // Calculate the offsets using the accurate actualInstant
    ZoneRules rules = zoneId.getRules();
    Duration dst = rules.getDaylightSavings(actualInstant);
    ZoneOffset offset = rules.getOffset(actualInstant);

    // Calculate the truncatedDateTime reported by the AceTime library using the
    // truncationCorrection.
    ZonedDateTime actualDateTime = ZonedDateTime.ofInstant(actualInstant, zoneId);
    LocalDateTime truncatedDateTime =
        actualDateTime.toLocalDateTime().minusSeconds(truncationCorrection);

    TestItem item = new TestItem();
    item.epochSeconds = (int) (actualInstant.getEpochSecond() - truncationCorrection
        - SECONDS_SINCE_UNIX_EPOCH);
    item.utcOffset = offset.getTotalSeconds() / 60;
    item.dstOffset = (int) dst.getSeconds() / 60;
    item.year = truncatedDateTime.getYear();
    item.month = truncatedDateTime.getMonthValue();
    item.day = truncatedDateTime.getDayOfMonth();
    item.hour = truncatedDateTime.getHour();
    item.minute = truncatedDateTime.getMinute();
    item.second = truncatedDateTime.getSecond();
    item.type = type;

    return item;
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
      writer.println("#include <AceTime.h>");
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
    writer.printf ("static const ValidationItem kValidationItems%s[] = {%n", normalizedName);
    writer.printf ("  //     epoch,  utc,  dst,    y,  m,  d,  h,  m,  s%n");

    for (TestItem item : testItems) {
      writer.printf("  { %10d, %4d, %4d, %4d, %2d, %2d, %2d, %2d, %2d }, // type=%c%n",
          item.epochSeconds,
          item.utcOffset,
          item.dstOffset,
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
    writer.printf ("const ValidationData kValidationData%s = {%n", normalizedName);
    writer.printf ("  &kZone%s /*zoneInfo*/,%n", normalizedName);
    writer.printf ("  sizeof(kValidationItems%s)/sizeof(ValidationItem) /*numItems*/,%n",
        normalizedName);
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
      writer.println("#include \"ValidationDataType.h\"");
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
        writer.printf("extern const ValidationData kValidationData%s;%n", normalizedName);
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
    return name.replace('/', '_').replace('-', '_');
  }

  /** Generate the validation_tests.cpp file. */
  private void printTestsCpp(Map<String, List<TestItem>> testData) throws IOException {
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
			writer.println("#include \"TransitionTest.h\"");
			writer.println("#include \"validation_data.h\"");
      writer.println();
      writer.printf ("// numZones: %d%n", activeCount);
      writer.printf ("// missingZones: %d%n", inactiveCount);

      // Create a test(TransitionTest, {zoneName}) entry for each zone, commenting out missing
      // zones.
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String comment = (entry.getValue() == null) ? "// " : "";
        String zoneName = entry.getKey();
        String normalizedName = normalizeName(zoneName);
        writer.printf("%stestF(TransitionTest, %s) {%n", comment, normalizedName);
        writer.printf("%s  assertValid(&ace_time::%s::kValidationData%s);%n",
            comment, dbNamespace, normalizedName);
        writer.printf("%s}%n", comment);
      }
    }

    System.out.printf("Created %s%n", testsFile);
  }

  private void printTestsCppInactiveEntry(PrintWriter writer, String zoneName) {
  }

  // List of transition corrections, serving the same function as the CORRECTIONS parameter in
  // tdgeneartor.py. Normally I would use an ImmutableMap<> but to avoid dependency to any external
  // libraries like Guava, I create this static map using old-school techniques.
  //
  // The LocalDateTime is the expected transition date from java.time.ZoneId. The Integer correction
  // is the number of seconds that the LocalDateTime needs to be shifted BACK to get the transition
  // calculated by AceTime.
  private static final Map<String, Map<LocalDateTime, Integer>> CORRECTIONS = new HashMap<>();
  {
    Map<LocalDateTime, Integer> GAZA = new HashMap<>();
    GAZA.put(LocalDateTime.of(2010, 3, 27, 0, 1), 60);
    GAZA.put(LocalDateTime.of(2010, 4, 1, 0, 1), 60);
    CORRECTIONS.put("Asia/Gaza", GAZA);

    Map<LocalDateTime, Integer> GOOSE_BAY = new HashMap<>();
    GOOSE_BAY.put(LocalDateTime.of(2000, 4, 2, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2000, 10, 29, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2001, 4, 1, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2001, 10, 28, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2002, 4, 7, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2002, 10, 27, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2003, 4, 6, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2003, 10, 26, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2004, 4, 4, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2004, 10, 31, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2005, 4, 3, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2005, 10, 30, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2006, 4, 2, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2006, 10, 29, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2007, 3, 11, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2007, 11, 4, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2008, 3, 9, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2008, 11, 2, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2009, 3, 8, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2009, 11, 1, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2010, 3, 14, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2010, 11, 7, 0, 1), 60);
    GOOSE_BAY.put(LocalDateTime.of(2011, 3, 13, 0, 1), 60);
    CORRECTIONS.put("America/Goose_Bay", GOOSE_BAY);

    Map<LocalDateTime, Integer> HEBRON = new HashMap<>();
    HEBRON.put(LocalDateTime.of(2011, 4, 1, 0, 1), 60);
    CORRECTIONS.put("Asia/Hebron", HEBRON);

    Map<LocalDateTime, Integer> MONCTON = new HashMap<>();
    MONCTON.put(LocalDateTime.of(2000, 4, 2, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2000, 10, 29, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2001, 4, 1, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2001, 10, 28, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2002, 4, 7, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2002, 10, 27, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2003, 4, 6, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2003, 10, 26, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2004, 4, 4, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2004, 10, 31, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2005, 4, 3, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2005, 10, 30, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2006, 4, 2, 0, 1), 60);
    MONCTON.put(LocalDateTime.of(2006, 10, 29, 0, 1), 60);
    CORRECTIONS.put("America/Moncton", MONCTON);

    Map<LocalDateTime, Integer> ST_JOHNS = new HashMap<>();
    ST_JOHNS.put(LocalDateTime.of(2000, 4, 2, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2000, 10, 29, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2001, 4, 1, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2001, 10, 28, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2002, 4, 7, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2002, 10, 27, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2003, 4, 6, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2003, 10, 26, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2004, 4, 4, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2004, 10, 31, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2005, 4, 3, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2005, 10, 30, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2006, 4, 2, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2006, 10, 29, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2007, 3, 11, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2007, 11, 4, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2008, 3, 9, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2008, 11, 2, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2009, 3, 8, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2009, 11, 1, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2010, 3, 14, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2010, 11, 7, 0, 1), 60);
    ST_JOHNS.put(LocalDateTime.of(2011, 3, 13, 0, 1), 60);
    CORRECTIONS.put("America/St_Johns", ST_JOHNS);
  }

  // constructor parameters
  private final String invocation;
  private final String scope;
  private final int startYear;
  private final int untilYear;

  // derived parameters
  private final String cppFile;
  private final String headerFile;
  private final String testsFile;;
  private final String dbNamespace;
}

class TestItem {
  int epochSeconds; // seconds from AceTime epoch (2000-01-01T00:00:00Z)
  int utcOffset; // total UTC offset in minutes
  int dstOffset; // DST shift from standard offset in minutes
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  char type;
}
