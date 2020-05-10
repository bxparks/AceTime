#!/usr/bin/env python3
#
# Copyright 2019 Brian T. Park
#
# MIT License

"""
Generate the Arduino validation data (validation_data.h and validation_data.cpp)
files for unit tests from the validation_data.json file on the STDIN.
"""

import argparse
import logging
import sys
import json
from validation.arvalgenerator import ArduinoValidationGenerator


def main() -> None:
    # Configure command line flags.
    parser = argparse.ArgumentParser(
        description='Generate validation_*.{h,cpp} files from JSON'
    )

    # Scope of the extracted TZ database
    parser.add_argument(
        '--scope',
        # basic: time zones for BasicZoneSpecifier
        # extended: time zones for ExtendedZoneSpecifier
        choices=['basic', 'extended'],
        help='Size of the generated database (basic|extended)',
        required=True,
    )

    # The tz_version does not affect any data processing. Its value is
    # copied into the various generated files and usually placed in the
    # comments section to describe the source of the data that generated the
    # various files.
    parser.add_argument(
        '--tz_version',
        help='Version string of the TZ files',
        required=True,
    )

    # C++ namespace and directory name where the zonedb zoneinfo files are
    # located.
    parser.add_argument(
        '--db_namespace',
        help='C++ namespace for the zonedb files (default: zonedb or zonedbx)',
        required=True,
    )

    # Target location of the generated files.
    parser.add_argument(
        '--output_dir',
        help='Location of the output directory',
        default='',
    )

    # DST blacklist JSON file.
    parser.add_argument(
        '--blacklist',
        type=str,
    )

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging. This should normally be executed after the
    # parser.parse_args() because it allows us set the logging.level using a
    # flag.
    logging.basicConfig(level=logging.INFO)

    # How the script was invoked
    invocation = ' '.join(sys.argv)

    # Read the JSON on the STDIN
    validation_data = json.load(sys.stdin)

    # Read the DST blacklist file if given.
    if args.blacklist:
        with open(args.blacklist) as f:
            blacklist = json.load(f)
    else:
        blacklist = {}

    # Generate the validation_*.{h, cpp} files
    generator = ArduinoValidationGenerator(
        invocation=invocation,
        tz_version=args.tz_version,
        scope=args.scope,
        db_namespace=args.db_namespace,
        validation_data=validation_data,
        blacklist=blacklist,
    )
    generator.generate_files(args.output_dir)


if __name__ == '__main__':
    main()
