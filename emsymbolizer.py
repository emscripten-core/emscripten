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
# Separate DWARF is not supported yet.

import argparse
import sys
from tools.symbolizer import *


def main(args):
  with webassembly.Module(args.wasm_file) as module:
    base = 16 if args.address.lower().startswith('0x') else 10
    address = int(args.address, base)

    if args.addrtype == 'code':
      address += get_codesec_offset(module)

    def print_loc(loc):
      if isinstance(loc, list):
        for l in loc:
          l.print()
      else:
        loc.print()

    if ((has_debug_line_section(module) and not args.source) or
       'dwarf' in args.source):
      print_loc(symbolize_address_symbolizer(module, address, is_dwarf=True))
    elif ((get_sourceMappingURL_section(module) and not args.source) or
          'sourcemap' in args.source):
      print_loc(symbolize_address_sourcemap(module, address, args.file))
    elif ((has_name_section(module) and not args.source) or
          'names' in args.source):
      print_loc(symbolize_address_symbolizer(module, address, is_dwarf=False))
    elif ((has_linking_section(module) and not args.source) or
          'symtab' in args.source):
      print_loc(symbolize_address_symbolizer(module, address, is_dwarf=False))
    elif (args.source == 'symbolmap'):
      print_loc(symbolize_address_symbolmap(module, address, args.file))
    else:
      raise Error('No .debug_line or sourceMappingURL section found in '
                  f'{module.filename}.'
                  " I don't know how to symbolize this file yet")


def get_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('-s', '--source', choices=['dwarf', 'sourcemap',
                                                 'names', 'symtab', 'symbolmap'],
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
