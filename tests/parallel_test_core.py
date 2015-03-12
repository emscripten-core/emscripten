#!/usr/bin/env python2
# This Python file uses the following encoding: utf-8

import os, sys, subprocess, multiprocessing
from runner import test_modes, PYTHON, path_from_root

# run slower ones first, to optimize total time
optimal_order = ['asm3i', 'asm1i', 'asm2nn', 'asm3', 'asm2', 'asm2g', 'asm2f', 'asm1', 'default']

assert set(optimal_order) == set(test_modes), 'need to update the list of slowest modes'

def run_mode(mode):
  print '<< running %s >>' % mode
  proc = subprocess.Popen([PYTHON, path_from_root('tests', 'runner.py'), mode], stdout=open(mode + '.out', 'w'), stderr=open(mode + '.err', 'w'))
  proc.communicate()
  print '<< %s finished >>' % mode

cores = int(os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())
pool = multiprocessing.Pool(processes=cores)
filenames = pool.map(run_mode, optimal_order, chunksize=1)

