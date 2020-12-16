#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.

"""
Read the raw TZ Database files at the location specified by `--input_dir` and
generate the zonedb files in various formats as determined by the '--action'
flag:

  * --action tzdb
      Generate file named 'tzdb.json' which is a JSON representation of the
      internal data structures converted from the raw TZ Database files.
  * --action zonedb
      Generate zone_infos.*, zone_policies.*, and sometimes the zone_registry.*
      files in the target language given by `--language` flag.
  * --action zonelist
      Write just the raw list of zone names named 'zones.txt'.

The `--output_dir` flag determines the directory where various files should
be created. If empty, it means the same as $PWD.

If `--action zonedb` is selected, there are 2 language options available
using the --language flag:

  * --language arduino
  * --language python

The raw TZ Database are parsed by extractor.py and processed by transformer.py.
The Transformer class accepts a number of options:

  * --scope {basic | extended)
  * --start_year {start}
  * --until_year {until}
  * --granularity {seconds}
  * --until_at_granularity {seconds}
  * --offset_granularity {seconds}
  * --strict

which determine which Rules or Zones are retained during the 'transformation'
process.

If `--language arduino` is selected, the following flags are used:

  * --db_namespace {db_namespace}
      Use the given identifier as the C++ namespace of the generated classes.

Examples:

    See tzcompiler.sh
"""

import argparse
import logging
import sys
from typing_extensions import Protocol
from tzdb.extractor import Extractor, ZonesMap, RulesMap
from tzdb.transformer import Transformer
from tzdb.tzdbcollector import TzDbCollector, TzDb
from zonedb.argenerator import ArduinoGenerator
from zonedb.pygenerator import PythonGenerator
from zonedb.ingenerator import InlineGenerator
from zonedb.zonelistgenerator import ZoneListGenerator
from zonedb.bufestimator import BufSizeEstimator, BufSizeInfo


# The value of `ExtendedZoneProcessor.kMaxTransitions` which determines the
# buffer size in the TransitionStorage class. The value of
# BufSizeInfo['max_buf_size'] calculated by BufSizeEstimator must be equal or
# smaller than this constant.
EXTENDED_ZONE_PROCESSOR_MAX_TRANSITIONS = 8


class Generator(Protocol):
    def generate_files(self, name: str) -> None:
        ...


def generate_zonedb(
    invocation: str,
    db_namespace: str,
    language: str,
    output_dir: str,
    tzdb: TzDb,
) -> None:
    """Generate the zonedb/ or zonedbx/ files for Python or Arduino,
    but probably mostly for Arduino.
    """
    logging.info('======== Generating zonedb files')
    generator: Generator

    # Create the Python or Arduino files as requested
    if language == 'python':
        logging.info('==== Creating Python zonedb files')
        generator = PythonGenerator(
            invocation=invocation,
            tzdb=tzdb,
        )
        generator.generate_files(output_dir)
    elif language == 'arduino':
        logging.info('==== Creating Arduino zonedb files')

        # Determine zonedb C++ namespace
        # TODO: Maybe move this into ArduinoGenerator?
        if not db_namespace:
            if tzdb['scope'] == 'basic':
                db_namespace = 'zonedb'
            elif tzdb['scope'] == 'extended':
                db_namespace = 'zonedbx'
            else:
                raise Exception(
                    f"db_namespace cannot be determined for "
                    f"scope '{tzdb['scope']}'"
                )

        generator = ArduinoGenerator(
            invocation=invocation,
            db_namespace=db_namespace,
            tzdb=tzdb,
        )
        generator.generate_files(output_dir)
    else:
        raise Exception(f"Unrecognized language '{language}'")


def estimate_buf_size(
    zones_map: ZonesMap,
    rules_map: RulesMap,
    start_year: int,
    until_year: int,
    ignore_buf_size_too_large: bool,
) -> BufSizeInfo:
    """Estimate the buf_size_info using InlineGenerator and BufSizeEstimator.
    """
    # Generate internal versions of zone_infos and zone_policies
    # so that ZoneSpecifier can be created.
    logging.info('==== Generating inlined zone_infos and zone_policies')
    inline_generator = InlineGenerator(zones_map, rules_map)
    (zone_infos, zone_policies) = inline_generator.generate_maps()
    logging.info(
        'zone_infos=%d; zone_policies=%d',
        len(zone_infos), len(zone_policies))

    # Generate the buf_size estimates for each zone, between start_year and
    # until_year.
    logging.info('==== Estimating transition buffer sizes')
    logging.info('Checking years in [%d, %d)', start_year, until_year)
    estimator = BufSizeEstimator(
        zone_infos=zone_infos,
        zone_policies=zone_policies,
        start_year=start_year,
        until_year=until_year,
    )
    buf_size_info = estimator.estimate()
    logging.info(
        'Num zones=%d; Max buffer size=%d',
        len(buf_size_info['buf_sizes']),
        buf_size_info['max_buf_size'],
    )
    if buf_size_info['max_buf_size'] \
            > EXTENDED_ZONE_PROCESSOR_MAX_TRANSITIONS:
        msg = (
            f"Max buffer size={buf_size_info['max_buf_size']} "
            f"is larger than ExtendedZoneProcessor.kMaxTransitions="
            f"{EXTENDED_ZONE_PROCESSOR_MAX_TRANSITIONS}"
        )
        if ignore_buf_size_too_large:
            logging.warning(msg)
        else:
            raise Exception(msg)
    return buf_size_info


