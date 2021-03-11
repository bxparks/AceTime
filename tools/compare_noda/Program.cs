/*
 * Copyright 2021 Brian T. Park
 *
 * MIT License
 */

using System;
using System.Collections.Generic;
using System.IO;
using NodaTime;
using NodaTime.Extensions;
using NodaTime.Text;
using NodaTime.TimeZones;

namespace compare_noda
{
    class Program
    {
        // Usage:
        // $ dotnet run -- [--help] [--start_year start] [--until_year until]
        //      < zones.txt
        //      > validation_data.json
        //
        // Based on the equivalent Java program 'compare_java'.
        //
        static void Main(string[] args)
        {
            // Parse command line flags
            int argc = args.Length;
            int argi = 0;
            /*
            if (argc == 0) {
                UsageAndExit(1);
            }
            */
            string start = "2000";
            string until = "2050";
            while (argc > 0)
            {
                string arg0 = args[argi];
                if ("--start_year".Equals(arg0))
                {
                    {argc--; argi++; arg0 = args[argi];} // shift-left
                    start = arg0;
                }
                else if ("--until_year".Equals(arg0))
                {
                    {argc--; argi++; arg0 = args[argi];} // shift-left
                    until = arg0;
                }
                else if ("--help".Equals(arg0))
                {
                    UsageAndExit(0);
                    break;
                }
                else if ("--".Equals(arg0))
                {
                    break;
                }
                else if (arg0.StartsWith("-"))
                {
                    Console.Error.WriteLine($"Unknown flag '{arg0}'");
                    UsageAndExit(1);
                }
                else if (! arg0.StartsWith("-"))
                {
                    break;
                }
                {argc--; argi++;} // shift-left
            }

            int startYear = int.Parse(start);
            int untilYear = int.Parse(until);

            List<string> zones = ReadZones();
            GenerateData generator = new GenerateData(startYear, untilYear);
            IDictionary<string, List<TestItem>> testData = generator.CreateTestData(zones);
            generator.PrintJson(testData);
        }

        private static void UsageAndExit(int exitCode)
        {
            string usage = "Usage: compare_noda [--start_year {year}] "
                + "[--until_year {year}] < zones.txt";
            if (exitCode == 0)
            {
                Console.WriteLine(usage);
            } else {
                Console.Error.WriteLine(usage);
            }
            Environment.Exit(exitCode);
        }

        private static List<string> ReadZones()
        {
            var zones = new List<string>();
            string line;
            while ((line = Console.ReadLine()) != null)
            {
                line = line.Trim();
                if (String.IsNullOrEmpty(line)) continue;
                if (line.StartsWith('#')) continue;
                zones.Add(line);
            }
            return zones;
        }
    }

    class GenerateData
    {
        private const int SECONDS_SINCE_UNIX_EPOCH = 946684800;
        private const string indentUnit = "  ";
        private const string jsonFile = "validation_data.json";

        public GenerateData(int startYear, int untilYear)
        {
            this.startYear = startYear;
            this.untilYear = untilYear;
        }

        public IDictionary<string, List<TestItem>> CreateTestData(List<string> zones)
        {
            var testData = new SortedDictionary<string, List<TestItem>>();
            foreach (string zone in zones)
            {
                List<TestItem> testItems = CreateValidationData(zone);
                testData.Add(zone, testItems);
            }
            return testData;
        }

        private List<TestItem> CreateValidationData(string zone)
        {
            DateTimeZone tz = DateTimeZoneProviders.Tzdb[zone];
            var startInstant = new LocalDateTime(startYear, 1, 1, 0, 0)
                .InZoneLeniently(tz).ToInstant();
            var untilInstant = new LocalDateTime(untilYear, 1, 1, 0, 0)
                .InZoneLeniently(tz).ToInstant();

            var testItems = new SortedDictionary<int, TestItem>();
            AddTestItemsFromZoneIntervals(testItems, tz, startInstant, untilInstant);
            AddTestItemsFromSampling(testItems, tz, startInstant, untilInstant);

            var items = new List<TestItem>();
            items.AddRange(testItems.Values);
            return items;
        }

        private void AddTestItemsFromZoneIntervals(
            IDictionary<int, TestItem> testItems,
            DateTimeZone tz,
            Instant startInstant,
            Instant untilInstant)
        {
            var intervals = tz.GetZoneIntervals(startInstant, untilInstant);
            foreach (ZoneInterval zi in intervals)
            {
                AddZoneInterval(testItems, tz, zi);
            }
        }

        private void AddZoneInterval(
            IDictionary<int, TestItem> testItems,
            DateTimeZone tz,
            ZoneInterval zi)
        {
            if (zi.HasStart)
            {
                var isoStart = zi.IsoLocalStart;
                if (isoStart.Year > startYear)
                {
                    // A: One minute before the transition
                    // B: Right after the DST transition.
                    Instant after = zi.Start;
                    Duration oneMinute = Duration.FromMinutes(1);
                    Instant before = after - oneMinute;

                    AddTestItem(testItems, tz, before, 'A');
                    AddTestItem(testItems, tz, after, 'B');
                }
            }
        }

        private static void AddTestItem(
            IDictionary<int, TestItem> testItems,
            DateTimeZone tz,
            Instant instant,
            char type)
        {
            TestItem testItem = CreateTestItem(tz, instant, type);
            if (testItems.ContainsKey(testItem.epochSeconds)) return;
            testItems.Add(testItem.epochSeconds, testItem);
        }

