#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Cleanses and transforms the 'zones' and 'rules' data so that it can be used to
generate the code for the static instances ZoneInfo and ZonePolicy classes.
"""

class Transformer:

    def __init__(self, rules, zones):
        self.rules = rules
        self.zones = zones
        self.transformed_rules = {}
        self.transformed_zones = {}

    @staticmethod
    def remove_rules_long_dst_letter(rules_map):
        """Return a new map which filters out rules with long DST letter.
        """
        results = {}
        for name, rules in rules_map.items():
            valid = True
            for rule in rules:
                if len(rule['letter']) > 1:
                    valid = False
                    break
            if valid:
                results[name] = rules
        return results

    @staticmethod
    def remove_zones_with_until_month(zones_map):
        """
        """
        results = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                if zone['until_month']:
                    valid = False
                    break
            if valid:
                results[name] = zones
        return results

    @staticmethod
    def remove_zones_with_offset_as_rules(zones_map):
        results = {}
        for name, zones in zones_map.items():
            valid = True
            for zone in zones:
                rule_name = zone['rules']
                if rule_name.isdigit():
                    valid = False
                    break
            if valid:
               results[name] = zones
        return results

    @staticmethod
    def remove_zone_entries_too_old(zones_map):
        """Remove zone entries which are too old, i.e. before 2000.
        """
        results = {}
        for name, zones in zones_map.items():
            keep_zones = []
            for zone in zones:
                if zone['until_year'] >= 2000:
                    keep_zones.append(zone)
            results[name] = keep_zones
        return results

    @staticmethod
    def mark_rules_used_by_zones(zones_map, rules_map):
        """Mark all rules which are required by various zones. There are 2 ways
        that a rule can be used by a zone entry:
        1) The rule's fromYear and toYear are >= 2000, or
        2) The rule is the most recent transition that happened before year
        2000.
        """
        for name, zone_entries in self.zones.items():
            begin_year = 2000
            for zone_entry in zone_entries:
                rule_name = zone_entry['rules']
                if rule_name == '-' or rule_name.isdigit():
                    continue

                rule_entries = self.rules.get(rule_name)
                if not rule_entries:
                    # This shouldn't happen
                    print('Zone name %s: Missing rule' % name)
                    continue

                # Mark rules in the [begin_year, until_year) interval.
                until_year = zone_entry['until_year']
                matching_rules = find_matching_rules(
                    rule_entries, begin_year, until_year)
                for rule in matching_rules:
                    rule['used'] = True

                # Check if there's a transition rule prior to the first year.
                prior_match = find_most_recent_prior_rule(
                        rule_entries, begin_year)
                if prior_match:
                    prior_match['used'] = True

                begin_year = until_year

    @staticmethod
    def remove_unused_rules(rules_map):
        results = {}
        for name, rules in rules_map.items():
            used_rules = []
            for rule in rules:
                if rule['used']:
                    used_rules.append(rule)
            results[name] = used_rules
        return results

    def transform(self):
        rules = self.remove_rules_long_dst_letter(self.rules)

        zones = self.remove_zones_with_until_month(self.zones)
        zones = self.remove_zones_with_offset_as_rules(zones)
        zones = self.remove_zone_entries_too_old(zones)

        self.mark_rules_used_by_zones(zones, rules)
        rules = self.remove_unused_rules(rules)

        self.transformed_rules = rules
        self.transformed_zones = zones

    def get_data(self):
        return (rules, zones)


def find_matching_rules(rule_entries, from_year, until_year):
    """Return the entries in rule_entries which overlap with the interval
    [from_year, until_year) inclusive to exclusive.
    """
    rules = []
    for rule_entry in rule_entries:
        if rule_entry['from'] <= until_year - 1 and \
                from_year <= rule_entry['to']:
            rules.append(rule_entry)
    return rules


def find_most_recent_prior_rule(rule_entries, year):
    """Find the most recent prior rule before the given year.
    """
    candidate = None
    for rule_entry in rule_entries:
        if rule_entry['to'] < year:
            if not candidate:
                candidate = rule_entry
                continue
            if rule_entry['to'] > candidate['to']:
                candidate = rule_entry
                continue
            if rule_entry['to'] == candidate['to'] and \
                    rule_entry['in_month'] > candidate['in_month']:
                candidate = rule_entry
    return candidate


