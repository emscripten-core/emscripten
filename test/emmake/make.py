# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil


def test(var):
  val = os.getenv(var)
  print('%s=%s' % (var, val))
  print(shutil.which(val))


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
