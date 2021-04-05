# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utilties for manipulating WebAssembly binaries from python.
"""

import logging

from . import shared

logger = logging.getLogger('shared')

# For the Emscripten-specific WASM metadata section, follows semver, changes
# whenever metadata section changes structure.
# NB: major version 0 implies no compatibility
# NB: when changing the metadata format, we should only append new fields, not
#     reorder, modify, or remove existing ones.
EMSCRIPTEN_METADATA_MAJOR, EMSCRIPTEN_METADATA_MINOR = (0, 3)
# For the JS/WASM ABI, specifies the minimum ABI version required of
# the WASM runtime implementation by the generated WASM binary. It follows
# semver and changes whenever C types change size/signedness or
# syscalls change signature. By semver, the maximum ABI version is
# implied to be less than (EMSCRIPTEN_ABI_MAJOR + 1, 0). On an ABI
# change, increment EMSCRIPTEN_ABI_MINOR if EMSCRIPTEN_ABI_MAJOR == 0
# or the ABI change is backwards compatible, otherwise increment
# EMSCRIPTEN_ABI_MAJOR and set EMSCRIPTEN_ABI_MINOR = 0.
EMSCRIPTEN_ABI_MAJOR, EMSCRIPTEN_ABI_MINOR = (0, 29)

WASM_PAGE_SIZE = 65536


def toLEB(num):
  assert num >= 0, 'TODO: signed'
  ret = bytearray()
  while 1:
    byte = num & 127
    num >>= 7
    more = num != 0
    if more:
      byte = byte | 128
    ret.append(byte)
    if not more:
      break
  return ret


def readLEB(buf, offset):
  result = 0
  shift = 0
  while True:
    byte = buf[offset]
    offset += 1
    result |= (byte & 0x7f) << shift
    if not (byte & 0x80):
      break
    shift += 7
  return (result, offset)


def add_emscripten_metadata(wasm_file):
  mem_size = shared.Settings.INITIAL_MEMORY // WASM_PAGE_SIZE
  global_base = shared.Settings.GLOBAL_BASE

  logger.debug('creating wasm emscripten metadata section with mem size %d' % mem_size)
  name = b'\x13emscripten_metadata' # section name, including prefixed size
  contents = (
    # metadata section version
    toLEB(EMSCRIPTEN_METADATA_MAJOR) +
    toLEB(EMSCRIPTEN_METADATA_MINOR) +

    # NB: The structure of the following should only be changed
    #     if EMSCRIPTEN_METADATA_MAJOR is incremented
    # Minimum ABI version
    toLEB(EMSCRIPTEN_ABI_MAJOR) +
    toLEB(EMSCRIPTEN_ABI_MINOR) +

    # Wasm backend, always 1 now
    toLEB(1) +

    toLEB(mem_size) +
    toLEB(0) +
    toLEB(global_base) +
    toLEB(0) +
    # dynamictopPtr, always 0 now
    toLEB(0) +

    # tempDoublePtr, always 0 in wasm backend
    toLEB(0) +

    toLEB(int(shared.Settings.STANDALONE_WASM))

    # NB: more data can be appended here as long as you increase
    #     the EMSCRIPTEN_METADATA_MINOR
  )

  orig = open(wasm_file, 'rb').read()
  with open(wasm_file, 'wb') as f:
    f.write(orig[0:8]) # copy magic number and version
    # write the special section
    f.write(b'\0') # user section is code 0
    # need to find the size of this section
    size = len(name) + len(contents)
    f.write(toLEB(size))
    f.write(name)
    f.write(contents)
    f.write(orig[8:])


class Module:
  """Extremely minimal wasm module reader.  Currently only used
  for parsing the dylink section."""
  def __init__(self, filename):
    with open(filename, 'rb') as f:
      self.buf = f.read()
    assert self.buf[:4] == b'\0asm'
    assert self.buf[4:8] == b'\x01\0\0\0'
    self.offset = 8

  def readByte(self):
    ret = self.buf[self.offset]
    self.offset += 1
    return ret

  def readLEB(self):
    ret, self.offset = readLEB(self.buf, self.offset)
    return ret

  def readString(self):
    size = self.readLEB()
    end = self.offset + size
    s = self.buf[self.offset:end]
    self.offset = end
    return s.decode('utf-8')


def parse_dylink_section(wasm_file):
  module = Module(wasm_file)

  # Read the existing section data
  section_type = module.readByte()
  section_size = module.readLEB()
  assert section_type == 0
  section_end = module.offset + section_size
  # section name
  section_name = module.readString()
  assert section_name == 'dylink'
  mem_size = module.readLEB()
  mem_align = module.readLEB()
  table_size = module.readLEB()
  table_align = module.readLEB()

  needed = []
  needed_count = module.readLEB()
  while needed_count:
    libname = module.readString()
    needed.append(libname)
    needed_count -= 1

  return (mem_size, mem_align, table_size, table_align, section_end, needed)


def update_dylink_section(wasm_file, extra_dynlibs):
  # A wasm shared library has a special "dylink" section, see tools-conventions repo.
  # This function updates this section, adding extra dynamic library dependencies.

  mem_size, mem_align, table_size, table_align, section_end, needed = parse_dylink_section(wasm_file)

  section_name = b'\06dylink' # section name, including prefixed size
  contents = (toLEB(mem_size) + toLEB(mem_align) +
              toLEB(table_size) + toLEB(0))

  # we extend "dylink" section with information about which shared libraries
  # our shared library needs. This is similar to DT_NEEDED entries in ELF.
  #
  # In theory we could avoid doing this, since every import in wasm has
  # "module" and "name" attributes, but currently emscripten almost always
  # uses just "env" for "module". This way we have to embed information about
  # required libraries for the dynamic linker somewhere, and "dylink" section
  # seems to be the most relevant place.
  #
  # Binary format of the extension:
  #
  #   needed_dynlibs_count        varuint32       ; number of needed shared libraries
  #   needed_dynlibs_entries      dynlib_entry*   ; repeated dynamic library entries as described below
  #
  # dynlib_entry:
  #
  #   dynlib_name_len             varuint32       ; length of dynlib_name_str in bytes
  #   dynlib_name_str             bytes           ; name of a needed dynamic library: valid UTF-8 byte sequence
  #
  # a proposal has been filed to include the extension into "dylink" specification:
  # https://github.com/WebAssembly/tool-conventions/pull/77
  needed += extra_dynlibs
  contents += toLEB(len(needed))
  for dyn_needed in needed:
    dyn_needed = dyn_needed.encode('utf-8')
    contents += toLEB(len(dyn_needed))
    contents += dyn_needed

  orig = open(wasm_file, 'rb').read()
  file_header = orig[:8]
  file_remainder = orig[section_end:]

  section_size = len(section_name) + len(contents)
  with open(wasm_file, 'wb') as f:
    # copy magic number and version
    f.write(file_header)
    # write the special section
    f.write(b'\0') # user section is code 0
    f.write(toLEB(section_size))
    f.write(section_name)
    f.write(contents)
    # copy rest of binary
    f.write(file_remainder)
