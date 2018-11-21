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

function usage() {
    echo 'Usage: process.sh version'
    exit 1
}


if [[ $# -ne 1 ]]; then
    usage
fi
tz_version=$1
shift

pushd $DB_DIR
git co $tz_version
popd

$DIRNAME/process.py --input_dir $DB_DIR --tz_version $tz_version "$@"

pushd $DB_DIR
git co master
popd
