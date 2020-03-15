#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License
"""
A Python version of the C++ ExtendedZoneSpecifier class to allow easier and
faster iteration of its algorithms. It is too cumbersome and tedious to
experiment and debug the C++ code in the Arduino environment.

The ZoneSpecifier class will normally be used by another class, such as the
TestDataGenerator or the Validator class to determine the DST transitions of a
particular year. However, a command line interface has been exposed for
debugging. See the examples below.

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
import importlib
from datetime import datetime
from datetime import timedelta
from datetime import timezone
from datetime import date
from typing import Any
from typing import Dict
from typing import List
from typing import NamedTuple
from typing import Optional
from typing import TYPE_CHECKING
from typing import Union
from typing import cast

from extractor import MIN_YEAR
from transformer import seconds_to_hms
from transformer import hms_to_seconds
from transformer import calc_day_of_month
from ingenerator import ZoneRule
from ingenerator import ZonePolicy
from ingenerator import ZoneEra
from ingenerator import ZoneInfo

# A datetime representation using seconds instead of h:m:s
DateTuple = NamedTuple('DateTuple', [
    ('y', int),
    ('M', int),
    ('d', int),
    ('ss', int),
    ('f', str),
])

# A tuple of (year, month)
YearMonthTuple = NamedTuple('YearMonthTuple', [
    ('y', int),
    ('M', int),
])

# UTC offset at the current time:
#   * total_offset = utc_offset + dst_offset
#   * utc_offset: seconds
#   * dst_offset: seconds
#   * abbrev
OffsetInfo = NamedTuple('OffsetInfo', [
    ('total_offset', int),
    ('utc_offset', int),
    ('dst_offset', int),
    ('abbrev', str),
])

# Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
# (2000-01-01 00:00:00)
SECONDS_SINCE_UNIX_EPOCH = 946684800

ACETIME_EPOCH = datetime(2000, 1, 1, tzinfo=timezone.utc)


class ZoneRuleCooked:
    """Internal representation of a ZoneRule dictionary in the zone_policies.py
    output file.
    """
    # yapf: disable
    __slots__ = [
        'fromYear',  # (int) from year
        'toYear',  # (int) to year, 1 to MAX_YEAR (9999) means 'max'
        'inMonth',  # (int) month index (1-12)
        'onDayOfWeek',  # (int) 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
        'onDayOfMonth',  # (int) (1-31), 0={last dayOfWeek match}
        'atSeconds',  # (int) atTime in seconds since 00:00:00
        'atTimeSuffix',  # (char) 's', 'w', 'u'
        'deltaSeconds',  # (int) offset from Standard time in seconds
        'letter',  # (str) Usually ('D', 'S', '-'), but sometimes longer
                   # (e.g. WAT, CAT, DD, +00, +02, CST).
    ]
    # yapf: enable

    # Hack because '__slots__' is unsupported by mypy. See
    # https://github.com/python/mypy/issues/5941.
    if TYPE_CHECKING:
        fromYear: int
        toYear: int
        inMonth: int
        onDayOfWeek: int
        onDayOfMonth: int
        atSeconds: int
        atTimeSuffix: str
        deltaSeconds: int
        letter: str

    def __init__(self, arg: ZoneRule):
        """Create a ZoneRuleCooked from a dict in zone_infos.py.
        """
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        for s in self.__slots__:
            setattr(self, s, None)

        for key, value in arg.items():
            setattr(self, key, value)


class ZonePolicyCooked:
    """Internal representation of a ZonePolicy dictionary in the
    zone_policies.py output file.
    """
    __slots__ = ['name', 'rules']

    # Hack because '__slots__' is unsupported by mypy. See
    # https://github.com/python/mypy/issues/5941.
    if TYPE_CHECKING:
        name: str
        rules: List[ZoneRuleCooked]

    def __init__(self, arg: ZonePolicy):
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        rules = [ZoneRuleCooked(i) for i in arg['rules']]
        self.name = arg['name']
        self.rules = rules


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
        'untilTimeSuffix',  # (char) '', 's', 'w', 'u'
    ]
    # yapf: enable

    # Hack because '__slots__' is unsupported by mypy. See
    # https://github.com/python/mypy/issues/5941.
    if TYPE_CHECKING:
        offsetSeconds: int
        zonePolicy: Union['ZonePolicyCooked', str]
        rulesDeltaSeconds: int
        format: str
        untilYear: int
        untilMonth: int
        untilDay: int
        untilSeconds: int
        untilTimeSuffix: str

    def __init__(self, arg: ZoneEra):
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
                    setattr(self,
                        key, ZonePolicyCooked(cast(ZonePolicy, value)))
                else:
                    raise Exception('zonePolicy value must be str or dict')
            else:
                setattr(self, key, value)

    @property
    def policyName(self) -> str:
        """Return the human-readable name of the zone policy used by
        this zoneEra (i.e. value of RULES column). Will be in one of 3 states:
        '-', ':' or a reference
        """
        if self.zonePolicy in ['-', ':']:
            return cast(str, self.zonePolicy)
        else:
            return cast(ZonePolicyCooked, self.zonePolicy).name


class ZoneInfoCooked:
    """Internal representation of a single ZoneInfo dictionary stored in the
    zone_infos.py file.
    """
    __slots__ = ['name', 'eras']

    # Hack because '__slots__' is unsupported by mypy. See
    # https://github.com/python/mypy/issues/5941.
    if TYPE_CHECKING:
        name: str
        eras: List[ZoneEraCooked]

    def __init__(self, arg: ZoneInfo):
        if not isinstance(arg, dict):
            raise Exception('Expected a dict')

        eras = [ZoneEraCooked(i) for i in arg['eras']]
        self.name = arg['name']
        self.eras = eras


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

    # Hack because '__slots__' is unsupported by mypy. See
    # https://github.com/python/mypy/issues/5941.
    if TYPE_CHECKING:
        startDateTime: DateTuple
        untilDateTime: DateTuple
        zoneEra: ZoneEraCooked

    def __init__(self, arg: Dict[str, Any]):
        for s in self.__slots__:
            setattr(self, s, None)
        if isinstance(arg, dict):
            for key, value in arg.items():
                setattr(self, key, value)
        elif isinstance(arg, ZoneMatch):
            for s in ZoneMatch.__slots__:
                setattr(self, s, getattr(arg, s))

    def copy(self) -> 'ZoneMatch':
        result = cast(ZoneMatch, self.__class__.__new__(self.__class__))
        for s in self.__slots__:
            setattr(result, s, getattr(self, s))
        return result

    def update(self, arg: Dict[str, Any]) -> None:
        for key, value in arg.items():
            setattr(self, key, value)

    def __repr__(self) -> str:
        return (
            'ZoneMatch(' + 'start: %s; ' + 'until: %s; ' + 'policyName: %s)'
        ) % (date_tuple_to_string(self.startDateTime),
             date_tuple_to_string(self.untilDateTime), self.zoneEra.policyName)


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
        # The start and until times are initially copied from ZoneMatch, then
        # updated later by _generate_start_until_times().
        #   * The untilDateTime comes from transitionTime of the *next*
        #   Transition.
        #   * The startDateTime is the current transitionTime converted into the
        #   UTC offset of the current Transition.
        'startDateTime',  # replaced with actual start time
        'untilDateTime',  # replaced with actual until time
        'zoneEra',  # (ZoneEra)

        # These are added for simple Match and named Match.
        # For a simple Transition, the transitionTime is the startTime of the
        # ZoneEra. For a named Transition, the transitionTime is the AT field of
        # the corresponding ZoneRule (see _create_transition_for_year()).
        'originalTransitionTime',  # (DateTuple) transition time before shifting
        'transitionTime',  # 'w' time
        'transitionTimeS',  # 's' time
        'transitionTimeU',  # 'u' time
        'abbrev',  # abbreviation
        'startEpochSecond',  # the starting time in epoch seconds

        # Added for named Match.
        'zoneRule',  # Defined for named Match.

        # Flag to indicate if Transition is active or not
        'isActive',  # Transition is inside ZoneMatch and is active
    ]

    # Hack because '__slots__' is unsupported by mypy. See
    # https://github.com/python/mypy/issues/5941.
    if TYPE_CHECKING:
        startDateTime: DateTuple
        untilDateTime: DateTuple
        zoneEra: ZoneEraCooked
        originalTransitionTime: DateTuple
        transitionTime: DateTuple
        transitionTimeS: DateTuple
        transitionTimeU: DateTuple
        abbrev: str
        startEpochSecond: int
        zoneRule: Optional[ZoneRuleCooked]
        isActive: bool

    def __init__(self, arg: ZoneMatch):
        for s in self.__slots__:
            setattr(self, s, None)
        if isinstance(arg, dict):
            for key, value in arg.items():
                setattr(self, key, value)
        elif isinstance(arg, ZoneMatch):
            for s in ZoneMatch.__slots__:
                setattr(self, s, getattr(arg, s))

    @property
    def format(self) -> str:
        return self.zoneEra.format

    @property
    def offsetSeconds(self) -> int:
        return self.zoneEra.offsetSeconds

    @property
    def letter(self) -> str:
        return self.zoneRule.letter if self.zoneRule else ''

    @property
    def deltaSeconds(self) -> int:
        return self.zoneRule.deltaSeconds if self.zoneRule \
            else self.zoneEra.rulesDeltaSeconds

    def copy(self) -> 'Transition':
        result = cast('Transition', self.__class__.__new__(self.__class__))
        for s in self.__slots__:
            setattr(result, s, getattr(self, s))
        return result

    def update(self, arg: Dict[str, Any]) -> None:
        for key, value in arg.items():
            setattr(self, key, value)

    def to_timezone_tuple(self) -> OffsetInfo:
        """Convert a Transition into a OffsetInfo.
        """
        return OffsetInfo(self.offsetSeconds + self.deltaSeconds,
                          self.offsetSeconds, self.deltaSeconds, self.abbrev)

    def __repr__(self) -> str:
        sepoch = self.startEpochSecond if self.startEpochSecond else 0
        policy_name = self.zoneEra.policyName
        offset_seconds = self.offsetSeconds
        delta_seconds = self.deltaSeconds
        format = self.format
        abbrev = self.abbrev if self.abbrev else ''

        # yapf: disable
        if policy_name in ['-', ':']:
            return ('Trans('
                + 'act: %s; '
                + 'tt: %s; '
                + 'st: %s; '
                + 'ut: %s; '
                + 'epch: %d; '
                + 'pol: %s; '
                + '%s; '
                + 'fmt: %s; '
                + 'ab: %s)') % (
                'y' if self.isActive else '-',
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
            zone_rule_from = cast(ZoneRuleCooked, zone_rule).fromYear
            zone_rule_to = cast(ZoneRuleCooked, zone_rule).toYear

            return ('Trans('
                + 'act: %s; '
                + 'tt: %s; '
                + 'st: %s; '
                + 'ut: %s; '
                + 'ot: %s; '
                + 'epch: %d; '
                + 'pol: %s[%d,%d]; '
                + '%s; '
                + 'fmt: %s(%s); '
                + 'ab: %s)') % (
                'y' if self.isActive else '-',
                date_tuple_to_string(self.transitionTime),
                date_tuple_to_string(self.startDateTime),
                date_tuple_to_string(self.untilDateTime),
                date_tuple_to_string(self.originalTransitionTime) \
                    if self.originalTransitionTime else '',
                sepoch, policy_name, zone_rule_from, zone_rule_to,
                to_utc_string(offset_seconds, delta_seconds),
                format, letter, abbrev)
        # yapf: enable


class ZoneSpecifier:
    """Extract DST transition information for a given ZoneInfo. The
    DST transition information can be retrieved using the following methods:

        * get_timezone_info_for_seconds(): get info using epoch_seconds (from
          2000-01-01 00:00:00 UTC)
        * get_timezone_info_for_datetime(): get info using a 'datetime.datetime'
          instance

    The DST transition information is returned as a tuple of (offset_seconds,
    dst_seconds, abbrev) which is valid at the given epoch_seconds or
    'datetime'.

    Both get_timezone_info_for_seconds() and get_timezone_info_for_datetime()
    call init_for_year() using a window size (e.g. 12, 13, 14 or 36 months)
    around the closest 'year' to the given argument. (The 'closest' year could
    be (year-1) if the datetime was on Jan 1 of the following year in UTC time).
    The window size can be specified using the 'viewing_months' parameter in the
    constructor.

    The init_for_year() method calculates the relevant Transitions for the given
    year and caches results. Subsequent queries for different epoch_seconds or
    'datetime' will be efficient if the closest 'year' is the same. See
    init_for_year() for high level explanation of the internal algorithm.

    Usage:
        zone_specifier = ZoneSpecifier(zone_info [, viewing_months, debug])

        # Validate matches and transitions
        zone_specifier.init_for_year(args.year)
        self.print_matches_and_transitions()

        # Get (offset_seconds, dst_seconds, abbrev) for an epoch_seconds.
        (offset_seconds, dst_seconds, abbrev) = \
            zone_specifier.get_timezone_info_for_seconds(epoch_seconds)

        # Get (offset_seconds, dst_seconds, abbrev) for a datetime.
        (offset_seconds, dst_seconds, abbrev) = \
            zone_specifier.get_timezone_info_for_datetime(dt)

    Note:
        The viewing_months parameter determines the month interval to use to
        calculate the transitions:

        * 12 = [year-Jan, (year+1)-Jan) (experimental)
        * 13 = [year-Jan, (year+1)-Feb) (works)
        * 14 = [(year-1)-Dec, (year+1)-Feb) (works)
        * 36 = [(year-1)-Jan, (year+2)-Jan) (not well tested,
               seems to mostly work except for 2000)
    """

    # Sentinel ZoneEra that represents the earliest zone era.
    ZONE_ERA_ANCHOR = ZoneEraCooked({
        'offsetSeconds': 0,
        'zonePolicy': '',
        'rulesDeltaSeconds': 0,
        'format': '',
        'untilYear': MIN_YEAR,
        'untilMonth': 1,
        'untilDay': 1,
        'untilSeconds': 0,
        'untilTimeSuffix': 'w',
    })

    def __init__(
        self,
        zone_info_data: ZoneInfo,
        viewing_months=14,
        debug=False,
        in_place_transitions=True,
        optimize_candidates=True,
    ):
        """Constructor.

        Args:
            zone_info_data (dict): one of the ZONE_INFO_xxx constants from
                zone_infos.py. It can contain a reference to a zone_policy_data
                map. We need to convert these into ZoneEraCooked and
                ZoneRuleCooked classes.
            viewing_months (int): size of the window to consider when
                determining the DST transitions (default: 14)
            debug (bool): set to True to enable logging
            in_place_transitions (bool): set to True to use
                ActiveSelectorInPlace class instead of ActiveSelectorBasic
                to determine the Transitions which overlap with the time
                interval specified by ZoneMatch
            optimize_candidates (bool): set to True to use
                CandidateFinderOptimized class instead of CandidateFinderBasic
                to obtain the list of candidate Transitions
        """
        self.zone_info = ZoneInfoCooked(zone_info_data)
        self.viewing_months = viewing_months
        self.in_place_transitions = in_place_transitions
        self.optimize_candidates = optimize_candidates

        # Used by init_*() to indicate the current year of interest.
        self.year = 0

        # List of ZoneMatch, i.e. ZoneEra which match the interval of interest.
        self.matches: List[ZoneMatch] = []

        # Cummulative list of all candidate Transitions across all calls to
        # find_candidate_transitions() method for the year given to
        # init_for_year(). It was initially thought to be useful for figuring
        # out the buffer size needed by the C++ implementation of this class but
        # the C++ code removes those candidate Transitions which aren't needed
        # after each iteration of ZoneMatch, and reuses the buffer consumed by
        # those ignored Transitions, so this cummulative list is not as useful.
        # It is useful to print out for debugging though.
        self.all_candidate_transitions: List[Transition] = []

        # List of matching (and active) Transition objects for the year given to
        # init_for_year().
        self.transitions: List[Transition] = []

        # The maximum value of (len(self.transitions) +
        # len(candidate_transitions)) across all calls to
        # _find_transitions_from_named_match() for the year given to
        # init_for_year(). The C++ version of this class uses a single pool of
        # Transitions to hold both active and candidate transitions. This value
        # should correspond to the largest number of slots consumed in the pool
        # by the C++ code.
        self.max_transition_buffer_size = 0

        self.debug = debug

    def get_transition_for_seconds(
        self,
        epoch_seconds: int,
    ) -> Optional[Transition]:
        """Return Transition for the given epoch_seconds.
        """
        self._init_for_second(epoch_seconds)
        return self._find_transition_for_seconds(epoch_seconds)

    def get_transition_for_datetime(self, dt: datetime) -> Optional[Transition]:
        """Return Transition for the given datetime.
        """
        self.init_for_year(dt.year)
        return self._find_transition_for_datetime(dt)

    def get_timezone_info_for_seconds(self, epoch_seconds: int):
        """Return a tuple of (total_offset, dst_seconds, abbrev).
        """
        self._init_for_second(epoch_seconds)

        # TODO(bpark): Check for None
        transition = cast(
            Transition, 
            self._find_transition_for_seconds(epoch_seconds)
        )
        return transition.to_timezone_tuple()

    def get_timezone_info_for_datetime(
        self, dt: datetime,
    ) -> Optional[OffsetInfo]:
        """Return the OffsetInfo of the Transition for a given datetime.
        """
        self.init_for_year(dt.year)
        transition = self._find_transition_for_datetime(dt)
        if transition:
            return transition.to_timezone_tuple()
        else:
            return None

    def init_for_year(self, year):
        """Initialize the Matches and Transitions for the year. Call this
        explicitly before accessing self.matches, self.transitions, and
        self.all_candidate_transitions. The high level algorithm is as follows:
            * Extract the list of ZoneEras which overlap with the given year
              and the given window size (e.g. 13, 14, 36 months). These
              are called ZoneMatches.
            * Find the list of Transitions corresponding to the ZoneMatches
              using _find_transitions_for_match().
            * Convert the transition times of the Transition objects into
              start and until times according to the UTC offset of each
              Transition.
            * Determine the start and until times of each transitions according
              to the wall time of each Transition.
            * Determine the time zone abbreviations (e.g. "PDT", "GMT") of
              each Transition.
        """
        if self.debug:
            logging.info('init_for_year(): year: %d' % year)
        # Check if cache filled
        if self.year == year:
            if self.debug:
                logging.info('init_for_year(): cached')
            return

        self.year = year
        self.max_transition_buffer_size = 0
        self.matches = []
        self.transitions = []
        self.all_candidate_transitions = []

        if self.viewing_months == 12:
            start_ym = YearMonthTuple(year, 1)
            until_ym = YearMonthTuple(year + 1, 1)
        elif self.viewing_months == 13:
            start_ym = YearMonthTuple(year, 1)
            until_ym = YearMonthTuple(year + 1, 2)
        elif self.viewing_months == 14:
            start_ym = YearMonthTuple(year - 1, 12)
            until_ym = YearMonthTuple(year + 1, 2)
        elif self.viewing_months == 36:
            start_ym = YearMonthTuple(year - 1, 1)
            until_ym = YearMonthTuple(year + 2, 1)
        else:
            raise Exception(
                'Unsupported viewing_months: %d' % self.viewing_months)

        if self.debug:
            logging.info('==== Finding matches')
        self.matches = self._find_matches(start_ym, until_ym)

        if self.debug:
            logging.info('==== Finding (raw) transitions')
        self._find_transitions(self.matches)

        # Some transitions from simple match may be in 's' or 'u', so convert
        # to 'w'.
        if self.debug:
            logging.info('==== Fixing transitions times')
        self._fix_transition_times(self.transitions)
        if self.debug:
            print_transitions(self.transitions)

        if self.debug:
            logging.info('==== Generating start and until times')
        self._generate_start_until_times(self.transitions)
        if self.debug:
            print_transitions(self.transitions)

        if self.debug:
            logging.info('==== Calculating abbreviations')
        self._calc_abbrev(self.transitions)
        if self.debug:
            print_transitions(self.transitions)

    def get_buffer_sizes(self, start_year, until_year):
        """Find the maximum number of actual transitions and the maximum number
        of candidate transitions across the given start_year and until_year.
        This is useful for determining that buffer size of the C++ version
        of this code which uses static sizes for the Transition buffers.

        Returns a tuple of tuples:
            ((max_actives, year), (max_buffer_size, year)).
        """
        max_actives = (0, 0)  # (count, year)
        max_buffer_size = (0, 0)  # (count, year)
        for year in range(start_year, until_year):
            self.init_for_year(year)

            # Number of active transitions.
            transition_count = len(self.transitions)
            if transition_count > max_actives[0]:
                max_actives = (transition_count, year)

            # Max size of the transition buffer.
            buffer_size = self.max_transition_buffer_size
            if buffer_size > max_buffer_size[0]:
                max_buffer_size = (buffer_size, year)

        return (max_actives, max_buffer_size)

    # The following methods are designed to be used internally.

    def _update_transition_buffer_size(self, candidate_transitions):
        """Update the statistics on the number of active Transitions
        and the size of the Transition buffer that may be required in the C++
        code.
        """
        total = len(candidate_transitions) + len(self.transitions)
        if total > self.max_transition_buffer_size:
            self.max_transition_buffer_size = total
        if self.debug:
            logging.info('max_transition_buffer_size: %s' %
                self.max_transition_buffer_size)
        self.all_candidate_transitions.extend(candidate_transitions)

    def _init_for_second(self, epoch_seconds):
        """Initialize the Transitions from the given epoch_seconds.
        """
        ldt = datetime.utcfromtimestamp(
            epoch_seconds + SECONDS_SINCE_UNIX_EPOCH)

        if self.viewing_months < 14:
            if ldt.month == 1 and ldt.day == 1:
                year = ldt.year - 1
            else:
                year = ldt.year
        else:
            # If viewing_months >= 14, then this shift to the nearest whole
            # year on Jan 1 or Dec 31 does not seem necessary since the unit
            # tests all pass without this.
            #
            #if ldt.month == 12 and ldt.day == 31:
            #    year = ldt.year + 1
            #elif ldt.month == 1 and ldt.day == 1:
            #    year = ldt.year - 1
            #else:
            #    year = ldt.year

            year = ldt.year

        self.init_for_year(year)

    def _find_transition_for_seconds(
        self,
        epoch_seconds: int,
    ) -> Optional[Transition]:
        """Return the matching transition, or None if not found.
        """
        matching_transition = None
        for transition in self.transitions:
            if transition.startEpochSecond <= epoch_seconds:
                matching_transition = transition
            elif transition.startEpochSecond > epoch_seconds:
                break
        return matching_transition

    def _find_transition_for_datetime(self, dt) -> Optional[Transition]:
        """Return the matching transition matching the local datetime 'dt',
        or None if not found.
        """
        matching_transition = None
        secs = hms_to_seconds(dt.hour, dt.minute, dt.second)
        dt_time = DateTuple(y=dt.year, M=dt.month, d=dt.day, ss=secs, f='w')
        for transition in self.transitions:
            start_time = transition.startDateTime
            until_time = transition.untilDateTime
            if start_time <= dt_time and dt_time < until_time:
                return transition
        return None

    def _find_matches(self, start_ym, until_ym):
        """Find the Zone Eras which overlap [start_ym, until_ym), ignoring
        day, time and timeSuffix. The start and until fields are truncated at
        the low and high end by start_ym and until_ym, respectively.

        The size of the [start_ym, until_ym) is determined by the viewing_months
        flag. Normally, viewing_months will be greater than one years, to
        compensate for our inability to precisely determine which local 'year'
        is associated with a given epochSecond.

        When the epochSeconds is converted to a year using the UTC timezone, the
        actual local DateTime could be Dec 31 of the previous year, or Jan 1 of
        the following year. Unfortunately, we don't know the local time zone
        offset until we generate the Transitions of the local year, and we don't
        know the local year until we generate the Transitions. To get around
        this problem, we create a [start_ym, until_ym) interval that's slightly
        larger than the current year of interest.

        If viewing_months==14, we include the prior December and subsequent
        January. This works well but often produces too many candidate
        Transitions because it spans 3 whole years, potentially 4 years for the
        'most recent prior year'.

        If viewing_months==13, we include only the subsequent January, which
        works because we push the year of interest to the next year if the
        epoch_seconds is on Dec 31.

        If viewing_months==12, this is an experimental option to see if we can
        reduce the number of candidate transitions.
        """
        zone_eras = self.zone_info.eras
        prev_era = self.ZONE_ERA_ANCHOR
        matches = []
        for zone_era in zone_eras:
            if self._era_overlaps_interval(prev_era, zone_era, start_ym,
                                           until_ym):
                match = self._create_match(prev_era, zone_era, start_ym,
                                           until_ym)
                if self.debug:
                    logging.info('_find_matches(): %s' % match)
                matches.append(match)
            prev_era = zone_era
        return matches

    def _find_transitions(self, matches):
        """Find the relevant transitions from the matching ZoneEras.
        This method must update self.transitions within the loop for each
        ZoneMatch, instead of collecting and returning the accumulated
        transitions array, to allow _update_transition_buffer_size() to can
        collect the buffer size statistics correctly using the intermediate
        self.transitions results.
        """
        if self.debug:
            logging.info('_find_transitions()')
        for match in matches:
            transitions_for_match = self._find_transitions_for_match(match)
            self.transitions.extend(transitions_for_match)

    def _find_transitions_for_match(self, match):
        """Determine if the given ZoneMatch is a simple ZoneMatch (contains an
        explicit DST offset) or named (references a named ZonePolicy to
        determine the DST offset). Then find the Transitions of the given match
        using the appropriate algorithm.
        """
        if self.debug:
            logging.info('_find_transitions_for_match(): %s' % match)

        zone_era = match.zoneEra
        zone_policy = zone_era.zonePolicy
        if zone_policy in ['-', ':']:
            return self._find_transitions_from_simple_match(match)
        else:
            return self._find_transitions_from_named_match(match)

    def _find_transitions_from_simple_match(self, match):
        """The zonePolicy is '-' or ':' then the Zone Era itself defines the UTC
        offset and the abbreviation. Returns a list of one Transition object,
        to make it compatible with the return type of
        _find_transitions_from_named_match().
        """
        if self.debug:
            logging.info('_find_transitions_from_simple_match(): %s' % match)
        zone_era = match.zoneEra
        transition = Transition(match)
        transition.update({
            'transitionTime': match.startDateTime,
        })
        transitions = [transition]
        self._update_transition_buffer_size(transitions)
        return transitions

    def _find_transitions_from_named_match(self, match):
        """Find the transitions of the named ZoneMatch. The search for the
        relevant Transition occurs in 2 passes:

            1 Find the candidate Transitions defined by the ZoneMatch using the
              *whole* years of the ZoneMatch (i.e. ignoring the month, day, and
              time fields). Whole years are used because the ZoneRules defined
              recurring rules based on whole years. This pass includes something
              called the "most recent prior" Transition, because we need to know
              the Transition that occurred just before the beginning of the
              given year. In this rough pass, multiple "prior" Transitions may
              be included as candidates.
            2 Precisely select the Transitions which are "active", as determined
              by the entire date fields of ZoneMatch (including month, day and
              time) fields. In this pass, only a single "most recent prior"
              Transition will be found.

        For each pass, I implemented 2 different algorithms (for a total of
        4 different independent combinations). The "Basic" versions are the
        earlier versions which use simpler code, at the expense of using more
        memory. The "Optimized" and "InPlace" versions are my subsequent
        improvements to those algorithms, making them use less memory and
        hopefully be faster. Using less memory is important because those
        algorithms will be reimplemenented in C++ for the Arduino
        microcontroller environments which have limited memory (~32kB of
        flash RAM, and ~2kB of static RAM).

        The 'self.max_transition_buffer_size' counter and
        'self.all_candidate_transitions' list attempt to track the amount of
        internal buffer space needed by the various algorithms. See comments in
        __init__().
        """
        zone_era = match.zoneEra
        zone_policy = zone_era.zonePolicy
        rules = zone_policy.rules
        if self.optimize_candidates:
            finder = CandidateFinderOptimized(self.debug)
        else:
            finder = CandidateFinderBasic(self.debug)

        # Find candidate transitions using whole years.
        if self.debug:
            logging.info('==== Get candidate transitions for named ZoneMatch')
        candidate_transitions = finder.find_candidate_transitions(match, rules)
        if self.debug:
            print_transitions(candidate_transitions)
        self._check_transitions_sorted(candidate_transitions)

        # Fix the transitions times, converting 's' and 'u' into 'w' uniformly.
        if self.debug:
            logging.info('_fix_transition_times()')
        self._fix_transition_times(candidate_transitions)
        if self.debug:
            print_transitions(candidate_transitions)
        self._check_transitions_sorted(candidate_transitions)

        # Update statistics on active transitions
        self._update_transition_buffer_size(candidate_transitions)

        # Select only those Transitions which overlap with the actual start and
        # until times of the ZoneMatch.
        if self.debug:
            logging.info('==== Select active transitions')
        if self.in_place_transitions:
            selector = ActiveSelectorInPlace(self.debug)
        else:
            selector = ActiveSelectorBasic(self.debug)
        try:
            transitions = selector.select_active_transitions(
                candidate_transitions, match)
        except:
            logging.exception("Zone '%s'; year '%04d'", self.zone_info.name,
                              self.year)
            raise
        if self.debug:
            print_transitions(transitions)

        # Verify that the "most recent prior" Transition is properly sorted.
        if self.debug:
            logging.info('==== Final check for sorted transitions')
        self._check_transitions_sorted(transitions)
        if self.debug:
            print_transitions(transitions)

        return transitions

    def print_matches_and_transitions(self):
        logging.info('---- Max Buffer Size: %s',
            self.max_transition_buffer_size)
        logging.info('---- Matches:')
        for m in self.matches:
            logging.info(m)
        logging.info('---- Transitions:')
        for t in self.transitions:
            logging.info(t)
        logging.info('---- Candidate Transitions:')
        for t in self.all_candidate_transitions:
            logging.info(t)

    @staticmethod
    def _check_transitions_sorted(transitions):
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

    @staticmethod
    def _create_match(prev_era, zone_era, start_ym, until_ym):
        """Create the Zone Match object for the given Zone Era, truncated at
        the low and high end by start_ym and until_ym:

            * ZoneMatch.startDateTime is prev_era.untilTime
            * ZoneMatch.untilDateTime is zone_era.untilTime
            * ZoneMatch.policy_name is '-', ':' or the string name of ZonePolicy

        The startDateTime of the current ZoneMatch is determined by the UNTIL
        datetime of the prev_era, which uses the UTC offset of the *previous*
        era, not the current era. Therefore, the startDateTime and untilDateTime
        is accurate to a resolution of one day. This is good enough to generate
        Transitions, which also will have dateTime fields accurate to within a
        day or so, assuming we don't have 2 DST transitions in a single day.

        See _fix_transition_times() which normalizes these start times to the
        wall time uniformly.
        """
        start_date_time = DateTuple(
            y=prev_era.untilYear,
            M=prev_era.untilMonth,
            d=prev_era.untilDay,
            ss=prev_era.untilSeconds,
            f=prev_era.untilTimeSuffix)
        if start_date_time < DateTuple(
                y=start_ym.y, M=start_ym.M, d=1, ss=0, f='w'):
            start_date_time = DateTuple(
                y=start_ym.y, M=start_ym.M, d=1, ss=0, f='w')

        until_date_time = DateTuple(
            y=zone_era.untilYear,
            M=zone_era.untilMonth,
            d=zone_era.untilDay,
            ss=zone_era.untilSeconds,
            f=zone_era.untilTimeSuffix)
        if until_date_time > DateTuple(
                y=until_ym.y, M=until_ym.M, d=1, ss=0, f='w'):
            until_date_time = DateTuple(
                y=until_ym.y, M=until_ym.M, d=1, ss=0, f='w')

        return ZoneMatch({
            'startDateTime': start_date_time,
            'untilDateTime': until_date_time,
            'zoneEra': zone_era
        })

    @staticmethod
    def _generate_start_until_times(transitions):
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

            # 2) Calculate the current startDateTime by shifting the transition
            # time into the current UTC offset. This algorithm should be able to
            # handle transition time of 24:00 (or even 25:00) of the previous
            # day.
            secs = (tt.ss - prev.offsetSeconds - prev.deltaSeconds +
                    transition.offsetSeconds + transition.deltaSeconds)
            #if secs < 0 or secs >= 24 * 60 * 60:
            #   (h, m, s) = seconds_to_hms(secs)
            #    logging.info(
            #        "Zone '%s': Transition startDateTime shifted into "
            #        + "a different day: (%02d:%02d:%02d)",
            #        self.zone_info['name'], h, m, s)
            st = datetime(tt.y, tt.M, tt.d, 0, 0, 0)
            st += timedelta(seconds=secs)
            secs = hms_to_seconds(st.hour, st.minute, st.second)
            transition.startDateTime = DateTuple(
                y=st.year, M=st.month, d=st.day, ss=secs, f=tt.f)

            # 3) The epochSecond of the 'transitionTime' is determined by the
            # UTC offset of the *previous* Transition. However, the
            # transitionTime can be represented by an illegal time (e.g. 24:00).
            # So, it is better to use the properly normalized startDateTime
            # (calculated above) with the *current* UTC offset.
            utc_offset_seconds = transition.offsetSeconds \
                + transition.deltaSeconds
            z = timezone(timedelta(seconds=utc_offset_seconds))
            dt = st.replace(tzinfo=z)
            epoch_second = int((dt - ACETIME_EPOCH).total_seconds())
            transition.startEpochSecond = epoch_second

            prev = transition
            is_after_first = True

        # Finally, fix the last transition's until time
        (udt, udts, udtu) = ZoneSpecifier._expand_date_tuple(
            transition.untilDateTime, transition.offsetSeconds,
            transition.deltaSeconds)
        transition.untilDateTime = udt

    @staticmethod
    def _fix_transition_times(transitions):
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
                ZoneSpecifier._expand_date_tuple(transition.transitionTime,
                    prev.offsetSeconds, prev.deltaSeconds)
            prev = transition

    @staticmethod
    def _expand_date_tuple(dt, offset_seconds, delta_seconds):
        """Convert 's', 'u', or 'w' time into the other 2 versions using the
        given base UTC offset and the delta DST offset. Return a tuple of
        *normalized* (wall, standard, utc) date tuples. The dates are normalized
        so that transitions occurring at 24:00:00 is moved to the next day.
        """
        delta_seconds = delta_seconds if delta_seconds else 0
        offset_seconds = offset_seconds if offset_seconds else 0

        if dt.f == 'w':
            dtw = dt
            dts = DateTuple(
                y=dt.y, M=dt.M, d=dt.d, ss=dtw.ss - delta_seconds, f='s')
            ss = dtw.ss - delta_seconds - offset_seconds
            dtu = DateTuple(y=dt.y, M=dt.M, d=dt.d, ss=ss, f='u')
        elif dt.f == 's':
            dts = dt
            dtw = DateTuple(
                y=dt.y, M=dt.M, d=dt.d, ss=dts.ss + delta_seconds, f='w')
            dtu = DateTuple(
                y=dt.y, M=dt.M, d=dt.d, ss=dts.ss - offset_seconds, f='u')
        elif dt.f == 'u':
            dtu = dt
            ss = dtu.ss + delta_seconds + offset_seconds
            dtw = DateTuple(y=dtu.y, M=dtu.M, d=dtu.d, ss=ss, f='w')
            dts = DateTuple(
                y=dtu.y, M=dtu.M, d=dtu.d, ss=dtu.ss + offset_seconds, f='s')
        else:
            logging.error("Unrecognized Rule.AT suffix '%s'; date=%s", dt.f,
                          dt)
            sys.exit(1)

        dtw = ZoneSpecifier._normalize_date_tuple(dtw)
        dts = ZoneSpecifier._normalize_date_tuple(dts)
        dtu = ZoneSpecifier._normalize_date_tuple(dtu)

        return (dtw, dts, dtu)

    @staticmethod
    def _normalize_date_tuple(tt):
        """Return the normalized DateTuple where the dt.ss could be negative or
        greater than 24h.
        """
        if tt.y == MIN_YEAR:
            return DateTuple(y=MIN_YEAR, M=1, d=1, ss=0, f=tt.f)

        try:
            st = datetime(tt.y, tt.M, tt.d, 0, 0, 0)
            delta = timedelta(seconds=tt.ss)
            st += delta
            secs = hms_to_seconds(st.hour, st.minute, st.second)
            return DateTuple(y=st.year, M=st.month, d=st.day, ss=secs, f=tt.f)
        except:
            logging.error('Invalid datetime: %s + %s', st, delta)
            sys.exit(1)

    @staticmethod
    def _calc_abbrev(transitions):
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

    @staticmethod
    def _era_overlaps_interval(prev_era, era, start_ym, until_ym):
        """Determines if era overlaps the interval [start_ym, until_ym),
        ignoring the day, time and timeSuffix. The start date of the current
        era is represented by the prev_era.UNTIL, so the interval of the current
        era is [start_era, until_era) = [prev_era.UNTIL, era.UNTIL). Overlap
        happens if (start_era < until_ym) and (until_era > start_ym).
        """
        return (ZoneSpecifier._compare_era_to_year_month(
            prev_era, until_ym.y, until_ym.M) < 0
                and ZoneSpecifier._compare_era_to_year_month(
                    era, start_ym.y, start_ym.M) > 0)

    @staticmethod
    def _compare_era_to_year_month(era, year, month):
        """Compare the zone_era with year, returning -1, 0 or 1. The day of
        month is implicitly 1. Ignore the untilTimeSuffix suffix. Maybe it's
        not needed in this context?
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


