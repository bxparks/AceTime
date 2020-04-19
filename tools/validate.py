#!/usr/bin/env python3
#
# Copyright 2020 Brian T. Park
#
# MIT License.

"""
Parse the IANA TZ Database files located at the --input_dir and validate the
internal zonedb files using the ZoneSpecifier class against pytz.
(Previous version of this was part of tzcompiler.py. Now extracted into
a separate script.)

Usage:
    validate.py [flags...]

Flags:

    The following flags are recognized and passed along into the various helper
    classes:

    Transformer:

        --scope (basic|extended)
        --start_year
        --until_year
        --granularity
        --until_at_granularity
        --offset_granularity
        --strict

    TestDataGenerator:

        --validation_start_year
        --validation_until_year

    ZoneSpecifier:

        --viewing_months
        --debug_specifier
        --in_place_transitions
        --optimize_candidates

    Validator:

        --zone {zone_name}
        --year {year}
        --validate_dst_offset
        --debug_validator

Examples:

    See validate.sh.
"""

import argparse
import logging

from tzdb.extractor import Extractor
from tzdb.transformer import Transformer
from zonedb.ingenerator import InlineGenerator, ZoneInfoMap, ZonePolicyMap
from validation.validator import Validator


def validate(
    zone_infos: ZoneInfoMap,
    zone_policies: ZonePolicyMap,
    zone: str,
    year: int,
    start_year: int,
    until_year: int,
    validate_buffer_size: bool,
    validate_test_data: bool,
    viewing_months: int,
    validate_dst_offset: bool,
    debug_validator: bool,
    debug_specifier: bool,
    in_place_transitions: bool,
    optimize_candidates: bool,
) -> None:

    # Set the default to set both --validate_buffer_size and
    # --validate_test_data if neither flags are given explicitly.
    if not validate_buffer_size and not validate_test_data:
        validate_buffer_size = True
        validate_test_data = True

    validator = Validator(
        zone_infos=zone_infos,
        zone_policies=zone_policies,
        viewing_months=viewing_months,
        validate_dst_offset=validate_dst_offset,
        debug_validator=debug_validator,
        debug_specifier=debug_specifier,
        zone_name=zone,
        year=year,
        start_year=start_year,
        until_year=until_year,
        in_place_transitions=in_place_transitions,
        optimize_candidates=optimize_candidates)

    if validate_buffer_size:
        logging.info('======== Validating transition buffer sizes')
        validator.validate_buffer_size()

    if validate_test_data:
        logging.info('======== Validating test data')
        validator.validate_test_data()


def main() -> None:
    # Configure command line flags.
    parser = argparse.ArgumentParser(
        description='Validate TZ zone files with ZoneSpecifier.'
    )

    # Extractor flags.
    parser.add_argument(
        '--input_dir', help='Location of the input directory', required=True)

    # Transformer flags.
    parser.add_argument(
        '--scope',
        # basic: 241 of the simpler time zones for BasicZoneSpecifier
        # extended: all 348 time zones for ExtendedZoneSpecifier
        choices=['basic', 'extended'],
        help='Size of the generated database (basic|extended)',
        required=True)
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

    # Validator flags.
    parser.add_argument(
        '--zone',
        help='Name of time zone to validate (default: all zones)',
    )
    parser.add_argument(
        '--year',
        help='Year to validate (default: start_year, until_year)',
        type=int,
    )
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
        # Not enabled by default because pytz DST seems to be buggy.
        help='Validate the DST offset as well as the total UTC offset',
        action="store_true")
    parser.add_argument(
        '--debug_validator',
        help='Enable debug output from Validator',
        action="store_true")

    # ZoneSpecifier flags
    parser.add_argument(
        '--viewing_months',
        help='Number of months to use for calculations (13, 14, 36)',
        type=int,
        default=14)
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

    # TestDataGenerator flag.
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

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging. This should normally be executed after the
    # parser.parse_args() because it allows us set the logging.level using a
    # flag.
    logging.basicConfig(level=logging.INFO)

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
    rules_map, zones_map, links_map = extractor.get_data()

    # Transform the TZ zones and rules
    logging.info('======== Transforming Zones and Rules')
    logging.info('Extracting years [%d, %d)', args.start_year, args.until_year)
    transformer = Transformer(
        zones_map,
        rules_map,
        links_map,
        args.scope,
        args.start_year,
        args.until_year,
        until_at_granularity,
        offset_granularity,
        args.strict,
    )
    transformer.transform()
    transformer.print_summary()
    (
        zones_map, rules_map, links_map, removed_zones, removed_policies,
        removed_links, notable_zones, notable_policies, notable_links,
        format_strings, zone_strings,
    ) = transformer.get_data()

    # Generate internal versions of zone_infos and zone_policies
    # so that ZoneSpecifier can be created.
    logging.info('======== Generating inlined zone_infos and zone_policies')
    inline_generator = InlineGenerator(zones_map, rules_map)
    (zone_infos, zone_policies) = inline_generator.generate_maps()
    logging.info('zone_infos=%d; zone_policies=%d', len(zone_infos),
                 len(zone_policies))

    # Set the defaults for validation_start_year and validation_until_year
    # if they were not specified.
    validation_start_year = (
        args.start_year
        if args.validation_start_year == 0
        else args.validation_start_year
    )
    validation_until_year = (
        args.until_year
        if args.validation_until_year == 0
        else args.validation_until_year
    )

    validate(
        zone_infos=zone_infos,
        zone_policies=zone_policies,
        zone=args.zone,
        year=args.year,
        start_year=validation_start_year,
        until_year=validation_until_year,
        validate_buffer_size=args.validate_buffer_size,
        validate_test_data=args.validate_test_data,
        viewing_months=args.viewing_months,
        validate_dst_offset=args.validate_dst_offset,
        debug_validator=args.debug_validator,
        debug_specifier=args.debug_specifier,
        in_place_transitions=args.in_place_transitions,
        optimize_candidates=args.optimize_candidates,
    )

    logging.info('======== Finished processing TZ Data files.')


if __name__ == '__main__':
    main()
