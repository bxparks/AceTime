/*
 * Generate the validation_data.* files for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using Howard Hinnant's date.h
 * and tz.h library.
 *
 * Usage:
 * $ ./test_data_generator.out \
 *    --scope (basic | extended) \
 *    --db_namespace (db) \
 *    [--start_year start] \
 *    [--until_year until] < zones.txt
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

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
const long SECONDS_SINCE_UNIX_EPOCH = 946684800;

const char VALIDATION_DATA_CPP[] = "validation_data.cpp";
const char VALIDATION_DATA_H[] = "validation_data.h";
const char VALIDATION_TESTS_CPP[] = "validation_tests.cpp";

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
  int utcOffset; // minutes
  int dstOffset; // minutes
  DateTime dateTime;
  char type; //'A', 'B', 'S', 'T' or 'Y'
};

// Command line arguments
string scope = "";
string dbNamespace = "";
bool isCustomDbNamespace;
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
 */
TestItem toTestItem(const time_zone& tz, sys_seconds st, char type) {
  sys_info info = tz.get_info(st);
  seconds unixSeconds = floor<seconds>(st.time_since_epoch());
  zoned_time<seconds> zdt = make_zoned(&tz, st);
  local_time<seconds> lt = zdt.get_local_time();
  DateTime dateTime = toDateTime(lt);
  return TestItem{
      unixSeconds.count() - SECONDS_SINCE_UNIX_EPOCH,
      (int)info.offset.count() / 60,
      (int)info.save.count(),
      dateTime,
      type
  };
}

