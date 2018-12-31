#!/usr/bin/env python2
# This Python file uses the following encoding: utf-8

'''
Parallel test runner
--------------------

You may want to run this with unbuffered output, python -u ...
'''

import os, sys, subprocess, multiprocessing, threading, time
from runner import test_modes, PYTHON, path_from_root

assert not os.environ.get('EM_SAVE_DIR'), 'Need separate directories to avoid the parallel tests clashing'

# run slower ones first, to optimize total time
optimal_order = [
  'asm2i',
  'asm2nn',
  'asm3',
  'asm2',
  'asm2g',
  'asm2f',
  'asm1',
  'default'
]
assert set(optimal_order) == set(test_modes), 'need to update the list of slowest modes'

# set up a background thread to report progress
class Watcher(threading.Thread):
  stop = False

  def run(self):
    last = -1
    bytes_read = {}
    bytes = {}
    for mode in optimal_order:
      bytes[mode] = ''
      bytes_read[mode] = 0

    running = True
    while running:
      time.sleep(2)
      if Watcher.stop: running = False
      for mode in optimal_order:
        logfile = mode + '.err'
        if os.path.exists(logfile):
          new_size = os.stat(logfile).st_size
          if new_size > bytes_read[mode]:
            with open(logfile, 'rb') as f:
              f.seek(bytes_read[mode])
              bytes[mode] += f.read(new_size - bytes_read[mode])
            bytes_read[mode] = new_size

            # Flush printed lines to stdout if we have enough worth of one full test.
            most_recent_test_start_pos = bytes[mode].rfind('(test_core.')
            if most_recent_test_start_pos != -1:
              most_recent_line_end = bytes[mode].rfind('\ntest_', 0, most_recent_test_start_pos)
              if most_recent_line_end != -1:
                lines_ready_to_print = bytes[mode][0:most_recent_line_end+1].strip()
                print lines_ready_to_print
                bytes[mode] = bytes[mode][most_recent_line_end+1:]

          # Flush all the remaining lines if we are quitting.
          if not running:
            print bytes[mode]

# run tests for one mode
def run_mode(args):
  mode = args[0]
  if len(args) > 1:
    # If args has multiple elements, then only the selected tests from that suite are run e.g. args=['asm1', 'test_hello_world', 'test_i64'] runs only tests asm1.test_hello_world and asm1.test_i64.
    args = args[1:]
    for i in range(len(args)):
      if args[i].startswith('test_'):
        args[i] = mode + '.' + args[i]
    print '<< running %s >>' % str(args)
  else:
    # If args has only one argument, e.g. args=['default'] or args=['asm1'], all tests are run in one suite.
    print '<< running %s >>' % mode

  proc = subprocess.Popen([PYTHON, path_from_root('tests', 'runner.py')] + args, stdout=open(mode + '.out', 'w'), stderr=open(mode + '.err', 'w'))
  proc.communicate()
  print '<< %s finished with exit code %d >>' % (mode, proc.returncode)
  return proc.returncode

def main():
  # clean up previous output
  for mode in optimal_order:
    if os.path.exists(mode + '.err'):
      os.unlink(mode + '.err')

  # prebuild the cache before starting the parallel run of the whole suite to avoid a race condition where each thread would start building the cache separately
  subprocess.check_call([PYTHON, path_from_root('embuilder.py'), 'build', 'ALL'])

  watcher = Watcher()
  watcher.start()

  # run all modes
  cores = int(os.environ.get('PARALLEL_SUITE_EMCC_CORES') or os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())
  pool = multiprocessing.Pool(processes=cores)
  args = [[x] + sys.argv[1:] for x in optimal_order]
  num_failures = pool.map(run_mode, args, chunksize=1)

  # quit watcher
  Watcher.stop = True

  return sum(num_failures)

if __name__ == '__main__':
  sys.exit(main())