def main() -> None:
    """
    Main driver for TZ Database compiler which parses the IANA TZ Database files
    located at the --input_dir and generates zoneinfo files and validation
    datasets for unit tests at --output_dir.

    Usage:
        tzcompiler.py [flags...]
    """
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Zone Info.')

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

    # Data pipeline selectors. Comma-separated list.
    # tzdb: generate 'tzdb.json'
    # zonedb: generate zonedb ('zone_infos.*', 'zone_poicies.*') files
    # zonelist: generate 'zones.txt' containing relavant zone names
    parser.add_argument(
        '--action',
        help='Type of target(s) to generate',
        required=True)

    # Language selector (for --action zonedb)
    parser.add_argument(
        '--language',
        choices=['arduino', 'python'],
        help='Target language (arduino|python)',
    )

    # For '--language arduino', the following flags are used.
    #
    # C++ namespace names for '--language arduino'. If not specified, it will
    # automatically be set to 'zonedb' or 'zonedbx' depending on the 'scope'.
    parser.add_argument(
        '--db_namespace',
        help='C++ namespace for the zonedb files (default: zonedb or zonedbx)')

    # The tz_version does not affect any data processing. Its value is
    # copied into the various generated files and usually placed in the
    # comments section to describe the source of the data that generated the
    # various files.
    parser.add_argument(
        '--tz_version',
        help='Version string of the TZ files',
        required=True,
    )

    # Target location of the generated files.
    parser.add_argument(
        '--output_dir',
        help='Location of the output directory',
        default='',
    )

    # Flag to ignore max_buf_size check. Needed on ExtendedHinnantDateTest if we
    # want to test the extended year range from 1974 to 2050, because one of the
    # zones requires a buf_size=9, but ExtendedZoneProcessor only supports 8.
    parser.add_argument(
        '--ignore_buf_size_too_large',
        help='Ignore transition buf size too large',
        action='store_true',
    )

    # Parse the command line arguments
    args = parser.parse_args()

    # Manually parse the comma-separated --action.
    actions = set(args.action.split(','))
    allowed_actions = set(['tzdb', 'zonedb', 'zonelist'])
    if not actions.issubset(allowed_actions):
        print(f'Invalid --action: {actions - allowed_actions}')
        sys.exit(1)

    # Configure logging. This should normally be executed after the
    # parser.parse_args() because it allows us set the logging.level using a
    # flag.
    logging.basicConfig(level=logging.INFO)

    # How the script was invoked
    invocation = ' '.join(sys.argv)

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
        zones_map=zones_map,
        rules_map=rules_map,
        links_map=links_map,
        scope=args.scope,
        start_year=args.start_year,
        until_year=args.until_year,
        until_at_granularity=until_at_granularity,
        offset_granularity=offset_granularity,
        strict=args.strict,
    )
    transformer.transform()
    transformer.print_summary()
    tdata = transformer.get_data()

    # Estimate the buffer size of ExtendedZoneProcessor.TransitionStorage.
    buf_size_info = estimate_buf_size(
        zones_map=tdata.zones_map,
        rules_map=tdata.rules_map,
        start_year=args.start_year,
        until_year=args.until_year,
        ignore_buf_size_too_large=args.ignore_buf_size_too_large,
    )

    # Collect TZ DB data into a single JSON-serializable object.
    tzdb_generator = TzDbCollector(
        tz_version=args.tz_version,
        tz_files=Extractor.ZONE_FILES,
        scope=args.scope,
        start_year=args.start_year,
        until_year=args.until_year,
        until_at_granularity=until_at_granularity,
        offset_granularity=offset_granularity,
        strict=args.strict,
        zones_map=tdata.zones_map,
        links_map=tdata.links_map,
        rules_map=tdata.rules_map,
        removed_zones=tdata.removed_zones,
        removed_links=tdata.removed_links,
        removed_policies=tdata.removed_policies,
        notable_zones=tdata.notable_zones,
        notable_links=tdata.notable_links,
        notable_policies=tdata.notable_policies,
        buf_size_info=buf_size_info,
    )
    tzdb = tzdb_generator.get_data()

    for action in actions:
        if action == 'zonedb':
            generate_zonedb(
                invocation=invocation,
                db_namespace=args.db_namespace,
                language=args.language,
                output_dir=args.output_dir,
                tzdb=tzdb,
            )
        elif action == 'tzdb':
            logging.info('======== Creating JSON zonedb files')
            tzdb_generator.generate_files(args.output_dir)
        elif action == 'zonelist':
            logging.info('======== Creating zones.txt')
            generator = ZoneListGenerator(
                invocation=invocation,
                tzdb=tzdb,
            )
            generator.generate_files(args.output_dir)
        else:
            logging.error(f"Unrecognized action '{action}'")
            sys.exit(1)

    logging.info('======== Finished processing TZ Data files.')


if __name__ == '__main__':
    main()
