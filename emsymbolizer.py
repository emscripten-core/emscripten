#!/usr/bin/env python3

# This is a utility for looking up the symbol names and/or file+line numbers
# of code addresses. There are several possible sources of this information,
# with varying granularity (listed here in approximate preference order).

# If the wasm has DWARF info, llvm-symbolizer can show the symbol, file, and
# line/column number, potentially including inlining.
# If the wasm has separate DWARF info, do the above with the side file
# If there is a source map, we can parse it to get file and line number.
# If there is an emscripten symbol map, we can use that to get the symbol name
# If there is a name section or symbol table, llvm-symbolizer can show the
#  symbol name.
# Separate DWARF and emscripten symbol maps are not supported yet.

import argparse
import json
import os
import re
import subprocess
import sys
from tools import shared
from tools import webassembly

LLVM_SYMBOLIZER = os.path.expanduser(
    shared.build_llvm_tool_path(shared.exe_suffix('llvm-symbolizer')))


class Error(BaseException):
  pass


# Class to treat location info in a uniform way across information sources.
class LocationInfo(object):
  def __init__(self, source=None, line=0, column=0, func=None):
    self.source = source
    self.line = line
    self.column = column
    self.func = func

  def print(self):
    source = self.source if self.source else '??'
    func = self.func if self.func else '??'
    print(f'{func}\n{source}:{self.line}:{self.column}')


def get_codesec_offset(module):
  sec = module.get_section(webassembly.SecType.CODE)
  if not sec:
    raise Error(f'No code section found in {module.filename}')
  return sec.offset


def has_debug_line_section(module):
  return module.get_custom_section('.debug_line') is not None


def has_name_section(module):
  return module.get_custom_section('name') is not None


def has_linking_section(module):
  return module.get_custom_section('linking') is not None


def symbolize_address_symbolizer(module, address, is_dwarf):
  if is_dwarf:
    vma_adjust = get_codesec_offset(module)
  else:
    vma_adjust = 0
  cmd = [LLVM_SYMBOLIZER, '-e', module.filename, f'--adjust-vma={vma_adjust}',
         str(address)]
  out = shared.run_process(cmd, stdout=subprocess.PIPE).stdout.strip()
  out_lines = out.splitlines()

  # Source location regex, e.g., /abc/def.c:3:5
  SOURCE_LOC_RE = re.compile(r'(.+):(\d+):(\d+)$')
  # llvm-symbolizer prints two lines per location. The first line contains a
  # function name, and the second contains a source location like
  # '/abc/def.c:3:5'. If the function or source info is not available, it will
  # be printed as '??', in which case we store None. If the line and column info
  # is not available, they will be printed as 0, which we store as is.
  for i in range(0, len(out_lines), 2):
    func, loc_str = out_lines[i], out_lines[i + 1]
    m = SOURCE_LOC_RE.match(loc_str)
    source, line, column = m.group(1), m.group(2), m.group(3)
    if func == '??':
      func = None
    if source == '??':
      source = None
    LocationInfo(source, line, column, func).print()


def get_sourceMappingURL_section(module):
  for sec in module.sections():
    if sec.name == "sourceMappingURL":
      return sec
  return None


class WasmSourceMap(object):
  class Location(object):
    def __init__(self, source=None, line=0, column=0, func=None):
      self.source = source
      self.line = line
      self.column = column
      self.func = func

  def __init__(self):
    self.version = None
    self.sources = []
    self.mappings = {}
    self.offsets = []

  def parse(self, filename):
    with open(filename) as f:
      source_map_json = json.loads(f.read())
      if shared.DEBUG:
        print(source_map_json)

    self.version = source_map_json['version']
    self.sources = source_map_json['sources']

    vlq_map = {}
    chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/='
    for i, c in enumerate(chars):
      vlq_map[c] = i

    def decodeVLQ(string):
      result = []
      shift = 0
      value = 0
      for c in string:
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
    for segment in source_map_json['mappings'].split(','):
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
      # TODO: see if we need the name, which is the next field (data[4])

      self.mappings[offset] = WasmSourceMap.Location(*info)
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
    assert nearest in self.mappings, 'Sourcemap has an offset with no mapping'
    info = self.mappings[nearest]
    return LocationInfo(
        self.sources[info.source] if info.source is not None else None,
        info.line,
        info.column
      )


def symbolize_address_sourcemap(module, address, force_file):
  URL = force_file
  if not URL:
    # If a sourcemap file is not forced, read it from the wasm module
    section = get_sourceMappingURL_section(module)
    assert section
    module.seek(section.offset)
    assert module.read_string() == 'sourceMappingURL'
    # TODO: support stripping/replacing a prefix from the URL
    URL = module.read_string()

  if shared.DEBUG:
    print(f'Source Mapping URL: {URL}')
  sm = WasmSourceMap()
  sm.parse(URL)
  if shared.DEBUG:
    csoff = get_codesec_offset(module)
    print(sm.mappings)
    # Print with section offsets to easily compare against dwarf
    for k, v in sm.mappings.items():
      print(f'{k-csoff:x}: {v}')
  sm.lookup(address).print()


def main(args):
  with webassembly.Module(args.wasm_file) as module:
    base = 16 if args.address.lower().startswith('0x') else 10
    address = int(args.address, base)

    if args.addrtype == 'code':
      address += get_codesec_offset(module)

    if ((has_debug_line_section(module) and not args.source) or
       'dwarf' in args.source):
      symbolize_address_symbolizer(module, address, is_dwarf=True)
    elif ((get_sourceMappingURL_section(module) and not args.source) or
          'sourcemap' in args.source):
      symbolize_address_sourcemap(module, address, args.file)
    elif ((has_name_section(module) and not args.source) or
          'names' in args.source):
      symbolize_address_symbolizer(module, address, is_dwarf=False)
    elif ((has_linking_section(module) and not args.source) or
          'symtab' in args.source):
      symbolize_address_symbolizer(module, address, is_dwarf=False)
    else:
      raise Error('No .debug_line or sourceMappingURL section found in '
                  f'{module.filename}.'
                  " I don't know how to symbolize this file yet")


def get_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('-s', '--source', choices=['dwarf', 'sourcemap',
                                                 'names', 'symtab'],
                      help='Force debug info source type', default=())
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
    shared.PRINT_SUBPROCS = 1
    shared.DEBUG = True
  return args


if __name__ == '__main__':
  try:
    rv = main(get_args())
  except (Error, webassembly.InvalidWasmError, OSError) as e:
    print(f'{sys.argv[0]}: {str(e)}', file=sys.stderr)
    rv = 1
  sys.exit(rv)
