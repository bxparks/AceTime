#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Cleanses and transforms the 'zones' and 'rules' data so that it can be used to
generate the code for the static instances ZoneInfo and ZonePolicy classes.
"""

import logging
import sys
import datetime

class Transformer:

    def __init__(self, zones_map, rules_map, python, start_year, granularity):
        """
        Arguments:
            zones_map: map of Zone names to Eras
            rules_map: map of Policy names to Rules
            python: generate zonedb for Python
            start_year: include only years on or after start_year
            granularity: retained AT or UNTIL fields in minutes
        """
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.python = python
        self.start_year = start_year
        self.granularity = granularity

        self.all_removed_zones = {} # map of zone name -> reason
        self.all_removed_policies = {} # map of policy name -> reason

    def get_data(self):
        """
        Returns a tuple of 4 data structures:

        'zones_map' is a map of (name -> zones[]), where each element in zones
        is another map with the following fields:

            offsetString: (string) (GMTOFF field) offset from UTC/GMT
            rules: (string) (RULES field) '-', ':' or the name of the Rule
                policy (if ':', then 'rulesDeltaMinutes' will be defined)
            format: (string) (FORMAT field) abbreviation with '%s' replaced with
                '%' (e.g. P%sT -> P%T, E%ST -> E%T, GMT/BST, SAST)
            untilYear: (int) 9999 means 'max'
            untilMonth: (int or None) 1-12
            untilDay: (string or None) 1-31, 'lastSun', 'Sun>=3'
            untilTime: (string or None) 'hh:mm'
            untilTimeModifier: (string or None) 'w', 's', 'u'
            rawLine: (string) original ZONE line in TZ file

            offsetMinutes: (int) offset from UTC/GMT in minutes
            offsetCode: (int) offset from UTC/GMT in 15-minute units
            rulesDeltaMinutes: (int or None) delta offset from UTC in minutes
                if RULES is DST offset string of the form hh:mm
            untilHour: (int or None) hour part of untilTime
            untilMinute: (int or None) minute part of untilTime
            untilMinutes: (int or None) untilTime converted into total minutes
            used: (boolean) indicates whether or not the rule is used by a zone

        'rules_map' is a map of (name -> rules[]), where each element in rules
        is another map with the following fields:

            fromYear: (int) from year
            toYear: (int) to year, 1 to 9999=max
            inMonth: (int) month index (1-12)
            onDay: (string) 'lastSun' or 'Sun>=2', or 'DD'
            atTime: (string) hour at which to transition to and from DST
            atTimeModifier: (char) 's', 'w', 'u'
            deltaOffset: (string) offset from Standard time
            letter: (char) 'D', 'S', '-'
            rawLine: (string) the original RULE line from the TZ file

            onDayOfWeek: (int) 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
            onDayOfMonth: (int) (1-31), 0={last dayOfWeek match}
            atHour: (int) atTime in integer units of hours from 00:00
            atMinute: (int) atTime in units of minutes from 00:00
            deltaMinutes: (int) offset from Standard time in minutes
            deltaCode: (int) offset code (15 min chunks) from Standard time
            shortName: (string) short name of the zone

        'all_removed_zones' is a map of the zones which were removed:
            name: name of zone removed
            reason: human readable reason

        'all_removed_policies' is a map of the policies (entire set of RULEs)
        which were removed:
            name: name of policy removed
            reason: human readable reason
        """
        return (self.zones_map, self.rules_map, self.all_removed_zones,
            self.all_removed_policies)

    def transform(self):
        zones_map = self.zones_map
        rules_map = self.rules_map

        logging.info('Found %s zone infos' % len(self.zones_map))
        logging.info('Found %s rule policies' % len(self.rules_map))

        zones_map = self.remove_zones_with_duplicate_short_names(zones_map)
        zones_map = self.remove_zones_without_slash(zones_map)
        zones_map = self.remove_zone_eras_too_old(zones_map)
        zones_map = self.create_zones_with_until_day(zones_map)
        zones_map = self.create_zones_with_expanded_until_time(zones_map)
        zones_map = self.remove_zones_invalid_until_time_modifier(zones_map)
        zones_map = self.create_zones_with_offset_minutes(zones_map)
        zones_map = self.create_zones_with_offset_code(zones_map)
        zones_map = self.create_zones_with_rules_expansion(zones_map)
        zones_map = self.remove_zones_with_non_monotonic_until(zones_map)

        #rules_map = self.remove_rules_multiple_transitions_in_month(rules_map)
        (zones_map, rules_map) = self.mark_rules_used_by_zones(
            zones_map, rules_map)
        rules_map = self.remove_rules_unused(rules_map)
        rules_map = self.remove_rules_out_of_bounds(rules_map)

        rules_map = self.create_rules_with_expanded_at_time(rules_map)
        rules_map = self.remove_rules_invalid_at_time_modifier(rules_map)
        rules_map = self.create_rules_with_delta_minute(rules_map)
        rules_map = self.create_rules_with_delta_code(rules_map)
        rules_map = self.create_rules_with_on_day_expansion(rules_map)
        #rules_map = self.remove_rules_with_border_transitions(rules_map)
        if not self.python:
            rules_map = self.remove_rules_long_dst_letter(rules_map)

        zones_map = self.remove_zones_without_rules(zones_map, rules_map)

        self.rules_map = rules_map
        self.zones_map = zones_map

        logging.info('=== Transformer Summary')
        logging.info('Removed %s zone infos' % len(self.all_removed_zones))
        logging.info('Remaining %s zone infos' % len(self.zones_map))
        logging.info('Remaining %s rule policies' % len(self.rules_map))
        logging.info('=== Transformer Summary End')

    def print_removed_map(self, removed_map):
        """Helper routine that prints the removed Zone rules or Zone eras along
        with the reason why it was removed.
        """
        for name, reason in sorted(removed_map.items()):
            print('  %s (%s)' % (name, reason), file=sys.stderr)

    # --------------------------------------------------------------------
    # Methods related to Zones.
    # --------------------------------------------------------------------

    def remove_zones_with_duplicate_short_names(self, zones_map):
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
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def remove_zones_without_slash(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            if name.rfind('/') >= 0:
                results[name] = zones
            else:
                removed_zones[name] = "no '/' in zone name"

        logging.info("Removed %s zone infos without '/' in name" %
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def remove_zone_eras_too_old(self, zones_map):
        """Remove zone eras which are too old, i.e. before self.start_year.
        """
        results = {}
        count = 0
        for name, zones in zones_map.items():
            keep_zones = []
            for zone in zones:
                if zone['untilYear'] >= self.start_year:
                    keep_zones.append(zone)
                else:
                    count += 1
            if keep_zones:
                results[name] = keep_zones

        logging.info("Removed %s zone eras before year %04d",
            count, self.start_year)
        return results

    def create_zones_with_until_day(self, zones_map):
        """Convert zone['untilDay'] from 'lastSun' or 'Sun>=1' to a precise day,
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
                until_day = zone['untilDay']
                if not until_day:
                    continue

                # parse the conditional expression in until_day
                (on_day_of_week, on_day_of_month) = \
                    parse_on_day_string(until_day)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    removed_zones[name] = "invalid untilDay '%s'" % until_day
                    break

                until_year = zone['untilYear']
                until_month = zone['untilMonth']
                until_day = calc_day_of_month(
                    until_year, until_month, on_day_of_week, on_day_of_month)
                zone['untilDay'] = until_day
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid untilDay",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def create_zones_with_expanded_until_time(self, zones_map):
        """ Create 'untilHour', 'untilMinute', and untilMinutes from
        zone['untilTime']. Set to 9999 if any error in the conversion.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                until_time = zone['untilTime']
                if until_time:
                    until_minutes = hour_string_to_minutes(until_time)
                    if until_minutes == 9999:
                        valid = False
                        removed_zones[name] = ("invalid UNTIL time '%s'" %
                            until_time)
                        break

                    until_hour = until_minutes // 60
                    until_minute = until_minutes % 60
                    if not self.python and until_minute != 0:
                        valid = False
                        removed_zones[name] = (
                            "non-integral UNTIL time '%s'" % until_time)
                        break
                    if until_minute % self.granularity != 0:
                        valid = False
                        removed_zones[name] = (
                            "UNTIL time '%s' must be multiples of '%s'"
                            % (until_time, self.granularity))
                        break
                else:
                    until_minutes = None
                    until_hour = None
                    until_minute = None
                zone['untilMinutes'] = until_minutes
                zone['untilHour'] = until_hour
                zone['untilMinute'] = until_minute
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid UNTIL time",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def remove_zones_invalid_until_time_modifier(self, zones_map):
        """Remove zones whose UNTIL time contains an unsupported modifier.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                modifier = zone['untilTimeModifier']
                modifier = modifier if modifier else 'w'
                zone['untilTimeModifier'] = modifier
                if modifier not in ['w', 's', 'u']:
                    # 'g' and 'z' is the same as 'u' and does not currently
                    # appear in any TZ file, so let's catch it because it
                    # could indicate a bug
                    valid = False
                    removed_zones[name] = (
                        "unsupported UNTIL time modifier '%s'" % modifier)
                    break
            if valid:
                results[name] = zones

        logging.info(
            "Removed %s zone infos with unsupported UNTIL time modifier",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_policies.update(removed_zones)
        return results

    def create_zones_with_offset_minutes(self, zones_map):
        """ Create zone['offsetMinutes'] from zone['offsetString'].
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                offset_string = zone['offsetString']
                offset_minutes = hour_string_to_minutes(offset_string)
                if offset_minutes == 9999:
                    valid = False
                    removed_zones[name] = ("invalid GMTOFF offset string '%s'" %
                        offset_string)
                    break
                zone['offsetMinutes'] = offset_minutes
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid offsetString",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def create_zones_with_offset_code(self, zones_map):
        """ Create zone['offsetCode'] from zone['offsetMinutes'].
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                offset_minutes = zone['offsetMinutes']
                if offset_minutes % 15 != 0:
                    valid = False
                    removed_zones[name] = (
                        "offsetMinutes '%s' not divisible by 15" %
                        offset_minutes)
                    break
                offset_code = int(offset_minutes / 15)
                zone['offsetCode'] = offset_code
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid offsetMinutes",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def create_zones_with_rules_expansion(self, zones_map):
        """ Create zone['rulesDeltaMinutes'] from zone['rules'].

        The RULES field can hold the following:
            * '-' no rules
            * a string reference to a set of Rules
            * a delta offset like "01:00" to be added to the GMTOFF field
                (see America/Argentina/San_Luis, Europe/Istanbul for example).
        After this method, the zone['rules'] contains 3 possible values:
            * '-' no rules, or
            * ':' which indicates that 'rulesDeltaMinutes' is defined, or
            * a string reference
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rules_string = zone['rules']
                if rules_string.find(':') >= 0:
                    if not self.python:
                        valid = False
                        removed_zones[name] = (
                            "offset in RULES '%s'" % rules_string)
                        break

                    rules_delta_minutes = hour_string_to_minutes(
                        rules_string)
                    if rules_delta_minutes == 9999:
                        valid = False
                        removed_zones[name] = ("invalid RULES string '%s'" %
                            rules_string)
                        break
                    if rules_delta_minutes % self.granularity != 0:
                        valid = False
                        removed_zones[name] = (
                            "RULES delta offset '%s' must be multiples of '%s'"
                            % (rules_string, self.granularity))
                        break
                    zone['rules'] = ':'
                    zone['rulesDeltaMinutes'] = rules_delta_minutes
                else:
                    zone['rulesDeltaMinutes'] = None
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid RULES",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def remove_zones_without_rules(self, zones_map, rules_map):
        """Remove zone eras whose RULES field contains a reference to
        a set of Rules, which cannot be found.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rule_name = zone['rules']
                if rule_name not in ['-', ':'] and rule_name not in rules_map:
                    valid = False
                    removed_zones[name] = "rule '%s' not found" % rule_name
                    break
            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos without rules" % len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def remove_zones_with_non_monotonic_until(self, zones_map):
        """Remove Zone infos whose UNTIL fields are:
            1) not monotonically increasing, or
            2) does not end in year=9999
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            prev_until = None
            current_until = None
            for zone in zones:
                current_until = (
                    zone['untilYear'],
                    zone['untilMonth'] if zone['untilMonth'] else 0,
                    zone['untilDay'] if zone['untilDay'] else 0,
                    zone['untilHour'] if zone['untilHour'] else 0
                )
                if prev_until:
                    if current_until <= prev_until:
                        valid = False
                        removed_zones[name] = (
                            'non increasing UNTIL: %s %s %s %s' % current_until)
                        break
                prev_until = current_until
            if valid and current_until[0] != 9999:
                valid = False
                removed_zones[name] = ('invalid final UNTIL: %s %s %s %s' %
                    current_until)

            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with invalid UNTIL fields",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    # --------------------------------------------------------------------
    # Methods related to Rules
    # --------------------------------------------------------------------

    def remove_rules_multiple_transitions_in_month(self, rules_map):
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
                    "Found '%s' transitions in year/month '%04d-%02d'" %
                    removals[name])
            else:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with multiple transitions in one month' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def remove_rules_long_dst_letter(self, rules_map):
        """Return a new map which filters out rules with long DST letter.
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                letter = rule['letter']
                if len(letter) > 1:
                    valid = False
                    removed_policies[name] = "LETTER '%s' too long" % letter
                    break
            if valid:
                results[name] = rules

        logging.info('Removed %s rule policies with long DST letter' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def remove_rules_invalid_at_time_modifier(self, rules_map):
        """Remove rules whose atTime contains an unsupported modifier.
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                modifier = rule['atTimeModifier']
                modifier = modifier if modifier else 'w'
                rule['atTimeModifier'] = modifier
                if modifier not in ['w', 's', 'u']:
                    # 'g' and 'z' is the same as 'u' and does not currently
                    # appear in any TZ file, so let's catch it because it
                    # could indicate a bug
                    valid = False
                    removed_policies[name] = (
                        "unsupported AT time modifier '%s'" % modifier)
                    break
            if valid:
                results[name] = rules

        logging.info("Removed %s rule policies with unsupported AT modifier"
            % len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def mark_rules_used_by_zones(self, zones_map, rules_map):
        """Mark all rules which are required by various zones. There are 2 ways
        that a rule can be used by a zone era:
            1) The rule's fromYear or toYear are >= self.start_year, or
            2) The rule is the most recent transition that happened before
            self.start_year.
        """
        for zone_name, eras in zones_map.items():
            begin_year = self.start_year
            for era in eras:
                policy_name = era['rules']
                if policy_name in ['-', ':']:
                    continue

                rules = rules_map.get(policy_name)
                if not rules:
                    logging.error("Zone '%s': Could not find policy '%s': "
                        + "should not happen",
                        zone_name, policy_name)
                    sys.exit(1)

                # Make all Rules which overlap with the current Zone Era.
                # Some Zone Era have an until_month, until_day and until_time
                # components. To be conservative, we need to expand the
                # until_year to the following year, so the effective zone era
                # interval becomes [begin_year, until_year+1).
                until_year = era['untilYear']
                matching_rules = find_matching_rules(
                    rules, begin_year, until_year + 1)
                for rule in matching_rules:
                    rule['used'] = True

                # Find latest Rules just prior to the begin_year.
                # Result: It looks like all of these prior rules are
                # already picked up by previous calls to find_matching_rules().
                prior_rules = find_latest_prior_rules(rules, begin_year)
                for rule in prior_rules:
                    rule['used'] = True

                # Find earliest Rules subsequent to the until_year mark.
                # Result: It looks like all of these prior rules are
                # already picked up by previous calls to find_matching_rules().
                subsequent_rules = find_earliest_subsequent_rules(
                    rules, until_year + 1)
                for rule in subsequent_rules:
                    rule['used'] = True

                begin_year = until_year

        return (zones_map, rules_map)

    def remove_rules_unused(self, rules_map):
        """Remove RULE entries which have not been marked as used by the
        mark_rules_used_by_zones() method. It is expected that all remaining
        RULE entries have FROM and TO fields which is greater than 1872 (the
        earliest year which can be represented by an int8_t toYearShort field,
        2000-128=1872). See also remove_rules_out_of_bounds().
        """
        results = {}
        removed_rule_count = 0
        removed_policies = {}
        for name, rules in rules_map.items():
            used_rules = []
            for rule in rules:
                if 'used' in rule:
                    used_rules.append(rule)
                else:
                    removed_rule_count += 1
            if used_rules:
                results[name] = used_rules
            else:
                removed_policies[name] = 'unused'

        logging.info('Removed %s rule policies (%s rules) not used' %
                (len(removed_policies), removed_rule_count))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def remove_rules_out_of_bounds(self, rules_map):
        """Remove policies which have FROM and TO fields do not fit in an
        int8_t. In other words, y < 1872 or (y > 2127 and y != 9999).
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                from_year = rule['fromYear']
                to_year = rule['toYear']
                if not is_year_short(from_year) or not is_year_short(from_year):
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
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def create_rules_with_on_day_expansion(self, rules_map):
        """ Create rule['onDayOfWeek'] and rule['onDayOfMonth']
            from rule['onDay'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                on_day = rule['onDay']
                (on_day_of_week, on_day_of_month) = parse_on_day_string(on_day)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    valid = False
                    removed_policies[name] = ("invalid onDay '%s'" % on_day)
                    break
                rule['onDayOfWeek'] = on_day_of_week
                rule['onDayOfMonth'] = on_day_of_month
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid onDay' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def remove_rules_with_border_transitions(self, rules_map):
        """Remove rules where the transition occurs near the border of a year
        boundary. In other words, in the last 2 or the first 2 days of a year.

        This routine determines if I can optimize zone_agent.py to consider only
        a one-year interval (instead of a 2-year interval) with just the latest
        prior Rule of the previous year, and the earliest subsequent Rule of the
        next year.

        There are 3 such Rules:
            * Arg (Transition in late year (2007-12-30))
            * Dhaka (Transition in late year (2009-12-31))
            * Ghana (Transition in late year (1920-12-31))
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                from_year = rule['fromYear']
                to_year = rule['toYear']
                month = rule['inMonth']
                on_day_of_week = rule['onDayOfWeek']
                on_day_of_month = rule['onDayOfMonth']
                if month == 1 and on_day_of_month in [1, 2]:
                    valid = False
                    removed_policies[name] = (
                        "Transition in early year (%04d-%02d-%02d)" %
                        (from_year, month, on_day_of_month))
                    break
                elif month == 12 and on_day_of_month in [0, 30, 31]:
                    valid = False
                    removed_policies[name] = (
                        "Transition in late year (%04d-%02d-%02d)" %
                        (from_year, month, on_day_of_month))
                    break
            if valid:
                results[name] = rules

        logging.info("Removed %s rule policies with border Transitions"
            % len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def create_rules_with_expanded_at_time(self, rules_map):
        """ Create 'atMinute' and 'atHour' parameters from rule['atTime'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                at_time = rule['atTime']
                at_minutes = hour_string_to_minutes(at_time)
                if at_minutes == 9999:
                    valid = False
                    removed_policies[name] = ("invalid AT time '%s'" % at_time)
                    break

                at_hour = at_minutes // 60
                at_minute = at_minutes % 60
                if not self.python and at_minute != 0:
                    valid = False
                    removed_policies[name] = (
                        "non-integral AT time '%s'" % at_time)
                    break
                if at_minute % self.granularity != 0:
                    valid = False
                    removed_policies[name] = (
                        "AT time '%s' must be multiples of '%s'" %
                        (at_time, self.granularity))
                    break

                rule['atMinute'] = at_minute
                rule['atHour'] = at_hour
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid atTime' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def create_rules_with_delta_minute(self, rules_map):
        """ Create rule['deltaMinutes'] from rule['deltaOffset'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                delta_offset = rule['deltaOffset']
                delta_minutes = hour_string_to_minutes(delta_offset)
                if delta_minutes == 9999:
                    valid = False
                    removed_policies[name] = ("invalid deltaOffset '%s'" %
                        delta_offset)
                    break
                rule['deltaMinutes'] = delta_minutes
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid deltaOffset' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def create_rules_with_delta_code(self, rules_map):
        """ Create rule['deltaCode'] from rule['deltaMinutes'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                delta_minutes = rule['deltaMinutes']
                if delta_minutes % 15 != 0:
                    valid = False
                    removed_policies[name] = (
                        "deltaMinutes '%s' not multiple of 15" % delta_minutes)
                    break
                delta_code = int(delta_minutes / 15)
                rule['deltaCode'] = delta_code
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid deltaMinutes' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
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


def hour_string_to_minutes(hs):
    """Converts the '+/-hh:mm' string into +/- total minutes from 00:00.
    Returns 9999 if there is a parsing error.
    """
    i = 0
    sign = 1
    if hs[i] == '-':
        sign = -1
        i += 1

    colon_index = hs.find(':')
    if colon_index < 0:
        hour_string = hs[i:]
        minute_string = '0'
    else:
        hour_string = hs[i:colon_index]
        minute_string = hs[colon_index + 1:]
    try:
        hour = int(hour_string)
        # Japan uses 24:00 and 25:00:
        # Rule  NAME    FROM    TO    TYPE  IN  ON      AT  	SAVE    LETTER/S
        # Rule  Japan   1948    only  -     May Sat>=1  24:00   1:00    D
        # Rule  Japan   1948    1951  -     Sep Sat>=8  25:00   0   	S
        # Rule  Japan   1949    only  -     Apr Sat>=1  24:00   1:00    D
        # Rule  Japan   1950    1951  -     May Sat>=1  24:00   1:00    D
        if hour > 25:
            return 9999
        minute = int(minute_string)
        if minute > 59:
            return 9999
        return sign * (hour * 60 + minute)
    except Exception as e:
        return 9999


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
        if rule['fromYear'] < era_until and era_from <= rule['toYear']:
            matches.append(rule)
    return matches


def find_latest_prior_rules(rules, year):
    """Find the most recent prior rules before the given year. The RULE.atTime
    field can be a conditional expression such as 'lastSun' or 'Mon>=8', so it's
    easiest to just compare the (year, month) only. Also, instead of looking for
    the single Rule that is the most recent, we grab all Rules that fit into the
    month bucket. There are 2 reasons:

    1) A handful of Zone Policies have multiple Rules in the same month. From
    remove_rules_multiple_transitions_in_month():

        * Egypt (Found '2' transitions in year/month '2010-09')
        * Palestine (Found '2' transitions in year/month '2011-08')
        * Spain (Found '2' transitions in year/month '1938-04')
        * Tunisia (Found '2' transitions in year/month '1943-04')

    2) A handful of Zone Policies have Rules which specify transitions in the
    last 2 days of the year. From remove_rules_with_border_transitions(), we
    find:
        * Arg (Transition in late year (2007-12-30))
        * Dhaka (Transition in late year (2009-12-31))
        * Ghana (Transition in late year (1920-12-31))

    By grabbing all Rules in the last month, we avoid the risk of accidentally
    leaving some Rules out.
    """
    candidates = []
    candidate_date = (0, 0) # sentinel date earlier than all real Rules
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
    find_matching_rules() of a Zone Era that stretched to YEAR_MAX, but I'm not
    100% sure that that's true, and there might be a weird edge case. This
    method helps prevent that edge case.

    Similar to find_latest_prior_rules(), we match all Rules in a given month,
    instead of looking single earliest Rule.
    """
    candidates = []
    candidate_date = (9999, 13) # sentinel date later than all real Rules
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


def is_year_short(year):
    """Determine if year fits in an int8_t field (i.e. a 'short' year).
    9999 is a marker for 'max'.
    """
    return year >= 1872 and (year == 9999 or year <= 2127)

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
