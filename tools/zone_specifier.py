#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License
"""
A Python version of the C++ ZoneSpecifier class to allow easier and faster
iteration of its algorithms. It is too cumbersome and tedious to experiment and
debug the C++ code in the Arduino environment.

Examples:

    # America/Los_Angeles for 2018-03-10T02:00:00
    $ ./zone_specifier.py --zone Los_Angeles --date 2019-03-10T01:59
    UTC-08:00+00:00 (PST)

    $ ./zone_specifier.py --zone Los_Angeles --date 2019-03-10T02:00
    Invalid time

    $ ./zone_specifier.py --zone Los_Angeles --date 2019-03-10T03:00
    UTC-08:00+01:00 (PDT)

    $ ./zone_specifier.py --zone Los_Angeles --date 2019-11-03T01:00
    UTC-08:00+01:00 (PDT)

    $ ./zone_specifier.py --zone Los_Angeles --date 2019-11-03T01:59
    UTC-08:00+01:00 (PDT)

    $ ./zone_specifier.py --zone Los_Angeles --date 2019-11-03T02:00
    UTC-08:00+00:00 (PST)

    # America/Indiana/Petersburg for the year 2006.
    $ ./zone_specifier.py --zone Petersburg --year 2006

    # America/Indiana/Indianapolis for the year 2006.
    $ ./zone_specifier.py --zone Indianapolis --year 2006

    # Australia/Darwin for the year 2006.
    $ ./zone_specifier.py --zone Darwin --year 2006

    $ ./zone_specifier.py --zone Adak --year 2000 --viewing_months 13 --debug
"""

import sys
import argparse
import logging
import collections
import traceback
from datetime import datetime
from datetime import timedelta
from datetime import timezone
from datetime import date
from extractor import MIN_YEAR
from transformer import seconds_to_hms
from transformer import hms_to_seconds
from zonedb.zone_policies import *
from zonedb.zone_infos import *

# A datetime representation using seconds instead of h:m:s
DateTuple = collections.namedtuple("DateTuple", "y m d ss f")

# A tuple of (year, month)
YearMonthTuple = collections.namedtuple("YearMonthTuple", "y m")

# Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
# (2000-01-01 00:00:00)
SECONDS_SINCE_UNIX_EPOCH = 946684800

ACETIME_EPOCH = datetime(2000, 1, 1, tzinfo=timezone.utc)


class ZoneInfoCooked:
    """Internal representation of a single ZoneInfo dictionary stored in the
    zone_infos.py file.
    """
    __slots__ = ['name', 'eras']

    def __init__(self, arg):
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        eras = [ZoneEraCooked(i) for i in arg['eras']]
        self.name = arg['name']
        self.eras = eras


class ZoneEraCooked:
    """Internal representation of the ZoneEra dictionary stored in the
    zone_infos.py file.
    """
    # yapf: disable
    __slots__ = [
        'offsetSeconds',  # (int) offset from UTC/GMT in seconds
        'zonePolicy',  # (ZonePolicyCooked or str) ZonePolicyCooked if 'RULES'
                       # field is a named policy, otherwise '-' or ':'
        'rulesDeltaSeconds',  # (int) delta offset from UTC in seconds
                              # if zonePolicy == ':'. Always 0 if zonePolicy is
                              # '-'.
        'format',  # (string) abbreviation format (e.g. P%sT, E%sT, GMT/BST)
        'untilYear',  # (int) MAX_UNTIL_YEAR means 'max'
        'untilMonth',  # (int) 1-12
        'untilDay',  # (int) 1-31
        'untilSeconds',  # (int) untilTime converted into total seconds
        'untilTimeModifier',  # (char) '', 's', 'w', 'u'
    ]
    # yapf: enable

    def __init__(self, arg):
        """Create a ZoneEraCooked from a dict in zone_infos.py. The 'zonePolicy'
        will be another 'dict', which needs to be converted to a
        ZonePolicyCooked object.
        """
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        for s in self.__slots__:
            setattr(self, s, None)

        for key, value in arg.items():
            if key == 'zonePolicy':
                if isinstance(value, str):
                    setattr(self, key, value)
                elif isinstance(value, dict):
                    setattr(self, key, ZonePolicyCooked(value))
                else:
                    raise Exception('zonePolicy value must be str or dict')
            else:
                setattr(self, key, value)

    @property
    def policyName(self):
        """Return the human-readable name of the zone policy used by
        this zoneEra (i.e. value of RULES column). Will be in one of 3 states:
        '-', ':' or a reference
        """
        if self.zonePolicy in ['-', ':']:
            return self.zonePolicy
        else:
            return self.zonePolicy.name


class ZonePolicyCooked:
    """Internal representation of a ZonePolicy dictionary in the
    zone_policies.py output file.
    """
    __slots__ = ['name', 'rules']

    def __init__(self, arg):
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        rules = [ZoneRuleCooked(i) for i in arg['rules']]
        self.name = arg['name']
        self.rules = rules


class ZoneRuleCooked:
    """Internal representation of a ZoneRule dictionary in the zone_policies.py
    output file.
    """
    __slots__ = [
        'fromYear',  # (int) from year
        'toYear',  # (int) to year, 1 to MAX_YEAR (9999) means 'max'
        'inMonth',  # (int) month index (1-12)
        'onDayOfWeek',  # (int) 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
        'onDayOfMonth',  # (int) (1-31), 0={last dayOfWeek match}
        'atSeconds',  # (int) atTime in seconds since 00:00:00
        'atTimeModifier',  # (char) 's', 'w', 'u'
        'deltaSeconds',  # (int) offset from Standard time in seconds
        'letter',  # (str) Usually ('D', 'S', '-'), but sometimes longer
                   # (e.g. WAT, CAT, DD, +00, +02, CST).
    ]

    def __init__(self, arg):
        """Create a ZoneRuleCooked from a dict in zone_infos.py.
        """
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        for s in self.__slots__:
            setattr(self, s, None)

        for key, value in arg.items():
            setattr(self, key, value)


