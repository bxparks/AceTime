#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Cleanses and transforms the 'zones' and 'rules' data so that it can be used for
code generation in the ArduinoGenerator, PythonGenerator or the InlineGenerator.
"""

import logging
import sys
import datetime
import extractor
from extractor import MAX_UNTIL_YEAR
from extractor import MIN_YEAR
from extractor import ZoneRuleRaw
from extractor import ZoneEraRaw


class Transformer:
    def __init__(self, zones_map, rules_map, language, start_year, granularity,
                 strict):
        """
        Arguments:
            zones_map: map of Zone names to ZoneEras
            rules_map: map of Policy names to ZoneRules
            language: (str) target language ('python', 'arduino', 'arduinox')
            start_year: (int) include only years on or after start_year
            granularity: (int) retained AT, SAVE, UNTIL, or RULES(offset)
                fields in seconds
            strict: (bool) throw out Zones or Rules which are not exactly
                on the time boundary defined by granularity
        """
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.language = language
        self.start_year = start_year
        self.granularity = granularity
        self.strict = strict

        self.all_removed_zones = {}  # map of zone name -> reason
        self.all_removed_policies = {}  # map of policy name -> reason

        self.all_notable_zones = {}  # map of zone name -> reason
        self.all_notable_policies = {}  # map of policy name -> reason

    def get_data(self):
        """
        Returns a tuple of 6 data structures:

        * 'zones_map' is a map of (name -> ZoneEraRaw[]).

        * 'rules_map' is a map of (name -> ZoneRuleRaw[]).

        * 'all_removed_zones' is a map of the zones which were removed:
            name: name of zone removed
            reason: human readable reason

        * 'all_removed_policies' is a map of the policies (entire set of RULEs)
        which were removed:
            name: name of policy removed
            reason: human readable reason

        * 'all_notable_zones' is a map of the zones which come with caveats,
          e.g., truncation of '00:01' to '00:00'.
            name: name of zone
            reason: human readable reason

        * 'all_notable_policies' is a map of the policies come with caveats:
            name: name of policy
            reason: human readable reason

        """
        return (self.zones_map, self.rules_map, self.all_removed_zones,
                self.all_removed_policies, self.all_notable_zones,
                self.all_notable_policies)

    def transform(self):
        zones_map = self.zones_map
        rules_map = self.rules_map

        logging.info('Found %s zone infos' % len(self.zones_map))
        logging.info('Found %s rule policies' % len(self.rules_map))

        zones_map = self._remove_zones_with_duplicate_short_names(zones_map)
        zones_map = self._remove_zones_without_slash(zones_map)
        zones_map = self._remove_zone_eras_too_old(zones_map)
        if self.language == 'arduino':
            zones_map = self._remove_zone_until_year_only_false(zones_map)
        zones_map = self._create_zones_with_until_day(zones_map)
        zones_map = self._create_zones_with_expanded_until_time(zones_map)
        zones_map = self._remove_zones_invalid_until_time_modifier(zones_map)
        zones_map = self._create_zones_with_expanded_offset_string(zones_map)
        zones_map = self._create_zones_with_rules_expansion(zones_map)
        zones_map = self._remove_zones_with_non_monotonic_until(zones_map)

        (zones_map, rules_map) = self._mark_rules_used_by_zones(
            zones_map, rules_map)
        rules_map = self._remove_rules_unused(rules_map)
        rules_map = self._remove_rules_out_of_bounds(rules_map)
        if self.language == 'arduino':
            rules_map = self._remove_rules_multiple_transitions_in_month(
                rules_map)

        rules_map = self._create_rules_with_expanded_at_time(rules_map)
        rules_map = self._remove_rules_invalid_at_time_modifier(rules_map)
        rules_map = self._create_rules_with_expanded_delta_offset(rules_map)
        rules_map = self._create_rules_with_on_day_expansion(rules_map)
        rules_map = self._create_rules_with_anchor_transition(rules_map)
        if self.language == 'arduino':
            rules_map = self._remove_rules_with_border_transitions(rules_map)
        if self.language == 'arduino' or self.language == 'arduinox':
            rules_map = self._remove_rules_long_dst_letter(rules_map)

        zones_map = self._remove_zones_without_rules(zones_map, rules_map)

        self.rules_map = rules_map
        self.zones_map = zones_map

    def print_summary(self):
        logging.info('-------- Transformer Summary')
        logging.info('---- Zones')
        logging.info('Removed %s infos' % len(self.all_removed_zones))
        logging.info('Noted %s infos' % len(self.all_notable_zones))
        logging.info('Generated %s infos' % len(self.zones_map))

        logging.info('---- Rules')
        logging.info('Removed %s policies' % len(self.all_removed_policies))
        logging.info('Noted %s policies' % len(self.all_notable_policies))
        logging.info('Generated %s policies' % len(self.rules_map))
        logging.info('-------- Transformer Summary End')

    def _print_removed_map(self, removed_map):
        """Helper routine that prints the removed Zone rules or Zone eras along
        with the reason why it was removed.
        """
        for name, reason in sorted(removed_map.items()):
            print('  %s (%s)' % (name, reason), file=sys.stderr)

    # --------------------------------------------------------------------
    # Methods related to Zones.
    # --------------------------------------------------------------------

    def _remove_zones_with_duplicate_short_names(self, zones_map):
        results = {}
        removed_zones = {}
        short_names = {}
        for name, zones in zones_map.items():
            short = short_name(name)
            if short in short_names:
                removed_zones[name] = "duplicate short name '%s'" % short
            else:
                short_names[short] = name
                results[name] = zones

        logging.info("Removed %s zone infos with duplicate short names" %
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def _remove_zones_without_slash(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            if name.rfind('/') >= 0:
                results[name] = zones
            else:
                removed_zones[name] = "no '/' in zone name"

        logging.info(
            "Removed %s zone infos without '/' in name" % len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def _remove_zone_eras_too_old(self, zones_map):
        """Remove zone eras which are too old, i.e. before (self.start_year-1).
        For start_year 2000, and viewing_months>13,
        ZoneSpecifier.init_for_year() could be called with 1999.
        """
        results = {}
        count = 0
        for name, zones in zones_map.items():
            keep_zones = []
            for zone in zones:
                if zone.untilYear >= self.start_year - 1:
                    keep_zones.append(zone)
                else:
                    count += 1
            if keep_zones:
                results[name] = keep_zones

        logging.info("Removed %s zone eras before year %04d", count,
                     self.start_year)
        return results

    def _remove_zone_until_year_only_false(self, zones_map):
        """Remove zones which have month, day or time in the UNTIL field.
        These are not supported by the early version of AutoZoneSpecifier.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if not zone.untilYearOnly:
                    valid = False
                    removed_zones[name] = "UNTIL contains month/day/time"
                    break
            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with UNTIL month/day/time",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def _create_zones_with_until_day(self, zones_map):
        """Convert zone.untilDay from 'lastSun' or 'Sun>=1' to a precise day,
        which is possible because the year and month are already known. For
        example:
            * Asia/Tbilisi 2005 3 lastSun 2:00
            * America/Grand_Turk 2015 Nov Sun>=1 2:00
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                until_day = zone.untilDay

                # Parse the conditional expression in until_day. We can resolve
                # the 'lastSun' and 'Sun>=X' to a specific day of month because
                # we know the year.
                (on_day_of_week, on_day_of_month) = \
                    parse_on_day_string(until_day)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    removed_zones[name] = "invalid untilDay '%s'" % until_day
                    break

                zone.untilDay = calc_day_of_month(
                    zone.untilYear, zone.untilMonth, on_day_of_week,
                    on_day_of_month)
            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with invalid untilDay",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def _create_zones_with_expanded_until_time(self, zones_map):
        """ Create 'untilSeconds' and 'untilSecondsTruncated' from 'untilTime'.
        """
        results = {}
        removed_zones = {}
        notable_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                until_time = zone.untilTime
                until_seconds = time_string_to_seconds(until_time)
                if until_seconds == INVALID_SECONDS:
                    valid = False
                    removed_zones[name] = (
                        "invalid UNTIL time '%s'" % until_time)
                    break
                if until_seconds < 0:
                    valid = False
                    removed_zones[name] = (
                        "negative UNTIL time '%s'" % until_time)
                    break

                until_seconds_truncated = truncate_to_granularity(
                    until_seconds, self.granularity)
                if until_seconds != until_seconds_truncated:
                    if self.strict:
                        valid = False
                        removed_zones[name] = (
                            "UNTIL time '%s' must be multiples of '%s' seconds"
                            % (until_time, self.granularity))
                        break
                    else:
                        notable_zones[name] = (
                            "UNTIL time '%s' truncated to '%s' seconds" %
                            (until_time, self.granularity))

                zone.untilSeconds = until_seconds
                zone.untilSecondsTruncated = until_seconds_truncated
            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with invalid UNTIL time",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        self.all_notable_zones.update(notable_zones)
        return results

    def _remove_zones_invalid_until_time_modifier(self, zones_map):
        """Remove zones whose UNTIL time contains an unsupported modifier.
        """
        # Determine which suffices are supported. The 'g' and 'z' is the same as
        # 'u' and does not currently appear in any TZ file, so let's catch it
        # because it could indicate a bug
        if self.language == 'arduino':
            supported_suffices = ['w']
        elif self.language == 'arduinox' or self.language == 'python':
            supported_suffices = ['w', 's', 'u']
        else:
            raise Exception('Unknown laguage: %s' % self.language)

        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                modifier = zone.untilTimeModifier
                modifier = modifier if modifier else 'w'
                zone.untilTimeModifier = modifier
                if modifier not in supported_suffices:
                    valid = False
                    removed_zones[name] = (
                        "unsupported UNTIL time modifier '%s'" % modifier)
                    break
            if valid:
                results[name] = zones

        logging.info(
            "Removed %s zone infos with unsupported UNTIL time modifier",
            len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_policies.update(removed_zones)
        return results

    def _create_zones_with_expanded_offset_string(self, zones_map):
        """ Create expanded offset 'offsetSeconds' from zone.offsetString.
        """
        results = {}
        removed_zones = {}
        notable_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                offset_string = zone.offsetString
                offset_seconds = time_string_to_seconds(offset_string)
                if offset_seconds == INVALID_SECONDS:
                    valid = False
                    removed_zones[name] = (
                        "invalid GMTOFF offset string '%s'" % offset_string)
                    break

                offset_seconds_truncated = truncate_to_granularity(
                    offset_seconds, self.granularity)
                if offset_seconds != offset_seconds_truncated:
                    if self.strict:
                        valid = False
                        removed_zones[name] = (
                            "GMTOFF '%s' must be multiples of '%s' seconds" %
                            (offset_string, self.granularity))
                        break
                    else:
                        notable_zones[name] = (
                            "GMTOFF '%s' truncated to '%s' seconds" %
                            (offset_string, self.granularity))

                zone.offsetSeconds = offset_seconds
                zone.offsetSecondsTruncated = offset_seconds_truncated

            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with invalid offsetString",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        self.all_notable_zones.update(notable_zones)
        return results

    def _create_zones_with_rules_expansion(self, zones_map):
        """ Create zone.rulesDeltaSeconds from zone.rules.

        The RULES field can hold the following:
            * '-' no rules
            * a string reference to a set of Rules
            * a delta offset like "01:00" to be added to the GMTOFF field
                (see America/Argentina/San_Luis, Europe/Istanbul for example).
        After this method, the zone.rules contains 3 possible values:
            * '-' no rules, or
            * ':' which indicates that 'rulesDeltaSeconds' is defined, or
            * a string reference
        """
        results = {}
        removed_zones = {}
        notable_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rules_string = zone.rules
                if rules_string.find(':') >= 0:
                    if self.language == 'arduino':
                        valid = False
                        removed_zones[name] = (
                            "offset in RULES '%s'" % rules_string)
                        break

                    rules_delta_seconds = time_string_to_seconds(rules_string)
                    if rules_delta_seconds == INVALID_SECONDS:
                        valid = False
                        removed_zones[name] = (
                            "invalid RULES string '%s'" % rules_string)
                        break
                    if rules_delta_seconds == 0:
                        valid = False
                        removed_zones[name] = (
                            "unexpected 0:00 RULES string '%s'" % rules_string)
                        break

                    rules_delta_seconds_truncated = truncate_to_granularity(
                        rules_delta_seconds, self.granularity)
                    if rules_delta_seconds != rules_delta_seconds_truncated:
                        if self.strict:
                            valid = False
                            removed_zones[name] = (
                                "RULES delta offset '%s' must be multiples of "
                                + "'%s' seconds" %
                                (rules_string, self.granularity))
                            break
                        else:
                            notable_zones[name] = (
                                "RULES delta offset '%s' truncated to" +
                                "'%s' seconds" %
                                (rules_string, self.granularity))

                    zone.rules = ':'
                    zone.rulesDeltaSeconds = rules_delta_seconds
                    zone.rulesDeltaSecondsTruncated = \
                        rules_delta_seconds_truncated
                else:
                    # If '-' or named policy, set to 0.
                    zone.rulesDeltaSeconds = 0
                    zone.rulesDeltaSecondsTruncated = 0
            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with invalid RULES",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        self.all_notable_zones.update(notable_zones)
        return results

    def _remove_zones_without_rules(self, zones_map, rules_map):
        """Remove zone eras whose RULES field contains a reference to
        a set of Rules, which cannot be found.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rule_name = zone.rules
                if rule_name not in ['-', ':'] and rule_name not in rules_map:
                    valid = False
                    removed_zones[name] = "policy '%s' not found" % rule_name
                    break
            if valid:
                results[name] = zones

        logging.info(
            "Removed %s zone infos without rules" % len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def _remove_zones_with_non_monotonic_until(self, zones_map):
        """Remove Zone infos whose UNTIL fields are:
            1) not monotonically increasing, or
            2) does not end in year=MAX_UNTIL_YEAR
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            prev_until = None
            for zone in zones:
                # yapf: disable
                current_until = (
                    zone.untilYear,
                    zone.untilMonth if zone.untilMonth else 0,
                    zone.untilDay if zone.untilDay else 0,
                    zone.untilSeconds if zone.untilSeconds else 0
                )
                # yapf: enable
                if prev_until:
                    if current_until <= prev_until:
                        valid = False
                        removed_zones[name] = (
                            'non increasing UNTIL: %04d-%02d-%02d %ds' %
                            current_until)
                        break
                prev_until = current_until
            if valid and current_until[0] != extractor.MAX_UNTIL_YEAR:
                valid = False
                removed_zones[name] = (
                    'invalid final UNTIL: %04d-%02d-%02d %ds' % current_until)

            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with invalid UNTIL fields",
                     len(removed_zones))
        self._print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    # --------------------------------------------------------------------
    # Methods related to Rules
    # --------------------------------------------------------------------

    def _remove_rules_multiple_transitions_in_month(self, rules_map):
        """Some Zone policies have Rules which specify multiple DST transitions
        within in the same month:
            * Egypt (Found '2' transitions in year/month '2010-09')
            * Palestine (Found '2' transitions in year/month '2011-08')
            * Spain (Found '2' transitions in year/month '1938-04')
            * Tunisia (Found '2' transitions in year/month '1943-04')
        """
        # First pass: collect number of transitions for each (year, month) pair.
        counts = {}
        for name, rules in rules_map.items():
            for rule in rules:
                from_year = rule.fromYear
                to_year = rule.toYear
                month = rule.inMonth
                for year in range(from_year, to_year + 1):
                    key = (name, year, month)
                    count = counts.get(key)
                    count = count + 1 if count else 1
                    counts[key] = count

        # Second pass: Collect rule policies which have multiple transitions
        # in one month.
        removals = {}
        for key, count in counts.items():
            if count > 1:
                policy_name = key[0]
                year = key[1]
                month = key[2]
                removals[policy_name] = (count, year, month)

        # Third pass: Remove rule policies with multiple counts.
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            removal = removals.get(name)
            if removal:
                removed_policies[name] = (
                    "Found %d transitions in year/month '%04d-%02d'" %
                    removals[name])
            else:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with multiple transitions in one month' %
            len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def _remove_rules_long_dst_letter(self, rules_map):
        """Return a new map which filters out rules with long DST letter.
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                letter = rule.letter
                if len(letter) > 1:
                    valid = False
                    removed_policies[name] = "LETTER '%s' too long" % letter
                    break
            if valid:
                results[name] = rules

        logging.info('Removed %s rule policies with long DST letter' %
                     len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def _remove_rules_invalid_at_time_modifier(self, rules_map):
        """Remove rules whose atTime contains an unsupported modifier. Current
        supported modifier is 'w', 's' and 'u'. The 'g' and 'z' are identifical
        to 'u' and they do not currently appear in any TZ file, so let's catch
        them because it could indicate a bug somewhere in our parser or
        somewhere else.
        """
        supported_suffices = ['w', 's', 'u']
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                modifier = rule.atTimeModifier
                modifier = modifier if modifier else 'w'
                rule.atTimeModifier = modifier
                if modifier not in supported_suffices:
                    valid = False
                    removed_policies[name] = (
                        "unsupported AT time modifier '%s'" % modifier)
                    break
            if valid:
                results[name] = rules

        logging.info("Removed %s rule policies with unsupported AT modifier" %
                     len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def _mark_rules_used_by_zones(self, zones_map, rules_map):
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
                policy_name = era.rules
                if policy_name in ['-', ':']:
                    continue

                rules = rules_map.get(policy_name)
                if not rules:
                    logging.error("Zone '%s': Could not find policy '%s': " +
                                  "should not happen", zone_name, policy_name)
                    sys.exit(1)

                # Make all Rules which overlap with the current Zone Era.
                # Some Zone Era have an until_month, until_day and until_time
                # components. To be conservative, we need to expand the
                # until_year to the following year, so the effective zone era
                # interval becomes [begin_year, until_year+1).
                until_year = era.untilYear
                matching_rules = find_matching_rules(rules, begin_year,
                                                     until_year + 1)
                for rule in matching_rules:
                    rule.used = True

                # Find latest Rules just prior to the begin_year.
                # Result: It looks like all of these prior rules are
                # already picked up by previous calls to find_matching_rules().
                prior_rules = find_latest_prior_rules(rules, begin_year)
                for rule in prior_rules:
                    rule.used = True

                # Find earliest Rules subsequent to the until_year mark.
                # Result: It looks like all of these prior rules are
                # already picked up by previous calls to find_matching_rules().
                subsequent_rules = find_earliest_subsequent_rules(
                    rules, until_year + 1)
                for rule in subsequent_rules:
                    rule.used = True

                begin_year = until_year

        return (zones_map, rules_map)

    def _remove_rules_unused(self, rules_map):
        """Remove RULE entries which have not been marked as used by the
        _mark_rules_used_by_zones() method. It is expected that all remaining
        RULE entries have FROM and TO fields which is greater than 1872 (the
        earliest year which can be represented by an int8_t toYearTiny field,
        (2000-128)==1872). See also _remove_rules_out_of_bounds().
        """
        results = {}
        removed_rule_count = 0
        removed_policies = {}
        for name, rules in rules_map.items():
            used_rules = []
            for rule in rules:
                if rule.used:
                    used_rules.append(rule)
                else:
                    removed_rule_count += 1
            if used_rules:
                results[name] = used_rules
            else:
                removed_policies[name] = 'unused'

        logging.info('Removed %s rule policies (%s rules) not used' %
                     (len(removed_policies), removed_rule_count))
        self.all_removed_policies.update(removed_policies)
        return results

    def _remove_rules_out_of_bounds(self, rules_map):
        """Remove policies which have FROM and TO fields do not fit in an
        int8_t. In other words, y < 1872 or (y > 2127 and y != 9999).
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                from_year = rule.fromYear
                to_year = rule.toYear
                if not is_year_tiny(from_year) or not is_year_tiny(from_year):
                    valid = False
                    removed_policies[name] = (
                        "fromYear (%s) or toYear (%s) out of bounds" %
                        (from_year, to_year))
                    break
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with fromYear or toYear out of bounds' %
            len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def _create_rules_with_on_day_expansion(self, rules_map):
        """Create rule.onDayOfWeek and rule.onDayOfMonth from
        rule.onDay.
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                on_day = rule.onDay
                (on_day_of_week, on_day_of_month) = parse_on_day_string(on_day)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    removed_policies[name] = ("invalid onDay '%s'" % on_day)
                    break
                rule.onDayOfWeek = on_day_of_week
                rule.onDayOfMonth = on_day_of_month
            if valid:
                results[name] = rules

        logging.info('Removed %s rule policies with invalid onDay' %
                     len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def _create_rules_with_anchor_transition(self, rules_map):
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
        anchored_policies = []
        for name, rules in rules_map.items():
            if not self._has_prior_rule(rules):
                anchor_rule = self._get_anchor_rule(rules)
                rules.insert(0, anchor_rule)
                anchored_policies.append(name)

        logging.info('Added anchor rule to %s rule policies: %s',
                     len(anchored_policies), anchored_policies)
        return rules_map

    def _has_prior_rule(self, rules):
        """Return True if rules has a rule prior to (self.start_year-1).
        """
        for rule in rules:
            from_year = rule.fromYear
            to_year = rule.toYear
            if from_year < self.start_year - 1:
                return True
        return False

    def _get_anchor_rule(self, rules):
        """Return the anchor rule that will act as the earliest rule with SAVE
        == 0.
        """
        anchor_rule = ZoneRuleRaw({
            'earliestDate': (MAX_UNTIL_YEAR, 12, 31),
        })
        for rule in rules:
            from_year = rule.fromYear
            in_month = rule.inMonth
            on_day_of_week = rule.onDayOfWeek
            on_day_of_month = rule.onDayOfMonth
            on_day = calc_day_of_month(from_year, in_month, on_day_of_week,
                                       on_day_of_month)
            rule_date = (from_year, in_month, on_day)
            rule.earliestDate = rule_date

            if rule.deltaSeconds == 0 and rule_date < anchor_rule.earliestDate:
                anchor_rule = rule

        anchor_rule = anchor_rule.copy()
        anchor_rule.fromYear = MIN_YEAR
        anchor_rule.toYear = MIN_YEAR
        anchor_rule.inMonth = 1
        anchor_rule.onDayOfWeek = 0
        anchor_rule.onDayOfMonth = 1
        anchor_rule.atTime = '0'
        anchor_rule.atTimeModifier = 'w'
        anchor_rule.deltaOffset = '0'
        anchor_rule.atSeconds = 0
        anchor_rule.atSecondsTruncated = 0
        anchor_rule.deltaSeconds = 0
        anchor_rule.deltaSecondsTruncated = 0
        anchor_rule.rawLine = 'Anchor: ' + anchor_rule.rawLine
        return anchor_rule

    def _remove_rules_with_border_transitions(self, rules_map):
        """Remove rules where the transition occurs on the first day of the
        year. That situation is not supported by AutoZoneSpecifier. On the other
        hand, a transition at the end of the year (12/31) is supported.
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                from_year = rule.fromYear
                to_year = rule.toYear
                month = rule.inMonth
                on_day_of_week = rule.onDayOfWeek
                on_day_of_month = rule.onDayOfMonth
                if from_year > MIN_YEAR and to_year > MIN_YEAR:
                    if month == 1 and on_day_of_month == 1:
                        valid = False
                        removed_policies[name] = (
                            "Transition in early year (%04d-%02d-%02d)" %
                            (from_year, month, on_day_of_month))
                        break
            if valid:
                results[name] = rules

        logging.info("Removed %s rule policies with border Transitions" %
                     len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def _create_rules_with_expanded_at_time(self, rules_map):
        """ Create 'atSeconds' parameter from rule.atTime.
        """
        results = {}
        removed_policies = {}
        notable_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                at_time = rule.atTime
                at_seconds = time_string_to_seconds(at_time)
                if at_seconds == INVALID_SECONDS:
                    valid = False
                    removed_policies[name] = ("invalid AT time '%s'" % at_time)
                    break
                if at_seconds < 0:
                    valid = False
                    removed_policies[name] = (
                        "negative AT time '%s'" % at_time)
                    break

                at_seconds_truncated = truncate_to_granularity(
                    at_seconds, self.granularity)
                if at_seconds != at_seconds_truncated:
                    if self.strict:
                        valid = False
                        removed_policies[name] = (
                            "AT time '%s' must be multiples of '%s' seconds" %
                            (at_time, self.granularity))
                        break
                    else:
                        notable_policies[name] = (
                            "AT time '%s' truncated to '%s' seconds" %
                            (at_time, self.granularity))

                rule.atSeconds = at_seconds
                rule.atSecondsTruncated = at_seconds_truncated
            if valid:
                results[name] = rules

        logging.info('Removed %s rule policies with invalid atTime' %
                     len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        self.all_notable_policies.update(notable_policies)
        return results

    def _create_rules_with_expanded_delta_offset(self, rules_map):
        """ Create 'deltaSeconds' and 'deltaSecondsTruncated' from
        rule.deltaOffset.
        """
        results = {}
        removed_policies = {}
        notable_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                delta_offset = rule.deltaOffset
                delta_seconds = time_string_to_seconds(delta_offset)
                if delta_seconds == INVALID_SECONDS:
                    valid = False
                    removed_policies[name] = (
                        "invalid deltaOffset '%s'" % delta_offset)
                    break

                delta_seconds_truncated = truncate_to_granularity(
                    delta_seconds, self.granularity)
                if delta_seconds != delta_seconds_truncated:
                    if self.strict:
                        valid = False
                        removed_policies[name] = (
                            "deltaOffset '%s' must be a multiple of " +
                            "'%s' seconds" % delta_offset, self.granularity)
                        break
                    else:
                        notable_policies[name] = (
                            "deltaOffset '%s' must be a multiple of " +
                            "'%s' seconds" % delta_offset, self.granularity)

                rule.deltaSeconds = delta_seconds
                rule.deltaSecondsTruncated = delta_seconds_truncated
            if valid:
                results[name] = rules

        logging.info('Removed %s rule policies with invalid deltaOffset' %
                     len(removed_policies))
        self._print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        self.all_notable_policies.update(notable_policies)
        return results


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


def parse_on_day_string(on_string):
    """Parse things like "Sun>=1", "lastSun", "20". Mon=1, Sun=7.
    Returns (on_day_of_week, on_day_of_month) where
        (0, dayOfMonth) = exact match on dayOfMonth
        (dayOfWeek, dayOfMonth) = matches dayOfWeek>=dayOfMonth
        (dayOfWeek, 0) = matches lastDayOfWeek
        (0, 0) = error
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

    return (0, 0)


INVALID_SECONDS = 999999  # 277h46m69s


def time_string_to_seconds(time_string):
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


def short_name(name):
    index = name.rfind('/')
    if index >= 0:
        short_name = name[index + 1:]
    else:
        short_name = name
    return short_name


def find_matching_rules(rules, era_from, era_until):
    """Return the rules which overlap with the Zone Era interval [eraFrom,
    eraUntil). The Rule interval is [ruleFrom, ruleTo + 1). Overlap happens
    (ruleFrom < eraUntil) && (eraFrom < ruleTo+1). The expression (eraFrom <
    ruleTo+1) can be written as (eraFrom <= ruleTo) since these values are
    integers.
    """
    matches = []
    for rule in rules:
        if rule.fromYear < era_until and era_from <= rule.toYear:
            matches.append(rule)
    return matches


def find_latest_prior_rules(rules, year):
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
        rule_year = rule.toYear
        rule_month = rule.inMonth
        if rule_year < year:
            rule_date = (rule_year, rule_month)
            if rule_date > candidate_date:
                candidate_date = rule_date
                candidates = [rule]
            elif rule_date == candidate_date:
                candidates.append(rule)
    return candidates


def find_earliest_subsequent_rules(rules, year):
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
    candidate_date = (extractor.MAX_YEAR, 13)
    for rule in rules:
        rule_year = rule.toYear
        rule_month = rule.inMonth
        if rule_year >= year:
            rule_date = (rule_year, rule_month)
            if rule_date < candidate_date:
                candidate_date = rule_date
                candidates = [rule]
            elif rule_date == candidate_date:
                candidates.append(rule)
    return candidates


def is_year_tiny(year):
    """Determine if year fits in an int8_t field. MAX_YEAR(9999) is a marker for
    'max'.
    """
    return year >= 1872 and (year == extractor.MAX_YEAR or year <= 2127)


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


def seconds_to_hms(seconds):
    """Convert seconds to (h,m,s). Works only for positive seconds.
    """
    s = seconds % 60
    minutes = seconds // 60
    m = minutes % 60
    h = minutes // 60
    return (h, m, s)


def hms_to_seconds(h, m, s):
    """Convert h:m:s to seconds.
    """
    return (h * 60 + m) * 60 + s


def div_to_zero(a, b):
    """Integer division (a/b) that truncates towards 0, instead of -infinity as
    is default for Python. Assumes b is positive, but a can be negative or
    positive.
    """
    return a // b if a >= 0 else (a - 1) // b + 1


def truncate_to_granularity(a, b):
    """Truncate a to the granularity of b.
    """
    return b * div_to_zero(a, b)