        private static TestItem CreateTestItem(DateTimeZone tz, Instant instant, char type)
        {
            ZoneInterval zi = tz.GetZoneInterval(instant);
            ZonedDateTime zdt = instant.InZone(tz);

            var testItem = new TestItem();
            testItem.epochSeconds = ToAceTimeEpochSeconds(instant.ToUnixTimeSeconds());
            testItem.utcOffset = zi.WallOffset.Seconds;
            testItem.dstOffset = zi.Savings.Seconds;
            testItem.year = zdt.Year;
            testItem.month = zdt.Month;
            testItem.day = zdt.Day;
            testItem.hour = zdt.Hour;
            testItem.minute = zdt.Minute;
            testItem.second = zdt.Second;
            testItem.abbrev = zi.Name;
            testItem.type = type;
            return testItem;
        }

        private void AddTestItemsFromSampling(
            IDictionary<int, TestItem> testItems,
            DateTimeZone tz,
            Instant startInstant,
            Instant untilInstant)
        {
            ZonedDateTime startDt = startInstant.InZone(tz);
            ZonedDateTime untilDt = untilInstant.InZone(tz);

            for (int year = startDt.Year; year < untilDt.Year; year++)
            {
                // Add the 1st of every month of every year.
                for (int month = 1; month <= 12; month++)
                {
                    ZonedDateTime zdt = new LocalDateTime(year, month, 1, 0, 0).InZoneLeniently(tz);
                    AddTestItem(testItems, tz, zdt.ToInstant(), 'S');
                }

                // Add the last day and hour of the year
                ZonedDateTime lastdt = new LocalDateTime(year, 12, 31, 23, 0).InZoneLeniently(tz);
                AddTestItem(testItems, tz, lastdt.ToInstant(), 'Y');
            }
        }

        // Serialize to JSON manually, for 2 reasons:
        // a) to reduce external dependencies,
        // b) to follow the Java code.
        public void PrintJson(IDictionary<string, List<TestItem>> testData)
        {
            string tzVersion = DateTimeZoneProviders.Tzdb.VersionId;

            Console.WriteLine("{");
            string indent0 = indentUnit;
            Console.WriteLine($"{indent0}\"start_year\": {startYear},");
            Console.WriteLine($"{indent0}\"until_year\": {untilYear},");
            Console.WriteLine($"{indent0}\"source\": \"NodaTime\",");
            Console.WriteLine($"{indent0}\"version\": \"3.0\",");
            Console.WriteLine($"{indent0}\"tz_version\": \"{tzVersion}\",");
            Console.WriteLine($"{indent0}\"has_valid_abbrev\": true,");
            Console.WriteLine($"{indent0}\"has_valid_dst\": true,");
            Console.WriteLine($"{indent0}\"test_data\": {{");

            int zoneCount = 1;
            int numZones = testData.Count;
            foreach (KeyValuePair<string, List<TestItem>> entry in testData)
            {
                var items = entry.Value;
                if (items == null)
                {
                    zoneCount++;
                    continue;
                }

                // Print the zone name
                string indent1 = indent0 + indentUnit;
                Console.WriteLine($"{indent1}\"{entry.Key}\": [");

                // Prin the testItems
                int itemCount = 1;
                foreach (TestItem item in items)
                {
                    string indent2 = indent1 + indentUnit;
                    Console.WriteLine($"{indent2}{{");
                    {
                        string indent3 = indent2 + indentUnit;
                        Console.WriteLine($"{indent3}\"epoch\": {item.epochSeconds},");
                        Console.WriteLine($"{indent3}\"total_offset\": {item.utcOffset},");
                        Console.WriteLine($"{indent3}\"dst_offset\": {item.dstOffset},");
                        Console.WriteLine($"{indent3}\"y\": {item.year},");
                        Console.WriteLine($"{indent3}\"M\": {item.month},");
                        Console.WriteLine($"{indent3}\"d\": {item.day},");
                        Console.WriteLine($"{indent3}\"h\": {item.hour},");
                        Console.WriteLine($"{indent3}\"m\": {item.minute},");
                        Console.WriteLine($"{indent3}\"s\": {item.second},");
                        Console.WriteLine($"{indent3}\"abbrev\": \"{item.abbrev}\",");
                        Console.WriteLine($"{indent3}\"type\": \"{item.type}\"");
                    }
                    string innerComma = itemCount < items.Count ? "," : "";
                    Console.WriteLine($"{indent2}}}{innerComma}");
                    itemCount++;
                }

                string outerComma = zoneCount < numZones ? "," : "";
                Console.WriteLine($"{indent1}]{outerComma}");
                zoneCount++;
            }

            Console.WriteLine($"{indent0}}}");
            Console.WriteLine("}");
        }

        private static int ToAceTimeEpochSeconds(long unixEpochSeconds)
        {
            return (int) (unixEpochSeconds - SECONDS_SINCE_UNIX_EPOCH);
        }

        private readonly int startYear;
        private readonly int untilYear;
    }

    struct TestItem
    {
        internal int epochSeconds; // seconds from AceTime epoch (2000-01-01T00:00:00Z)
        internal int utcOffset; // total UTC offset in seconds
        internal int dstOffset; // DST shift from standard offset in seconds
        internal int year;
        internal int month;
        internal int day;
        internal int hour;
        internal int minute;
        internal int second;
        internal string abbrev;
        internal char type;
    }
}
