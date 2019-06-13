import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Writer;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.OffsetDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.zone.ZoneOffsetTransition;
import java.time.zone.ZoneRules;
import java.time.zone.ZoneRulesException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;

/**
 * Generate the validation_data.* files for AceTime.
 *
 * {@code
 * $ javac TestDataGenerator.java
 * $ java TestDataGenerator --scope (basic | extended) [--startYear start] [--untilYear until]
 * }
 */
public class TestDataGenerator {
  // Number of seconds from Unix epoch (1970-01-01T00:00:00Z) to AceTime epoch
  // (2000-01-01T00:00:00Z).
  private static final int SECONDS_SINCE_UNIX_EPOCH = 946684800;

  public static void main(String[] argv) throws IOException {
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
      } else if ("--untilYear".equals(arg0)) {
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
    TestDataGenerator generator = new TestDataGenerator(scope, startYear, untilYear, zones);
    generator.process();
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

  /** Return the list of zone names from the System.in. */
  private static List<String> readZones() throws IOException {
    System.out.println("readZones():");
    List<String> zones = new ArrayList<>();
    try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in))) {
      String line;
      while ((line = reader.readLine()) != null) {
        zones.add(line);
      }
    }
    return zones;
  }

  private TestDataGenerator(String scope, int startYear, int untilYear, List<String> zones) {
    this.scope = scope;
    this.startYear = startYear;
    this.untilYear = untilYear;
    this.zones = zones;

    if (scope == "basic") {
      this.cppFile = "validation_data.cpp";
      this.headerFile = "validation_data.h";
      this.dbNamespace = "zonedb";
    } else {
      this.cppFile = "validation_data.cpp";
      this.headerFile = "validation_data.h";
      this.dbNamespace = "zonedbx";
    }
  }

  private void process() throws IOException {
    System.out.println("process():");
    Map<String, List<TestItem>> testData = createTestData();
    printCpp(testData);
    printHeader(testData);
  }

  private Map<String, List<TestItem>> createTestData() {
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

  private List<TestItem> createValidationData(ZoneId zoneId) {
    Instant startInstant = ZonedDateTime.of(startYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();
    Instant untilInstant = ZonedDateTime.of(untilYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();
    List<TestItem> testItems = new ArrayList<>();

    addTestItemsFromTransitions(testItems, zoneId, startInstant, untilInstant);
    addTestItemsFromSampling(testItems, zoneId, startInstant, untilInstant);

    return testItems;
  }

  private static void addTestItemsFromTransitions(List<TestItem> testItems, ZoneId zoneId,
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

      // Add test items before and at the current instant.
      Instant instantBefore = currentInstant.minusSeconds(1);
      testItems.add(createTestItem(instantBefore, zoneId, 'A'));
      testItems.add(createTestItem(currentInstant, zoneId, 'B'));

      prevInstant = currentInstant;
    }
  }

  // Add intervening sample test items if the jump between transitions is too large.
  private static void addTestItemsFromSampling(List<TestItem> testItems, ZoneId zoneId,
      Instant startInstant, Instant untilInstant) {
    ZonedDateTime startDateTime = ZonedDateTime.ofInstant(startInstant, zoneId);
    ZonedDateTime untilDateTime = ZonedDateTime.ofInstant(untilInstant, zoneId);
    YearMonth startYm = new YearMonth(startDateTime.getYear(), startDateTime.getMonthValue());
    YearMonth untilYm = new YearMonth(untilDateTime.getYear(), untilDateTime.getMonthValue());

    YearMonth currentYm = new YearMonth(startYm.year, startYm.month);
    currentYm.incrementOneMonth();
    while (currentYm.compareTo(untilYm) < 0) {
      ZonedDateTime currentDateTime = ZonedDateTime.of(currentYm.year, currentYm.month, 1,
          0, 0, 0, 0, zoneId);
      Instant currentInstant = currentDateTime.toInstant();
      testItems.add(createTestItem(currentInstant, zoneId, 'S'));
      currentYm.incrementOneMonth();
    }
  }

  /** Helper class that emulates a 2-tuple of (year, month) with a compareTo() method. */
  private static class YearMonth {
    YearMonth(int year, int month) {
      this.year = year;
      this.month = month;
    }

    int compareTo(YearMonth other) {
      if (year > other.year) {
        return 1;
      }
      if (year < other.year) {
        return -1;
      }
      if (month > other.month) {
        return 1;
      }
      if (month < other.month) {
        return -1;
      }
      return 0;
    }

    void incrementOneMonth() {
      month++;
      if (month > 12) {
        year++;
        month = 1;
      }
    }

    private int year;
    private int month;
  }

  private static TestItem createTestItem(Instant instant, ZoneId zoneId, char type) {
    ZonedDateTime dt = ZonedDateTime.ofInstant(instant, zoneId);
    ZoneRules rules = zoneId.getRules();
    Duration dst = rules.getDaylightSavings(instant);
    ZoneOffset offset = rules.getOffset(instant);

    TestItem item = new TestItem();
    item.epochSecond = (int) (dt.toEpochSecond() - SECONDS_SINCE_UNIX_EPOCH);
    item.utcOffset = offset.getTotalSeconds();
    item.dstOffset = (int) dst.getSeconds();
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
      writer.println("#include <AceTime.h>");
      writer.println("#include \"validation_data.h\"");
      writer.println("namespace ace_time {");
      writer.printf("namespace %s {%n", dbNamespace);

      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String zoneName = entry.getKey();
        List<TestItem> testItems = entry.getValue();
        printDataToFile(writer, zoneName, testItems);
      }

      writer.println("}");
      writer.println("}");
    }

    System.out.printf("Created %s%n", cppFile);
  }

  private void printHeader(Map<String, List<TestItem>> testData) {
  }

  private void printDataToFile(PrintWriter writer, String zoneName, List<TestItem> testItems) {
    writer.println(zoneName);
    for (TestItem item : testItems) {
      writer.printf("  %d %d %d %d %d %d %d %c%n",
          item.epochSecond,
          item.year,
          item.month,
          item.day,
          item.hour,
          item.minute,
          item.second,
          item.type);
    }
  }

  private final String scope;
  private final List<String> zones;

  private final String cppFile;
  private final String headerFile;
  private final String dbNamespace;

  private final int startYear;
  private final int untilYear;
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