class CandidateFinderBasic:
    def __init__(self, debug):
        self.debug = debug

    def find_candidate_transitions(self, match, rules):
        """Get the list of candidate transitions from the 'rules' which overlap
        the whole years [start_y, end_y] (inclusive)) defined by the given
        ZoneMatch. This list includes transitions that may become the "most
        recent prior" transition. We use whole years because 'rules' define
        repetitive transitions using whole years.
        """
        if self.debug:
            logging.info('Basic.find_candidate_transitions()')

        start_y = match.startDateTime.y
        until = match.untilDateTime
        if until.M == 1 and until.d == 1 and until.ss == 0:
            end_y = until.y - 1
        else:
            end_y = until.y

        transitions = []
        for rule in rules:
            from_year = rule.fromYear
            to_year = rule.toYear
            years = self.get_candidate_years(from_year, to_year, start_y,
                                             end_y)
            for year in years:
                _add_transition_sorted(transitions,
                                       _create_transition_for_year(
                                           year, rule, match))

        return transitions

    @staticmethod
    def get_candidate_years(from_year, to_year, start_year, end_year):
        """Return the array of years within the Rule's [from_year, to_year]
        range which should be evaluated to obtain the transitions necessary for
        the matched ZoneEra that spans [start_year, end_year].
            1) Include all years which overlap [start_year, end_year].
            2) Add the latest year prior to [start_year]. This is guaranteed to
            exists because we added an anchor rule at year 0 for those zone
            policies that need it.
        If [start_year, end_year] spans a 3-year interval (which will be the
        case for all supported values of 'viewing_months'), then the maximum
        number of elements in 'years' will be 4.
        """
        years = _get_interior_years(from_year, to_year, start_year, end_year)

        # Add most recent Rule year prior to Match years.
        prior_year = _get_most_recent_prior_year(from_year, to_year,
                                                 start_year, end_year)
        if prior_year >= 0:
            years.append(prior_year)

        return years


