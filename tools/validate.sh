#!/usr/bin/env bash
#
# Copyright 2018 Brian T. Park
#
# MIT License
#
# Shell script wrapper around validate.py. It performs the following:
#
#   1) Run 'git checkout' on the TZ Database repository (located at
#   $PWD/../../tz) to retrieve the TZ version specified by the (--tag).
#
#   2) Run the validate.py script to process the zone files, passing along the
#   approriate flags.
#
#   3) The validate.py script runs the Validator class and runs the
#   validate_buffer_size() and the validate_test_data() method on the class.
#
# Usage:
#
#   $ validate.sh --tag {tag} --scope (basic|extended) [other flags...]
#
# Flags:
#
#   Flags which are not recognized by this script are passed directly into
#   the validate.py script for further processing. Some examples are:
#
# Examples:
#
#   $ validate.sh --tag 2018i --scope basic

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the TZ git repository, assumed to be a sibling of AceTime repository.
INPUT_DIR=$DIRNAME/../../tz

# Output generated code to the current directory
OUTPUT_DIR=$PWD

function usage() {
    echo 'Usage: validate.sh --tag tag --scope (basic|extended)'
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

echo "\$ git checkout $tag"
git checkout -q $tag

echo '$ popd'
popd

echo \$ $DIRNAME/validate.py --input_dir $INPUT_DIR $@
$DIRNAME/validate.py --input_dir $INPUT_DIR "$@"

echo "\$ pushd $INPUT_DIR"
pushd $INPUT_DIR

echo '$ git checkout main'
git checkout -q main

echo '$ popd'
popd
