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
# Rule	NAME	FROM	TO	TYPE	IN	ON	AT	SAVE	LETTER
Rule	US	2007	max	-	Mar	Sun>=8	2:00	1:00	D
Rule	US	2007	max	-	Nov	Sun>=1	2:00	0	S

Zone entries have the following columns:
# Zone	NAME		GMTOFF	RULES	FORMAT	[UNTIL]
Zone America/Chicago	-5:50:36 -	LMT	1883 Nov 18 12:09:24
			-6:00	US	C%sT	1920

Usage:
    cat africa antarctica asia australasia europe northamerica southamerica \
        | ./extractor.py --cpp_file file.cpp --header_file file.h 
"""

import argparse
import logging
import sys


class Extractor:
    """Reads each test data section from the given file-like object (e.g.

        extractor = Extractor(sys.stdin)
        extractor.parse_zone_file()
    """

    # Recognized tags.
    # TODO: Change to a hash set to speed up the lookup if many are added.
    TAG_TOKENS = ['DATA', 'ERRORS', 'SCHEMA', 'END']

    def __init__(self, input, **kwargs):
        self.input = input
        self.next_line = None
        self.rule_lines = {} # dictionary of ruleName to lines[]
        self.zone_lines = {} # dictionary of zoneName to lines[]
        self.link_lines = {} # dictionary of linkName to lines[]
        self.rules = {}
        self.zones = {}

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
                    rule_entry = self.process_rule_line(name, line)
                    if rule_entry:
                        add_item(self.rules, name, rule_entry)
                except Exception as e:
                    logging.error('Exception %s: %s', e, line)
                    continue

    def process_rule_line(self, name, line):
        tokens = line.split()

        # Check for valid year. We can ignore everything before 2000.
        from_year = tokens[2]
        to_year = tokens[3]
        if to_year == 'only':
            to_year = from_year
        if from_year < '2000' and to_year < '2000':
            return None

        # Check for LETTER field longer than 1 character
        letter = tokens[9]
        if len(letter) > 1:
            logging.error(
                'Rule %s: LETTER field (%s) more than 1 character: %s',
                name, letter, line)
            return None

        # Add rule entry
        rule_entry = {
            'from': from_year,
            'to': to_year,
            'in_month': tokens[5],
            'on_day': tokens[6],
            'at_time': tokens[7],
            'save': tokens[8],
            'letter': letter,
        }
        normalize_rule_entry(rule_entry)

        return rule_entry

    def process_zones(self):
        for name, lines in self.zone_lines.items():
            for line in lines:
                try:
                    zone_entry = self.process_zone_line(name, line)
                    if zone_entry:
                        add_item(self.zones, name, zone_entry)
                except Exception as e:
                    logging.error('Exception %s: %s', e, line)
                    continue


    def process_zone_line(self, name, line):
        tokens = line.split()

        if len(tokens) >= 4:
            until_year = tokens[3]
        else:
            until_year = '9999'

        # Skip zone entries before 2000
        if until_year < '2000':
            return None

        # Create the zone entry
        zone_entry = {
            'gmtoff': tokens[0],
            'rules': tokens[1],
            'format': tokens[2],
            'until_year': until_year,
        }
        if len(tokens) >= 5:
            until_month = tokens[4]
            if until_month:
                # TODO: relax this constraint because too many zones seem
                # to use the month, day and time fields in the UNTIL field.
                raise Exception(
                    'Zone %s: Unsupported month in until_date (%s %s)' %
                    (name, until_year, until_month))
            zone_entry['until_month'] = until_month
        if len(tokens) >= 6:
            zone_entry['until_day'] = tokens[5]
        if len(tokens) >= 7:
            zone_entry['until_time'] = tokens[6]

        normalize_zone_entry(zone_entry)
        return zone_entry

    def print_summary(self):
        print('Rule lines count: %s' % len(self.rule_lines))
        print('Zone lines count: %s' % len(self.zone_lines))
        print('Link lines count: %s' % len(self.link_lines))
        print('Rules count: %s' % len(self.rules))
        print('Zones count: %s' % len(self.zones))

        for name, rules in self.rules.items():
            print('Rules %s' % name)
            for rule in rules:
                print(rule)
        
        for name, zones in self.zones.items():
            print('Zones %s' % name)
            for zone in zones:
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


def normalize_rule_entry(rule):
    """ Normalize a dictionary that represents a 'Rule' line from the TZ
    database. The result is a modified dictionary with the following fields:
        from: from year (int)
        to: to year (int), 9999=max
        in: month index (1-12)
        on_day_of_week: 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
        on_day_of_month: (1-31), 0={last dayOfWeek match}
        at_hour: hour to transition to and from DST
        at_modifier: 's', 'w', 'g', 'u', 'z'
        letter: 'D', 'S', '-'
    """
    rule['from'] = int(rule['from'])
    to_year = rule['to']
    if to_year == 'max':
        rule['to'] = 9999
    else:
        rule['to'] = int(to_year)
    rule['in_month'] = MONTH_TO_MONTH_INDEX[rule['in_month']]
    (on_day_of_week, on_day_of_month) = parse_on_day_string(rule['on_day'])
    rule['on_day_of_week'] = on_day_of_week
    rule['on_day_of_month'] = on_day_of_month
    (at_hour, at_modifier) = parse_at_hour_string(rule['at_time'])
    rule['at_hour'] = at_hour
    rule['at_modifier'] = at_modifier


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


def normalize_zone_entry(entry):
    """Normalize an entry from dictionary that represents one line of 
    a 'Zone' record. The result is a modified entry with:

        offsetCode: off set from UTC/GMT in 15 minute increments (int)
        rules: name of the Rule in effect (string)
        format: abbreviation with '%s' replaced with '%'
                (e.g. P%sT -> P%T, GMT/BST, SAST)
        until_year: (int)
    """
    offset_code = hour_string_to_offset_code(entry['gmtoff'])
    abbrev = entry['format'].replace('%s', '%')

    entry['offset_code'] = offset_code
    entry['abbrev'] = abbrev
    entry['until_year'] = int(entry['until_year'])


def hour_string_to_offset_code(hs):
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
        minute_string = hs[colon_index+1:]
    hour = int(hour_string)
    minute = int(minute_string)
    if minute % 15 != 0:
        raise Exception('Cannot support GMTOFF (%s)' % hs)
    offset_code = sign * (hour * 4 + minute/15)
    return offset_code
    

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
    args = parser.parse_args()

    extractor = Extractor(
        sys.stdin,
        cpp_file=args.cpp_file,
        header_file=args.header_file)
    extractor.parse_zone_file()
    extractor.process_rules()
    extractor.process_zones()
    extractor.print_summary()


if __name__ == '__main__':
    main()
