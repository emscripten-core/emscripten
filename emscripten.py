#!/usr/bin/python2

import argparse
import json
import os
import subprocess
import sys
import tempfile
import tools.shared as shared


# Temporary files that should be deleted once teh program is finished.
TEMP_FILES_TO_CLEAN = []
# The data layout used by llvm-gcc (as opposed to clang).
GCC_DATA_LAYOUT = ('target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16'
                   '-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64'
                   '-v128:128:128-a0:0:64-f80:32:32-f128:128:128-n8:16:32"')


def path_from_root(*target):
  """Returns the absolute path to the target from the emscripten root."""
  abspath = os.path.abspath(os.path.dirname(__file__))
  return os.path.join(os.path.sep, *(abspath.split(os.sep) + list(target)))


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
    out = get_temp_file('.bc')
    ret = subprocess.call([shared.LLVM_AS, '-o=-', filepath], stdout=out)
    out.close()
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
    out = get_temp_file('.ll')
    command = [shared.LLVM_DIS, '-o=-', filepath] + shared.LLVM_DIS_OPTS
    ret = subprocess.call(command, stdout=out)
    out.close()
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
  out = get_temp_file('.bc')
  opts = shared.pick_llvm_opts(3, True)
  ret = subprocess.call([shared.LLVM_OPT, '-o=-', filepath] + opts, stdout=out)
  out.close()
  if ret != 0: raise RuntimeError('Could not optimize %s.' % filepath)
  return out.name


def link(*objects):
  """Links multiple LLVM bitcode files into a single file.

  Args:
    objects: The bitcode files to link.

  Returns:
    The path to the linked file.
  """
  out = get_temp_file('.bc')
  ret = subprocess.call([shared.LLVM_LINK] + list(objects), stdout=out)
  out.close()
  if ret != 0: raise RuntimeError('Could not link %s.' % objects)
  return out.name


def compile_malloc(compiler):
  """Compiles dlmalloc to LLVM bitcode.

  Args:
    compiler: The compiler command to use, a path to either clang or llvm-gcc.

  Returns:
    The path to the compiled dlmalloc as an LLVM bitcode (.bc) file.
  """
  src = path_from_root('src', 'dlmalloc.c')
  out = get_temp_file('.bc')
  includes = '-I' + path_from_root('src', 'include')
  command = [compiler, '-c', '-g', '-emit-llvm', '-m32', '-o-', includes, src]
  ret = subprocess.call(command, stdout=out)
  out.close()
  if ret != 0: raise RuntimeError('Could not compile dlmalloc.')
  return out.name


def determine_compiler(filepath):
  """Determines whether a given file uses llvm-gcc or clang data layout.

  Args:
    filepath: The .bc or .ll file containing the bitcode/assembly to test.

  Returns:
    The path to the compiler, either llvm-gcc or clang.
  """
  assembly = open(disassemble(filepath)).read()
  is_gcc = GCC_DATA_LAYOUT in assembly
  return shared.to_cc(shared.LLVM_GCC if is_gcc else shared.CLANG)


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
  data = open(infile, 'r').read()
  compiler = path_from_root('src', 'compiler.js')
  subprocess.Popen(shared.COMPILER_ENGINE + [compiler],
                   stdin=subprocess.PIPE,
                   stdout=outfile,
                   cwd=path_from_root('src'),
                   stderr=subprocess.STDOUT).communicate(settings + '\n' + data)
  outfile.close()


def main(args):
  # Construct a final linked and disassembled file.
  if args.dlmalloc or args.optimize or not has_annotations(args.infile):
    args.infile = assemble(args.infile)
    if args.dlmalloc:
      malloc = compile_malloc(determine_compiler(args.infile))
      args.infile = link(args.infile, malloc)
    if args.optimize: args.infile = optimize(args.infile)
  args.infile = disassemble(args.infile)

  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.split('=', 1)
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

  # Compile the assembly to Javascript.
  emscript(args.infile, json.dumps(settings), args.outfile)


if __name__ == '__main__':
  parser = argparse.ArgumentParser(
      description='Compile LLVM assembly to Javascript.',
      epilog='You should have an ~/.emscripten file set up; see settings.py.')
  parser.add_argument('infile',
                      help='The LLVM assembly file to compile, either in '
                           'human-readable (*.ll) or in bitcode (*.bc) format.')
  parser.add_argument('-O', '--optimize',
                      default=False,
                      action='store_true',
                      help='Run LLVM optimizations on the input.')
  parser.add_argument('-m', '--dlmalloc',
                      default=False,
                      action='store_true',
                      help='Use dlmalloc. Without, uses a dummy allocator.')
  parser.add_argument('-o', '--outfile',
                      default=sys.stdout,
                      type=argparse.FileType('w'),
                      help='Where to write the output; defaults to stdout.')
  parser.add_argument('-s', '--settings',
                      default=[],
                      nargs=argparse.ZERO_OR_MORE,
                      metavar='FOO=BAR',
                      help='Overrides for settings defined in settings.js.')

  try:
    main(parser.parse_args())
  finally:
    # Clean up temporary files.
    for filename in TEMP_FILES_TO_CLEAN:
      os.unlink(filename)
