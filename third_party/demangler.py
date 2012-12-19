#!/usr/bin/python2

'''
Simple tool to run the demangler.

(C) 2010 Alon Zakai, MIT licensed

Usage: demangler.py FILENAME SPLITTER

Make sure you define ~/.emscripten, and fill it with something like

JS_ENGINE=[os.path.expanduser('~/Dev/v8/d8')]
JS_ENGINE_PARAMS=['--']

or

JS_ENGINE=[os.path.expanduser('~/Dev/tracemonkey/js/src/js')]
JS_ENGINE_PARAMS=[]

'''

import os, sys, subprocess, re

__rootpath__ = os.path.dirname(os.path.abspath(__file__))
def path_from_root(*pathelems):
  return os.path.join(os.path.sep, *(__rootpath__.split(os.sep)[:-1] + list(pathelems)))
sys.path += [path_from_root('')]
from tools.shared import *

data = open(sys.argv[1], 'r').readlines()

SEEN = {}
for line in data:
  if len(line) < 4: continue
  m = re.match('^  function (?P<func>[^(]+)\(.*', line) # generated code
  if not m:
    m = re.match('^ + _*\d+: (?P<func>[^ ]+) \(\d+.*', line) # profiling output
  if not m: continue
  func = m.groups('func')[0]
  if func in SEEN: continue
  SEEN[func] = True
  cleaned = run_js(JS_ENGINE, path_from_root('third_party', 'gcc_demangler.js'), [func[1:]])
  if cleaned is None: continue
  if 'Fatal exception' in cleaned: continue
  cleaned = cleaned[1:-2]
  if cleaned == '(null)': continue
  if ' throw ' in cleaned: continue
  print func, '=', cleaned

