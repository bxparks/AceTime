#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Main driver for the Extractor, Printer, and Transformer.

Usage:
    cat africa antarctica asia australasia europe northamerica southamerica \
        | ./processor.py {flags}
"""

import argparse
import logging
import sys

from printer import Printer
from extractor import Extractor

def main():
    """Read the test data chunks from the STDIN and print them out. The ability
    to run this from the command line is intended mostly for testing purposes.

    Usage:
        process.py [flags...] < test_data.txt
    """
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Zone Info.')
    parser.add_argument(
        '--output_dir', help='Location of the output directory', required=False)
    parser.add_argument(
        '--extract', help='Extract the TZ data file',
        action="store_true",
        default=True)
    parser.add_argument(
        '--print_summary',
        help='Print summary of rules and zones',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_rules',
        help='Print list of rules',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_rules_historical',
        help='Print rules which start and end before year 2000',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_rules_long_dst_letter',
        help='Print rules with long DST letter',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones',
        help='Print list of zones',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_short_name',
        help='Print the short zone names',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_with_until_month',
        help='Print the Zones with "UNTIL" month fields',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_with_offset_as_rules',
        help='Print rules which contains a DST offset in the RULES column',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_without_rules',
        help='Print rules which contain "-" in the RULES column',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_with_unknown_rules',
        help='Print rules which contain a valid RULES that cannot be found',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_zones_requiring_historic_rules',
        help='Print rules which require historic transition rules',
        action="store_true",
        default=False)
    args = parser.parse_args()

    # How the script was invoked
    invocation = " ".join(sys.argv)

    # Extract the TZ files
    extractor = Extractor(sys.stdin)
    if args.extract:
        extractor.parse_zone_file()
        extractor.process_rules()
        extractor.process_zones()

    # Extractor summary
    if args.print_summary:
        extractor.print_summary()

    # Print various slices of the data
    (zones, rules) = extractor.get_data()
    printer = Printer(zones, rules)
    # rules
    if args.print_rules:
        printer.print_rules()
    if args.print_rules_historical:
        printer.print_rules_historical()
    if args.print_rules_long_dst_letter:
        printer.print_rules_long_dst_letter()
    # zones
    if args.print_zones:
        printer.print_zones()
    if args.print_zones_short_name:
        printer.print_zones_short_name()
    if args.print_zones_with_until_month:
        printer.print_zones_with_until_month()
    if args.print_zones_with_offset_as_rules:
        printer.print_zones_with_offset_as_rules()
    if args.print_zones_without_rules:
        printer.print_zones_without_rules()
    if args.print_zones_with_unknown_rules:
        printer.print_zones_with_unknown_rules()
    if args.print_zones_with_unknown_rules:
        printer.print_zones_with_unknown_rules()
    if args.print_zones_requiring_historic_rules:
        printer.print_zones_requiring_historic_rules()


if __name__ == '__main__':
    main()
