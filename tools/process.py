#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Main driver for the Extractor, Printer, Transformer, and the Generators.
The data processing pipeline looks like this:

          TZDB files
              |
              v
          Extractor --> Printer
              |
              v
         Transformer --> Printer
           /       \
          v         v
ArduinoGenerator    PythonGenerator
        /               \
       v                 v
zone_infos.{h,cpp}      zone_infos.py
zone_policies.{h,cpp}   zone_policies.py
                             |
                             v
                        zone_agent.py
"""

import argparse
import logging
import sys

from printer import Printer
from extractor import Extractor
from transformer import Transformer
from argenerator import ArduinoGenerator
from pygenerator import PythonGenerator

def main():
    """Read the test data chunks from the STDIN and print them out. The ability
    to run this from the command line is intended mostly for testing purposes.

    Usage:
        process.py [flags...]
    """
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Zone Info.')

    # General
    parser.add_argument(
        '--python',
        help='Generate Python files instead of C++',
        action="store_true",
        default=False)

    # Extractor
    parser.add_argument(
        '--input_dir', help='Location of the input directory', required=True)
    parser.add_argument(
        '--print_summary',
        help='Print summary of rules and zones',
        action="store_true",
        default=False)

    # Transformer
    parser.add_argument(
        '--start_year',
        help='Starting year of Zone Eras (default: 2000)',
        type=int,
        default=2000)
    parser.add_argument(
        '--granularity',
        help='Retained time values (UNTIL, AT, SAVE, RULES) fields '
            + 'in seconds (default: 900)',
        type=int,
        default=900)
    parser.add_argument(
        '--strict',
        help='Remove zones and rules not aligned at granularity time boundary',
        action="store_true",
        default=False)

    # Printer
    parser.add_argument(
        '--print_rules',
        help='Print list of rules',
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
        '--print_transformed_zones',
        help='Print transformed zones',
        action="store_true",
        default=False)
    parser.add_argument(
        '--print_transformed_rules',
        help='Print transformed rules',
        action="store_true",
        default=False)

    # Generators
    parser.add_argument(
        '--tz_version', help='Version string of the TZ files', required=True)
    parser.add_argument(
        '--output_dir', help='Location of the output directory', required=False)

    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    # How the script was invoked
    invocation = ' '.join(sys.argv)

    # Extract the TZ files
    extractor = Extractor(args.input_dir)
    extractor.parse()
    extractor.print_summary()
    (zones, rules) = extractor.get_data()

    # Print various slices of the data
    printer = Printer(zones, rules)
    # rules
    if args.print_rules:
        printer.print_rules()
    # zones
    if args.print_zones:
        printer.print_zones()
    if args.print_zones_short_name:
        printer.print_zones_short_name()

    # Extractor summary
    if args.print_summary:
        extractor.print_summary()

    # Transform the TZ zones and rules
    transformer = Transformer(zones, rules, args.python, args.start_year,
        args.granularity, args.strict)
    transformer.transform()
    (zones, rules, removed_zones, removed_policies, notable_zones,
        notable_policies) = transformer.get_data()

    # printer for the transformer
    printer = Printer(zones, rules)

    # print the transformed data
    if args.print_transformed_zones:
        printer.print_zones()
    if args.print_transformed_rules:
        printer.print_rules()

    # create the generator (Python or C++
    if args.python:
        generator = PythonGenerator(invocation, args.tz_version,
            Extractor.ZONE_FILES, zones, rules, removed_zones, removed_policies,
            notable_zones, notable_policies)
    else:
        generator = ArduinoGenerator(invocation, args.tz_version,
            Extractor.ZONE_FILES, zones, rules, removed_zones, removed_policies,
            notable_zones, notable_policies)

    # generate files
    if args.output_dir:
        generator.generate_files(args.output_dir)

if __name__ == '__main__':
    main()
