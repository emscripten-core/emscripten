#!/bin/bash
# Based on binaryen/scripts/clang-format-diff.sh
# TODO(sbc): Switch to pre-packaged github actions once we find one that
# is mature enough for our needs.

set -o errexit
set -o pipefail
set -o xtrace

if [ -n "$1" ]; then
  BRANCH="$1"
else
  BRANCH="@{upstream}"
fi

MERGE_BASE=$(git merge-base $BRANCH HEAD)
FORMAT_ARGS=${MERGE_BASE}
if [ -n "$LLVM_VERSION" ]; then
  FORMAT_ARGS="--binary=clang-format-${LLVM_VERSION} ${FORMAT_ARGS}"
fi

git clang-format ${FORMAT_ARGS} --diff
