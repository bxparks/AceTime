#!/bin/bash
#
# Copyright 2020 Brian T. Park
#
# MIT License
#
# Copy the minimal set of tz database files from the git repo that tracks the
# IANA TZ database at https://github.com/eggert/tz/, into a directory named
# {tag} in the current directory.
#
# Usage:
#
#   $ copytz.sh [--source {source}] {tag}
#
# Example:
#
#   $ copytz.sh 2019c
#
# which copies the relevant TZDB files into the directory:
#
#   ./2019c/

# Use flock(1) to ensure that only a single instance of this script runs at the
# same time. The script will wait up to 10 seconds for another instance to
# finish before exiting with an error code. This is needed because the script
# runs a 'git checkout' command on the git repo. If 2 different instances used
# different --tag values, they would interfere with each other.
[[ "${FLOCKER}" != "$0" ]] && exec env FLOCKER="$0" flock -ew 10 "$0" "$0" "$@"

set -eu

DIRNAME=$(realpath $(dirname $0))

# Default TZ git repository.
SOURCE_DIR=$(realpath $DIRNAME/../../tz)

# TZ files to copy for completeness. The tzcompiler.py will not use all of them.
TZ_FILES="\
africa
antarctica
asia
australasia
backward
etcetera
europe
factory
northamerica
southamerica
systemv
"

function usage() {
    echo 'Usage: copytz.sh [--source {src}] tag'
    exit 1
}

tag=''
git_repo=$SOURCE_DIR
while [[ $# -gt 0 ]]; do
    case $1 in
        --source) shift; git_repo=$1 ;;
        --help|-h) usage ;;
        -*) echo "Unknown flag '$1'"; usage ;;
        *) break ;;
    esac
    shift
done
if [[ $# -ne 1 ]]; then
    echo "Missing {tag}"
    usage
fi
tag=$1

# Create the target directory named $tag"
echo "+ mkdir -p $tag"
mkdir -p $tag
target_path=$(realpath $tag)

# Check out the $tag
echo "+ cd $git_repo"
cd $git_repo
echo "+ git checkout $tag"
git checkout -q $tag

# Copy the files into the target directory.
copy_cmd="cp $(echo $TZ_FILES) $target_path"
echo "+ $copy_cmd"
eval "$copy_cmd"

# Set the repo to the 'master' branch.
echo "+ git checkout master"
git checkout -q master

echo "==== Copied TZ files into '$tag' directory"
