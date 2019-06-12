import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.Writer;
import java.io.IOException;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.OffsetDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.zone.ZoneOffsetTransition;
import java.time.zone.ZoneRules;
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
 * $ javac TestDataGenerator.java BasicZones.java ExtendedZones.java
 * $ java TestDataGenerator
 * }
 */
public class TestDataGenerator {
  private static final int SECONDS_SINCE_UNIX_EPOCH = 946684800;
  private static final String CPP_FILE = "validation.cpp";

  public static void main(String[] argv) throws IOException {
    LocalDateTime ldt = LocalDateTime.now();
    ZoneId zoneLosAngeles = ZoneId.of("America/Los_Angeles");
    ZonedDateTime zdt = ZonedDateTime.ofInstant(Instant.now(), zoneLosAngeles);
    System.out.println("Hello TestDataGenerator");
    System.out.println("LocalDateTime is " + ldt);
    System.out.println("ZonedDateTime is " + zdt);

    printAvailableZoneIds();

    TestDataGenerator generator = new TestDataGenerator();
    generator.process();
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

  private void process() throws IOException {
    System.out.println("Found " + BasicZones.ZONES.length + " zones");
    Map<String, List<TestItem>> testData = new TreeMap<>();
    for (String zoneName : BasicZones.ZONES) {
      System.out.println("Processing zone " + zoneName);
      ZoneId zoneId = ZoneId.of(zoneName);
      if (zoneId == null) {
        System.out.println("  Zone not found");
        continue;
      }

      List<TestItem> testItems = createValidationData(zoneId);
      testData.put(zoneName, testItems);
    }

    try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(CPP_FILE)))) {
      for (Map.Entry<String, List<TestItem>> entry : testData.entrySet()) {
        String zoneName = entry.getKey();
        List<TestItem> testItems = entry.getValue();
        printDataToFile(writer, zoneName, testItems);
      }
    }
  }

  private void printDataToFile(PrintWriter writer, String zoneName, List<TestItem> testItems) {
    writer.println(zoneName);
    for (TestItem item : testItems) {
      writer.printf("  %d %d %d %d %d %d %d %c%n",
          item.epochSeconds,
          item.year,
          item.month,
          item.day,
          item.hour,
          item.minute,
          item.second,
          item.type);
    }
  }

  private List<TestItem> createValidationData(ZoneId zoneId) {
    Instant instant = LocalDateTime.of(startYear, 1, 1, 0, 0, 0)
        .toInstant(ZoneOffset.UTC);
    ZoneRules rules = zoneId.getRules();
    List<TestItem> testItems = new ArrayList<>();
    while (true) {
      ZoneOffsetTransition transition = rules.nextTransition(instant);
      if (transition == null) {
        break;
      }
      instant = transition.getInstant();
      LocalDateTime dt = LocalDateTime.ofInstant(instant, zoneId);
      if (dt.getYear() > endYear) {
        break;
      }

      Instant instantBefore = instant.minusSeconds(1);
      ZoneOffset offsetBefore = transition.getOffsetBefore();
      TestItem item = createTestItem(instantBefore, offsetBefore, 'A');
      testItems.add(item);

      ZoneOffset offsetAfter = transition.getOffsetAfter();
      item = createTestItem(instant, offsetAfter, 'B');
      testItems.add(item);
    }

    return testItems;
  }

  private TestItem createTestItem(Instant instant, ZoneOffset offset, char type) {
    LocalDateTime ldt = LocalDateTime.ofInstant(instant, offset);
    TestItem item = new TestItem();

    item.epochSeconds = (int) (instant.toEpochMilli() / 1000 - SECONDS_SINCE_UNIX_EPOCH);
    item.utcOffset = offset.getTotalSeconds();
    item.year = ldt.getYear();
    item.month = ldt.getMonthValue();
    item.day = ldt.getDayOfMonth();
    item.hour = ldt.getHour();
    item.minute = ldt.getMinute();
    item.second = ldt.getSecond();
    item.type = type;

    return item;
  }

  private final int startYear = 2000;
  private final int endYear = 2050;
}

class TestItem {
  int epochSeconds;
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
