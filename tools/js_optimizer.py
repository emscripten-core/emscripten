#!/usr/bin/env python3
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import subprocess
import re
import json
import shutil

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools.toolchain_profiler import ToolchainProfiler
from tools.utils import path_from_root
from tools import building, config, shared, utils

temp_files = shared.get_temp_files()


ACORN_OPTIMIZER = path_from_root('tools/acorn-optimizer.mjs')

NUM_CHUNKS_PER_CORE = 3
MIN_CHUNK_SIZE = int(os.environ.get('EMCC_JSOPT_MIN_CHUNK_SIZE') or 512 * 1024) # configuring this is just for debugging purposes
MAX_CHUNK_SIZE = int(os.environ.get('EMCC_JSOPT_MAX_CHUNK_SIZE') or 5 * 1024 * 1024)

WINDOWS = sys.platform.startswith('win')

DEBUG = os.environ.get('EMCC_DEBUG')

func_sig = re.compile(r'function ([_\w$]+)\(')
func_sig_json = re.compile(r'\["defun", ?"([_\w$]+)",')
import_sig = re.compile(r'(var|const) ([_\w$]+ *=[^;]+);')


def get_acorn_cmd():
  node = config.NODE_JS
  if not any('--stack-size' in arg for arg in node):
    # Use an 8Mb stack (rather than the ~1Mb default) when running the
    # js optimizer since larger inputs can cause terser to use a lot of stack.
    node.append('--stack-size=8192')
  return node + [ACORN_OPTIMIZER]


def split_funcs(js):
  # split properly even if there are no newlines,
  # which is important for deterministic builds (as which functions
  # are in each chunk may differ, so we need to split them up and combine
  # them all together later and sort them deterministically)
  parts = ['function ' + part for part in js.split('function ')[1:]]
  funcs = []
  for func in parts:
    m = func_sig.search(func)
    if not m:
      continue
    ident = m.group(1)
    assert ident
    funcs.append((ident, func))
  return funcs


class Minifier:
  """minification support. We calculate minification of
  globals here, then pass that into the parallel acorn-optimizer.mjs runners which
  perform minification of locals.
  """

  def __init__(self, js):
    self.js = js
    self.symbols_file = None
    self.profiling_funcs = False

  def minify_shell(self, shell, minify_whitespace):
    # Run through acorn-optimizer.mjs to find and minify the global symbols
    # We send it the globals, which it parses at the proper time. JS decides how
    # to minify all global names, we receive a dictionary back, which is then
    # used by the function processors

    shell = shell.replace('0.0', '13371337') # avoid optimizer doing 0.0 => 0

    # Find all globals in the JS functions code

    if not self.profiling_funcs:
      self.globs = [m.group(1) for m in func_sig.finditer(self.js)]
      if len(self.globs) == 0:
        self.globs = [m.group(1) for m in func_sig_json.finditer(self.js)]
    else:
      self.globs = []

    with temp_files.get_file('.minifyglobals.js') as temp_file:
      with open(temp_file, 'w') as f:
        f.write(shell)
        f.write('\n')
        f.write('// EXTRA_INFO:' + json.dumps(self.serialize()))

      cmd = get_acorn_cmd() + [temp_file, 'minifyGlobals']
      if minify_whitespace:
        cmd.append('--minify-whitespace')
      output = shared.run_process(cmd, stdout=subprocess.PIPE).stdout

    assert len(output) and not output.startswith('Assertion failed'), 'Error in js optimizer: ' + output
    code, metadata = output.split('// EXTRA_INFO:')
    self.globs = json.loads(metadata)

    if self.symbols_file:
      mapping = '\n'.join(f'{value}:{key}' for key, value in self.globs.items())
      utils.write_file(self.symbols_file, mapping + '\n')
      print('wrote symbol map file to', self.symbols_file, file=sys.stderr)

    return code.replace('13371337', '0.0')

  def serialize(self):
    return {
      'globals': self.globs
    }


start_funcs_marker = '// EMSCRIPTEN_START_FUNCS\n'
end_funcs_marker = '// EMSCRIPTEN_END_FUNCS\n'
start_asm_marker = '// EMSCRIPTEN_START_ASM\n'
end_asm_marker = '// EMSCRIPTEN_END_ASM\n'


# Given a set of functions of form (ident, text), and a preferred chunk size,
# generates a set of chunks for parallel processing and caching.
@ToolchainProfiler.profile()
def chunkify(funcs, chunk_size):
  chunks = []
  # initialize reasonably, the rest of the funcs we need to split out
  curr = []
  total_size = 0
  for func in funcs:
    curr_size = len(func[1])
    if total_size + curr_size < chunk_size:
      curr.append(func)
      total_size += curr_size
    else:
      chunks.append(curr)
      curr = [func]
      total_size = curr_size
  if curr:
    chunks.append(curr)
    curr = None
  return [''.join(func[1] for func in chunk) for chunk in chunks] # remove function names


