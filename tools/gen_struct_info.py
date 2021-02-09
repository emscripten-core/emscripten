#!/usr/bin/env python3
# coding=utf-8
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This tool extracts information about structs and defines from the C headers.

The JSON input format is as follows:
[
  {
    'file': 'some/header.h',
    'structs': {
      'struct_name': [
        'field1',
        'field2',
        'field3',
        {
          'field4': [
            'nested1',
            'nested2',
            {
              'nested3': [
                'deep_nested1',
                ...
              ]
            }
            ...
          ]
        },
        'field5'
      ],
      'other_struct': [
        'field1',
        'field2',
        ...
      ]
    },
    'defines': [
      'DEFINE_1',
      'DEFINE_2',
      ['f', 'FLOAT_DEFINE'],
      'DEFINE_3',
      ...
    ]
  },
  {
    'file': 'some/other/header.h',
    ...
  }
]

Please note that the 'f' for 'FLOAT_DEFINE' is just the format passed to printf(), you can put anything printf() understands.
If you call this script with the flag "-f" and pass a header file, it will create an automated boilerplate for you.

The JSON output format is based on the return value of Runtime.generateStructInfo().
{
  'structs': {
    'struct_name': {
      '__size__': <the struct's size>,
      'field1': <field1's offset>,
      'field2': <field2's offset>,
      'field3': <field3's offset>,
      'field4': {
        '__size__': <field4's size>,
        'nested1': <nested1's offset>,
        ...
      },
      ...
    }
  },
  'defines': {
    'DEFINE_1': <DEFINE_1's value>,
    ...
  }
}
"""

import sys
import os
import re
import json
import argparse
import tempfile
import subprocess

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools import shared
from tools import system_libs

QUIET = (__name__ != '__main__')
DEBUG = False


def show(msg):
  if shared.DEBUG or not QUIET:
    sys.stderr.write('gen_struct_info: %s\n' % msg)


# The following three functions generate C code. The output of the compiled code will be
# parsed later on and then put back together into a dict structure by parse_c_output().
#
# Example:
#   c_descent('test1', code)
#   c_set('item', 'i%i', '111', code)
#   c_set('item2', 'i%i', '9', code)
#   c_set('item3', 's%s', '"Hello"', code)
#   c_ascent(code)
#   c_set('outer', 'f%f', '0.999', code)
#
# Will result in:
#   {
#     'test1': {
#       'item': 111,
#       'item2': 9,
#       'item3': 'Hello',
#     },
#     'outer': 0.999
#   }
def c_set(name, type_, value, code):
  code.append('printf("K' + name + '\\n");')
  code.append('printf("V' + type_ + '\\n", ' + value + ');')


def c_descent(name, code):
  code.append('printf("D' + name + '\\n");')


def c_ascent(code):
  code.append('printf("A\\n");')


def parse_c_output(lines):
  result = {}
  cur_level = result
  parent = []
  key = None

  for line in lines:
    arg = line[1:].strip()
    if line[0] == 'K':
      # This is a key
      key = arg
    elif line[0] == 'V':
      # A value
      if arg[0] == 'i':
        arg = int(arg[1:])
      elif arg[0] == 'f':
        arg = float(arg[1:])
      elif arg[0] == 's':
        arg = arg[1:]

      cur_level[key] = arg
    elif line[0] == 'D':
      # Remember the current level as the last parent.
      parent.append(cur_level)

      # We descend one level.
      cur_level[arg] = {}
      cur_level = cur_level[arg]
    elif line[0] == 'A':
      # We return to the parent dict. (One level up.)
      cur_level = parent.pop()

  return result


def gen_inspect_code(path, struct, code):
  if path[0][-1] == '#':
    path[0] = path[0][:-1]
    prefix = ''
  else:
    prefix = 'struct '

  c_descent(path[-1], code)

  if len(path) == 1:
    c_set('__size__', 'i%zu', 'sizeof (' + prefix + path[0] + ')', code)
  else:
    c_set('__size__', 'i%zu', 'sizeof ((' + prefix + path[0] + ' *)0)->' + '.'.join(path[1:]), code)
    # c_set('__offset__', 'i%zu', 'offsetof(' + prefix + path[0] + ', ' + '.'.join(path[1:]) + ')', code)

  for field in struct:
    if isinstance(field, dict):
      # We have to recurse to inspect the nested dict.
      fname = list(field.keys())[0]
      gen_inspect_code(path + [fname], field[fname], code)
    else:
      c_set(field, 'i%zu', 'offsetof(' + prefix + path[0] + ', ' + '.'.join(path[1:] + [field]) + ')', code)

  c_ascent(code)


def inspect_headers(headers, cpp_opts):
  code = ['#include <stdio.h>', '#include <stddef.h>']
  for header in headers:
    code.append('#include "' + header['name'] + '"')

  code.append('int main() {')
  c_descent('structs', code)
  for header in headers:
    for name, struct in header['structs'].items():
      gen_inspect_code([name], struct, code)

  c_ascent(code)
  c_descent('defines', code)
  for header in headers:
    for name, type_ in header['defines'].items():
      # Add the necessary python type, if missing.
      if '%' not in type_:
        if type_[-1] in ('d', 'i', 'u'):
          # integer
          type_ = 'i%' + type_
        elif type_[-1] in ('f', 'F', 'e', 'E', 'g', 'G'):
          # float
          type_ = 'f%' + type_
        elif type_[-1] in ('x', 'X', 'a', 'A', 'c', 's'):
          # hexadecimal or string
          type_ = 's%' + type_

      c_set(name, type_, name, code)

  code.append('return 0;')
  code.append('}')

  # Write the source code to a temporary file.
  src_file = tempfile.mkstemp('.c')
  show('Generating C code... ' + src_file[1])
  os.write(src_file[0], shared.asbytes('\n'.join(code)))

  js_file = tempfile.mkstemp('.js')

  # Check sanity early on before populating the cache with libcompiler_rt
  # If we don't do this the parallel build of compiler_rt will run while holding the cache
  # lock and with EM_EXCLUSIVE_CACHE_ACCESS set causing N processes to race to run sanity checks.
  # While this is not in itself serious problem it is wasteful and noise on stdout.
  # For the same reason we run this early in embuilder.py and emcc.py.
  # TODO(sbc): If we can remove EM_EXCLUSIVE_CACHE_ACCESS then this would not longer be needed.
  shared.check_sanity()

  compiler_rt = system_libs.Library.get_usable_variations()['libcompiler_rt'].get_path()

  # Close all unneeded FDs.
  os.close(src_file[0])
  os.close(js_file[0])

  info = []
  # Compile the program.
  show('Compiling generated code...')
  # -Oz optimizes enough to avoid warnings on code size/num locals
  cmd = [shared.EMCC] + cpp_opts + ['-o', js_file[1], src_file[1],
                                    '-O0',
                                    '-Werror',
                                    '-Wno-format',
                                    '-nostdlib',
                                    compiler_rt,
                                    '-I', shared.path_from_root(),
                                    '-s', 'BOOTSTRAPPING_STRUCT_INFO=1',
                                    '-s', 'STRICT',
                                    # Use SINGLE_FILE=1 so there is only a single
                                    # file to cleanup.
                                    '-s', 'SINGLE_FILE']

  # Default behavior for emcc is to warn for binaryen version check mismatches
  # so we should try to match that behavior.
  cmd += ['-Wno-error=version-check']

  # TODO(sbc): Remove this one we remove the test_em_config_env_var test
  cmd += ['-Wno-deprecated']

  if shared.Settings.LTO:
    cmd += ['-flto=' + shared.Settings.LTO]

  show(shared.shlex_join(cmd))
  try:
    subprocess.check_call(cmd)
  except subprocess.CalledProcessError as e:
    sys.stderr.write('FAIL: Compilation failed!: %s\n' % e.cmd)
    sys.exit(1)

  # Run the compiled program.
  show('Calling generated program... ' + js_file[1])
  info = shared.run_js_tool(js_file[1], stdout=shared.PIPE).splitlines()

  # Remove all temporary files.
  os.unlink(src_file[1])

  if os.path.exists(js_file[1]):
    os.unlink(js_file[1])

  # Parse the output of the program into a dict.
  return parse_c_output(info)


def merge_info(target, src):
  for key, value in src['defines'].items():
    if key in target['defines']:
      raise Exception('duplicate define: %s' % key)
    target['defines'][key] = value

  for key, value in src['structs'].items():
    if key in target['structs']:
      raise Exception('duplicate struct: %s' % key)
    target['structs'][key] = value


def inspect_code(headers, cpp_opts):
  if not DEBUG:
    info = inspect_headers(headers, cpp_opts)
  else:
    info = {'defines': {}, 'structs': {}}
    for header in headers:
      merge_info(info, inspect_headers([header], cpp_opts))
  return info


def parse_json(path):
  header_files = []

  with open(path, 'r') as stream:
    # Remove comments before loading the JSON.
    data = json.loads(re.sub(r'//.*\n', '', stream.read()))

  if not isinstance(data, list):
    data = [data]

  for item in data:
    for key in item.keys():
      if key not in ['file', 'defines', 'structs']:
        raise 'Unexpected key in json file: %s' % key

    header = {'name': item['file'], 'structs': {}, 'defines': {}}
    for name, data in item.get('structs', {}).items():
      if name in header['structs']:
        show('WARN: Description of struct "' + name + '" in file "' + item['file'] + '" replaces an existing description!')

      header['structs'][name] = data

    for part in item.get('defines', []):
      if not isinstance(part, list):
        # If no type is specified, assume integer.
        part = ['i', part]

      if part[1] in header['defines']:
        show('WARN: Description of define "' + part[1] + '" in file "' + item['file'] + '" replaces an existing description!')

      header['defines'][part[1]] = part[0]

    header_files.append(header)

  return header_files


def output_json(obj, stream=None):
  if stream is None:
    stream = sys.stdout
  elif isinstance(stream, str):
    stream = open(stream, 'w')

  json.dump(obj, stream, indent=4, sort_keys=True)

  stream.write('\n')
  stream.close()


def main(args):
  global QUIET

  default_json_files = [
      shared.path_from_root('src', 'struct_info.json'),
      shared.path_from_root('src', 'struct_info_internal.json')
  ]
  parser = argparse.ArgumentParser(description='Generate JSON infos for structs.')
  parser.add_argument('json', nargs='*',
                      help='JSON file with a list of structs and their fields (defaults to src/struct_info.json)',
                      default=default_json_files)
  parser.add_argument('-q', dest='quiet', action='store_true', default=False,
                      help='Don\'t output anything besides error messages.')
  parser.add_argument('-o', dest='output', metavar='path', default=None,
                      help='Path to the JSON file that will be written. If omitted, the generated data will be printed to stdout.')
  parser.add_argument('-I', dest='includes', metavar='dir', action='append', default=[],
                      help='Add directory to include search path')
  parser.add_argument('-D', dest='defines', metavar='define', action='append', default=[],
                      help='Pass a define to the preprocessor')
  parser.add_argument('-U', dest='undefines', metavar='undefine', action='append', default=[],
                      help='Pass an undefine to the preprocessor')
  args = parser.parse_args(args)

  QUIET = args.quiet

  # Avoid parsing problems due to gcc specifc syntax.
  cpp_opts = ['-D_GNU_SOURCE']

  # Add the user options to the list as well.
  for path in args.includes:
    cpp_opts.append('-I' + path)

  for arg in args.defines:
    cpp_opts.append('-D' + arg)

  for arg in args.undefines:
    cpp_opts.append('-U' + arg)

  # Look for structs in all passed headers.
  info = {'defines': {}, 'structs': {}}

  for f in args.json:
    # This is a JSON file, parse it.
    header_files = parse_json(f)
    # Inspect all collected structs.
    info_fragment = inspect_code(header_files, cpp_opts)
    merge_info(info, info_fragment)

  output_json(info, args.output)
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
