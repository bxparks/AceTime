/*
 * Generate the validation_data.* files for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using Howard Hinnant's date.h
 * and tz.h library.
 *
 * Usage:
 * $ ./test_data_generator.out \
 *    --scope (basic | extended) \
 *    --tz_version {version} \
 *    [--db_namespace {db}] \
 *    [--start_year start] \
 *    [--until_year until] \
 *    < zones.txt
 */

#include <stdio.h>
#include <iostream>
#include <regex>
#include <chrono>
#include <date/date.h>
#include <date/tz.h>

using namespace date;
using namespace std::chrono;
using namespace std;

/** DateTime components. */
struct DateTime {
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
};

/**
 * A test item, containing the epochSeconds with its expected DateTime
 * components.
 */
struct TestItem {
  long epochSeconds;
  int utcOffset; // seconds
  int dstOffset; // seconds
  string abbrev;
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  char type; //'A', 'B', 'S', 'T' or 'Y'
};

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
const long SECONDS_SINCE_UNIX_EPOCH = 946684800;

// Output files
const char VALIDATION_DATA_CPP[] = "validation_data.cpp";
const char VALIDATION_DATA_H[] = "validation_data.h";
const char VALIDATION_TESTS_CPP[] = "validation_tests.cpp";
const char VALIDATION_DATA_JSON[] = "validation_data.json";

// Command line arguments
string scope = "";
int startYear = 2000;
int untilYear = 2050;

/**
 * Convert a zoned_time<> (which is an aggregation of time_zone and sys_time<>,
 * and sys_time<> is an alias for a std::chrono::time_point<>) into components.
 * See
 * https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#components_to_time_point
 * which describes how to convert a time_point<> into components. (I don't know
 * why it has to be so complicated...)
 */
DateTime toDateTime(local_time<seconds> lt) {
  auto daypoint = floor<days>(lt);
  auto ymd = year_month_day(daypoint);
  auto tod = make_time(lt - daypoint);
  return DateTime{
    int(ymd.year()),
    unsigned(ymd.month()),
    unsigned(ymd.day()),
    (int)tod.hours().count(),
    (int)tod.minutes().count(),
    (int)tod.seconds().count()
  };
}

/**
 * Convert the Unix epoch seconds into a ZonedDateTime, then convert that into
 * TestItem that has the Date/Time components broken out, along with the
 * expected DST offset and abbreviation.
 *
 * According to https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes
 * sys_info has the following structure:
 *
 * struct sys_info
 * {
 *     second_point         begin;
 *     second_point         end;
 *     std::chrono::seconds offset;
 *     std::chrono::minutes save;
 *     std::string          abbrev;
 * };
 */
TestItem toTestItem(const time_zone& tz, sys_seconds st, char type) {
  sys_info info = tz.get_info(st);
  seconds unixSeconds = floor<seconds>(st.time_since_epoch());
  zoned_time<seconds> zdt = make_zoned(&tz, st);
  local_time<seconds> lt = zdt.get_local_time();
  DateTime dateTime = toDateTime(lt);
  return TestItem{
      unixSeconds.count() - SECONDS_SINCE_UNIX_EPOCH,
      (int)info.offset.count(),
      (int)info.save.count() * 60,
      info.abbrev,
      dateTime.year,
      dateTime.month,
      dateTime.day,
      dateTime.hour,
      dateTime.minute,
      dateTime.second,
      type
  };
}

typedef map<string, vector<TestItem>> TestData;

void addTestItem(TestData& testData, const string& zoneName,
    const TestItem& item) {
  auto it = testData.find(zoneName);
  if (it == testData.end()) {
    testData[zoneName] = vector<TestItem>();
  }
  // Argh: There's probably a way to reuse the 'it' iterator and avoid
  // a second lookup but I don't have the time and patience to figure out the
  // C++ map<> API, and this is good enough for this program.
  auto &v = testData[zoneName];
  v.push_back(item);
}

/**
 * Add a TestItem for one second before a DST transition, and right at the
 * the DST transition.
 */
void addTransitions(TestData& testData, const time_zone& tz,
    const string& zoneName, int startYear, int untilYear) {
  sys_seconds begin = sys_days{January/1/startYear} + seconds(0);
  sys_seconds end = sys_days{January/1/untilYear} + seconds(0);

   do {
    // One second before the DST transition.
    sys_seconds before = begin - seconds(1);
    auto item = toTestItem(tz, before, 'A');
    addTestItem(testData, zoneName, item);

    // At the DST transition.
    item = toTestItem(tz, begin, 'B');
    addTestItem(testData, zoneName, item);

    sys_info info = tz.get_info(begin);
    begin = info.end;
  } while (begin < end);
}

