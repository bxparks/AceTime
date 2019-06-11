import java.time.Instant;
import java.time.LocalDateTime;
import java.time.OffsetDateTime;
import java.time.ZonedDateTime;
import java.time.ZoneOffset;
import java.time.ZoneId;
import java.time.zone.ZoneOffsetTransition;
import java.time.zone.ZoneRules;
import java.util.List;
import java.util.Set;
import java.util.SortedSet;
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

  public static void main(String[] argv) {
    LocalDateTime ldt = LocalDateTime.now();
    ZoneId zoneLosAngeles = ZoneId.of("America/Los_Angeles");
    ZonedDateTime zdt = ZonedDateTime.ofInstant(Instant.now(), zoneLosAngeles);
    System.out.println("Hello TestDataGenerator");
    System.out.println("LocalDateTime is " + ldt);
    System.out.println("ZonedDateTime is " + zdt);

    printBasicZones();
    printAvaiableZoneIds();
    printHistoricalTransitions(zoneLosAngeles);
    printFutureTransitions(zoneLosAngeles);

    TestDataGenerator generator = new TestDataGenerator();
    generator.process();
  }

  // Print out the list of all ZoneIds
  static void printAvaiableZoneIds() {
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

  static void printHistoricalTransitions(ZoneId zoneId) {
    // There seems to be a precompiled list of historical transitions.
    // For Los Angeles, there are 127, from 1883-11-18T12:00 to
    // 2008-11-02T01:00. (Why only 2008?)
    ZoneRules rules = zoneId.getRules();
    List<ZoneOffsetTransition> transitions = rules.getTransitions();
    System.out.println("Num(Transitions) is " + transitions.size());
    for (ZoneOffsetTransition transition : transitions) {
      LocalDateTime l = transition.getDateTimeAfter();
      System.out.println("  Transition: " + l);
    }
  }

  static void printFutureTransitions(ZoneId zoneId) {
    // Use nexTransition() to get future transitions from now to 2050.
    System.out.println("Future transitions");
    Instant instant = Instant.now();
    ZoneRules rules = zoneId.getRules();
    int count = 0;
    while (true) {
      ZoneOffsetTransition transition = rules.nextTransition(instant);
      instant = transition.getInstant();
      LocalDateTime dt = LocalDateTime.ofInstant(instant, zoneId);
      if (dt.getYear() > 2050) {
        break;
      }

      ZoneOffset offsetAfter = transition.getOffsetAfter();
      System.out.println("  Transition: " + dt + "; offset: " + offsetAfter.getTotalSeconds());
      count++;
    }
    System.out.println("Found " + count + " transitions");
  }

  // Print the list of zones in BasicZones.
  static void printBasicZones() {
    System.out.println("Found " + BasicZones.ZONES.length + " zones");
    for (String zone : BasicZones.ZONES) {
      System.out.println("  Zone: " + zone);
    }
  }

  private void process() {
    ZoneId zoneLosAngeles = ZoneId.of("America/Los_Angeles");
    createValidationData(zoneLosAngeles);
  }

  private List<TestItem> createValidationData(ZoneId zoneId) {
    Instant instant = LocalDateTime.of(startYear, 1, 1, 0, 0, 0)
        .toInstant(ZoneOffset.UTC);
    ZoneRules rules = zoneId.getRules();
    List<TestItem> testItems = new ArrayList<>();
    while (true) {
      ZoneOffsetTransition transition = rules.nextTransition(instant);
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
  }

  private TestItem createTestItem(Instant instant, ZoneOffset offset, char type) {
    LocalDateTime ldt = LocalDateTime.ofInstant(instant, offset);
    TestItem item = new TestItem();

    item.epochSeconds = instant.toEpochMillis() / 1000 - SECONDS_SINCE_UNIX_EPOCH;
    item.utcOffset = offset.getTotalSeconds();
    item.year = ldt.getYear();
    item.month = ldt.getMonth();
    item.day = ldt.getDayOfMonth();
    item.hour = ldt.getHour();
    item.minute = ldt.getMinute();
    item.second = ldt.getSecond();
    item.type = type;
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