class ZoneMatch:
    """A version of ZoneEra that overlaps with the [start, end) interval of
    interest. The interval is usually a 14-month interval that begins a month
    before the year of interest, and extends a month after the year of interest.
    """
    __slots__ = [
        'startDateTime',  # (DateTuple) the untilTime of the previous ZoneEra
        'untilDateTime',  # (DateTuple) the untilTime of the current ZoneEra
        'zoneEra',  # (ZoneEra) the ZoneEra corresponding to this match
    ]

    def __init__(self, arg):
        for s in self.__slots__:
            setattr(self, s, None)
        if isinstance(arg, dict):
            for key, value in arg.items():
                setattr(self, key, value)
        elif isinstance(arg, ZoneMatch):
            for s in ZoneMatch.__slots__:
                setattr(self, s, getattr(arg, s))

    def copy(self):
        result = self.__class__.__new__(self.__class__)
        for s in self.__slots__:
            setattr(result, s, getattr(self, s))
        return result

    def update(self, arg):
        for key, value in arg.items():
            setattr(self, key, value)

    def __repr__(self):
        return (
            'ZoneMatch(' + 'start: %s; ' + 'until: %s; ' + 'policyName: %s)'
        ) % (date_tuple_to_string(self.startDateTime),
             date_tuple_to_string(self.untilDateTime),
             self.zoneEra.policyName)


class Transition:
    """A description of a potential change in DST offset. It can come from
    a number of sources:
        1) An instance of a ZoneRule that was referenced by the RULES column,
        instantiated for the given year, which then determines the start date
        and until date.
        2) A boundary between one ZoneEra and the next ZoneEra.
        3) A ZoneRule that has been shifted to the boundary of a ZoneEra.
    """
    __slots__ = [
        # Copied from ZoneMatch
        'startDateTime',  # (DateTuple), replaced with actual start time
        'untilDateTime',  # (DateTuple), replaced with actual until time
        'zoneEra',  # (ZoneEra)

        # Added for simple Match and named Match.
        'originalTransitionTime',  # (DateTuple) transition time before shifting
        'transitionTime',  # (DateTuple) 'w' time
        'transitionTimeS',  # (DateTuple) 's' time
        'transitionTimeU',  # (DateTuple) 'u' time
        'abbrev',  # (str) abbreviation
        'startEpochSecond',  # (int) the starting time in epoch seconds

        # Added for named Match.
        'zoneRule',  # (ZoneRule or None) Defined for named Match.
    ]

    def __init__(self, arg):
        for s in self.__slots__:
            setattr(self, s, None)
        if isinstance(arg, dict):
            for key, value in arg.items():
                setattr(self, key, value)
        elif isinstance(arg, ZoneMatch):
            for s in ZoneMatch.__slots__:
                setattr(self, s, getattr(arg, s))

    @property
    def format(self):
        return self.zoneEra.format  # (str)

    @property
    def offsetSeconds(self):
        return self.zoneEra.offsetSeconds  # (int)

    @property
    def letter(self):
        return self.zoneRule.letter if self.zoneRule else ''

    @property
    def deltaSeconds(self):
        return self.zoneRule.deltaSeconds if self.zoneRule \
            else self.zoneEra.rulesDeltaSeconds

    def copy(self):
        result = self.__class__.__new__(self.__class__)
        for s in self.__slots__:
            setattr(result, s, getattr(self, s))
        return result

    def update(self, arg):
        for key, value in arg.items():
            setattr(self, key, value)

    def __repr__(self):
        sepoch = self.startEpochSecond if self.startEpochSecond else 0
        policy_name = self.zoneEra.policyName
        offset_seconds = self.offsetSeconds
        delta_seconds = self.deltaSeconds
        format = self.format
        abbrev = self.abbrev if self.abbrev else ''

        # yapf: disable
        if policy_name in ['-', ':']:
            return ('Transition('
                + 'transition: %s; '
                + 'start: %s; '
                + 'until: %s; '
                + 'sepoch: %d; '
                + 'policy: %s; '
                + '%s; '
                + 'format: %s; '
                + 'abbrev: %s)') % (
                date_tuple_to_string(self.transitionTime),
                date_tuple_to_string(self.startDateTime),
                date_tuple_to_string(self.untilDateTime),
                sepoch, policy_name,
                to_utc_string(offset_seconds, delta_seconds),
                format, abbrev)
        else:
            delta_seconds = self.deltaSeconds
            letter = self.letter
            zone_rule = self.zoneRule
            zone_rule_from = zone_rule.fromYear
            zone_rule_to = zone_rule.toYear

            if self.originalTransitionTime:
                return ('Transition(transition: %s; '
                     + 'start: %s; '
                     + 'until: %s; '
                     + 'orig: %s; '
                     + 'sepoch: %d; '
                     + 'policy: %s[%d,%d]; '
                     + '%s; '
                     + 'format: %s(%s); '
                     + 'abbrev: %s)') % (
                     date_tuple_to_string(self.transitionTime),
                     date_tuple_to_string(self.startDateTime),
                     date_tuple_to_string(self.untilDateTime),
                     date_tuple_to_string(self.originalTransitionTime),
                     sepoch, policy_name, zone_rule_from, zone_rule_to,
                     to_utc_string(offset_seconds, delta_seconds),
                     format, letter, abbrev)
            else:
                return ('Transition(transition: %s; '
                    + 'start: %s; '
                    + 'until: %s; '
                    + 'sepoch: %d; '
                    + 'policy: %s[%d,%d]; '
                    + '%s; '
                    + 'format: %s(%s); '
                    + 'abbrev: %s)') % (
                    date_tuple_to_string(self.transitionTime),
                    date_tuple_to_string(self.startDateTime),
                    date_tuple_to_string(self.untilDateTime),
                    sepoch, policy_name, zone_rule_from, zone_rule_to,
                    to_utc_string(offset_seconds, delta_seconds),
                    format, letter, abbrev)
            # yapf: enable


