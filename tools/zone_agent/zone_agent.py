#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License
"""
A Python version of the C++ ZoneAgent class to allow easier and faster iteration
of its algorithms. It is too cumbersome and tedious to experiment and debug the
C++ code in the Arduino environment.

Examples:

    # America/Indiana/Petersburg for the year 2006.
    $ ./zone_agent.py --zone Petersburg --year 2006

    # America/Indiana/Indianapolis for the year 2006.
    $ ./zone_agent.py --zone Indianapolis --year 2006

    # Australia/Darwin for the year 2006.
    $ ./zone_agent.py --zone Darwin --year 2006
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

    # Sentinel Zone Entry that represents the earliest zone entry, since
    # we kept only those after year 2000.
    ZONE_ENTRY_ANCHOR = {
        'untilYearShort': 0,
        'untilMonth': 1,
        'untilDay': 1,
        'untilHour': 0
    }

    # Sentinel Zone Rule that represents the earliest zone rule.
    ZONE_RULE_ANCHOR = {
        'fromYear': 0,
        'toYear': 0
    }

    def __init__(self, zone_info):
        """zone_info is one of the ZONE_INFO_xxx entries from zone_infos.py.
        """
        self.year = 0
        self.isFilled = False
        self.numMatches = 0
        self.zone_info = zone_info

        # List of matching zone entries. Map of the form;
        # {
        #   'startDateTime': (int, int, int, int),
        #   'untilDatetime': (int, int, int, int),
        #   'policyName': string,
        #   'zoneEntry': ZoneEntry
        # }
        self.matches = []

        # List of matching transitions. Map of the form;
        # {
        #   'startDateTime': (int, int, int, int),
        #   'untilDatetime': (int, int, int, int),
        #   'policyName': string,
        #   'zoneEntry': ZoneEntry
        #
        #   'transitionTime': (int, int, int, int), # named Rule
        #   'zoneRule': ZoneRule, # named Rule
        #   'offsetCode': int,
        #   'deltaCode': int, # named Rule
        #   'format': string,
        #   'letter': string # named Rule
        # }
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
        self.find_transitions(year)

    def print_status(self):
        print('Matches:')
        for match in self.matches:
            self.print_match(match)
        print('Transitions:')
        for transition in sorted(self.transitions,
            key=lambda x: x['transitionTime']):
            self.print_transition(transition)

    def print_match(self, match):
        policy_name = match['policyName']
        format = match['zoneEntry']['format']
        print(('start: %s-%s-%s %s:00; until: %s-%s-%s %s:00; policy: %s; '
            + 'format: %s') % (
            match['startDateTime'] + match['untilDateTime'] +
            (policy_name, format)))

    def print_transition(self, transition):
        policy_name = transition['policyName']
        offset_code = transition['offsetCode']
        format = transition['format']

        if policy_name == '-':
            params = transition['transitionTime'] + (
                policy_name, offset_code, format)
            print(('transition: %s-%s-%s %s:00; policy: %s; offsetCode: %s; '
                + 'format: %s') % params)
        else:
            delta_code = transition['deltaCode']
            letter = transition['letter']
            zone_rule = transition['zoneRule']
            zone_rule_from = zone_rule['fromYear']
            zone_rule_to = zone_rule['toYear']
            original_transition_time = transition['originalTransitionTime'] \
                if 'originalTransitionTime' in transition \
                else None

            if original_transition_time:
                params = transition['transitionTime'] \
                    + original_transition_time + (
                    policy_name, zone_rule_from, zone_rule_to, offset_code,
                    delta_code, format, letter)
                print(('transition: %s-%s-%s %s:00; original: %s-%s-%s %s:00; '
                    + 'policy: %s; from: %s; '
                    + 'to: %s; offsetCode: %s; deltaCode: %s; format: %s; '
                    + 'letter: %s')
                    % params)
            else:
                params = transition['transitionTime'] + (
                    policy_name, zone_rule_from, zone_rule_to, offset_code,
                    delta_code, format, letter)
                print(('transition: %s-%s-%s %s:00; policy: %s; from: %s; '
                    + 'to: %s; offsetCode: %s; deltaCode: %s; format: %s; '
                    + 'letter: %s')
                    % params)

    def find_matches(self, year):
        year_short = year - 2000
        zone_entries = self.zone_info['entries']
        prev_entry = self.ZONE_ENTRY_ANCHOR
        for zone_entry in zone_entries:
            if overlaps_with_year(prev_entry, zone_entry, year_short):
                zone_policy = zone_entry['zonePolicy']
                policy_name = zone_policy['name'] if zone_policy else '-'
                start_date_time = (
                    prev_entry['untilYearShort'] + self.EPOCH_YEAR,
                    prev_entry['untilMonth'],
                    prev_entry['untilDay'],
                    prev_entry['untilHour'])
                until_date_time = (
                    zone_entry['untilYearShort'] + self.EPOCH_YEAR,
                    zone_entry['untilMonth'],
                    zone_entry['untilDay'],
                    zone_entry['untilHour'])
                match = {
                    'startDateTime': start_date_time,
                    'untilDateTime': until_date_time,
                    'policyName': policy_name,
                    'zoneEntry': zone_entry
                }
                self.matches.append(match)
            prev_entry = zone_entry

    def find_transitions(self, year):
        for match in self.matches:
            self.find_transitions_from_match(year, match)

    def find_transitions_from_match(self, year, match):
        """Find all transitions of match in given year.
        """
        zone_entry = match['zoneEntry']
        zone_policy = zone_entry['zonePolicy']

        if zone_policy:
            self.find_transitions_from_named_match(year, match)
        else:
            self.find_transitions_from_simple_match(year, match)

    def find_transitions_from_simple_match(self, year, match):
        """The zonePolicy is '-', then the Zone Entry itself defines the UTC
        offset and the abbreviation.
        """
        zone_entry = match['zoneEntry']
        transition = match.copy()
        transition.update({
            'transitionTime': match['startDateTime'],
            'offsetCode': zone_entry['offsetCode'],
            'format': zone_entry['format']
        })
        self.transitions.append(transition)

    def find_transitions_from_named_match(self, year, match):
        """Find all transitions of named policy in the match interval [start,
        until) and within the given year.
        Need to check only [year, until), [start, until) or [start, year+1).
        """
        match = calc_effective_match(year, match)
        zone_entry = match['zoneEntry']
        zone_policy = zone_entry['zonePolicy']
        rules = zone_policy['rules']
        latest_prior_transition = None
        start_transition_found = False
        for rule in rules:
            rule_from_year = rule['fromYear']
            rule_to_year = rule['toYear']

            rule_versus_year = compare_rule_to_year(rule, year)
            if rule_versus_year > 0:
                continue

            candidate_prior_transition = None
            if rule_versus_year < 0:
                candidate_prior_transition = self.create_transition_for_year(
                    rule_to_year, match, rule)
            else: # rule_versus_year == 0:
                transition = self.create_transition_for_year(year, match, rule)
                transition_time = transition['transitionTime']
                transition_compared_to_match = \
                    compare_transition_to_match(year, transition_time, match)
                if transition_compared_to_match == 0:
                    self.transitions.append(transition)
                    if transition_time == match['startDateTime']:
                        start_transition_found = True

                    # Check if the candidate transition occurs in the previous
                    # year.
                    if rule_from_year <= year - 1:
                        candidate_prior_transition = \
                            self.create_transition_for_year(
                                year - 1, match, rule)
                elif transition_compared_to_match < -1:
                    candidate_prior_transition = transition

            # Find the latest candidate transition.
            if (not start_transition_found and candidate_prior_transition
                and (not latest_prior_transition or
                candidate_prior_transition['transitionTime'] > \
                latest_prior_transition['transitionTime'])):
                latest_prior_transition = candidate_prior_transition

        # Add the latest prior transition
        if not start_transition_found and latest_prior_transition:
            original_transition_time = latest_prior_transition['transitionTime']
            latest_prior_transition['transitionTime'] = match['startDateTime']
            latest_prior_transition['originalTransitionTime'] = \
                original_transition_time
            self.transitions.append(latest_prior_transition)

    def create_transition_for_year(self, year, match, rule):
        """Create the transition map item from the given rule for the given
        year. Assume that the rule applies to the given year (i.e. FROM <= year
        <= TO).
        """
        zone_entry = match['zoneEntry']
        transition_time = calc_transition_time(year, rule)
        transition = match.copy()
        transition.update({
            'transitionTime': transition_time,
            'zoneRule': rule,
            'offsetCode': zone_entry['offsetCode'],
            'deltaCode': rule['deltaCode'],
            'format': zone_entry['format'],
            'letter': rule['letter'],
        })
        return transition

def compare_rule_to_year(rule, year):
    """Determine if rule is < year, overlaps with year, or > year.
    """
    to_year = rule['toYear']
    from_year = rule['fromYear']
    if to_year < year:
        return -1
    if from_year > year:
        return 1
    return 0

def calc_effective_match(year, match):
    start_date_time = match['startDateTime']
    if start_date_time < (year, 1, 1, 0):
        start_date_time = (year, 1, 1, 0)

    until_date_time = match['untilDateTime']
    if until_date_time > (year + 1, 1, 1, 0):
        until_date_time = (year + 1, 1, 1, 0)

    eff_match = match.copy()
    eff_match['startDateTime'] = start_date_time
    eff_match['untilDateTime'] = until_date_time
    return eff_match

def compare_transition_to_match(year, transition_time, match):
    """Determine if transition_time applies to given range of the match,
    returning -1 if less than match, 0 within match, +1 more than match.
    """
    start = match['startDateTime']
    until = match['untilDateTime']
    if transition_time < start:
        return -1
    if until <= transition_time:
        return 1
    return 0

def calc_transition_time(year, rule):
    """Return the (year, month, day, hour) of the Rule in given year.
    """
    rule_month = rule['inMonth']
    rule_hour = rule['atHour']
    rule_day_of_month = calc_day_of_month(year, rule_month,
        rule['onDayOfWeek'], rule['onDayOfMonth'])
    return (year, rule_month, rule_day_of_month, rule_hour)

def calc_day_of_month(year, month, on_day_of_week, on_day_of_month):
    """Return the actual day of month of expressions such as
    (onDayOfWeek >= onDayOfMonth) or (lastMon).
    """
    if on_day_of_week == 0:
        return on_day_of_month

    if on_day_of_month == 0:
        # lastXxx == (Xxx >= (daysInMonth - 6))
        on_day_of_month = days_in_month(year, month) - 6
    limit_date = datetime.date(year, month, on_day_of_month)
    day_of_week_shift = (on_day_of_week - limit_date.isoweekday() + 7) % 7
    return on_day_of_month + day_of_week_shift

def days_in_month(year, month):
    """Return the number of days in the given (year, month).
    """
    DAYS_IN_MONTH = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    is_leap = (year % 4 == 0) and ((year % 100 != 0) or (year % 400) == 0)
    days = DAYS_IN_MONTH[month - 1]
    if month == 2:
        days += is_leap
    return days

def compare_zone_rule(year, a, b):
    """Compare Zone rule a and b, assuming they are valid prior to given year.
    Return -1, 0, 1 depending on (a < b, == b, or > b).
    """
    a_year = largest_rule_year(year, a)
    b_year = largest_rule_year(year, b)

    if a_year < b_year: return -1;
    if a_year > b_year: return 1;

    # Assume that a Zone Policy does not contain a year where more than one
    # transition occurs in a given month. So we don't need to check
    # the atHour or atHourModifier fields.
    if a['inMonth'] < b['inMonth']: return -1;
    if a['inMonth'] > b['inMonth']: return 1;

    return 0 # should never happen

def largest_rule_year(year, rule):
    """Return the largest year of the rule *prior* to the given year.
    Return 0 if the rule is not valid before the given 'year'.
    """
    if rule['toYear'] < year:
        return rule['toYear']
    if rule['fromYear'] < year:
        return year - 1
    return 0

def overlaps_with_year(prev_entry, entry, year_short):
    """Determines if entry overlaps with the given year, i.e.
    [year_short, year_short+1). The prev_entry is used to extract the start
    date of the current entry.
    """
    return (compare_entry_to_year(prev_entry, year_short + 1) < 0 and
        compare_entry_to_year(entry, year_short) >= 0)


def compare_entry_to_year(entry, year_short):
    """Compare the zone_entry with year, returning -1, 0 or 1.
    """
    if not entry:
        return -1
    if entry['untilYearShort'] < year_short:
        return -1
    if entry['untilYearShort'] > year_short:
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