class CandidateFinderOptimized:
    def __init__(self, debug):
        self.debug = debug

    def find_candidate_transitions(self, match, rules):
        """Similar to CandidateFinderBasic.find_candidate_transitions() except
        that prior Transitions which are obviously non-candidates are filtered
        out early. This reduces the size of the statically allocated Transitions
        array in the C++ implementation.
        """
        if self.debug:
            logging.info('Optimized.find_candidate_transitions()')

        start_y = match.startDateTime.y
        end_y = match.untilDateTime.y
        until = match.untilDateTime
        if until.M == 1 and until.d == 1 and until.ss == 0:
            end_y = until.y - 1
        else:
            end_y = until.y

        transitions = []
        prior_transition = None
        for rule in rules:
            from_year = rule.fromYear
            to_year = rule.toYear
            years = _get_interior_years(from_year, to_year, start_y, end_y)
            if self.debug:
                logging.info(
                    'find_candidate_transitions(): interior years: %s', years)

            for year in years:
                transition = _create_transition_for_year(year, rule, match)
                comp = _compare_transition_to_match_fuzzy(transition, match)
                if comp < 0:
                    prior_transition = self._calc_prior_transition(
                        prior_transition, transition)
                elif comp == 1:
                    _add_transition_sorted(transitions, transition)

            prior_year = _get_most_recent_prior_year(from_year, to_year,
                                                     start_y, end_y)
            if self.debug:
                logging.info('find_candidate_transitions(): prior year: %s',
                             prior_year)
            if prior_year >= 0:
                transition = _create_transition_for_year(
                    prior_year, rule, match)
                prior_transition = self._calc_prior_transition(
                    prior_transition, transition)
        if prior_transition:
            _add_transition_sorted(transitions, prior_transition)

        return transitions

    @staticmethod
    def _calc_prior_transition(prior_transition, transition):
        """Return the latest prior transition.
        """
        if prior_transition:
            if transition.transitionTime > prior_transition.transitionTime:
                return transition
            else:
                return prior_transition
        else:
            return transition


