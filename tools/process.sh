#!/usr/bin/env bash
#
# Copyright 2018 Brian T. Park
#
# MIT License

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the github repository.
INPUT_DIR=$HOME/dev/tz

# Output directory points to AceTime/src/ace_time/zonedb
OUTPUT_DIR=../src/ace_time/zonedb

function usage() {
    echo 'Usage: process.sh [--code] [--tag tag] [python_flags...]'
    exit 1
}

pass_thru_flags=''
while [[ $# -gt 0 ]]; do
    case $1 in
        --code) output_option="--output_dir $OUTPUT_DIR" ;;
        --tag) shift; tag=$1 ;;
        --help|-h) usage ;;
        -*) break ;;
        *) break ;;
    esac
    shift
done

pushd $INPUT_DIR
git co $tag
popd

$DIRNAME/process.py \
    --input_dir $INPUT_DIR \
    --tz_version $tag \
    $output_option \
    "$@"

pushd $INPUT_DIR
git co master
popd
