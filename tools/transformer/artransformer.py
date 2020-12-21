# Copyright 2020 Brian T. Park
#
# MIT License

from typing import NamedTuple
from typing import Optional
from collections import OrderedDict
import logging
from zonedb.data_types import ZonesMap
from zonedb.data_types import PoliciesMap
from zonedb.data_types import LettersMap
from zonedb.data_types import IndexedLetters
from zonedb.data_types import TransformerResult
from zonedb.data_types import EPOCH_YEAR
from zonedb.data_types import MAX_YEAR
from zonedb.data_types import MAX_YEAR_TINY
from zonedb.data_types import MIN_YEAR
from zonedb.data_types import MIN_YEAR_TINY
from zonedb.data_types import MAX_UNTIL_YEAR
from zonedb.data_types import MAX_UNTIL_YEAR_TINY
from zonedb.data_types import add_comment


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
        self.zones_map = tresult.zones_map
        self.policies_map = tresult.policies_map
        self.links_map = tresult.links_map
        self.scope = scope
        self.start_year = start_year
        self.until_year = until_year

    def transform(self) -> None:
        self.letters_map = _collect_letter_strings(self.policies_map)
        self.policies_map = self._process_rules(self.policies_map)
        self.zones_map = self._process_eras(self.zones_map)

    def get_data(self) -> TransformerResult:
        return TransformerResult(
            zones_map=self.zones_map,
            policies_map=self.policies_map,
            links_map=self.tresult.links_map,
            letters_map=self.letters_map,
            removed_zones=self.tresult.removed_zones,
            removed_policies=self.tresult.removed_policies,
            removed_links=self.tresult.removed_links,
            notable_zones=self.tresult.notable_zones,
            notable_policies=self.tresult.notable_policies,
            notable_links=self.tresult.notable_links,
        )

    def print_summary(self) -> None:
        logging.info(
            "Summary"
            f": Zones {len(self.zones_map)}"
            f"; Policies {len(self.policies_map)}"
            f"; Links {len(self.links_map)}"
        )

    def _process_rules(self, policies_map: PoliciesMap) -> PoliciesMap:
        for policy_name, rules in policies_map.items():
            for rule in rules:
                rule['fromYearTiny'] = _to_tiny_year(rule['fromYear'])
                rule['toYearTiny'] = _to_tiny_year(rule['toYear'])

                # Convert atSeconds to atTimeCode and atTimeModifier
                encoded_at_time = _to_encoded_time(
                    seconds=rule['atSecondsTruncated'],
                    suffix=rule['atTimeSuffix'],
                )
                rule['atTimeCode'] = encoded_at_time.time_code
                rule['atTimeMinute'] = encoded_at_time.time_minute
                rule['atTimeModifier'] = encoded_at_time.modifier

                # Check if AT is not on 15-minute boundary
                if encoded_at_time.time_minute != 0:
                    logging.info(
                        f"Notable policy: {policy_name}: "
                        f"AT '{rule['atTime']}' not on 15-minute boundary"
                    )
                    add_comment(
                        self.tresult.notable_policies, policy_name,
                        f"AT '{rule['atTime']}' not on 15-minute boundary"
                    )

                # These will always be integers because transformer.py
                # truncated them to 900 seconds appropriately.
                encoded_delta = _to_rule_offset(
                    delta_seconds=rule['deltaSecondsTruncated'],
                    scope=self.scope,
                )
                rule['deltaCode'] = encoded_delta.delta_code
                rule['deltaCodeEncoded'] = encoded_delta.delta_code_encoded

                letter = rule['letter']
                rule['letterIndex'] = _to_letter_index(
                    letter=letter,
                    indexed_letters=self.letters_map.get(policy_name),
                )
                if len(letter) > 1:
                    add_comment(
                        self.tresult.notable_policies, policy_name,
                        f"LETTER '{letter}' not single character"
                    )

        return self.policies_map

    def _process_eras(self, zones_map: ZonesMap) -> ZonesMap:
        for zone_name, eras in zones_map.items():
            for era in eras:

                # Determine the current delta seconds, based on the RULES field.
                rule_policy_name = era['rules']
                if rule_policy_name == ':':
                    delta_seconds = era['rulesDeltaSecondsTruncated']
                else:
                    delta_seconds = 0

                # Generate the STDOFF and DST delta offset codes.
                encoded_offset = _to_offset_and_delta(
                    offset_seconds=era['offsetSecondsTruncated'],
                    delta_seconds=delta_seconds,
                    scope=self.scope,
                )
                era['offsetCode'] = encoded_offset.offset_code
                era['offsetMinute'] = encoded_offset.offset_minute
                era['deltaCode'] = encoded_offset.delta_code
                era['deltaCodeEncoded'] = encoded_offset.delta_code_encoded

                # Check if STDOFF is not on 15-minute boundary
                if encoded_offset.offset_minute != 0:
                    logging.info(
                        f"Notable zone: {zone_name}: "
                        f"STDOFF '{era['offsetString']}' "
                        "not on 15-minute boundary"
                    )
                    add_comment(
                        self.tresult.notable_zones, zone_name,
                        f"STDOFF '{era['offsetString']}' "
                        "not on 15-minute boundary"
                    )

                # Generate the UNTIL fields needed by Arduino ZoneProcessors
                era['untilYearTiny'] = _to_tiny_until_year(era['untilYear'])
                encoded_until_time = _to_encoded_time(
                    seconds=era['untilSecondsTruncated'],
                    suffix=era['untilTimeSuffix'],
                )
                era['untilTimeCode'] = encoded_until_time.time_code
                era['untilTimeMinute'] = encoded_until_time.time_minute
                era['untilTimeModifier'] = encoded_until_time.modifier

                # Check if UNTIL is not on 15-minute boundary
                if encoded_until_time.time_minute != 0:
                    logging.info(
                        f"Notable zone: {zone_name}: "
                        f"UNTIL '{era['untilTime']}' not on 15-minute boundary"
                    )
                    add_comment(
                        self.tresult.notable_zones, zone_name,
                        f"UNTIL '{era['untilTime']}' not on 15-minute boundary"
                    )

                # FORMAT field for Arduino C++ replaces %s with just a %.
                era['formatShort'] = era['format'].replace('%s', '%')

        return self.zones_map


def _collect_letter_strings(policies_map: PoliciesMap) -> LettersMap:
    """Loop through all ZoneRules and collect the LETTERs which are
    more than one letter long into self.letters_map.
    """
    letters_map: LettersMap = OrderedDict()
    for policy_name, rules in sorted(policies_map.items()):
        letters = set()
        for rule in rules:
            if len(rule['letter']) > 1:
                letters.add(rule['letter'])

        if letters:
            indexed_letters: IndexedLetters = OrderedDict()
            index = 0
            for letter in sorted(letters):
                indexed_letters[letter] = index
                index += 1
            letters_map[policy_name] = indexed_letters
    return letters_map


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
    indexed_letters: Optional[IndexedLetters]
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
