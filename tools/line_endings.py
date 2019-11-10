#!/usr/bin/env python
# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os
import sys


def convert_line_endings(text, from_eol, to_eol):
  if from_eol == to_eol:
    return text
  return text.replace(from_eol, to_eol)


def convert_line_endings_in_file(filename, from_eol, to_eol):
  if from_eol == to_eol:
    return # No conversion needed

  with open(filename, 'rb') as f:
    text = f.read()
  text = convert_line_endings(text, from_eol.encode(), to_eol.encode())
  with open(filename, 'wb') as f:
    f.write(text)


def check_line_endings(filename, expect_only=None, print_errors=True, print_info=False):
  """Detect inconsitent/invalid line endings.

  This function checks and prints out the detected line endings in the given
  file. If the file only contains either Windows \\r\\n line endings or Unix \\n
  line endings, it returns 0.  Otherwise, in the presence of old macOS or
  mixed/malformed line endings, a non-zero error code is returned.
  """
  if not os.path.exists(filename):
    if print_errors:
      print('File not found: ' + filename, file=sys.stderr)
    return 1

  with open(filename, 'rb') as f:
    data = f.read()

  index = data.find(b"\r\r\n")
  if index != -1:
    if print_errors:
      print("File '" + filename + "' contains BAD line endings of form \\r\\r\\n!", file=sys.stderr)
      bad_line = data[index - 50:index + 50].replace(b'\r', b'\\r').replace(b'\n', b'\\n')
      print("Content around the location: '" + bad_line + "'", file=sys.stderr)
    return 1 # Bad line endings in file, return a non-zero process exit code.

  has_dos_line_endings = False
  has_unix_line_endings = False
  dos_line_ending_example = ''
  dos_line_ending_count = 0
  unix_line_ending_example = ''
  unix_line_ending_count = 0

  index = data.find(b'\r\n')
  if index != -1:
    dos_line_ending_example = data[index - 50:index + 50].replace(b'\r', b'\\r').replace(b'\n', b'\\n')
    dos_line_ending_count = data.count(b'\r\n')
    # Replace all DOS line endings with some other character, and continue testing what's left.
    data = data.replace(b'\r\n', b'A')
    has_dos_line_endings = True

  index = data.find(b'\r\n')
  if index != -1:
    unix_line_ending_example = data[index - 50:index + 50].replace(b'\r', b'\\r').replace(b'\n', b'\\n')
    unix_line_ending_count = data.count(b'\n')
    has_unix_line_endings = True

  index = data.find(b'\r')
  if index != -1:
    old_macos_line_ending_example = data[index - 50:index + 50].replace(b'\r', b'\\r').replace(b'\n', b'\\n')
    if print_errors:
      print('File \'' + filename + '\' contains OLD macOS line endings "\\r"', file=sys.stderr)
      print("Content around an OLD macOS line ending location: '" + old_macos_line_ending_example + "'", file=sys.stderr)
    # We don't want to use the old macOS (9.x) line endings anywhere.
    return 1

  if has_dos_line_endings and has_unix_line_endings:
    if print_errors:
      print('File \'' + filename + '\' contains both DOS "\\r\\n" and UNIX "\\n" line endings! (' + str(dos_line_ending_count) + ' DOS line endings, ' + str(unix_line_ending_count) + ' UNIX line endings)', file=sys.stderr)
      print("Content around a DOS line ending location: '" + dos_line_ending_example + "'", file=sys.stderr)
      print("Content around an UNIX line ending location: '" + unix_line_ending_example + "'", file=sys.stderr)
    # Mixed line endings
    return 1

  if print_info:
    if has_dos_line_endings:
      print('File \'' + filename + '\' contains DOS "\\r\\n" line endings.')
    if has_unix_line_endings:
      print('File \'' + filename + '\' contains UNIX "\\n" line endings.')

  if expect_only == '\n' and has_dos_line_endings:
    if print_errors:
      print('File \'' + filename + '\' contains DOS "\\r\\n" line endings! (' + str(dos_line_ending_count) + ' DOS line endings), but expected only UNIX line endings!', file=sys.stderr)
      print("Content around a DOS line ending location: '" + dos_line_ending_example + "'", file=sys.stderr)
    return 1 # DOS line endings, but expected UNIX

  if expect_only == '\r\n' and has_unix_line_endings:
    if print_errors:
      print('File \'' + filename + '\' contains UNIX "\\n" line endings! (' + str(unix_line_ending_count) + ' UNIX line endings), but expected only DOS line endings!', file=sys.stderr)
      print("Content around a UNIX line ending location: '" + unix_line_ending_example + "'", file=sys.stderr)
    return 1 # UNIX line endings, but expected DOS

  return 0


if __name__ == '__main__':
  if len(sys.argv) != 2:
    print('Unknown command line ' + str(sys.argv) + '!', file=sys.stderr)
    print('Usage: ' + sys.argv[0] + ' <filename>', file=sys.stderr)
    sys.exit(1)
  sys.exit(check_line_endings(sys.argv[1], print_info=True))
