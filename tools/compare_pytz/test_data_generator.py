#!/usr/bin/env python3
#
# Copyright 2020 Brian T. Park
#
# MIT License
#
"""
Generate the 'validation_data.json' containing the validation test data from
the pytz (using .tdgenerator.TestDataGenerator) given the 'zones.txt' file
from the tzcompiler.py.

Usage
$ ./test_data_generator.py \
  --scope (basic | extended) \
  --tz_version {version}
  [--start_year start] \
  [--until_year until] \
  < zones.txt
"""

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


def generate(
    invocation: str,
    tz_version: str,
    scope: str,
    start_year: int,
    until_year: int,
    output_dir: str,
) -> None:
    """Generate the validation_data.json file."""
    zones = read_zones()

    generator = TestDataGenerator(start_year, until_year)
    generator.create_test_data(zones)
    validation_data = generator.get_validation_data()

    json_generator = JsonValidationGenerator(
        invocation=invocation,
        tz_version=tz_version,
        validation_data=validation_data)
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
        invocation=invocation,
        tz_version=args.tz_version,
        scope=args.scope,
        start_year=args.start_year,
        until_year=args.until_year,
        output_dir=args.output_dir,
    )


if __name__ == '__main__':
    main()
