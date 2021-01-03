# Copyright 2020 Brian T. Park
#
# MIT License

from typing import NamedTuple
from typing import Optional
from typing import Dict
from typing import Set
from typing import Tuple
from collections import OrderedDict, Counter
import itertools
import logging
from transformer.transformer import hash_name
from data_types.at_types import ZonesMap
from data_types.at_types import PoliciesMap
from data_types.at_types import LinksMap
from data_types.at_types import LettersPerPolicy
from data_types.at_types import IndexMap
from data_types.at_types import TransformerResult
from data_types.at_types import EPOCH_YEAR
from data_types.at_types import MAX_YEAR
from data_types.at_types import MAX_YEAR_TINY
from data_types.at_types import MIN_YEAR
from data_types.at_types import MIN_YEAR_TINY
from data_types.at_types import MAX_UNTIL_YEAR
from data_types.at_types import MAX_UNTIL_YEAR_TINY
from data_types.at_types import add_comment


class ArduinoTransformer:
    """Process the ZonesMap and PoliciesMap for the zone_info.{h,cpp} and
    zone_policies.{h,cpp} files required on Arduino. Produces a new
    TransformerResult from get_data().
    """

    def __init__(
        self,
        tresult: TransformerResult,
        scope: str,
        start_year: int,
        until_year: int,
    ) -> None:
        self.tresult = tresult
        self.scope = scope
        self.start_year = start_year
        self.until_year = until_year

        self.zones_map = tresult.zones_map
        self.policies_map = tresult.policies_map
        self.links_map = tresult.links_map

    def transform(self) -> None:
        self.letters_per_policy, self.letters_map = \
            _collect_letter_strings(self.policies_map)
        self.formats_map = _collect_format_strings(self.zones_map)
        self.policies_map = self._process_rules(self.policies_map)
        self.zones_map = self._process_eras(self.zones_map)
        self.zone_ids = _generate_zone_ids(self.zones_map)
        self.link_ids = _generate_link_ids(self.links_map)
        self.fragments_map = _generate_fragments(self.zones_map, self.links_map)

    def get_data(self) -> TransformerResult:
        return TransformerResult(
            zones_map=self.zones_map,
            policies_map=self.policies_map,
            links_map=self.tresult.links_map,
            removed_zones=self.tresult.removed_zones,
            removed_policies=self.tresult.removed_policies,
            removed_links=self.tresult.removed_links,
            notable_zones=self.tresult.notable_zones,
            notable_policies=self.tresult.notable_policies,
            notable_links=self.tresult.notable_links,
            zone_ids=self.zone_ids,
            link_ids=self.link_ids,
            letters_per_policy=self.letters_per_policy,
            letters_map=self.letters_map,
            formats_map=self.formats_map,
            fragments_map=self.fragments_map,
        )

    def print_summary(self) -> None:
        logging.info(
            "Summary"
            f": {len(self.zones_map)} Zones"
            f"; {len(self.policies_map)} Policies"
            f"; {len(self.links_map)} Links"
        )

    def _process_rules(self, policies_map: PoliciesMap) -> PoliciesMap:
        """Convert various ZoneRule fields into values that are consumed by the
        ZoneInfo and ZonePolicy classes of the Arduino AceTime library.
        """
        for policy_name, rules in policies_map.items():
            for rule in rules:
                rule['from_year_tiny'] = _to_tiny_year(rule['from_year'])
                rule['to_year_tiny'] = _to_tiny_year(rule['to_year'])

                # Convert at_seconds to at_time_code and at_time_modifier
                encoded_at_time = _to_encoded_time(
                    seconds=rule['at_seconds_truncated'],
                    suffix=rule['at_time_suffix'],
                )
                rule['at_time_code'] = encoded_at_time.time_code
                rule['at_time_minute'] = encoded_at_time.time_minute
                rule['at_time_modifier'] = encoded_at_time.modifier

                # Check if AT is not on 15-minute boundary
                if encoded_at_time.time_minute != 0:
                    logging.info(
                        f"Notable policy: {policy_name}: "
                        f"AT '{rule['at_time']}' not on 15-minute boundary"
                    )
                    add_comment(
                        self.tresult.notable_policies, policy_name,
                        f"AT '{rule['at_time']}' not on 15-minute boundary"
                    )

                # These will always be integers because transformer.py
                # truncated them to 900 seconds appropriately.
                encoded_delta = _to_rule_offset(
                    delta_seconds=rule['delta_seconds_truncated'],
                    scope=self.scope,
                )
                rule['delta_code'] = encoded_delta.delta_code
                rule['delta_code_encoded'] = encoded_delta.delta_code_encoded

                # Get letter indexes, per policy and global
                letter = rule['letter']
                rule['letter_index'] = _to_letter_index(
                    letter=letter,
                    indexed_letters=self.letters_map,
                )
                rule['letter_index_per_policy'] = _to_letter_index(
                    letter=letter,
                    indexed_letters=self.letters_per_policy.get(policy_name),
                )
                if len(letter) > 1:
                    add_comment(
                        self.tresult.notable_policies, policy_name,
                        f"LETTER '{letter}' not single character"
                    )

        return self.policies_map

    def _process_eras(self, zones_map: ZonesMap) -> ZonesMap:
        """Convert various ZoneRule fields into values that are consumed by the
        ZoneInfo and ZonePolicy classes of the Arduino AceTime library.
        """
        for zone_name, eras in zones_map.items():
            for era in eras:

                # Determine the current delta seconds, based on the RULES field.
                rule_policy_name = era['rules']
                if rule_policy_name == ':':
                    delta_seconds = era['rules_delta_seconds_truncated']
                else:
                    delta_seconds = 0

                # Generate the STDOFF and DST delta offset codes.
                encoded_offset = _to_offset_and_delta(
                    offset_seconds=era['offset_seconds_truncated'],
                    delta_seconds=delta_seconds,
                    scope=self.scope,
                )
                era['offset_code'] = encoded_offset.offset_code
                era['offset_minute'] = encoded_offset.offset_minute
                era['delta_code'] = encoded_offset.delta_code
                era['delta_code_encoded'] = encoded_offset.delta_code_encoded

                # Check if STDOFF is not on 15-minute boundary
                if encoded_offset.offset_minute != 0:
                    logging.info(
                        f"Notable zone: {zone_name}: "
                        f"STDOFF '{era['offset_string']}' "
                        "not on 15-minute boundary"
                    )
                    add_comment(
                        self.tresult.notable_zones, zone_name,
                        f"STDOFF '{era['offset_string']}' "
                        "not on 15-minute boundary"
                    )

                # Generate the UNTIL fields needed by Arduino ZoneProcessors
                era['until_year_tiny'] = _to_tiny_until_year(era['until_year'])
                encoded_until_time = _to_encoded_time(
                    seconds=era['until_seconds_truncated'],
                    suffix=era['until_time_suffix'],
                )
                era['until_time_code'] = encoded_until_time.time_code
                era['until_time_minute'] = encoded_until_time.time_minute
                era['until_time_modifier'] = encoded_until_time.modifier

                # Check if UNTIL is not on 15-minute boundary
                if encoded_until_time.time_minute != 0:
                    logging.info(
                        f"Notable zone: {zone_name}: "
                        f"UNTIL '{era['until_time']}' not on 15-minute boundary"
                    )
                    add_comment(
                        self.tresult.notable_zones, zone_name,
                        f"UNTIL '{era['until_time']}' not on 15-minute boundary"
                    )

                # FORMAT field for Arduino C++ replaces %s with just a %.
                era['format_short'] = era['format'].replace('%s', '%')

        return self.zones_map


