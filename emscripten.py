#!/usr/bin/python

import os, sys, subprocess

COMPILER = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'src', 'parser.js')

def emscripten(filename, js_engine, settings):
  data = open(filename, 'r').read()
  cwd = os.getcwd()
  os.chdir(os.path.dirname(COMPILER))
  subprocess.Popen([js_engine, COMPILER], stdin=subprocess.PIPE).communicate(settings+'\n'+data)[0]
  os.chdir(cwd)

if __name__ == '__main__':
  if sys.argv.__len__() != 3:
    print '''\nEmscripten usage:          emscripten.py INFILE PATH-TO-JS-ENGINE [SETTINGS]\n'''
  else:
    emscripten(sys.argv[1], sys.argv[2],  sys.argv[3] if len(sys.argv) == 4 else "{}")

