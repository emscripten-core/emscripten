# coding=utf-8
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# noqa: E241

from functools import wraps
import glob
import gzip
import importlib
import itertools
import json
import os
import re
import select
import shlex
import shutil
import subprocess
import sys
import tarfile
import time
from pathlib import Path
from subprocess import PIPE, STDOUT

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: test/runner other')

from tools.building import get_building_env
from tools.shared import config
from tools.shared import EMCC, EMXX, EMAR, EMRANLIB, FILE_PACKAGER, LLVM_NM
from tools.shared import CLANG_CC, CLANG_CXX, LLVM_AR, LLVM_DWARFDUMP, LLVM_DWP, EMCMAKE, EMCONFIGURE, WASM_LD
from common import RunnerCore, path_from_root, is_slow_test, ensure_dir, disabled, make_executable
from common import env_modify, no_mac, no_windows, only_windows, requires_native_clang, with_env_modify
from common import create_file, parameterized, NON_ZERO, node_pthreads, TEST_ROOT, test_file
from common import compiler_for, EMBUILDER, requires_v8, requires_node, requires_wasm64, requires_node_canary
from common import requires_wasm_exnref, crossplatform, with_all_eh_sjlj, with_all_sjlj
from common import also_with_standalone_wasm, also_with_wasm2js, also_with_noderawfs, also_with_wasmfs
from common import also_with_minimal_runtime, also_with_wasm_bigint, also_with_wasm64, flaky
from common import EMTEST_BUILD_VERBOSE, PYTHON, WEBIDL_BINDER
from common import requires_network, parameterize
from tools import shared, building, utils, response_file, cache
from tools.utils import read_file, write_file, delete_file, read_binary, MACOS, WINDOWS
import common
import jsrun
import clang_native
from tools import line_endings
from tools import webassembly
from tools.settings import settings

scons_path = shutil.which('scons')
emmake = shared.bat_suffix(path_from_root('emmake'))
emconfig = shared.bat_suffix(path_from_root('em-config'))
emsize = shared.bat_suffix(path_from_root('emsize'))
emprofile = shared.bat_suffix(path_from_root('emprofile'))
emstrip = shared.bat_suffix(path_from_root('emstrip'))
emsymbolizer = shared.bat_suffix(path_from_root('emsymbolizer'))
wasm_opt = Path(building.get_binaryen_bin(), 'wasm-opt')


def is_bitcode(filename):
  try:
    # look for magic signature
    b = open(filename, 'rb').read(4)
    if b[:2] == b'BC':
      return True
    # on macOS, there is a 20-byte prefix which starts with little endian
    # encoding of 0x0B17C0DE
    elif b == b'\xDE\xC0\x17\x0B':
      b = bytearray(open(filename, 'rb').read(22))
      return b[20:] == b'BC'
  except IndexError:
    # not enough characters in the input
    # note that logging will be done on the caller function
    pass
  return False


def uses_canonical_tmp(func):
  """Decorator that signals the use of the canonical temp by a test method.

  This decorator takes care of cleaning the directory after the
  test to satisfy the leak detector.
  """
  @wraps(func)
  def decorated(self, *args, **kwargs):
    # Before running the test completely remove the canonical_tmp
    if os.path.exists(self.canonical_temp_dir):
      shutil.rmtree(self.canonical_temp_dir)
    try:
      func(self, *args, **kwargs)
    finally:
      # Make sure the test isn't lying about the fact that it uses
      # canonical_tmp
      self.assertTrue(os.path.exists(self.canonical_temp_dir))
      # Remove the temp dir in a try-finally, as otherwise if the
      # test fails we would not clean it up, and if leak detection
      # is set we will show that error instead of the actual one.
      shutil.rmtree(self.canonical_temp_dir)

  return decorated


def with_both_compilers(f):
  assert callable(f)

  parameterize(f, {'': (EMCC,),
                   'emxx': (EMXX,)})
  return f


def wasmfs_all_backends(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, backend, *args, **kwargs):
    self.set_setting('WASMFS')
    self.emcc_args.append('-DWASMFS')
    self.emcc_args.append(f'-D{backend}')
    f(self, *args, **kwargs)

  parameterize(metafunc, {'': ('WASMFS_MEMORY_BACKEND',),
                          'node': ('WASMFS_NODE_BACKEND',)})
  return metafunc


def also_with_wasmfs_all_backends(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, backend, *args, **kwargs):
    if backend:
      self.set_setting('WASMFS')
      self.emcc_args.append('-DWASMFS')
      self.emcc_args.append(f'-D{backend}')
    f(self, *args, **kwargs)

  parameterize(metafunc, {'': (None,),
                          'wasmfs': ('WASMFS_MEMORY_BACKEND',),
                          'wasmfs_node': ('WASMFS_NODE_BACKEND',)})
  return metafunc


def requires_tool(tool):
  assert not callable(tool)

  def decorate(func):
    assert callable(func)

    @wraps(func)
    def decorated(self, *args, **kwargs):
      if not shutil.which(tool):
        if f'EMTEST_SKIP_{tool.upper()}' in os.environ:
          self.skipTest(f'test requires ccache and EMTEST_SKIP_{tool.upper()} is set')
        else:
          self.fail(f'{tool} required to run this test.  Use EMTEST_SKIP_{tool.upper()} to skip')
      return func(self, *args, **kwargs)

    return decorated

  return decorate


def requires_ninja(func):
  assert callable(func)
  return requires_tool('ninja')(func)


def requires_scons(func):
  assert callable(func)
  return requires_tool('scons')(func)


def requires_pkg_config(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    if not shutil.which('pkg-config'):
      if 'EMTEST_SKIP_PKG_CONFIG' in os.environ:
        self.skipTest('test requires pkg-config and EMTEST_SKIP_PKG_CONFIG is set')
      else:
        self.fail('pkg-config is required to run this test')
    return func(self, *args, **kwargs)

  return decorated


def requires_jspi(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_jspi()
    return func(self, *args, **kwargs)

  return decorated


def llvm_nm(file):
  output = shared.run_process([LLVM_NM, file], stdout=PIPE).stdout

  symbols = {
    'defs': set(),
    'undefs': set(),
    'commons': set(),
  }

  for line in output.splitlines():
    # Skip address, which is always fixed-length 8 chars (plus 2
    # leading chars `: ` and one trailing space)
    status = line[9]
    symbol = line[11:]

    if status == 'U':
      symbols['undefs'].add(symbol)
    elif status == 'C':
      symbols['commons'].add(symbol)
    elif status == status.upper():
      symbols['defs'].add(symbol)

  return symbols


def get_file_gzipped_size(f):
  f_gz = f + '.gz'
  with gzip.open(f_gz, 'wb') as gzf:
    gzf.write(read_binary(f))
  size = os.path.getsize(f_gz)
  delete_file(f_gz)
  return size


def deminify_syms(names, minification_map):
  deminify_map = {}
  for line in read_file(minification_map).splitlines():
    minified_name, name = line.split(':')
    deminify_map[minified_name] = name
  # Include both the original name and the deminified name so that
  # changes to minification are also visible.
  return [f'{name} ({deminify_map[name]})' for name in names]


class other(RunnerCore):
  def assertIsObjectFile(self, filename):
    self.assertTrue(building.is_wasm(filename))

  def assertIsWasmDylib(self, filename):
    self.assertTrue(building.is_wasm_dylib(filename))

  def do_other_test(self, testname, emcc_args=None, **kwargs):
    return self.do_run_in_out_file_test(test_file('other', testname), emcc_args=emcc_args, **kwargs)

  def run_on_pty(self, cmd):
    master, slave = os.openpty()
    output = []
    print(cmd)

    try:
      with env_modify({'TERM': 'xterm-color'}):
        proc = subprocess.Popen(cmd, stdout=slave, stderr=slave)
        while proc.poll() is None:
          r, w, x = select.select([master], [], [], 1)
          if r:
            output.append(os.read(master, 1024))
        return (proc.returncode, b''.join(output))
    finally:
      os.close(master)
      os.close(slave)

  def parse_wasm(self, filename):
    wat = self.get_wasm_text(filename)
    imports = []
    exports = []
    funcs = []
    for line in wat.splitlines():
      line = line.strip()
      if line.startswith('(import '):
        line = line.strip('()')
        parts = line.split()
        module = parts[1].strip('"')
        name = parts[2].strip('"')
        imports.append('%s.%s' % (module, name))
      if line.startswith('(export '):
        line = line.strip('()')
        name = line.split()[1].strip('"')
        exports.append(name)
      if line.startswith('(func '):
        line = line.strip('()')
        name = line.split()[1].strip('"')
        funcs.append(name)
    return imports, exports, funcs

  # Test that running `emcc -v` always works even in the presence of `EMCC_CFLAGS`.
  # This needs to work because many tools run `emcc -v` internally and it should
  # always work even if the user has `EMCC_CFLAGS` set.
  @with_env_modify({'EMCC_CFLAGS': '-should -be -ignored'})
  @with_both_compilers
  @crossplatform
  def test_emcc_v(self, compiler):
    # -v, without input files
    proc = self.run_process([compiler, '-v'], stdout=PIPE, stderr=PIPE)
    self.assertEqual(proc.stdout, '')
    # assert that the emcc message comes first.  We had a bug where the sub-process output
    # from clang would be flushed to stderr first.
    self.assertContained('emcc (Emscripten gcc/clang-like replacement', proc.stderr)
    self.assertTrue(proc.stderr.startswith('emcc (Emscripten gcc/clang-like replacement'))
    self.assertContained('clang version ', proc.stderr)
    self.assertContained('GNU', proc.stderr)
    self.assertContained('Target: wasm32-unknown-emscripten', proc.stderr)
    self.assertNotContained('this is dangerous', proc.stderr)

  def test_log_subcommands(self):
    # `-v` when combined with other arguments will trace the subcommands
    # that get run
    proc = self.run_process([EMCC, '-v', test_file('hello_world.c')], stdout=PIPE, stderr=PIPE)
    self.assertContained(CLANG_CC, proc.stderr)
    self.assertContained(WASM_LD, proc.stderr)
    self.assertExists('a.out.js')

  def test_skip_subcommands(self):
    # The -### flag is like `-v` but it doesn't actaully execute the sub-commands
    proc = self.run_process([EMCC, '-###', test_file('hello_world.c')], stdout=PIPE, stderr=PIPE)
    self.assertContained(CLANG_CC, proc.stderr)
    self.assertContained(WASM_LD, proc.stderr)
    self.assertNotExists('a.out.js')

  def test_emcc_check(self):
    proc = self.run_process([EMCC, '--check'], stdout=PIPE, stderr=PIPE)
    self.assertEqual(proc.stdout, '')
    self.assertContained('emcc (Emscripten gcc/clang-like replacement', proc.stderr)
    self.assertContained('Running sanity checks', proc.stderr)
    proc = self.run_process([EMCC, '--check'], stdout=PIPE, stderr=PIPE)
    self.assertContained('Running sanity checks', proc.stderr)

  @with_both_compilers
  def test_emcc_generate_config(self, compiler):
    config_path = './emscripten_config'
    with env_modify({'EM_CONFIG': config_path}):
      self.assertNotExists(config_path)
      self.run_process([compiler, '--generate-config'])
      self.assertExists(config_path)
      config_contents = read_file(config_path)
      self.assertContained('LLVM_ROOT', config_contents)
      os.remove(config_path)

  @parameterized({
    '': ([],),
    'node': (['-sENVIRONMENT=node'],),
  })
  def test_emcc_output_mjs(self, args):
    self.run_process([EMCC, '-o', 'hello_world.mjs',
                      '--extern-post-js', test_file('modularize_post_js.js'),
                      test_file('hello_world.c')] + args)
    src = read_file('hello_world.mjs')
    self.assertContained('export default Module;', src)
    self.assertContained('hello, world!', self.run_js('hello_world.mjs'))

  @parameterized({
    '': ([],),
    'node': (['-sENVIRONMENT=node'],),
  })
  @node_pthreads
  def test_emcc_output_worker_mjs(self, args):
    os.mkdir('subdir')
    self.run_process([EMCC, '-o', 'subdir/hello_world.mjs',
                      '-sEXIT_RUNTIME', '-sPROXY_TO_PTHREAD', '-pthread', '-O1',
                      '--extern-post-js', test_file('modularize_post_js.js'),
                      test_file('hello_world.c')] + args)
    src = read_file('subdir/hello_world.mjs')
    self.assertContained("new URL('hello_world.wasm', import.meta.url)", src)
    self.assertContained("new Worker(new URL('hello_world.mjs', import.meta.url), workerOptions)", src)
    self.assertContained('export default Module;', src)
    self.assertContained('hello, world!', self.run_js('subdir/hello_world.mjs'))

  @node_pthreads
  def test_emcc_output_worker_mjs_single_file(self):
    self.run_process([EMCC, '-o', 'hello_world.mjs', '-pthread',
                      '--extern-post-js', test_file('modularize_post_js.js'),
                      test_file('hello_world.c'), '-sSINGLE_FILE'])
    src = read_file('hello_world.mjs')
    self.assertNotContained("new URL('data:", src)
    self.assertContained("new Worker(new URL('hello_world.mjs', import.meta.url), workerOptions)", src)
    self.assertContained('hello, world!', self.run_js('hello_world.mjs'))

  def test_emcc_output_mjs_closure(self):
    self.run_process([EMCC, '-o', 'hello_world.mjs',
                      '--extern-post-js', test_file('modularize_post_js.js'),
                      test_file('hello_world.c'), '--closure=1'])
    src = read_file('hello_world.mjs')
    self.assertContained('new URL("hello_world.wasm", import.meta.url)', src)
    self.assertContained('hello, world!', self.run_js('hello_world.mjs'))

  def test_emcc_output_mjs_web_no_import_meta(self):
    # Ensure we don't emit import.meta.url at all for:
    # ENVIRONMENT=web + EXPORT_ES6 + USE_ES6_IMPORT_META=0
    self.run_process([EMCC, '-o', 'hello_world.mjs',
                      test_file('hello_world.c'),
                      '-sENVIRONMENT=web', '-sUSE_ES6_IMPORT_META=0'])
    src = read_file('hello_world.mjs')
    self.assertNotContained('import.meta.url', src)
    self.assertContained('export default Module;', src)

  def test_export_es6_implies_modularize(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORT_ES6'])
    src = read_file('a.out.js')
    self.assertContained('export default Module;', src)

  def test_export_es6_requires_modularize(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sEXPORT_ES6', '-sMODULARIZE=0'])
    self.assertContained('EXPORT_ES6 requires MODULARIZE to be set', err)

  def test_export_es6_node_requires_import_meta(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'),
                            '-sENVIRONMENT=node', '-sEXPORT_ES6', '-sUSE_ES6_IMPORT_META=0'])
    self.assertContained('EXPORT_ES6 and ENVIRONMENT=*node* requires USE_ES6_IMPORT_META to be set', err)

  def test_export_es6_allows_export_in_post_js(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-O3', '-sEXPORT_ES6', '--post-js', test_file('export_module.js')])
    src = read_file('a.out.js')
    self.assertContained('export{doNothing};', src)

  @parameterized({
    '': (False,),
    'package_json': (True,),
  })
  @parameterized({
    '': ([],),
    # load a worker before startup to check ES6 modules there as well
    'pthreads': (['-pthread', '-sPTHREAD_POOL_SIZE=1'],),
  })
  def test_export_es6(self, package_json, args):
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORT_ES6',
                      '-o', 'hello.mjs'] + args)
    # In ES6 mode we use MODULARIZE, so we must instantiate an instance of the
    # module to run it.
    create_file('runner.mjs', '''
      import Hello from "./hello.mjs";
      Hello();
    ''')

    self.assertContained('hello, world!', self.run_js('runner.mjs'))

  def test_emcc_out_file(self):
    # Verify that "-ofile" works in addition to "-o" "file"
    self.run_process([EMCC, '-c', '-ofoo.o', test_file('hello_world.c')])
    self.assertExists('foo.o')
    self.run_process([EMCC, '-ofoo.js', 'foo.o'])
    self.assertExists('foo.js')

  @parameterized({
    'c': [EMCC, '.c'],
    'cxx': [EMXX, '.cpp'],
  })
  def test_emcc_basics(self, compiler, suffix):
    # emcc src.cpp ==> writes a.out.js and a.out.wasm
    self.run_process([compiler, test_file('hello_world' + suffix)])
    self.assertExists('a.out.js')
    self.assertExists('a.out.wasm')
    self.assertContained('hello, world!', self.run_js('a.out.js'))

    # --version
    output = self.run_process([compiler, '--version'], stdout=PIPE, stderr=PIPE)
    output = output.stdout.replace('\r', '')
    self.assertContained('emcc (Emscripten gcc/clang-like replacement', output)
    self.assertContained('''Copyright (C) 2014 the Emscripten authors (see AUTHORS.txt)
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
''', output)

    # --help
    output = self.run_process([compiler, '--help'], stdout=PIPE, stderr=PIPE)
    self.assertContained('Display this information', output.stdout)
    self.assertContained('Most clang options will work', output.stdout)

    # -dumpversion
    output = self.run_process([compiler, '-dumpversion'], stdout=PIPE, stderr=PIPE)
    self.assertEqual(utils.EMSCRIPTEN_VERSION, output.stdout.strip())

    # properly report source code errors, and stop there
    self.clear()
    stderr = self.expect_fail([compiler, test_file('hello_world_error' + suffix)])
    self.assertNotContained('IOError', stderr) # no python stack
    self.assertNotContained('Traceback', stderr) # no python stack
    self.assertContained('error: invalid preprocessing directive', stderr)
    self.assertContained(["error: use of undeclared identifier 'cheez", "error: unknown type name 'cheez'"], stderr)
    self.assertContained('errors generated.', stderr.splitlines()[-2])

  def test_dumpmachine(self):
    output = self.run_process([EMCC, '-dumpmachine'], stdout=PIPE, stderr=PIPE).stdout
    self.assertContained('wasm32-unknown-emscripten', output)

    # Test the -print-target-triple llvm alias for -dumpmachine
    output = self.run_process([EMCC, '-print-target-triple'], stdout=PIPE, stderr=PIPE).stdout
    self.assertContained('wasm32-unknown-emscripten', output)

    output = self.run_process([EMCC, '--print-target-triple'], stdout=PIPE, stderr=PIPE).stdout
    self.assertContained('wasm32-unknown-emscripten', output)

    # Test that -sMEMORY64 triggers the wasm64 triple
    output = self.run_process([EMCC, '-sMEMORY64', '-dumpmachine'], stdout=PIPE, stderr=PIPE).stdout
    self.assertContained('wasm64-unknown-emscripten', output)

  @parameterized({
    'c': [EMCC, '.c'],
    'cxx': [EMXX, '.cpp']
  })
  def test_emcc_2(self, compiler, suffix):
    # emcc src.cpp -c    and   emcc -c src.cpp -o src.[o|foo|so] ==> should always give an object file
    for args in ([], ['-o', 'src.o'], ['-o', 'src.foo'], ['-o', 'src.so']):
      print('args:', args)
      target = args[1] if len(args) == 2 else 'hello_world.o'
      self.clear()
      self.run_process([compiler, '-c', test_file('hello_world' + suffix)] + args)
      self.assertIsObjectFile(target)
      syms = llvm_nm(target)
      self.assertIn('main', syms['defs'])
      # we also expect to have the '__original_main' wrapper and __main_void alias.
      # TODO(sbc): Should be 4 once https://reviews.llvm.org/D75277 lands
      self.assertIn(len(syms['defs']), (2, 3))
      self.run_process([compiler, target, '-o', target + '.js'])
      self.assertContained('hello, world!', self.run_js(target + '.js'))

  def test_bc_output_warning(self):
    err = self.run_process([EMCC, '-c', test_file('hello_world.c'), '-o', 'out.bc'], stderr=PIPE).stderr
    self.assertContained('emcc: warning: .bc output file suffix used without -flto or -emit-llvm', err)

  def test_bc_as_archive(self):
    self.run_process([EMCC, '-c', test_file('hello_world.c'), '-flto', '-o', 'out.a'])
    self.run_process([EMCC, 'out.a'])

  @parameterized({
    'c': [EMCC, '.c'],
    'cxx': [EMXX, '.cpp']
  })
  def test_emcc_3(self, compiler, suffix):
    # handle singleton archives
    self.run_process([compiler, '-c', test_file('hello_world' + suffix), '-o', 'a.o'])
    self.run_process([LLVM_AR, 'r', 'a.a', 'a.o'], stdout=PIPE, stderr=PIPE)
    self.run_process([compiler, 'a.a'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

    # emcc [..] -o [path] ==> should work with absolute paths
    for path in (os.path.abspath(Path('../file1.js')), Path('b_dir/file2.js')):
      print(path)
      os.chdir(self.get_dir())
      self.clear()
      print(os.listdir(os.getcwd()))
      ensure_dir('a_dir/b_dir')
      os.chdir('a_dir')
      # use single file so we don't have more files to clean up
      self.run_process([compiler, test_file('hello_world' + suffix), '-o', path, '-sSINGLE_FILE'])
      last = os.getcwd()
      os.chdir(os.path.dirname(path))
      self.assertContained('hello, world!', self.run_js(os.path.basename(path)))
      os.chdir(last)
      delete_file(path)

  @is_slow_test
  @with_both_compilers
  def test_emcc_4(self, compiler):
    # Optimization: emcc src.cpp -o something.js [-Ox]. -O0 is the same as not specifying any optimization setting
    # link_param are used after compiling first
    for params, opt_level, link_params, closure, has_malloc in [
      (['-o', 'something.js'],                          0, None, 0, 1),
      (['-o', 'something.js', '-O0', '-g'],             0, None, 0, 0),
      (['-o', 'something.js', '-O1'],                   1, None, 0, 0),
      (['-o', 'something.js', '-O1', '-g'],             1, None, 0, 0), # no closure since debug
      (['-o', 'something.js', '-O2'],                   2, None, 0, 1),
      (['-o', 'something.js', '-O2', '-g'],             2, None, 0, 0),
      (['-o', 'something.js', '-Os'],                   2, None, 0, 1),
      (['-o', 'something.js', '-O3'],                   3, None, 0, 1),
      # and, test compiling first
      (['-c', '-o', 'something.o'],        0, [],      0, 0),
      (['-c', '-o', 'something.o', '-O0'], 0, [],      0, 0),
      (['-c', '-o', 'something.o', '-O1'], 1, ['-O1'], 0, 0),
      (['-c', '-o', 'something.o', '-O2'], 2, ['-O2'], 0, 0),
      (['-c', '-o', 'something.o', '-O3'], 3, ['-O3'], 0, 0),
      (['-O1', '-c', '-o', 'something.o'], 1, [],      0, 0),
      # non-wasm
      (['-sWASM=0', '-o', 'something.js'],                          0, None, 0, 1),
      (['-sWASM=0', '-o', 'something.js', '-O0', '-g'],             0, None, 0, 0),
      (['-sWASM=0', '-o', 'something.js', '-O1'],                   1, None, 0, 0),
      (['-sWASM=0', '-o', 'something.js', '-O1', '-g'],             1, None, 0, 0), # no closure since debug
      (['-sWASM=0', '-o', 'something.js', '-O2'],                   2, None, 0, 1),
      (['-sWASM=0', '-o', 'something.js', '-O2', '-g'],             2, None, 0, 0),
      (['-sWASM=0', '-o', 'something.js', '-Os'],                   2, None, 0, 1),
      (['-sWASM=0', '-o', 'something.js', '-O3'],                   3, None, 0, 1),
      # and, test compiling to bitcode first
      (['-flto', '-c', '-o', 'something.o'],        0, [],      0, 0),
      (['-flto', '-c', '-o', 'something.o', '-O0'], 0, [],      0, 0),
      (['-flto', '-c', '-o', 'something.o', '-O1'], 1, ['-O1'], 0, 0),
      (['-flto', '-c', '-o', 'something.o', '-O2'], 2, ['-O2'], 0, 0),
      (['-flto', '-c', '-o', 'something.o', '-O3'], 3, ['-O3'], 0, 0),
      (['-flto', '-O1', '-c', '-o', 'something.o'], 1, [],      0, 0),
    ]:
      print(params, opt_level, link_params, closure, has_malloc)
      self.clear()
      keep_debug = '-g' in params
      if has_malloc:
        filename = test_file('hello_world_loop_malloc.c')
      else:
        filename = test_file('hello_world_loop.c')
      args = [compiler, filename] + params
      print('..', args)
      output = self.run_process(args, stdout=PIPE, stderr=PIPE)
      assert len(output.stdout) == 0, output.stdout
      if link_params is not None:
        self.assertExists('something.o', output.stderr)
        obj_args = [compiler, 'something.o', '-o', 'something.js'] + link_params
        print('....', obj_args)
        output = self.run_process(obj_args, stdout=PIPE, stderr=PIPE)
      self.assertExists('something.js', output.stderr)
      self.assertContained('hello, world!', self.run_js('something.js'))

      # Verify optimization level etc. in the generated code
      # XXX these are quite sensitive, and will need updating when code generation changes
      generated = read_file('something.js')
      main = self.get_func(generated, '_main') if 'function _main' in generated else generated
      assert 'new Uint16Array' in generated and 'new Uint32Array' in generated, 'typed arrays 2 should be used by default'
      assert 'SAFE_HEAP_LOAD' not in generated, 'safe heap should not be used by default'
      assert 'SAFE_HEAP_STORE' not in generated, 'safe heap should not be used by default'
      assert ': while(' not in main, 'when relooping we also js-optimize, so there should be no labelled whiles'
      if closure:
        if opt_level == 0:
          assert '._main =' in generated, 'closure compiler should have been run'
        elif opt_level >= 1:
          assert '._main=' in generated, 'closure compiler should have been run (and output should be minified)'
      else:
        # closure has not been run, we can do some additional checks. TODO: figure out how to do these even with closure
        assert '._main = ' not in generated, 'closure compiler should not have been run'
        if keep_debug:
          self.assertContainedIf("assert(!Module['STACK_SIZE']", generated, opt_level == 0)
        if 'WASM=0' in params:
          looks_unminified = ' = {}' in generated and ' = []' in generated
          looks_minified = '={}' in generated and '=[]' and ';var' in generated
          assert not (looks_minified and looks_unminified)
          if opt_level == 0 or '-g' in params:
            assert looks_unminified
          elif opt_level >= 2:
            assert looks_minified

  def test_multiple_sources(self):
    # Compiling two sources at a time should work.
    cmd = [EMCC, '-c', test_file('twopart_main.cpp'), test_file('twopart_side.c')]
    self.run_process(cmd)

    # Object files should be generated by default in the current working
    # directory, and not alongside the sources.
    self.assertExists('twopart_main.o')
    self.assertExists('twopart_side.o')
    self.assertNotExists(test_file('twopart_main.o'))
    self.assertNotExists(test_file('twopart_side.o'))

    # But it is an error if '-o' is also specified.
    self.clear()
    err = self.expect_fail(cmd + ['-o', 'out.o'])

    self.assertContained('clang: error: cannot specify -o when generating multiple output files', err)
    self.assertNotExists('twopart_main.o')
    self.assertNotExists('twopart_side.o')
    self.assertNotExists(test_file('twopart_main.o'))
    self.assertNotExists(test_file('twopart_side.o'))

  def test_tsearch(self):
    self.do_other_test('test_tsearch.c')

  @crossplatform
  def test_libc_progname(self):
    self.do_other_test('test_libc_progname.c')

  def test_combining_object_files(self):
    # Compiling two files with -c will generate separate object files
    self.run_process([EMCC, test_file('twopart_main.cpp'), test_file('twopart_side.c'), '-c'])
    self.assertExists('twopart_main.o')
    self.assertExists('twopart_side.o')

    # Linking with just one of them is expected to fail
    err = self.expect_fail([EMCC, 'twopart_main.o'])
    self.assertContained('undefined symbol: theFunc', err)

    # Linking with both should work
    self.run_process([EMCC, 'twopart_main.o', 'twopart_side.o'])
    self.assertContained('side got: hello from main, over', self.run_js('a.out.js'))

    # Combining object files into another object should also work, using the `-r` flag
    err = self.run_process([EMCC, '-r', 'twopart_main.o', 'twopart_side.o', '-o', 'combined.o'], stderr=PIPE).stderr
    self.assertNotContained('warning:', err)
    # Warn about legecy support for outputing object file without `-r`, `-c` or `-shared`
    err = self.run_process([EMCC, 'twopart_main.o', 'twopart_side.o', '-o', 'combined2.o'], stderr=PIPE).stderr
    self.assertContained('warning: object file output extension (.o) used for non-object output', err)

    # Should be two symbols (and in the wasm backend, also __original_main)
    syms = llvm_nm('combined.o')
    self.assertIn('main', syms['defs'])
    # TODO(sbc): Should be 4 once https://reviews.llvm.org/D75277 lands
    self.assertIn(len(syms['defs']), (4, 3))

    self.run_process([EMCC, 'combined.o', '-o', 'combined.o.js'])
    self.assertContained('side got: hello from main, over', self.run_js('combined.o.js'))

  def test_combining_object_files_from_archive(self):
    # Compiling two files with -c will generate separate object files
    self.run_process([EMCC, test_file('twopart_main.cpp'), test_file('twopart_side.c'), '-c'])
    self.assertExists('twopart_main.o')
    self.assertExists('twopart_side.o')

    # Combining object files into a library archive should work
    self.run_process([EMAR, 'crs', 'combined.a', 'twopart_main.o', 'twopart_side.o'])
    self.assertExists('combined.a')

    # Combining library archive into an object should yield a valid object, using the `-r` flag
    self.run_process([EMXX, '-r', '-o', 'combined.o', '-Wl,--whole-archive', 'combined.a'])
    self.assertIsObjectFile('combined.o')

    # Should be two symbols (and in the wasm backend, also __original_main)
    syms = llvm_nm('combined.o')
    self.assertIn('main', syms['defs'])
    # TODO(sbc): Should be 3 once https://reviews.llvm.org/D75277 lands
    self.assertIn(len(syms['defs']), (3, 4))

    self.run_process([EMXX, 'combined.o', '-o', 'combined.o.js'])
    self.assertContained('side got: hello from main, over', self.run_js('combined.o.js'))

  def test_js_transform(self):
    create_file('t.py', '''
import sys
f = open(sys.argv[1], 'a')
f.write('transformed!')
f.close()
''')

    err = self.run_process([EMCC, test_file('hello_world.c'), '-gsource-map', '--js-transform', '%s t.py' % (PYTHON)], stderr=PIPE).stderr
    self.assertContained('disabling source maps because a js transform is being done', err)
    self.assertIn('transformed!', read_file('a.out.js'))

  @parameterized({
    '': [[]],
    'O1': [['-O1']],
    'O2': [['-O2']],
    'O3': [['-O3']],
  })
  def test_emcc_asm_v_wasm(self, opts):
    for mode in ([], ['-sWASM=0']):
      self.clear()
      wasm = '=0' not in str(mode)
      print('  mode', mode, 'wasm?', wasm)
      self.run_process([EMCC, test_file('hello_world.c'), '-sENVIRONMENT=node,shell'] + opts + mode)
      self.assertExists('a.out.js')
      if wasm:
        self.assertExists('a.out.wasm')
      for engine in config.JS_ENGINES:
        print('    engine', engine)
        out = self.run_js('a.out.js', engine=engine)
        self.assertContained('hello, world!', out)

  @crossplatform
  def test_emcc_cflags(self):
    output = self.run_process([EMCC, '--cflags'], stdout=PIPE)
    flags = output.stdout.strip()
    self.assertContained('-target wasm32-unknown-emscripten', flags)
    self.assertContained('--sysroot=', flags)
    output = self.run_process([EMXX, '--cflags'], stdout=PIPE)
    flags = output.stdout.strip()
    self.assertContained('-target wasm32-unknown-emscripten', flags)
    self.assertContained('--sysroot=', flags)
    # check they work
    cmd = [CLANG_CC, test_file('hello_world.c')] + shlex.split(flags.replace('\\', '\\\\')) + ['-c', '-o', 'out.o']
    self.run_process(cmd)
    self.run_process([EMCC, 'out.o'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  @crossplatform
  @parameterized({
    '': [[]],
    'lto': [['-flto']],
    'wasm64': [['-sMEMORY64']],
  })
  def test_print_search_dirs(self, args):
    output = self.run_process([EMCC, '-print-search-dirs'] + args, stdout=PIPE).stdout
    output2 = self.run_process([EMCC, '-print-search-dirs'] + args, stdout=PIPE).stdout
    self.assertEqual(output, output2)
    self.assertContained('programs: =', output)
    self.assertContained('libraries: =', output)
    libpath = output.split('libraries: =', 1)[1].strip()
    libpath = libpath.split(os.pathsep)
    libpath = [Path(p) for p in libpath]
    settings.LTO = '-flto' in args
    settings.MEMORY64 = int('-sMEMORY64' in args)
    expected = cache.get_lib_dir(absolute=True)
    self.assertIn(expected, libpath)

  @crossplatform
  @parameterized({
    '': [[]],
    'lto': [['-flto']],
    'wasm64': [['-sMEMORY64']],
  })
  def test_print_libgcc_file_name(self, args):
    output = self.run_process([EMCC, '-print-libgcc-file-name'] + args, stdout=PIPE).stdout
    output2 = self.run_process([EMCC, '--print-libgcc-file-name'] + args, stdout=PIPE).stdout
    self.assertEqual(output, output2)
    settings.LTO = '-flto' in args
    settings.MEMORY64 = int('-sMEMORY64' in args)
    libdir = cache.get_lib_dir(absolute=True)
    expected = os.path.join(libdir, 'libcompiler_rt.a')
    self.assertEqual(output.strip(), expected)

  @crossplatform
  def test_print_resource_dir(self):
    output = self.run_process([EMCC, '-print-resource-dir'], stdout=PIPE).stdout
    print(output)
    lines = output.strip().splitlines()
    self.assertEqual(len(lines), 1)
    resource_dir = os.path.normcase(lines[0])
    llvm_root = os.path.normcase(os.path.dirname(config.LLVM_ROOT))
    self.assertContained(llvm_root, resource_dir)

  @crossplatform
  @parameterized({
    '': [[]],
    'lto': [['-flto']],
    'wasm64': [['-sMEMORY64', '-Wno-experimental']],
  })
  def test_print_file_name(self, args):
    # make sure the corresponding version of libc exists in the cache
    self.run_process([EMCC, test_file('hello_world.c'), '-O2'] + args)
    output = self.run_process([EMCC, '-print-file-name=libc.a'] + args, stdout=PIPE).stdout
    output2 = self.run_process([EMCC, '--print-file-name=libc.a'] + args, stdout=PIPE).stdout
    self.assertEqual(output, output2)
    filename = Path(output)
    settings.LTO = '-flto' in args
    settings.MEMORY64 = int('-sMEMORY64' in args)
    self.assertContained(cache.get_lib_name('libc.a'), str(filename))

  def test_emar_em_config_flag(self):
    # Test that the --em-config flag is accepted but not passed down do llvm-ar.
    # We expand this in case the EM_CONFIG is ~/.emscripten (default)
    conf = os.path.expanduser(config.EM_CONFIG)
    proc = self.run_process([EMAR, '--em-config', conf, '--version'], stdout=PIPE, stderr=PIPE)
    self.assertEqual(proc.stderr, "")
    self.assertContained('LLVM', proc.stdout)

  def test_em_config_missing_arg(self):
    out = self.expect_fail([EMCC, '--em-config'])
    self.assertContained('error: --em-config must be followed by a filename', out)

  def test_emsize(self):
    # test binaryen generated by running:
    #   emcc test/hello_world.c -Oz --closure 1 -o test/other/test_emsize.js
    expected = read_file(test_file('other/test_emsize.out'))
    cmd = [emsize, test_file('other/test_emsize.js')]
    for command in (cmd, cmd + ['--format=sysv']):
      output = self.run_process(command, stdout=PIPE).stdout
      self.assertContained(expected, output)

  def test_emstrip(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-g', '-o', 'hello.js'])
    output = self.run_process([common.LLVM_OBJDUMP, '-h', 'hello.wasm'], stdout=PIPE).stdout
    self.assertContained('.debug_info', output)
    self.run_process([emstrip, 'hello.wasm'])
    output = self.run_process([common.LLVM_OBJDUMP, '-h', 'hello.wasm'], stdout=PIPE).stdout
    self.assertNotContained('.debug_info', output)

  @is_slow_test
  @crossplatform
  @parameterized({
    # ('directory to the test', 'output filename', ['extra args to pass to
    # CMake']) Testing all combinations would be too much work and the test
    # would take 10 minutes+ to finish (CMake feature detection is slow), so
    # combine multiple features into one to try to cover as much as possible
    # while still keeping this test in sensible time limit.
    'js':          ('target_js',      'test_cmake.js',         ['-DCMAKE_BUILD_TYPE=Debug']),
    'html':        ('target_html',    'hello_world_gles.html', ['-DCMAKE_BUILD_TYPE=Release']),
    'library':     ('target_library', 'libtest_cmake.a',       ['-DCMAKE_BUILD_TYPE=MinSizeRel']),
    'static_cpp':  ('target_library', 'libtest_cmake.a',       ['-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DCPP_LIBRARY_TYPE=STATIC']),
    'stdproperty': ('stdproperty',    'helloworld.js',         []),
    'post_build':  ('post_build',     'hello.js',              []),
  })
  def test_cmake(self, test_dir, output_file, cmake_args):
    # Test all supported generators.
    if WINDOWS:
      generators = ['MinGW Makefiles', 'NMake Makefiles']
    else:
      generators = ['Unix Makefiles', 'Ninja', 'Eclipse CDT4 - Ninja']

    configurations = {
      'MinGW Makefiles'     : {'build'   : ['mingw32-make'] }, # noqa
      'NMake Makefiles'     : {'build'   : ['nmake', '/NOLOGO']}, # noqa
      'Unix Makefiles'      : {'build'   : ['make']}, # noqa
      'Ninja'               : {'build'   : ['ninja']}, # noqa
      'Eclipse CDT4 - Ninja': {'build'   : ['ninja']}, # noqa
    }
    for generator in generators:
      conf = configurations[generator]

      if not shutil.which(conf['build'][0]):
        # Use simple test if applicable
        print('Skipping %s test for CMake support; build tool found found: %s.' % (generator, conf['build'][0]))
        continue

      cmakelistsdir = test_file('cmake', test_dir)
      builddir = 'out_' + generator.replace(' ', '_').lower()
      os.mkdir(builddir)
      with utils.chdir(builddir):
        # Run Cmake
        cmd = [EMCMAKE, 'cmake'] + cmake_args + ['-G', generator, cmakelistsdir]

        env = os.environ.copy()
        # https://github.com/emscripten-core/emscripten/pull/5145: Check that CMake works even if EMCC_SKIP_SANITY_CHECK=1 is passed.
        if test_dir == 'target_html':
          env['EMCC_SKIP_SANITY_CHECK'] = '1'
        print(str(cmd))
        self.run_process(cmd, env=env, stdout=None if EMTEST_BUILD_VERBOSE >= 2 else PIPE, stderr=None if EMTEST_BUILD_VERBOSE >= 1 else PIPE)

        # Build
        cmd = conf['build']
        if EMTEST_BUILD_VERBOSE >= 3 and 'Ninja' not in generator:
          cmd += ['VERBOSE=1']
        self.run_process(cmd, stdout=None if EMTEST_BUILD_VERBOSE >= 2 else PIPE)
        self.assertExists(output_file, 'building a cmake-generated Makefile failed to produce an output file %s!' % output_file)

        # Run through node, if CMake produced a .js file.
        if output_file.endswith('.js'):
          ret = self.run_js(output_file)
          self.assertTextDataIdentical(read_file(cmakelistsdir + '/out.txt').strip(), ret.strip())

        if test_dir == 'post_build':
          ret = self.run_process(['ctest'], env=env)

  # Test that the various CMAKE_xxx_COMPILE_FEATURES that are advertised for the Emscripten
  # toolchain match with the actual language features that Clang supports.
  # If we update LLVM version and this test fails, copy over the new advertised features from Clang
  # and place them to cmake/Modules/Platform/Emscripten.cmake.
  @no_windows('Skipped on Windows because CMake does not configure native Clang builds well on Windows.')
  def test_cmake_compile_features(self):
    os.mkdir('build_native')
    cmd = ['cmake',
           '-DCMAKE_C_COMPILER=' + CLANG_CC, '-DCMAKE_C_FLAGS=--target=' + clang_native.get_native_triple(),
           '-DCMAKE_CXX_COMPILER=' + CLANG_CXX, '-DCMAKE_CXX_FLAGS=--target=' + clang_native.get_native_triple(),
           test_file('cmake/stdproperty')]
    print(str(cmd))
    native_features = self.run_process(cmd, stdout=PIPE, cwd='build_native').stdout

    os.mkdir('build_emcc')
    cmd = [EMCMAKE, 'cmake', test_file('cmake/stdproperty')]
    print(str(cmd))
    emscripten_features = self.run_process(cmd, stdout=PIPE, cwd='build_emcc').stdout

    native_features = '\n'.join([x for x in native_features.split('\n') if '***' in x])
    emscripten_features = '\n'.join([x for x in emscripten_features.split('\n') if '***' in x])
    self.assertTextDataIdentical(native_features, emscripten_features)

  # Test that the user's explicitly specified generator is always honored
  # Internally we override the generator on windows, unles the user specifies one
  # Test require Ninja to be installed
  @requires_ninja
  def test_cmake_explicit_generator(self):
    # use -Wno-dev to suppress an irrelevant warning about the test files only.
    cmd = [EMCMAKE, 'cmake', '-GNinja', '-Wno-dev', test_file('cmake/cpp_lib')]
    self.run_process(cmd)
    self.assertExists(self.get_dir() + '/build.ninja')

  # Tests that it's possible to pass C++11 or GNU++11 build modes to CMake by building code that
  # needs C++11 (embind)
  @requires_ninja
  @parameterized({
    '': [[]],
    '_no_gnu': [['-DNO_GNU_EXTENSIONS=1']],
  })
  def test_cmake_with_embind_cpp11_mode(self, args):
    # Use ninja generator here since we assume its always installed on our build/test machines.
    configure = [EMCMAKE, 'cmake', '-GNinja', test_file('cmake/cmake_with_emval')] + args
    self.run_process(configure)
    build = ['cmake', '--build', '.']
    self.run_process(build)

    out = self.run_js('cmake_with_emval.js')
    if '-DNO_GNU_EXTENSIONS=1' in args:
      self.assertContained('Hello! __STRICT_ANSI__: 1, __cplusplus: 201103', out)
    else:
      self.assertContained('Hello! __STRICT_ANSI__: 0, __cplusplus: 201103', out)

  # Tests that the Emscripten CMake toolchain option
  def test_cmake_bitcode_static_libraries(self):
    # Test that this option produces an error
    err = self.expect_fail([EMCMAKE, 'cmake', test_file('cmake/static_lib'), '-DEMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES=ON'])
    self.assertContained('EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES is not compatible with the', err)

  @parameterized({
    '': ['0'],
    '_suffix': ['1'],
  })
  def test_cmake_static_lib(self, custom):
    # Test that one is able to use custom suffixes for static libraries.
    # (sometimes projects want to emulate stuff, and do weird things like files
    # with ".so" suffix which are in fact either ar archives or bitcode files)
    self.run_process([EMCMAKE, 'cmake', f'-DSET_CUSTOM_SUFFIX_IN_PROJECT={custom}', test_file('cmake/static_lib')])
    self.run_process(['cmake', '--build', '.'])
    if custom == '1':
      self.assertTrue(building.is_ar('myprefix_static_lib.somecustomsuffix'))
    else:
      self.assertTrue(building.is_ar('libstatic_lib.a'))

  # Tests that cmake functions which require evaluation via the node runtime run properly with pthreads
  def test_cmake_pthreads(self):
    self.run_process([EMCMAKE, 'cmake', '-DCMAKE_C_FLAGS=-pthread', test_file('cmake/target_js')])

  # Tests that the CMake variable EMSCRIPTEN_VERSION is properly provided to user CMake scripts
  def test_cmake_emscripten_version(self):
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/emscripten_version')])
    self.clear()
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/emscripten_version'), '-DEMSCRIPTEN_FORCE_COMPILERS=OFF'])

  def test_cmake_emscripten_system_processor(self):
    cmake_dir = test_file('cmake/emscripten_system_processor')
    # The default CMAKE_SYSTEM_PROCESSOR is x86.
    out = self.run_process([EMCMAKE, 'cmake', cmake_dir], stdout=PIPE).stdout
    self.assertContained('CMAKE_SYSTEM_PROCESSOR is x86', out)

    # It can be overridden by setting EMSCRIPTEN_SYSTEM_PROCESSOR.
    out = self.run_process(
      [EMCMAKE, 'cmake', cmake_dir, '-DEMSCRIPTEN_SYSTEM_PROCESSOR=arm'], stdout=PIPE).stdout
    self.assertContained('CMAKE_SYSTEM_PROCESSOR is arm', out)

  @requires_network
  def test_cmake_find_stuff(self):
    # Ensure that zlib exists in the sysroot
    self.run_process([EMCC, test_file('hello_world.c'), '-sUSE_ZLIB'])
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/find_stuff')])

  def test_cmake_install(self):
    # Build and install a library `foo`
    os.mkdir('build1')
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/install_lib')], cwd='build1')
    self.run_process(['cmake', '--build', 'build1'])
    # newer versions of cmake support --install but we currently have 3.10.2 in CI
    # so we using `--build --target install` instead.
    self.run_process(['cmake', '--build', 'build1', '--target', 'install'])
    # Build an application that uses `find_package` to locate and use the above library.
    os.mkdir('build2')
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/find_package')], cwd='build2')
    self.run_process(['cmake', '--build', 'build2'])
    self.assertContained('foo: 42\n', self.run_js('build2/Bar.js'))
    self.run_process(['cmake', '--build', 'build2', '--target', 'install'])

  @requires_network
  def test_cmake_find_modules(self):
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/find_modules')])
    self.run_process(['cmake', '--build', '.'])
    output = self.run_js('test_prog.js')
    self.assertContained('AL_VERSION: 1.1', output)
    self.assertContained('SDL version: 2.', output)

  def test_cmake_threads(self):
    self.run_process([EMCMAKE, 'cmake', test_file('cmake/threads')])
    self.run_process(['cmake', '--build', '.'])

  @requires_pkg_config
  def test_cmake_find_pkg_config(self):
    out = self.run_process([EMCMAKE, 'cmake', test_file('cmake/find_pkg_config')], stdout=PIPE).stdout
    libdir = cache.get_sysroot_dir('local/lib/pkgconfig')
    libdir += os.path.pathsep + cache.get_sysroot_dir('lib/pkgconfig')
    self.assertContained('PKG_CONFIG_LIBDIR: ' + libdir, out)

  @requires_pkg_config
  @crossplatform
  def test_pkg_config_packages(self):
    packages = [
      ('egl', '10.2.2'),
      ('glesv2', '10.2.2'),
      ('glfw3', '3.2.1'),
      ('sdl', '1.2.15'),
    ]
    for package, version in packages:
        out = self.run_process([emmake, 'pkg-config', '--modversion', package], stdout=PIPE).stdout
        self.assertContained(version, out)

  @parameterized({
    '': [None],
    'wasm64': ['-sMEMORY64'],
    'pthreads': ['-pthread'],
  })
  def test_cmake_check_type_size(self, cflag):
    if cflag == '-sMEMORY64':
      self.require_wasm64()
    cmd = [EMCMAKE, 'cmake', test_file('cmake/check_type_size')]
    if cflag:
      cmd += [f'-DCMAKE_CXX_FLAGS={cflag}', f'-DCMAKE_C_FLAGS={cflag}']
    output = self.run_process(cmd, stdout=PIPE).stdout
    if cflag == '-sMEMORY64':
      self.assertContained('CMAKE_SIZEOF_VOID_P -> 8', output)
    else:
      self.assertContained('CMAKE_SIZEOF_VOID_P -> 4', output)

    # Verify that this test works without needing to run node.  We do this by breaking node
    # execution.
    self.run_process(cmd + ['-DCMAKE_CROSSCOMPILING_EMULATOR=/missing_binary'])

  @crossplatform
  def test_system_include_paths(self):
    # Verify that all default include paths are within `emscripten/system`

    def verify_includes(stderr):
      self.assertContained('<...> search starts here:', stderr)
      assert stderr.count('End of search list.') == 1, stderr
      start = stderr.index('<...> search starts here:')
      end = stderr.index('End of search list.')
      includes = stderr[start:end]
      includes = [i.strip() for i in includes.splitlines()[1:]]
      cache.ensure_setup()
      cachedir = os.path.normpath(cache.cachedir)
      llvmroot = os.path.normpath(os.path.dirname(config.LLVM_ROOT))
      for i in includes:
        i = os.path.normpath(i)
        # we also allow for the cache include directory and llvm's own builtin includes.
        # all other include paths should be inside the sysroot.
        if i.startswith(cachedir) or i.startswith(llvmroot):
          continue
        self.assertContained(path_from_root('system'), i)

    err = self.run_process([EMCC, test_file('hello_world.c'), '-v'], stderr=PIPE).stderr
    verify_includes(err)
    err = self.run_process([EMXX, test_file('hello_world.cpp'), '-v'], stderr=PIPE).stderr
    verify_includes(err)

  @with_both_compilers
  def test_failure_error_code(self, compiler):
    # Test that if one file is missing from the build, then emcc shouldn't succeed, and shouldn't produce an output file.
    self.expect_fail([compiler, test_file('hello_world.c'), 'this_file_is_missing.c', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))

  @with_both_compilers
  def test_failure_modularize_and_catch_rejection(self, compiler):
    # Test that if sMODULARIZE and sNODEJS_CATCH_REJECTION are both enabled, then emcc shouldn't succeed, and shouldn't produce an output file.
    self.expect_fail([compiler, test_file('hello_world.c'), '-sMODULARIZE', '-sNODEJS_CATCH_REJECTION', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))

  @with_both_compilers
  def test_failure_modularize_and_catch_exit(self, compiler):
    # Test that if sMODULARIZE and sNODEJS_CATCH_EXIT are both enabled, then emcc shouldn't succeed, and shouldn't produce an output file.
    self.expect_fail([compiler, test_file('hello_world.c'), '-sMODULARIZE', '-sNODEJS_CATCH_EXIT', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))

  def test_use_cxx(self):
    create_file('empty_file', ' ')
    dash_xc = self.run_process([EMCC, '-v', '-xc', 'empty_file'], stderr=PIPE).stderr
    self.assertNotContained('-x c++', dash_xc)
    dash_xcpp = self.run_process([EMCC, '-v', '-xc++', 'empty_file'], stderr=PIPE).stderr
    self.assertContained('-x c++', dash_xcpp)

  @with_both_compilers
  def test_cxx11(self, compiler):
    for std in ('-std=c++11', '--std=c++11'):
      self.run_process([compiler, std, test_file('hello_cxx11.cpp')])

  # Regression test for issue #4522: Incorrect CC vs CXX detection
  @with_both_compilers
  def test_incorrect_c_detection(self, compiler):
    # This auto-detection only works for the compile phase.
    # For linking you need to use `em++` or pass `-x c++`
    create_file('test.c', 'foo\n')
    self.run_process([compiler, '-c', '-lembind', '--embed-file', 'test.c', test_file('hello_world.cpp')])

  def test_odd_suffixes(self):
    for suffix in ('CPP', 'c++', 'C++', 'cxx', 'CXX', 'cc', 'CC'):
      self.clear()
      print(suffix)
      shutil.copy(test_file('hello_world.c'), 'test.' + suffix)
      self.do_runf('test.' + suffix, 'hello, world!')

    for suffix in ('lo'):
      self.clear()
      print(suffix)
      self.run_process([EMCC, test_file('hello_world.c'), '-shared', '-o', 'binary.' + suffix])
      self.run_process([EMCC, 'binary.' + suffix])
      self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_preprocessed_input(self):
    # .i and .ii files are assumed to be the output the pre-processor so clang doesn't add include
    # paths.  This means we can only compile and run things that don't contain includes.
    for suffix in ('.i', '.ii'):
      create_file('simple' + suffix, '''
        #ifdef __cplusplus
        extern "C" {
        #endif
        int puts(const char *s);
        #ifdef __cplusplus
        }
        #endif
        int main() { puts("hello"); }
        ''')
      self.do_runf('simple' + suffix, 'hello')

      create_file('with_include' + suffix, '#include <stdio.h>\nint main() { puts("hello"); }')
      err = self.expect_fail([EMCC, 'with_include' + suffix])
      self.assertContained('fatal error: \'stdio.h\' file not found', err)

  def test_wl_linkflags(self):
    # Test path -L and -l via -Wl, arguments and -Wl, response files
    create_file('main.c', '''
      void printey();
      int main() {
        printey();
        return 0;
      }
    ''')
    create_file('libfile.c', '''
      #include <stdio.h>
      void printey() {
        printf("hello from lib\\n");
      }
    ''')
    create_file('linkflags.txt', '''
    -L.
    -lfoo
    ''')
    self.run_process([EMCC, '-o', 'libfile.o', '-c', 'libfile.c'])
    self.run_process([EMAR, 'cr', 'libfoo.a', 'libfile.o'])
    self.run_process([EMCC, 'main.c', '-L.', '-lfoo'])
    self.run_process([EMCC, 'main.c', '-Wl,-L.', '-Wl,-lfoo'])
    self.run_process([EMCC, 'main.c', '-Wl,@linkflags.txt'])

  def test_wl_stackfirst(self):
    cmd = [EMCC, test_file('hello_world.c'), '-Wl,--stack-first']
    self.run_process(cmd + ['-O0'])
    self.run_process(cmd + ['-O2'])
    err = self.expect_fail(cmd + ['-fsanitize=address'])
    self.assertContained('error: --stack-first is not compatible with asan', err)
    err = self.expect_fail(cmd + ['-sGLOBAL_BASE=1024'])
    self.assertContained('error: --stack-first is not compatible with -sGLOBAL_BASE', err)

  @parameterized({
    # In a simple -O0 build we do not set --low-memory-unused (as the stack is
    # first, which is nice for debugging but bad for code size (larger globals)
    # and bad for the low-memory-unused trick.
    '': ([], False),
    # When we optimize, we do.
    'O2': (['-O2'], True),
    # But a low global base prevents it.
    'O2_GB_512': (['-O2', '-sGLOBAL_BASE=512'], False),
    # A large-enough global base allows it.
    'O2_GB_1024': (['-O2', '-sGLOBAL_BASE=1024'], True),
    # Forcing the stack to be first in the linker prevents it.
    'linker_flag': (['-O2', '-Wl,--stack-first'], False),
  })
  def test_binaryen_low_memory_unused(self, args, low_memory_unused):
    cmd = [EMCC, test_file('hello_world.c'), '-v'] + args
    err = self.run_process(cmd, stdout=PIPE, stderr=PIPE).stderr
    self.assertContainedIf('--low-memory-unused ', err, low_memory_unused)

  def test_l_link(self):
    # Linking with -lLIBNAME and -L/DIRNAME should work, also should work with spaces
    create_file('main.c', '''
      extern void printey();
      int main() {
        printey();
        return 0;
      }
    ''')
    create_file('libfile.c', '''
      #include <stdio.h>
      void printey() {
        printf("hello from lib\\n");
      }
    ''')

    ensure_dir('libdir')
    libfile = Path('libdir/libfile.so')
    aout = 'a.out.js'

    def build(path, args):
      self.run_process([EMCC, path] + args)

    # Test linking the library built here by emcc
    build('libfile.c', ['-c'])
    shutil.move('libfile.o', libfile)
    build('main.c', ['-L' + 'libdir', '-lfile'])

    self.assertContained('hello from lib', self.run_js(aout))

    # Also test execution with `-l c` and space-separated library linking syntax
    os.remove(aout)
    build('libfile.c', ['-c', '-l', 'c'])
    shutil.move('libfile.o', libfile)
    build('main.c', ['-L', 'libdir', '-l', 'file'])

    self.assertContained('hello from lib', self.run_js(aout))

    # Must not leave unneeded linker stubs
    self.assertNotExists('a.out')
    self.assertNotExists('a.exe')

  def test_commons_link(self):
    create_file('a.h', r'''
      #if !defined(A_H)
      #define A_H
      extern int foo[8];
      #endif
    ''')
    create_file('a.c', r'''
      #include "a.h"
      int foo[8];
    ''')
    create_file('main.c', r'''
      #include <stdio.h>
      #include "a.h"

      int main() {
        printf("|%d|\n", foo[0]);
        return 0;
      }
    ''')

    self.run_process([EMCC, '-o', 'a.o', '-c', 'a.c'])
    self.run_process([EMAR, 'rv', 'library.a', 'a.o'])
    self.run_process([EMCC, '-o', 'main.o', '-c', 'main.c'])
    self.run_process([EMCC, '-o', 'a.js', 'main.o', 'library.a'])
    self.assertContained('|0|', self.run_js('a.js'))

  @parameterized({
    'expand_symlinks': [[]],
    'no_canonical_prefixes': [['-no-canonical-prefixes']],
  })
  @no_windows('Windows does not support symlinks')
  def test_symlink_points_to_bad_suffix(self, flags):
    """Tests compiling a symlink where foobar.c points to foobar.xxx.

    In this case, we should always successfully compile the code."""
    create_file('foobar.xxx', 'int main(){ return 0; }')
    os.symlink('foobar.xxx', 'foobar.c')
    self.run_process([EMCC, 'foobar.c', '-c', '-o', 'foobar.o'] + flags)

  @no_windows('Windows does not support symlinks')
  def test_symlink_has_bad_suffix(self):
    """Tests that compiling foobar.xxx fails even if it points to foobar.c.
    """
    create_file('foobar.c', 'int main(){ return 0; }')
    os.symlink('foobar.c', 'foobar.xxx')
    err = self.expect_fail([EMCC, 'foobar.xxx', '-o', 'foobar.js'])
    self.assertContained(['unknown file type: foobar.xxx', "archive member 'native.o' is neither Wasm object file nor LLVM bitcode"], err)

  def test_multiply_defined_libsymbols(self):
    create_file('libA.c', 'int mult() { return 1; }')
    create_file('a2.c', 'void x() {}')
    create_file('b2.c', 'void y() {}')
    create_file('main.c', r'''
      #include <stdio.h>
      int mult();
      int main() {
        printf("result: %d\n", mult());
        return 0;
      }
    ''')

    self.emcc_args.remove('-Werror')
    self.emcc('libA.c', ['-shared'], output_filename='libA.so')

    self.emcc('a2.c', ['-r', '-L.', '-lA', '-o', 'a2.o'])
    self.emcc('b2.c', ['-r', '-L.', '-lA', '-o', 'b2.o'])

    self.emcc('main.c', ['-L.', '-lA', 'a2.o', 'b2.o'])

    self.assertContained('result: 1', self.run_js('a.out.js'))

  def test_multiply_defined_libsymbols_2(self):
    create_file('a.c', "int x() { return 55; }")
    create_file('b.c', "int y() { return 2; }")
    create_file('c.c', "int z() { return 5; }")
    create_file('main.c', r'''
      #include <stdio.h>
      int x();
      int y();
      int z();
      int main() {
        printf("result: %d\n", x() + y() + z());
        return 0;
      }
    ''')

    self.emcc('a.c', ['-c']) # a.o
    self.emcc('b.c', ['-c']) # b.o
    self.emcc('c.c', ['-c']) # c.o
    building.emar('cr', 'libLIB.a', ['a.o', 'b.o']) # libLIB.a with a and b

    # a is in the lib AND in an .o, so should be ignored in the lib. We do still need b from the lib though
    self.emcc('main.c', ['a.o', 'c.o', '-L.', '-lLIB'])

    self.assertContained('result: 62', self.run_js('a.out.js'))

  def test_link_group(self):
    create_file('lib.c', 'int x() { return 42; }')

    create_file('main.c', r'''
      #include <stdio.h>
      int x();
      int main() {
        printf("result: %d\n", x());
        return 0;
      }
    ''')

    self.emcc('lib.c', ['-c']) # lib.o
    lib_name = 'libLIB.a'
    building.emar('cr', lib_name, ['lib.o']) # libLIB.a with lib.o

    def test(compiler, main_name, lib_args, err_expected):
      print(err_expected)
      output = self.run_process([compiler, main_name, '-o', 'a.out.js'] + lib_args, stderr=PIPE, check=not err_expected)
      if err_expected:
        self.assertContained(err_expected, output.stderr)
      else:
        self.assertNotContained('undefined symbol', output.stderr)
        out_js = 'a.out.js'
        self.assertExists(out_js)
        self.assertContained('result: 42', self.run_js(out_js))

    test(EMCC, 'main.c', ['-Wl,--start-group', lib_name, '-Wl,--end-group'], None)
    test(EMCC, 'main.c', ['-Wl,--start-group', lib_name], None)

    print('embind test with groups')

    create_file('main.cpp', r'''
      #include <stdio.h>
      #include <emscripten/val.h>
      using namespace emscripten;
      extern "C" int x();
      int main() {
        int y = -x();
        y = val::global("Math").call<int>("abs", y);
        printf("result: %d\n", y);
        return 0;
      }
    ''')
    test(EMXX, 'main.cpp', ['-Wl,--start-group', lib_name, '-Wl,--end-group', '-lembind'], None)

  def test_whole_archive(self):
    # Verify that -Wl,--whole-archive includes the static constructor from the
    # otherwise unreferenced library.
    self.run_process([EMCC, '-c', '-o', 'main.o', test_file('test_whole_archive/main.c')])
    self.run_process([EMCC, '-c', '-o', 'testlib.o', test_file('test_whole_archive/testlib.c')])
    self.run_process([EMAR, 'crs', 'libtest.a', 'testlib.o'])

    self.run_process([EMCC, '-Wl,--whole-archive', 'libtest.a', '-Wl,--no-whole-archive', 'main.o'])
    self.assertContained('foo is: 42\n', self.run_js('a.out.js'))

    self.run_process([EMCC, '-Wl,-whole-archive', 'libtest.a', '-Wl,-no-whole-archive', 'main.o'])
    self.assertContained('foo is: 42\n', self.run_js('a.out.js'))

    # Verify the --no-whole-archive prevents the inclusion of the ctor
    self.run_process([EMCC, '-Wl,-whole-archive', '-Wl,--no-whole-archive', 'libtest.a', 'main.o'])
    self.assertContained('foo is: 0\n', self.run_js('a.out.js'))

  def test_whole_archive_48156(self):
    # Regression test for http://llvm.org/PR48156
    # TODO: distill this test further and move to lld
    self.run_process([EMXX, '-c', '-o', 'foo.o', '-O1',
                      test_file('test_whole_archive_foo.cpp')])
    self.run_process([EMXX, '-c', '-o', 'main.o', '-O1',
                      test_file('test_whole_archive_main.cpp')])
    self.run_process([EMAR, 'rc', 'libfoo.a', 'foo.o'])
    self.run_process([EMAR, 'rc', 'libmain.a', 'main.o'])
    self.run_process([
        EMXX, test_file('test_whole_archive_init.cpp'),
        '-O1', 'libfoo.a', '-Wl,--whole-archive', 'libmain.a', '-Wl,--no-whole-archive'])
    self.assertContained('Result: 11', self.run_js('a.out.js'))

  def test_link_group_bitcode(self):
    create_file('1.c', r'''
int f(void);
int main() {
  f();
  return 0;
}
''')
    create_file('2.c', r'''
#include <stdio.h>
int f() {
  printf("Hello\n");
  return 0;
}
''')

    self.run_process([EMCC, '-flto', '-o', '1.o', '-c', '1.c'])
    self.run_process([EMCC, '-flto', '-o', '2.o', '-c', '2.c'])
    self.run_process([EMAR, 'crs', '2.a', '2.o'])
    self.run_process([EMCC, '-r', '-flto', '-o', 'out.o', '-Wl,--start-group', '2.a', '1.o', '-Wl,--end-group'])
    self.run_process([EMCC, 'out.o'])
    self.assertContained('Hello', self.run_js('a.out.js'))

  # We deliberately ignore duplicate input files in order to allow
  # "libA.so" on the command line twice. This is not really .so support
  # and the .so files are really object files.
  def test_redundant_link(self):
    create_file('libA.c', 'int mult() { return 1; }')
    create_file('main.c', r'''
      #include <stdio.h>
      int mult();
      int main() {
        printf("result: %d\n", mult());
        return 0;
      }
    ''')

    self.emcc_args.remove('-Werror')
    self.emcc('libA.c', ['-shared'], output_filename='libA.so')
    self.emcc('main.c', ['libA.so', 'libA.so'], output_filename='a.out.js')
    self.assertContained('result: 1', self.run_js('a.out.js'))

  @no_mac('https://github.com/emscripten-core/emscripten/issues/16649')
  @crossplatform
  def test_dot_a_all_contents_invalid(self):
    # check that we error if an object file in a .a is not valid bitcode.
    # do not silently ignore native object files, which may have been
    # built by mistake
    create_file('native.c', 'int native() { return 5; }')
    create_file('main.c', 'extern int native(); int main() { return native(); }')
    self.run_process([CLANG_CC, 'native.c', '-c', '-o', 'native.o'] +
                     clang_native.get_clang_native_args())
    self.run_process([EMAR, 'crs', 'libfoo.a', 'native.o'])
    stderr = self.expect_fail([EMCC, 'main.c', 'libfoo.a'])
    self.assertContained(['unknown file type', "libfoo.a: archive member 'native.o' is neither Wasm object file nor LLVM bitcode"], stderr)

  def test_export_all(self):
    lib = r'''
      #include <stdio.h>
      void libf1() { printf("libf1\n"); }
      void libf2() { printf("libf2\n"); }
    '''
    create_file('lib.c', lib)

    create_file('pre.js', '''
      Module.onRuntimeInitialized = () => {
        _libf1();
        _libf2();
      };
    ''')

    # Explicitly test with -Oz to ensure libc_optz is included alongside
    # libc when `--whole-archive` is used.
    self.emcc('lib.c', ['-Oz', '-sEXPORT_ALL', '-sLINKABLE', '--pre-js', 'pre.js'], output_filename='a.out.js')
    self.assertContained('libf1\nlibf2\n', self.run_js('a.out.js'))

  def test_export_keepalive(self):
    create_file('main.c', r'''
      #include <emscripten.h>
      EMSCRIPTEN_KEEPALIVE int libf1() { return 42; }
    ''')

    create_file('pre.js', '''
      Module.onRuntimeInitialized = () => {
        out(Module._libf1 ? Module._libf1() : 'unexported');
      };
    ''')

    # By default, all kept alive functions should be exported.
    self.do_runf('main.c', '42\n', emcc_args=['--pre-js', 'pre.js'])

    # Ensures that EXPORT_KEEPALIVE=0 remove the exports
    self.do_runf('main.c', 'unexported\n', emcc_args=['-sEXPORT_KEEPALIVE=0', '--pre-js', 'pre.js'])

  def test_minimal_modularize_export_keepalive(self):
    self.set_setting('MODULARIZE')
    self.set_setting('MINIMAL_RUNTIME')

    create_file('main.c', r'''
      #include <emscripten.h>
      EMSCRIPTEN_KEEPALIVE int libf1() { return 42; }
    ''')

    # With MINIMAL_RUNTIME, the module instantiation function isn't exported neither as a UMD nor as
    # an ES6 module.
    # Thus, it's impossible to use `require` or `import` and instead run the module
    # as part of --extern-post-js.
    create_file('post.js', 'Module().then((mod) => console.log(mod._libf1()));')
    self.emcc_args += ['--extern-post-js=post.js']

    # By default, no symbols should be exported when using MINIMAL_RUNTIME.
    self.emcc('main.c', [])
    self.assertContained('TypeError: mod._libf1 is not a function', self.run_js('a.out.js', assert_returncode=NON_ZERO))

    # Ensures that EXPORT_KEEPALIVE=1 exports the symbols.
    self.emcc('main.c', ['-sEXPORT_KEEPALIVE=1'])
    self.assertContained('42\n', self.run_js('a.out.js'))

  @crossplatform
  def test_minimal_runtime_export_all_modularize(self):
    """This test ensures that MODULARIZE and EXPORT_ALL work simultaneously.

    In addition, it ensures that EXPORT_ALL is honored while using MINIMAL_RUNTIME.
    """

    create_file('main.c', r'''
      #include <stdio.h>
      #include <emscripten.h>
      EMSCRIPTEN_KEEPALIVE void libf1() { printf("libf1\n"); }
      EMSCRIPTEN_KEEPALIVE void libf2() { printf("libf2\n"); }
    ''')

    self.emcc('main.c', ['-sMODULARIZE=1', '-sMINIMAL_RUNTIME=2', '-sEXPORT_ALL', '-sEXPORT_ES6'], output_filename='test.mjs')

    # We must expose __dirname and require globally because emscripten
    # uses those under the hood.
    create_file('main.mjs', '''
      import { dirname } from 'path';
      import { createRequire } from 'module';
      import { fileURLToPath } from 'url';

      // `fileURLToPath` is used to get a valid path on Windows.
      globalThis.__dirname = dirname(fileURLToPath(import.meta.url));
      globalThis.require = createRequire(import.meta.url);

      import Test from './test.mjs';
      async function main() {
        const mod = await Test();
        mod._libf1();
        mod._libf2();
      }
      main();
    ''')
    self.assertContained('libf1\nlibf2\n', self.run_js('main.mjs'))

  def test_export_all_and_exported_functions(self):
    # EXPORT_ALL should not export library functions by default.
    # This means that to export library function you also need to explicitly
    # list them in EXPORTED_FUNCTIONS.
    lib = r'''
      #include <stdio.h>
      #include <emscripten.h>
      EMSCRIPTEN_KEEPALIVE void libfunc() { puts("libfunc\n"); }
      void libfunc2() { puts("libfunc2\n"); }
    '''
    create_file('lib.c', lib)
    create_file('pre.js', '''
      Module.onRuntimeInitialized = () => {
        _libfunc();
        _libfunc2();
      };
    ''')

    # libfunc2 should not be linked by default, even with EXPORT_ALL
    self.emcc('lib.c', ['-sEXPORT_ALL', '--pre-js', 'pre.js'], output_filename='a.out.js')
    err = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('_libfunc2 is not defined', err)

    self.emcc('lib.c', ['-sEXPORTED_FUNCTIONS=_libfunc2', '-sEXPORT_ALL', '--pre-js', 'pre.js'], output_filename='a.out.js')
    self.assertContained('libfunc\n', self.run_js('a.out.js'))

  @also_with_wasmfs
  @crossplatform
  @parameterized({
    '': ([],),
    'closure': (['-O2', '--closure=1'],),
  })
  def test_stdin(self, args):
    create_file('in.txt', 'abcdef\nghijkl\n')
    self.set_setting('ENVIRONMENT', 'node,shell')
    self.emcc(test_file('module/test_stdin.c'), args=args, output_filename='out.js')

    for engine in config.JS_ENGINES:
      engine[0] = os.path.normpath(engine[0])
      # work around a bug in python's subprocess module
      # (we'd use self.run_js() normally)
      delete_file('out.txt')
      cmd = jsrun.make_command(os.path.normpath('out.js'), engine)
      cmd = shared.shlex_join(cmd)
      print(cmd, file=sys.stderr)
      if WINDOWS:
        os.system(f'type "in.txt" | {cmd} >out.txt')
      else: # posix
        os.system(f'cat in.txt | {cmd} > out.txt')
      self.assertContained('abcdef\nghijkl\neof', read_file('out.txt'))

  @crossplatform
  def test_module_stdin(self):
    self.set_setting('FORCE_FILESYSTEM')
    create_file('pre.js', '''
const data = 'hello, world!\\n'.split('').map(c => c.charCodeAt(0));
Module['stdin'] = () => data.shift() || null;
''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_runf('module/test_stdin.c', 'hello, world!')

  @crossplatform
  def test_module_stdout_stderr(self):
    self.set_setting('FORCE_FILESYSTEM')
    create_file('pre.js', '''
let stdout = [];
let stderr = [];

Module['stdout'] = (char) => stdout.push(char);
Module['stderr'] = (char) => stderr.push(char);
Module['postRun'] = () => {
    assert(stderr.length === 0, 'stderr should be empty. \\n' +
        'stderr: \\n' + stderr);
    assert(UTF8ArrayToString(stdout).startsWith('hello, world!'), 'stdout should start with the famous greeting. \\n' +
        'stdout: \\n' + stdout);
}
''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_runf('hello_world.c')

  @crossplatform
  def test_module_print_printerr(self):
    self.set_setting('FORCE_FILESYSTEM')
    create_file('pre.js', '''
let stdout = '';
let stderr = '';

Module['print'] = (text) => stdout += text;
Module['printErr'] = (text) => stderr += text;
Module['postRun'] = () => {
    assert(stderr === '', 'stderr should be empty. \\n' +
        'stderr: \\n' + stderr);
    assert(stdout.startsWith('hello, world!'), 'stdout should start with the famous greeting. \\n' +
        'stdout: \\n' + stdout);
}
''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_runf('hello_world.c')

  def test_ungetc_fscanf(self):
    create_file('main.c', r'''
      #include <stdio.h>
      int main(int argc, char const *argv[])
      {
          char str[4] = {0};
          FILE* f = fopen("my_test.input", "r");
          if (f == NULL) {
              printf("cannot open file\n");
              return -1;
          }
          ungetc('x', f);
          ungetc('y', f);
          ungetc('z', f);
          fscanf(f, "%3s", str);
          printf("%s\n", str);
          return 0;
      }
    ''')
    create_file('my_test.input', 'abc')
    self.emcc('main.c', ['--embed-file', 'my_test.input'], output_filename='a.out.js')
    self.assertContained('zyx', self.run_process(config.JS_ENGINES[0] + ['a.out.js'], stdout=PIPE, stderr=PIPE).stdout)

  def test_abspaths(self):
    # Includes with absolute paths are generally dangerous, things like -I/usr/.. will get to system
    # local headers, not our portable ones.
    for args, expected in [(['-I/usr/something', '-Wwarn-absolute-paths'], True),
                           (['-L/usr/something', '-Wwarn-absolute-paths'], True),
                           (['-I/usr/something'], False),
                           (['-L/usr/something'], False),
                           (['-I/usr/something', '-Wno-warn-absolute-paths'], False),
                           (['-L/usr/something', '-Wno-warn-absolute-paths'], False),
                           (['-Isubdir/something', '-Wwarn-absolute-paths'], False),
                           (['-Lsubdir/something', '-Wwarn-absolute-paths'], False),
                           ([], False)]:
      print(args, expected)
      proc = self.run_process([EMCC, test_file('hello_world.c')] + args, stderr=PIPE)
      WARNING = 'encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript)'
      self.assertContainedIf(WARNING, proc.stderr, expected)

  def test_identical_basenames(self):
    # Issue 287: files in different dirs but with the same basename get confused as the same,
    # causing multiply defined symbol errors
    ensure_dir('foo')
    ensure_dir('bar')
    create_file('foo/main.c', '''
      extern void printey();
      int main() {
        printey();
        return 0;
      }
    ''')
    create_file('bar/main.c', '''
      #include <stdio.h>
      void printey() { printf("hello there\\n"); }
    ''')

    self.run_process([EMCC, Path('foo/main.c'), Path('bar/main.c')])
    self.assertContained('hello there', self.run_js('a.out.js'))

    # ditto with first creating .o files
    delete_file('a.out.js')
    self.run_process([EMCC, '-c', Path('foo/main.c'), '-o', Path('foo/main.o')])
    self.run_process([EMCC, '-c', Path('bar/main.c'), '-o', Path('bar/main.o')])
    self.run_process([EMCC, Path('foo/main.o'), Path('bar/main.o')])
    self.assertContained('hello there', self.run_js('a.out.js'))

  def test_main_a(self):
    # if main() is in a .a, we need to pull in that .a
    create_file('main.c', r'''
      #include <stdio.h>
      extern int f();
      int main() {
        printf("result: %d.\n", f());
        return 0;
      }
    ''')

    create_file('other.c', r'''
      #include <stdio.h>
      int f() { return 12346; }
    ''')

    self.run_process([EMCC, '-c', 'main.c'])
    self.run_process([EMCC, '-c', 'other.c'])

    self.run_process([EMAR, 'cr', 'libmain.a', 'main.o'])

    self.run_process([EMCC, 'other.o', 'libmain.a'])

    self.assertContained('result: 12346.', self.run_js('a.out.js'))

  def test_multiple_archives_duplicate_basenames(self):
    create_file('common.c', r'''
      #include <stdio.h>
      void a(void) {
        printf("a\n");
      }
    ''')
    self.run_process([EMCC, 'common.c', '-c', '-o', 'common.o'])
    delete_file('liba.a')
    self.run_process([EMAR, 'rc', 'liba.a', 'common.o'])

    create_file('common.c', r'''
      #include <stdio.h>
      void b(void) {
        printf("b\n");
      }
    ''')
    self.run_process([EMCC, 'common.c', '-c', '-o', 'common.o'])
    delete_file('libb.a')
    self.run_process([EMAR, 'rc', 'libb.a', 'common.o'])

    create_file('main.c', r'''
      void a(void);
      void b(void);
      int main() {
        a();
        b();
      }
    ''')

    self.run_process([EMCC, 'main.c', '-L.', '-la', '-lb'])
    self.assertContained('a\nb\n', self.run_js('a.out.js'))

  def test_archive_duplicate_basenames(self):
    ensure_dir('a')
    create_file('a/common.c', r'''
      #include <stdio.h>
      void a(void) {
        printf("a\n");
      }
    ''')
    self.run_process([EMCC, Path('a/common.c'), '-c', '-o', Path('a/common.o')])

    ensure_dir('b')
    create_file('b/common.c', r'''
      #include <stdio.h>
      void b(void) {
        printf("b...\n");
      }
    ''')
    self.run_process([EMCC, Path('b/common.c'), '-c', '-o', Path('b/common.o')])

    delete_file('liba.a')
    self.run_process([EMAR, 'rc', 'liba.a', Path('a/common.o'), Path('b/common.o')])

    # Verify that archive contains basenames with hashes to avoid duplication
    text = self.run_process([EMAR, 't', 'liba.a'], stdout=PIPE).stdout
    self.assertEqual(text.count('common'), 2)
    for line in text.split('\n'):
      # should not have huge hash names
      self.assertLess(len(line), 20, line)

    create_file('main.c', r'''
      void a(void);
      void b(void);
      int main() {
        a();
        b();
      }
    ''')
    err = self.run_process([EMCC, 'main.c', '-L.', '-la'], stderr=PIPE).stderr
    self.assertNotIn('archive file contains duplicate entries', err)
    self.assertContained('a\nb...\n', self.run_js('a.out.js'))

    # Using llvm-ar directly should cause duplicate basenames
    delete_file('libdup.a')
    self.run_process([LLVM_AR, 'rc', 'libdup.a', Path('a/common.o'), Path('b/common.o')])
    text = self.run_process([EMAR, 't', 'libdup.a'], stdout=PIPE).stdout
    self.assertEqual(text.count('common.o'), 2)

    # With fastcomp we don't support duplicate members so this should generate
    # a warning.  With the wasm backend (lld) this is fully supported.
    cmd = [EMCC, 'main.c', '-L.', '-ldup']
    self.run_process(cmd)
    self.assertContained('a\nb...\n', self.run_js('a.out.js'))

  def test_export_from_archive(self):
    export_name = 'this_is_an_entry_point'
    full_export_name = '_this_is_an_entry_point'

    create_file('export.c', r'''
      #include <stdio.h>
      void this_is_an_entry_point(void) {
        printf("Hello, world!\n");
      }
    ''')
    self.run_process([EMCC, 'export.c', '-c', '-o', 'export.o'])
    self.run_process([EMAR, 'rc', 'libexport.a', 'export.o'])

    create_file('main.c', r'''
      int main() {
        return 0;
      }
    ''')

    # Sanity check: the symbol should not be linked in if not requested.
    self.run_process([EMCC, 'main.c', '-L.', '-lexport'])
    self.assertFalse(self.is_exported_in_wasm(export_name, 'a.out.wasm'))

    # Exporting it causes it to appear in the output.
    self.run_process([EMCC, 'main.c', '-L.', '-lexport', '-sEXPORTED_FUNCTIONS=%s' % full_export_name])
    self.assertTrue(self.is_exported_in_wasm(export_name, 'a.out.wasm'))

  @parameterized({
    'embed': (['--embed-file', 'somefile.txt'],),
    'embed_twice': (['--embed-file', 'somefile.txt', '--embed-file', 'somefile.txt'],),
    'preload': (['--preload-file', 'somefile.txt'],),
    'preload_closure': (['--preload-file', 'somefile.txt', '-O2', '--closure=1'],),
    'preload_and_embed': (['--preload-file', 'somefile.txt', '--embed-file', 'hello.txt'],)
  })
  @requires_node
  def test_include_file(self, args):
    create_file('somefile.txt', 'hello from a file with lots of data and stuff in it thank you very much')
    create_file('hello.txt', 'hello world')
    create_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      int main() {
        FILE *f = fopen("somefile.txt", "r");
        assert(f);
        char buf[100];
        int rtn = fread(buf, 1, 20, f);
        assert(rtn == 20);
        buf[20] = 0;
        fclose(f);
        printf("|%s|\n", buf);
        return 0;
      }
    ''')

    self.run_process([EMCC, 'main.c'] + args)
    # run in node.js to ensure we verify that file preloading works there
    result = self.run_js('a.out.js')
    self.assertContained('|hello from a file wi|', result)

  @parameterized({
    '': ([],),
    'wasmfs': (['-sWASMFS'],),
  })
  @crossplatform
  def test_embed_file_dup(self, args):
    ensure_dir('tst/test1')
    ensure_dir('tst/test2')

    create_file('tst/aa.txt', 'frist')
    create_file('tst/test1/aa.txt', 'sacond')
    create_file('tst/test2/aa.txt', 'thard')
    create_file('main.c', r'''
      #include <stdio.h>
      #include <string.h>
      void print_file(const char *name) {
        FILE *f = fopen(name, "r");
        char buf[100];
        memset(buf, 0, 100);
        fread(buf, 1, 20, f);
        buf[20] = 0;
        fclose(f);
        printf("|%s|\n", buf);
      }
      int main() {
        print_file("tst/aa.txt");
        print_file("tst/test1/aa.txt");
        print_file("tst/test2/aa.txt");
        return 0;
      }
    ''')

    self.do_runf('main.c', '|frist|\n|sacond|\n|thard|\n',
                 emcc_args=['--embed-file', 'tst'] + args)

  def test_exclude_file(self):
    ensure_dir('tst/abc.exe')
    ensure_dir('tst/abc.txt')

    create_file('tst/hello.exe', 'hello')
    create_file('tst/hello.txt', 'world')
    create_file('tst/abc.exe/foo', 'emscripten')
    create_file('tst/abc.txt/bar', '!!!')
    create_file('main.c', r'''
      #include <stdio.h>
      int main() {
        if(fopen("tst/hello.exe", "rb")) printf("Failed\n");
        if(!fopen("tst/hello.txt", "rb")) printf("Failed\n");
        if(fopen("tst/abc.exe/foo", "rb")) printf("Failed\n");
        if(!fopen("tst/abc.txt/bar", "rb")) printf("Failed\n");

        return 0;
      }
    ''')

    self.run_process([EMCC, 'main.c', '--embed-file', 'tst', '--exclude-file', '*.exe'])
    self.assertEqual(self.run_js('a.out.js').strip(), '')

  def test_dylink_strict(self):
    self.do_run_in_out_file_test('hello_world.c', emcc_args=['-sSTRICT', '-sMAIN_MODULE=1'])

  def test_dylink_exceptions_and_assetions(self):
    # Linking side modules using the STL and exceptions should not abort with
    # "function in Table but not functionsInTableMap" when using ASSERTIONS=2

    # A side module that uses the STL enables exceptions.
    create_file('side.cpp', r'''
      #include <vector>
      std::vector<int> v;
      std::vector<int> side(int n) {
          for (int i=0; i<n; i++) v.push_back(i);
          return v;
      }
      ''')
    self.run_process([
      EMXX,
      '-o', 'side.wasm',
      'side.cpp',
      '-sSIDE_MODULE',
      '-sDISABLE_EXCEPTION_CATCHING=0',
      '-sASSERTIONS=2'])

    create_file('main.cpp', r'''
      #include <stdio.h>
      #include <vector>
      std::vector<int> side(int n);
      int main(void) {
          auto v = side(10);
          for (auto i : v) printf("%d", i);
          printf("\n");
          return 0;
      }
      ''')

    self.do_runf(
      'main.cpp',
      '0123456789',
      emcc_args=[
        '-sMAIN_MODULE',
        '-sDISABLE_EXCEPTION_CATCHING=0',
        '-sASSERTIONS=2',
        'side.wasm',
      ])

  def test_multidynamic_link(self):
    # Linking the same dynamic library in statically will error, normally, since we statically link
    # it, causing dupe symbols

    def test(link_flags, lib_suffix):
      print(link_flags, lib_suffix)

      self.clear()
      ensure_dir('libdir')

      create_file('main.c', r'''
        #include <stdio.h>
        extern void printey();
        extern void printother();
        int main() {
          printf("*");
          printey();
          printf("\n");
          printother();
          printf("\n");
          printf("*\n");
          return 0;
        }
      ''')

      create_file('libdir/libfile.c', '''
        #include <stdio.h>
        void printey() {
          printf("hello from lib");
        }
      ''')

      create_file('libdir/libother.c', '''
        #include <stdio.h>
        extern void printey();
        void printother() {
          printf("|");
          printey();
          printf("|");
        }
      ''')

      # Build libfile normally into an .so
      self.run_process([EMCC, Path('libdir/libfile.c'), '-shared', '-o', Path('libdir/libfile.so' + lib_suffix)])
      # Build libother and dynamically link it to libfile
      self.run_process([EMCC, '-Llibdir', Path('libdir/libother.c')] + link_flags + ['-shared', '-o', Path('libdir/libother.so')])
      # Build the main file, linking in both the libs
      self.run_process([EMCC, '-Llibdir', os.path.join('main.c')] + link_flags + ['-lother', '-c'])
      print('...')
      # The normal build system is over. We need to do an additional step to link in the dynamic
      # libraries, since we ignored them before
      self.run_process([EMCC, '-Llibdir', 'main.o'] + link_flags + ['-lother'])

      self.assertContained('*hello from lib\n|hello from lib|\n*\n', self.run_js('a.out.js'))

    test(['-lfile'], '') # -l, auto detection from library path
    test([self.in_dir('libdir/libfile.so.3.1.4.1.5.9')], '.3.1.4.1.5.9') # handle libX.so.1.2.3 as well

  @node_pthreads
  def test_dylink_pthread_static_data(self):
    # Test that a side module uses the same static data region for global objects across all threads

    # A side module with a global object with a constructor.
    # * The global object must have a non-zero initial value to make sure that
    #   the memory is zero-initialized only once (and not once per thread).
    # * The global object must have a constructor to make sure that it is
    #   constructed only once (and not once per thread).
    create_file('side.cpp', r'''
      struct Data {
          Data() : value(42) {}
          int value;
      } data;
      int * get_address() {
          return &data.value;
      }
      ''')
    self.run_process([
      EMCC,
      '-o', 'side.wasm',
      'side.cpp',
      '-pthread', '-Wno-experimental',
      '-sSIDE_MODULE'])

    create_file('main.cpp', r'''
      #include <stdio.h>
      #include <thread>
      int * get_address();
      int main(void) {
          *get_address() = 123;
          std::thread([]{
            printf("%d\n", *get_address());
          }).join();
          return 0;
      }
      ''')

    self.do_runf(
      'main.cpp',
      '123',
      emcc_args=[
        '-pthread', '-Wno-experimental',
        '-sPROXY_TO_PTHREAD',
        '-sEXIT_RUNTIME',
        '-sMAIN_MODULE=2',
        'side.wasm',
      ])

  def test_dylink_pthread_warning(self):
    err = self.expect_fail([EMCC, '-Werror', '-sMAIN_MODULE', '-pthread', test_file('hello_world.c')])
    self.assertContained('error: -sMAIN_MODULE + pthreads is experimental', err)

  @node_pthreads
  def test_dylink_pthread_bigint_em_asm(self):
    self.set_setting('MAIN_MODULE', 2)
    self.set_setting('WASM_BIGINT')
    self.emcc_args += ['-Wno-experimental', '-pthread']
    self.do_runf('hello_world_em_asm.c', 'hello, world')

  @node_pthreads
  def test_dylink_pthread_bigint_em_js(self):
    self.set_setting('MAIN_MODULE', 2)
    self.set_setting('WASM_BIGINT')
    self.set_setting('EXPORTED_FUNCTIONS', '_malloc,_main')
    self.emcc_args += ['-Wno-experimental', '-pthread']
    self.do_runf('core/test_em_js.cpp')

  @node_pthreads
  @parameterized({
    '': (False,),
    'flipped': (True,),
  })
  def test_dylink_pthread_comdat(self, flipped):
    # Test that the comdat info for `Foo`, which is defined in the side module,
    # is visible to the main module.
    create_file('foo.h', r'''
    struct Foo {
      Foo() {
        method();
      }
      // Making this method virtual causes the comdat group for the
      // class to only be defined in the side module.
      virtual void method() const;
    };
    ''')
    create_file('main.cpp', r'''
      #include "foo.h"
      #include <typeinfo>
      #include <emscripten/console.h>

      // Foo constructor calls a virtual function, with the vtable defined
      // in the side module. This verifies that the side module's data
      // reloctions are applied before calling static constructors in the
      // main module.
      Foo g_foo;

      int main() {
        emscripten_outf("main: Foo typeid: %s", typeid(Foo).name());

        Foo().method();
        return 0;
      }
    ''')
    create_file('side.cpp', r'''
      #include "foo.h"
      #include <typeinfo>
      #include <emscripten/console.h>

      void Foo::method() const {
        emscripten_outf("side: Foo typeid: %s", typeid(Foo).name());
      }
      ''')
    if flipped:
      side = 'main.cpp'
      main = 'side.cpp'
    else:
      self.skipTest('https://reviews.llvm.org/D128515')
      side = 'side.cpp'
      main = 'main.cpp'
    self.run_process([
      EMCC,
      '-o', 'libside.wasm',
      side,
      '-pthread', '-Wno-experimental',
      '-sSIDE_MODULE'])
    self.do_runf(
      main,
      'main: Foo typeid: 3Foo\nside: Foo typeid: 3Foo\n',
      emcc_args=[
        '-pthread', '-Wno-experimental',
        '-sPROXY_TO_PTHREAD',
        '-sEXIT_RUNTIME',
        '-sMAIN_MODULE=2',
        'libside.wasm',
      ])

  def test_dylink_no_autoload(self):
    create_file('main.c', r'''
      #include <stdio.h>
      int sidey();
      int main() {
        printf("sidey: %d\n", sidey());
        return 0;
      }''')
    create_file('side.c', 'int sidey() { return 42; }')
    self.run_process([EMCC, '-sSIDE_MODULE', 'side.c', '-o', 'libside.wasm'])

    # First show everything working as expected with AUTOLOAD_DYLIBS
    self.run_process([EMCC, '-sMAIN_MODULE=2', 'main.c', 'libside.wasm'])
    output = self.run_js('a.out.js')
    self.assertContained('sidey: 42\n', output)

    # Same again but with NO_AUTOLOAD_DYLIBS.   This time we expect the call to sidey
    # to fail at runtime.
    self.run_process([EMCC, '-sMAIN_MODULE=2', 'main.c', 'libside.wasm', '-sNO_AUTOLOAD_DYLIBS'])
    output = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained("external symbol 'sidey' is missing. perhaps a side module was not linked in?", output)

    # Now with NO_AUTOLOAD_DYLIBS, but with manual loading of libside.wasm using loadDynamicLibrary
    create_file('pre.js', '''
    Module.preRun = () => loadDynamicLibrary('libside.wasm');
    ''')
    self.run_process([EMCC, '-sMAIN_MODULE=2', 'main.c', 'libside.wasm', '-sNO_AUTOLOAD_DYLIBS', '--pre-js=pre.js'])
    output = self.run_js('a.out.js')
    self.assertContained('sidey: 42\n', output)

  def test_dylink_dependencies(self):
    create_file('side1.c', r'''
    #include <stdio.h>
    #include <stdlib.h>

    void side2();

    void side1() {
        printf("side1\n");
        side2();
    }
    ''')
    create_file('side2.c', r'''
    #include <stdio.h>
    #include <stdlib.h>

    void side2() {
        printf("side2\n");
    }
    ''')
    create_file('main.c', '''
    void side1();

    int main() {
        side1();
        return 0;
    }
    ''')
    self.emcc('side2.c', ['-fPIC', '-sSIDE_MODULE', '-olibside2.so'])
    self.emcc('side1.c', ['-fPIC', '-sSIDE_MODULE', '-olibside1.so', 'libside2.so'])
    cmd = [EMCC, 'main.c', '-fPIC', '-sMAIN_MODULE=2', 'libside1.so']

    # Unless `.` is added to the library path the libside2.so won't be found.
    err = self.expect_fail(cmd)
    self.assertContained('emcc: error: libside1.so: shared library dependency not found in library path: `libside2.so`.', err)

    # Adding -L. to the library path makes it work.
    self.run_process(cmd + ['-L.'])
    self.run_js('a.out.js')

  def test_dylink_LEGACY_GL_EMULATION(self):
    # LEGACY_GL_EMULATION wraps JS library functions. This test ensure that when it does
    # so it preserves the `.sig` attributes needed by dynamic linking.
    create_file('test.c', r'''
#include <GLES2/gl2.h>
#include <stdio.h>

int main() {
  printf("glUseProgram: %p\n", &glUseProgram);
  printf("done\n");
  return 0;
}''')
    self.do_runf('test.c', 'done\n', emcc_args=['-sLEGACY_GL_EMULATION', '-sMAIN_MODULE=2'])

  def test_js_link(self):
    create_file('before.js', '''
      var MESSAGE = 'hello from js';
      // Module is initialized with empty object by default, so if there are no keys - nothing was run yet
      if (Object.keys(Module).length) throw 'This code should run before anything else!';
    ''')
    create_file('after.js', '''
      out(MESSAGE);
    ''')

    self.do_runf(test_file('hello_world.c'), 'hello, world!\nhello from js\n',
                 emcc_args=['--pre-js', 'before.js', '--post-js', 'after.js', '-sWASM_ASYNC_COMPILATION=0'])

  def test_sdl_none(self):
    create_file('main.c', r'''
      #include <stdio.h>
      #include <SDL.h>

      int main() {
        return 0;
      }
    ''')
    err = self.expect_fail([EMCC, 'main.c'])
    self.assertContained('SDL.h:1:2: error: "To use the emscripten port of SDL use -sUSE_SDL or -sUSE_SDL=2"', err)
    self.run_process([EMCC, 'main.c', '-sUSE_SDL'])

  def test_sdl_endianness(self):
    create_file('main.c', r'''
      #include <stdio.h>
      #include <SDL/SDL.h>

      int main() {
        printf("%d, %d, %d\n", SDL_BYTEORDER, SDL_LIL_ENDIAN, SDL_BIG_ENDIAN);
        return 0;
      }
    ''')
    self.do_runf('main.c', '1234, 1234, 4321\n')

  def test_sdl_scan_code_from_key(self):
    create_file('main.c', r'''
      #include <stdio.h>
      #include <SDL/SDL_keyboard.h>

      int main() {
        printf("%d\n", SDL_GetScancodeFromKey(35));
        return 0;
      }
    ''')
    self.do_runf('main.c', '204\n')

  @requires_network
  def test_sdl2_mixer_wav(self):
    self.emcc(test_file('browser/test_sdl2_mixer_wav.c'), ['-sUSE_SDL_MIXER=2'], output_filename='a.out.js')
    self.emcc(test_file('browser/test_sdl2_mixer_wav.c'), ['--use-port=sdl2_mixer'], output_filename='a.out.js')

  def test_sdl2_linkable(self):
    # Ensure that SDL2 can be built with LINKABLE.  This implies there are no undefined
    # symbols in the library (because LINKABLE includes the entire library).
    self.emcc(test_file('browser/test_sdl2_misc.c'), ['-sLINKABLE', '-sUSE_SDL=2'], output_filename='a.out.js')
    self.emcc(test_file('browser/test_sdl2_misc.c'), ['-sLINKABLE', '--use-port=sdl2'], output_filename='a.out.js')

  @requires_network
  def test_sdl2_gfx_linkable(self):
    # Same as above but for sdl2_gfx library
    self.emcc(test_file('browser/test_sdl2_misc.c'), ['-Wl,-fatal-warnings', '-sLINKABLE', '-sUSE_SDL_GFX=2'], output_filename='a.out.js')
    self.emcc(test_file('browser/test_sdl2_misc.c'), ['-Wl,-fatal-warnings', '-sLINKABLE', '--use-port=sdl2_gfx'], output_filename='a.out.js')

  @requires_network
  def test_libpng(self):
    shutil.copy(test_file('third_party/libpng/pngtest.png'), '.')
    self.do_runf('third_party/libpng/pngtest.c', 'libpng passes test',
                 emcc_args=['--embed-file', 'pngtest.png', '-sUSE_LIBPNG'])
    self.do_runf('third_party/libpng/pngtest.c', 'libpng passes test',
                 emcc_args=['--embed-file', 'pngtest.png', '--use-port=libpng'])

  @node_pthreads
  @requires_network
  def test_libpng_with_pthreads(self):
    shutil.copy(test_file('third_party/libpng/pngtest.png'), '.')
    self.do_runf('third_party/libpng/pngtest.c', 'libpng passes test',
                 emcc_args=['--embed-file', 'pngtest.png', '-sUSE_LIBPNG', '-pthread'])

  @requires_network
  def test_giflib(self):
    # giftext.c contains a sprintf warning
    self.emcc_args += ['-Wno-fortify-source']
    shutil.copy(test_file('third_party/giflib/treescap.gif'), '.')
    self.do_runf('third_party/giflib/giftext.c',
                 'GIF file terminated normally',
                 emcc_args=['--embed-file', 'treescap.gif', '-sUSE_GIFLIB'],
                 args=['treescap.gif'])
    # Same again with -sMAIN_MODULE (See #18537)
    self.do_runf('third_party/giflib/giftext.c',
                 'GIF file terminated normally',
                 emcc_args=['--embed-file', 'treescap.gif', '-sUSE_GIFLIB', '-sMAIN_MODULE'],
                 args=['treescap.gif'])
    self.do_runf('third_party/giflib/giftext.c',
                 'GIF file terminated normally',
                 emcc_args=['--embed-file', 'treescap.gif', '--use-port=giflib'],
                 args=['treescap.gif'])

  @requires_network
  def test_libjpeg(self):
    shutil.copy(test_file('screenshot.jpg'), '.')
    self.do_runf('jpeg_test.c', 'Image is 600 by 450 with 3 components',
                 emcc_args=['--embed-file', 'screenshot.jpg', '-sUSE_LIBJPEG'],
                 args=['screenshot.jpg'])
    self.do_runf('jpeg_test.c', 'Image is 600 by 450 with 3 components',
                 emcc_args=['--embed-file', 'screenshot.jpg', '--use-port=libjpeg'],
                 args=['screenshot.jpg'])

  @requires_network
  def test_bullet(self):
    self.do_runf('bullet_hello_world.cpp', 'BULLET RUNNING', emcc_args=['-sUSE_BULLET'])
    self.do_runf('bullet_hello_world.cpp', 'BULLET RUNNING', emcc_args=['--use-port=bullet'])

  @requires_network
  def test_vorbis(self):
    # This will also test if ogg compiles, because vorbis depends on ogg
    self.do_runf('vorbis_test.c', 'ALL OK', emcc_args=['-sUSE_VORBIS'])
    self.do_runf('vorbis_test.c', 'ALL OK', emcc_args=['--use-port=vorbis'])

  @requires_network
  def test_bzip2(self):
    self.do_runf('bzip2_test.c', 'usage: unzcrash filename',
                 emcc_args=['-sUSE_BZIP2', '-Wno-pointer-sign'])
    self.do_runf('bzip2_test.c', 'usage: unzcrash filename',
                 emcc_args=['--use-port=bzip2', '-Wno-pointer-sign'])

  @with_all_sjlj
  @requires_network
  @crossplatform
  def test_freetype(self):
    # copy the Liberation Sans Bold truetype file located in the
    # <emscripten_root>/test/freetype to the compilation folder
    shutil.copy2(test_file('freetype/LiberationSansBold.ttf'), os.getcwd())
    self.emcc_args += ['--embed-file', 'LiberationSansBold.ttf']
    # the test program will print an ascii representation of a bitmap where the
    # 'w' character has been rendered using the Liberation Sans Bold font.
    # See test_freetype.out
    self.do_run_in_out_file_test('test_freetype.c', emcc_args=['-sUSE_FREETYPE'])
    self.do_run_in_out_file_test('test_freetype.c', emcc_args=['--use-port=freetype'])

  @requires_network
  def test_freetype_with_pthreads(self):
    # Verify that freetype supports compilation requiring pthreads
    self.emcc(test_file('test_freetype.c'), ['-pthread', '-sUSE_FREETYPE'], output_filename='a.out.js')

  @requires_network
  def test_icu(self):
    self.set_setting('USE_ICU')
    self.do_runf('other/test_icu.cpp')

  @requires_network
  def test_sdl2_ttf(self):
    # This is a compile-only to test to verify that sdl2-ttf (and freetype and harfbuzz) are buildable.
    self.emcc(test_file('browser/test_sdl2_ttf.c'), args=['-sUSE_SDL=2', '-sUSE_SDL_TTF=2'], output_filename='a.out.js')
    self.emcc(test_file('browser/test_sdl2_ttf.c'), args=['--use-port=sdl2', '--use-port=sdl2_ttf'], output_filename='a.out.js')

  @requires_network
  def test_contrib_ports(self):
    # Verify that contrib ports can be used (using the only contrib port available ATM, but can be replaced
    # with a different contrib port when there is another one
    self.emcc(test_file('other/test_contrib_ports.cpp'), ['--use-port=contrib.glfw3'])

  @crossplatform
  def test_external_ports_simple(self):
    if config.FROZEN_CACHE:
      self.skipTest("test doesn't work with frozen cache")
    simple_port_path = test_file("other/ports/simple.py")
    self.do_runf('other/test_external_ports_simple.c', emcc_args=[f'--use-port={simple_port_path}'])

  @crossplatform
  @requires_network
  def test_external_ports(self):
    if config.FROZEN_CACHE:
      self.skipTest("test doesn't work with frozen cache")
    external_port_path = test_file("other/ports/external.py")
    # testing no option
    self.do_runf('other/test_external_ports.c', 'value1=0&value2=0&value3=v3\n', emcc_args=[f'--use-port={external_port_path}'])
    # testing 1 option
    self.do_runf('other/test_external_ports.c', 'value1=12&value2=0&value3=v3\n', emcc_args=[f'--use-port={external_port_path}:value1=12'])
    # testing 2 options
    self.do_runf('other/test_external_ports.c', 'value1=12&value2=36&value3=v3\n', emcc_args=[f'--use-port={external_port_path}:value1=12:value2=36'])
    # testing ':' escape
    self.do_runf('other/test_external_ports.c', 'value1=12&value2=36&value3=v:3\n', emcc_args=[f'--use-port={external_port_path}:value1=12:value3=v::3:value2=36'])
    # testing dependency
    self.do_runf('other/test_external_ports.c', 'mpg123=45\n', emcc_args=[f'--use-port={external_port_path}:dependency=mpg123'])
    # testing invalid dependency
    stderr = self.expect_fail([EMCC, test_file('other/test_external_ports.c'), f'--use-port={external_port_path}:dependency=invalid', '-o', 'a4.out.js'])
    self.assertFalse(os.path.exists('a4.out.js'))
    self.assertContained('unknown dependency `invalid` for port `external`', stderr)
    # testing help
    stdout = self.run_process([EMCC, test_file('other/test_external_ports.c'), f'--use-port={external_port_path}:help'], stdout=PIPE).stdout
    self.assertContained('''external (--use-port=external; Test License)
Test Description
Options:
* value1: Value for define TEST_VALUE_1
* value2: Value for define TEST_VALUE_2
* value3: String value
* dependency: A dependency
More info: https://emscripten.org
''', stdout)

  def test_link_memcpy(self):
    # memcpy can show up *after* optimizations, so after our opportunity to link in libc, so it must be special-cased
    create_file('main.c', r'''
      #include <stdio.h>

      int main(int argc, char **argv) {
        int num = argc + 10;
        char buf[num], buf2[num];
        for (int i = 0; i < num; i++) {
          buf[i] = i*i+i/3;
        }
        for (int i = 1; i < num; i++) {
          buf[i] += buf[i-1];
        }
        for (int i = 0; i < num; i++) {
          buf2[i] = buf[i];
        }
        for (int i = 1; i < num; i++) {
          buf2[i] += buf2[i-1];
        }
        for (int i = 0; i < num; i++) {
          printf("%d:%d\n", i, buf2[i]);
        }
        return 0;
      }
    ''')
    self.run_process([EMCC, '-O2', 'main.c'])
    output = self.run_js('a.out.js')
    self.assertContained('''0:0
1:1
2:6
3:21
4:53
5:111
6:-49
7:98
8:55
9:96
10:-16
''', output)
    self.assertNotContained('warning: library.js memcpy should not be running, it is only for testing!', output)

  @parameterized({
    '': ('out.js',),
    'standalone': ('out.wasm',)
  })
  def test_undefined_exported_function(self, outfile):
    cmd = [EMCC, test_file('hello_world.c'), '-o', outfile]
    self.run_process(cmd)

    # Adding a missing symbol to EXPORTED_FUNCTIONS should cause a link failure
    cmd += ['-sEXPORTED_FUNCTIONS=_foobar']
    err = self.expect_fail(cmd)
    self.assertContained('wasm-ld: error: symbol exported via --export not found: foobar', err)

    # Adding -sERROR_ON_UNDEFINED_SYMBOLS=0 means the error gets reported later
    # by emscripten.py.
    cmd += ['-sERROR_ON_UNDEFINED_SYMBOLS=0']
    err = self.expect_fail(cmd)
    self.assertContained('undefined exported symbol: "_foobar"', err)

    # setting `-Wno-undefined` should suppress the error
    cmd += ['-Wno-undefined']
    self.run_process(cmd)

  @parameterized({
    '': ('out.js',),
    'standalone': ('out.wasm',)
  })
  def test_undefined_exported_js_function(self, outfile):
    cmd = [EMXX, test_file('hello_world.cpp'), '-o', outfile]
    self.run_process(cmd)

    # adding a missing symbol to EXPORTED_FUNCTIONS should cause failure
    cmd += ['-sEXPORTED_FUNCTIONS=foobar']
    err = self.expect_fail(cmd)
    self.assertContained('undefined exported symbol: "foobar"', err)

    # setting `-Wno-undefined` should suppress the error
    cmd += ['-Wno-undefined']
    self.run_process(cmd)

  @parameterized({
    '': [[]],
    'O1': [['-O1']],
    'GL2': [['-sMAX_WEBGL_VERSION=2']],
  })
  @parameterized({
    'warn': ['WARN'],
    'error': ['ERROR'],
    'ignore': [None]
  })
  def test_undefined_symbols(self, args, action):
    create_file('main.c', r'''
      #include <stdio.h>
      #include <SDL.h>
      #include "SDL/SDL_opengl.h"

      void something();
      void elsey();

      int main() {
        // pull in gl proc stuff, avoid warnings on emulation funcs
        printf("%p", SDL_GL_GetProcAddress("glGenTextures"));
        something();
        elsey();
        return 0;
      }
      ''')

    for value in ([0, 1]):
      delete_file('a.out.js')
      print('checking %s' % value)
      extra = ['-s', action + '_ON_UNDEFINED_SYMBOLS=%d' % value] if action else []
      proc = self.run_process([EMCC, '-sUSE_SDL', '-sGL_ENABLE_GET_PROC_ADDRESS', 'main.c'] + extra + args, stderr=PIPE, check=False)
      if common.EMTEST_VERBOSE:
        print(proc.stderr)
      if value or action is None:
        # The default is that we error in undefined symbols
        self.assertContained('undefined symbol: something', proc.stderr)
        self.assertContained('undefined symbol: elsey', proc.stderr)
        check_success = False
      elif action == 'ERROR' and not value:
        # Error disables, should only warn
        self.assertContained('warning: undefined symbol: something', proc.stderr)
        self.assertContained('warning: undefined symbol: elsey', proc.stderr)
        self.assertNotContained('undefined symbol: emscripten_', proc.stderr)
        check_success = True
      elif action == 'WARN' and not value:
        # Disabled warning should imply disabling errors
        self.assertNotContained('undefined symbol', proc.stderr)
        check_success = True

      if check_success:
        self.assertEqual(proc.returncode, 0)
        self.assertTrue(os.path.exists('a.out.js'))
      else:
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(os.path.exists('a.out.js'))

  def test_undefined_data_symbols(self):
    create_file('main.c', r'''
    extern int foo;

    int main() {
      return foo;
    }
    ''')
    output = self.expect_fail([EMCC, 'main.c'])
    self.assertContained('undefined symbol: foo', output)

    # With -Wl,--unresolved-symbols=ignore-all or -Wl,--allow-undefined
    # the linker should ignore any undefined data symbols.
    self.run_process([EMCC, 'main.c', '-Wl,--unresolved-symbols=ignore-all'])
    self.run_process([EMCC, 'main.c', '-Wl,--allow-undefined'])

  def test_GetProcAddress_LEGACY_GL_EMULATION(self):
    # without legacy gl emulation, getting a proc from there should fail
    self.do_other_test('test_GetProcAddress_LEGACY_GL_EMULATION.c', args=['0'], emcc_args=['-sLEGACY_GL_EMULATION=0', '-sGL_ENABLE_GET_PROC_ADDRESS'])
    # with it, it should work
    self.do_other_test('test_GetProcAddress_LEGACY_GL_EMULATION.c', args=['1'], emcc_args=['-sLEGACY_GL_EMULATION', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  # Verifies that is user is building without -sGL_ENABLE_GET_PROC_ADDRESS, then
  # at link time they should get a helpful error message guiding them to enable
  # the option.
  def test_get_proc_address_error_message(self):
    err = self.expect_fail([EMCC, '-sGL_ENABLE_GET_PROC_ADDRESS=0', test_file('other/test_GetProcAddress_LEGACY_GL_EMULATION.c')])
    self.assertContained('error: linker: Undefined symbol: SDL_GL_GetProcAddress(). Please pass -sGL_ENABLE_GET_PROC_ADDRESS at link time to link in SDL_GL_GetProcAddress().', err)

  @parameterized({
    '': (False, False),
    'no_initial_run': (True, False),
    'run_dep': (False, True),
  })
  def test_prepost(self, no_initial_run, run_dep):
    create_file('pre.js', '''
      var Module = {
        preRun: () => out('pre-run'),
        postRun: () => out('post-run')
      };
      ''')

    self.run_process([EMCC, test_file('hello_world.c'), '--pre-js', 'pre.js', '-sWASM_ASYNC_COMPILATION=0'])
    self.assertContained('pre-run\nhello, world!\npost-run\n', self.run_js('a.out.js'))

    # addRunDependency during preRun should prevent main, and post-run from
    # running.
    with open('pre.js', 'a') as f:
      f.write('Module.preRun = () => { out("add-dep"); addRunDependency(); }\n')
    self.run_process([EMCC, test_file('hello_world.c'), '--pre-js', 'pre.js', '-sWASM_ASYNC_COMPILATION=0'])
    output = self.run_js('a.out.js')
    self.assertContained('add-dep\n', output)
    self.assertNotContained('hello, world!\n', output)
    self.assertNotContained('post-run\n', output)

    # noInitialRun prevents run
    args = ['-sWASM_ASYNC_COMPILATION=0', '-sEXPORTED_RUNTIME_METHODS=callMain']
    if no_initial_run:
      args += ['-sINVOKE_RUN=0']
    if run_dep:
      create_file('pre.js', 'Module.preRun = () => addRunDependency("test");')
      create_file('post.js', 'removeRunDependency("test");')
      args += ['--pre-js', 'pre.js', '--post-js', 'post.js']

    self.run_process([EMCC, test_file('hello_world.c')] + args)
    output = self.run_js('a.out.js')
    self.assertContainedIf('hello, world!', output, not no_initial_run)

    if no_initial_run:
      # Calling main later should still work, filesystem etc. must be set up.
      print('call main later')
      src = read_file('a.out.js')
      src += '\nout("callMain -> " + Module.callMain());\n'
      create_file('a.out.js', src)
      self.assertContained('hello, world!\ncallMain -> 0\n', self.run_js('a.out.js'))

  def test_preinit(self):
    create_file('pre.js', '''
      var Module = {
        preRun: () => out('pre-run'),
        postRun: () => out('post-run'),
        preInit: () => out('pre-init')
      };
    ''')
    self.do_runf(test_file('hello_world.c'),
                 'pre-init\npre-run\nhello, world!\npost-run\n',
                 emcc_args=['--pre-js', 'pre.js'])

  def test_prepost2(self):
    create_file('pre.js', 'Module.preRun = () => out("pre-run");')
    create_file('pre2.js', 'Module.postRun = () => out("post-run");')
    self.do_runf(test_file('hello_world.c'), 'pre-run\nhello, world!\npost-run\n',
                 emcc_args=['--pre-js', 'pre.js', '--pre-js', 'pre2.js'])

  def test_prepre(self):
    create_file('pre.js', '''
      Module.preRun = [() => out('pre-run')];
    ''')
    create_file('pre2.js', '''
      Module.preRun.push(() => out('prepre'));
    ''')
    self.do_runf(test_file('hello_world.c'), 'prepre\npre-run\nhello, world!\n',
                 emcc_args=['--pre-js', 'pre.js', '--pre-js', 'pre2.js'])

  def test_extern_prepost(self):
    create_file('extern-pre.js', '// I am an external pre.\n')
    create_file('extern-post.js', '// I am an external post.\n')
    self.run_process([EMCC, '-O2', test_file('hello_world.c'), '--extern-pre-js', 'extern-pre.js', '--extern-post-js', 'extern-post.js', '--closure=1'])
    # the files should be included, and externally - not as part of optimized
    # code, so they are the very first and last things, and they are not
    # minified.
    js = read_file('a.out.js')
    pre = js.index('// I am an external pre.')
    post = js.index('// I am an external post.')
    # ignore some slack - newlines and other things. we just care about the
    # big picture here
    SLACK = 50
    self.assertLess(pre, post)
    self.assertLess(pre, SLACK)
    self.assertGreater(post, len(js) - SLACK)
    # make sure the slack is tiny compared to the whole program
    self.assertGreater(len(js), 100 * SLACK)

  @parameterized({
    'minifyGlobals': (['minifyGlobals'],),
    'minifyLocals': (['minifyLocals'],),
    'JSDCE': (['JSDCE'],),
    'JSDCE-hasOwnProperty': (['JSDCE'],),
    'JSDCE-defaultArg': (['JSDCE'],),
    'JSDCE-fors': (['JSDCE'],),
    'JSDCE-objectPattern': (['JSDCE'],),
    'AJSDCE': (['AJSDCE'],),
    'emitDCEGraph': (['emitDCEGraph', '--no-print'],),
    'emitDCEGraph-closure': (['emitDCEGraph', '--no-print', '--closure-friendly'], 'emitDCEGraph.js'),
    'emitDCEGraph2': (['emitDCEGraph', '--no-print'],),
    'emitDCEGraph3': (['emitDCEGraph', '--no-print'],),
    'emitDCEGraph4': (['emitDCEGraph', '--no-print'],),
    'emitDCEGraph5': (['emitDCEGraph', '--no-print'],),
    'minimal-runtime-applyDCEGraphRemovals': (['applyDCEGraphRemovals'],),
    'applyDCEGraphRemovals': (['applyDCEGraphRemovals'],),
    'applyImportAndExportNameChanges': (['applyImportAndExportNameChanges'],),
    'applyImportAndExportNameChanges2': (['applyImportAndExportNameChanges'],),
    'minimal-runtime-emitDCEGraph': (['emitDCEGraph', '--no-print'],),
    'minimal-runtime-2-emitDCEGraph': (['emitDCEGraph', '--no-print'],),
    'standalone-emitDCEGraph': (['emitDCEGraph', '--no-print'],),
    'emittedJSPreservesParens': ([],),
    'growableHeap': (['growableHeap'],),
    'unsignPointers': (['unsignPointers', '--closure-friendly'],),
    'asanify': (['asanify'],),
    'safeHeap': (['safeHeap'],),
    'object-literals': ([],),
    'LittleEndianHeap': (['littleEndianHeap'],),
  })
  @crossplatform
  def test_js_optimizer(self, passes, filename=None):
    if not filename:
      testname = self.id().split('.')[-1]
      filename = utils.removeprefix(testname, 'test_js_optimizer_') + '.js'
    filename = test_file('js_optimizer', filename)
    expected_file = shared.unsuffixed(filename) + '-output.js'
    # test calling optimizer
    js = self.run_process(config.NODE_JS + [path_from_root('tools/acorn-optimizer.mjs'), filename] + passes, stdin=PIPE, stdout=PIPE).stdout
    if common.EMTEST_REBASELINE:
      write_file(expected_file, js)
    else:
      self.assertFileContents(expected_file, js)

  def test_js_optimizer_huge(self):
    # Stress test the chunkifying code in js_optimizer.py
    lines = ['// EMSCRIPTEN_START_FUNCS']
    for i in range(1000_000):
      lines.append('function v%d()\n {\n var someLongNameToMakeThisLineLong = %d\n }' % (i, i))
    lines.append('// EMSCRIPTEN_END_FUNCS\n')
    create_file('huge.js', '\n'.join(lines))
    self.assertGreater(os.path.getsize('huge.js'), 50_000_000)
    self.run_process([PYTHON, path_from_root('tools/js_optimizer.py'), 'huge.js', '--minify-whitespace'])

  @parameterized({
    'wasm2js': ('wasm2js', ['minifyNames']),
    'constructor': ('constructor', ['minifyNames'])
  })
  @crossplatform
  def test_js_optimizer_py(self, name, passes):
    # run the js optimizer python script. this differs from test_js_optimizer
    # which runs the internal js optimizer JS script directly (which the python
    # script calls)
    shutil.copy(test_file('js_optimizer', name + '.js'), '.')
    self.run_process([PYTHON, path_from_root('tools/js_optimizer.py'), name + '.js'] + passes)
    actual = read_file(name + '.js.jsopt.js')
    self.assertFileContents(test_file('js_optimizer', name + '-output.js'), actual)

  def test_m_mm(self):
    create_file('foo.c', '#include <emscripten.h>')
    for opt in ('M', 'MM'):
      proc = self.run_process([EMCC, 'foo.c', '-' + opt], stdout=PIPE, stderr=PIPE)
      self.assertContained('foo.o: ', proc.stdout)
      self.assertNotContained('error', proc.stderr)

  @uses_canonical_tmp
  def test_emcc_debug_files(self):
    for opts in (0, 1, 2, 3):
      for debug in (None, '1', '2'):
        print(opts, debug)
        if os.path.exists(self.canonical_temp_dir):
          shutil.rmtree(self.canonical_temp_dir)

        with env_modify({'EMCC_DEBUG': debug}):
          self.run_process([EMCC, test_file('hello_world.c'), '-O' + str(opts)], stderr=PIPE)
          if debug is None:
            self.assertFalse(os.path.exists(self.canonical_temp_dir))
          else:
            print(sorted(os.listdir(self.canonical_temp_dir)))
            self.assertExists(os.path.join(self.canonical_temp_dir, 'emcc-03-original.js'))

  def test_debuginfo_line_tables_only(self):
    def test(do_compile):
      do_compile([])
      no_size = os.path.getsize('a.out.wasm')
      do_compile(['-gline-tables-only'])
      line_size = os.path.getsize('a.out.wasm')
      do_compile(['-g'])
      full_size = os.path.getsize('a.out.wasm')
      return (no_size, line_size, full_size)

    def compile_to_object(compile_args):
      self.run_process([EMCC, test_file('hello_world.c'), '-c', '-o', 'a.out.wasm'] + compile_args)

    no_size, line_size, full_size = test(compile_to_object)
    self.assertLess(no_size, line_size)
    self.assertLess(line_size, full_size)

    def compile_to_executable(compile_args, link_args):
      # compile with the specified args
      self.run_process([EMCC, test_file('hello_world.c'), '-c', '-o', 'a.o'] + compile_args)
      # link with debug info
      self.run_process([EMCC, 'a.o'] + link_args)

    def compile_to_debug_executable(compile_args):
      return compile_to_executable(compile_args, ['-g'])

    no_size, line_size, full_size = test(compile_to_debug_executable)
    self.assertLess(no_size, line_size)
    self.assertLess(line_size, full_size)

    def compile_to_release_executable(compile_args):
      return compile_to_executable(compile_args, ['-O1'])

    no_size, line_size, full_size = test(compile_to_release_executable)
    self.assertEqual(no_size, line_size)
    self.assertEqual(line_size, full_size)

    # "-O0 executable" means compiling without optimizations but *also* without
    # -g (so, not a true debug build). the results here may change over time,
    # since we are telling emcc both to try to do as little as possible during
    # link (-O0), but also that debug info is not needed (no -g). if we end up
    # doing post-link changes then we will strip the debug info, but if not then
    # we don't.
    def compile_to_O0_executable(compile_args):
      return compile_to_executable(compile_args, [])

    no_size, line_size, full_size = test(compile_to_O0_executable)
    self.assertEqual(no_size, line_size)
    self.assertEqual(line_size, full_size)

  # Verify the existence (or lack thereof) of DWARF info in the given wasm file
  def verify_dwarf(self, wasm_file, verify_func):
    self.assertExists(wasm_file)
    info = self.run_process([LLVM_DWARFDUMP, '--all', wasm_file], stdout=PIPE).stdout
    verify_func('DW_TAG_subprogram', info) # Subprogram entry in .debug_info
    verify_func('debug_line[0x', info) # Line table

  def verify_dwarf_exists(self, wasm_file):
    self.verify_dwarf(wasm_file, self.assertIn)

  def verify_dwarf_does_not_exist(self, wasm_file):
    self.verify_dwarf(wasm_file, self.assertNotIn)

  # Verify if the given file name contains a source map
  def verify_source_map_exists(self, map_file):
    self.assertExists(map_file)
    data = json.load(open(map_file))
    # Simply check the existence of required sections
    self.assertIn('version', data)
    self.assertIn('sources', data)
    self.assertIn('mappings', data)

  def verify_custom_sec_existence(self, wasm_file, section_name, expect_existence):
    with webassembly.Module(wasm_file) as module:
      section = module.get_custom_section(section_name)
      if expect_existence:
        self.assertIsNotNone(section, f'section {section_name} unexpectedly missing')
      else:
        self.assertIsNone(section, f'section {section_name} unexpectedly found')

  def test_dwarf(self):
    def compile_with_dwarf(args, output):
      # Test that -g enables dwarf info in object files and linked wasm
      self.run_process([EMCC, test_file('hello_world.c'), '-o', output, '-g'] + args)
    compile_with_dwarf(['-c'], 'a.o')
    self.verify_dwarf_exists('a.o')
    compile_with_dwarf([], 'a.js')
    self.verify_dwarf_exists('a.wasm')

  def test_dwarf_sourcemap_names(self):
    source_file = 'hello_world.c'
    js_file = 'a.out.js'
    wasm_file = 'a.out.wasm'
    map_file = 'a.out.wasm.map'

    for (flags, expect_dwarf, expect_sourcemap, expect_names) in [
      ([], False, False, False),
      (['-g0'], False, False, False),
      (['-g1'], False, False, False),
      (['-g1', '-O2'], False, False, False),
      (['-O2'], False, False, False),
      (['--minify=0'], False, False, False),
      # last numeric g flag "wins", so g0 overrides -g
      (['-g', '-g0'], False, False, False),
      (['-g2'], False, False, True),
      (['-gline-tables-only'], True, False, True),
      (['--profiling'], False, False, True),
      (['--profiling-funcs'], False, False, True),
      (['-O2', '--profiling-funcs'], False, False, True),
      (['-g'], True, False, True),
      (['-g3'], True, False, True),
      (['-O1', '-g'], True, False, True),
      (['-O3', '-g'], True, False, True),
      (['-gsplit-dwarf'], True, False, True),
      # TODO: It seems odd that -gsource-map leaves behind a name section. Should it?
      (['-gsource-map'], False, True, True),
      (['-g1', '-Oz', '-gsource-map'], False, True, True),
      # -g0 does not override -gsource-map but does remove name section. TODO: should it?
      (['-gsource-map', '-g0'], False, True, False),
      # --emit-symbol-map should not affect the results
      (['--emit-symbol-map', '-gsource-map'], False, True, True),
      (['--emit-symbol-map'], False, False, False),
      (['--emit-symbol-map', '-Oz'], False, False, False),
      (['-sASYNCIFY=1', '-g0'], False, False, False),
      (['-sASYNCIFY=1', '-gsource-map'], False, True, True),
      (['-g', '-gsource-map'], True, True, True),
      (['-g2', '-gsource-map'], False, True, True),
      # (['-gsplit-dwarf', '-gsource-map'], True, True, True), TODO this currently fails!
      (['-gsource-map', '-sWASM_BIGINT', '-sERROR_ON_WASM_CHANGES_AFTER_LINK'], False, True, True),
    ]:
      print(flags, expect_dwarf, expect_sourcemap, expect_names)
      self.emcc(test_file(source_file), flags, js_file)
      self.assertExists(js_file)
      assertion = self.assertIn if expect_dwarf else self.assertNotIn
      self.verify_dwarf(wasm_file, assertion)

      self.verify_custom_sec_existence(wasm_file, 'sourceMappingURL', expect_sourcemap)
      if expect_sourcemap:
        self.verify_source_map_exists(map_file)
      else:
        self.assertFalse(os.path.isfile(map_file), 'Sourcemap unexpectedly exists')

      self.verify_custom_sec_existence(wasm_file, 'name', expect_names)

      self.clear()

  @requires_scons
  @with_env_modify({'EMSCRIPTEN_ROOT': path_from_root()})
  def test_scons(self):
    # this test copies the site_scons directory alongside the test
    shutil.copytree(test_file('scons/simple'), 'test')
    shutil.copytree(path_from_root('tools/scons/site_scons'), Path('test/site_scons'))
    with utils.chdir('test'):
      self.run_process(['scons'])
      output = self.run_js('scons_integration.js', assert_returncode=5)
    self.assertContained('If you see this - the world is all right!', output)

  @requires_scons
  @with_env_modify({
    'EMSCRIPTEN_ROOT': path_from_root(),
    'EMSCONS_PKG_CONFIG_LIBDIR': '/pkg/config/libdir',
    'EMSCONS_PKG_CONFIG_PATH': '/pkg/config/path',
  })
  def test_scons_env(self):
    # this test copies the site_scons directory alongside the test
    shutil.copytree(test_file('scons/env'), 'test')
    shutil.copytree(path_from_root('tools/scons/site_scons'), Path('test/site_scons'))

    expected_to_propagate = json.dumps({
      'CC': path_from_root('emcc'),
      'CXX': path_from_root('em++'),
      'AR': path_from_root('emar'),
      'RANLIB': path_from_root('emranlib'),
      'ENV': {
        'PKG_CONFIG_LIBDIR': '/pkg/config/libdir',
        'PKG_CONFIG_PATH': '/pkg/config/path',
      }
    })

    with utils.chdir('test'):
      self.run_process(['scons', '--expected-env', expected_to_propagate])

  @requires_scons
  def test_scons_env_no_emscons(self):
    shutil.copytree(test_file('scons/env'), 'test')
    shutil.copytree(path_from_root('tools/scons/site_scons'), Path('test/site_scons'))

    expected_to_propagate = json.dumps({
      'CC': 'emcc',
      'CXX': 'em++',
      'AR': 'emar',
      'RANLIB': 'emranlib',
      'ENV': {
        'PKG_CONFIG_LIBDIR': None,
        'PKG_CONFIG_PATH': None,
      }
    })

    with utils.chdir('test'):
      self.run_process(['scons', '--expected-env', expected_to_propagate])

  @requires_scons
  def test_emscons(self):
    shutil.copytree(test_file('scons/simple'), 'test')
    with utils.chdir('test'):
      self.run_process([path_from_root('emscons'), 'scons'])
      output = self.run_js('scons_integration.js', assert_returncode=5)
    self.assertContained('If you see this - the world is all right!', output)

  @requires_scons
  def test_emscons_env(self):
    shutil.copytree(test_file('scons/env'), 'test')

    building_env = get_building_env()
    expected_to_propagate = json.dumps({
      'CC': path_from_root('emcc'),
      'CXX': path_from_root('em++'),
      'AR': path_from_root('emar'),
      'RANLIB': path_from_root('emranlib'),
      'ENV': {
        'PKG_CONFIG_LIBDIR': building_env['PKG_CONFIG_LIBDIR'],
        'PKG_CONFIG_PATH': building_env['PKG_CONFIG_PATH'],
      }
    })

    with utils.chdir('test'):
      self.run_process([path_from_root('emscons'), 'scons', '--expected-env', expected_to_propagate])

  def test_embind_fail(self):
    out = self.expect_fail([EMXX, test_file('embind/test_unsigned.cpp')])
    self.assertContained("undefined symbol: _embind_register_function", out)

  def test_embind_invalid_overload(self):
    expected = 'BindingError: Cannot register multiple overloads of a function with the same number of arguments'
    self.do_runf(test_file('embind/test_embind_invalid_overload.cpp'), expected, emcc_args=['-lembind'], assert_returncode=NON_ZERO)

  def test_embind_asyncify(self):
    create_file('post.js', '''
      addOnPostRun(() => {
        Module.sleep(10);
        out('done');
      });
    ''')
    create_file('main.cpp', r'''
      #include <emscripten.h>
      #include <emscripten/bind.h>
      using namespace emscripten;
      EMSCRIPTEN_BINDINGS(asyncify) {
          function("sleep", &emscripten_sleep);
      }
    ''')
    self.do_runf('main.cpp', 'done', emcc_args=['-lembind', '-sASYNCIFY', '--post-js', 'post.js'])

  @parameterized({
    '': ['-sDYNAMIC_EXECUTION=1'],
    'no_dynamic': ['-sDYNAMIC_EXECUTION=0'],
  })
  @requires_jspi
  def test_embind_jspi(self, extra):
    self.emcc_args += ['-lembind', '-g']
    self.emcc_args += [extra]

    self.do_runf('embind/embind_jspi_test.cpp', 'done')

  def test_embind_no_function(self):
    create_file('post.js', '''
      Module.onRuntimeInitialized = () => {
        out((new Module['MyClass'](42)).x);
      };
    ''')
    create_file('main.cpp', r'''
      #include <emscripten.h>
      #include <emscripten/bind.h>
      using namespace emscripten;
      class MyClass {
      public:
          MyClass(int x) : x(x) {}

          int getX() const {return x;}
          void setX(int newX) {x = newX;}
      private:
          int x;
      };
      EMSCRIPTEN_BINDINGS(my_module) {
          class_<MyClass>("MyClass")
              .constructor<int>()
              .property("x", &MyClass::getX, &MyClass::setX);
      }
    ''')
    self.do_runf('main.cpp', '42', emcc_args=['-lembind', '--post-js', 'post.js'])

  def test_embind_closure_no_dynamic_execution(self):
    create_file('post.js', '''
      Module['onRuntimeInitialized'] = () => {
        out(Module['foo'](10));
        out(Module['bar']());
      };
    ''')
    create_file('main.cpp', r'''
      #include <string>
      #include <emscripten/console.h>
      #include <emscripten/bind.h>
      int foo(int x) { return x; }
      void bar() {
        emscripten::val(123).call<std::string>("toString");
        emscripten::val jarray = emscripten::val::global("Float32Array").new_(10);
        emscripten_console_log("ok");
      }
      EMSCRIPTEN_BINDINGS(baz) {
        emscripten::function("foo", &foo);
        emscripten::function("bar", &bar);
      }
    ''')
    self.set_setting('INCOMING_MODULE_JS_API', 'onRuntimeInitialized')
    self.set_setting('STRICT')
    self.set_setting('NO_DYNAMIC_EXECUTION')
    self.do_runf('main.cpp', '10\nok\n',
                 emcc_args=['--no-entry', '-lembind', '-O2', '--closure=1', '--minify=0', '--post-js=post.js'])

  @is_slow_test
  @parameterized({
    '': [],
    'no_utf8': ['-sEMBIND_STD_STRING_IS_UTF8=0'],
    'no_dynamic': ['-sDYNAMIC_EXECUTION=0'],
    'aot_js': ['-sDYNAMIC_EXECUTION=0', '-sEMBIND_AOT', '-DSKIP_UNBOUND_TYPES'],
    'wasm64': ['-sMEMORY64', '-Wno-experimental'],
    '2gb': ['-sINITIAL_MEMORY=2200mb', '-sGLOBAL_BASE=2gb'],
  })
  @parameterized({
    # With no arguments we are effectively testing c++17 since it is the default.
    '': [],
    # Ensure embind compiles under C++11 which is the miniumum supported version.
    'cxx11': ['-std=c++11'],
    'o1': ['-O1'],
    'o2': ['-O2'],
    'o2_mem_growth': ['-O2', '-sALLOW_MEMORY_GROWTH', test_file('embind/isMemoryGrowthEnabled=true.cpp')],
    'o2_closure': ['-O2', '--closure=1', '--closure-args', '--externs ' + shlex.quote(test_file('embind/underscore-externs.js'))],
    'strict_js': ['-sSTRICT_JS']
  })
  def test_embind(self, *extra_args):
    if '-sMEMORY64' in extra_args:
      self.require_wasm64()
    self.emcc_args += [
      '--no-entry',
      # This test explicitly creates std::string from unsigned char pointers
      # which is deprecated in upstream LLVM.
      '-Wno-deprecated-declarations',
      '-lembind',
      '-sRETAIN_COMPILER_SETTINGS',
      '-sEXPORTED_RUNTIME_METHODS=getCompilerSetting,setDelayFunction,flushPendingDeletes',
      '-sWASM_ASYNC_COMPILATION=0',
      # This test uses a `CustomSmartPtr` class which has 1MB of data embedded in
      # it which means we need more stack space than normal.
      '-sSTACK_SIZE=2MB',
      '--pre-js', test_file('embind/test.pre.js'),
      '--post-js', test_file('embind/test.post.js'),
    ]
    self.emcc_args += extra_args

    js_file = self.build(test_file('embind/embind_test.cpp'))

    testFiles = [
      test_file('embind/underscore-1.4.2.js'),
      test_file('embind/imvu_test_adapter.js'),
      test_file('embind/embind.test.js'),
    ]

    if '-sDYNAMIC_EXECUTION=0' in extra_args:
      js_binary_str = read_file(js_file)
      self.assertNotContained('new Function(', js_binary_str)
      self.assertNotContained('eval(', js_binary_str)

    with open(js_file, 'ab') as f:
      for tf in testFiles:
        f.write(read_binary(tf))

    output = self.run_js(js_file)
    self.assertNotContained('FAIL', output)

  @requires_node
  def test_embind_finalization(self):
    self.run_process(
      [EMXX,
       test_file('embind/test_finalization.cpp'),
       '--pre-js', test_file('embind/test_finalization.js'),
       '-lembind']
    )
    self.node_args += ['--expose-gc']
    output = self.run_js('a.out.js')
    self.assertContained('Constructed from C++ destructed', output)
    self.assertContained('Constructed from JS destructed', output)
    self.assertNotContained('Foo* destructed', output)

  def test_embind_return_value_policy(self):
    self.emcc_args += ['-lembind']

    self.do_runf('embind/test_return_value_policy.cpp')

  @requires_jspi
  @parameterized({
    '': [['-sJSPI_EXPORTS=async*']],
    'deprecated': [['-Wno-deprecated', '-sASYNCIFY_EXPORTS=async*']]
  })
  def test_jspi_wildcard(self, opts):
    self.emcc_args += opts

    self.do_runf('other/test_jspi_wildcard.c', 'done')

  @requires_jspi
  def test_jspi_add_function(self):
    # make sure synchronous functions in the wasmTable aren't processed with Asyncify.makeAsyncFunction
    self.emcc_args += [
      '-sASYNCIFY=2',
      '-sEXPORTED_RUNTIME_METHODS=addFunction,dynCall',
      '-sALLOW_TABLE_GROWTH=1',
      '-Wno-experimental']
    self.do_runf('other/test_jspi_add_function.c', 'done')

  @parameterized({
    '': [[]],
    'with_jsgen': [['-sEMBIND_AOT']]
  })
  def test_embind_tsgen(self, opts):
    # Check that TypeScript generation works and that the program is runs as
    # expected.
    self.do_runf('other/embind_tsgen.cpp', 'main ran',
                 emcc_args=['-lembind', '--emit-tsd', 'embind_tsgen.d.ts'] + opts)

    # Test that the output compiles with a TS file that uses the defintions.
    cmd = shared.get_npm_cmd('tsc') + ['embind_tsgen.d.ts', '--noEmit']
    shared.check_call(cmd)

    actual = read_file('embind_tsgen.d.ts')
    self.assertFileContents(test_file('other/embind_tsgen.d.ts'), actual)

  def test_embind_tsgen_ignore(self):
    create_file('fail.js', 'assert(false);')
    self.emcc_args += ['-lembind', '--emit-tsd', 'embind_tsgen.d.ts']
    # These extra arguments are not related to TS binding generation but we want to
    # verify that they do not interfere with it.
    extra_args = ['-sALLOW_MEMORY_GROWTH=1',
                  '-Wno-pthreads-mem-growth',
                  '-sMAXIMUM_MEMORY=4GB',
                  '--pre-js', 'fail.js',
                  '--post-js', 'fail.js',
                  '--extern-pre-js', 'fail.js',
                  '--extern-post-js', 'fail.js',
                  '-sENVIRONMENT=worker',
                  '--use-preload-cache',
                  '--preload-file', 'fail.js',
                  '-O3',
                  '-msimd128',
                  '-pthread',
                  '-sPROXY_TO_PTHREAD',
                  '-sPTHREAD_POOL_SIZE=1',
                  '-sSINGLE_FILE',
                  '-lembind', # Test duplicated link option.
                  ]
    self.emcc(test_file('other/embind_tsgen.cpp'), extra_args)
    self.assertFileContents(test_file('other/embind_tsgen_ignore_1.d.ts'), read_file('embind_tsgen.d.ts'))
    # Test these args separately since they conflict with arguments in the first test.
    extra_args = ['-sMODULARIZE',
                  '--embed-file', 'fail.js',
                  '-sMINIMAL_RUNTIME=2',
                  '-sEXPORT_ES6=1',
                  '-sASSERTIONS=0',
                  '-sSTRICT=1']
    self.emcc(test_file('other/embind_tsgen.cpp'), extra_args)
    self.assertFileContents(test_file('other/embind_tsgen_ignore_2.d.ts'), read_file('embind_tsgen.d.ts'))
    # Also test this separately since it conflicts with other settings.
    extra_args = ['-sWASM=0']
    self.emcc(test_file('other/embind_tsgen.cpp'), extra_args)
    self.assertFileContents(test_file('other/embind_tsgen_ignore_3.d.ts'), read_file('embind_tsgen.d.ts'))

  def test_embind_tsgen_worker_env(self):
    self.emcc_args += ['-lembind', '--emit-tsd', 'embind_tsgen.d.ts']
    # Passing -sWASM_WORKERS or -sPROXY_TO_WORKER requires the 'worker' environment
    # at link time. Verify that TS binding generation still works in this case.
    for flag in ('-sWASM_WORKERS', '-sPROXY_TO_WORKER'):
      self.emcc(test_file('other/embind_tsgen.cpp'), [flag])
      self.assertFileContents(test_file('other/embind_tsgen.d.ts'), read_file('embind_tsgen.d.ts'))

  def test_embind_tsgen_dylink(self):
    create_file('side.h', r'''
      void someLibraryFunc();
    ''')
    create_file('side.cpp', r'''
      #include "side.h"
      void someLibraryFunc() {}
    ''')
    create_file('main.cpp', r'''
      #include "side.h"
      #include <emscripten/bind.h>
      void mainLibraryFunc() {}

      EMSCRIPTEN_BINDINGS( MainLibrary ) {
        emscripten::function("mainLibraryFunc", &mainLibraryFunc );
        emscripten::function("someLibraryFunc", &someLibraryFunc );
      }
    ''')
    self.run_process([
      EMCC,
      '-o', 'libside.wasm',
      'side.cpp',
      '-sSIDE_MODULE'])
    self.emcc('main.cpp', ['libside.wasm', '-sMAIN_MODULE=2', '-lembind', '--emit-tsd', 'embind_tsgen.d.ts'])

  def test_embind_tsgen_test_embind(self):
    self.run_process([EMXX, test_file('embind/embind_test.cpp'),
                      '-lembind', '--emit-tsd', 'embind_tsgen_test_embind.d.ts',
                      # This test explicitly creates std::string from unsigned char pointers
                      # which is deprecated in upstream LLVM.
                      '-Wno-deprecated-declarations',
                      # TypeScript generation requires all type to be bound.
                      '-DSKIP_UNBOUND_TYPES'] + self.get_emcc_args())
    self.assertExists('embind_tsgen_test_embind.d.ts')

  def test_embind_tsgen_val(self):
    # Check that any dependencies from val still works with TS generation enabled.
    self.run_process([EMCC, test_file('other/embind_tsgen_val.cpp'),
                      '-lembind', '--emit-tsd', 'embind_tsgen_val.d.ts'])
    self.assertExists('embind_tsgen_val.d.ts')

  def test_embind_tsgen_bigint(self):
    args = [EMXX, test_file('other/embind_tsgen_bigint.cpp'), '-lembind', '--emit-tsd', 'embind_tsgen_bigint.d.ts']
    # Check that TypeScript generation fails when code contains bigints but their support is not enabled
    stderr = self.expect_fail(args)
    self.assertContained("Missing primitive type to TS type for 'int64_t", stderr)
    # Check that TypeScript generation works when bigint support is enabled
    self.run_process(args + ['-sWASM_BIGINT'])
    self.assertFileContents(test_file('other/embind_tsgen_bigint.d.ts'), read_file('embind_tsgen_bigint.d.ts'))

  @requires_wasm64
  def test_embind_tsgen_memory64(self):
    # Check that when memory64 is enabled longs & unsigned longs are mapped to bigint in the generated TS bindings
    self.run_process([EMXX, test_file('other/embind_tsgen_memory64.cpp'),
                      '-lembind', '--emit-tsd', 'embind_tsgen_memory64.d.ts', '-sMEMORY64', '-Wno-experimental'] +
                     self.get_emcc_args())
    self.assertFileContents(test_file('other/embind_tsgen_memory64.d.ts'), read_file('embind_tsgen_memory64.d.ts'))

  @parameterized({
    '': [0],
    'wasm_exnref': [1]
  })
  def test_embind_tsgen_exceptions(self, wasm_exnref):
    self.set_setting('WASM_EXNREF', wasm_exnref)
    # Check that when Wasm exceptions and assertions are enabled bindings still generate.
    self.run_process([EMXX, test_file('other/embind_tsgen.cpp'),
                      '-lembind', '-fwasm-exceptions', '-sASSERTIONS',
                      # Use the deprecated `--embind-emit-tsd` to ensure it
                      # still works until removed.
                      '--embind-emit-tsd', 'embind_tsgen.d.ts', '-Wno-deprecated'] +
                     self.get_emcc_args())
    self.assertFileContents(test_file('other/embind_tsgen.d.ts'), read_file('embind_tsgen.d.ts'))

  def test_embind_jsgen_method_pointer_stability(self):
    self.emcc_args += ['-lembind', '-sEMBIND_AOT']
    # Test that when method pointers are allocated at different addresses that
    # AOT JS generation still works correctly.
    self.do_runf('other/embind_jsgen_method_pointer_stability.cpp', 'done')

  def test_emit_tsd(self):
    self.run_process([EMCC, test_file('other/test_emit_tsd.c'),
                      '--emit-tsd', 'test_emit_tsd.d.ts', '-sEXPORT_ES6',
                      '-sMODULARIZE', '-sEXPORTED_RUNTIME_METHODS=UTF8ArrayToString,wasmTable',
                      '-Wno-experimental', '-o', 'test_emit_tsd.js'] +
                     self.get_emcc_args())
    self.assertFileContents(test_file('other/test_emit_tsd.d.ts'), read_file('test_emit_tsd.d.ts'))
    # Test that the output compiles with a TS file that uses the defintions.
    cmd = shared.get_npm_cmd('tsc') + [test_file('other/test_tsd.ts'), '--noEmit']
    shared.check_call(cmd)

  def test_emit_tsd_sync_compilation(self):
    self.run_process([EMCC, test_file('other/test_emit_tsd.c'),
                      '--emit-tsd', 'test_emit_tsd_sync.d.ts',
                      '-sMODULARIZE', '-sWASM_ASYNC_COMPILATION=0',
                      '-o', 'test_emit_tsd_sync.js'] +
                     self.get_emcc_args())
    self.assertFileContents(test_file('other/test_emit_tsd_sync.d.ts'), read_file('test_emit_tsd_sync.d.ts'))
    # Test that the output compiles with a TS file that uses the defintions.
    cmd = shared.get_npm_cmd('tsc') + [test_file('other/test_tsd_sync.ts'), '--noEmit']
    shared.check_call(cmd)

  def test_emit_tsd_wasm_only(self):
    err = self.expect_fail([EMCC, test_file('other/test_emit_tsd.c'),
                            '--emit-tsd', 'test_emit_tsd_wasm_only.d.ts', '-o', 'out.wasm'])
    self.assertContained('Wasm only output is not compatible --emit-tsd', err)

  def test_emconfig(self):
    output = self.run_process([emconfig, 'LLVM_ROOT'], stdout=PIPE).stdout.strip()
    self.assertEqual(output, config.LLVM_ROOT)
    # EMSCRIPTEN_ROOT is kind of special since it should always report the locaton of em-config
    # itself (its not configurable via the config file but driven by the location for arg0)
    output = self.run_process([emconfig, 'EMSCRIPTEN_ROOT'], stdout=PIPE).stdout.strip()
    self.assertEqual(output, os.path.dirname(emconfig))
    invalid = 'Usage: em-config VAR_NAME'
    # Don't accept variables that do not exist
    output = self.expect_fail([emconfig, 'VAR_WHICH_DOES_NOT_EXIST']).strip()
    self.assertEqual(output, invalid)
    # Don't accept no arguments
    output = self.expect_fail([emconfig]).strip()
    self.assertEqual(output, invalid)
    # Don't accept more than one variable
    output = self.expect_fail([emconfig, 'LLVM_ROOT', 'EMCC']).strip()
    self.assertEqual(output, invalid)
    # Don't accept arbitrary python code
    output = self.expect_fail([emconfig, 'sys.argv[1]']).strip()
    self.assertEqual(output, invalid)

  def test_link_s(self):
    # -s OPT=VALUE can conflict with -s as a linker option. We warn and ignore
    create_file('main.c', r'''
      void something();

      int main() {
        something();
        return 0;
      }
    ''')
    create_file('supp.c', r'''
      #include <stdio.h>

      void something() {
        printf("yello\n");
      }
    ''')
    self.run_process([EMCC, '-c', 'main.c', '-o', 'main.o'])
    self.run_process([EMCC, '-c', 'supp.c', '-o', 'supp.o'])

    self.run_process([EMCC, 'main.o', '-s', 'supp.o', '-sSAFE_HEAP'])
    self.assertContained('yello', self.run_js('a.out.js'))
    # Check that valid -s option had an effect'
    self.assertContained('SAFE_HEAP', read_file('a.out.js'))

  def test_conftest_s_flag_passing(self):
    create_file('conftest.c', r'''
      int main() {
        return 0;
      }
    ''')
    # the name "conftest.c" is enough to make us use a configure-like mode,
    # the same as if EMMAKEN_JUST_CONFIGURE=1 were set in the env.
    cmd = [EMCC, '-sASSERTIONS', 'conftest.c', '-o', 'conftest']
    output = self.run_process(cmd, stderr=PIPE)
    self.assertNotContained('emcc: warning: treating -s as linker option', output.stderr)
    self.assertExists('conftest')

  def test_file_packager(self):
    ensure_dir('subdir')
    create_file('data1.txt', 'data1')

    os.chdir('subdir')
    create_file('data2.txt', 'data2')

    def check(text):
      self.assertGreater(len(text), 0)
      empty_lines = 0
      # check the generated is relatively tidy
      for line in text.splitlines():
        if line and line[-1].isspace():
          self.fail('output contains trailing whitespace: `%s`' % line)

        if line.strip():
          empty_lines = 0
        else:
          empty_lines += 1
          if empty_lines > 1:
            self.fail('output contains more then one empty line in row')

    # relative path must be within/below the current dir
    stderr = self.expect_fail([FILE_PACKAGER, 'test.data', '--quiet', '--preload', '../data1.txt'])
    self.assertContained('which is not contained within the current directory', stderr)

    # relative path that ends up under us is cool
    proc = self.run_process([FILE_PACKAGER, 'test.data', '--quiet', '--preload', '../subdir/data2.txt'], stderr=PIPE, stdout=PIPE)
    self.assertEqual(proc.stderr, '')
    check(proc.stdout)

    # direct path leads to the same code being generated - relative path does not make us do anything different
    proc2 = self.run_process([FILE_PACKAGER, 'test.data', '--quiet', '--preload', 'data2.txt'], stderr=PIPE, stdout=PIPE)
    check(proc2.stdout)
    self.assertEqual(proc2.stderr, '')

    def clean(txt):
      lines = txt.splitlines()
      lines = [l for l in lines if 'PACKAGE_UUID' not in l and 'loadPackage({' not in l]
      return ''.join(lines)

    self.assertTextDataIdentical(clean(proc.stdout), clean(proc2.stdout))

    # verify '--separate-metadata' option produces separate metadata file
    os.chdir('..')

    self.run_process([FILE_PACKAGER, 'test.data', '--quiet', '--preload', 'data1.txt', '--preload', 'subdir/data2.txt', '--js-output=immutable.js', '--separate-metadata', '--use-preload-cache'])
    self.assertExists('immutable.js.metadata')
    # verify js output JS file is not touched when the metadata is separated
    orig_timestamp = os.path.getmtime('immutable.js')
    orig_content = read_file('immutable.js')
    # ensure some time passes before running the packager again so that if it does touch the
    # js file it will end up with the different timestamp.
    time.sleep(1.0)
    self.run_process([FILE_PACKAGER, 'test.data', '--quiet', '--preload', 'data1.txt', '--preload', 'subdir/data2.txt', '--js-output=immutable.js', '--separate-metadata', '--use-preload-cache'])
    # assert both file content and timestamp are the same as reference copy
    self.assertTextDataIdentical(orig_content, read_file('immutable.js'))
    self.assertEqual(orig_timestamp, os.path.getmtime('immutable.js'))
    # verify the content of metadata file is correct
    metadata = json.loads(read_file('immutable.js.metadata'))
    self.assertEqual(len(metadata['files']), 2)
    assert metadata['files'][0]['start'] == 0 and metadata['files'][0]['end'] == len('data1') and metadata['files'][0]['filename'] == '/data1.txt'
    assert metadata['files'][1]['start'] == len('data1') and metadata['files'][1]['end'] == len('data1') + len('data2') and metadata['files'][1]['filename'] == '/subdir/data2.txt'
    assert metadata['remote_package_size'] == len('data1') + len('data2')

    self.assertEqual(metadata['package_uuid'], 'sha256-53ddc03623f867c7d4a631ded19c2613f2cb61d47b6aa214f47ff3cc15445bcd')

  def test_file_packager_unicode(self):
    unicode_name = 'unicode…☃'
    try:
      ensure_dir(unicode_name)
    except OSError:
      print("we failed to even create a unicode dir, so on this OS, we can't test this")
      return
    full = os.path.join(unicode_name, 'data.txt')
    create_file(full, 'data')
    proc = self.run_process([FILE_PACKAGER, 'test.data', '--preload', full], stdout=PIPE, stderr=PIPE)
    assert len(proc.stdout), proc.stderr
    assert json.dumps(unicode_name) in proc.stdout, proc.stdout
    print(len(proc.stderr))

  def test_file_packager_directory_with_single_quote(self):
    single_quote_name = "direc'tory"
    ensure_dir(single_quote_name)
    full = os.path.join(single_quote_name, 'data.txt')
    create_file(full, 'data')
    proc = self.run_process([FILE_PACKAGER, 'test.data', '--preload', full], stdout=PIPE, stderr=PIPE)
    assert len(proc.stdout), proc.stderr
    # ensure not invalid JavaScript
    assert "'direc'tory'" not in proc.stdout
    assert json.dumps("direc'tory") in proc.stdout

  def test_file_packager_mention_FORCE_FILESYSTEM(self):
    MESSAGE = 'Remember to build the main file with `-sFORCE_FILESYSTEM` so that it includes support for loading this file package'
    create_file('data.txt', 'data1')
    # mention when running standalone
    err = self.run_process([FILE_PACKAGER, 'test.data', '--preload', 'data.txt'], stdout=PIPE, stderr=PIPE).stderr
    self.assertContained(MESSAGE, err)
    # do not mention from emcc
    err = self.run_process([EMCC, test_file('hello_world.c'), '--preload-file', 'data.txt'], stdout=PIPE, stderr=PIPE).stderr
    self.assertEqual(len(err), 0)

  def test_file_packager_returns_error_if_target_equal_to_jsoutput(self):
    MESSAGE = 'error: TARGET should not be the same value of --js-output'
    err = self.expect_fail([FILE_PACKAGER, 'test.data', '--js-output=test.data'])
    self.assertContained(MESSAGE, err)

  def test_file_packager_embed(self):
    create_file('data.txt', 'hello data')

    # Without --obj-output we issue a warning
    err = self.run_process([FILE_PACKAGER, 'test.data', '--embed', 'data.txt', '--js-output=data.js'], stderr=PIPE).stderr
    self.assertContained('--obj-output is recommended when using --embed', err)

    self.run_process([FILE_PACKAGER, 'test.data', '--embed', 'data.txt', '--obj-output=data.o'])

    create_file('test.c', '''
    #include <stdio.h>

    int main() {
      FILE* f = fopen("data.txt", "r");
      char buf[64];
      int rtn = fread(buf, 1, 64, f);
      buf[rtn] = '\\0';
      fclose(f);
      printf("%s\\n", buf);
      return 0;
    }
    ''')
    self.run_process([EMCC, 'test.c', 'data.o', '-sFORCE_FILESYSTEM'])
    output = self.run_js('a.out.js')
    self.assertContained('hello data', output)

  @crossplatform
  def test_file_packager_depfile(self):
    create_file('data1.txt', 'data1')
    ensure_dir('subdir')
    create_file('subdir/data2.txt', 'data2')

    self.run_process([FILE_PACKAGER, 'test.data', '--js-output=test.js', '--depfile=test.data.d', '--from-emcc', '--preload', '.'])
    output = read_file('test.data.d')
    file_packager = utils.normalize_path(shared.replace_suffix(FILE_PACKAGER, '.py'))
    file_packager = file_packager.replace(' ', '\\ ')
    lines = output.splitlines()
    split = lines.index(': \\')
    before, after = set(lines[:split]), set(lines[split + 1:])
    # Set comparison used because depfile is not order-sensitive.
    self.assertTrue('test.data \\' in before)
    self.assertTrue('test.js \\' in before)
    self.assertTrue(file_packager + ' \\' in after)
    self.assertTrue('. \\' in after)
    self.assertTrue('./data1.txt \\' in after)
    self.assertTrue('./subdir \\' in after)
    self.assertTrue('./subdir/data2.txt \\' in after)

  def test_file_packager_modularize(self):
    create_file('somefile.txt', 'hello world')
    self.run_process([FILE_PACKAGER, 'test.data', '--js-output=embed.js', '--preload', 'somefile.txt'])

    create_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      int main() {
        FILE *f = fopen("somefile.txt", "r");
        assert(f);
        char buf[20] = { 0 };
        int rtn = fread(buf, 1, 20, f);
        fclose(f);
        printf("|%s|\n", buf);
        return 0;
      }
    ''')

    create_file('post.js', 'MyModule(Module).then(() => console.log("done"));')

    self.run_process([EMCC, 'main.c', '--extern-pre-js=embed.js', '--extern-post-js=post.js', '-sMODULARIZE', '-sEXPORT_NAME=MyModule', '-sFORCE_FILESYSTEM'])

    result = self.run_js('a.out.js')
    self.assertContained('|hello world|', result)

  def test_sdl_headless(self):
    shutil.copy(test_file('screenshot.png'), 'example.png')
    self.do_other_test('test_sdl_headless.c', emcc_args=['-sHEADLESS'])

  def test_preprocess(self):
    # Pass -Werror to prevent regressions such as https://github.com/emscripten-core/emscripten/pull/9661
    out = self.run_process([EMCC, test_file('hello_world.c'), '-E', '-Werror'], stdout=PIPE).stdout
    self.assertNotExists('a.out.js')
    self.assertNotExists('a.out')
    # Test explicitly that the output contains a line typically written by the preprocessor.
    self.assertContained('# 1 ', out)
    self.assertContained('hello_world.c"', out)
    self.assertContained('printf("hello, world!', out)

  def test_preprocess_multi(self):
    out = self.run_process([EMCC, test_file('hello_world.c'), test_file('hello_world.c'), '-E'], stdout=PIPE).stdout
    self.assertEqual(out.count('printf("hello, world!'), 2)

  def test_syntax_only_valid(self):
    result = self.run_process([EMCC, test_file('hello_world.c'), '-fsyntax-only'], stdout=PIPE, stderr=STDOUT)
    self.assertEqual(result.stdout, '')
    self.assertNotExists('a.out.js')

  def test_syntax_only_invalid(self):
    create_file('src.c', 'int main() {')
    err = self.expect_fail([EMCC, 'src.c', '-fsyntax-only'])
    self.assertContained("src.c:1:13: error: expected '}'", err)
    self.assertNotExists('a.out.js')

  # `demangle` is a legacy JS function on longer used by emscripten
  # TODO(sbc): Remove `demangle` and this test.
  def test_demangle(self):
    create_file('src.cpp', '''
      #include <stdio.h>
      #include <emscripten.h>

      void two(char c) {
        EM_ASM(out(stackTrace()));
      }
      void one(int x) {
        two(x % 17);
      }
      int main() {
        EM_ASM(out(demangle('__Znwm'))); // check for no aborts
        EM_ASM(out(demangle('_main')));
        EM_ASM(out(demangle('__Z2f2v')));
        EM_ASM(out(demangle('__Z12abcdabcdabcdi')));
        EM_ASM(out(demangle('__ZL12abcdabcdabcdi')));
        EM_ASM(out(demangle('__Z4testcsifdPvPiPc')));
        EM_ASM(out(demangle('__ZN4test5moarrEcslfdPvPiPc')));
        EM_ASM(out(demangle('__ZN4Waka1f12a234123412345pointEv')));
        EM_ASM(out(demangle('__Z3FooIiEvv')));
        EM_ASM(out(demangle('__Z3FooIidEvi')));
        EM_ASM(out(demangle('__ZN3Foo3BarILi5EEEvv')));
        EM_ASM(out(demangle('__ZNK10__cxxabiv120__si_class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib')));
        EM_ASM(out(demangle('__Z9parsewordRPKciRi')));
        EM_ASM(out(demangle('__Z5multiwahtjmxyz')));
        EM_ASM(out(demangle('__Z1aA32_iPA5_c')));
        EM_ASM(out(demangle('__ZN21FWakaGLXFleeflsMarfooC2EjjjPKvbjj')));
        EM_ASM(out(demangle('__ZN5wakaw2Cm10RasterBaseINS_6watwat9PolocatorEE8merbine1INS4_2OREEEvPKjj'))); // we get this wrong, but at least emit a '?'
        one(17);
        return 0;
      }
    ''')

    # full demangle support

    self.run_process([EMXX, 'src.cpp', '-sDEMANGLE_SUPPORT'])
    output = self.run_js('a.out.js')
    self.assertContained('''operator new(unsigned long)
_main
f2()
abcdabcdabcd(int)
abcdabcdabcd(int)
test(char, short, int, float, double, void*, int*, char*)
test::moarr(char, short, long, float, double, void*, int*, char*)
Waka::f::a23412341234::point()
void Foo<int>()
void Foo<int, double>(int)
void Foo::Bar<5>()
__cxxabiv1::__si_class_type_info::search_below_dst(__cxxabiv1::__dynamic_cast_info*, void const*, int, bool) const
parseword(char const*&, int, int&)
multi(wchar_t, signed char, unsigned char, unsigned short, unsigned int, unsigned long, long long, unsigned long long, ...)
a(int [32], char (*) [5])
FWakaGLXFleeflsMarfoo::FWakaGLXFleeflsMarfoo(unsigned int, unsigned int, unsigned int, void const*, bool, unsigned int, unsigned int)
void wakaw::Cm::RasterBase<wakaw::watwat::Polocator>::merbine1<wakaw::Cm::RasterBase<wakaw::watwat::Polocator>::OR>(unsigned int const*, unsigned int)
''', output)
    # test for multiple functions in one stack trace
    self.run_process([EMXX, 'src.cpp', '-sDEMANGLE_SUPPORT', '-g'])
    output = self.run_js('a.out.js')
    self.assertIn('one(int)', output)
    self.assertIn('two(char)', output)

  def test_demangle_cpp(self):
    create_file('src.cpp', '''
      #include <stdio.h>
      #include <emscripten.h>
      #include <cxxabi.h>
      #include <assert.h>

      int main() {
        char out[256];
        int status = 1;
        size_t length = 255;
        abi::__cxa_demangle("_ZN4Waka1f12a234123412345pointEv", out, &length, &status);
        assert(status == 0);
        printf("%s\\n", out);
        return 0;
      }
    ''')

    self.do_runf('src.cpp', 'Waka::f::a23412341234::point()')

  # Test that malloc() -> OOM -> abort() -> stackTrace() -> jsStackTrace()
  # cycle will not cycle back to malloc to produce an infinite loop.
  def test_demangle_malloc_infinite_loop_crash(self):
    self.run_process([EMXX, test_file('malloc_demangle_infinite_loop.cpp'), '-g', '-sABORTING_MALLOC'])
    output = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    if output.count('Cannot enlarge memory arrays') > 5:
      print(output)
    self.assertLess(output.count('Cannot enlarge memory arrays'),  6)

  @requires_node
  def test_module_exports_with_closure(self):
    # This test checks that module.export is retained when JavaScript
    # is minified by compiling with --closure 1
    # This is important as if module.export is not present the Module
    # object will not be visible to node.js

    # compile without --closure=1
    self.run_process([EMCC, test_file('module_exports/test.c'),
                      '-o', 'test.js', '-O2',
                      '-sEXPORTED_FUNCTIONS=_bufferTest,_malloc,_free',
                      '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap',
                      '-sWASM_ASYNC_COMPILATION=0'])

    # Check that test.js compiled without --closure=1 contains "module['exports'] = Module;"
    self.assertContained('module["exports"]=Module', read_file('test.js'))

    # Check that main.js (which requires test.js) completes successfully when run in node.js
    # in order to check that the exports are indeed functioning correctly.
    shutil.copy(test_file('module_exports/main.js'), '.')
    self.assertContained('bufferTest finished', self.run_js('main.js'))

    # Delete test.js again and check it's gone.
    delete_file('test.js')

    # compile with --closure=1
    self.run_process([EMCC, test_file('module_exports/test.c'),
                      '-o', 'test.js', '-O2', '--closure=1',
                      '-sEXPORTED_FUNCTIONS=_bufferTest,_malloc,_free',
                      '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap',
                      '-sWASM_ASYNC_COMPILATION=0'])

    # Check that test.js compiled with --closure 1 contains "module.exports", we want to verify that
    # "module['exports']" got minified to "module.exports" when compiling with --closure 1
    self.assertContained('module.exports=', read_file('test.js'))

    # Check that main.js (which requires test.js) completes successfully when run in node.js
    # in order to check that the exports are indeed functioning correctly.
    self.assertContained('bufferTest finished', self.run_js('main.js'))

  @requires_node
  def test_node_catch_exit(self):
    # Test that in top level JS exceptions are caught and rethrown when NODEJS_EXIT_CATCH=1
    # is set but not by default.
    create_file('count.c', '''
      #include <string.h>
      int count(const char *str) {
        return (int)strlen(str);
      }
    ''')

    create_file('index.js', '''
      const count = require('./count.js');

      console.log(xxx); //< here is the ReferenceError
    ''')

    reference_error_text = 'console.log(xxx); //< here is the ReferenceError'

    self.run_process([EMCC, 'count.c', '-o', 'count.js', '-sNODEJS_CATCH_EXIT=1'])

    # Check that the ReferenceError is caught and rethrown and thus the original error line is masked
    self.assertNotContained(reference_error_text,
                            self.run_js('index.js', assert_returncode=NON_ZERO))

    self.run_process([EMCC, 'count.c', '-o', 'count.js'])

    # Check that the ReferenceError is not caught, so we see the error properly
    self.assertContained(reference_error_text,
                         self.run_js('index.js', assert_returncode=NON_ZERO))

  @requires_node
  def test_exported_runtime_methods(self):
    # Test with node.js that the EXPORTED_RUNTIME_METHODS setting is
    # considered by libraries
    create_file('count.c', '''
      #include <string.h>
      int count(const char *str) {
        return (int)strlen(str);
      }
    ''')

    create_file('index.js', '''
      const count = require('./count.js');
      console.log(count.FS_writeFile);
      count.onRuntimeInitialized = () => {
        if (count.wasmExports && 'count' in count.wasmExports) {
          console.log('wasmExports found');
        } else {
          console.log('wasmExports NOT found');
        }
      };
    ''')

    self.run_process([EMCC, 'count.c', '-sFORCE_FILESYSTEM', '-sEXPORTED_FUNCTIONS=_count',
                      '-sEXPORTED_RUNTIME_METHODS=FS_writeFile,wasmExports', '-o', 'count.js'])

    # Check that the Module.FS_writeFile exists
    out = self.run_js('index.js')
    self.assertNotContained('undefined', out)
    self.assertContained('wasmExports found', out)

    self.run_process([EMCC, 'count.c', '-sFORCE_FILESYSTEM', '-o', 'count.js'])

    # Check that the Module.FS_writeFile is not exported
    out = self.run_js('index.js', assert_returncode=NON_ZERO)
    self.assertContained('undefined', out),
    self.assertContained("Aborted('wasmExports' was not exported. add it to EXPORTED_RUNTIME_METHODS", out)

  def test_exported_runtime_methods_from_js_library(self):
    create_file('pre.js', '''
      Module.onRuntimeInitialized = () => {
        out(Module.ptrToString(88));
        out('done');
      };
    ''')
    self.do_runf('hello_world.c', 'done', emcc_args=['--pre-js=pre.js', '-sEXPORTED_RUNTIME_METHODS=ptrToString'])

  @crossplatform
  def test_fs_stream_proto(self):
    create_file('src.c', br'''
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int main()
{
    long file_size = 0;
    int h = open("src.c", O_RDONLY, 0666);
    if (0 != h) {
        FILE* file = fdopen(h, "rb");
        if (0 != file) {
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
        } else {
            printf("fdopen() failed: %s\n", strerror(errno));
            return 10;
        }
        close(h);
        printf("File size: %ld\n", file_size);
    } else {
        printf("open() failed: %s\n", strerror(errno));
        return 10;
    }
    return 0;
}
''', binary=True)
    self.run_process([EMCC, 'src.c', '--embed-file', 'src.c', '-sENVIRONMENT=node,shell'])
    for engine in config.JS_ENGINES:
      out = self.run_js('a.out.js', engine=engine)
      self.assertContained('File size: 682', out)

  @node_pthreads
  def test_node_emscripten_num_logical_cores(self):
    # Test with node.js that the emscripten_num_logical_cores method is working
    create_file('src.c', r'''
#include <emscripten/threading.h>
#include <stdio.h>
#include <assert.h>

int main() {
  int num = emscripten_num_logical_cores();
  assert(num != 0);
  puts("ok");
}
''')
    self.do_runf('src.c', 'ok')

  def test_proxyfs(self):
    # This test supposes that 3 different programs share the same directory and files.
    # The same JS object is not used for each of them
    # But 'require' function caches JS objects.
    # If we just load same js-file multiple times like following code,
    # these programs (m0,m1,m2) share the same JS object.
    #
    #   var m0 = require('./proxyfs_test.js');
    #   var m1 = require('./proxyfs_test.js');
    #   var m2 = require('./proxyfs_test.js');
    #
    # To separate js-objects for each of them, following 'require' use different js-files.
    #
    #   var m0 = require('./proxyfs_test.js');
    #   var m1 = require('./proxyfs_test1.js');
    #   var m2 = require('./proxyfs_test2.js');
    #
    create_file('proxyfs_test_main.js', r'''
var m0 = require('./proxyfs_test.js');
var m1 = require('./proxyfs_test1.js');
var m2 = require('./proxyfs_test2.js');

var section;
function print(str){
  process.stdout.write(section+":"+str+":");
}

m0.FS.mkdir('/working');
m0.FS.mount(m0.PROXYFS,{root:'/',fs:m1.FS},'/working');
m0.FS.mkdir('/working2');
m0.FS.mount(m0.PROXYFS,{root:'/',fs:m2.FS},'/working2');

section = "child m1 reads and writes local file.";
print("m1 read embed");
m1.ccall('myreade','number',[],[]);
print("m1 write");console.log("");
m1.ccall('mywrite0','number',['number'],[1]);
print("m1 read");
m1.ccall('myread0','number',[],[]);

section = "child m2 reads and writes local file.";
print("m2 read embed");
m2.ccall('myreade','number',[],[]);
print("m2 write");console.log("");
m2.ccall('mywrite0','number',['number'],[2]);
print("m2 read");
m2.ccall('myread0','number',[],[]);

section = "child m1 reads local file.";
print("m1 read");
m1.ccall('myread0','number',[],[]);

section = "parent m0 accesses children's file.";
print("m0 access existing");
m0.ccall('myaccess0existing','number',[],[]);
print("m0 access absent");
m0.ccall('myaccess0absent','number',[],[]);

section = "child m1 accesses local file.";
print("m1 access existing");
m1.ccall('myaccess1existing','number',[],[]);
print("m1 access absent");
m1.ccall('myaccess1absent','number',[],[]);

section = "parent m0 reads and writes local and children's file.";
print("m0 read embed");
m0.ccall('myreade','number',[],[]);
print("m0 read m1");
m0.ccall('myread1','number',[],[]);
print("m0 read m2");
m0.ccall('myread2','number',[],[]);

section = "m0,m1 and m2 verify local files.";
print("m0 write");console.log("");
m0.ccall('mywrite0','number',['number'],[0]);
print("m0 read");
m0.ccall('myread0','number',[],[]);
print("m1 read");
m1.ccall('myread0','number',[],[]);
print("m2 read");
m2.ccall('myread0','number',[],[]);

print("m0 read embed");
m0.ccall('myreade','number',[],[]);
print("m1 read embed");
m1.ccall('myreade','number',[],[]);
print("m2 read embed");
m2.ccall('myreade','number',[],[]);

section = "parent m0 writes and reads children's files.";
print("m0 write m1");console.log("");
m0.ccall('mywrite1','number',[],[]);
print("m0 read m1");
m0.ccall('myread1','number',[],[]);
print("m0 write m2");console.log("");
m0.ccall('mywrite2','number',[],[]);
print("m0 read m2");
m0.ccall('myread2','number',[],[]);
print("m1 read");
m1.ccall('myread0','number',[],[]);
print("m2 read");
m2.ccall('myread0','number',[],[]);
print("m0 read m0");
m0.ccall('myread0','number',[],[]);

section = "parent m0 renames a file in child fs.";
m0.FS.writeFile('/working/test', 'testme');
m0.FS.rename('/working/test', '/working/test.bak');
console.log(section + ":renamed file accessible by the new name:" + m0.FS.analyzePath('/working/test.bak').exists);
console.log(section + ":renamed file accessible by the old name:" + m0.FS.analyzePath('/working/test').exists);

section = "test seek.";
print("file size");
m0.ccall('myreadSeekEnd', 'number', [], []);
''')

    create_file('proxyfs_pre.js', r'''
Module["noInitialRun"]=true;
''')

    create_file('proxyfs_embed.txt', 'test\n')

    create_file('proxyfs_test.c', r'''
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE int mywrite1() {
  FILE* out = fopen("/working/hoge.txt","w");
  fprintf(out,"test1\n");
  fclose(out);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myread1() {
  FILE* in = fopen("/working/hoge.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int mywrite2() {
  FILE* out = fopen("/working2/hoge.txt","w");
  fprintf(out,"test2\n");
  fclose(out);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myread2() {
  FILE* in = fopen("/working2/hoge.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int mywrite0(int i) {
  FILE* out = fopen("hoge.txt","w");
  fprintf(out,"test0_%d\n",i);
  fclose(out);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myread0() {
  FILE* in = fopen("hoge.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess0existing() {
  int canAccess = access("/working/hoge.txt",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess0absent() {
  int canAccess = access("/working/nosuchfile",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess1existing() {
  int canAccess = access("/hoge.txt",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess1absent() {
  int canAccess = access("/nosuchfile",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myreade() {
  FILE* in = fopen("proxyfs_embed.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myreadSeekEnd() {
  FILE* in = fopen("/working2/hoge.txt","r");

  fseek(in, 0L, SEEK_END);
  int fileSize = ftell(in);
  fseek(in, 0L, SEEK_SET);
  printf("%d\n", fileSize);

  fclose(in);
  return 0;
}
''')

    self.run_process([EMCC,
                      '-o', 'proxyfs_test.js', 'proxyfs_test.c',
                      '--embed-file', 'proxyfs_embed.txt', '--pre-js', 'proxyfs_pre.js',
                      '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,FS,PROXYFS',
                      '-lproxyfs.js',
                      '-sWASM_ASYNC_COMPILATION=0'])
    # Following shutil.copy just prevent 'require' of node.js from caching js-object.
    # See https://nodejs.org/api/modules.html
    shutil.copy('proxyfs_test.js', 'proxyfs_test1.js')
    shutil.copy('proxyfs_test.js', 'proxyfs_test2.js')
    out = self.run_js('proxyfs_test_main.js')
    section = "child m1 reads and writes local file."
    self.assertContained(section + ":m1 read embed:test", out)
    self.assertContained(section + ":m1 write:", out)
    self.assertContained(section + ":m1 read:test0_1", out)
    section = "child m2 reads and writes local file."
    self.assertContained(section + ":m2 read embed:test", out)
    self.assertContained(section + ":m2 write:", out)
    self.assertContained(section + ":m2 read:test0_2", out)
    section = "child m1 reads local file."
    self.assertContained(section + ":m1 read:test0_1", out)
    section = "parent m0 accesses children's file."
    self.assertContained(section + ":m0 access existing:access=0", out)
    self.assertContained(section + ":m0 access absent:access=-1", out)
    section = "child m1 accesses local file."
    self.assertContained(section + ":m1 access existing:access=0", out)
    self.assertContained(section + ":m1 access absent:access=-1", out)
    section = "parent m0 reads and writes local and children's file."
    self.assertContained(section + ":m0 read embed:test", out)
    self.assertContained(section + ":m0 read m1:test0_1", out)
    self.assertContained(section + ":m0 read m2:test0_2", out)
    section = "m0,m1 and m2 verify local files."
    self.assertContained(section + ":m0 write:", out)
    self.assertContained(section + ":m0 read:test0_0", out)
    self.assertContained(section + ":m1 read:test0_1", out)
    self.assertContained(section + ":m2 read:test0_2", out)
    self.assertContained(section + ":m0 read embed:test", out)
    self.assertContained(section + ":m1 read embed:test", out)
    self.assertContained(section + ":m2 read embed:test", out)
    section = "parent m0 writes and reads children's files."
    self.assertContained(section + ":m0 write m1:", out)
    self.assertContained(section + ":m0 read m1:test1", out)
    self.assertContained(section + ":m0 write m2:", out)
    self.assertContained(section + ":m0 read m2:test2", out)
    self.assertContained(section + ":m1 read:test1", out)
    self.assertContained(section + ":m2 read:test2", out)
    self.assertContained(section + ":m0 read m0:test0_0", out)
    section = "parent m0 renames a file in child fs."
    self.assertContained(section + ":renamed file accessible by the new name:true", out)
    self.assertContained(section + ":renamed file accessible by the old name:false", out)
    section = "test seek."
    self.assertContained(section + ":file size:6", out)

  def test_dependency_file(self):
    # Issue 1732: -MMD (and friends) create dependency files that need to be
    # copied from the temporary directory.

    create_file('test.c', r'''
      #include "test.h"

      void my_function()
      {
      }
    ''')
    create_file('test.h', r'''
      void my_function();
    ''')

    self.run_process([EMCC, '-MMD', '-c', 'test.c', '-o', 'test.o'])
    self.assertExists('test.d')
    deps = read_file('test.d')
    # Look for ': ' instead of just ':' to not confuse C:\path\ notation with make "target: deps" rule. Not perfect, but good enough for this test.
    head, tail = deps.split(': ', 2)
    self.assertContained('test.o', head)
    self.assertContained('test.c', tail)
    self.assertContained('test.h', tail)

  def test_dependency_file_2(self):
    shutil.copy(test_file('hello_world.c'), 'a.c')
    self.run_process([EMCC, 'a.c', '-MMD', '-MF', 'test.d', '-c'])
    self.assertContained('a.o: a.c\n', read_file('test.d'))

    shutil.copy(test_file('hello_world.c'), 'a.c')
    self.run_process([EMCC, 'a.c', '-MMD', '-MF', 'test2.d', '-c', '-o', 'test.o'])
    self.assertContained('test.o: a.c\n', read_file('test2.d'))

    shutil.copy(test_file('hello_world.c'), 'a.c')
    ensure_dir('obj')
    self.run_process([EMCC, 'a.c', '-MMD', '-MF', 'test3.d', '-c', '-o', 'obj/test.o'])
    self.assertContained('obj/test.o: a.c\n', read_file('test3.d'))

  def test_compilation_database(self):
    shutil.copy(test_file('hello_world.c'), 'a.c')
    self.run_process([EMCC, 'a.c', '-MJ', 'hello.json', '-c', '-o', 'test.o'])
    self.assertContained('"file": "a.c", "output": "test.o"', read_file('hello.json'))

  def test_js_lib_no_override(self):
    create_file('duplicated_func.c', '''
      #include <stdio.h>
      extern int duplicatedFunc();

      int main() {
        int res = duplicatedFunc();
        printf("*%d*\\n", res);
        return 0;
      }
    ''')
    create_file('duplicated_func_1.js', '''
      addToLibrary(
        { duplicatedFunc: () => 1 },
        { noOverride: true }
      );
    ''')
    create_file('duplicated_func_2.js', '''
      addToLibrary(
        { duplicatedFunc: () => 2 },
        { noOverride: true }
      );
    ''')

    self.emcc_args += ['--js-library', 'duplicated_func_1.js', '--js-library', 'duplicated_func_2.js']
    err = self.expect_fail([EMCC, 'duplicated_func.c'] + self.get_emcc_args())
    self.assertContained('duplicated_func_2.js: Symbol re-definition in JavaScript library: duplicatedFunc. Do not use noOverride if this is intended', err)

  def test_override_stub(self):
    self.do_other_test('test_override_stub.c')

  def test_js_lib_missing_sig(self):
    create_file('some_func.c', '''
      #include <stdio.h>
      extern int someFunc();

      int main() {
        int res = someFunc();
        printf("*%d*\\n", res);
        return 0;
      }
    ''')
    create_file('some_func.js', '''
      addToLibrary(
        { someFunc: () => 1 },
        { checkSig: true }
      );
    ''')

    self.emcc_args += ['--js-library', 'some_func.js']
    err = self.expect_fail([EMCC, 'some_func.c'] + self.get_emcc_args())
    self.assertContained('some_func.js: __sig is missing for function: someFunc. Do not use checkSig if this is intended', err)

  def test_js_lib_extra_args(self):
    # Verify that extra arguments in addition to those listed in `__sig` are still present
    # in the generated JS library function.
    # See https://github.com/emscripten-core/emscripten/issues/21056
    create_file('some_func.js', '''
      addToLibrary({
        someFunc: (arg1, arg2) => {
          err('arg1:' + arg1);
          err('arg2:' + arg2);
        },
        someFunc__sig: 'pp',
      });
    ''')
    create_file('test.c', '''
    void someFunc(long p);
    int main() {
      someFunc(42);
    }
    ''')
    self.emcc_args += ['--js-library', 'some_func.js', '-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=4Gb']
    self.do_runf('test.c', 'arg1:42\narg2:undefined\n')

  def test_js_lib_quoted_key(self):
    create_file('lib.js', r'''
addToLibrary({
  __internal_data:{
    '<' : 0,
    'white space' : 1
  },
  foo__deps: ['__internal_data'],
  foo: () => {
    return 0;
  }
});
''')

    self.do_run_in_out_file_test('hello_world.c', emcc_args=['--js-library', 'lib.js'])

  def test_js_lib_proxying(self):
    # Regression test for a bug we had where jsifier would find and use
    # the inner function in a library function consisting of a single
    # line arrow function.
    # See https://github.com/emscripten-core/emscripten/issues/20264
    create_file('lib.js', r'''
addToLibrary({
  $doNotCall: (x) => {},
  foo__deps: ['$doNotCall'],
  foo__proxy: 'sync',
  foo: () => doNotCall(() => {
    out('should never see this');
  }),
});
''')
    create_file('src.c', r'''
    #include <stdio.h>
    void foo();
    int main() {
      printf("main\n");
      foo();
      printf("done\n");
    }
    ''')
    self.do_runf('src.c', 'main\ndone\n', emcc_args=['-sEXIT_RUNTIME', '-pthread', '-sPROXY_TO_PTHREAD', '--js-library', 'lib.js'])

  def test_js_lib_method_syntax(self):
    create_file('lib.js', r'''
addToLibrary({
  foo() {
    out('foo');
  },
});
''')
    create_file('src.c', r'''
    #include <stdio.h>
    void foo();
    int main() {
      foo();
    }
    ''')
    self.do_runf('src.c', 'foo', emcc_args=['--js-library', 'lib.js'])

  def test_js_lib_exported(self):
    create_file('lib.js', r'''
addToLibrary({
 jslibfunc: (x) => 2 * x
});
''')
    create_file('src.c', r'''
#include <emscripten.h>
#include <stdio.h>
int jslibfunc(int x);
int main() {
  printf("c calling: %d\n", jslibfunc(6));
  EM_ASM({
    out('js calling: ' + Module['_jslibfunc'](5) + '.');
  });
}
''')
    self.do_runf('src.c', 'c calling: 12\njs calling: 10.',
                 emcc_args=['--js-library', 'lib.js', '-sEXPORTED_FUNCTIONS=_main,_jslibfunc'])

  def test_js_lib_using_asm_lib(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__deps: ['asmlibfunc'],
  jslibfunc: (x) => 2 * _asmlibfunc(x),

  asmlibfunc__asm: true,
  asmlibfunc__sig: 'ii',
  asmlibfunc: (x) => {
    x = x | 0;
    return x + 1 | 0;
  }
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc(int x);
int main() {
  printf("c calling: %d\n", jslibfunc(6));
}
''')
    self.do_runf('src.c', 'c calling: 14\n', emcc_args=['--js-library', 'lib.js'])

  def test_js_lib_errors(self):
    create_file('lib.js', '''\
// This is a library file
#endif // line 2
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained('lib.js:2: #endif without matching #if', err)

    create_file('lib.js', '''\
// This is a library file

#else // line 3
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained('lib.js:3: #else without matching #if', err)

  def test_js_internal_deps(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__deps: ['$callRuntimeCallbacks'],
  jslibfunc: (x) => {
    callRuntimeCallbacks();
  },
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc();
int main() {
  printf("c calling: %d\n", jslibfunc());
}
''')
    err = self.run_process([EMCC, 'src.c', '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained("warning: user library symbol 'jslibfunc' depends on internal symbol '$callRuntimeCallbacks'", err)

  def test_js_lib_sig_redefinition(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__sig: 'ii',
  jslibfunc: (x) => {},
});

addToLibrary({
  jslibfunc__sig: 'ii',
  jslibfunc: (x) => {},
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc();
int main() {
  printf("c calling: %d\n", jslibfunc());
}
''')
    err = self.run_process([EMCC, 'src.c', '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained('lib.js: signature redefinition for: jslibfunc__sig', err)

    # Add another redefinition, this time not matching
    create_file('lib2.js', r'''
addToLibrary({
  jslibfunc__sig: 'pp',
  jslibfunc: (x) => {},
});
''')
    err = self.expect_fail([EMCC, 'src.c', '--js-library', 'lib.js', '--js-library', 'lib2.js'])
    self.assertContained('lib2.js: signature redefinition for: jslibfunc__sig. (old=ii vs new=pp)', err)

  def test_js_lib_invalid_deps(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__deps: 'hello',
  jslibfunc: (x) => {},
});
''')

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained('lib.js: JS library directive jslibfunc__deps=hello is of type \'string\', but it should be an array', err)

    create_file('lib2.js', r'''
addToLibrary({
  jslibfunc__deps: [1,2,3],
  jslibfunc: (x) => {},
});
''')

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib2.js'])
    self.assertContained("lib2.js: __deps entries must be of type 'string' or 'function' not 'number': jslibfunc__deps", err)

  def test_js_lib_invalid_decorator(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__async: 'hello',
  jslibfunc: (x) => {},
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained("lib.js: Decorator (jslibfunc__async} has wrong type. Expected 'boolean' not 'string'", err)

  def test_js_lib_i53abi(self):
    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc__i53abi: true,
  jslibfunc: (x) => { return 42 },
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=jslibfunc', '--js-library', 'lib.js'])
    self.assertContained("error: JS library error: '__i53abi' decorator requires '__sig' decorator: 'jslibfunc'", err)

    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc__i53abi: true,
  jslibfunc__sig: 'ii',
  jslibfunc: (x) => { return 42 },
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=jslibfunc', '--js-library', 'lib.js'])
    self.assertContained("error: JS library error: '__i53abi' only makes sense when '__sig' includes 'j' (int64): 'jslibfunc'", err)

  def test_js_lib_legacy(self):
    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc: (x) => { return 42 },
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc();
int main() {
  printf("main: %d\n", jslibfunc());
}
''')
    self.do_runf('src.c', 'main: 42\n', emcc_args=['--js-library', 'lib.js'])

  # Tests that users can pass custom JS options from command line using
  # the -jsDfoo=val syntax:
  # See https://github.com/emscripten-core/emscripten/issues/10580.
  def test_js_lib_custom_settings(self):
    self.emcc_args += ['--js-library', test_file('core/test_custom_js_settings.js'), '-jsDCUSTOM_JS_OPTION=1']
    self.do_other_test('test_js_lib_custom_settings.c')

    self.assertContained('cannot change built-in settings values with a -jsD directive', self.expect_fail([EMCC, '-jsDWASM=0']))

  def test_EMCC_BUILD_DIR(self):
    # EMCC_BUILD_DIR env var contains the dir we were building in, when running the js compiler (e.g. when
    # running a js library). We force the cwd to be src/ for technical reasons, so this lets you find out
    # where you were.
    create_file('lib.js', r'''
printErr('dir was ' + process.env.EMCC_BUILD_DIR);
''')
    err = self.run_process([EMXX, test_file('hello_world.c'), '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained('dir was ' + os.path.realpath(os.path.normpath(self.get_dir())), err)

  def test_float_h(self):
    process = self.run_process([EMCC, test_file('float+.c')], stdout=PIPE, stderr=PIPE)
    assert process.returncode == 0, 'float.h should agree with our system: ' + process.stdout + '\n\n\n' + process.stderr

  def test_output_is_dir(self):
    ensure_dir('out_dir')
    err = self.expect_fail([EMCC, '-c', test_file('hello_world.c'), '-o', 'out_dir/'])
    self.assertContained('error: unable to open output file', err)

  def test_doublestart_bug(self):
    create_file('code.c', r'''
#include <stdio.h>
#include <emscripten.h>

void main_loop(void) {
    static int cnt = 0;
    if (++cnt >= 10) emscripten_cancel_main_loop();
}

int main(void) {
    printf("This should only appear once.\n");
    emscripten_set_main_loop(main_loop, 10, 0);
    return 0;
}
''')

    create_file('pre.js', r'''
Module["preRun"] = () => {
  addRunDependency('test_run_dependency');
  removeRunDependency('test_run_dependency');
};
''')

    self.run_process([EMCC, 'code.c', '--pre-js', 'pre.js'])
    output = self.run_js('a.out.js')

    self.assertEqual(output.count('This should only appear once.'), 1)

  def test_module_print(self):
    create_file('code.c', r'''
#include <stdio.h>
int main(void) {
  printf("123456789\n");
  return 0;
}
''')

    create_file('pre.js', r'''
Module.print = (x) => { throw '<{(' + x + ')}>' };
''')

    self.run_process([EMCC, 'code.c', '--pre-js', 'pre.js'])
    output = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained(r'<{(123456789)}>', output)

  def test_precompiled_headers_warnings(self):
    # Check that we don't have any underlying warnings from clang, this can happen if we
    # pass any link flags to when building a pch.
    create_file('header.h', '#define X 5\n')
    self.run_process([EMCC, '-Werror', '-xc++-header', 'header.h'])

  @parameterized({
    'gch': ['gch'],
    'pch': ['pch'],
  })
  def test_precompiled_headers(self, suffix):
    create_file('header.h', '#define X 5\n')
    self.run_process([EMCC, '-xc++-header', 'header.h', '-c'])
    self.assertExists('header.h.pch') # default output is pch
    if suffix != 'pch':
      self.run_process([EMCC, '-xc++-header', 'header.h', '-o', 'header.h.' + suffix])
      self.assertBinaryEqual('header.h.pch', 'header.h.' + suffix)

    create_file('src.cpp', r'''
#include <stdio.h>
int main() {
  printf("|%d|\n", X);
  return 0;
}
''')
    self.run_process([EMCC, 'src.cpp', '-include', 'header.h'])

    output = self.run_js('a.out.js')
    self.assertContained('|5|', output)

    # also verify that the gch is actually used
    err = self.run_process([EMXX, 'src.cpp', '-include', 'header.h', '-Xclang', '-print-stats'], stderr=PIPE).stderr
    self.assertTextDataContained('*** PCH/Modules Loaded:\nModule: header.h.pch', err)
    # and sanity check it is not mentioned when not
    delete_file('header.h.' + suffix)
    err = self.run_process([EMXX, 'src.cpp', '-include', 'header.h', '-Xclang', '-print-stats'], stderr=PIPE).stderr
    self.assertNotContained('*** PCH/Modules Loaded:\nModule: header.h.' + suffix, err.replace('\r\n', '\n'))

    # with specified target via -o
    delete_file('header.h.' + suffix)
    self.run_process([EMCC, '-xc++-header', 'header.h', '-o', 'my.' + suffix])
    self.assertExists('my.' + suffix)

    # -include-pch flag
    self.run_process([EMCC, '-xc++-header', 'header.h', '-o', 'header.h.' + suffix])
    self.run_process([EMXX, 'src.cpp', '-include-pch', 'header.h.' + suffix])
    output = self.run_js('a.out.js')
    self.assertContained('|5|', output)

  def test_LEGACY_VM_SUPPORT(self):
    # when modern features are lacking, we can polyfill them or at least warn
    create_file('pre.js', 'Math.imul = undefined;')

    def test(expected, opts):
      print(opts)
      self.run_process([EMCC, test_file('hello_world.c'), '--pre-js', 'pre.js'] + opts)
      self.assertContained(expected, self.run_js('a.out.js', assert_returncode=0 if opts else NON_ZERO))

    # when legacy is needed, we show an error indicating so
    test('build with LEGACY_VM_SUPPORT', [])
    # legacy + disabling wasm works
    test('hello, world!', ['-sLEGACY_VM_SUPPORT', '-sWASM=0'])

  def test_on_abort(self):
    expected_output = 'Module.onAbort was called'

    def add_on_abort_and_verify(extra=''):
      js = read_file('a.out.js')
      with open('a.out.js', 'w') as f:
        f.write("var Module = { onAbort: () => console.log('%s') };\n" % expected_output)
        f.write(extra + '\n')
        f.write(js)
      self.assertContained(expected_output, self.run_js('a.out.js', assert_returncode=NON_ZERO))

    # test direct abort() C call

    create_file('src.c', '''
        #include <stdlib.h>
        int main() {
          abort();
        }
      ''')
    self.run_process([EMCC, 'src.c', '-sWASM_ASYNC_COMPILATION=0'])
    add_on_abort_and_verify()

    # test direct abort() JS call

    create_file('src.c', '''
        #include <emscripten.h>
        int main() {
          EM_ASM({ abort() });
        }
      ''')
    self.run_process([EMCC, 'src.c', '-sWASM_ASYNC_COMPILATION=0'])
    add_on_abort_and_verify()

    # test throwing in an abort handler, and catching that

    create_file('src.c', '''
        #include <emscripten.h>
        int main() {
          EM_ASM({
            try {
              out('first');
              abort();
            } catch (e) {
              out('second');
              abort();
              throw e;
            }
          });
        }
      ''')
    self.run_process([EMCC, 'src.c', '-sWASM_ASYNC_COMPILATION=0'])
    js = read_file('a.out.js')
    with open('a.out.js', 'w') as f:
      f.write("var Module = { onAbort: () => { console.log('%s'); throw 're-throw'; } };\n" % expected_output)
      f.write(js)
    out = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    print(out)
    self.assertContained(expected_output, out)
    self.assertContained('re-throw', out)
    self.assertContained('first', out)
    self.assertContained('second', out)
    self.assertEqual(out.count(expected_output), 2)

    # test an abort during startup
    self.run_process([EMCC, test_file('hello_world.c')])
    os.remove('a.out.wasm') # trigger onAbort by intentionally causing startup to fail
    add_on_abort_and_verify()

  @also_with_wasm2js
  @parameterized({
    '': ([],),
    '01': (['-O1', '-g2'],),
    'O2': (['-O2', '-g2', '-flto'],),
  })
  def test_no_exit_runtime(self, opts):
    create_file('code.cpp', r'''
#include <stdio.h>

template<int x>
struct Waste {
  Waste() {
    printf("coming around %d\n", x);
  }
  ~Waste() {
    printf("going away %d\n", x);
  }
};

Waste<1> w1;
Waste<2> w2;
Waste<3> w3;
Waste<4> w4;
Waste<5> w5;

int main(int argc, char **argv) {
  return 0;
}
    ''')

    for no_exit in (1, 0):
      print(no_exit)
      cmd = [EMXX] + opts + ['code.cpp', '-sEXIT_RUNTIME=' + str(1 - no_exit)] + self.get_emcc_args()
      if self.is_wasm():
        cmd += ['--profiling-funcs'] # for function names
      self.run_process(cmd)
      output = self.run_js('a.out.js')
      src = read_file('a.out.js')
      if self.is_wasm():
        src += '\n' + self.get_wasm_text('a.out.wasm')
      exit = 1 - no_exit
      print('  exit:', exit)
      self.assertContained('coming around', output)
      self.assertContainedIf('going away', output, exit)
      # The wasm backend uses atexit to register destructors when
      # constructors are called  There is currently no way to exclude
      # these destructors from the wasm binary.
      # TODO(sbc): Re-enabled these assertions once the wasm backend
      # is able to eliminate these.
      # assert ('atexit(' in src) == exit, 'atexit should not appear in src when EXIT_RUNTIME=0'
      # assert ('_ZN5WasteILi2EED' in src) == exit, 'destructors should not appear if no exit:\n' + src

  def test_no_exit_runtime_warnings_flush(self):
    # check we warn if there is unflushed info
    create_file('code.c', r'''
#include <stdio.h>
#include <emscripten/emscripten.h>
int main(int argc, char **argv) {
  printf("hello\n");
  printf("world"); // no newline, not flushed
#if FLUSH
  printf("\n");
#endif
#if KEEPALIVE
  emscripten_exit_with_live_runtime();
#endif
}
''')
    create_file('code.cpp', r'''
#include <iostream>
#include <emscripten/emscripten.h>
int main() {
  using namespace std;
  cout << "hello" << std::endl;
  cout << "world"; // no newline, not flushed
#if FLUSH
  std::cout << std::endl;
#endif
#if KEEPALIVE
  emscripten_exit_with_live_runtime();
#endif
}
''')
    warning = 'warning: stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1'

    def test(cxx, no_exit, assertions, flush=0, keepalive=0, filesystem=1):
      if cxx:
        cmd = [EMXX, 'code.cpp']
      else:
        cmd = [EMCC, 'code.c']
      print('%s: no_exit=%d assertions=%d flush=%d keepalive=%d filesystem=%d' % (cmd[1], no_exit, assertions, flush, keepalive, filesystem))
      cmd += ['-sEXIT_RUNTIME=%d' % (1 - no_exit), '-sASSERTIONS=%d' % assertions]
      if flush:
        cmd += ['-DFLUSH']
      if keepalive:
        cmd += ['-DKEEPALIVE']
      if not filesystem:
        cmd += ['-sNO_FILESYSTEM']
      self.run_process(cmd)
      output = self.run_js('a.out.js')
      exit = 1 - no_exit
      self.assertContained('hello', output)
      self.assertContainedIf('world', output, exit or flush)
      self.assertContainedIf(warning, output, no_exit and assertions and not flush and not keepalive)

    # Run just one test with KEEPALIVE set.  In this case we don't expect to see any kind
    # of warning because we are explicitly requesting the runtime stay alive for later use.
    test(cxx=0, no_exit=1, assertions=1, keepalive=1)
    test(cxx=0, no_exit=1, assertions=1, filesystem=0)

    for cxx in (0, 1):
      for no_exit in (0, 1):
        for assertions in (0, 1):
          for flush in (0, 1):
            test(cxx, no_exit, assertions, flush)

  def test_extra_opt_levels(self):
    # Opt levels that we don't tend to test elsewhere
    for opt in ('-Og', '-Ofast'):
      print(opt)
      proc = self.run_process([EMCC, '-v', test_file('hello_world.c'), opt], stderr=PIPE)
      self.assertContained(opt, proc.stderr)
      self.assertContained('hello, world!', self.run_js('a.out.js'))

  @parameterized({
    '': [[]],
    'O1': [['-O1']],
  })
  def test_fs_after_main(self, args):
    self.do_runf('test_fs_after_main.c', 'Test passed.')

  def test_oz_size(self):
    sizes = {}
    for name, args in [
        ('0', []),
        ('1', ['-O1']),
        ('2', ['-O2']),
        ('s', ['-Os']),
        ('z', ['-Oz']),
        ('3', ['-O3']),
        ('g', ['-Og']),
      ]:
      print(name, args)
      self.clear()
      self.run_process([EMCC, '-c', path_from_root('system/lib/dlmalloc.c')] + args)
      sizes[name] = os.path.getsize('dlmalloc.o')
    print(sizes)
    opt_min = min(sizes['1'], sizes['2'], sizes['3'], sizes['s'], sizes['z'], sizes['g'])
    opt_max = max(sizes['1'], sizes['2'], sizes['3'], sizes['s'], sizes['z'], sizes['g'])
    # 'opt builds are all fairly close'
    self.assertLess(opt_min - opt_max, opt_max * 0.1)
    # unopt build is quite larger'
    self.assertGreater(sizes['0'], (1.20 * opt_max))

  @disabled('relies on fastcomp EXIT_RUNTIME=0 optimization not implemented/disabled')
  def test_global_inits(self):
    create_file('inc.h', r'''
#include <stdio.h>

template<int x>
struct Waste {
  int state;
  Waste() : state(10) {}
  void test(int a) {
    printf("%d\n", a + state);
  }
  ~Waste() {
    printf("going away %d\n", x);
  }
};

Waste<3> *getMore();
''')

    create_file('main.cpp', r'''
#include "inc.h"

Waste<1> mw1;
Waste<2> mw2;

int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  mw1.state += argc;
  mw2.state += argc;
  mw1.test(5);
  mw2.test(6);
  getMore()->test(0);
  return 0;
}
''')

    create_file('side.cpp', r'''
#include "inc.h"

Waste<3> sw3;

Waste<3> *getMore() {
  return &sw3;
}
''')

    for opts, has_global in [
      (['-O2', '-g', '-sEXIT_RUNTIME'], True),
      # no-exit-runtime removes the atexits, and then globalgce can work
      # it's magic to remove the global initializer entirely
      (['-O2', '-g'], False),
      (['-Os', '-g', '-sEXIT_RUNTIME'], True),
      (['-Os', '-g'], False),
      (['-O2', '-g', '-flto', '-sEXIT_RUNTIME'], True),
      (['-O2', '-g', '-flto'], False),
    ]:
      print(opts, has_global)
      self.run_process([EMXX, 'main.cpp', '-c'] + opts)
      self.run_process([EMXX, 'side.cpp', '-c'] + opts)
      self.run_process([EMCC, 'main.o', 'side.o'] + opts)
      self.run_js('a.out.js')
      src = read_file('a.out.js')
      self.assertContained('argc: 1\n16\n17\n10\n', self.run_js('a.out.js'))
      self.assertContainedIf('globalCtors', src, has_global)

  def test_implicit_func(self):
    # EMCC makes -Wimplicit-function-declaration an error by default in all modes. Upstream LLVM
    # emits a warning in gnu89 mode, but otherwise emcc's behavior is identical to upstream.
    IMPLICIT_C89 = "error: implicit declaration of function 'strnlen'"
    # Also check for -Wincompatible-function-pointer-types (it became an error in LLVM 16)
    INCOMPATIBLE = ': incompatible function pointer types'

    stderr = self.expect_fail(
        [EMCC, path_from_root('test/other/test_implicit_func.c'), '-c', '-o', 'implicit_func.o', '-std=gnu89'])
    self.assertContained(IMPLICIT_C89, stderr)
    self.assertContained(INCOMPATIBLE, stderr)

  @requires_native_clang
  @crossplatform
  def test_bad_triple(self):
    # compile a minimal program, with as few dependencies as possible, as
    # native building on CI may not always work well
    create_file('minimal.c', 'int main() { return 0; }')
    self.run_process([CLANG_CC, 'minimal.c', '-target', 'x86_64-linux', '-c', '-emit-llvm', '-o', 'a.bc'] + clang_native.get_clang_native_args(), env=clang_native.get_clang_native_env())
    err = self.expect_fail([EMCC, '-Werror', 'a.bc'])
    self.assertContained('error: overriding the module target triple with wasm32-unknown-emscripten [-Werror,-Woverride-module]', err)

  def test_valid_abspath(self):
    # Test whether abspath warning appears
    abs_include_path = os.path.abspath(self.get_dir())
    err = self.run_process([EMCC, '-I%s' % abs_include_path, '-Wwarn-absolute-paths', test_file('hello_world.c')], stderr=PIPE).stderr
    warning = '-I or -L of an absolute path "-I%s" encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript).' % abs_include_path
    self.assertContained(warning, err)

    # Passing an absolute path to a directory inside the emscripten tree is always ok and should not issue a warning.
    abs_include_path = TEST_ROOT
    err = self.run_process([EMCC, '-I%s' % abs_include_path, '-Wwarn-absolute-paths', test_file('hello_world.c')], stderr=PIPE).stderr
    warning = '-I or -L of an absolute path "-I%s" encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript).' % abs_include_path
    self.assertNotContained(warning, err)

    # Hide warning for this include path
    err = self.run_process([EMCC, '--valid-abspath', abs_include_path, '-I%s' % abs_include_path, '-Wwarn-absolute-paths', test_file('hello_world.c')], stderr=PIPE).stderr
    self.assertNotContained(warning, err)

  def test_valid_abspath_2(self):
    if WINDOWS:
      abs_include_path = 'C:\\nowhere\\at\\all'
    else:
      abs_include_path = '/nowhere/at/all'
    cmd = [EMCC, test_file('hello_world.c'), '--valid-abspath', abs_include_path, '-I%s' % abs_include_path]
    self.run_process(cmd)
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_warn_dylibs(self):
    shared_suffixes = ['.so', '.dylib', '.dll']

    for suffix in ('.o', '.bc', '.so', '.dylib', '.js', '.html'):
      print(suffix)
      cmd = [EMCC, test_file('hello_world.c'), '-o', 'out' + suffix]
      if suffix in ['.o', '.bc']:
        cmd.append('-c')
      if suffix in ['.dylib', '.so']:
        cmd.append('-shared')
      err = self.run_process(cmd, stderr=PIPE).stderr
      warning = 'linking a library with `-shared` will emit a static object file'
      self.assertContainedIf(warning, err, suffix in shared_suffixes)

  @crossplatform
  @parameterized({
    'O2': [['-O2']],
    'O3': [['-O3']],
  })
  @parameterized({
    '': [1],
    'wasm2js': [0],
    'wasm2js_2': [2],
  })
  def test_symbol_map(self, opts, wasm):
    def get_symbols_lines(symbols_file):
      self.assertTrue(os.path.isfile(symbols_file), "Symbols file %s isn't created" % symbols_file)
      # check that the map is correct
      symbols = read_file(symbols_file)
      lines = [line.split(':') for line in symbols.strip().split('\n')]
      return lines

    def get_minified_middle(symbols_file):
      minified_middle = None
      for minified, full in get_symbols_lines(symbols_file):
        # handle both fastcomp and wasm backend notation
        if full == 'middle':
          minified_middle = minified
          break
      return minified_middle

    def guess_symbols_file_type(symbols_file):
      for _minified, full in get_symbols_lines(symbols_file):
        # define symbolication file by JS specific entries
        if full in ['FUNCTION_TABLE', 'HEAP32']:
          return 'js'
      return 'wasm'

    UNMINIFIED_HEAP8 = 'var HEAP8 = new '
    UNMINIFIED_MIDDLE = 'function middle'

    self.clear()
    create_file('src.c', r'''
#include <emscripten.h>

EM_JS(int, run_js, (), {
out(new Error().stack);
return 0;
});

EMSCRIPTEN_KEEPALIVE
void middle() {
if (run_js()) {
  // fake recursion that is never reached, to avoid inlining in binaryen and LLVM
  middle();
}
}

int main() {
EM_ASM({ _middle() });
}
''')
    cmd = [EMCC, 'src.c', '--emit-symbol-map'] + opts
    if wasm != 1:
      cmd.append(f'-sWASM={wasm}')
    self.run_process(cmd)

    minified_middle = get_minified_middle('a.out.js.symbols')
    self.assertNotEqual(minified_middle, None, "Missing minified 'middle' function")
    if wasm:
      # stack traces are standardized enough that we can easily check that the
      # minified name is actually in the output
      stack_trace_reference = 'wasm-function[%s]' % minified_middle
      out = self.run_js('a.out.js')
      self.assertContained(stack_trace_reference, out)
      # make sure there are no symbols in the wasm itself
      wat = self.get_wasm_text('a.out.wasm')
      for func_start in ('(func $middle', '(func $_middle'):
        self.assertNotContained(func_start, wat)

    # Ensure symbols file type according to `-sWASM=` mode
    if wasm == 0:
      self.assertEqual(guess_symbols_file_type('a.out.js.symbols'), 'js', 'Primary symbols file should store JS mappings')
    elif wasm == 1:
      self.assertEqual(guess_symbols_file_type('a.out.js.symbols'), 'wasm', 'Primary symbols file should store Wasm mappings')
    elif wasm == 2:
      # special case when both JS and Wasm targets are created
      minified_middle_2 = get_minified_middle('a.out.wasm.js.symbols')
      self.assertNotEqual(minified_middle_2, None, "Missing minified 'middle' function")
      self.assertEqual(guess_symbols_file_type('a.out.js.symbols'), 'wasm', 'Primary symbols file should store Wasm mappings')
      self.assertEqual(guess_symbols_file_type('a.out.wasm.js.symbols'), 'js', 'Secondary symbols file should store JS mappings')
    return

    # check we don't keep unnecessary debug info with wasm2js when emitting
    # a symbol map
    if wasm == 0 and '-O' in str(opts):
      js = read_file('a.out.js')
      self.assertNotContained(UNMINIFIED_HEAP8, js)
      self.assertNotContained(UNMINIFIED_MIDDLE, js)
      # verify those patterns would exist with more debug info
      self.run_process(cmd + ['--profiling-funcs'])
      js = read_file('a.out.js')
      self.assertContained(UNMINIFIED_HEAP8, js)
      self.assertContained(UNMINIFIED_MIDDLE, js)

  @parameterized({
    '': [[]],
    # bigint support is interesting to test here because it changes which
    # binaryen tools get run, which can affect how debug info is kept around
    'bigint': [['-sWASM_BIGINT']],
  })
  def test_symbol_map_output_size(self, args):
    # build with and without a symbol map and verify that the sizes are the
    # same. using a symbol map should add the map on the side, but not increase
    # the build size.
    # -Oz is used here to run as many optimizations as possible, to check for
    # any difference in how the optimizer operates
    self.run_process([EMCC, test_file('hello_world.c'), '-Oz', '-o', 'test1.js'] + args)
    self.run_process([EMCC, test_file('hello_world.c'), '-Oz', '-o', 'test2.js', '--emit-symbol-map'] + args)

    self.assertEqual(os.path.getsize('test1.js'), os.path.getsize('test2.js'))

    def get_code_section_size(filename):
      with webassembly.Module(filename) as module:
        return module.get_section(webassembly.SecType.CODE).size

    self.assertEqual(get_code_section_size('test1.wasm'), get_code_section_size('test2.wasm'))

  def test_bitcode_linking(self):
    # emcc used to be able to link bitcode together, but these days partial linking
    # always outputs an object file.
    self.run_process([EMCC, '-flto', '-c', test_file('hello_world.c')])
    self.assertExists('hello_world.o')
    self.run_process([EMCC, '-flto', '-r', 'hello_world.o', '-o', 'hello_world2.o'])
    is_bitcode('hello_world.o')
    building.is_wasm('hello_world2.o')

  @parameterized({
    '': (True, False),
    'safe_heap': (True, True),
    'wasm2js': (False, False),
    'wasm2js_safe_heap': (False, True),
  })
  def test_bad_function_pointer_cast(self, wasm, safe):
    create_file('src.cpp', r'''
#include <stdio.h>

typedef int (*callback) (int, ...);

int impl(int foo) {
  printf("Hello, world.\n");
  return 0;
}

int main() {
  volatile callback f = (callback) impl;
  f(0); /* This fails with or without additional arguments. */
  return 0;
}
''')

    for opts in (0, 1, 2):
      for emulate_casts in (0, 1):
        for relocatable in (0, 1):
          # wasm2js is not compatible with relocatable mode
          if not wasm and relocatable:
            continue
          cmd = [EMXX, 'src.cpp', '-O' + str(opts)]
          if not wasm:
            cmd += ['-sWASM=0']
          if safe:
            cmd += ['-sSAFE_HEAP']
          if emulate_casts:
            cmd += ['-sEMULATE_FUNCTION_POINTER_CASTS']
          if relocatable:
            cmd += ['-sRELOCATABLE'] # disables asm-optimized safe heap
          print(cmd)
          self.run_process(cmd)
          returncode = 0 if emulate_casts or not wasm else NON_ZERO
          output = self.run_js('a.out.js', assert_returncode=returncode)
          if emulate_casts or wasm == 0:
            # success!
            self.assertContained('Hello, world.', output)
          else:
            # otherwise, the error depends on the mode we are in
            # wasm trap raised by the vm
            self.assertContained('function signature mismatch', output)

  def test_bad_export(self):
    for m in ('', ' '):
      self.clear()
      cmd = [EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS=["' + m + '_main"]']
      print(cmd)
      stderr = self.run_process(cmd, stderr=PIPE, check=False).stderr
      if m:
        self.assertContained('undefined exported symbol: " _main"', stderr)
      else:
        self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_no_dynamic_execution(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-O1', '-sDYNAMIC_EXECUTION=0'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))
    src = read_file('a.out.js')
    self.assertNotContained('eval(', src)
    self.assertNotContained('eval.', src)
    self.assertNotContained('new Function', src)
    delete_file('a.out.js')

    # Test that --preload-file doesn't add an use of eval().
    create_file('temp.txt', "foo\n")
    self.run_process([EMCC, test_file('hello_world.c'), '-O1',
                      '-sDYNAMIC_EXECUTION=0', '--preload-file', 'temp.txt'])
    src = read_file('a.out.js')
    self.assertNotContained('eval(', src)
    self.assertNotContained('eval.', src)
    self.assertNotContained('new Function', src)
    delete_file('a.out.js')

    # Test that -sDYNAMIC_EXECUTION=0 and -sRELOCATABLE are allowed together.
    self.do_runf('hello_world.c', emcc_args=['-O1', '-sDYNAMIC_EXECUTION=0', '-sRELOCATABLE'])

    create_file('test.c', r'''
      #include <emscripten/emscripten.h>
      int main() {
        emscripten_run_script("console.log('hello from script');");
        return 0;
      }
      ''')

    # Test that emscripten_run_script() aborts when -sDYNAMIC_EXECUTION=0
    self.run_process([EMCC, 'test.c', '-O1', '-sDYNAMIC_EXECUTION=0'])
    self.assertContained('DYNAMIC_EXECUTION=0 was set, cannot eval', self.run_js('a.out.js', assert_returncode=NON_ZERO))
    delete_file('a.out.js')

    # Test that emscripten_run_script() posts a warning when -sDYNAMIC_EXECUTION=2
    self.run_process([EMCC, 'test.c', '-O1', '-sDYNAMIC_EXECUTION=2'])
    self.assertContained('Warning: DYNAMIC_EXECUTION=2 was set, but calling eval in the following location:', self.run_js('a.out.js'))
    self.assertContained('hello from script', self.run_js('a.out.js'))
    delete_file('a.out.js')

  def test_init_file_at_offset(self):
    create_file('src.c', r'''
      #include <stdio.h>
      int main() {
        int data = 0x12345678;
        FILE *f = fopen("test.dat", "wb");
        fseek(f, 100, SEEK_CUR);
        fwrite(&data, 4, 1, f);
        fclose(f);

        int data2;
        f = fopen("test.dat", "rb");
        fread(&data2, 4, 1, f); // should read 0s, not that int we wrote at an offset
        printf("read: %d\n", data2);
        fseek(f, 0, SEEK_END);
        long size = ftell(f); // should be 104, not 4
        fclose(f);
        printf("file size is %ld\n", size);
      }
    ''')
    self.do_runf('src.c', 'read: 0\nfile size is 104\n')

  @no_mac("TODO: investigate different Node FS semantics on Mac")
  @no_windows("TODO: investigate different Node FS semantics on Windows")
  @also_with_wasmfs_all_backends
  def test_unlink(self):
    self.do_other_test('test_unlink.cpp')

  @crossplatform
  def test_argv0_node(self):
    create_file('code.c', r'''
#include <stdio.h>
int main(int argc, char **argv) {
  printf("I am %s.\n", argv[0]);
  return 0;
}
''')

    output = self.do_runf('code.c')
    self.assertContained('I am ' + utils.normalize_path(os.path.realpath(self.get_dir())) + '/code.js', utils.normalize_path(output))

  @parameterized({
    'no_exit_runtime': [True],
    '': [False],
  })
  def test_returncode(self, no_exit):
    create_file('src.c', r'''
      #include <stdio.h>
      #include <stdlib.h>
      int main() {
      #if CALL_EXIT
        exit(CODE);
      #else
        return CODE;
      #endif
      }
    ''')
    for code in (0, 123):
      for call_exit in (0, 1):
        for async_compile in (0, 1):
          self.run_process([EMCC, 'src.c', '-sENVIRONMENT=node,shell', '-DCODE=%d' % code, '-sEXIT_RUNTIME=%d' % (1 - no_exit), '-DCALL_EXIT=%d' % call_exit, '-sWASM_ASYNC_COMPILATION=%d' % async_compile])
          for engine in config.JS_ENGINES:
            # async compilation can't return a code in d8
            if async_compile and engine == config.V8_ENGINE:
              continue
            print(code, call_exit, async_compile, engine)
            proc = self.run_process(engine + ['a.out.js'], stderr=PIPE, check=False)
            msg = 'but keepRuntimeAlive() is set (counter=0) due to an async operation, so halting execution but not exiting the runtime'
            if no_exit and call_exit:
              self.assertContained(msg, proc.stderr)
            else:
              self.assertNotContained(msg, proc.stderr)
            # we always emit the right exit code, whether we exit the runtime or not
            self.assertEqual(proc.returncode, code)

  def test_emscripten_force_exit_NO_EXIT_RUNTIME(self):
    create_file('src.c', r'''
      #include <emscripten.h>
      int main() {
      #if CALL_EXIT
        emscripten_force_exit(0);
      #endif
      }
    ''')
    for no_exit in (0, 1):
      for call_exit in (0, 1):
        self.run_process([EMCC, 'src.c', '-sEXIT_RUNTIME=%d' % (1 - no_exit), '-DCALL_EXIT=%d' % call_exit])
        print(no_exit, call_exit)
        out = self.run_js('a.out.js')
        assert ('emscripten_force_exit cannot actually shut down the runtime, as the build does not have EXIT_RUNTIME set' in out) == (no_exit and call_exit), out

  def test_mkdir_silly(self):
    create_file('src.c', r'''
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("\n");
  for (int i = 1; i < argc; i++) {
    printf("%d:\n", i);
    int ok = mkdir(argv[i], S_IRWXU|S_IRWXG|S_IRWXO);
    printf("  make %s: %d\n", argv[i], ok);
    DIR *dir = opendir(argv[i]);
    printf("  open %s: %d\n", argv[i], dir != NULL);
    if (dir) {
      struct dirent *entry;
      while ((entry = readdir(dir))) {
        printf("  %s, %d\n", entry->d_name, entry->d_type);
      }
    }
  }
}
    ''')
    self.run_process([EMCC, 'src.c'])

    # cannot create /, can open
    self.assertContained(r'''
1:
  make /: -1
  open /: 1
  ., 4
  .., 4
  tmp, 4
  home, 4
  dev, 4
  proc, 4
''', self.run_js('a.out.js', args=['/']))
    # cannot create empty name, cannot open
    self.assertContained(r'''
1:
  make : -1
  open : 0
''', self.run_js('a.out.js', args=['']))
    # can create unnormalized path, can open
    self.assertContained(r'''
1:
  make /a//: 0
  open /a//: 1
  ., 4
  .., 4
''', self.run_js('a.out.js', args=['/a//']))
    # can create child unnormalized
    self.assertContained(r'''
1:
  make /a: 0
  open /a: 1
  ., 4
  .., 4
2:
  make /a//b//: 0
  open /a//b//: 1
  ., 4
  .., 4
''', self.run_js('a.out.js', args=['/a', '/a//b//']))

  def test_stat_silly(self):
    create_file('src.c', r'''
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    const char *path = argv[i];
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
      printf("Failed to stat path: '%s'; errno=%d\n", path, errno);
    } else {
      printf("stat success on '%s'\n", path);
    }
  }
}
    ''')
    self.do_runf('src.c', r'''Failed to stat path: '/a'; errno=44
Failed to stat path: ''; errno=44
''', args=['/a', ''])

  def test_symlink_silly(self):
    create_file('src.c', r'''
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
  if (symlink(argv[1], argv[2]) != 0) {
    printf("Failed to symlink paths: %s, %s; errno=%d\n", argv[1], argv[2], errno);
  } else {
    printf("ok\n");
  }
}
    ''')
    self.run_process([EMCC, 'src.c'])

    # cannot symlink nonexistents
    self.assertContained(r'Failed to symlink paths: , abc; errno=44', self.run_js('a.out.js', args=['', 'abc']))
    self.assertContained(r'Failed to symlink paths: , ; errno=44', self.run_js('a.out.js', args=['', '']))
    self.assertContained(r'ok', self.run_js('a.out.js', args=['123', 'abc']))
    self.assertContained(r'Failed to symlink paths: abc, ; errno=44', self.run_js('a.out.js', args=['abc', '']))

  def test_rename_silly(self):
    create_file('src.c', r'''
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv) {
  if (rename(argv[1], argv[2]) != 0) {
    printf("Failed to rename paths: %s, %s; errno=%d\n", argv[1], argv[2], errno);
  } else {
    printf("ok\n");
  }
}
''')
    self.run_process([EMCC, 'src.c'])

    # cannot symlink nonexistents
    self.assertContained(r'Failed to rename paths: , abc; errno=44', self.run_js('a.out.js', args=['', 'abc']))
    self.assertContained(r'Failed to rename paths: , ; errno=44', self.run_js('a.out.js', args=['', '']))
    self.assertContained(r'Failed to rename paths: 123, abc; errno=44', self.run_js('a.out.js', args=['123', 'abc']))
    self.assertContained(r'Failed to rename paths: abc, ; errno=44', self.run_js('a.out.js', args=['abc', '']))

  def test_readdir_r_silly(self):
    create_file('src.cpp', r'''
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
using std::endl;
namespace
{
  void check(const bool result)
  {
    if(not result) {
      std::cout << "Check failed!" << endl;
      throw "bad";
    }
  }
  // Do a recursive directory listing of the directory whose path is specified
  // by \a name.
  void ls(const std::string& name, std::size_t indent = 0)
  {
    ::DIR *dir;
    struct ::dirent *entry;
    if(indent == 0) {
      std::cout << name << endl;
      ++indent;
    }
    // Make sure we can open the directory.  This should also catch cases where
    // the empty string is passed in.
    if (not (dir = ::opendir(name.c_str()))) {
      const int error = errno;
      std::cout
        << "Failed to open directory: " << name << "; " << error << endl;
      return;
    }
    // Just checking the sanity.
    if (name.empty()) {
      std::cout
        << "Managed to open a directory whose name was the empty string.."
        << endl;
      check(::closedir(dir) != -1);
      return;
    }
    // Iterate over the entries in the directory.
    while ((entry = ::readdir(dir))) {
      const std::string entryName(entry->d_name);
      if (entryName == "." || entryName == "..") {
        // Skip the dot entries.
        continue;
      }
      const std::string indentStr(indent * 2, ' ');
      if (entryName.empty()) {
        std::cout
          << indentStr << "\"\": Found empty string as a "
          << (entry->d_type == DT_DIR ? "directory" : "file")
          << " entry!" << endl;
        continue;
      } else {
        std::cout << indentStr << entryName
                  << (entry->d_type == DT_DIR ? "/" : "") << endl;
      }
      if (entry->d_type == DT_DIR) {
        // We found a subdirectory; recurse.
        ls(std::string(name + (name == "/" ? "" : "/" ) + entryName),
           indent + 1);
      }
    }
    // Close our handle.
    check(::closedir(dir) != -1);
  }
  void touch(const std::string &path)
  {
    const int fd = ::open(path.c_str(), O_CREAT | O_TRUNC, 0644);
    check(fd != -1);
    check(::close(fd) != -1);
  }
}
int main()
{
  check(::mkdir("dir", 0755) == 0);
  touch("dir/a");
  touch("dir/b");
  touch("dir/c");
  touch("dir/d");
  touch("dir/e");
  std::cout << "Before:" << endl;
  ls("dir");
  std::cout << endl;
  // Attempt to delete entries as we walk the (single) directory.
  ::DIR * const dir = ::opendir("dir");
  check(dir != NULL);
  struct ::dirent *entry;
  while((entry = ::readdir(dir)) != NULL) {
    const std::string name(entry->d_name);
    // Skip "." and "..".
    if(name == "." || name == "..") {
      continue;
    }
    // Unlink it.
    std::cout << "Unlinking " << name << endl;
    check(::unlink(("dir/" + name).c_str()) != -1);
  }
  check(::closedir(dir) != -1);
  std::cout << "After:" << endl;
  ls("dir");
  std::cout << endl;
  return 0;
}
    ''')
    # cannot symlink nonexistents
    self.do_runf('src.cpp', r'''Before:
dir
  a
  b
  c
  d
  e

Unlinking a
Unlinking b
Unlinking c
Unlinking d
Unlinking e
After:
dir
''', args=['', 'abc'])

  def test_emversion(self):
    create_file('src.c', r'''
      #include <stdio.h>
      #include <emscripten/version.h>
      int main() {
        printf("major: %d\n", __EMSCRIPTEN_major__);
        printf("minor: %d\n", __EMSCRIPTEN_minor__);
        printf("tiny: %d\n", __EMSCRIPTEN_tiny__);
      }
    ''')
    expected = '''\
major: %d
minor: %d
tiny: %d
''' % (utils.EMSCRIPTEN_VERSION_MAJOR, utils.EMSCRIPTEN_VERSION_MINOR, utils.EMSCRIPTEN_VERSION_TINY)
    self.do_runf('src.c', expected)
    self.do_runf('src.c', expected, emcc_args=['-sSTRICT'])

  def test_libc_files_without_syscalls(self):
    # a program which includes FS due to libc js library support, but has no syscalls,
    # so full FS support would normally be optimized out
    create_file('src.c', r'''
#include <sys/time.h>
#include <stddef.h>
int main() {
  return utimes(NULL, NULL);
}''')
    self.run_process([EMCC, 'src.c'])

  def test_syscall_no_filesystem(self):
    # a program which includes a non-trivial syscall, but disables the filesystem.
    create_file('src.c', r'''
#include <sys/time.h>
#include <fcntl.h>
int main() {
  return openat(0, "foo", 0);
}''')
    self.run_process([EMCC, 'src.c', '-sNO_FILESYSTEM'])

  def test_dylink_no_filesystem(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-sMAIN_MODULE=2', '-sNO_FILESYSTEM'])

  def test_dylink_4gb_max(self):
    # Test main module with 4GB of memory. we need to emit a "maximum"
    # clause then, even though 4GB is the maximum; see
    # https://github.com/emscripten-core/emscripten/issues/14130
    self.set_setting('MAIN_MODULE', '1')
    self.set_setting('ALLOW_MEMORY_GROWTH', '1')
    self.set_setting('MAXIMUM_MEMORY', '4GB')
    self.do_runf('hello_world.c')

  def test_dashS(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-S'])
    self.assertExists('hello_world.s')

  def assertIsLLVMAsm(self, filename):
    bitcode = read_file(filename)
    self.assertContained('target triple = "', bitcode)

  def test_dashS_ll_input(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-S', '-emit-llvm'])
    self.assertIsLLVMAsm('hello_world.ll')
    self.run_process([EMCC, 'hello_world.ll', '-S', '-emit-llvm', '-o', 'another.ll'])
    self.assertIsLLVMAsm('another.ll')

  def test_dashS_stdout(self):
    stdout = self.run_process([EMCC, test_file('hello_world.c'), '-S', '-o', '-'], stdout=PIPE).stdout
    self.assertEqual(os.listdir('.'), [])
    self.assertContained('hello_world.c', stdout)

  def test_emit_llvm_asm(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-S', '-emit-llvm'])
    self.assertIsLLVMAsm('hello_world.ll')

  def test_emit_llvm(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-c', '-emit-llvm'])
    self.assertTrue(is_bitcode('hello_world.bc'))

  def test_compile_ll_file(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-S', '-emit-llvm'])
    err = self.run_process([EMCC, '-v', '-c', 'hello_world.ll', '-o', 'hello_world.o'], stderr=PIPE).stderr
    # Verify that `-mllvm` flags are passed when compiling `.ll` files.
    self.assertContained('-mllvm -enable-emscripten-sjlj', err)
    self.run_process([EMCC, 'hello_world.o'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_dashE(self):
    create_file('src.cpp', r'''#include <emscripten.h>
__EMSCRIPTEN_major__ __EMSCRIPTEN_minor__ __EMSCRIPTEN_tiny__ EMSCRIPTEN_KEEPALIVE
''')

    def test(args):
      print(args)
      out = self.run_process([EMXX, 'src.cpp', '-E'] + args, stdout=PIPE).stdout
      self.assertContained('%d %d %d __attribute__((used))' % (utils.EMSCRIPTEN_VERSION_MAJOR, utils.EMSCRIPTEN_VERSION_MINOR, utils.EMSCRIPTEN_VERSION_TINY), out)

    test([])
    test(['-lembind'])

  def test_dashE_respect_dashO(self):
    # issue #3365
    with_dash_o = self.run_process([EMCC, test_file('hello_world.c'), '-E', '-o', 'ignored.js'], stdout=PIPE, stderr=PIPE).stdout
    without_dash_o = self.run_process([EMCC, test_file('hello_world.c'), '-E'], stdout=PIPE, stderr=PIPE).stdout
    self.assertEqual(len(with_dash_o), 0)
    self.assertNotEqual(len(without_dash_o), 0)

  def test_dashM(self):
    out = self.run_process([EMCC, test_file('hello_world.c'), '-M'], stdout=PIPE).stdout
    self.assertContained('hello_world.o:', out) # Verify output is just a dependency rule instead of bitcode or js

  def test_dashM_respect_dashO(self):
    # issue #3365
    with_dash_o = self.run_process([EMCC, test_file('hello_world.c'), '-M', '-o', 'ignored.js'], stdout=PIPE).stdout
    without_dash_o = self.run_process([EMCC, test_file('hello_world.c'), '-M'], stdout=PIPE).stdout
    self.assertEqual(len(with_dash_o), 0)
    self.assertNotEqual(len(without_dash_o), 0)

  def test_malloc_implicit(self):
    self.do_other_test('test_malloc_implicit.cpp')

  def test_switch64phi(self):
    # issue 2539, fastcomp segfault on phi-i64 interaction
    create_file('src.cpp', r'''
#include <cstdint>
#include <limits>
#include <cstdio>

//============================================================================

namespace
{
  class int_adapter {
  public:
    typedef ::int64_t int_type;

    int_adapter(int_type v = 0)
      : value_(v) {}
    static const int_adapter pos_infinity() {
      return (::std::numeric_limits<int_type>::max)();
    }
    static const int_adapter neg_infinity() {
      return (::std::numeric_limits<int_type>::min)();
    }
    static const int_adapter not_a_number() {
      return (::std::numeric_limits<int_type>::max)()-1;
    }
    static bool is_neg_inf(int_type v) {
      return (v == neg_infinity().as_number());
    }
    static bool is_pos_inf(int_type v) {
      return (v == pos_infinity().as_number());
    }
    static bool is_not_a_number(int_type v) {
      return (v == not_a_number().as_number());
    }

    bool is_infinity() const {
      return (value_ == neg_infinity().as_number() ||
              value_ == pos_infinity().as_number());
    }
    bool is_special() const {
      return(is_infinity() || value_ == not_a_number().as_number());
    }
    bool operator<(const int_adapter& rhs) const {
      if(value_ == not_a_number().as_number()
         || rhs.value_ == not_a_number().as_number()) {
        return false;
      }
      if(value_ < rhs.value_) return true;
      return false;
    }
    int_type as_number() const {
      return value_;
    }

    int_adapter operator-(const int_adapter& rhs) const {
      if(is_special() || rhs.is_special()) {
        if (rhs.is_pos_inf(rhs.as_number())) {
          return int_adapter(1);
        }
        if (rhs.is_neg_inf(rhs.as_number())) {
          return int_adapter();
        }
      }
      return int_adapter();
    }


  private:
    int_type value_;
  };

  class time_iterator {
  public:
    time_iterator(int_adapter t, int_adapter d)
      : current_(t),
        offset_(d)
    {}

    time_iterator& operator--() {
      current_ = int_adapter(current_ - offset_);
      return *this;
    }

    bool operator>=(const int_adapter& t) {
      return not (current_ < t);
    }

  private:
    int_adapter current_;
    int_adapter offset_;
  };

  void iterate_backward(const int_adapter *answers, const int_adapter& td) {
    int_adapter end = answers[0];
    time_iterator titr(end, td);

    std::puts("");
    for (; titr >= answers[0]; --titr) {
    }
  }
}

int main() {
  const int_adapter answer1[] = {};
  iterate_backward(NULL, int_adapter());
  iterate_backward(answer1, int_adapter());
}
    ''')
    self.run_process([EMXX, 'src.cpp', '-O2', '-sSAFE_HEAP'])

  def test_bad_lookup(self):
    self.do_runf(path_from_root('test/filesystem/bad_lookup.cpp'), expected_output='ok')

  @parameterized({
    'none': [{'EMCC_FORCE_STDLIBS': None}, False],
    # forced libs is ok, they were there anyhow
    'normal': [{'EMCC_FORCE_STDLIBS': 'libc,libc++abi,libc++'}, False],
    # partial list, but ok since we grab them as needed
    'parial': [{'EMCC_FORCE_STDLIBS': 'libc++'}, False],
    # fail! not enough stdlibs
    'partial_only': [{'EMCC_FORCE_STDLIBS': 'libc++,libc,libc++abi', 'EMCC_ONLY_FORCED_STDLIBS': '1'}, True],
    # force all the needed stdlibs, so this works even though we ignore the input file
    'full_only': [{'EMCC_FORCE_STDLIBS': 'libc,libc++abi,libc++,libmalloc', 'EMCC_ONLY_FORCED_STDLIBS': '1'}, False],
  })
  def test_only_force_stdlibs(self, env, fail):
    cmd = [EMXX, test_file('hello_libcxx.cpp')]
    with env_modify(env):
      if fail:
        err = self.expect_fail(cmd)
        self.assertContained('undefined symbol: emscripten_builtin_memalign', err)
      else:
        err = self.run_process(cmd, stderr=PIPE).stderr
        if 'EMCC_ONLY_FORCED_STDLIBS' in env:
          self.assertContained('EMCC_ONLY_FORCED_STDLIBS is deprecated', err)
        else:
          self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_only_force_stdlibs_2(self):
    create_file('src.cpp', r'''
#include <iostream>
#include <stdexcept>

int main()
{
  try {
    throw std::exception();
    std::cout << "got here" << std::endl;
  }
  catch (const std::exception& ex) {
    std::cout << "Caught exception: " << ex.what() << std::endl;
  }
}
''')
    with env_modify({'EMCC_FORCE_STDLIBS': 'libc,libc++abi,libc++,libmalloc', 'EMCC_ONLY_FORCED_STDLIBS': '1'}):
      self.run_process([EMXX, 'src.cpp', '-sDISABLE_EXCEPTION_CATCHING=0'])
    self.assertContained('Caught exception: std::exception', self.run_js('a.out.js'))

  @with_env_modify({'EMCC_FORCE_STDLIBS': '1'})
  def test_force_stdlibs(self):
    self.do_runf('hello_world.c')
    # See https://github.com/emscripten-core/emscripten/issues/22161
    self.do_runf('hello_world.c', emcc_args=['-sWASM_BIGINT'])

  @crossplatform
  def test_strftime_zZ(self):
    if MACOS:
      self.skipTest('setting LC_ALL is not compatible with macOS python')

    tz_lang_infos = [
      {'env': {'LC_ALL': 'en_GB', 'TZ': 'Europe/London'}, 'expected_utc': 'UTC+0100'},
      {'env': {'LC_ALL': 'th_TH', 'TZ': 'Asia/Bangkok'}, 'expected_utc': 'UTC+0700'},
      {'env': {'LC_ALL': 'ar-AE', 'TZ': 'Asia/Dubai'}, 'expected_utc': 'UTC+0400'},
      {'env': {'LC_ALL': 'en-US', 'TZ': 'America/Los_Angeles'}, 'expected_utc': 'UTC-0700'}
    ]

    for tz_lang_info in tz_lang_infos:
      with env_modify(tz_lang_info['env']):
        self.do_runf('other/test_strftime_zZ.c', 'The current timezone is: %s' % (tz_lang_info['expected_utc']))

  def test_strptime_symmetry(self):
    self.do_other_test('test_strptime_symmetry.c')

  @also_with_wasmfs
  def test_truncate_from_0(self):
    create_file('src.cpp', r'''
#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::endl;

//============================================================================
// :: Helpers

namespace
{
  // Returns the size of the regular file specified as 'path'.
  ::off_t getSize(const char* const path)
  {
    // Stat the file and make sure that it's the expected size.
    struct ::stat path_stat;
    if (::stat(path, &path_stat) != 0) {
      const int error = errno;
      std::cout
        << "Failed to lstat path: " << path << "; errno=" << error << "; "
        << std::strerror(error) << endl;
      return -1;
    }

    std::cout
      << "Size of file is: " << path_stat.st_size << endl;
    return path_stat.st_size;
  }

  // Causes the regular file specified in 'path' to have a size of 'length'
  // bytes.
  void resize(const char* const path,
              const ::off_t length)
  {
    std::cout
      << "Truncating file=" << path << " to length=" << length << endl;
    if (::truncate(path, length) == -1)
    {
      const int error = errno;
      std::cout
        << "Failed to truncate file=" << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
    }

    const ::off_t size = getSize(path);
    if (size != length) {
      std::cout
        << "Failed to truncate file=" << path << " to length=" << length
        << "; got size=" << size << endl;
    }
  }

  // Helper to create a file with the given content.
  void createFile(const std::string& path, const std::string& content)
  {
    std::cout
      << "Creating file: " << path << " with content=" << content << endl;

    const int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
      const int error = errno;
      std::cout
        << "Failed to open file for writing: " << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
      return;
    }

    if (::write(fd, content.c_str(), content.size()) != content.size()) {
      const int error = errno;
      std::cout
        << "Failed to write content=" << content << " to file=" << path
        << "; errno=" << error << "; " << std::strerror(error) << endl;

      // Fall through to close FD.
    }

    ::close(fd);
  }
}

//============================================================================
// :: Entry Point
int main()
{
  const char* const file = "/tmp/file";
  createFile(file, "This is some content");
  getSize(file);
  resize(file, 32);
  resize(file, 17);
  resize(file, 0);

  // This throws a JS exception.
  resize(file, 32);
  return 0;
}
''')
    self.do_runf('src.cpp', r'''Creating file: /tmp/file with content=This is some content
Size of file is: 20
Truncating file=/tmp/file to length=32
Size of file is: 32
Truncating file=/tmp/file to length=17
Size of file is: 17
Truncating file=/tmp/file to length=0
Size of file is: 0
Truncating file=/tmp/file to length=32
Size of file is: 32
''')

  def test_create_readonly(self):
    create_file('src.cpp', r'''
#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

using std::endl;

//============================================================================
// :: Helpers

namespace
{
  // Helper to create a read-only file with content.
  void readOnlyFile(const std::string& path, const std::string& content)
  {
    std::cout
      << "Creating file: " << path << " with content of size="
      << content.size() << endl;

    const int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, 0400);
    if (fd == -1) {
      const int error = errno;
      std::cout
        << "Failed to open file for writing: " << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
      return;
    }

    // Write the content to the file.
    ssize_t result = 0;
    if ((result = ::write(fd, content.data(), content.size()))
        != ssize_t(content.size()))
    {
      const int error = errno;
      std::cout
        << "Failed to write to file=" << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
      // Fall through to close the file.
    }
    else {
      std::cout
        << "Data written to file=" << path << "; successfully wrote "
        << result << " bytes" << endl;
    }

    ::close(fd);
  }
}

//============================================================================
// :: Entry Point

int main() {
  const char* const file = "/tmp/file";
  unlink(file);
  readOnlyFile(file, "This content should get written because the file "
                     "does not yet exist and so, only the mode of the "
                     "containing directory will influence my ability to "
                     "create and open the file. The mode of the file only "
                     "applies to opening of the stream, not subsequent stream "
                     "operations after stream has opened.\n\n");
  readOnlyFile(file, "This should not get written because the file already "
                     "exists and is read-only.\n\n");
}
''')
    self.do_runf('src.cpp', r'''Creating file: /tmp/file with content of size=292
Data written to file=/tmp/file; successfully wrote 292 bytes
Creating file: /tmp/file with content of size=79
Failed to open file for writing: /tmp/file; errno=2; Permission denied
''')

  def test_embed_file_large(self):
    # If such long files are encoded on one line,
    # they overflow the interpreter's limit
    large_size = int(1500000)
    create_file('large.txt', 'x' * large_size)
    create_file('src.c', r'''
      #include <stdio.h>
      #include <unistd.h>
      int main() {
          FILE* fp = fopen("large.txt", "r");
          if (fp) {
              printf("ok\n");
              fseek(fp, 0L, SEEK_END);
              printf("%ld\n", ftell(fp));
          } else {
              printf("failed to open large file.txt\n");
          }
          return 0;
      }
    ''')
    self.run_process([EMCC, 'src.c', '--embed-file', 'large.txt'])
    for engine in config.JS_ENGINES:
      if engine == config.V8_ENGINE:
        continue # ooms
      print(engine)
      self.assertContained('ok\n' + str(large_size) + '\n', self.run_js('a.out.js', engine=engine))

  def test_force_exit(self):
    create_file('src.c', r'''
#include <emscripten/console.h>
#include <emscripten/emscripten.h>

void callback() {
  emscripten_out("callback pre()");
  emscripten_force_exit(42);
  emscripten_out("callback post()");
}

int main() {
  emscripten_async_call(callback, NULL, 100);
  emscripten_exit_with_live_runtime();
  return 123;
}
''')
    self.run_process([EMCC, 'src.c'])
    output = self.run_js('a.out.js', assert_returncode=42)
    self.assertContained('callback pre()', output)
    self.assertNotContained('callback post()', output)

  def test_bad_locale(self):
    create_file('src.c', r'''
#include <locale.h>
#include <stdio.h>
#include <wctype.h>

int main(int argc, char **argv) {
  const char *locale = (argc > 1 ? argv[1] : "C");
  const char *actual = setlocale(LC_ALL, locale);
  if (actual == NULL) {
    printf("%s locale not supported\n", locale);
    return 0;
  }
  printf("locale set to %s: %s\n", locale, actual);
}
''')
    self.run_process([EMCC, 'src.c'])

    self.assertContained('locale set to C: C',
                         self.run_js('a.out.js', args=['C']))
    self.assertContained('locale set to waka: waka',
                         self.run_js('a.out.js', args=['waka']))

  def test_browser_language_detection(self):
    # Test HTTP Accept-Language parsing by simulating navigator.languages #8751
    self.run_process([EMCC,
                      test_file('test_browser_language_detection.c')])
    self.assertContained('C.UTF-8', self.run_js('a.out.js'))

    # Accept-Language: fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3
    create_file('preamble.js', r'''navigator = {};
      navigator.languages = [ "fr", "fr-FR", "en-US", "en" ];''')
    self.run_process([EMCC, '--pre-js', 'preamble.js',
                      test_file('test_browser_language_detection.c')])
    self.assertContained('fr.UTF-8', self.run_js('a.out.js'))

    # Accept-Language: fr-FR,fr;q=0.8,en-US;q=0.5,en;q=0.3
    create_file('preamble.js', r'''navigator = {};
      navigator.languages = [ "fr-FR", "fr", "en-US", "en" ];''')
    self.emcc_args += ['--pre-js', 'preamble.js']
    self.do_runf('test_browser_language_detection.c', 'fr_FR.UTF-8')

  def test_js_main(self):
    # try to add a main() from JS, at runtime. this is not supported (the
    # compiler needs to know at compile time about main).
    create_file('pre_main.js', r'''
      Module['_main'] = () => {};
    ''')
    create_file('src.cpp', '')
    self.emcc_args += ['--pre-js', 'pre_main.js']
    self.do_runf('src.cpp', 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]', assert_returncode=NON_ZERO)

  def test_locale_wrong(self):
    create_file('src.cpp', r'''
#include <locale>
#include <iostream>
#include <stdexcept>

int main(const int argc, const char * const * const argv) {
  const char * const name = argc > 1 ? argv[1] : "C";

  try {
    const std::locale locale(name);
    std::cout
      << "Constructed locale \"" << name << "\"\n"
      << "This locale is "
      << (locale == std::locale::global(locale) ? "" : "not ")
      << "the global locale.\n"
      << "This locale is " << (locale == std::locale::classic() ? "" : "not ")
      << "the C locale." << std::endl;

  } catch(const std::runtime_error &ex) {
    std::cout
      << "Can't construct locale \"" << name << "\": " << ex.what()
      << std::endl;
    return 1;

  } catch(...) {
    std::cout
      << "FAIL: Unexpected exception constructing locale \"" << name << '\"'
      << std::endl;
    return 127;
  }
}
''')
    self.run_process([EMXX, 'src.cpp', '-sDISABLE_EXCEPTION_CATCHING=0'])
    self.assertContained('''\
Constructed locale "C"
This locale is the global locale.
This locale is the C locale.
''', self.run_js('a.out.js', args=['C']))
    self.assertContained('''\
Constructed locale "waka"
This locale is not the global locale.
This locale is not the C locale.
''', self.run_js('a.out.js', args=['waka']))

  def test_cleanup_os(self):
    # issue 2644
    def test(args, be_clean):
      print(args)
      self.clear()
      shutil.copy(test_file('hello_world.c'), 'a.c')
      create_file('b.c', ' ')
      self.run_process([EMCC, 'a.c', 'b.c'] + args)
      clutter = glob.glob('*.o')
      if be_clean:
        assert len(clutter) == 0, 'should not leave clutter ' + str(clutter)
      else:
         assert len(clutter) == 2, 'should leave .o files'
    test(['-o', 'c.so', '-r'], True)
    test(['-o', 'c.js'], True)
    test(['-o', 'c.html'], True)
    test(['-c'], False)

  def test_dash_g_object(self):
    def test(opts):
      print(opts)
      self.run_process([EMCC, '-c', test_file('hello_world.c'), '-o', 'a_.o'] + opts)
      sizes = {'_': os.path.getsize('a_.o')}
      self.run_process([EMCC, '-c', test_file('hello_world.c'), '-g', '-o', 'ag.o'] + opts)
      sizes['g'] = os.path.getsize('ag.o')
      for i in range(5):
        self.run_process([EMCC, '-c', test_file('hello_world.c'), '-g' + str(i), '-o', 'a' + str(i) + '.o'] + opts)
        sizes[i] = os.path.getsize('a' + str(i) + '.o')
      print('  ', sizes)
      assert sizes['_'] == sizes[0] == sizes[1] == sizes[2], 'no debug means no llvm debug info ' + str(sizes)
      assert sizes['g'] == sizes[3] == sizes[4], '-g or -gsource-map means llvm debug info ' + str(sizes)
      assert sizes['_'] < sizes['g'], 'llvm debug info has positive size ' + str(sizes)
    test([])
    test(['-O1'])

  def test_no_filesystem(self):
    FS_MARKER = 'var FS'
    # fopen forces full filesystem support
    self.run_process([EMCC, test_file('hello_world_fopen.c'), '-sASSERTIONS=0'])
    yes_size = os.path.getsize('a.out.js')
    self.assertContained('hello, world!', self.run_js('a.out.js'))
    self.assertContained(FS_MARKER, read_file('a.out.js'))
    self.run_process([EMCC, test_file('hello_world.c'), '-sASSERTIONS=0'])
    no_size = os.path.getsize('a.out.js')
    self.assertContained('hello, world!', self.run_js('a.out.js'))
    self.assertNotContained(FS_MARKER, read_file('a.out.js'))
    print('yes fs, no fs:', yes_size, no_size)
    # ~100K of FS code is removed
    self.assertGreater(yes_size - no_size, 90000)
    self.assertLess(no_size, 360000)

  def test_no_filesystem_libcxx(self):
    self.set_setting('FILESYSTEM', 0)
    self.do_runf('hello_libcxx.cpp', 'hello, world!')

  @is_slow_test
  def test_no_nuthin(self):
    # check FILESYSTEM is automatically set, and effective

    def test(opts, absolute):
      print('opts, absolute:', opts, absolute)
      sizes = {}

      def do(name, source, moar_opts):
        self.clear()
        # pad the name to a common length so that doesn't effect the size of the
        # output
        padded_name = name + '_' * (20 - len(name))
        self.run_process([EMCC, test_file(source), '-o', padded_name + '.js'] + self.get_emcc_args() + opts + moar_opts)
        sizes[name] = os.path.getsize(padded_name + '.js')
        if os.path.exists(padded_name + '.wasm'):
          sizes[name] += os.path.getsize(padded_name + '.wasm')
        self.assertContained('hello, world!', self.run_js(padded_name + '.js'))

      do('normal', 'hello_world_fopen.c', [])
      do('no_fs', 'hello_world.c', []) # without fopen, we should auto-detect we do not need full fs support and can do FILESYSTEM=0
      do('no_fs_manual', 'hello_world.c', ['-sFILESYSTEM=0'])
      print('  ', sizes)
      self.assertLess(sizes['no_fs'], sizes['normal'])
      self.assertLess(sizes['no_fs'], absolute)
      # manual can usually remove a tiny bit more
      self.assertLess(sizes['no_fs_manual'], sizes['no_fs'] + 30)

    test(['-sASSERTIONS=0'], 120000) # we don't care about code size with assertions
    test(['-O1'], 91000)
    test(['-O2'], 46000)
    test(['-O3', '--closure=1'], 17000)
    # js too
    # -Wclosure is needed due to
    # https://github.com/google/closure-compiler/issues/4108
    test(['-O3', '--closure=1', '-Wno-closure', '-sWASM=0'], 36000)
    test(['-O3', '--closure=2', '-Wno-closure', '-sWASM=0'], 33000) # might change now and then

  def test_no_main_loop(self):
    MAINLOOP = 'var MainLoop'

    self.run_process([EMCC, test_file('hello_world.c')])
    self.assertNotContained(MAINLOOP, read_file('a.out.js'))

    # uses emscripten_set_main_loop, which needs MainLoop
    self.run_process([EMCC, test_file('browser_main_loop.c')])
    self.assertContained(MAINLOOP, read_file('a.out.js'))

  def test_EXPORTED_RUNTIME_METHODS(self):
    def test(opts, has, not_has):
      print(opts, has, not_has)
      self.clear()
      # check without assertions, as with assertions we add stubs for the things we remove (which
      # print nice error messages)
      self.run_process([EMCC, test_file('hello_world.c'), '-sASSERTIONS=0'] + opts)
      self.assertContained('hello, world!', self.run_js('a.out.js'))
      src = read_file('a.out.js')
      self.assertContained(has, src)
      self.assertNotContained(not_has, src)

    test([], "Module['", "Module['waka")
    test(['-sEXPORTED_RUNTIME_METHODS=[]'], '', "Module['addRunDependency")
    test(['-sEXPORTED_RUNTIME_METHODS=addRunDependency'], "Module['addRunDependency", "Module['waka")
    test(['-sEXPORTED_RUNTIME_METHODS=[]', '-sEXPORTED_RUNTIME_METHODS=addRunDependency'], "Module['addRunDependency", "Module['waka")

  def test_stat_fail_alongtheway(self):
    self.do_other_test('test_stat_fail_alongtheway.c')

  def test_link_with_a_static(self):
    create_file('x.c', r'''
int init_weakref(int a, int b) {
  return a + b;
}
''')
    create_file('y.c', r'''
static int init_weakref(void) { // inlined in -O2, not in -O0 where it shows up in llvm-nm as 't'
  return 150;
}

int testy(void) {
  return init_weakref();
}
''')
    create_file('z.c', r'''
extern int init_weakref(int, int);
extern int testy(void);

int main(void) {
  return testy() + init_weakref(5, 6);
}
''')
    self.run_process([EMCC, '-c', 'x.c', '-o', 'x.o'])
    self.run_process([EMCC, '-c', 'y.c', '-o', 'y.o'])
    self.run_process([EMCC, '-c', 'z.c', '-o', 'z.o'])
    delete_file('libtest.a')
    self.run_process([EMAR, 'rc', 'libtest.a', 'y.o'])
    self.run_process([EMAR, 'rc', 'libtest.a', 'x.o'])
    self.run_process([EMRANLIB, 'libtest.a'])

    for args in ([], ['-O2']):
      print('args:', args)
      self.run_process([EMCC, 'z.o', 'libtest.a'] + args)
      self.run_js('a.out.js', assert_returncode=161)

  def test_link_with_bad_o_in_a(self):
    # when building a .a, we force-include all the objects inside it. but, some
    # may not be valid bitcode, e.g. if it contains metadata or something else
    # weird. we should just ignore those
    self.run_process([EMCC, '-c', test_file('hello_world.c'), '-o', 'hello_world.o'])
    create_file('bad.obj', 'this is not a good file, it should be ignored!')
    self.run_process([LLVM_AR, 'cr', 'libfoo.a', 'hello_world.o', 'bad.obj'])
    self.run_process([EMCC, 'libfoo.a'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  @requires_node
  def test_require(self):
    inname = test_file('hello_world.c')
    self.emcc(inname, args=['-sASSERTIONS=0'], output_filename='a.out.js')
    create_file('run.js', 'require("./a.out.js")')
    output = self.run_js('run.js')
    self.assertEqual('hello, world!\n', output)

  @requires_node
  def test_require_modularize(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sASSERTIONS=0'])
    src = read_file('a.out.js')
    self.assertContained('module.exports = Module;', src)
    create_file('run.js', 'var m = require("./a.out.js"); m();')
    output = self.run_js('run.js')
    self.assertEqual(output, 'hello, world!\n')
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sEXPORT_NAME="NotModule"', '-sASSERTIONS=0'])
    self.assertContained('module.exports = NotModule;', read_file('a.out.js'))
    output = self.run_js('run.js')
    self.assertEqual(output, 'hello, world!\n')
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE'])
    # We call require() twice to ensure it returns wrapper function each time
    create_file('require_twice.js', 'require("./a.out.js")();var m = require("./a.out.js"); m();')
    output = self.run_js('require_twice.js')
    self.assertEqual(output, 'hello, world!\nhello, world!\n')

  def test_modularize_strict(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sSTRICT'])
    create_file('run.js', 'var m = require("./a.out.js"); m();')
    output = self.run_js('run.js')
    self.assertEqual(output, 'hello, world!\n')

  @parameterized({
    '': ([],),
    'export_name': (['-sEXPORT_NAME=Foo'],),
    'closure': (['-sEXPORT_NAME=Foo', '--closure=1'],),
  })
  @crossplatform
  def test_modularize_incoming(self, args):
    self.run_process([EMCC, test_file('hello_world.c'), '-o', 'out.mjs'] + self.get_emcc_args() + args)
    create_file('run.mjs', '''
    import Module from './out.mjs';
    await Module({onRuntimeInitialized: () => console.log('done init')})
      .then(() => console.log('got module'));
    ''')
    output = self.run_js('run.mjs')
    self.assertContained('done init\nhello, world!\ngot module\n', output)

  @crossplatform
  @node_pthreads
  @flaky('https://github.com/emscripten-core/emscripten/issues/19683')
  # The flakiness of this test is very high on macOS so just disable it
  # completely.
  @no_mac('https://github.com/emscripten-core/emscripten/issues/19683')
  def test_pthread_print_override_modularize(self):
    self.set_setting('EXPORT_NAME', 'Test')
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.set_setting('MODULARIZE')
    create_file('main.c', '''
      #include <emscripten/console.h>

      int main() {
        emscripten_out("hello, world!");
        return 0;
      }
    ''')
    create_file('main.js', '''
      const Test = require('./test.js');

      async function main() {
        await Test({
          // world -> earth
          print: (text) => console.log(text.replace('world', 'earth'))
        });
      }
      main();
    ''')

    self.emcc('main.c', output_filename='test.js')
    output = self.run_js('main.js')
    self.assertNotContained('hello, world!', output)
    self.assertContained('hello, earth!', output)

  def test_define_modularize(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sASSERTIONS=0'])
    src = 'var module = 0; ' + read_file('a.out.js')
    create_file('a.out.js', src)
    self.assertContained("define([], () => Module);", src)

    create_file('run_module.js', '''
var m;
(global.define = (deps, factory) => { m = factory(); }).amd = true;
require("./a.out.js");
m();
''')
    output = self.run_js('run_module.js')
    self.assertContained('hello, world!\n', output)

    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sEXPORT_NAME="NotModule"', '-sASSERTIONS=0'])
    src = 'var module = 0; ' + read_file('a.out.js')
    create_file('a.out.js', src)
    self.assertContained("define([], () => NotModule);", src)

    output = self.run_js('run_module.js')
    self.assertContained('hello, world!\n', output)

  def test_EXPORT_NAME_with_html(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-o', 'a.html', '-sEXPORT_NAME=Other'])
    self.assertContained('error: customizing EXPORT_NAME requires that the HTML be customized to use that name', err)

  def test_modularize_sync_compilation(self):
    create_file('post.js', r'''
console.log('before');
var result = Module();
// It should be an object.
console.log('typeof result: ' + typeof result);
// And it should have the exports that Module has, showing it is Module in fact.
console.log('typeof _main: ' + typeof result._main);
// And it should not be a Promise.
console.log('typeof result.then: ' + typeof result.then);
console.log('after');
''')
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-sMODULARIZE',
                      '-sWASM_ASYNC_COMPILATION=0',
                      '--extern-post-js', 'post.js'])
    self.assertContained('''\
before
hello, world!
typeof result: object
typeof _main: function
typeof result.then: undefined
after
''', self.run_js('a.out.js'))

  def test_export_all_3142(self):
    create_file('src.cpp', r'''
typedef unsigned int Bit32u;

struct S_Descriptor {
    Bit32u limit_0_15   :16;
    Bit32u base_0_15    :16;
    Bit32u base_16_23   :8;
};

class Descriptor {
public:
    Descriptor() { saved.fill[0] = saved.fill[1] = 0; }
    union {
        S_Descriptor seg;
        Bit32u fill[2];
    } saved;
};

Descriptor desc;
    ''')
    self.run_process([EMXX, 'src.cpp', '-O2', '-sEXPORT_ALL'])
    self.assertExists('a.out.js')

  def test_emmake_emconfigure(self):
    def check(what, args, fail=True, expect=''):
      args = [what] + args
      print(what, args, fail, expect)
      output = self.run_process(args, stdout=PIPE, stderr=PIPE, check=False)
      assert ('is a helper for' in output.stderr) == fail
      assert ('Typical usage' in output.stderr) == fail
      self.assertContained(expect, output.stdout)
    check(emmake, [])
    check(EMCONFIGURE, [])
    check(emmake, ['--version'])
    check(EMCONFIGURE, ['--version'])
    check(emmake, ['make'], fail=False)
    check(EMCONFIGURE, ['configure'], fail=False)
    check(EMCONFIGURE, ['./configure'], fail=False)
    check(EMCMAKE, ['cmake'], fail=False)

    create_file('test.py', '''
import os
print(os.environ.get('CROSS_COMPILE'))
''')
    check(EMCONFIGURE, [PYTHON, 'test.py'], expect=path_from_root('em'), fail=False)
    check(emmake, [PYTHON, 'test.py'], expect=path_from_root('em'), fail=False)

    create_file('test.py', '''
import os
print(os.environ.get('NM'))
''')
    check(EMCONFIGURE, [PYTHON, 'test.py'], expect=shared.LLVM_NM, fail=False)

    create_file('test.c', 'int main() { return 0; }')
    os.mkdir('test_cache')
    with env_modify({'EM_CACHE': os.path.abspath('test_cache')}):
      check(EMCONFIGURE, [EMCC, 'test.c'], fail=False)

  def test_emmake_python(self):
    # simulates a configure/make script that looks for things like CC, AR, etc., and which we should
    # not confuse by setting those vars to something containing `python X` as the script checks for
    # the existence of an executable.
    self.run_process([emmake, PYTHON, test_file('emmake/make.py')])

  @crossplatform
  def test_sdl2_config(self):
    for args, expected in [
      [['--version'], '2.0.10'],
      [['--cflags'], '-sUSE_SDL=2'],
      [['--libs'], '-sUSE_SDL=2'],
      [['--cflags', '--libs'], '-sUSE_SDL=2'],
    ]:
      print(args, expected)
      out = self.run_process([shared.bat_suffix(cache.get_sysroot_dir('bin/sdl2-config'))] + args,
                             stdout=PIPE, stderr=PIPE).stdout
      self.assertContained(expected, out)
      print('via emmake')
      out = self.run_process([emmake, 'sdl2-config'] + args, stdout=PIPE, stderr=PIPE).stdout
      self.assertContained(expected, out)

  def test_module_onexit(self):
    create_file('src.c', r'''
#include <emscripten.h>
int main() {
  EM_ASM({
    Module.onExit = (status) => out('exiting now, status ' + status);
  });
  return 14;
}
''')
    self.run_process([EMCC, 'src.c', '-sEXIT_RUNTIME'])
    self.assertContained('exiting now, status 14', self.run_js('a.out.js', assert_returncode=14))

  def test_NO_aliasing(self):
    # the NO_ prefix flips boolean options
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXIT_RUNTIME'])
    exit_1 = read_file('a.out.js')
    self.run_process([EMCC, test_file('hello_world.c'), '-sNO_EXIT_RUNTIME=0'])
    no_exit_0 = read_file('a.out.js')
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXIT_RUNTIME=0'])
    exit_0 = read_file('a.out.js')

    assert exit_1 == no_exit_0
    assert exit_1 != exit_0

  def test_underscore_exit(self):
    create_file('src.c', r'''
#include <unistd.h>
int main() {
  _exit(0); // should not end up in an infinite loop with non-underscore exit
}
''')
    self.run_process([EMCC, 'src.c'])
    self.assertContained('', self.run_js('a.out.js'))

  def test_file_packager_huge(self):
    MESSAGE = 'warning: file packager is creating an asset bundle of 257 MB. this is very large, and browsers might have trouble loading it'
    create_file('huge.dat', 'a' * (1024 * 1024 * 257))
    create_file('tiny.dat', 'a')
    err = self.run_process([FILE_PACKAGER, 'test.data', '--preload', 'tiny.dat'], stdout=PIPE, stderr=PIPE).stderr
    self.assertNotContained(MESSAGE, err)
    err = self.run_process([FILE_PACKAGER, 'test.data', '--preload', 'huge.dat'], stdout=PIPE, stderr=PIPE).stderr
    self.assertContained(MESSAGE, err)
    self.clear()

  @parameterized({
    '': (True,),
    'wasm2js': (False,),
  })
  def test_massive_alloc(self, wasm):
    create_file('main.c', r'''
#include <stdio.h>
#include <stdlib.h>

int main() {
  volatile long x = (long)malloc(1024 * 1024 * 1400);
  return x == 0; // can't alloc it, but don't fail catastrophically, expect null
}
    ''')
    cmd = [EMCC, 'main.c', '-sALLOW_MEMORY_GROWTH', '-sINITIAL_MEMORY=16MB']
    if not wasm:
      cmd += ['-sWASM=0']
    self.run_process(cmd)
    # just care about message regarding allocating over 1GB of memory
    output = self.run_js('a.out.js')
    if not wasm:
      self.assertContained('Warning: Enlarging memory arrays, this is not fast! 16777216,1468137472\n', output)

  @parameterized({
    '': (False,),
    'growth': (True,),
  })
  @also_with_wasm2js
  def test_failing_alloc(self, growth):
    # Force memory growth to fail at runtime
    self.add_pre_run('growMemory = (size) => false;')
    for pre_fail, post_fail, opts in [
      ('', '', []),
      ('EM_ASM( Module.temp = _sbrk() );', 'EM_ASM( assert(Module.temp === _sbrk(), "must not adjust brk when an alloc fails!") );', []),
    ]:
      for aborting_args in ([], ['-sABORTING_MALLOC=0']):
        create_file('main.cpp', r'''
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <emscripten.h>

#define CHUNK_SIZE (10 * 1024 * 1024)

int main() {
  std::vector<void*> allocs;
  bool has = false;
  while (1) {
    printf("trying an allocation\n");
    %s
    void* curr = malloc(CHUNK_SIZE);
    if (!curr) {
      %s
      break;
    }
    has = true;
    printf("allocated another chunk, %%zu so far\n", allocs.size());
    allocs.push_back(curr);
  }
  assert(has);
  printf("an allocation failed!\n");
#ifdef SPLIT
  return 0;
#endif
  while (1) {
    assert(allocs.size() > 0);
    void *curr = allocs.back();
    allocs.pop_back();
    free(curr);
    printf("freed one\n");
    if (malloc(CHUNK_SIZE)) break;
  }
  printf("managed another malloc!\n");
}
''' % (pre_fail, post_fail))
        args = [EMXX, 'main.cpp', '-sEXPORTED_FUNCTIONS=_main,_sbrk', '-sINITIAL_MEMORY=16MB'] + opts + aborting_args
        if growth:
          args += ['-sALLOW_MEMORY_GROWTH']
        # growth disables aborting by default, but it can be overridden
        aborting = not aborting_args and not growth
        print('test_failing_alloc', args, pre_fail)
        self.run_process(args)
        # growth also disables aborting
        can_manage_another = not aborting
        split = '-DSPLIT' in args
        print('can manage another:', can_manage_another, 'split:', split, 'aborting:', aborting)
        output = self.run_js('a.out.js', assert_returncode=0 if can_manage_another else NON_ZERO)
        if can_manage_another:
          self.assertContained('an allocation failed!\n', output)
          if not split:
            # split memory allocation may fail due to GC objects no longer being allocatable,
            # and we can't expect to recover from that deterministically. So just check we
            # get to the fail.
            # otherwise, we should fail eventually, then free, then succeed
            self.assertContained('managed another malloc!\n', output)
        else:
          # we should see an abort
          self.assertContained('Aborted(Cannot enlarge memory arrays', output)
          if growth:
            # when growth is enabled, the default is to not abort, so just explain that
            self.assertContained('If you want malloc to return NULL (0) instead of this abort, do not link with -sABORTING_MALLOC', output)
          else:
            # when growth is not enabled, suggest 3 possible solutions (start with more memory, allow growth, or don't abort)
            self.assertContained(('higher than the current value 16777216,', 'higher than the current value 33554432,'), output)
            self.assertContained('compile with -sALLOW_MEMORY_GROWTH', output)
            self.assertContained('compile with -sABORTING_MALLOC=0', output)

  def test_failing_growth_2gb(self):
    create_file('test.c', r'''
#include <stdio.h>
#include <stdlib.h>

void* out;
int main() {
  while (1) {
    puts("loop...");
    out = malloc(1024 * 1024);
    if (!out) {
      puts("done");
      return 0;
    }
  }
}
''')

    self.run_process([EMCC, '-O1', 'test.c', '-sALLOW_MEMORY_GROWTH'])
    self.assertContained('done', self.run_js('a.out.js'))

  @requires_wasm64
  @requires_node_canary
  def test_failing_growth_wasm64(self):
    self.require_wasm64()
    create_file('test.c', r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <emscripten/heap.h>

void* out;

int main() {
  printf("&out = %p\n", &out);
  assert((uintptr_t)&out > (2 * 1024 * 1024 * 1024ll));
  while (1) {
    out = malloc(10 * 1024 * 1024);
    printf("alloc: %p\n", out);
    if (!out) {
      printf("malloc fail with emscripten_get_heap_size: %zu\n", emscripten_get_heap_size());
      printf("done\n");
      return 0;
    }
  }
}
''')
    self.do_runf('test.c', 'done\n', emcc_args=['-sGLOBAL_BASE=2Gb', '-sTOTAL_MEMORY=4Gb', '-sMAXIMUM_MEMORY=5Gb', '-sALLOW_MEMORY_GROWTH', '-sMEMORY64', '-Wno-experimental'])

  def test_libcxx_minimal(self):
    create_file('vector.cpp', r'''
#include <vector>
int main(int argc, char** argv) {
  std::vector<void*> v;
  for (int i = 0 ; i < argc; i++) {
    v.push_back(nullptr);
  }
  return v.size();
}
''')

    self.run_process([EMXX, '-O2', 'vector.cpp', '-o', 'vector.js'])
    self.run_process([EMXX, '-O2', test_file('hello_libcxx.cpp'), '-o', 'iostream.js'])

    vector = os.path.getsize('vector.js')
    iostream = os.path.getsize('iostream.js')
    print(vector, iostream)

    self.assertGreater(vector, 1000)
    # we can strip out almost all of libcxx when just using vector
    self.assertLess(2.25 * vector, iostream)

  @parameterized({
    '': ('1',),
    # TODO(sbc): make dynamic linking work with wasm2js
    # 'wasm2js': ('0',)
  })
  def test_minimal_dynamic(self, wasm):
    library_file = 'library.wasm' if wasm else 'library.js'

    def test(name, main_args, library_args, expected='hello from main\nhello from library', assert_returncode=0):
      print(f'testing {name}', main_args, library_args)
      self.clear()
      create_file('library.c', r'''
        #include <stdio.h>
        void library_func() {
        #ifdef USE_PRINTF
          printf("hello from library: %p\n", &library_func);
        #else
          puts("hello from library");
        #endif
        }
      ''')
      # -fno-builtin to prevent printf -> iprintf optimization
      self.run_process([EMCC, 'library.c', '-fno-builtin', '-sSIDE_MODULE', '-O2', '-o', library_file, '-sWASM=' + wasm, '-sEXPORT_ALL'] + library_args)
      create_file('main.c', r'''
        #include <dlfcn.h>
        #include <stdio.h>
        int main() {
          puts("hello from main");
          void *lib_handle = dlopen("%s", RTLD_NOW);
          if (!lib_handle) {
            puts("cannot load side module");
            puts(dlerror());
            return 1;
          }
          typedef void (*voidfunc)();
          voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
          if (!x) puts("cannot find side function");
          else x();
        }
      ''' % library_file)
      self.run_process([EMCC, 'main.c', '--embed-file', library_file, '-O2', '-sWASM=' + wasm] + main_args)
      self.assertContained(expected, self.run_js('a.out.js', assert_returncode=assert_returncode))
      size = os.path.getsize('a.out.js')
      if wasm:
        size += os.path.getsize('a.out.wasm')
      side_size = os.path.getsize(library_file)
      print(f'  sizes {name}: {size}, {side_size}')
      return (size, side_size)

    def percent_diff(x, y):
      small = min(x, y)
      large = max(x, y)
      return float(100 * large) / small - 100

    full = test('full', main_args=['-sMAIN_MODULE'], library_args=[])
    # printf is not used in main, but libc was linked in, so it's there
    printf = test('printf', main_args=['-sMAIN_MODULE'], library_args=['-DUSE_PRINTF'])

    # main module tests

    # dce in main, and it fails since puts is not exported
    test('dce', main_args=['-sMAIN_MODULE=2'], library_args=[], expected=('is not a function', 'cannot', 'undefined'), assert_returncode=NON_ZERO)

    # with exporting, it works
    dce = test('dce', main_args=['-sMAIN_MODULE=2', '-sEXPORTED_FUNCTIONS=_main,_puts'], library_args=[])

    # printf is not used in main, and we dce, so we failz
    dce_fail = test('dce_fail', main_args=['-sMAIN_MODULE=2'], library_args=['-DUSE_PRINTF'], expected=('is not a function', 'cannot', 'undefined'), assert_returncode=NON_ZERO)

    # exporting printf in main keeps it alive for the library
    test('dce_save', main_args=['-sMAIN_MODULE=2', '-sEXPORTED_FUNCTIONS=_main,_printf,_puts'], library_args=['-DUSE_PRINTF'])

    self.assertLess(percent_diff(full[0], printf[0]), 4)
    self.assertLess(percent_diff(dce[0], dce_fail[0]), 4)
    self.assertLess(dce[0], 0.2 * full[0]) # big effect, 80%+ is gone

    # side module tests

    # mode 2, so dce in side, but library_func is not exported, so it is dce'd
    side_dce_fail = test('side_dce_fail', main_args=['-sMAIN_MODULE'], library_args=['-sSIDE_MODULE=2'], expected='cannot find side function')
    # mode 2, so dce in side, and library_func is not exported
    side_dce_work = test('side_dce_fail', main_args=['-sMAIN_MODULE'], library_args=['-sSIDE_MODULE=2', '-sEXPORTED_FUNCTIONS=_library_func'], expected='hello from library')

    self.assertLess(side_dce_fail[1], 0.95 * side_dce_work[1]) # removing that function saves a chunk

  def test_RUNTIME_LINKED_LIBS(self):
    # Verify that the legacy `-sRUNTIME_LINKED_LIBS` option acts the same as passing a
    # library on the command line directly.
    create_file('side.c', 'int foo() { return 42; }')
    create_file('main.c', '#include <assert.h>\nextern int foo(); int main() { assert(foo() == 42); return 0; }')

    self.run_process([EMCC, '-O2', 'side.c', '-sSIDE_MODULE', '-o', 'side.wasm'])
    self.run_process([EMCC, '-O2', 'main.c', '-sMAIN_MODULE', '-o', 'main.js', 'side.wasm'])
    self.run_js('main.js')

    err = self.run_process([EMCC, '-O2', 'main.c', '-sMAIN_MODULE', '-o', 'main2.js', '-sRUNTIME_LINKED_LIBS=side.wasm'], stderr=PIPE).stderr
    self.assertContained('emcc: warning: RUNTIME_LINKED_LIBS is deprecated', err)
    self.run_js('main2.js')

    self.assertBinaryEqual('main.wasm', 'main2.wasm')

  @parameterized({
    '': ([],),
    'pthread': (['-g', '-pthread', '-Wno-experimental', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'],),
  })
  def test_ld_library_path(self, args):
    if args:
      self.setup_node_pthreads()
    create_file('hello1.c', r'''
#include <stdio.h>

void hello1() {
  printf("Hello1\n");
  return;
}
''')
    create_file('hello2.c', r'''
#include <stdio.h>

void hello2() {
  printf("Hello2\n");
  return;
}
''')
    create_file('hello3.c', r'''
#include <stdio.h>

void hello3() {
  printf ("Hello3\n");
  return;
}
''')
    create_file('hello4.c', r'''
#include <stdio.h>
#include <math.h>

double hello4(double x) {
  printf("Hello4\n");
  return fmod(x, 2.0);
}
''')
    create_file('pre.js', r'''
Module.preRun = () => {
  ENV['LD_LIBRARY_PATH']='/lib:/usr/lib:/usr/local/lib';
};
''')
    create_file('main.c', r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main() {
  void *h;
  void (*f)();
  double (*f2)(double);

  h = dlopen("libhello1.wasm", RTLD_NOW);
  assert(h);
  f = dlsym(h, "hello1");
  assert(f);
  f();
  dlclose(h);

  h = dlopen("libhello2.wasm", RTLD_NOW);
  assert(h);
  f = dlsym(h, "hello2");
  assert(f);
  f();
  dlclose(h);

  h = dlopen("libhello3.wasm", RTLD_NOW);
  assert(h);
  f = dlsym(h, "hello3");
  assert(f);
  f();
  dlclose(h);

  h = dlopen("/usr/local/lib/libhello4.wasm", RTLD_NOW);
  assert(h);
  f2 = dlsym(h, "hello4");
  assert(f2);
  double result = f2(5.5);
  dlclose(h);

  if (result == 1.5) {
    printf("Ok\n");
  }
  return 0;
}
''')
    self.run_process([EMCC, '-o', 'hello1.wasm', 'hello1.c', '-sSIDE_MODULE'] + args)
    self.run_process([EMCC, '-o', 'hello2.wasm', 'hello2.c', '-sSIDE_MODULE'] + args)
    self.run_process([EMCC, '-o', 'hello3.wasm', 'hello3.c', '-sSIDE_MODULE'] + args)
    self.run_process([EMCC, '-o', 'hello4.wasm', 'hello4.c', '-sSIDE_MODULE'] + args)
    self.run_process([EMCC, '--profiling-funcs', '-o', 'main.js', 'main.c', '-sMAIN_MODULE=2', '-sINITIAL_MEMORY=32Mb',
                      '--embed-file', 'hello1.wasm@/lib/libhello1.wasm',
                      '--embed-file', 'hello2.wasm@/usr/lib/libhello2.wasm',
                      '--embed-file', 'hello3.wasm@/libhello3.wasm',
                      '--embed-file', 'hello4.wasm@/usr/local/lib/libhello4.wasm',
                      'hello1.wasm', 'hello2.wasm', 'hello3.wasm', 'hello4.wasm', '-sNO_AUTOLOAD_DYLIBS',
                      '--pre-js', 'pre.js'] + args)
    out = self.run_js('main.js')
    self.assertContained('Hello1', out)
    self.assertContained('Hello2', out)
    self.assertContained('Hello3', out)
    self.assertContained('Hello4', out)
    self.assertContained('Ok', out)

  def test_dlopen_bad_flags(self):
    create_file('main.c', r'''
#include <dlfcn.h>
#include <stdio.h>

int main() {
  void* h = dlopen("lib.so", 0);
  if (h) {
    printf("expected dlopen to fail\n");
    return 1;
  }
  printf("%s\n", dlerror());
  return 0;
}
''')
    self.run_process([EMCC, 'main.c', '-sMAIN_MODULE=2'])
    out = self.run_js('a.out.js')
    self.assertContained('invalid mode for dlopen(): Either RTLD_LAZY or RTLD_NOW is required', out)

  def test_dlopen_constructors(self):
    create_file('side.c', r'''
      #include <stdio.h>
      #include <assert.h>

      static int foo;
      static int* ptr = &foo;

      void check_relocations(void) {
        assert(ptr == &foo);
      }

      __attribute__((constructor)) void ctor(void) {
        printf("foo address: %p\n", ptr);
        // Check that relocations have already been applied by the time
        // constructor functions run.
        check_relocations();
        printf("done ctor\n");
      }
      ''')
    create_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>

      int main() {
        void (*check) (void);
        void* h = dlopen("libside.wasm", RTLD_NOW);
        assert(h);
        check = dlsym(h, "check_relocations");
        assert(check);
        check();
        printf("done\n");
        return 0;
      }''')
    self.run_process([EMCC, '-g', '-o', 'libside.wasm', 'side.c', '-sSIDE_MODULE'])
    self.run_process([EMCC, '-g', '-sMAIN_MODULE=2', 'main.c', 'libside.wasm', '-sNO_AUTOLOAD_DYLIBS'])
    self.assertContained('done', self.run_js('a.out.js'))
    # Repeat the test without NO_AUTOLOAD_DYLIBS
    self.run_process([EMCC, '-g', '-sMAIN_MODULE=2', 'main.c', 'libside.wasm'])
    self.assertContained('done', self.run_js('a.out.js'))

  def test_dlopen_rtld_global(self):
    # This test checks RTLD_GLOBAL where a module is loaded
    # before the module providing a global it needs is. in asm.js we use JS
    # to create a redirection function. In wasm we just have wasm, so we
    # need to introspect the wasm module. Browsers may add that eventually,
    # or we could ship a little library that does it.
    create_file('hello1.c', r'''
#include <stdio.h>

extern int hello1_val;
int hello1_val = 3;

void hello1(int i) {
  printf("hello1_val by hello1:%d\n",hello1_val);
  printf("Hello%d\n",i);
}
''')
    create_file('hello2.c', r'''
#include <stdio.h>

extern int hello1_val;
extern void hello1(int);

void hello2(int i) {
  void (*f) (int);
  printf("hello1_val by hello2:%d\n",hello1_val);
  f = hello1;
  f(i);
}
''')
    create_file('main.c', r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main(int argc,char** argv) {
  void *h;
  void *h2;
  void (*f) (int);
  h = dlopen("libhello1.wasm", RTLD_NOW|RTLD_GLOBAL);
  h2 = dlopen("libhello2.wasm", RTLD_NOW|RTLD_GLOBAL);
  f = dlsym(h, "hello1");
  f(1);
  f = dlsym(h2, "hello2");
  f(2);
  dlclose(h);
  dlclose(h2);
  return 0;
}
''')

    self.run_process([EMCC, '-o', 'libhello1.wasm', 'hello1.c', '-sSIDE_MODULE', '-sEXPORT_ALL'])
    self.run_process([EMCC, '-o', 'libhello2.wasm', 'hello2.c', '-sSIDE_MODULE', '-sEXPORT_ALL'])
    self.run_process([EMCC, '-o', 'main.js', 'main.c', '-sMAIN_MODULE',
                      '--embed-file', 'libhello1.wasm',
                      '--embed-file', 'libhello2.wasm'])
    out = self.run_js('main.js')
    self.assertContained('Hello1', out)
    self.assertContained('Hello2', out)
    self.assertContained('hello1_val by hello1:3', out)
    self.assertContained('hello1_val by hello2:3', out)

  def test_dlopen_async(self):
    create_file('side.c', 'int foo = 42;\n')
    create_file('pre.js', r'''
Module.preRun = () => {
  ENV['LD_LIBRARY_PATH']='/usr/lib';
};
''')
    self.run_process([EMCC, 'side.c', '-o', 'tmp.so', '-sSIDE_MODULE'])
    self.set_setting('MAIN_MODULE', 2)
    self.do_other_test('test_dlopen_async.c', ['--pre-js=pre.js', '--embed-file', 'tmp.so@/usr/lib/libside.so'])

  def test_dlopen_promise(self):
    create_file('side.c', 'int foo = 42;\n')
    self.run_process([EMCC, 'side.c', '-o', 'libside.so', '-sSIDE_MODULE'])
    self.set_setting('MAIN_MODULE', 2)
    self.do_other_test('test_dlopen_promise.c')

  @parameterized({
    # Under node this should work even without ASYNCIFY because we can do
    # synchronous loading via readBinary
    '': (0,),
    'asyncify': (1,),
    'jspi': (2,),
  })
  def test_dlopen_blocking(self, asyncify):
    self.run_process([EMCC, test_file('other/test_dlopen_blocking_side.c'), '-o', 'libside.so', '-sSIDE_MODULE'])
    self.set_setting('MAIN_MODULE', 2)
    self.set_setting('NO_AUTOLOAD_DYLIBS')
    if asyncify:
      self.set_setting('ASYNCIFY', asyncify)
      if asyncify == 1:
        self.set_setting('EXIT_RUNTIME')
      if asyncify == 2:
        self.require_jspi()
    self.emcc_args.append('libside.so')
    self.do_other_test('test_dlopen_blocking.c')

  def test_dlsym_rtld_default(self):
    create_file('side.c', r'''
    int baz() {
      return 99;
    }
    ''')
    self.run_process([EMCC, '-o', 'libside.so', 'side.c', '-sSIDE_MODULE'])
    create_file('main.c', r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE int foo() {
  return 42;
}

EMSCRIPTEN_KEEPALIVE int64_t foo64() {
  return 64;
}

int main(int argc, char** argv) {
  int (*f)();
  f = dlsym(RTLD_DEFAULT, "foo");
  assert(f);
  printf("foo -> %d\n", f());
  assert(f() == 42);

  int64_t (*f64)();
  f64 = dlsym(RTLD_DEFAULT, "foo64");
  assert(f64);
  printf("foo64 -> %lld\n", f64());
  assert(f64() == 64);

  // Missing function
  f = dlsym(RTLD_DEFAULT, "bar");
  printf("bar -> %p\n", f);
  assert(f == NULL);

  // Function from side module that was loaded at startup
  f = dlsym(RTLD_DEFAULT, "baz");
  assert(f);
  printf("baz -> %p\n", f);
  assert(f() == 99);

  // Check that dlopen()'ing libside.so gives that same
  // address for baz.
  void* handle = dlopen("libside.so", RTLD_NOW);
  assert(handle);
  int (*baz)() = dlsym(handle, "baz");
  assert(baz);
  printf("baz -> %p\n", baz);
  assert(baz() == 99);
  assert(baz == f);

  return 0;
}
''')
    self.do_runf('main.c', emcc_args=['-sMAIN_MODULE=2', 'libside.so'])

  def test_dlsym_rtld_default_js_symbol(self):
    create_file('lib.js', '''
      addToLibrary({
       foo__sig: 'ii',
       foo: function(f) { return f + 10 },
       bar: function(f) {  returnf + 10 },
      });
      ''')
    create_file('main.c', r'''
#include <stdio.h>
#include <utime.h>
#include <sys/types.h>
#include <dlfcn.h>

typedef int (*func_type_t)(int arg);

int main(int argc, char** argv) {
  func_type_t fp = (func_type_t)dlsym(RTLD_DEFAULT, argv[1]);
  if (!fp) {
    printf("dlsym failed: %s\n", dlerror());
    return 1;
  }
  printf("%s -> %d\n", argv[1], fp(10));
  return 0;
}
''')
    self.run_process([EMCC, 'main.c',
                      '--js-library=lib.js',
                      '-sMAIN_MODULE=2',
                      '-sEXPORTED_FUNCTIONS=_main,_foo,_bar'])

    # Fist test the successful use of a JS function with dlsym
    out = self.run_js('a.out.js', args=['foo'])
    self.assertContained('foo -> 20', out)

    # Now test the failure case for when __sig is not present
    out = self.run_js('a.out.js', args=['bar'], assert_returncode=NON_ZERO)
    self.assertContained('Missing signature argument to addFunction: function _bar', out)

  def test_main_module_without_exceptions_message(self):
    # A side module that needs exceptions needs a main module with that
    # support enabled; show a clear message in that case.
    create_file('side.cpp', r'''
      #include <exception>
      #include <stdio.h>

      extern "C" void test_throw() {
        try {
          throw 42;
        } catch(int x) {
          printf("catch %d.\n", x);
          return;
        }
        puts("bad location");
      }
    ''')
    create_file('main.cpp', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      #include <dlfcn.h>

      typedef void (*voidf)();

      int main() {
        void* h = dlopen("libside.wasm", RTLD_NOW);
        assert(h);
        voidf f = (voidf)dlsym(h, "test_throw");
        assert(f);
        f();
        return 0;
      }
      ''')
    self.run_process([EMXX, '-o', 'libside.wasm', 'side.cpp', '-sSIDE_MODULE', '-fexceptions'])

    def build_main(args):
      print(args)
      with env_modify({'EMCC_FORCE_STDLIBS': 'libc++abi'}):
        self.run_process([EMXX, 'main.cpp', '-sMAIN_MODULE',
                          '--embed-file', 'libside.wasm'] + args)

    build_main([])
    out = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('Exception thrown, but exception catching is not enabled.', out)
    self.assertContained('note: in dynamic linking, if a side module wants exceptions, the main module must be built with that support', out)

    build_main(['-fexceptions'])
    out = self.run_js('a.out.js')
    self.assertContained('catch 42', out)

  def test_emscripten_print_double(self):
    create_file('src.c', r'''
#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

void test(double d) {
  char buffer[100], buffer2[100];
  unsigned len, len2, len3;
  len = emscripten_print_double(d, NULL, -1);
  len2 = emscripten_print_double(d, buffer, len+1);
  assert(len == len2);
  buffer[len] = 0;
  len3 = snprintf(buffer2, 100, "%g", d);
  printf("|%g : %u : %s : %s : %d|\n", d, len, buffer, buffer2, len3);
}

int main() {
  printf("\n");
  test(0);
  test(1);
  test(-1);
  test(1.234);
  test(-1.234);
  test(1.1234E20);
  test(-1.1234E20);
  test(1.1234E-20);
  test(-1.1234E-20);
  test(1.0/0.0);
  test(-1.0/0.0);
}
''')
    self.run_process([EMCC, 'src.c'])
    out = self.run_js('a.out.js')
    self.assertContained('''
|0 : 1 : 0 : 0 : 1|
|1 : 1 : 1 : 1 : 1|
|-1 : 2 : -1 : -1 : 2|
|1.234 : 5 : 1.234 : 1.234 : 5|
|-1.234 : 6 : -1.234 : -1.234 : 6|
|1.1234e+20 : 21 : 112340000000000000000 : 1.1234e+20 : 10|
|-1.1234e+20 : 22 : -112340000000000000000 : -1.1234e+20 : 11|
|1.1234e-20 : 10 : 1.1234e-20 : 1.1234e-20 : 10|
|-1.1234e-20 : 11 : -1.1234e-20 : -1.1234e-20 : 11|
|inf : 8 : Infinity : inf : 3|
|-inf : 9 : -Infinity : -inf : 4|
''', out)

  def test_emscripten_scan_stack(self):
    create_file('src.cpp', r'''
#include <set>
#include <emscripten.h>
#include <stdio.h>
#include <assert.h>

std::set<int> seenInts;

void scan(void* x, void* y) {
  printf("scan\n");
  int* p = (int*)x;
  int* q = (int*)y;
  // The callback sends us the [low, high) range.
  assert(p < q);
  // The range is of a reasonable size - not all of memory.
  assert(q - p < 100);
  while (p < q) {
    seenInts.insert(*p);
    p++;
  }
}

int main() {
  int x;
  int* y = &x;
  *y = 12345678;
  emscripten_scan_stack(scan);
  assert(seenInts.count(12345678));
  puts("ok");
}
''')
    self.run_process([EMXX, 'src.cpp'])
    self.assertContained('ok', self.run_js('a.out.js'))

  def test_no_warn_exported_jslibfunc(self):
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-sEXPORTED_FUNCTIONS=_main,_alGetError'])

    # Same again but with `_alGet` wich does not exist.  This is a regression
    # test for a bug we had where any prefix of a valid function was accepted.
    err = self.expect_fail([EMCC, test_file('hello_world.c'),
                            '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=alGetError',
                            '-sEXPORTED_FUNCTIONS=_main,_alGet'])
    self.assertContained('wasm-ld: error: symbol exported via --export not found: alGet', err)

  def test_musl_syscalls(self):
    self.run_process([EMCC, test_file('hello_world.c')])
    src = read_file('a.out.js')
    # there should be no musl syscalls in hello world output
    self.assertNotContained('__syscall', src)

  @crossplatform
  def test_emcc_dev_null(self):
    out = self.run_process([EMCC, '-dM', '-E', '-x', 'c', os.devnull], stdout=PIPE).stdout
    self.assertContained('#define __EMSCRIPTEN__ 1', out) # all our defines should show up

  def test_umask_0(self):
    create_file('src.c', r'''\
#include <sys/stat.h>
#include <stdio.h>
int main() {
  umask(0);
  printf("hello, world!\n");
}
''')
    self.run_process([EMCC, 'src.c'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  @crossplatform
  @also_with_wasmfs
  def test_umask(self):
    self.do_runf('other/test_umask.c', 'success')

  def test_no_missing_symbols(self):
    # simple hello world should not show any missing symbols
    self.run_process([EMCC, test_file('hello_world.c')])

    # main() is implemented in C, and even if requested from JS, we should not warn
    create_file('library_foo.js', '''
addToLibrary({
  my_js__deps: ['main'],
  my_js: (function() {
      return () => console.log("hello " + _nonexistingvariable);
  }()),
});
''')
    create_file('test.c', '''\
#include <stdio.h>
#include <stdlib.h>

void my_js();

int main() {
  my_js();
  return EXIT_SUCCESS;
}
''')
    self.run_process([EMCC, 'test.c', '--js-library', 'library_foo.js'])

    # but we do error on a missing js var
    create_file('library_foo_missing.js', '''
addToLibrary({
  my_js__deps: ['main', 'nonexistingvariable'],
  my_js: (function() {
      return () => console.log("hello " + _nonexistingvariable);
  }()),
});
''')
    err = self.expect_fail([EMCC, 'test.c', '--js-library', 'library_foo_missing.js'])
    self.assertContained('undefined symbol: nonexistingvariable. Required by my_js', err)

    # and also for missing C code, of course (without the --js-library, it's just a missing C method)
    err = self.expect_fail([EMCC, 'test.c'])
    self.assertContained('undefined symbol: my_js', err)

  def test_js_lib_native_deps(self):
    # Verify that memset (which lives in compiled code), can be specified as a JS library
    # dependency.
    create_file('lib.js', r'''
addToLibrary({
  depper__deps: ['memset'],
  depper: (ptr) => {
    _memset(ptr, 'd'.charCodeAt(0), 10);
  },
});
''')
    create_file('test.c', r'''
#include <stdio.h>

void depper(char*);

int main(int argc, char** argv) {
  char buffer[11] = { 0 };
  depper(buffer);
  puts(buffer);
}
''')

    self.do_runf('test.c', 'dddddddddd\n', emcc_args=['--js-library', 'lib.js'])

  def test_js_lib_native_deps_extra(self):
    # Similar to above but the JS symbol is not used by the native code.
    # Instead is it explicitly injected using `extraLibraryFuncs`.
    create_file('lib.js', r'''
addToLibrary({
  jsfunc__deps: ['raise'],
  jsfunc: (ptr) => {
    _raise(1);
  },
});
extraLibraryFuncs.push('jsfunc');
''')
    self.do_runf('hello_world.c', emcc_args=['--js-library', 'lib.js'])

  @crossplatform
  @also_with_wasmfs
  def test_realpath(self):
    ensure_dir('boot')
    create_file('boot/README.txt', ' ')
    self.do_other_test('test_realpath.c', emcc_args=['-sSAFE_HEAP', '--embed-file', 'boot'])

  @crossplatform
  @parameterized({
    '': ([],),
    # WasmFS requires FORCE_FILESYSTEM for the full JS API (FS.mkdir etc.).
    'wasmfs': (['-sWASMFS', '-sFORCE_FILESYSTEM'],),
  })
  def test_realpath_nodefs(self, args):
    create_file('TEST_NODEFS.txt', ' ')
    self.do_other_test('test_realpath_nodefs.c', emcc_args=args + ['-lnodefs.js'])

  @also_with_wasmfs
  def test_realpath_2(self):
    ensure_dir('Folder')
    create_file('testfile.txt', '')
    create_file('Folder/testfile.txt', '')
    self.do_other_test('test_realpath_2.c', emcc_args=['--embed-file', 'testfile.txt', '--embed-file', 'Folder'])

  @with_env_modify({'EMCC_LOGGING': '0'})  # this test assumes no emcc output
  def test_no_warnings(self):
    # build once before to make sure system libs etc. exist
    self.run_process([EMXX, test_file('hello_libcxx.cpp')])
    # check that there is nothing in stderr for a regular compile
    err = self.run_process([EMXX, test_file('hello_libcxx.cpp')], stderr=PIPE).stderr
    self.assertEqual(err, '')

  @crossplatform
  def test_dlmalloc_modes(self):
    create_file('src.c', r'''
      #include <stdlib.h>
      #include <stdio.h>
      int main() {
        void* c = malloc(1024);
        free(c);
        free(c);
        printf("double-freed\n");
      }
    ''')
    self.run_process([EMCC, 'src.c'])
    self.assertContained('double-freed', self.run_js('a.out.js'))
    # in debug mode, the double-free is caught
    self.run_process([EMCC, 'src.c', '-sASSERTIONS=2'])
    out = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('native code called abort()', out)

  def test_mallocs(self):
    def run(opts):
      print(opts)
      sizes = {}
      for malloc, name in (
        ('dlmalloc', 'dlmalloc'),
        (None, 'default'),
        ('emmalloc', 'emmalloc'),
        ('mimalloc', 'mimalloc'),
      ):
        print(malloc, name)
        args = opts[:]
        if malloc:
          args += ['-sMALLOC=%s' % malloc]
        print(args)
        self.emcc(test_file('hello_libcxx.cpp'), args=args)
        sizes[name] = os.path.getsize('a.out.wasm')
      print(sizes)
      # dlmalloc is the default
      self.assertEqual(sizes['dlmalloc'], sizes['default'])
      # emmalloc is much smaller
      self.assertLess(sizes['emmalloc'], sizes['dlmalloc'] - 5000)
      # mimalloc is much larger
      self.assertGreater(sizes['mimalloc'], sizes['dlmalloc'] - 25000)
    run([])
    run(['-O2'])

  def test_emmalloc_2GB(self):
    def test(args, text=None):
      if text:
        stderr = self.expect_fail([EMCC, test_file('hello_world.c'), '-sMALLOC=emmalloc'] + args)
        self.assertContained(text, stderr)
      else:
        self.run_process([EMCC, test_file('hello_world.c'), '-sMALLOC=emmalloc'] + args)

    test(['-sALLOW_MEMORY_GROWTH'])
    test(['-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=1GB'])
    test(['-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=4GB'])

  def test_emmalloc_high_align(self):
    self.do_other_test('test_emmalloc_high_align.c',
                       emcc_args=['-sMALLOC=emmalloc', '-sINITIAL_MEMORY=128MB'])

  def test_2GB_plus(self):
    # when the heap size can be over 2GB, we rewrite pointers to be unsigned
    def test(page_diff):
      args = [EMCC, test_file('hello_world.c'), '-O2', '-sALLOW_MEMORY_GROWTH']
      if page_diff is not None:
        args += ['-sMAXIMUM_MEMORY=%d' % (2**31 + page_diff * 64 * 1024)]
      print(args)
      self.run_process(args)
      return os.path.getsize('a.out.js')

    less = test(-1)
    equal = test(0)
    more = test(1)
    none = test(None)

    # exactly 2GB still doesn't require unsigned pointers, as we can't address
    # the 2GB location in memory
    self.assertEqual(less, equal)
    self.assertLess(equal, more)
    # not specifying maximum memory does not result in unsigned pointers, as the
    # default maximum memory is 2GB.
    self.assertEqual(less, none)

  @parameterized({
    # atm we only test mimalloc here, as we don't need extra coverage for
    # dlmalloc/emmalloc, and this is the main test we have for mimalloc
    'mimalloc':          ('mimalloc', ['-DWORKERS=1'],),
    'mimalloc_pthreads': ('mimalloc', ['-DWORKERS=4', '-pthread'],),
  })
  def test_malloc_multithreading(self, allocator, args):
    args = args + [
      '-O2',
      '-DTOTAL=10000',
      '-sINITIAL_MEMORY=128mb',
      '-sTOTAL_STACK=1mb',
      f'-sMALLOC={allocator}',
    ]
    self.do_other_test('test_malloc_multithreading.cpp', emcc_args=args)

  @parameterized({
    '': ([], 'testbind.js'),
    'bigint': (['-sWASM_BIGINT'], 'testbind_bigint.js'),
  })
  @requires_node
  def test_i64_return_value(self, args, bind_js):
    # This test checks that the most significant 32 bits of a 64 bit long are correctly made available
    # to native JavaScript applications that wish to interact with compiled code returning 64 bit longs.
    # The MS 32 bits should be available in getTempRet0() even when compiled with -O2 --closure 1

    # Compile test.c and wrap it in a native JavaScript binding so we can call our compiled function from JS.
    self.run_process([EMCC, test_file('return64bit/test.c'),
                      '--pre-js', test_file('return64bit/testbindstart.js'),
                      '--pre-js', test_file('return64bit', bind_js),
                      '--post-js', test_file('return64bit/testbindend.js'),
                      '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$dynCall',
                      '-sEXPORTED_RUNTIME_METHODS=getTempRet0',
                      '-sEXPORTED_FUNCTIONS=_test_return64', '-o', 'test.js', '-O2',
                      '--closure=1', '-g1', '-sWASM_ASYNC_COMPILATION=0'] + args)

    # Simple test program to load the test.js binding library and call the binding to the
    # C function returning the 64 bit long.
    create_file('testrun.js', '''
      var test = require("./test.js");
      test.runtest();
    ''')

    # Run the test and confirm the output is as expected.
    self.node_args += shared.node_bigint_flags(self.get_nodejs())
    out = self.run_js('testrun.js')
    self.assertContained('''\
input = 0xaabbccdd11223344
low = 5678
high = 1234
input = 0xabcdef1912345678
low = 5678
high = 1234
''', out)

  def test_lib_include_flags(self):
    self.run_process([EMCC] + '-l m -l c -I'.split() + [test_file('include_test'), test_file('lib_include_flags.c')])

  def test_dash_s_link_flag(self):
    # -s is also a valid link flag.  We try to distingish between this case and when
    # its used to set a settings based on looking at the argument that follows.

    # Test the case when -s is the last flag
    self.run_process([EMCC, test_file('hello_world.c'), '-s'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

    # Test the case when the following flag is all uppercase but starts with a `-`
    self.run_process([EMCC, test_file('hello_world.c'), '-s', '-DFOO'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

    # Test that case when the following flag is not all uppercase
    self.run_process([EMCC, '-s', test_file('hello_world.c')])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_dash_s_response_file_string(self):
    create_file('response_file.txt', 'MyModule\n')
    create_file('response_file.json', '"MyModule"\n')
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORT_NAME=@response_file.txt'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORT_NAME=@response_file.json'])

  def test_dash_s_response_file_list(self):
    create_file('response_file.txt', '_main\n_malloc\n')
    create_file('response_file.json', '["_main", "_malloc"]\n')
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS=@response_file.txt'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS=@response_file.json'])

  def test_dash_s_response_file_list_with_comments(self):
    create_file('response_file.txt', '_main\n#_nope_ish_nope\n_malloc\n')
    self.run_process([EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS=@response_file.txt'])

  def test_dash_s_response_file_misssing(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS=@foo'])
    self.assertContained('error: foo: file not found parsing argument: EXPORTED_FUNCTIONS=@foo', err)

  def test_dash_s_unclosed_quote(self):
    # Unclosed quote
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-s', "TEST_KEY='MISSING_QUOTE"])
    self.assertNotContained('AssertionError', err) # Do not mention that it is an assertion error
    self.assertContained('error: error parsing "-s" setting', err)
    self.assertContained('unclosed quoted string. expected final character to be "\'"', err)

  def test_dash_s_single_quote(self):
    # Only one quote
    err = self.expect_fail([EMCC, test_file('hello_world.c'), "-sTEST_KEY='"])
    self.assertNotContained('AssertionError', err) # Do not mention that it is an assertion error
    self.assertContained('error: error parsing "-s" setting', err)
    self.assertContained('unclosed quoted string.', err)

  def test_dash_s_unclosed_list(self):
    # Unclosed list
    err = self.expect_fail([EMCC, test_file('hello_world.cpp'), "-sTEST_KEY=[Value1, Value2"])
    self.assertNotContained('AssertionError', err) # Do not mention that it is an assertion error
    self.assertContained('error: error parsing "-s" setting', err)
    self.assertContained('unterminated string list. expected final character to be "]"', err)

  def test_dash_s_valid_list(self):
    err = self.expect_fail([EMCC, test_file('hello_world.cpp'), "-sTEST_KEY=[Value1, \"Value2\"]"])
    self.assertNotContained('error parsing "-s" setting', err)

  def test_dash_s_wrong_type(self):
    err = self.expect_fail([EMCC, test_file('hello_world.cpp'), '-sEXIT_RUNTIME=[foo,bar]'])
    self.assertContained('error: setting `EXIT_RUNTIME` expects `bool` but got `list`', err)
    err = self.expect_fail([EMCC, test_file('hello_world.cpp'), '-sEXIT_RUNTIME=true'])
    self.assertContained('error: attempt to set `EXIT_RUNTIME` to `true`; use 1/0 to set boolean setting', err)

  def test_dash_s_typo(self):
    # with suggestions
    stderr = self.expect_fail([EMCC, test_file('hello_world.c'), '-sDISABLE_EXCEPTION_CATCH'])
    self.assertContained("Attempt to set a non-existent setting: 'DISABLE_EXCEPTION_CATCH'", stderr)
    self.assertContained('did you mean one of DISABLE_EXCEPTION_CATCHING', stderr)
    # no suggestions
    stderr = self.expect_fail([EMCC, test_file('hello_world.c'), '-sCHEEZ'])
    self.assertContained("perhaps a typo in emcc\'s  -sX=Y  notation?", stderr)
    self.assertContained('(see src/settings.js for valid values)', stderr)
    # suggestions do not include renamed legacy settings
    stderr = self.expect_fail([EMCC, test_file('hello_world.c'), '-sZBINARYEN_ASYNC_COMPILATION'])
    self.assertContained("Attempt to set a non-existent setting: 'ZBINARYEN_ASYNC_COMPILATION'", stderr)
    self.assertNotContained(' BINARYEN_ASYNC_COMPILATION', stderr)

  def test_dash_s_with_space(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-s', 'EXPORT_ALL'])
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-s', 'EXPORTED_FUNCTIONS=foo'])
    self.assertContained('error: undefined exported symbol: "foo"', err)

  def test_dash_s_hex(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-nostdlib', '-sERROR_ON_UNDEFINED_SYMBOLS=0'])
    # Ensure that 0x0 is parsed as a zero and not as the string '0x0'.
    self.run_process([EMCC, test_file('hello_world.c'), '-nostdlib', '-sERROR_ON_UNDEFINED_SYMBOLS=0x0'])

  def test_dash_s_bad_json_types(self):
    # Dict rather than string/list
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS={"a":1}'])
    self.assertContained("settings must be strings or lists (not $<class 'dict'>", err)

    # List element is not a string
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sEXPORTED_FUNCTIONS=[{"a":1}]'])
    self.assertContained("list members in settings must be strings (not $<class 'dict'>)", err)

  def test_zeroinit(self):
    create_file('src.c', r'''
#include <stdio.h>
int buf[1048576];
int main() {
  printf("hello, world! %d\n", buf[123456]);
  return 0;
}
''')
    self.run_process([EMCC, 'src.c', '-O2'])
    size = os.path.getsize('a.out.wasm')
    # size should be much smaller than the size of that zero-initialized buffer
    self.assertLess(size, 123456 / 2)

  def test_canonicalize_nan_warning(self):
    create_file('src.c', r'''
#include <stdio.h>

union U {
  int x;
  float y;
} a;

int main() {
  a.x = 0x7FC01234;
  printf("%f\n", a.y);
  printf("0x%x\n", a.x);
  return 0;
}
''')

    self.run_process([EMCC, 'src.c', '-O1'])
    out = self.run_js('a.out.js')
    self.assertContained('nan\n', out)
    self.assertContained('0x7fc01234\n', out)

  def test_memory_growth_noasm(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-O2', '-sALLOW_MEMORY_GROWTH'])
    src = read_file('a.out.js')
    assert 'use asm' not in src

  def test_EM_ASM_i64(self):
    expected = 'Invalid character 106("j") in readEmAsmArgs!'
    self.do_runf('other/test_em_asm_i64.cpp',
                 expected_output=expected,
                 assert_returncode=NON_ZERO)

    self.set_setting('WASM_BIGINT')
    self.do_other_test('test_em_asm_i64.cpp')
    self.do_other_test('test_em_asm_i64.cpp', force_c=True)

  def test_eval_ctor_ordering(self):
    # ensure order of execution remains correct, even with a bad ctor
    def test(p1, p2, p3, last, expected):
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>
        volatile int total = 0;
        struct C {
          C(int x) {
            volatile int y = x;
            y++;
            y--;
            if (y == 0xf) {
              // A printf can't be evalled ahead of time, so this will stop
              // us.
              printf("you can't eval me ahead of time\n");
            }
            total <<= 4;
            total += int(y);
          }
        };
        C __attribute__((init_priority(%d))) c1(0x5);
        C __attribute__((init_priority(%d))) c2(0x8);
        C __attribute__((init_priority(%d))) c3(%d);
        int main() {
          printf("total is 0x%%x.\n", total);
        }
      ''' % (p1, p2, p3, last)
      create_file('src.cpp', src)
      self.run_process([EMXX, 'src.cpp', '-O2', '-sEVAL_CTORS', '-profiling-funcs'])
      self.assertContained('total is %s.' % hex(expected), self.run_js('a.out.js'))
      shutil.copy('a.out.js', 'x' + hex(expected) + '.js')
      shutil.copy('a.out.wasm', 'x' + hex(expected) + '.wasm')
      return os.path.getsize('a.out.wasm')

    print('no bad ctor')
    first  = test(1000, 2000, 3000, 0xe, 0x58e) # noqa
    second = test(3000, 1000, 2000, 0xe, 0x8e5) # noqa
    third  = test(2000, 3000, 1000, 0xe, 0xe58) # noqa
    print(first, second, third)
    assert first == second and second == third
    print('with bad ctor')
    first  = test(1000, 2000, 3000, 0xf, 0x58f) # noqa; 2 will succeed
    second = test(3000, 1000, 2000, 0xf, 0x8f5) # noqa; 1 will succedd
    third  = test(2000, 3000, 1000, 0xf, 0xf58) # noqa; 0 will succeed
    print(first, second, third)
    self.assertLess(first, second)
    self.assertLess(second, third)

  @uses_canonical_tmp
  @with_env_modify({'EMCC_DEBUG': '1'})
  def test_eval_ctors_debug_output(self):
    create_file('lib.js', r'''
addToLibrary({
  external_thing: () => {}
});
''')
    create_file('src.cpp', r'''
extern "C" void external_thing();
struct C {
  C() { external_thing(); } // don't remove this!
};
C c;
int main() {}
    ''')
    err = self.run_process([EMXX, 'src.cpp', '--js-library', 'lib.js', '-O2', '-sEVAL_CTORS'], stderr=PIPE).stderr
    # logging should show we failed, and why
    self.assertNotContained('ctor_evaller: not successful', err)
    self.assertContained('could not eval: call import: env.external_thing', err)

  @uses_canonical_tmp
  @with_env_modify({'EMCC_DEBUG': '1'})
  @parameterized({
    # StackIR optimizations should happen once at the end, and not multiple
    # times in the middle. In -O2 we simply do them in the main wasm-opt
    # invocation, while in -O3 we run wasm-opt later, and so we should disable
    # StackIR first and enable it at the end.
    'O2': (['-O2'], False),
    'O3': (['-O3'], True),
  })
  def test_binaryen_stack_ir(self, opts, disable_and_enable):
    err = self.run_process([EMXX, test_file('hello_world.c')] + opts, stderr=PIPE).stderr
    DISABLE = '--no-stack-ir'
    ENABLE = '--optimize-stack-ir'
    self.assertContainedIf(DISABLE, err, disable_and_enable)
    self.assertContainedIf(ENABLE, err, disable_and_enable)
    # they should also appear at most once each
    self.assertLess(err.count(DISABLE), 2)
    self.assertLess(err.count(ENABLE), 2)

  def test_override_js_execution_environment(self):
    create_file('main.c', r'''
      #include <emscripten.h>
      int main() {
        EM_ASM({
          out('environment is WEB? ' + ENVIRONMENT_IS_WEB);
          out('environment is WORKER? ' + ENVIRONMENT_IS_WORKER);
          out('environment is NODE? ' + ENVIRONMENT_IS_NODE);
          out('environment is SHELL? ' + ENVIRONMENT_IS_SHELL);
        });
      }
''')
    # use SINGLE_FILE since we don't want to depend on loading a side .wasm file on the environment in this test;
    # with the wrong env we have very odd failures
    self.run_process([EMCC, 'main.c', '-sSINGLE_FILE'])
    src = read_file('a.out.js')
    envs = ['web', 'worker', 'node', 'shell']
    for env in envs:
      for engine in config.JS_ENGINES:
        if engine == config.V8_ENGINE:
          continue # ban v8, weird failures
        actual = 'NODE' if engine == config.NODE_JS_TEST else 'SHELL'
        print(env, actual, engine)
        module = {'ENVIRONMENT': env}
        if env != actual:
          # avoid problems with arguments detection, which may cause very odd failures with the wrong environment code
          module['arguments'] = []
        curr = 'var Module = %s;\n' % str(module)
        print('    ' + curr)
        create_file('test.js', curr + src)
        seen = self.run_js('test.js', engine=engine, assert_returncode=NON_ZERO)
        self.assertContained('Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node', seen)

  def test_override_c_environ(self):
    create_file('pre.js', r'''
      Module.preRun = () => { ENV.hello = 'world'; ENV.LANG = undefined; }
    ''')
    create_file('src.c', r'''
      #include <stdlib.h>
      #include <stdio.h>
      int main() {
        printf("|%s|\n", getenv("hello"));
        printf("LANG is %s\n", getenv("LANG") ? "set" : "not set");
      }
    ''')
    self.run_process([EMCC, 'src.c', '--pre-js', 'pre.js'])
    output = self.run_js('a.out.js')
    self.assertContained('|world|', output)
    self.assertContained('LANG is not set', output)

    create_file('pre.js', r'''
      Module.preRun = (module) => { module.ENV.hello = 'world' }
    ''')
    self.run_process([EMCC, 'src.c', '--pre-js', 'pre.js', '-sEXPORTED_RUNTIME_METHODS=ENV'])
    self.assertContained('|world|', self.run_js('a.out.js'))

    self.run_process([EMCC, 'src.c', '--pre-js', 'pre.js', '-sEXPORTED_RUNTIME_METHODS=ENV', '-sMODULARIZE'])
    create_file('run.js', 'require("./a.out.js")();')
    output = self.run_js('run.js')
    self.assertContained('|world|', output)

  @also_with_wasmfs
  def test_warn_no_filesystem(self):
    error = 'Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with -sFORCE_FILESYSTEM'

    self.emcc(test_file('hello_world.c'))
    seen = self.run_js('a.out.js')
    self.assertNotContained(error, seen)

    def test(contents):
      create_file('src.c', r'''
  #include <stdio.h>
  #include <emscripten.h>
  int main() {
    EM_ASM({ %s });
    printf("hello, world!\n");
    return 0;
  }
  ''' % contents)
      self.do_runf('src.c', error, assert_returncode=NON_ZERO)

    # might appear in handwritten code
    test("FS.init()")
    test("FS.createPreloadedFile('waka waka, just warning check')")
    test("FS.createDataFile('waka waka, just warning check')")
    test("FS.analyzePath('waka waka, just warning check')")
    # might appear in filesystem code from a separate script tag
    test("Module['FS_createDataFile']('waka waka, just warning check')")
    test("Module['FS_createPreloadedFile']('waka waka, just warning check')")

    # text is in the source when needed, but when forcing FS, it isn't there
    self.emcc('src.c')
    self.assertContained(error, read_file('a.out.js'))
    self.emcc('src.c', args=['-sFORCE_FILESYSTEM']) # forcing FS means no need
    self.assertNotContained(error, read_file('a.out.js'))
    self.emcc('src.c', args=['-sASSERTIONS=0']) # no assertions, no need
    self.assertNotContained(error, read_file('a.out.js'))
    self.emcc('src.c', args=['-O2']) # optimized, so no assertions
    self.assertNotContained(error, read_file('a.out.js'))

  def test_warn_module_out_err(self):
    def test(contents, expected, args=[], assert_returncode=0):  # noqa
      create_file('src.c', r'''
        #include <emscripten.h>
        int main() {
          EM_ASM({ %s });
          return 0;
        }
        ''' % contents)
      self.do_runf('src.c', expected, emcc_args=args, assert_returncode=assert_returncode)

    # error shown (when assertions are on)
    error = 'was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)'
    test("Module.out('x')", error, assert_returncode=NON_ZERO)
    test("Module['out']('x')", error, assert_returncode=NON_ZERO)
    test("Module.err('x')", error, assert_returncode=NON_ZERO)
    test("Module['err']('x')", error, assert_returncode=NON_ZERO)

    # when exported, all good
    test("Module['out']('print'); Module['err']('err'); ", 'print\nerr', ['-sEXPORTED_RUNTIME_METHODS=out,err'])

    # test backwards compatibility
    test("Module['print']('print'); Module['printErr']('err'); ", 'print\nerr', ['-sEXPORTED_RUNTIME_METHODS=print,printErr', '-Wno-js-compiler'])

  @parameterized({
    '': ('hello_world.c',),
    'argv': ('hello_world_argv.c',),
  })
  @parameterized({
    '': ([],),
    'O2': (['-O2'],),
  })
  def test_warn_unexported_main(self, filename, args):
    warning = 'emcc: warning: `main` is defined in the input files, but `_main` is not in `EXPORTED_FUNCTIONS`. Add it to this list if you want `main` to run. [-Wunused-main]'

    proc = self.run_process([EMCC, test_file(filename), '-sEXPORTED_FUNCTIONS=[]'] + args, stderr=PIPE)
    # This warning only shows up when ASSERTIONS are enabled.
    # We run both ways those to ensure that main doesn't get run in either case.
    if '-O2' in args:
      self.assertNotContained(warning, proc.stderr)
    else:
      self.assertContained(warning, proc.stderr)

    # Verify that main indeed does not run
    output = self.run_js('a.out.js')
    self.assertEqual('', output)

  def test_source_file_with_fixed_language_mode(self):
    create_file('src_tmp_fixed_lang', '''
#include <string>
#include <iostream>

int main() {
  std::cout << "Test_source_fixed_lang_hello" << std::endl;
  return 0;
}
    ''')
    self.run_process([EMXX, '-Wall', '-x', 'c++', 'src_tmp_fixed_lang'])
    self.assertContained('Test_source_fixed_lang_hello', self.run_js('a.out.js'))

    stderr = self.expect_fail([EMXX, '-Wall', 'src_tmp_fixed_lang'])
    self.assertContained('unknown file type: src_tmp_fixed_lang', stderr)

  def test_disable_inlining(self):
    create_file('test.c', r'''
#include <stdio.h>

static void foo() {
  printf("foo\n");
}

int main() {
  foo();
  return 0;
}
''')

    # Without the 'INLINING_LIMIT', -O2 inlines foo() and then DCEs it because it has
    # no callers and is static
    cmd = [EMCC, 'test.c', '-O2', '-o', 'test.o', '-c', '-sINLINING_LIMIT']
    self.run_process(cmd)
    output = self.run_process([common.LLVM_OBJDUMP, '-t', 'test.o'], stdout=PIPE).stdout
    self.assertContained('foo', output)

  @crossplatform
  def test_output_eol(self):
    for params in ([], ['--proxy-to-worker'], ['--proxy-to-worker', '-sWASM=0']):
      for output_suffix in ('html', 'js'):
        for eol in ('windows', 'linux'):
          files = ['a.js']
          if output_suffix == 'html':
            files += ['a.html']
          cmd = [EMCC, test_file('hello_world.c'), '-o', 'a.' + output_suffix, '--output_eol', eol] + params
          self.run_process(cmd)
          for f in files:
            print(str(cmd) + ' ' + str(params) + ' ' + eol + ' ' + f)
            self.assertExists(f)
            if eol == 'linux':
              expected_ending = '\n'
            else:
              expected_ending = '\r\n'

            ret = line_endings.check_line_endings(f, expect_only=expected_ending)
            self.assertEqual(ret, 0)

          for f in files:
            delete_file(f)

  def test_binaryen_warn_mem(self):
    # if user changes INITIAL_MEMORY at runtime, the wasm module may not accept the memory import if
    # it is too big/small
    create_file('pre.js', 'Module.INITIAL_MEMORY = 50 * 1024 * 1024')
    self.run_process([EMCC, test_file('hello_world.c'), '-sINITIAL_MEMORY=' + str(16 * 1024 * 1024), '--pre-js', 'pre.js', '-sWASM_ASYNC_COMPILATION=0', '-sIMPORTED_MEMORY'])
    out = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('LinkError', out)
    self.assertContained("memory import 2 has a larger maximum size 800 than the module's declared maximum", out)
    self.assertNotContained('hello, world!', out)
    # and with memory growth, all should be good
    self.run_process([EMCC, test_file('hello_world.c'), '-sINITIAL_MEMORY=' + str(16 * 1024 * 1024), '--pre-js', 'pre.js', '-sALLOW_MEMORY_GROWTH', '-sWASM_ASYNC_COMPILATION=0', '-sIMPORTED_MEMORY'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  @parameterized({
    '': ([], 16 * 1024 * 1024), # Default behavior: 16MB initial heap
    'explicit': (['-sINITIAL_HEAP=64KB'], 64 * 1024), # Explicitly set initial heap is passed
    'with_initial_memory': (['-sINITIAL_MEMORY=40MB'], 0), # Backwards compatibility: no initial heap (we can't tell if it'll fit)
    'with_maximum_memory': (['-sMAXIMUM_MEMORY=40MB', '-sALLOW_MEMORY_GROWTH=1'], 0), # Backwards compatibility: no initial heap (we can't tell if it'll fit)
    'with_all': (['-sINITIAL_HEAP=128KB', '-sINITIAL_MEMORY=20MB', '-sMAXIMUM_MEMORY=40MB', '-sALLOW_MEMORY_GROWTH=1'], 128 * 1024),
    'limited_by_initial_memory': (['-sINITIAL_HEAP=10MB', '-sINITIAL_MEMORY=10MB'], None), # Not enough space for stack
    'limited_by_maximum_memory': (['-sINITIAL_HEAP=5MB', '-sMAXIMUM_MEMORY=5MB', '-sALLOW_MEMORY_GROWTH=1'], None), # Not enough space for stack
  })
  def test_initial_heap(self, args, expected_initial_heap):
    cmd = [EMCC, test_file('hello_world.c'), '-v'] + args

    if expected_initial_heap is None:
      out = self.expect_fail(cmd)
      self.assertContained('wasm-ld: error:', out)
      return

    out = self.run_process(cmd, stderr=PIPE)
    if expected_initial_heap != 0:
      self.assertContained(f'--initial-heap={expected_initial_heap}', out.stderr)
    else:
      self.assertNotContained('--initial-heap=', out.stderr)

  def test_memory_size(self):
    for args, expect_initial, expect_max in [
        ([], 320, 320),
        (['-sALLOW_MEMORY_GROWTH'], 320, 32768),
        (['-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=40MB'], 320, 640),
      ]:
      cmd = [EMCC, test_file('hello_world.c'), '-O2', '-sINITIAL_MEMORY=20MB'] + args
      print(' '.join(cmd))
      self.run_process(cmd)
      wat = self.get_wasm_text('a.out.wasm')
      memories = [l for l in wat.splitlines() if '(memory ' in l]
      self.assertEqual(len(memories), 2)
      line = memories[0]
      parts = line.strip().replace('(', '').replace(')', '').split()
      print(parts)
      self.assertEqual(parts[2], str(expect_initial))
      self.assertEqual(parts[3], str(expect_max))

  def test_invalid_mem(self):
    # A large amount is fine, multiple of 16MB or not
    self.run_process([EMCC, test_file('hello_world.c'), '-sINITIAL_MEMORY=33MB'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sINITIAL_MEMORY=32MB'])

    # A tiny amount is fine in wasm
    self.run_process([EMCC, test_file('hello_world.c'), '-sINITIAL_MEMORY=65536', '-sSTACK_SIZE=1024'])
    # And the program works!
    self.assertContained('hello, world!', self.run_js('a.out.js'))

    # Must be a multiple of 64KB
    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-sINITIAL_HEAP=32505857', '-sALLOW_MEMORY_GROWTH']) # 31MB + 1 byte
    self.assertContained('INITIAL_HEAP must be a multiple of WebAssembly page size (64KiB)', ret)

    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-sINITIAL_MEMORY=33554433']) # 32MB + 1 byte
    self.assertContained('INITIAL_MEMORY must be a multiple of WebAssembly page size (64KiB)', ret)

    self.run_process([EMCC, test_file('hello_world.c'), '-sMAXIMUM_MEMORY=33MB', '-sALLOW_MEMORY_GROWTH'])

    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-sMAXIMUM_MEMORY=34603009', '-sALLOW_MEMORY_GROWTH']) # 33MB + 1 byte
    self.assertContained('MAXIMUM_MEMORY must be a multiple of WebAssembly page size (64KiB)', ret)

  def test_invalid_memory_max(self):
    err = self.expect_fail([EMCC, '-Werror', test_file('hello_world.c'), '-sMAXIMUM_MEMORY=41943040'])
    self.assertContained('emcc: error: MAXIMUM_MEMORY is only meaningful with ALLOW_MEMORY_GROWTH', err)

  def test_dasho_invalid_dir(self):
    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-o', Path('NONEXISTING_DIRECTORY/out.js')])
    self.assertContained('specified output file (NONEXISTING_DIRECTORY%sout.js) is in a directory that does not exist' % os.path.sep, ret)

  def test_dasho_is_dir(self):
    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-o', '.'])
    self.assertContained('emcc: error: cannot write output file `.`: Is a directory', ret)

    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-o', '.', '--oformat=wasm'])
    self.assertContained('wasm-ld: error: cannot open output file .:', ret)
    # Linux/Mac and Windows's error messages are slightly different
    self.assertContained(['Is a directory', 'is a directory'], ret)

    ret = self.expect_fail([EMCC, test_file('hello_world.c'), '-o', '.', '--oformat=html'])
    self.assertContained('emcc: error: cannot write output file `.`: Is a directory', ret)

  def test_binaryen_ctors(self):
    # ctor order must be identical to js builds, deterministically
    create_file('src.cpp', r'''
      #include <stdio.h>
      struct A {
        A() { puts("constructing A!"); }
      };
      A a;
      struct B {
        B() { puts("constructing B!"); }
      };
      B b;
      int main() {}
    ''')
    self.run_process([EMXX, 'src.cpp'])
    correct = self.run_js('a.out.js')
    for args in ([], ['-sRELOCATABLE']):
      print(args)
      self.run_process([EMXX, 'src.cpp', '-o', 'b.out.js'] + args)
      seen = self.run_js('b.out.js')
      assert correct == seen, correct + '\n vs \n' + seen

  # test debug info and debuggability of JS output
  @crossplatform
  def test_binaryen_debug(self):
    for args, expect_emit_text, expect_clean_js, expect_whitespace_js, expect_closured in [
        (['-O0'], False, False, True, False),
        (['-O0', '-g1'], False, False, True, False),
        (['-O0', '-g2'], False, False, True, False), # in -g2+, we emit -g to asm2wasm so function names are saved
        (['-O0', '-g'], True, False, True, False),
        (['-O0', '--profiling-funcs'], False, False, True, False),
        (['-O1'],        False, False, True, False),
        (['-O2'],        False, True,  False, False),
        (['-O2', '-g1'], False, False, True, False),
        (['-O2', '-g'],  True,  False, True, False),
        (['-O2', '--closure=1'],         False, True, False, True),
        (['-O2', '--closure=1', '-g1'],  False, True, True,  True),
      ]:
      print(args, expect_emit_text, expect_clean_js, expect_whitespace_js, expect_closured)
      delete_file('a.out.wat')
      cmd = [EMCC, test_file('hello_world.c')] + args
      print(' '.join(cmd))
      self.run_process(cmd)
      js = read_file('a.out.js')
      assert expect_clean_js == ('// ' not in js), 'cleaned-up js must not have comments'
      assert expect_whitespace_js == ('{\n  ' in js), 'whitespace-minified js must not have excess spacing'
      assert expect_closured == ('var a;' in js or 'var a,' in js or 'var a=' in js or 'var a ' in js), 'closured js must have tiny variable names'

  @uses_canonical_tmp
  def test_binaryen_ignore_implicit_traps(self):
    sizes = []
    with env_modify({'EMCC_DEBUG': '1'}):
      for args, expect in [
          ([], False),
          (['-sBINARYEN_IGNORE_IMPLICIT_TRAPS'], True),
        ]:
        print(args, expect)
        cmd = [EMXX, test_file('hello_libcxx.cpp'), '-O3'] + args
        print(' '.join(cmd))
        err = self.run_process(cmd, stdout=PIPE, stderr=PIPE).stderr
        self.assertContainedIf('--ignore-implicit-traps ', err, expect)
        sizes.append(os.path.getsize('a.out.wasm'))
    print('sizes:', sizes)
    # sizes must be different, as the flag has an impact
    self.assertEqual(len(set(sizes)), 2)

  def test_binaryen_passes_extra(self):
    def build(args):
      return self.run_process([EMCC, test_file('hello_world.c'), '-O3'] + args, stdout=PIPE).stdout

    build([])
    base_size = os.path.getsize('a.out.wasm')
    out = build(['-sBINARYEN_EXTRA_PASSES="--metrics"'])
    # and --metrics output appears
    self.assertContained('[funcs]', out)
    # adding --metrics should not affect code size
    self.assertEqual(base_size, os.path.getsize('a.out.wasm'))

  def check_expected_size_in_file(self, desc, filename, size):
    if common.EMTEST_REBASELINE:
      create_file(filename, f'{size}\n', absolute=True)
    size_slack = 0.05
    expected_size = int(read_file(filename).strip())
    delta = size - expected_size
    ratio = abs(delta) / float(expected_size)
    print('  seen %s size: %d (expected: %d) (delta: %d), ratio to expected: %f' % (desc, size, expected_size, delta, ratio))
    self.assertLess(ratio, size_slack)

  @crossplatform
  def test_unoptimized_code_size(self):
    # We don't care too about unoptimized code size but we would like to keep it
    # under control to a certain extent.  This test allows us to track major
    # changes to the size of the unoptimized and unminified code size.
    # Run with `--rebase` when this test fails.
    self.build(test_file('hello_world.c'), emcc_args=['-O0', '--output_eol=linux'])
    self.check_expected_size_in_file('wasm',
                                     test_file('other/test_unoptimized_code_size.wasm.size'),
                                     os.path.getsize('hello_world.wasm'))
    self.check_expected_size_in_file('js',
                                     test_file('other/test_unoptimized_code_size.js.size'),
                                     os.path.getsize('hello_world.js'))

    self.build(test_file('hello_world.c'), emcc_args=['-O0', '--output_eol=linux', '-sASSERTIONS=0'], output_basename='no_asserts')
    self.check_expected_size_in_file('wasm',
                                     test_file('other/test_unoptimized_code_size_no_asserts.wasm.size'),
                                     os.path.getsize('no_asserts.wasm'))
    self.check_expected_size_in_file('js',
                                     test_file('other/test_unoptimized_code_size_no_asserts.js.size'),
                                     os.path.getsize('no_asserts.js'))

    self.build(test_file('hello_world.c'), emcc_args=['-O0', '--output_eol=linux', '-sSTRICT'], output_basename='strict')
    self.check_expected_size_in_file('wasm',
                                     test_file('other/test_unoptimized_code_size_strict.wasm.size'),
                                     os.path.getsize('strict.wasm'))
    self.check_expected_size_in_file('js',
                                     test_file('other/test_unoptimized_code_size_strict.js.size'),
                                     os.path.getsize('strict.js'))

  def run_codesize_test(self, filename, args=[], expected_exists=[], expected_not_exists=[],  # noqa
                        check_funcs=True):

    # in -Os, -Oz, we remove imports wasm doesn't need
    print('Running metadce test: %s:' % filename, args, expected_exists, expected_not_exists, check_funcs)
    filename = test_file('other/codesize', filename)
    expected_basename = test_file('other/codesize', self.id().split('.')[-1])

    # Run once without closure and parse output to find wasmImports
    build_cmd = [compiler_for(filename), filename, '--output_eol=linux', '--emit-minification-map=minify.map'] + args + self.get_emcc_args()
    self.run_process(build_cmd + ['-g2'])
    # find the imports we send from JS
    # TODO(sbc): Find a way to do that that doesn't depend on internal details of
    # the generated code.
    js = read_file('a.out.js')
    start = js.find('wasmImports = ')
    self.assertNotEqual(start, -1)
    end = js.find('}', start)
    self.assertNotEqual(end, -1)
    start = js.find('{', start)
    self.assertNotEqual(start, -1)
    relevant = js[start + 2:end - 1]
    relevant = relevant.replace(' ', '').replace('"', '').replace("'", '')
    relevant = relevant.replace('/**@export*/', '')
    relevant = relevant.split(',')
    sent = [x.split(':')[0].strip() for x in relevant]
    sent = [x for x in sent if x]
    # Deminify the sent list, if minification occured
    if os.path.exists('minify.map'):
      sent = deminify_syms(sent, 'minify.map')
      os.remove('minify.map')
    sent.sort()

    self.run_process(build_cmd + ['--profiling-funcs', '--closure=1'])

    for exists in expected_exists:
      self.assertIn(exists, sent)
    for not_exists in expected_not_exists:
      self.assertNotIn(not_exists, sent)

    # measure the wasm size without the name section
    building.strip('a.out.wasm', 'a.out.nodebug.wasm', sections=['name'])
    wasm_size = os.path.getsize('a.out.nodebug.wasm')
    size_file = expected_basename + '.size'
    js_size = os.path.getsize('a.out.js')
    gz_size = get_file_gzipped_size('a.out.js')
    js_size_file = expected_basename + '.jssize'
    gz_size_file = expected_basename + '.gzsize'
    self.check_expected_size_in_file('wasm', size_file, wasm_size)
    self.check_expected_size_in_file('js', js_size_file, js_size)
    self.check_expected_size_in_file('gz', gz_size_file, gz_size)

    imports, exports, funcs = self.parse_wasm('a.out.wasm')
    # Deminify the imports/export lists, if minification occured
    if os.path.exists('minify.map'):
      exports = deminify_syms(exports, 'minify.map')
      imports = [i.split('.', 1)[1] for i in imports]
      imports = deminify_syms(imports, 'minify.map')
    imports.sort()
    exports.sort()
    funcs.sort()

    # filter out _NNN suffixed that can be the result of bitcode linking when
    # internal symbol names collide.
    def strip_numeric_suffixes(funcname):
      parts = funcname.split('_')
      while parts:
        if parts[-1].isdigit():
          parts.pop()
        else:
          break
      return '_'.join(parts)

    funcs = [strip_numeric_suffixes(f) for f in funcs]

    sent_file = expected_basename + '.sent'
    sent_data = '\n'.join(sent) + '\n'
    self.assertFileContents(sent_file, sent_data)

    filename = expected_basename + '.imports'
    data = '\n'.join(imports) + '\n'
    self.assertFileContents(filename, data)

    filename = expected_basename + '.exports'
    data = '\n'.join(exports) + '\n'
    self.assertFileContents(filename, data)

    if check_funcs:
      filename = expected_basename + '.funcs'
      data = '\n'.join(funcs) + '\n'
      self.assertFileContents(filename, data)

  @parameterized({
    'O0': ([],      [], ['waka']), # noqa
    'O1': (['-O1'], [], ['waka']), # noqa
    'O2': (['-O2'], [], ['waka']), # noqa
    # in -O3, -Os and -Oz we metadce, and they shrink it down to the minimal output we want
    'O3': (['-O3'], [], []), # noqa
    'Os': (['-Os'], [], []), # noqa
    'Oz': (['-Oz'], [], []), # noqa
    'Os_mr': (['-Os', '-sMINIMAL_RUNTIME'], [], [], 74), # noqa
    # EVAL_CTORS also removes the __wasm_call_ctors function
    'Oz-ctors': (['-Oz', '-sEVAL_CTORS'], [], []), # noqa
    '64': (['-Oz', '-sMEMORY64', '-Wno-experimental'], [], []), # noqa
    # WasmFS should not be fully linked into a minimal program.
    'wasmfs': (['-Oz', '-sWASMFS'], [], []), # noqa
  })
  def test_codesize_minimal(self, *args):
    self.set_setting('STRICT')
    self.emcc_args.append('--no-entry')
    self.run_codesize_test('minimal.c', *args)

  @node_pthreads
  def test_codesize_minimal_pthreads(self):
    self.run_codesize_test('minimal_main.c', ['-Oz', '-pthread', '-sPROXY_TO_PTHREAD'])

  @parameterized({
    'noexcept': (['-O2'],                    [], ['waka']), # noqa
    # exceptions increases code size significantly
    'except':   (['-O2', '-fexceptions'],    [], ['waka']), # noqa
    # exceptions does not pull in demangling by default, which increases code size
    'mangle':   (['-O2', '-fexceptions',
                  '-sDEMANGLE_SUPPORT', '-Wno-deprecated'], [], ['waka']), # noqa
    # Wasm EH's code size increase is smaller than that of Emscripten EH
    'except_wasm':   (['-O2', '-fwasm-exceptions'], [], ['waka']),
    'except_wasm_exnref':   (['-O2', '-fwasm-exceptions', '-sWASM_EXNREF'], [], ['waka']),
    # eval_ctors 1 can partially optimize, but runs into getenv() for locale
    # code. mode 2 ignores those and fully optimizes out the ctors
    'ctors1':    (['-O2', '-sEVAL_CTORS'],   [], ['waka']),
    'ctors2':    (['-O2', '-sEVAL_CTORS=2'], [], ['waka']),
    'wasmfs':    (['-O2', '-sWASMFS'],       [], ['waka']),
    'lto':       (['-Oz', '-flto'],          [], ['waka']),
  })
  def test_codesize_cxx(self, *args):
    # do not check functions in this test as there are a lot of libc++ functions
    # pulled in here, and small LLVM backend changes can affect their size and
    # lead to different inlining decisions which add or remove a function
    self.run_codesize_test('hello_libcxx.cpp', *args, check_funcs=False)

  @parameterized({
    'O0': ([],      [], ['waka']), # noqa
    'O1': (['-O1'], [], ['waka']), # noqa
    'O2': (['-O2'], [], ['waka']), # noqa
    'O3': (['-O3'], [], []), # noqa; in -O3, -Os and -Oz we metadce
    'Os': (['-Os'], [], []), # noqa
    'Oz': (['-Oz'], [], []), # noqa
    # finally, check what happens when we export nothing. wasm should be almost empty
    'export_nothing':
          (['-Os', '-sEXPORTED_FUNCTIONS=[]'],    [], []), # noqa
    # we don't metadce with linkable code! other modules may want stuff
    # TODO(sbc): Investivate why the number of exports is order of magnitude
    # larger for wasm backend.
    'dylink': (['-O3', '-sMAIN_MODULE=2'], [], []), # noqa
    # WasmFS should not be fully linked into a hello world program.
    'wasmfs': (['-O3', '-sWASMFS'],        [], []), # noqa
  })
  def test_codesize_hello(self, *args):
    self.run_codesize_test('hello_world.c', *args)

  @parameterized({
    'O3':                 ('mem.c', ['-O3'],
                           [], []),         # noqa
    # argc/argv support code etc. is in the wasm
    'O3_standalone':      ('mem.c', ['-O3', '-sSTANDALONE_WASM'],
                           [], []),         # noqa
    # without argc/argv, no support code for them is emitted
    'O3_standalone_narg': ('mem_no_argv.c', ['-O3', '-sSTANDALONE_WASM'],
                           [], []),         # noqa
    # without main, no support code for argc/argv is emitted either
    'O3_standalone_lib':  ('mem_no_main.c', ['-O3', '-sSTANDALONE_WASM', '--no-entry'],
                           [], []),         # noqa
    # Growth support code is in JS, no significant change in the wasm
    'O3_grow':            ('mem.c', ['-O3', '-sALLOW_MEMORY_GROWTH'],
                           [], []),         # noqa
    # Growth support code is in the wasm
    'O3_grow_standalone': ('mem.c', ['-O3', '-sALLOW_MEMORY_GROWTH', '-sSTANDALONE_WASM'],
                           [], []),         # noqa
    # without argc/argv, no support code for them is emitted, even with lto
    'O3_standalone_narg_flto':
                          ('mem_no_argv.c', ['-O3', '-sSTANDALONE_WASM', '-flto'],
                           [], []),         # noqa
  })
  def test_codesize_mem(self, filename, *args):
    self.run_codesize_test(filename, *args)

  @parameterized({
    'O3':            (['-O3'],                      [], []), # noqa
    # argc/argv support code etc. is in the wasm
    'O3_standalone': (['-O3', '-sSTANDALONE_WASM'], [], []), # noqa
  })
  def test_codesize_libcxxabi_message(self, *args):
    self.run_codesize_test('libcxxabi_message.cpp', *args)

  @parameterized({
    'js_fs':  (['-O3', '-sNO_WASMFS'], [], []), # noqa
    'wasmfs': (['-O3', '-sWASMFS'],    [], []), # noqa
  })
  def test_codesize_files(self, *args):
    self.run_codesize_test('files.cpp', *args)

  # ensures runtime exports work, even with metadce
  @parameterized({
    '': (False,),
    'legacy': (True,)
  })
  def test_exported_runtime_methods_metadce(self, use_legacy_name):
    exports = ['stackSave', 'stackRestore', 'stackAlloc', 'FS']
    setting_name = 'EXPORTED_RUNTIME_METHODS'
    if use_legacy_name:
      setting_name = 'EXTRA_EXPORTED_RUNTIME_METHODS'
    err = self.run_process([EMCC, test_file('hello_world.c'), '-Os', '-s%s=%s' % (setting_name, ','.join(exports))], stderr=PIPE).stderr
    if use_legacy_name:
      self.assertContained('warning: EXTRA_EXPORTED_RUNTIME_METHODS is deprecated (please use EXPORTED_RUNTIME_METHODS instead). Please open a bug if you have a continuing need for this setting [-Wdeprecated]', err)
    js = read_file('a.out.js')
    for export in exports:
      self.assertContained(f'Module["{export}"]', js)

  def test_legalize_js_ffi(self):
    # test disabling of JS FFI legalization
    for (args, js_ffi) in [
        (['-sLEGALIZE_JS_FFI=1', '-sSIDE_MODULE', '-O1'], True),
        (['-sLEGALIZE_JS_FFI=0', '-sSIDE_MODULE', '-O1'], False),
        (['-sLEGALIZE_JS_FFI=0', '-sSIDE_MODULE', '-O0'], False),
        (['-sLEGALIZE_JS_FFI=1', '-sWARN_ON_UNDEFINED_SYMBOLS=0', '-O0'], True),
        (['-sLEGALIZE_JS_FFI=0', '-sWARN_ON_UNDEFINED_SYMBOLS=0', '-O0'], False),
      ]:
      print(args)
      delete_file('a.out.wasm')
      cmd = [EMCC, test_file('other/ffi.c'), '-g', '-o', 'a.out.wasm'] + args
      print(' '.join(cmd))
      self.run_process(cmd)
      text = self.get_wasm_text('a.out.wasm')
      # remove internal comments and extra whitespace
      text = re.sub(r'\(;[^;]+;\)', '', text)
      text = re.sub(r'\$var\$*.', '', text)
      text = re.sub(r'param \$\d+', 'param ', text)
      text = re.sub(r' +', ' ', text)
      e_add_f32 = re.search(r'func \$add_f \(param f32\) \(param f32\) \(result f32\)', text)
      assert e_add_f32, 'add_f export missing'
      i_i64_i32 = re.search(r'import "env" "import_ll" .*\(param i32 i32\) \(result i32\)', text)
      i_i64_i64 = re.search(r'import "env" "import_ll" .*\(param i64\) \(result i64\)', text)
      e_i64_i32 = re.search(r'func \$legalstub\$add_ll \(param i32\) \(param i32\) \(param i32\) \(param i32\) \(result i32\)', text)
      if js_ffi:
        assert i_i64_i32,     'i64 not converted to i32 in imports'
        assert not i_i64_i64, 'i64 not converted to i32 in imports'
        assert e_i64_i32,     'i64 not converted to i32 in exports'
      else:
        assert not i_i64_i32, 'i64 converted to i32 in imports'
        assert i_i64_i64,     'i64 converted to i32 in imports'
        assert not e_i64_i32, 'i64 converted to i32 in exports'

  @disabled('https://github.com/WebAssembly/binaryen/pull/6428')
  def test_no_legalize_js_ffi(self):
    for legalizing in (0, 1):
      # test minimal JS FFI legalization for invoke and dyncalls
      args = ['-sMAIN_MODULE=2', '-O3', '-sDISABLE_EXCEPTION_CATCHING=0', '-g']
      if not legalizing:
        args.append('-sLEGALIZE_JS_FFI=0')
      self.run_process([EMXX, test_file('other/noffi.cpp')] + args)
      text = self.get_wasm_text('a.out.wasm')
      # Verify that legalization either did, or did not, occur
      self.assertContainedIf('$legalimport', text, legalizing)
      self.assertContainedIf('$legalstub', text, legalizing)

  def test_export_aliasee(self):
    # build side module
    args = ['-sSIDE_MODULE']
    cmd = [EMCC, test_file('other/alias/side.c'), '-g', '-o', 'side.wasm'] + args
    print(' '.join(cmd))
    self.run_process(cmd)

    # build main module
    args = ['-g', '-sEXPORTED_FUNCTIONS=_main,_foo', '-sMAIN_MODULE=2', '-lnodefs.js']
    cmd = [EMCC, test_file('other/alias/main.c'), '-o', 'main.js'] + args
    print(' '.join(cmd))
    self.run_process(cmd)

    # run the program
    self.assertContained('success', self.run_js('main.js'))

  def test_sysconf_phys_pages(self):
    def run(args, expected):
      cmd = [EMCC, test_file('unistd/sysconf_phys_pages.c')] + args
      print(str(cmd))
      self.run_process(cmd)
      result = self.run_js('a.out.js').strip()
      self.assertEqual(result, f'{expected}, errno: 0')

    run([], 258)
    run(['-sINITIAL_MEMORY=32MB'], 512)
    run(['-sINITIAL_MEMORY=32MB', '-sALLOW_MEMORY_GROWTH'], (2 * 1024 * 1024 * 1024) // webassembly.WASM_PAGE_SIZE)
    run(['-sINITIAL_MEMORY=32MB', '-sALLOW_MEMORY_GROWTH', '-sWASM=0'], (2 * 1024 * 1024 * 1024) // webassembly.WASM_PAGE_SIZE)

  def test_wasm_target_and_STANDALONE_WASM(self):
    # STANDALONE_WASM means we never minify imports and exports.
    for opts, potentially_expect_minified_exports_and_imports in (
      ([],                           False),
      (['-sSTANDALONE_WASM'],        False),
      (['-O2'],                      False),
      (['-O3'],                      True),
      (['-O3', '-sSTANDALONE_WASM'], False),
      (['-Os'],                      True),
    ):
      # targeting .wasm (without .js) means we enable STANDALONE_WASM automatically, and don't minify imports/exports
      for target in ('out.js', 'out.wasm'):
        expect_minified_exports_and_imports = potentially_expect_minified_exports_and_imports and target.endswith('.js')
        standalone = target.endswith('.wasm') or 'STANDALONE_WASM' in opts
        print(opts, potentially_expect_minified_exports_and_imports, target, ' => ', expect_minified_exports_and_imports, standalone)

        self.clear()
        self.run_process([EMCC, test_file('hello_world.c'), '-o', target] + opts)
        self.assertExists('out.wasm')
        if target.endswith('.wasm'):
          # only wasm requested
          self.assertNotExists('out.js')
        wat = self.get_wasm_text('out.wasm')
        wat_lines = wat.split('\n')
        exports = [line.strip().split(' ')[1].replace('"', '') for line in wat_lines if "(export " in line]
        imports = [line.strip().split(' ')[2].replace('"', '') for line in wat_lines if "(import " in line]
        exports_and_imports = exports + imports
        print('  exports', exports)
        print('  imports', imports)
        if expect_minified_exports_and_imports:
          self.assertContained('a', exports_and_imports)
        else:
          self.assertNotContained('a', exports_and_imports)
        if standalone:
          self.assertContained('fd_write', exports_and_imports, 'standalone mode preserves import names for WASI APIs')
        # verify the wasm runs with the JS
        if target.endswith('.js'):
          self.assertContained('hello, world!', self.run_js('out.js'))
        # verify a standalone wasm
        if standalone:
          for engine in config.WASM_ENGINES:
            print(engine)
            self.assertContained('hello, world!', self.run_js('out.wasm', engine=engine))

  def test_side_module_naming(self):
    # SIDE_MODULE should work with any arbirary filename
    for opts, target in [([], 'a.out.wasm'),
                         (['-o', 'lib.wasm'], 'lib.wasm'),
                         (['-o', 'lib.so'], 'lib.so'),
                         (['-o', 'foo.bar'], 'foo.bar')]:
      # specified target
      print('building: ' + target)
      self.clear()
      self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-Werror'] + opts)
      for x in os.listdir('.'):
        self.assertFalse(x.endswith('.js'))
      self.assertTrue(building.is_wasm_dylib(target))

      create_file('main.c', '')
      self.do_runf('main.c', emcc_args=['-sMAIN_MODULE=2', 'main.c', '-Werror', target])

  def test_side_module_missing(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'libside1.wasm'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'libside2.wasm', 'libside1.wasm'])
    # When linking against `libside2.wasm` (which depends on libside1.wasm) that library path is used
    # to locate `libside1.wasm`.  Expect the link to fail with an unmodified library path.
    err = self.expect_fail([EMCC, '-sMAIN_MODULE=2', test_file('hello_world.c'), 'libside2.wasm'])
    self.assertContained('libside2.wasm: shared library dependency not found in library path: `libside1.wasm`', err)

    # But succeed if `.` is added the library path.
    self.run_process([EMCC, '-sMAIN_MODULE=2', test_file('hello_world.c'), '-L.', 'libside2.wasm'])

  def test_side_module_transitive_deps(self):
    # Build three side modules in a dependency chain
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'libside1.wasm'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'libside2.wasm', 'libside1.wasm'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'libside3.wasm', 'libside2.wasm'])

    # Link should succeed if and only if the end of the chain can be found
    final_link = [EMCC, '-sMAIN_MODULE=2', test_file('hello_world.c'), '-L.', 'libside3.wasm']
    self.run_process(final_link)
    os.remove('libside1.wasm')
    err = self.expect_fail(final_link)
    self.assertContained('error: libside2.wasm: shared library dependency not found in library path: `libside1.wasm`', err)

  def test_side_module_folder_deps(self):
    # Build side modules in a subfolder
    os.mkdir('subdir')
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'subdir/libside1.so'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sSIDE_MODULE', '-o', 'subdir/libside2.so', '-L', 'subdir', '-lside1'])
    self.run_process([EMCC, test_file('hello_world.c'), '-sMAIN_MODULE', '-o', 'main.js', '-L', 'subdir', '-lside2'])

  @is_slow_test
  @parameterized({
    '': ([],),
    '01': (['-O1'],),
    'O2': (['-O2'],),
    'O3': (['-O3'],),
    'Os': (['-Os'],),
    'Oz': (['-Oz'],),
  })
  def test_lto(self, args):
    # test building of non-wasm-object-files libraries, building with them, and running them

    # test codegen in lto mode, and compare to normal (wasm object) mode
    src = test_file('hello_libcxx.cpp')
    # wasm in object
    self.run_process([EMXX, src] + args + ['-c', '-o', 'hello_obj.o'])
    self.assertTrue(building.is_wasm('hello_obj.o'))
    self.assertFalse(is_bitcode('hello_obj.o'))

    # bitcode in object
    self.run_process([EMXX, src] + args + ['-c', '-o', 'hello_bitcode.o', '-flto'])
    self.assertFalse(building.is_wasm('hello_bitcode.o'))
    self.assertTrue(is_bitcode('hello_bitcode.o'))

    # use bitcode object (LTO)
    self.run_process([EMXX, 'hello_bitcode.o'] + args + ['-flto'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))
    # use bitcode object (non-LTO)
    self.run_process([EMXX, 'hello_bitcode.o'] + args)
    self.assertContained('hello, world!', self.run_js('a.out.js'))

    # use native object (LTO)
    self.run_process([EMXX, 'hello_obj.o'] + args + ['-flto'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))
    # use native object (non-LTO)
    self.run_process([EMXX, 'hello_obj.o'] + args)
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  @parameterized({
    'noexcept': [],
    'except': ['-sDISABLE_EXCEPTION_CATCHING=0'],
    'except_wasm': ['-fwasm-exceptions'],
    'except_wasm_exnref': ['-fwasm-exceptions', '-sWASM_EXNREF']
  })
  def test_lto_libcxx(self, *args):
    self.run_process([EMXX, test_file('hello_libcxx.cpp'), '-flto'] + list(args))

  def test_lto_flags(self):
    for flags, expect_bitcode in [
      ([], False),
      (['-flto'], True),
      (['-flto=thin'], True),
      (['-sWASM_OBJECT_FILES=0'], True),
      (['-sWASM_OBJECT_FILES'], False),
    ]:
      self.run_process([EMCC, test_file('hello_world.c')] + flags + ['-c', '-o', 'a.o'])
      seen_bitcode = is_bitcode('a.o')
      self.assertEqual(expect_bitcode, seen_bitcode, 'must emit LTO-capable bitcode when flags indicate so (%s)' % str(flags))

  # We have LTO tests covered in 'wasmltoN' targets in test_core.py, but they
  # don't run as a part of Emscripten CI, so we add a separate LTO test here.
  @requires_wasm_exnref
  def test_lto_wasm_exceptions(self):
    self.set_setting('EXCEPTION_DEBUG')
    self.emcc_args += ['-fwasm-exceptions', '-flto']
    self.do_run_in_out_file_test('core/test_exceptions.cpp', out_suffix='_caught')
    # New Wasm EH with exnref
    self.set_setting('WASM_EXNREF')
    self.do_run_in_out_file_test('core/test_exceptions.cpp', out_suffix='_caught')

  @parameterized({
    '': ([],),
    'O2': (['-O2'],),
  })
  def test_missing_wasm(self, args):
    # Check that in debug builds we show a good error message if there is no wasm support
    create_file('pre.js', 'WebAssembly = undefined;\n')
    self.run_process([EMCC, test_file('hello_world.c'), '--pre-js', 'pre.js'] + args)
    out = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContainedIf('no native wasm support detected', out, not args)

  def test_exceptions_c_linker(self):
    # Test that we don't try to create __cxa_find_matching_catch_xx function automatically
    # when not linking as C++.
    stderr = self.expect_fail([EMCC, '-sSTRICT', test_file('other/test_exceptions_c_linker.c')])
    self.assertContained('error: undefined symbol: __cxa_find_matching_catch_1', stderr)

  @with_all_eh_sjlj
  def test_exceptions_stack_trace_and_message(self):
    src = r'''
      #include <stdexcept>

      void bar() {
        throw std::runtime_error("my message");
      }
      void foo() {
        try {
          bar();
        } catch (const std::invalid_argument &err) {}
      }
      int main() {
        foo();
        return 0;
      }
    '''
    self.emcc_args += ['-g']

    # Stack trace and message example for this example code:
    # exiting due to exception: [object WebAssembly.Exception],Error: std::runtime_error,my message
    #     at src.wasm.__cxa_throw (wasm://wasm/009a7c9a:wasm-function[1551]:0x24367)
    #     at src.wasm.bar() (wasm://wasm/009a7c9a:wasm-function[12]:0xf53)
    #     at src.wasm.foo() (wasm://wasm/009a7c9a:wasm-function[19]:0x154e)
    #     at __original_main (wasm://wasm/009a7c9a:wasm-function[20]:0x15a6)
    #     at src.wasm.main (wasm://wasm/009a7c9a:wasm-function[56]:0x25be)
    #     at test.js:833:22
    #     at callMain (test.js:4567:15)
    #     at doRun (test.js:4621:23)
    #     at run (test.js:4636:5)
    stack_trace_checks = [
      'std::runtime_error[:,][ ]?my message',  # 'std::runtime_error: my message' for Emscripten EH
      'at (src.wasm.)?_?__cxa_throw',  # '___cxa_throw' (JS symbol) for Emscripten EH
      'at (src.wasm.)?bar',
      'at (src.wasm.)?foo',
      'at (src.wasm.)?main']

    if '-fwasm-exceptions' in self.emcc_args:
      # FIXME Node v18.13 (LTS as of Jan 2023) has not yet implemented the new
      # optional 'traceStack' option in WebAssembly.Exception constructor
      # (https://developer.mozilla.org/en-US/docs/WebAssembly/JavaScript_interface/Exception/Exception)
      # and embeds stack traces unconditionally. Change this back to
      # self.require_wasm_eh() if this issue is fixed later.
      self.require_v8()

    # Stack traces are enabled when either of ASSERTIONS or
    # EXCEPTION_STACK_TRACES is enabled. You can't disable
    # EXCEPTION_STACK_TRACES when ASSERTIONS is enabled.

    # Prints stack traces
    self.set_setting('ASSERTIONS', 1)
    self.set_setting('EXCEPTION_STACK_TRACES', 1)
    self.do_run(src, assert_all=True, assert_returncode=NON_ZERO,
                expected_output=stack_trace_checks, regex=True)

    # Prints stack traces
    self.set_setting('ASSERTIONS', 0)
    self.set_setting('EXCEPTION_STACK_TRACES', 1)
    self.do_run(src, assert_all=True, assert_returncode=NON_ZERO,
                expected_output=stack_trace_checks, regex=True)

    # Not allowed
    self.set_setting('ASSERTIONS', 1)
    self.set_setting('EXCEPTION_STACK_TRACES', 0)
    create_file('src.cpp', src)
    err = self.expect_fail([EMCC, 'src.cpp'] + self.get_emcc_args())
    self.assertContained('error: EXCEPTION_STACK_TRACES cannot be disabled when ASSERTIONS are enabled', err)

    # Doesn't print stack traces
    self.set_setting('ASSERTIONS', 0)
    self.set_setting('EXCEPTION_STACK_TRACES', 0)
    err = self.do_run(src, assert_returncode=NON_ZERO)
    for check in stack_trace_checks:
      self.assertFalse(re.search(check, err), 'Expected regex "%s" to not match on:\n%s' % (check, err))

  @with_all_eh_sjlj
  def test_exceptions_rethrow_stack_trace_and_message(self):
    self.emcc_args += ['-g']
    if '-fwasm-exceptions' in self.emcc_args:
      # FIXME Node v18.13 (LTS as of Jan 2023) has not yet implemented the new
      # optional 'traceStack' option in WebAssembly.Exception constructor
      # (https://developer.mozilla.org/en-US/docs/WebAssembly/JavaScript_interface/Exception/Exception)
      # and embeds stack traces unconditionally. Change this back to
      # self.require_wasm_eh() if this issue is fixed later.
      self.require_v8()
    # Rethrowing exception currently loses the stack trace before the rethrowing
    # due to how rethrowing is implemented. So in the examples below we don't
    # print 'bar' at the moment.
    # TODO Make rethrow preserve stack traces before rethrowing?
    rethrow_src1 = r'''
      #include <stdexcept>

      void important_function() {
        throw std::runtime_error("my message");
      }
      void foo() {
        try {
          important_function();
        } catch (...) {
          throw; // rethrowing by throw;
        }
      }
      int main() {
        foo();
        return 0;
      }
    '''
    rethrow_src2 = r'''
      #include <stdexcept>

      void important_function() {
        throw std::runtime_error("my message");
      }
      void foo() {
        try {
          important_function();
        } catch (...) {
          auto e = std::current_exception();
          std::rethrow_exception(e); // rethrowing by std::rethrow_exception
        }
      }
      int main() {
        foo();
        return 0;
      }
    '''
    rethrow_stack_trace_checks = [
      'std::runtime_error[:,][ ]?my message',  # 'std::runtime_error: my message' for Emscripten EH
      'at ((src.wasm.)?_?__cxa_rethrow|___resumeException)',  # '___resumeException' (JS symbol) for Emscripten EH
      'at (src.wasm.)?foo',
      'at (src.wasm.)?main']

    self.set_setting('ASSERTIONS', 1)
    err = self.do_run(rethrow_src1, assert_all=True, assert_returncode=NON_ZERO,
                      expected_output=rethrow_stack_trace_checks, regex=True)
    self.assertNotContained('important_function', err)
    err = self.do_run(rethrow_src2, assert_all=True, assert_returncode=NON_ZERO,
                      expected_output=rethrow_stack_trace_checks, regex=True)
    self.assertNotContained('important_function', err)

  @with_all_eh_sjlj
  def test_exceptions_exit_runtime(self):
    self.set_setting('EXIT_RUNTIME')
    self.do_other_test('test_exceptions_exit_runtime.cpp')

  @requires_node
  def test_jsrun(self):
    print(config.NODE_JS)
    jsrun.WORKING_ENGINES = {}
    # Test that engine check passes
    self.assertTrue(jsrun.check_engine(config.NODE_JS))
    # Run it a second time (cache hit)
    self.assertTrue(jsrun.check_engine(config.NODE_JS))

    # Test that engine check fails
    bogus_engine = ['/fake/inline4']
    self.assertFalse(jsrun.check_engine(bogus_engine))
    self.assertFalse(jsrun.check_engine(bogus_engine))

    # Test the other possible way (list vs string) to express an engine
    if type(config.NODE_JS) is list:
      engine2 = config.NODE_JS[0]
    else:
      engine2 = [config.NODE_JS]
    self.assertTrue(jsrun.check_engine(engine2))

    # Test that self.run_js requires the engine
    self.run_js(test_file('hello_world.js'), config.NODE_JS)
    caught_exit = 0
    try:
      self.run_js(test_file('hello_world.js'), bogus_engine)
    except SystemExit as e:
      caught_exit = e.code
    self.assertEqual(1, caught_exit, 'Did not catch SystemExit with bogus JS engine')

  def test_error_on_missing_libraries(self):
    # -llsomenonexistingfile is an error by default
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-lsomenonexistingfile'])
    self.assertContained('wasm-ld: error: unable to find library -lsomenonexistingfile', err)

  # Tests that if user accidentally attempts to link native object code, we show an error
  def test_native_link_error_message(self):
    self.run_process([CLANG_CC, '--target=' + clang_native.get_native_triple(), '-c', test_file('hello_123.c'), '-o', 'hello_123.o'])
    err = self.expect_fail([EMCC, 'hello_123.o', '-o', 'hello_123.js'])
    self.assertContained('unknown file type: hello_123.o', err)

  # Tests that we should give a clear error on INITIAL_MEMORY not being enough for static initialization + stack
  def test_clear_error_on_massive_static_data(self):
    create_file('src.c', '''
        char muchData[128 * 1024];
        int main() {
          return (int)(long)&muchData;
        }
    ''')
    err = self.expect_fail([EMCC, 'src.c', '-sSTACK_SIZE=1KB', '-sINITIAL_MEMORY=64KB'])
    self.assertContained('wasm-ld: error: initial memory too small', err)

  def test_o_level_clamp(self):
    for level in (3, 4, 20):
      err = self.run_process([EMCC, '-O' + str(level), test_file('hello_world.c')], stderr=PIPE).stderr
      self.assertContainedIf("optimization level '-O" + str(level) + "' is not supported; using '-O3' instead", err, level > 3)

  def test_o_level_invalid(self):
    # Test that string values, and negative integers are not accepted
    err = self.expect_fail([EMCC, '-Ofoo', test_file('hello_world.c')])
    self.assertContained('emcc: error: invalid optimization level: -Ofoo', err)
    err = self.expect_fail([EMCC, '-O-10', test_file('hello_world.c')])
    self.assertContained('emcc: error: invalid optimization level: -O-10', err)

  def test_g_level_invalid(self):
    # Bad integer values are handled by emcc
    err = self.expect_fail([EMCC, '-g5', test_file('hello_world.c')])
    self.assertContained('emcc: error: invalid debug level: -g5', err)
    err = self.expect_fail([EMCC, '-g-10', test_file('hello_world.c')])
    self.assertContained('emcc: error: invalid debug level: -g-10', err)
    # Unknown string values are passed through to clang which will error out
    err = self.expect_fail([EMCC, '-gfoo', test_file('hello_world.c')])
    self.assertContained("error: unknown argument: '-gfoo'", err)

  # Tests that if user specifies multiple -o output directives, then the last one will take precedence
  def test_multiple_o_files(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-o', 'a.js', '-o', 'b.js'])
    assert os.path.isfile('b.js')
    assert not os.path.isfile('a.js')

  # Tests that Emscripten-provided header files can be cleanly included standalone.
  # Also check they can be included in C code (where possible).
  @is_slow_test
  def test_standalone_system_headers(self):
    # Test oldest C standard, and the default C standard
    # This also tests that each header file is self contained and includes
    # everything it needs.
    directories = {'': []}
    for elem in os.listdir(path_from_root('system/include')):
      if elem in ('compat', 'fakesdl'):
        continue
      full = path_from_root('system/include', elem)
      if os.path.isdir(full):
        directories[elem] = os.listdir(full)
      else:
        directories[''].append(elem)

    for directory, headers in directories.items():
      print('dir: ' + directory)
      for header in headers:
        if not header.endswith('.h'):
          continue
        print('header: ' + header)
        # These headers cannot be included in isolation.
        # e.g: error: unknown type name 'EGLDisplay'
        if header in ['eglext.h', 'SDL_config_macosx.h', 'glext.h', 'gl2ext.h']:
          continue
        # These headers are C++ only and cannot be included from C code.
        # But we still want to check they can be included on there own without
        # any errors or warnings.
        cxx_only = header in [
          'wire.h', 'val.h', 'bind.h',
          'webgpu_cpp.h', 'webgpu_cpp_chained_struct.h', 'webgpu_enum_class_bitmasks.h',
        ]
        if directory:
          header = f'{directory}/{header}'
        inc = f'#include <{header}>\n__attribute__((weak)) int foo;\n'
        if cxx_only:
          create_file('a.cxx', inc)
          create_file('b.cxx', inc)
          self.run_process([EMXX, '-Werror', '-Wall', '-pedantic', 'a.cxx', 'b.cxx'])
        else:
          create_file('a.c', inc)
          create_file('b.c', inc)
          for std in ([], ['-std=c89']):
            self.run_process([EMCC] + std + ['-Werror', '-Wall', '-pedantic', 'a.c', 'b.c'])

  @is_slow_test
  @parameterized({
    '': (True,),
    'disabled': (False,),
  })
  @also_with_wasm2js
  def test_single_file(self, single_file_enabled):
    for (debug_enabled,
         closure_enabled) in itertools.product([True, False], repeat=2):
      # skip unhelpful option combinations
      if closure_enabled and debug_enabled:
        continue

      cmd = [EMCC, test_file('hello_world.c')] + self.get_emcc_args()

      if single_file_enabled:
        expect_wasm = False
        cmd += ['-sSINGLE_FILE']
      else:
        expect_wasm = self.is_wasm()

      if debug_enabled:
        cmd += ['-g']
      if closure_enabled:
        cmd += ['--closure=1', '-Wno-closure']

      self.clear()

      def do_test(cmd):
        print(' '.join(cmd))
        self.run_process(cmd)
        print(os.listdir('.'))
        if expect_wasm:
          self.assertExists('a.out.wasm')
        else:
          self.assertNotExists('a.out.wasm')
        self.assertNotExists('a.out.wat')
        self.assertContained('hello, world!', self.run_js('a.out.js'))

      do_test(cmd)

      # additional combinations that are not part of the big product()

      if debug_enabled:
        separate_dwarf_cmd = cmd + ['-gseparate-dwarf']
        if self.is_wasm2js():
          self.expect_fail(separate_dwarf_cmd)
        else:
          do_test(separate_dwarf_cmd)
          self.assertExists('a.out.wasm.debug.wasm')

  @requires_v8
  def test_single_file_shell(self):
    self.do_runf('hello_world.c', emcc_args=['-sSINGLE_FILE'])

  @requires_v8
  def test_single_file_shell_sync_compile(self):
    self.do_runf('hello_world.c', emcc_args=['-sSINGLE_FILE', '-sWASM_ASYNC_COMPILATION=0'])

  def test_single_file_no_clobber_wasm(self):
    create_file('hello_world.wasm', 'not wasm')
    self.do_runf('hello_world.c', emcc_args=['-sSINGLE_FILE'])
    self.assertExists('hello_world.js')
    self.assertFileContents('hello_world.wasm', 'not wasm')

  def test_wasm2js_no_clobber_wasm(self):
    create_file('hello_world.wasm', 'not wasm')
    self.do_runf('hello_world.c', emcc_args=['-sWASM=0'])
    self.assertExists('hello_world.js')
    self.assertFileContents('hello_world.wasm', 'not wasm')

  def test_emar_M(self):
    create_file('file1', ' ')
    create_file('file2', ' ')
    self.run_process([EMAR, 'cr', 'file1.a', 'file1'])
    self.run_process([EMAR, 'cr', 'file2.a', 'file2'])
    self.run_process([EMAR, '-M'], input='''create combined.a
addlib file1.a
addlib file2.a
save
end
''')
    result = self.run_process([EMAR, 't', 'combined.a'], stdout=PIPE).stdout
    self.assertContained('file1', result)
    self.assertContained('file2', result)

  def test_emar_duplicate_inputs(self):
    # Verify the we can supply the same intput muliple times without
    # confusing emar.py:
    # See https://github.com/emscripten-core/emscripten/issues/9733
    create_file('file1', ' ')
    self.run_process([EMAR, 'cr', 'file1.a', 'file1', 'file1'])

  @crossplatform
  def test_emar_response_file(self):
    # Test that special character such as single quotes in filenames survive being
    # sent via response file
    create_file("file'1", ' ')
    create_file("file'2", ' ')
    create_file("hyvää päivää", ' ')
    create_file("snowman freezes covid ☃ 🦠", ' ')
    rsp = response_file.create_response_file(("file'1", "file'2", "hyvää päivää", "snowman freezes covid ☃ 🦠"), shared.TEMP_DIR)
    building.emar('cr', 'libfoo.a', ['@' + rsp])

  def test_response_file_bom(self):
    # Modern CMake version create response fils in UTF-8 but with BOM
    # at the begining.  Test that we can handle this.
    # https://docs.python.org/3/library/codecs.html#encodings-and-unicode
    create_file('test.rsp', b'\xef\xbb\xbf--version', binary=True)
    self.run_process([EMCC, '@test.rsp'])

  def test_archive_non_objects(self):
    create_file('file.txt', 'test file')
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    # No index added.
    # --format=darwin (the default on OSX has a strange issue where it add extra
    # newlines to files: https://bugs.llvm.org/show_bug.cgi?id=42562
    self.run_process([EMAR, 'crS', '--format=gnu', 'libfoo.a', 'file.txt', 'hello_world.o'])
    self.run_process([EMCC, test_file('hello_world.c'), 'libfoo.a'])

  def test_archive_thin(self):
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    # The `T` flag means "thin"
    self.run_process([EMAR, 'crT', 'libhello.a', 'hello_world.o'])
    self.run_process([EMCC, 'libhello.a'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_flag_aliases(self):
    def assert_aliases_match(flag1, flag2, flagarg, extra_args):
      results = {}
      for f in (flag1, flag2):
        self.run_process([EMCC, test_file('hello_world.c'), '-s', f + '=' + flagarg] + extra_args)
        results[f + '.js'] = read_file('a.out.js')
        results[f + '.wasm'] = read_binary('a.out.wasm')
      self.assertEqual(results[flag1 + '.js'], results[flag2 + '.js'], 'js results should be identical')
      self.assertEqual(results[flag1 + '.wasm'], results[flag2 + '.wasm'], 'wasm results should be identical')

    assert_aliases_match('INITIAL_MEMORY', 'TOTAL_MEMORY', '16777216', [])
    assert_aliases_match('INITIAL_MEMORY', 'TOTAL_MEMORY', '64MB', [])
    assert_aliases_match('MAXIMUM_MEMORY', 'WASM_MEM_MAX', '16777216', ['-sALLOW_MEMORY_GROWTH'])
    assert_aliases_match('MAXIMUM_MEMORY', 'BINARYEN_MEM_MAX', '16777216', ['-sALLOW_MEMORY_GROWTH'])

  def test_IGNORE_CLOSURE_COMPILER_ERRORS(self):
    create_file('pre.js', r'''
      // make closure compiler very very angry
      var dupe = 1;
      var dupe = 2;
      function Node() {
        throw 'Node is a DOM thing too, and use the ' + dupe;
      }
      function Node() {
        throw '(duplicate) Node is a DOM thing too, and also use the ' + dupe;
      }
    ''')

    def test(check, extra):
      cmd = [EMCC, test_file('hello_world.c'), '-O2', '--closure=1', '--pre-js', 'pre.js'] + extra
      proc = self.run_process(cmd, check=check, stderr=PIPE)
      if not check:
        self.assertNotEqual(proc.returncode, 0)
      return proc

    WARNING = 'Variable dupe declared more than once'

    proc = test(check=False, extra=[])
    self.assertContained(WARNING, proc.stderr)
    proc = test(check=True, extra=['-sIGNORE_CLOSURE_COMPILER_ERRORS'])
    self.assertNotContained(WARNING, proc.stderr)

  @parameterized({
    '': ([],),
    'asyncify': (['-sASYNCIFY'],),
    # set max_memory to 4GB to test handleI64Signatures() with GL emulation
    'gl_emu': (['-sLEGACY_GL_EMULATION', '-sMAXIMUM_MEMORY=4GB', '-sALLOW_MEMORY_GROWTH'],),
    'no_exception_throwing': (['-sDISABLE_EXCEPTION_THROWING'],),
    'minimal_runtime': (['-sMINIMAL_RUNTIME'],),
    'embind': (['-lembind'],),
  })
  def test_full_js_library(self, args):
    self.run_process([EMCC, test_file('hello_world.c'), '-sSTRICT_JS', '-sINCLUDE_FULL_LIBRARY'] + args)

  def test_full_js_library_undefined(self):
    create_file('main.c', 'void foo(); int main() { foo(); return 0; }')
    err = self.expect_fail([EMCC, 'main.c', '-sSTRICT_JS', '-sINCLUDE_FULL_LIBRARY'])
    self.assertContained('undefined symbol: foo', err)

  def test_full_js_library_except(self):
    self.set_setting('INCLUDE_FULL_LIBRARY', 1)
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)
    self.do_other_test('test_full_js_library_except.cpp')

  @crossplatform
  @parameterized({
    '': [[]],
    # bigint support is interesting to test here because it changes which
    # binaryen tools get run, which can affect how debug info is kept around
    'bigint': [['-sWASM_BIGINT']],
    'pthread': [['-pthread', '-Wno-experimental']],
    'pthread_offscreen': [['-pthread', '-Wno-experimental', '-sOFFSCREEN_FRAMEBUFFER']],
    'wasmfs': [['-sWASMFS']],
    'min_webgl_version': [['-sMIN_WEBGL_VERSION=2', '-sLEGACY_GL_EMULATION=0']],
  })
  def test_closure_full_js_library(self, args):
    # Test for closure errors and warnings in the entire JS library.
    self.build(test_file('hello_world.c'), emcc_args=[
      '--closure=1',
      '--minify=0',
      '-Werror=closure',
      '-sINCLUDE_FULL_LIBRARY',
      '-sOFFSCREEN_FRAMEBUFFER',
      # Enable as many features as possible in order to maximise
      # tha amount of library code we inculde here.
      '-sMAIN_MODULE',
      '-sFETCH',
      '-sFETCH_SUPPORT_INDEXEDDB',
      '-sLEGACY_GL_EMULATION',
      '-sMAX_WEBGL_VERSION=2',
    ] + args)

    # Check that closure doesn't minify certain attributes.
    # This list allows us to verify that it's safe to use normal accessors over
    # string accessors.
    glsyms = [
      'compressedTexImage2D',
      'compressedTexSubImage2D',
      'compressedTexImage3D',
      'compressedTexSubImage3D',
      'bindVertexArray',
      'deleteVertexArray',
      'createVertexArray',
      'isVertexArray',
      'getQueryParameter',
      'drawElementsInstanced',
      'drawBuffers',
      'drawArraysInstanced',
      'vertexAttribDivisor',
      'getInternalformatParameter',
      'beginQuery',
      'getQuery',
      'clearBufferfv',
      'clearBufferuiv',
      'getFragDataLocation',
      'vertexAttribIPointer',
      'samplerParameteri',
      'samplerParameterf',
      'isTransformFeedback',
      'deleteTransformFeedback',
      'transformFeedbackVaryings',
      'getSamplerParameter',
      'uniformBlockBindin',
      'vertexAttribIPointer',
    ]
    js = read_file('hello_world.js')
    for sym in glsyms:
      self.assertContained('.' + sym, js)

  @also_with_wasm64
  def test_closure_webgpu(self):
    # This test can be removed if USE_WEBGPU is later included in INCLUDE_FULL_LIBRARY.
    self.build(test_file('hello_world.c'), emcc_args=[
      '--closure=1',
      '-Werror=closure',
      '-sINCLUDE_FULL_LIBRARY',
      '-sUSE_WEBGPU'
    ])

  # Tests --closure-args command line flag
  @crossplatform
  def test_closure_externs(self):
    # Test with relocate path to the externs file to ensure that incoming relative paths
    # are translated correctly (Since closure runs with a different CWD)
    shutil.copy(test_file('test_closure_externs.js'), 'local_externs.js')
    test_cases = (
      ['--closure-args', '--externs "local_externs.js"'],
      ['--closure-args', '--externs=local_externs.js'],
      ['--closure-args=--externs=local_externs.js'],
    )
    for args in test_cases:
      self.run_process([EMCC, test_file('hello_world.c'),
                        '--closure=1',
                        '--pre-js', test_file('test_closure_externs_pre_js.js')] +
                       args)

  # Tests that it is possible to enable the Closure compiler via --closure=1 even if any of the input files reside in a path with unicode characters.
  def test_closure_cmdline_utf8_chars(self):
    test = "☃ äö Ć € ' 🦠.c"
    shutil.copy(test_file('hello_world.c'), test)
    externs = '💩' + test
    create_file(externs, '')
    self.run_process([EMCC, test, '--closure=1', '--closure-args', '--externs "' + externs + '"'])

  def test_closure_type_annotations(self):
    # Verify that certain type annotations exist to allow closure to avoid
    # ambiguity and maximize optimization opportunities in user code.
    #
    # Currently we check for a fixed set of known attribute names which
    # have been reported as unannoted in the past:
    attributes = ['put', 'getContext', 'contains', 'stopPropagation', 'pause']
    methods = ''
    for attribute in attributes:
      methods += f'''
        this.{attribute} = function() {{
          console.error("my {attribute}");
        }};
      '''
    create_file('post.js', '''
      /** @constructor */
      function Foo() {
        this.bar = function() {
          console.error("my bar");
        };
        this.baz = function() {
          console.error("my baz");
        };
        %s
        return this;
      }

      function getObj() {
        return new Foo();
      }

      var testobj = getObj();
      testobj.bar();

      /** Also keep alive certain library functions */
      Module['keepalive'] = [_emscripten_start_fetch, _emscripten_pause_main_loop, _SDL_AudioQuit];
    ''' % methods)

    self.build(test_file('hello_world.c'), emcc_args=[
      '--closure=1',
      '-sINCLUDE_FULL_LIBRARY',
      '-sFETCH',
      '-sFETCH_SUPPORT_INDEXEDDB',
      '-Werror=closure',
      '--post-js=post.js'
    ])
    code = read_file('hello_world.js')
    # `bar` method is used so should not be DCE'd
    self.assertContained('my bar', code)
    # `baz` method is not used
    self.assertNotContained('my baz', code)

    for attribute in attributes:
      # None of the attributes in our list should be used either and should therefore
      # be DCE'd unless there is some usage of that name within emscripten that is
      # not annotated.
      if 'my ' + attribute in code:
        self.assertFalse('Attribute `%s` could not be DCEd' % attribute)

  @crossplatform
  @with_env_modify({'EMPROFILE': '1'})
  def test_toolchain_profiler(self):
    # Verify some basic functionality of EMPROFILE
    self.run_process([emprofile, '--reset'])
    err = self.expect_fail([emprofile, '--graph'])
    self.assertContained('No profiler logs were found', err)

    self.run_process([EMCC, test_file('hello_world.c')])
    self.assertEqual('hello, world!', self.run_js('a.out.js').strip())

    self.run_process([emprofile, '--graph'])
    self.assertTrue(glob.glob('toolchain_profiler.results*.html'))

  @with_env_modify({'EMPROFILE': '2'})
  def test_toolchain_profiler_stderr(self):
    stderr = self.run_process([EMCC, test_file('hello_world.c')], stderr=PIPE).stderr
    self.assertContained('start block "main"', stderr)
    self.assertContained('block "main" took', stderr)

  @also_with_wasmfs
  @crossplatform
  def test_noderawfs(self):
    self.run_process([EMXX, test_file('fs/test_fopen_write.cpp'), '-sNODERAWFS'] + self.get_emcc_args())
    self.assertContained("read 11 bytes. Result: Hello data!", self.run_js('a.out.js'))

    # NODERAWFS should directly write on OS file system
    self.assertEqual("Hello data!", read_file('hello_file.txt'))

  def test_noderawfs_disables_embedding(self):
    expected = '--preload-file and --embed-file cannot be used with NODERAWFS which disables virtual filesystem'
    base = [EMCC, test_file('hello_world.c'), '-sNODERAWFS']
    create_file('somefile', 'foo')
    err = self.expect_fail(base + ['--preload-file', 'somefile'])
    self.assertContained(expected, err)
    err = self.expect_fail(base + ['--embed-file', 'somefile'])
    self.assertContained(expected, err)

  def test_noderawfs_access_abspath(self):
    create_file('foo', 'bar')
    create_file('access.c', r'''
      #include <unistd.h>
      int main(int argc, char** argv) {
        return access(argv[1], F_OK);
      }
    ''')
    self.run_process([EMCC, 'access.c', '-sNODERAWFS'])
    self.run_js('a.out.js', args=[os.path.abspath('foo')])

  def test_noderawfs_readfile_prerun(self):
    create_file('foo', 'bar')
    self.add_pre_run("console.log(FS.readFile('foo', { encoding: 'utf8' }));")
    self.do_runf('hello_world.c', 'bar', emcc_args=['-sNODERAWFS', '-sFORCE_FILESYSTEM'])

  @disabled('https://github.com/nodejs/node/issues/18265')
  def test_node_code_caching(self):
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-sNODE_CODE_CACHING',
                      '-sWASM_ASYNC_COMPILATION=0'])

    def get_cached():
      cached = glob.glob('a.out.wasm.*.cached')
      if not cached:
        return None
      self.assertEqual(len(cached), 1)
      return cached[0]

    # running the program makes it cache the code
    self.assertFalse(get_cached())
    self.assertEqual('hello, world!', self.run_js('a.out.js').strip())
    self.assertTrue(get_cached(), 'should be a cache file')

    # hard to test it actually uses it to speed itself up, but test that it
    # does try to deserialize it at least
    create_file(get_cached(), 'waka waka')
    ERROR = 'NODE_CODE_CACHING: failed to deserialize, bad cache file?'
    self.assertContained(ERROR, self.run_js('a.out.js'))
    # we cached proper code after showing that error
    self.assertEqual(read_binary(get_cached()).count(b'waka'), 0)
    self.assertNotContained(ERROR, self.run_js('a.out.js'))

  @with_env_modify({'LC_ALL': 'C'})
  def test_autotools_shared_check(self):
    expected = ': supported targets:.* elf'
    out = self.run_process([EMCC, '--help'], stdout=PIPE).stdout
    assert re.search(expected, out)

  @also_with_wasmfs
  def test_ioctl_window_size(self):
      self.do_other_test('test_ioctl_window_size.cpp')

  @also_with_wasmfs
  def test_ioctl(self):
    # ioctl requires filesystem
    self.do_other_test('test_ioctl.c', emcc_args=['-sFORCE_FILESYSTEM'])

  # @also_with_noderawfs # NODERAWFS needs to implement the ioctl syscalls, see issue #22264.
  def test_ioctl_termios(self):
    # ioctl requires filesystem
    self.do_other_test('test_ioctl_termios.c', emcc_args=['-sFORCE_FILESYSTEM'])

  def test_fd_closed(self):
    self.do_other_test('test_fd_closed.cpp')

  def test_fflush(self):
    # fflush without the full filesystem won't quite work
    self.do_other_test('test_fflush.cpp')

  @also_with_wasmfs
  def test_fflush_fs(self):
    # fflush with the full filesystem will flush from libc, but not the JS logging, which awaits a newline
    self.do_other_test('test_fflush_fs.cpp', emcc_args=['-sFORCE_FILESYSTEM'])

  @also_with_noderawfs
  def test_fflush_fs_exit(self):
    # on exit, we can send out a newline as no more code will run
    self.do_other_test('test_fflush_fs_exit.cpp', emcc_args=['-sFORCE_FILESYSTEM', '-sEXIT_RUNTIME'])

  def test_extern_weak(self):
    self.do_other_test('test_extern_weak.c')

  def test_extern_weak_dynamic(self):
    # If the symbols are left undefined we should get the same expected output as with static
    # linking (i.e. null symbol addresses);
    self.do_other_test('test_extern_weak.c', emcc_args=['-sMAIN_MODULE=2'])
    self.run_process([EMCC, '-o', 'libside.wasm', test_file('other/test_extern_weak_side.c'), '-sSIDE_MODULE'])
    self.do_other_test('test_extern_weak.c', out_suffix='.resolved', emcc_args=['-sMAIN_MODULE=2', 'libside.wasm'])

  def test_main_module_without_main(self):
    create_file('pre.js', 'Module.onRuntimeInitialized = () => Module._foo();')
    create_file('src.c', r'''
#include <emscripten.h>
#include <emscripten/console.h>
EMSCRIPTEN_KEEPALIVE void foo() {
  emscripten_out("bar");
}
''')
    self.do_runf('src.c', 'bar', emcc_args=['--pre-js', 'pre.js', '-sMAIN_MODULE=2'])

  @crossplatform
  def test_js_optimizer_parse_error(self):
    # check we show a proper understandable error for JS parse problems
    create_file('src.c', r'''
#include <emscripten.h>
EM_JS(void, js, (void), {
  var x = !<->5.; // wtf... this will error on the '<'
});
int main() {
  js();
}
''')
    stderr = self.expect_fail([EMCC, 'src.c', '-O2'] + self.get_emcc_args())
    self.assertContained(('''
function js() { var x = !<->5.; }
                         ^
'''), stderr)

  @crossplatform
  def test_js_optimizer_chunk_size_determinism(self):
    def build():
      self.run_process([EMCC, test_file('hello_world.c'), '-O3', '-sWASM=0'])
      # FIXME: newline differences can exist, ignore for now
      return read_file('a.out.js').replace('\n', '')

    normal = build()

    with env_modify({
      'EMCC_JSOPT_MIN_CHUNK_SIZE': '1',
      'EMCC_JSOPT_MAX_CHUNK_SIZE': '1'
    }):
      tiny = build()

    with env_modify({
      'EMCC_JSOPT_MIN_CHUNK_SIZE': '4294967296',
      'EMCC_JSOPT_MAX_CHUNK_SIZE': '4294967296'
    }):
      huge = build()

    self.assertIdentical(normal, tiny)
    self.assertIdentical(normal, huge)

  @crossplatform
  def test_js_optimizer_verbose(self):
    # build at -O3 with wasm2js to use as much as possible of the JS
    # optimization code, and verify it works ok in verbose mode
    self.run_process([EMCC, test_file('hello_world.c'), '-O3', '-sWASM=0',
                      '-sVERBOSE'], stdout=PIPE, stderr=PIPE)

  def test_pthreads_growth_and_unsigned(self):
    create_file('src.c', r'''
#include <emscripten.h>

int main() {
  EM_ASM({
    HEAP8.set([1,2,3], $0);
  }, 1024);
}''')
    self.run_process([EMCC, 'src.c', '-O2', '--profiling', '-pthread',
                      '-sMAXIMUM_MEMORY=4GB', '-sALLOW_MEMORY_GROWTH'])
    # growable-heap must not interfere with heap unsigning, and vice versa:
    # we must have both applied, that is
    #   - GROWABLE_HEAP_I8() replaces HEAP8
    #   - $0 gets an >>> 0 unsigning
    self.assertContained('GROWABLE_HEAP_I8().set([ 1, 2, 3 ], $0 >>> 0)',
                         read_file('a.out.js'))

  @parameterized({
    '': ([],), # noqa
    'O3': (['-O3'],), # noqa
    'closure': (['--closure=1'],), # noqa
    'closure_O3': (['--closure=1', '-O3'],), # noqa
  })
  def test_EM_ASM_ES6(self, args):
    create_file('src.c', r'''
#include <emscripten.h>
int main() {
  EM_ASM({
    let x = (a, b) => 5; // valid ES6
    async function y() {} // valid ES2017
    out('hello!');
    return x;
  });
}
''')
    self.do_runf('src.c', 'hello!', emcc_args=args)

  def test_check_sourcemapurl(self):
    if self.is_wasm2js():
      self.skipTest('only supported with wasm')
    self.run_process([EMCC, test_file('hello_123.c'), '-gsource-map', '-o', 'a.js', '--source-map-base', 'dir/'])
    output = read_binary('a.wasm')
    # has sourceMappingURL section content and points to 'dir/a.wasm.map' file
    source_mapping_url_content = webassembly.to_leb(len('sourceMappingURL')) + b'sourceMappingURL' + webassembly.to_leb(len('dir/a.wasm.map')) + b'dir/a.wasm.map'
    self.assertEqual(output.count(source_mapping_url_content), 1)
    # make sure no DWARF debug info sections remain - they would just waste space
    self.assertNotIn(b'.debug_', output)

  def test_check_source_map_args(self):
    # -gsource-map is needed for source maps; -g is not enough
    self.run_process([EMCC, test_file('hello_world.c'), '-g'])
    self.assertNotExists('a.out.wasm.map')
    self.run_process([EMCC, test_file('hello_world.c'), '-gsource-map'])
    self.assertExists('a.out.wasm.map')
    os.unlink('a.out.wasm.map')
    err = self.run_process([EMCC, test_file('hello_world.c'), '-g4'], stderr=subprocess.PIPE).stderr
    self.assertIn('please replace -g4 with -gsource-map', err)
    self.assertExists('a.out.wasm.map')

  @parameterized({
    'normal': [],
    'profiling': ['--profiling'] # -gsource-map --profiling should still emit a source map; see #8584
  })
  def test_check_sourcemapurl_default(self, *args):
    if self.is_wasm2js():
      self.skipTest('only supported with wasm')

    self.run_process([EMCC, test_file('hello_123.c'), '-gsource-map', '-o', 'a.js'] + list(args))
    output = read_binary('a.wasm')
    # has sourceMappingURL section content and points to 'a.wasm.map' file
    source_mapping_url_content = webassembly.to_leb(len('sourceMappingURL')) + b'sourceMappingURL' + webassembly.to_leb(len('a.wasm.map')) + b'a.wasm.map'
    self.assertIn(source_mapping_url_content, output)

  def test_wasm_sourcemap(self):
    # The no_main.c will be read (from relative location) due to speficied "-s"
    shutil.copy(test_file('other/wasm_sourcemap/no_main.c'), '.')
    wasm_map_cmd = [PYTHON, path_from_root('tools/wasm-sourcemap.py'),
                    '--sources', '--prefix', '=wasm-src://',
                    '--load-prefix', '/emscripten/test/other/wasm_sourcemap=.',
                    '--dwarfdump-output',
                    test_file('other/wasm_sourcemap/foo.wasm.dump'),
                    '-o', 'a.out.wasm.map',
                    test_file('other/wasm_sourcemap/foo.wasm'),
                    '--basepath=' + os.getcwd()]
    self.run_process(wasm_map_cmd)
    output = read_file('a.out.wasm.map')
    # has "sources" entry with file (includes also `--prefix =wasm-src:///` replacement)
    self.assertIn('wasm-src:///emscripten/test/other/wasm_sourcemap/no_main.c', output)
    # has "sourcesContent" entry with source code (included with `-s` option)
    self.assertIn('int foo()', output)
    # has some entries
    self.assertRegexpMatches(output, r'"mappings":\s*"[A-Za-z0-9+/]')

  def test_wasm_sourcemap_dead(self):
    wasm_map_cmd = [PYTHON, path_from_root('tools/wasm-sourcemap.py'),
                    '--dwarfdump-output',
                    test_file('other/wasm_sourcemap_dead/t.wasm.dump'),
                    '-o', 'a.out.wasm.map',
                    test_file('other/wasm_sourcemap_dead/t.wasm'),
                    '--basepath=' + os.getcwd()]
    self.run_process(wasm_map_cmd, stdout=PIPE, stderr=PIPE)
    output = read_file('a.out.wasm.map')
    # has only two entries
    self.assertRegexpMatches(output, r'"mappings":\s*"[A-Za-z0-9+/]+,[A-Za-z0-9+/]+"')

  def test_wasm_sourcemap_relative_paths(self):
    ensure_dir('build')

    def test(infile, source_map_added_dir=''):
      expected_source_map_path = 'a.cpp'
      if source_map_added_dir:
        expected_source_map_path = source_map_added_dir + '/' + expected_source_map_path
      print(infile, expected_source_map_path)
      shutil.copy(test_file('hello_123.c'), infile)
      infiles = [
        infile,
        os.path.abspath(infile),
        './' + infile
      ]
      for curr in infiles:
        print('  ', curr)
        print('    ', 'same CWD for compiler and linker')
        self.run_process([EMCC, curr, '-gsource-map'])
        self.assertIn('"%s"' % expected_source_map_path, read_file('a.out.wasm.map'))

        print('    ', 'different CWD for compiler and linker')
        self.run_process([EMCC, curr, '-g', '-c', '-o', 'build/a.o'])
        self.run_process([EMCC, 'a.o', '-gsource-map'], cwd='build')
        self.assertIn('"../%s"' % expected_source_map_path, read_file('build/a.out.wasm.map'))

    test('a.cpp')

    ensure_dir('inner')
    test('inner/a.cpp', 'inner')

  def test_wasm_sourcemap_extract_comp_dir_map(self):
    wasm_sourcemap = importlib.import_module('tools.wasm-sourcemap')

    def test(dump_file):
      dwarfdump_output = read_file(
          test_file(
              os.path.join('other/wasm_sourcemap_extract_comp_dir_map',
                           dump_file)))
      map_stmt_list_to_comp_dir = wasm_sourcemap.extract_comp_dir_map(
          dwarfdump_output)
      self.assertEqual(map_stmt_list_to_comp_dir,
                       {'0x00000000': '/emsdk/emscripten'})

    # Make sure we can extract the compilation directories no matter what the
    # order of `DW_AT_*` attributes is.
    test('foo.wasm.dump')
    test('bar.wasm.dump')

  def get_instr_addr(self, text, filename):
    '''
    Runs llvm-objdump to get the address of the first occurrence of the
    specified line within the given function. llvm-objdump's output format
    example is as follows:
    ...
    00000004 <foo>:
          ...
          6: 41 00         i32.const       0
          ...
    The addresses here are the offsets to the start of the file. Returns
    the address string in hexadecimal.
    '''
    out = self.run_process([common.LLVM_OBJDUMP, '-d', filename],
                           stdout=PIPE).stdout.strip()
    out_lines = out.splitlines()
    found = False
    for line in out_lines:
      if text in line:
        offset = line.strip().split(':')[0]
        found = True
        break
    assert found
    return '0x' + offset

  def test_emsymbolizer_srcloc(self):
    'Test emsymbolizer use cases that provide src location granularity info'
    def check_dwarf_loc_info(address, funcs, locs):
      out = self.run_process(
          [emsymbolizer, '-s', 'dwarf', 'test_dwarf.wasm', address],
          stdout=PIPE).stdout
      for func in funcs:
        self.assertIn(func, out)
      for loc in locs:
        self.assertIn(loc, out)

    def check_source_map_loc_info(address, loc):
      out = self.run_process(
          [emsymbolizer, '-s', 'sourcemap', 'test_dwarf.wasm', address],
          stdout=PIPE).stdout
      self.assertIn(loc, out)

    # We test two locations within test_dwarf.c:
    # out_to_js(0);     // line 6
    # __builtin_trap(); // line 13
    self.run_process([EMCC, test_file('core/test_dwarf.c'),
                      '-g', '-gsource-map', '-O1', '-o', 'test_dwarf.js'])
    # Address of out_to_js(0) within foo(), uninlined
    out_to_js_call_addr = self.get_instr_addr('call\t0', 'test_dwarf.wasm')
    # Address of __builtin_trap() within bar(), inlined into main()
    unreachable_addr = self.get_instr_addr('unreachable', 'test_dwarf.wasm')

    # Function name of out_to_js(0) within foo(), uninlined
    out_to_js_call_func = ['foo']
    # Function names of __builtin_trap() within bar(), inlined into main(). The
    # first one corresponds to the innermost inlined function.
    unreachable_func = ['bar', 'main']

    # Source location of out_to_js(0) within foo(), uninlined
    out_to_js_call_loc = ['test_dwarf.c:6:3']
    # Source locations of __builtin_trap() within bar(), inlined into main().
    # The first one corresponds to the innermost inlined location.
    unreachable_loc = ['test_dwarf.c:13:3', 'test_dwarf.c:18:3']

    # 1. Test DWARF + source map together
    # For DWARF, we check for the full inlined info for both function names and
    # source locations. Source maps provide neither function names nor inlined
    # info. So we only check for the source location of the outermost function.
    check_dwarf_loc_info(out_to_js_call_addr, out_to_js_call_func,
                         out_to_js_call_loc)
    check_source_map_loc_info(out_to_js_call_addr, out_to_js_call_loc[0])
    check_dwarf_loc_info(unreachable_addr, unreachable_func, unreachable_loc)
    check_source_map_loc_info(unreachable_addr, unreachable_loc[0])

    # 2. Test source map only
    # The addresses, function names, and source locations are the same across
    # the builds because they are relative offsets from the code section, so we
    # don't need to recompute them
    self.run_process([EMCC, test_file('core/test_dwarf.c'),
                      '-gsource-map', '-O1', '-o', 'test_dwarf.js'])
    check_source_map_loc_info(out_to_js_call_addr, out_to_js_call_loc[0])
    check_source_map_loc_info(unreachable_addr, unreachable_loc[0])

    # 3. Test DWARF only
    self.run_process([EMCC, test_file('core/test_dwarf.c'),
                      '-g', '-O1', '-o', 'test_dwarf.js'])
    check_dwarf_loc_info(out_to_js_call_addr, out_to_js_call_func,
                         out_to_js_call_loc)
    check_dwarf_loc_info(unreachable_addr, unreachable_func, unreachable_loc)

  def test_emsymbolizer_functions(self):
    'Test emsymbolizer use cases that only provide function-granularity info'
    def check_func_info(filename, address, func):
      out = self.run_process(
        [emsymbolizer, filename, address], stdout=PIPE).stdout
      self.assertIn(func, out)

    # 1. Test name section only
    self.run_process([EMCC, test_file('core/test_dwarf.c'),
                      '--profiling-funcs', '-O1', '-o', 'test_dwarf.js'])
    with webassembly.Module('test_dwarf.wasm') as wasm:
      self.assertTrue(wasm.has_name_section())
      self.assertIsNone(wasm.get_custom_section('.debug_info'))
    # Address of out_to_js(0) within foo(), uninlined
    out_to_js_call_addr = self.get_instr_addr('call\t0', 'test_dwarf.wasm')
    # Address of __builtin_trap() within bar(), inlined into main()
    unreachable_addr = self.get_instr_addr('unreachable', 'test_dwarf.wasm')
    check_func_info('test_dwarf.wasm', out_to_js_call_addr, 'foo')
    # The name section will not show bar, as it's inlined into main
    check_func_info('test_dwarf.wasm', unreachable_addr, '__original_main')

  def test_separate_dwarf(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-g'])
    self.assertExists('a.out.wasm')
    self.assertNotExists('a.out.wasm.debug.wasm')
    self.run_process([EMCC, test_file('hello_world.c'), '-gseparate-dwarf'])
    self.assertExists('a.out.wasm')
    self.assertExists('a.out.wasm.debug.wasm')
    self.assertLess(os.path.getsize('a.out.wasm'), os.path.getsize('a.out.wasm.debug.wasm'))
    # the special section should also exist, that refers to the side debug file
    wasm = read_binary('a.out.wasm')
    self.assertIn(b'external_debug_info', wasm)
    self.assertIn(b'a.out.wasm.debug.wasm', wasm)

    # building to a subdirectory should still leave a relative path, which
    # assumes the debug file is alongside the main one
    os.mkdir('subdir')
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-gseparate-dwarf',
                      '-o', Path('subdir/output.js')])
    wasm = read_binary('subdir/output.wasm')
    self.assertIn(b'output.wasm.debug.wasm', wasm)
    # check both unix-style slashes and the system's slashes, so that we don't
    # assume the encoding of the section in this test
    self.assertNotIn(b'subdir/output.wasm.debug.wasm', wasm)
    self.assertNotIn(bytes(os.path.join('subdir', 'output.wasm.debug.wasm'), 'ascii'), wasm)

    # Check that the dwarf file has only dwarf, name, and non-code sections
    with webassembly.Module('subdir/output.wasm.debug.wasm') as debug_wasm:
      if not debug_wasm.has_name_section():
        self.fail('name section not found in separate dwarf file')
      for sec in debug_wasm.sections():
        # TODO(https://github.com/emscripten-core/emscripten/issues/13084):
        # Re-enable this code once the debugger extension can handle wasm files
        # with name sections but no code sections.
        # if sec.type == webassembly.SecType.CODE:
        #   self.fail(f'section of type "{sec.type}" found in separate dwarf file')
        if sec.name and sec.name != 'name' and not sec.name.startswith('.debug'):
          self.fail(f'non-debug section "{sec.name}" found in separate dwarf file')

    # Check that dwarfdump can dump the debug info
    dwdump = self.run_process(
        [LLVM_DWARFDUMP, 'subdir/output.wasm.debug.wasm', '-name', 'main'],
        stdout=PIPE).stdout
    # Basic check that the debug info is more than a skeleton. If so it will
    # have a subprogram descriptor for main
    self.assertIn('DW_TAG_subprogram', dwdump)
    self.assertIn('DW_AT_name\t("main")', dwdump)

  def test_split_dwarf_dwp(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-g', '-gsplit-dwarf'])
    self.assertExists('a.out.wasm')
    self.assertExists('hello_world.dwo')

    # The wasm will have full debug info for libc (ignore that), but only a
    # skeleton for hello_world.c (no subprogram for main)
    dwdump = self.run_process([LLVM_DWARFDUMP, 'a.out.wasm'], stdout=PIPE).stdout
    self.assertIn('DW_AT_GNU_dwo_name\t("hello_world.dwo")', dwdump)
    self.assertNotIn('DW_AT_name\t("main")', dwdump)

    # The dwo will have a subprogram for main in a section with a .dwo suffix
    dwdump = self.run_process([LLVM_DWARFDUMP, 'hello_world.dwo'],
                              stdout=PIPE).stdout
    self.assertIn('.debug_info.dwo contents:', dwdump)
    self.assertIn('DW_AT_GNU_dwo_name\t("hello_world.dwo")', dwdump)
    self.assertIn('DW_AT_name\t("main")', dwdump)

    # Check that dwp runs, and results in usable output as well
    self.run_process([LLVM_DWP, '-e', 'a.out.wasm', '-o', 'a.out.wasm.dwp'])
    self.assertExists('a.out.wasm.dwp')
    self.run_process([LLVM_DWARFDUMP, 'a.out.wasm.dwp'], stdout=PIPE).stdout
    self.assertIn('.debug_info.dwo contents:', dwdump)
    self.assertIn('DW_AT_GNU_dwo_name\t("hello_world.dwo")', dwdump)
    self.assertIn('DW_AT_name\t("main")', dwdump)

  def test_separate_dwarf_with_filename(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-gseparate-dwarf=with_dwarf.wasm'])
    self.assertNotExists('a.out.wasm.debug.wasm')
    self.assertExists('with_dwarf.wasm')
    # the correct notation is to have exactly one '=' and in the right place
    for invalid in ('-gseparate-dwarf=x=', '-gseparate-dwarfy=', '-gseparate-dwarf-hmm'):
      stderr = self.expect_fail([EMCC, test_file('hello_world.c'), invalid])
      self.assertContained('invalid -gseparate-dwarf=FILENAME notation', stderr)

    # building to a subdirectory, but with the debug file in another place,
    # should leave a relative path to the debug wasm
    os.mkdir('subdir')
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-o', Path('subdir/output.js'),
                      '-gseparate-dwarf=with_dwarf2.wasm'])
    self.assertExists('with_dwarf2.wasm')
    wasm = read_binary('subdir/output.wasm')
    self.assertIn(b'../with_dwarf2.wasm', wasm)

  def test_separate_dwarf_with_filename_and_path(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-gseparate-dwarf=with_dwarf.wasm'])
    self.assertIn(b'with_dwarf.wasm', read_binary('a.out.wasm'))
    self.run_process([EMCC, test_file('hello_world.c'), '-gseparate-dwarf=with_dwarf.wasm',
                      '-sSEPARATE_DWARF_URL=http://somewhere.com/hosted.wasm'])
    self.assertIn(b'somewhere.com/hosted.wasm', read_binary('a.out.wasm'))

  @crossplatform
  def test_dwarf_system_lib(self):
    if config.FROZEN_CACHE:
      self.skipTest("test doesn't work with frozen cache")
    self.run_process([EMBUILDER, 'build', 'libemmalloc', '--force'])
    libc = os.path.join(config.CACHE, 'sysroot', 'lib', 'wasm32-emscripten', 'libemmalloc.a')
    self.assertExists(libc)

    dwdump = self.run_process(
        [LLVM_DWARFDUMP, libc, '-debug-info', '-debug-line', '--recurse-depth=0'],
        stdout=PIPE).stdout
    # Check that the embedded location of the source file is correct.
    self.assertIn('DW_AT_name\t("system/lib/emmalloc.c")', dwdump)
    self.assertIn('DW_AT_comp_dir\t("/emsdk/emscripten")', dwdump)

  @parameterized({
    'O0': (['-O0'],),
    'O1': (['-O1'],),
    'O2': (['-O2'],),
  })
  def test_wasm_producers_section(self, args):
    self.run_process([EMCC, test_file('hello_world.c')] + args)
    # if there is no producers section expected by default, verify that, and
    # see that the flag works to add it.
    self.verify_custom_sec_existence('a.out.wasm', 'producers', False)
    size = os.path.getsize('a.out.wasm')
    self.run_process([EMCC, test_file('hello_world.c'), '-sEMIT_PRODUCERS_SECTION'] + args)
    self.verify_custom_sec_existence('a.out.wasm', 'producers', True)
    size_with_section = os.path.getsize('a.out.wasm')
    self.assertLess(size, size_with_section)

  @parameterized({
    '':       ([],),
    # in some modes we skip wasm-emscripten-finalize, which normally strips the
    # features section for us, so add testing for those
    'bigint': (['-sWASM_BIGINT'],),
    'wasm64': (['-sMEMORY64'],),
  })
  def test_wasm_features_section(self, args):
    # The features section should never be in our output, when we optimize.
    self.run_process([EMCC, test_file('hello_world.c'), '-O2'] + args)
    self.verify_custom_sec_existence('a.out.wasm', 'target_features', False)

  def test_js_preprocess(self):
    # Use stderr rather than stdout here because stdout is redirected to the output JS file itself.
    create_file('lib.js', '''
#if MAIN_MODULE == 1
console.error('JSLIB: MAIN_MODULE=1');
#elif MAIN_MODULE == 2
console.error('JSLIB: MAIN_MODULE=2');
#elif EXIT_RUNTIME
console.error('JSLIB: EXIT_RUNTIME');
#else
console.error('JSLIB: none of the above');
#endif
''')

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained('JSLIB: none of the above', err)
    self.assertNotContained('JSLIB: MAIN_MODULE', err)
    self.assertNotContained('JSLIB: EXIT_RUNTIME', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js', '-sMAIN_MODULE'], stderr=PIPE).stderr
    self.assertContained('JSLIB: MAIN_MODULE=1', err)
    self.assertNotContained('JSLIB: EXIT_RUNTIME', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js', '-sMAIN_MODULE=2'], stderr=PIPE).stderr
    self.assertContained('JSLIB: MAIN_MODULE=2', err)
    self.assertNotContained('JSLIB: EXIT_RUNTIME', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js', '-sEXIT_RUNTIME'], stderr=PIPE).stderr
    self.assertContained('JSLIB: EXIT_RUNTIME', err)
    self.assertNotContained('JSLIB: MAIN_MODULE', err)

  @crossplatform
  def test_html_preprocess(self):
    src_file = test_file('module/test_stdin.c')
    output_file = 'test_stdin.html'
    shell_file = test_file('module/test_html_preprocess.html')

    self.run_process([EMCC, '-o', output_file, src_file, '--shell-file', shell_file, '-sASSERTIONS=0'], stdout=PIPE, stderr=PIPE)
    output = read_file(output_file)
    self.assertContained('''<style>
/* Disable preprocessing inside style block as syntax is ambiguous with CSS */
#include {background-color: black;}
#if { background-color: red;}
#else {background-color: blue;}
#endif {background-color: green;}
#xxx {background-color: purple;}
</style>
T1:(else) ASSERTIONS != 1
T2:ASSERTIONS != 1
T3:ASSERTIONS < 2
T4:(else) ASSERTIONS <= 1
T5:(else) ASSERTIONS
T6:!ASSERTIONS''', output)

    self.run_process([EMCC, '-o', output_file, src_file, '--shell-file', shell_file, '-sASSERTIONS'], stdout=PIPE, stderr=PIPE)
    output = read_file(output_file)
    self.assertContained("""<style>
/* Disable preprocessing inside style block as syntax is ambiguous with CSS */
#include {background-color: black;}
#if { background-color: red;}
#else {background-color: blue;}
#endif {background-color: green;}
#xxx {background-color: purple;}
</style>
T1:ASSERTIONS == 1
T2:(else) ASSERTIONS == 1
T3:ASSERTIONS < 2
T4:(else) ASSERTIONS <= 1
T5:ASSERTIONS
T6:(else) !ASSERTIONS""", output)

    self.run_process([EMCC, '-o', output_file, src_file, '--shell-file', shell_file, '-sASSERTIONS=2'], stdout=PIPE, stderr=PIPE)
    output = read_file(output_file)
    self.assertContained("""<style>
/* Disable preprocessing inside style block as syntax is ambiguous with CSS */
#include {background-color: black;}
#if { background-color: red;}
#else {background-color: blue;}
#endif {background-color: green;}
#xxx {background-color: purple;}
</style>
T1:(else) ASSERTIONS != 1
T2:ASSERTIONS != 1
T3:(else) ASSERTIONS >= 2
T4:ASSERTIONS > 1
T5:ASSERTIONS
T6:(else) !ASSERTIONS""", output)

  # Tests that Emscripten-compiled applications can be run from a relative path with node command line that is different than the current working directory.
  @requires_node
  def test_node_js_run_from_different_directory(self):
    ensure_dir('subdir')
    self.run_process([EMCC, test_file('hello_world.c'), '-o', Path('subdir/a.js'), '-O3'])
    ret = self.run_js('subdir/a.js')
    self.assertContained('hello, world!', ret)

  # Tests that a pthreads + modularize build can be run in node js
  @node_pthreads
  @parameterized({
    '': (False,),
    'es6': (True,),
  })
  def test_node_js_pthread_module(self, es6):
    # create module loader script
    if es6:
      ext = '.mjs'
      create_file('moduleLoader.mjs', '''
        import test_module from "./subdir/module.mjs";
        test_module().then((test_module_instance) => {
          console.log("done");
        });
        ''')
    else:
      ext = '.js'
      create_file('moduleLoader.js', '''
        const test_module = require("./subdir/module.js");
        test_module().then((test_module_instance) => {
          console.log("done");
        });
        ''')
    ensure_dir('subdir')

    # build hello_world.c
    self.run_process([EMCC, test_file('hello_world.c'), '-o', 'subdir/module' + ext, '-pthread', '-sPTHREAD_POOL_SIZE=2', '-sMODULARIZE', '-sEXPORT_NAME=test_module'] + self.get_emcc_args())

    # run the module
    ret = self.run_js('moduleLoader' + ext)
    self.assertContained('hello, world!\ndone\n', ret)

    create_file('workerLoader.js', f'''
      const {{ Worker, isMainThread }} = require('worker_threads');
      new Worker('./moduleLoader{ext}');
      ''')

    # run the same module, but inside of a worker
    ret = self.run_js('workerLoader.js')
    self.assertContained('hello, world!\ndone\n', ret)

  @no_windows('node system() does not seem to work, see https://github.com/emscripten-core/emscripten/pull/10547')
  @requires_node
  def test_system_node_js(self):
    self.do_runf('test_system.c', 'Hello from echo', emcc_args=['-DENV_NODE'])

  def test_node_eval(self):
    self.run_process([EMCC, '-sENVIRONMENT=node', test_file('hello_world.c'), '-o', 'a.js', '-O3'])
    js = read_file('a.js')
    ret = self.run_process(config.NODE_JS_TEST + ['-e', js], stdout=PIPE).stdout
    self.assertContained('hello, world!', ret)

  def test_is_bitcode(self):
    fname = 'tmp.o'

    with open(fname, 'wb') as f:
      f.write(b'foo')
    self.assertFalse(is_bitcode(fname))

    with open(fname, 'wb') as f:
      f.write(b'\xDE\xC0\x17\x0B')
      f.write(16 * b'\x00')
      f.write(b'BC')
    self.assertTrue(is_bitcode(fname))

    with open(fname, 'wb') as f:
      f.write(b'BC')
    self.assertTrue(is_bitcode(fname))

  def test_is_ar(self):
    fname = 'tmp.a'

    with open(fname, 'wb') as f:
      f.write(b'foo')
    self.assertFalse(building.is_ar(fname))

    with open(fname, 'wb') as f:
      f.write(b'!<arch>\n')
    self.assertTrue(building.is_ar(fname))

  def test_dash_s_list_parsing(self):
    create_file('src.c', r'''
        #include <stdio.h>
        void a() { printf("a\n"); }
        void b() { printf("b\n"); }
        void c() { printf("c\n"); }
        void d() { printf("d\n"); }
      ''')
    create_file('response.json', '''\
[
"_a",
"_b",
"_c",
"_d"
]
''')
    create_file('response.txt', '''\
_a
_b
_c
_d
''')

    for export_arg, expected in [
      # No quotes needed
      ('EXPORTED_FUNCTIONS=[_a,_b,_c,_d]', ''),
      # No quotes with spaces
      ('EXPORTED_FUNCTIONS=[_a, _b, _c, _d]', ''),
      # No brackets needed either
      ('EXPORTED_FUNCTIONS=_a,_b,_c,_d', ''),
      # No brackets with spaced
      ('EXPORTED_FUNCTIONS=_a, _b, _c, _d', ''),
      # extra space at end - should be ignored
      ("EXPORTED_FUNCTIONS=['_a', '_b', '_c', '_d' ]", ''),
      # extra newline in response file - should be ignored
      ("EXPORTED_FUNCTIONS=@response.json", ''),
      # Simple one-per-line response file format
      ("EXPORTED_FUNCTIONS=@response.txt", ''),
      # stray slash
      ("EXPORTED_FUNCTIONS=['_a', '_b', \\'_c', '_d']", '''undefined exported symbol: "\\\\'_c'"'''),
      # stray slash
      ("EXPORTED_FUNCTIONS=['_a', '_b',\\ '_c', '_d']", '''undefined exported symbol: "\\\\ '_c'"'''),
      # stray slash
      ('EXPORTED_FUNCTIONS=["_a", "_b", \\"_c", "_d"]', 'undefined exported symbol: "\\\\"_c""'),
      # stray slash
      ('EXPORTED_FUNCTIONS=["_a", "_b",\\ "_c", "_d"]', 'undefined exported symbol: "\\\\ "_c"'),
      # missing comma
      ('EXPORTED_FUNCTIONS=["_a", "_b" "_c", "_d"]', 'wasm-ld: error: symbol exported via --export not found: b" "_c'),
    ]:
      print(export_arg)
      proc = self.run_process([EMCC, 'src.c', '-s', export_arg], stdout=PIPE, stderr=PIPE, check=not expected)
      print(proc.stderr)
      if not expected:
        js = read_file('a.out.js')
        for sym in ('_a', '_b', '_c', '_d'):
          self.assertContained(f'var {sym} = ', js)
      else:
        self.assertNotEqual(proc.returncode, 0)
        self.assertContained(expected, proc.stderr)

  def test_asyncify_escaping(self):
    proc = self.run_process([EMCC, test_file('hello_world.c'), '-sASYNCIFY', '-s', "ASYNCIFY_ONLY=[DOS_ReadFile(unsigned short, unsigned char*, unsigned short*, bool)]"], stdout=PIPE, stderr=PIPE)
    self.assertContained('emcc: ASYNCIFY list contains an item without balanced parentheses', proc.stderr)
    self.assertContained('   DOS_ReadFile(unsigned short', proc.stderr)
    self.assertContained('Try using a response file', proc.stderr)

  def test_asyncify_response_file(self):
    create_file('a.txt', r'''[
  "DOS_ReadFile(unsigned short, unsigned char*, unsigned short*, bool)"
]
''')

    create_file('b.txt', 'DOS_ReadFile(unsigned short, unsigned char*, unsigned short*, bool)')
    for file in ('a.txt', 'b.txt'):
      proc = self.run_process([EMCC, test_file('hello_world.c'), '-sASYNCIFY', f'-sASYNCIFY_ONLY=@{file}'], stdout=PIPE, stderr=PIPE)
      # we should parse the response file properly, and then issue a proper warning for the missing function
      self.assertContained(
          'Asyncify onlylist contained a non-matching pattern: DOS_ReadFile(unsigned short, unsigned char*, unsigned short*, bool)',
          proc.stderr)

  def test_asyncify_advise(self):
    src = test_file('other/asyncify_advise.c')

    self.set_setting('ASYNCIFY')
    self.set_setting('ASYNCIFY_ADVISE')
    self.set_setting('ASYNCIFY_IMPORTS', ['async_func'])

    out = self.run_process([EMCC, src, '-o', 'asyncify_advise.js'] + self.get_emcc_args(), stdout=PIPE).stdout
    self.assertContained('[asyncify] main can', out)
    self.assertContained('[asyncify] a can', out)
    self.assertContained('[asyncify] c can', out)
    self.assertContained('[asyncify] e can', out)
    self.assertContained('[asyncify] g can', out)
    self.assertContained('[asyncify] i can', out)

    self.set_setting('ASYNCIFY_REMOVE', ['e'])
    out = self.run_process([EMCC, src, '-o', 'asyncify_advise.js'] + self.get_emcc_args(), stdout=PIPE).stdout
    self.assertContained('[asyncify] main can', out)
    self.assertNotContained('[asyncify] a can', out)
    self.assertNotContained('[asyncify] c can', out)
    self.assertNotContained('[asyncify] e can', out)
    self.assertContained('[asyncify] g can', out)
    self.assertContained('[asyncify] i can', out)

  def test_asyncify_stack_overflow(self):
    self.emcc_args = ['-sASYNCIFY', '-sASYNCIFY_STACK_SIZE=4']

    # The unreachable error on small stack sizes is not super-helpful. Try at
    # least to hint at increasing the stack size.

    def test(args, expected):
      self.do_runf('other/test_asyncify_stack_overflow.cpp',
                   emcc_args=args,
                   assert_returncode=common.NON_ZERO,
                   expected_output=[expected])

    test(['-sASSERTIONS=0'],
         'Aborted(RuntimeError: unreachable). Build with -sASSERTIONS for more info.')

    test(['-sASSERTIONS=1'],
         'Aborted(RuntimeError: unreachable). "unreachable" may be due to ASYNCIFY_STACK_SIZE not being large enough (try increasing it)')

  # Sockets and networking

  def test_inet(self):
    self.do_runf('third_party/sha1.c', 'SHA1=15dd99a1991e0b3826fede3deffc1feba42278e6')
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>

      int main() {
        printf("*%x,%x,%x,%x,%x,%x*\n", htonl(0xa1b2c3d4), htonl(0xfe3572e0), htonl(0x07abcdf0), htons(0xabcd), ntohl(0x43211234), ntohs(0xbeaf));
        in_addr_t i = inet_addr("190.180.10.78");
        printf("%x\n", i);
        return 0;
      }
    '''
    self.do_run(src, '*d4c3b2a1,e07235fe,f0cdab07,cdab,34122143,afbe*\n4e0ab4be\n')

  def test_inet2(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>

      int main() {
        struct in_addr x, x2;
        int *y = (int*)&x;
        *y = 0x12345678;
        printf("%s\n", inet_ntoa(x));
        int r = inet_aton(inet_ntoa(x), &x2);
        printf("%s\n", inet_ntoa(x2));
        return 0;
      }
    '''
    self.do_run(src, '120.86.52.18\n120.86.52.18\n')

  def test_inet3(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>
      #include <sys/socket.h>
      int main() {
        char dst[64];
        struct in_addr x, x2;
        int *y = (int*)&x;
        *y = 0x12345678;
        printf("%s\n", inet_ntop(AF_INET,&x,dst,sizeof dst));
        int r = inet_aton(inet_ntoa(x), &x2);
        printf("%s\n", inet_ntop(AF_INET,&x2,dst,sizeof dst));
        return 0;
      }
    '''
    self.do_run(src, '120.86.52.18\n120.86.52.18\n')

  def test_inet4(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>
      #include <sys/socket.h>

      void test(const char *test_addr, bool first=true){
          char str[40];
          struct in6_addr addr;
          unsigned char *p = (unsigned char*)&addr;
          int ret;
          ret = inet_pton(AF_INET6,test_addr,&addr);
          if(ret == -1) return;
          if(ret == 0) return;
          if(inet_ntop(AF_INET6,&addr,str,sizeof(str)) == NULL ) return;
          printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x - %s\n",
               p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],str);
          if (first) test(str, false); // check again, on our output
      }
      int main(){
          test("::");
          test("::1");
          test("::1.2.3.4");
          test("::17.18.19.20");
          test("::ffff:1.2.3.4");
          test("1::ffff");
          test("::255.255.255.255");
          test("0:ff00:1::");
          test("0:ff::");
          test("abcd::");
          test("ffff::a");
          test("ffff::a:b");
          test("ffff::a:b:c");
          test("ffff::a:b:c:d");
          test("ffff::a:b:c:d:e");
          test("::1:2:0:0:0");
          test("0:0:1:2:3::");
          test("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
          test("1::255.255.255.255");

          //below should fail and not produce results..
          test("1.2.3.4");
          test("");
          test("-");

          printf("ok.\n");
      }
    '''
    self.do_run(src, r'''0000:0000:0000:0000:0000:0000:0000:0000 - ::
0000:0000:0000:0000:0000:0000:0000:0000 - ::
0000:0000:0000:0000:0000:0000:0000:0001 - ::1
0000:0000:0000:0000:0000:0000:0000:0001 - ::1
0000:0000:0000:0000:0000:0000:0102:0304 - ::102:304
0000:0000:0000:0000:0000:0000:0102:0304 - ::102:304
0000:0000:0000:0000:0000:0000:1112:1314 - ::1112:1314
0000:0000:0000:0000:0000:0000:1112:1314 - ::1112:1314
0000:0000:0000:0000:0000:ffff:0102:0304 - ::ffff:1.2.3.4
0000:0000:0000:0000:0000:ffff:0102:0304 - ::ffff:1.2.3.4
0001:0000:0000:0000:0000:0000:0000:ffff - 1::ffff
0001:0000:0000:0000:0000:0000:0000:ffff - 1::ffff
0000:0000:0000:0000:0000:0000:ffff:ffff - ::ffff:ffff
0000:0000:0000:0000:0000:0000:ffff:ffff - ::ffff:ffff
0000:ff00:0001:0000:0000:0000:0000:0000 - 0:ff00:1::
0000:ff00:0001:0000:0000:0000:0000:0000 - 0:ff00:1::
0000:00ff:0000:0000:0000:0000:0000:0000 - 0:ff::
0000:00ff:0000:0000:0000:0000:0000:0000 - 0:ff::
abcd:0000:0000:0000:0000:0000:0000:0000 - abcd::
abcd:0000:0000:0000:0000:0000:0000:0000 - abcd::
ffff:0000:0000:0000:0000:0000:0000:000a - ffff::a
ffff:0000:0000:0000:0000:0000:0000:000a - ffff::a
ffff:0000:0000:0000:0000:0000:000a:000b - ffff::a:b
ffff:0000:0000:0000:0000:0000:000a:000b - ffff::a:b
ffff:0000:0000:0000:0000:000a:000b:000c - ffff::a:b:c
ffff:0000:0000:0000:0000:000a:000b:000c - ffff::a:b:c
ffff:0000:0000:0000:000a:000b:000c:000d - ffff::a:b:c:d
ffff:0000:0000:0000:000a:000b:000c:000d - ffff::a:b:c:d
ffff:0000:0000:000a:000b:000c:000d:000e - ffff::a:b:c:d:e
ffff:0000:0000:000a:000b:000c:000d:000e - ffff::a:b:c:d:e
0000:0000:0000:0001:0002:0000:0000:0000 - ::1:2:0:0:0
0000:0000:0000:0001:0002:0000:0000:0000 - ::1:2:0:0:0
0000:0000:0001:0002:0003:0000:0000:0000 - 0:0:1:2:3::
0000:0000:0001:0002:0003:0000:0000:0000 - 0:0:1:2:3::
ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff - ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff - ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
0001:0000:0000:0000:0000:0000:ffff:ffff - 1::ffff:ffff
0001:0000:0000:0000:0000:0000:ffff:ffff - 1::ffff:ffff
ok.
''')

  def test_getsockname_unconnected_socket(self):
    self.do_run(r'''
      #include <sys/socket.h>
      #include <stdio.h>
      #include <assert.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <string.h>
      int main() {
        int fd;
        int z;
        fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in adr_inet;
        socklen_t len_inet = sizeof adr_inet;
        z = getsockname(fd, (struct sockaddr *)&adr_inet, &len_inet);
        if (z != 0) {
          perror("getsockname error");
          return 1;
        }
        char buffer[1000];
        sprintf(buffer, "%s:%u", inet_ntoa(adr_inet.sin_addr), (unsigned)ntohs(adr_inet.sin_port));
        const char *correct = "0.0.0.0:0";
        printf("got (expected) socket: %s (%s), size %lu (%lu)\n", buffer, correct, strlen(buffer), strlen(correct));
        assert(strlen(buffer) == strlen(correct));
        assert(strcmp(buffer, correct) == 0);
        puts("success.");
      }
    ''', 'success.')

  def test_getpeername_unconnected_socket(self):
    self.do_run(r'''
      #include <sys/socket.h>
      #include <stdio.h>
      #include <assert.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <string.h>
      int main() {
        int fd;
        int z;
        fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in adr_inet;
        socklen_t len_inet = sizeof adr_inet;
        z = getpeername(fd, (struct sockaddr *)&adr_inet, &len_inet);
        if (z != 0) {
          perror("getpeername error");
          return 1;
        }
        puts("unexpected success.");
      }
    ''', 'getpeername error: Socket not connected', assert_returncode=NON_ZERO)

  def test_getsockname_addrlen(self):
    self.do_runf('sockets/test_getsockname_addrlen.c', 'success')

  def test_sin_zero(self):
    self.do_runf('sockets/test_sin_zero.c', 'success')

  def test_getaddrinfo(self):
    self.do_runf('sockets/test_getaddrinfo.c', 'success')

  def test_getnameinfo(self):
    self.do_runf('sockets/test_getnameinfo.c', 'success')

  def test_gethostbyname(self):
    self.do_run_in_out_file_test('sockets/test_gethostbyname.c')

  def test_getprotobyname(self):
    self.do_runf('sockets/test_getprotobyname.c', 'success')

  def test_create_socket(self):
    self.do_runf('sockets/test_create_socket.c', 'success')

  def test_socketpair(self):
    self.do_run(r'''
      #include <sys/socket.h>
      #include <stdio.h>
      int main() {
        int fd[2];
        int err;
        err = socketpair(AF_INET, SOCK_STREAM, 0, fd);
        if (err != 0) {
          perror("socketpair error");
          return 1;
        }
        puts("unexpected success.");
      }
    ''', 'socketpair error: Function not implemented', assert_returncode=NON_ZERO)

  def test_link(self):
    self.do_run(r'''
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>

int main () {
    void* thing = gethostbyname("bing.com");
    ssize_t rval = recv (0, thing, 0, 0);
    rval = send (0, thing, 0, 0);
    return 0;
}''', '', force_c=True)

  def test_linking_recv(self):
    self.do_run(r'''
      #include <sys/types.h>
      #include <sys/socket.h>
      int main(void) {
        recv(0, 0, 0, 0);
        return 0;
      }
    ''', '', force_c=True)

  def test_linking_send(self):
    self.do_run(r'''
      #include <sys/types.h>
      #include <sys/socket.h>
      int main(void) {
        send(0, 0, 0, 0);
        return 0;
      }
    ''', '', force_c=True)

  # This test verifies that function names embedded into the build with --js-library (JS functions exported to wasm)
  # are minified when -O3 is used
  def test_js_function_names_are_minified(self):
    def check_size(f, expected_size):
      if not os.path.isfile(f):
        return # Nonexistent file passes in this check
      obtained_size = os.path.getsize(f)
      print('size of generated ' + f + ': ' + str(obtained_size))
      delete_file(f)
      self.assertLess(obtained_size, expected_size)

    self.run_process([PYTHON, test_file('gen_many_js_functions.py'), 'library_long.js', 'main_long.c'])
    for wasm in ([], ['-sWASM=0']):
      # Currently we rely on Closure for full minification of every appearance of JS function names.
      # TODO: Add minification also for non-Closure users and add [] to this list to test minification without Closure.
      for closure in [['--closure=1']]:
        args = [EMCC, '-O3', '--js-library', 'library_long.js', 'main_long.c', '-o', 'a.html'] + wasm + closure
        print(' '.join(args))
        self.run_process(args)

        ret = self.run_js('a.js')
        self.assertTextDataIdentical('Sum of numbers from 1 to 1000: 500500 (expected 500500)', ret.strip())

        check_size('a.js', 150000)
        check_size('a.wasm', 80000)

  # Checks that C++ exceptions managing invoke_*() wrappers will not be generated if exceptions are disabled
  def test_no_invoke_functions_are_generated_if_exception_catching_is_disabled(self):
    self.skipTest('Skipping other.test_no_invoke_functions_are_generated_if_exception_catching_is_disabled: Enable after new version of fastcomp has been tagged')
    for args in ([], ['-sWASM=0']):
      self.run_process([EMXX, test_file('hello_world.cpp'), '-sDISABLE_EXCEPTION_CATCHING', '-o', 'a.html'] + args)
      output = read_file('a.js')
      self.assertContained('_main', output) # Smoke test that we actually compiled
      self.assertNotContained('invoke_', output)

  # Verifies that only the minimal needed set of invoke_*() functions will be generated when C++ exceptions are enabled
  def test_no_excessive_invoke_functions_are_generated_when_exceptions_are_enabled(self):
    self.skipTest('Skipping other.test_no_excessive_invoke_functions_are_generated_when_exceptions_are_enabled: Enable after new version of fastcomp has been tagged')
    for args in ([], ['-sWASM=0']):
      self.run_process([EMXX, test_file('invoke_i.cpp'), '-sDISABLE_EXCEPTION_CATCHING=0', '-o', 'a.html'] + args)
      output = read_file('a.js')
      self.assertContained('invoke_i', output)
      self.assertNotContained('invoke_ii', output)
      self.assertNotContained('invoke_v', output)

  @parameterized({
    'O0': (False, ['-O0']), # noqa
    'O0_emit': (True, ['-O0', '-sEMIT_EMSCRIPTEN_LICENSE']), # noqa
    'O2': (False, ['-O2']), # noqa
    'O2_emit': (True, ['-O2', '-sEMIT_EMSCRIPTEN_LICENSE']), # noqa
    'O2_js_emit': (True, ['-O2', '-sEMIT_EMSCRIPTEN_LICENSE', '-sWASM=0']), # noqa
    'O2_closure': (False, ['-O2', '--closure=1']), # noqa
    'O2_closure_emit': (True, ['-O2', '-sEMIT_EMSCRIPTEN_LICENSE', '--closure=1']), # noqa
    'O2_closure_js_emit': (True, ['-O2', '-sEMIT_EMSCRIPTEN_LICENSE', '--closure=1', '-sWASM=0']), # noqa
  })
  def test_emscripten_license(self, expect_license, args):
    # fastcomp does not support the new license flag
    self.run_process([EMCC, test_file('hello_world.c')] + args)
    js = read_file('a.out.js')
    licenses_found = len(re.findall('Copyright [0-9]* The Emscripten Authors', js))
    if expect_license:
      self.assertNotEqual(licenses_found, 0, 'Unable to find license block in output file!')
      self.assertEqual(licenses_found, 1, 'Found too many license blocks in the output file!')
    else:
      self.assertEqual(licenses_found, 0, 'Found a license block in the output file, but it should not have been there!')

  # This test verifies that the generated exports from wasm module only reference the
  # unminified exported name exactly once.  (need to contain the export name once for unminified
  # access from calling code, and should not have the unminified name exist more than once, that
  # would be wasteful for size)
  @parameterized({
    '': ([],),
    'closure': (['--closure=1'],),
  })
  @parameterized({
    'O2': (['-O2'],),
    'O3': (['-O3'],),
    'Os': (['-Os'],),
  })
  @parameterized({
    'sync': (['-sWASM_ASYNC_COMPILATION=0'],),
    'wasm2js': (['-sWASM=0', '-Wno-closure'],),
  })
  def test_function_exports_are_small(self, args, opt, closure):
    extra_args = args + opt + closure
    args = [EMCC, test_file('long_function_name_in_export.c'), '-o', 'a.html', '-sENVIRONMENT=web', '-sDECLARE_ASM_MODULE_EXPORTS=0', '-Werror'] + extra_args
    self.run_process(args)

    output = read_file('a.js')
    delete_file('a.js')
    self.assertNotContained('_thisIsAFunctionExportedFromAsmJsOrWasmWithVeryLongFunction', output)

    # TODO: Add stricter testing when Wasm side is also optimized: (currently Wasm does still need
    # to reference exports multiple times)
    if '-sWASM=0' in args:
      num_times_export_is_referenced = output.count('thisIsAFunctionExportedFromAsmJsOrWasmWithVeryLongFunction')
      self.assertEqual(num_times_export_is_referenced, 1)

  @parameterized({
    'hello_world_wasm': ('hello_world', False, True),
    'hello_world_wasm2js': ('hello_world', True, True),
    'random_printf_wasm': ('random_printf', False),
    'random_printf_wasm2js': ('random_printf', True),
    'hello_webgl_wasm': ('hello_webgl', False),
    'hello_webgl_wasm2js': ('hello_webgl', True),
    'hello_webgl2_wasm': ('hello_webgl2', False),
    'hello_webgl2_wasm2js': ('hello_webgl2', True),
    'math': ('math', False),
    'hello_wasm_worker': ('hello_wasm_worker', False, True),
    'hello_embind_val': ('embind_val', False),
    'hello_embind': ('embind_hello', False),
  })
  @crossplatform
  def test_minimal_runtime_code_size(self, test_name, js, compare_js_output=False):
    smallest_code_size_args = ['-sMINIMAL_RUNTIME=2',
                               '-sENVIRONMENT=web',
                               '-sTEXTDECODER=2',
                               '-sABORTING_MALLOC=0',
                               '-sALLOW_MEMORY_GROWTH=0',
                               '-sDECLARE_ASM_MODULE_EXPORTS',
                               '-sMALLOC=emmalloc',
                               '-sGL_EMULATE_GLES_VERSION_STRING_FORMAT=0',
                               '-sGL_EXTENSIONS_IN_PREFIXED_FORMAT=0',
                               '-sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0',
                               '-sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0',
                               '-sGL_TRACK_ERRORS=0',
                               '-sGL_POOL_TEMP_BUFFERS=0',
                               '-sGL_WORKAROUND_SAFARI_GETCONTEXT_BUG=0',
                               '-sGL_ENABLE_GET_PROC_ADDRESS=0',
                               '-sNO_FILESYSTEM',
                               '-sSTRICT',
                               '--output_eol', 'linux',
                               '-Oz',
                               '--closure=1',
                               '-DNDEBUG',
                               '-ffast-math']

    wasm2js = ['-sWASM=0']

    math_sources = [test_file('code_size/math.c')]
    hello_world_sources = [test_file('small_hello_world.c'),
                           '-sMALLOC=none']
    random_printf_sources = [test_file('hello_random_printf.c'),
                             '-sMALLOC=none',
                             '-sSINGLE_FILE']
    hello_webgl_sources = [test_file('minimal_webgl/main.c'),
                           test_file('minimal_webgl/webgl.c'),
                           '--js-library', test_file('minimal_webgl/library_js.js'),
                           '-lGL',
                           '-sMODULARIZE']
    hello_webgl2_sources = hello_webgl_sources + ['-sMAX_WEBGL_VERSION=2']
    hello_wasm_worker_sources = [test_file('wasm_worker/wasm_worker_code_size.c'), '-sWASM_WORKERS', '-sENVIRONMENT=web,worker']
    embind_hello_sources = [test_file('code_size/embind_hello_world.cpp'), '-lembind']
    embind_val_sources = [test_file('code_size/embind_val_hello_world.cpp'),
                          '-lembind',
                          '-fno-rtti',
                          '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0',
                          '-sDYNAMIC_EXECUTION=0']

    sources = {
      'hello_world': hello_world_sources,
      'random_printf': random_printf_sources,
      'hello_webgl': hello_webgl_sources,
      'math': math_sources,
      'hello_webgl2': hello_webgl2_sources,
      'hello_wasm_worker': hello_wasm_worker_sources,
      'embind_val': embind_val_sources,
      'embind_hello': embind_hello_sources,
    }[test_name]

    def print_percent(actual, expected):
      if actual == expected:
        return ''
      return ' ({:+.2f}%)'.format((actual - expected) * 100.0 / expected)

    outputs = ['a.html', 'a.js']
    if '-sWASM_WORKERS' in sources:
      outputs += ['a.ww.js']

    args = smallest_code_size_args[:]

    if js:
      args += wasm2js
      test_name += '_wasm2js'
    else:
      outputs += ['a.wasm']
      test_name += '_wasm'

    if '-sSINGLE_FILE' in sources:
      outputs = ['a.html']

    results_file = test_file('code_size', test_name + '.json')

    expected_results = {}
    try:
      expected_results = json.loads(read_file(results_file))
    except Exception:
      if not common.EMTEST_REBASELINE:
        raise

    args = [compiler_for(sources[0]), '-o', 'a.html'] + args + sources
    print(shared.shlex_join(args))
    self.run_process(args)

    # For certain tests, don't just check the output size but check
    # the full JS output matches the expectations.  That means that
    # any change that touches those core lines of output will need
    # to rebaseline this test.  However:
    # a) such changes deserve extra scrutiny
    # b) such changes should be few and far between
    # c) rebaselining is trivial (just run with --rebaseline)
    # Note that we do not compare the full wasm output since that is
    # even more fragile and can change with LLVM updates.
    if compare_js_output:
      js_out = test_file('code_size', test_name + '.js')
      terser = shared.get_npm_cmd('terser')
      # N.b. this requires node in PATH, it does not run against NODE from
      # Emscripten config file. If you have this line fail, make sure 'node' is
      # visible in PATH.
      self.run_process(terser + ['-b', 'beautify=true', 'a.js', '-o', 'pretty.js'])
      self.assertFileContents(js_out, read_file('pretty.js'))

    obtained_results = {}
    total_output_size = 0
    total_expected_size = 0
    total_output_size_gz = 0
    total_expected_size_gz = 0
    for f in outputs:
      f_gz = f + '.gz'
      expected_size = expected_results[f] if f in expected_results else float('inf')
      expected_size_gz = expected_results[f_gz] if f_gz in expected_results else float('inf')
      size = os.path.getsize(f)
      size_gz = get_file_gzipped_size(f)

      obtained_results[f] = size
      obtained_results[f_gz] = size_gz

      if size != expected_size and (f.endswith('.js') or f.endswith('.html')):
        print('Contents of ' + f + ': ')
        print(read_file(f))

      print('size of ' + f + ' == ' + str(size) + ', expected ' + str(expected_size) + ', delta=' + str(size - expected_size) + print_percent(size, expected_size))
      print('size of ' + f_gz + ' == ' + str(size_gz) + ', expected ' + str(expected_size_gz) + ', delta=' + str(size_gz - expected_size_gz) + print_percent(size_gz, expected_size_gz))

      # N.B. even though the test code above prints out gzip compressed sizes, regression testing is done against uncompressed sizes
      # this is because optimizing for compressed sizes can be unpredictable and sometimes counterproductive
      total_output_size += size
      total_expected_size += expected_size

      total_output_size_gz += size_gz
      total_expected_size_gz += expected_size_gz

    obtained_results['total'] = total_output_size
    obtained_results['total_gz'] = total_output_size_gz
    print('Total output size=' + str(total_output_size) + ' bytes, expected total size=' + str(total_expected_size) + ', delta=' + str(total_output_size - total_expected_size) + print_percent(total_output_size, total_expected_size))
    print('Total output size gzipped=' + str(total_output_size_gz) + ' bytes, expected total size gzipped=' + str(total_expected_size_gz) + ', delta=' + str(total_output_size_gz - total_expected_size_gz) + print_percent(total_output_size_gz, total_expected_size_gz))

    if common.EMTEST_REBASELINE:
      create_file(results_file, json.dumps(obtained_results, indent=2) + '\n', absolute=True)
    else:
      if total_output_size > total_expected_size:
        print(f'Oops, overall generated code size regressed by {total_output_size - total_expected_size} bytes!')
        print('If this is expected, rerun the test with --rebaseline to update the expected sizes')
      if total_output_size < total_expected_size:
        print(f'Hey amazing, overall generated code size was improved by {total_expected_size - total_output_size} bytes!')
        print('If this is expected, rerun the test with --rebaseline to update the expected sizes')
      self.assertEqual(total_output_size, total_expected_size)

  # Tests the library_c_preprocessor.js functionality.
  @crossplatform
  def test_c_preprocessor(self):
    self.do_runf('test_c_preprocessor.c', emcc_args=['--js-library', path_from_root('src/library_c_preprocessor.js')])

  # Test that legacy settings that have been fixed to a specific value and their value can no longer be changed,
  def test_legacy_settings_forbidden_to_change(self):
    stderr = self.expect_fail([EMCC, '-sMEMFS_APPEND_TO_TYPED_ARRAYS=0', test_file('hello_world.c')])
    self.assertContained('emcc: error: invalid command line setting `-sMEMFS_APPEND_TO_TYPED_ARRAYS=0`: Starting from Emscripten 1.38.26, MEMFS_APPEND_TO_TYPED_ARRAYS=0 is no longer supported', stderr)

    self.run_process([EMCC, '-sMEMFS_APPEND_TO_TYPED_ARRAYS', test_file('hello_world.c')])
    self.run_process([EMCC, '-sPRECISE_I64_MATH=2', test_file('hello_world.c')])

  def test_jsmath(self):
    self.run_process([EMXX, test_file('other/jsmath.cpp'), '-Os', '-o', 'normal.js', '--closure', '0'])
    normal_js_size = os.path.getsize('normal.js')
    normal_wasm_size = os.path.getsize('normal.wasm')
    self.run_process([EMXX, test_file('other/jsmath.cpp'), '-Os', '-o', 'jsmath.js', '-sJS_MATH', '--closure', '0'])
    jsmath_js_size = os.path.getsize('jsmath.js')
    jsmath_wasm_size = os.path.getsize('jsmath.wasm')
    # js math increases JS size, but decreases wasm, and wins overall
    # it would win more with closure, but no point in making the test slower)
    self.assertLess(normal_js_size, jsmath_js_size)
    self.assertLess(jsmath_wasm_size, normal_wasm_size)
    self.assertLess(jsmath_js_size + jsmath_wasm_size, 0.90 * (normal_js_size + normal_wasm_size))
    # js math has almost identical output, but misses some corner cases, 4 out of 34
    normal = self.run_js('normal.js').splitlines()
    jsmath = self.run_js('jsmath.js').splitlines()
    self.assertEqual(len(normal), len(jsmath))
    diff = 0
    for i in range(len(normal)):
      if normal[i] != jsmath[i]:
        diff += 1
    self.assertLess(diff, 5)

  def test_strict_mode_hello_world(self):
    # Verify that strict mode can be used for simple hello world program both
    # via the environment EMCC_STRICT=1 and from the command line `-sSTRICT`
    self.do_runf('hello_world.c', emcc_args=['-sSTRICT'])
    with env_modify({'EMCC_STRICT': '1'}):
      self.do_run_in_out_file_test('hello_world.c')

  def test_strict_mode_full_library(self):
    self.do_runf('hello_world.c', emcc_args=['-sSTRICT', '-sINCLUDE_FULL_LIBRARY'])

  def test_legacy_settings(self):
    cmd = [EMCC, test_file('hello_world.c'), '-sSPLIT_MEMORY=0']

    # By default warnings are not shown
    stderr = self.run_process(cmd, stderr=PIPE).stderr
    self.assertNotContained('warning', stderr)

    # Adding or -Wlegacy-settings enables the warning
    stderr = self.run_process(cmd + ['-Wlegacy-settings'], stderr=PIPE).stderr
    self.assertContained('warning: use of legacy setting: SPLIT_MEMORY', stderr)
    self.assertContained('[-Wlegacy-settings]', stderr)

  def test_strict_mode_legacy_settings(self):
    cmd = [EMCC, test_file('hello_world.c'), '-sSPLIT_MEMORY=0']
    self.run_process(cmd)

    stderr = self.expect_fail(cmd + ['-sSTRICT'])
    self.assertContained('legacy setting used in strict mode: SPLIT_MEMORY', stderr)

    with env_modify({'EMCC_STRICT': '1'}):
      stderr = self.expect_fail(cmd)
      self.assertContained('legacy setting used in strict mode: SPLIT_MEMORY', stderr)

  def test_strict_mode_legacy_settings_runtime(self):
    # Verify that legacy settings are not accessible at runtime under strict
    # mode.
    self.set_setting('RETAIN_COMPILER_SETTINGS')
    src = r'''\
    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      printf("BINARYEN_METHOD: %s\n", (char*)emscripten_get_compiler_setting("BINARYEN_METHOD"));
      return 0;
    }
    '''
    self.do_run(src, 'BINARYEN_METHOD: native-wasm')
    with env_modify({'EMCC_STRICT': '1'}):
      self.do_run(src, 'invalid compiler setting: BINARYEN_METHOD')
    self.set_setting('STRICT')
    self.do_run(src, 'invalid compiler setting: BINARYEN_METHOD')

  def test_renamed_setting(self):
    # Verify that renamed settings are available by either name (when not in
    # strict mode.
    self.set_setting('RETAIN_COMPILER_SETTINGS')
    src = r'''\
    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      printf("%ld %ld\n",
        emscripten_get_compiler_setting("BINARYEN_ASYNC_COMPILATION"),
        emscripten_get_compiler_setting("WASM_ASYNC_COMPILATION"));
      return 0;
    }
    '''

    # Setting the new name should set both
    self.set_setting('WASM_ASYNC_COMPILATION', 0)
    self.do_run(src, '0 0')
    self.set_setting('WASM_ASYNC_COMPILATION')
    self.do_run(src, '1 1')
    self.clear_setting('WASM_ASYNC_COMPILATION')

    # Setting the old name should set both
    self.set_setting('BINARYEN_ASYNC_COMPILATION', 0)
    self.do_run(src, '0 0')
    self.set_setting('BINARYEN_ASYNC_COMPILATION')
    self.do_run(src, '1 1')

  def test_strict_mode_legacy_settings_library(self):
    create_file('lib.js', r'''
#if SPLIT_MEMORY
#endif
''')
    cmd = [EMCC, test_file('hello_world.c'), '-o', 'out.js', '--js-library', 'lib.js']
    self.run_process(cmd)
    self.assertContained('ReferenceError: SPLIT_MEMORY is not defined', self.expect_fail(cmd + ['-sSTRICT']))
    with env_modify({'EMCC_STRICT': '1'}):
      self.assertContained('ReferenceError: SPLIT_MEMORY is not defined', self.expect_fail(cmd))

  def test_strict_mode_link_cxx(self):
    # In strict mode C++ programs fail to link unless run with `em++`.
    self.run_process([EMXX, '-sSTRICT', test_file('hello_libcxx.cpp')])
    err = self.expect_fail([EMCC, '-sSTRICT', test_file('hello_libcxx.cpp')])
    self.assertContained('undefined symbol: std::__2::cout', err)

  def test_strict_mode_override(self):
    create_file('empty.c', '')
    # IGNORE_MISSING_MAIN default to 1 so this works by default
    self.run_process([EMCC, 'empty.c'])
    # strict mode disables it causing a link error
    self.expect_fail([EMCC, '-sSTRICT', 'empty.c'])
    # explicly setting IGNORE_MISSING_MAIN overrides the STRICT setting
    self.run_process([EMCC, '-sSTRICT', '-sIGNORE_MISSING_MAIN', 'empty.c'])

  # Tests the difference between options -sSAFE_HEAP=1 and -sSAFE_HEAP=2.
  @also_with_wasm64
  def test_safe_heap_2(self):
    self.do_runf('safe_heap_2.c', 'alignment fault',
                 emcc_args=['-sSAFE_HEAP=1'], assert_returncode=NON_ZERO)

    self.do_runf('safe_heap_2.c', '0 1 2 3 4',
                 emcc_args=['-sSAFE_HEAP=2'])

  @also_with_wasm2js
  def test_safe_heap_log(self):
    self.set_setting('SAFE_HEAP')
    self.set_setting('SAFE_HEAP_LOG')
    self.do_runf('hello_world.c', 'SAFE_HEAP load: ')

  def test_mini_printfs(self):
    def test(code):
      create_file('src.c', '''
          #include <stdio.h>
          void* unknown_value;
          int main() {
            %s
          }
      ''' % code)
      self.run_process([EMCC, 'src.c', '-O1'])
      return os.path.getsize('a.out.wasm')

    i = test('printf("%d", *(int*)unknown_value);')
    f = test('printf("%f", *(double*)unknown_value);')
    lf = test('printf("%Lf", *(long double*)unknown_value);')
    both = test('printf("%d", *(int*)unknown_value); printf("%Lf", *(long double*)unknown_value);')
    print(f'int:{i} float:{f} double:{lf}: both{both}')

    # iprintf is much smaller than printf with float support
    self.assertGreater(i, f - 3500)
    self.assertLess(i, f - 3000)
    # __small_printf is somewhat smaller than printf with long double support
    self.assertGreater(f, lf - 900)
    self.assertLess(f, lf - 500)
    # both is a little bigger still
    self.assertGreater(lf, both - 110)
    self.assertLess(lf, both - 50)

  @parameterized({
    'normal': ([], '''\
0.000051 => -5.123719529365189373493194580078e-05
0.000051 => -5.123719300544352718866300544498e-05
0.000051 => -5.123719300544352718866300544498e-05
'''),
    'full_long_double': (['-sPRINTF_LONG_DOUBLE'], '''\
0.000051 => -5.123719529365189373493194580078e-05
0.000051 => -5.123719300544352718866300544498e-05
0.000051 => -5.123719300544352710023893104250e-05
'''),
  })
  def test_long_double_printing(self, args, expected):
    create_file('src.c', r'''
#include <stdio.h>

int main(void) {
    float f = 5.123456789e-5;
    double d = 5.123456789e-5;
    long double ld = 5.123456789e-5;
    printf("%f => %.30e\n", f, f  / (f - 1));
    printf("%f => %.30e\n", d, d / (d - 1));
    printf("%Lf => %.30Le\n", ld, ld / (ld - 1));
}
    ''')
    self.do_runf('src.c', expected, emcc_args=args)

  # Tests that passing -sMALLOC=none will not include system malloc() to the build.
  def test_malloc_none(self):
    stderr = self.expect_fail([EMCC, test_file('malloc_none.c'), '-sMALLOC=none'])
    self.assertContained('undefined symbol: malloc', stderr)

  @parameterized({
    'c': ['c', []],
    'cpp': ['cpp', []],
    'growth': ['cpp', ['-sALLOW_MEMORY_GROWTH']],
    'wasmfs': ['c', ['-sWASMFS']],
  })
  def test_lsan_leaks(self, ext, args):
    self.do_runf(
      'other/test_lsan_leaks.' + ext,
      emcc_args=['-fsanitize=leak'] + args,
      assert_returncode=NON_ZERO,
      assert_all=True,
      expected_output=[
        'Direct leak of 2048 byte(s) in 1 object(s) allocated from',
        'Direct leak of 1337 byte(s) in 1 object(s) allocated from',
        'Direct leak of 42 byte(s) in 1 object(s) allocated from',
      ])

  @parameterized({
    'c': ['c', [
      r'in malloc .*lsan_interceptors\.cpp:\d+:\d+',
      r'(?im)in f (|[/a-z\.]:).*/test_lsan_leaks\.c:6:21$',
      r'(?im)in main (|[/a-z\.]:).*/test_lsan_leaks\.c:10:16$',
      r'(?im)in main (|[/a-z\.]:).*/test_lsan_leaks\.c:12:3$',
      r'(?im)in main (|[/a-z\.]:).*/test_lsan_leaks\.c:13:3$',
    ]],
    'cpp': ['cpp', [
      r'in operator new\[\]\(unsigned long\) .*lsan_interceptors\.cpp:\d+:\d+',
      r'(?im)in f\(\) (|[/a-z\.]:).*/test_lsan_leaks\.cpp:4:21$',
      r'(?im)in main (|[/a-z\.]:).*/test_lsan_leaks\.cpp:8:16$',
      r'(?im)in main (|[/a-z\.]:).*/test_lsan_leaks\.cpp:10:3$',
      r'(?im)in main (|[/a-z\.]:).*/test_lsan_leaks\.cpp:11:3$',
    ]],
  })
  def test_lsan_stack_trace(self, ext, regexes):
    self.do_runf(
      'other/test_lsan_leaks.' + ext,
      emcc_args=['-fsanitize=leak', '-gsource-map'],
      regex=True,
      assert_all=True,
      assert_returncode=NON_ZERO,
      expected_output=[
        r'Direct leak of 2048 byte\(s\) in 1 object\(s\) allocated from',
        r'Direct leak of 1337 byte\(s\) in 1 object\(s\) allocated from',
        r'Direct leak of 42 byte\(s\) in 1 object\(s\) allocated from',
      ] + regexes)

  @parameterized({
    'c': ['c'],
    'cpp': ['cpp'],
  })
  def test_lsan_no_leak(self, ext):
    self.do_runf('other/test_lsan_no_leak.' + ext,
                 regex=True,
                 emcc_args=['-fsanitize=leak', '-sASSERTIONS=0'],
                 expected_output=[r'^\s*$'])

  def test_lsan_no_stack_trace(self):
    self.do_runf(
      'other/test_lsan_leaks.c',
      assert_all=True,
      emcc_args=['-fsanitize=leak', '-DDISABLE_CONTEXT'],
      assert_returncode=NON_ZERO,
      expected_output=[
        'Direct leak of 3427 byte(s) in 3 object(s) allocated from:',
        'SUMMARY: LeakSanitizer: 3427 byte(s) leaked in 3 allocation(s).',
      ])

  @also_with_standalone_wasm()
  def test_asan_null_deref(self):
    self.do_runf(
      'other/test_asan_null_deref.c',
      emcc_args=['-fsanitize=address'],
      assert_returncode=NON_ZERO,
      expected_output=[
        'AddressSanitizer: null-pointer-dereference on address',
      ])

  def test_asan_sync_compilation(self):
    self.set_setting('WASM_ASYNC_COMPILATION', 0)
    self.do_runf(
      'other/test_asan_null_deref.c',
      emcc_args=['-fsanitize=address', '-gsource-map'],
      assert_returncode=NON_ZERO,
      expected_output=[
        'AddressSanitizer: null-pointer-dereference on address',
      ])

  def test_asan_memory_growth(self):
    self.do_runf(
      'other/test_asan_null_deref.c',
      emcc_args=['-fsanitize=address', '-sALLOW_MEMORY_GROWTH'],
      assert_returncode=NON_ZERO,
      expected_output=[
        'AddressSanitizer: null-pointer-dereference on address',
      ])

  def test_asan_no_stack_trace(self):
    self.do_runf(
      'other/test_lsan_leaks.c',
      assert_all=True,
      emcc_args=['-fsanitize=address', '-DDISABLE_CONTEXT'],
      assert_returncode=NON_ZERO,
      expected_output=[
        'Direct leak of 3427 byte(s) in 3 object(s) allocated from:',
        'SUMMARY: AddressSanitizer: 3427 byte(s) leaked in 3 allocation(s).',
      ])

  def test_asan_pthread_stubs(self):
    self.do_runf('other/test_asan_pthread_stubs.c', emcc_args=['-fsanitize=address'])

  def test_asan_strncpy(self):
    # Regression test for asan false positives in strncpy:
    # https://github.com/emscripten-core/emscripten/issues/14618
    self.do_runf('other/test_asan_strncpy.c', emcc_args=['-fsanitize=address'])

  @node_pthreads
  def test_proxy_to_pthread_stack(self):
    # Check that the proxied main gets run with STACK_SIZE setting and not
    # DEFAULT_PTHREAD_STACK_SIZE.
    self.do_runf('other/test_proxy_to_pthread_stack.c',
                 ['success'],
                 emcc_args=['-pthread', '-sPROXY_TO_PTHREAD',
                            '-sDEFAULT_PTHREAD_STACK_SIZE=64kb',
                            '-sSTACK_SIZE=128kb', '-sEXIT_RUNTIME',
                            '--profiling-funcs'])

  @parameterized({
    '': ([],),
    'sync': (['-sWASM_ASYNC_COMPILATION=0'],),
  })
  def test_offset_converter(self, args):
    self.set_setting('USE_OFFSET_CONVERTER')
    self.do_runf('other/test_offset_converter.c', 'ok', emcc_args=['--profiling-funcs'] + args)

  @parameterized({
    '': ([],),
    'sync': (['-sWASM_ASYNC_COMPILATION=0'],),
  })
  def test_offset_converter_source_map(self, args):
    self.set_setting('USE_OFFSET_CONVERTER')
    self.set_setting('LOAD_SOURCE_MAP')
    self.do_runf('other/test_offset_converter.c', 'ok', emcc_args=['-gsource-map', '-DUSE_SOURCE_MAP'] + args)

  @no_windows('ptys and select are not available on windows')
  def test_build_error_color(self):
    create_file('src.c', 'int main() {')
    returncode, output = self.run_on_pty([EMCC, 'src.c'])
    self.assertNotEqual(returncode, 0)
    self.assertIn(b"\x1b[1msrc.c:1:13: \x1b[0m\x1b[0;1;31merror: \x1b[0m\x1b[1mexpected '}'\x1b[0m", output)
    self.assertIn(b"\x1b[31merror: ", output)

  @parameterized({
    'fno_diagnostics_color': ['-fno-diagnostics-color'],
    'fdiagnostics_color_never': ['-fdiagnostics-color=never'],
  })
  @no_windows('ptys and select are not available on windows')
  def test_pty_no_color(self, flag):
    create_file('src.c', 'int main() {')

    returncode, output = self.run_on_pty([EMCC, flag, 'src.c'])
    self.assertNotEqual(returncode, 0)
    self.assertNotIn(b'\x1b', output)

  def test_sanitizer_color(self):
    create_file('src.c', '''
      #include <emscripten.h>
      int main() {
        int *p = 0, q;
        EM_ASM({ Module.printWithColors = true; });
        q = *p;
      }
    ''')
    self.do_runf('src.c', '\x1b[1msrc.c', emcc_args=['-fsanitize=null'])

  def test_main_reads_params(self):
    create_file('no.c', '''
      int main() {
        return 42;
      }
    ''')
    self.run_process([EMCC, 'no.c', '-O3', '-o', 'no.js'])
    no = os.path.getsize('no.js')
    create_file('yes.c', '''
      int main(int argc, char **argv) {
        return (long)argv[argc-1];
      }
    ''')
    self.run_process([EMCC, 'yes.c', '-O3', '-o', 'yes.js'])
    yes = os.path.getsize('yes.js')
    # not having to set up argc/argv allows us to avoid including a
    # significant amount of JS for string support (which is not needed
    # otherwise in such a trivial program).
    self.assertLess(no, 0.95 * yes)

  def test_INCOMING_MODULE_JS_API(self):
    def test(args):
      self.run_process([EMCC, test_file('hello_world.c'), '-O3', '--closure=1', '-sENVIRONMENT=node,shell', '--output_eol=linux'] + args)
      for engine in config.JS_ENGINES:
        self.assertContained('hello, world!', self.run_js('a.out.js', engine=engine))
      return os.path.getsize('a.out.js')
    normal = test([])
    changed = test(['-sINCOMING_MODULE_JS_API=[]'])
    print('sizes', normal, changed)
    # Changing this option to [] should decrease code size.
    self.assertLess(changed, normal)
    # Check an absolute code size as well, with some slack.
    self.check_expected_size_in_file('js',
                                     test_file('other/test_INCOMING_MODULE_JS_API.js.size'),
                                     changed)

  def test_INCOMING_MODULE_JS_API_missing(self):
    create_file('pre.js', 'Module.onRuntimeInitialized = () => out("initialized");')
    self.emcc_args += ['--pre-js=pre.js']
    self.do_runf('hello_world.c', 'initialized')

    # The INCOMING_MODULE_JS_API setting can limit the incoming module
    # API, and we assert if the incoming module has a property that
    # is ignored due to this setting.
    self.set_setting('INCOMING_MODULE_JS_API', [])

    self.do_runf('hello_world.c', 'Aborted(`Module.onRuntimeInitialized` was supplied but `onRuntimeInitialized` not included in INCOMING_MODULE_JS_API)', assert_returncode=NON_ZERO)

  def test_llvm_includes(self):
    create_file('atomics.c', '#include <stdatomic.h>')
    self.build('atomics.c')

  def test_mmap_and_munmap(self):
    emcc_args = []
    for f in ('data_ro.dat', 'data_rw.dat'):
      create_file(f, 'Test file')
      emcc_args.extend(['--embed-file', f])
    self.do_other_test('test_mmap_and_munmap.c', emcc_args)

  def test_mmap_and_munmap_anonymous(self):
    self.do_other_test('test_mmap_and_munmap_anonymous.cpp', emcc_args=['-sNO_FILESYSTEM'])

  def test_mmap_and_munmap_anonymous_asan(self):
    self.do_other_test('test_mmap_and_munmap_anonymous.cpp', emcc_args=['-sNO_FILESYSTEM', '-fsanitize=address'])

  def test_mmap_memorygrowth(self):
    self.do_other_test('test_mmap_memorygrowth.cpp', ['-sALLOW_MEMORY_GROWTH'])

  @also_with_noderawfs
  def test_mmap_empty(self):
    self.do_other_test('test_mmap_empty.c')

  def test_mmap_empty_wasmfs(self):
    self.do_other_test('test_mmap_empty.c', emcc_args=['-sWASMFS'])

  def test_files_and_module_assignment(self):
    # a pre-js can set Module to a new object or otherwise undo file preloading/
    # embedding changes to Module.preRun. we show an error to avoid confusion
    create_file('pre.js', 'Module = {};')
    create_file('src.c', r'''
      #include <stdio.h>
      int main() {
        printf("file exists: %p\n", fopen("src.cpp", "rb"));
      }
    ''')
    expected = 'Module.preRun should exist because file support used it; did a pre-js delete it?'
    self.do_runf('src.c', expected, emcc_args=['--pre-js=pre.js', '--preload-file=src.c'],
                 assert_returncode=NON_ZERO)

    def test_error(pre):
      create_file('pre.js', pre)
      expected = 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?'
      self.do_runf('src.c', expected, emcc_args=['--pre-js=pre.js', '--preload-file=src.c'],  assert_returncode=NON_ZERO)

    # error if the user replaces Module or Module.preRun
    test_error('Module = { preRun: [] };')
    test_error('Module.preRun = [];')

  def test_EMSCRIPTEN_and_STRICT(self):
    # __EMSCRIPTEN__ is the proper define; we support EMSCRIPTEN for legacy
    # code, unless STRICT is enabled.
    create_file('src.c', '''
      #ifndef EMSCRIPTEN
      #error "not defined"
      #endif
    ''')
    self.run_process([EMCC, 'src.c', '-c'])
    self.expect_fail([EMCC, 'src.c', '-sSTRICT', '-c'])

  def test_exception_settings(self):
    for catching, throwing, opts in itertools.product([0, 1], repeat=3):
      cmd = [EMXX, test_file('other/exceptions_modes_symbols_defined.cpp'), '-sDISABLE_EXCEPTION_THROWING=%d' % (1 - throwing), '-sDISABLE_EXCEPTION_CATCHING=%d' % (1 - catching), '-O%d' % opts]
      print(cmd)
      if not throwing and not catching:
        self.assertContained('DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but such support is required', self.expect_fail(cmd))
      elif not throwing and catching:
        self.assertContained('DISABLE_EXCEPTION_THROWING was set (probably from -fno-exceptions) but is not compatible with enabling exception catching (DISABLE_EXCEPTION_CATCHING=0)', self.expect_fail(cmd))
      else:
        self.run_process(cmd)

  def test_fignore_exceptions(self):
    # the new clang flag -fignore-exceptions basically is the same as -sDISABLE_EXCEPTION_CATCHING,
    # that is, it allows throwing, but emits no support code for catching.
    self.run_process([EMXX, test_file('other/exceptions_modes_symbols_defined.cpp'), '-sDISABLE_EXCEPTION_CATCHING=0'])
    enable_size = os.path.getsize('a.out.wasm')
    self.run_process([EMXX, test_file('other/exceptions_modes_symbols_defined.cpp'), '-sDISABLE_EXCEPTION_CATCHING'])
    disable_size = os.path.getsize('a.out.wasm')
    self.run_process([EMXX, test_file('other/exceptions_modes_symbols_defined.cpp'), '-s', '-fignore-exceptions'])
    ignore_size = os.path.getsize('a.out.wasm')
    self.assertGreater(enable_size, disable_size)
    self.assertEqual(disable_size, ignore_size)

  @parameterized({
    # exceptions are off by default
    'off':  ([], [], False),
    # enabling exceptions at link and compile works
    'on': (['-fexceptions'], ['-fexceptions'], True),
    # just compile isn't enough as the JS runtime lacks support
    'compile_only': (['-fexceptions'], [], False),
    # just link isn't enough as codegen didn't emit exceptions support
    'link_only': ([], ['-fexceptions'], False),
    'standalone': (['-fexceptions'], ['-fexceptions', '-sSTANDALONE_WASM', '-sWASM_BIGINT'], True),
  })
  def test_f_exception(self, compile_flags, link_flags, expect_caught):
    create_file('src.cpp', r'''
      #include <stdio.h>
      int main () {
        try {
          throw 42;
        } catch (int e) {
          printf("CAUGHT: %d\n", e);
        }
        return 0;
      }
    ''')
    self.run_process([EMXX, 'src.cpp', '-c', '-o', 'src.o'] + compile_flags)
    self.run_process([EMXX, 'src.o'] + link_flags)
    result = self.run_js('a.out.js', assert_returncode=0 if expect_caught else NON_ZERO)
    if not expect_caught:
      self.assertContainedIf('exception catching is disabled, this exception cannot be caught', result, expect_caught)
    self.assertContainedIf('CAUGHT', result, expect_caught)

  def test_exceptions_with_closure_and_without_catching(self):
    # using invokes will require setThrew(), and closure will error if it is not
    # defined. this test checks that we define it even without catching any
    # exceptions (if we did catch exceptions, that would include library code
    # that would use setThrew() anyhow)
    create_file('src.cpp', r'''
      #include <stdio.h>
      #include <emscripten.h>

      struct A {
        ~A() {
          puts("~A");
        }
      };

      int main() {
        // Construct an instance of a class with a destructor, which will cause the
        // use of invokes to ensure its destructor runs.
        A a;
        throw 5;
      }
      ''')
    self.run_process([EMCC, 'src.cpp', '-fexceptions', '--closure=1'])

  def test_assertions_on_internal_api_changes(self):
    create_file('src.c', r'''
      #include <emscripten.h>
      int main(int argc, char **argv) {
        EM_ASM({
          try {
            Module['read'];
            out('it should not be there');
          } catch(e) {
            out('error: ' + e);
          }
          try {
            Module['asm'];
            out('it should not be there');
          } catch(e) {
            out('error: ' + e);
          }
        });
      }
    ''')
    expected = [
      '`Module.asm` has been replaced by `wasmExports`',
    ]
    self.do_runf('src.c', expected, assert_all=True, emcc_args=['-sASSERTIONS'])

  def test_assertions_on_incoming_module_api_changes(self):
    create_file('pre.js', 'Module.read = () => {};')
    self.do_runf('hello_world.c', 'Module.read option was removed',
                 emcc_args=['-sASSERTIONS', '--pre-js', 'pre.js'],
                 assert_returncode=NON_ZERO)

  def test_assertions_on_outgoing_module_api_changes(self):
    create_file('src.cpp', r'''
      #include <emscripten.h>
      int main() {
        EM_ASM({
          out();
          function check(name) {
            try {
              Module[name];
              out("success: " + name);
            } catch(e) {
            }
          }
          check("read");
          // TODO check("setWindowTitle");
          check("wasmBinary");
          check("arguments");
        });
      }
    ''')
    expected = '''
Aborted(`Module.wasmBinary` has been replaced by `wasmBinary` (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name))
Aborted(`Module.arguments` has been replaced by `arguments_` (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name))
'''
    self.do_runf('src.cpp', expected, emcc_args=['-sASSERTIONS'])

  def test_modularize_assertions_on_ready_promise(self):
    # check that when assertions are on we give useful error messages for
    # mistakenly thinking the Promise is an instance. I.e., once you could do
    # Module()._main to get an instance and the main function, but after
    # the breaking change in #10697 Module() now returns a promise, and to get
    # the instance you must use .then() to get a callback with the instance.
    create_file('test.js', r'''
      try {
        Module()._main;
      } catch(e) {
        console.log(e);
      }
      try {
        Module().onRuntimeInitialized = 42;
      } catch(e) {
        console.log(e);
      }
    ''')
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sASSERTIONS', '--extern-post-js', 'test.js'])
    # A return code of 1 is from an uncaught exception not handled by
    # the domain or the 'uncaughtException' event handler.
    out = self.run_js('a.out.js', assert_returncode=1)
    self.assertContained('You are getting _main on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js', out)
    self.assertContained('You are setting onRuntimeInitialized on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js', out)

  def test_modularize_assertions_on_reject_promise(self):
    # Check that there is an uncaught exception in modularize mode.
    # Once we added an `uncaughtException` handler to the global process
    # but after a breaking change in #18743 it is now expected that the
    # user will handle the exception themselves in Modularize mode.
    create_file('test.js', r'''
      Promise.reject();
    ''')
    self.run_process([EMCC, test_file('hello_world.c'), '-sMODULARIZE', '-sASSERTIONS', '--extern-post-js', 'test.js'])
    # A return code of 1 is from an uncaught exception not handled by
    # the domain or the 'uncaughtException' event handler.
    out = self.run_js('a.out.js', assert_returncode=1)
    self.assertContained('UnhandledPromiseRejection: This error originated either by throwing inside of an async function without a catch block, or by rejecting a promise which was not handled with .catch(). The promise rejected with the reason "undefined".', out)

  def test_assertions_on_reject_promise(self):
    # Check that promise rejections give the correct error code
    # when -sASSERTIONS are enabled
    create_file('test.js', r'''
      Promise.reject();
    ''')
    self.run_process([EMCC, test_file('hello_world.c'), '-sASSERTIONS', '--extern-post-js', 'test.js'])
    # Node exits with 1 on Uncaught Fatal Exception (including unhandled rejections)
    self.run_js('a.out.js', assert_returncode=1)

  def test_on_reject_promise(self):
    # Check that promise rejections give the correct error code
    create_file('test.js', r'''
      Promise.reject();
    ''')
    self.run_process([EMCC, test_file('hello_world.c'), '--extern-post-js', 'test.js'])
    # Node exits with 1 on Uncaught Fatal Exception (including unhandled rejections)
    err = self.run_js('a.out.js', assert_returncode=1)
    self.assertContained('UnhandledPromiseRejection', err)

  def test_em_asm_duplicate_strings(self):
    # We had a regression where tow different EM_ASM strings from two diffferent
    # object files were de-duplicated in wasm-emscripten-finalize.  This used to
    # work when we used zero-based index for store the JS strings, but once we
    # switched to absolute addresses the string needs to exist twice in the JS
    # file.
    create_file('foo.c', '''
      #include <emscripten.h>
      void foo() {
        EM_ASM({ out('Hello, world!'); });
      }
    ''')
    create_file('main.c', '''
      #include <emscripten.h>

      void foo();

      int main() {
        foo();
        EM_ASM({ out('Hello, world!'); });
        return 0;
      }
    ''')
    self.run_process([EMCC, '-c', 'foo.c'])
    self.run_process([EMCC, '-c', 'main.c'])
    self.run_process([EMCC, 'foo.o', 'main.o'])
    self.assertContained('Hello, world!\nHello, world!\n', self.run_js('a.out.js'))

  def test_em_asm_c89(self):
    create_file('src.c', '''
      #include <emscripten/em_asm.h>
      int main(void) {
        EM_ASM({ out('hello'); });
      }\n''')
    self.run_process([EMCC, '-c', 'src.c',
                      '-pedantic', '-Wall', '-Werror',
                      '-Wno-gnu-zero-variadic-macro-arguments',
                      '-Wno-c23-extensions'])

  def test_em_asm_strict_c(self):
    create_file('src.c', '''
      #include <emscripten/em_asm.h>
      int main() {
        EM_ASM({ out('Hello, world!'); });
      }
    ''')
    err = self.expect_fail([EMCC, '-std=c11', 'src.c'])
    self.assertIn('EM_ASM does not work in -std=c* modes, use -std=gnu* modes instead', err)

  def test_em_asm_invalid(self):
    # Test that invalid EM_ASM in side modules since is detected at build time.
    err = self.expect_fail([EMCC, '-sSIDE_MODULE', test_file('other/test_em_asm_invalid.c')])
    self.assertContained("SyntaxError: Unexpected token '*'", err)
    self.assertContained('emcc: error: EM_ASM function validation failed', err)

  def test_boost_graph(self):
    self.do_runf('test_boost_graph.cpp', emcc_args=['-std=c++14', '-sUSE_BOOST_HEADERS'])
    self.do_runf('test_boost_graph.cpp', emcc_args=['-std=c++14', '--use-port=boost_headers'])

  def test_setjmp_em_asm(self):
    create_file('src.c', '''
      #include <emscripten.h>
      #include <setjmp.h>

      int main() {
        jmp_buf buf;
        setjmp(buf);
        EM_ASM({
          out("hello world");
        });
      }
    ''')
    err = self.expect_fail([EMCC, 'src.c'])
    self.assertIn('Cannot use EM_ASM* alongside setjmp/longjmp', err)
    self.assertIn('Please consider using EM_JS, or move the EM_ASM into another function.', err)

  def test_setjmp_emulated_casts(self):
    # using setjmp causes invokes(), and EMULATE_FUNCTION_POINTER_CASTS changes
    # how the wasm table works; test that they work together properly
    create_file('src.c', r'''
      #include <stdio.h>
      #include <setjmp.h>
      int main() {
        jmp_buf jb;
        if (!setjmp(jb)) {
          printf("ok\n");
          longjmp(jb, 1);
        } else {
          printf("done\n");
        }
      }
    ''')
    self.do_runf('src.c', 'ok\ndone\n', emcc_args=['-sEMULATE_FUNCTION_POINTER_CASTS'])

  def test_no_lto(self):
    # This used to fail because settings.LTO didn't reflect `-fno-lto`.
    # See bug https://github.com/emscripten-core/emscripten/issues/20308
    create_file('src.c', r'''
      #include <stdio.h>
      #include <setjmp.h>
      int main() {
        jmp_buf jb;
        if (!setjmp(jb)) {
          printf("ok\n");
          longjmp(jb, 1);
        } else {
          printf("done\n");
        }
      }
    ''')
    self.do_runf('src.c', 'ok\ndone\n', emcc_args=['-flto', '-fno-lto'])

  def test_missing_stdlibs(self):
    # Certain standard libraries are expected to be useable via -l flags but
    # don't actually exist in our standard library path.  Make sure we don't
    # error out when linking with these flags.
    self.run_process([EMCC, test_file('hello_world.c'), '-lm', '-ldl', '-lrt', '-lpthread'])

  def test_supported_linker_flags(self):
    out = self.run_process([EMCC, test_file('hello_world.c'), '-Wl,-rpath=foo'], stderr=PIPE).stderr
    self.assertContained('warning: ignoring unsupported linker flag: `-rpath=foo`', out)

    out = self.run_process([EMCC, test_file('hello_world.c'), '-Wl,-rpath-link,foo'], stderr=PIPE).stderr
    self.assertContained('warning: ignoring unsupported linker flag: `-rpath-link`', out)

    out = self.run_process([EMCC, test_file('hello_world.c'),
                            '-Wl,--no-check-features,-mllvm,--data-sections'], stderr=PIPE).stderr
    self.assertNotContained('warning: ignoring unsupported linker flag', out)

    out = self.run_process([EMCC, test_file('hello_world.c'), '-Wl,-allow-shlib-undefined'], stderr=PIPE).stderr
    self.assertContained('warning: ignoring unsupported linker flag: `-allow-shlib-undefined`', out)

    out = self.run_process([EMCC, test_file('hello_world.c'), '-Wl,--allow-shlib-undefined'], stderr=PIPE).stderr
    self.assertContained('warning: ignoring unsupported linker flag: `--allow-shlib-undefined`', out)

    out = self.run_process([EMCC, test_file('hello_world.c'), '-Wl,-version-script,foo'], stderr=PIPE).stderr
    self.assertContained('warning: ignoring unsupported linker flag: `-version-script`', out)

  def test_supported_linker_flag_skip_next(self):
    # Regression test for a bug where skipping an unsupported linker flag
    # could skip the next unrelated linker flag.
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-Wl,-rpath=foo', '-lbar'])
    self.assertContained('error: unable to find library -lbar', err)

  def test_linker_flags_pass_through(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-Wl,--waka'])
    self.assertContained('wasm-ld: error: unknown argument: --waka', err)

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-Xlinker', '--waka'])
    self.assertContained('wasm-ld: error: unknown argument: --waka', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '-z', 'foo'], stderr=PIPE).stderr
    self.assertContained('wasm-ld: warning: unknown -z value: foo', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '-zfoo'], stderr=PIPE).stderr
    self.assertContained('wasm-ld: warning: unknown -z value: foo', err)

  def test_linker_flags_unused(self):
    err = self.run_process([EMCC, test_file('hello_world.c'), '-c', '-lbar'], stderr=PIPE).stderr
    self.assertContained("warning: -lbar: 'linker' input unused [-Wunused-command-line-argument]", err)

    # Check that we don't see these "input unused" errors for linker flags when
    # compiling and linking in single step (i.e. ensure that we don't pass them to clang when
    # compiling internally).
    err = self.run_process([EMCC, test_file('hello_world.c'), '-Wl,-static', '-Xlinker', '-static'], stderr=PIPE).stderr
    self.assertNotContained("input unused", err)

  def test_linker_input_unused(self):
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    err = self.run_process([EMCC, 'hello_world.o', '-c', '-o', 'out.o'], stderr=PIPE).stderr
    self.assertContained("clang: warning: hello_world.o: 'linker' input unused [-Wunused-command-line-argument]", err)
    # In this case the compiler does not produce any output file.
    self.assertNotExists('out.o')

  def test_non_wasm_without_wasm_in_vm(self):
    # Test that our non-wasm output does not depend on wasm support in the vm.
    self.run_process([EMCC, test_file('hello_world.c'), '-sWASM=0', '-sENVIRONMENT=node,shell'])
    js = read_file('a.out.js')
    create_file('a.out.js', 'var WebAssembly = null;\n' + js)
    for engine in config.JS_ENGINES:
      self.assertContained('hello, world!', self.run_js('a.out.js', engine=engine))

  def test_empty_output_extension(self):
    # Default to JS output when no extension is present
    self.run_process([EMCC, test_file('hello_world.c'), '-Werror', '-o', 'hello'])
    self.assertContained('hello, world!', self.run_js('hello'))

  def test_backwards_deps_in_archive(self):
    # Test that JS dependencies on native code work for code linked via
    # static archives using -l<name>
    self.run_process([EMCC, '-c', test_file('sockets/test_gethostbyname.c'), '-o', 'a.o'])
    self.run_process([LLVM_AR, 'cr', 'liba.a', 'a.o'])
    create_file('empty.c', 'static int foo = 0;')
    self.do_runf('empty.c', 'success', emcc_args=['-la', '-L.'])

  def test_warning_flags(self):
    self.run_process([EMCC, '-c', '-o', 'hello.o', test_file('hello_world.c')])
    cmd = [EMCC, 'hello.o', '-o', 'a.js', '-g', '--closure=1']

    # warning that is enabled by default
    stderr = self.run_process(cmd, stderr=PIPE).stderr
    self.assertContained('emcc: warning: disabling closure because debug info was requested [-Wemcc]', stderr)

    # -w to suppress warnings
    stderr = self.run_process(cmd + ['-w'], stderr=PIPE).stderr
    self.assertNotContained('warning', stderr)

    # -Wno-invalid-input to suppress just this one warning
    stderr = self.run_process(cmd + ['-Wno-emcc'], stderr=PIPE).stderr
    self.assertNotContained('warning', stderr)

    # with -Werror should fail
    stderr = self.expect_fail(cmd + ['-Werror'])
    self.assertContained('error: disabling closure because debug info was requested [-Wemcc] [-Werror]', stderr)

    # with -Werror + -Wno-error=<type> should only warn
    stderr = self.run_process(cmd + ['-Werror', '-Wno-error=emcc'], stderr=PIPE).stderr
    self.assertContained('emcc: warning: disabling closure because debug info was requested [-Wemcc]', stderr)

    # check that `-Werror=foo` also enales foo
    stderr = self.expect_fail(cmd + ['-Werror=legacy-settings', '-sTOTAL_MEMORY'])
    self.assertContained('error: use of legacy setting: TOTAL_MEMORY (setting renamed to INITIAL_MEMORY) [-Wlegacy-settings] [-Werror]', stderr)

    # check that `-Wno-pthreads-mem` disables pthread + ALLOW_GROWTH_MEMORY warning
    stderr = self.run_process(cmd + ['-Wno-pthreads-mem-growth', '-pthread', '-sALLOW_MEMORY_GROWTH'], stderr=PIPE).stderr
    self.assertNotContained('pthreads + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271', stderr)

  def test_emranlib(self):
    create_file('foo.c', 'int foo = 1;')
    create_file('bar.c', 'int bar = 2;')
    self.run_process([EMCC, '-c', 'foo.c', 'bar.c'])

    # Create a library with no archive map
    self.run_process([EMAR, 'crS', 'liba.a', 'foo.o', 'bar.o'])
    output = self.run_process([shared.LLVM_NM, '--print-armap', 'liba.a'], stdout=PIPE).stdout
    self.assertNotContained('Archive map', output)

    # Add an archive map
    self.run_process([EMRANLIB, 'liba.a'])
    output = self.run_process([shared.LLVM_NM, '--print-armap', 'liba.a'], stdout=PIPE).stdout
    self.assertContained('Archive map', output)

  def test_pthread_stub(self):
    # Verify that programs containing pthread code can still work even
    # without enabling threads.  This is possible becase we link in
    # libpthread_stub.a
    self.do_other_test('test_pthread_stub.c')

  @node_pthreads
  def test_main_pthread_join_detach(self):
    # Verify that we're unable to join the main thread
    self.do_other_test('test_pthread_self_join_detach.c')

  @node_pthreads
  def test_proxy_pthread_join_detach(self):
    # Verify that we're unable to detach or join the proxied main thread
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.do_other_test('test_pthread_self_join_detach.c')

  @node_pthreads
  def test_pthread_asyncify(self):
    # We had a infinite recursion bug when enabling PTHREADS_DEBUG + ASYNCIFY.
    # This was because PTHREADS_DEBUG calls back into WebAssembly for each call to `err()`.
    self.set_setting('PTHREADS_DEBUG')
    self.set_setting('ASYNCIFY')
    self.set_setting('PTHREAD_POOL_SIZE', 2)
    self.do_other_test('test_pthread_asyncify.c')

  @node_pthreads
  def test_pthread_reuse(self):
    self.set_setting('PTHREAD_POOL_SIZE', 1)
    self.do_other_test('test_pthread_reuse.c')

  @parameterized({
    '': ([],),
    'offscreen_canvas': (['-sOFFSCREENCANVAS_SUPPORT', '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$GL'],),
  })
  @node_pthreads
  def test_pthread_hello(self, args):
    self.do_other_test('test_pthread_hello.c', args)

  @node_pthreads
  def test_pthread_relocatable(self):
    self.do_run_in_out_file_test('hello_world.c', emcc_args=['-sRELOCATABLE'])

  @node_pthreads
  def test_pthread_unavailable(self):
    # Run a simple hello world program that uses pthreads
    self.emcc_args += ['-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME']
    self.do_run_in_out_file_test('hello_world.c')

    # Now run the same program but with SharedArrayBuffer undefined, it should run
    # fine and then fail on the first call to pthread_create.
    create_file('pre.js', 'SharedArrayBuffer = undefined\n')
    expected = 'pthread_create: environment does not support SharedArrayBuffer, pthreads are not available'
    self.do_runf('hello_world.c', expected, assert_returncode=NON_ZERO, emcc_args=['--pre-js=pre.js'])

  def test_stdin_preprocess(self):
    create_file('temp.h', '#include <string>')
    outputStdin = self.run_process([EMCC, '-x', 'c++', '-dM', '-E', '-'], input="#include <string>", stdout=PIPE).stdout
    outputFile = self.run_process([EMCC, '-x', 'c++', '-dM', '-E', 'temp.h'], stdout=PIPE).stdout
    self.assertTextDataIdentical(outputStdin, outputFile)

  def test_stdin_compile_only(self):
    # Should fail without -x lang specifier
    src = read_file(test_file('hello_world.cpp'))
    err = self.expect_fail([EMCC, '-c', '-'], input=src)
    self.assertContained('error: -E or -x required when input is from standard input', err)

    self.run_process([EMCC, '-c', '-o', 'out.o', '-x', 'c++', '-'], input=src)
    self.assertExists('out.o')

    # Same again but without an explicit output filename
    self.run_process([EMCC, '-c', '-x', 'c++', '-'], input=src)
    self.assertExists('-.o')

  def test_stdin_compile_and_link(self):
    self.run_process([EMCC, '-x', 'c++', '-'], input=read_file(test_file('hello_world.cpp')))
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  def test_stdout_link(self):
    # linking to stdout `-` doesn't work, we have no way to pass such an output filename
    # through post-link tools such as binaryen.
    err = self.expect_fail([EMCC, '-o', '-', test_file('hello_world.c')])
    self.assertContained('invalid output filename: `-`', err)
    self.assertNotExists('-')

    err = self.expect_fail([EMCC, '-o', '-foo', test_file('hello_world.c')])
    self.assertContained('invalid output filename: `-foo`', err)
    self.assertNotExists('-foo')

  def test_immutable_after_link(self):
    # some builds are guaranteed to not require any binaryen work after wasm-ld
    def ok(args, filename='hello_world.cpp', expected='hello, world!'):
      print('ok', args, filename)
      args += ['-sERROR_ON_WASM_CHANGES_AFTER_LINK']
      self.do_runf(filename, expected, emcc_args=args)

    # -O0 with BigInt support (to avoid the need for legalization)
    required_flags = ['-sWASM_BIGINT']
    ok(required_flags)
    # Same with DWARF
    ok(required_flags + ['-g'])
    # Function pointer calls from JS work too
    ok(required_flags, filename='hello_world_main_loop.cpp')
    # -O1 is ok as we don't run wasm-opt there (but no higher, see below)
    ok(required_flags + ['-O1'])
    # Exception support shouldn't require changes after linking
    ok(required_flags + ['-fexceptions'])
    # Standalone mode should not do anything special to the wasm.
    ok(required_flags + ['-sSTANDALONE_WASM'])

    # other builds fail with a standard message + extra details
    def fail(args, details):
      print('fail', args, details)
      args += ['-sERROR_ON_WASM_CHANGES_AFTER_LINK']
      err = self.expect_fail([EMCC, test_file('hello_world.c')] + args)
      self.assertContained('changes to the wasm are required after link, but disallowed by ERROR_ON_WASM_CHANGES_AFTER_LINK', err)
      self.assertContained(details, err)

    # plain -O0
    legalization_message = 'to disable int64 legalization (which requires changes after link) use -sWASM_BIGINT'
    fail([], legalization_message)
    # optimized builds even without legalization
    optimization_message = '-O2+ optimizations always require changes, build with -O0 or -O1 instead'
    fail(required_flags + ['-O2'], optimization_message)
    fail(required_flags + ['-O3'], optimization_message)

  @crossplatform
  def test_output_to_nowhere(self):
    self.run_process([EMCC, test_file('hello_world.c'), '-o', os.devnull, '-c'])

  # Test that passing -sMIN_X_VERSION=-1 on the command line will result in browser X being not supported at all.
  # I.e. -sMIN_X_VERSION=-1 is equal to -sMIN_X_VERSION=Infinity
  def test_drop_support_for_browser(self):
    # Test that -1 means "not supported"
    self.run_process([EMCC, test_file('test_html5_core.c')])
    self.assertContained('document.webkitFullscreenEnabled', read_file('a.out.js'))
    self.run_process([EMCC, test_file('test_html5_core.c'), '-sMIN_SAFARI_VERSION=-1'])
    self.assertNotContained('document.webkitFullscreenEnabled', read_file('a.out.js'))

  def test_errno_type(self):
    create_file('errno_type.c', '''
#include <errno.h>

// Use of these constants in C preprocessor comparisons should work.
#if EPERM > 0
#define DAV1D_ERR(e) (-(e))
#else
#define DAV1D_ERR(e) (e)
#endif
''')
    self.run_process([EMCC, 'errno_type.c'])

  @also_with_wasmfs
  def test_standalone_syscalls(self):
    self.run_process([EMXX, test_file('other/test_standalone_syscalls.cpp'), '-o', 'test.wasm'])
    expected = read_file(test_file('other/test_standalone_syscalls.out'))
    for engine in config.WASM_ENGINES:
      self.assertContained(expected, self.run_js('test.wasm', engine))

  @parameterized({
    'wasm2js': (['-sWASM=0'],),
    'modularize': (['-sMODULARIZE', '--extern-post-js', test_file('modularize_post_js.js')],),
  })
  def test_promise_polyfill(self, constant_args):
    def test(args, expect_fail):
      # legacy browsers may lack Promise, which wasm2js depends on. see what
      # happens when we kill the global Promise function.
      self.run_process([EMCC, test_file('hello_world.c')] + constant_args + args)
      js = read_file('a.out.js')
      create_file('a.out.js', 'Promise = undefined;\n' + js)
      return self.run_js('a.out.js', assert_returncode=NON_ZERO if expect_fail else 0)

    # we fail without legacy support
    test([], expect_fail=True)

    # but work with it
    output = test(['-sLEGACY_VM_SUPPORT'], expect_fail=False)
    self.assertContained('hello, world!', output)

    # unless we explicitly disable polyfills
    test(['-sLEGACY_VM_SUPPORT', '-sNO_POLYFILL'], expect_fail=True)

  @parameterized({
    '': ([],),
    'assertions': (['-sASSERTIONS'],),
    'closure': (['-sASSERTIONS', '--closure=1'],),
    'dylink': (['-sMAIN_MODULE'],),
  })
  def test_webgpu_compiletest(self, args):
    self.run_process([EMXX, test_file('webgpu_jsvalstore.cpp'), '-sUSE_WEBGPU', '-sASYNCIFY'] + args)

  def test_signature_mismatch(self):
    create_file('a.c', 'void foo(); int main() { foo(); return 0; }')
    create_file('b.c', 'int foo() { return 1; }')
    stderr = self.run_process([EMCC, 'a.c', 'b.c'], stderr=PIPE).stderr
    self.assertContained('function signature mismatch: foo', stderr)
    self.expect_fail([EMCC, '-Wl,--fatal-warnings', 'a.c', 'b.c'])
    # STRICT mode implies fatal warnings
    self.expect_fail([EMCC, '-sSTRICT', 'a.c', 'b.c'])
    # Unless `--no-fatal-warnings` is explictly passed
    stderr = self.run_process([EMCC, '-sSTRICT', '-Wl,--no-fatal-warnings', 'a.c', 'b.c'], stderr=PIPE).stderr
    self.assertContained('function signature mismatch: foo', stderr)

  # Verifies that warning messages that Closure outputs are recorded to console
  def test_closure_warnings(self):
    # Default should be no warnings
    proc = self.run_process([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1'], stderr=PIPE)
    self.assertNotContained('WARNING', proc.stderr)
    proc = self.run_process([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-Wno-closure'], stderr=PIPE)
    self.assertNotContained('WARNING', proc.stderr)

    proc = self.run_process([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-Wclosure'], stderr=PIPE)
    self.assertContained('WARNING - [JSC_REFERENCE_BEFORE_DECLARE] Variable referenced before declaration', proc.stderr)

    self.expect_fail([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-Werror=closure'])

    # Run the same tests again with deprecated `-sCLOSURE_WARNINGS` setting instead
    proc = self.run_process([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-Wno-deprecated'], stderr=PIPE)
    self.assertNotContained('WARNING', proc.stderr)
    proc = self.run_process([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-sCLOSURE_WARNINGS=quiet', '-Wno-deprecated'], stderr=PIPE)
    self.assertNotContained('WARNING', proc.stderr)

    proc = self.run_process([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-sCLOSURE_WARNINGS=warn', '-Wno-deprecated'], stderr=PIPE)
    self.assertContained('WARNING - [JSC_REFERENCE_BEFORE_DECLARE] Variable referenced before declaration', proc.stderr)

    self.expect_fail([EMCC, test_file('test_closure_warning.c'), '-O3', '--closure=1', '-sCLOSURE_WARNINGS=error', '-Wno-deprecated'])

  def test_bitcode_input(self):
    # Verify that bitcode files are accepted as input
    create_file('main.c', 'void foo(); int main() { return 0; }')
    self.run_process([EMCC, '-emit-llvm', '-c', '-o', 'main.bc', 'main.c'])
    self.assertTrue(is_bitcode('main.bc'))
    self.run_process([EMCC, '-c', '-o', 'main.o', 'main.bc'])
    self.assertTrue(building.is_wasm('main.o'))

  @with_env_modify({'EMCC_LOGGING': '0'})  # this test assumes no emcc output
  def test_nostdlib(self):
    err = 'undefined symbol'
    self.assertContained(err, self.expect_fail([EMCC, test_file('unistd/close.c'), '-nostdlib']))
    self.assertContained(err, self.expect_fail([EMCC, test_file('unistd/close.c'), '-nodefaultlibs']))

    # Build again but with explit system libraries
    libs = ['-lc', '-lcompiler_rt']
    self.run_process([EMCC, test_file('unistd/close.c'), '-nostdlib'] + libs)
    self.run_process([EMCC, test_file('unistd/close.c'), '-nodefaultlibs'] + libs)
    self.run_process([EMCC, test_file('unistd/close.c'), '-nolibc', '-lc'])
    self.run_process([EMCC, test_file('unistd/close.c'), '-nostartfiles'])

  def test_argument_match(self):
    # Verify that emcc arguments match precisely.  We had a bug where only the prefix
    # was matched
    self.run_process([EMCC, test_file('hello_world.c'), '--minify=0'])
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--minifyXX'])
    # The clang error message changed from 'unsupported' to 'unknown' so
    # for now handle both options.
    self.assertContained("clang: error: (unsupported option|unknown argument:) '--minifyXX'", err, regex=True)

  def test_argument_missing(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--minify'])
    self.assertContained("error: option '--minify' requires an argument", err)

  def test_argument_missing_file(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--pre-js', 'foo.js'])
    self.assertContained("emcc: error: '--pre-js': file not found: 'foo.js'", err)

  def test_default_to_cxx(self):
    create_file('foo.h', '#include <string.h>')
    create_file('cxxfoo.h', '#include <string>')

    # Compiling a C++ header using `em++` works.
    self.run_process([EMXX, '-c', 'cxxfoo.h'])

    # Compiling the same header using `emcc` fails, just like `clang`
    err = self.expect_fail([EMCC, '-c', 'cxxfoo.h', '-sSTRICT'])
    self.assertContained("'string' file not found", err)

    # But it works if we pass and explicit language mode.
    self.run_process([EMCC, '-c', 'cxxfoo.h', '-x', 'c++-header'])
    self.run_process([EMCC, '-c', 'cxxfoo.h', '-x', 'c++'])

  @parameterized({
    '': ([],),
    'minimal': (['-sMINIMAL_RUNTIME', '-sSUPPORT_ERRNO'],),
  })
  def test_support_errno(self, args):
    self.emcc_args += args + ['-sEXPORTED_FUNCTIONS=_main,___errno_location', '-Wno-deprecated']

    self.do_other_test('test_support_errno.c')
    size_default = os.path.getsize('test_support_errno.js')

    # Run the same test again but with SUPPORT_ERRNO disabled.  This time we don't expect errno
    # to be set after the failing syscall.
    self.emcc_args += ['-sSUPPORT_ERRNO=0']
    self.do_other_test('test_support_errno.c', out_suffix='_disabled')

    # Verify the JS output was smaller
    self.assertLess(os.path.getsize('test_support_errno.js'), size_default)

  def test_assembly(self):
    self.run_process([EMCC, '-c', test_file('other/test_asm.s'), '-o', 'foo.o'])
    self.do_other_test('test_asm.c', libraries=['foo.o'])

  def test_assembly_preprocessed(self):
    self.run_process([EMCC, '-c', test_file('other/test_asm_cpp.S'), '-o', 'foo.o'])
    self.do_other_test('test_asm.c', libraries=['foo.o'])

  @parameterized({
    '': (['-DUSE_KEEPALIVE'],),
    'minimal': (['-DUSE_KEEPALIVE', '-sMINIMAL_RUNTIME'],),
    'command_line': (['-sEXPORTED_FUNCTIONS=_g_foo,_main'],),
  })
  def test_export_global_address(self, args):
    self.do_other_test('test_export_global_address.c', emcc_args=args)

  def test_linker_version(self):
    out = self.run_process([EMCC, '-Wl,--version'], stdout=PIPE).stdout
    self.assertContained('LLD ', out)

  # Tests that if a JS library function is missing, the linker will print out which function
  # depended on the missing function.
  def test_chained_js_error_diagnostics(self):
    err = self.expect_fail([EMCC, test_file('test_chained_js_error_diagnostics.c'), '--js-library', test_file('test_chained_js_error_diagnostics.js')])
    self.assertContained('emscripten_js_symbols.so: undefined symbol: nonexistent_function. Required by foo', err)

    # Test without chaining.  In this case we don't include the JS library at
    # all resulting in `foo` being undefined in the native code.
    err = self.expect_fail([EMCC, test_file('test_chained_js_error_diagnostics.c')])
    self.assertContained('undefined symbol: foo', err)
    self.assertNotContained('referenced by top-level compiled C/C++ code', err)

  def test_xclang_flag(self):
    create_file('foo.h', ' ')
    self.run_process([EMCC, '-c', '-o', 'out.o', '-Xclang', '-include', '-Xclang', 'foo.h', test_file('hello_world.c')])

  def test_emcc_size_parsing(self):
    create_file('foo.h', ' ')
    err = self.expect_fail([EMCC, '-sTOTAL_MEMORY=X', 'foo.h'])
    self.assertContained('error: invalid byte size `X`.  Valid suffixes are: kb, mb, gb, tb', err)
    err = self.expect_fail([EMCC, '-sTOTAL_MEMORY=11PB', 'foo.h'])
    self.assertContained('error: invalid byte size `11PB`.  Valid suffixes are: kb, mb, gb, tb', err)

  def test_native_call_before_init(self):
    self.set_setting('ASSERTIONS')
    self.set_setting('EXPORTED_FUNCTIONS', ['_foo'])
    self.add_pre_run('out("calling foo"); Module["_foo"]();')
    create_file('foo.c', '#include <stdio.h>\nint foo() { puts("foo called"); return 3; }')
    self.build('foo.c')
    out = self.run_js('foo.js', assert_returncode=NON_ZERO)
    self.assertContained('native function `foo` called before runtime initialization', out)

  def test_native_call_after_exit(self):
    self.set_setting('ASSERTIONS')
    self.set_setting('EXIT_RUNTIME')
    self.add_on_exit('out("calling main again"); Module["_main"]();')
    create_file('foo.c', '#include <stdio.h>\nint main() { puts("foo called"); return 0; }')
    self.build('foo.c')
    out = self.run_js('foo.js', assert_returncode=NON_ZERO)
    self.assertContained('native function `main` called after runtime exit', out)

  def test_native_call_nargs(self):
    self.set_setting('ASSERTIONS')
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_foo'])
    create_file('foo.c', r'''
      #include <emscripten.h>
      void foo(int arg) {}
      int main() {
        EM_ASM(_foo(99, 100));
      }
    ''')
    self.build('foo.c')
    out = self.run_js('foo.js', assert_returncode=NON_ZERO)
    self.assertContained('native function `foo` called with 2 args but expects 1', out)

  def test_metadce_wasm2js_i64(self):
    # handling i64 unsigned remainder brings in some i64 support code. metadce
    # must not remove it.
    create_file('src.c', r'''
int main(int argc, char **argv) {
  // Intentionally do not print anything, to not bring in more code than we
  // need to test - this only tests that we do not crash, which we would if
  // metadce broke us.
  unsigned long long x = argc;
  // do some i64 math, but return 0
  return (x % (x - 20)) == 42;
}''')
    self.do_runf('src.c', emcc_args=['-O3', '-sWASM=0'])

  def test_deterministic(self):
    # test some things that may not be nondeterministic
    create_file('src.c', r'''
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main () {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  printf("C now: %lld %ld\n", now.tv_sec, now.tv_nsec);
  printf("js now: %f\n", emscripten_get_now());
  printf("C randoms: %d %d %d\n", rand(), rand(), rand());
  printf("JS random: %d\n", EM_ASM_INT({ return Math.random() }));
}
''')
    self.run_process([EMCC, 'src.c', '-sDETERMINISTIC'] + self.get_emcc_args())
    one = self.run_js('a.out.js')
    # ensure even if the time resolution is 1 second, that if we see the real
    # time we'll see a difference
    time.sleep(2)
    two = self.run_js('a.out.js')
    self.assertIdentical(one, two)

  def test_err(self):
    self.do_other_test('test_err.c')

  def test_euidaccess(self):
    self.do_other_test('test_euidaccess.c')

  def test_shared_flag(self):
    # Test that `-shared` flag causes object file generation but gives a warning
    err = self.run_process([EMCC, '-shared', test_file('hello_world.c'), '-o', 'out.foo'], stderr=PIPE).stderr
    self.assertContained('linking a library with `-shared` will emit a static object', err)
    self.assertIsObjectFile('out.foo')

    # Test that using an executable output name overrides the `-shared` flag, but produces a warning.
    err = self.run_process([EMCC, '-shared', test_file('hello_world.c'), '-o', 'out.js'],
                           stderr=PIPE).stderr
    self.assertContained('warning: -shared/-r used with executable output suffix', err)
    self.run_js('out.js')

  def test_shared_and_side_module_flag(self):
    # Test that `-shared` and `-sSIDE_MODULE` flag causes wasm dylib generation without a warning.
    err = self.run_process([EMCC, '-shared', '-sSIDE_MODULE', test_file('hello_world.c'), '-o', 'out.foo'], stderr=PIPE).stderr
    self.assertNotContained('linking a library with `-shared` will emit a static object', err)
    self.assertIsWasmDylib('out.foo')

    # Test that `-shared` and `-sSIDE_MODULE` flag causes wasm dylib generation without a warning even if given executable output name.
    err = self.run_process([EMCC, '-shared', '-sSIDE_MODULE', test_file('hello_world.c'), '-o', 'out.wasm'],
                           stderr=PIPE).stderr
    self.assertNotContained('warning: -shared/-r used with executable output suffix', err)
    self.assertIsWasmDylib('out.wasm')

  @no_windows('windows does not support shbang syntax')
  @with_env_modify({'EMMAKEN_JUST_CONFIGURE': '1'})
  def test_autoconf_mode(self):
    self.run_process([EMCC, test_file('hello_world.c')])
    # Test that output name is just `a.out` and that it is directly executable
    output = self.run_process([os.path.abspath('a.out')], stdout=PIPE).stdout
    self.assertContained('hello, world!', output)

  def test_standalone_export_main(self):
    # Tests that explicitly exported `_main` does not fail, even though `_start` is the entry
    # point.
    # We should consider making this a warning since the `_main` export is redundant.
    self.run_process([EMCC, '-sEXPORTED_FUNCTIONS=_main', '-sSTANDALONE_WASM', test_file('core/test_hello_world.c')])

  @requires_wasm_exnref
  def test_standalone_wasm_exceptions(self):
    self.set_setting('STANDALONE_WASM')
    self.set_setting('WASM_BIGINT')
    self.wasm_engines = []
    self.emcc_args += ['-fwasm-exceptions']
    self.do_run_in_out_file_test('core/test_exceptions.cpp', out_suffix='_caught')
    # New Wasm EH with exnref
    self.set_setting('WASM_EXNREF')
    self.do_run_in_out_file_test('core/test_exceptions.cpp', out_suffix='_caught')

  def test_missing_malloc_export(self):
    # we used to include malloc by default. show a clear error in builds with
    # ASSERTIONS to help with any confusion when the user calls malloc/free
    # directly
    create_file('unincluded_malloc.c', r'''
      #include <emscripten.h>
      int main() {
        EM_ASM({
          try {
            _malloc(1);
          } catch(e) {
            out('exception:', e);
          }
          try {
            _free();
          } catch(e) {
            out('exception:', e);
          }
        });
      }
    ''')
    self.do_runf('unincluded_malloc.c', (
      'malloc() called but not included in the build - add `_malloc` to EXPORTED_FUNCTIONS',
      'free() called but not included in the build - add `_free` to EXPORTED_FUNCTIONS'), assert_all=True)

  def test_getrusage(self):
    self.do_runf('other/test_getrusage.c')

  @with_env_modify({'EMMAKEN_COMPILER': shared.CLANG_CC})
  def test_emmaken_compiler(self):
    stderr = self.expect_fail([EMCC, '-c', test_file('core/test_hello_world.c')])
    self.assertContained('emcc: error: `EMMAKEN_COMPILER` is no longer supported', stderr)

  @with_env_modify({'EMMAKEN_CFLAGS': '-O2'})
  def test_emmaken_cflags(self):
    stderr = self.expect_fail([EMCC, '-c', test_file('core/test_hello_world.c')])
    self.assertContained('emcc: error: `EMMAKEN_CFLAGS` is no longer supported', stderr)

  @no_windows('relies on a shell script')
  def test_compiler_wrapper(self):
    create_file('wrapper.sh', '''\
#!/bin/sh
echo "wrapping compiler call: $@"
exec "$@"
    ''')
    make_executable('wrapper.sh')
    with env_modify({'EM_COMPILER_WRAPPER': './wrapper.sh'}):
      stdout = self.run_process([EMCC, '-c', test_file('core/test_hello_world.c')], stdout=PIPE).stdout
    self.assertContained('wrapping compiler call: ', stdout)
    self.assertExists('test_hello_world.o')

    stdout = self.run_process([EMCC, '-c', test_file('core/test_hello_world.c'), '--compiler-wrapper=./wrapper.sh'], stdout=PIPE).stdout
    self.assertContained('wrapping compiler call: ', stdout)
    self.assertExists('test_hello_world.o')

  @requires_tool('ccache')
  @with_env_modify({'EM_COMPILER_WRAPPER': 'ccache'})
  def test_compiler_wrapper_ccache(self):
    self.do_runf('hello_world.c', 'hello, world!')

  def test_llvm_option_dash_o(self):
    # emcc used to interpret -mllvm's option value as the output file if it
    # began with -o
    stderr = self.run_process(
      [EMCC, '-v', '-o', 'llvm_option_dash_o_output', '-mllvm',
       '-opt-bisect-limit=1', test_file('hello_world.c')],
      stderr=PIPE).stderr

    self.assertExists('llvm_option_dash_o_output')
    self.assertNotExists('pt-bisect-limit=1')
    self.assertContained(' -mllvm -opt-bisect-limit=1 ', stderr)

    # Regression test for #12236: the '-mllvm' argument was indexed instead of
    # its value, and the index was out of bounds if the argument was sixth or
    # further on the command line
    self.run_process(
      [EMCC, '-DFOO', '-DBAR', '-DFOOBAR', '-DBARFOO',
       '-o', 'llvm_option_dash_o_output', '-mllvm', '-opt-bisect-limit=1',
       test_file('hello_world.c')])

  def test_SYSCALL_DEBUG(self):
    self.set_setting('SYSCALL_DEBUG')
    self.do_runf('hello_world.c', 'syscall! fd_write: [1,')

  def test_LIBRARY_DEBUG(self):
    self.set_setting('LIBRARY_DEBUG')
    self.do_runf('hello_world.c', '[library call:_fd_write: 0x00000001 (1)')

  def test_SUPPORT_LONGJMP_executable(self):
    err = self.expect_fail([EMCC, test_file('core/test_longjmp.c'), '-sSUPPORT_LONGJMP=0'])
    self.assertContained('error: longjmp support was disabled (SUPPORT_LONGJMP=0), but it is required by the code (either set SUPPORT_LONGJMP=1, or remove uses of it in the project)', err)

  def test_SUPPORT_LONGJMP_object(self):
    # compile the object *with* support, but link without
    self.run_process([EMCC, test_file('core/test_longjmp.c'), '-c', '-sSUPPORT_LONGJMP', '-o', 'a.o'])
    err = self.expect_fail([EMCC, 'a.o', '-sSUPPORT_LONGJMP=0'])
    self.assertContained('error: longjmp support was disabled (SUPPORT_LONGJMP=0), but it is required by the code (either set SUPPORT_LONGJMP=1, or remove uses of it in the project)', err)

  def test_SUPPORT_LONGJMP_wasm(self):
    # Tests if -sSUPPORT_LONGJMP=wasm alone is enough to use Wasm SjLj, i.e., it
    # automatically sets DISABLE_EXCEPTION_THROWING to 1, which is 0 by default,
    # because Emscripten EH and Wasm SjLj cannot be used at the same time.
    self.run_process([EMCC, test_file('core/test_longjmp.c'), '-c', '-sSUPPORT_LONGJMP=wasm', '-o', 'a.o'])

  @parameterized({
    '': [[]],
    'trusted': [['-sTRUSTED_TYPES']]
  })
  def test_pthread_export_es6(self, args):
    self.run_process([EMCC, test_file('hello_world.c'), '-o', 'out.mjs', '-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'] + args)
    create_file('runner.mjs', '''
      import Hello from "./out.mjs";
      Hello();
    ''')
    output = self.run_js('runner.mjs')
    self.assertContained('hello, world!', output)

  def test_jslib_clobber_i(self):
    # Regression check for an issue we have where a library clobbering the global `i` variable could
    # prevent processing of further libraries.
    create_file('lib1.js', 'for (var i = 0; i < 100; i++) {}')
    create_file('lib2.js', '''
      addToLibrary({
        foo: () => {}
      });
      ''')
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=foo',
                      '--js-library=lib1.js',
                      '--js-library=lib2.js'])

  def test_jslib_bad_config(self):
    create_file('lib.js', '''
      addToLibrary({
       foo__sig: 'ii',
      });
      ''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library=lib.js'])
    self.assertContained("lib.js: Missing library element 'foo' for library config 'foo__sig'", err)

  def test_jslib_ifdef(self):
    create_file('lib.js', '''
      #ifdef ASSERTIONS
      var foo;
      #endif
      ''')
    proc = self.run_process([EMCC, test_file('hello_world.c'), '--js-library=lib.js'], stderr=PIPE)
    self.assertContained('lib.js: use of #ifdef in js library.  Use #if instead.', proc.stderr)

  def test_jslib_mangling(self):
    create_file('lib.js', '''
      addToLibrary({
        $__foo: () => 43,
      });
      ''')
    self.run_process([EMCC, test_file('hello_world.c'), '--js-library=lib.js', '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$__foo'])

  def test_jslib_exported_functions(self):
    create_file('lib.js', '''
      addToLibrary({
        $Foo: () => 43,
      });
      ''')
    self.run_process([EMCC, test_file('hello_world.c'), '--js-library=lib.js', '-sEXPORTED_FUNCTIONS=Foo,_main'])
    self.assertContained("Module['Foo'] = ", read_file('a.out.js'))

  def test_wasm2js_no_dylink(self):
    for arg in ('-sMAIN_MODULE', '-sSIDE_MODULE', '-sRELOCATABLE'):
      print(arg)
      err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sWASM=0', arg])
      self.assertContained('WASM2JS is not compatible with relocatable output', err)

  def test_wasm2js_standalone(self):
    self.do_run_in_out_file_test('hello_world.c', emcc_args=['-sSTANDALONE_WASM', '-sWASM=0'])

  def test_oformat(self):
    self.run_process([EMCC, test_file('hello_world.c'), '--oformat=wasm', '-o', 'out.foo'])
    self.assertTrue(building.is_wasm('out.foo'))
    self.clear()

    self.run_process([EMCC, test_file('hello_world.c'), '--oformat=html', '-o', 'out.foo'])
    self.assertFalse(building.is_wasm('out.foo'))
    self.assertContained('<html ', read_file('out.foo'))
    self.clear()

    self.run_process([EMCC, test_file('hello_world.c'), '--oformat=js', '-o', 'out.foo'])
    self.assertFalse(building.is_wasm('out.foo'))
    self.assertContained('new ExitStatus', read_file('out.foo'))
    self.clear()

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--oformat=foo'])
    self.assertContained("error: invalid output format: `foo` (must be one of ['object', 'wasm', 'js', 'mjs', 'html', 'bare']", err)

  # Tests that the old format of {{{ makeDynCall('sig') }}}(func, param1) works
  def test_old_makeDynCall_syntax(self):
    err = self.run_process([EMCC, test_file('test_old_dyncall_format.c'), '--js-library', test_file('library_test_old_dyncall_format.js')], stderr=PIPE).stderr
    self.assertContained('syntax for makeDynCall has changed', err)

  # Test that {{{ makeDynCall('sig', 'this.foo') }}} macro works, i.e. when 'this.' is referenced inside the macro block.
  # For this test verify the different build options that generate anonymous enclosing function scopes. (DYNCALLS and MEMORY64)
  @parameterized({
    'plain': [[]],
    'dyncalls': [['-sDYNCALLS']]
  })
  def test_this_in_dyncall(self, args):
    self.do_run_in_out_file_test('no_this_in_dyncall.c', emcc_args=['--js-library', test_file('no_this_in_dyncall.js')] + args)

  @requires_wasm64
  def test_this_in_dyncall_memory64(self):
    self.do_run_in_out_file_test('no_this_in_dyncall.c', emcc_args=['--js-library', test_file('no_this_in_dyncall.js'), '-sMEMORY64', '-Wno-experimental'])

  # Tests that dynCalls are produced in Closure-safe way in DYNCALLS mode when no actual dynCalls are used
  @parameterized({
    'plain': [[]],
    'asyncify': [['-sASYNCIFY']],
    'asyncify_bigint': [['-sASYNCIFY', '-sWASM_BIGINT']]})
  def test_closure_safe(self, args):
    self.run_process([EMCC, test_file('hello_world.c'), '--closure=1'] + args)

  def test_post_link(self):
    err = self.run_process([EMCC, test_file('hello_world.c'), '--oformat=bare', '-o', 'bare.wasm'], stderr=PIPE).stderr
    self.assertContained('--oformat=bare/--post-link are experimental and subject to change', err)
    err = self.run_process([EMCC, '--post-link', 'bare.wasm'], stderr=PIPE).stderr
    self.assertContained('--oformat=bare/--post-link are experimental and subject to change', err)
    err = self.assertContained('hello, world!', self.run_js('a.out.js'))

  def compile_with_wasi_sdk(self, filename, output):
    sysroot = os.environ.get('EMTEST_WASI_SYSROOT')
    if not sysroot:
      self.skipTest('EMTEST_WASI_SYSROOT not found in environment')
    sysroot = os.path.expanduser(sysroot)
    self.run_process([CLANG_CC, '--sysroot=' + sysroot, '--target=wasm32-wasi', filename, '-o', output])

  def test_run_wasi_sdk_output(self):
    self.compile_with_wasi_sdk(test_file('hello_world.c'), 'hello.wasm')
    self.run_process([EMCC, '--post-link', '-sPURE_WASI', 'hello.wasm'])
    self.assertContained('hello, world!', self.run_js('a.out.js'))

  # Test that Closure prints out clear readable error messages when there are errors.
  def test_closure_errors(self):
    err = self.expect_fail([EMCC, test_file('closure_error.c'), '-O2', '--closure=1'])
    lines = err.split('\n')

    def find_substr_index(s):
      for i, line in enumerate(lines):
        if s in line:
          return i
      return -1

    idx1 = find_substr_index('[JSC_UNDEFINED_VARIABLE] variable thisVarDoesNotExist is undeclared')
    idx2 = find_substr_index('[JSC_UNDEFINED_VARIABLE] variable thisVarDoesNotExistEither is undeclared')
    self.assertNotEqual(idx1, -1)
    self.assertNotEqual(idx2, -1)
    # The errors must be present on distinct lines.
    self.assertNotEqual(idx1, idx2)

  # Make sure that --cpuprofiler compiles with --closure 1
  def test_cpuprofiler_closure(self):
    # TODO: Enable '-Werror=closure' in the following, but that has currently regressed.
    self.run_process([EMCC, test_file('hello_world.c'), '-O2', '--closure=1', '--cpuprofiler'])

  # Make sure that --memoryprofiler compiles with --closure 1
  def test_memoryprofiler_closure(self):
    # TODO: Enable '-Werror=closure' in the following, but that has currently regressed.
    self.run_process([EMCC, test_file('hello_world.c'), '-O2', '--closure=1', '--memoryprofiler'])

  # Make sure that --threadprofiler compiles with --closure 1
  def test_threadprofiler_closure(self):
    # TODO: Enable '-Werror=closure' in the following, but that has currently regressed.
    self.run_process([EMCC, test_file('hello_world.c'), '-O2', '-pthread', '--closure=1', '--threadprofiler', '-sASSERTIONS'])

  @node_pthreads
  def test_threadprofiler(self):
    self.run_process([EMCC, test_file('test_threadprofiler.cpp'), '-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME', '--threadprofiler', '-sASSERTIONS'])
    output = self.run_js('a.out.js')
    self.assertRegex(output, r'Thread "Browser main thread" \(0x.*\) now: running.')
    self.assertRegex(output, r'Thread "Application main thread" \(0x.*\) now: waiting for a futex.')
    self.assertRegex(output, r'Thread "test worker" \(0x.*\) now: sleeping.')

  def test_syslog(self):
    self.do_other_test('test_syslog.c')

  def test_syscall_stubs(self):
    self.do_other_test('test_syscall_stubs.c')

  @parameterized({
    '': (False, False),
    'custom': (True, False),
    'jspi': (False, True),
  })
  def test_split_module(self, customLoader, jspi):
    self.set_setting('SPLIT_MODULE')
    self.emcc_args += ['-g', '-Wno-experimental']
    self.emcc_args += ['--post-js', test_file('other/test_split_module.post.js')]
    if customLoader:
      self.emcc_args += ['--pre-js', test_file('other/test_load_split_module.pre.js')]
    if jspi:
      self.require_jspi()
      self.emcc_args += ['-g', '-sJSPI_EXPORTS=say_hello']
    self.emcc_args += ['-sEXPORTED_FUNCTIONS=_malloc,_free']
    output = self.do_other_test('test_split_module.c')
    if jspi:
      # TODO remove this when https://chromium-review.googlesource.com/c/v8/v8/+/4159854
      # lands.
      # d8 doesn't support writing a file yet, so extract it from the output.
      create_file('profile.data', bytearray(json.loads(output[output.find('['):output.find(']') + 1])), True)
    self.assertExists('test_split_module.wasm')
    self.assertExists('test_split_module.wasm.orig')
    self.assertExists('profile.data')

    wasm_split = os.path.join(building.get_binaryen_bin(), 'wasm-split')
    wasm_split_run = [wasm_split, '-g', '--enable-mutable-globals', '--export-prefix=%', 'test_split_module.wasm.orig', '-o1', 'primary.wasm', '-o2', 'secondary.wasm', '--profile=profile.data']
    if jspi:
      wasm_split_run += ['--jspi', '--enable-reference-types']
    self.run_process(wasm_split_run)

    os.remove('test_split_module.wasm')
    os.rename('primary.wasm', 'test_split_module.wasm')
    os.rename('secondary.wasm', 'test_split_module.deferred.wasm')
    result = self.run_js('test_split_module.js')
    self.assertNotIn('profile', result)
    self.assertContainedIf('Custom handler for loading split module.', result, condition=customLoader)
    self.assertIn('Hello! answer: 42', result)
    if jspi:
      self.assertIn('result is promise', result)

  def test_split_main_module(self):
    # Set and reasonably large initial table size to avoid test fragility.
    # The actual number of slots needed is closer to 18 but we don't want
    # this test to fail every time that changes.
    initialTableSize = 100

    side_src = test_file('other/lib_hello.c')
    post_js = test_file('other/test_split_module.post.js')

    self.run_process([EMCC, side_src, '-sSIDE_MODULE', '-g', '-o', 'libhello.wasm'])

    self.emcc_args += ['-g']
    self.emcc_args += ['-sMAIN_MODULE=2']
    self.emcc_args += ['-sEXPORTED_FUNCTIONS=_printf,_malloc,_free']
    self.emcc_args += ['-sSPLIT_MODULE', '-Wno-experimental']
    self.emcc_args += ['--embed-file', 'libhello.wasm']
    self.emcc_args += ['--post-js', post_js]
    self.emcc_args += [f'-sINITIAL_TABLE={initialTableSize}']

    self.do_other_test('test_split_main_module.c')

    self.assertExists('test_split_main_module.wasm')
    self.assertExists('test_split_main_module.wasm.orig')
    self.assertExists('profile.data')

    wasm_split = os.path.join(building.get_binaryen_bin(), 'wasm-split')
    self.run_process([wasm_split, '-g',
                      'test_split_main_module.wasm.orig',
                      '--export-prefix=%',
                      f'--initial-table={initialTableSize}',
                      '--profile=profile.data',
                      '-o1', 'primary.wasm',
                      '-o2', 'secondary.wasm',
                      '--enable-mutable-globals'])

    os.remove('test_split_main_module.wasm')
    os.rename('primary.wasm', 'test_split_main_module.wasm')
    os.rename('secondary.wasm', 'test_split_main_module.deferred.wasm')
    result = self.run_js('test_split_main_module.js')
    self.assertNotIn('profile', result)
    self.assertIn('Hello from main!', result)
    self.assertIn('Hello from lib!', result)

  @crossplatform
  def test_gen_struct_info(self):
    # This test will start failing whenever the struct info changes (e.g. offset or defines
    # change).  However it's easy to rebaseline with --rebaseline.
    self.run_process([PYTHON, path_from_root('tools/maint/gen_struct_info.py'), '-o', 'out.json'])
    self.assertFileContents(path_from_root('src/struct_info_generated.json'), read_file('out.json'))

    # Same again for wasm64
    node_version = shared.get_node_version(self.get_nodejs())
    if node_version and node_version >= (14, 0, 0):
      self.run_process([PYTHON, path_from_root('tools/maint/gen_struct_info.py'), '--wasm64', '-o', 'out.json'])
      self.assertFileContents(path_from_root('src/struct_info_generated_wasm64.json'), read_file('out.json'))

  @crossplatform
  def test_gen_sig_info(self):
    # This tests is fragile and will need updating any time a JS library
    # function is added or its signature changed.  However it's easy to
    # rebaseline with --rebaseline.
    self.run_process([PYTHON, path_from_root('tools/maint/gen_sig_info.py'), '-o', 'out.js'])
    self.assertFileContents(path_from_root('src/library_sigs.js'), read_file('out.js'))

  def test_gen_struct_info_env(self):
    # gen_struct_info.py builds C code in a very specific and low level way.  We don't want
    # EMCC_CFLAGS (or any of the other environment variables that might effect compilation or
    # linking) to effect the internal building and running of this code.
    # For example -O2 causes printf -> iprintf which will fail with undefined symbol iprintf.
    with env_modify({'EMCC_CFLAGS': '-O2 BAD_ARG', 'EMCC_FORCE_STDLIBS': '1', 'EMCC_ONLY_FORCED_STDLIBS': '1'}):
      self.run_process([PYTHON, path_from_root('tools/maint/gen_struct_info.py'), '-o', 'out.json'])

  def test_relocatable_limited_exports(self):
    # Building with RELOCATABLE should *not* automatically export all sybmols.
    self.run_process([EMCC, test_file('hello_world.c'), '-sRELOCATABLE', '-o', 'out.wasm'])

    # Building with RELOCATABLE + LINKABLE should include and export all of the standard library
    self.run_process([EMCC, test_file('hello_world.c'), '-sRELOCATABLE', '-sLINKABLE', '-o', 'out_linkable.wasm'])

    exports = self.parse_wasm('out.wasm')[1]
    exports_linkable = self.parse_wasm('out_linkable.wasm')[1]

    self.assertLess(len(exports), 20)
    self.assertGreater(len(exports_linkable), 1000)
    self.assertIn('sendmsg', exports_linkable)
    self.assertNotIn('sendmsg', exports)

  @requires_v8
  def test_shell_Oz(self):
    # regression test for -Oz working on non-web, non-node environments that
    # lack TextDecoder
    self.do_run_in_out_file_test('hello_world.c', emcc_args=['-Oz'])

  def test_runtime_keepalive(self):
    self.uses_es6 = True
    # Depends on Module['onExit']
    self.set_setting('EXIT_RUNTIME')
    self.do_other_test('test_runtime_keepalive.cpp')

  @crossplatform
  def test_em_js_invalid(self):
    # Test that invalid EM_JS in side modules since is detected at build time.
    err = self.expect_fail([EMCC, '-sSIDE_MODULE', test_file('other/test_em_js_invalid.c')])
    self.assertContained("SyntaxError: Unexpected token '*'", err)
    self.assertContained('emcc: error: EM_JS function validation failed', err)

  @crossplatform
  def test_em_js_side_module(self):
    self.build(test_file('other/test_em_js_side.c'), js_outfile=False, emcc_args=['-sSIDE_MODULE'], output_basename='side')
    self.do_other_test('test_em_js_main.c', emcc_args=['-sMAIN_MODULE=2', 'side.wasm'])

  def test_em_js_main_module(self):
    self.set_setting('MAIN_MODULE', 2)
    self.set_setting('EXPORTED_FUNCTIONS', '_main,_malloc')
    self.do_runf('core/test_em_js.cpp')

  def test_em_js_main_module_address(self):
    # This works under static linking but is known to fail with dynamic linking
    # See https://github.com/emscripten-core/emscripten/issues/18494
    self.do_runf('other/test_em_js_main_module_address.c')

    self.set_setting('MAIN_MODULE', 2)
    expected = 'Aborted(Assertion failed: Missing signature argument to addFunction: function foo() { err("hello"); })'
    self.do_runf('other/test_em_js_main_module_address.c', expected, assert_returncode=NON_ZERO)

  def test_em_js_external_usage(self):
    # Verify that EM_JS functions can be called from other source files, even in the case
    # when they are not used within the defining file.
    create_file('em_js.c', r'''
      #include <emscripten/em_js.h>

      EM_JS(void, js_func, (), {
        out('js_func called');
      });

      // js_func is unused within this file
    ''')
    create_file('main.c', '''
      #include <stdio.h>

      void js_func();
      int main() {
        js_func();
      }
    ''')
    self.run_process([EMCC, 'em_js.c', '-c'])
    self.do_runf('main.c', 'js_func called\n', emcc_args=['em_js.o'])

  # On Windows maximum command line length is 32767 characters. Create such a long build line by linking together
  # several .o files to test that emcc internally uses response files properly when calling llvm-nm and wasm-ld.
  @is_slow_test
  def test_windows_long_link_response_file(self):
    decls = ''
    calls = ''
    files = []

    def create_o(name, i):
      nonlocal decls, calls, files
      f = name + '.c'
      create_file(f, 'int %s() { return %d; }' % (name, i))
      files += [f]
      decls += 'int %s();' % name
      calls += 'value += %s();' % name

    count = 1000
    for i in range(count):
      name = 'a' + str(i)
      for _ in range(5):
        name += name
      create_o(name, i)

    create_file('main.c', '#include<stdio.h>\n%s int main() { int value = 0; %s printf("%%d\\n", value); }' % (decls, calls))

    assert sum(len(f) for f in files) > 32767

    self.run_process(building.get_command_with_possible_response_file([EMCC, 'main.c'] + files))
    self.assertContained(str(count * (count - 1) // 2), self.run_js('a.out.js'))

  # Tests that the filename suffix of the response files can be used to detect which encoding the file is.
  @crossplatform
  def test_response_file_encoding(self):
    create_file('äö.c', 'int main(){}')

    open('a.rsp', 'w', encoding='utf-8').write('äö.c') # Write a response file with unicode contents ...
    self.run_process([EMCC, '@a.rsp']) # ... and test that in the absence of a file suffix, it is autodetected to utf-8.

    open('a.rsp.cp437', 'w', encoding='cp437').write('äö.c') # Write a response file with Windows CP-437 encoding ...
    self.run_process([EMCC, '@a.rsp.cp437']) # ... and test that with the explicit suffix present, it is properly decoded

    import locale
    preferred_encoding = locale.getpreferredencoding(do_setlocale=False)
    print('Python locale preferredencoding: ' + preferred_encoding)
    open('a.rsp', 'w', encoding=preferred_encoding).write('äö.c') # Write a response file using Python preferred encoding
    self.run_process([EMCC, '@a.rsp']) # ... and test that it is properly autodetected.

  def test_output_name_collision(self):
    # Ensure that the secondary filenames never collide with the primary output filename
    # In this case we explicitly ask for JS to be created in a file with the `.wasm` suffix.
    # Even though this doesn't make much sense the `--oformat` flag is designed to override
    # any implicit type that we might infer from the output name.
    self.run_process([EMCC, '-o', 'hello.wasm', '--oformat=js', test_file('hello_world.c')])
    self.assertExists('hello.wasm')
    self.assertExists('hello_.wasm')
    self.assertContained('hello, world!', self.run_js('hello.wasm'))

  def test_main_module_no_undefined(self):
    # Test that ERROR_ON_UNDEFINED_SYMBOLS works with MAIN_MODULE.
    self.do_runf('hello_world.c', emcc_args=['-sMAIN_MODULE', '-sERROR_ON_UNDEFINED_SYMBOLS'])

  def test_reverse_deps_allow_undefined(self):
    # Check that reverse deps are still included even when -sERROR_ON_UNDEFINED_SYMBOLS=0.
    create_file('test.c', '''
    #include <assert.h>
    #include <stdio.h>
    #include <netdb.h>

    int main() {
      // Reference in getaddrinfo which has reverse deps on malloc and htons
      // We expect these to be exported even when -sERROR_ON_UNDEFINED_SYMBOLS=0.
      printf("%p\\n", &getaddrinfo);
      return 0;
    }
    ''')
    self.do_runf('test.c', emcc_args=['-sERROR_ON_UNDEFINED_SYMBOLS=0'])

  @parameterized({
    'relocatable': ('-sRELOCATABLE',),
    'linkable': ('-sLINKABLE',),
    'main_module': ('-sMAIN_MODULE',),
  })
  def test_check_undefined(self, flag):
    # positive case: no undefined symbols
    self.run_process([EMCC, flag, '-sERROR_ON_UNDEFINED_SYMBOLS', test_file('hello_world.c')])
    self.run_js('a.out.js')

    # negative case: foo is undefined in test_check_undefined.c
    err = self.expect_fail([EMCC, flag, '-sERROR_ON_UNDEFINED_SYMBOLS', test_file('other/test_check_undefined.c')])
    self.assertContained('undefined symbol: foo', err)

  @also_with_wasm64
  @parameterized({
    'asyncify': (['-sASYNCIFY'],),
  })
  def test_missing_symbols_at_runtime(self, args):
    # We deliberately pick a symbol there that takes a pointer as an argument.
    # We had a regression where the pointer-handling wrapper function could
    # not be created because the "missing functions" stubs don't take any
    # arguments.
    create_file('test.c', '''
    #include <GL/gl.h>

    int main() {
      glGetTexLevelParameteriv(0, 0, 0, 0);
    }
    ''')

    expected = 'Aborted(missing function: glGetTexLevelParameteriv)'
    self.do_runf('test.c', expected,
                 emcc_args=['-sWARN_ON_UNDEFINED_SYMBOLS=0', '-sAUTO_JS_LIBRARIES=0'] + args,
                 assert_returncode=NON_ZERO)

  @with_env_modify({'EMMAKEN_NO_SDK': '1'})
  def test_EMMAKEN_NO_SDK(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c')])
    self.assertContained('emcc: error: EMMAKEN_NO_SDK is no longer supported', err)

  @parameterized({
    'default': ('', '2147483648'),
    '1GB': ('-sMAXIMUM_MEMORY=1GB', '1073741824'),
    # for 4GB we return 1 wasm page less than 4GB, as 4GB cannot fit in a 32bit
    # integer
    '4GB': ('-sMAXIMUM_MEMORY=4GB', '4294901760'),
  })
  def test_emscripten_get_heap_max(self, arg, expected):
    create_file('get.c', r'''
      #include <emscripten/heap.h>
      #include <stdio.h>
      int main() {
        printf("max: |%zu|\n", emscripten_get_heap_max());
      }
    ''')
    self.do_runf('get.c', f'max: |{expected}|', emcc_args=['-sALLOW_MEMORY_GROWTH', arg])

  def test_auto_ptr_cxx17(self):
    # Test that its still possible to use auto_ptr, even in C++17
    self.do_other_test('test_auto_ptr_cxx17.cpp', emcc_args=[
      '-std=c++17',
      '-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR',
      '-Wno-deprecated-declarations'])

  @crossplatform
  def test_special_chars_in_arguments(self):
    # We had some regressions where the windows `.bat` files that run the compiler
    # driver were failing to accept certain special characters such as `(`, `)` and `!`.
    # See https://github.com/emscripten-core/emscripten/issues/14063
    create_file('test(file).c', 'int main() { return 0; }')
    create_file('test!.c', 'int main() { return 0; }')
    self.run_process([EMCC, 'test(file).c'])
    self.run_process([EMCC, 'test!.c'])

  @no_windows('relies on a shell script')
  def test_report_subprocess_signals(self):
    # Test that when subprocess is killed by signal we report the signal name
    create_file('die.sh', '''\
#!/bin/sh
kill -9 $$
    ''')
    make_executable('die.sh')
    with env_modify({'EM_COMPILER_WRAPPER': './die.sh'}):
      err = self.expect_fail([EMCC, test_file('hello_world.c')])
      self.assertContained('failed (received SIGKILL (-9))', err)

  def test_concepts(self):
    self.do_runf('other/test_concepts.cpp', '', emcc_args=['-std=c++20'])

  def test_std_cmp(self):
    self.do_runf('other/test_std_cmp.cpp', '', emcc_args=['-std=c++20'])

  def test_link_only_setting_warning(self):
    err = self.run_process([EMCC, '-sALLOW_MEMORY_GROWTH', '-c', test_file('hello_world.c')], stderr=PIPE).stderr
    self.assertContained("warning: linker setting ignored during compilation: 'ALLOW_MEMORY_GROWTH' [-Wunused-command-line-argument]", err)

  def test_link_only_flag_warning(self):
    err = self.run_process([EMCC, '--embed-file', 'file', '-c', test_file('hello_world.c')], stderr=PIPE).stderr
    self.assertContained("warning: linker flag ignored during compilation: '--embed-file' [-Wunused-command-line-argument]", err)

  def test_no_deprecated(self):
    # Test that -Wno-deprecated is passed on to clang driver
    create_file('test.c', '''\
        __attribute__((deprecated)) int foo();
        int main() { return foo(); }
    ''')
    err = self.expect_fail([EMCC, '-c', '-Werror', 'test.c'])
    self.assertContained("error: 'foo' is deprecated", err)
    self.run_process([EMCC, '-c', '-Werror', '-Wno-deprecated', 'test.c'])

  def test_bad_export_name(self):
    err = self.expect_fail([EMCC, '-sEXPORT_NAME=foo bar', test_file('hello_world.c')])
    self.assertContained('error: EXPORT_NAME is not a valid JS identifier: `foo bar`', err)

  def test_offset_convertor_plus_wasm2js(self):
    err = self.expect_fail([EMCC, '-sUSE_OFFSET_CONVERTER', '-sWASM=0', test_file('hello_world.c')])
    self.assertContained('wasm2js is not compatible with USE_OFFSET_CONVERTER', err)

  def test_standard_library_mapping(self):
    # Test the `-l` flags on the command line get mapped the correct libraries variant
    libs = ['-lc', '-lbulkmemory', '-lcompiler_rt', '-lmalloc']
    err = self.run_process([EMCC, test_file('hello_world.c'), '-pthread', '-nodefaultlibs', '-v'] + libs, stderr=PIPE).stderr

    # Check that the linker was run with `-mt` variants because `-pthread` was passed.
    self.assertContained(' -lc-mt-debug ', err)
    self.assertContained(' -ldlmalloc-mt ', err)
    self.assertContained(' -lcompiler_rt-mt ', err)

  def test_explicit_gl_linking(self):
    # Test that libGL can be linked explicitly via `-lGL` rather than implicitly.
    # Here we use NO_AUTO_NATIVE_LIBRARIES to disable the implicitly linking that normally
    # includes the native GL library.
    self.run_process([EMCC, test_file('other/test_explicit_gl_linking.c'), '-sNO_AUTO_NATIVE_LIBRARIES', '-lGL', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  def test_no_main_with_PROXY_TO_PTHREAD(self):
    create_file('lib.c', r'''
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE
void foo() {}
''')
    err = self.expect_fail([EMCC, 'lib.c', '-pthread', '-sPROXY_TO_PTHREAD'])
    self.assertContained('crt1_proxy_main.o: undefined symbol: main', err)

  def test_archive_bad_extension(self):
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    self.run_process([EMAR, 'crs', 'libtest.bc', 'hello_world.o'])
    err = self.expect_fail([EMCC, 'libtest.bc'])
    self.assertContained('libtest.bc:1:2: error: expected integer', err)

  def test_split_dwarf_implicit_compile(self):
    # Verify that the dwo file is generated in the current working directory, even when implicitly
    # compiling (compile+link).
    self.run_process([EMCC, test_file('hello_world.c'), '-g', '-gsplit-dwarf'])
    self.assertExists('hello_world.dwo')

  @parameterized({
    '': [[]],
    'strict': [['-sSTRICT']],
    'no_allow': [['-sALLOW_UNIMPLEMENTED_SYSCALLS=0']],
  })
  def test_unimplemented_syscalls(self, args):
    create_file('main.c', '''
    #include <assert.h>
    #include <errno.h>
    #include <sys/mman.h>

    int main() {
      assert(mincore(0, 0, 0) == -1);
      assert(errno == ENOSYS);
      return 0;
    }
    ''')
    cmd = [EMCC, 'main.c', '-sASSERTIONS'] + args
    if args:
      err = self.expect_fail(cmd)
      self.assertContained('libc-debug.a(mincore.o): undefined symbol: __syscall_mincore', err)
    else:
      self.run_process(cmd)
      err = self.run_js('a.out.js')
      self.assertContained('warning: unsupported syscall: __syscall_mincore', err)

      # Setting ASSERTIONS=0 should avoid the runtime warning
      self.run_process(cmd + ['-sASSERTIONS=0'])
      err = self.run_js('a.out.js')
      self.assertNotContained('warning: unsupported syscall', err)

  @also_with_wasm64
  def test_unimplemented_syscalls_dlopen(self):
    cmd = [EMCC, test_file('other/test_dlopen_blocking.c')] + self.get_emcc_args()
    self.run_process(cmd)
    err = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('dlopen failed: dynamic linking not enabled', err)

    # If we build the same thing with ALLOW_UNIMPLEMENTED_SYSCALLS=0 we
    # expect a link-time failure rather than a runtime one.
    cmd += ['-sALLOW_UNIMPLEMENTED_SYSCALLS=0']
    err = self.expect_fail(cmd)
    self.assertContained('undefined symbol: dlopen', err)

  def test_unimplemented_syscalls_dladdr(self):
    create_file('main.c', '''
    #include <assert.h>
    #include <dlfcn.h>

    int main() {
      Dl_info info;
      int rtn = dladdr(&main, &info);
      assert(rtn == 0);
      return 0;
    }
    ''')

    self.do_runf('main.c')
    self.do_runf('main.c', emcc_args=['-sMAIN_MODULE=2'])

  @requires_v8
  def test_missing_shell_support(self):
    # By default shell support is not included
    self.run_process([EMCC, test_file('hello_world.c')])
    err = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('shell environment detected but not enabled at build time.', err)

  def test_removed_runtime_function(self):
    create_file('post.js', 'alignMemory(100, 4);')
    self.run_process([EMCC, test_file('hello_world.c'), '--post-js=post.js'])
    err = self.run_js('a.out.js', assert_returncode=NON_ZERO)
    self.assertContained('`alignMemory` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line', err)

  # Tests that it is possible to hook into/override a symbol defined in a system library.
  def test_override_system_js_lib_symbol(self):
    # This test verifies it is possible to override a symbol from WebGL library.

    # When WebGL is implicitly linked in, the implicit linking should happen before any user
    # --js-libraries, so that they can adjust the behavior afterwards.
    self.do_run_in_out_file_test('test_override_system_js_lib_symbol.c', emcc_args=['--js-library', test_file('test_override_system_js_lib_symbol.js'), '-sMAX_WEBGL_VERSION=2'])

    # When WebGL is explicitly linked to in strict mode, the linking order on command line should enable overriding.
    self.emcc_args += ['-sAUTO_JS_LIBRARIES=0', '-sMAX_WEBGL_VERSION=2', '-lwebgl.js', '--js-library', test_file('test_override_system_js_lib_symbol.js')]
    self.do_run_in_out_file_test('test_override_system_js_lib_symbol.c')

  @node_pthreads
  def test_pthread_lsan_no_leak(self):
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.emcc_args += ['-gsource-map', '-pthread']
    self.do_run_in_out_file_test('pthread/test_pthread_lsan_no_leak.cpp', emcc_args=['-fsanitize=leak'])
    self.do_run_in_out_file_test('pthread/test_pthread_lsan_no_leak.cpp', emcc_args=['-fsanitize=address'])

  @node_pthreads
  def test_pthread_lsan_leak(self):
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.add_pre_run("Module['LSAN_OPTIONS'] = 'exitcode=0'")
    self.emcc_args += ['-gsource-map', '-pthread']
    expected = [
      'Direct leak of 3432 byte(s) in 1 object(s) allocated from',
      'test_pthread_lsan_leak.cpp:18:17',
      'Direct leak of 2048 byte(s) in 1 object(s) allocated from',
      'test_pthread_lsan_leak.cpp:36:10',
      'Direct leak of 1337 byte(s) in 1 object(s) allocated from',
      'test_pthread_lsan_leak.cpp:30:16',
      'Direct leak of 1234 byte(s) in 1 object(s) allocated from',
      'test_pthread_lsan_leak.cpp:20:13',
      'Direct leak of 420 byte(s) in 1 object(s) allocated from',
      'test_pthread_lsan_leak.cpp:31:13',
      'Direct leak of 42 byte(s) in 1 object(s) allocated from',
      'test_pthread_lsan_leak.cpp:13:21',
      'test_pthread_lsan_leak.cpp:35:3',
      '8513 byte(s) leaked in 6 allocation(s).',
    ]
    self.do_runf('pthread/test_pthread_lsan_leak.cpp', expected, assert_all=True, emcc_args=['-fsanitize=leak'])
    self.do_runf('pthread/test_pthread_lsan_leak.cpp', expected, assert_all=True, emcc_args=['-fsanitize=address'])

  @node_pthreads
  def test_pthread_js_exception(self):
    # Ensure that JS exceptions propagate back to the main main thread and cause node
    # to exit with an error.
    self.emcc_args.append('-pthread')
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.build(test_file('other/test_pthread_js_exception.c'))
    err = self.run_js('test_pthread_js_exception.js', assert_returncode=NON_ZERO)
    self.assertContained('missing is not defined', err)

  def test_config_closure_compiler(self):
    self.run_process([EMCC, test_file('hello_world.c'), '--closure=1'])
    with env_modify({'EM_CLOSURE_COMPILER': sys.executable}):
      err = self.expect_fail([EMCC, test_file('hello_world.c'), '--closure=1'])
    self.assertContained('closure compiler', err)
    self.assertContained(sys.executable, err)
    self.assertContained('not execute properly!', err)

  def test_node_unhandled_rejection(self):
    create_file('pre.js', '''
    async function foo() {
      abort("this error will become an unhandled rejection");
    }
    async function doReject() {
      return foo();
    }
    ''')
    create_file('main.c', '''
    #include <emscripten.h>

    int main() {
      EM_ASM(setTimeout(doReject, 0));
      emscripten_exit_with_live_runtime();
      __builtin_trap();
    }
    ''')

    # With NODEJS_CATCH_REJECTION we expect the unhandled rejection to cause a non-zero
    # exit code and log the stack trace correctly.
    self.build('main.c', emcc_args=['--pre-js=pre.js', '-sNODEJS_CATCH_REJECTION'])
    output = self.run_js('main.js', assert_returncode=NON_ZERO)
    self.assertContained('unhandledRejection', read_file('main.js'))
    self.assertContained('RuntimeError: Aborted(this error will become an unhandled rejection)', output)
    self.assertContained('at foo (', output)

    # Without NODEJS_CATCH_REJECTION we expect node to log the unhandled rejection
    # but return 0.
    self.node_args = [a for a in self.node_args if '--unhandled-rejections' not in a]
    self.build('main.c', emcc_args=['--pre-js=pre.js', '-sNODEJS_CATCH_REJECTION=0'])
    self.assertNotContained('unhandledRejection', read_file('main.js'))

    if shared.get_node_version(self.get_nodejs())[0] >= 15:
      self.skipTest('old behaviour of node JS cannot be tested on node v15 or above')

    output = self.run_js('main.js')
    self.assertContained('RuntimeError: Aborted(this error will become an unhandled rejection)', output)
    self.assertContained('at foo (', output)

  def test_default_pthread_stack_size(self):
    self.do_runf('other/test_default_pthread_stack_size.c')

    # Same again with pthreads enabled
    self.setup_node_pthreads()
    self.do_other_test('test_default_pthread_stack_size.c')

    # Same again but with a custom stack size
    self.emcc_args += ['-DEXPECTED_STACK_SIZE=1024', '-sDEFAULT_PTHREAD_STACK_SIZE=1024']
    self.do_other_test('test_default_pthread_stack_size.c')

    # Same again but with a --proxy-to-worker
    self.emcc_args += ['--proxy-to-worker']
    self.do_other_test('test_default_pthread_stack_size.c')

  def test_emscripten_set_immediate(self):
    self.do_runf('emscripten_set_immediate.c')

  def test_emscripten_set_immediate_loop(self):
    self.do_runf('emscripten_set_immediate_loop.c')

  @parameterized({
    '': ([],),
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'],),
  })
  def test_emscripten_main_loop(self, args):
    self.do_runf('test_emscripten_main_loop.c', emcc_args=args)

  def test_emscripten_main_loop_and_blocker(self):
    self.do_runf('test_emscripten_main_loop_and_blocker.c')

  def test_emscripten_main_loop_settimeout(self):
    self.do_runf('test_emscripten_main_loop_settimeout.c')

  def test_emscripten_main_loop_setimmediate(self):
    self.do_runf('test_emscripten_main_loop_setimmediate.c')

  @node_pthreads
  def test_pthread_trap(self):
    # TODO(https://github.com/emscripten-core/emscripten/issues/15161):
    # Make this work without PROXY_TO_PTHREAD
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.emcc_args += ['--profiling-funcs', '-pthread']
    output = self.do_runf('pthread/test_pthread_trap.c', assert_returncode=NON_ZERO)
    self.assertContained('sent an error!', output)
    self.assertContained('at (test_pthread_trap.wasm.)?thread_main', output, regex=True)

  @node_pthreads
  def test_pthread_kill(self):
    self.do_run_in_out_file_test('pthread/test_pthread_kill.c')

  @node_pthreads
  def test_emscripten_set_interval(self):
    self.do_runf('emscripten_set_interval.c', args=['-pthread', '-sPROXY_TO_PTHREAD'])

  # Test emscripten_console_log(), emscripten_console_warn() and emscripten_console_error()
  def test_emscripten_console_log(self):
    self.do_run_in_out_file_test('emscripten_console_log.c', emcc_args=['--pre-js', test_file('emscripten_console_log_pre.js')])

  # Tests emscripten_unwind_to_js_event_loop() behavior
  def test_emscripten_unwind_to_js_event_loop(self, *args):
    self.do_runf('test_emscripten_unwind_to_js_event_loop.c')

  @node_pthreads
  def test_emscripten_set_timeout(self):
    self.do_runf('emscripten_set_timeout.c', args=['-pthread', '-sPROXY_TO_PTHREAD'])

  @node_pthreads
  def test_emscripten_set_timeout_loop(self):
    self.do_runf('emscripten_set_timeout_loop.c', args=['-pthread', '-sPROXY_TO_PTHREAD'])

  # Verify that we are able to successfully compile a script when the Windows 7
  # and Python workaround env. vars are enabled.
  # See https://bugs.python.org/issue34780
  @with_env_modify({'EM_WORKAROUND_PYTHON_BUG_34780': '1',
                    'EM_WORKAROUND_WIN7_BAD_ERRORLEVEL_BUG': '1'})
  def test_windows_batch_script_workaround(self):
    self.run_process([EMCC, test_file('hello_world.c')])
    self.assertExists('a.out.js')

  @node_pthreads
  def test_pthread_out_err(self):
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.emcc_args.append('-pthread')
    self.do_other_test('test_pthread_out_err.c')

  @node_pthreads
  def test_pthread_icu(self):
    self.set_setting('USE_ICU')
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.emcc_args.append('-pthread')
    self.do_other_test('test_pthread_icu.cpp')

  @node_pthreads
  def test_pthread_set_main_loop(self):
    self.do_other_test('test_pthread_set_main_loop.c')

  # unistd tests

  def test_unistd_confstr(self):
    self.do_run_in_out_file_test('unistd/confstr.c')

  def test_unistd_ttyname(self):
    self.do_runf('unistd/ttyname.c', 'success')

  def test_unistd_pathconf(self):
    self.do_run_in_out_file_test('unistd/pathconf.c')

  def test_unistd_swab(self):
    self.do_run_in_out_file_test('unistd/swab.c')

  def test_unistd_isatty(self):
    self.do_runf('unistd/isatty.c', 'success')

  def test_unistd_login(self):
    self.do_run_in_out_file_test('unistd/login.c')

  def test_unistd_sleep(self):
    self.do_run_in_out_file_test('unistd/sleep.c')

  @also_with_wasmfs
  def test_unistd_fstatfs(self):
    self.do_run_in_out_file_test('unistd/fstatfs.c')

  @no_windows("test is Linux-specific")
  @no_mac("test is Linux-specific")
  @requires_node
  def test_unistd_close_noderawfs(self):
    self.set_setting('NODERAWFS')
    create_file('pre.js', f'''
const {{ execSync }} = require('child_process');

const cmd = 'find /proc/' + process.pid + '/fd -lname "{self.get_dir()}*" -printf "%l\\\\n" || true';
let openFilesPre;

Module.preRun = () => {{
  openFilesPre = execSync(cmd, {{ stdio: ['ignore', 'pipe', 'ignore'] }}).toString();
}}
Module.postRun = () => {{
  const openFilesPost = execSync(cmd, {{ stdio: ['ignore', 'pipe', 'ignore'] }}).toString();
  assert(openFilesPre == openFilesPost, 'File descriptors should not leak. \\n' +
    'Pre: \\n' + openFilesPre +
    'Post: \\n' + openFilesPost);
}}
''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_run_in_out_file_test('unistd/close.c')

  # WASMFS tests

  # TODO: This test will only work with the new file system.
  # Addresses this issue: https://github.com/emscripten-core/emscripten/issues/4017
  # The new file system also correctly identifies errors that the JS file system missed.
  def test_wasmfs_dup(self):
    self.set_setting('WASMFS')
    self.do_run_in_out_file_test('wasmfs/wasmfs_dup.c')

  @also_with_wasmfs
  def test_unistd_open(self):
    self.do_run_in_out_file_test('wasmfs/wasmfs_open.c')

  @also_with_wasmfs
  def test_unistd_open_append(self):
    self.do_run_in_out_file_test('wasmfs/wasmfs_open_append.c')

  @also_with_wasmfs
  def test_unistd_stat(self):
    self.do_runf('wasmfs/wasmfs_stat.c')

  @also_with_wasmfs
  def test_unistd_create(self):
    self.set_setting('WASMFS')
    self.do_run_in_out_file_test('wasmfs/wasmfs_create.c')

  def test_unistd_fdatasync(self):
    # TODO: Remove this test in favor of unistd/misc.c
    self.set_setting('WASMFS')
    self.do_run_in_out_file_test('wasmfs/wasmfs_fdatasync.c')

  @also_with_wasmfs
  def test_unistd_seek(self):
    self.do_run_in_out_file_test('wasmfs/wasmfs_seek.c')

  @also_with_wasmfs
  def test_unistd_mkdir(self):
    self.do_run_in_out_file_test('wasmfs/wasmfs_mkdir.c')

  @also_with_wasmfs
  def test_unistd_cwd(self):
    self.do_run_in_out_file_test('wasmfs/wasmfs_chdir.c')

  def test_unistd_chown(self):
    # TODO: Remove this test in favor of unistd/misc.c
    self.set_setting('WASMFS')
    self.do_run_in_out_file_test('wasmfs/wasmfs_chown.c')

  @wasmfs_all_backends
  def test_wasmfs_getdents(self):
    # Run only in WASMFS for now.
    self.set_setting('FORCE_FILESYSTEM')
    self.do_run_in_out_file_test('wasmfs/wasmfs_getdents.c')

  @wasmfs_all_backends
  @also_with_wasm_bigint
  def test_wasmfs_readfile(self):
    self.set_setting('FORCE_FILESYSTEM')
    self.do_run_in_out_file_test('wasmfs/wasmfs_readfile.c')

  def test_wasmfs_jsfile(self):
    self.set_setting('WASMFS')
    self.do_run_in_out_file_test('wasmfs/wasmfs_jsfile.c')

  def test_wasmfs_before_preload(self):
    self.set_setting('WASMFS')
    os.mkdir('js_backend_files')
    create_file('js_backend_files/file.dat', 'data')
    self.emcc_args += ['--preload-file', 'js_backend_files/file.dat']
    self.do_run_in_out_file_test('wasmfs/wasmfs_before_preload.c')

  def test_hello_world_above_2gb(self):
    self.do_run_in_out_file_test('hello_world.c', emcc_args=['-sGLOBAL_BASE=2GB', '-sINITIAL_MEMORY=3GB'])

  def test_hello_function(self):
    # hello_function.cpp is referenced/used in the docs.  This test ensures that it
    # at least compiles.
    # (It seems odd that we ship the entire test/ directory to all our users and
    # reference them in our docs.  Should we move this file to somewhere else such
    # as `examples/`?)
    self.run_process([EMCC, test_file('hello_function.cpp'), '-o', 'function.html', '-sEXPORTED_FUNCTIONS=_int_sqrt', '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap'])

  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  @crossplatform
  def test_es5_transpile(self, args):
    self.emcc_args += ['-Wno-transpile'] + args

    # Create a library file that uses the following ES6 features
    # - let/const
    # - arrow funcs
    # - for..of
    # - object.assign
    # - nullish coalescing & chaining
    # - logical assignment
    create_file('es6_library.js', '''\
    addToLibrary({
      foo: function(arg="hello", ...args) {
        // Object.assign + let
        let obj = Object.assign({}, {prop:1});
        err('prop: ' + obj.prop);

        // for .. of
        for (var elem of [42, 43]) {
          err('array elem: ' + elem);
        }

        // arrow funcs + const
        const bar = () => 2;
        err('bar: ' + bar());

        // Computed property names
        var key = 'mykey';
        var obj2 = {
          [key]: 42,
        };
        err('computed prop: ' + obj2[key]);

        // Method syntax
        var obj3 = {
          myMethod() { return 43 },
        };
        global['foo'] = obj3;
        err('myMethod: ' + obj3.myMethod());

        // Nullish coalescing
        var definitely = global['maybe'] ?? {};

        // Optional chaining
        global['maybe']
          ?.subObj
          ?.[key]
          ?.func
          ?.();

        // Logical assignment
        var obj4 = null;
        obj4 ??= 0;
        obj4 ||= 1;
        obj4 &&= 2;

        console.log(...args);
      }
    });
    ''')
    expected = '''\
prop: 1
array elem: 42
array elem: 43
bar: 2
computed prop: 42
myMethod: 43
'''

    create_file('test.c', 'extern void foo(); int main() { foo(); }')
    self.emcc_args += ['--js-library', 'es6_library.js']
    self.uses_es6 = True

    def check_for_es6(filename, expect):
      js = read_file(filename)
      if expect:
        self.assertContained('foo(arg="hello"', js)
        self.assertContained(['() => 2', '()=>2'], js)
        self.assertContained('const ', js)
        self.assertContained('?.[', js)
        self.assertContained('?.(', js)
        self.assertContained('??=', js)
        self.assertContained('||=', js)
        self.assertContained('&&=', js)
        self.assertContained('...', js)
      else:
        self.verify_es5(filename)
        self.assertNotContained('foo(arg=', js)
        self.assertNotContained('() => 2', js)
        self.assertNotContained('()=>2', js)
        self.assertNotContained('const ', js)
        self.assertNotContained('??', js)
        self.assertNotContained('?.', js)
        self.assertNotContained('||=', js)
        self.assertNotContained('&&=', js)
        self.assertNotContained('...args', js)

    # Check that under normal circumstances none of these features get
    # removed / transpiled.
    print('base case')
    self.do_runf('test.c', expected)
    check_for_es6('test.js', True)

    # If we select and older browser than closure will kick in by default
    # to transpile.
    print('with old browser')
    self.emcc_args.remove('-Werror')
    self.set_setting('LEGACY_VM_SUPPORT')
    self.do_runf('test.c', expected, output_basename='test_old')
    check_for_es6('test_old.js', False)
    if '-O3' in args:
      # Verify that output is minified
      self.assertEqual(len(read_file('test_old.js').splitlines()), 1)

    # If we add `-sPOLYFILL=0` that transpiler is not run at all
    print('with old browser + -sPOLYFILL=0')
    self.do_runf('test.c', expected, emcc_args=['-sPOLYFILL=0'], output_basename='test_no_closure')
    check_for_es6('test_no_closure.js', True)

    # Test that transpiling is compatible with `--closure=1`
    print('with old browser + --closure=1')
    self.do_runf('test.c', expected, emcc_args=['--closure=1'], output_basename='test_closure')
    check_for_es6('test_closure.js', False)

  def test_gmtime_noleak(self):
    # Confirm that gmtime_r does not leak when called in isolation.
    self.emcc_args.append('-fsanitize=leak')
    self.do_other_test('test_gmtime_noleak.c')

  def test_build_fetch_tests(self):
    # We can't run these outside of the browser, but at least we can
    # make sure they build.
    self.set_setting('FETCH')
    self.build(test_file('fetch/test_fetch_to_memory_sync.c'))
    self.build(test_file('fetch/test_fetch_to_memory_async.c'))
    self.build(test_file('fetch/test_fetch_persist.c'))
    self.build(test_file('fetch/test_fetch_idb_delete.c'))
    self.build(test_file('fetch/test_fetch_idb_store.c'))
    self.build(test_file('fetch/test_fetch_stream_async.c'))
    self.build(test_file('fetch/test_fetch_sync.c'))
    self.build(test_file('fetch/test_fetch_progress.c'))

  def test_fetch_init_node(self):
    # Make sure that `Fetch` initialises correctly under Node where
    # IndexedDB isn't available.
    create_file('src.c', r'''
#include <stdio.h>
int main() {
  puts("ok");
}
''')
    self.do_runf('src.c', 'ok', emcc_args=['-sFETCH', '-sEXPORTED_RUNTIME_METHODS=Fetch'])

  # Test that using llvm-nm works when response files are in use, and inputs are linked using relative paths.
  # llvm-nm has a quirk that it does not remove escape chars when printing out filenames.
  @with_env_modify({'EM_FORCE_RESPONSE_FILES': '1'})
  def test_llvm_nm_relative_paths_works_with_response_files(self):
    os.mkdir('foo')
    # Test creating a library file with a relative path in a subdir, so it gets a double backslash "\\" in the generated llvm-nm output.
    # Also add a space to stress space escaping, e.g. "\ ".
    create_file(os.path.join('foo', 'foo bar.c'), r'''
      #include <time.h>
      #include <stdint.h>
      time_t foo()
      {
        int64_t secondsSinceEpoch = 0;
        struct tm* utcTime = gmtime((time_t*)&secondsSinceEpoch);
        return mktime(utcTime);
      }
    ''')
    create_file('main.c', r'''
      #include <stdio.h>
      #include <time.h>
      time_t foo(void);
      int main()
      {
        printf("%d\n", (int)foo());
      }
    ''')
    self.run_process([EMCC, 'main.c', os.path.join('foo', 'foo bar.c')])

  def test_tutorial(self):
    # Ensure that files referenced in Tutorial.rst are buildable
    self.run_process([EMCC, test_file('hello_world_file.cpp')])

  def test_stdint_limits(self):
    self.do_other_test('test_stdint_limits.c')

  def test_legacy_runtime(self):
    self.set_setting('EXPORTED_FUNCTIONS', ['_malloc', '_main'])
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', ['$intArrayFromString', '$ALLOC_NORMAL'])

    # By default `LEGACY_RUNTIME` is disabled and `allocate` is not available.
    self.set_setting('EXPORTED_RUNTIME_METHODS', ['ALLOC_NORMAL'])
    self.emcc_args += ['-Wno-deprecated']
    self.do_runf('other/test_legacy_runtime.c',
                 '`allocate` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line',
                 assert_returncode=NON_ZERO)

    # When we enable `LEGACY_RUNTIME`, `allocate` should be available.
    self.set_setting('LEGACY_RUNTIME', 1)
    self.do_runf('other/test_legacy_runtime.c')

    # Adding it to EXPORTED_RUNTIME_METHODS should also make it available.
    self.clear_setting('LEGACY_RUNTIME')
    self.set_setting('EXPORTED_RUNTIME_METHODS', ['allocate'])
    self.do_runf('other/test_legacy_runtime.c', 'hello from js')

    # In strict mode the library function is not even available, so we get a build time error
    self.set_setting('STRICT')
    self.clear_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE')
    for opt in ('-O0', '-O3'):
      err = self.expect_fail([EMCC, test_file('other/test_legacy_runtime.c'), opt] + self.get_emcc_args())
      self.assertContained('warning: invalid item in EXPORTED_RUNTIME_METHODS: allocate', err)

  def test_fetch_settings(self):
    create_file('pre.js', '''
    Module.fetchSettings = { cache: 'no-store' };
    ''')
    self.emcc_args += ['--pre-js=pre.js']
    self.do_runf('hello_world.c', '`Module.fetchSettings` was supplied but `fetchSettings` not included in INCOMING_MODULE_JS_API', assert_returncode=NON_ZERO)

    # Try again with INCOMING_MODULE_JS_API set
    self.set_setting('INCOMING_MODULE_JS_API', 'fetchSettings')
    self.do_run_in_out_file_test('hello_world.c')
    src = read_file('hello_world.js')
    self.assertContained("fetch(binaryFile, Module['fetchSettings'] || ", src)

  # Tests using the #warning directive in JS library files
  def test_warning_in_js_libraries(self):
    proc = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', test_file('warning_in_js_libraries.js')], stdout=PIPE, stderr=PIPE)
    self.assertNotContained('This warning should not be present!', proc.stderr)
    self.assertContained('warning_in_js_libraries.js:5: #warning This is a warning string!', proc.stderr)
    self.assertContained('warning_in_js_libraries.js:7: #warning This is a second warning string!', proc.stderr)

  # Tests using the #error directive in JS library files
  def test_error_in_js_libraries(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', test_file('error_in_js_libraries.js')])
    self.assertNotContained('This error should not be present!', err)
    self.assertContained('error_in_js_libraries.js:5: #error This is an error string!', err)
    self.assertContained('error_in_js_libraries.js:7: #error This is a second error string!', err)

  # Tests building with -sSHARED_MEMORY
  @also_with_minimal_runtime
  def test_shared_memory(self):
    self.do_runf('wasm_worker/shared_memory.c', '0', emcc_args=[])
    self.node_args += shared.node_pthread_flags(self.get_nodejs())
    self.do_runf('wasm_worker/shared_memory.c', '1', emcc_args=['-sSHARED_MEMORY'])
    self.do_runf('wasm_worker/shared_memory.c', '1', emcc_args=['-sWASM_WORKERS'])
    self.do_runf('wasm_worker/shared_memory.c', '1', emcc_args=['-pthread'])

  # Tests C preprocessor flags with -sSHARED_MEMORY
  @also_with_minimal_runtime
  def test_shared_memory_preprocessor_flags(self):
    self.run_process([EMCC, '-c', test_file('wasm_worker/shared_memory_preprocessor_flags.c'), '-sSHARED_MEMORY'])

  # Tests C preprocessor flags with -sWASM_WORKERS
  @also_with_minimal_runtime
  def test_wasm_worker_preprocessor_flags(self):
    self.run_process([EMCC, '-c', test_file('wasm_worker/wasm_worker_preprocessor_flags.c'), '-sWASM_WORKERS'])

  @parameterized({
    # we will warn here since -O2 runs the optimizer and -g enables DWARF
    'O2_g': (True, ['-O2', '-g'],),
    # asyncify will force wasm-opt to run as well, so we warn here too
    'asyncify_g': (True, ['-sASYNCIFY', '-g'],),
    # with --profiling-funcs however we do not use DWARF (we just emit the
    # names section) and will not warn.
    'O2_pfuncs': (False, ['-O2', '--profiling-funcs'],),
  })
  def test_debug_opt_warning(self, should_fail, args):
    if should_fail:
      err = self.expect_fail([EMCC, test_file('hello_world.c'), '-Werror'] + args)
      self.assertContained('error: running limited binaryen optimizations because DWARF info requested (or indirectly required) [-Wlimited-postlink-optimizations]', err)
    else:
      self.run_process([EMCC, test_file('hello_world.c'), '-Werror'] + args)

  @also_with_minimal_runtime
  def test_wasm_worker_closure(self):
    self.run_process([EMCC, test_file('wasm_worker/lock_async_acquire.c'), '-O2', '-sWASM_WORKERS', '--closure=1'])

  def test_wasm_worker_errors(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sWASM_WORKERS', '-sSINGLE_FILE'])
    self.assertContained('-sSINGLE_FILE is not supported with -sWASM_WORKERS', err)
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sWASM_WORKERS', '-sPROXY_TO_WORKER'])
    self.assertContained('-sPROXY_TO_WORKER is not supported with -sWASM_WORKERS', err)
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sWASM_WORKERS', '-sRELOCATABLE'])
    self.assertContained('dynamic linking is not supported with -sWASM_WORKERS', err)

  def test_clock_nanosleep(self):
    self.do_runf('other/test_clock_nanosleep.c')

  # Tests the internal test suite of tools/unsafe_optimizations.js
  def test_unsafe_optimizations(self):
    self.run_process(config.NODE_JS_TEST + [path_from_root('tools', 'unsafe_optimizations.mjs'), '--test'])

  @requires_v8
  def test_extended_const(self):
    self.v8_args += ['--experimental-wasm-extended-const']
    # Export at least one global so that we exercise the parsing of the global section.
    self.do_runf('hello_world.c', emcc_args=['-sEXPORTED_FUNCTIONS=_main,___stdout_used', '-mextended-const', '-sMAIN_MODULE=2'])
    wat = self.get_wasm_text('hello_world.wasm')
    # Test that extended-const expressions are used in the data segments.
    self.assertContained(r'\(data (\$\S+ )?\(offset \(i32.add\s+\(global.get \$\S+\)\s+\(i32.const \d+\)', wat, regex=True)
    # Test that extended-const expressions are used in at least one global initializer.
    self.assertContained(r'\(global \$\S+ i32 \(i32.add\s+\(global.get \$\S+\)\s+\(i32.const \d+\)', wat, regex=True)

  # Smoketest for MEMORY64 setting.  Most of the testing of MEMORY64 is by way of the wasm64
  # variant of the core test suite.
  @parameterized({
    'O0': (['-O0'],),
    'O1': (['-O1'],),
    'O2': (['-O2'],),
    'O3': (['-O3'],),
    'Oz': (['-Oz'],),
  })
  @requires_wasm64
  def test_memory64(self, args):
    self.do_run_in_out_file_test('core/test_hello_argc.c', args=['hello', 'world'], emcc_args=['-sMEMORY64', '-Wno-experimental'] + args)

  # Verfy that MAIN_MODULE=1 (which includes all symbols from all libraries)
  # works with -sPROXY_POSIX_SOCKETS and -Oz, both of which affect linking of
  # system libraries in different ways.
  @also_with_wasmfs
  def test_dylink_proxy_posix_sockets_oz(self):
    self.do_runf('hello_world.cpp', emcc_args=['-lwebsocket.js', '-sMAIN_MODULE=1', '-sPROXY_POSIX_SOCKETS', '-Oz'])

  def test_in_tree_header_usage(self):
    # Using headers directly from where they live in the source tree does not work.
    # Verify that we generate a useful warning when folks try to do this.
    create_file('test.c', '#include <emscripten.h>')
    err = self.expect_fail([EMCC, '-I' + path_from_root('system/include'), 'test.c'])
    self.assertContained('#error "Including files directly from the emscripten source tree is not supported', err)

  def test_multiple_g_flags(self):
    # Verify that a second -g argument overrides the first
    self.run_process([EMCC, test_file('hello_world.c'), '-c', '-g'])
    self.assertIn(b'.debug', read_binary('hello_world.o'))
    self.run_process([EMCC, test_file('hello_world.c'), '-c', '-g', '-g0'])
    self.assertNotIn(b'.debug', read_binary('hello_world.o'))

  @requires_v8
  def test_jspi_code_size(self):
    # use iostream code here to purposefully get a fairly large wasm file, so
    # that our size comparisons later are meaningful
    create_file('main.cpp', r'''
      #include <emscripten.h>
      #include <iostream>
      int main() {
        std::cout << "nap time\n";
        emscripten_sleep(1);
        std::cout << "i am awake\n";
      }
    ''')
    expected = 'nap time\ni am awake\n'

    shared_args = ['-Os', '-sENVIRONMENT=shell']
    self.run_process([EMXX, 'main.cpp', '-sASYNCIFY'] + shared_args)
    self.assertContained(expected, self.run_js('a.out.js'))
    asyncify_size = os.path.getsize('a.out.wasm')

    self.run_process([EMXX, 'main.cpp', '-sASYNCIFY=2'] + shared_args)

    # enable stack switching and other relevant features (like reference types
    # for the return value of externref)
    self.v8_args.append('--experimental-wasm-stack-switching')

    self.assertContained(expected, self.run_js('a.out.js'))
    stack_switching_size = os.path.getsize('a.out.wasm')

    # also compare to code size without asyncify or stack switching
    self.run_process([EMXX, 'main.cpp'] + shared_args)
    nothing_size = os.path.getsize('a.out.wasm')

    # stack switching does not asyncify the code, which means it is very close
    # to the normal "nothing" size, and much smaller than the asyncified size
    self.assertLess(stack_switching_size, 0.60 * asyncify_size)
    self.assertLess(abs(stack_switching_size - nothing_size), 0.01 * nothing_size)

  def test_no_cfi(self):
    err = self.expect_fail([EMCC, '-fsanitize=cfi', '-flto', test_file('hello_world.c')])
    self.assertContained('emcc: error: emscripten does not currently support -fsanitize=cfi', err)

  @also_with_wasm_bigint
  def test_parseTools(self):
    # Suppress js compiler warnings because we deliberately use legacy parseTools functions
    self.emcc_args += ['-Wno-js-compiler', '--js-library', test_file('other/test_parseTools.js')]
    self.do_other_test('test_parseTools.c')

    # If we run ths same test with -sASSERTIONS=2 we expect it to fail because it
    # involves writing numbers that are exceed the side of the type.
    expected = 'Aborted(Assertion failed: value (316059037807746200000) too large to write as 64-bit value)'
    self.do_runf('other/test_parseTools.c', expected, emcc_args=['-sASSERTIONS=2'], assert_returncode=NON_ZERO)

  def test_lto_atexit(self):
    self.emcc_args.append('-flto')

    # Without EXIT_RUNTIME we don't expect the dtor to run at all
    output = self.do_runf('other/test_lto_atexit.c', 'main done')
    self.assertNotContained('my_dtor', output)

    # With EXIT_RUNTIME we expect to see the dtor running.
    self.set_setting('EXIT_RUNTIME')
    self.do_runf('other/test_lto_atexit.c', 'main done\nmy_dtor\n')

  @crossplatform
  def test_prejs_unicode(self):
    create_file('script.js', r'''
      console.log('↓');
    ''')
    self.do_runf('hello_world.c', '↓', emcc_args=['--pre-js=script.js'])

  def test_xlocale(self):
    # Test for xlocale.h compatibility header
    self.do_other_test('test_xlocale.c')

  def test_print_map(self):
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    out = self.run_process([EMCC, 'hello_world.o', '-Wl,--print-map'], stdout=PIPE).stdout
    self.assertContained('hello_world.o:(__original_main)', out)
    out2 = self.run_process([EMCC, 'hello_world.o', '-Wl,-M'], stdout=PIPE).stdout
    self.assertEqual(out, out2)

  def test_rust_gxx_personality_v0(self):
    self.do_run(r'''
      #include <stdio.h>
      #include <stdint.h>
      extern "C" {
        int __gxx_personality_v0(int version, void* actions, uint64_t exception_class, void* exception_object, void* context);
      }
      int main() {
        __gxx_personality_v0(0, NULL, 0, NULL, NULL);
        return 0;
      }
    ''', assert_returncode=NON_ZERO, emcc_args=['-fexceptions'])

  def test_bigint64array_polyfill(self):
    bigint64array = read_file(path_from_root('src/polyfill/bigint64array.js'))
    test_code = read_file(test_file('test_bigint64array_polyfill.js'))
    bigint_list = [
      0,
      1,
      -1,
      5,
      (1 << 64),
      (1 << 64) - 1,
      (1 << 64) + 1,
      (1 << 63),
      (1 << 63) - 1,
      (1 << 63) + 1,
    ]
    bigint_list_strs = [str(x) for x in bigint_list]

    bigint_list_unsigned = [x % (1 << 64) for x in bigint_list]
    bigint_list_signed = [
      x if x < 0 else (x % (1 << 64)) - 2 * (x & (1 << 63)) for x in bigint_list
    ]
    bigint_list_unsigned_n = [f'{x}n' for x in bigint_list_unsigned]
    bigint_list_signed_n = [f'{x}n' for x in bigint_list_signed]

    bigint64array = '\n'.join(bigint64array.splitlines()[3:])

    create_file(
      'test.js',
      f'''
      let bigint_list = {bigint_list_strs}.map(x => BigInt(x));
      let arr1signed = new BigInt64Array(20);
      let arr1unsigned = new BigUint64Array(20);
      delete globalThis.BigInt64Array;
      ''' + bigint64array + test_code
    )
    output = json.loads(self.run_js('test.js'))
    self.assertEqual(output['BigInt64Array_name'], 'createBigInt64Array')
    for key in ('arr1_to_arr1', 'arr1_to_arr2', 'arr2_to_arr1'):
      print(key + '_unsigned')
      self.assertEqual(output[key + '_unsigned'], bigint_list_unsigned_n)
    for key in ('arr1_to_arr1', 'arr1_to_arr2', 'arr2_to_arr1'):
      print(key + '_signed')
      self.assertEqual(output[key + '_signed'], bigint_list_signed_n)

    self.assertEqual(output['arr2_slice'], ['2n', '3n', '4n', '5n'])
    self.assertEqual(output['arr2_subarray'], ['2n', '3n', '4n', '5n'])

    for m, [v1, v2] in output['assertEquals']:
      self.assertEqual(v1, v2, msg=m)

  def test_warn_once(self):
    create_file('main.c', r'''\
      #include <stdio.h>
      #include <emscripten.h>

      EM_JS_DEPS(main, "$warnOnce");

      int main() {
        EM_ASM({
          warnOnce("foo");
          // Second call should not output anything
          warnOnce("foo");
        });
        printf("done\n");
      }
    ''')
    self.do_runf('main.c', 'warning: foo\ndone\n')

  def test_dyncallwrapper(self):
    self.set_setting("MAIN_MODULE", 1)
    expected = """\
2 7
i: 2 j: 8589934599 f: 3.120000 d: 77.120000
j1: 8589934599, j2: 30064771074, j3: 12884901891
"""
    self.do_runf("test_runtime_dyncall_wrapper.c", expected)

  def test_compile_with_cache_lock(self):
    # Verify that, after warming the cache, running emcc does not require the cache lock.
    # Previously we would acquire the lock during sanity checking (even when the check
    # passed) which meant the second process here would deadlock.
    if config.FROZEN_CACHE:
      self.skipTest("test doesn't work with frozen cache")
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    with cache.lock('testing'):
      self.run_process([EMCC, '-c', test_file('hello_world.c')])

  def test_recursive_cache_lock(self):
    if config.FROZEN_CACHE:
      self.skipTest("test doesn't work with frozen cache")
    with cache.lock('testing'):
      err = self.expect_fail([EMBUILDER, 'build', 'libc', '--force'], expect_traceback=True)
    self.assertContained('AssertionError: attempt to lock the cache while a parent process is holding the lock', err)

  @also_with_wasmfs
  def test_fs_icase(self):
    # c++20 for ends_with().
    self.do_other_test('test_fs_icase.cpp', emcc_args=['-sCASE_INSENSITIVE_FS', '-std=c++20'])

  def test_strict_js_closure(self):
    self.do_runf('hello_world.c', emcc_args=['-sSTRICT_JS', '-Werror=closure', '--closure=1', '-O3'])

  def test_em_js_deps(self):
    # Check that EM_JS_DEPS works. Specifically, multiple different instances in different
    # object files.
    create_file('f1.c', '''
    #include <emscripten.h>

    EM_JS_DEPS(other, "$stringToUTF8OnStack");
    ''')
    create_file('f2.c', '''
    #include <emscripten.h>

    EM_JS_DEPS(main, "$getHeapMax");

    int main() {
      EM_ASM({
        err(getHeapMax());
        var x = stackSave();
        stringToUTF8OnStack("hello");
        stackRestore(x);
      });
      return 0;
    }
    ''')
    self.do_runf('f2.c', emcc_args=['f1.c'])

  def test_em_js_deps_anon_ns(self):
    # Check that EM_JS_DEPS is not eliminated in
    # an anonymous C++ namespace.
    create_file('test_em_js_deps.cpp', '''
    #include <emscripten.h>

    namespace {
      EM_JS_DEPS(test, "$stringToUTF8OnStack");
    }

    int main() {
      EM_ASM({
        var x = stackSave();
        stringToUTF8OnStack("hello");
        stackRestore(x);
      });
    }
    ''')
    self.do_runf('test_em_js_deps.cpp')

  @no_mac('https://github.com/emscripten-core/emscripten/issues/18175')
  @crossplatform
  def test_stack_overflow(self):
    self.set_setting('STACK_OVERFLOW_CHECK', 1)
    self.emcc_args += ['-O1', '--profiling-funcs']
    self.do_runf('core/stack_overflow.c',
                 'Stack overflow detected.  You can try increasing -sSTACK_SIZE',
                 assert_returncode=NON_ZERO)

  @crossplatform
  def test_reproduce(self):
    self.run_process([EMCC, '-sASSERTIONS=1', '--reproduce=foo.tar', test_file('hello_world.c')])
    self.assertExists('foo.tar')
    names = []
    root = os.path.splitdrive(path_from_root())[1][1:]
    root = utils.normalize_path(root)
    print('root: %s' % root)
    with tarfile.open('foo.tar') as f:
      for name in f.getnames():
        print('name: %s' % name)
        names.append(name.replace(root, '<root>'))
      f.extractall()
    names = '\n'.join(sorted(names)) + '\n'
    expected = '''\
foo/<root>/test/hello_world.c
foo/response.txt
foo/version.txt
'''
    self.assertTextDataIdentical(expected, names)
    expected = '''\
-sASSERTIONS=1
<root>/test/hello_world.c
'''
    response = read_file('foo/response.txt')
    response = utils.normalize_path(response)
    response = response.replace(root, '<root>')
    self.assertTextDataIdentical(expected, response)

  def test_min_browser_version(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-Wno-transpile', '-Werror', '-sWASM_BIGINT', '-sMIN_SAFARI_VERSION=120000'])
    self.assertContained('emcc: error: MIN_SAFARI_VERSION=120000 is not compatible with WASM_BIGINT (150000 or above required)', err)

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-Wno-transpile', '-Werror', '-pthread', '-sMIN_CHROME_VERSION=73'])
    self.assertContained('emcc: error: MIN_CHROME_VERSION=73 is not compatible with pthreads (74 or above required)', err)

  def test_signext_lowering(self):
    # Use `-v` to show the sub-commands being run by emcc.
    cmd = [EMCC, test_file('other/test_signext_lowering.c'), '-v']

    # By default we don't expect the lowering pass to be run.
    err = self.run_process(cmd, stderr=subprocess.PIPE).stderr
    self.assertNotContained('--signext-lowering', err)

    # Specifying an older browser version should trigger the lowering pass
    err = self.run_process(cmd + ['-sMIN_SAFARI_VERSION=120000'], stderr=subprocess.PIPE).stderr
    self.assertContained('--signext-lowering', err)
    err = self.run_process(cmd + ['-sMIN_FIREFOX_VERSION=61'], stderr=subprocess.PIPE).stderr
    self.assertContained('--signext-lowering', err)
    err = self.run_process(cmd + ['-sMIN_CHROME_VERSION=73'], stderr=subprocess.PIPE).stderr
    self.assertContained('--signext-lowering', err)

  @flaky('https://github.com/emscripten-core/emscripten/issues/20125')
  def test_itimer(self):
    self.do_other_test('test_itimer.c')

  @node_pthreads
  @flaky('https://github.com/emscripten-core/emscripten/issues/20125')
  def test_itimer_pthread(self):
    self.do_other_test('test_itimer.c')

  def test_itimer_standalone(self):
    self.emcc_args += ['-sSTANDALONE_WASM', '-sWASM_BIGINT']
    self.do_other_test('test_itimer_standalone.c')
    for engine in config.WASM_ENGINES:
      print('wasm engine', engine)
      self.assertContained('done', self.run_js('test_itimer_standalone.wasm', engine))

  @node_pthreads
  @flaky('https://github.com/emscripten-core/emscripten/issues/20125')
  def test_itimer_proxy_to_pthread(self):
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.do_other_test('test_itimer.c')

  @node_pthreads
  def test_dbg(self):
    create_file('pre.js', '''
    dbg('start');
    Module.onRuntimeInitialized = () => dbg('done init');
    ''')
    expected = '''\
start
w:0,t:0x[0-9a-fA-F]+: done init
hello, world!
w:0,t:0x[0-9a-fA-F]+: native dbg message
w:0,t:0x[0-9a-fA-F]+: hello
w:0,t:0x[0-9a-fA-F]+: formatted: 42
'''
    self.emcc_args.append('--pre-js=pre.js')
    # Verify that, after initialization, dbg() messages are prefixed with
    # worker and thread ID.
    self.do_runf('other/test_dbg.c', expected, regex=True)

    # When assertions are disabled `dbg` function is not defined
    self.do_runf('other/test_dbg.c',
                 'ReferenceError: dbg is not defined',
                 emcc_args=['-DNDEBUG', '-sASSERTIONS=0'],
                 assert_returncode=NON_ZERO)

  def test_standalone_settings(self):
    base_cmd = [EMCC, test_file('hello_world.c'), '-sSTANDALONE_WASM']

    err = self.expect_fail(base_cmd + ['-sMINIMAL_RUNTIME'])
    self.assertContained('error: MINIMAL_RUNTIME reduces JS size, and is incompatible with STANDALONE_WASM which focuses on ignoring JS anyhow and being 100% wasm', err)

    err = self.expect_fail(base_cmd + ['-sMEMORY_GROWTH_GEOMETRIC_CAP=1mb'])
    self.assertContained('error: MEMORY_GROWTH_GEOMETRIC_CAP is not compatible with STANDALONE_WASM', err)

    err = self.expect_fail(base_cmd + ['-sMEMORY_GROWTH_LINEAR_STEP=1mb'])
    self.assertContained('error: MEMORY_GROWTH_LINEAR_STEP is not compatible with STANDALONE_WASM', err)

  @is_slow_test
  def test_googletest(self):
    # TODO(sbc): Should we package gtest as an emscripten "port"?  I guess we should if
    # we plan on using it in more places.
    self.emcc_args += [
      '-I' + test_file('third_party/googletest/googletest'),
      '-I' + test_file('third_party/googletest/googletest/include'),
      test_file('third_party/googletest/googletest/src/gtest-all.cc'),
      test_file('third_party/googletest/googletest/src/gtest_main.cc'),
    ]
    self.do_other_test('test_googletest.cc')

  def test_parseTools_legacy(self):
    create_file('post.js', '''
      err(_foo());
    ''')
    create_file('lib.js', '''
      addToLibrary({
        foo: () => {{{ Runtime.POINTER_SIZE }}}
      });
    ''')
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', 'foo')
    self.do_runf('hello_world.c', '4\nhello, world!',
                 emcc_args=['--post-js=post.js', '--js-library=lib.js'])

  @requires_node
  def test_min_node_version(self):
    node_version = shared.get_node_version(self.get_nodejs())
    node_version = '.'.join(str(x) for x in node_version)
    self.set_setting('MIN_NODE_VERSION', 210000)
    expected = 'This emscripten-generated code requires node v21.0.0 (detected v%s' % node_version
    self.do_runf('hello_world.c', expected, assert_returncode=NON_ZERO)

  def test_deprecated_macros(self):
    create_file('main.c', '''
    #include <assert.h>
    #include <stdio.h>
    #include <emscripten/threading.h>

    int main() {
      printf("%d\\n", emscripten_main_browser_thread_id());
      assert(emscripten_main_browser_thread_id());
      return 0;
    }
    ''')
    err = self.run_process([EMCC, 'main.c'], stderr=PIPE).stderr
    expected = "warning: macro 'emscripten_main_browser_thread_id' has been marked as deprecated: use emscripten_main_runtime_thread_id instead [-Wdeprecated-pragma]"
    self.assertContained(expected, err)

  @node_pthreads
  def test_USE_PTHREADS(self):
    self.emcc_args.remove('-pthread')
    self.set_setting('USE_PTHREADS')
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    self.do_runf('pthread/test_pthread_create.c')

  def test_cpp_module(self):
    self.run_process([EMXX, '-std=c++20', test_file('other/hello_world.cppm'), '--precompile', '-o', 'hello_world.pcm'])
    self.do_other_test('test_cpp_module.cpp', emcc_args=['-std=c++20', '-fprebuilt-module-path=.', 'hello_world.pcm'])

  @crossplatform
  def test_pthreads_flag(self):
    # We support just the singular form of `-pthread`, like gcc
    # Clang supports the plural form too but I think just due to historical accident:
    # See https://github.com/llvm/llvm-project/commit/c800391fb974cdaaa62bd74435f76408c2e5ceae
    err = self.expect_fail([EMCC, '-pthreads', '-c', test_file('hello_world.c')])
    self.assertContained('emcc: error: unrecognized command-line option `-pthreads`; did you mean `-pthread`?', err)

  def test_missing_struct_info(self):
    create_file('lib.js', '''
      {{{ C_STRUCTS.Foo }}}
    ''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library=lib.js'])
    self.assertContained('Error: Missing C struct Foo! If you just added it to struct_info.json, you need to run ./tools/maint/gen_struct_info.py (then run a second time with --wasm64)', err)

    create_file('lib.js', '''
      {{{ C_DEFINES.Foo }}}
    ''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library=lib.js'])
    self.assertContained('Error: Missing C define Foo! If you just added it to struct_info.json, you need to run ./tools/maint/gen_struct_info.py (then run a second time with --wasm64)', err)

  def run_wasi_test_suite_test(self, name):
    if not os.path.exists(path_from_root('test/third_party/wasi-test-suite')):
      self.fail('wasi-testsuite not found; run `git submodule update --init`')
    self.node_args += shared.node_bigint_flags(self.get_nodejs())
    wasm = path_from_root('test', 'third_party', 'wasi-test-suite', name + '.wasm')
    with open(path_from_root('test', 'third_party', 'wasi-test-suite', name + '.json')) as f:
      config = json.load(f)
    exit_code = config.get('exitCode', 0)
    args = config.get('args', [])
    env = config.get('env', [])
    if env:
      env = [f'ENV["{key}"] = "{value}";' for key, value in env.items()]
      env = '\n'.join(env)
      create_file('env.js', 'Module.preRun = () => { %s };' % env)
      self.emcc_args.append('--pre-js=env.js')
    self.run_process([EMCC, '-Wno-experimental', '--post-link', '-g',
                      '-sPURE_WASI', '-lnodefs.js', '-lnoderawfs.js',
                      wasm, '-o', name + '.js'] + self.get_emcc_args(main_file=True))

    output = self.run_js(name + '.js', args=args, assert_returncode=exit_code)
    if 'stdout' in config:
      self.assertContained(config['stdout'], output)

  @requires_node
  def test_wasi_std_env_args(self):
    create_file('pre.js', 'Module["thisProgram"] = "std_env_args.wasm"')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.run_wasi_test_suite_test('std_env_args')

  @requires_node
  def test_wasi_std_env_vars(self):
    self.run_wasi_test_suite_test('std_env_vars')

  @requires_node
  def test_wasi_std_io_stdout(self):
    self.run_wasi_test_suite_test('std_io_stdout')

  @requires_node
  def test_wasi_std_io_stderr(self):
    self.run_wasi_test_suite_test('std_io_stderr')

  @also_with_wasmfs
  @requires_node
  def test_wasi_clock_res_get(self):
    self.run_wasi_test_suite_test('wasi_clock_res_get')

  @requires_node
  def test_wasi_clock_time_get(self):
    self.run_wasi_test_suite_test('wasi_clock_time_get')

  @requires_node
  def test_wasi_fd_fdstat_get(self):
    self.run_wasi_test_suite_test('wasi_fd_fdstat_get')

  @requires_node
  def test_wasi_wasi_fd_write_file(self):
    self.run_wasi_test_suite_test('wasi_fd_write_file')
    self.assertEqual(read_file('new_file'), 'new_file')

  @requires_node
  def test_wasi_wasi_fd_write_stdout(self):
    self.run_wasi_test_suite_test('wasi_fd_write_stdout')

  @requires_node
  def test_wasi_wasi_fd_write_stderr(self):
    self.run_wasi_test_suite_test('wasi_fd_write_stderr')

  @requires_node
  def test_wasi_proc_exit(self):
    self.run_wasi_test_suite_test('wasi_proc_exit')

  @requires_node
  def test_wasi_random_get(self):
    self.run_wasi_test_suite_test('wasi_random_get')

  @requires_node
  def test_wasi_sched_yield(self):
    self.run_wasi_test_suite_test('wasi_sched_yield')

  def test_wasi_with_sjlj(self):
    # When PURE_WASI is set and Wasm exception is not being used, we turn off
    # SUPPORT_LONGJMP by default because it uses a JS-based simulation of
    # longjmp.
    self.set_setting('PURE_WASI')
    err = self.expect_fail([EMCC, test_file('core/test_longjmp.c')] + self.get_emcc_args())
    self.assertContained('error: longjmp support was disabled (SUPPORT_LONGJMP=0)', err)

    # When using Wasm exception, SUPPORT_LONGJMP defaults to 'wasm', which does
    # not use the JS-based support. This should succeed.
    self.emcc_args.append('-fwasm-exceptions')
    # -fwasm-exceptions exports __cpp_exception, so this is necessary
    self.set_setting('DEFAULT_TO_CXX')
    self.do_runf(test_file('core/test_longjmp.c'), emcc_args=self.get_emcc_args())

  def test_memops_bulk_memory(self):
    self.emcc_args += ['--profiling-funcs', '-fno-builtin']

    def run(args, expect_bulk_mem):
      self.do_runf('other/test_memops_bulk_memory.c', emcc_args=args)
      funcs = self.parse_wasm('test_memops_bulk_memory.wasm')[2]
      js = read_file('test_memops_bulk_memory.js')
      if expect_bulk_mem:
        self.assertNotContained('_emscripten_memcpy_js', js)
        self.assertIn('$_emscripten_memcpy_bulkmem', funcs)
      else:
        self.assertContained('_emscripten_memcpy_js', js)
        self.assertNotIn('$_emscripten_memcpy_bulkmem', funcs)

    # By default we expect to find `_emscripten_memcpy_js` in the generaed JS
    # and not to find the `emscripten_memcpy_bulkmem` function on the wasm
    # side.
    run([], expect_bulk_mem=False)

    # With bulk memory enabled we expect *not* to find it.
    run(['-mbulk-memory'], expect_bulk_mem=True)

    run(['-mbulk-memory', '-mno-bulk-memory'], expect_bulk_mem=False)

    # -pthread implicitly enables bulk memory too.
    self.setup_node_pthreads()
    run(['-pthread'], expect_bulk_mem=True)

  def test_memory_init_file_unsupported(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--memory-init-file=1'])
    self.assertContained('error: --memory-init-file is no longer supported', err)

  @node_pthreads
  def test_node_pthreads_err_out(self):
    create_file('post.js', 'err(1, 2, "hello"); out("foo", 42);')
    self.do_runf('hello_world.c', '1 2 hello\nfoo 42\n', emcc_args=['--post-js=post.js'])

  @only_windows('This test verifies Windows batch script behavior against bug https://github.com/microsoft/terminal/issues/15212')
  @with_env_modify({'PATH': path_from_root() + os.pathsep + os.getenv('PATH')})
  def test_windows_batch_file_dp0_expansion_bug(self):
    create_file('build_with_quotes.bat',  f'@"emcc" {test_file("hello_world.c")}')
    self.run_process(['build_with_quotes.bat'])

  @only_windows('Check that directory permissions are properly retrieved on Windows')
  @requires_node
  def test_windows_nodefs_execution_permission(self):
    src = r'''
    #include <assert.h>
    #include <emscripten.h>
    #include <sys/stat.h>
    #include <string.h>
    #include <stdio.h>

    void setup() {
      EM_ASM(
        FS.mkdir('/working');
        FS.mount(NODEFS, { root: '.' }, '/working');
        FS.mkdir('/working/new-dir');
        FS.writeFile('/working/new-dir/test.txt', 'test');
      );
    }

    void test() {
      int err;
      struct stat s;
      memset(&s, 0, sizeof(s));
      err = stat("/working/new-dir", &s);
      assert(S_ISDIR(s.st_mode));
      assert(s.st_mode & S_IXUSR);
      assert(s.st_mode & S_IXGRP);
      assert(s.st_mode & S_IXOTH);

      err = stat("/working/new-dir/test.txt", &s);
      assert(s.st_mode & S_IXUSR);
      assert(s.st_mode & S_IXGRP);
      assert(s.st_mode & S_IXOTH);

      puts("success");
    }

    int main(int argc, char * argv[]) {
      setup();
      test();
      return EXIT_SUCCESS;
    }
    '''
    self.do_run(src, emcc_args=['-lnodefs.js'])

  @parameterized({
    'wasm2js': (True,),
    '': (False,)
  })
  def test_add_js_function(self, wasm2js):
    self.set_setting('INVOKE_RUN', 0)
    self.set_setting('WASM_ASYNC_COMPILATION', 0)
    self.set_setting('ALLOW_TABLE_GROWTH')
    self.set_setting('EXPORTED_RUNTIME_METHODS', ['callMain'])
    if wasm2js:
      self.set_setting('WASM', 0)
    self.emcc_args += ['--post-js', test_file('interop/test_add_function_post.js')]

    print('basics')
    self.do_run_in_out_file_test('interop/test_add_function.cpp')

    print('with ALLOW_TABLE_GROWTH=0')
    self.set_setting('ALLOW_TABLE_GROWTH', 0)
    expected = 'Unable to grow wasm table'
    if wasm2js:
      # in wasm2js the error message doesn't come from the VM, but from our
      # emulation code. when ASSERTIONS are enabled we show a clear message, but
      # in optimized builds we don't waste code size on that, and the JS engine
      # shows a generic error.
      expected = 'wasmTable.grow is not a function'

    self.do_runf('interop/test_add_function.cpp', expected, assert_returncode=NON_ZERO)

    print('- with table growth')
    self.set_setting('ALLOW_TABLE_GROWTH')
    self.emcc_args += ['-DGROWTH']
    # enable costly assertions to verify correct table behavior
    self.set_setting('ASSERTIONS', 2)
    self.do_run_in_out_file_test('interop/test_add_function.cpp', interleaved_output=False)

  @parameterized({
    'memory64_wasm_function': (True, True),
    'wasm_function': (False, True),
    'memory64': (True, False),
    '': (False, False)
  })
  @requires_v8
  def test_add_js_function_bigint(self, memory64, wasm_function):
    self.set_setting('WASM_BIGINT')

    if memory64:
      self.require_wasm64()

    if not wasm_function:
      create_file('pre.js', 'delete WebAssembly.Function;')
      self.emcc_args.append('--pre-js=pre.js')

    self.set_setting('ALLOW_TABLE_GROWTH')
    create_file('main.c', r'''
      #include <emscripten.h>
      #include <assert.h>

      EM_JS_DEPS(deps, "$addFunction");

      typedef long long (functype)(long long);

      int main() {
        functype* f = (functype *)EM_ASM_INT({
          return addFunction((num) => {
              return num + 4294967296n;
          }, 'jj');
        });
        assert(f(26) == 26 + 4294967296);
        assert(f(493921253191) == 493921253191 + 4294967296);
      }
    ''')

    self.do_runf('main.c', '')

  @parameterized({
    '': ([],),
    'pthread': (['-g', '-pthread', '-Wno-experimental', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'],),
  })
  def test_preload_module(self, args):
    if args:
      self.setup_node_pthreads()
    # TODO(sbc): This test is copyied from test_browser.py.  Perhaps find a better way to
    # share code between them.
    create_file('library.c', r'''
      #include <stdio.h>
      int library_func() {
        return 42;
      }
    ''')
    self.run_process([EMCC, 'library.c', '-sSIDE_MODULE', '-o', 'tmp.so'] + args)
    create_file('main.c', r'''
      #include <assert.h>
      #include <dlfcn.h>
      #include <stdio.h>
      #include <sys/stat.h>
      #include <emscripten.h>
      #include <emscripten/threading.h>

      int main() {
        // Check the file exists in the VFS
        struct stat statbuf;
        assert(stat("/library.so", &statbuf) == 0);

        // Check that it was preloaded.
        // The preloading actually only happens on the main thread where the filesystem
        // lives.  On worker threads the module object is shared via preloadedModules.
        if (emscripten_is_main_runtime_thread()) {
          int found = EM_ASM_INT(
            return preloadedWasm['/library.so'] !== undefined;
          );
          assert(found);
        } else {
          int found = EM_ASM_INT(
            err(sharedModules);
            return sharedModules['/library.so'] !== undefined;
          );
          assert(found);
        }
        void *lib_handle = dlopen("/library.so", RTLD_NOW);
        assert(lib_handle);
        typedef int (*voidfunc)();
        voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
        assert(x);
        assert(x() == 42);
        printf("done\n");
        return 0;
      }
    ''')
    self.do_runf('main.c', 'done\n', emcc_args=['-sMAIN_MODULE=2', '--preload-file', 'tmp.so@library.so', '--use-preload-plugins'] + args)

  @node_pthreads
  def test_standalone_whole_archive(self):
    self.emcc_args += ['-sSTANDALONE_WASM', '-pthread', '-Wl,--whole-archive', '-lbulkmemory', '-lstandalonewasm', '-Wl,--no-whole-archive']
    self.do_runf('hello_world.c')

  @parameterized({
    '':   ([],),
    '_single_file': (['-sSINGLE_FILE'],),
    '_single_file_es6': (['-sSINGLE_FILE', '-sEXPORT_ES6', '--extern-post-js', test_file('modularize_post_js.js')],),
  })
  def test_proxy_to_worker(self, args):
    self.do_runf('hello_world.c', emcc_args=['--proxy-to-worker'] + args)

  @also_with_standalone_wasm()
  def test_console_out(self):
    self.do_other_test('test_console_out.c', regex=True)

  @requires_wasm64
  def test_explicit_target(self):
    self.do_runf('hello_world.c', emcc_args=['-target', 'wasm32'])
    self.do_runf('hello_world.c', emcc_args=['-target', 'wasm64-unknown-emscripten', '-Wno-experimental'])

    self.do_runf('hello_world.c', emcc_args=['--target=wasm32'])
    self.do_runf('hello_world.c', emcc_args=['--target=wasm64-unknown-emscripten', '-Wno-experimental'])

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-target', 'wasm32', '-sMEMORY64'])
    self.assertContained('emcc: error: wasm32 target is not compatible with -sMEMORY64', err)

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--target=arm64'])
    self.assertContained('emcc: error: unsupported target: arm64 (emcc only supports wasm64-unknown-emscripten and wasm32-unknown-emscripten', err)

  def test_quick_exit(self):
    self.do_other_test('test_quick_exit.c')

  @requires_wasm64
  @requires_node_canary
  def test_memory64_proxies(self):
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-sMEMORY64=1',
                      '-sINITIAL_MEMORY=5gb',
                      '-sMAXIMUM_MEMORY=5gb',
                      '-sALLOW_MEMORY_GROWTH',
                      '-sEXPORTED_FUNCTIONS=_malloc,_main',
                      '-Wno-experimental',
                      '--extern-post-js', test_file('other/test_memory64_proxies.js')])
    self.run_js('a.out.js')

  def test_no_minify(self):
    # Test that comments are preserved with `--minify=0` is used, even in `-Oz` builds.
    # This allows the output of emscripten to be run through the closure compiler as
    # as a separate build step.
    create_file('pre.js', '''
    /**
     * This comment should be preserved
     */
    console.log('hello');
    ''')
    comment = 'This comment should be preserved'

    self.run_process([EMCC, test_file('hello_world.c'), '--pre-js=pre.js', '-Oz'])
    content = read_file('a.out.js')
    self.assertNotContained(comment, content)

    self.run_process([EMCC, test_file('hello_world.c'), '--pre-js=pre.js', '-Oz', '--minify=0'])
    content = read_file('a.out.js')
    self.assertContained(comment, content)

  def test_no_minify_and_later_closure(self):
    # test that running closure after --minify=0 works
    self.run_process([EMCC, test_file('hello_libcxx.cpp'), '-O2', '--minify=0'])
    temp = building.closure_compiler('a.out.js',
                                     advanced=True,
                                     extra_closure_args=['--formatting', 'PRETTY_PRINT'])
    shutil.copy(temp, 'closured.js')
    self.assertContained('hello, world!', self.run_js('closured.js'))

  def test_table_base(self):
    create_file('test.c', r'''
    #include <stdio.h>
    int main() {
      printf("addr = %p\n", &printf);
    }''')
    self.do_runf('test.c', 'addr = 0x1\n')
    self.do_runf('test.c', 'addr = 0x400\n', emcc_args=['-sTABLE_BASE=1024'])

  def test_webidl_empty(self):
    create_file('test.idl', '')
    self.run_process([WEBIDL_BINDER, 'test.idl', 'glue'])
    self.assertExists('glue.cpp')
    self.assertExists('glue.js')
    self.emcc('glue.cpp', ['-c', '-Wall', '-Werror'])

  def test_noExitRuntime(self):
    onexit_called = 'onExit called'
    create_file('pre.js', f'Module.onExit = () => console.log("${onexit_called}");\n')
    self.emcc_args += ['--pre-js=pre.js']
    self.set_setting('EXIT_RUNTIME')

    # Normally, with EXIT_RUNTIME set we expect onExit to be called.
    output = self.do_run_in_out_file_test('hello_world.c')
    self.assertContained(onexit_called, output)

    # However, if we set `Module.noExitRuntime = true`, then it should
    # not be called.
    create_file('noexit.js', 'Module.noExitRuntime = true;\n')
    output = self.do_run_in_out_file_test('hello_world.c', emcc_args=['--pre-js=noexit.js'])
    self.assertNotContained(onexit_called, output)

    # Setting the internal `noExitRuntime` after startup should have the
    # same effect.
    create_file('noexit_oninit.js', 'Module.preRun = () => { noExitRuntime = true; }')
    output = self.do_run_in_out_file_test('hello_world.c', emcc_args=['--pre-js=noexit_oninit.js'])
    self.assertNotContained(onexit_called, output)

  def test_noExitRuntime_deps(self):
    create_file('lib.js', r'''
addToLibrary({
  foo__deps: ['$noExitRuntime'],
  foo: () => {
    return 0;
  }
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library=lib.js', '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=foo'])
    self.assertContained('error: noExitRuntime cannot be referenced via __deps mechanism', err)

  def test_hello_world_argv(self):
    self.do_runf('hello_world_argv.c', 'hello, world! (1)')

  def test_arguments_global(self):
    self.emcc(test_file('hello_world_argv.c'), ['-sENVIRONMENT=web', '-sSTRICT', '--closure=1', '-O2'])

  @parameterized({
    'no_std_exp':   (['-DEMMALLOC_NO_STD_EXPORTS'],),
    # When we let emmalloc build with the standard exports like malloc,
    # emmalloc == malloc.
    'with_std_exp': (['-DTEST_EMMALLOC_IS_MALLOC'],),
  })
  def test_emmalloc_in_addition(self, args):
    # Test that we can use emmalloc in addition to another malloc impl. When we
    # build emmalloc using -DEMMALLOC_NO_STD_EXPORTS it will not export malloc
    # etc., and only provide the emmalloc_malloc etc. family of functions that
    # we can use.
    emmalloc = path_from_root('system', 'lib', 'emmalloc.c')
    self.run_process([EMCC, test_file('other/test_emmalloc_in_addition.c'), emmalloc] + args)
    self.assertContained('success', self.run_js('a.out.js'))

  def test_unused_destructor(self):
    self.do_runf('other/test_unused_destructor.c', emcc_args=['-flto', '-O2'])
    # Verify that the string constant in the destructor is not included in the binary
    self.assertNotIn(b'hello from dtor', read_binary('test_unused_destructor.wasm'))

  def test_strip_all(self):
    def has_debug_section(wasm):
      with webassembly.Module('hello_world.wasm') as wasm:
        return wasm.get_custom_section('.debug_info') is not None

    # Use -O2 to ensure wasm-opt gets run
    self.emcc_args += ['-g', '-O2']

    # First, verify that `-g` produces a debug section
    self.do_runf('hello_world.c')
    self.assertTrue(has_debug_section('hello_world.wasm'))

    # Test `-Wl,--strip-all` will strip the debug section, but that the
    # the target features section is preserved so that later phases
    # (e.g. wasm-opt) can read it.
    self.do_runf('hello_world.c', emcc_args=['-Wl,--strip-all', '-pthread'])
    self.assertFalse(has_debug_section('hello_world.wasm'))

    # Verify that `-Wl,-s` and `-s` also both have the same effect
    self.do_runf('hello_world.c', emcc_args=['-Wl,-s', '-pthread'])
    self.assertFalse(has_debug_section('hello_world.wasm'))

    self.do_runf('hello_world.c', emcc_args=['-s', '-pthread'])
    self.assertFalse(has_debug_section('hello_world.wasm'))

  def test_embind_no_duplicate_symbols(self):
    # Embind implementation lives almost entirely in headers, which have special rules
    # around symbol deduplication during linking. Ensure that including Embind headers
    # in two different object files doesn't lead to linking errors.
    create_file('a.cpp', '#include <emscripten/bind.h>')
    create_file('b.cpp', '#include <emscripten/bind.h>')
    self.run_process([EMXX, '-std=c++23', '-lembind', 'a.cpp', 'b.cpp'])

  def test_no_pthread(self):
    self.do_runf('hello_world.c', emcc_args=['-pthread', '-no-pthread'])
    self.assertExists('hello_world.js')
    self.assertNotContained('Worker', read_file('hello_world.js'))

  def test_sysroot_includes_first(self):
    self.do_other_test('test_stdint_limits.c', emcc_args=['-std=c11', '-iwithsysroot/include'])

  def test_force_filesystem_error(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sFILESYSTEM=0', '-sFORCE_FILESYSTEM'])
    self.assertContained('emcc: error: `-sFORCE_FILESYSTEM` cannot be used with `-sFILESYSTEM=0`', err)

  def test_aligned_alloc(self):
    self.do_runf('test_aligned_alloc.c', '',
                 emcc_args=['-Wno-non-power-of-two-alignment'])

  def test_erf(self):
    self.do_other_test('test_erf.c')

  def test_math_hyperbolic(self):
    self.do_other_test('test_math_hyperbolic.c')

  def test_frexp(self):
    self.do_other_test('test_frexp.c')

  def test_fcvt(self):
    self.do_other_test('test_fcvt.cpp')

  def test_llrint(self):
    self.do_other_test('test_llrint.c')

  def test_strings(self):
    self.do_other_test('test_strings.c', args=['wowie', 'too', '74'])

  def test_strcmp_uni(self):
    self.do_other_test('test_strcmp_uni.c')

  def test_strndup(self):
    self.do_other_test('test_strndup.c')

  def test_errar(self):
    self.do_other_test('test_errar.c')

  def test_wcslen(self):
    self.do_other_test('test_wcslen.c')

  def test_regex(self):
    self.do_other_test('test_regex.c')

  def test_isdigit_l(self):
    self.do_other_test('test_isdigit_l.cpp')

  def test_iswdigit(self):
    self.do_other_test('test_iswdigit.c')

  def test_complex(self):
    self.do_other_test('test_complex.c')

  def test_dynamic_cast(self):
    self.do_other_test('test_dynamic_cast.cpp')

  def test_dynamic_cast_b(self):
    self.do_other_test('test_dynamic_cast_b.cpp')

  def test_dynamic_cast_2(self):
    self.do_other_test('test_dynamic_cast_2.cpp')

  def test_no_input_files(self):
    err = self.expect_fail([EMCC, '-c'])
    self.assertContained('clang: error: no input files', err)

    err = self.expect_fail([EMCC])
    self.assertContained('emcc: error: no input files', err)

  def test_embind_negative_enum_values(self):
    # Test if negative enum values are printed correctly and not overflown to
    # large values when CAN_ADDRESS_2GB is true.
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>
      #include <emscripten/bind.h>

      using namespace emscripten;

      int main() {
        EM_ASM(
        console.log(Module.value.neg.value);
        console.log(Module.value.zero.value);
        console.log(Module.value.pos.value);
        );
      }

      enum class value {
        neg = -1,
        zero = 0,
        pos = 1,
      };

      EMSCRIPTEN_BINDINGS(utility) {
        enum_<value>("value")
          .value("neg", value::neg)
          .value("zero", value::zero)
          .value("pos", value::pos);
      }
    '''
    expected = '-1\n0\n1\n'
    self.do_run(src, expected_output=expected,
                emcc_args=['-lembind', '-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=4GB'])

  @crossplatform
  def test_no_extra_output(self):
    self.run_process([EMCC, '-c', test_file('hello_world.c')])
    output = self.run_process([EMCC, '-c', test_file('hello_world.c')], stdout=PIPE, stderr=STDOUT).stdout
    self.assertEqual(output, '')

  def test_browser_too_old(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sMIN_CHROME_VERSION=10'])
    self.assertContained('emcc: error: MIN_CHROME_VERSION older than 32 is not supported', err)

  def test_js_only_settings(self):
    err = self.run_process([EMCC, test_file('hello_world.c'), '-o', 'foo.wasm', '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=emscripten_get_heap_max'], stderr=PIPE).stderr
    self.assertContained('emcc: warning: DEFAULT_LIBRARY_FUNCS_TO_INCLUDE is only valid when generating JavaScript output', err)

  def test_uuid(self):
    # We run this test in Node/SPIDERMONKEY and browser environments because we
    # try to make use of high quality crypto random number generators such as
    # crypto.getRandomValues or randomBytes (if available).

    # Use closure compiler so we can check that require('crypto').randomBytes and
    # window.crypto.getRandomValues doesn't get minified out.
    self.do_runf('test_uuid.c', emcc_args=['-O2', '--closure=1', '-luuid'])

    js_out = read_file('test_uuid.js')

    # Check that test.js compiled with --closure 1 contains ").randomBytes" and
    # "window.crypto.getRandomValues"
    self.assertContained(").randomBytes", js_out)
    self.assertContained("window.crypto.getRandomValues", js_out)

  def test_wasm64_no_asan(self):
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sMEMORY64', '-fsanitize=address'])
    self.assertContained('error: MEMORY64 does not yet work with ASAN', err)

  @crossplatform
  def test_js_preprocess_pre_post(self):
    create_file('pre.js', '''
    #preprocess
    #if ASSERTIONS
    console.log('assertions enabled')
    #else
    console.log('assertions disabled')
    #endif
    ''')
    create_file('post.js', '''
    #preprocess
    console.log({{{ POINTER_SIZE }}});
    ''')
    self.emcc_args += ['--pre-js', 'pre.js', '--post-js', 'post.js']
    self.do_runf('hello_world.c', 'assertions enabled\n4', emcc_args=['-sASSERTIONS=1'])
    self.do_runf('hello_world.c', 'assertions disabled\n4', emcc_args=['-sASSERTIONS=0'])
    self.assertNotContained('#preprocess', read_file('hello_world.js'))

  @with_both_compilers
  def test_use_port_errors(self, compiler):
    stderr = self.expect_fail([compiler, test_file('hello_world.c'), '--use-port=invalid', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))
    self.assertContained('error with `--use-port=invalid` | invalid port name: `invalid`', stderr)
    stderr = self.expect_fail([compiler, test_file('hello_world.c'), '--use-port=sdl2:opt1=v1', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))
    self.assertContained('error with `--use-port=sdl2:opt1=v1` | no options available for port `sdl2`', stderr)
    stderr = self.expect_fail([compiler, test_file('hello_world.c'), '--use-port=sdl2_image:format=jpg', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))
    self.assertContained('error with `--use-port=sdl2_image:format=jpg` | `format` is not supported', stderr)
    stderr = self.expect_fail([compiler, test_file('hello_world.c'), '--use-port=sdl2_image:formats', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))
    self.assertContained('error with `--use-port=sdl2_image:formats` | `formats` is missing a value', stderr)
    stderr = self.expect_fail([compiler, test_file('hello_world.c'), '--use-port=sdl2_image:formats=jpg:formats=png', '-o', 'out.js'])
    self.assertFalse(os.path.exists('out.js'))
    self.assertContained('error with `--use-port=sdl2_image:formats=jpg:formats=png` | duplicate option `formats`', stderr)

  def test_mimalloc_headers(self):
    src = r'''
      #include <mimalloc.h>

      int main() {
          mi_option_enable(mi_option_verbose);
          return 0;
      }
    '''
    self.do_run(src, emcc_args=['-sMALLOC=mimalloc'])

  def test_SUPPORT_BIG_ENDIAN(self):
    # Just a simple build-only test for now
    self.run_process([EMCC, '-sSUPPORT_BIG_ENDIAN', test_file('hello_world.c')])

  @parameterized({
    'noexcept': ['-fno-exceptions'],
    'default': [],
    'except': ['-sDISABLE_EXCEPTION_CATCHING=0'],
    'except_wasm': ['-fwasm-exceptions'],
    'except_wasm_exnref': ['-fwasm-exceptions', '-sWASM_EXNREF']
  })
  def test_std_promise_link(self, *args):
    # Regression test for a bug where std::promise's destructor caused a link
    # error with __cxa_init_primary_exception when no exception argument was
    # given (which defaults to -fignore-exceptions)
    create_file('src.cpp', r'''
      #include <future>
      int main() {
        std::promise<int> p;
        return 0;
      }
    ''')
    self.run_process([EMXX, 'src.cpp', '-pthread'] + list(args))

  def test_stack_protector(self):
    self.do_other_test('test_stack_protector.c', emcc_args=['-fstack-protector'], assert_returncode=NON_ZERO)

  def test_save_temp(self):
    self.run_process([EMCC, '--save-temps', test_file('hello_world.c')])
    self.assertExists('a.out.js')
    # clang itself takes care of creating these three
    self.assertExists('hello_world.i')
    self.assertExists('hello_world.s')
    self.assertExists('hello_world.bc')
    # emcc takes care of creating the .o
    self.assertExists('hello_world.o')

  def test_extra_struct_info(self):
    stderr = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', test_file('other/test_extra_struct_info.js')], stderr=PIPE).stderr
    self.assertContained('(before) AF_INET=2', stderr)
    self.assertContained('(after) AF_INET=42', stderr)

  @also_with_wasmfs
  def test_fs_writev_partial_write(self):
    self.set_setting('FORCE_FILESYSTEM')
    self.do_run_in_out_file_test('fs/test_writev_partial_write.c')

  @requires_v8
  @parameterized({
    '': [[]],
    'O3': [['-O3']]
  })
  def test_fp16(self, opts):
    self.v8_args += ['--experimental-wasm-fp16']
    # TODO Remove this. Liftoff is currently broken for this test.
    # https://chromium-review.googlesource.com/c/v8/v8/+/5842546
    self.v8_args += ['--no-liftoff']
    self.emcc_args = ['-msimd128', '-mfp16', '-sENVIRONMENT=shell'] + opts
    self.do_runf('test_fp16.c')

  def test_embool(self):
    self.do_other_test('test_embool.c')
