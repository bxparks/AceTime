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

A Zone Policy is composed of a collection of Zone Rules. A Zone Rule record has
the following columns:

# Rule  NAME    FROM    TO    TYPE IN   ON      AT      SAVE    LETTER
Rule    US      2007    max   -    Mar  Sun>=8  2:00    1:00    D
Rule    US      2007    max   -    Nov  Sun>=1  2:00    0       S

A Zone Info is composed of a collection of Zone Eras. A Zone Era is each line
of the following and has the following columns:
# Zone  NAME                GMTOFF      RULES   FORMAT  [UNTIL]
Zone    America/Chicago     -5:50:36    -       LMT     1883 Nov 18 12:09:24
                            -6:00       US      C%sT    1920
                            ...
                            -6:00       US      C%sT
The UNTIL column should be monotonically increasing and the last Zone era line
has an empty UNTIL field.
"""

import argparse
import logging
import sys
import os


# AceTime Epoch is 2000-01-01 00:00:00
EPOCH_YEAR = 2000

# Indicate max UNTIL year (represented by empty field).
MAX_UNTIL_YEAR = 10000

# Tiny (int8_t) version of MAX_UNTIL_YEAR_TINY.
MAX_UNTIL_YEAR_TINY = 127

# Indicate max TO or FROM year.
MAX_YEAR = MAX_UNTIL_YEAR - 1

# Tiny (int8_t) version of MAX_YEAR.
MAX_YEAR_TINY = MAX_UNTIL_YEAR_TINY - 1

# Minimum valid TO or FROM year.
MIN_YEAR = 1

# Tiny (int8_t) version of MIN_YEAR.
MIN_YEAR_TINY = -128

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
        self.zones = {} # map of zoneName to zoneEra[], ZoneEra = {}
        self.ignored_rule_lines = 0
        self.ignored_zone_lines = 0
        self.invalid_rule_lines = 0
        self.invalid_zone_lines = 0

    def parse(self):
        """Read the zoneinfo files from TZ Database and create the 'zones' and
        'rules' maps which can be retrieved by get_data().
        """
        self.parse_zone_files()
        self.process_rules()
        self.process_zones()

    def get_data(self):
        """
        Returns 2 dictionaries: zones_map and rules_map.

        The zones_map contains:
            offsetString: (string) offset from UTC/GMT
            rules: (string) name of the Rule in effect, '-', or minute offset
            format: (string) abbreviation format (e.g. P%sT, E%ST, GMT/BST)
            untilYear: (int) MAX_UNTIL_YEAR means 'max'
            untilMonth: (int) 1-12
            untilDay: (string) e.g. '1', 'lastSun', 'Sun>=3', etc
            untilTime: (string) e.g. '2:00', '00:01'
            untilTimeModifier: (char) '', 's', 'w', 'g', 'u', 'z'
            rawLine: (string) original ZONE line in TZ file

        The rules_map contains:
            fromYear: (int) from year
            toYear: (int) to year, 0000 to MAX_YEAR=max
            inMonth: (int) month index (1-12)
            onDay: (string) 'lastSun' or 'Sun>=2', or 'DD'
            atTime: (string) the time when transition to and from DST happens
            atTimeModifier: (char) '', 's', 'w', 'g', 'u', 'z'
            deltaOffset: (string) offset from Standard time ('SAVE' field)
            letter: (char) 'D', 'S', '-'
            rawLine: (string) the original RULE line from the TZ file
        """
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
                    zone_era = process_zone_line(line)
                    if zone_era:
                        add_item(self.zones, name, zone_era)
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

        logging.info('=== Extractor Summary')
        logging.info('Rule lines count: %s' % len(self.rule_lines))
        logging.info('Zone lines count: %s' % len(self.zone_lines))
        logging.info('Link lines count: %s' % len(self.link_lines))
        logging.info('Rules name count: %s' % len(self.rules))
        logging.info('Zones name count: %s' % len(self.zones))
        logging.info('Rule entry count: %s' % rule_entry_count)
        logging.info('Zone entry count: %s' % zone_entry_count)
        logging.info('Ignored Rule lines: %s' % self.ignored_rule_lines)
        logging.info('Ignored Zone lines: %s' % self.ignored_zone_lines)
        logging.info('Invalid Rule lines: %s' % self.invalid_rule_lines)
        logging.info('Invalid Zone lines: %s' % self.invalid_zone_lines)
        logging.info('=== Extractor Summary End')


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

def process_rule_line(line):
    """Normalize a dictionary that represents a 'Rule' line from the TZ
    database. Contains the following fields:
    Rule NAME FROM TO TYPE IN ON AT SAVE LETTER
    0    1    2    3  4    5  6  7  8    9

    These represent transitions from Daylight to/from Standard.
    """
    tokens = line.split()

    # Check for valid year.
    from_year = int(tokens[2])
    to_year_string = tokens[3]
    if to_year_string == 'only':
        to_year = from_year
    elif to_year_string == 'max':
        to_year = MAX_YEAR
    else:
        to_year = int(to_year_string)

    in_month = MONTH_TO_MONTH_INDEX[tokens[5]]
    on_day = tokens[6]
    (at_time, at_time_modifier) = parse_at_time_string(tokens[7])
    delta_offset = tokens[8]

    # Return map corresponding to a ZoneRule instance
    return {
        'fromYear': from_year,
        'toYear': to_year,
        'inMonth': in_month,
        'onDay': on_day,
        'atTime': at_time,
        'atTimeModifier': at_time_modifier,
        'deltaOffset': delta_offset,
        'letter': tokens[9],
        'rawLine': line,
    }


def parse_at_time_string(at_string):
    """Parses the '2:00s' string into '2:00' and 's'. If there is no suffix,
    returns a '' as the suffix.
    """
    modifier = at_string[-1:]
    if modifier.isdigit():
        modifier = ''
        at_time = at_string
    else:
        at_time = at_string[:-1]
    if modifier not in ['', 'w', 's', 'u', 'g', 'z']:
        raise Exception('Invalid AT modifier (%s)' % modifier)
    return (at_time, modifier)


def process_zone_line(line):
    """Normalize an zone era from dictionary that represents one line of
    a 'Zone' record. The columns are:
    GMTOFF	 RULES	FORMAT	[UNTIL]
    0        1      2       3
    -5:50:36 -      LMT     1883 Nov 18 12:09:24
    -6:00    US     C%sT    1920
    """
    tokens = line.split()

    # GMTOFF
    offset_string = tokens[0]

    # 'RULES' field can be:
    rules_string = tokens[1]

    # check 'until' year
    if len(tokens) >= 4:
        until_year = int(tokens[3])
    else:
        until_year = MAX_UNTIL_YEAR

    # check for additional components of 'UNTIL' field
    if len(tokens) >= 5:
        until_month = MONTH_TO_MONTH_INDEX[tokens[4]]
    else:
        until_month = 1

    if len(tokens) >= 6:
        until_day = tokens[5]
    else:
        until_day = '1'

    if len(tokens) >= 7:
        (until_time, until_time_modifier) = parse_at_time_string(tokens[6])
    else:
        until_time = '00:00'
        until_time_modifier = 'w'

    # FORMAT
    format = tokens[2]

    # Return map corresponding to a ZoneEra instance
    return {
        'offsetString': offset_string,
        'rules': rules_string,
        'format': format,
        'untilYear': until_year,
        'untilMonth': until_month,
        'untilDay': until_day,
        'untilTime': until_time,
        'untilTimeModifier': until_time_modifier,
        'rawLine': line,
    }
