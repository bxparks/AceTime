# Copyright 2020 Brian T. Park
#
# MIT License

from typing import List, Dict, Iterable
from typing import Optional
from typing import NamedTuple
from typing import Set
from typing import cast
from collections import OrderedDict
from typing_extensions import TypedDict

"""
Data types created or consumed by various classes under the tools package. These
allow typing checking to be performed using mypy. Also contains global constants
used by multiple packages.
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
    deltaOffset: str  # DST offset from Standard time ('SAVE' field)
    letter: str  # 'D', 'S', '-', but sometimes longer 'DD', 'CAT', etc.
    rawLine: str  # the original RULE line from the TZ file

    # Derived from above by transformer.py.
    onDayOfWeek: int  # 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
    onDayOfMonth: int  # 1-31 "dow>=xx", -(1-31) "dow<=xx", 0={lastXxx}
    atSeconds: int  # atTime in seconds since 00:00:00
    atSecondsTruncated: int  # atSeconds truncated to granularity
    deltaSeconds: int  # offset from Standard time in seconds
    deltaSecondsTruncated: int  # deltaSeconds truncated to granularity
    used: Optional[bool]  # whether or not the rule is used by a zone

    # Derived from above by artransformer.py
    fromYearTiny: int  # fromYear - 2000, with special cases for MIN and MAX
    toYearTiny: int  # fromYear - 2000, with special cases for MIN and MAX
    atTimeCode: int  # atTime in units of 15-min
    atTimeMinute: int  # atTime remainder minutes
    atTimeModifier: int  # 's', 'w' or 'u' + atTimeMinute
    deltaCode: int  # DST delta offset in units of 15-min
    deltaCodeEncoded: int  # encoded version of deltaCode
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
    offsetString: str   # STD offset from UTC/GMT
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
    offsetSeconds: int  # STD offset from UTC/GMT in seconds
    offsetSecondsTruncated: int  # offsetSeconds truncated to granularity
    # If RULES is a DST offset string of the form # hh:mm[:ss], then 'rules' is
    # set to ':', and'rulesDeltaSeconds' contains the parsed delta offset from
    # UTC in seconds. If RULES is '-', then this is set to 0.
    rulesDeltaSeconds: int
    rulesDeltaSecondsTruncated: int  # rulesDeltaSeconds trunc. to granularity
    untilDay: int  # 1-31
    untilSeconds: int  # untilTime converted into total seconds
    untilSecondsTruncated: int  # untilSeconds after truncation

    # Derived from above by artransformer.py
    offsetCode: int  # STD offset in units of 15-min
    offsetMinute: int  # STD offset remainder minutes
    deltaCode: int  # DST offset in units of 15-min
    deltaCodeEncoded: int  # deltaCode + offsetMinute, encoded into 2 x 4-bits
    untilYearTiny: int  # untilYear - 2000, with special cases for MIN and MAX
    untilTimeCode: int  # untilTime in units of 15-min
    untilTimeMinute: int  # untilTime remainder minutes
    untilTimeModifier: int  # 's', 'w' or 'u' + untilTimeMinute
    formatShort: str  # Arduino version of format with %s -> %


# Map of policyName -> ZoneRuleRaw[]. Created by extractor.py.
PoliciesMap = Dict[str, List[ZoneRuleRaw]]

# Map of zoneName -> ZoneEraRaw[]. Created by extractor.py.
ZonesMap = Dict[str, List[ZoneEraRaw]]

# Map of linkName -> zoneName. Created by extractor.py.
LinksMap = Dict[str, str]

# -----------------------------------------------------------------------------
# Data types generated by transformer.py and artransformer.py
# -----------------------------------------------------------------------------

# Map of {str -> index}, for example, to store a sorted list of all LETTER
# strings with their associated self-index into the map. Created by
# artransformer.py.
IndexMap = Dict[str, int]

# Map of LETTER strings that are more than 1-character long, grouped by
# ZonePolicy. Allows the 'letter' index to be localzed to the given ZonePolicy
# (i.e. will only be 0 or 1). Created by artransformer.py. map{policy_name ->
# map{letter -> index}}
LettersPerPolicy = Dict[str, IndexMap]

# Map of {name -> Set[reason]} used by Transformer to collect de-duped error
# messages or warnings. A set() collection does not serialize well to JSON, so #
# jsongenerator.py will convert these into {name -> List[Comment]} internally.
# We use an Iterable[str] instead of a Union[Dict[], Dict[]] or even a Dict[str,
# Union[Set[], List[]] to avoid a LOT of headaches with mypy type checking.
CommentsMap = Dict[str, Iterable[str]]


class TransformerResult(NamedTuple):
    """Result type of Transformer.get_data().
    * zones_map: (zoneName -> ZoneEraRaw[]).
    * policies_map: (policyName -> ZoneRuleRaw[]).
    * links_map: (linkName -> zoneName)
    * removed_zones: {zoneName -> reasons[]}
    * removed_policies: {policyName -> reasons[]}
    * removed_links: {linkName -> reasons[]}
    * notable_zones: {zoneName -> reasons[]}
    * notable_policies: {policyName -> reasons[]}
    * notable_links: {linkName -> reasons[]}
    * zone_ids: {zoneName -> zoneHash}
    * letters_per_policy: {policyName -> {letter -> index}}
    * letters_map: {letter -> index}
    * formats_map: {format -> index}
    """
    zones_map: ZonesMap
    policies_map: PoliciesMap
    links_map: LinksMap
    removed_zones: CommentsMap
    removed_policies: CommentsMap
    removed_links: CommentsMap
    notable_zones: CommentsMap
    notable_policies: CommentsMap
    notable_links: CommentsMap
    zone_ids: Dict[str, int]
    letters_per_policy: LettersPerPolicy
    letters_map: IndexMap
    formats_map: IndexMap


def add_comment(comments: CommentsMap, name: str, reason: str) -> None:
    """Add the human readable 'reason' to the 'comments' CommentsMap.
    """
    reasons = cast(Optional[Set[str]], comments.get(name))
    if not reasons:
        reasons = set()
        comments[name] = reasons
    reasons.add(reason)


def merge_comments(target: CommentsMap, new: CommentsMap) -> None:
    """Merge 'new' CommentsMap into 'target' CommentsMap.
    """
    for name, new_reasons in new.items():
        old_reasons = cast(Optional[Set[str]], target.get(name))
        if not old_reasons:
            old_reasons = set()
            target[name] = old_reasons
        old_reasons.update(new_reasons)


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
    tz_version_number: int
    tz_files: List[str]
    scope: str
    start_year: int
    until_year: int
    until_at_granularity: int
    offset_granularity: int
    delta_granularity: int
    strict: bool
    num_zones: int

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

    # Data from ArduinoTransformer
    zone_ids: Dict[str, int]  # hash(zoneName)
    letters_per_policy: LettersPerPolicy  # multi-char letters by zonePolicy
    letters_map: IndexMap  # all multi-character letters
    formats_map: IndexMap  # shortened format strings.


def create_zone_info_database(
    tz_version: str,
    tz_files: List[str],
    scope: str,
    start_year: int,
    until_year: int,
    until_at_granularity: int,
    offset_granularity: int,
    delta_granularity: int,
    strict: bool,
    tresult: TransformerResult,
    buf_size_info: BufSizeInfo,
) -> ZoneInfoDatabase:
    """Return an instance of ZoneInfoDatabase from the various ingrediants."""

    return {
        # Context data.
        'tz_version': tz_version,
        'tz_version_number': _to_version_number(tz_version),
        'tz_files': tz_files,
        'scope': scope,
        'start_year': start_year,
        'until_year': until_year,
        'until_at_granularity': until_at_granularity,
        'offset_granularity': offset_granularity,
        'delta_granularity': delta_granularity,
        'strict': strict,
        'num_zones': len(tresult.zones_map),

        # Data from Extractor filtered through Transformer.
        'zones_map': tresult.zones_map,
        'policies_map': tresult.policies_map,
        'links_map': tresult.links_map,

        # Data from Transformer.
        'removed_zones': _sort_comments(tresult.removed_zones),
        'removed_links': _sort_comments(tresult.removed_links),
        'removed_policies': _sort_comments(tresult.removed_policies),
        'notable_zones': _sort_comments(tresult.notable_zones),
        'notable_links': _sort_comments(tresult.notable_links),
        'notable_policies': _sort_comments(tresult.notable_policies),

        # Data from BufSizeEstimator
        'buf_sizes': buf_size_info['buf_sizes'],
        'max_buf_size': buf_size_info['max_buf_size'],

        # Data from ArduinoTransformer
        'zone_ids': tresult.zone_ids,
        'letters_per_policy': tresult.letters_per_policy,
        'letters_map': tresult.letters_map,
        'formats_map': tresult.formats_map,
    }


def _sort_comments(comments: CommentsMap) -> CommentsMap:
    """Sort and convert {name -> Set(str)} to {name -> List(str)} to provide
    deterministic ordering.
    """
    return OrderedDict(
        (k, list(sorted(v)))
        for k, v in sorted(comments.items())
    )


def _to_version_number(version: str) -> int:
    """Convert version string (e.g. '2020a') to an integer of the form YYNN
    (e.g. '2001'), where YY is (year - 2000) and NN is the patch number,
    where 'a' is 01.
    """
    year = version[0:4]
    patch = version[4]
    return (int(year) - 2000) * 100 + (ord(patch) - ord('a') + 1)
