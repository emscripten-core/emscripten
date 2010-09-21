#!/usr/bin/python

import os, sys, subprocess

COMPILER = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'src', 'compiler.js')
CONFIG_FILE = os.path.expanduser('~/.emscripten')
JS_ENGINE = None

if os.path.exists(CONFIG_FILE):
  exec(open(CONFIG_FILE, 'r').read())

def emscripten(filename, settings):
  data = open(filename, 'r').read()
  cwd = os.getcwd()
  os.chdir(os.path.dirname(COMPILER))
  subprocess.Popen([JS_ENGINE, COMPILER], stdin=subprocess.PIPE).communicate(settings+'\n'+data)[0]
  os.chdir(cwd)

if __name__ == '__main__':
  if sys.argv.__len__() not in [2,3,4]:
    print '''
Emscripten usage:    emscripten.py [INFILE PATH-TO-JS-ENGINE] [SETTINGS]

  INFILE must be in human-readable LLVM disassembly form (i.e., as text,
      not binary).
  PATH-TO-JS-ENGINE should be a path to the JavaScript engine used to
      run the compiler (which is in JavaScript itself). You can later use
      the same engine to run the code, or another one, that is a separate
      issue. If you do not provide this parameter, you should define
      JS_ENGINE = ... in a file at ~/.emscripten.
  SETTINGS is an optional set of compiler settings, overriding the defaults.
'''
  else:
    if len(sys.argv) >= 3:
      JS_ENGINE = sys.argv[2]
    emscripten(sys.argv[1], sys.argv[3] if len(sys.argv) == 4 else "{}")

