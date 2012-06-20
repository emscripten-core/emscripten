#!/usr/bin/env python

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

import json
import optparse
import os
import subprocess
import re
import sys

if not os.environ.get('EMSCRIPTEN_SUPPRESS_USAGE_WARNING'):
  print >> sys.stderr, '''
==============================================================
WARNING: You should normally never use this! Use emcc instead.
==============================================================
  '''

from tools import shared

__rootpath__ = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  """Returns the absolute path for which the given path elements are
  relative to the emscripten root.
  """
  return os.path.join(__rootpath__, *pathelems)

temp_files = shared.TempFiles()


def emscript(infile, settings, outfile, libraries=[]):
  """Runs the emscripten LLVM-to-JS compiler.

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted string of settings that overrides the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """
  settings_file = temp_files.get('.txt').name # Save settings to a file to work around v8 issue 1579
  s = open(settings_file, 'w')
  s.write(settings)
  s.close()
  compiler = path_from_root('src', 'compiler.js')
  shared.run_js(compiler, shared.COMPILER_ENGINE, [settings_file, infile] + libraries, stdout=outfile, cwd=path_from_root('src'))
  outfile.close()


def main(args):
  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    settings[name] = json.loads(value)

  # Add header defines to settings
  defines = {}
  include_root = path_from_root('system', 'include')
  headers = args.headers[0].split(',') if len(args.headers) > 0 else []
  seen_headers = set()
  while len(headers) > 0:
    header = headers.pop(0)
    if not os.path.isabs(header):
      header = os.path.join(include_root, header)
    seen_headers.add(header)
    for line in open(header, 'r'):
      line = line.replace('\t', ' ')
      m = re.match('^ *# *define +(?P<name>[-\w_.]+) +\(?(?P<value>[-\w_.|]+)\)?.*', line)
      if not m:
        # Catch enum defines of a very limited sort
        m = re.match('^ +(?P<name>[A-Z_\d]+) += +(?P<value>\d+).*', line)
      if m:
        if m.group('name') != m.group('value'):
          defines[m.group('name')] = m.group('value')
        #else:
        #  print 'Warning: %s #defined to itself' % m.group('name') # XXX this can happen if we are set to be equal to an enum (with the same name)
      m = re.match('^ *# *include *["<](?P<name>[\w_.-/]+)[">].*', line)
      if m:
        # Find this file
        found = False
        for w in [w for w in os.walk(include_root)]:
          for f in w[2]:
            curr = os.path.join(w[0], f)
            if curr.endswith(m.group('name')) and curr not in seen_headers:
              headers.append(curr)
              found = True
              break
          if found: break
        #assert found, 'Could not find header: ' + m.group('name')
  if len(defines) > 0:
    def lookup(value):
      try:
        while not unicode(value).isnumeric():
          value = defines[value]
        return value
      except:
        pass
      try: # 0x300 etc.
        value = eval(value)
        return value
      except:
        pass
      try: # CONST1|CONST2
        parts = map(lookup, value.split('|'))
        value = reduce(lambda a, b: a|b, map(eval, parts))
        return value
      except:
        pass
      return None
    for key, value in defines.items():
      value = lookup(value)
      if value is not None:
        defines[key] = str(value)
      else:
        del defines[key]
    #print >> sys.stderr, 'new defs:', str(defines).replace(',', ',\n  '), '\n\n'
    settings.setdefault('C_DEFINES', {}).update(defines)

  # libraries
  libraries = args.libraries[0].split(',') if len(args.libraries) > 0 else []

  # Compile the assembly to Javascript.
  emscript(args.infile, json.dumps(settings), args.outfile, libraries)

if __name__ == '__main__':
  parser = optparse.OptionParser(
      usage='usage: %prog [-h] [-H HEADERS] [-o OUTFILE] [-s FOO=BAR]* infile',
      description=('You should normally never use this! Use emcc instead. '
                   'This is a wrapper around the JS compiler, converting .ll to .js.'),
      epilog='')
  parser.add_option('-H', '--headers',
                    default=[],
                    action='append',
                    help='System headers (comma separated) whose #defines should be exposed to the compiled code.')
  parser.add_option('-L', '--libraries',
                    default=[],
                    action='append',
                    help='Library files (comma separated) to use in addition to those in emscripten src/library_*.')
  parser.add_option('-o', '--outfile',
                    default=sys.stdout,
                    help='Where to write the output; defaults to stdout.')
  parser.add_option('-s', '--setting',
                    dest='settings',
                    default=[],
                    action='append',
                    metavar='FOO=BAR',
                    help=('Overrides for settings defined in settings.js. '
                          'May occur multiple times.'))

  # Convert to the same format that argparse would have produced.
  keywords, positional = parser.parse_args()
  if len(positional) != 1:
    raise RuntimeError('Must provide exactly one positional argument.')
  keywords.infile = os.path.abspath(positional[0])
  if isinstance(keywords.outfile, basestring):
    keywords.outfile = open(keywords.outfile, 'w')

  temp_files.run_and_clean(lambda: main(keywords))

