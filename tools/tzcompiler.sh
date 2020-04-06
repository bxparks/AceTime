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
#       --action (tzdb|zonedb|zonelist)
#       --language (python|arduino)
#       --scope (basic|extended)
#       [other flags...]
#
# There are 3 high-level modes of this script, depending on the --action flag:
#
#   * tzdb
#       * Generate the 'tzdb.json' file which represents our internal
#       representation of the TZ Database.
#   * zonedb
#       * Generate the 'zone_infos.*', 'zone_policies.*' files for
#       different target languages specified by the '--language' flag,
#       e.g. Arduino or Python.
#   * zonelist
#       * Generate the 'zones.txt' file which contains the list of Zone names
#       which are supported by the given --language and --scope.
#
# Examples:
#
#   $ tzcompiler.sh --tag 2018i --action tzdb --scope basic
#       Generates tzdb.json file in the current directory.
#
#   $ tzcompiler.sh --tag 2018i --action zonedb --language arduino \
#           --scope basic
#       Generates zone*.{h,cpp} files in the current directory.
#
#   $ tzcompiler.sh --tag 2018i --action zonedb --language python \
#           --scope basic
#       Generates zone*.py files in the current directory.
#
#   $ tzcompiler.sh --tag 2018i --action zonelist --scope basic
#       Generate the 'zones.txt' file in the current directory.
#
# Flags:
#
# Flags which are not recognized by this script are passed directly into
# the tzcompile.py script for further processing. Some examples are:
#
#   Transformer:
#
#       --scope (basic|extended)
#           Select the size/scope of the zone_info dataset.
#       --start_year
#           Retain TZ information since this year (default 2000).
#       --granularity
#           Retain time value fields in seconds (default 900)
#       --strict
#           Remove zone and rules not aligned at time granularity.
#
# See Also:
#
#   validate.sh

set -eu

# Can't use $(realpath $(dirname $0)) because realpath doesn't exist on MacOS
DIRNAME=$(dirname $0)

# Point to the TZ git repository, assumed to be a sibling of AceTime repository.
INPUT_DIR=$DIRNAME/../../tz

# Output generated code to the current directory
OUTPUT_DIR=$PWD

function usage() {
    echo 'Usage: tzcompiler.sh --tag tag --action (tzdb|zonedb|zonelist)'
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

echo "\$ git checkout $tag"
git checkout $tag

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

echo '$ git checkout master'
git checkout master

echo '$ popd'
popd
