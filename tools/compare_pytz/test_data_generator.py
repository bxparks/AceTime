#!/usr/bin/env python3
#
# Copyright 2020 Brian T. Park
#
# MIT License
#
# Generate the validation_data.{h,cpp} and validation_test.cpp files
# given the 'zones.txt' file from the tzcompiler.py.
#
# Usage
# $ ./test_data_generator.py \
#   --scope (basic | extended) \
#   --tz_version {version}
#   [--db_namespace {db}] \
#   [--start_year start] \
#   [--until_year until] \
#   [--format (json|cpp)]
#   < zones.txt
#

import sys
from os.path import (dirname, abspath)

# Insert the parent directory into the sys.path so that this script can pretend
# to be running from the parent diretory and have access to all the python
# modules under the ./tools directory. Python is annoyingly opinionated about
# package path resolution and tries to force us to put this script in the parent
# directory. But due to the integration with non-python binaries in
# 'compare_cpp' and 'compare_java', it makes far more sense for this script to
# be physically living in this directory rather than the parent directory So we
# hack the sys.path. See https://stackoverflow.com/questions/4383571.
sys.path.insert(1, dirname(dirname(abspath(__file__))))  # noqa

import logging
from argparse import ArgumentParser
from typing import List

# Can't use relative import (.tdgenerator) here because PEP 3122 got rejected
# https://mail.python.org/pipermail/python-3000/2007-April/006793.html.
from compare_pytz.tdgenerator import TestDataGenerator
from compare_pytz.jsonvalgenerator import JsonValidationGenerator
from validation.arvalgenerator import ArduinoValidationGenerator


def generate(
    invocation: str,
    db_namespace: str,
    tz_version: str,
    start_year: int,
    until_year: int,
    format: str,
    output_dir: str,
) -> None:
    """Generate the validation_*.* files."""
    zones = read_zones()

    generator = TestDataGenerator(start_year, until_year)
    generator.create_test_data(zones)
    validation_data = generator.get_validation_data()

    if format == 'cpp':
        arval_generator = ArduinoValidationGenerator(
            invocation, tz_version, db_namespace, validation_data)
        arval_generator.generate_files(output_dir)
    else:
        json_generator = JsonValidationGenerator(
            invocation, tz_version, validation_data)
        json_generator.generate_files(output_dir)


def read_zones() -> List[str]:
    """Read the list of zone_names from the sys.stdin."""
    zones: List[str] = []
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        if line.startswith('#'):
            continue
        zones.append(line)
    return zones


def main() -> None:
    parser = ArgumentParser(description='Generate Test Data.')

    # Scope (of the zones in the database):
    # basic: 241 of the simpler time zones for BasicZoneSpecifier
    # extended: all 348 time zones for ExtendedZoneSpecifier
    # TODO: Remove, does not seem to be needed.
    parser.add_argument(
        '--scope',
        help='Size of the generated database (basic|extended)',
        required=True)

    # C++ namespace names for language=arduino. If not specified, it will
    # automatically be set to 'zonedb' or 'zonedbx' depending on the 'scope'.
    parser.add_argument(
        '--db_namespace',
        help='C++ namespace for the zonedb files (default: zonedb or zonedbx)')

    # Options for file generators
    parser.add_argument(
        '--tz_version', help='Version string of the TZ files', required=True)

    parser.add_argument(
        '--start_year',
        help='Start year of validation (default: start_year)',
        type=int,
        default=2000)
    parser.add_argument(
        '--until_year',
        help='Until year of validation (default: 2038)',
        type=int,
        default=2038)

    # Ouput format
    parser.add_argument(
        '--format',
        help='Output format',
        choices=('json', 'cpp'),
        default='cpp',
    )

    # Optional output directory.
    parser.add_argument(
        '--output_dir',
        default='',
        help='Location of the output directory',
    )

    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    invocation = ' '.join(sys.argv)

    generate(
        invocation,
        args.db_namespace,
        args.tz_version,
        args.start_year,
        args.until_year,
        args.format,
        args.output_dir,
    )


if __name__ == '__main__':
    main()
