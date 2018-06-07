#!/usr/bin/env python

# Utility tools that extracts DWARF information encoded in a wasm output
# produced by the LLVM tools, and encodes it as a wasm source map. Additionally,
# it can collect original sources, change files prefixes, and strip debug
# sections from a wasm file.

from subprocess import Popen, PIPE
import re
import json
import argparse
import os, sys

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools.shared import LLVM_ROOT

def parse_args():
  parser = argparse.ArgumentParser(prog='wasm-sourcemap.py')
  parser.add_argument('wasm', help='wasm file')
  parser.add_argument('-o', '--output', help='output source map')
  parser.add_argument('-p', '--prefix', nargs='*', help='replace source filename prefix', default=[])
  parser.add_argument('-s', '--sources', action='store_true', help='read and embed source files')
  parser.add_argument('-w', nargs='?', help='set output wasm file')
  parser.add_argument('-x', '--strip', action='store_true', help='removes debug and linking sections')
  parser.add_argument('-u', '--source-map-url', nargs='?', help='specifies sourceMappingURL section contest')
  parser.add_argument('--llvm-dwarfdump', nargs='?', help=argparse.SUPPRESS)
  return parser.parse_args()

VLQ_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
def encode_vlq(n):
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
  print('Strip debug sections')
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
        continue # skip debug related sections
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
  print('Append sourceMappingURL section')
  section_name = "sourceMappingURL"
  section_content = encode_uint_var(len(section_name)) + section_name + encode_uint_var(len(url)) + url
  return wasm + encode_uint_var(0) + encode_uint_var(len(section_content)) + section_content

def get_code_section_offset(wasm):
  print('Read sections index')
  pos = 8

  while pos < len(wasm):
    section_id, pos_ = read_var_uint(wasm, pos)
    section_size, pos = read_var_uint(wasm, pos_)
    if section_id == 10:
      return pos
    pos = pos + section_size

def read_dwarf_entries(wasm, dwarfdump):
  if dwarfdump:
    output = open(dwarfdump, 'r').read()
  else:
    print('Reading DWARF information from %s' % wasm)
    llvm_dwarfdump = os.path.join(LLVM_ROOT, 'llvm-dwarfdump')
    process = Popen([llvm_dwarfdump, "-debug-line", wasm], stdout=PIPE)
    (output, err) = process.communicate()
    exit_code = process.wait()
    if exit_code != 0:
      print('Error during llvm-dwarfdump execution (%s)' % exit_code)
      exit(1)

  entries = []
  debug_line_chunks = re.split(r"(debug_line\[0x[0-9a-f]*\])", output)
  for i in range(1,len(debug_line_chunks),2):
    line_chunk = debug_line_chunks[i + 1]

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

    include_directories = {'0': ""}
    for dir in re.finditer(r"include_directories\[\s*(\d+)\] = \"([^\"]*)", line_chunk):
      include_directories[dir.group(1)] = dir.group(2)

    files = {}
    for file in re.finditer(r"file_names\[\s*(\d+)\]:\s+name: \"([^\"]*)\"\s+dir_index: (\d+)", line_chunk):
      dir = include_directories[file.group(3)]
      file_path = (dir + '/' if dir != '' else '') + file.group(2)
      files[file.group(1)] = file_path


    for line in re.finditer(r"\n0x([0-9a-f]+)\s+(\d+)\s+(\d+)\s+(\d+)", line_chunk):
      entry = {'address': int(line.group(1), 16), 'line': int(line.group(2)), 'column': int(line.group(3)), 'file': files[line.group(4)]}
      entries.append(entry)
  return entries

def build_sourcemap(entries, code_section_offset, prefixes, collect_sources):
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
    if line == 0 or column == 0:
      continue
    address = entry['address'] + code_section_offset
    file_name = entry['file']
    if file_name not in sources_map:
      source_id = len(sources)
      sources_map[file_name] = source_id
      source_name = file_name
      for p in prefixes:
        if file_name.startswith(p['prefix']):
          if p['replacement'] is None:
            source_name = file_name[len(p['prefix'])::]
          else:
            source_name = p['replacement'] + file_name[len(p['prefix'])::]
          break
      sources.append(source_name)
      if collect_sources:
        try:
          with open(file_name, 'r') as infile:
            source_content = infile.read()
          sources_content.append(source_content)
        except:
          print('Failed to read source: %s' % file_name)
          sources_content.append(None)
    else:
      source_id = sources_map[file_name]

    address_delta = address - last_address
    source_id_delta = source_id - last_source_id
    line_delta = line - last_line
    column_delta = column - last_column
    mappings.append(encode_vlq(address_delta) + encode_vlq(source_id_delta) + encode_vlq(line_delta) + encode_vlq(column_delta))
    last_address = address
    last_source_id = source_id
    last_line = line
    last_column = column
  return {'version': 3, 'names': [], 'sources': sources, 'sourcesContent': sources_content, 'mappings': ','.join(mappings)}

def main():
  args = parse_args()

  wasm_input = args.wasm
  with open(wasm_input, 'rb') as infile:
    wasm = infile.read()

  entries = read_dwarf_entries(wasm_input, args.llvm_dwarfdump)

  code_section_offset = get_code_section_offset(wasm)

  prefixes = []
  for p in args.prefix:
    if '=' in p:
      prefix, replacement = p.split('=')
      prefixes.append({'prefix': prefix, 'replacement': replacement})
    else:
      prefixes.append({'prefix': p, 'replacement': None})

  print('Saving to %s' % args.output)
  map = build_sourcemap(entries, code_section_offset, prefixes, args.sources)
  with open(args.output, 'w') as outfile:
    json.dump(map, outfile)

  if args.strip:
    wasm = strip_debug_sections(wasm)

  if args.source_map_url:
    wasm = append_source_mapping(wasm, args.source_map_url)

  if args.w:
    print('Saving wasm to %s' % args.w)
    with open(args.w, 'wb') as outfile:
      outfile.write(wasm)

  print('Done.')

if __name__ == '__main__':
  sys.exit(main())