class ZoneSpecifier:
    """Extract DST transition information for a given ZoneInfo.

    Usage:
        zone_specifier = ZoneSpecifier(zone_info [, viewing_months, debug])

        # Validate matches and transitions
        (matches, transitions) = zone_specifier.get_matches_and_transitions(
            args.year)
        print_matches_and_transitions(matches, transitions)

        # Get (offset_seconds, dst_seconds, abbrev) for an epoch_seconds.
        (offset_seconds, dst_seconds, abbrev) = \
                zone_specifier.get_timezone_info_from_seconds(epoch_seconds)

        # Get (offset_seconds, dst_seconds, abbrev) for a datetime.
        (offset_seconds, dst_seconds, abbrev) = \
                zone_specifier.get_timezone_info_from_datetime(dt)

    Note:
        The viewing_months determines the month interval to use to calculate
        the transitions.
        * 13 = [year-Jan, (year+1)-Feb) (buggy)
        * 14 = [(year-1)-Dec, (year+1)-Feb) (works)
        * 36 = [(year-1)-Jan, (year+2)-Jan) (buggy)
    """

    # Sentinel ZoneEra that represents the earliest zone era.
    ZONE_ERA_ANCHOR = ZoneEraCooked({
        'untilYear': MIN_YEAR,
        'untilMonth': 1,
        'untilDay': 1,
        'untilSeconds': 0,
        'untilTimeModifier': 'w'
    })

    def __init__(self, zone_info_data, viewing_months=14, debug=False):
        """zone_info_data map is one of the ZONE_INFO_xxx constants from
        zone_infos.py. It can contain a reference to a zone_policy_data map. We
        need to convert these into ZoneEraCooked and ZoneRuleCooked classes.
        """
        self.zone_info = ZoneInfoCooked(zone_info_data)
        self.viewing_months = viewing_months

        # Used by init_*() to indicate the current year of interest.
        self.year = 0

        # List of ZoneMatch, i.e. ZoneEra which match the interval of interest.
        self.matches = []

        # List of matching Transition objects.
        self.transitions = []

        self.debug = debug

    def get_matches_and_transitions(self, year):
        """Returns a tuple of (matches, transitions). Used by validator.py
        for validation.
        """
        self.init_for_year(year)
        return (self.matches, self.transitions)

    def get_transition_from_seconds(self, epoch_seconds):
        """Return Transition for the given epoch_seconds.
        """
        self.init_for_second(epoch_seconds)
        return self.find_transition_from_seconds(epoch_seconds)

    def get_transition_from_datetime(self, dt):
        """Return Transition for the given datetime.
        """
        self.init_for_year(dt.year)
        return self.find_transition_from_datetime(dt)

    def get_timezone_info_from_seconds(self, epoch_seconds):
        """Return a tuple of (offset_seconds, dst_seconds, abbrev).
        The total UTC offset is (offset_seconds + dst_seconds).
        """
        self.init_for_second(epoch_seconds)
        transition = self.find_transition_from_seconds(epoch_seconds)
        return self.timezone_info_from_transition(transition)

    def get_timezone_info_from_datetime(self, dt):
        """Return a tuple of (offset_seconds, dst_seconds, abbrev) for datetime.
        """
        self.init_for_year(dt.year)
        transition = self.find_transition_from_datetime(dt)
        if not transition:
            return (None, None, None)
        else:
            return self.timezone_info_from_transition(transition)

    # The following methods are internal methods.

    def init_for_second(self, epoch_seconds):
        """Initialize the Transitions from the given epoch_seconds.
        """
        ldt = datetime.utcfromtimestamp(
            epoch_seconds + SECONDS_SINCE_UNIX_EPOCH)
        self.init_for_year(ldt.year)

    def init_for_year(self, year):
        """Initialize the Transitions for the year.
        """
        # Check if cache filled
        if self.year == year:
            return

        if self.debug:
            logging.info('init_for_year(): year: %d' % year)
        self.year = year

        if self.viewing_months == 13:
            start_ym = YearMonthTuple(year, 1)
            until_ym = YearMonthTuple(year + 1, 2)
        elif self.viewing_months == 14:
            start_ym = YearMonthTuple(year - 1, 12)
            until_ym = YearMonthTuple(year + 1, 2)
        elif self.viewing_months == 36:
            start_ym = YearMonthTuple(year - 1, 1)
            until_ym = YearMonthTuple(year + 2, 1)
        else:
            raise Exception('Unsupported viewing_months: %d' %
                self.viewing_months)

        if self.debug:
            logging.info('==== Finding matches')
        self.matches = self.find_matches(start_ym, until_ym)

        if self.debug:
            logging.info('==== Finding (raw) transitions')
        self.transitions = self.find_transitions(
            self.matches, start_ym, until_ym)
        if self.debug:
            print_matches_and_transitions(self.matches, self.transitions)

        # Some transitions from simple match may be in 's' or 'u', so convert
        # to 'w'.
        if self.debug:
            logging.info('==== Fixing transitions times')
        fix_transition_times(self.transitions)
        if self.debug:
            print_matches_and_transitions(self.matches, self.transitions)

        if self.debug:
            logging.info('==== Generating start and until times')
        generate_start_until_times(self.transitions)
        if self.debug:
            print_matches_and_transitions(self.matches, self.transitions)

        if self.debug:
            logging.info('==== Calculating abbreviations')
        calc_abbrev(self.transitions)
        if self.debug:
            print_matches_and_transitions(self.matches, self.transitions)

    def timezone_info_from_transition(self, transition):
        return (transition.offsetSeconds, transition.deltaSeconds,
            transition.abbrev)

    def find_transition_from_seconds(self, epoch_seconds):
        """Return the matching transition, or None if not found.
        """
        matching_transition = None
        for transition in self.transitions:
            if transition.startEpochSecond <= epoch_seconds:
                matching_transition = transition
            elif transition.startEpochSecond > epoch_seconds:
                break
        return matching_transition

    def find_transition_from_datetime(self, dt):
        """Return the matching transition matching the local datetime 'dt',
        or None if not found.
        """
        matching_transition = None
        secs = hms_to_seconds(dt.hour, dt.minute, dt.second)
        dt_time = DateTuple(y=dt.year, m=dt.month, d=dt.day, ss=secs, f='w')
        for transition in self.transitions:
            start_time = transition.startDateTime
            until_time = transition.untilDateTime
            if start_time <= dt_time and dt_time < until_time:
                return transition
        return None

    def find_matches(self, start_ym, until_ym):
        """Find the Zone Eras which overlap [start_ym, until_ym).
        Experimentation shows that [start_ym, until_ym) must include the Dec
        before the target year, and Jan following the target year. Otherwise, we
        miss transitions that happens in the previous year that affects the
        transition properties on Jan 1 of the target year.
        """
        zone_eras = self.zone_info.eras
        prev_era = self.ZONE_ERA_ANCHOR
        matches = []
        for zone_era in zone_eras:
            if era_overlaps_interval(prev_era, zone_era, start_ym, until_ym):
                match = create_match(prev_era, zone_era)
                if self.debug:
                    logging.info('==== find_matches(): %s' % match)
                matches.append(match)
            prev_era = zone_era
        return matches

    def find_transitions(self, matches, start_ym, until_ym):
        """Find the relevant transitions from the matching ZoneEras, for the
        interval [start_ym, until_ym).
        """
        transitions = []
        for match in matches:
            transitions.extend(
                self.find_transitions_from_match(start_ym, until_ym, match))
        return transitions

    def find_transitions_from_match(self, start_ym, until_ym, match):
        """Find all transitions of match from [start_ym, until_ym).

        We generate slightly over one year's worth because we are caught in a
        Catch-22 situation. When trying to determine the UTC offsets, we need to
        convert a epochSecond to the local DateTime. If we knew the local year
        of the epochSeconds when converted to the local DateTime, then we would
        need only the Transitions of the given local year. However, the
        epochSeconds could convert to Dec 31 or Jan 1 in UTC timezone, which
        means that the local year could shift to the next or previous year in
        the local time zone. But we don't know the local time zone offset until
        we generate the Transitions of the local year, and we don't know the
        local year until we generate the Transitions. To get around this
        problem, we generate the transitions for the year prior and the year
        after the UTC year.
        """
        zone_era = match.zoneEra
        zone_policy = zone_era.zonePolicy

        match = calc_effective_match(start_ym, until_ym, match)
        if self.debug:
            logging.info(
                '==== find_transitions_from_match(): effective match: %s' %
                match)
        if zone_policy in ['-', ':']:
            return self.find_transitions_from_simple_match(match)
        else:
            return self.find_transitions_from_named_match(match)

    def find_transitions_from_simple_match(self, match):
        """The zonePolicy is '-' or ':' then the Zone Era itself defines the UTC
        offset and the abbreviation.
        """
        zone_era = match.zoneEra
        transition = Transition(match)
        transition.update({
            'transitionTime': match.startDateTime,
        })
        return [transition]


