#!/usr/bin/env bash
#
# Copyright 2018 Brian T. Park
#
# MIT License
#
# Usage:
#
#   $ tzcompiler.sh --tag {tag}
#       --action (zonedb|validate|unittest)
#       --language (python|arduino)
#       --scope (basic|extended)
#       [other flags...]
#
# Examples:
#
#   $ tzcompiler.sh --tag 2018i --action zonedb --language python
#           --scope basic
#       - generates zone*.py files in the current directory
#
#   $ tzcompiler.sh --tag 2018i --action zonedb --language arduino
#           --scope basic
#       - generates zone*.{h,cpp} files in the current directory
#
#   $ tzcompiler.sh --tag 2018i --action zonedb --language arduino
#           --scope extended
#       - generates extended zone*.{h,cpp} files in the current directory
#
#   $ tzcompiler.sh --tag 2018i --action unittest --language arduino
#           --scope basic
#       - generates test data for ValidationTest unit test
#
#   $ tzcompiler.sh --tag 2018i --action validate --language python
#           --scope basic
#       - validate the internal zone_info and zone_policies data
#
#   $ tzcompiler.sh --tag 2018i --action validate --language arduino
#           --scope basic
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
#       --validate_buffer_size
#       --validate_test_data
#       --validate_dst_offset
#           Validate DST offsets as well (many false positives due to pytz).

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the github repository.
INPUT_DIR=$HOME/dev/tz

# Output generated code to the current directory
OUTPUT_DIR=$PWD

function usage() {
    echo 'Usage: tzcompiler.sh --tag tag --action (zonedb|validate|unittest)'
    echo '      --language (python|arduino) --scope (basic|extended)'
    echo '      [...other python_flags...]'
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
