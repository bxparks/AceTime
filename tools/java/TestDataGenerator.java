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
    System.out.println("readZones():");
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

    if ("basic".equals(scope)) {
      this.cppFile = "validation_data.cpp";
      this.headerFile = "validation_data.h";
      this.dbNamespace = "zonedb";
    } else {
      this.cppFile = "validation_data.cpp";
      this.headerFile = "validation_data.h";
      this.dbNamespace = "zonedbx";
    }
  }

  private void process(List<String> zones) throws IOException {
    System.out.println("process():");
    Map<String, List<TestItem>> testData = createTestData(zones);
    printCpp(testData);
    printHeader(testData);
  }

  /** Create list of TestItems for each zone in this.zones. */
  private Map<String, List<TestItem>> createTestData(List<String> zones) {
    System.out.println("createTestData():");
    Map<String, List<TestItem>> testData = new TreeMap<>();
    for (String zoneName : zones) {
      ZoneId zoneId;
      try {
        zoneId = ZoneId.of(zoneName);
      } catch (ZoneRulesException e) {
        System.out.printf("Zone '%s' not found%n", zoneName);
        continue;
      }
      List<TestItem> testItems = createValidationData(zoneId);
      testData.put(zoneName, testItems);
    }
    return testData;
  }

  /** Return a list of TestItems for zoneId sorted by epochSecond. */
  private List<TestItem> createValidationData(ZoneId zoneId) {
    Instant startInstant = ZonedDateTime.of(startYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();
    Instant untilInstant = ZonedDateTime.of(untilYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();
    Map<Long, TestItem> testItems = new TreeMap<>();

    addTestItemsFromTransitions(testItems, zoneId, startInstant, untilInstant);
    addTestItemsFromSampling(testItems, zoneId, startInstant, untilInstant);

    List<TestItem> sortedItems = new ArrayList<>();
    for (TestItem item : testItems.values()) {
      sortedItems.add(item);
    }

    return sortedItems;
  }

  private static void addTestItemsFromTransitions(Map<Long, TestItem> testItems, ZoneId zoneId,
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
      Instant currentInstant = transition.getInstant();
      ZonedDateTime currentDateTime = ZonedDateTime.ofInstant(currentInstant, zoneId);
      if (currentDateTime.getYear() >= untilYear) {
        break;
      }

      // Check for corrections
      int correctionOffset = getCorrectionOffset(zoneId, currentDateTime);

      // One second before the transition, and at the transition
      addTestItem(testItems, currentInstant.minusSeconds(1), correctionOffset, zoneId, 'A');
      addTestItem(testItems, currentInstant, correctionOffset, zoneId, 'B');

      prevInstant = currentInstant;
    }
  }

  // Check for corrections
  private static int getCorrectionOffset(ZoneId zoneId, ZonedDateTime currentDateTime) {
    String name = zoneId.getId();
    Map<LocalDateTime, Integer> corrections = CORRECTIONS.get(name);
    if (corrections == null) return 0;

    LocalDateTime correctionTime = currentDateTime.toLocalDateTime();
    System.out.println("Correction Time: " + correctionTime);
    if (correctionTime.equals(LocalDateTime.of(2000, 4, 2, 0, 1))) {
      System.out.println("EQUALS");
    }
    Integer correction = corrections.get(correctionTime);
    if (correction == null) return 0;

    return correction;
  }

  /** Add intervening sample test items from startInstant to untilInstant for zoneId. */
  private static void addTestItemsFromSampling(Map<Long, TestItem> testItems, ZoneId zoneId,
      Instant startInstant, Instant untilInstant) {
    ZonedDateTime startDateTime = ZonedDateTime.ofInstant(startInstant, zoneId);
    ZonedDateTime untilDateTime = ZonedDateTime.ofInstant(untilInstant, zoneId);

    for (int year = startDateTime.getYear(); year < untilDateTime.getYear(); year++) {
      for (int month = 1; month <= 12; month++) {
        ZonedDateTime currentDateTime = ZonedDateTime.of(year, month, 1, 0, 0, 0, 0, zoneId);
        int correctionOffset = getCorrectionOffset(zoneId, currentDateTime);
        addTestItem(testItems, currentDateTime.toInstant(), correctionOffset, zoneId, 'S');
      }
      // Add {year}-12-31-23:00:00
      ZonedDateTime currentDateTime = ZonedDateTime.of(year, 12, 31, 23, 0, 0, 0, zoneId);
      int correctionOffset = getCorrectionOffset(zoneId, currentDateTime);
      addTestItem(testItems, currentDateTime.toInstant(), correctionOffset, zoneId, 'Y');
    }
  }

  /**
   * Add the TestItem at actualInstant, but with the renderingInstant corrected by
   * renderingCorrection.
   */
  private static void addTestItem(Map<Long, TestItem> testItems, Instant actualInstant,
      int renderingCorrection, ZoneId zoneId, char type) {
    Instant renderingInstant = actualInstant.minusSeconds(renderingCorrection);
    long renderingSecond = renderingInstant.toEpochMilli()/1000;
    TestItem testItem = createTestItem(actualInstant, renderingInstant, zoneId, type);
    testItems.put(renderingSecond, testItem);
  }

  /**
   * Create a test item using the actualInstant to determine the offsets, but using renderingInstant
   * when creating the TestItem object. This supports zones whose transition occurs at 00:01, which
   * AceTime truncates to 00:00.
   */
  private static TestItem createTestItem(Instant actualInstant, Instant renderingInstant,
      ZoneId zoneId, char type) {
    ZonedDateTime dt = ZonedDateTime.ofInstant(renderingInstant, zoneId);
    ZoneRules rules = zoneId.getRules();
    Duration dst = rules.getDaylightSavings(actualInstant);
    ZoneOffset offset = rules.getOffset(actualInstant);

    TestItem item = new TestItem();
    item.epochSecond = (int) (dt.toEpochSecond() - SECONDS_SINCE_UNIX_EPOCH);
    item.utcOffset = offset.getTotalSeconds() / 60;
    item.dstOffset = (int) dst.getSeconds() / 60;
    item.year = dt.getYear();
    item.month = dt.getMonthValue();
    item.day = dt.getDayOfMonth();
    item.hour = dt.getHour();
    item.minute = dt.getMinute();
    item.second = dt.getSecond();
    item.type = type;

    return item;
  }

  private void printCpp(Map<String, List<TestItem>> testData) throws IOException {
    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(cppFile)))) {
      writer.println("// This file was auto-generated by the following script:");
      writer.println();
      writer.printf ("// $ %s", invocation);
      writer.println();
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
    writer.printf ("  //     epoch, utc,  dst,     y,  m,  d,  h,  m,  s%n");

    for (TestItem item : testItems) {
      writer.printf("  { %10d, %4d, %4d, %4d, %2d, %2d, %2d, %2d, %2d }, // type=%c%n",
          item.epochSecond,
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
    writer.println("};");

    writer.println();
    writer.printf ("const ValidationData kValidationData%s = {%n", normalizedName);
    writer.printf ("  &kZone%s /*zoneInfo*/,%n", normalizedName);
    writer.printf ("  sizeof(kValidationItems%s)/sizeof(ValidationItem) /*numItems*/,%n",
        normalizedName);
    writer.printf ("  kValidationItems%s /*items*/,%n", normalizedName);
    writer.println("};");
  }

  private void printHeader(Map<String, List<TestItem>> testData) throws IOException {
    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(headerFile)))) {
      writer.println("// This file was auto-generated by the following script:");
      writer.println();
      writer.printf ("// $ %s", invocation);
      writer.println();
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

      writer.printf("// numZones: %d%n", testData.size());
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String zoneName = entry.getKey();
        String normalizedName = normalizeName(zoneName);
        writer.printf("extern const ValidationData kValidationData%s;%n", normalizedName);
      }

      writer.println("}");
      writer.println("}");
      writer.println();
      writer.println("#endif");
    }

    System.out.printf("Created %s%n", headerFile);
  }

  private static String normalizeName(String name) {
    return name.replace('/', '_').replace('-', '_');
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
  private final String dbNamespace;
}

class TestItem {
  int epochSecond;
  int utcOffset;
  int dstOffset;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  char type;
}