class ActiveSelectorBasic:
    def __init__(self, debug):
        self.debug = debug

    def select_active_transitions(self, transitions, match):
        """Select those Transitions which overlap with the ZoneMatch interval
        which may not be at year boundary. Also select the latest prior
        transition before the given ZoneMatch, shifting the transition time to
        the start of the ZoneMatch. The returned array of transitions is likely
        to be unsorted again, since the latest prior transition is added to the
        end.
        """
        if self.debug:
            logging.info('ActiveSelectorBasic.select_active_transitions()')

        # Commulative results of _process_transition()
        results = {
            'startTransitionFound': None,
            'latestPriorTransition': None,
            'transitions': []
        }

        # Categorize each transition
        for transition in transitions:
            self._process_transition(match, transition, results)
        transitions = results['transitions']

        # Add the latest prior transition. Adding this at the end of the array
        # will likely cause the transitions to become unsorted, requiring
        # another sorting pass.
        if not results.get('startTransitionFound'):
            prior_transition = results.get('latestPriorTransition')
            if not prior_transition:
                raise Exception(
                    'Prior transition not found; should not happen')

            # Adjust the transition time to be the start of the ZoneMatch.
            prior_transition = prior_transition.copy()
            prior_transition.originalTransitionTime = \
                prior_transition.transitionTime
            prior_transition.transitionTime = match.startDateTime
            _add_transition_sorted(transitions, prior_transition)

        return transitions

    @staticmethod
    def _process_transition(match, transition, results):
        """Compare the given transition to the given match, checking the
        following situations:

        1) If the Transition is outside the time range of the ZoneMatch,
        ignore the transition.
        2) If the Transition is within the matching ZoneMatch, it is added
        to the map at results['transitions'].
        2a) If the Transition occurs at the very start of the ZoneMatch, then
        set the flag "startTransitionFound" to true.
        3) If the Transition is earlier than the ZoneMatch, then add it to the
        'latestPriorTransition' if it is the largest prior transition.

        This method assumes that the transition time of the Transition has been
        fixed using the _fix_transition_times() method, so that the comparison
        with the ZoneMatch can occur accurately.

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
        # Determine if the transition falls within the match range.
        transition_compared_to_match = _compare_transition_to_match(
            transition, match)
        if transition_compared_to_match == 2:
            return
        elif transition_compared_to_match in [0, 1]:
            _add_transition_sorted(results['transitions'], transition)
            if transition_compared_to_match == 0:
                results['startTransitionFound'] = True
        else:  # transition_compared_to_match < 0:
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


class ActiveSelectorInPlace:
    def __init__(self, debug):
        self.debug = debug

    def select_active_transitions(self, transitions, match):
        """Similar to ActiveSelectorBasic.select_active_transitions() except
        that it does not use any additional dynamically allocated array of
        Transitions. It uses the Transition.isActive flag to mark if a
        Transition is active or not.
        """
        if self.debug:
            logging.info('ActiveSelectorInPlace.select_active_transitions()')

        prior = None
        for transition in transitions:
            prior = self._process_transition(match, transition, prior)

        if prior and prior.transitionTime < match.startDateTime:
            prior.originalTransitionTime = prior.transitionTime
            prior.transitionTime = match.startDateTime

        active_transitions = []
        for transition in transitions:
            if transition.isActive:
                active_transitions.append(transition)
        return active_transitions

    @staticmethod
    def _process_transition(match, transition, prior):
        """A version of ActiveSelectorBasic._process_transition() that does
        not allocate new array members, rather uses an internal flag. This
        assumes that all Transitions have been fixed using
        _fix_transition_times().
        """
        transition_compared_to_match = _compare_transition_to_match(
            transition, match)
        if transition_compared_to_match == 2:
            transition.isActive = False
        elif transition_compared_to_match == 1:
            transition.isActive = True
        elif transition_compared_to_match == 0:
            transition.isActive = True
            if prior:
                prior.isActive = False
            prior = transition
        else:  # transition_compared_to_match < 0:
            if prior:
                if transition.transitionTime > prior.transitionTime:
                    prior.isActive = False
                    transition.isActive = True
                    prior = transition
            else:
                transition.isActive = True
                prior = transition
        return prior


def print_transitions(transitions):
    logging.info('Num transitions: %d' % len(transitions))
    for t in transitions:
        logging.info(t)


def _add_transition_sorted(transitions, transition):
    """Add the transition to the transitions array so that it is sorted by
    transitionTime. This is not normally how this would be done in Python. This
    is emulating the code that would be written in an Arduino C++ environment,
    without dynamic arrays and a sort() function. This will allow this class to
    be more easily ported to C++. The O(N^2) insertion sort algorithm should be
    fast enough since N<=5.
    """
    transitions.append(transition)
    for i in range(len(transitions) - 1, 0, -1):
        curr = transitions[i]
        prev = transitions[i - 1]
        if _compare_date_tuple(curr.transitionTime, prev.transitionTime) < 0:
            transitions[i - 1] = curr
            transitions[i] = prev


def _compare_date_tuple(a, b):
    if a.y < b.y: return -1
    if a.y > b.y: return 1
    if a.M < b.M: return -1
    if a.M > b.M: return 1
    if a.d < b.d: return -1
    if a.d > b.d: return 1
    return 0


def _create_transition_for_year(year, rule, match):
    """Create the transition from the given 'rule' for the given 'year'.
    Return None if 'year' does not overlap with the [from, to] of the rule. The
    Transition object is a replica of the underlying Match object, with
    additional bookkeeping info.
    """
    transition_time = _get_transition_time(year, rule)
    zone_era = match.zoneEra
    transition = Transition(match)
    transition.update({
        'transitionTime': transition_time,
        'zoneRule': rule,
    })
    return transition


def _get_interior_years(from_year, to_year, start_year, end_year):
    """Return the Rule years that overlap with the Match[start_year, end_year].
    """
    years = []
    for year in range(start_year, end_year + 1):
        if from_year <= year and year <= to_year:
            years.append(year)
    return years


def _get_most_recent_prior_year(from_year, to_year, start_year, end_year):
    """Return the most recent prior year of the rule[from_year, to_year].
    Return -1 if the rule[from_year, to_year] has no prior year to the
    match[start_year, end_year].
    """
    if from_year < start_year:
        if to_year < start_year:
            return to_year
        else:
            return start_year - 1
    else:
        return -1


def _compare_transition_to_match(transition, match):
    """Determine if transition_time applies to given range of the match.
    To compare the Transition time to the ZoneMatch time properly, the
    transition time of the Transition should be expanded to include all 3
    versions ('w', 's', and 'u') of the time stamp. When comparing against the
    ZoneMatch.startDateTime and ZoneMatch.untilDateTime, the version will be
    determined by the suffix of those parameters.

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
        raise Exception("Unknown suffix: %s" % match_start)
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
        raise Exception("Unknown suffix: %s" % match_until)
    if match_until <= transition_time:
        return 2

    return 1


