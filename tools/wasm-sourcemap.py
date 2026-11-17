#!/usr/bin/env python3
# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utility tools that extracts DWARF information from a WebAssembly file.

This script take the output produced by the LLVM tools, and encodes it as
a wasm source map. Additionally, it can collect original sources, change
files prefixes, and strip debug sections from a wasm file.
"""

import argparse
import bisect
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
  # FIXME: Generate Scopes info. For now this only reads the functions.
  parser.add_argument('--scopes', action='store_true', help=argparse.SUPPRESS)
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


def encode_unsigned_vlq(x):
  assert x >= 0
  VLQ_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
  result = ""
  while x > 31:
    result += VLQ_CHARS[32 + (x & 31)]
    x >>= 5
  return result + VLQ_CHARS[x]


def encode_vlq(n):
  return encode_unsigned_vlq((n << 1) if n >= 0 else ((-n << 1) + 1))


def read_var_uint(wasm, pos):
  n = 0
  shift = 0
  b = ord(wasm[pos:pos + 1])
  pos += 1
  while b >= 128:
    n |= ((b - 128) << shift)
    b = ord(wasm[pos:pos + 1])
    pos += 1
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
      name = str(wasm[name_pos:name_end])
      if name in {'linking', 'sourceMappingURL'} or name.startswith(('reloc..debug_', '.debug_')):
        continue  # skip debug related sections
    stripped += wasm[section_start:pos]

  return stripped


def encode_uint_var(n):
  result = bytearray()
  while n > 127:
    result.append(128 | (n & 127))
    n >>= 7
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
    pos += section_size


# The [start, end) of each function body, as offsets in the code section like
# DWARF addresses.
def get_function_bodies(wasm, code_section_offset):
  bodies = []
  count, pos = read_var_uint(wasm, code_section_offset)
  for _ in range(count):
    size, body = read_var_uint(wasm, pos)
    pos = body + size
    bodies.append((body - code_section_offset, pos - code_section_offset))
  return bodies


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
  mangled_names = sorted({n for n in names if n.startswith(('_Z', '_R'))})
  if not mangled_names:
    return {}
  if not os.path.exists(LLVM_CXXFILT):
    logger.warning('llvm-cxxfilt does not exist')
    return {}

  # Gather all mangled names and call llvm-cxxfilt only once for all of them
  input_str = '\n'.join(mangled_names)
  proc = shared.check_call([LLVM_CXXFILT], input=input_str, stdout=shared.PIPE, stderr=shared.PIPE, text=True)
  if proc.returncode != 0:
    logger.warning(f'llvm-cxxfilt failed: {proc.stderr}')
    return {}

  demangled_list = proc.stdout.splitlines()
  if len(demangled_list) != len(mangled_names):
    logger.warning('llvm-cxxfilt output length mismatch')
    return {}

  return dict(zip(mangled_names, demangled_list, strict=True))


class FuncRange:
  def __init__(self, name, low_pc, high_pc, die):
    self.name = name
    self.low_pc = low_pc
    self.high_pc = high_pc
    # The DIE of the function (see extract_func_ranges). A function with
    # several code ranges has a FuncRange for each.
    self.die = die
    # The range this one is inlined into, and the ranges inlined into it
    self.parent = None
    self.children = []


def die_string(die, attr):
  # A file index that the line table doesn't have reads (0x00000009)
  value = die.get(attr)
  if not value or value[0] != '"':
    return None
  if '\\' in value:
    value = decode_octal_encoded_utf8(value)
  return value[1:-1]


def die_number(die, attr):
  # Dead code reads "(dead code)"
  value = die.get(attr)
  return int(value, 0) if value and value[0].isdigit() else 0


# This function parses DW_TAG_subprogram entries and gets low_pc and high_pc for
# each function in a list of FuncRanges. The result list will be sorted in the
# increasing order of low_pcs.
def extract_func_ranges(text):
  # This function handles five cases:
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
  #    DW_AT_abstract_origin for the original function name, and
  #    DW_AT_call_file, DW_AT_call_line and DW_AT_call_column for the call.
  # 0x0000011a:   DW_TAG_inlined_subroutine
  #                 DW_AT_abstract_origin  (0x000000da "_ZN7MyClass3barEv")
  #                 DW_AT_low_pc  (0x00000078)
  #                 DW_AT_high_pc  (0x00000083)
  #                 DW_AT_call_file  ("/src/t.cpp")
  #                 DW_AT_call_line  (12)
  #                 DW_AT_call_column  (10)
  #                 ...
  #
  # 5. DW_TAG_subprogram or DW_TAG_inlined_subroutine with DW_AT_ranges
  #    instead of DW_AT_low_pc and DW_AT_high_pc, for code in several ranges.
  #                 DW_AT_ranges  (0x00000000
  #                    [0x00000002, 0x00000008)
  #                    [0x0000000a, 0x0000000e))
  #
  # Each DIE is a dict of its attributes, with its offset, its kind, the names
  # of the DW_TAG_namespace DIEs around it, the function it is in ('owner'),
  # and 'key', the offset of the DIE that describes its source function: the
  # end of the DW_AT_abstract_origin and DW_AT_specification chain, with its
  # declaration ('decl'). Functions in dead code (tombstoned addresses) have
  # no ranges.

  # Pattern to find DWARF tags (the dump has only the tags we need, see
  # read_dwarf_info)
  tag_pattern = re.compile(r'^0x([0-9a-f]+):( +)DW_TAG_(\w+)', re.MULTILINE)
  # DWARF 2 and 3 have DW_AT_MIPS_linkage_name
  attr_pattern = re.compile(r'DW_AT_(?:MIPS_)?(linkage_name|name|comp_dir|decl_file|decl_line|call_file|call_line|call_column)\s+\((.*)\)$', re.MULTILINE)
  low_pc_pattern = re.compile(r'DW_AT_low_pc\s+\(0x([0-9a-f]+)\)')
  high_pc_pattern = re.compile(r'DW_AT_high_pc\s+\(0x([0-9a-f]+)\)')
  ranges_pattern = re.compile(r'^\s+\[0x([0-9a-f]+), 0x([0-9a-f]+)\)', re.MULTILINE)
  origin_pattern = re.compile(r'DW_AT_(?:abstract_origin|specification)\s+\(0x([0-9a-f]+)')

  dies = {}
  funcs = []
  func_ranges = []
  # (depth, name) of the DW_TAG_namespace DIEs around the current one
  namespaces = []
  # The function the following inlined calls are in
  owner = None
  # The main source file of the current compile unit
  unit_file = None

  def attributes(start, end):
    return {'DW_AT_' + attr: value for attr, value in attr_pattern.findall(text, start, end)}

  def parse_tag(match, end):
    nonlocal owner, unit_file
    depth = len(match.group(2))
    tag = match.group(3)
    while namespaces and namespaces[-1][0] >= depth:
      namespaces.pop()
    start = match.end()
    if tag in {'compile_unit', 'skeleton_unit'}:
      unit = attributes(start, end)
      unit_file = os.path.join(die_string(unit, 'DW_AT_comp_dir') or '', die_string(unit, 'DW_AT_name') or '')
      # Nothing carries over from the previous compile unit
      namespaces.clear()
      owner = None
      return
    if tag == 'namespace':
      # An anonymous namespace has no name; call it what llvm-cxxfilt does, so
      # functions without a linkage name get the same names as the others
      name = die_string(attributes(start, end), 'DW_AT_name') or '(anonymous namespace)'
      namespaces.append((depth, name))
      return
    if tag not in {'subprogram', 'inlined_subroutine'}:
      return
    die = attributes(start, end)
    die['offset'] = int(match.group(1), 16)
    die['unit_file'] = unit_file
    die['inlined'] = tag == 'inlined_subroutine'
    die['namespaces'] = [name for _, name in namespaces]
    m = origin_pattern.search(text, start, end)
    die['origin'] = int(m.group(1), 16) if m else None
    dies[die['offset']] = die

    ranges = [(int(low, 16), int(high, 16)) for low, high in ranges_pattern.findall(text, start, end)]
    low_pc = low_pc_pattern.search(text, start, end)
    high_pc = high_pc_pattern.search(text, start, end)
    if low_pc and high_pc:
      ranges.append((int(low_pc.group(1), 16), int(high_pc.group(1), 16)))
    # Merge the ranges that touch, and drop dead code
    merged = []
    for low, high in sorted(r for r in ranges if 0 < r[0] < r[1] <= 0xffffffff):
      if merged and low <= merged[-1][1]:
        merged[-1] = (merged[-1][0], max(high, merged[-1][1]))
      else:
        merged.append((low, high))
    if not die['inlined'] and (ranges or high_pc):
      owner = die if merged else None
    die['owner'] = owner
    if merged and owner:
      funcs.append(die)
      func_ranges.extend(FuncRange(None, low, high, die) for low, high in merged)

  prev = None
  for match in tag_pattern.finditer(text):
    if prev:
      parse_tag(prev, match.start())
    prev = match
  if prev:
    parse_tag(prev, len(text))

  # The end of the chain is the declaration, e.g. in a class. The definition
  # on the way to it has the linkage name of lambdas and local classes and
  # the declaration line of out-of-line methods, so take each attribute from
  # the nearest DIE that has it (clang omits DW_AT_decl_file when unchanged).
  missing = None
  for func in funcs:
    nearest = {}
    die = func
    for _ in range(16):
      for attr in ('DW_AT_linkage_name', 'DW_AT_decl_file', 'DW_AT_decl_line'):
        if attr not in nearest and attr in die:
          nearest[attr] = die[attr]
      origin = dies.get(die['origin'])
      if die['origin'] is not None and not origin:
        missing = missing or die['origin']
      if not origin or origin['inlined']:
        break
      die = origin
    func['key'] = die['offset']
    die.update(nearest)
  if missing:
    # The DIE is in a tag that read_dwarf_info doesn't ask llvm-dwarfdump for
    logger.warning(f'scopes: function declarations not found (e.g. DIE 0x{missing:08x}), so some functions have no name')
  if re.search(r'DW_TAG_skeleton_unit|DW_AT_(?:GNU_)?dwo_name', text):
    logger.warning('scopes: functions in split DWARF (-gsplit-dwarf) are not included')

  # Name each source function after its demangled linkage name, or else its
  # DW_AT_name in its namespaces ("main" has the linkage name
  # "__main_argc_argv").
  keys = dict.fromkeys(func['key'] for func in funcs)
  linkage_names = {key: die_string(dies[key], 'DW_AT_linkage_name') for key in keys}
  demangled_map = demangle_names(n for n in linkage_names.values() if n)
  names = {}
  decls = {}
  for key in keys:
    die = dies[key]
    linkage_name = linkage_names[key]
    name = die_string(die, 'DW_AT_name')
    if name and die['namespaces']:
      name = '::'.join([*die['namespaces'], name])
    names[key] = demangled_map.get(linkage_name) or name or linkage_name
    decl_file = die_string(die, 'DW_AT_decl_file')
    if decl_file and die_number(die, 'DW_AT_decl_line'):
      decls[key] = (decl_file, die_number(die, 'DW_AT_decl_line') - 1)
  for func in funcs:
    func['decl'] = decls.get(func['key'])
  for func_range in func_ranges:
    func_range.name = names[func_range.die['key']]

  # To correctly identify the innermost function for a given address,
  # func_ranges is sorted primarily by low_pc in ascending order and secondarily
  # by DIE offset. This ensures that for overlapping ranges, the more specific
  # (inner) range appears later in the list, even when they start together.
  func_ranges.sort(key=lambda item: (item.low_pc, item.die['offset']))
  return func_ranges


# The ranges of the functions in the code: for each function body, those of
# the DIE whose range is the body, or else of the first one inside it. C++
# inline functions are in the DWARF of each compile unit that uses them, and
# the linker points the copies it drops, and weak definitions that a strong
# one overrides, at the code it keeps, with their own sizes.
def code_func_ranges(func_ranges, function_bodies):
  starts = [start for start, _ in function_bodies]
  candidates = []
  best = {}
  for func_range in func_ranges:
    func = func_range.die
    body = bisect.bisect_right(starts, func_range.low_pc) - 1
    if func['inlined'] or body < 0 or func_range.high_pc > function_bodies[body][1]:
      continue
    candidates.append((body, func_range))
    rank = ((func_range.low_pc, func_range.high_pc) != function_bodies[body], func['offset'])
    best[body] = min(best.get(body, rank), rank)
  return {id(func_range) for body, func_range in candidates if func_range.die['offset'] == best[body][1]}


# Nests each inlined range in the innermost range of its function around its
# start, where DWARF has its parent DIE, and cuts it at that range's end.
# The function ranges that code_func_ranges doesn't pick are dropped with the
# calls inlined into them. Returns the ranges of the functions, and all the
# ranges that are kept.
def nest_func_ranges(func_ranges, function_bodies):
  code_ranges = code_func_ranges(func_ranges, function_bodies)
  top = []
  kept = []
  # The ranges of each function that are around the current address
  active_funcs = {}
  last_end = 0
  for func_range in func_ranges:
    func = func_range.die
    active = active_funcs.setdefault(func['owner']['offset'], [])
    active[:] = [f for f in active if f.high_pc > func_range.low_pc]
    if func['inlined']:
      if not active:
        continue
      func_range.parent = active[-1]
      func_range.high_pc = min(func_range.high_pc, func_range.parent.high_pc)
      func_range.parent.children.append(func_range)
      func.setdefault('parent', func_range.parent.die)
    elif id(func_range) not in code_ranges or func_range.low_pc < last_end:
      continue
    else:
      top.append(func_range)
      last_end = func_range.high_pc
    active.append(func_range)
    kept.append(func_range)
  return top, kept


def read_dwarf_info(wasm, options):
  if options.dwarfdump_output:
    output = utils.read_file(options.dwarfdump_output)
  elif options.dwarfdump:
    logger.debug(f'Reading DWARF information from {wasm}')
    if not os.path.exists(options.dwarfdump):
      utils.exit_with_error('llvm-dwarfdump not found: ' + options.dwarfdump)
    dwarfdump_cmd = [options.dwarfdump, '-debug-info', '-debug-line', wasm]
    if options.scopes:
      # llvm-dwarfdump only shows the children of the DIEs it shows, so ask
      # for the functions and every tag LLVM puts them in (the scopes of
      # DISubprograms):
      #  - compile_unit: the source file (skeleton_unit: split DWARF)
      #  - subprogram, inlined_subroutine: functions and inlined calls, and
      #    lexical_block: inlined calls in a block
      #  - namespace and module: functions in a namespace (Rust paths) or a
      #    module (Clang modules, Fortran)
      #  - class_type, structure_type, union_type, enumeration_type (methods
      #    of C-like Rust enums) and interface_type: method declarations
      # extract_func_ranges warns when a declaration is missing.
      for tag in ['compile_unit', 'skeleton_unit', 'subprogram', 'inlined_subroutine', 'lexical_block',
                  'namespace', 'module', 'class_type', 'structure_type', 'union_type', 'enumeration_type',
                  'interface_type']:
        dwarfdump_cmd += ['-t', 'DW_TAG_' + tag]
    else:
      # We only need the top-level DW_TAG_compile_unit tags when not generating
      # scopes
      dwarfdump_cmd += ['--recurse-depth=0']
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
  current_match = None
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

  if options.scopes:
    func_ranges = extract_func_ranges(debug_info)
  else:
    func_ranges = []
  return entries, func_ranges


def build_sourcemap(entries, func_ranges, code_section_offset, options, function_bodies=()):
  base_path = options.basepath
  collect_sources = options.sources
  prefixes = SourceMapPrefixes(options.prefix, options.load_prefix, base_path)

  sources = []
  sources_content = []
  mappings = []
  sources_map = {}
  last_address = 0
  last_source_id = 0
  last_line = 1
  last_column = 1
  last_was_unmapped = False

  for entry in entries:
    line = entry['line']
    column = entry['column']
    # line 0 means no source location; end the previous mapping with a
    # segment that has only the offset, which the spec uses for unmapped
    # code. The other fields are deltas, so they keep their last values.
    if line == 0:
      if mappings and not last_was_unmapped:
        address = entry['address'] + code_section_offset
        mappings.append(encode_vlq(address - last_address))
        last_address = address
        last_was_unmapped = True
      continue
    last_was_unmapped = False
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
          source_content = utils.read_file(load_name)
          sources_content.append(source_content)
        except OSError:
          print(f'Failed to read source: {load_name}')
          sources_content.append(None)
    else:
      source_id = sources_map[source_name]

    address_delta = address - last_address
    source_id_delta = source_id - last_source_id
    line_delta = line - last_line
    column_delta = column - last_column
    last_address = address
    last_source_id = source_id
    last_line = line
    last_column = column
    mappings.append(encode_vlq(address_delta) + encode_vlq(source_id_delta) + encode_vlq(line_delta) + encode_vlq(column_delta))

  if func_ranges:
    top, kept = nest_func_ranges(func_ranges, function_bodies)
    logger.debug(f'{len(top)} functions with code, {len(kept) - len(top)} inlined calls')

  return {'version': 3,
          'sources': sources,
          'sourcesContent': sources_content,
          'names': [],
          'mappings': ','.join(mappings)}


def main(args):
  options = parse_args(args)

  wasm_input = options.wasm
  with open(wasm_input, 'rb') as infile:
    wasm = infile.read()

  entries, func_ranges = read_dwarf_info(wasm_input, options)

  code_section_offset = get_code_section_offset(wasm)
  function_bodies = get_function_bodies(wasm, code_section_offset) if func_ranges else []

  logger.debug(f'Saving to {options.output}')
  map = build_sourcemap(entries, func_ranges, code_section_offset, options, function_bodies)
  with open(options.output, 'w', encoding='utf-8') as outfile:
    json.dump(map, outfile, separators=(',', ':'), ensure_ascii=False)

  if options.strip:
    wasm = strip_debug_sections(wasm)

  if options.source_map_url:
    wasm = append_source_mapping(wasm, options.source_map_url)

  if options.w:
    logger.debug(f'Saving wasm to {options.w}')
    with open(options.w, 'wb') as outfile:
      outfile.write(wasm)

  logger.debug('Done')
  return 0


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG if os.environ.get('EMCC_DEBUG') else logging.INFO)
  sys.exit(main(sys.argv[1:]))