void addTestItem(map<string, vector<TestItem>>& testData,
    const string& zoneName, const TestItem& item) {
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
void addTransitions(map<string, vector<TestItem>>& testData,
    const time_zone& tz, const string& zoneName, int startYear, int untilYear) {
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
void addMonthlySamples(map<string, vector<TestItem>>& testData,
    const time_zone& tz, const string& zoneName, int startYear, int untilYear) {

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
void processZone(map<string, vector<TestItem>>& testData,
    const string& zoneName, int startYear, int untilYear) {
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
  map<string, vector<TestItem>> testData;
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

/**
 * Replace all occurences of 'from' string to 'to' string.
 * See https://stackoverflow.com/questions/2896600.
 */
void replaceAll(string& source, const string& from, const string& to) {
	string newString;
	newString.reserve(source.length());  // avoids a few memory allocations

	string::size_type lastPos = 0;
	string::size_type findPos;

	while(string::npos != (findPos = source.find(from, lastPos))) {
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
	}

	// Care for the rest after last occurrence
	newString += source.substr(lastPos);

	source.swap(newString);
}

/**
 * Convert "America/Los_Angeles" into an identifier that can be used in a C/C++
 * variable, is "America_Los_Angeles". Uses the same algorithm as the
 * argenerator.py script.
 */
string normalizeName(const string& name) {
  string tmp = name;
  replaceAll(tmp, "+", "_PLUS_");
	replaceAll(tmp, "-", "_");
  return regex_replace(tmp, regex("[^0-9a-zA-Z]"), "_");
}

/** Sort the TestItems according to epochSeconds. */
void sortTestData(map<string, vector<TestItem>>& testData) {
  for (auto& p : testData) {
    sort(p.second.begin(), p.second.end(),
      [](const TestItem& a, const TestItem& b) {
        return a.epochSeconds < b.epochSeconds;
      }
    );
  }
}

void printTestItem(FILE* fp, const TestItem& item) {
  fprintf(fp,
      "  { %10ld, %4d, %4d, %4d, %2u, %2u, %2d, %2d, %2d }, // type=%c\n",
      item.epochSeconds,
      item.utcOffset,
      item.dstOffset,
      item.dateTime.year,
      item.dateTime.month,
      item.dateTime.day,
      item.dateTime.hour,
      item.dateTime.minute,
      item.dateTime.second,
      item.type);
}

/** Generate the validation_data.cpp file for timezone tz. */
void printDataCpp(const map<string, vector<TestItem>>& testData) {
  FILE* fp = fopen(VALIDATION_DATA_CPP, "w");

  fprintf(fp,
      "// This is an auto-generated file using the Hinnant Date Library.\n");
  fprintf(fp, "// DO NOT EDIT\n");
  fprintf(fp, "\n");
  fprintf(fp, "#include <AceTime.h>\n");
  if (isCustomDbNamespace) {
    fprintf(fp, "#include \"%s/zone_infos.h\"\n", dbNamespace.c_str());
    fprintf(fp, "#include \"%s/zone_policies.h\"\n", dbNamespace.c_str());
  }
  fprintf(fp, "#include \"validation_data.h\"\n");
  fprintf(fp, "\n");
  fprintf(fp, "namespace ace_time {\n");
  fprintf(fp, "namespace %s {\n", dbNamespace.c_str());
  fprintf(fp, "\n");

  for (const auto& p : testData) {
    const string& zoneName = p.first;
    const string normalizedName = normalizeName(zoneName);
    const vector<TestItem>& testItems = p.second;

    fprintf(fp,
    "//--------------------------------------------------------------------\n");
    fprintf(fp, "// Zone name: %s\n", zoneName.c_str());
    fprintf(fp,
    "//--------------------------------------------------------------------\n");
    fprintf(fp, "\n");

    fprintf(fp, "static const ValidationItem kValidationItems%s[] = {\n",
        normalizedName.c_str());
    fprintf(fp, "  //     epoch,  utc,  dst,    y,  m,  d,  h,  m,  s\n");
    for (const TestItem& item : testItems) {
      printTestItem(fp, item);
    }
    fprintf(fp, "};\n");
    fprintf(fp, "\n");

    fprintf(fp, "const ValidationData kValidationData%s = {\n",
        normalizedName.c_str());
    fprintf(fp, "  &kZone%s /*zoneInfo*/,\n", normalizedName.c_str());
    fprintf(fp,
      "  sizeof(kValidationItems%s)/sizeof(ValidationItem) /*numItems*/\n",
      normalizedName.c_str());
    fprintf(fp, "  kValidationItems%s /*items*/,\n", normalizedName.c_str());
    fprintf(fp, "};\n");
    fprintf(fp, "\n");
  }
  fprintf(fp, "}\n");
  fprintf(fp, "}\n");

  fclose(fp);
}

/** Create validation_data.h file. */
void printDataHeader(const map<string, vector<TestItem>>& testData) {
  FILE* fp = fopen(VALIDATION_DATA_H, "w");

  fprintf(fp,
      "// This is an auto-generated file using the Hinnant Date Library.\n");
  fprintf(fp, "// DO NOT EDIT\n");
  fprintf(fp, "\n");
  fprintf(fp, "#ifndef ACE_TIME_VALIDATION_TEST_VALIDATION_DATA_H\n");
  fprintf(fp, "#define ACE_TIME_VALIDATION_TEST_VALIDATION_DATA_H\n");
  fprintf(fp, "\n");
  fprintf(fp, "#include \"ValidationDataType.h\"\n");
  fprintf(fp, "\n");
  fprintf(fp, "namespace ace_time {\n");
  fprintf(fp, "namespace %s {\n", dbNamespace.c_str());
  fprintf(fp, "\n");

  for (const auto& p : testData) {
    const string& zoneName = p.first;
    const string normalizedName = normalizeName(zoneName);
    fprintf(fp, "extern const ValidationData kValidationData%s;\n",
      normalizedName.c_str());
  }

  fprintf(fp, "\n");
  fprintf(fp, "#endif\n");
  fprintf(fp, "\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n");

  fclose(fp);
}

/** Create validation_tests.cpp file. */
void printTestsCpp(const map<string, vector<TestItem>>& testData) {
  FILE* fp = fopen(VALIDATION_TESTS_CPP, "w");

  fprintf(fp,
      "// This is an auto-generated file using the Hinnant Date Library.\n");
  fprintf(fp, "// DO NOT EDIT\n");
  fprintf(fp, "\n");

  fprintf(fp, "#include <AUnit.h>\n");
  fprintf(fp, "#include \"TransitionTest.h\"\n");
  fprintf(fp, "#include \"validation_data.h\"\n");
  fprintf(fp, "\n");

  for (const auto& p : testData) {
    const string& zoneName = p.first;
    const string normalizedName = normalizeName(zoneName);
    fprintf(fp, "testF(TransitionTest, %s) {\n", normalizedName.c_str());
    fprintf(fp, "  assertValid(&ace_time::%s::kValidationData%s\n",
        dbNamespace.c_str(),
        normalizedName.c_str());
    fprintf(fp, "}\n");
  }

  fclose(fp);
}

void usageAndExit() {
  fprintf(stderr,
    "Usage: test_data_generator --scope (basic | extended) --db_namespace db\n"
    "   [--start_year start] [--until_year until] < zones.txt\n");
  exit(1);
}

#define SHIFT(argc, argv) do { argc--; argv++; } while(0)
#define ARG_EQUALS(s, t) (strcmp(s, t) == 0)

int main(int argc, const char* const* argv) {
  // Parse command line flags.
  string start = "2000";
  string until = "2050";
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
    } else if (ARG_EQUALS(argv[0], "--db_namespace")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      dbNamespace = argv[0];
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
  if (dbNamespace.empty()) {
    fprintf(stderr, "Must give --db_namespace {db} flagn\n");
    usageAndExit();
  }
  isCustomDbNamespace = (dbNamespace != "zonedb" && dbNamespace != "zonedbx");

  startYear = atoi(start.c_str());
  untilYear = atoi(until.c_str());

  // Process the zones on the STDIN
  vector<string> zones = readZones();
  map<string, vector<TestItem>> testData = processZones(zones);
  sortTestData(testData);
  printDataCpp(testData);
  printDataHeader(testData);
  printTestsCpp(testData);

  fprintf(stderr, "Created %s\n", VALIDATION_DATA_CPP);
  fprintf(stderr, "Created %s\n", VALIDATION_DATA_H);
  fprintf(stderr, "Created %s\n", VALIDATION_TESTS_CPP);
  return 0;
}
