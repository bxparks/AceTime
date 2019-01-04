#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Main driver for TZ Database compiler. The data processing pipeline looks like
this:

                  TZDB files
                      |
                      v
                  Extractor --> Printer
                      |
                      v
                 Transformer --> Printer
                 /    |   \
                v     |    v
ArduinoGenerator      |   PythonGenerator
        /             |        \
       v              |         v
zone_infos.{h,cpp}    |        zone_infos.py
zone_policies.{h,cpp} |        zone_policies.py
                      |            |
                      |            v
                      |        zone_agent.py
                      v
               InlineGenerator
                      |
                      |
                      v
                  Validator
"""

import argparse
import logging
import sys

from printer import Printer
from extractor import Extractor
from transformer import Transformer
from argenerator import ArduinoGenerator
from pygenerator import PythonGenerator
from ingenerator import InlineGenerator
from validator import Validator

def main():
    """Read the test data chunks from the STDIN and print them out. The ability
    to run this from the command line is intended mostly for testing purposes.

    Usage:
        tzcompiler.py [flags...]
    """
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Zone Info.')

    # Extractor
    parser.add_argument(
        '--input_dir', help='Location of the input directory', required=True)

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
        action='store_true',
        default=False)

    # Printer
    parser.add_argument(
        '--print_rules',
        help='Print list of rules',
        action='store_true',
        default=False)
    parser.add_argument(
        '--print_zones',
        help='Print list of zones',
        action='store_true',
        default=False)
    parser.add_argument(
        '--print_zones_short_name',
        help='Print the short zone names',
        action='store_true',
        default=False)
    parser.add_argument(
        '--print_transformed_zones',
        help='Print transformed zones',
        action='store_true',
        default=False)
    parser.add_argument(
        '--print_transformed_rules',
        help='Print transformed rules',
        action='store_true',
        default=False)

    # File generators
    parser.add_argument(
        '--python', help='Generate Python files', action='store_true')
    parser.add_argument(
        '--arduino', help='Generate Arduino files', action='store_true')
    parser.add_argument(
        '--tz_version', help='Version string of the TZ files', required=True)
    parser.add_argument(
        '--output_dir', help='Location of the output directory')

    # Validator
    parser.add_argument(
        '--validate',
        help='Validate the zone_infos and zone_policies maps',
        action='store_true',
        default=False)
    parser.add_argument('--optimized', help='Optimize the year interval',
        action="store_true")
    parser.add_argument('--validate_dst_offset',
        help='Validate the DST offset as well as the total UTC offset',
        action="store_true")
    parser.add_argument('--validate_hours',
        help='Validate all 24 hours of a day instead of a single sample hour',
        action="store_true")

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    # How the script was invoked
    invocation = ' '.join(sys.argv)

    # Extract the TZ files
    logging.info('======== Extracting TZ Data files...')
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

    # Transform the TZ zones and rules
    logging.info('======== Transforming Zones and Rules...')
    transformer = Transformer(zones, rules, args.python, args.start_year,
        args.granularity, args.strict)
    transformer.transform()
    transformer.print_summary()
    (zones, rules, removed_zones, removed_policies, notable_zones,
        notable_policies) = transformer.get_data()

    # Printer for the transformer
    printer = Printer(zones, rules)

    # Print the transformed data
    if args.print_transformed_zones:
        printer.print_zones()
    if args.print_transformed_rules:
        printer.print_rules()

    # Create the Python or Arduino files if requested
    if args.python:
        logging.info('======== Creating Python zonedb files...')
        generator = PythonGenerator(invocation, args.tz_version,
            Extractor.ZONE_FILES, zones, rules, removed_zones, removed_policies,
            notable_zones, notable_policies)
        if not args.output_dir:
            logging.error('Must provide --output_dir to generate Python files')
            sys.exit(1)
        generator.generate_files(args.output_dir)
    if args.arduino:
        logging.info('======== Creating Arduino zonedb files...')
        generator = ArduinoGenerator(invocation, args.tz_version,
            Extractor.ZONE_FILES, zones, rules, removed_zones, removed_policies,
            notable_zones, notable_policies)
        if not args.output_dir:
            logging.error('Must provide --output_dir to generate Arduino files')
            sys.exit(1)
        generator.generate_files(args.output_dir)

    # Validate the zone_infos and zone_policies if requested
    if args.validate:
        logging.info(
            '======== Generating inlined zone_infos and zone_policies...')
        inline_generator = InlineGenerator(zones, rules)
        (zone_infos, zone_policies) = inline_generator.generate_maps()
        logging.info('zone_infos=%d; zone_policies=%d', len(zone_infos),
            len(zone_policies))

        validator = Validator(zone_infos, zone_policies, args.optimized,
            args.validate_dst_offset, args.validate_hours)

        logging.info('======== Validating transition buffer sizes...')
        validator.validate_transition_buffer_size()

        logging.info('======== Validating DST transitions...')
        validator.validate_dst_transitions()

    logging.info('======== Finished processing TZ Data files.')

if __name__ == '__main__':
    main()