def _collect_letter_strings(
    policies_map: PoliciesMap,
) -> Tuple[LettersPerPolicy, IndexMap]:
    """Loop through all ZoneRules and collect:
    1) a sorted collection of all multi-LETTERs, with their self index,
    2) collection of multi-LETTERs, grouped by policyName
    """
    letters_per_policy: LettersPerPolicy = OrderedDict()
    all_letters: Set[str] = set()
    for policy_name, rules in sorted(policies_map.items()):
        policy_letters: Set[str] = set()
        for rule in rules:
            letter = rule['letter']
            if len(letter) > 1:
                all_letters.add(letter)
                policy_letters.add(letter)

        if policy_letters:
            indexed_letters: IndexMap = OrderedDict()
            index = 0
            for letter in sorted(policy_letters):
                indexed_letters[letter] = index
                index += 1
            letters_per_policy[policy_name] = indexed_letters

    # Create a map of all multi-letters
    index = 0
    letters_map: IndexMap = OrderedDict()
    for letter in sorted(all_letters):
        letters_map[letter] = index
        index += 1

    return letters_per_policy, letters_map


def _collect_format_strings(zones_map: ZonesMap) -> IndexMap:
    """Collect the 'formats' field and return a map of indexes."""
    short_formats: Set[str] = set()
    for zone_name, eras in zones_map.items():
        for era in eras:
            format = era['format']
            short_format = format.replace('%s', '%')
            short_formats.add(short_format)

    index = 0
    short_formats_map: IndexMap = OrderedDict()
    for short_format in sorted(short_formats):
        short_formats_map[short_format] = index
        index += 1

    return short_formats_map


