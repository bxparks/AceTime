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
"""

import argparse
import logging
import sys
import os


class Extractor:
    """Reads each test data section from the given file-like object (e.g.
    sys.stdin).

    Usage:

        extractor = Extractor(input_dir)
        extractor.parse_zone_files()
        extractor.process_rules()
        extractor.process_zones()
        extractor.print_summary()
		(zones, rules) = extractor.get_data()
        ...
    """

    ZONE_FILES = [
        'africa',
        'antarctica',
        'asia',
        'australasia',
        'europe',
        'northamerica',
        'southamerica',
    ]

    def __init__(self, input_dir, **kwargs):
        self.input_dir = input_dir

        self.next_line = None
        self.rule_lines = {}  # dictionary of ruleName to lines[]
        self.zone_lines = {}  # dictionary of zoneName to lines[]
        self.link_lines = {}  # dictionary of linkName to lines[]
        self.rules = {} # map of ruleName to ruleEntry[], RuleEntry = {}
        self.zones = {} # map fo ruleName to zoneEntry[], ZoneEntry = {}
        self.ignored_rule_lines = 0
        self.ignored_zone_lines = 0
        self.invalid_rule_lines = 0
        self.invalid_zone_lines = 0

    def get_data(self):
        return (self.zones, self.rules)

    def parse_zone_files(self):
        logging.basicConfig(level=logging.INFO)
        for file_name in self.ZONE_FILES:
            full_filename = os.path.join(self.input_dir, file_name)
            logging.info('Processing %s' % full_filename)
            with open(full_filename, 'r', encoding='utf-8') as f:
                self.parse_zone_file(f)

    def parse_zone_file(self, input):
        in_zone_mode = False
        prev_tag = ''
        prev_name = ''
        while True:
            line = self.read_line(input)
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

    def read_line(self, input):
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
            line = input.readline()

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
    """Normalize a dictionary that represents a 'Rule' line from the TZ
    database. Contains the following fields:
    Rule NAME FROM TO TYPE IN ON AT SAVE LETTER
    0    1    2    3  4    5  6  7  8    9

    These represent transitions from Daylight to/from Standard. If the 'from'
    and 'to' entries are before 2000, then the last transition remains in
    effect, so we need to capture all transitions in the database, even the ones
    before year 2000.

    Returns a dictionary with the following fields:
        fromYear: (int) from year
        toYear: (int) to year, 2000 to 9999=max
        inMonth: (int) month index (1-12)
        onDayOfWeek: (int) 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
        onDayOfMonth: (int) (1-31), 0={last dayOfWeek match}
        atHour: (string) the time when transition to and from DST happens
        atMinute: (int) version of atHour in units of 'minutes from 00:00'
        atHourModifier: (char) 's', 'w', 'g', 'u', 'z'
        deltaMinutes: (int) offset code from Standard time
        letter: (char) 'D', 'S', '-'
        rawLine: (string) the original RULE line from the TZ file
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
    (at_hour, at_hour_modifier) = parse_at_hour_string(tokens[7])
    at_minute = hour_string_to_offset_minutes(at_hour)
    delta_minutes = hour_string_to_offset_minutes(tokens[8])

    # Return map corresponding to a ZoneRule instance
    return {
        'fromYear': from_year,
        'toYear': to_year,
        'inMonth': in_month,
        'onDayOfWeek': on_day_of_week,
        'onDayOfMonth': on_day_of_month,
        'atHour': at_hour,
        'atMinute': at_minute,
        'atHourModifier': at_hour_modifier,
        'deltaMinutes': delta_minutes, # need conversion to deltaCode
        'letter': tokens[9],
        'rawLine': line,
    }


def parse_on_day_string(on_string):
    """Parse things like "Mon>=1", "lastTue", "20".
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
    """Parses the '2:00s' string into '2:00' and 's'.
    """
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

        offsetMinutes: (int) offset from UTC/GMT in minutes
        rules: (string) name of the Rule in effect, '-', or minute offset
        format: (string) abbreviation with '%s' replaced with '%'
                (e.g. P%sT -> P%T, E%ST -> E%T, GMT/BST, SAST)
        untilYear: (int) 2000-9999
        untilMonth: (int) 1-12 optional
        untilDay: (string) 1-31, 'lastSun', 'Sun>=3', etc
        untilTime: (string) optional
        rawLine: (string) original ZONE line in TZ file
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
    format = tokens[2].replace('%s', '%')

    # the 'RULES' field can be:
    #   * '-' no rules
    #   * a string reference to a rule
    #   * an offset like "01:00" (e.g. America/Argentina/San_Luis,
    #       Europe/Istanbul).
    rules_string = tokens[1]
    if rules_string.find(':') >= 0:
        rules_string = str(hour_string_to_offset_minutes(rules_string))

    # Return map corresponding to a ZoneEntry instance
    return {
        'offsetMinutes': offset_minutes,
        'rules': rules_string,
        'format': format,
        'untilYear': until_year,
        'untilMonth': until_month,
        'untilDay': until_day,
        'untilTime': until_time,
        'rawLine': line,
    }


def hour_string_to_offset_minutes(hs):
    """Converts the '+/-hh:mm' string into +/- minute offset.
    """
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
