# Copyright 2020 Brian T. Park
#
# MIT License
"""
Process the ZonesMap and PoliciesMap for the zonedb files required on Arduino.
Produce ArZonesMap, ArPoliciesMap.
"""

from typing import Tuple
from typing import Optional
from collections import OrderedDict
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


class ArduinoTransformer:

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

    def _process_rules(self, policies_map: PoliciesMap) -> PoliciesMap:
        for policy_name, rules in policies_map.items():
            for rule in rules:
                rule['fromYearTiny'] = _to_tiny_year(rule['fromYear'])
                rule['toYearTiny'] = _to_tiny_year(rule['toYear'])

                at_time_code, at_time_modifier = _to_code_and_modifier(
                    seconds=rule['atSecondsTruncated'],
                    suffix=rule['atTimeSuffix'],
                )
                rule['atTimeCode'] = at_time_code
                rule['atTimeModifier'] = at_time_modifier

                # These will always be integers because transformer.py
                # truncated them to 900 seconds appropriately.
                if self.scope == 'extended':
                    delta_code = rule['deltaSecondsTruncated'] // 900 + 4
                else:
                    delta_code = rule['deltaSecondsTruncated'] // 900
                rule['deltaCode'] = delta_code

                rule['letterIndex'] = _to_letter_index(
                    letter=rule['letter'],
                    indexed_letters=self.letters_map.get(policy_name),
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
                if self.scope == 'extended':
                    offset_code, delta_code = _to_extended_offset_and_delta(
                        era['offsetSecondsTruncated'], delta_seconds)
                else:
                    offset_code, delta_code = _to_basic_offset_and_delta(
                        era['offsetSecondsTruncated'], delta_seconds)
                era['offsetCode'] = offset_code
                era['deltaCode'] = delta_code

                # Generate the UNTIL fields
                era['untilYearTiny'] = _to_tiny_until_year(era['untilYear'])
                until_time_code, until_time_modifier = _to_code_and_modifier(
                    seconds=era['untilSecondsTruncated'],
                    suffix=era['untilTimeSuffix'],
                )
                era['untilTimeCode'] = until_time_code
                era['untilTimeModifier'] = until_time_modifier

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


def _to_code_and_modifier(
    seconds: int,
    suffix: str,
) -> Tuple[int, int]:
    """Return the packed (code, modifier) uint8_t integers that hold
    the AT or UNTIL timeCode, timeMinute and the suffix.
    """
    time_code = seconds // 900
    time_minute = seconds % 900 // 60
    modifier_code = _to_modifier_code(suffix)
    return time_code, time_minute + modifier_code


def _to_modifier_code(suffix: str) -> int:
    """Return the modifier integer code corresponding to 'w', 's', and 'u'
    suffix character in the TZ database files. Corresponds to the kSuffixW,
    kSuffixS, kSuffixU constants in ZoneContext.h.
    """
    if suffix == 'w':
        return 0x0
    elif suffix == 's':
        return 0x10
    elif suffix == 'u':
        return 0x20
    else:
        raise Exception(f'Unknown suffix {suffix}')


def _to_basic_offset_and_delta(
    offset_seconds: int,
    delta_seconds: int,
) -> Tuple[int, int]:
    """Return the (offset_code, delta_code) suitable for a BasicZoneProcessor.
    Both the offset_code and delta_code have a 15-minute resolution.
    """
    offset_code = offset_seconds // 900
    delta_code = delta_seconds // 900
    return offset_code, delta_code


def _to_extended_offset_and_delta(
    offset_seconds: int,
    delta_seconds: int,
) -> Tuple[int, int]:
    """Return the (offset_code, delta_code) suitable for an
    ExtendedZoneProcessor which maintains a one-minute resolution for
    offset_seconds.

    * The offset_seconds is stored as the 'offset_code' in multiples of
      15-minutes.
    * The remaining offsetMinutes is stored in the top 4-bits of the 'deltaCode'
      field.
    * The lower 4-bits of 'deltaCode' stores the 'delta_seconds' in multiples of
      15 minutes, shifted by one hour so that it can represent a DST shift in
      the range of -1:00 to +2:45.

      The 'deltaCode' field of the tuple is returned as a string containing the
      C++ expression to allow easier debugging.
    """
    offset_code = offset_seconds // 900  # truncate to -infinty
    offset_minute = (offset_seconds % 900) // 60  # always positive

    # Calculate the base_delta_code in units of 15 minutes, offset by 1h:
    # (delta_seconds + 1h) / 15m so that it's always positive. We can store that
    # in the lower 4-bits of the uint8_t field, which will handle delta_seconds
    # from -1:00h to +2:45h
    base_delta_code = delta_seconds // 900 + 4

    return offset_code, (offset_minute << 4) + base_delta_code


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
