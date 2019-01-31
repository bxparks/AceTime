#!/usr/bin/env bash
#
# Copyright 2018 Brian T. Park
#
# MIT License
#
# Usage:
#
#   $ tzcompiler.sh --tag {tag} (--zonedb|--validate|--unittest)
#       [--python|--arduino | --arduinox] [other flags...]
#
# Examples:
#
#   $ tzcompiler.sh --tag 2018i --zonedb --python
#       - generates zone*.py files in the current directory
#
#   $ tzcompiler.sh --tag 2018i --zonedb --arduino
#       - generates zone*.{h,cpp} files in the current directory
#
#   $ tzcompiler.sh --tag 2018i --zonedb --arduinox
#       - generates extended zone*.{h,cpp} files in the current directory
#
#   $ tzcompiler.sh --tag 2018i --unittest --arduino
#       - generates test data for ValidationTest unit test
#
#   $ tzcompiler.sh --tag 2018i --validate --python
#       - validate the internal zone_info and zone_policies data
#
#   $ tzcompiler.sh --tag 2018i --validate --arduino
#       - validate the internal zone_info and zone_policies data
#
# Flags
#
#   Transformer flags:
#
#       --start_year
#           Retain TZ information since this year (default 2000).
#       --granularity
#           Retain time value fields in seconds (default 900)
#       --strict
#           Remove zone and rules not aligned at time granularity.
#
#   Validator:
#       --validate
#           Validate the zone_infos and zone_policies
#       --validate_dst_offset
#           Validate DST offsets as well (many false positives due to pytz).
#       --validate_hours
#           Validate 0h to 23h of each day from 2000 to 2038, instead of
#           just the transition days. (Takes long time).

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the github repository.
INPUT_DIR=$HOME/dev/tz

# Output generated code to the current directory
OUTPUT_DIR=$PWD

function usage() {
    echo 'Usage: tzcompiler.sh --tag tag [--python | --arduino | --arduinox ]'
    echo '      (--zonedb|--validate|--unittest) [...other python_flags...]'
    exit 1
}

pass_thru_flags=''
tag=''
while [[ $# -gt 0 ]]; do
    case $1 in
        --tag) shift; tag=$1 ;;
        --help|-h) usage ;;
        -*) break ;;
        *) break ;;
    esac
    shift
done
if [[ "$tag" == '' ]]; then
    usage
fi

echo "\$ pushd $INPUT_DIR"
pushd $INPUT_DIR

echo "\$ git co $tag"
git co $tag

echo '$ popd'
popd

echo \$ $DIRNAME/tzcompiler.py \
    --input_dir $INPUT_DIR \
    --output_dir $OUTPUT_DIR \
    --tz_version $tag \
    $@
$DIRNAME/tzcompiler.py \
    --input_dir $INPUT_DIR \
    --output_dir $OUTPUT_DIR \
    --tz_version $tag \
    "$@"

echo "\$ pushd $INPUT_DIR"
pushd $INPUT_DIR

echo '$ git co master'
git co master

echo '$ popd'
popd
