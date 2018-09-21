#!/usr/bin/env python
# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Add or verify emscripten license header in source files."""

import sys
import os
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
__rootpath__ = os.path.dirname(script_dir)

cpp_license = '''\
// Copyright %s The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
'''

py_license = '''\
# Copyright %s The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.
'''

c_license = '''\
/*
 * Copyright %s The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
'''

c_license_base = '''\
/*
 * Copyright %s The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
'''

exclude_filenames = [
    'system/include/',
    'system/lib/libc/musl/',
    'system/lib/html5/dom_pk_codes.c',
    'system/lib/dlmalloc.c',
    'third_party/',
    'tools/eliminator/node_modules/',
    'tests/freetype/src/',
    'tests/bullet/src/',
    'tests/poppler/poppler/',
    'tests/box2d/',
    'tests/glbook/',
    'tests/lzma/lzma/',
    'tests/nbody-java/',
    'tests/cube2hash/',
    'tests/optimizer/',
    'tests/sqlite/sqlite3.c',
    'site/source/_themes/',
]

exclude_contents = ['Copyright', 'LICENSE.TXT', 'PUBLIC DOMAIN']


def process_file(filename):
  if any(filename.startswith(ex) for ex in exclude_filenames):
    return
  ext = os.path.splitext(filename)[1]
  if ext not in ('.py', '.c', '.cpp', '.h', '.js'):
    return
  with open(filename) as f:
    contents = f.read()
  header = '\n'.join(contents.splitlines()[:30])
  if any(ex in header for ex in exclude_contents):
    return
  output = subprocess.check_output(['git', 'log', '--pretty=format:%cd', '--date=format:%Y', filename])
  year = output.splitlines()[-1].split()[0]
  print(filename)
  with open(filename, 'w') as f:
    if ext == '.py':
      if contents.startswith('#!'):
        line1, rest = contents.split('\n', 1)
        f.write(line1 + '\n')
        contents = rest
      f.write(py_license % year)
      if not contents.startswith('\n'):
        f.write('\n')
    elif ext in ('.c', '.h'):
      f.write(c_license % year)
      if not contents.startswith('\n'):
        f.write('\n')
    elif ext in ('.cpp', '.js'):
        if contents.startswith('/*\n'):
          contents = contents[3:]
          f.write(c_license_base % year)
        else:
          f.write(cpp_license % year)
          if not contents.startswith('\n'):
            f.write('\n')
    else:
      assert False
    f.write(contents)


def main():
  os.chdir(__rootpath__)
  filenames = sys.argv[1:]
  if not filenames:
    filenames = subprocess.check_output(['git', 'ls-files']).splitlines()
  for filename in filenames:
    process_file(filename)
  return 0


if __name__ == '__main__':
  sys.exit(main())
