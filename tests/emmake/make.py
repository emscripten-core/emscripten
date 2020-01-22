# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os


def which(program):
  def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

  fpath, fname = os.path.split(program)
  if fpath:
    if is_exe(program):
      return program
  else:
    for path in os.getenv("PATH", "").split(os.pathsep):
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file
  raise Exception('that is very bad')


def test(var):
  val = os.getenv(var, '')
  print('%s=%s' % (var, val))
  print(which(val))


def check_ar():
  print("Testing...")
  test("CC")
  test("CXX")
  test("AR")
  test("LD")
  test("NM")
  test("LDSHARED")
  test("RANLIB")
  print("Done.")


check_ar()
