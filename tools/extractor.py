#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Parses the zone info files in the TZ Database, and produces the various .cpp and
.h files needed for the AceTime library. The zone files which containa valid
"Rule" are:

    africa
    antarctica
    asia
    australasia
    backzone
    europe
    northamerica
    southamerica
    systemv

Of these, the following are not relevant:

    backzone - contains zones differing before 1970
    systemv - 'SystemV' zone

Rule entries have the following columns:
# Rule  NAME    FROM    TO    TYPE IN   ON      AT      SAVE    LETTER
Rule    US      2007    max   -    Mar  Sun>=8  2:00    1:00    D
Rule    US      2007    max   -    Nov  Sun>=1  2:00    0       S

Zone entries have the following columns:
# Zone  NAME                GMTOFF      RULES   FORMAT  [UNTIL]
Zone    America/Chicago     -5:50:36    -       LMT     1883 Nov 18 12:09:24
                            -6:00       US      C%sT    1920

Usage:
    cat africa antarctica asia australasia europe northamerica southamerica \
        | ./extractor.py {flags}
"""

import argparse
import logging
import sys


class Extractor:
    """Reads each test data section from the given file-like object (e.g.
    sys.stdin).

    Usage:

        extractor = Extractor(invocation, sys.stdin, cpp_file, header_file)
        extractor.parse_zone_file()
        extractor.process_rules()
        extractor.process_zones()

        extractor.print_rules()
        extractor.print_zones()
        extractor.print_summary()
    """

    def __init__(self, invocation, input, **kwargs):
        self.invocation = invocation
        self.input = input

        self.next_line = None
        self.rule_lines = {}  # dictionary of ruleName to lines[]
        self.zone_lines = {}  # dictionary of zoneName to lines[]
        self.link_lines = {}  # dictionary of linkName to lines[]
        self.rules = {}
        self.zones = {}
        self.ignored_rule_lines = 0
        self.ignored_zone_lines = 0
        self.invalid_rule_lines = 0
        self.invalid_zone_lines = 0

    def parse_zone_file(self):
        in_zone_mode = False
        prev_tag = ''
        prev_name = ''
        while True:
            line = self.read_line()
            if line is None:
                break

            tag = line[:4]
            if tag == 'Rule':
                tokens = line.split()
                add_item(self.rule_lines, tokens[1], line)
                in_zone_mode = False
            elif tag == 'Link':
                tokens = line.split()
                add_item(self.link_lines, tokens[1], line)
                in_zone_mode = False
            elif tag == 'Zone':
                tokens = line.split()
                add_item(self.zone_lines, tokens[1], ' '.join(tokens[2:]))
                in_zone_mode = True
                prev_tag = tag
                prev_name = tokens[1]
            elif tag[0] == '\t' and in_zone_mode:
                add_item(self.zone_lines, prev_name, line)

    def process_rules(self):
        for name, lines in self.rule_lines.items():
            for line in lines:
                try:
                    rule_entry = process_rule_line(line)
                    if rule_entry:
                        add_item(self.rules, name, rule_entry)
                    else:
                        self.ignored_rule_lines += 1
                except Exception as e:
                    logging.exception('Exception %s: %s', e, line)
                    self.invalid_rule_lines += 1

    def process_zones(self):
        for name, lines in self.zone_lines.items():
            for line in lines:
                try:
                    zone_entry = process_zone_line(line)
                    if zone_entry:
                        add_item(self.zones, name, zone_entry)
                    else:
                        self.ignored_zone_lines += 1
                except Exception as e:
                    logging.exception('Exception %s: %s', e, line)
                    self.invalid_zone_lines += 1

    def print_summary(self):
        rule_entry_count = 0
        for name, rules in self.rules.items():
            for rule in rules:
                rule_entry_count += 1

        zone_entry_count = 0
        for name, zones in self.zones.items():
            for zone in zones:
                zone_entry_count += 1

        print('Rule lines count: %s' % len(self.rule_lines))
        print('Zone lines count: %s' % len(self.zone_lines))
        print('Link lines count: %s' % len(self.link_lines))
        print('Rules name count: %s' % len(self.rules))
        print('Zones name count: %s' % len(self.zones))
        print('Rule entry count: %s' % rule_entry_count)
        print('Zone entry count: %s' % zone_entry_count)
        print('Ignored Rule lines: %s' % self.ignored_rule_lines)
        print('Ignored Zone lines: %s' % self.ignored_zone_lines)
        print('Invalid Rule lines: %s' % self.invalid_rule_lines)
        print('Invalid Zone lines: %s' % self.invalid_zone_lines)

    def print_rules(self):
        for name, rules in self.rules.items():
            print('Rule name %s' % name)
            for rule in rules:
                print(rule)

    def print_invalid_rules(self):
        for name, rules in self.rules.items():
            print('Rule name %s' % name)
            for rule in rules:
                if rule['from_year'] < 2000 and rule['to_year'] < 2000:
                    print('from or to year < 2000: %s' % rule)
                if len(rule['letter']) > 1:
                    print('LETTER (%s) more than 1 char: %s' % line)

    def print_rules_long_dst_letter(self):
        """Print rules with multiple characters in the DST 'letter' field.
        """
        for name, rules in self.rules.items():
            name_printed = False
            for rule in rules:
                if len(rule['letter']) > 1:
                    if not name_printed:
                        print('Rule name %s' % name)
                        name_printed = True
                    print(rule)

    def print_zones(self):
        for name, zones in self.zones.items():
            print('Zone name %s' % name)
            for zone in zones:
                print(zone)

    def print_zones_short_name(self):
        """Print the last component in the "a/b/c" zone names.
        """
        for name, zones in self.zones.items():
            index = name.rfind('/')
            if index >= 0:
                short_name = name[index + 1:]
            else:
                short_name = name
            print(short_name)

    def print_zones_with_until_month(self):
        """Print the zones which have months in the 'UNTIL' field.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                if zone['until_month']:
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_without_rules(self):
        """Print zones whose RULES column is "-" which indicates NO rules.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                rule_name = zone['rules']
                if rule_name == '-':
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_with_offset_as_rules(self):
        """Print zones which point to a DST offset in its RULES column.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                rule_name = zone['rules']
                if rule_name.isdigit():
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_with_unknown_rules(self):
        """Print zones whose RULES is a reference that cannot be found.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                rule_name = zone['rules']
                if rule_name != '-' and not rule_name.isdigit() \
                        and rule_name not in self.rules:
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def read_line(self):
        """Return the next line, while supporting a one-line push_back().
        Comment lines begin with a '#' character and are skipped.
        Blank lines are skipped.
        Prepending and trailing whitespaces are stripped.
        Return 'None' if EOF is reached.
        """
        if self.next_line:
            line = self.next_line
            self.next_line = None
            return line

        while True:
            line = self.input.readline()

            # EOF
            if line == '':
                return None

            # remove trailing comments
            i = line.find('#')
            if i >= 0:
                line = line[:i]

            # strip any trailing whitespaces
            line = line.rstrip()

            # skip any blank lines after stripping
            if not line:
                continue

            return line


def add_item(table, name, line):
    array = table.get(name)
    if not array:
        array = []
        table[name] = array
    array.append(line)


MONTH_TO_MONTH_INDEX = {
    'Jan': 1,
    'Feb': 2,
    'Mar': 3,
    'Apr': 4,
    'May': 5,
    'Jun': 6,
    'Jul': 7,
    'Aug': 8,
    'Sep': 9,
    'Oct': 10,
    'Nov': 11,
    'Dec': 12
}

WEEK_TO_WEEK_INDEX = {
    'Mon': 1,
    'Tue': 2,
    'Wed': 3,
    'Thu': 4,
    'Fri': 5,
    'Sat': 6,
    'Sun': 7,
}


def process_rule_line(line):
    """ Normalize a dictionary that represents a 'Rule' line from the TZ
    database. Contains the following fields:
    Rule NAME FROM TO TYPE IN ON AT SAVE LETTER
    0    1    2    3  4    5  6  7  8    9

    These represent transitions from Daylight to/from Standard. If the 'from'
    and 'to' entries are before 2000, then the last transition remains in
    effect, so we need to capture all transitions in the database, even the ones
    before year 2000.

    Returns a dictionary with the following fields:
        from: from year (int)
        to: to year (int), 9999=max
        in: month index (1-12)
        on_day_of_week: 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
        on_day_of_month: (1-31), 0={last dayOfWeek match}
        at_hour: hour to transition to and from DST
        at_modifier: 's', 'w', 'g', 'u', 'z'
        delta_minutes: offset code from Standard time (int)
        letter: 'D', 'S', '-'
    """
    tokens = line.split()

    # Check for valid year.
    from_year = int(tokens[2])
    to_year_string = tokens[3]
    if to_year_string == 'only':
        to_year = from_year
    elif to_year_string == 'max':
        to_year = 9999
    else:
        to_year = int(to_year_string)

    in_month = MONTH_TO_MONTH_INDEX[tokens[5]]
    (on_day_of_week, on_day_of_month) = parse_on_day_string(tokens[6])
    (at_hour, at_modifier) = parse_at_hour_string(tokens[7])
    delta_minutes = hour_string_to_offset_minutes(tokens[8])

    return {
        'from': from_year,
        'to': to_year,
        'in_month': in_month,
        'on_day_of_week': on_day_of_week,
        'on_day_of_month': on_day_of_month,
        'at_hour': at_hour,
        'at_modifier': at_modifier,
        'delta_minutes': delta_minutes,
        'letter': tokens[9],
    }


def parse_on_day_string(on_string):
    """ Parse things like "Mon>=1", "lastTue", "20".
    Returns (on_day_of_week, on_day_of_month) where
        (0, dayOfMonth) = exact match on dayOfMonth
        (dayOfWeek, dayOfMonth) = matches dayOfWeek>=dayOfMonth
        (dayOfWeek, 0) = matches lastDayOfWeek
    """
    if on_string.isdigit():
        return (0, int(on_string))

    if on_string[:4] == 'last':
        dayOfWeek = on_string[4:]
        return (WEEK_TO_WEEK_INDEX[dayOfWeek], 0)

    greater_than_equal_index = on_string.find('>=')
    if greater_than_equal_index >= 0:
        dayOfWeek = on_string[:greater_than_equal_index]
        dayOfMonth = on_string[greater_than_equal_index + 2:]
        return (WEEK_TO_WEEK_INDEX[dayOfWeek], int(dayOfMonth))

    raise Exception('Unable to parse ON string %s' % on_string)


def parse_at_hour_string(at_string):
    modifier = at_string[-1:]
    if modifier.isdigit():
        modifier = 'w'
        at_hour = at_string
    else:
        at_hour = at_string[:-1]
    if modifier not in ['w', 's', 'u', 'g', 'z']:
        raise Exception('Invalid at_hour modifier (%s)' % modifier)
    return (at_hour, modifier)


def process_zone_line(line):
    """Normalize an entry from dictionary that represents one line of
    a 'Zone' record. The columns are:
    GMTOFF	 RULES	FORMAT	[UNTIL]
    0        1      2       3
    -5:50:36 -      LMT     1883 Nov 18 12:09:24
    -6:00    US     C%sT    1920

    Return a dictionary with:

        offset_minutes: offset from UTC/GMT in minutes (int)
        rules: name of the Rule in effect, '-', or minute offset (string)
        abbrev: abbreviation with '%s' replaced with '%'
                (e.g. P%sT -> P%T, E%ST -> E%T, GMT/BST, SAST)
        until_year: 2000-9999 (int)
        until_month: 1-12 (int) optional
        until_day: (string) 1-31, lastSun, Sun>=3, etc
        until_time: (string) optional
    """
    tokens = line.split()

    # check 'until' year
    if len(tokens) >= 4:
        until_year = int(tokens[3])
    else:
        until_year = 9999
    if until_year < 2000:
        return None

    # check for additional components of 'UNTIL' field
    if len(tokens) >= 5:
        until_month = MONTH_TO_MONTH_INDEX[tokens[4]]
    else:
        until_month = None
    if len(tokens) >= 6:
        until_day = tokens[5]
    else:
        until_day = None
    if len(tokens) >= 7:
        until_time = tokens[6]
    else:
        until_time = None

    offset_minutes = hour_string_to_offset_minutes(tokens[0])
    abbrev = tokens[2].replace('%s', '%')

    # the 'RULES' field can be:
    #   * '-' no rules
    #   * a string reference to a rule
    #   * an offset like "01:00" (e.g. America/Argentina/San_Luis,
    #       Europe/Istanbul).
    rules_string = tokens[1]
    if rules_string.find(':') >= 0:
        rules_string = str(hour_string_to_offset_minutes(rules_string))

    # Create the zone entry
    return {
        'offset_minutes': offset_minutes,
        'rules': rules_string,
        'abbrev': abbrev,
        'until_year': until_year,
        'until_month': until_month,
        'until_day': until_day,
        'until_time': until_time,
    }


def hour_string_to_offset_minutes(hs):
    i = 0
    sign = 1
    if hs[i] == '-':
        sign = -1
        i += 1

    colon_index = hs.find(':')
    if colon_index < 0:
        hour_string = hs[i:]
        minute_string = '0'
    else:
        hour_string = hs[i:colon_index]
        minute_string = hs[colon_index + 1:]
    hour = int(hour_string)
    minute = int(minute_string)
    return sign * (hour * 60 + minute)


def main():
    """Read the test data chunks from the STDIN and print them out. The ability
    to run this from the command line is intended mostly for testing purposes.

    Usage:
        extractor.py < test_data.txt
    """
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Zone Info.')
    parser.add_argument(
        '--cpp_file', help='Name of the output .cpp file', required=False)
    parser.add_argument(
        '--header_file', help='Name of the output .h file', required=False)
    parser.add_argument(
        '--print_rules',
        help='Print list of rules',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_rules_long_dst_letter',
        help='Print rules with long DST letter',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones',
        help='Print list of zones',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_short_name',
        help='Print the short zone names',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_with_until_month',
        help='Print the Zones with "UNTIL" month fields',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_with_offset_as_rules',
        help='Print rules which contains a DST offset in the RULES column',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_without_rules',
        help='Print rules which contain "-" in the RULES column',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_with_unknown_rules',
        help='Print rules which contain a valid RULES that cannot be found',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_summary',
        help='Print summary of rules and zones',
        action="store_true",
        default=False)
    args = parser.parse_args()

    # How the script was invoked
    invocation = " ".join(sys.argv)

    extractor = Extractor(
        invocation,
        sys.stdin,
        cpp_file=args.cpp_file,
        header_file=args.header_file)

    extractor.parse_zone_file()
    extractor.process_rules()
    extractor.process_zones()
    if args.print_rules:
        extractor.print_rules()
    if args.print_zones:
        extractor.print_zones()
    if args.print_summary:
        extractor.print_summary()
    if args.print_rules_long_dst_letter:
        extractor.print_rules_long_dst_letter()
    if args.print_zones_short_name:
        extractor.print_zones_short_name()
    if args.print_zones_with_until_month:
        extractor.print_zones_with_until_month()
    if args.print_zones_with_offset_as_rules:
        extractor.print_zones_with_offset_as_rules()
    if args.print_zones_without_rules:
        extractor.print_zones_without_rules()
    if args.print_zones_with_unknown_rules:
        extractor.print_zones_with_unknown_rules()


if __name__ == '__main__':
    main()