#    def find_transitions_from_named_match_old(self, match):
#        """
#        Find the relevant transitions of the named policy in the Match interval
#        [startDateTime, untilDateTime).
#
#        If a Zone Era use a named Match before any transition is defined then
#        we must follow the special instructions given in
#        https://data.iana.org/time-zones/tz-how-to.html, where we use the
#        earliest transition and shift it back in time to the starting point of
#        the named Match, but clobber the SAVE to be 0 while keeping the LETTER.
#
#        The algorithm is the following:
#
#        * Loop for each Rule entry in the Zone policy given by the Match:
#            * Obtain the candidate years and the corresponding Transitiosn
#            * For each Transition:
#                * If Transition occurs >= Match.until, ignore it.
#                * If Transition occurs within [Match.start, Match.until):
#                    * Add to the Transitions collection.
#                    * If Transition == Match.start:
#                        * Set startTransitionFound flag.
#                * Else Transition is < Match.start:
#                    * If not startTransitionFound:
#                        * Nominate as latest prior Transition.
#        * If not startTransitionFound:
#            * If latest prior transition exists:
#                * Shift the prior Transition to Match.start
#                * Add to Transitions collection.
#        """
#        zone_era = match.zoneEra
#        zone_policy = zone_era.zonePolicy
#        rules = zone_policy.rules
#        start_dt = match.startDateTime
#        start_y = start_dt.y
#
#        # If the until datetime is exactly Jan 1 00:00, then we don't need to
#        # consider a Transition in untilYear. To be sure, we would have to
#        # verify that all smaller components (hour, minute, second) are also
#        # exactly zero. We can be a little lazy and just assume that these
#        # smaller components are non-zero, so we check the entire untilYear. If
#        # the Transition falls outside of the matched ZoneEra, then it will get
#        # properly filtered out in process_transition().
#        until_dt = match.untilDateTime
#        end_y = until_dt.y
#
#        # For each Rule, process the Transition for each whole year within
#        # the given 'match'.
#        results = {
#            'transitions': []
#        }
#        for rule in rules:
#            from_year = rule.fromYear
#            to_year = rule.toYear
#            years = get_candidate_years(from_year, to_year, start_y, end_y)
#            for year in years:
#                transition = create_transition_for_year(year, rule, match)
#                if transition:
#                    process_transition(match, transition, results)
#
#        # Add the resulting transitions that overlap with the match.
#        self.transitions.extend(results['transitions'])
#
#        # Add the latest prior transition
#        if not results.get('startTransitionFound'):
#            prior_transition = results.get('latestPriorTransition')
#            if not prior_transition:
#                logging.error(
#                    "Zone '%s'; year '%04d': No prior transition found!",
#                    self.zone_info['name'], self.year)
#                sys.exit(1)
#
#            prior_transition = prior_transition.copy()
#            original_time = prior_transition.transitionTime
#            prior_transition.transitionTime = match.startDateTime
#            prior_transition.originalTransitionTime = original_time
#            self.transitions.append(prior_transition)

    def find_transitions_from_named_match(self, match):
        """Find the transitions of the named effective ZoneMatch. Only the year
        component of the match.startDateTime and match.untilDateTime are used,
        and the month component is ignored. This causes --viewing_months=13 to
        break because the boundary is at the year transition.
        """
        zone_era = match.zoneEra
        zone_policy = zone_era.zonePolicy
        rules = zone_policy.rules

        if self.debug:
            logging.info('==== Get candidate transitions')
        transitions = get_candidate_transitions(match, rules)
        transitions = sort_transitions(transitions)
        check_transitions_sorted(transitions)
        if self.debug:
            print_transitions(transitions)

        if self.debug:
            logging.info('==== Fix transition times')
        fix_transition_times(transitions)
        check_transitions_sorted(transitions)
        if self.debug:
            print_transitions(transitions)

        if self.debug:
            logging.info('==== Select active transitions')
        transitions = select_active_transitions(transitions, match)
        if transitions == None:
            logging.error("Zone '%s'; year '%04d': No prior transition found!",
                          self.zone_info['name'], self.year)
            sys.exit(1)
        if self.debug:
            print_transitions(transitions)

        # Second sorting necessary because the "most recent prior" is placed at
        # the end of the list.
        if self.debug:
            logging.info('==== Final check for sorted transitions')
        transitions = sort_transitions(transitions)
        check_transitions_sorted(transitions)

        return transitions


