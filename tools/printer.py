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

    def print_zones_short_name(self):
        """Print the last component in the "a/b/c" zone names. Used to determine
        if the last component is unique. Currently, it seems to be.
        """
        for name, eras in self.zones.items():
            print(short_name(name))