/**
 * Add a TestItem for the 1st of each month (using the local time)
 * as a sanity sample, to make sure things are working, even for timezones with
 * no DST transitions. See
 * https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#obtaining-a-time_point-from-ymd-hms-components
 * to get code for converting date/time components to a time_point<> (aka
 * sys_time<>).
 */
void addMonthlySamples(TestData& testData, const time_zone& tz,
    const string& zoneName, int startYear, int untilYear) {

  for (int y = startYear; y < untilYear; y++) {
    // Add the 1st of every month...
    for (int m = 1; m <= 12; m++) {
      char type = 'S';

      // ...unless that day is ambiguous, in which case the Hinnant date
      // library throws an exception. Unfortunately, I cannot understand
      // the documentation to figure out how to do what I want, so just punt
      // and use the next day.
      for (int d = 1; d < 29; d++) {
        local_days ld = local_days{month(m)/d/year(y)};
        try {
          zoned_time<seconds> zdt = make_zoned(&tz, ld + seconds(0));

          sys_seconds ss = zdt.get_sys_time();
          TestItem item = toTestItem(tz, ss, type);
          addTestItem(testData, zoneName, item);
          break;
        } catch (...) {
          type = 'T';
        }
      }
    }

    // Add the last day of the year...
    local_days ld = local_days{year(y)/December/1};
    try {
      zoned_time<seconds> zdt = make_zoned(&tz, ld + seconds(0));
      sys_seconds ss = zdt.get_sys_time();
      TestItem item = toTestItem(tz, ss, 'Y');
      addTestItem(testData, zoneName, item);
    } catch (...) {
      // ...unless it's an ambiguous date, in which case just skip it.
    }
  }
}

/** Insert TestItems for the given 'zoneName' into testData. */
void processZone(TestData& testData, const string& zoneName,
    int startYear, int untilYear) {
  auto* tzp = locate_zone(zoneName);
  if (tzp == nullptr) {
    fprintf(stderr, "Zone %s not found\n", zoneName.c_str());
    return;
  }

  addTransitions(testData, *tzp, zoneName, startYear, untilYear);
  addMonthlySamples(testData, *tzp, zoneName, startYear, untilYear);
}

/**
 * Trim from start (in place). See https://stackoverflow.com/questions/216823
 */
inline void ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
			return !isspace(ch);
	}));
}

/** Process each zoneName in zones and insert into testData map. */
map<string, vector<TestItem>> processZones(const vector<string>& zones) {
  TestData testData;
  for (string zoneName : zones) {
    processZone(testData, zoneName, startYear, untilYear);
  }
  return testData;
}

