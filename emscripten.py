#!/usr/bin/python

import os, sys, subprocess

COMPILER = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'src', 'parser.js')

def emscripten(filename, output_filename, js_engine):
  cwd = os.getcwd()
  os.chdir(os.path.dirname(COMPILER))
  if output_filename is not None:
    subprocess.Popen([js_engine] + [COMPILER], stdin=open(filename, 'r'), stdout=open(output_filename, 'w'), stderr=subprocess.STDOUT).communicate()[0]
  else:
    subprocess.Popen([js_engine] + [COMPILER], stdin=open(filename, 'r')).communicate()[0]
  os.chdir(cwd)

if __name__ == '__main__':
  if sys.argv.__len__() != 3:
    print '''\nEmscripten usage:          emscripten.py INFILE PATH-TO-JS-ENGINE\n'''
  else:
    emscripten(sys.argv[1], None, sys.argv[2])