def _to_tiny_year(year: int) -> int:
    """Convert 16-bit year into 8-bit year, taking into account special
    values for MIN and MAX years.
    """
    if year == MAX_YEAR:
        return MAX_YEAR_TINY
    elif year == MIN_YEAR:
        return MIN_YEAR_TINY
    else:
        return year - EPOCH_YEAR


def _to_tiny_until_year(year: int) -> int:
    """Convert 16-bit UNTIL year into 8-bit UNTIL year, taking into account
    special values for MIN and MAX years.
    """
    if year == MAX_UNTIL_YEAR:
        return MAX_UNTIL_YEAR_TINY
    elif year == MIN_YEAR:
        return MIN_YEAR_TINY
    else:
        return year - EPOCH_YEAR


class EncodedTime(NamedTuple):
    """Break apart a time in seconds with a suffix (e.g. 02:00w) into the
    following parts so that it can be encoded in 2 bytes with a resolution of
    1-minute:

        * time_code: Time of day, in units of 15 minutes. Since time_code will
          be placed in an 8-bit field with a range of -127 to 127 (-128 is an
          error flag), the range of time that this can represent is -31:45 to
          +31:59. I believe all time of day in the TZ database files are
          positive, but it will occasionally have time strings of "25:00" which
          means 1am the next day.
        * time_minute: Remainder minutes (if any) which will be placed in the
          bottom 4-bits (0-14) of the modifier. This quantity is already
          included in modifier, so the purpose of this field is to allow
          the caller to check for a non-zero value for logging purposes.
        * suffix_code: An integer code that can be placed in the top 4-bits
          (e.g. 0x00, 0x10, 0x20).
        * modifier: suffix_code + time_minute

    (Note: In hindsight, I probably should have flipped the top and bottom 4-bit
    locations of the suffix_code an time_minute, so that the
    EncodedTime.time_minute field is in the same location as
    EncodedOffset.time_minute field.)
    """
    time_code: int
    time_minute: int
    suffix_code: int
    modifier: int


def _to_encoded_time(
    seconds: int,
    suffix: str,
) -> EncodedTime:
    """Return the EncodedTime tuple that represents the AT or UNTIL time, with a
    resolution of 1-minute, along with an encoding of its suffix (i.e. 's', 'w',
    'u').
    """
    time_code = seconds // 900
    time_minute = seconds % 900 // 60
    suffix_code = _to_suffix_code(suffix)
    modifier = time_minute + suffix_code
    return EncodedTime(
        time_code=time_code,
        time_minute=time_minute,
        suffix_code=suffix_code,
        modifier=modifier,
    )


def _to_suffix_code(suffix: str) -> int:
    """Return the integer code corresponding to 'w', 's', and 'u' suffix
    character in the TZ database files that can be placed in the top 4-bits of
    the 'modifier' field. Corresponds to the kSuffixW, kSuffixS, kSuffixU
    constants in ZoneContext.h.
    """
    if suffix == 'w':
        return 0x00
    elif suffix == 's':
        return 0x10
    elif suffix == 'u':
        return 0x20
    else:
        raise Exception(f'Unknown suffix {suffix}')


class EncodedRuleOffset(NamedTuple):
    """Encode the DST offset extracted from the SAVE column of the Rule entries.

    * delta_code: delta offset in units of 15-min
    * delta_code_encoded:
        * basic: same as delta_code
        * extended: delta_code + 4 (1h)
    """
    delta_code: int
    delta_code_encoded: int