/** Read the 'zones.txt' from the stdin, and process each zone. */
vector<string> readZones() {
  vector<string> zones;
  string line;
  while (getline(cin, line)) {
		ltrim(line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;
    zones.push_back(line);
  }

  return zones;
}

/** Sort the TestItems according to epochSeconds. */
void sortTestData(TestData& testData) {
  for (auto& p : testData) {
    sort(p.second.begin(), p.second.end(),
      [](const TestItem& a, const TestItem& b) {
        return a.epochSeconds < b.epochSeconds;
      }
    );
  }
}

/**
 * Generate the validation_data.json file. Adopted from TestDataGenerator.java.
 */
void printJson(const TestData& testData) {
  FILE* fp = fopen(VALIDATION_DATA_JSON, "w");

  string indentUnit = "  ";
  fprintf(fp, "{\n");
  string indent0 = indentUnit;
  fprintf(fp, "%s\"start_year\": %d,\n", indent0.c_str(), startYear);
  fprintf(fp, "%s\"until_year\": %d,\n", indent0.c_str(), untilYear);
  fprintf(fp, "%s\"source\": \"Hinnant Date\",\n", indent0.c_str());
  fprintf(fp, "%s\"version\": \"%s\",\n",
      indent0.c_str(), date::get_tzdb().version.c_str());
  fprintf(fp, "%s\"has_abbrev\": true,\n", indent0.c_str());
  fprintf(fp, "%s\"has_valid_dst\": true,\n", indent0.c_str());
  fprintf(fp, "%s\"test_data\": {\n", indent0.c_str());

  // Print each zone
  int zoneCount = 1;
  int numZones = testData.size();
  for (const auto& zoneEntry : testData) {
    string indent1 = indent0 + indentUnit;
    string zoneName = zoneEntry.first;
    fprintf(fp, "%s\"%s\": [\n", indent1.c_str(), zoneName.c_str());

    // Print each testItem
    int itemCount = 1;
    const vector<TestItem>& items = zoneEntry.second;
    for (const TestItem& item : items) {
      string indent2 = indent1 + indentUnit;
      fprintf(fp, "%s{\n", indent2.c_str());
      {
        string indent3 = indent2 + indentUnit;
        fprintf(fp, "%s\"epoch\": %ld,\n", indent3.c_str(), item.epochSeconds);
        fprintf(fp, "%s\"total_offset\": %d,\n",
            indent3.c_str(), item.utcOffset);
        fprintf(fp, "%s\"dst_offset\": %d,\n", indent3.c_str(), item.dstOffset);
        fprintf(fp, "%s\"y\": %d,\n", indent3.c_str(), item.year);
        fprintf(fp, "%s\"M\": %d,\n", indent3.c_str(), item.month);
        fprintf(fp, "%s\"d\": %d,\n", indent3.c_str(), item.day);
        fprintf(fp, "%s\"h\": %d,\n", indent3.c_str(), item.hour);
        fprintf(fp, "%s\"m\": %d,\n", indent3.c_str(), item.minute);
        fprintf(fp, "%s\"s\": %d,\n", indent3.c_str(), item.second);
        fprintf(fp, "%s\"abbrev\": \"%s\",\n",
            indent3.c_str(), item.abbrev.c_str());
        fprintf(fp, "%s\"type\": \"%c\"\n", indent3.c_str(), item.type);
      }
      fprintf(fp, "%s}%s\n", indent2.c_str(),
          (itemCount < (int)items.size()) ? "," : "");
      itemCount++;
    }

    fprintf(fp, "%s]%s\n", indent1.c_str(), (zoneCount < numZones) ? "," : "");
    zoneCount++;
  }

  fprintf(fp, "%s}\n", indent0.c_str());
  fprintf(fp, "}\n");

  fclose(fp);

  fprintf(stderr, "Created %s\n", VALIDATION_DATA_JSON);
}

void usageAndExit() {
  fprintf(stderr,
    "Usage: test_data_generator --scope (basic | extended)\n"
    "   --tz_version {version} [--db_namespace db]\n"
    "   [--start_year start] [--until_year until]\n"
    "   < zones.txt\n");
  exit(1);
}

#define SHIFT(argc, argv) do { argc--; argv++; } while(0)
#define ARG_EQUALS(s, t) (strcmp(s, t) == 0)

int main(int argc, const char* const* argv) {
  // Parse command line flags.
  string start = "2000";
  string until = "2050";
  string tzVersion = "";

  SHIFT(argc, argv);
  while (argc > 0) {
    if (ARG_EQUALS(argv[0], "--scope")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      scope = argv[0];
    } else if (ARG_EQUALS(argv[0], "--start_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      start = argv[0];
    } else if (ARG_EQUALS(argv[0], "--until_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      until = argv[0];
    } else if (ARG_EQUALS(argv[0], "--tz_version")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      tzVersion = argv[0];
    } else if (ARG_EQUALS(argv[0], "--")) {
      SHIFT(argc, argv);
      break;
    } else if (strncmp(argv[0], "-", 1) == 0) {
      fprintf(stderr, "Unknonwn flag '%s'\n", argv[0]);
      usageAndExit();
    } else {
      break;
    }
    SHIFT(argc, argv);
  }
  if (scope != "basic" && scope != "extended") {
    fprintf(stderr, "Unknown --scope '%s'\n", scope.c_str());
    usageAndExit();
  }
  if (tzVersion.empty()) {
    fprintf(stderr, "Must give --tz_version flag'\n");
    usageAndExit();
  }

  startYear = atoi(start.c_str());
  untilYear = atoi(until.c_str());

  // Load the TZ Database at a specific version. See
  // https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#thoughts-on-reloading-the-iana-tzdb-for-long-running-programs
  // and https://howardhinnant.github.io/date/tz.html#database.
  if (! remote_download(tzVersion)) {
    fprintf(stderr, "Failed to download TZ Version %s\n", tzVersion.c_str());
    exit(1);
  }
  if (! remote_install(tzVersion)) {
    fprintf(stderr, "Failed to install TZ Version %s\n", tzVersion.c_str());
    exit(1);
  }
  reload_tzdb();
  fprintf(stderr, "Loaded TZ Version %s\n", tzVersion.c_str());

  // Process the zones on the STDIN
  vector<string> zones = readZones();
  TestData testData = processZones(zones);
  sortTestData(testData);
  printJson(testData);
  return 0;
}
