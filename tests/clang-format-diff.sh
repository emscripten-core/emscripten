#!/bin/bash
# Based on binaryen/scripts/clang-format-diff.sh
# TODO(sbc): Switch to pre-packaged github actions once we find one that
# is mature enough for our needs.

set -o errexit

if [ "$#" -gt 0 ]; then
  BRANCH=$1
else
  BRANCH=origin/master
fi

MERGE_BASE=$(git merge-base $BRANCH HEAD)
FORMAT_MSG=$(git clang-format $MERGE_BASE -q --diff)
if [ -n "$FORMAT_MSG" -a "$FORMAT_MSG" != "no modified files to format" ]
then
  echo "Please run git clang-format before committing, or apply this diff:"
  echo
  # Run git clang-format again, this time without capruting stdout.  This way
  # clang-format format the message nicely and add color.
  git clang-format $MERGE_BASE -q --diff
  exit 1
fi