def convert_data_to_objects(zi):
    """Convert the dictionary of zone info, zone era, zone policy and zone rule
    information into ZoneInfo, ZoneEra, ZonePolicy and ZoneRule objects.
    """


def create_match(prev_era, zone_era):
    """Create the Zone Match object for the given Zone Era.
        * ZoneMatch.startTime is prev_era.untilTime
        * ZoneMatch.policy_name is '-', ':' or the string name of ZonePolicy
    """
    # The subtlety here is that the prev_era's 'until datetime' is expressed
    # using the UTC offset of the *previous* era, not the current era. This is
    # probably good enough for sorting, assuming we don't have 2 DST transitions
    # in a single day. See fix_transition_times() which normalizes these start
    # times to the wall time uniformly.
    start_date_time = DateTuple(
        y=prev_era.untilYear,
        m=prev_era.untilMonth,
        d=prev_era.untilDay,
        ss=prev_era.untilSeconds,
        f=prev_era.untilTimeModifier)
    until_date_time = DateTuple(
        y=zone_era.untilYear,
        m=zone_era.untilMonth,
        d=zone_era.untilDay,
        ss=zone_era.untilSeconds,
        f=zone_era.untilTimeModifier)
    return ZoneMatch({
        'startDateTime': start_date_time,
        'untilDateTime': until_date_time,
        'zoneEra': zone_era
    })


def select_active_transitions(transitions, match):
    """Select those Transitions which overlap with the effective ZoneMatch
    interval which may not be at year boundary. Also select the latest prior
    transition before the given ZoneMatch, shifting the transition time to the
    start of the ZoneMatch. The returned array of transitions is likely to be
    unsorted again, since the latest prior transition is added to the end.
    """

    # Commulative results of process_transition()
    results = {
        'startTransitionFound': None,
        'latestPriorTransition': None,
        'transitions': []
    }

    # Categorize each transition
    for transition in transitions:
        process_transition(match, transition, results)
    transitions = results['transitions']

    # Add the latest prior transition. Adding this at the end of the array
    # will likely cause the transitions to become unsorted, requiring another
    # sorting pass.
    if not results.get('startTransitionFound'):
        prior_transition = results.get('latestPriorTransition')
        if not prior_transition:
            return None # should not happen, indicate an error

        # Adjust the transition time to be the start of the ZoneMatch.
        prior_transition = prior_transition.copy()
        original_time = prior_transition.transitionTime
        prior_transition.transitionTime = match.startDateTime
        prior_transition.originalTransitionTime = original_time
        transitions.append(prior_transition)

    return transitions


