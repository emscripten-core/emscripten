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


def toLEB(x):
  assert x >= 0, 'TODO: signed'
  ret = []
  while 1:
    byte = x & 127
    x >>= 7
    more = x != 0
    if more:
      byte = byte | 128
    ret.append(byte)
    if not more:
      break
  return bytearray(ret)


def readLEB(buf, offset):
  result = 0
  shift = 0
  while True:
    byte = bytearray(buf[offset:offset + 1])[0]
    offset += 1
    result |= (byte & 0x7f) << shift
    if not (byte & 0x80):
      break
    shift += 7
  return (result, offset)


def add_emscripten_metadata(wasm_file):
  WASM_PAGE_SIZE = 65536

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


def parse_dylink_section(wasm_file):
  wasm = open(wasm_file, 'rb').read()
  section_name = b'\06dylink' # section name, including prefixed size

  # Read the existing section data
  offset = 8
  assert wasm[offset] == 0
  offset += 1
  size, offset = readLEB(wasm, offset)
  section_end = offset + size
  section = wasm[offset:section_end]
  # section name
  assert section.startswith(section_name)
  offset = len(section_name)
  mem_size, offset = readLEB(section, offset)
  mem_align, offset = readLEB(section, offset)
  table_size, offset = readLEB(section, offset)
  table_align, offset = readLEB(section, offset)

  return (mem_size, mem_align, table_size, table_align, section_end)


def add_dylink_section(wasm_file, needed_dynlibs):
  # A wasm shared library has a special "dylink" section, see tools-conventions repo.
  # This function adds this section to the beginning on the given file.

  mem_size, mem_align, table_size, table_align, section_end = parse_dylink_section(wasm_file)
  logger.debug('creating wasm dynamic library with mem size %d, table size %d, align %d' % (mem_size, table_size, mem_align))

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
  contents += toLEB(len(needed_dynlibs))
  for dyn_needed in needed_dynlibs:
    dyn_needed = bytes(shared.asbytes(dyn_needed))
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
