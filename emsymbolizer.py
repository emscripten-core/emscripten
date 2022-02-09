#!/usr/bin/env python3

# This is a utility for looking up the symbol names and/or file+line numbers
# of code addresses. There are several possible sources of this information,
# with varying granularity (listed here in approximate preference order).

# If the wasm has DWARF info, llvm-symbolizer can show the symbol, file, and
# line/column number, potentially including inlining.
# If the wasm has separate DWARF info, do the above with the side file
# If there is a source map, we can parse it to get file and line number.
# If there is an emscripten symbol map, we can parse that to get the symbol name
# If there is a name section or symbol table, llvm-nm can show the symbol name.

import argparse
from collections import namedtuple
import json
import os
import sys
from tools import shared
from tools import webassembly
from tools.shared import check_call

LLVM_SYMBOLIZER = os.path.expanduser(
    shared.build_llvm_tool_path(shared.exe_suffix('llvm-symbolizer')))


class Error(BaseException):
  pass


def get_codesec_offset(module):
  for sec in module.sections():
    if sec.type == webassembly.SecType.CODE:
      return sec.offset
  raise Error(f'No code section found in {module.filename}')


def has_debug_line_section(module):
  for sec in module.sections():
    if sec.name == ".debug_line":
      return True
  return False


def symbolize_address_dwarf(module, address):
  vma_adjust = get_codesec_offset(module)
  cmd = [LLVM_SYMBOLIZER, '-e', module.filename, f'--adjust-vma={vma_adjust}',
         str(address)]
  check_call(cmd)


def get_sourceMappingURL_section(module):
  for sec in module.sections():
    if sec.name == "sourceMappingURL":
      return sec
  return None


class WasmSourceMap(object):
  # This implementation is derived from emscripten's sourcemap-support.js
  Location = namedtuple('Location',
                        ['source', 'line', 'column', 'name'])

  def __init__(self):
    self.version = None
    self.sources = []
    self.names = []
    self.mapping = {}
    self.offsets = []

  def parse(self, filename):
    with open(filename) as f:
      source_map_json = json.loads(f.read())
      if shared.DEBUG:
        print(source_map_json)

    self.version = source_map_json['version']
    self.sources = source_map_json['sources']
    self.names = source_map_json['names']

    vlq_map = {}
    chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/='
    for i, c in enumerate(chars):
      vlq_map[c] = i

    def decodeVLQ(string):
      result = []
      shift = 0
      value = 0
      for i, c in enumerate(string):
        try:
          integer = vlq_map[c]
        except ValueError:
          raise Error(f'Invalid character ({c}) in VLQ')
        value += (integer & 31) << shift
        if integer & 32:
          shift += 5
        else:
          negate = value & 1
          value >>= 1
          result.append(-value if negate else value)
          value = shift = 0
      return result

    offset = 0
    src = 0
    line = 1
    col = 1
    name = 0
    for i, segment in enumerate(source_map_json['mappings'].split(',')):
      data = decodeVLQ(segment)
      info = []

      offset += data[0]
      if len(data) >= 2:
        src += data[1]
        info.append(src)
      if len(data) >= 3:
        line += data[2]
        info.append(line)
      if len(data) >= 4:
        col += data[3]
        info.append(col)
      if len(data) >= 5:
        name += data[4]
        info.append(name)

      self.mapping[offset] = WasmSourceMap.Location(*info)
      self.offsets.append(offset)
    self.offsets.sort()

  def find_offset(self, offset):
    # Find the largest mapped offset <= the search offset
    lo = 0
    hi = len(self.offsets)

    while lo < hi:
      mid = (lo + hi) // 2
      if self.offsets[mid] > offset:
        hi = mid
      else:
        lo = mid + 1
    return self.offsets[lo - 1]

  def lookup(self, offset):
    nearest = self.find_offset(offset)
    assert nearest in self.mapping, 'Sourcemap has an offset with no mapping'
    info = self.mapping[nearest]

    # TODO: it's kind of icky to use Location for both the internal indexed
    # location and external string version. Once we have more uniform output
    # format and API for the various backends (e.g SM vs DWARF vs others), this
    # could be improved.
    return WasmSourceMap.Location(
        self.sources[info.source] if info.source is not None else None,
        info.line,
        info.column,
        self.names[info.name] if info.name is not None else None)


def symbolize_address_sourcemap(module, address, force_file):
  URL = force_file
  if not URL:
    # If a sourcemap file is not forced, read it from the wasm module
    section = get_sourceMappingURL_section(module)
    assert section
    module.seek(section.offset)
    assert module.readString() == 'sourceMappingURL'
    # TODO: support stripping/replacing a prefix from the URL
    URL = module.readString()

  if shared.DEBUG:
    print(f'Source Mapping URL: {URL}')
  sm = WasmSourceMap()
  sm.parse(URL)
  if shared.DEBUG:
    csoff = get_codesec_offset(module)
    print(sm.mapping)
    # Print with section offsets to easily compare against dwarf
    for k, v in sm.mapping.items():
      print(f'{k-csoff:x}: {v}')
  print(sm.lookup(address))


def main(args):
  module = webassembly.Module(args.wasm_file)
  base = 16 if args.address.lower().startswith('0x') else 10
  address = int(args.address, base)
  symbolized = 0

  if args.addrtype == 'code':
    address += get_codesec_offset(module)

  if ((has_debug_line_section(module) and not args.source) or
     'dwarf' in args.source):
    symbolize_address_dwarf(module, address)
    symbolized += 1

  if ((get_sourceMappingURL_section(module) and not args.source) or
     'sourcemap' in args.source):
    symbolize_address_sourcemap(module, address, args.file)
    symbolized += 1

  if not symbolized:
    raise Error('No .debug_line or sourceMappingURL section found in '
                f'{module.filename}.'
                " I don't know how to symbolize this file yet")


def get_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('-s', '--source', help='Force debug info source type',
                      default=())
  parser.add_argument('-f', '--file', action='store',
                      help='Force debug info source file')
  parser.add_argument('-t', '--addrtype', choices=['code', 'file'],
                      default='file',
                      help='Address type (code section or file offset)')
  parser.add_argument('-v', '--verbose', action='store_true',
                      help='Print verbose info for debugging this script')
  parser.add_argument('wasm_file', help='Wasm file')
  parser.add_argument('address', help='Address to lookup')
  args = parser.parse_args()
  if args.verbose:
    shared.PRINT_STAGES = 1
    shared.DEBUG = True
  return args


if __name__ == '__main__':
  print('Warning: the command-line and output format of this tool are not '
        'finalized yet', file=sys.stderr)
  try:
    rv = main(get_args())
  except (Error, webassembly.InvalidWasmError, OSError) as e:
    print(f'{sys.argv[0]}: {str(e)}', file=sys.stderr)
    rv = 1
  sys.exit(rv)