def _compare_transition_to_match_fuzzy(
    transition: Transition,
    match: ZoneMatch,
) -> int:
    """Like _compare_transition_to_match() except perform a fuzzy match
    within at least one-month of the match.start or match.until.

    A value of 0 is never returned since we cannot make a direct comparison
    to match_start.

    Return:
        * -1 if less than match
        * 1 if within match,
        * 2 if greater than match
    """
    tt = transition.transitionTime
    transition_time = 12 * tt.y + tt.M

    ms = match.startDateTime
    match_start = 12 * ms.y + ms.M
    if transition_time < match_start - 1:
        return -1

    mu = match.untilDateTime
    match_until = 12 * mu.y + mu.M
    if match_until + 2 <= transition_time:
        return 2

    return 1


def _get_transition_time(year: int, rule: ZoneRuleCooked) -> DateTuple:
    """Return the (year, month, day, seconds, suffix) of the Rule in given
    year.
    """
    month, day = calc_day_of_month(year, rule.inMonth, rule.onDayOfWeek,
                                   rule.onDayOfMonth)
    seconds = rule.atSeconds
    suffix = rule.atTimeSuffix
    return DateTuple(y=year, M=month, d=day, ss=seconds, f=suffix)


def date_tuple_to_string(dt: DateTuple) -> str:
    (h, m, s) = seconds_to_hms(dt.ss)
    return '%04d-%02d-%02d %02d:%02d%s' % (dt.y, dt.M, dt.d, h, m, dt.f)