def generate_start_until_times(transitions):
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

    All transitionTimes ought to be in 'w' mode by the time this is called.
    """

    # As before, bootstrap the prev transition with the first transition
    # so that we have a UTC offset to work with.
    prev = transitions[0]
    is_after_first = False
    for transition in transitions:
        tt = transition.transitionTime

        # 1) Update the 'untilDateTime' of the previous Transition.
        if is_after_first:
            prev.untilDateTime = tt

        # 2) Calculate the current startDateTime by shifting the transition time
        # into the current UTC offset. This algorithm should be able to handle
        # transition time of 24:00 (or even 25:00) of the previous day.
        secs = (tt.ss - prev.offsetSeconds - prev.deltaSeconds +
                transition.offsetSeconds + transition.deltaSeconds)
        #if secs < 0 or secs >= 24 * 60 * 60:
        #   (h, m, s) = seconds_to_hms(secs)
        #    logging.info(
        #        "Zone '%s': Transition startDateTime shifted into "
        #        + "a different day: (%02d:%02d:%02d)",
        #        self.zone_info['name'], h, m, s)
        st = datetime(tt.y, tt.m, tt.d, 0, 0, 0)
        st += timedelta(seconds=secs)
        secs = hms_to_seconds(st.hour, st.minute, st.second)
        transition.startDateTime = DateTuple(
            y=st.year, m=st.month, d=st.day, ss=secs, f=tt.f)

        # 3) The epochSecond of the 'transitionTime' is determined by the
        # UTC offset of the *previous* Transition. However, the
        # transitionTime represent by an illegal date (e.g. 24:00). So, it
        # is better to use the properly normalized startDateTime (calculated
        # above) with the *current* UTC offset.
        utc_offset_seconds = transition.offsetSeconds \
            + transition.deltaSeconds
        z = timezone(timedelta(seconds=utc_offset_seconds))
        dt = st.replace(tzinfo=z)
        epoch_second = int((dt - ACETIME_EPOCH).total_seconds())
        transition.startEpochSecond = epoch_second

        prev = transition
        is_after_first = True

    # Finally, fix the last transition's until time
    (udt, udts, udtu) = expand_date_tuple(transition.untilDateTime,
                                          transition.offsetSeconds,
                                          transition.deltaSeconds)
    transition.untilDateTime = udt


def get_candidate_transitions(match, rules):
    """Get the list of candidate transitions from the list of 'rules' which
    overlap the whole years [start_y, end_y] (inclusive)) defined by the given
    'match' ZoneEra.
    """
    # Use whole years because 'rules' define repetitive transitions
    # using whole years.
    start_y = match.startDateTime.y
    end_y = match.untilDateTime.y

    transitions = []
    for rule in rules:
        from_year = rule.fromYear
        to_year = rule.toYear
        years = get_candidate_years(from_year, to_year, start_y, end_y)
        for year in years:
            transitions.append(create_transition_for_year(year, rule, match))
    return transitions


def sort_transitions(transitions):
    """Sort the transitions according to (y, m, d), ignoring ss and
    modifier. This assumes that only one Rule in a single day.
    """

    def date_tuple_to_sort_key(t):
        return (t.y, t.m, t.d)

    try:
        ts = sorted(
            transitions,
            key=lambda x: date_tuple_to_sort_key(x.transitionTime))
    except e as Exception:
        logging.exception('Exception caught: %s' % e)
        print_transitions(transitions)
        sys.exit(1)
    return ts

def check_transitions_sorted(transitions):
    """Check transitions are sorted.
    """
    prev = None
    for transition in transitions:
        if not prev:
            prev = transition
            continue
        if prev.transitionTime > transition.transitionTime:
            print_transitions(transitions)
            raise Exception('Transitions not sorted')


def create_transition_for_year(year, rule, match):
    """Create the transition from the given 'rule' for the given 'year'.
    (Don't need to check if it overlaps with the given 'match' since that is
    done in process_transition()). Return None if 'year' does not overlap
    with the [from, to] of the rule. The Transition object is a replica of
    the underlying Match object, with additional bookkeeping info.
    """
    # Check if [Rule.from, Rule.to] overlaps with year.
    from_year = rule.fromYear
    to_year = rule.toYear
    if year < from_year or to_year < year:
        return None

    transition_time = get_transition_time(year, rule)
    zone_era = match.zoneEra
    transition = Transition(match)
    transition.update({
        'transitionTime': transition_time,
        'zoneRule': rule,
    })
    return transition


def fix_transition_times(transitions):
    """Convert the transtion['transitionTime'] to the wall time ('w') of
    the previous rule's time offset. The Transition time comes from either:
        1) The UNTIL field of the previous Zone Era entry, or
        2) The (inMonth, onDay, atSeconds) fields of the Zone Rule.

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
    # 100% correct with respect to the TZ Database but it will be good
    # enough for the first transition that we care about.
    prev = transitions[0].copy()
    for transition in transitions:
        (transition.transitionTime, transition.transitionTimeS,
            transition.transitionTimeU) = \
            expand_date_tuple(transition.transitionTime,
                prev.offsetSeconds, prev.deltaSeconds)
        prev = transition


def expand_date_tuple(dt, offset_seconds, delta_seconds):
    """Convert 's', 'u', or 'w' time into the other 2 versions using the given
    base UTC offset and the delta DST offset. Return a tuple of
    (wall, standard, utc) date tuples.
    """
    delta_seconds = delta_seconds if delta_seconds else 0
    offset_seconds = offset_seconds if offset_seconds else 0

    if dt.f == 'w':
        dtw = dt
        dts = DateTuple(
            y=dt.y, m=dt.m, d=dt.d, ss=dtw.ss - delta_seconds, f='s')
        ss = dtw.ss - delta_seconds - offset_seconds
        dtu = DateTuple(y=dt.y, m=dt.m, d=dt.d, ss=ss, f='u')
    elif dt.f == 's':
        dts = dt
        dtw = DateTuple(
            y=dt.y, m=dt.m, d=dt.d, ss=dts.ss + delta_seconds, f='w')
        dtu = DateTuple(
            y=dt.y, m=dt.m, d=dt.d, ss=dts.ss - offset_seconds, f='u')
    elif dt.f == 'u':
        dtu = dt
        ss = dtu.ss + delta_seconds + offset_seconds
        dtw = DateTuple(y=dtu.y, m=dtu.m, d=dtu.d, ss=ss, f='w')
        dts = DateTuple(
            y=dtu.y, m=dtu.m, d=dtu.d, ss=dtu.ss + offset_seconds, f='s')
    else:
        logging.error("Unrecognized Rule.AT suffix '%s'; date=%s", dt.f, dt)
        sys.exit(1)

    dtw = normalize_date_tuple(dtw)
    dts = normalize_date_tuple(dts)
    dtu = normalize_date_tuple(dtu)

    return (dtw, dts, dtu)


