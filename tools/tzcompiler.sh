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
    echo 'Usage: tzcompiler.sh --tag tag [--python] [--arduino] [--validate]'
    echo '       [python_flags...]'
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
