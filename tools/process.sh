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

cat $DB_DIR/africa \
    $DB_DIR/antarctica \
    $DB_DIR/asia \
    $DB_DIR/australasia \
    $DB_DIR/europe \
    $DB_DIR/northamerica \
    $DB_DIR/southamerica \
        | $DIRNAME/process.py "$@"