def to_utc_string(utcoffset: int, dstoffset: int) -> str:
    return 'UTC%s%s' % (seconds_to_hm_string(utcoffset),
                        seconds_to_hm_string(dstoffset))


def seconds_to_hm_string(secs: int) -> str:
    if secs < 0:
        hms = seconds_to_hms(-secs)
        return '-%02d:%02d' % (hms[0], hms[1])
    else:
        hms = seconds_to_hms(secs)
        return '+%02d:%02d' % (hms[0], hms[1])


def main():
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Zone Agent.')
    parser.add_argument(
        '--viewing_months',
        help='Number of months to use for calculations (12, 13, 14, 36)',
        type=int,
        default=14)
    parser.add_argument(
        '--transition',
        help='Print the transition instead of timezone info',
        action='store_true')
    parser.add_argument(
        '--debug', help='Print debugging info', action='store_true')
    parser.add_argument(
        '--in_place_transitions',
        help='Use in-place Transition array to determine Active Transitions',
        action="store_true")
    parser.add_argument(
        '--optimize_candidates',
        help='Optimize the candidate transitions',
        action='store_true')
    parser.add_argument('--zone', help='Name of time zone', required=True)
    parser.add_argument('--year', help='Year of interest', type=int)
    parser.add_argument('--date', help='DateTime of interest')
    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    # Find the zone. Dynmaically import the 'zone_infos' to avoid forcing
    # tzcompiler.py to statically depend on the zone_infos.py and
    # zone_policies.py, which are generated by tzcompiler.py itself.
    zone_infos = importlib.import_module('zonedb.zone_infos')
    zone_info = zone_infos.ZONE_INFO_MAP.get(args.zone)
    if not zone_info:
        logging.error("Zone '%s' not found", args.zone)
        sys.exit(1)

    # Create the ZoneSpecifier for zone
    zone_specifier = ZoneSpecifier(
        zone_info_data=zone_info,
        viewing_months=args.viewing_months,
        debug=args.debug,
        in_place_transitions=args.in_place_transitions,
        optimize_candidates=args.optimize_candidates)

    if args.year:
        zone_specifier.init_for_year(args.year)
        if args.debug:
            logging.info('==== Final matches and transitions')
        zone_specifier.print_matches_and_transitions()
    elif args.date:
        dt: datetime = datetime.strptime(args.date, "%Y-%m-%dT%H:%M")
        if args.transition:
            transition = zone_specifier.get_transition_for_datetime(dt)
            if transition:
                logging.info(transition)
            else:
                logging.error('Transition not found')
        else:
            offset_info = zone_specifier.get_timezone_info_for_datetime(dt)
            if not offset_info:
                logging.info('Invalid time')
            else:
                logging.info('%s (%s)',
                             to_utc_string(
                                offset_info.utc_offset,
                                offset_info.dst_offset,
                            ),
                            offset_info.abbrev)
    else:
        print("One of --year or --date must be provided")
        sys.exit(1)


if __name__ == '__main__':
    main()
