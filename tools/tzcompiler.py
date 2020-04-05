#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.

import argparse
import logging
import sys
from typing_extensions import Protocol

from extractor import Extractor
from transformer import Transformer
from argenerator import ArduinoGenerator
from jsongenerator import JsonGenerator
from pygenerator import PythonGenerator
from ingenerator import InlineGenerator
from zonelistgenerator import ZoneListGenerator
from validator import Validator
from bufestimator import BufSizeEstimator


class Generator(Protocol):
    def generate_files(self, name: str) -> None:
        ...


if __name__ == '__main__':
    """
    Main driver for TZ Database compiler which parses the IANA TZ Database files
    located at the --input_dir and generates zoneinfo files and validation
    datasets for unit tests at --output_dir.

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
        help='Start year of Zone Eras (default: 2000)',
        type=int,
        default=2000)
    parser.add_argument(
        '--until_year',
        help='Until year of Zone Eras (default: 2038)',
        type=int,
        default=2038)
    parser.add_argument(
        '--granularity',
        help=(
            'Truncate UNTIL, AT, SAVE and RULES fields to '
            + 'this many seconds (default: 60)'
        ),
        type=int)
    parser.add_argument(
        '--until_at_granularity',
        help=(
            'Truncate UNTIL and AT fields to this many seconds '
            + '(default: --granularity)'
        ),
        type=int)
    parser.add_argument(
        '--offset_granularity',
        help=(
            'Truncate SAVE, RULES (offset) fields to this many seconds'
            + '(default: --granularity)'
        ),
        type=int)
    parser.add_argument(
        '--strict',
        help='Remove zones and rules not aligned at granularity time boundary',
        action='store_true',
        default=False)

    # Flags for the TestDataGenerator. If not given (default 0), then
    # the validation_start_year will be set to start_year, and the
    # validation_until_year will be set to until_year.
    #
    # pytz cannot handle dates after the end of 32-bit Unix time_t type
    # (2038-01-19T03:14:07Z), see
    # https://answers.launchpad.net/pytz/+question/262216, so the
    # validation_until_year cannot be greater than 2038.
    parser.add_argument(
        '--validation_start_year',
        help='Start year of ZoneSpecifier validation (default: start_year)',
        type=int,
        default=0)
    parser.add_argument(
        '--validation_until_year',
        help='Until year of ZoneSpecifier validation (default: 2038)',
        type=int,
        default=0)

    # Data pipeline selectors:
    #
    # zonedb: generate zonedb files
    # zonelist: generate zones.txt, list of relavant zones
    # validate: validate both buffer size and validation data
    parser.add_argument(
        '--action',
        help='Data pipeline action selector',
        choices=['zonedb', 'zonelist', 'validate'],
        required=True)

    # Language selector (for --action zonedb)
    parser.add_argument(
        '--language',
        choices=['arduino', 'python', 'json'],
        help='Target language (arduino|python|json)',
    )

    # Scope (of the zones in the database):
    # basic: 241 of the simpler time zones for BasicZoneSpecifier
    # extended: all 348 time zones for ExtendedZoneSpecifier
    parser.add_argument(
        '--scope',
        help='Size of the generated database (basic|extended)',
        required=True)

    # C++ namespace names for language=arduino. If not specified, it will
    # automatically be set to 'zonedb' or 'zonedbx' depending on the 'scope'.
    parser.add_argument(
        '--db_namespace',
        help='C++ namespace for the zonedb files (default: zonedb or zonedbx)')

    # Enable zone_strings.{h,cpp} if requested
    parser.add_argument(
        '--generate_zone_strings',
        help='Generate Arduino zone_strings.{h,cpp} files',
        action='store_true')

    # Options for file generators
    parser.add_argument(
        '--tz_version', help='Version string of the TZ files', required=True)
    parser.add_argument(
        '--output_dir', help='Location of the output directory')

    # Validator
    parser.add_argument('--zone', help='Name of time zone to validate')
    parser.add_argument('--year', help='Year to validate', type=int)
    parser.add_argument(
        '--viewing_months',
        help='Number of months to use for calculations (13, 14, 36)',
        type=int,
        default=14)
    parser.add_argument(
        '--validate_buffer_size',
        help='Validate the transition buffer size',
        action="store_true")
    parser.add_argument(
        '--validate_test_data',
        help='Validate the TestDataGenerator with pytz',
        action="store_true")
    parser.add_argument(
        '--validate_dst_offset',
        help='Validate the DST offset as well as the total UTC offset',
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
        '--optimize_candidates',
        help='Optimize the candidate transitions',
        action='store_true')

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging. This should normally be executed after the
    # parser.parse_args() because it allows us set the logging.level using a
    # flag.
    logging.basicConfig(level=logging.INFO)

    # Set the defaults for validation_start_year and validation_until_year
    # if they were not specified.
    validation_start_year = args.start_year if args.validation_start_year == 0 \
        else args.validation_start_year
    validation_until_year = args.until_year if args.validation_until_year == 0 \
        else args.validation_until_year

    # How the script was invoked
    invocation = ' '.join(sys.argv)

    # Determine zonedb namespace
    if args.db_namespace:
        db_namespace = args.db_namespace
    else:
        db_namespace = ''
        if args.scope == 'basic':
            db_namespace = 'zonedb'
        if args.scope == 'extended':
            db_namespace = 'zonedbx'

    # Define scope-dependent granularity if not overridden by flag
    if args.granularity:
        until_at_granularity = args.granularity
        offset_granularity = args.granularity
    else:
        if args.until_at_granularity:
            until_at_granularity = args.until_at_granularity
        else:
            until_at_granularity = 60

        if args.offset_granularity:
            offset_granularity = args.offset_granularity
        else:
            if args.scope == 'basic':
                offset_granularity = 900
            else:
                offset_granularity = 60

    logging.info('Using UNTIL/AT granularity: %d', until_at_granularity)
    logging.info(
        'Using RULES/SAVE (offset) granularity: %d',
        offset_granularity)

    # Extract the TZ files
    logging.info('======== Extracting TZ Data files')
    extractor = Extractor(args.input_dir)
    extractor.parse()
    extractor.print_summary()

    # Transform the TZ zones and rules
    logging.info('======== Transforming Zones and Rules')
    logging.info('Extracting years [%d, %d)', args.start_year, args.until_year)
    transformer = Transformer(
        extractor.zones_map,
        extractor.rules_map,
        extractor.links_map,
        args.scope,
        args.start_year,
        args.until_year,
        until_at_granularity,
        offset_granularity,
        args.strict,
    )
    transformer.transform()
    transformer.print_summary()

    # Generate internal versions of zone_infos and zone_policies
    # so that ZoneSpecifier can be created.
    logging.info('======== Generating inlined zone_infos and zone_policies')
    inline_generator = InlineGenerator(
        transformer.zones_map, transformer.rules_map)
    (zone_infos, zone_policies) = inline_generator.generate_maps()
    logging.info('zone_infos=%d; zone_policies=%d', len(zone_infos),
                 len(zone_policies))

    # Generate the buf_size estimates for each zone, between start_year and
    # until_year.
    logging.info('======== Estimating transition buffer sizes')
    logging.info('Checking years in [%d, %d)', args.start_year, args.until_year)
    estimator = BufSizeEstimator(
        zone_infos, zone_policies, args.start_year, args.until_year)
    (buf_sizes, max_size) = estimator.estimate()
    logging.info('Num zones=%d; Max buffer size=%d', len(buf_sizes), max_size)

    if args.action == 'zonedb':
        generator: Generator
        # Create the Python or Arduino files if requested
        if not args.output_dir:
            logging.error('Must provide --output_dir to generate zonedb files')
            sys.exit(1)
        if args.language == 'python':
            logging.info('======== Creating Python zonedb files')
            generator = PythonGenerator(
                invocation,
                args.tz_version,
                Extractor.ZONE_FILES,
                transformer.zones_map,
                transformer.rules_map,
                transformer.all_removed_zones,
                transformer.all_removed_policies,
                transformer.all_notable_zones,
                transformer.all_notable_policies)
            generator.generate_files(args.output_dir)
        elif args.language == 'arduino':
            logging.info('======== Creating Arduino zonedb files')
            generator = ArduinoGenerator(
                invocation=invocation,
                tz_version=args.tz_version,
                tz_files=Extractor.ZONE_FILES,
                scope=args.scope,
                db_namespace=db_namespace,
                generate_zone_strings=args.generate_zone_strings,
                start_year=args.start_year,
                until_year=args.until_year,
                zones_map=transformer.zones_map,
                links_map=transformer.links_map,
                rules_map=transformer.rules_map,
                removed_zones=transformer.all_removed_zones,
                removed_links=transformer.all_removed_links,
                removed_policies=transformer.all_removed_policies,
                notable_zones=transformer.all_notable_zones,
                notable_links=transformer.all_notable_links,
                notable_policies=transformer.all_notable_policies,
                format_strings=transformer.format_strings,
                zone_strings=transformer.zone_strings,
                buf_sizes=buf_sizes)
            generator.generate_files(args.output_dir)
        elif args.language == 'json':
            logging.info('======== Creating JSON zonedb files')
            generator = JsonGenerator(
                invocation=invocation,
                tz_version=args.tz_version,
                tz_files=Extractor.ZONE_FILES,
                scope=args.scope,
                db_namespace=db_namespace,
                start_year=args.start_year,
                until_year=args.until_year,
                rules_map=transformer.rules_map,
                zones_map=transformer.zones_map,
                links_map=transformer.links_map,
                removed_zones=transformer.all_removed_zones,
                removed_links=transformer.all_removed_links,
                removed_policies=transformer.all_removed_policies,
                notable_zones=transformer.all_notable_zones,
                notable_links=transformer.all_notable_links,
                notable_policies=transformer.all_notable_policies,
                format_strings=transformer.format_strings,
                zone_strings=transformer.zone_strings,
            )
            generator.generate_files(args.output_dir)
        else:
            raise Exception("Unrecognized language '%s'" % args.language)
    elif args.action == 'zonelist':
        generator = ZoneListGenerator(
            invocation=invocation,
            tz_version=args.tz_version,
            tz_files=Extractor.ZONE_FILES,
            scope=args.scope,
            zones_map=transformer.zones_map)
        generator.generate_files(args.output_dir)
    elif args.action == 'validate':

        # Set the default to set both --validate_buffer_size and
        # --validate_test_data if neither flags are given explicitly.
        validate_buffer_size = False
        validate_test_data = False
        if args.validate_buffer_size:
            validate_buffer_size = True
        if args.validate_test_data:
            validate_test_data = True
        if not validate_buffer_size and not validate_test_data:
            validate_buffer_size = True
            validate_test_data = True

        validator = Validator(
            zone_infos=zone_infos,
            zone_policies=zone_policies,
            viewing_months=args.viewing_months,
            validate_dst_offset=args.validate_dst_offset,
            debug_validator=args.debug_validator,
            debug_specifier=args.debug_specifier,
            zone_name=args.zone,
            year=args.year,
            start_year=validation_start_year,
            until_year=validation_until_year,
            in_place_transitions=args.in_place_transitions,
            optimize_candidates=args.optimize_candidates)

        if validate_buffer_size:
            logging.info('======== Validating transition buffer sizes')
            validator.validate_buffer_size()

        if validate_test_data:
            logging.info('======== Validating test data')
            validator.validate_test_data()
    else:
        logging.error(f"Unrecognized action '{args.action}'")
        sys.exit(1)

    logging.info('======== Finished processing TZ Data files.')
