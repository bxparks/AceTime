#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Cleanses and transforms the 'zones' and 'rules' data so that it can be used to
generate the code for the static instances ZoneInfo and ZonePolicy classes.
"""

from transformer import find_matching_rules
from transformer import find_most_recent_prior_rule
from transformer import short_name

class Printer:

    def __init__(self, zones, rules):
        self.zones = zones
        self.rules = rules

    def print_rules(self):
        for name, rules in self.rules.items():
            print('Rule name %s' % name)
            for rule in rules:
                print(rule)

    def print_rules_historical(self):
        """Print rules whose fromYear and toYear occur before 2000.
        Some of these are required for zones whose most recent DST transition
        happened a long time ago.
        """
        for name, rules in self.rules.items():
            print('Rule name %s' % name)
            for rule in rules:
                if rule['from'] < 2000 and rule['to'] < 2000:
                    print(rule)

    def print_rules_long_dst_letter(self):
        """Print rules with multiple characters in the DST 'letter' field.
        Currently NOT supported by the ZoneRule class because the
        ZoneRule::letter field is a single uint8_t.
        """
        for name, rules in self.rules.items():
            name_printed = False
            for rule in rules:
                if len(rule['letter']) > 1:
                    if not name_printed:
                        print('Rule name %s' % name)
                        name_printed = True
                    print(rule)

    def print_zones(self):
        for name, zones in self.zones.items():
            print('Zone name %s' % name)
            for zone in zones:
                print(zone)

    def print_zones_short_name(self):
        """Print the last component in the "a/b/c" zone names. Used to determine
        if the last component is unique. Currently, it seems to be.
        """
        for name, zones in self.zones.items():
            print(short_name(name))

    def print_zones_with_until_month(self):
        """Print the zones which have months in the 'UNTIL' field.
        Currently NOT supported by the ZoneEra class which contains only
        a 'untilYear' field.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                if zone['until_month']:
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_without_rules(self):
        """Print zones whose RULES column is "-" which indicates NO rules.
        Supported by the ZoneEra class by setting the zonePolicy field
        to nullptr.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                rule_name = zone['rules']
                if rule_name == '-':
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_with_offset_as_rules(self):
        """Print zones which point to a DST offset in its RULES column.
        There seems to be only 2 zones that does this: Europe/Istanbul and
        America/Argentina/San_Luis. Not sure how to support this. Do we need to
        add another field in ZoneEra, or can we just clobber the
        ZoneEra::offsetCode with this zone offset?
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                rule_name = zone['rules']
                if rule_name.isdigit():
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_with_unknown_rules(self):
        """Print zones whose RULES is a reference that cannot be found.
        There should be NO rule that matches this because it doesn't make sense.
        """
        for name, zones in self.zones.items():
            name_printed = False
            for zone in zones:
                rule_name = zone['rules']
                if rule_name != '-' and not rule_name.isdigit() \
                        and rule_name not in self.rules:
                    if not name_printed:
                        print('Zone name %s' % name)
                        name_printed = True
                    print(zone)

    def print_zones_requiring_historic_rules(self):
        """Print zones which contains a zone rule entry which does not have a
        directly matching transition rule (after the year 2000). This means that
        we need to go back to historical records before year 2000 to find the
        most recent prior transition rule.
        """
        for name, zone_eras in self.zones.items():
            begin_year = 2000
            for zone_era in zone_eras:
                rule_name = zone_era['rules']
                if rule_name == '-' or rule_name.isdigit():
                    continue

                rule_entries = self.rules.get(rule_name)
                if not rule_entries:
                    # This shouldn't happen
                    print('Zone name %s: Missing rule' % name)
                    continue

                # Check if there's exists a transition rule during the
                # [begin_year, until_year) interval.
                until_year = zone_era['until_year']
                if find_matching_rules(rule_entries, begin_year, until_year):
                    begin_year = until_year
                    continue

                # Check if there's a transition rule prior to the first year.
                prior_match = find_most_recent_prior_rule(
                        rule_entries, begin_year)
                if prior_match:
                    begin_year = until_year
                    print('Zone name %s: %s' % (name, zone_era))
                    print('    Matching rule: %s' % prior_match)
                    begin_year = until_year
                    continue

                print('Zone name %s: No matching rule: %s' % (name, zone_era))

                begin_year = until_year

