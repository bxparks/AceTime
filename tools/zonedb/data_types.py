# Copyright 2020 Brian T. Park
#
# MIT License

from typing import List, Dict
from typing import Optional
from typing import NamedTuple
from typing_extensions import TypedDict

"""
Data types created or consumed by various classes under the tools package.
These allow typing checking to be performed using mypy.
"""

# -----------------------------------------------------------------------------
# Constants used by various modules.
# -----------------------------------------------------------------------------

# AceTime Epoch is 2000-01-01 00:00:00
EPOCH_YEAR: int = 2000

# Indicate +Infinity UNTIL year (represented by empty field).
MAX_UNTIL_YEAR: int = 10000

# Tiny (int8_t) version of MAX_UNTIL_YEAR_TINY.
MAX_UNTIL_YEAR_TINY: int = 127

# Indicate max TO or FROM year.
MAX_YEAR: int = MAX_UNTIL_YEAR - 1

# Tiny (int8_t) version of MAX_YEAR.
MAX_YEAR_TINY: int = MAX_UNTIL_YEAR_TINY - 1

# Marker year to indicate -Infinity year.
MIN_YEAR: int = 0

# Tiny (int8_t) version of MIN_YEAR. Can't be -128 because that's
# used for INVALID_YEAR_TINY.
MIN_YEAR_TINY: int = -127

# Indicate an invalid year.
INVALID_YEAR: int = -1

# Tiny (int8_t) version of INVALID_YEAR.
INVALID_YEAR_TINY: int = -128

# Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
# (2000-01-01 00:00:00)
SECONDS_SINCE_UNIX_EPOCH = 946684800


# -----------------------------------------------------------------------------
# Data types produced mostly by extractor.py. Some fields are incrementally
# added by transformer.py and artransformer.py.
# -----------------------------------------------------------------------------

class ZoneRuleRaw(TypedDict, total=False):
    """Represents the input records corresponding to the 'RULE' lines in a
    tz database file. Those entries look like this:

    # Rule  NAME    FROM    TO    TYPE IN   ON      AT      SAVE    LETTER
    Rule    US      2007    max   -    Mar  Sun>=8  2:00    1:00    D
    Rule    US      2007    max   -    Nov  Sun>=1  2:00    0       S
    """
    fromYear: int  # from year
    toYear: int  # to year, 1 to MAX_YEAR (9999) means 'max'
    inMonth: int  # month index (1-12)
    onDay: str  # 'lastSun' or 'Sun>=2', or 'dayOfMonth'
    atTime: str  # hour at which to transition to and from DST
    atTimeSuffix: str  # 's', 'w', 'u'
    deltaOffset: str  # offset from Standard time ('SAVE' field)
    letter: str  # 'D', 'S', '-'
    rawLine: str  # the original RULE line from the TZ file

    # Derived from above by transformer.py.
    onDayOfWeek: int  # 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
    onDayOfMonth: int  # 1-31 "dow>=xx", -(1-31) "dow<=xx", 0={lastXxx}
    atSeconds: int  # atTime in seconds since 00:00:00
    atSecondsTruncated: int  # atSeconds after truncation
    deltaSeconds: int  # offset from Standard time in seconds
    deltaSecondsTruncated: int  # deltaSeconds after truncation
    used: Optional[bool]  # whether or not the rule is used by a zone

    # Derived from above by artransformer.py
    fromYearTiny: int  # fromYear - 2000, with special cases for MIN and MAX
    toYearTiny: int  # fromYear - 2000, with special cases for MIN and MAX
    atTimeCode: int  # units of 15-minutes
    atTimeModifier: int  # 's', 'w' or 'u' + atTimeMinute
    # DST offset in 15-min increments. For extended, the +1h is added before
    # encoding, for a total DST offset range of -1h00m to +2h45m.
    deltaCode: int
    letterIndex: int  # index into letters[], or -1 if all are single character


class ZoneEraRaw(TypedDict, total=False):
    """Represents the input records corresponding to the 'ZONE' lines in a
    tz database file. Those entries look like this:

    # Zone  NAME                STDOFF      RULES   FORMAT  [UNTIL]
    Zone    America/Chicago     -5:50:36    -       LMT     1883 Nov 18 12:09:24
                                -6:00       US      C%sT    1920
                                ...
                                -6:00       US      C%sT

    """
    offsetString: str   # offset from UTC/GMT
    rules: str  # name of the Rule in effect, '-', or ':'
    format: str  # abbreviation format (e.g. P%sT, E%sT, GMT/BST)
    untilYear: int  # MAX_UNTIL_YEAR means 'max'
    untilYearOnly: bool  # true if only the year is given
    untilMonth: int  # 1-12
    untilDayString: str  # e.g. 'lastSun', 'Sun>=3', or '1'-'31'
    untilTime: str  # e.g. '2:00', '00:01'
    untilTimeSuffix: str  # '', 's', 'w', 'g', 'u', 'z'
    rawLine: str  # original ZONE line in TZ file

    # Derived from above by transfomer.py
    offsetSeconds: int  # offset from UTC/GMT in seconds
    offsetSecondsTruncated: int  # offsetSeconds truncation granularity
    # If 'rules' is set to ':', then this contains the parsed delta offset from
    # UTC in seconds if RULES is DST offset string of the form hh:mm[:ss].
    rulesDeltaSeconds: int
    rulesDeltaSecondsTruncated: int  # rulesDeltaSeconds after truncation
    untilDay: int  # 1-31
    untilSeconds: int  # untilTime converted into total seconds
    untilSecondsTruncated: int  # untilSeconds after truncation

    # Derived from above by artransformer.py
    offsetCode: int  # STD offset in 15-min increments.
    # deltaCode is the DST offset in 15-min increments for basic. For extended,
    # the top 4 bits are the offsetMinutes. The bottom 4-bits are used to store
    # the (deltaCode + 1h), for a total DST shift range of -1h00m to 2h45m
    deltaCode: int
    untilYearTiny: int  # untilYear - 2000, with special cases for MIN and MAX
    untilTimeCode: int  # units of 15-minutes
    untilTimeModifier: int  # 's', 'w' or 'u' + untilTimeMinute
    formatShort: str  # Arduino version of format with %s -> %


