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
from math import floor, log
import os
import re
from subprocess import Popen, PIPE
from pathlib import Path
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools import utils

logger = logging.getLogger('wasm-sourcemap')


def parse_args():
  parser = argparse.ArgumentParser(prog='wasm-sourcemap.py', description=__doc__)
  parser.add_argument('wasm', help='wasm file')
  parser.add_argument('-o', '--output', help='output source map')
  parser.add_argument('-p', '--prefix', nargs='*', help='replace source debug filename prefix for source map', default=[])
  parser.add_argument('-s', '--sources', action='store_true', help='read and embed source files from file system into source map')
  parser.add_argument('-l', '--load-prefix', nargs='*', help='replace source debug filename prefix for reading sources from file system (see also --sources)', default=[])
  parser.add_argument('-w', nargs='?', help='set output wasm file')
  parser.add_argument('-x', '--strip', action='store_true', help='removes debug and linking sections')
  parser.add_argument('-u', '--source-map-url', nargs='?', help='specifies sourceMappingURL section contest')
  parser.add_argument('--dwarfdump', help="path to llvm-dwarfdump executable")
  parser.add_argument('--dwarfdump-output', nargs='?', help=argparse.SUPPRESS)
  parser.add_argument('--basepath', help='base path for source files, which will be relative to this')
  return parser.parse_args()


class Prefixes:
  def __init__(self, args):
    prefixes = []
    for p in args:
      if '=' in p:
        prefix, replacement = p.split('=')
        prefixes.append({'prefix': prefix, 'replacement': replacement})
      else:
        prefixes.append({'prefix': p, 'replacement': None})
    self.prefixes = prefixes
    self.cache = {}

  def resolve(self, name):
    if name in self.cache:
      return self.cache[name]

    for p in self.prefixes:
      if name.startswith(p['prefix']):
        if p['replacement'] is None:
          result = utils.removeprefix(name, p['prefix'])
        else:
          result = p['replacement'] + utils.removeprefix(name, p['prefix'])
        break
    self.cache[name] = result
    return result


# SourceMapPrefixes contains resolver for file names that are:
#  - "sources" is for names that output to source maps JSON
#  - "load" is for paths that used to load source text
class SourceMapPrefixes:
  def __init__(self, sources, load):
    self.sources = sources
    self.load = load

  def provided(self):
    return bool(self.sources.prefixes or self.load.prefixes)


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
      if name == "linking" or name == "sourceMappingURL" or name.startswith("reloc..debug_") or name.startswith(".debug_"):
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


def extract_comp_dir_map(text):
  map_stmt_list_to_comp_dir = {}
  chunks = re.split(r"0x[0-9a-f]*: DW_TAG_compile_unit", text)
  for chunk in chunks[1:]:
    stmt_list_match = re.search(r"DW_AT_stmt_list\s+\((0x[0-9a-f]*)\)", chunk)
    if stmt_list_match is not None:
      stmt_list = stmt_list_match.group(1)
      comp_dir_match = re.search(r"DW_AT_comp_dir\s+\(\"([^\"]+)\"\)", chunk)
      comp_dir = comp_dir_match.group(1) if comp_dir_match is not None else ''
      map_stmt_list_to_comp_dir[stmt_list] = comp_dir
  return map_stmt_list_to_comp_dir


def read_dwarf_entries(wasm, options):
  if options.dwarfdump_output:
    output = Path(options.dwarfdump_output).read_bytes()
  elif options.dwarfdump:
    logger.debug('Reading DWARF information from %s' % wasm)
    if not os.path.exists(options.dwarfdump):
      logger.error('llvm-dwarfdump not found: ' + options.dwarfdump)
      sys.exit(1)
    process = Popen([options.dwarfdump, '-debug-info', '-debug-line', '--recurse-depth=0', wasm], stdout=PIPE)
    output, err = process.communicate()
    exit_code = process.wait()
    if exit_code != 0:
      logger.error('Error during llvm-dwarfdump execution (%s)' % exit_code)
      sys.exit(1)
  else:
    logger.error('Please specify either --dwarfdump or --dwarfdump-output')
    sys.exit(1)

  entries = []
  debug_line_chunks = re.split(r"debug_line\[(0x[0-9a-f]*)\]", output.decode('utf-8'))
  map_stmt_list_to_comp_dir = extract_comp_dir_map(debug_line_chunks[0])
  for stmt_list, line_chunk in zip(debug_line_chunks[1::2], debug_line_chunks[2::2]):
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
    for dir in re.finditer(r"include_directories\[\s*(\d+)\] = \"([^\"]*)", line_chunk):
      include_directories[dir.group(1)] = os.path.join(comp_dir, dir.group(2))

    files = {}
    for file in re.finditer(r"file_names\[\s*(\d+)\]:\s+name: \"([^\"]*)\"\s+dir_index: (\d+)", line_chunk):
      dir = include_directories[file.group(3)]
      file_path = os.path.join(dir, file.group(2))
      files[file.group(1)] = file_path

    for line in re.finditer(r"\n0x([0-9a-f]+)\s+(\d+)\s+(\d+)\s+(\d+)(.*?end_sequence)?", line_chunk):
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

  remove_dead_entries(entries)

  # return entries sorted by the address field
  return sorted(entries, key=lambda entry: entry['address'])


def build_sourcemap(entries, code_section_offset, prefixes, collect_sources, base_path):
  sources = []
  sources_content = [] if collect_sources else None
  mappings = []
  sources_map = {}
  last_address = 0
  last_source_id = 0
  last_line = 1
  last_column = 1
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
    file_name = entry['file']
    file_name = utils.normalize_path(file_name)
    # if prefixes were provided, we use that; otherwise, we emit a relative
    # path
    if prefixes.provided():
      source_name = prefixes.sources.resolve(file_name)
    else:
      try:
        file_name = os.path.relpath(file_name, base_path)
      except ValueError:
        file_name = os.path.abspath(file_name)
      file_name = utils.normalize_path(file_name)
      source_name = file_name
    if source_name not in sources_map:
      source_id = len(sources)
      sources_map[source_name] = source_id
      sources.append(source_name)
      if collect_sources:
        load_name = prefixes.load.resolve(file_name)
        try:
          with open(load_name, 'r') as infile:
            source_content = infile.read()
          sources_content.append(source_content)
        except IOError:
          print('Failed to read source: %s' % load_name)
          sources_content.append(None)
    else:
      source_id = sources_map[source_name]

    address_delta = address - last_address
    source_id_delta = source_id - last_source_id
    line_delta = line - last_line
    column_delta = column - last_column
    mappings.append(encode_vlq(address_delta) + encode_vlq(source_id_delta) + encode_vlq(line_delta) + encode_vlq(column_delta))
    last_address = address
    last_source_id = source_id
    last_line = line
    last_column = column
  return {'version': 3,
          'sources': sources,
          'sourcesContent': sources_content,
          'names': [],
          'mappings': ','.join(mappings)}


def main():
  options = parse_args()

  wasm_input = options.wasm
  with open(wasm_input, 'rb') as infile:
    wasm = infile.read()

  entries = read_dwarf_entries(wasm_input, options)

  code_section_offset = get_code_section_offset(wasm)

  prefixes = SourceMapPrefixes(sources=Prefixes(options.prefix), load=Prefixes(options.load_prefix))

  logger.debug('Saving to %s' % options.output)
  map = build_sourcemap(entries, code_section_offset, prefixes, options.sources, options.basepath)
  with open(options.output, 'w') as outfile:
    json.dump(map, outfile, separators=(',', ':'))

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
  sys.exit(main())
