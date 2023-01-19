#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""
This is the Emscripten coverage tool.

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

import errno
import os
import shutil
import sys
import uuid
from glob import glob

import coverage.cmdline # type: ignore

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def main():
  # We set EMSDK_PYTHON to point to this file, which is executable via #! line.
  # Emscripten uses EMSDK_PYTHON to invoke all python subprocesses. By making this
  # script run all python subprocesses, all of them will execute under the
  # watchful eye of emcoverage.py, and resulting in their code coverage being
  # tracked.
  os.environ['EMSDK_PYTHON'] = os.path.abspath(__file__)

  store = os.path.join(SCRIPT_DIR, 'coverage')

  if len(sys.argv) < 2 or sys.argv[1] == 'help':
    print(__doc__.replace('emcoverage.py', sys.argv[0]).strip())
    return

  if sys.argv[1] == 'reset':
    shutil.rmtree(store)
    return

  if sys.argv[1] in ('html', 'report', 'xml'):
    old_argv = sys.argv
    sys.argv = ['coverage', 'combine'] + glob(os.path.join(store, '*'))
    try:
      coverage.cmdline.main()
    except SystemExit:
      pass
    sys.argv = old_argv + ['-i']
    return coverage.cmdline.main()

  if not os.path.exists(sys.argv[1]):
    # If argv[1] is not a file path, instead try to interpret it as an emscripten command.
    # This allows `emcoverage.py emcc` or `emcoverage.py embuilder` to work.
    sys.argv[1] = os.path.join(os.path.dirname(sys.executable), '..', sys.argv[1] + '.py')

  try:
    os.mkdir(store)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise
  os.environ['COVERAGE_FILE'] = os.path.join(store, str(uuid.uuid4()))
  sys.argv[0:1] = ['coverage', 'run', '--parallel-mode', '--']

  return coverage.cmdline.main()


if __name__ == '__main__':
  sys.exit(main())