def _to_rule_offset(
    delta_seconds: int,
    scope: str,
) -> EncodedRuleOffset:
    """Convert the delta_seconds extracted from the SAVE column of a RULE entry
    to an EncodedRuleOffset. The transformer.py ensures that all entries are in
    multiples of 15-minutes, so we don't need to worry about remainder minutes.
    """
    delta_code = delta_seconds // 900
    # TODO: Maybe the encoding should be unified between 'basic' and 'extended'
    if scope == 'extended':
        delta_code_encoded = delta_code + 4
    else:
        delta_code_encoded = delta_code
    return EncodedRuleOffset(
        delta_code=delta_code,
        delta_code_encoded=delta_code_encoded,
    )


class EncodedOffset(NamedTuple):
    """Encode the STD offset and DST offset into 2 8-bit integer fields.

    * offset_code: STD offset in units of 15-minutes
    * offset_minute: Remainder minutes (must be always 0 for scope=basic).
        This quantity is already included in delta_code, so the purpose of
        this field is to allow the caller to check for a non-zero value
        and log a warning or error message.
    * delta_code: delta offset in units of 15-minutes
    * delta_code_encoded:
        * basic: same as delta_code
        * extended: The lower 4-bits is delta_code + 4 (i.e. 1h). Allows
            encoding from -1:00 to +2:45. The upper 4-bits holds the
            offset_minute.
    """
    offset_code: int
    offset_minute: int
    delta_code: int
    delta_code_encoded: int


def _to_offset_and_delta(
    offset_seconds: int,
    delta_seconds: int,
    scope: str,
) -> EncodedOffset:
    """Convert offset_seconds and delta_seconds to an EncodedOffset suitable for
    a BasicZoneProcessor or ExtendedZoneProcessor.
    """
    offset_code = offset_seconds // 900  # truncate to -infinty
    offset_minute = (offset_seconds % 900) // 60  # always positive
    delta_code = delta_seconds // 900
    if scope == 'extended':
        delta_code_encoded = (offset_minute << 4) + (delta_code + 4)
    else:
        delta_code_encoded = delta_code

    return EncodedOffset(
        offset_code=offset_code,
        offset_minute=offset_minute,
        delta_code=delta_code,
        delta_code_encoded=delta_code_encoded,
    )


def _to_letter_index(
    letter: str,
    indexed_letters: Optional[IndexMap]
) -> int:
    """
    Return an index into the indexed_letters if len(letter) > 1.
    Otherwise if letter is 1-character long, return -1.
    """
    if len(letter) > 1:
        if not indexed_letters:
            raise Exception(
                f'No indexed_letters provided for len({letter}) > 1')
        letter_index = indexed_letters[letter]
        if letter_index >= 32:
            raise Exception('Number of indexed letters >= 32')
    elif len(letter) == 1:
        letter_index = -1
    else:
        raise Exception('len(letter) == 0; should not happen')

    return letter_index


def _generate_zone_ids(
    zones_map: ZonesMap,
) -> Dict[str, int]:
    """Generate {zoneName -> zoneId} map of zones."""
    ids: Dict[str, int] = {name: hash_name(name) for name in zones_map.keys()}
    return OrderedDict(sorted(ids.items()))


def _generate_link_ids(
    links_map: LinksMap,
) -> Dict[str, int]:
    """Generate {linkName -> linkId} map of links."""
    ids: Dict[str, int] = {name: hash_name(name) for name in links_map.keys()}
    return OrderedDict(sorted(ids.items()))


def _generate_fragments(zones_map: ZonesMap, links_map: LinksMap) -> IndexMap:
    """Generate a list of fragments and their indexes, sorted by fragment.
    E.g. { "Africa": 1, "America": 2, ... }
    """
    # Collect the frequency of fragments longer than 3 characters
    fragments: Dict[str, int] = Counter()
    for name in itertools.chain(zones_map.keys(), links_map.keys()):
        fragment = _extract_fragment(name)
        if len(fragment) > 3:
            fragments[fragment] += 1

    # Collect fragments which occur more than 3 times.
    fragments_map: IndexMap = OrderedDict()
    index = 1  # start at 1 because '\0' is the c-string termination char
    for fragment, count in sorted(fragments.items()):
        if count > 3:
            fragments_map[fragment] = index
            index += 1
        else:
            logging.info(
                f"Ignoring fragment '{fragment}' with count {count}, too few"
            )

    # Make sure that index is < 32, before ASCII-space.
    if index >= 32:
        raise Exception("Too many fragments {index}")

    return fragments_map


def _extract_fragment(name: str) -> str:
    """Return the fragment before '/' or None if no '/' in name."""
    pos = name.find('/')
    if pos < 0:
        return ""
    return name[:pos]
