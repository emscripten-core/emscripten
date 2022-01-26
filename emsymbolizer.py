#!/usr/bin/env python3

# There are several ways symbols or addresses can be looked up.
# Addr -> line:
# llvm-symbolizer, using debug info and section offset
# ?, using source map and file offset
# Addr -> Symbol
# ?, using symbol map and file offset?
# Symbol -> Addr (nm, section offset)

# If the wasm has debug info, use llvm-symbolizer (convert the addresses)
# If there is a source map, use it (do we have a parser?)
# If there is a symbol map, use it (interpolate the addresses)
# If there is a name section use llvm-nm (convert the addresses)

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
    print(f'{sys.argv[0]}: {str(e)}')
    rv = 1
  sys.exit(rv)
