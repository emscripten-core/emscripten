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
  if sys.argv.__len__() not in [3,4]:
    print '''
Emscripten usage:    emscripten.py INFILE PATH-TO-JS-ENGINE [SETTINGS]

  INFILE must be in human-readable LLVM disassembly form (i.e., as text,
      not binary).
  PATH-TO-JS-ENGINE should be a path to the JavaScript engine used to
      run the compiler (which is in JavaScript itself). You can later use
      the same engine to run the code, or another one, that is a separate
      issue.
  SETTINGS is an optional set of compiler settings, overriding the defaults.
'''
  else:
    emscripten(sys.argv[1], sys.argv[2],  sys.argv[3] if len(sys.argv) == 4 else "{}")

