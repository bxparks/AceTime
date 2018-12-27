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

    def __init__(self, zones_map, rules_map, print_removed):
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.print_removed = print_removed
        self.all_removed_zones = {} # map of zone name -> reason
        self.all_removed_policies = {} # map of policy name -> reason

    def get_data(self):
        """
        Returns a tuple of 4 data structures:

        'zones_map' is a map of (name -> zones[]), where each element in zones
        is another map with the following fields:

            offsetHour: (string) (GMTOFF field) offset from UTC/GMT
            rules: (string) (RULES field) name of the Rule in effect, '-', or
                    "hh:mm" delta
            format: (string) (FORMAT field) abbreviation with '%s' replaced with
                    '%' (e.g. P%sT -> P%T, E%ST -> E%T, GMT/BST, SAST)
            untilYear: (int) 9999 means 'max'
            untilMonth: (int) 1-12 or None
            untilDay: (string) 1-31, 'lastSun', 'Sun>=3', or None
            untilTime: (string) 'hh:mm', or None
            rawLine: (string) original ZONE line in TZ file

            offsetMinute: (int) offset from UTC/GMT in minutes
            offsetCode: (int) offset from UTC/GMT in 15-minute units
            rulesDeltaMinute: (int) delta offset from UTC in minutes
            untilHour: (int) untilTime converted into 0-23
            used: (boolean) indicates whether or not the rule is used by a zone

        'rules_map' is a map of (name -> rules[]), where each element in rules
        is another map with the following fields:

            fromYear: (int) from year
            toYear: (int) to year, 2000 to 9999=max
            inMonth: (int) month index (1-12)
            onDay: (string) 'lastSun' or 'Sun>=2', or 'DD'
            atTime: (string) hour at which to transition to and from DST
            atTimeModifier: (char) 's', 'w', 'u'
            deltaHour: (string) offset from Standard time
            letter: (char) 'D', 'S', '-'
            rawLine: (string) the original RULE line from the TZ file

            onDayOfWeek: (int) 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
            onDayOfMonth: (int) (1-31), 0={last dayOfWeek match}
            atHour: (int) atTime in integer units of hours from 00:00
            atMinute: (int) atTime in units of minutes from 00:00
            deltaMinute: (int) offset from Standard time in minutes
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
        zones_map = self.create_zones_with_until_hour(zones_map)
        zones_map = self.create_zones_with_offset_minute(zones_map)
        zones_map = self.create_zones_with_offset_code(zones_map)
        zones_map = self.create_zones_with_rules_expansion(zones_map)
        zones_map = self.remove_zones_with_offset_as_rules(zones_map)
        zones_map = self.remove_zones_with_non_monotonic_until(zones_map)
        zones_map = self.remove_zones_with_multiple_records_per_year(zones_map)

        (zones_map, rules_map) = self.mark_rules_used_by_zones(
            zones_map, rules_map)
        rules_map = self.remove_rules_unused(rules_map)
        rules_map = self.remove_rules_out_of_bounds(rules_map)

        rules_map = self.create_rules_with_expanded_at_time(rules_map)
        rules_map = self.remove_rules_invalid_at_time_modifier(rules_map)
        rules_map = self.create_rules_with_delta_minute(rules_map)
        rules_map = self.create_rules_with_delta_code(rules_map)
        rules_map = self.create_rules_with_on_day_expansion(rules_map)
        rules_map = self.remove_rules_long_dst_letter(rules_map)

        zones_map = self.remove_zones_without_rules(zones_map, rules_map)

        self.rules_map = rules_map
        self.zones_map = zones_map

        logging.info('=== Summary ===')
        logging.info('Removed %s zone infos' % len(self.all_removed_zones))
        logging.info('Remaining %s zone infos' % len(self.zones_map))
        logging.info('Remaining %s rule policies' % len(self.rules_map))

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

    @staticmethod
    def remove_zone_eras_too_old(zones_map):
        """Remove zone entries which are too old, i.e. before 2000.
        """
        results = {}
        count = 0
        for name, zones in zones_map.items():
            keep_zones = []
            for zone in zones:
                if zone['untilYear'] >= 2000:
                    keep_zones.append(zone)
                else:
                    count += 1
            if keep_zones:
                results[name] = keep_zones

        logging.info("Removed %s zone entries before year 2000" % count)
        return results

    def print_removed_map(self, removed_map):
        if self.print_removed:
            for name, reason in sorted(removed_map.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)

    def remove_zones_with_offset_as_rules(self, zones_map):
        """
        Remove Zones with an offset in the RULES column. This method must be
        called after create_zones_with_rules_expansion() which creates the
        zone['rulesDeltaMinute'] entry.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if 'rulesDeltaMinute' in zone:
                    valid = False
                    removed_zones[name] = "offset in RULES '%s'" % zone['rules']
                    break
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with UTC offset in 'rules' field"
            % len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
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

    def create_zones_with_until_hour(self, zones_map):
        """ Create zone['untilHour'] from zone['untilTime']. Set to 9999 if
        any error in the conversion.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                until_time = zone['untilTime']
                if until_time:
                    until_minute = hour_string_to_minute(until_time)
                    if until_minute == 9999 or until_minute % 60 != 0:
                        valid = False
                        removed_zones[name] = ("invalid untilTime '%s'" %
                            until_time)
                        break
                    until_hour = until_minute // 60
                else:
                    until_hour = None
                zone['untilHour'] = until_hour
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid untilHour",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def create_zones_with_offset_minute(self, zones_map):
        """ Create zone['offsetMinute'] from zone['offsetHour'].
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                offset_hour = zone['offsetHour']
                offset_minute = hour_string_to_minute(offset_hour)
                if offset_minute == 9999:
                    valid = False
                    removed_zones[name] = ("invalid offsetHour '%s'" %
                        offset_hour)
                    break
                zone['offsetMinute'] = offset_minute
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid offsetHour",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def create_zones_with_offset_code(self, zones_map):
        """ Create zone['offsetCode'] from zone['offsetMinute'].
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                offset_minute = zone['offsetMinute']
                if offset_minute % 15 != 0:
                    valid = False
                    removed_zones[name] = (
                        "offsetMinute '%s' not divisible by 15" % offset_minute)
                    break
                offset_code = int(offset_minute / 15)
                zone['offsetCode'] = offset_code
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid offsetMinute",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def create_zones_with_rules_expansion(self, zones_map):
        """ Create zone['rulesDeltaMinute'] from zone['rules'].

        The RULES field can hold the following:
            * '-' no rules
            * a string reference to a set of Rules
            * a delta offset like "01:00" to be added to the GMTOFF field
                (see America/Argentina/San_Luis, Europe/Istanbul for example).
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rules_string = zone['rules']
                if rules_string.find(':') >= 0:
                    rules_delta_minute = hour_string_to_minute(rules_string)
                    if rules_delta_minute == 9999:
                        valid = False
                        removed_zones[name] = ("invalid RULES string '%s'" %
                            rules_string)
                        break
                    zone['rulesDeltaMinute'] = rules_delta_minute
            if valid:
               results[name] = zones

        logging.info("Removed %s zone infos with invalid RULES",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
        return results

    def remove_zones_without_rules(self, zones_map, rules_map):
        """Remove Zone entries whose RULES field contains a reference to
        a set of Rules, which cannot be found.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rule_name = zone['rules']
                if rule_name != '-' and rule_name not in rules_map:
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

    def remove_zones_with_multiple_records_per_year(self, zones_map):
        """Remove Zones with multiple records per year. Currently not supported
        but should be in the future.
        """
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            prev_until_year = None
            current_until_year = None
            for zone in zones:
                current_until_year = zone['untilYear']
                if prev_until_year:
                    if current_until_year == prev_until_year:
                        valid = False
                        removed_zones[name] = (
                            'multiple records for year %s' %
                            current_until_year)
                        break
                prev_until_year = current_until_year
            if valid:
                results[name] = zones

        logging.info("Removed %s zone infos with multiple records per year",
            len(removed_zones))
        self.print_removed_map(removed_zones)
        self.all_removed_zones.update(removed_zones)
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

    @staticmethod
    def mark_rules_used_by_zones(zones_map, rules_map):
        """Mark all rules which are required by various zones. There are 2 ways
        that a rule can be used by a zone entry:
        1) The rule's fromYear and toYear are >= 2000, or
        2) The rule is the most recent transition that happened before year
        2000.
        """
        for name, eras in zones_map.items():
            begin_year = 2000
            for era in eras:
                rule_name = era['rules']
                if rule_name == '-' or rule_name.isdigit():
                    continue

                rule_entries = rules_map.get(rule_name)
                if not rule_entries:
                    logging.error(
                        'Zone name %s: Missing rule: should not happen', name)
                    continue

                # Some Zone entries have an until_month, until_day and
                # until_time. To make sure that we include rules which happen to
                # match the extra fields, let's collect rules which overlap with
                # [begin_year, until_year + 1).
                until_year = era['untilYear']
                matching_rules = find_matching_rules(
                    rule_entries, begin_year, until_year + 1)
                for rule in matching_rules:
                    rule['used'] = True

                # Check if there's a transition rule prior to the first year.
                prior_match = find_most_recent_prior_rule(
                        rule_entries, begin_year)
                if prior_match:
                    prior_match['used'] = True

                begin_year = until_year

        return (zones_map, rules_map)

    def remove_rules_unused(self, rules_map):
        """Remove RULE entries which have not been marked as used by the
        mark_rules_used_by_zones() method. It is expected that all remaining
        RULE entries have FROM and TO fields which is greater than 1872 (the
        earliest year which can be represented by an int8_t toYearShort field,
        2000 - 128).
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

    def create_rules_with_expanded_at_time(self, rules_map):
        """ Create 'atMinute' and 'atHour' parameters from rule['atTime'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                at_time = rule['atTime']
                at_minute = hour_string_to_minute(at_time)
                if at_minute == 9999:
                    valid = False
                    removed_policies[name] = ("invalid AT time '%s'" % at_time)
                    break
                if  at_minute % 60 != 0:
                    valid = False
                    removed_policies[name] = ("non-integral AT time '%s'" %
                        rule['atTime'])
                    break

                rule['atMinute'] = at_minute
                rule['atHour'] = at_minute // 60
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid atTime' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def create_rules_with_delta_minute(self, rules_map):
        """ Create rule['deltaMinute'] from rule['deltaHour'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                delta_hour = rule['deltaHour']
                delta_minute = hour_string_to_minute(delta_hour)
                if delta_minute == 9999:
                    valid = False
                    removed_policies[name] = ("invalid deltaHour '%s'" %
                        delta_hour)
                    break
                rule['deltaMinute'] = delta_minute
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid deltaHour' %
            len(removed_policies))
        self.print_removed_map(removed_policies)
        self.all_removed_policies.update(removed_policies)
        return results

    def create_rules_with_delta_code(self, rules_map):
        """ Create rule['deltaCode'] from rule['deltaMinute'].
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                delta_minute = rule['deltaMinute']
                if delta_minute % 15 != 0:
                    valid = False
                    removed_policies[name] = (
                        "deltaMinute '%s' not multiple of 15" % delta_minute)
                    break
                delta_code = int(delta_minute / 15)
                rule['deltaCode'] = delta_code
            if valid:
                results[name] = rules

        logging.info(
            'Removed %s rule policies with invalid deltaMinute' %
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


def hour_string_to_minute(hs):
    """Converts the '+/-hh:mm' string into +/- minute offset.
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


def find_matching_rules(rule_entries, from_year, until_year):
    """Return the entries in rule_entries which overlap with the interval
    [from_year, until_year) inclusive to exclusive.
    """
    rules = []
    for rule_entry in rule_entries:
        if rule_entry['fromYear'] <= until_year - 1 and \
                from_year <= rule_entry['toYear']:
            rules.append(rule_entry)
    return rules


def find_most_recent_prior_rule(rule_entries, year):
    """Find the most recent prior rule before the given year.
    """
    candidate = None
    for rule_entry in rule_entries:
        if rule_entry['toYear'] < year:
            if not candidate:
                candidate = rule_entry
                continue
            if rule_entry['toYear'] > candidate['toYear']:
                candidate = rule_entry
                continue
            if rule_entry['toYear'] == candidate['toYear'] and \
                    rule_entry['inMonth'] > candidate['inMonth']:
                candidate = rule_entry
    return candidate


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
