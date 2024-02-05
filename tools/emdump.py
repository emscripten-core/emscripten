#!/usr/bin/env python3
# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# -*- Mode: python -*-

"""emdump.py prints out statistics about compiled code sizes
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path


# If true, we are printing delta information between two data sets. If false, we are just printing symbol info for a single data set
diffing_two_data_sets = False

# Global command line options
options = None


# Given a string s and an index i, counts how many times character ch is repeated looking backwards at s[i], s[i-1], s[i-2], s[i-3], ...
def rcount(s, ch, i):
  j = i
  while j > 0 and s[j] == ch:
    j -= 1
  return i - j


# Finds the index where a "foo" or 'foo' string ends in the given string s. Given string s and index 'start' to a string symbol " or ', finds the matching index where the string ends.
# This takes into account escapes in the middle, i.e. "foo\\\\\\\"bar" will be properly matched.
def find_unescaped_end(s, ch, start, end):
  if s[start] != ch:
    raise Exception('Index start should point to starting occurrence of ch')
  start += 1
  while start < end:
    if s[start] == ch and rcount(s, '\\', start - 1) % 2 == 0:
      return start
    start += 1
  return -1


# Transforms linear index to string to file, column pair. (for debugging use only, need to build index->file:line mapping table for batch operations)
def idx_to_line_col(s, i):
  line = s.count('\n', 0, i) + 1
  last_n = s.rfind('\n', 0, i)
  return 'line ' + str(line) + ', column ' + str(i - last_n) + ' (idx ' + str(i) + ')'


# Given a string, returns brace_map dictionary that maps starting parens/brackets/braces indices to their ending positions.
# This can be brittle since we are not able to parse JS proper, but good enough for Emscripten compiled output. (some debugging code retained in body if you run into a tricky case)
def parse_parens(s):
  brace_map = {}

  parens = [] # ()
  brackets = [] # []
  braces = [] # {}

  i = 0
  end = len(s)
  while i < end:
    ch = s[i]
    if ch == '/':
      if i < end and s[i + 1] == '/':
        # prev = i
        i = s.find('\n', i)
        # print(idx_to_line_col(s, prev) + ' starts // comment, skipping to ' + idx_to_line_col(s, i))
      if i < end and s[i + 1] == '*':
        # prev = i
        i = s.find('*/', i + 2) + 1
        # print(idx_to_line_col(s, prev) + ' starts /* comment, skipping to ' + idx_to_line_col(s, i))
    elif ch == '"' and rcount(s, '\\', i - 1) % 2 == 0:
      # prev = i
      i = find_unescaped_end(s, '"', i, end)
      # print(idx_to_line_col(s, prev) + ' is a "" string, skipping to ' + idx_to_line_col(s, i))
    elif ch == "'" and rcount(s, '\\', i - 1) % 2 == 0:
      # prev = i
      i = find_unescaped_end(s, "'", i, end)
      # print(idx_to_line_col(s, prev) + ' is a \'\' string, skipping to ' + idx_to_line_col(s, i))
    elif ch == '^': # Ignore parens/brackets/braces if the previous character was a '^'. This is a bit of a heuristic, '^)' occur commonly in Emscripten generated regexes
      i += 1
    elif ch == '(':
      if rcount(s, '\\', i - 1) % 2 == 0:
        parens.append(i)
      # print(idx_to_line_col(s, i) + ' has (')
    elif ch == '[':
      if rcount(s, '\\', i - 1) % 2 == 0:
        brackets.append(i)
      # print(idx_to_line_col(s, i) + ' has [')
    elif ch == '{':
      if rcount(s, '\\', i - 1) % 2 == 0:
        braces.append(i)
      # print(idx_to_line_col(s, i) + ' has {')
    elif ch == ')':
      if rcount(s, '\\', i - 1) % 2 == 0:
        # print(idx_to_line_col(s, i) + ' has )')
        if len(parens) > 0:
          brace_map[parens.pop()] = i
        # else: print('Warning: ' + idx_to_line_col(s, i) + ' has ), but could not find the opening parenthesis.')
    elif ch == ']':
      if rcount(s, '\\', i - 1) % 2 == 0:
        # print(idx_to_line_col(s, i) + ' has ]')
        if len(brackets) > 0:
          brace_map[brackets.pop()] = i
        # else: print('Warning: ' + idx_to_line_col(s, i) + ' has ], but could not find the opening bracket.')
    elif ch == '}':
      if rcount(s, '\\', i - 1) % 2 == 0:
        # print(idx_to_line_col(s, i) + ' has }')
        if len(braces) > 0:
          brace_map[braces.pop()] = i
        # else: print('Warning: ' + idx_to_line_col(s, i) + ' has }, but could not find the opening brace.')
    i += 1
  return brace_map


# Valid characters in Emscripten outputted JS content (in reality valid character set is much more complex, but do not need that here)
def is_javascript_symbol_char(ch):
  i = ord(ch)
  return (i >= 97 and i <= 122) or (i >= 65 and i <= 90) or (i >= 48 and i <= 57) or i == 36 or i == 95 # a-z, A-Z, 0-9, $, _


def cxxfilt():
  filt = shutil.which('llvm-cxxfilt')
  if filt:
    return filt
  return shutil.which('c++filt')


# Runs the given symbols list through c++filt to demangle.
def cpp_demangle(symbols):
  try:
    filt = cxxfilt()
    if not filt:
      print('"llvm-cxxfilt" or "c++filt" executable is not found, demangled symbol names will not be available')
      return ''
    proc = subprocess.Popen([cxxfilt(), '--strip-underscore'], stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    output = proc.communicate(input=symbols)
    return output[0].replace('\r\n', '\n')
  except Exception:
    return ''


# Given a data set, fills in the 'demangled_data' field for each entry.
def find_demangled_names(data):
  if not data or len(data) == 0:
    return
  data_lines = list(data.keys())
  demangled_names = cpp_demangle('\n'.join(data_lines)).split('\n')
  for i in range(len(data)):
    mangled = data_lines[i]
    data[mangled]['demangled_name'] = demangled_names[i].strip() if i < len(demangled_names) else mangled


# Merges a new_entry with an old entry with the same name accumulating to its size (or adds new)
def merge_entry_to_existing(existing_data, new_entry, total_source_set_size):
  name = new_entry['unminified_name']
  if name in existing_data:
    ex = existing_data[name]
    num_times_occurs_1 = ex['num_times_occurs'] if 'num_times_occurs' in ex else 1
    num_times_occurs_2 = new_entry['num_times_occurs'] if 'num_times_occurs' in new_entry else 1
    existing_data[name] = {
      'lines': ex['lines'] + new_entry['lines'],
      'bytes': ex['bytes'] + new_entry['bytes'],
      'demangled_name': ex['demangled_name'] if 'demangled_name' in ex else (new_entry['demangled_name'] if 'demangled_name' in new_entry else new_entry['minified_name']),
      'minified_name': ex['minified_name'],
      'unminified_name': ex['unminified_name'],
      'function_parameters': ex['function_parameters'],
      'type': ex['type'],
      'percentage': (ex['bytes'] + new_entry['bytes']) * 100.0 / total_source_set_size,
      'num_times_occurs': num_times_occurs_1 + num_times_occurs_2
    }
  else:
    existing_data[name] = new_entry


def merge_to_data_set(to_set, from_set, total_source_set_size):
  for key, value in from_set.items():
    if diffing_two_data_sets:
      merge_entry_to_existing(to_set, value, total_source_set_size)
    else:
      # if key in to_set:
      #    key = s + '__' + key
      to_set[key] = value


# Builds up a dataset of functions and variables in the given JavaScript file (JS or asm.js)
def analyze_javascript_file_contents(filename, file_contents, total_source_set_size, symbol_map=None):
  data = {}
  brace_map = parse_parens(file_contents)
  parse_pos = 0
  prev_end_pos = 0
  file_len = len(file_contents)
  func_regex = re.compile(r'function\s+([\w$]+)\s*\(([\w\s$,]*?)\)\s*{') # Search for "function foo (param1, param2, ..., paranN) {"
  var_block_regex = re.compile(r'var\s+(\w+)\s*=\s*([{\[\(])') # Search for "var foo = {"
  var_regex = re.compile(r'var\s+([\w]+)\s*=\s*[\w\s,]*?;') # Search for "var foo = .... ;"
  unaccounted_bytes = 0
  unaccounted_lines = 0

  asm_start = file_contents.find('use asm')
  asm_start_brace = -1
  asm_end_brace = -1
  asm_type = 'asmjs'
  if asm_start < 0:
    asm_start = file_contents.find('almost asm')
    asm_type = '~asmjs'
  if asm_start >= 0:
    asm_start_brace = file_contents.rfind('{', 0, asm_start)
    if asm_start_brace >= 0:
      asm_end_brace = brace_map[asm_start_brace] if asm_start_brace in brace_map else file_len

  func_pos = -1
  var_pos = -1
  while parse_pos < file_len:
    if func_pos < parse_pos:
      func_pos = file_contents.find('function ', parse_pos)
    if func_pos < 0:
      func_pos = file_len
    if var_pos < parse_pos:
      var_pos = file_contents.find('var ', parse_pos)
    if var_pos < 0:
      var_pos = file_len
    if min(func_pos, var_pos) >= file_len:
      break
    next_pos = min(func_pos, var_pos)
    parse_pos = next_pos + 1

    # Skip this occurrence of 'function' if it had a prefix as part of some other string, e.g. 'foofunction'
    if next_pos > 0 and is_javascript_symbol_char(file_contents[next_pos - 1]):
      continue

    if next_pos > prev_end_pos:
      unaccounted_lines += file_contents.count('\n', prev_end_pos, next_pos) + 1
      unaccounted_bytes += next_pos - prev_end_pos
      if options.dump_unaccounted_larger_than >= 0 and next_pos - prev_end_pos > options.dump_unaccounted_larger_than:
        print('--- Unaccounted ' + str(next_pos - prev_end_pos) + ' bytes in ' + filename + ':')
        print(file_contents[prev_end_pos:next_pos])
        print('===')
    prev_end_pos = next_pos

    # Verify that this position actually starts a function by testing against a regex (this is much slower than substring search,
    # which is why it's done as a second step, instead of as primary way to search)
    if next_pos == func_pos:
      func_match = func_regex.match(file_contents[func_pos:])
      if not func_match:
        continue

      # find starting and ending braces { } for the function
      start_brace = file_contents.find('{', func_pos)
      if start_brace < 0:
        break # Must be at the end of file
      if start_brace not in brace_map:
        print('Warning: ' + idx_to_line_col(file_contents, start_brace) + ' cannot parse function start brace, skipping.')
        continue
      end_brace = brace_map[start_brace]
      if end_brace < 0:
        break # Must be at the end of file

      num_bytes = end_brace + 1 - func_pos
      num_lines = file_contents.count('\n', func_pos, end_brace) + 1
      prev_end_pos = parse_pos = end_brace + 1

      function_type = asm_type if func_pos >= asm_start_brace and end_brace <= asm_end_brace else 'js'
      minified_name = func_match.group(1)
      function_parameters = func_match.group(2).strip()
      if symbol_map and minified_name in symbol_map and function_type == asm_type:
        unminified_name = symbol_map[minified_name]
      else:
        unminified_name = minified_name
      data[unminified_name] = {
        'lines': num_lines,
        'bytes': num_bytes,
        'minified_name': minified_name,
        'unminified_name': unminified_name,
        'function_parameters': function_parameters,
        'type': function_type,
        'percentage': num_bytes * 100.0 / total_source_set_size
      }
    else: # This is a variable
      var_block_match = var_block_regex.match(file_contents[var_pos:])
      if var_block_match:
        # find starting and ending braces { } for the var
        start_brace = file_contents.find(var_block_match.group(2), var_pos)
        if start_brace < 0:
          break # Must be at the end of file
        if start_brace not in brace_map:
          print('Warning: ' + idx_to_line_col(file_contents, start_brace) + ' cannot parse variable start brace, skipping.')
          continue
        end_brace = brace_map[start_brace]
        if end_brace < 0:
          break # Must be at the end of file
        minified_name = var_block_match.group(1)
      else:
        start_brace = var_pos
        var_match = var_regex.match(file_contents[var_pos:])
        if not var_match:
          continue
        end_brace = file_contents.find(';', var_pos)
        minified_name = var_match.group(1)

      # Special case ignore the 'var wasmExports = (function(global, env, buffer) { 'use asm'; ... }; ' variable that contains all the asm.js code.
      # Ignoring this variable lets all the asm.js code be trated as functions in this parser, instead of assigning them to the asm variable.
      if file_contents[start_brace] == '(' and ("'use asm'" in file_contents[var_pos:end_brace] or '"use asm"' in file_contents[var_pos:end_brace] or "'almost asm'" in file_contents[var_pos:end_brace] or '"almost asm"' in file_contents[var_pos:end_brace]):
        continue

      num_bytes = end_brace + 1 - var_pos
      num_lines = file_contents.count('\n', var_pos, end_brace) + 1
      prev_end_pos = parse_pos = end_brace + 1

      var_type = 'asm_var' if func_pos >= asm_start_brace and end_brace <= asm_end_brace else 'var'

      if symbol_map and minified_name in symbol_map and var_type == 'asm_var':
        unminified_name = symbol_map[minified_name].strip()
      else:
        unminified_name = minified_name
      data[unminified_name] = {
        'lines': num_lines,
        'bytes': num_bytes,
        'minified_name': minified_name,
        'unminified_name': unminified_name,
        'function_parameters': '',
        'type': var_type,
        'percentage': num_bytes * 100.0 / total_source_set_size
      }

  if options.list_unaccounted:
    if diffing_two_data_sets:
      unaccounted_name = '$unaccounted_js_content' # If diffing two data sets, must make the names of the unaccounted content blocks be comparable
    else:
      unaccounted_name = '$unaccounted_js_content_in("' + os.path.basename(filename) + '")'
    unaccounted_entry = {
      'lines': unaccounted_lines,
      'bytes': unaccounted_bytes,
      'minified_name': unaccounted_name,
      'unminified_name': unaccounted_name,
      'function_parameters': '',
      'type': '[UNKN]',
      'percentage': unaccounted_bytes * 100.0 / total_source_set_size
    }
    merge_entry_to_existing(data, unaccounted_entry, total_source_set_size)

  return data


def analyze_javascript_file(filename, total_source_set_size, symbol_map=None):
  file_contents = Path(filename).read_text()
  print('Analyzing JS file ' + filename + ', ' + str(len(file_contents)) + ' bytes...')
  return analyze_javascript_file_contents(filename, file_contents, total_source_set_size, symbol_map)


def analyze_html_file(filename, total_source_set_size, symbol_map=None):
  file_contents = Path(filename).read_text()
  print('Analyzing HTML file ' + filename + ', ' + str(len(file_contents)) + ' bytes...')
  data = {}
  parse_pos = 0
  file_len = len(file_contents)
  unaccounted_bytes = 0
  unaccounted_lines = 0

  while parse_pos < file_len:
    script_pos = file_contents.find('<script', parse_pos)
    if script_pos < 0:
      break
    script_pos = file_contents.find('>', script_pos)
    if script_pos < 0:
      break
    script_pos += 1
    script_end_pos = file_contents.find('</script>', script_pos)
    if script_end_pos < 0:
      break

    if script_pos > parse_pos:
      unaccounted_bytes += script_pos - parse_pos
      unaccounted_lines += file_contents.count('\n', parse_pos, script_pos) + 1
    data_set = analyze_javascript_file_contents(filename, file_contents[script_pos:script_end_pos], total_source_set_size, symbol_map)
    merge_to_data_set(data, data_set, total_source_set_size)
    parse_pos = script_end_pos

  if file_len > parse_pos:
    unaccounted_bytes += file_len - parse_pos
    unaccounted_lines += file_contents.count('\n', parse_pos, file_len) + 1

  if options.list_unaccounted and unaccounted_bytes > 0:
    if diffing_two_data_sets:
      unaccounted_name = '$unaccounted_html_content' # If diffing two data sets, must make the names of the unaccounted content blocks be comparable
    else:
      unaccounted_name = '$unaccounted_html_content_in("' + os.path.basename(filename) + '")'
    unaccounted_entry = {
      'lines': unaccounted_lines,
      'bytes': unaccounted_bytes,
      'minified_name': unaccounted_name,
      'unminified_name': unaccounted_name,
      'function_parameters': '',
      'type': 'HTML',
      'percentage': unaccounted_bytes * 100.0 / total_source_set_size
    }
    merge_entry_to_existing(data, unaccounted_entry, total_source_set_size)

  return data


def analyze_source_file(filename, total_source_set_size, symbol_map=None):
  if '.htm' in os.path.basename(filename).lower():
    return analyze_html_file(filename, total_source_set_size, symbol_map)
  else:
    return analyze_javascript_file(filename, total_source_set_size, symbol_map)


def common_compare(data1, data2):
  fns1 = set(data1.keys())
  fns2 = set(data2.keys())
  commonfns = fns1.intersection(fns2)
  commonlinediff = 0
  commonbytediff = 0
  for fn in commonfns:
    d1 = data1[fn]
    d2 = data2[fn]
    commonlinediff += d2['lines'] - d1['lines']
    commonbytediff += d2['bytes'] - d1['bytes']
  linesword = 'more' if commonlinediff >= 0 else 'less'
  bytesword = 'more' if commonbytediff >= 0 else 'less'
  print('set 2 has {} lines {} than set 1 in {} common functions'.format(abs(commonlinediff), linesword, len(commonfns)))
  print('set 2 has {} bytes {} than set 1 in {} common functions'.format(str(abs(commonbytediff)), bytesword, len(commonfns)))


def uniq_compare(data1, data2):
  fns1 = set(data1.keys())
  fns2 = set(data2.keys())
  uniqfns1 = fns1 - fns2
  uniqfns2 = fns2 - fns1
  uniqlines1 = 0
  uniqbytes1 = 0
  uniqlines2 = 0
  uniqbytes2 = 0
  for fn in uniqfns1:
    d = data1[fn]
    uniqlines1 += d['lines']
    uniqbytes1 += d['bytes']
  for fn in uniqfns2:
    d = data2[fn]
    uniqlines2 += d['lines']
    uniqbytes2 += d['bytes']
  uniqcountdiff = len(uniqfns2) - len(uniqfns1)
  assert len(fns2) - len(fns1) == uniqcountdiff
  uniqlinediff = uniqlines2 - uniqlines1
  uniqbytediff = uniqbytes2 - uniqbytes1
  countword = 'more' if uniqcountdiff >= 0 else 'less'
  linesword = 'more' if uniqlinediff >= 0 else 'less'
  bytesword = 'more' if uniqbytediff >= 0 else 'less'
  print('set 2 has {} functions {} than set 1 overall (unique: {} vs {})'.format(abs(uniqcountdiff), countword, len(uniqfns2), len(uniqfns1)))
  print('set 2 has {} lines {} than set 1 overall in unique functions'.format(abs(uniqlinediff), linesword))
  print('set 2 has {} bytes {} than set 1 overall in unique functions'.format(str(abs(uniqbytediff)), bytesword))


# Use a bunch of regexps to simplify the demangled name
DEM_RE = None


def simplify_cxx_name(name):
  global DEM_RE
  if DEM_RE is None:
    DEM_RE = []
    string_m = re.compile(r'std::__2::basic_string<char, std::__2::char_traits<char>, std::__2::allocator<char> >')
    DEM_RE.append(lambda s: string_m.sub(r'std::string', s))
    vec_m = re.compile(r'std::__2::vector<([^,]+), std::__2::allocator<\1\s*> >')
    DEM_RE.append(lambda s: vec_m.sub(r'std::vector<\1>', s))
    unordered_map_m = re.compile(r'std::__2::unordered_map<([^,]+), ([^,]+), std::__2::hash<\1\s*>, std::__2::equal_to<\1\s*>, std::__2::allocator<std::__2::pair<\1 const, \2> > >')
    DEM_RE.append(lambda s: unordered_map_m.sub(r'std::unordered_map<\1, \2>', s))
    sort_m = re.compile(r'std::__2::__sort<std::__2::__less<([^,]+), \1\s*>&, \1\*>\(\1\*, \1\*, std::__2::__less<\1, \1\s*>&\)')
    DEM_RE.append(lambda s: sort_m.sub(r'std::sort(\1*, \1*)', s))
    DEM_RE.append(lambda s: s.replace('std::__2::', 'std::'))

  for dem in DEM_RE:
    name = dem(name)
  return name


# 'foo(int, float)' -> 'foo'
def function_args_removed(s):
  if '(' in s:
    return s[:s.find('(')]
  else:
    return s


# 'foo(int, float)' -> 'int, float)'
def function_args_part(s):
  if '(' in s:
    return s[s.find('(') + 1:]
  else:
    return ''


def sort_key_py2(key_value):
  return key_value[1][options.sort]


# Apparently for python 3, one will use the following, but currently untested
# def sort_key_py3(key, value):
#   return value[options.sort]

def print_symbol_info(data, total_source_set_size):
  data = list(data.items())
  data.sort(key=sort_key_py2, reverse=not options.sort_ascending)

  total_size = 0
  for unminified_name, e in data:
    if options.only_unique_1 and e['in_set_2']:
      continue
    if options.only_unique_2 and e['in_set_1']:
      continue
    if options.only_common and (not e['in_set_1'] or not e['in_set_2']):
      continue
    prev_bytes = e['prev_bytes'] if 'prev_bytes' in e else 0
    if max(e['bytes'], prev_bytes) < options.filter_size:
      continue
    if e['bytes'] == prev_bytes and options.only_changes:
      continue

    minified_name = e['minified_name']
    demangled_name = e['demangled_name']
    if options.simplify_cxx:
      demangled_name = simplify_cxx_name(demangled_name)

    if '(' not in demangled_name and 'js' in e['type']:
      demangled_name_with_args = demangled_name + '(' + e['function_parameters'] + ')'
    else:
      demangled_name_with_args = demangled_name
    demangled_name = function_args_removed(demangled_name)

    if options.filter_name not in demangled_name_with_args.lower():
      continue

    if e['function_parameters']:
      unminified_name_with_args = unminified_name + '(' + e['function_parameters'] + ')'
      minified_name_with_args = minified_name + '(' + e['function_parameters'] + ')'
    elif 'js' in e['type']:
      unminified_name_with_args = unminified_name + '()'
      minified_name_with_args = minified_name + '()'
    else:
      unminified_name_with_args = unminified_name
      minified_name_with_args = minified_name

    # Build up the function name to print based on the desired formatting specifiers (mangled/minified/unminified, yes/no args)
    print_name = []
    for i in options.print_format:
      if i == 'd':
        print_name += [demangled_name]
      elif i == 'u':
        print_name += [unminified_name]
      elif i == 'm':
        print_name += [minified_name]
      elif i == 'D':
        print_name += [demangled_name_with_args]
      elif i == 'U':
        print_name += [unminified_name_with_args]
      elif i == 'M':
        print_name += [minified_name_with_args]

    # Collapse names that are identical
    i = 0
    while i + 1 < len(print_name):
      if print_name[i] == print_name[i + 1]:
        print_name = print_name[:i] + print_name[i + 1:]
        continue
      n1 = function_args_removed(print_name[i])
      n2 = function_args_removed(print_name[i + 1])
      args1 = function_args_part(print_name[i])
      args2 = function_args_part(print_name[i + 1])
      if n1 == n2 and (not args1 or not args2):
        if not args1:
          print_name = print_name[:i] + print_name[i + 1:]
        else:
          print_name = print_name[:i + 1] + print_name[i + 2:]
        continue
      i += 1

    print_name = ' ; '.join(print_name)
    if 'num_times_occurs' in e:
      print_name = '[' + str(e['num_times_occurs']) + ' times] ' + print_name
    delta_string = ' %+8d (%+6.2f%%)' % (e['bytes'] - e['prev_bytes'], e['percentage'] - e['prev_percentage']) if diffing_two_data_sets else ''
    print('%6d lines %7s (%5.2f%%) %s: %8s %s' % (e['lines'], str(e['bytes']), e['percentage'], delta_string, e['type'], print_name))

    total_size += e['bytes']

  if total_size < total_source_set_size:
    print('Total size of printed functions: ' + str(total_size) + ' bytes. (%.2f%% of all symbols)' % (total_size * 100.0 / total_source_set_size))
  else:
    print('Total size of printed functions: ' + str(total_size) + ' bytes.')


# Parses Emscripten compiler generated .symbols map file for minified->unminified mappings
def read_symbol_map(filename):
  if not filename:
    return
  symbol_map = {}
  for line in open(filename):
    minified, unminified = line.split(':')
    symbol_map[minified.strip()] = unminified.strip()
  return symbol_map


# Locates foo.js to foo.js.symbols or foo.html.symbols based on default output name rules for Emscripten compiler
def guess_symbol_map_file_location(sources, symbol_map_file):
  if os.path.isfile(symbol_map_file):
    return symbol_map_file
  for s in sources:
    if os.path.isfile(s + '.symbols'):
      return s + '.symbols'
    if os.path.isfile(s.replace('.js', '.html') + '.symbols'):
      return s.replace('.js', '.html') + '.symbols'
  return None


# Returns total byte size of the given list of source files
def count_file_set_size(sources):
  total_size = 0
  for s in sources:
    total_size += os.path.getsize(s)
  return total_size


# Merges two given data sets into one large data set with diffing information
def diff_data_sets(data1, data2):
  all_keys = set().union(data1.keys(), data2.keys())
  diffed_data = {}
  for k in all_keys:
    if k in data2:
      e = data2[k].copy()
      e['in_set_2'] = True
      if k in data1:
        prev = data1[k]
        e['prev_percentage'] = prev['percentage']
        e['prev_bytes'] = prev['bytes']
        e['prev_lines'] = prev['lines']
        e['in_set_1'] = True
      else:
        e['prev_percentage'] = 0
        e['prev_bytes'] = 0
        e['prev_lines'] = 0
        e['in_set_1'] = False
    else:
      e = data1[k].copy()
      e['prev_percentage'] = e['percentage']
      e['prev_lines'] = e['lines']
      e['prev_bytes'] = e['bytes']
      e['in_set_1'] = True
      if k in data2:
        e['percentage'] = prev['percentage']
        e['bytes'] = prev['bytes']
        e['lines'] = prev['lines']
        e['in_set_2'] = True
      else:
        e['percentage'] = 0
        e['bytes'] = 0
        e['lines'] = 0
        e['in_set_2'] = False
    e['delta'] = e['bytes'] - e['prev_bytes']
    e['delta_percentage'] = e['percentage'] - e['prev_percentage']
    e['abs_delta'] = abs(e['bytes'] - e['prev_bytes'])
    diffed_data[k] = e
  return diffed_data


# Given string s and start index that contains a (, {, <, [, ", or ', finds forward the index where the token closes (taking nesting into account)
def find_index_of_closing_token(s, start):
  start_ch = s[start]
  if start_ch == '(':
    end_ch = ')'
  elif start_ch == '{':
    end_ch = '}'
  elif start_ch == '<':
    end_ch = '>'
  elif start_ch == '[':
    end_ch = ']'
  elif start_ch == '"':
    end_ch = '"'
  elif start_ch == "'":
    end_ch = "'"
  else:
    raise Exception('Unknown start token ' + start_ch + ', string ' + s + ', start ' + start)

  i = start + 1
  nesting_count = 1
  while i < len(s):
    if s[i] == end_ch:
      nesting_count -= 1
      if nesting_count <= 0:
        return i
    elif s[i] == start_ch:
      nesting_count += 1
    i += 1
  return i


def compute_templates_collapsed_name(demangled_name):
  i = 0
  generic_template_name = 'T'
  type_names = {}
  while True:
    i = demangled_name.find('<', i)
    if i < 0:
      return demangled_name

    end = find_index_of_closing_token(demangled_name, i)
    if end < 0:
      return demangled_name

    i += 1
    template_type = demangled_name[i:end]
    if template_type in type_names:
      template_name = type_names[template_type]
    else:
      template_name = generic_template_name
      type_names[template_type] = generic_template_name
      generic_template_name = chr(ord(generic_template_name) + 1)

    demangled_name = demangled_name[:i] + template_name + demangled_name[end:]


def collapse_templates(data_set, total_source_set_size, no_function_args):
  collapsed_data_set = {}
  keys = data_set.keys()
  for k in keys:
    e = data_set[k]
    if 'demangled_name' in e:
      demangled_name = compute_templates_collapsed_name(e['demangled_name'])
      if no_function_args:
        demangled_name = function_args_removed(demangled_name)
      e['demangled_name'] = e['unminified_name'] = demangled_name
    merge_entry_to_existing(collapsed_data_set, e, total_source_set_size)
  return collapsed_data_set


def print_function_args(options):
  return 'D' in options.print_format or 'U' in options.print_format or 'M' in options.print_format


def main():
  global options, diffing_two_data_sets
  usage_str = "emdump.py prints out statistics about compiled code sizes.\npython emdump.py --file a.js [--file2 b.js]"
  parser = argparse.ArgumentParser(usage=usage_str)

  parser.add_argument('--file', dest='file', default=[], nargs='*',
                      help='Specifies the compiled JavaScript build file to analyze.')

  parser.add_argument('--file1', dest='file1', default=[], nargs='*',
                      help='Specifies the compiled JavaScript build file to analyze.')

  parser.add_argument('--symbol-map', dest='symbol_map', default='',
                      help='Specifies a filename to the symbol map file that can be used to unminify function and variable names.')

  parser.add_argument('--file2', dest='file2', default=[], nargs='*',
                      help='Specifies a second compiled JavaScript build file to analyze.')

  parser.add_argument('--symbol-map2', dest='symbol_map2', default='',
                      help='Specifies a filename to a second symbol map file that will be used to unminify function and variable names of file2.')

  parser.add_argument('--list-unaccounted', dest='list_unaccounted', type=int, default=1,
                      help='Pass --list-unaccounted=0 to skip listing a summary entry of unaccounted content')

  parser.add_argument('--dump-unaccounted-larger-than', dest='dump_unaccounted_larger_than', type=int, default=-1,
                      help='If an integer value >= 0 is specified, all unaccounted strings of content longer than the given value will be printed out to the console.\n(Note that it is common to have several unaccounted blocks, this is provided for curiosity/debugging/optimization ideas)')

  parser.add_argument('--only-unique-1', dest='only_unique_1', action='store_true', default=False,
                      help='If two data sets are specified, prints out only the symbols that are present in set 1, but not in set 2')

  parser.add_argument('--only-unique-2', dest='only_unique_2', action='store_true', default=False,
                      help='If two data sets are specified, prints out only the symbols that are present in set 2, but not in set 1')

  parser.add_argument('--only-common', dest='only_common', action='store_true', default=False,
                      help='If two data sets are specified, prints out only the symbols that are common to both data sets')

  parser.add_argument('--only-changes', dest='only_changes', action='store_true', default=False,
                      help='If two data sets are specified, prints out only the symbols that have changed size or are added/removed')

  parser.add_argument('--only-summarize', dest='only_summarize', action='store_true', default=False,
                      help='If specified, detailed information about each symbol is not printed, but only summary data is shown.')

  parser.add_argument('--filter-name', dest='filter_name', default='',
                      help='Only prints out information about symbols that contain the given filter substring in their demangled names. The filtering is always performed in lower case.')

  parser.add_argument('--filter-size', dest='filter_size', type=int, default=0,
                      help='Only prints out information about symbols that are (or were) larger than the given amount of bytes.')

  parser.add_argument('--sort', dest='sort', default='bytes',
                      help='Specifies the data column to sort output by. Possible values are: lines, bytes, delta, abs_delta, type, minified, unminified, demangled')

  parser.add_argument('--print-format', dest='print_format', default='DM',
                      help='Specifies the naming format for the symbols. Possible options are one of: m, u, d, du, dm, um, dum. Here "m" denotes minified, "u" denotes unminified, and "d" denotes demangled. Specify any combination of the characters in upper case to print out function parameters.\nDefault: DM.')

  parser.add_argument('--sort-ascending', dest='sort_ascending', action='store_true', default=False,
                      help='If true, reverses the sorting order to be ascending instead of default descending.')

  parser.add_argument('--simplify-cxx', dest='simplify_cxx', action='store_true', default=False,
                      help='Simplify C++ STL types as much as possible in the output')

  parser.add_argument('--group-templates', dest='group_templates', action='store_true', default=False,
                      help='Group/collapse all C++ templates with Foo<asdf> and Foo<qwer> to generic Foo<T>')

  options = parser.parse_args()
  options.file = options.file + options.file1

  if not options.file:
    print('Specify a set of JavaScript build output files to analyze with --file file1.js file2.js ... fileN.js.\nRun python emdump.py --help to see all options.')
    return 1

  options.filter_name = options.filter_name.lower()

  diffing_two_data_sets = len(options.file2) > 0
  if not diffing_two_data_sets:
    if options.only_unique_1:
      print('Error: Must specify two data sets with --file a.js b.js c.js --file2 d.js e.js f.js to diff in order to use --only-unique-symbols-in-set-1 option!')
      sys.exit(1)

    if options.only_unique_2:
      print('Error: Must specify two data sets with --file a.js b.js c.js --file2 d.js e.js f.js to diff in order to use --only-unique-symbols-in-set-2 option!')
      sys.exit(1)

    if options.only_common:
      print('Error: Must specify two data sets with --file a.js b.js c.js --file2 d.js e.js f.js to diff in order to use --only-common-symbols option!')
      sys.exit(1)

  # Validate column sorting input:
  valid_sort_options = ['lines', 'bytes', 'delta', 'abs_delta', 'type', 'minified', 'unminified', 'demangled']
  if options.sort not in valid_sort_options:
    print('Invalid sort option ' + options.sort + ' specified! Choose one of: ' + ', '.join(valid_sort_options) + '.')
    sys.exit(1)
  if options.sort == 'minified':
    options.sort = 'minified_name'
  if options.sort == 'unminified':
    options.sort = 'unminified_name'
  if options.sort == 'demangled':
    options.sort = 'demangled_name'

  if 'delta' in options.sort and not diffing_two_data_sets:
    print('Error: Must specify two data sets with --file a.js b.js c.js --file2 d.js e.js f.js to diff in order to use --sort=' + options.sort)
    sys.exit(1)

  # Autoguess .symbols file location based on default Emscripten build output, to save the need to type it out in the common case
  options.symbol_map = guess_symbol_map_file_location(options.file, options.symbol_map)
  options.symbol_map2 = guess_symbol_map_file_location(options.file2, options.symbol_map2)

  symbol_map1 = read_symbol_map(options.symbol_map)
  symbol_map2 = read_symbol_map(options.symbol_map2)

  set1_size = count_file_set_size(options.file)
  data1 = {}
  for s in options.file:
    data = analyze_source_file(s, set1_size, symbol_map1)
    merge_to_data_set(data1, data, set1_size)

  set2_size = count_file_set_size(options.file2)
  data2 = {}
  for s in options.file2:
    data = analyze_source_file(s, set2_size, symbol_map2)
    merge_to_data_set(data2, data, set2_size)

  find_demangled_names(data1)
  find_demangled_names(data2)

  if options.group_templates:
    data1 = collapse_templates(data1, set1_size, not print_function_args(options))
    data2 = collapse_templates(data2, set2_size, not print_function_args(options))

  if diffing_two_data_sets:
    diffed_data = diff_data_sets(data1, data2)
    if not options.only_summarize:
      print_symbol_info(diffed_data, set2_size)
      print('')
    print('set 2 is %d bytes, which is %+.2f%% %s than set 1 size (%d bytes)' % (set2_size, (set2_size - set1_size) * 100.0 / set2_size, 'more' if set2_size > set1_size else 'less', set1_size))
    uniq_compare(data1, data2)
    common_compare(data1, data2)
  else:
    if not options.only_summarize:
      print_symbol_info(data1, set1_size)
    # TODO: print some kind of summary?

  return 0


if __name__ == '__main__':
  sys.exit(main())
