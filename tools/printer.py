#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Cleanses and transforms the 'zones' and 'rules' data so that it can be used to
generate the code for the static instances ZoneInfo and ZonePolicy classes.
"""

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
