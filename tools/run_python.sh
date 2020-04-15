#!/bin/sh
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.
#
# Entry point for running python scripts on UNIX systems.

if [ -z $PYTHON ]; then
  PYTHON=$(which python3)
fi

if [ -z $PYTHON ]; then
  PYTHON=$(which python)
fi

if [ -z $PYTHON ]; then
  PYTHON=$(which python2)
fi

if [ -z $PYTHON ]; then
  print 'unable to find python in $PATH'
  exit 1
fi

exec $PYTHON "$0.py" "$@"
