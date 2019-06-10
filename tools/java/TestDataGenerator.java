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
 * $ javac TestDataGenerator.java
 * $ java TestDataGenerator
 * }
 */
public class TestDataGenerator {
  public static void main(String[] argv) {
    LocalDateTime ldt = LocalDateTime.now();
    ZoneId zoneLosAngeles = ZoneId.of("America/Los_Angeles");
    ZonedDateTime zdt = ZonedDateTime.ofInstant(
        Instant.now(), zoneLosAngeles);
    System.out.println("Hello TestDataGenerator");
    System.out.println("LocalDateTime is " + ldt);
    System.out.println("ZonedDateTime is " + zdt);

    // Print out the list of all ZoneIds
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

    // There seems to be a precompiled list of historical transitions.
    // For Los Angeles, there are 127, from 1883-11-18T12:00 to
    // 2008-11-02T01:00. (Why only 2008?)
    ZoneRules rules = zoneLosAngeles.getRules();
    List<ZoneOffsetTransition> transitions = rules.getTransitions();
    System.out.println("Num(Transitions) is " + transitions.size());
    for (ZoneOffsetTransition transition : transitions) {
      LocalDateTime l = transition.getDateTimeAfter();
      System.out.println("  Transition: " + l);
    }

    // Use nexTransition() to get future transitions from now to 2050.
    System.out.println("Future transitions");
    Instant instant = Instant.now();
    int count = 0;
    while (true) {
      ZoneOffsetTransition transition = rules.nextTransition(instant);
      ZoneOffset offsetAfter = transition.getOffsetAfter();
      instant = transition.getInstant();
      LocalDateTime dt = LocalDateTime.ofInstant(instant, zoneLosAngeles);
      if (dt.getYear() > 2050) {
        break;
      }

      System.out.println("  Transition: " + dt
          + "; offset: " + offsetAfter.getTotalSeconds());
      count++;
    }
    System.out.println("Found " + count + " transitions");
  }
}
