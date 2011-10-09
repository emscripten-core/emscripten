#!/usr/bin/python

import json
import optparse
import os
import subprocess
import re
import sys
import tempfile
from tools import shared


# Temporary files that should be deleted once the program is finished.
TEMP_FILES_TO_CLEAN = []


__rootpath__ = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  """Returns the absolute path for which the given path elements are
  relative to the emscripten root.
  """
  return os.path.join(__rootpath__, *pathelems)


def get_temp_file(suffix):
  """Returns a named temp file  with the given prefix."""
  named_file = tempfile.NamedTemporaryFile(
      dir=shared.TEMP_DIR, suffix=suffix, delete=False)
  TEMP_FILES_TO_CLEAN.append(named_file.name)
  return named_file


def assemble(filepath):
  """Converts human-readable LLVM assembly to binary LLVM bitcode.

  Args:
    filepath: The path to the file to assemble. If the name ends with ".bc", the
      file is assumed to be in bitcode format already.

  Returns:
    The path to the assembled file.
  """
  if not filepath.endswith('.bc'):
    command = [shared.LLVM_AS, '-o=-', filepath]
    with get_temp_file('.bc') as out: ret = subprocess.call(command, stdout=out)
    if ret != 0: raise RuntimeError('Could not assemble %s.' % filepath)
    filepath = out.name
  return filepath


def disassemble(filepath):
  """Converts binary LLVM bitcode to human-readable LLVM assembly.

  Args:
    filepath: The path to the file to disassemble. If the name ends with ".ll",
      the file is assumed to be in human-readable assembly format already.

  Returns:
    The path to the disassembled file.
  """
  if not filepath.endswith('.ll'):
    command = [shared.LLVM_DIS, '-o=-', filepath] + shared.LLVM_DIS_OPTS
    with get_temp_file('.ll') as out: ret = subprocess.call(command, stdout=out)
    if ret != 0: raise RuntimeError('Could not disassemble %s.' % filepath)
    filepath = out.name
  return filepath


def optimize(filepath):
  """Runs LLVM's optimization passes on a given bitcode file.

  Args:
    filepath: The path to the bitcode file to optimize.

  Returns:
    The path to the optimized file.
  """
  command = [shared.LLVM_OPT, '-o=-', filepath] + shared.pick_llvm_opts(3, True)
  with get_temp_file('.bc') as out: ret = subprocess.call(command, stdout=out)
  if ret != 0: raise RuntimeError('Could not optimize %s.' % filepath)
  return out.name


def link(*objects):
  """Links multiple LLVM bitcode files into a single file.

  Args:
    objects: The bitcode files to link.

  Returns:
    The path to the linked file.
  """
  command = [shared.LLVM_LINK] + list(objects)
  with get_temp_file('.bc') as out: ret = subprocess.call(command, stdout=out)
  if ret != 0: raise RuntimeError('Could not link %s.' % objects)
  return out.name


def compile_malloc():
  """Compiles dlmalloc to LLVM bitcode.

  Returns:
    The path to the compiled dlmalloc as an LLVM bitcode (.bc) file.
  """
  src = path_from_root('src', 'dlmalloc.c')
  includes = '-I' + path_from_root('src', 'include')
  command = [shared.CLANG, '-c', '-g', '-emit-llvm'] + shared.COMPILER_OPTS + ['-o-', includes, src]
  with get_temp_file('.bc') as out: ret = subprocess.call(command, stdout=out)
  if ret != 0: raise RuntimeError('Could not compile dlmalloc.')
  return out.name


def has_annotations(filepath):
  """Tests whether an assembly file contains annotations.

  Args:
    filepath: The .ll file containing the assembly to check.

  Returns:
    Whether the provided file is valid assembly and has annotations.
  """
  return filepath.endswith('.ll') and '[#uses=' in open(filepath).read()


def emscript(infile, settings, outfile):
  """Runs the emscripten LLVM-to-JS compiler.

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted string of settings that overrides the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """
  settings_file = get_temp_file('.txt').name # Save settings to a file to work around v8 issue 1579
  s = open(settings_file, 'w')
  s.write(settings)
  s.close()
  compiler = path_from_root('src', 'compiler.js')
  shared.run_js(shared.COMPILER_ENGINE, compiler, [settings_file, infile], stdout=outfile, stderr=subprocess.STDOUT, cwd=path_from_root('src'))
  outfile.close()


def main(args):
  # Construct a final linked and disassembled file.
  if args.dlmalloc or args.optimize or not has_annotations(args.infile):
    args.infile = assemble(args.infile)
    if args.dlmalloc:
      malloc = compile_malloc()
      args.infile = link(args.infile, malloc)
    if args.optimize: args.infile = optimize(args.infile)
  args.infile = disassemble(args.infile)

  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    settings[name] = json.loads(value)

  # Adjust sign correction for dlmalloc.
  if args.dlmalloc:
    CORRECT_SIGNS = settings.get('CORRECT_SIGNS', 0)
    if CORRECT_SIGNS in (0, 2):
      path = path_from_root('src', 'dlmalloc.c')
      old_lines = settings.get('CORRECT_SIGNS_LINES', [])
      line_nums = [4816, 4191, 4246, 4199, 4205, 4235, 4227]
      lines = old_lines + [path + ':' + str(i) for i in line_nums]
      settings['CORRECT_SIGNS'] = 2
      settings['CORRECT_SIGNS_LINES'] = lines

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
    settings['C_DEFINES'] = defines

  # Compile the assembly to Javascript.
  emscript(args.infile, json.dumps(settings), args.outfile)


if __name__ == '__main__':
  parser = optparse.OptionParser(
      usage='usage: %prog [-h] [-O] [-m] [-H HEADERS] [-o OUTFILE] [-s FOO=BAR]* infile',
      description=('Compile an LLVM assembly file to Javascript. Accepts both '
                   'human-readable (*.ll) and bitcode (*.bc) formats.'),
      epilog='You should have an ~/.emscripten file set up; see settings.py.')
  parser.add_option('-O', '--optimize',
                    default=False,
                    action='store_true',
                    help='Run LLVM optimizations on the input.')
  parser.add_option('-m', '--dlmalloc',
                    default=False,
                    action='store_true',
                    help='Use dlmalloc. Without, uses a dummy allocator. Warning: This will force a re-disassembly, so .ll line numbers will change.')
  parser.add_option('-H', '--headers',
                    default=[],
                    action='append',
                    help='System headers (comma separated) whose #defines should be exposed to the compiled code.')
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

  try:
    main(keywords)
  finally:
    for filename in TEMP_FILES_TO_CLEAN:
      os.unlink(filename)