@ToolchainProfiler.profile_block('js_optimizer.run_on_file')
def run_on_file(filename, passes, extra_info=None):
  with ToolchainProfiler.profile_block('js_optimizer.split_markers'):
    if not isinstance(passes, list):
      passes = [passes]

    js = utils.read_file(filename)
    if os.linesep != '\n':
      js = js.replace(os.linesep, '\n') # we assume \n in the splitting code

    # Find markers
    start_funcs = js.find(start_funcs_marker)
    end_funcs = js.rfind(end_funcs_marker)

    if start_funcs < 0 or end_funcs < start_funcs:
      shared.exit_with_error('invalid input file. Did not contain appropriate markers. (start_funcs: %s, end_funcs: %s' % (start_funcs, end_funcs))

    minify_globals = 'minifyNames' in passes
    if minify_globals:
      passes = [p if p != 'minifyNames' else 'minifyLocals' for p in passes]
      start_asm = js.find(start_asm_marker)
      end_asm = js.rfind(end_asm_marker)
      assert (start_asm >= 0) == (end_asm >= 0)

    closure = 'closure' in passes
    if closure:
      passes = [p for p in passes if p != 'closure'] # we will do it manually

    cleanup = 'cleanup' in passes
    if cleanup:
      passes = [p for p in passes if p != 'cleanup'] # we will do it manually

  if not minify_globals:
    with ToolchainProfiler.profile_block('js_optimizer.no_minify_globals'):
      pre = js[:start_funcs + len(start_funcs_marker)]
      post = js[end_funcs + len(end_funcs_marker):]
      js = js[start_funcs + len(start_funcs_marker):end_funcs]
      # can have Module[..] and inlining prevention code, push those to post
      finals = []

      def process(line):
        if line and (line.startswith(('Module[', 'if (globalScope)')) or line.endswith('["X"]=1;')):
          finals.append(line)
          return False
        return True

      js = '\n'.join(line for line in js.split('\n') if process(line))
      post = '\n'.join(finals) + '\n' + post
      post = end_funcs_marker + post
  else:
    with ToolchainProfiler.profile_block('js_optimizer.minify_globals'):
      # We need to split out the asm shell as well, for minification
      pre = js[:start_asm + len(start_asm_marker)]
      post = js[end_asm:]
      asm_shell = js[start_asm + len(start_asm_marker):start_funcs + len(start_funcs_marker)] + '''
EMSCRIPTEN_FUNCS();
''' + js[end_funcs + len(end_funcs_marker):end_asm + len(end_asm_marker)]
      js = js[start_funcs + len(start_funcs_marker):end_funcs]

      # we assume there is a maximum of one new name per line
      minifier = Minifier(js)

      def check_symbol_mapping(p):
        if p.startswith('symbolMap='):
          minifier.symbols_file = p.split('=', 1)[1]
          return False
        if p == 'profilingFuncs':
          minifier.profiling_funcs = True
          return False
        return True

      passes = [p for p in passes if check_symbol_mapping(p)]
      asm_shell_pre, asm_shell_post = minifier.minify_shell(asm_shell, '--minify-whitespace' in passes).split('EMSCRIPTEN_FUNCS();')
      asm_shell_post = asm_shell_post.replace('});', '})')
      pre += asm_shell_pre + '\n' + start_funcs_marker
      post = end_funcs_marker + asm_shell_post + post

      minify_info = minifier.serialize()

      if extra_info:
        for key, value in extra_info.items():
          assert key not in minify_info or value == minify_info[key], [key, value, minify_info[key]]
          minify_info[key] = value

      # if DEBUG:
      #   print >> sys.stderr, 'minify info:', minify_info

  with ToolchainProfiler.profile_block('js_optimizer.split'):
    total_size = len(js)
    funcs = split_funcs(js)
    js = None

  with ToolchainProfiler.profile_block('js_optimizer.split_to_chunks'):
    # if we are making source maps, we want our debug numbering to start from the
    # top of the file, so avoid breaking the JS into chunks

    intended_num_chunks = round(shared.get_num_cores() * NUM_CHUNKS_PER_CORE)
    chunk_size = min(MAX_CHUNK_SIZE, max(MIN_CHUNK_SIZE, total_size / intended_num_chunks))
    chunks = chunkify(funcs, chunk_size)

    chunks = [chunk for chunk in chunks if chunk]
    if DEBUG:
      lengths = [len(c) for c in chunks]
      if not lengths:
        lengths = [0]
      print('chunkification: num funcs:', len(funcs), 'actual num chunks:', len(chunks), 'chunk size range:', max(lengths), '-', min(lengths), file=sys.stderr)
    funcs = None

    serialized_extra_info = ''
    if minify_globals:
      assert not extra_info
      serialized_extra_info += '// EXTRA_INFO:' + json.dumps(minify_info)
    elif extra_info:
      serialized_extra_info += '// EXTRA_INFO:' + json.dumps(extra_info)
    with ToolchainProfiler.profile_block('js_optimizer.write_chunks'):
      def write_chunk(chunk, i):
        temp_file = temp_files.get('.jsfunc_%d.js' % i).name
        utils.write_file(temp_file, chunk + serialized_extra_info)
        return temp_file
      filenames = [write_chunk(chunk, i) for i, chunk in enumerate(chunks)]

  with ToolchainProfiler.profile_block('run_optimizer'):
    commands = [get_acorn_cmd() + [f] + passes for f in filenames]
    filenames = shared.run_multiple_processes(commands, route_stdout_to_temp_files_suffix='js_opt.jo.js')

  with ToolchainProfiler.profile_block('split_closure_cleanup'):
    if closure or cleanup:
      # run on the shell code, everything but what we acorn-optimize
      start_asm = '// EMSCRIPTEN_START_ASM\n'
      end_asm = '// EMSCRIPTEN_END_ASM\n'
      cl_sep = 'wakaUnknownBefore(); var asm=wakaUnknownAfter(wakaGlobal,wakaEnv,wakaBuffer)\n'

      with temp_files.get_file('.cl.js') as cle:
        pre_1, pre_2 = pre.split(start_asm)
        post_1, post_2 = post.split(end_asm)
        with open(cle, 'w') as f:
          f.write(pre_1)
          f.write(cl_sep)
          f.write(post_2)
        cld = cle
        if closure:
          if DEBUG:
            print('running closure on shell code', file=sys.stderr)
          cld = building.closure_compiler(cld, pretty='--minify-whitespace' not in passes)
          temp_files.note(cld)
        elif cleanup:
          if DEBUG:
            print('running cleanup on shell code', file=sys.stderr)
          acorn_passes = ['JSDCE']
          if '--minify-whitespace' in passes:
            acorn_passes.append('--minify-whitespace')
          cld = building.acorn_optimizer(cld, acorn_passes)
          temp_files.note(cld)
        coutput = utils.read_file(cld)

      coutput = coutput.replace('wakaUnknownBefore();', start_asm)
      after = 'wakaUnknownAfter'
      start = coutput.find(after)
      end = coutput.find(')', start)
      # If the closure comment to suppress useless code is present, we need to look one
      # brace past it, as the first is in there. Otherwise, the first brace is the
      # start of the function body (what we want).
      USELESS_CODE_COMMENT = '/** @suppress {uselessCode} */ '
      USELESS_CODE_COMMENT_BODY = 'uselessCode'
      brace = pre_2.find('{') + 1
      has_useless_code_comment = False
      if pre_2[brace:brace + len(USELESS_CODE_COMMENT_BODY)] == USELESS_CODE_COMMENT_BODY:
        brace = pre_2.find('{', brace) + 1
        has_useless_code_comment = True
      pre = coutput[:start] + '(' + (USELESS_CODE_COMMENT if has_useless_code_comment else '') + 'function(global,env,buffer) {\n' + pre_2[brace:]
      post = post_1 + end_asm + coutput[end + 1:]

  filename += '.jo.js'
  temp_files.note(filename)

  with open(filename, 'w') as f:
    with ToolchainProfiler.profile_block('write_pre'):
      f.write(pre)
      pre = None

    with ToolchainProfiler.profile_block('sort_or_concat'):
      # sort functions by size, to make diffing easier and to improve aot times
      funcses = []
      for out_file in filenames:
        funcses.append(split_funcs(utils.read_file(out_file)))
      funcs = [item for sublist in funcses for item in sublist]
      funcses = None
      if not os.environ.get('EMCC_NO_OPT_SORT'):
        funcs.sort(key=lambda x: (len(x[1]), x[0]), reverse=True)

      for func in funcs:
        f.write(func[1])
      funcs = None

    with ToolchainProfiler.profile_block('write_post'):
      f.write('\n')
      f.write(post)
      f.write('\n')

  return filename


def main():
  last = sys.argv[-1]
  if '{' in last:
    extra_info = json.loads(last)
    sys.argv = sys.argv[:-1]
  else:
    extra_info = None
  out = run_on_file(sys.argv[1], sys.argv[2:], extra_info=extra_info)
  shutil.copyfile(out, sys.argv[1] + '.jsopt.js')
  return 0


if __name__ == '__main__':
  sys.exit(main())
