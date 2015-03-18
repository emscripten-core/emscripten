#!/usr/bin/env python2
# This Python file uses the following encoding: utf-8

import os, sys, subprocess, multiprocessing, threading, time
from runner import test_modes, PYTHON, path_from_root

# run slower ones first, to optimize total time
optimal_order = ['asm3i', 'asm1i', 'asm2nn', 'asm3', 'asm2', 'asm2g', 'asm2f', 'asm1', 'default']
assert set(optimal_order) == set(test_modes), 'need to update the list of slowest modes'

# clean up previous output
for mode in optimal_order:
  if os.path.exists(mode + '.err'):
    os.unlink(mode + '.err')

# set up a background thread to report progress
class Watcher(threading.Thread):
  stop = False

  def run(self):
    last = -1
    while not Watcher.stop:
      total = 0
      for mode in optimal_order:
        if os.path.exists(mode + '.err'):
          total += os.stat(mode + '.err').st_size
      if total != last:
        last = total
        print >> sys.stderr, '[parallel_test_copy.py watcher] total output: %d' % total
      time.sleep(1)

watcher = Watcher()
watcher.start()

# run tests for one mode
def run_mode(mode):
  print '<< running %s >>' % mode
  proc = subprocess.Popen([PYTHON, path_from_root('tests', 'runner.py'), mode], stdout=open(mode + '.out', 'w'), stderr=open(mode + '.err', 'w'))
  proc.communicate()
  print '<< %s finished >>' % mode

# run all modes
cores = int(os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())
pool = multiprocessing.Pool(processes=cores)
filenames = pool.map(run_mode, optimal_order, chunksize=1)

# quit watcher
Watcher.stop = True

# emit all outputs
for mode in optimal_order:
  print >> sys.stderr, '=== %s ===' % mode
  if os.path.exists(mode + '.err'):
    print >> sys.stderr, open(mode + '.err').read()
  print >> sys.stderr

