#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Cleanses and transforms the Zone, Rule and Link entries for processing by
various AceTime algorithms. The data will be consumed by code generation classes
(ArduinoGenerator, PythonGenerator) or by the InlineZoneInfo to generate zone
info records internally.
"""

import logging
import sys
import re
import datetime
from collections import OrderedDict
from typing import Dict
from typing import List
from typing import Optional
from typing import Set
from typing import Tuple
from typing import cast
from typing_extensions import TypedDict
from .extractor import MAX_UNTIL_YEAR
from .extractor import MIN_YEAR
from .extractor import MAX_YEAR
from .data_types import ZoneRuleRaw
from .data_types import ZoneEraRaw
from .data_types import ZonesMap
from .data_types import PoliciesMap
from .data_types import LinksMap
from .data_types import TransformerResult

# Map of zoneName -> Set[Comment] used internally by Transformer to collect
# de-duped error messages or warnings. Exported as data_types.CommentsMap.
CommentsCollection = Dict[str, Set[str]]

# Map of policyName -> zoneName[] used internally by Transformer.  The list of
# all Zones which references the given policy. TODO: Should probably be renamed
# PoliciesToZones.
RulesToZones = Dict[str, List[str]]


class Transformer:
    def __init__(
        self,
        zones_map: ZonesMap,
        policies_map: PoliciesMap,
        links_map: LinksMap,
        scope: str,
        start_year: int,
        until_year: int,
        until_at_granularity: int,
        offset_granularity: int,
        strict: bool,
    ):
        """
        Args:
            zones_map: Zone names to ZoneEras
            policies_map: Policy names to ZoneRules
            links_map: Link name to Zone Name
            scope: scope of database (basic, or extended)
            start_year: include only years on or after start_year
            until_year: include only years valid before until_year
            until_at_granularity: truncate UNTIL, AT to this many seconds
            offset_granularity: SAVE, RULES(offset) to this many seconds
            strict: throw out Zones or Rules which are not exactly
                on the time boundary defined by granularity
        """
        self.zones_map = zones_map
        self.policies_map = policies_map
        self.links_map = links_map
        self.scope = scope
        self.start_year = start_year
        self.until_year = until_year
        self.until_at_granularity = until_at_granularity
        self.offset_granularity = offset_granularity
        self.strict = strict

        self.original_zone_count = len(zones_map)
        self.original_rule_count = len(policies_map)
        self.original_link_count = len(links_map)

        self.all_removed_zones: CommentsCollection = {}  # name -> reason[]
        self.all_removed_policies: CommentsCollection = {}  # name -> reason[]
        self.all_removed_links: CommentsCollection = {}  # link -> reason[]

        self.all_notable_zones: CommentsCollection = {}  # zone -> reason[]
        self.all_notable_policies: CommentsCollection = {}  # policy -> reason[]
        self.all_notable_links: CommentsCollection = {}  # link -> reason[]

    def transform(self) -> None:
        """
        Transforms the zones_map and policies_map given in the constructor
        through a series of filters, and produces the following results
        which can be retrieved using the get_zone_data() function.

        * self.zones_map: map of (zoneName -> ZoneEraRaw[]).
        * self.policies_map: map of (policyName -> ZoneRuleRaw[]).
        * self.links_map: map of (linkName -> zoneName)
        * self.all_removed_zones: map of the zones which were removed:
            name: name of zone
            reasons: human readable reasons
        * self.all_removed_policies: map of the policies which were removed:
            name: name of policy
            reasons: human readable reasons
        * self.all_removed_links: map of removed
            name: name of link
            reasons: human readable reasons
         self.all_notable_zones: map of the zones with caveats,
            e.g., truncation of '00:01' to '00:00'.
            name: name of zone
            reasons: human readable reasons
        * self.all_notable_policies: map of policies with caveats:
            name: name of policy
            reasons: human readable reasons
        * self.all_notable_links: map of links with caveats:
            name: name of link
            reasons: human readable reasons
        """

        zones_map = self.zones_map
        policies_map = self.policies_map
        links_map = self.links_map

        logging.info('Found %s zone infos', len(self.zones_map))
        logging.info('Found %s rule policies', len(self.policies_map))

        # Part 1: Transform the zones_map
        # zones_map = self._remove_zones_without_slash(zones_map)
        zones_map = self._detect_hash_collisions(zones_map)
        zones_map = self._remove_zone_eras_too_old(zones_map)
        zones_map = self._remove_zone_eras_too_new(zones_map)
        zones_map = self._remove_zones_without_eras(zones_map)
        if self.scope == 'basic':
            zones_map = self._remove_zone_until_year_only_false(zones_map)
        zones_map = self._create_zones_with_until_day(zones_map)
        zones_map = self._create_zones_with_expanded_until_time(zones_map)
        zones_map = self._remove_zones_invalid_until_time_suffix(zones_map)
        zones_map = self._create_zones_with_expanded_offset_string(zones_map)
        zones_map = self._remove_zones_with_invalid_rules_format_combo(
            zones_map)
        zones_map = self._create_zones_with_rules_expansion(zones_map)
        zones_map = self._remove_zones_with_non_monotonic_until(zones_map)

        # Part 2: Transformations requring both zones_map and policies_map.
        (zones_map, policies_map) = self._mark_rules_used_by_zones(
            zones_map, policies_map)
        rules_to_zones = _create_rules_to_zones(zones_map, policies_map)

        # Part 3: Transform the policies_map
        policies_map = self._remove_rules_unused(policies_map)
        policies_map = self._remove_rules_out_of_bounds(policies_map)
        if self.scope == 'basic':
            policies_map = self._remove_rules_multiple_transitions_in_month(
                policies_map)
        policies_map = self._create_rules_with_expanded_at_time(
            policies_map, rules_to_zones)
        policies_map = self._remove_rules_invalid_at_time_suffix(policies_map)
        policies_map = self._create_rules_with_expanded_delta_offset(
            policies_map)
        policies_map = self._create_rules_with_on_day_expansion(policies_map)
        policies_map = self._create_rules_with_anchor_transition(policies_map)
        if self.scope == 'basic':
            policies_map = self._remove_rules_with_border_transitions(
                policies_map)
        if self.scope == 'basic':
            policies_map = self._remove_rules_long_dst_letter(policies_map)

        # Part 4: Go back to zones_map and remove unused.
        zones_map = self._remove_zones_without_rules(zones_map, policies_map)

        # Part 5: Remove links which point to removed zones.
        links_map = self.remove_links_to_missing_zones(links_map, zones_map)

        # Part 6: Remove zones and links whose normalized names conflict.
        # For example, "GTM+0" and "GMT-0" will normalize to the same
        # kZoneGMT_0, so cannot be used.
        zones_map, links_map = self.remove_zones_and_links_with_similar_names(
            zones_map, links_map)

        # Part 7: Replace the original maps with the transformed ones.
        self.policies_map = policies_map
        self.zones_map = zones_map
        self.links_map = links_map

    def get_data(self) -> TransformerResult:
        """Return the result of the transform() operation. The fields of type
        CommentsCollection (using Set) are converted to CommentsMap (using List)
        to allow direct serialization to JSON.
        """
        return TransformerResult(
            zones_map=self.zones_map,
            policies_map=self.policies_map,
            links_map=self.links_map,
            removed_zones={
                k: list(v)
                for k, v in self.all_removed_zones.items()
            },
            removed_policies={
                k: list(v)
                for k, v in self.all_removed_policies.items()
            },
            removed_links={
                k: list(v)
                for k, v in self.all_removed_links.items()
            },
            notable_zones={
                k: list(v)
                for k, v in self.all_notable_zones.items()
            },
            notable_policies={
                k: list(v)
                for k, v in self.all_notable_policies.items()
            },
            notable_links={
                k: list(v)
                for k, v in self.all_notable_links.items()
            },
        )

    def print_summary(self) -> None:
        logging.info(
            f"Summary: Zones: total={self.original_zone_count}"
            f"; generated={len(self.zones_map)}"
            f"; removed={len(self.all_removed_zones)}"
            f"; noted={len(self.all_notable_zones)}")

        logging.info(
            f"Summary: Rules: total={self.original_rule_count}"
            f"; generated={len(self.policies_map)}"
            f"; removed={len(self.all_removed_policies)}"
            f"; noted={len(self.all_notable_policies)}")

        logging.info(
            f"Summary: Links: total={self.original_link_count}"
            f"; generated={len(self.links_map)}"
            f"; removed={len(self.all_removed_links)}"
            f"; noted={len(self.all_notable_links)}")

    def _print_removed_map(self, removed_map: CommentsCollection) -> None:
        """Helper routine that prints the removed Zone rules or Zone eras along
        with the reason why it was removed.
        """
        name: str
        reasons: Set[str]
        for name, reasons in sorted(removed_map.items()):
            print(f'  {name} ({reasons})', file=sys.stderr)

    # --------------------------------------------------------------------
    # Methods related to Zones.
    # --------------------------------------------------------------------

    def _remove_zones_without_slash(self, zones_map: ZonesMap) -> ZonesMap:
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            if name.rfind('/') >= 0:
                results[name] = eras
            else:
                _add_reason(removed_zones, name, "No '/' in zone name")

        logging.info(
            "Removed %s zone infos without '/' in name",
            len(removed_zones)
        )
        _merge_reasons(self.all_removed_zones, removed_zones)
        return results

    def _detect_hash_collisions(self, zones_map: ZonesMap) -> ZonesMap:
        """Detect a hash collision of a zone name and throw an exception. With
        only a few hundred zone names and a 32-bit hash, the chances of a
        collision is extremely low. However, if it ever happens, it is a severe
        error because we must guarantee that each zone name has a unique and
        stable hash for the life of this library. If this exception ever
        happens, we must create another hash for the colliding zone name, and
        keep the second hash unique and stable as well.
        """
        hashes: Dict[int, str] = {}
        for name, _ in zones_map.items():
            h = hash_name(name)
            colliding_name = hashes.get(h)
            if colliding_name:
                raise Exception("Hash collision: {name} and {colliding_name}")
            else:
                hashes[h] = name
        return zones_map

    def _remove_zone_eras_too_old(self, zones_map: ZonesMap) -> ZonesMap:
        """Remove zone eras which are too old, i.e. before (self.start_year-1).
        For start_year 2000, and viewing_months>13,
        ZoneSpecifier.init_for_year() could be called with 1999.
        """
        results: ZonesMap = {}
        count = 0
        for name, eras in zones_map.items():
            keep_eras: List[ZoneEraRaw] = []
            for era in eras:
                if era['untilYear'] >= self.start_year - 1:
                    keep_eras.append(era)
                else:
                    count += 1
            if keep_eras:
                results[name] = keep_eras

        logging.info(
            'Removed %s zone eras before year %04d',
            count,
            self.start_year,
        )
        return results

    def _remove_zone_eras_too_new(self, zones_map: ZonesMap) -> ZonesMap:
        """Remove zone eras which are too new, i.e. after self.until_year.
        We need at least one year after the last valid year (i.e. until_year),
        so we need zone eras valid to at least until_year. For
        viewing_months=36, we need until_year + 1. So let's remove zone eras
        which starts at until_year + 2 or greater.

        TODO: If a zone era is removed because it is too far in the future, it
        is no longer guaranteed that the last zone era ends with MAX_UNTIL_YEAR.
        If the ZoneSpecifier code is called with a year greater than
        self.until_year, it may cause a loop to crash.
        """
        results: ZonesMap = {}
        count = 0
        for name, eras in zones_map.items():
            keep_eras: List[ZoneEraRaw] = []
            start_year = MIN_YEAR
            for era in eras:
                if start_year <= self.until_year + 1:
                    keep_eras.append(era)
                else:
                    count += 1
                # the next era's start year is this era's until_year
                start_year = era['untilYear']
            if keep_eras:
                results[name] = keep_eras

        logging.info(
            "Removed %s zone eras starting after %04d",
            count,
            self.until_year,
        )
        return results

    def _remove_zones_without_eras(self, zones_map: ZonesMap) -> ZonesMap:
        """Remove zones without any eras, which can happen if the start_year and
        until_year are too narrow. This prevents the C++ code from crashing.
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            if eras:
                results[name] = eras
            else:
                _add_reason(removed_zones, name, "no ZoneEra found")

        logging.info(
            "Removed %s zone infos without ZoneEras", len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        return results

    def _remove_zone_until_year_only_false(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Remove zones which have month, day or time in the UNTIL field.
        These are not supported by BasicZoneSpecifier.
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                if not era['untilYearOnly']:
                    valid = False
                    _add_reason(
                        removed_zones, name, "UNTIL contains month/day/time")
                    break
            if valid:
                results[name] = eras

        logging.info("Removed %s zone infos with UNTIL month/day/time",
                     len(removed_zones))
        _merge_reasons(self.all_removed_zones, removed_zones)
        return results

    def _create_zones_with_until_day(self, zones_map: ZonesMap) -> ZonesMap:
        """Convert zone.untilDay from 'lastSun' or 'Sun>=1' to a precise day,
        which is possible because the year and month are already known. For
        example:
            * Zone Asia/Tbilisi 2005 3 lastSun 2:00
            * Zone America/Grand_Turk 2015 Nov Sun>=1 2:00
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        notable_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                until_day_string = era['untilDayString']

                # Parse the conditional expression in until_day_string. We can
                # resolve the 'lastSun', 'Sun>=X' and 'Fri<=X' to a specific day
                # of month because we know the year.
                (on_day_of_week, on_day_of_month) = \
                    _parse_on_day_string(until_day_string)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"invalid untilDay '{until_day_string}'")
                    break

                month, day = calc_day_of_month(
                    era['untilYear'], era['untilMonth'], on_day_of_week,
                    on_day_of_month)
                if month == 0:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"Shift to previous year unsupported for "
                        f"{until_day_string}")
                    break
                if month == 13:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"Shift to following year unsupported for "
                        f"{until_day_string}")

                if era['untilMonth'] != month:
                    _add_reason(
                        notable_zones, name,
                        f"untilMonth shifted from '{era['untilMonth']}' to "
                        f"'{month}' due to {until_day_string}")
                era['untilMonth'], era['untilDay'] = month, day

            if valid:
                results[name] = eras

        logging.info("Removed %s zone infos with invalid untilDay",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        _merge_reasons(self.all_notable_zones, notable_zones)
        return results

    def _create_zones_with_expanded_until_time(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """ Create 'untilSeconds' and 'untilSecondsTruncated' from 'untilTime'.
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        notable_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                until_time = era['untilTime']
                until_seconds = time_string_to_seconds(until_time)
                if until_seconds == INVALID_SECONDS:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"invalid UNTIL time '{until_time}'")
                    break
                if until_seconds < 0:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"negative UNTIL time '{until_time}'")
                    break

                until_seconds_truncated = truncate_to_granularity(
                    until_seconds, self.until_at_granularity)
                if until_seconds != until_seconds_truncated:
                    if self.strict:
                        valid = False
                        _add_reason(
                            removed_zones, name,
                            f"UNTIL time '{until_time}' must be multiples "
                            f"of '{self.until_at_granularity}' seconds")
                        break
                    else:
                        hm = seconds_to_hm_string(until_seconds_truncated)
                        _add_reason(
                            notable_zones, name,
                            f"UNTIL time '{until_time}' truncated to '{hm}'")

                era['untilSeconds'] = until_seconds
                era['untilSecondsTruncated'] = until_seconds_truncated
            if valid:
                results[name] = eras

        logging.info("Removed %s zone infos with invalid UNTIL time",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        _merge_reasons(self.all_notable_zones, notable_zones)
        return results

    def _remove_zones_invalid_until_time_suffix(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Remove zones whose UNTIL time contains an unsupported suffix.
        """
        # Define the supported time suffices. Basic supports only 'w', while
        # Extended supports all suffixes. The 'g' and 'z' is the same as 'u' and
        # does not currently appear in any TZ file, so let's catch it because it
        # could indicate a bug
        if self.scope == 'basic':
            supported_suffices = ['w']
        else:
            supported_suffices = ['w', 's', 'u']

        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                suffix = era['untilTimeSuffix']
                suffix = suffix if suffix else 'w'
                era['untilTimeSuffix'] = suffix
                if suffix not in supported_suffices:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"unsupported UNTIL time suffix '{suffix}'")
                    break
            if valid:
                results[name] = eras

        logging.info(
            "Removed %s zone infos with unsupported UNTIL time suffix",
            len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_policies, removed_zones)
        return results

    def _create_zones_with_expanded_offset_string(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """ Create expanded offset 'offsetSeconds' from zone.offsetString.
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        notable_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                offset_string = era['offsetString']
                offset_seconds = time_string_to_seconds(offset_string)
                if offset_seconds == INVALID_SECONDS:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"invalid STDOFF '{offset_string}'")
                    break

                # Truncate to requested granularity.
                offset_seconds_truncated = truncate_to_granularity(
                    offset_seconds, self.offset_granularity)
                if offset_seconds != offset_seconds_truncated:
                    if self.strict:
                        valid = False
                        _add_reason(
                            removed_zones, name,
                            f"STDOFF '{offset_string}' must be multiples of "
                            f"'{self.offset_granularity}' seconds")
                        break
                    else:
                        hm = seconds_to_hm_string(offset_seconds_truncated)
                        _add_reason(
                            notable_zones, name,
                            f"STDOFF '{offset_string}' truncated to '{hm}'")

                # Check that offset seconds can fit in a timeCode field
                # implemented as a signed byte in multiples of 15-minutes.
                offset_code = div_to_zero(offset_seconds_truncated, 900)
                if offset_code < -127 or offset_code > 127:
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"STDOFF '{offset_string}' too large for 8-bits")
                    break

                era['offsetSeconds'] = offset_seconds
                era['offsetSecondsTruncated'] = offset_seconds_truncated

            if valid:
                results[name] = eras

        logging.info("Removed %s zones with invalid offsetString",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        _merge_reasons(self.all_notable_zones, notable_zones)
        return results

    def _remove_zones_with_invalid_rules_format_combo(
        self, zones_map: ZonesMap
    ) -> ZonesMap:
        """Check for valid FORMAT field.

        First, it should always exist.

        If the RULES is fixed (i.e. contains '-' or a 'hh:mm' offset, then
        FORMAT can contain only the '/' . It cannot contain a '%' because there
        would be no RULE entry with a LETTER that can replace the '%'.

        If the RULES is a reference to a named RULE, then it seems reasonable to
        always expect a '%' or a '/' but we cannot make this strict. There are
        cases where the FORMAT contains neither, for example,
        Africa/Johannesburg where it defines DST transitions for 1942-1944, but
        there seems to be no corresponding change in the abbreviation so FORMAT
        contains no '%' or '/'. Generate a warning for now.
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        notable_zones: CommentsCollection = {}
        for zone_name, eras in zones_map.items():
            valid = True
            for era in eras:
                if not era['format']:
                    _add_reason(removed_zones, zone_name, 'FORMAT is empty')
                    valid = False
                    break

                if era['rules'] == '-' or ':' in era['rules']:
                    if '%' in era['format']:
                        _add_reason(
                            removed_zones, zone_name,
                            "RULES is fixed but FORMAT contains '%'")
                        valid = False
                        break
                else:
                    if not ('%' in era['format'] or '/' in era['format']):
                        _add_reason(
                            notable_zones, zone_name,
                            "RULES not fixed but FORMAT is missing "
                            + "'%' or '/'")

            if valid:
                results[zone_name] = eras

        logging.info("Removed %s zones with invalid RULES and FORMAT combo",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        _merge_reasons(self.all_notable_zones, notable_zones)
        return results

    def _create_zones_with_rules_expansion(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Expand and normalize the zone.rules field (RULES) and create
        zone.rulesDeltaSeconds from zone.rules.

        The RULES field can hold the following:
            * '-' no rules
            * a string reference to a set of Rules
            * a delta offset like "01:00" to be added to the STDOFF field
                (see America/Argentina/San_Luis, Europe/Istanbul for example).
        After this method, the zone.rules contains 3 possible values:
            * '-' no rules, or
            * ':' which indicates that 'rulesDeltaSeconds' is defined, or
            * a string reference of the zone policy containing the rules
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        notable_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                rules_string = era['rules']
                if rules_string.find(':') >= 0:
                    if self.scope == 'basic':
                        valid = False
                        _add_reason(
                            removed_zones, name,
                            f"offset in RULES '{rules_string}'")
                        break

                    rules_delta_seconds = time_string_to_seconds(rules_string)
                    if rules_delta_seconds == INVALID_SECONDS:
                        valid = False
                        _add_reason(
                            removed_zones, name,
                            f"invalid RULES string '{rules_string}'")
                        break
                    if rules_delta_seconds == 0:
                        valid = False
                        _add_reason(
                            removed_zones, name,
                            f"unexpected 0:00 RULES string '{rules_string}'")
                        break

                    rules_delta_seconds_truncated = truncate_to_granularity(
                        rules_delta_seconds, self.offset_granularity)
                    if rules_delta_seconds != rules_delta_seconds_truncated:
                        if self.strict:
                            valid = False
                            _add_reason(
                                removed_zones, name,
                                f"RULES delta offset '{rules_string}' must be "
                                f"multiples of '{self.offset_granularity}' "
                                f"seconds")
                            break
                        else:
                            hm = seconds_to_hm_string(
                                rules_delta_seconds_truncated)
                            _add_reason(
                                notable_zones, name,
                                f"RULES delta offset '{rules_string}'"
                                f"truncated to '{hm}'")

                    era['rules'] = ':'
                    era['rulesDeltaSeconds'] = rules_delta_seconds
                    era['rulesDeltaSecondsTruncated'] = \
                        rules_delta_seconds_truncated
                else:
                    # If '-' or named policy, set to 0.
                    era['rulesDeltaSeconds'] = 0
                    era['rulesDeltaSecondsTruncated'] = 0
            if valid:
                results[name] = eras

        logging.info("Removed %s zone infos with invalid RULES",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        _merge_reasons(self.all_notable_zones, notable_zones)
        return results

    def _remove_zones_without_rules(
        self, zones_map: ZonesMap, policies_map: PoliciesMap
    ) -> ZonesMap:
        """Remove zone eras whose RULES field contains a reference to
        a set of Rules, which cannot be found.
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                rule_name = era['rules']
                if (rule_name not in ['-', ':']
                        and rule_name not in policies_map):
                    valid = False
                    _add_reason(
                        removed_zones, name,
                        f"policy '{rule_name}' not found")
                    break
            if valid:
                results[name] = eras

        logging.info(
            "Removed %s zone infos without rules", len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        return results

    def _remove_zones_with_non_monotonic_until(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Remove Zone infos whose UNTIL fields are:
            1) not monotonically increasing, or
            2) does not end in year=MAX_UNTIL_YEAR
        """
        results: ZonesMap = {}
        removed_zones: CommentsCollection = {}
        for name, eras in zones_map.items():
            valid = True
            prev_until = None
            for era in eras:
                # yapf: disable
                current_until = (
                    era['untilYear'],
                    era['untilMonth'] if era['untilMonth'] else 0,
                    era['untilDay'] if era['untilDayString'] else 0,
                    era['untilSeconds'] if era['untilSeconds'] else 0
                )
                # yapf: enable
                if prev_until:
                    if current_until <= prev_until:
                        valid = False
                        _add_reason(
                            removed_zones,
                            name,
                            'non increasing UNTIL: '
                            f'{current_until[0]:04}-'
                            f'{current_until[1]:02}-'
                            f'{current_until[2]:02} '
                            f'{current_until[3]}s'
                        )
                        break
                prev_until = current_until
            if valid and current_until[0] != MAX_UNTIL_YEAR:
                valid = False
                _add_reason(
                    removed_zones,
                    name,
                    (
                        'invalid final UNTIL: '
                        f'{current_until[0]:04}-'
                        f'{current_until[1]:02}-'
                        f'{current_until[2]:02} '
                        f'{current_until[3]}s'
                    )
                )

            if valid:
                results[name] = eras

        logging.info("Removed %s zone infos with invalid UNTIL fields",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        _merge_reasons(self.all_removed_zones, removed_zones)
        return results

    # --------------------------------------------------------------------
    # Methods related to Rules
    # --------------------------------------------------------------------

    def _remove_rules_multiple_transitions_in_month(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Some Zone policies have Rules which specify multiple DST transitions
        within in the same month:
            * Egypt (Found '2' transitions in year/month '2010-09')
            * Palestine (Found '2' transitions in year/month '2011-08')
            * Spain (Found '2' transitions in year/month '1938-04')
            * Tunisia (Found '2' transitions in year/month '1943-04')
        """
        CountsMap = Dict[Tuple[str, int, int], int]

        # First pass: collect number of transitions for each (year, month) pair.
        counts: CountsMap = {}
        for name, rules in policies_map.items():
            for rule in rules:
                from_year = rule['fromYear']
                to_year = rule['toYear']
                month = rule['inMonth']
                for year in range(from_year, to_year + 1):
                    key = (name, year, month)
                    count = counts.get(key)
                    count = count + 1 if count else 1
                    counts[key] = count

        # Second pass: Collect rule policies which have multiple transitions
        # in one month.
        removals: Dict[str, Tuple[int, int, int]] = {}
        for key, count in counts.items():
            if count > 1:
                policy_name = key[0]
                year = key[1]
                month = key[2]
                removals[policy_name] = (count, year, month)

        # Third pass: Remove rule policies with multiple counts.
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            removal = removals.get(name)
            if removal:
                _add_reason(
                    removed_policies,
                    name,
                    f"Found {removal[0]} transitions in year/month "
                    f"'{removal[1]:04}-{removal[2]:02}'"
                )
            else:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with multiple transitions in one month',
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _remove_rules_long_dst_letter(
        self,
        policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Return a new map which filters out rules with long DST letter.
        """
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                letter = rule['letter']
                if len(letter) > 1:
                    valid = False
                    _add_reason(
                        removed_policies, name,
                        f"LETTER '{letter}' too long")
                    break
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with long DST letter',
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _remove_rules_invalid_at_time_suffix(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Remove rules whose atTime contains an unsupported suffix. Current
        supported suffix is 'w', 's' and 'u'. The 'g' and 'z' are identifical
        to 'u' and they do not currently appear in any TZ file, so let's catch
        them because it could indicate a bug somewhere in our parser or
        somewhere else.
        """
        supported_suffices = ['w', 's', 'u']
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                suffix = rule['atTimeSuffix']
                suffix = suffix if suffix else 'w'
                rule['atTimeSuffix'] = suffix
                if suffix not in supported_suffices:
                    valid = False
                    _add_reason(
                        removed_policies, name,
                        f"unsupported AT time suffix '{suffix}'")
                    break
            if valid:
                results[name] = rules

        logging.info(
            "Removed %s rule policies with unsupported AT suffix",
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _mark_rules_used_by_zones(
        self, zones_map: ZonesMap, policies_map: PoliciesMap,
    ) -> Tuple[ZonesMap, PoliciesMap]:
        """Mark all rules which are required by various zones. There are 2 ways
        that a rule can be used by a zone era:
            1) The rule's fromYear or toYear are >= (self.start_year - 1), or
            2) The rule is the most recent transition that happened before
            self.start_year.

        If start_year == 2000, this will pick up rules for 1998. This is because
        if viewing_months == 13, then ZoneSpecifier.init_for_year() could be
        called with 1999, which then needs rules for 1998 to extract the "most
        recent prior" Transition before Jan 1, 1999.

        For viewing_months==14, init_for_year() will always be called with 2000
        or higher, so we just need 1999 data to get the most recent prior
        Transition before Jan 1, 2000.
        """
        for zone_name, eras in zones_map.items():
            begin_year = self.start_year - 1
            for era in eras:
                policy_name = era['rules']
                if policy_name in ['-', ':']:
                    continue

                rules = policies_map.get(policy_name)
                if not rules:
                    logging.error(
                        "Zone '%s': Could not find policy '%s': "
                        + "should not happen", zone_name, policy_name)
                    sys.exit(1)

                # Make all Rules which overlap with the current Zone Era.
                # Some Zone Era have an until_month, until_day and until_time
                # components. To be conservative, we need to expand the
                # until_year to the following year, so the effective zone era
                # interval becomes [begin_year, until_year+1).
                until_year = min(era['untilYear'], self.until_year)
                matching_rules = find_matching_rules(rules, begin_year,
                                                     until_year + 1)
                for rule in matching_rules:
                    rule['used'] = True

                # Find latest Rules just prior to the begin_year.
                # Result: It looks like all of these prior rules are
                # already picked up by previous calls to find_matching_rules().
                prior_rules = find_latest_prior_rules(rules, begin_year)
                for rule in prior_rules:
                    rule['used'] = True

                # Find earliest Rules subsequent to the until_year mark.
                # Result: It looks like all of these subsequent rules are
                # already picked up by previous calls to find_matching_rules().
                subsequent_rules = find_earliest_subsequent_rules(
                    rules, until_year + 1)
                for rule in subsequent_rules:
                    rule['used'] = True

                # Set the begin year of the next ZoneEra
                begin_year = era['untilYear']

        return (zones_map, policies_map)

    def _remove_rules_unused(self, policies_map: PoliciesMap) -> PoliciesMap:
        """Remove RULE entries which have not been marked as used by the
        _mark_rules_used_by_zones() method. It is expected that all remaining
        RULE entries have FROM and TO fields which is greater than 1872 (the
        earliest year which can be represented by an int8_t toYearTiny field,
        (2000-128)==1872). See also _remove_rules_out_of_bounds().
        """
        results: PoliciesMap = {}
        removed_rule_count = 0
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            used_rules = []
            for rule in rules:
                if rule.get('used'):
                    used_rules.append(rule)
                    # Set the 'used' to None to remove from JSON output.
                    del rule['used']
                else:
                    removed_rule_count += 1
            if used_rules:
                results[name] = used_rules
            else:
                _add_reason(removed_policies, name, 'unused')

        logging.info(
            'Removed %s rule policies (%s rules) not used',
            len(removed_policies),
            removed_rule_count
        )
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _remove_rules_out_of_bounds(
        self,
        policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Remove policies which have FROM and TO fields do not fit in an
        int8_t. In other words, y < 1872 or (y > 2127 and y != 9999).
        """
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                from_year = rule['fromYear']
                to_year = rule['toYear']
                if not is_year_tiny(from_year) or not is_year_tiny(from_year):
                    valid = False
                    _add_reason(
                        removed_policies, name,
                        f"fromYear ({from_year}) or toYear ({to_year}) "
                        f" out of bounds")
                    break
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with fromYear or toYear out of bounds',
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _create_rules_with_on_day_expansion(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Create rule['onDayOfWeek'] and rule['onDayOfMonth'] from
        rule['onDay']. The onDayOfMonth will be negative if "<=" is used.
        """
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                on_day = rule['onDay']
                (on_day_of_week, on_day_of_month) = _parse_on_day_string(on_day)

                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    _add_reason(
                        removed_policies, name,
                        f"invalid onDay '{on_day}'")
                    break

                # *ZoneProcessor.h classes currently do not support
                # "dayOfWeek<=6" or "dayOfWeek>=26" if the shift causes the year
                # to change.
                if on_day_of_week != 0 and on_day_of_month != 0:
                    if (-7 <= on_day_of_month
                            and on_day_of_month < -1
                            and rule['inMonth'] == 1):
                        valid = False
                        _add_reason(
                            removed_policies, name,
                            f"cannot shift '{on_day}' from Jan to prev year")
                        break
                    if 26 <= on_day_of_month and rule['inMonth'] == 12:
                        valid = False
                        _add_reason(
                            removed_policies, name,
                            f"cannot shift '{on_day}' from Dec to next year")
                        break

                rule['onDayOfWeek'] = on_day_of_week
                rule['onDayOfMonth'] = on_day_of_month
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid onDay',
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _create_rules_with_anchor_transition(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Create a synthetic transition with SAVE == 0 which is earlier than
        the self.start_year of interest. Some zone policies have zone rules
        whose earliest entry starts after the self.start_year. According to
        https://data.iana.org/time-zones/tz-how-to.html, the initial LETTER
        should be deduced from the first RULE whose SAVE == 0.

        As of 2018i, 6 zone policies are affected:
            ['Troll', 'Armenia', 'Dhaka', 'Pakistan', 'WS', 'SanLuis']
        corresponding to 4 zones:
            Pacific/Apia, Asia/Dhaka, Asia/Karachi, Asia/Yerevan
        """
        anchored_policies: List[str] = []
        for name, rules in policies_map.items():
            if not self._has_prior_rule(rules):
                anchor_rule = self._get_anchor_rule(rules)
                rules.insert(0, anchor_rule)
                anchored_policies.append(name)

        logging.info(
            'Added anchor rule to %s rule policies: %s',
            len(anchored_policies),
            anchored_policies
        )
        return policies_map

    def _has_prior_rule(self, rules: List[ZoneRuleRaw]) -> bool:
        """Return True if rules has a rule prior to (self.start_year-1).
        """
        for rule in rules:
            from_year = rule['fromYear']
            if from_year < self.start_year - 1:
                return True
        return False

    def _get_anchor_rule(self, rules: List[ZoneRuleRaw]) -> ZoneRuleRaw:
        """Return the anchor rule that will act as the earliest rule with SAVE
        == 0.
        """
        AnchorInfo = TypedDict('AnchorInfo', {
            'earliestDate': Tuple[int, int, int],
            'rule': Optional[ZoneRuleRaw],
        })

        anchor_info: AnchorInfo = {
            'earliestDate': (MAX_UNTIL_YEAR, 12, 31),
            'rule': None,
        }
        # rules will never be empty, so this will always produce a
        # non-empty anchor_info['rule'].
        for rule in rules:
            from_year = rule['fromYear']
            in_month = rule['inMonth']
            on_day_of_week = rule['onDayOfWeek']
            on_day_of_month = rule['onDayOfMonth']
            month, day = calc_day_of_month(
                from_year, in_month, on_day_of_week, on_day_of_month)
            rule_date = (from_year, month, day)

            if (rule['deltaSeconds'] == 0
                    and rule_date < anchor_info['earliestDate']):
                anchor_info['earliestDate'] = rule_date
                anchor_info['rule'] = rule

        anchor_rule = cast(ZoneRuleRaw, anchor_info['rule']).copy()
        anchor_rule['fromYear'] = MIN_YEAR
        anchor_rule['toYear'] = MIN_YEAR
        anchor_rule['inMonth'] = 1
        anchor_rule['onDayOfWeek'] = 0
        anchor_rule['onDayOfMonth'] = 1
        anchor_rule['atTime'] = '0'
        anchor_rule['atTimeSuffix'] = 'w'
        anchor_rule['deltaOffset'] = '0'
        anchor_rule['atSeconds'] = 0
        anchor_rule['atSecondsTruncated'] = 0
        anchor_rule['deltaSeconds'] = 0
        anchor_rule['deltaSecondsTruncated'] = 0
        anchor_rule['rawLine'] = 'Anchor: ' + anchor_rule['rawLine']
        return anchor_rule

    def _remove_rules_with_border_transitions(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Remove rules where the transition occurs on the first day of the
        year (Jan 1). That situation is not supported by BasicZoneSpecifier. On
        the other hand, a transition at the end of the year (Dec 31) is
        supported by BasicZoneSpecifier.
        """
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                from_year = rule['fromYear']
                to_year = rule['toYear']
                month = rule['inMonth']
                on_day_of_month = rule['onDayOfMonth']
                if from_year > MIN_YEAR and to_year > MIN_YEAR:
                    if month == 1 and on_day_of_month == 1:
                        valid = False
                        _add_reason(
                            removed_policies, name,
                            "Transition in early year (%04d-%02d-%02d)" %
                            (from_year, month, on_day_of_month))
                        break
            if valid:
                results[name] = rules

        logging.info(
            "Removed %s rule policies with border Transitions",
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        return results

    def _create_rules_with_expanded_at_time(
        self,
        policies_map: PoliciesMap,
        rules_to_zones: RulesToZones,
    ) -> PoliciesMap:
        """ Create 'atSeconds' parameter from rule['atTime'].
        """
        results: PoliciesMap = {}
        removed_policies: CommentsCollection = {}
        notable_policies: CommentsCollection = {}
        for policy_name, rules in policies_map.items():
            valid = True
            for rule in rules:
                at_time = rule['atTime']
                at_seconds = time_string_to_seconds(at_time)
                if at_seconds == INVALID_SECONDS:
                    valid = False
                    _add_reason(
                        removed_policies, policy_name,
                        f"invalid AT time '{at_time}'" % at_time)
                    break
                if at_seconds < 0:
                    valid = False
                    _add_reason(
                        removed_policies, policy_name,
                        f"negative AT time '{at_time}'" % at_time)
                    break

                at_seconds_truncated = truncate_to_granularity(
                    at_seconds, self.until_at_granularity)
                if at_seconds != at_seconds_truncated:
                    if self.strict:
                        valid = False
                        _add_reason(
                            removed_policies, policy_name,
                            f"AT time '{at_time}' must be multiples of "
                            f"'{self.until_at_granularity}' seconds")
                        break
                    else:
                        hm = seconds_to_hm_string(at_seconds_truncated)
                        _add_reason(
                            notable_policies, policy_name,
                            f"AT time '{at_time}' truncated to '{hm}'")
                        # Add warning about the affected zones.
                        zone_names = rules_to_zones.get(policy_name)
                        if zone_names:
                            for zone_name in zone_names:
                                hm = seconds_to_hm_string(at_seconds_truncated)
                                _add_reason(
                                    self.all_notable_zones, zone_name,
                                    f"AT time '{at_time}' of "
                                    f"RULE '{policy_name}' "
                                    f"truncated to '{hm}'")

                rule['atSeconds'] = at_seconds
                rule['atSecondsTruncated'] = at_seconds_truncated
            if valid:
                results[policy_name] = rules

        logging.info(
            'Removed %s rule policies with invalid atTime',
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        _merge_reasons(self.all_notable_policies, notable_policies)
        return results

    def _create_rules_with_expanded_delta_offset(
        self,
        policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """ Create 'deltaSeconds' and 'deltaSecondsTruncated' from
        rule['deltaOffset'].
        """
        results = {}
        removed_policies: CommentsCollection = {}
        notable_policies: CommentsCollection = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                delta_offset = rule['deltaOffset']
                delta_seconds = time_string_to_seconds(delta_offset)
                if delta_seconds == INVALID_SECONDS:
                    valid = False
                    _add_reason(
                        removed_policies, name,
                        f"invalid deltaOffset '{delta_offset}'")
                    break

                # Truncate to requested granularity.
                delta_seconds_truncated = truncate_to_granularity(
                    delta_seconds, self.offset_granularity)
                if delta_seconds != delta_seconds_truncated:
                    if self.strict:
                        valid = False
                        _add_reason(
                            removed_policies, name,
                            f"deltaOffset '{delta_offset}' must be "
                            f"a multiple of '{self.offset_granularity}' "
                            f"seconds")
                        break
                    else:
                        _add_reason(
                            notable_policies, name,
                            f"deltaOffset '{delta_offset}' truncated to"
                            f"a multiple of '{self.offset_granularity}' "
                            f"seconds")

                # Check that delta seconds can fit in a 4-bit timeCode field
                # with 15-minute granularity, defined as (timeCode =
                # delta_seconds / 900s + 1h) which encodes -1:00 as 0 and 3:45
                # as 15.
                delta_code = div_to_zero(delta_seconds_truncated, 900) + 4
                if delta_code < 0 or delta_code > 15:
                    valid = False
                    _add_reason(
                        removed_policies, name,
                        f"deltaOffset '{delta_offset}' too large for 4-bits")
                    break

                rule['deltaSeconds'] = delta_seconds
                rule['deltaSecondsTruncated'] = delta_seconds_truncated
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid deltaOffset',
            len(removed_policies)
        )
        self._print_removed_map(removed_policies)
        _merge_reasons(self.all_removed_policies, removed_policies)
        _merge_reasons(self.all_notable_policies, notable_policies)
        return results

    # --------------------------------------------------------------------
    # Methods related to Links.
    # --------------------------------------------------------------------

    def remove_links_to_missing_zones(
        self,
        links_map: LinksMap,
        zones_map: ZonesMap
    ) -> LinksMap:
        results = {}
        removed_links: CommentsCollection = {}
        for link_name, zone_name in links_map.items():
            if zones_map.get(zone_name):
                results[link_name] = zone_name
            else:
                _add_reason(
                    removed_links, link_name,
                    f'Target Zone "{zone_name}" missing')

        logging.info('Removed %s links with missing zones', len(removed_links))
        _merge_reasons(self.all_removed_links, removed_links)
        return results

    def remove_zones_and_links_with_similar_names(
        self,
        zones_map: ZonesMap,
        links_map: LinksMap,
    ) -> Tuple[ZonesMap, LinksMap]:
        normalized_names: Dict[str, str] = {}  # normalized_name, name
        result_zones: ZonesMap = {}
        result_links: LinksMap = {}
        removed_zones: CommentsCollection = {}
        removed_links: CommentsCollection = {}

        # Check for duplicate zone names.
        for zone_name, zone in zones_map.items():
            nname = normalize_name(zone_name)
            if normalized_names.get(nname):
                _add_reason(
                    removed_zones, zone_name,
                    'Duplicate normalized name')
            else:
                normalized_names[nname] = zone_name
                result_zones[zone_name] = zone

        # Then strike out any duplicate links.
        for link_name, link in links_map.items():
            nname = normalize_name(link_name)
            if normalized_names.get(nname):
                _add_reason(
                    removed_links, link_name,
                    'Duplicate normalized name')
            else:
                normalized_names[nname] = link_name
                result_links[link_name] = link

        logging.info(
            'Removed %d Zones and %s Links with duplicate names',
            len(removed_zones),
            len(removed_links))
        _merge_reasons(self.all_removed_zones, removed_zones)
        _merge_reasons(self.all_removed_links, removed_links)
        return result_zones, result_links


# ISO-8601 specifies Monday=1, Sunday=7
WEEK_TO_WEEK_INDEX = {
    'Mon': 1,
    'Tue': 2,
    'Wed': 3,
    'Thu': 4,
    'Fri': 5,
    'Sat': 6,
    'Sun': 7,
}


def _parse_on_day_string(on_string: str) -> Tuple[int, int]:
    """Parse things like "Sun>=1", "lastSun", "20", "Fri<=2".
    Returns (on_day_of_week, on_day_of_month) where
        (0, dayOfMonth) = exact match on dayOfMonth
        (dayOfWeek, dayOfMonth) = matches dayOfWeek>=dayOfMonth
        (dayOfWeek, -dayOfMonth) = matches dayOfWeek<=dayOfMonth
        (dayOfWeek, 0) = matches lastDayOfWeek
        (0, 0) = syntax error

    where
        dayOfWeek is represented by a number (Mon=1, ..., Sun=7),
        dayOfMonth is 0, 1-31 (if >=), or (-1)-(-31) (if <=).

    """
    if on_string.isdigit():
        return (0, int(on_string))

    if on_string[:4] == 'last':
        dayOfWeek = on_string[4:]
        if dayOfWeek not in WEEK_TO_WEEK_INDEX:
            return (0, 0)
        return (WEEK_TO_WEEK_INDEX[dayOfWeek], 0)

    greater_than_equal_index = on_string.find('>=')
    if greater_than_equal_index >= 0:
        dayOfWeek = on_string[:greater_than_equal_index]
        dayOfMonth = on_string[greater_than_equal_index + 2:]
        if dayOfWeek not in WEEK_TO_WEEK_INDEX:
            return (0, 0)
        return (WEEK_TO_WEEK_INDEX[dayOfWeek], int(dayOfMonth))

    less_than_equal_index = on_string.find('<=')
    if less_than_equal_index >= 0:
        dayOfWeek = on_string[:less_than_equal_index]
        dayOfMonth = on_string[less_than_equal_index + 2:]
        if dayOfWeek not in WEEK_TO_WEEK_INDEX:
            return (0, 0)
        return (WEEK_TO_WEEK_INDEX[dayOfWeek], -int(dayOfMonth))

    return (0, 0)


INVALID_SECONDS = 999999  # 277h46m69s


def time_string_to_seconds(time_string: str) -> int:
    """Converts the '[-]hh:mm:ss' string into +/- total seconds from 00:00.
    Returns INVALID_SECONDS if there is a parsing error.
    """
    sign = 1
    if time_string[0] == '-':
        sign = -1
        time_string = time_string[1:]

    try:
        elems = time_string.split(':')
        if len(elems) == 0:
            return INVALID_SECONDS
        hour = int(elems[0])
        minute = int(elems[1]) if len(elems) > 1 else 0
        second = int(elems[2]) if len(elems) > 2 else 0
        if len(elems) > 3:
            return INVALID_SECONDS
    except Exception:
        return INVALID_SECONDS

    # A number of countries use 24:00, and Japan uses 25:00(!).
    # Rule  Japan   1948    1951  -     Sep Sat>=8  25:00   0   	S
    if hour > 25:
        return INVALID_SECONDS
    if minute > 59:
        return INVALID_SECONDS
    if second > 59:
        return INVALID_SECONDS
    return sign * ((hour * 60 + minute) * 60 + second)


def find_matching_rules(
    rules: List[ZoneRuleRaw],
    era_from: int,
    era_until: int,
) -> List[ZoneRuleRaw]:
    """Return the rules which overlap with the Zone Era interval [eraFrom,
    eraUntil). The Rule interval is [ruleFrom, ruleTo + 1). Overlap happens
    (ruleFrom < eraUntil) && (eraFrom < ruleTo+1). The expression (eraFrom <
    ruleTo+1) can be written as (eraFrom <= ruleTo) since these values are
    integers.
    """
    matches = []
    for rule in rules:
        if rule['fromYear'] < era_until and era_from <= rule['toYear']:
            matches.append(rule)
    return matches


def find_latest_prior_rules(
    rules: List[ZoneRuleRaw],
    year: int,
) -> List[ZoneRuleRaw]:
    """Find the most recent prior rules before the given year. The RULE.atTime
    field can be a conditional expression such as 'lastSun' or 'Mon>=8', so it's
    easiest to just compare the (year, month) only. Also, instead of looking for
    the single Rule that is the most recent, we grab all Rules that fit into the
    month bucket. There are 2 reasons:

    1) A handful of Zone Policies have multiple Rules in the same month. From
    _remove_rules_multiple_transitions_in_month():

        * Egypt (Found 2 transitions in year/month '2010-09')
        * Palestine (Found 2 transitions in year/month '2011-08')
        * Spain (Found 2 transitions in year/month '1938-04')
        * Tunisia (Found 2 transitions in year/month '1943-04')

    2) A handful of Zone Policies have Rules which specify transitions in the
    last 2 days of the year. From _remove_rules_with_border_transitions(), we
    find:
        * Arg (Transition in late year (2007-12-30))
        * Dhaka (Transition in late year (2009-12-31))
        * Ghana (Transition in late year (1920-12-31))

    By grabbing all Rules in the last month, we avoid the risk of accidentally
    leaving some Rules out.
    """
    candidates = []
    candidate_date = (0, 0)  # sentinel date earlier than all real Rules
    for rule in rules:
        rule_year = rule['toYear']
        rule_month = rule['inMonth']
        if rule_year < year:
            rule_date = (rule_year, rule_month)
            if rule_date > candidate_date:
                candidate_date = rule_date
                candidates = [rule]
            elif rule_date == candidate_date:
                candidates.append(rule)
    return candidates


def find_earliest_subsequent_rules(
    rules: List[ZoneRuleRaw],
    year: int,
) -> List[ZoneRuleRaw]:
    """Find the ealiest subsequent rules on or after the given year. This deals
    with the case where the following (admittedly unlikely) set of conditions
    happen:

    1) an epochSeconds is converted to a UTC dateTime,
    2) we look for Transitions in the current UTC year, but the actual
    year in the local timezone is actually in the following year,
    3) there exists a Rule that specifies a Transition in the first day of the
    new year, which does not get picked up without this scan.

    It's likely that any such Rule would get picked up by the normal
    find_matching_rules() of a Zone Era that stretched to MAX_YEAR, but I'm not
    100% sure that that's true, and there might be a weird edge case. This
    method helps prevent that edge case.

    Similar to find_latest_prior_rules(), we match all Rules in a given month,
    instead of looking single earliest Rule.
    """
    candidates = []
    # sentinel date later than all real Rules
    candidate_date = (MAX_YEAR, 13)
    for rule in rules:
        rule_year = rule['toYear']
        rule_month = rule['inMonth']
        if rule_year >= year:
            rule_date = (rule_year, rule_month)
            if rule_date < candidate_date:
                candidate_date = rule_date
                candidates = [rule]
            elif rule_date == candidate_date:
                candidates.append(rule)
    return candidates


def is_year_tiny(year: int) -> bool:
    """Determine if year fits in an int8_t field. MAX_YEAR(9999) is a marker for
    'max'.
    """
    return year >= 1872 and (year == MAX_YEAR or year <= 2127)


def calc_day_of_month(
    year: int,
    month: int,
    on_day_of_week: int,
    on_day_of_month: int,
) -> Tuple[int, int]:
    """Return the actual (month, day) of expressions such as
    (onDayOfWeek >= onDayOfMonth), (onDayOfWeek <= onDayOfMonth), or (lastMon)
    See BasicZoneSpecifier::calcStartDayOfMonth(). Shifts into previous or
    next month can occur.

    Return (13, xx) if a shift to the next year occurs
    Return (0, xx) if a shift to the previous year occurs
    """
    if on_day_of_week == 0:
        return (month, on_day_of_month)

    if on_day_of_month >= 0:
        days_in_month = _days_in_month(year, month)

        # Handle lastXxx by transforming it into (Xxx >= (daysInMonth - 6))
        if on_day_of_month == 0:
            on_day_of_month = days_in_month - 6

        limit_date = datetime.date(year, month, on_day_of_month)
        day_of_week_shift = (on_day_of_week - limit_date.isoweekday() + 7) % 7
        day = on_day_of_month + day_of_week_shift
        if day > days_in_month:
            day -= days_in_month
            month += 1
        return (month, day)
    else:
        on_day_of_month = -on_day_of_month
        limit_date = datetime.date(year, month, on_day_of_month)
        day_of_week_shift = (limit_date.isoweekday() - on_day_of_week + 7) % 7
        day = on_day_of_month - day_of_week_shift
        if day < 1:
            month -= 1
            days_in_prev_month = _days_in_month(year, month)
            day += days_in_prev_month
        return (month, day)


def _days_in_month(year: int, month: int) -> int:
    """Return the number of days in the given (year, month). The
    month is usually 1-12, but can be 0 to indicate December of the previous
    year, and 13 to indicate Jan of the following year.
    """
    DAYS_IN_MONTH = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    is_leap = (year % 4 == 0) and ((year % 100 != 0) or (year % 400) == 0)
    days = DAYS_IN_MONTH[(month - 1) % 12]
    if month == 2:
        days += is_leap
    return days


def seconds_to_hms(seconds: int) -> Tuple[int, int, int]:
    """Convert seconds to (h,m,s). Works only for positive seconds.
    """
    s = seconds % 60
    minutes = seconds // 60
    m = minutes % 60
    h = minutes // 60
    return (h, m, s)


def seconds_to_hm_string(seconds: int) -> str:
    """Convert seconds to hh:mm. Assumes that seconds is multiples of 60.
    """
    if seconds < 0:
        sign = '-'
        seconds = -seconds
    else:
        sign = ''
    minutes = seconds // 60
    m = minutes % 60
    h = minutes // 60
    return f'{sign}{h:02}:{m:02}'


def hms_to_seconds(h: int, m: int, s: int) -> int:
    """Convert h:m:s to seconds.
    """
    return (h * 60 + m) * 60 + s


def div_to_zero(a: int, b: int) -> int:
    """Integer division (a/b) that truncates towards 0, instead of -infinity as
    is default for Python. Assumes b is positive, but a can be negative or
    positive.
    """
    return a // b if a >= 0 else (a - 1) // b + 1


def truncate_to_granularity(a: int, b: int) -> int:
    """Truncate a to the granularity of b.
    """
    return b * div_to_zero(a, b)


def add_string(strings: 'OrderedDict[str, int]', name: str) -> int:
    """Add the 'name' to the strings (must be an OrderedDict), and return its
    index into the array of strings. If the 'name' already exists, then return
    the previous index. Otherwise, create a new index, and return that.
    """
    if not isinstance(strings, OrderedDict):
        raise Exception('strings must be an OrderedDict')
    index = strings.get(name)
    if index is None:
        index = len(strings)
        strings[name] = index
    return index  # index will never be None


def _create_rules_to_zones(
    zones_map: ZonesMap,
    policies_map: PoliciesMap,
) -> RulesToZones:
    """Normally Zones point to Rules. This method causes the reverse to happen,
    making Rules know about Zones, by creating a map of {rule_name ->
    zone_full_name[]}. This allows us to determine which zones that may be
    affected by a change in a particular Rule. Must be called after
    _create_zones_with_rules_expansion() to normalize the RULES column
    (zone.rules).
    """
    rules_to_zones: RulesToZones = {}
    for full_name, eras in zones_map.items():
        for era in eras:
            rule_name = era['rules']
            if rule_name not in ['-', ':']:
                zones = rules_to_zones.get(rule_name)
                if not zones:
                    zones = []
                    rules_to_zones[rule_name] = zones
                zones.append(full_name)
    return rules_to_zones


def normalize_name(name: str) -> str:
    """Replace hyphen (-) and slash (/) with underscore (_) to generate valid
    C++ and Python symbols.
    """
    name = name.replace('+', '_PLUS_')
    return re.sub('[^a-zA-Z0-9_]', '_', name)


def normalize_raw(raw_line: str) -> str:
    """Replace hard tabs with 4 spaces.
    """
    return raw_line.replace('\t', '    ')


def hash_name(name: str) -> int:
    """Return the hash of the zone name. Implement the djb2 algorithm:
    https://stackoverflow.com/questions/7666509 and
    http://www.cse.yorku.ca/~oz/hash.html
    """
    U32_MOD = 2**32
    hash = 5381
    for c in name:
        hash = (33 * hash + ord(c)) % U32_MOD
    return hash


def _add_reason(m: CommentsCollection, name: str, reason: str) -> None:
    """Add the human readable 'reason' to a map of {name -> Set(reasons)}.
    """
    reasons = m.get(name)
    if not reasons:
        reasons = set()
        m[name] = reasons
    reasons.add(reason)


def _merge_reasons(m: CommentsCollection, n: CommentsCollection) -> None:
    """Given 2 dict of {name -> Set(reasons)}, merge n into m.
    """
    for name, new_reasons in n.items():
        old_reasons = m.get(name)
        if not old_reasons:
            old_reasons = set()
            m[name] = old_reasons
        old_reasons.update(new_reasons)
