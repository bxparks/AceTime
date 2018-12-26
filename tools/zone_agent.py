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
from zonedb.zone_policies import *
from zonedb.zone_infos import *

class ZoneAgent:
    EPOCH_YEAR = 2000

    MAX_CACHE_ENTRIES = 4

    # Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
    # (2000-01-01 00:00:00)
    SECONDS_SINCE_UNIX_EPOCH = 946684800

    # The value of ZONE_INFO['eras']['untilYearShort'] which represents
    # 'max'. Stored as a int8_t in C++.
    MAX_UNTIL_YEAR_SHORT = 127

    # Sentinel ZoneEra that represents the earliest zone era, since
    # we kept only those after year 2000.
    ZONE_ERA_ANCHOR = {
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
        """zone_info is one of the ZONE_INFO_xxx constants from zone_infos.py.
        """
        self.year = 0
        self.isFilled = False
        self.numMatches = 0
        self.zone_info = zone_info

        # List of matching zone eras. Map of the form;
        # {
        #   'startDateTime': (int, int, int, int),
        #   'untilDatetime': (int, int, int, int),
        #   'policyName': string,
        #   'zoneEra': ZoneEra
        # }
        self.matches = []

        # List of matching transitions. Map of the form;
        # {
        #   'startDateTime': (int, int, int, int),
        #   'untilDatetime': (int, int, int, int),
        #   'policyName': string,
        #   'zoneEra': ZoneEra
        #
        #   # Added for simple Match and named Match.
        #   'offsetCode': int, # from ZoneEra
        #   'format': string, # from ZoneEra
        #   'transitionTime': (int, int, int, int), # from Rule or Match
        #
        #   # Added for named Match.
        #   'zoneRule': ZoneRule, # from Rule
        #   'deltaCode': int, # from Rule
        #   'letter': string # from Rule
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
        format = match['zoneEra']['format']
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
        """Find the Zone Era which overlap the 2 years from 'year' through the
        next year, i.e the interval [year, year+2).
        """
        year_short = year - 2000
        zone_eras = self.zone_info['eras']
        prev_era = self.ZONE_ERA_ANCHOR
        for zone_era in zone_eras:
            if era_overlaps_with_year(prev_era, zone_era, year_short):
                zone_policy = zone_era['zonePolicy']
                policy_name = zone_policy['name'] if zone_policy else '-'
                start_date_time = (
                    # TODO: The subtlety here is that the prev_era's 'until
                    # datetime' is expressed in the UTC offset of the *previous*
                    # era, not the current era. This is good enough for
                    # sorting (assuming we don't have have 2 DST transitions in
                    # a single day (!). But eventually, we need to fix-up these
                    # 'until' fields so that they expressed in the UTC offset of
                    # the current era.
                    prev_era['untilYearShort'] + self.EPOCH_YEAR,
                    prev_era['untilMonth'],
                    prev_era['untilDay'],
                    prev_era['untilHour'])
                until_date_time = (
                    zone_era['untilYearShort'] + self.EPOCH_YEAR,
                    zone_era['untilMonth'],
                    zone_era['untilDay'],
                    zone_era['untilHour'])
                match = {
                    'startDateTime': start_date_time,
                    'untilDateTime': until_date_time,
                    'policyName': policy_name,
                    'zoneEra': zone_era
                }
                self.matches.append(match)
            prev_era = zone_era

    def find_transitions(self, year):
        """Find the relevant transitions from the matching ZoneEras , for the
        2 years starting with year.
        """
        for match in self.matches:
            self.find_transitions_from_match(year, match)

    def find_transitions_from_match(self, year, match):
        """Find all transitions of match in given year.
        """
        zone_era = match['zoneEra']
        zone_policy = zone_era['zonePolicy']

        if zone_policy:
            self.find_transitions_from_named_match(year, match)
        else:
            self.find_transitions_from_simple_match(year, match)

    def find_transitions_from_simple_match(self, year, match):
        """The zonePolicy is '-', then the Zone Era itself defines the UTC
        offset and the abbreviation.
        """
        zone_era = match['zoneEra']
        transition = match.copy()
        transition.update({
            'offsetCode': zone_era['offsetCode'],
            'format': zone_era['format'],
            'transitionTime': match['startDateTime'],
        })
        self.transitions.append(transition)

    def find_transitions_from_named_match(self, year, match):
        """Find the relevant transitions of the named policy in the Match
        interval [start, until) for the 2 years of [year, year+2). We generate 2
        years worth because we are caught in a Catch-22 situation when trying to
        determine the UTC offsets needed to convert a epochSecond to the local
        DateTime.

        If we knew the local year of the epochSeconds when converted to the
        local DateTime, then we would need only the Transitions of the given
        local year. However, the epochSeconds could convert to Dec 31 or Jan 1
        in UTC timezone, which means that the local year could shift to the next
        or previous year in the local time zone. But we don't know the local
        time zone offset until we generate the Transitions of the local year,
        and we don't know the local year until we generate the Transitions.

        To get around this problem, if the UTC month of the converted
        epochSecond falls from July to December, we generate the Transitions for
        UTC.year and UTC.year+1. If the UTC month falls in Jan to June, we
        generate the Transitions for UTC.year-1 and UTC.year. If the start of
        the 2 year interval also requires the determination of the latest
        prior Transition for that policy, we do that as well.

        The algorithm is the following:

        * Calculate the effective Match window which overlaps with the two-year
          interval [year, year+2):
            * Match.start = max(Match.start, year)
            * Match.until = min(Match.until, year+2)
        * Loop for each Rule entry in the Zone policy given by the Match:
            * Find the Transitions that may occur in (year, year+1, and the
              largest Rule.year < year), for a maximum 3 Transitions.
            * For each Transition:
                * If Transition occurs >= Match.until, ignore it.
                * If Transition occurs within [Match.start, Match.until):
                    * Add to the Transitions collection.
                    * If Transition == Match.start:
                        * Set startTransitionFound flag.
                * Else Transition is < Match.start:
                    * If not startTransitionFound:
                        * Nominate as latest prior Transition.
        * If not startTransitionFound:
            * If latest prior transition exists:
                * Shift the prior Transition to Match.start
                * Add to Transitions collection.
        """
        match = calc_effective_match(year, match)
        zone_era = match['zoneEra']
        zone_policy = zone_era['zonePolicy']
        rules = zone_policy['rules']

        results = {}
        # For each Rule, process the 3 possible Transitions.
        for rule in rules:
            transition = self.create_transition_for_year(year, rule, match)
            if transition:
                self.process_transition(year, match, transition, results)

            transition = self.create_transition_for_year(year+1, rule, match)
            if transition:
                self.process_transition(year, match, transition, results)

            transition = self.create_transition_prior_to_year(year, rule, match)
            if transition:
                self.process_transition(year, match, transition, results)

        # Add the latest prior transition
        if not results.get('startTransitionFound'):
            prior_transition = results.get('latestPriorTransition')
            if not prior_transition:
                logging.error("No prior transition found!")
                sys.exit(1)
            original_time = prior_transition['transitionTime']
            prior_transition['transitionTime'] = match['startDateTime']
            prior_transition['originalTransitionTime'] = original_time
            self.transitions.append(prior_transition)

    def process_transition(self, year, match, transition, results):
        """Process the given transition. The 'results' is a map that keeps
        track of the processing:
            * If transition >= match.until:
                * do nothing
            * If transition within match:
                * add transition to self.transitions
                * if transition == match.start
                    * set results['startTransitionFound'] = True
            * If transition < match:
                * if not startTransitionFound:
                    * set results['latestPriorTransition'] = latest
        """
        transition_time = transition['transitionTime']
        transition_compared_to_match = compare_transition_to_match(
            year, transition_time, match)
        if transition_compared_to_match > 0:
            return
        elif transition_compared_to_match == 0:
            self.transitions.append(transition)
            if transition_time == match['startDateTime']:
                results['startTransitionFound'] = True
        else: # transition_compared_to_match < -1:
            # Determine the latest prior transition
            if results.get('startTransitionFound'):
                return

            latest_prior_transition = results.get('latestPriorTransition')
            if not latest_prior_transition:
                results['latestPriorTransition'] = transition
                return

            latest_prior_time = latest_prior_transition['transitionTime']
            if transition_time > latest_prior_time:
                results['latestPriorTransition'] = transition


    def create_transition_for_year(self, year, rule, match):
        """Create the transition from the given rule for the given year, if
        the rule overlaps with the given year. Return None if it does not
        overlap. The Transition object is a replica of the underlying Match
        object, with additional bookkeeping info.
        """
        # Check if year overlaps with [Rule.from, Rule.to].
        from_year = rule['fromYear']
        to_year = rule['toYear']
        if year < from_year or to_year < year:
            return None

        transition_time = get_transition_time(year, rule)
        zone_era = match['zoneEra']
        transition = match.copy()
        transition.update({
            'offsetCode': zone_era['offsetCode'],
            'format': zone_era['format'],
            'transitionTime': transition_time,
            'zoneRule': rule,
            'deltaCode': rule['deltaCode'],
            'letter': rule['letter'],
        })
        return transition

    def create_transition_prior_to_year(self, year, rule, match):
        """Create the transition from the given rule corresponding to
        the latest rule.year just prior to given year.
        """
        # Get the most recent prior year
        from_year = rule['fromYear']
        to_year = rule['toYear']
        if from_year >= year:
            return None
        if to_year < year:
            prior_year = to_year
        else:
            prior_year = year - 1

        transition_time = get_transition_time(prior_year, rule)
        zone_era = match['zoneEra']
        transition = match.copy()
        transition.update({
            'offsetCode': zone_era['offsetCode'],
            'format': zone_era['format'],
            'transitionTime': transition_time,
            'zoneRule': rule,
            'deltaCode': rule['deltaCode'],
            'letter': rule['letter'],
        })
        return transition


def calc_effective_match(year, match):
    """Generate a version of match which overlaps the 2 year interval from
    [year, year+2).
    """
    start_date_time = match['startDateTime']
    if start_date_time < (year, 1, 1, 0):
        start_date_time = (year, 1, 1, 0)

    until_date_time = match['untilDateTime']
    if until_date_time > (year + 2, 1, 1, 0):
        until_date_time = (year + 2, 1, 1, 0)

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


def get_transition_time(year, rule):
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


def era_overlaps_with_year(prev_era, era, year_short):
    """Determines if era overlaps with 2 years that starts with given year,
    i.e. [year_short, year_short+2). The start date of the current era is
    represented by the prev_era.UNTIL, so the interval of the current era is
    [start, end) = [prev_era.UNTIL, era.UNTIL). Overlap happens if (start <
    year_short+2) and (end > year_short).
    """
    return (compare_era_to_year(prev_era, year_short + 2) < 0 and
        compare_era_to_year(era, year_short) > 0)


def compare_era_to_year(era, year_short):
    """Compare the zone_era with year, returning -1, 0 or 1.
    """
    if not era:
        return -1
    if era['untilYearShort'] < year_short:
        return -1
    if era['untilYearShort'] > year_short:
        return 1
    if era['untilMonth'] > 1:
        return 1
    if era['untilDay'] > 1:
        return 1
    if era['untilHour'] > 0:
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
