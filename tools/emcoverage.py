#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Emscripten coverage tool.

Usage: emcoverage.py <help|reset|report|html|xml|COMMAND> ...

Special commands:
  - help:   show this message
  - reset:  remove all gathered coverage information
  - report: show a quick overview of gathered coverage information
  - html:   generate coverage as a set of HTML files in ./htmlcov/
  - xml:    generate XML coverage report in ./coverage.xml

Otherwise, you can run any python script or Emscripten command, for example:
  - emcoverage.py ./test/runner.py core0
  - emcoverage.py emcc file1.c file2.c

Running a command under emcoverage.py will collect the code coverage
information. Every run under emcoverage.py is additive, and no coverage
information from previous runs is erased, unless explicitly done via
emcoverage.py reset.

To display the gathered coverage information, use one of the three subcommands:
report, html, xml.
"""

import os
import shutil
import sys
from glob import glob

import coverage.cmdline  # type: ignore

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def main():
  # We set EMSDK_PYTHON to point to this file, which is executable via #! line.
  # Emscripten uses EMSDK_PYTHON to invoke all python subprocesses. By making this
  # script run all python subprocesses, all of them will execute under the
  # watchful eye of emcoverage.py, resulting in their code coverage being
  # tracked.
  os.environ['EMSDK_PYTHON'] = os.path.abspath(__file__)

  store = os.path.join(SCRIPT_DIR, 'coverage')
  os.environ['COVERAGE_FILE'] = os.path.join(store, 'coverage')

  if len(sys.argv) < 2 or sys.argv[1] in {'help', '-h', '--help'}:
    print(__doc__.replace('emcoverage.py', sys.argv[0]).strip())
    return 0

  if sys.argv[1] == 'reset':
    shutil.rmtree(store, ignore_errors=True)
    return 0

  if sys.argv[1] in {'html', 'report', 'xml'}:
    if glob(os.path.join(store, 'coverage.*')):
      coverage.cmdline.main(['combine'])
    return coverage.cmdline.main([*sys.argv[1:], '-i'])

  if sys.argv[1] == '-E':
    sys.argv.pop(1)

  # If argv[1] is an emscripten command rather than a python script path, resolve it
  # to the corresponding python script living alongside it.
  candidate = os.path.splitext(sys.argv[1])[0] + '.py'
  if os.path.exists(candidate):
    sys.argv[1] = candidate

  os.makedirs(store, exist_ok=True)
  return coverage.cmdline.main(['run', '--parallel-mode', '--', *sys.argv[1:]])


if __name__ == '__main__':
  sys.exit(main())
