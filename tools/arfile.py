#!/usr/bin/env python
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utility functions for parsing 'ar' files.

This is needed in emscripten because command line tools such as llvm-ar are not
able to deal with archives containing many files with the same name.  Despite
this, linkers are expected to handle this case and emscripten needs to emulate
linker behaviour when using the fastcomp backend.

See https://en.wikipedia.org/wiki/Ar_(Unix)
"""

from __future__ import print_function

import struct
import os
import sys

MAGIC = b'!<arch>\n'
builtin_open = open


class ArError(Exception):
  """Base exception."""
  pass


class ArInfo(object):
  def __init__(self, name, offset, timestamp, owner, group, mode, size, data):
    self.name = name
    self.offset = offset
    self.timestamp = timestamp
    self.owner = owner
    self.group = group
    self.mode = mode
    self.size = size
    self.data = data


class ArFile(object):
  def __init__(self, filename):
    self.filename = filename
    self._file = builtin_open(filename, 'rb')
    magic = self._file.read(len(MAGIC))
    if MAGIC != magic:
      raise ArError('not an ar file: ' + filename)
    self.members = []
    self.members_map = {}
    self.offset_to_info = {}

  def _read_member(self):
    offset = self._file.tell()
    name = self._file.read(16)
    if len(name) == 0:
      return None
    name = name.strip()
    timestamp = self._file.read(12).strip()
    owner = self._file.read(6).strip()
    group = self._file.read(6).strip()
    mode = self._file.read(8).strip()
    size = int(self._file.read(10))
    ending = self._file.read(2)
    if ending != b'\x60\n':
      raise ArError('invalid ar header')
    data = self._file.read(size)
    if mode.strip():
      mode = int(mode)
    if owner.strip():
      owner = int(owner)
    if group.strip():
      group = int(group)
    if size % 2:
      if self._file.read(1) != '\n':
        raise ArError('invalid ar header')

    return ArInfo(name.decode('utf-8'), offset, timestamp, owner, group, mode, size, data)

  def next(self):
    while True:
      # Keep reading entries until we find a non-special one
      info = self._read_member()
      if not info:
        return None
      if info.name == '//':
        # Special file containing long filenames
        self.name_data = info.data
      elif info.name == '/':
        # Special file containing symbol table
        num_entries = struct.unpack('>I', info.data[:4])[0]
        self.sym_offsets = struct.unpack('>%dI' % num_entries, info.data[4:4 + 4 * num_entries])
        symbol_data = info.data[4 + 4 * num_entries:-1]
        symbol_data = symbol_data.rstrip(b'\0')
        if symbol_data:
          self.symbols = symbol_data.split(b'\0')
        else:
          self.symbols = []
        if len(self.symbols) != num_entries:
          raise ArError('invalid symbol table')
      else:
        break

    # This entry has a name from the "//" name section.
    if info.name[0] == '/':
      name_offset = int(info.name[1:])
      if name_offset < 0 or name_offset >= len(self.name_data):
        raise ArError('invalid extended filename section')
      name_end = self.name_data.find(b'\n', name_offset)
      info.name = self.name_data[name_offset:name_end].decode('utf-8')
    info.name = info.name.rstrip('/')
    self.members.append(info)
    self.members_map[info.name] = info
    self.offset_to_info[info.offset] = info
    return info

  def getsymbols(self):
    return zip(self.symbols, self.sym_offsets)

  def getmember(self, id):
    """Polymophic member accessor that takes either and index or a name."""
    if isinstance(id, int):
      return self.getmember_by_index(id)
    return self.getmember_by_name(id)

  def getmember_by_name(self, name):
    self.getmembers()
    return self.members_map[name]

  def getmember_by_index(self, index):
    self.getmembers()
    return self.members[index]

  def getmembers(self):
    while self.next():
      pass
    return self.members

  def list(self):
    for m in self.getmembers():
      sys.stdout.write(m.name + '\n')

  def extractall(self, path="."):
    names_written = set()
    for m in self.getmembers():
      filename = m.name
      if filename in names_written:
        basename = filename
        count = 1
        while filename in names_written:
          filename = basename + '.' + str(count)
          count += 1

      names_written.add(filename)
      full_name = os.path.join(path, filename)
      with builtin_open(full_name, 'wb') as f:
        f.write(m.data)

    return sorted(list(names_written))

  def close(self):
    self._file.close()

  def __enter__(self):
    return self

  def __exit__(self, type, value, traceback):
    self.close()


def open(filename):
  return ArFile(filename)


def is_arfile(filename):
  """Return True if name points to a ar archive that we
  are able to handle, else return False.
  """
  try:
    t = open(filename)
    t.close()
    return True
  except ArError:
    return False


if __name__ == '__main__':
  open(sys.argv[1]).list()
  open(sys.argv[1]).extractall()