def normalize_date_tuple(tt):
    """Return the normalized DateTuple where the dt.ss could be negative or
    greater than 24h.
    """
    if tt.y == MIN_YEAR:
        return DateTuple(y=MIN_YEAR, m=1, d=1, ss=0, f=tt.f)

    try:
        st = datetime(tt.y, tt.m, tt.d, 0, 0, 0)
        delta = timedelta(seconds=tt.ss)
        st += delta
        secs = hms_to_seconds(st.hour, st.minute, st.second)
        return DateTuple(y=st.year, m=st.month, d=st.day, ss=secs, f=tt.f)
    except:
        logging.error('Invalid datetime: %s + %s', st, delta)
        sys.exit(1)


def calc_abbrev(transitions):
    """Calculate the time zone abbreviations for each Transition.
    There are several cases:
        1) 'format' contains 'A/B', meaning 'A' for standard time, and 'B'
            for DST time.
        2) 'format' contains a %s, which substitutes the 'letter'
            2a) If 'letter' is '-', replace with nothing.
            2b) The 'format' could be just a '%s'.
    """
    for transition in transitions:
        format = transition.format
        delta_seconds = transition.deltaSeconds

        index = format.find('/')
        if index >= 0:
            if delta_seconds == 0:
                abbrev = format[:index]
            else:
                abbrev = format[index + 1:]
        elif format.find('%s') >= 0:
            letter = transition.letter
            if letter == '-': letter = ''
            abbrev = format % letter
        else:
            abbrev = format

        transition.abbrev = abbrev


def process_transition(match, transition, results):
    """Process the given transition, checking the following situations:
    1) If the Transition is outside the time range of the effective ZoneEra,
    ignore the transition.
    2) If the Transition is within the matching effective ZoneEra, it is added
    to the map at results['transitions'].
    2a) If the Transition occurs at the very start of the ZoneEra, then
    set the flag "startTransitionFound" to true.
    3) If the Transition is earlier than the effective ZoneEra, then add
    it to the 'latestPriorTransition' if it is the largest prior transition.

    The 'results' is a map that keeps track of the processing, and contains:
        {
            'startTransitionFound': bool,
            'latestPriorTransition': transition,
            'transitions': {}
        }

    where:
        * If transition >= match.until:
            * do nothing
        * If transition within match:
            * add transition to results['transitions']
            * if transition == match.start
                * set results['startTransitionFound'] = True
        * If transition < match:
            * if not startTransitionFound:
                * set results['latestPriorTransition'] = latest
    """
    # Determine if the transition falls within the effective match range.
    transition_compared_to_match = compare_transition_to_match(
        transition, match)
    if transition_compared_to_match == 2:
        return
    elif transition_compared_to_match in [0, 1]:
        results['transitions'].append(transition)
        if transition_compared_to_match == 0:
            results['startTransitionFound'] = True
    else:  # transition_compared_to_match < -1:
        # If a Transition exists on the start bounary of the ZoneMatch,
        # then we don't need to search for the latest prior.
        if results.get('startTransitionFound'):
            return

        # Determine the latest prior transition
        latest_prior_transition = results.get('latestPriorTransition')
        if not latest_prior_transition:
            results['latestPriorTransition'] = transition
        else:
            transition_time = transition.transitionTime
            if transition_time > latest_prior_transition.transitionTime:
                results['latestPriorTransition'] = transition


def get_candidate_years(from_year, to_year, start_year, end_year):
    """Return the array of years within the Rule's [from_year, to_year] range
    which should be evaluated to obtain the transitions necessary for the
    matched ZoneEra that spans [start_year, end_year].
        1) Include all years which overlap [start_year, end_year].
        2) Add the latest year prior to [start_year]. This is guaranteed to
        exists because we added an anchor rule at year 0 for those zone policies
        that need it.
    If [start_year, end_year] spans a 3-year interval (which will always
    be the case), then the maximum number of elements in 'years' will be 4.
    """
    years = set()
    for year in range(start_year, end_year + 1):
        if from_year <= year and year <= to_year:
            years.add(year)

    if from_year < start_year:
        if to_year < start_year:
            years.add(to_year)
        else:
            years.add(start_year - 1)
    return years


def calc_effective_match(start_ym, until_ym, match):
    """Generate a version of match which overlaps the interval
    [start_ym, until_ym).
    """
    start_date_time = match.startDateTime
    if start_date_time < DateTuple(
            y=start_ym.y, m=start_ym.m, d=1, ss=0, f='w'):
        start_date_time = DateTuple(
            y=start_ym.y, m=start_ym.m, d=1, ss=0, f='w')

    until_date_time = match.untilDateTime
    if until_date_time > DateTuple(
            y=until_ym.y, m=until_ym.m, d=1, ss=0, f='w'):
        until_date_time = DateTuple(
            y=until_ym.y, m=until_ym.m, d=1, ss=0, f='w')

    eff_match = match.copy()
    eff_match.startDateTime = start_date_time
    eff_match.untilDateTime = until_date_time
    return eff_match


