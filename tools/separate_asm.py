# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Separates out the core asm module out of an emscripten output file.

This is useful because it lets you load the asm module first, then the main script, which on some browsers uses less memory
'''

import os
import sys

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

infile = sys.argv[1]
asmfile = sys.argv[2]
otherfile = sys.argv[3]
asm_module_name = sys.argv[4] if len(sys.argv) >= 5 else 'Module["asm"]'


def find_index_of_closing_parens(string, idx):
  assert string[idx] == '('
  idx += 1
  nesting_level = 1
  while nesting_level > 0 and idx < len(string):
    if string[idx] == '(':
      nesting_level += 1
    elif string[idx] == ')':
      nesting_level -= 1
      if nesting_level == 0:
        return idx
    idx += 1
  raise Exception('Failed to find closing parenthesis for string ' + string + ', start parentheses at idx=' + str(idx))


# Given a string of form // EMSCRIPTEN_START_ASM\n {asm.js here} // EMSCRIPTEN_END_ASM,
# extracts out the asm.js expression. The asm.js expression may be of form var asm={asm.js expression}; or
# (asm.js expression)(...); depending on how efficiently Closure optimized the code
def find_asm_block(asm_js):
  asm_start = asm_js.find('// EMSCRIPTEN_START_ASM')
  assert asm_start >= 0, 'Could not find asm.js block in ' + asm_js
  asm_start += len('// EMSCRIPTEN_START_ASM')
  asm_start = asm_js.find(asm_js[asm_start:].strip())
  if asm_js[asm_start] == '(': # Closure optimized away the asm variable, i.e. asm.js content was of form (asm.js expression)(...);
    asm_function_start = asm_js.find('function(', asm_start)
    start_idx = asm_js.rfind('(', 0, asm_function_start)
    close_idx = find_index_of_closing_parens(asm_js, start_idx)
    asm_search = asm_replace = asm_js[start_idx:close_idx + 1]
    # Strip redundant () parens around the module object if there are some
    if asm_replace[0] == '(' and asm_replace[-1] == ')':
      asm_replace = asm_replace[1:-1]
    # Strip redundant "(0,...)" that Closure generates
    if asm_js[start_idx - 3:start_idx] == '(0,':
      asm_search = '(0,' + asm_search
      redundant_closing_parens = find_index_of_closing_parens(asm_js, start_idx - 3)
      asm_js = asm_js[0:redundant_closing_parens] + asm_js[redundant_closing_parens + 1:]
    return (asm_search, asm_replace, asm_js)
  elif asm_js.find('var ', asm_start) == asm_start: # asm.js content is of form var asm={asm.js expression};
    asmjs_end = asm_js.rfind('// EMSCRIPTEN_END_ASM')
    asm_block = asm_js[asm_js.find('=', asm_start) + 1:asmjs_end]
    return (asm_block, asm_block, asm_js)
  raise Exception('Unable to parse asm.js block in ' + asm_js)


everything = open(infile).read()
asm_search, asm_replace, everything = find_asm_block(everything)
if 'var Module' in everything:
  everything = everything.replace(asm_search, 'Module["asm"]')
else:
  # closure compiler removes |var Module|, we need to find the closured name
  # seek a pattern like (e.ENVIRONMENT), which is in the shell.js if-cascade for the ENVIRONMENT override
  import re
  m = re.search(r'(\w+)\s*=\s*"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__"', everything)
  if not m:
    m = re.search(r'(\w+)=typeof Module !== \'undefined\' \? Module : {}', everything)
  if not m:
    m = re.search(r'\((\w+)\.ENVIRONMENT\)', everything)
  if not m:
    m = re.search(r'(\w+)\.arguments\s*=\s*\[\];', everything)
  assert m, 'cannot figure out the closured name of Module statically' + everything
  closured_name = m.group(1)
  everything = everything.replace(asm_search, closured_name + '["asm"]')

with open(asmfile, 'w') as o:
  o.write(asm_module_name + '=')
  o.write(asm_replace)

with open(otherfile, 'w') as o:
  o.write(everything)
