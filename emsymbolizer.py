#!/usr/bin/env python3

# This is a utility for looking up the symbol names and/or file+line numbers
# of code addresses. There are several possible sources of this information,
# with varying granularity (listed here in approximate preference order).

# If the wasm has DWARF info, llvm-symbolizer can show the symbol, file, and
# line/column number, potentially including inlining.
# If there is a source map, we can parse it to get file and line number.
# If there is an emscripten symbol map, we can parse that to get the symbol name
# If there is a name section or symbol table, llvm-nm can show the symbol name.

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


def main(argv):
  wasm_file = argv[1]
  print('Warning: the command-line and output format of this file are not '
        'finalized yet', file=sys.stderr)
  module = webassembly.Module(wasm_file)

  if not has_debug_line_section(module):
    raise Error(f"No .debug_line section found in {module.filename}."
                " I don't know how to symbolize this file yet")

  symbolize_address_dwarf(module, int(argv[2], 16))
  return 0


if __name__ == '__main__':
  try:
    rv = main(sys.argv)
  except (Error, webassembly.InvalidWasmError, OSError) as e:
    print(f'{sys.argv[0]}: {str(e)}', file=sys.stderr)
    rv = 1
  sys.exit(rv)