def compare_transition_to_match(transition, match):
    """Determine if transition_time applies to given range of the match.
    Return:
        * -1 if less than match
        * 0 if equal to match_start
        * 1 if within match,
        * 2 if greater than match
    """
    match_start = match.startDateTime
    if match_start.f == 'w':
        transition_time = transition.transitionTime
    elif match_start.f == 's':
        transition_time = transition.transitionTimeS
    elif match_start.f == 'u':
        transition_time = transition.transitionTimeU
    else:
        raise Exception("Unknown modifier: %s" % match_start)
    if transition_time < match_start:
        return -1
    if transition_time == match_start:
        return 0

    match_until = match.untilDateTime
    if match_until.f == 'w':
        transition_time = transition.transitionTime
    elif match_until.f == 's':
        transition_time = transition.transitionTimeS
    elif match_until.f == 'u':
        transition_time = transition.transitionTimeU
    else:
        raise Exception("Unknown modifier: %s" % match_until)
    if match_until <= transition_time:
        return 2

    return 1


def get_transition_time(year, rule):
    """Return the (year, month, day, seconds, modifier) of the Rule in given
    year.
    """
    month = rule.inMonth
    day_of_month = calc_day_of_month(year, month, rule.onDayOfWeek,
                                     rule.onDayOfMonth)
    seconds = rule.atSeconds
    modifier = rule.atTimeModifier
    return DateTuple(y=year, m=month, d=day_of_month, ss=seconds, f=modifier)


def calc_day_of_month(year, month, on_day_of_week, on_day_of_month):
    """Return the actual day of month of expressions such as
    (onDayOfWeek >= onDayOfMonth) or (lastMon).
    """
    if on_day_of_week == 0:
        return on_day_of_month

    if on_day_of_month == 0:
        # lastXxx == (Xxx >= (daysInMonth - 6))
        on_day_of_month = days_in_month(year, month) - 6
    limit_date = date(year, month, on_day_of_month)
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


def era_overlaps_interval(prev_era, era, start_ym, until_ym):
    """Determines if era overlaps the interval [start_ym, until_ym). The start
    date of the current era is represented by the prev_era.UNTIL, so the
    interval of the current era is [start_era, until_era) = [prev_era.UNTIL,
    era.UNTIL). Overlap happens if (start_era < until_ym) and (until_era >
    start_ym).
    """
    return (compare_era_to_year_month(prev_era, until_ym.y, until_ym.m) < 0
            and compare_era_to_year_month(era, start_ym.y, start_ym.m) > 0)


def compare_era_to_year_month(era, year, month):
    """Compare the zone_era with year, returning -1, 0 or 1. The day of month is
    implicitly 1. Ignore the untilTimeModifier suffix. Maybe it's not needed in
    this context?
    """
    if era.untilYear < year:
        return -1
    if era.untilYear > year:
        return 1
    if era.untilMonth < month:
        return -1
    if era.untilMonth > month:
        return 1
    if era.untilDay > 1:
        return 1
    if era.untilSeconds < 0:
        return -1
    if era.untilSeconds > 0:
        return 1
    return 0


def date_tuple_to_string(dt):
    (h, m, s) = seconds_to_hms(dt.ss)
    return '%04d-%02d-%02d %02d:%02d:%02d%s' % (dt.y, dt.m, dt.d, h, m, s,
                                                dt.f)


def to_utc_string(utcoffset, dstoffset):
    return 'UTC%s%s' % (seconds_to_hm_string(utcoffset),
                        seconds_to_hm_string(dstoffset))


def seconds_to_hm_string(secs):
    if secs < 0:
        hms = seconds_to_hms(-secs)
        return '-%02d:%02d' % (hms[0], hms[1])
    else:
        hms = seconds_to_hms(secs)
        return '+%02d:%02d' % (hms[0], hms[1])


EPOCH_DATETIME = datetime(2000, 1, 1, 0, 0, 0)


def print_matches_and_transitions(matches, transitions):
    logging.info('---- Matches:')
    for m in matches:
        logging.info(m)
    logging.info('---- Transitions:')
    for t in transitions:
        logging.info(t)


def print_transitions(transitions):
    for t in transitions:
        logging.info(t)


def main():
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Zone Agent.')
    parser.add_argument(
        '--viewing_months',
        help='Number of months to use for calculations (13, 14, 36)',
        type=int, default=14)
    parser.add_argument(
        '--transition', help='Print the transition instead of timezone info',
        action="store_true")
    parser.add_argument(
        '--debug', help='Print debugging info', action="store_true")
    parser.add_argument('--zone', help='Name of time zone', required=True)
    parser.add_argument('--year', help='Year of interest', type=int)
    parser.add_argument('--date', help='DateTime of interest')
    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    # Find the zone.
    zone_info = ZONE_INFO_MAP.get(args.zone)
    if not zone_info:
        logging.error("Zone '%s' not found", args.zone)
        sys.exit(1)

    # Create the ZoneSpecifier for zone
    zone_specifier = ZoneSpecifier(zone_info, args.viewing_months, args.debug)

    if args.year:
        (matches, transitions) = zone_specifier.get_matches_and_transitions(
            args.year)
        print_matches_and_transitions(matches, transitions)

    elif args.date:
        dt = datetime.strptime(args.date, "%Y-%m-%dT%H:%M")
        if args.transition:
            transition = zone_specifier.get_transition_from_datetime(dt)
            if transition:
                logging.info(transition)
            else:
                logging.error('Transition not found')
        else:
            (offset_seconds, dst_seconds, abbrev) = \
                    zone_specifier.get_timezone_info_from_datetime(dt)
            if not offset_seconds:
                logging.info('Invalid time');
            else:
                logging.info('%s (%s)',
                    to_utc_string(offset_seconds, dst_seconds),
                    abbrev)
    else:
        print("One of --year or --date must be provided")
        sys.exit(1)


if __name__ == '__main__':
    main()
