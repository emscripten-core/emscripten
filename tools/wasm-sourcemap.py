#!/usr/bin/env python
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
from collections import OrderedDict, namedtuple
import json
import logging
from math import floor, log
import os
import re
import subprocess
import sys


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

    result = name
    for p in self.prefixes:
      if name.startswith(p['prefix']):
        if p['replacement'] is None:
          result = name[len(p['prefix'])::]
        else:
          result = p['replacement'] + name[len(p['prefix'])::]
        break
    self.cache[name] = result
    return result


# SourceMapPrefixes contains resolver for file names that are:
#  - "sources" is for names that output to source maps JSON
#  - "load" is for paths that used to load source text
SourceMapPrefixes = namedtuple('SourceMapPrefixes', 'sources, load')


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
  logging.debug('Strip debug sections')
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
  logging.debug('Append sourceMappingURL section')
  section_name = "sourceMappingURL"
  section_content = encode_uint_var(len(section_name)) + section_name + encode_uint_var(len(url)) + url
  return wasm + encode_uint_var(0) + encode_uint_var(len(section_content)) + section_content


def get_code_section_offset(wasm):
  logging.debug('Read sections index')
  pos = 8

  while pos < len(wasm):
    section_id, pos_ = read_var_uint(wasm, pos)
    section_size, pos = read_var_uint(wasm, pos_)
    if section_id == 10:
      return pos
    pos = pos + section_size


def process_sources_references(map, prefixes, collect_sources):
  logging.debug('Re-map and collect sources')
  sources = map['sources']
  sources_content = []
  for source_id in range(len(sources)):
    file_name = sources[source_id]
    source_name = prefixes.sources.resolve(file_name)
    sources[source_id] = source_name
    if collect_sources:
      load_name = prefixes.load.resolve(file_name)
      try:
        with open(load_name, 'r') as infile:
          source_content = infile.read()
        sources_content.append(source_content)
      except:
        print('Failed to read source: %s' % load_name)
        sources_content.append(None)
  map['sourcesContent'] = sources_content

def dwarf_to_json(wasm):
  logging.debug('Read %s and convert to json' % wasm)

  js = os.path.join(os.path.dirname(sys.argv[0]), "dwarf_to_json.js")
  output = subprocess.check_output(["node", js, wasm])

  return json.loads(output)

def main():
  options = parse_args()

  wasm_input = options.wasm
  map = dwarf_to_json(wasm_input)

  prefixes = SourceMapPrefixes(sources=Prefixes(options.prefix), load=Prefixes(options.load_prefix))
  process_sources_references(map, prefixes, options.sources)

  logging.debug('Saving to %s' % options.output)
  with open(options.output, 'w') as outfile:
    json.dump(map, outfile, separators=(',', ':'))

  with open(wasm_input, 'rb') as infile:
    wasm = infile.read()

  if options.strip:
    wasm = strip_debug_sections(wasm)

  if options.source_map_url:
    wasm = append_source_mapping(wasm, options.source_map_url)

  if options.w:
    logging.debug('Saving wasm to %s' % options.w)
    with open(options.w, 'wb') as outfile:
      outfile.write(wasm)

  logging.debug('Done')
  return 0


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG if os.environ.get('EMCC_DEBUG') else logging.INFO)
  sys.exit(main())
