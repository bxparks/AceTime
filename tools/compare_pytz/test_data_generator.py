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
$ ./test_data_generator.py [--start_year start] [--until_year until] \
    [--validate_dst] < zones.txt
"""

import sys
from os.path import (join, dirname, abspath)

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
import json
from argparse import ArgumentParser
from typing import List

# Can't use relative import (.tdgenerator) here because PEP 3122 got rejected
# https://mail.python.org/pipermail/python-3000/2007-April/006793.html.
from compare_pytz.tdgenerator import TestDataGenerator


class Generator:
    def __init__(
        self,
        invocation: str,
        start_year: int,
        until_year: int,
        output_dir: str,
        validate_dst: bool,
    ):
        self.invocation = invocation
        self.start_year = start_year
        self.until_year = until_year
        self.output_dir = output_dir
        self.validate_dst = validate_dst

        self.filename = 'validation_data.json'

    def generate(self) -> None:
        """Generate the validation_data.json file."""

        # Read the zones from the STDIN
        zones = read_zones()

        # Generate the test data set.
        test_generator = TestDataGenerator(
            start_year=self.start_year,
            until_year=self.until_year,
            validate_dst=self.validate_dst,
        )
        test_generator.create_test_data(zones)
        validation_data = test_generator.get_validation_data()

        # Write out the validation_data.json file.
        full_filename = join(self.output_dir, self.filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(validation_data, output_file, indent=2)
            print(file=output_file)  # add terminating newline
        logging.info("Created %s", full_filename)


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
    parser.add_argument(
        '--validate_dst',
        help='Set flag to validate the DST offset',
        action='store_true')

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

    generator = Generator(
        invocation=invocation,
        start_year=args.start_year,
        until_year=args.until_year,
        output_dir=args.output_dir,
        validate_dst=args.validate_dst,
    )
    generator.generate()


if __name__ == '__main__':
    main()
