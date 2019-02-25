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
                        Transformer
                        /    |   \
                       v     |    v
       ArduinoGenerator      |   PythonGenerator
               /             |        \
              v              |         v
     zone_infos.{h,cpp}      |        zone_infos.py
     zone_policies.{h,cpp}   |        zone_policies.py
                             v                    |
                        InlineGenerator           |
                             |         \          |
                             |          \         |
                             v           v        v
                  TestDataGenerator <--- ZoneSpecifier
                        /        \
                       /          \
                      /            \
                     v              v
ArduinoValidationGenerator        PythonValidationGenerator
            |                                 |
            v                                 v
   validation_data.{h,cpp}           validation_data.py
   validation_tests.cpp
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
from tdgenerator import TestDataGenerator
from arvalgenerator import ArduinoValidationGenerator
from pyvalgenerator import PythonValidationGenerator
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
        help='Retained time values (UNTIL, AT, SAVE, RULES) fields ' +
        'in seconds (default: 900)',
        type=int,
        default=900)
    parser.add_argument(
        '--strict',
        help='Remove zones and rules not aligned at granularity time boundary',
        action='store_true',
        default=False)

    # Printer
    parser.add_argument(
        '--print_zones_short_name',
        help='Print the short zone names',
        action='store_true',
        default=False)

    # Data pipeline selectors
    parser.add_argument(
        '--zonedb', help='Generate ZoneDB files', action='store_true')
    parser.add_argument(
        '--validate',
        help='Validate the zone_infos and zone_policies maps',
        action='store_true')
    parser.add_argument(
        '--unittest', help='Generate Unit Test files', action='store_true')

    # Target language selectors
    parser.add_argument(
        '--python', help='Generate Python files', action='store_true')
    parser.add_argument(
        '--arduino', help='Generate Arduino files', action='store_true')
    parser.add_argument(
        '--arduinox', help='Generate Extended Arduino files',
        action='store_true')

    # File generators
    parser.add_argument(
        '--tz_version', help='Version string of the TZ files', required=True)
    parser.add_argument(
        '--output_dir', help='Location of the output directory')

    # Validator
    parser.add_argument('--zone', help='Name of time zone to validate',
        default='')
    parser.add_argument(
        '--viewing_months',
        help='Number of months to use for calculations (13, 14, 36)',
        type=int, default=14)
    parser.add_argument(
        '--validate_dst_offset',
        help='Validate the DST offset as well as the total UTC offset',
        action="store_true")
    parser.add_argument(
        '--validate_hours',
        help='Validate all 24 hours of a day instead of a single sample hour',
        action="store_true")
    parser.add_argument(
        '--debug_validator',
        help='Enable debug output from Validator',
        action="store_true")
    parser.add_argument(
        '--debug_specifier',
        help='Enable debug output from ZoneSpecifier',
        action="store_true")
    parser.add_argument(
        '--in_place_transitions',
        help='Use in-place Transition array to determine Active Transitions',
        action="store_true")
    parser.add_argument(
        '--optimize_candidates', help='Optimize the candidate transitions',
        action='store_true')

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    # How the script was invoked
    invocation = ' '.join(sys.argv)

    # Select target language
    if args.arduino:
        language = 'arduino'
    elif args.arduinox:
        language = 'arduinox'
    else:
        language = 'python'

    # Extract the TZ files
    logging.info('======== Extracting TZ Data files...')
    extractor = Extractor(args.input_dir)
    extractor.parse()
    extractor.print_summary()
    (zones, rules) = extractor.get_data()

    # Print various slices of the data
    printer = Printer(zones, rules)
    # zones
    if args.print_zones_short_name:
        printer.print_zones_short_name()

    # Transform the TZ zones and rules
    logging.info('======== Transforming Zones and Rules...')
    transformer = Transformer(zones, rules, language, args.start_year,
                              args.granularity, args.strict)
    transformer.transform()
    transformer.print_summary()
    (zones, rules, removed_zones, removed_policies, notable_zones,
     notable_policies) = transformer.get_data()

    # Validate the zone_infos and zone_policies if requested
    logging.info(
        '======== Generating inlined zone_infos and zone_policies...')
    inline_generator = InlineGenerator(zones, rules)
    (zone_infos, zone_policies) = inline_generator.generate_maps()
    logging.info('zone_infos=%d; zone_policies=%d', len(zone_infos),
                 len(zone_policies))

    if args.zonedb:
        # Create the Python or Arduino files if requested
        if not args.output_dir:
            logging.error(
                'Must provide --output_dir to generate Python files')
            sys.exit(1)
        if language == 'python':
            logging.info('======== Creating Python zonedb files...')
            generator = PythonGenerator(
                invocation, args.tz_version, Extractor.ZONE_FILES, zones, rules,
                removed_zones, removed_policies, notable_zones,
                notable_policies)
            generator.generate_files(args.output_dir)
        elif language == 'arduino' or language == 'arduinox':
            extended = (language == 'arduinox')
            logging.info('======== Creating Arduino zonedb files...')
            generator = ArduinoGenerator(
                invocation, args.tz_version, Extractor.ZONE_FILES, zones, rules,
                removed_zones, removed_policies, notable_zones,
                notable_policies, extended)
            generator.generate_files(args.output_dir)
        else:
            raise Exception("Unrecognized language '%s'" % language)
    elif args.unittest:
        logging.info('======== Generating unit test files...')

        # Generate test data for unit test.
        logging.info('Generating test data')
        data_generator = TestDataGenerator(zone_infos, zone_policies)
        (test_data, num_items) = data_generator.create_test_data()
        logging.info('test_data=%d', len(test_data))

        # Generate validation data files
        logging.info('Generating test validation files')
        if language == 'arduino':
            extended = (language == 'arduinox')
            arval_generator = ArduinoValidationGenerator(
                invocation, args.tz_version, test_data, num_items,
                extended)
            arval_generator.generate_files(args.output_dir)
        elif language == 'python':
            pyval_generator = PythonValidationGenerator(
                invocation, args.tz_version, test_data, num_items)
            pyval_generator.generate_files(args.output_dir)
        else:
            raise Exception("Unrecognized language '%s'" % language)
    elif args.validate:
        validator = Validator(
            zone_infos=zone_infos,
            zone_policies=zone_policies,
            viewing_months=args.viewing_months,
            validate_dst_offset=args.validate_dst_offset,
            validate_hours=args.validate_hours,
            debug_validator=args.debug_validator,
            debug_specifier=args.debug_specifier,
            zone_name=args.zone,
            in_place_transitions=args.in_place_transitions,
            optimize_candidates=args.optimize_candidates)

        logging.info('======== Validating transition buffer sizes...')
        validator.validate_transition_buffer_size()

        logging.info('======== Validating test data...')
        validator.validate_sequentially()
    else:
        logging.error('One of (--zonedb, --validate, --unittest) must be given')
        sys.exit(1)

    logging.info('======== Finished processing TZ Data files.')


if __name__ == '__main__':
    main()
