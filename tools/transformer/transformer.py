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
from typing import Tuple
from typing_extensions import TypedDict
from data_types.at_types import ZoneRuleRaw
from data_types.at_types import ZoneEraRaw
from data_types.at_types import ZonesMap
from data_types.at_types import PoliciesMap
from data_types.at_types import LinksMap
from data_types.at_types import CommentsMap
from data_types.at_types import TransformerResult, add_comment, merge_comments
from data_types.at_types import MAX_UNTIL_YEAR
from data_types.at_types import MIN_YEAR
from data_types.at_types import MAX_YEAR

INVALID_SECONDS = 999999  # 277h46m69s

# Map of policyName -> zoneName[] used internally by Transformer to track the
# Zones which references the given policyName.
PoliciesToZones = Dict[str, List[str]]


class Transformer:
    def __init__(
        self,
        tresult: TransformerResult,
        scope: str,
        start_year: int,
        until_year: int,
        until_at_granularity: int,
        offset_granularity: int,
        delta_granularity: int,
        strict: bool,
    ):
        """
        Args:
            tresult: TransformerResult
            scope: scope of database (basic, or extended)
            start_year: include only years on or after start_year
            until_year: include only years valid before until_year
            until_at_granularity: truncate UNTIL, AT to this many seconds
            offset_granularity: truncate STDOFF (offset) to this many seconds
            delta_granularity: truncate SAVE (offset), RULES (rulesOffset) to
                    this many seconds
            strict: throw out Zones or Rules which are not exactly
                    on the time boundary defined by granularity
        """
        self.tresult = tresult
        self.zones_map = tresult.zones_map
        self.policies_map = tresult.policies_map
        self.links_map = tresult.links_map
        self.scope = scope
        self.start_year = start_year
        self.until_year = until_year
        self.until_at_granularity = until_at_granularity
        self.offset_granularity = offset_granularity
        self.delta_granularity = delta_granularity
        self.strict = strict

        self.all_removed_zones: CommentsMap = {}
        self.all_removed_policies: CommentsMap = {}
        self.all_removed_links: CommentsMap = {}
        self.all_notable_zones: CommentsMap = {}
        self.all_notable_policies: CommentsMap = {}
        self.all_notable_links: CommentsMap = {}

        self.original_zone_count = len(self.zones_map)
        self.original_rule_count = len(self.policies_map)
        self.original_link_count = len(self.links_map)

    def transform(self) -> None:
        """
        Transforms the zones_map and policies_map given in the constructor
        through a series of filters, and produces the TransformerResult
        can be retrieved using the get_data() function.
        """

        zones_map = self.zones_map
        policies_map = self.policies_map
        links_map = self.links_map

        logging.info(
            'Found %d zones, %d policies, %d links',
            len(self.zones_map),
            len(self.policies_map),
            len(self.links_map),
        )

        # Part 1: Transform the zones_map
        # zones_map = self._remove_zones_without_slash(zones_map)
        zones_map, links_map = self._detect_hash_collisions(
            zones_map=zones_map, links_map=links_map)
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
        zones_map, policies_map = self._mark_rules_used_by_zones(
            zones_map=zones_map, policies_map=policies_map)
        policies_to_zones = _create_policies_to_zones(zones_map, policies_map)

        # Part 3: Transform the policies_map
        policies_map = self._remove_rules_unused(policies_map)
        policies_map = self._remove_rules_out_of_bounds(policies_map)
        if self.scope == 'basic':
            policies_map = self._remove_rules_multiple_transitions_in_month(
                policies_map)
        policies_map = self._create_rules_with_expanded_at_time(
            policies_map, policies_to_zones)
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
        zones_map = self._remove_zones_without_rules(
            zones_map=zones_map, policies_map=policies_map)

        # Part 5: Remove links which point to removed zones.
        links_map = self.remove_links_to_missing_zones(links_map, zones_map)

        # Part 6: Remove zones and links whose normalized names conflict.
        zones_map, links_map = self.remove_zones_and_links_with_similar_names(
            zones_map=zones_map, links_map=links_map)

        # Part 7: Replace the original maps with the transformed ones.
        self.policies_map = policies_map
        self.zones_map = zones_map
        self.links_map = links_map

    def get_data(self) -> TransformerResult:
        """Merge the result of transform() into the original tresult."""
        merge_comments(self.tresult.removed_zones, self.all_removed_zones)
        merge_comments(self.tresult.removed_policies, self.all_removed_policies)
        merge_comments(self.tresult.removed_links, self.all_removed_links)
        merge_comments(self.tresult.notable_zones, self.all_notable_zones)
        merge_comments(self.tresult.notable_policies, self.all_notable_policies)
        merge_comments(self.tresult.notable_links, self.all_notable_links)
        return TransformerResult(
            zones_map=self.zones_map,
            policies_map=self.policies_map,
            links_map=self.links_map,
            removed_zones=self.tresult.removed_zones,
            removed_policies=self.tresult.removed_policies,
            removed_links=self.tresult.removed_links,
            notable_zones=self.tresult.notable_zones,
            notable_policies=self.tresult.notable_policies,
            notable_links=self.tresult.notable_links,
            zone_ids=self.tresult.zone_ids,
            link_ids=self.tresult.link_ids,
            letters_per_policy=self.tresult.letters_per_policy,
            letters_map=self.tresult.letters_map,
            formats_map=self.tresult.formats_map,
            fragments_map=self.tresult.fragments_map,
            compressed_names=self.tresult.compressed_names,
        )

    def print_summary(self) -> None:
        logging.info(
            f"Summary: Zones: original={self.original_zone_count}"
            f"; generated={len(self.zones_map)}"
            f"; removed={len(self.all_removed_zones)}"
            f"; noted={len(self.all_notable_zones)}")

        logging.info(
            f"Summary: Policies: original={self.original_rule_count}"
            f"; generated={len(self.policies_map)}"
            f"; removed={len(self.all_removed_policies)}"
            f"; noted={len(self.all_notable_policies)}")

        logging.info(
            f"Summary: Links: original={self.original_link_count}"
            f"; generated={len(self.links_map)}"
            f"; removed={len(self.all_removed_links)}"
            f"; noted={len(self.all_notable_links)}")

    def _print_comments_map(
        self,
        *,
        removed_map: CommentsMap,
        explanation: str,
        notable_map: Optional[CommentsMap] = None,
    ) -> None:
        """Helper routine that prints the 'Removed' Zone rules or Zone eras
        along with the reason why it was removed. Print up to a maximum of
        MAX_COMMENTS zones or eras. Also prints the 'Notable' rules or eras if
        available.
        """
        # Print summary line, e.g.:
        # "Removed 0 rule policies with from_year or to_year out of bounds"
        logging.info(f'Removed {len(removed_map)} {explanation}')

        # Print all lines if len() <= MAX_COMMENTS. Otherwise, print top half of
        # MAX_COMMENTS and bottom half of MAX_COMMENTS.
        MAX_COMMENTS = 5
        sorted_map = sorted(removed_map.items())
        num_items = len(sorted_map)
        if num_items <= MAX_COMMENTS:
            for name, reasons in sorted_map:
                logging.info(f'- {name} ({reasons})')
        else:
            index = 0
            ellipses_printed = False
            limit = (MAX_COMMENTS - 1) // 2
            for name, reasons in sorted_map:
                if ((index >= 0 and index < limit)
                        or (index >= num_items - limit and index < num_items)):
                    logging.info(f'- {name} ({reasons})')
                else:
                    if not ellipses_printed:
                        logging.info('- [...]')
                        ellipses_printed = True
                index += 1

        # Print notable zones, eras or links if given.
        if notable_map:
            logging.info(f'Noted {len(notable_map)} {explanation}')
            for name, reasons in sorted(notable_map.items()):
                logging.info(f'- {name} ({reasons})')

    # --------------------------------------------------------------------
    # Methods related to Zones.
    # --------------------------------------------------------------------

    def _remove_zones_without_slash(self, zones_map: ZonesMap) -> ZonesMap:
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            if name.rfind('/') >= 0:
                results[name] = eras
            else:
                add_comment(removed_zones, name, "No '/' in zone name")

        logging.info(
            "Removed %s zone infos without '/' in name",
            len(removed_zones)
        )
        merge_comments(self.all_removed_zones, removed_zones)
        return results

    def _detect_hash_collisions(
        self,
        zones_map: ZonesMap,
        links_map: LinksMap,
    ) -> Tuple[ZonesMap, LinksMap]:
        """Detect a hash collision of a zone name or a link name and throw an
        exception. With only about ~400 zone names and ~200 link names, the
        chances of a collision using a 32-bit hash is extremely low. However, if
        it ever happens, it is a severe error because we must guarantee that
        each zone name has a unique and stable hash for the life of this
        library.

        If this exception ever happens, we must create another hash for the
        colliding zone name, and keep the second hash unique and stable as well.
        A possible solution is to keep an internal list of colliding hashes
        (which ought to be few), and use a second hash function on the original
        zone name or link name to generate the new hash, and then use the the
        2nd hash for the 2nd name, while keeping the 1st hash for the original
        name. Because the hash of the 1st name must be remain unchanged.
        """
        hashes: Dict[int, str] = {}

        # Check zone names
        for name, _ in zones_map.items():
            h = hash_name(name)
            colliding_name = hashes.get(h)
            if colliding_name:
                raise Exception(
                    "Hash collision: "
                    f"Zone {name} with existing {colliding_name}"
                )
            hashes[h] = name

        # Check link names
        for name, _ in links_map.items():
            h = hash_name(name)
            colliding_name = hashes.get(h)
            if colliding_name:
                raise Exception(
                    "Hash collision: "
                    f"Link {name} with existing {colliding_name}"
                )
            hashes[h] = name

        logging.info('Detected no hash collisions')
        return zones_map, links_map

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
                if era['until_year'] >= self.start_year - 1:
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
                start_year = era['until_year']
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
        removed_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            if eras:
                results[name] = eras
            else:
                add_comment(removed_zones, name, "no ZoneEra found")

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos without ZoneEras',
        )
        merge_comments(self.all_removed_zones, removed_zones)
        return results

    def _remove_zone_until_year_only_false(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Remove zones which have month, day or time in the UNTIL field.
        These are not supported by BasicZoneSpecifier.
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                if not era['until_year_only']:
                    valid = False
                    add_comment(
                        removed_zones, name, "UNTIL contains month/day/time")
                    break
            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos with UNTIL month/day/time',
        )
        merge_comments(self.all_removed_zones, removed_zones)
        return results

    def _create_zones_with_until_day(self, zones_map: ZonesMap) -> ZonesMap:
        """Convert zone.until_day from 'lastSun' or 'Sun>=1' to a precise day,
        which is possible because the year and month are already known. For
        example:
            * Zone Asia/Tbilisi 2005 3 lastSun 2:00
            * Zone America/Grand_Turk 2015 Nov Sun>=1 2:00
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        notable_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                until_day_string = era['until_day_string']

                # Parse the conditional expression in until_day_string. We can
                # resolve the 'lastSun', 'Sun>=X' and 'Fri<=X' to a specific day
                # of month because we know the year.
                (on_day_of_week, on_day_of_month) = \
                    _parse_on_day_string(until_day_string)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"invalid until_day '{until_day_string}'")
                    break

                month, day = calc_day_of_month(
                    era['until_year'], era['until_month'], on_day_of_week,
                    on_day_of_month)
                if month == 0:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"Shift to previous year unsupported for "
                        f"{until_day_string}")
                    break
                if month == 13:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"Shift to following year unsupported for "
                        f"{until_day_string}")

                if era['until_month'] != month:
                    add_comment(
                        notable_zones, name,
                        f"until_month shifted from '{era['until_month']}' to "
                        f"'{month}' due to {until_day_string}")
                era['until_month'], era['until_day'] = month, day

            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos with invalid until_day',
            notable_map=notable_zones,
        )
        merge_comments(self.all_removed_zones, removed_zones)
        merge_comments(self.all_notable_zones, notable_zones)
        return results

    def _create_zones_with_expanded_until_time(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """ Create 'until_seconds' and 'until_seconds_truncated' from
        'until_time'.
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        notable_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                until_time = era['until_time']
                until_seconds = time_string_to_seconds(until_time)
                if until_seconds == INVALID_SECONDS:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"invalid UNTIL time '{until_time}'")
                    break
                if until_seconds < 0:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"negative UNTIL time '{until_time}'")
                    break

                until_seconds_truncated = truncate_to_granularity(
                    until_seconds, self.until_at_granularity)
                if until_seconds != until_seconds_truncated:
                    if self.strict:
                        valid = False
                        add_comment(
                            removed_zones, name,
                            f"UNTIL time '{until_time}' must be multiples "
                            f"of '{self.until_at_granularity}' seconds")
                        break
                    else:
                        hm = seconds_to_hm_string(until_seconds_truncated)
                        add_comment(
                            notable_zones, name,
                            f"UNTIL time '{until_time}' truncated to '{hm}'")

                era['until_seconds'] = until_seconds
                era['until_seconds_truncated'] = until_seconds_truncated
            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos with invalid UNTIL time',
            notable_map=notable_zones,
        )
        merge_comments(self.all_removed_zones, removed_zones)
        merge_comments(self.all_notable_zones, notable_zones)
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
        removed_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                suffix = era['until_time_suffix']
                suffix = suffix if suffix else 'w'
                era['until_time_suffix'] = suffix
                if suffix not in supported_suffices:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"unsupported UNTIL time suffix '{suffix}'")
                    break
            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos with unsupported UNTIL time suffix',
        )
        merge_comments(self.all_removed_policies, removed_zones)
        return results

    def _create_zones_with_expanded_offset_string(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """ Create expanded offset 'offset_seconds' from zone.offset_string.
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        notable_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                offset_string = era['offset_string']
                offset_seconds = time_string_to_seconds(offset_string)
                if offset_seconds == INVALID_SECONDS:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"invalid STDOFF '{offset_string}'")
                    break

                # Truncate offset to requested granularity.
                offset_seconds_truncated = truncate_to_granularity(
                    offset_seconds, self.offset_granularity)
                if offset_seconds != offset_seconds_truncated:
                    if self.strict:
                        valid = False
                        add_comment(
                            removed_zones, name,
                            f"STDOFF '{offset_string}' must be multiples of "
                            f"'{self.offset_granularity}' seconds")
                        break
                    else:
                        hm = seconds_to_hm_string(offset_seconds_truncated)
                        add_comment(
                            notable_zones, name,
                            f"STDOFF '{offset_string}' truncated to '{hm}'")

                # Check that offset seconds can fit in a timeCode field
                # implemented as a signed byte in multiples of 15-minutes.
                offset_code = div_to_zero(offset_seconds_truncated, 900)
                if offset_code < -127 or offset_code > 127:
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"STDOFF '{offset_string}' too large for 8-bits")
                    break

                era['offset_seconds'] = offset_seconds
                era['offset_seconds_truncated'] = offset_seconds_truncated

            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zones with invalid offset_string',
            notable_map=notable_zones,
        )
        merge_comments(self.all_removed_zones, removed_zones)
        merge_comments(self.all_notable_zones, notable_zones)
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
        removed_zones: CommentsMap = {}
        notable_zones: CommentsMap = {}
        for zone_name, eras in zones_map.items():
            valid = True
            for era in eras:
                if not era['format']:
                    add_comment(removed_zones, zone_name, 'FORMAT is empty')
                    valid = False
                    break

                if era['rules'] == '-' or ':' in era['rules']:
                    if '%' in era['format']:
                        add_comment(
                            removed_zones, zone_name,
                            "RULES is fixed but FORMAT contains '%'")
                        valid = False
                        break
                else:
                    if not ('%' in era['format'] or '/' in era['format']):
                        add_comment(
                            notable_zones, zone_name,
                            "RULES not fixed but FORMAT is missing "
                            + "'%' or '/'")

            if valid:
                results[zone_name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zones with invalid RULES and FORMAT combo',
            notable_map=notable_zones,
        )
        merge_comments(self.all_removed_zones, removed_zones)
        merge_comments(self.all_notable_zones, notable_zones)
        return results

    def _create_zones_with_rules_expansion(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Expand and normalize the zone.rules field (RULES) and create
        zone.rules_delta_seconds from zone.rules.

        The RULES field can hold the following:
            * '-' no rules
            * a string reference to a set of Rules
            * a delta offset like "01:00" to be added to the STDOFF field
                (see America/Argentina/San_Luis, Europe/Istanbul for example).
        After this method, the zone.rules contains 3 possible values:
            * '-' no rules, or
            * ':' which indicates that 'rules_delta_seconds' is defined, or
            * a string reference of the zone policy containing the rules
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        notable_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                rules_string = era['rules']
                if rules_string.find(':') >= 0:
                    if self.scope == 'basic':
                        valid = False
                        add_comment(
                            removed_zones, name,
                            f"offset in RULES '{rules_string}'")
                        break

                    rules_delta_seconds = time_string_to_seconds(rules_string)
                    if rules_delta_seconds == INVALID_SECONDS:
                        valid = False
                        add_comment(
                            removed_zones, name,
                            f"invalid RULES string '{rules_string}'")
                        break
                    if rules_delta_seconds == 0:
                        valid = False
                        add_comment(
                            removed_zones, name,
                            f"unexpected 0:00 RULES string '{rules_string}'")
                        break

                    # Check that RULES delta is a multiple of 15-minutes
                    # (or whatever delta_granularity is set to).
                    rules_delta_seconds_truncated = truncate_to_granularity(
                        rules_delta_seconds, self.delta_granularity)
                    if rules_delta_seconds != rules_delta_seconds_truncated:
                        if self.strict:
                            valid = False
                            add_comment(
                                removed_zones, name,
                                f"RULES delta '{rules_string}' must be "
                                f"multiples of '{self.delta_granularity}' "
                                f"seconds")
                            break
                        else:
                            hm = seconds_to_hm_string(
                                rules_delta_seconds_truncated)
                            add_comment(
                                notable_zones, name,
                                f"RULES delta offset '{rules_string}'"
                                f"truncated to '{hm}'")

                    # Check that rules_delta fits inside 4-bits, because that's
                    # how it is stored in the Arduino zonedb files.
                    rules_delta_code = rules_delta_seconds_truncated // 900
                    if rules_delta_code < -4 or rules_delta_code > 11:
                        valid = False
                        add_comment(
                            removed_zones, name,
                            f"RULES '{rules_string}' too large for 4-bits")
                        break

                    # Set the ZoneEra['rules'] to ':' to indicate that the RULES
                    # field is a DST offset.
                    era['rules'] = ':'
                    era['rules_delta_seconds'] = rules_delta_seconds
                    era['rules_delta_seconds_truncated'] = \
                        rules_delta_seconds_truncated
                else:
                    # If '-' or named policy, set to 0.
                    era['rules_delta_seconds'] = 0
                    era['rules_delta_seconds_truncated'] = 0
            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos with invalid RULES',
            notable_map=notable_zones,
        )
        merge_comments(self.all_removed_zones, removed_zones)
        merge_comments(self.all_notable_zones, notable_zones)
        return results

    def _remove_zones_without_rules(
        self, zones_map: ZonesMap, policies_map: PoliciesMap
    ) -> ZonesMap:
        """Remove zone eras whose RULES field contains a reference to
        a set of Rules, which cannot be found.
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            for era in eras:
                policy_name = era['rules']
                if (policy_name not in ['-', ':']
                        and policy_name not in policies_map):
                    valid = False
                    add_comment(
                        removed_zones, name,
                        f"policy '{policy_name}' not found")
                    break
            if valid:
                results[name] = eras

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos without rules',
        )
        merge_comments(self.all_removed_zones, removed_zones)
        return results

    def _remove_zones_with_non_monotonic_until(
        self, zones_map: ZonesMap,
    ) -> ZonesMap:
        """Remove Zone infos whose UNTIL fields are:
            1) not monotonically increasing, or
            2) does not end in year=MAX_UNTIL_YEAR
        """
        results: ZonesMap = {}
        removed_zones: CommentsMap = {}
        for name, eras in zones_map.items():
            valid = True
            prev_until = None
            for era in eras:
                # yapf: disable
                current_until = (
                    era['until_year'],
                    era['until_month'] if era['until_month'] else 0,
                    era['until_day'] if era['until_day_string'] else 0,
                    era['until_seconds'] if era['until_seconds'] else 0
                )
                # yapf: enable
                if prev_until:
                    if current_until <= prev_until:
                        valid = False
                        add_comment(
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
                add_comment(
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

        self._print_comments_map(
            removed_map=removed_zones,
            explanation='zone infos with invalid UNTIL fields',
        )
        merge_comments(self.all_removed_zones, removed_zones)
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
                from_year = rule['from_year']
                to_year = rule['to_year']
                month = rule['in_month']
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
        removed_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            removal = removals.get(name)
            if removal:
                add_comment(
                    removed_policies,
                    name,
                    f"Found {removal[0]} transitions in year/month "
                    f"'{removal[1]:04}-{removal[2]:02}'"
                )
            else:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with multiple transitions in one month',
        )
        merge_comments(self.all_removed_policies, removed_policies)
        return results

    def _remove_rules_long_dst_letter(
        self,
        policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Return a new map which filters out rules with long DST letter.
        """
        results: PoliciesMap = {}
        removed_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                letter = rule['letter']
                if len(letter) > 1:
                    valid = False
                    add_comment(
                        removed_policies, name,
                        f"LETTER '{letter}' too long")
                    break
            if valid:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with long DST letter',
        )
        merge_comments(self.all_removed_policies, removed_policies)
        return results

    def _remove_rules_invalid_at_time_suffix(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Remove rules whose at_time contains an unsupported suffix. Current
        supported suffix is 'w', 's' and 'u'. The 'g' and 'z' are identifical
        to 'u' and they do not currently appear in any TZ file, so let's catch
        them because it could indicate a bug somewhere in our parser or
        somewhere else.
        """
        supported_suffices = ['w', 's', 'u']
        results: PoliciesMap = {}
        removed_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                suffix = rule['at_time_suffix']
                suffix = suffix if suffix else 'w'
                rule['at_time_suffix'] = suffix
                if suffix not in supported_suffices:
                    valid = False
                    add_comment(
                        removed_policies, name,
                        f"unsupported AT time suffix '{suffix}'")
                    break
            if valid:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with unsupported AT suffix',
        )
        merge_comments(self.all_removed_policies, removed_policies)
        return results

    def _mark_rules_used_by_zones(
        self, zones_map: ZonesMap, policies_map: PoliciesMap,
    ) -> Tuple[ZonesMap, PoliciesMap]:
        """Mark all rules which are required by various zones. There are 2 ways
        that a rule can be used by a zone era:
            1) The rule's from_year or to_year are >= (self.start_year - 1), or
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
                until_year = min(era['until_year'], self.until_year)
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
                begin_year = era['until_year']

        return (zones_map, policies_map)

    def _remove_rules_unused(self, policies_map: PoliciesMap) -> PoliciesMap:
        """Remove RULE entries which have not been marked as used by the
        _mark_rules_used_by_zones() method. It is expected that all remaining
        RULE entries have FROM and TO fields which is greater than 1872 (the
        earliest year which can be represented by an int8_t to_year_tiny field,
        (2000-128)==1872). See also _remove_rules_out_of_bounds().
        """
        results: PoliciesMap = {}
        removed_rule_count = 0
        removed_policies: CommentsMap = {}
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
                add_comment(removed_policies, name, 'unused')

        logging.info(
            'Removed %s rule policies (with %s rules) not used',
            len(removed_policies),
            removed_rule_count
        )
        merge_comments(self.all_removed_policies, removed_policies)
        return results

    def _remove_rules_out_of_bounds(
        self,
        policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Remove policies which have FROM and TO fields do not fit in an
        int8_t. In other words, y < 1872 or (y > 2127 and y != 9999).
        """
        results: PoliciesMap = {}
        removed_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                from_year = rule['from_year']
                to_year = rule['to_year']
                if not is_year_tiny(from_year) or not is_year_tiny(from_year):
                    valid = False
                    add_comment(
                        removed_policies, name,
                        f"from_year ({from_year}) or to_year ({to_year}) "
                        f" out of bounds")
                    break
            if valid:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with from_year or to_year out of bounds',
        )
        merge_comments(self.all_removed_policies, removed_policies)
        return results

    def _create_rules_with_on_day_expansion(
        self, policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """Create rule['on_day_of_week'] and rule['on_day_of_month'] from
        rule['on_day']. The on_day_of_month will be negative if "<=" is used.
        """
        results: PoliciesMap = {}
        removed_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                on_day = rule['on_day']
                (on_day_of_week, on_day_of_month) = _parse_on_day_string(on_day)

                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    add_comment(
                        removed_policies, name,
                        f"invalid on_day '{on_day}'")
                    break

                # *ZoneProcessor.h classes currently do not support
                # "dayOfWeek<=6" or "dayOfWeek>=26" if the shift causes the year
                # to change.
                if on_day_of_week != 0 and on_day_of_month != 0:
                    if (-7 <= on_day_of_month
                            and on_day_of_month < -1
                            and rule['in_month'] == 1):
                        valid = False
                        add_comment(
                            removed_policies, name,
                            f"cannot shift '{on_day}' from Jan to prev year")
                        break
                    if 26 <= on_day_of_month and rule['in_month'] == 12:
                        valid = False
                        add_comment(
                            removed_policies, name,
                            f"cannot shift '{on_day}' from Dec to next year")
                        break

                rule['on_day_of_week'] = on_day_of_week
                rule['on_day_of_month'] = on_day_of_month
            if valid:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with invalid on_day',
        )
        merge_comments(self.all_removed_policies, removed_policies)
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
            from_year = rule['from_year']
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
            from_year = rule['from_year']
            in_month = rule['in_month']
            on_day_of_week = rule['on_day_of_week']
            on_day_of_month = rule['on_day_of_month']
            month, day = calc_day_of_month(
                from_year, in_month, on_day_of_week, on_day_of_month)
            rule_date = (from_year, month, day)

            if (rule['delta_seconds'] == 0
                    and rule_date < anchor_info['earliestDate']):
                anchor_info['earliestDate'] = rule_date
                anchor_info['rule'] = rule

        assert anchor_info['rule'] is not None
        anchor_rule = anchor_info['rule'].copy()
        anchor_rule['from_year'] = MIN_YEAR
        anchor_rule['to_year'] = MIN_YEAR
        anchor_rule['in_month'] = 1
        anchor_rule['on_day_of_week'] = 0
        anchor_rule['on_day_of_month'] = 1
        anchor_rule['at_time'] = '0'
        anchor_rule['at_time_suffix'] = 'w'
        anchor_rule['delta_offset'] = '0'
        anchor_rule['at_seconds'] = 0
        anchor_rule['at_seconds_truncated'] = 0
        anchor_rule['delta_seconds'] = 0
        anchor_rule['delta_seconds_truncated'] = 0
        anchor_rule['raw_line'] = 'Anchor: ' + anchor_rule['raw_line']
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
        removed_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                from_year = rule['from_year']
                to_year = rule['to_year']
                month = rule['in_month']
                on_day_of_month = rule['on_day_of_month']
                if from_year > MIN_YEAR and to_year > MIN_YEAR:
                    if month == 1 and on_day_of_month == 1:
                        valid = False
                        add_comment(
                            removed_policies, name,
                            "Transition on Jan 1 not supported "
                            f"({from_year:04}-{month:02}-{on_day_of_month:02})"
                        )
                        break
            if valid:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with border Transitions',
        )
        merge_comments(self.all_removed_policies, removed_policies)
        return results

    def _create_rules_with_expanded_at_time(
        self,
        policies_map: PoliciesMap,
        policies_to_zones: PoliciesToZones,
    ) -> PoliciesMap:
        """ Create 'at_seconds' parameter from rule['at_time'].
        """
        results: PoliciesMap = {}
        removed_policies: CommentsMap = {}
        notable_policies: CommentsMap = {}
        for policy_name, rules in policies_map.items():
            valid = True
            for rule in rules:
                at_time = rule['at_time']
                at_seconds = time_string_to_seconds(at_time)
                if at_seconds == INVALID_SECONDS:
                    valid = False
                    add_comment(
                        removed_policies, policy_name,
                        f"invalid AT time '{at_time}'" % at_time)
                    break
                if at_seconds < 0:
                    valid = False
                    add_comment(
                        removed_policies, policy_name,
                        f"negative AT time '{at_time}'" % at_time)
                    break

                at_seconds_truncated = truncate_to_granularity(
                    at_seconds, self.until_at_granularity)
                if at_seconds != at_seconds_truncated:
                    if self.strict:
                        valid = False
                        add_comment(
                            removed_policies, policy_name,
                            f"AT time '{at_time}' must be multiples of "
                            f"'{self.until_at_granularity}' seconds")
                        break
                    else:
                        hm = seconds_to_hm_string(at_seconds_truncated)
                        add_comment(
                            notable_policies, policy_name,
                            f"AT time '{at_time}' truncated to '{hm}'")
                        # Add warning about the affected zones.
                        zone_names = policies_to_zones.get(policy_name)
                        if zone_names:
                            for zone_name in zone_names:
                                hm = seconds_to_hm_string(at_seconds_truncated)
                                add_comment(
                                    self.all_notable_zones, zone_name,
                                    f"AT time '{at_time}' of "
                                    f"RULE '{policy_name}' "
                                    f"truncated to '{hm}'")

                rule['at_seconds'] = at_seconds
                rule['at_seconds_truncated'] = at_seconds_truncated
            if valid:
                results[policy_name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with invalid at_time',
            notable_map=notable_policies,
        )
        merge_comments(self.all_removed_policies, removed_policies)
        merge_comments(self.all_notable_policies, notable_policies)
        return results

    def _create_rules_with_expanded_delta_offset(
        self,
        policies_map: PoliciesMap,
    ) -> PoliciesMap:
        """ Create 'delta_seconds' and 'delta_seconds_truncated' from
        rule['delta_offset'].
        """
        results = {}
        removed_policies: CommentsMap = {}
        notable_policies: CommentsMap = {}
        for name, rules in policies_map.items():
            valid = True
            for rule in rules:
                delta_offset = rule['delta_offset']
                delta_seconds = time_string_to_seconds(delta_offset)
                if delta_seconds == INVALID_SECONDS:
                    valid = False
                    add_comment(
                        removed_policies, name,
                        f"invalid SAVE (delta_offset) '{delta_offset}'")
                    break

                # Truncate to requested granularity.
                delta_seconds_truncated = truncate_to_granularity(
                    delta_seconds, self.delta_granularity)
                if delta_seconds != delta_seconds_truncated:
                    if self.strict:
                        valid = False
                        add_comment(
                            removed_policies, name,
                            f"SAVE (delta_offset) '{delta_offset}' must be "
                            f"a multiple of '{self.delta_granularity}' "
                            f"seconds")
                        break
                    else:
                        add_comment(
                            notable_policies, name,
                            f"SAVE delta_offset '{delta_offset}' truncated to"
                            f"a multiple of '{self.delta_granularity}' "
                            f"seconds")

                # Check that delta seconds can fit in a 4-bit timeCode field
                # with 15-minute granularity, defined as (timeCode =
                # delta_seconds / 900s + 1h) which encodes -1:00 as 0 and 3:45
                # as 15.
                delta_code = delta_seconds_truncated // 900
                if delta_code < -4 or delta_code > 11:
                    valid = False
                    add_comment(
                        removed_policies, name,
                        f"SAVE delta_offset '{delta_offset}' "
                        "too large for 4-bits")
                    break

                rule['delta_seconds'] = delta_seconds
                rule['delta_seconds_truncated'] = delta_seconds_truncated
            if valid:
                results[name] = rules

        self._print_comments_map(
            removed_map=removed_policies,
            explanation='rule policies with invalid SAVE (delta_offset)',
            notable_map=notable_policies,
        )
        merge_comments(self.all_removed_policies, removed_policies)
        merge_comments(self.all_notable_policies, notable_policies)
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
        removed_links: CommentsMap = {}
        for link_name, zone_name in links_map.items():
            if zones_map.get(zone_name):
                results[link_name] = zone_name
            else:
                add_comment(
                    removed_links, link_name,
                    f'Target Zone "{zone_name}" missing')

        self._print_comments_map(
            removed_map=removed_links,
            explanation='links with missing zones',
        )
        merge_comments(self.all_removed_links, removed_links)
        return results

    def remove_zones_and_links_with_similar_names(
        self,
        zones_map: ZonesMap,
        links_map: LinksMap,
    ) -> Tuple[ZonesMap, LinksMap]:
        """Currently, there are no conflicts, but if there were 2 zones names
        like "Etc/GMT-0" and "Etc/GMT_0", both would normalize to "Etc/GMT_0",
        producing a symbol "kZoneEtc_GMT_0, so one of them is thrown out.
        """
        normalized_names: Dict[str, str] = {}  # normalized_name, name
        result_zones: ZonesMap = {}
        result_links: LinksMap = {}
        removed_zones: CommentsMap = {}
        removed_links: CommentsMap = {}

        # Check for duplicate zone names.
        for zone_name, zone in zones_map.items():
            nname = normalize_name(zone_name)
            if normalized_names.get(nname):
                add_comment(
                    removed_zones, zone_name,
                    'Duplicate normalized name')
            else:
                normalized_names[nname] = zone_name
                result_zones[zone_name] = zone

        # Then strike out any duplicate links.
        for link_name, link in links_map.items():
            nname = normalize_name(link_name)
            if normalized_names.get(nname):
                add_comment(
                    removed_links, link_name,
                    'Duplicate normalized name')
            else:
                normalized_names[nname] = link_name
                result_links[link_name] = link

        logging.info(
            'Removed %d Zones and %s Links with duplicate names',
            len(removed_zones),
            len(removed_links))
        merge_comments(self.all_removed_zones, removed_zones)
        merge_comments(self.all_removed_links, removed_links)
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
        if rule['from_year'] < era_until and era_from <= rule['to_year']:
            matches.append(rule)
    return matches


