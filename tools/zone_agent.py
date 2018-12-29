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
    # AceTime Epoch is 2000-01-01 00:00:00
    EPOCH_YEAR = 2000

    # Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
    # (2000-01-01 00:00:00)
    SECONDS_SINCE_UNIX_EPOCH = 946684800

    # The biggest 'untilYearShort' when represented by as a int8_t in C++.
    MAX_UNTIL_YEAR_SHORT = 127

    # The smallest 'untilYearShort' when represented by as a int8_t in C++.
    MIN_UNTIL_YEAR_SHORT = -128

    # Sentinel ZoneEra that represents the earliest zone era, since
    # we kept only those after year 2000.
    ZONE_ERA_ANCHOR = {
        'untilYearShort': MIN_UNTIL_YEAR_SHORT,
        'untilMonth': 1,
        'untilDay': 1,
        'untilHour': 0,
        'untilMinute': 0,
        'untilTimeModifier': 'w'
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
        #   'startDateTime': (y, m, d, h, m, modifier),
        #   'untilDatetime': (y, m, d, h, m, modifier),
        #   'policyName': string,
        #   'zoneEra': ZoneEra
        # }
        self.matches = []

        # List of matching transitions. Map of the form;
        # {
        #   # Copied from ZoneEra
        #   'startDateTime': (y, m, d, h, m, modifier), # replaced later
        #   'untilDatetime': (y, m, d, h, m, modifier), # replaced later
        #   'policyName': string, # '-', ':', or symbolic reference
        #   'zoneEra': ZoneEra
        #
        #   # Added for simple Match and named Match.
        #   'transitionTime': (y, m, d, h, m, modifier), # from Rule or Match
        #   'offsetMinutes': int, # from ZoneEra
        #   'deltaMinutes': int, # from ZoneRule or ZoneEra
        #   'format': string, # from ZoneEra
        #   'abbrev': string, # abbreviation
        #
        #   # Added for named Match.
        #   'zoneRule': ZoneRule, # from Rule
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
        self.transitions = sorted(self.transitions,
            key=lambda x: x['transitionTime'])
        self.fix_start_times()
        self.calc_abbrev()
        self.generate_start_until_times()

    def print_status(self):
        print('Matches:')
        for match in self.matches:
            self.print_match(match)

        print('Transitions:')
        for transition in self.transitions:
            self.print_transition(transition)


    def print_match(self, match):
        policy_name = match['policyName']
        format = match['zoneEra']['format']
        print(('start: %4d-%02d-%02d %02d:%02d%s; '
            + 'until: %4d-%02d-%02d %02d:%02d%s; '
            + 'policy: %s; format: %s') % (
            match['startDateTime'] + match['untilDateTime'] +
            (policy_name, format)))

    def print_transition(self, transition):
        tt = transition['transitionTime']
        sdt = transition['startDateTime']
        udt = transition['untilDateTime']
        policy_name = transition['policyName']
        offset_minutes = transition['offsetMinutes']
        delta_minutes = transition['deltaMinutes']
        if delta_minutes == None: delta_minutes = 0
        format = transition['format']
        abbrev = transition['abbrev']

        if policy_name in ['-', ':']:
            params = tt + sdt + udt + (
                policy_name, offset_minutes, delta_minutes, format, abbrev)
            print(('transition: %4d-%02d-%02d %02d:%02d%s; '
                + 'start: %4d-%02d-%02d %02d:%02d%s; '
                + 'until: %4d-%02d-%02d %02d:%02d%s; '
                + 'policy: %s; '
                + 'UTC%+d%+d; '
                + 'format: %s; '
                + 'abbrev: %s')
                % params)
        else:
            delta_minutes = transition['deltaMinutes']
            letter = transition['letter']
            zone_rule = transition['zoneRule']
            zone_rule_from = zone_rule['fromYear']
            zone_rule_to = zone_rule['toYear']
            ott = transition['originalTransitionTime'] \
                if 'originalTransitionTime' in transition else None

            if ott:
                params = tt + sdt + udt + ott + (
                    policy_name, zone_rule_from, zone_rule_to, offset_minutes,
                    delta_minutes, format, letter, abbrev)
                print(('transition: %4d-%02d-%02d %02d:%02d%s; '
                    + 'start: %4d-%02d-%02d %02d:%02d%s; '
                    + 'until: %4d-%02d-%02d %02d:%02d%s; '
                    + 'original: %4d-%02d-%02d %02d:%02d%s; '
                    + 'policy: %s[%d,%d]; '
                    + 'UTC%+d%+d; '
                    + 'format: %s(%s); '
                    + 'abbrev: %s')
                    % params)
            else:
                params = tt + sdt + udt + (
                    policy_name, zone_rule_from, zone_rule_to,
                    offset_minutes, delta_minutes, format, letter, abbrev)
                print(('transition: %4d-%02d-%02d %02d:%02d%s; '
                    + 'start: %4d-%02d-%02d %02d:%02d%s; '
                    + 'until: %4d-%02d-%02d %02d:%02d%s; '
                    + 'policy: %s[%d,%d]; '
                    + 'UTC%+d%+d; '
                    + 'format: %s(%s); '
                    + 'abbrev: %s')
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
                # zonePoicy one of 3 states: '-', ':' or a reference
                zone_policy = zone_era['zonePolicy']
                if zone_policy in ['-', ':']:
                    policy_name = zone_policy
                else:
                    policy_name = zone_policy['name']

                start_date_time = (
                    # The subtlety here is that the prev_era's 'until datetime'
                    # is expressed using the UTC offset of the *previous* era,
                    # not the current era. This is probably good enough for
                    # sorting, assuming we don't have 2 DST transitions in a
                    # single day. See fix_start_times() which normalizes these
                    # start times to the wall time uniformly.
                    prev_era['untilYearShort'] + self.EPOCH_YEAR,
                    prev_era['untilMonth'],
                    prev_era['untilDay'],
                    prev_era['untilHour'],
                    prev_era['untilMinute'],
                    prev_era['untilTimeModifier'])
                until_date_time = (
                    zone_era['untilYearShort'] + self.EPOCH_YEAR,
                    zone_era['untilMonth'],
                    zone_era['untilDay'],
                    zone_era['untilHour'],
                    zone_era['untilMinute'],
                    zone_era['untilTimeModifier'])
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

        if zone_policy in ['-', ':']:
            self.find_transitions_from_simple_match(year, match)
        else:
            self.find_transitions_from_named_match(year, match)

    def find_transitions_from_simple_match(self, year, match):
        """The zonePolicy is '-' or ':' then the Zone Era itself defines the UTC
        offset and the abbreviation.
        """
        zone_era = match['zoneEra']
        transition = match.copy()
        transition.update({
            'offsetMinutes': zone_era['offsetMinutes'],
            'deltaMinutes': zone_era['rulesDeltaMinutes'],
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
            'offsetMinutes': zone_era['offsetMinutes'],
            'format': zone_era['format'],
            'transitionTime': transition_time,
            'zoneRule': rule,
            'deltaMinutes': rule['deltaMinutes'],
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
            'offsetMinutes': zone_era['offsetMinutes'],
            'format': zone_era['format'],
            'transitionTime': transition_time,
            'zoneRule': rule,
            'deltaMinutes': rule['deltaMinutes'],
            'letter': rule['letter'],
        })
        return transition

    def fix_start_times(self):
        """The Transition time comes from either:
            1) The UNTIL field of the previous Zone Era entry, or
            2) The (inMonth, onDay, atHour) fields of the Zone Rule.

        In most cases these times are specified as the wall clock 'w' by
        default, but a few cases use 's' (standard) or 'u' (utc). We don't need
        to support 'g' and 'z' because they mean exactly the same as 'u' and
        they don't appear anywhere in the current TZ files. The transformer.py
        will detect and filter those out.

        To convert these into the more common 'wall' time, we need to
        use the UTC offset of the *previous* Transition.
        """
        # Bootstrap the transition with the first transition, effectively
        # extending the first transition backwards to -infinity. This won't be
        # 100% correct with respect to the TZ Database because we kept only the
        # transitions spanning 2 years. But, for the dateTime or epochSecond
        # that we care about in the ZoneAgent, it will be good enough because we
        # are guaranteed at the time Instant of interest will always be after
        # the 2nd half of the first year. If the first transition at the
        # beginning of the first year is slightly off, it doesn't matter.
        prev = self.transitions[0].copy()
        for transition in self.transitions:
            prev_start_time = prev['transitionTime']
            prev_delta_minutes = prev.get('deltaMinutes')
            prev_delta_minutes = prev_delta_minutes if prev_delta_minutes else 0

            start_time = transition['transitionTime']
            start_modifier = start_time[5]
            if start_modifier == 'w':
                pass
            elif start_modifier == 's':
                mins = hour_minute_to_minutes(start_time[3], start_time[4])
                mins += prev_delta_minutes
                hm = minutes_to_hour_minute(mins)
                transition['transitionTime'] = (start_time[0], start_time[1],
                    start_time[2], hm[0], hm[1], 'w')
            elif start_modifier == 'u':
                prev_offset_minutes = prev.get('offsetMinutes')
                prev_offset_minutes = prev_offset_minutes \
                    if prev_offset_minutes else 0
                mins = hour_minute_to_minutes(start_time[3], start_time[4])
                mins += prev_delta_minutes + prev_offset_minutes
                hm = minutes_to_hour_minute(mins)
                transition['transitionTime'] = (start_time[0], start_time[1],
                    start_time[2], hm[0], hm[1], 'w')
            else:
                logging.error(
                    "Unrecognized Rule.AT suffix '%s'; start_time=%s",
                    start_modifier, start_time)
                sys.exit(1)
            prev = transition

    def calc_abbrev(self):
        """Calculate the time zone abbreviations for each Transition.
        There are several cases:
            1) 'format' contains 'A/B', meaning 'A' for standard time, and 'B'
                for DST time.
            2) 'format' contains a %s, which substitutes the 'letter'
                2a) If 'letter' is '-', replace with nothing.
                2b) The 'format' could be just a '%s'.
        """
        for transition in self.transitions:
            format = transition['format']
            delta_minutes = transition['deltaMinutes']

            index = format.find('/')
            if index >= 0:
                if delta_minutes == 0:
                    abbrev = format[:index]
                else:
                    abbrev = format[index+1:]
            elif format.find('%s') >= 0:
                letter = transition['letter']
                if letter == '-': letter = ''
                abbrev = format % letter
            else:
                abbrev = format

            transition['abbrev'] = abbrev

    def generate_start_until_times(self):
        """Calculate the various start and until times of the Transitions in the
        following way:
            1) The 'untilDateTime' of the previous Transition is the
            'transitionTime' of the current Transition with no adjustments.
            2) The local 'startDateTime' of the current Transition is
            the current 'transitionTime' - (prevOffset + prevDelta) +
            (currentOffset + currentDelta).
            3) The 'startEpochSecond' of the current Transition is the
            'transitionTime' using the UTC offset of the *previous* Transition.
        Got all that?
        """
        # As before, bootstrap the prev transition with the first transition
        # so that we have a UTC offset to work with.
        prev = self.transitions[0]
        is_after_first = False
        for transition in self.transitions:
            tt = transition['transitionTime']

            # 1) Update the 'untilDateTime' of the previous Transition.
            if is_after_first:
                prev['untilDateTime'] = tt

            # 2) Calculate the current startDateTime by shifting the time
            # into the current UTC offset.
            mins = hour_minute_to_minutes(tt[3], tt[4])
            mins += (-(prev['offsetMinutes'] + prev['deltaMinutes']) +
                (transition['offsetMinutes'] + transition['deltaMinutes']))
            if mins < 0 or mins >= 24 * 60:
                logging.info(
                    "Transition startDateTime shifted into a different day: "
                    + "(%02d:%02d)", h, m)
            (h, m) = minutes_to_hour_minute(mins)
            st = datetime.datetime(tt[0], tt[1], tt[2], 0, 0, 0)
            st += datetime.timedelta(minutes = mins)
            transition['startDateTime'] = (st.year, st.month, st.day, st.hour,
                st.minute, tt[5])

            # 3) The epochSecond of the 'transitionTime' is determined by the
            # UTC offset of the *previous* Transition. However, the
            # transitionTime represent by an illegal date (e.g. 24:00). So, it
            # is better to use the properly normalized startDateTime with the
            # *current* UTC offset.
            utc_offset_minutes = prev['offsetMinutes'] + prev['deltaMinutes']
            z = datetime.timezone(
                datetime.timedelta(minutes=utc_offset_minutes))
            t = st.replace(tzinfo=z)
            epoch_second = int(t.timestamp())
            transition['startEpochSecond'] = epoch_second

            prev = transition
            is_after_first = True


def calc_effective_match(year, match):
    """Generate a version of match which overlaps the 2 year interval from
    [year, year+2).
    """
    start_date_time = match['startDateTime']
    if start_date_time < (year, 1, 1, 0, 0, 'w'):
        start_date_time = (year, 1, 1, 0, 0, 'w')

    until_date_time = match['untilDateTime']
    if until_date_time > (year + 2, 1, 1, 0, 0, 'w'):
        until_date_time = (year + 2, 1, 1, 0, 0, 'w')

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
    month = rule['inMonth']
    hour = rule['atHour']
    minute = rule['atMinute']
    day_of_month = calc_day_of_month(
        year, month, rule['onDayOfWeek'], rule['onDayOfMonth'])
    modifier = rule['atTimeModifier']
    return (year, month, day_of_month, hour, minute, modifier)


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
    """Compare the zone_era with year, returning -1, 0 or 1. Ignore the
    untilTimeModifier suffix. Maybe it's not needed in this context?
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
    if era['untilMinute'] > 0:
        return 1
    return 0


def add_hour_minute(a, b):
    am = hour_minute_to_minutes(a[0], a[1])
    bm = hour_minute_to_minutes(b[0], b[1])
    cm = am + bm
    return minutes_to_hour_minute(cm)


def minutes_to_hour_minute(m):
    return (m // 60, m % 60)


def hour_minute_to_minutes(h, m):
    return 60 * h + m


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
