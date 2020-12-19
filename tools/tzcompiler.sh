#!/usr/bin/env bash
#
# Copyright 2018 Brian T. Park
#
# MIT License
#
# Shell script wrapper around tzcompiler.py. The main purpose is to run 'git
# checkout' on the TZ Database repository (located at $PWD/../../tz) to retrieve
# the TZ version specified by the (--tag). It then runs the tzcompiler.py
# script to process the zone files. The location of the TZ Database is passed
# into the tzcompiler.py using the --input_dir flag. The various 'validation_*'
# files are produced in the directory specified by the --output_dir flag.
#
# Usage:
#
#   $ tzcompiler.sh --tag {tag}
#       --language (python|arduino|json|zonelist)
#       --scope (basic|extended)
#       [other flags...]
#
# There are 4 targets depending on the --language flag:
#
#   * arduino
#       * Generate the 'zone_infos.{h,cpp}', 'zone_policies.{h,cpp}' and
#         'zone_registry.{h,cpp} files for Arduino.
#   * python
#       * Generate the 'zone_infos.py', 'zone_policies.py' files for
#         `zone_specifier.py` class in Python.
#   * json
#       * Generate the 'zonedb.json'.
#   * zonelist
#       * Generate the 'zones.txt' file which contains the list of Zone names.
#
# The '--action' flag has been reduced to a single option, and is now optional.
#
# Examples:
#
#   $ tzcompiler.sh --tag 2018i --language json --scope basic
#       Generates zonedb.json file in the current directory.
#
#   $ tzcompiler.sh --tag 2018i --language arduino --scope basic
#       Generates zone*.{h,cpp} files in the current directory.
#
#   $ tzcompiler.sh --tag 2018i --language python --scope basic
#       Generates zone*.py files in the current directory.
#
#   $ tzcompiler.sh --tag 2018i --language zonelist --scope basic
#       Generate the 'zones.txt' file in the current directory.
#
# See Also:
#
#   validate.sh

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the TZ git repository, assumed to be a sibling of AceTime repository.
INPUT_DIR=$(realpath $DIRNAME/../../tz)

# Output generated code to the current directory
OUTPUT_DIR=$PWD

function usage() {
    echo 'Usage: tzcompiler.sh '
    echo '      --tag tag '
    echo '      --language (python|arduino|json) '
    echo '      --scope (basic|extended) '
    echo '      [...other python_flags...] '
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

echo "\$ git checkout master"
git checkout -q master

echo '$ popd'
popd
