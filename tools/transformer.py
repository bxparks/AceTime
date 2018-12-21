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

class Transformer:

    def __init__(self, zones_map, rules_map, print_removed):
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.print_removed = print_removed
        self.all_removed_zones = []

    def get_data(self):
        """
        Returns a tuple of 3 data structures:

        'rules_map' is a map of (name -> rules[]), where each element in rules
        is another map with the following fields:

            fromYear: (int) from year
            toYear: (int) to year, 2000 to 9999=max
            inMonth: (int) month index (1-12)
            onDay: (string) 'lastSun' or 'Sun>=2', or 'DD'
            atHour: (string) hour at which to transition to and from DST
            atHourModifier: (char) 's', 'w', 'g', 'u', 'z'
            deltaHour: (string) offset from Standard time
            letter: (char) 'D', 'S', '-'
            rawLine: (string) the original RULE line from the TZ file

            onDayOfWeek: (int) 1=Monday, 7=Sunday, 0={exact dayOfMonth match}
            onDayOfMonth: (int) (1-31), 0={last dayOfWeek match}
            atMinute: (int) atHour in units of minutes from 00:00
            deltaMinute: (int) offset from Standard time in minutes
            deltaCode: (int) offset code (15 min chunks) from Standard time
            shortName: (string) short name of the zone

        'zones_map' is a map of (name -> zones[]), where each element in zones
        is another map with the following fields:

            offsetHour: (string) (GMTOFF field) offset from UTC/GMT
            rules: (string) (RULES field) name of the Rule in effect, '-', or
                    "hh:mm" delta
            format: (string) (FORMAT field) abbreviation with '%s' replaced with
                    '%' (e.g. P%sT -> P%T, E%ST -> E%T, GMT/BST, SAST)
            untilYear: (int) 9999 means 'max'
            untilMonth: (int) 1-12 optional
            untilDay: (string) 1-31, 'lastSun', 'Sun>=3', etc
            untilTime: (string) optional
            rawLine: (string) original ZONE line in TZ file

            offsetMinute: (int) offset from UTC/GMT in minutes
            offsetCode: (int) offset from UTC/GMT in 15-minute units
            rulesDeltaMinute: (int) delta offset from UTC in minutes
            used: (boolean) indicates whether or not the rule is used by a zone

        'all_removed_zones' is a list of the names of zones which were removed
        for one reason or another.
        """
        return (self.zones_map, self.rules_map, self.all_removed_zones)

    def transform(self):
        zones_map = self.zones_map
        rules_map = self.rules_map

        logging.info('Found %s zone infos' % len(self.zones_map))
        logging.info('Found %s rule policies' % len(self.rules_map))

        zones_map = self.remove_zone_entries_too_old(zones_map)
        zones_map = self.create_zones_with_offset_minute(zones_map)
        zones_map = self.create_zones_with_offset_code(zones_map)
        zones_map = self.create_zones_with_rules_expansion(zones_map)
        zones_map = self.remove_zones_with_until_time(zones_map)
        zones_map = self.remove_zones_with_until_day(zones_map)
        zones_map = self.remove_zones_with_until_month(zones_map)
        zones_map = self.remove_zones_with_offset_as_rules(zones_map)
        zones_map = self.remove_zones_without_slash(zones_map)

        (zones_map, rules_map) = self.mark_rules_used_by_zones(
            zones_map, rules_map)
        rules_map = self.remove_rules_unused(rules_map)
        rules_map = self.remove_rules_out_of_bounds(rules_map)

        rules_map = self.create_rules_with_at_minute(rules_map)
        rules_map = self.create_rules_with_delta_minute(rules_map)
        rules_map = self.create_rules_with_delta_code(rules_map)
        rules_map = self.create_rules_with_on_day_expansion(rules_map)
        rules_map = self.remove_rules_long_dst_letter(rules_map)
        rules_map = self.remove_rules_invalid_at_hour(rules_map)

        zones_map = self.remove_zones_without_rules(zones_map, rules_map)

        self.rules_map = rules_map
        self.zones_map = zones_map

        logging.info('Removed %s zone infos' % len(self.all_removed_zones))
        logging.info('Remaining %s zone infos' % len(self.zones_map))
        logging.info('Remaining %s rule policies' % len(self.rules_map))

    @staticmethod
    def remove_zone_entries_too_old(zones_map):
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

    def remove_zones_with_until_time(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if zone['untilTime']:
                    valid = False
                    removed_zones[name] = '%s %s %s %s' % (
                        zone['untilYear'], zone['untilMonth'], zone['untilDay'],
                        zone['untilTime'])
                    break
            if valid:
                results[name] = zones
        logging.info("Removed %s zone infos with unsupported untilTime"
            % len(removed_zones))
        if self.print_removed:
            for name, reason in sorted(removed_zones.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        self.all_removed_zones.extend(removed_zones.keys())
        return results

    def remove_zones_with_until_day(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if zone['untilDay']:
                    valid = False
                    removed_zones[name] = '%s %s %s' % (
                        zone['untilYear'], zone['untilMonth'], zone['untilDay'])
                    break
            if valid:
                results[name] = zones
        logging.info("Removed %s zone infos with unsupported untilDay"
            % len(removed_zones))
        if self.print_removed:
            for name, reason in sorted(removed_zones.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        self.all_removed_zones.extend(removed_zones.keys())
        return results

    def remove_zones_with_until_month(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if zone['untilMonth']:
                    valid = False
                    removed_zones[name] = '%s %s' % (
                        zone['untilYear'], zone['untilMonth'])
                    break
            if valid:
                results[name] = zones
        logging.info("Removed %s zone infos with unsupported untilMonth"
            % len(removed_zones))
        if self.print_removed:
            for name, reason in sorted(removed_zones.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        self.all_removed_zones.extend(removed_zones.keys())
        return results

    def remove_zones_with_offset_as_rules(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if 'rulesDeltaMinute' in zone:
                    valid = False
                    removed_zones[name] = rule_name
                    break
            if valid:
               results[name] = zones
        logging.info("Removed %s zone infos with UTC offset in 'rules' field"
            % len(removed_zones))
        if self.print_removed:
            for name, reason in sorted(removed_zones.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        self.all_removed_zones.extend(removed_zones.keys())
        return results

    def remove_zones_without_slash(self, zones_map):
        results = {}
        removed_zones = {}
        for name, zones in zones_map.items():
            if name.rfind('/') >= 0:
               results[name] = zones
            else:
                removed_zones[name] = 'missing /'
        logging.info("Removed %s zone infos without '/' in name" %
            len(removed_zones))
        if self.print_removed:
            for name, reason in sorted(removed_zones.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        self.all_removed_zones.extend(removed_zones.keys())
        return results

    @staticmethod
    def create_zones_with_offset_minute(zones_map):
        """ Create zone['offsetMinute'] from zone['offsetHour'].
        """
        for name, zones in zones_map.items():
            for zone in zones:
                offset_hour = zone['offsetHour']
                offset_minute = hour_string_to_minute(offset_hour)
                if offset_minute == 9999:
                    logging.error(
                        "Zone %s: could not parse offsetHour (%s)",
                        name , offset_hour)
                    continue
                zone['offsetMinute'] = offset_minute
        return zones_map

    @staticmethod
    def create_zones_with_offset_code(zones_map):
        """ Create zone['offsetCode'] from zone['offsetMinute'].
        """
        for name, zones in zones_map.items():
            for zone in zones:
                offset_minute = zone['offsetMinute']
                if offset_minute % 15 != 0:
                    logging.error(
                        "Zone %s: offset minutes not multiple of 15: %s",
                        name, offset_minute)
                    continue
                offset_code = int(offset_minute / 15)
                zone['offsetCode'] = offset_code
        return zones_map

    @staticmethod
    def create_zones_with_rules_expansion(zones_map):
        """ Create zone['rulesDeltaMinute'] from zone['rules'].

        The RULES field can hold the following:
            * '-' no rules
            * a string reference to a set of Rules
            * a delta offset like "01:00" to be added to the GMTOFF field
                (see America/Argentina/San_Luis, Europe/Istanbul for example).
        """
        for name, zones in zones_map.items():
            for zone in zones:
                rules_string = zone['rules']
                if rules_string.find(':') >= 0:
                    rules_delta_minute = hour_string_to_minute(rules_string)
                    if rules_delta_minute == 9999:
                        logging.error(
                            "Zone %s: could not parse RULES delta string (%s)",
                            name , rules_string)
                        continue
                    zone['rulesDeltaMinute'] = rules_delta_minute
        return zones_map

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
                    removed_zones[name] = 'Rule %s not found' % rule_name
                    break
            if valid:
                results[name] = zones
        logging.info("Removed %s zone infos without rules" % len(removed_zones))
        if self.print_removed:
            for name, reason in sorted(removed_zones.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        self.all_removed_zones.extend(removed_zones.keys())
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
                    removed_policies[name] = letter
                    break
            if valid:
                results[name] = rules
        logging.info('Removed %s rule policies with long DST letter' %
            len(removed_policies))
        if self.print_removed:
            for name, reason in sorted(removed_policies.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        return results

    def remove_rules_invalid_at_hour(self, rules_map):
        """Remove rules whose atHour occurs off hour.
        """
        results = {}
        removed_policies = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                at_minute = rule['atMinute']
                if  at_minute % 60 != 0:
                    valid = False
                    removed_policies[name] = rule['atHour']
                    break
            if valid:
                results[name] = rules
        logging.info('Removed %s rule policies with non-integral atHour'
            % len(removed_policies))
        if self.print_removed:
            for name, reason in sorted(removed_policies.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        return results

    @staticmethod
    def mark_rules_used_by_zones(zones_map, rules_map):
        """Mark all rules which are required by various zones. There are 2 ways
        that a rule can be used by a zone entry:
        1) The rule's fromYear and toYear are >= 2000, or
        2) The rule is the most recent transition that happened before year
        2000.
        """
        for name, zone_entries in zones_map.items():
            begin_year = 2000
            for zone_entry in zone_entries:
                rule_name = zone_entry['rules']
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
                until_year = zone_entry['untilYear']
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
        if self.print_removed:
            for name, reason in sorted(removed_policies.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
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
                    removed_policies[name] = (
                        "fromYear (%s) or toYear (%s) out of bounds" %
                        (from_year, to_year))
                    valid = False
                    break;
            if valid:
                results[name] = rules
        logging.info(
            'Removed %s rule policies with fromYear or toYear out of bounds' %
            len(removed_policies))
        if self.print_removed:
            for name, reason in sorted(removed_policies.items()):
                print('  %s (%s)' % (name, reason), file=sys.stderr)
        return results

    @staticmethod
    def create_rules_with_on_day_expansion(rules_map):
        """ Create rule['onDayOfWeek'] and rule['onDayOfMonth']
            from rule['onDay'].
        """
        for name, rules in rules_map.items():
            for rule in rules:
                on_day = rule['onDay']
                (on_day_of_week, on_day_of_month) = parse_on_day_string(on_day)
                if (on_day_of_week, on_day_of_month) == (0, 0):
                    logging.error(
                        "Rule %s: could not parse onDay (%s)", name, on_day)
                    continue
                rule['onDayOfWeek'] = on_day_of_week
                rule['onDayOfMonth'] = on_day_of_month
        return rules_map

    @staticmethod
    def create_rules_with_at_minute(rules_map):
        """ Create rule['atMinute'] from rule['atHour'].
        """
        for name, rules in rules_map.items():
            for rule in rules:
                at_hour = rule['atHour']
                at_minute = hour_string_to_minute(at_hour)
                if at_minute == 9999:
                    logging.error(
                        "Rule %s: could not parse atHour (%s)", name, at_hour)
                    continue
                rule['atMinute'] = at_minute
        return rules_map

    @staticmethod
    def create_rules_with_delta_minute(rules_map):
        """ Create rule['deltaMinute'] from rule['deltaHour'].
        """
        for name, rules in rules_map.items():
            for rule in rules:
                delta_hour = rule['deltaHour']
                delta_minute = hour_string_to_minute(delta_hour)
                if delta_minute == 9999:
                    logging.error(
                        "Rule %s: could not parse deltaHour (%s)",
                        name, delta_hour)
                    continue
                rule['deltaMinute'] = delta_minute
        return rules_map

    @staticmethod
    def create_rules_with_delta_code(rules_map):
        """ Create rule['deltaCode'] from rule['deltaMinute'].
        """
        for name, rules in rules_map.items():
            for rule in rules:
                delta_minute = rule['deltaMinute']
                if delta_minute % 15 != 0:
                    logging.error(
                        "Rule %s: deltaMinute not multiple of 15: %s"
                        % (name, delta_minute))
                    continue
                delta_code = int(delta_minute / 15)
                rule['deltaCode'] = delta_code
        return rules_map


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
