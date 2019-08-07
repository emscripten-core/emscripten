#!/usr/bin/env python3
import errno
import os
import shutil
import sys
import uuid
from glob import glob

from coverage.cmdline import main

if __name__ == '__main__':
  sys.executable = os.path.abspath(__file__)

  store = os.path.join(os.path.dirname(sys.executable), 'coverage')
  normal_run = True

  if len(sys.argv) < 2 or sys.argv[1] == 'help':
    print('Emscripten coverage helper')
    print('Run emscripten commands like: ./tools/emcoverage.py emcc ...')
    print('Run ./tools/emcoverage.py report/html/xml to get reports')
    print('Run ./tools/emcoverage.py reset to clean up old data and start anew')
    sys.exit()
  elif sys.argv[1] == 'reset':
    shutil.rmtree(store)
    sys.exit()
  elif sys.argv[1] in ('html', 'report', 'xml'):
    old_argv = sys.argv
    sys.argv = ['coverage', 'combine'] + glob(os.path.join(store, '*'))
    try:
      main()
    except SystemExit:
      pass
    sys.argv = old_argv
    sys.exit(main())
  elif not os.path.exists(sys.argv[1]):
    sys.argv[1] = os.path.join(os.path.dirname(sys.executable), sys.argv[1] + '.py')

  try:
    os.mkdir(store)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise
  os.environ['COVERAGE_FILE'] = os.path.join(store, str(uuid.uuid4()))
  sys.argv[0:1] = ['coverage', 'run', '--timid', '--']

  sys.exit(main())
