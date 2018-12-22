#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License
"""
A Python version of the C++ ZoneAgent class to allow easier and faster iteration
of its algorithms. It is too cumbersome and tedious to experiment and debug the
C++ code in the Arduino environment.
"""

import sys
import argparse
import datetime
import logging
from zone_policies import *
from zone_infos import *

class ZoneAgent:
    EPOCH_YEAR = 2000

    MAX_CACHE_ENTRIES = 4

    # Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
    # (2000-01-01 00:00:00)
    SECONDS_SINCE_UNIX_EPOCH = 946684800

    # The value of ZONE_INFO['entries']['untilYearShort'] which represents
    # 'max'. Stored as a int8_t in C++.
    MAX_UNTIL_YEAR_SHORT = 127

    # Sentinel Zone entry that represents the earliest zone entry.
    ZONE_ENTRY_ANCHOR = {
        "untilYearShort": -128,
        "untilMonth": 1,
        "untilDay": 1,
        "untilHour": 0
    }

    def __init__(self, zone_info):
        """zone_info is one of the ZONE_INFO_xxx entries from zone_infos.py.
        """
        self.year = 0
        self.isFilled = False
        self.numMatches = 0
        self.zone_info = zone_info
        self.matches = [] # list of matching zone entries
        self.transitions = [] # list of transitions

    def is_dst(self, epoch_seconds):
        return False

    def get_utc_offset(self, epoch_seconds):
        """Return UTC offset in minutes.
        """
        return 0

    def get_abbrev(self, epoch_seconds):
        """Return the timezone abbreviation at epoch_seconds.
        """

    def init_second(self, epoch_seconds):
        ldt = datetime.datetime.utcfromtimestamp(
            epoch_seconds + self.SECONDS_SINCE_UNIX_EPOCH)
        init_year(ldt.year)

    def init_year(self, year):
        self.find_matches(year)

    def find_matches(self, year):
        yearShort = year - 2000
        entries = self.zone_info['entries']
        prev_entry = self.ZONE_ENTRY_ANCHOR
        for entry in entries:
            if overlaps_with_year(prev_entry, entry, yearShort):
                match = {
                    'startYearShort': prev_entry['untilYearShort'],
                    'startMonth': prev_entry['untilMonth'],
                    'startDay': prev_entry['untilDay'],
                    'startHour': prev_entry['untilHour'],
                    'entry': entry
                }
                self.matches.append(match)
            prev_entry = entry

    def find_last_transition_prior_to(yearShort):

    def print_status(self):
        print('Matches:')
        for match in self.matches:
            self.print_match(match)

    def print_match(self, match):
        entry = match['entry']
        policyName = entry['zonePolicy']['name'] if entry['zonePolicy'] else '-'
        print('start: %s-%s-%s %s:00; policy: %s' % (
            match['startYearShort'] + self.EPOCH_YEAR,
            match['startMonth'],
            match['startDay'],
            match['startHour'],
            policyName))


def overlaps_with_year(prev_entry, entry, yearShort):
    """Determines if entry overlaps with the given year, i.e.
    [yearShort, yearShort+1). The prev_entry is used to extract the start
    date of the current entry.
    """
    return (compare_entry_to_year(prev_entry, yearShort + 1) <= 0 and
        compare_entry_to_year(entry, yearShort) > 0)


def compare_entry_to_year(entry, yearShort):
    """Compare the zone_entry with year, returning -1, 0 or 1.
    """
    if not entry:
        return -1
    if entry['untilYearShort'] < yearShort:
        return -1
    if entry['untilYearShort'] > yearShort:
        return 1
    if entry['untilMonth'] > 1:
        return 1
    if entry['untilDay'] > 1:
        return 1
    if entry['untilHour'] > 0:
        return 1
    return 0


def main():
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Zone Agent.')
    parser.add_argument('--year', help='Year of interest', type=int,
        required=True)
    parser.add_argument('--zone', help='Name of time zone',
        required=True)
    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    zone_info = ZONE_INFO_MAP.get(args.zone)
    if not zone_info:
        logging.error("Zone '%s' not found", args.zone)
        sys.exit(1)

    logging.info("Loading Zone info for '%s'", args.zone)
    zone_agent = ZoneAgent(zone_info)
    zone_agent.init_year(args.year)
    zone_agent.print_status()


if __name__ == '__main__':
    main()
