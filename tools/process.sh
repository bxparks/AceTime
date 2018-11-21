#!/usr/bin/env bash
#
# Copyright 2018 Brian T. Park
#
# MIT License

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the github repository.
DB_DIR=$HOME/dev/tz

$DIRNAME/process.py --input_dir $DB_DIR "$@"