# Map of policyName -> ZoneRuleRaw[]. Created by extractor.py.
PoliciesMap = Dict[str, List[ZoneRuleRaw]]

# Map of zoneName -> ZoneEraRaw[]. Created by extractor.py.
ZonesMap = Dict[str, List[ZoneEraRaw]]

# Map of linkName -> zoneName. Created by extractor.py.
LinksMap = Dict[str, str]

# -----------------------------------------------------------------------------
# Data types generated by artransformer.py
# -----------------------------------------------------------------------------

# List of LETTER strings that are more than 1-character long. Created by
# artransformer.py.
# map{policy_name -> map{letter -> index}}
IndexedLetters = Dict[str, int]
LettersMap = Dict[str, IndexedLetters]


class ArduinoTransformerResult(NamedTuple):
    """Result type of ArduinoTransformer.get_data()."""
    zones_map: ZonesMap
    policies_map: PoliciesMap
    letters_map: LettersMap


# -----------------------------------------------------------------------------
# Data types generated by transformer.py
# -----------------------------------------------------------------------------

# Map of zoneName -> List[Comment] created by transformer.py. Same as
# transformer.CommentsCollection but converted to a List so that it can be
# serialized as JSON.
CommentsMap = Dict[str, List[str]]


class TransformerResult(NamedTuple):
    """Result type of Transformer.get_data()."""
    zones_map: ZonesMap
    policies_map: PoliciesMap
    links_map: LinksMap
    removed_zones: CommentsMap
    removed_policies: CommentsMap
    removed_links: CommentsMap
    notable_zones: CommentsMap
    notable_policies: CommentsMap
    notable_links: CommentsMap


# -----------------------------------------------------------------------------
# Data types used by bufestimator.py
# -----------------------------------------------------------------------------

# zoneName -> bufSize
BufSizeMap = Dict[str, int]


class BufSizeInfo(TypedDict):
    """Return type of BufSizeEstimator.estimate()."""
    buf_sizes: BufSizeMap
    max_buf_size: int  # maximum of all bufSize


# -----------------------------------------------------------------------------
# The master ZoneInfo database which can be rendered into different forms by
# various generators (e.g. JSON, or Arduino C++).
# -----------------------------------------------------------------------------

class ZoneInfoDatabase(TypedDict):
    """The complete internal representation of the TZ Database files after
    processing them for the AceTime library.
    """

    # Context data.
    tz_version: str
    tz_files: List[str]
    scope: str
    start_year: int
    until_year: int
    until_at_granularity: int
    offset_granularity: int
    strict: bool

    # Data from Extractor filtered through Transformer
    zones_map: ZonesMap
    policies_map: PoliciesMap
    links_map: LinksMap

    # Data from Transformer
    removed_zones: CommentsMap
    removed_links: CommentsMap
    removed_policies: CommentsMap
    notable_zones: CommentsMap
    notable_links: CommentsMap
    notable_policies: CommentsMap

    # Data from BufSizeEstimator
    buf_sizes: BufSizeMap
    max_buf_size: int

    # ZoneIds
    zone_ids: Dict[str, int]

    # ZonePolicy letters that are more than 1 character long.
    letters_map: LettersMap


def create_zone_info_database(
    tz_version: str,
    tz_files: List[str],
    scope: str,
    start_year: int,
    until_year: int,
    until_at_granularity: int,
    offset_granularity: int,
    strict: bool,
    zones_map: ZonesMap,
    policies_map: PoliciesMap,
    links_map: LinksMap,
    removed_zones: CommentsMap,
    removed_links: CommentsMap,
    removed_policies: CommentsMap,
    notable_zones: CommentsMap,
    notable_links: CommentsMap,
    notable_policies: CommentsMap,
    buf_size_info: BufSizeInfo,
    zone_ids: Dict[str, int],
    letters_map: LettersMap,
) -> ZoneInfoDatabase:
    """Return an instance of ZoneInfoDatabase from the various ingrediants."""
    return {
        # Context data.
        'tz_version': tz_version,
        'tz_files': tz_files,
        'scope': scope,
        'start_year': start_year,
        'until_year': until_year,
        'until_at_granularity': until_at_granularity,
        'offset_granularity': offset_granularity,
        'strict': strict,

        # Data from Extractor filtered through Transformer.
        'zones_map': zones_map,
        'policies_map': policies_map,
        'links_map': links_map,

        # Data from Transformer.
        'removed_zones': removed_zones,
        'removed_links': removed_links,
        'removed_policies': removed_policies,
        'notable_zones': notable_zones,
        'notable_links': notable_links,
        'notable_policies': notable_policies,

        # Data from BufSizeEstimator
        'buf_sizes': buf_size_info['buf_sizes'],
        'max_buf_size': buf_size_info['max_buf_size'],

        # ZoneIds
        'zone_ids': zone_ids,

        # Letters map
        'letters_map': letters_map,
    }
