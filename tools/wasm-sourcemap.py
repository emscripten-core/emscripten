#!/usr/bin/env python3
# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utility tools that extracts DWARF information encoded in a wasm output
produced by the LLVM tools, and encodes it as a wasm source map. Additionally,
it can collect original sources, change files prefixes, and strip debug
sections from a wasm file.
"""

import argparse
import json
import logging
import os
import re
import sys
from math import floor, log

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools import shared, utils
from tools.system_libs import DETERMINISTIC_PREFIX

LLVM_CXXFILT = shared.llvm_tool_path('llvm-cxxfilt')

EMSCRIPTEN_PREFIX = utils.normalize_path(utils.path_from_root())

logger = logging.getLogger('wasm-sourcemap')


def parse_args(args):
  parser = argparse.ArgumentParser(prog='wasm-sourcemap.py', description=__doc__)
  parser.add_argument('wasm', help='wasm file')
  parser.add_argument('-o', '--output', help='output source map')
  parser.add_argument('-p', '--prefix', nargs='*', help='replace source debug filename prefix for source map', default=[])
  parser.add_argument('-s', '--sources', action='store_true', help='read and embed source files from file system into source map')
  parser.add_argument('-l', '--load-prefix', nargs='*', help='replace source debug filename prefix for reading sources from file system (see also --sources)', default=[])
  parser.add_argument('-w', nargs='?', help='set output wasm file')
  parser.add_argument('-x', '--strip', action='store_true', help='removes debug and linking sections')
  parser.add_argument('-u', '--source-map-url', nargs='?', help='specifies sourceMappingURL section content')
  parser.add_argument('--dwarfdump', help="path to llvm-dwarfdump executable")
  parser.add_argument('--dwarfdump-output', nargs='?', help=argparse.SUPPRESS)
  parser.add_argument('--basepath', help='base path for source files, which will be relative to this')
  return parser.parse_args(args)


class Prefixes:
  def __init__(self, args, base_path=None, preserve_deterministic_prefix=True):
    prefixes = []
    for p in args:
      if '=' in p:
        prefix, replacement = p.split('=')
        prefixes.append({'prefix': utils.normalize_path(prefix), 'replacement': replacement})
      else:
        prefixes.append({'prefix': utils.normalize_path(p), 'replacement': ''})
    self.base_path = utils.normalize_path(base_path) if base_path is not None else None
    self.preserve_deterministic_prefix = preserve_deterministic_prefix
    self.prefixes = prefixes
    self.cache = {}

  def resolve(self, name):
    if name in self.cache:
      return self.cache[name]

    source = name
    if not self.preserve_deterministic_prefix and name.startswith(DETERMINISTIC_PREFIX):
      source = EMSCRIPTEN_PREFIX + name.removeprefix(DETERMINISTIC_PREFIX)

    provided = False
    for p in self.prefixes:
      if source.startswith(p['prefix']):
        source = p['replacement'] + source.removeprefix(p['prefix'])
        provided = True
        break

    # If prefixes were provided, we use that; otherwise if base_path is set, we
    # emit a relative path. For files with deterministic prefix, we never use
    # a relative path, precisely to preserve determinism, and because it would
    # still point to the wrong location, so we leave the filepath untouched to
    # let users map it to the proper location using prefix options.
    if not (source.startswith(DETERMINISTIC_PREFIX) or provided or self.base_path is None):
      try:
        source = os.path.relpath(source, self.base_path)
      except ValueError:
        source = os.path.abspath(source)
      source = utils.normalize_path(source)

    self.cache[name] = source
    return source


# SourceMapPrefixes contains resolver for file names that are:
#  - "sources" is for names that output to source maps JSON
#  - "load" is for paths that used to load source text
class SourceMapPrefixes:
  def __init__(self, sources, load, base_path):
    self.sources = Prefixes(sources, base_path=base_path)
    self.load = Prefixes(load, preserve_deterministic_prefix=False)


def encode_vlq(n):
  VLQ_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
  x = (n << 1) if n >= 0 else ((-n << 1) + 1)
  result = ""
  while x > 31:
    result = result + VLQ_CHARS[32 + (x & 31)]
    x = x >> 5
  return result + VLQ_CHARS[x]


def read_var_uint(wasm, pos):
  n = 0
  shift = 0
  b = ord(wasm[pos:pos + 1])
  pos = pos + 1
  while b >= 128:
    n = n | ((b - 128) << shift)
    b = ord(wasm[pos:pos + 1])
    pos = pos + 1
    shift += 7
  return n + (b << shift), pos


def strip_debug_sections(wasm):
  logger.debug('Strip debug sections')
  pos = 8
  stripped = wasm[:pos]

  while pos < len(wasm):
    section_start = pos
    section_id, pos_ = read_var_uint(wasm, pos)
    section_size, section_body = read_var_uint(wasm, pos_)
    pos = section_body + section_size
    if section_id == 0:
      name_len, name_pos = read_var_uint(wasm, section_body)
      name_end = name_pos + name_len
      name = wasm[name_pos:name_end]
      if name in {'linking', 'sourceMappingURL'} or name.startswith(('reloc..debug_', '.debug_')):
        continue  # skip debug related sections
    stripped = stripped + wasm[section_start:pos]

  return stripped


def encode_uint_var(n):
  result = bytearray()
  while n > 127:
    result.append(128 | (n & 127))
    n = n >> 7
  result.append(n)
  return bytes(result)


def append_source_mapping(wasm, url):
  logger.debug('Append sourceMappingURL section')
  section_name = "sourceMappingURL"
  section_content = encode_uint_var(len(section_name)) + section_name.encode() + encode_uint_var(len(url)) + url.encode()
  return wasm + encode_uint_var(0) + encode_uint_var(len(section_content)) + section_content


def get_code_section_offset(wasm):
  logger.debug('Read sections index')
  pos = 8

  while pos < len(wasm):
    section_id, pos_ = read_var_uint(wasm, pos)
    section_size, pos = read_var_uint(wasm, pos_)
    if section_id == 10:
      return pos
    pos = pos + section_size


def remove_dead_entries(entries):
  # Remove entries for dead functions. It is a heuristics to ignore data if the
  # function starting address near to 0 (is equal to its size field length).
  block_start = 0
  cur_entry = 0
  while cur_entry < len(entries):
    if not entries[cur_entry]['eos']:
      cur_entry += 1
      continue
    fn_start = entries[block_start]['address']
    # Calculate the LEB encoded function size (including size field)
    fn_size_length = floor(log(entries[cur_entry]['address'] - fn_start + 1, 128)) + 1
    min_live_offset = 1 + fn_size_length # 1 byte is for code section entries
    if fn_start < min_live_offset:
      # Remove dead code debug info block.
      del entries[block_start:cur_entry + 1]
      cur_entry = block_start
      continue
    cur_entry += 1
    block_start = cur_entry


# Given a string that has non-ASCII UTF-8 bytes 128-255 stored as octal sequences (\200 - \377), decode
# the sequences back to UTF-8. E.g. "C:\\\303\244 \303\266\\emsdk\\emscripten\\main" -> "C:\\ä ö\\emsdk\\emscripten\\main"
def decode_octal_encoded_utf8(str):
  out = bytearray(len(str))
  i = 0
  o = 0
  final_length = len(str)
  in_escape = False
  while i < len(str):
    if not in_escape and str[i] == '\\' and (str[i + 1] == '2' or str[i + 1] == '3'):
      out[o] = int(str[i + 1:i + 4], 8)
      i += 4
      final_length -= 3
      in_escape = False
    else:
      out[o] = ord(str[i])
      in_escape = False if in_escape else (str[i] == '\\')
      i += 1
    o += 1
  return out[:final_length].decode('utf-8')


def extract_comp_dir_map(text):
  compile_unit_pattern = re.compile(r"0x[0-9a-f]*: DW_TAG_compile_unit")
  stmt_list_pattern = re.compile(r"DW_AT_stmt_list\s+\((0x[0-9a-f]*)\)")
  comp_dir_pattern = re.compile(r"DW_AT_comp_dir\s+\(\"([^\"]+)\"\)")

  map_stmt_list_to_comp_dir = {}
  iterator = compile_unit_pattern.finditer(text)
  current_match = next(iterator, None)

  while current_match:
    next_match = next(iterator, None)
    start = current_match.end()
    end = next_match.start() if next_match else len(text)

    stmt_list_match = stmt_list_pattern.search(text, start, end)
    if stmt_list_match is not None:
      stmt_list = stmt_list_match.group(1)
      comp_dir_match = comp_dir_pattern.search(text, start, end)
      comp_dir = decode_octal_encoded_utf8(comp_dir_match.group(1)) if comp_dir_match is not None else ''
      map_stmt_list_to_comp_dir[stmt_list] = comp_dir

    current_match = next_match

  return map_stmt_list_to_comp_dir


def demangle_names(names):
  # Only demangle names that look mangled
  mangled_names = sorted({n for n in names if n.startswith('_Z')})
  if not mangled_names:
    return {}
  if not os.path.exists(LLVM_CXXFILT):
    logger.warning('llvm-cxxfilt does not exist')
    return {}

  # Gather all mangled names and call llvm-cxxfilt only once for all of them
  input_str = '\n'.join(mangled_names)
  proc = shared.check_call([LLVM_CXXFILT], input=input_str, stdout=shared.PIPE, stderr=shared.PIPE, text=True)
  if proc.returncode != 0:
    logger.warning('llvm-cxxfilt failed: %s' % proc.stderr)
    return {}

  demangled_list = proc.stdout.splitlines()
  if len(demangled_list) != len(mangled_names):
    logger.warning('llvm-cxxfilt output length mismatch')
    return {}

  return dict(zip(mangled_names, demangled_list, strict=True))


class FuncRange:
  def __init__(self, name, low_pc, high_pc):
    self.name = name
    self.low_pc = low_pc
    self.high_pc = high_pc


# This function parses DW_TAG_subprogram entries and gets low_pc and high_pc for
# each function in a list of FuncRanges. The result list will be sorted in the
# increasing order of low_pcs.
def extract_func_ranges(text):
  # This function handles four cases:
  # 1. DW_TAG_subprogram with DW_AT_name, DW_AT_low_pc, and DW_AT_high_pc.
  # 0x000000ba:   DW_TAG_subprogram
  #                 DW_AT_low_pc  (0x0000005f)
  #                 DW_AT_high_pc  (0x00000071)
  #                 DW_AT_name  ("foo")
  #                 ...
  #
  # 2. DW_TAG_subprogram with DW_AT_linkage_name, DW_AT_low_pc, and
  #    DW_AT_high_pc. Applies to mangled C++ functions.
  #    (We parse DW_AT_linkage_name instead of DW_AT_name here.)
  # 0x000000ba:   DW_TAG_subprogram
  #                 DW_AT_low_pc  (0x0000005f)
  #                 DW_AT_high_pc  (0x00000071)
  #                 DW_AT_linkage_name  ("_ZN7MyClass3fooEv")
  #                 DW_AT_name  ("foo")
  #                 ...
  #
  # 3. DW_TAG_subprogram with DW_AT_specification, DW_AT_low_pc, and
  #    DW_AT_high_pc. C++ function info can be split into two DIEs (one with
  #    DW_AT_linkage_name and  DW_AT_declaration (true) and the other with
  #    DW_AT_specification). In this case we parse DW_AT_specification for the
  #    function name.
  # 0x0000006d:   DW_TAG_subprogram
  #                 DW_AT_linkage_name  ("_ZN7MyClass3fooEv")
  #                 DW_AT_name  ("foo")
  #                 DW_AT_declaration  (true)
  #                 ...
  # 0x00000097:   DW_TAG_subprogram
  #                 DW_AT_low_pc  (0x00000007)
  #                 DW_AT_high_pc  (0x0000004c)
  #                 DW_AT_specification  (0x0000006d "_ZN7MyClass3fooEv")
  #                 ...
  #
  # 4. DW_TAG_inlined_subroutine with DW_AT_abstract_origin, DW_AT_low_pc, and
  #    DW_AT_high_pc. This represents an inlined function. We parse
  #    DW_AT_abstract_origin for the original function name.
  # 0x0000011a:   DW_TAG_inlined_subroutine
  #                 DW_AT_abstract_origin  (0x000000da "_ZN7MyClass3barEv")
  #                 DW_AT_low_pc  (0x00000078)
  #                 DW_AT_high_pc  (0x00000083)
  #                 ...

  # Pattern to find the start of the NEXT DWARF tag (boundary marker)
  next_tag_pattern = re.compile(r'\n0x[0-9a-f]+:')
  # Pattern to find DWARF tags for functions (Subprogram or Inlined) directly
  func_pattern = re.compile(r'DW_TAG_(?:subprogram|inlined_subroutine)')

  low_pc_pattern = re.compile(r'DW_AT_low_pc\s+\(0x([0-9a-f]+)\)')
  high_pc_pattern = re.compile(r'DW_AT_high_pc\s+\(0x([0-9a-f]+)\)')
  abstract_origin_pattern = re.compile(r'DW_AT_abstract_origin\s+\(0x[0-9a-f]+\s+"([^"]+)"\)')
  linkage_name_pattern = re.compile(r'DW_AT_linkage_name\s+\("([^"]+)"\)')
  name_pattern = re.compile(r'DW_AT_name\s+\("([^"]+)"\)')
  specification_pattern = re.compile(r'DW_AT_specification\s+\(0x[0-9a-f]+\s+"([^"]+)"\)')

  def get_name_from_tag(start, end):
    m = linkage_name_pattern.search(text, start, end)
    if m:
      return m.group(1)
    m = name_pattern.search(text, start, end)
    if m:
      return m.group(1)
    # If name is missing, check for DW_AT_specification annotation
    m = specification_pattern.search(text, start, end)
    if m:
      return m.group(1)
    return None

  func_ranges = []
  for match in func_pattern.finditer(text):
    # Search from the end of the tag name (e.g. after "DW_TAG_subprogram").
    # Attributes are expected to follow.
    search_start = match.end()

    # Search until the beginning of the next tag
    m_next = next_tag_pattern.search(text, search_start)
    search_end = m_next.start() if m_next else len(text)

    name = None
    low_pc = None
    high_pc = None
    m = low_pc_pattern.search(text, search_start, search_end)
    if m:
      low_pc = int(m.group(1), 16)
    m = high_pc_pattern.search(text, search_start, search_end)
    if m:
      high_pc = int(m.group(1), 16)

    if 'DW_TAG_subprogram' in match.group(0):
      name = get_name_from_tag(search_start, search_end)
    else: # is_inlined
      m = abstract_origin_pattern.search(text, search_start, search_end)
      if m:
        name = m.group(1)

    if name and low_pc is not None and high_pc is not None:
      func_ranges.append(FuncRange(name, low_pc, high_pc))

  # Demangle names
  all_names = [item.name for item in func_ranges]
  demangled_map = demangle_names(all_names)
  for func_range in func_ranges:
    if func_range.name in demangled_map:
      func_range.name = demangled_map[func_range.name]

  # To correctly identify the innermost function for a given address,
  # func_ranges is sorted primarily by low_pc in ascending order and secondarily
  # by high_pc in descending order. This ensures that for overlapping ranges,
  # the more specific (inner) range appears later in the list.
  func_ranges.sort(key=lambda item: (item.low_pc, -item.high_pc))
  return func_ranges


def read_dwarf_info(wasm, options):
  if options.dwarfdump_output:
    output = utils.read_file(options.dwarfdump_output)
  elif options.dwarfdump:
    logger.debug('Reading DWARF information from %s' % wasm)
    if not os.path.exists(options.dwarfdump):
      utils.exit_with_error('llvm-dwarfdump not found: ' + options.dwarfdump)
    # We need only three tags in the debug info: DW_TAG_compile_unit for
    # source location, and DW_TAG_subprogram and DW_TAG_inlined_subroutine
    # for the function ranges.
    dwarfdump_cmd = [options.dwarfdump, '-debug-info', '-debug-line', wasm,
                     '-t', 'DW_TAG_compile_unit', '-t', 'DW_TAG_subprogram',
                     '-t', 'DW_TAG_inlined_subroutine']
    proc = shared.check_call(dwarfdump_cmd, stdout=shared.PIPE)
    output = proc.stdout
  else:
    utils.exit_with_error('Please specify either --dwarfdump or --dwarfdump-output')

  debug_line_pattern = re.compile(r"debug_line\[(0x[0-9a-f]*)\]")
  include_dir_pattern = re.compile(r"include_directories\[\s*(\d+)\] = \"([^\"]*)")
  file_pattern = re.compile(r"file_names\[\s*(\d+)\]:\s+name: \"([^\"]*)\"\s+dir_index: (\d+)")
  line_pattern = re.compile(r"\n0x([0-9a-f]+)\s+(\d+)\s+(\d+)\s+(\d+)(.*?end_sequence)?")

  entries = []
  iterator = debug_line_pattern.finditer(output)
  try:
    current_match = next(iterator)
    debug_info_end = current_match.start() # end of .debug_info contents
  except StopIteration:
    debug_info_end = len(output)

  debug_info = output[:debug_info_end] # .debug_info contents
  map_stmt_list_to_comp_dir = extract_comp_dir_map(debug_info)

  while current_match:
    next_match = next(iterator, None)

    stmt_list = current_match.group(1)
    start = current_match.end()
    end = next_match.start() if next_match else len(output)

    comp_dir = map_stmt_list_to_comp_dir.get(stmt_list, '')

    # include_directories[  1] = "/Users/yury/Work/junk/sqlite-playground/src"
    # file_names[  1]:
    #            name: "playground.c"
    #       dir_index: 1
    #        mod_time: 0x00000000
    #          length: 0x00000000
    #
    # Address            Line   Column File   ISA Discriminator Flags
    # ------------------ ------ ------ ------ --- ------------- -------------
    # 0x0000000000000006     22      0      1   0             0  is_stmt
    # 0x0000000000000007     23     10      1   0             0  is_stmt prologue_end
    # 0x000000000000000f     23      3      1   0             0
    # 0x0000000000000010     23      3      1   0             0  end_sequence
    # 0x0000000000000011     28      0      1   0             0  is_stmt

    include_directories = {'0': comp_dir}
    for dir in include_dir_pattern.finditer(output, start, end):
      include_directories[dir.group(1)] = os.path.join(comp_dir, decode_octal_encoded_utf8(dir.group(2)))

    files = {}
    for file in file_pattern.finditer(output, start, end):
      dir = include_directories[file.group(3)]
      file_path = os.path.join(dir, decode_octal_encoded_utf8(file.group(2)))
      files[file.group(1)] = file_path

    for line in line_pattern.finditer(output, start, end):
      entry = {'address': int(line.group(1), 16), 'line': int(line.group(2)), 'column': int(line.group(3)), 'file': files[line.group(4)], 'eos': line.group(5) is not None}
      if not entry['eos']:
        entries.append(entry)
      else:
        # move end of function to the last END operator
        entry['address'] -= 1
        if entries[-1]['address'] == entry['address']:
          # last entry has the same address, reusing
          entries[-1]['eos'] = True
        else:
          entries.append(entry)

    current_match = next_match

  remove_dead_entries(entries)

  # return entries sorted by the address field
  entries = sorted(entries, key=lambda entry: entry['address'])

  func_ranges = extract_func_ranges(debug_info)
  return entries, func_ranges


def build_sourcemap(entries, func_ranges, code_section_offset, options):
  base_path = options.basepath
  collect_sources = options.sources
  prefixes = SourceMapPrefixes(options.prefix, options.load_prefix, base_path)

  # Add code section offset to the low/high pc in the function PC ranges
  for func_range in func_ranges:
    func_range.low_pc += code_section_offset
    func_range.high_pc += code_section_offset

  sources = []
  sources_content = []
  # There can be duplicate names in case an original source function has
  # multiple disjoint PC ranges or is inlined to multiple callsites. Make the
  # 'names' list a unique list of names, and map the function ranges to the
  # indices in that list.
  names = sorted({item.name for item in func_ranges})
  name_to_id = {name: i for i, name in enumerate(names)}
  mappings = []
  sources_map = {}
  last_address = 0
  last_source_id = 0
  last_line = 1
  last_column = 1
  last_func_id = 0

  active_funcs = []
  next_func_range_id = 0

  # Get the function name ID that the given address falls into
  def get_function_id(address):
    nonlocal active_funcs
    nonlocal next_func_range_id

    # Maintain a list of "active functions" whose ranges currently cover the
    # address. As the address advances, it adds new functions that start and
    # removes functions that end. The last function remaining in the active list
    # at any point is the innermost function.
    while next_func_range_id < len(func_ranges) and func_ranges[next_func_range_id].low_pc <= address:
      # active_funcs contains (high_pc, id) pair
      active_funcs.append((func_ranges[next_func_range_id].high_pc, next_func_range_id))
      next_func_range_id += 1
    active_funcs = [f for f in active_funcs if f[0] > address]

    if active_funcs:
      func_range_id = active_funcs[-1][1]
      name = func_ranges[func_range_id].name
      return name_to_id[name]
    return None

  for entry in entries:
    line = entry['line']
    column = entry['column']
    # ignore entries with line 0
    if line == 0:
      continue
    # start at least at column 1
    if column == 0:
      column = 1

    address = entry['address'] + code_section_offset
    file_name = utils.normalize_path(entry['file'])
    source_name = prefixes.sources.resolve(file_name)

    if source_name not in sources_map:
      source_id = len(sources)
      sources_map[source_name] = source_id
      sources.append(source_name)
      if collect_sources:
        load_name = prefixes.load.resolve(file_name)
        try:
          with open(load_name) as infile:
            source_content = infile.read()
          sources_content.append(source_content)
        except OSError:
          print('Failed to read source: %s' % load_name)
          sources_content.append(None)
    else:
      source_id = sources_map[source_name]
    func_id = get_function_id(address)

    address_delta = address - last_address
    source_id_delta = source_id - last_source_id
    line_delta = line - last_line
    column_delta = column - last_column
    last_address = address
    last_source_id = source_id
    last_line = line
    last_column = column
    mapping = encode_vlq(address_delta) + encode_vlq(source_id_delta) + encode_vlq(line_delta) + encode_vlq(column_delta)
    if func_id is not None:
      func_id_delta = func_id - last_func_id
      last_func_id = func_id
      mapping += encode_vlq(func_id_delta)
    mappings.append(mapping)

  return {'version': 3,
          'sources': sources,
          'sourcesContent': sources_content,
          'names': names,
          'mappings': ','.join(mappings)}


def main(args):
  options = parse_args(args)

  wasm_input = options.wasm
  with open(wasm_input, 'rb') as infile:
    wasm = infile.read()

  entries, func_ranges = read_dwarf_info(wasm_input, options)

  code_section_offset = get_code_section_offset(wasm)

  logger.debug('Saving to %s' % options.output)
  map = build_sourcemap(entries, func_ranges, code_section_offset, options)
  with open(options.output, 'w', encoding='utf-8') as outfile:
    json.dump(map, outfile, separators=(',', ':'), ensure_ascii=False)

  if options.strip:
    wasm = strip_debug_sections(wasm)

  if options.source_map_url:
    wasm = append_source_mapping(wasm, options.source_map_url)

  if options.w:
    logger.debug('Saving wasm to %s' % options.w)
    with open(options.w, 'wb') as outfile:
      outfile.write(wasm)

  logger.debug('Done')
  return 0


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG if os.environ.get('EMCC_DEBUG') else logging.INFO)
  sys.exit(main(sys.argv[1:]))
