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

# Output generated code to the current directory
OUTPUT_DIR=$PWD

function usage() {
    echo 'Usage: process.sh --tag tag [--[py]code] [python_flags...]'
    exit 1
}

pass_thru_flags=''
tag=''
output_option=''
while [[ $# -gt 0 ]]; do
    case $1 in
        --code) output_option="--output_dir $OUTPUT_DIR" ;;
        --pycode) output_option="--python --output_dir $OUTPUT_DIR" ;;
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

echo \$ $DIRNAME/process.py \
    --input_dir $INPUT_DIR \
    --tz_version $tag \
    $output_option \
    $@
$DIRNAME/process.py \
    --input_dir $INPUT_DIR \
    --tz_version $tag \
    $output_option \
    "$@"

echo "\$ pushd $INPUT_DIR"
pushd $INPUT_DIR

echo '$ git co master'
git co master

echo '$ popd'
popd