def find_latest_prior_rules(
    rules: List[ZoneRuleRaw],
    year: int,
) -> List[ZoneRuleRaw]:
    """Find the most recent prior rules before the given year. The RULE.at_time
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
        rule_year = rule['to_year']
        rule_month = rule['in_month']
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
        rule_year = rule['to_year']
        rule_month = rule['in_month']
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
    (on_day_of_week >= on_day_of_month), (on_day_of_week <= on_day_of_month), or
    (lastMon) See BasicZoneSpecifier::calcStartDayOfMonth(). Shifts into
    previous or next month can occur.

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


def _create_policies_to_zones(
    zones_map: ZonesMap,
    policies_map: PoliciesMap,
) -> PoliciesToZones:
    """Normally Zones point to Rules. This method causes the reverse to happen,
    making Rules know about Zones, by creating a map of {policy_name ->
    zone_full_name[]}. This allows us to determine which zones that may be
    affected by a change in a particular Rule. Must be called after
    _create_zones_with_rules_expansion() to normalize the RULES column
    (zone.rules).
    """
    policies_to_zones: PoliciesToZones = {}
    for full_name, eras in zones_map.items():
        for era in eras:
            policy_name = era['rules']
            if policy_name not in ['-', ':']:
                zones = policies_to_zones.get(policy_name)
                if not zones:
                    zones = []
                    policies_to_zones[policy_name] = zones
                zones.append(full_name)
    return policies_to_zones


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
