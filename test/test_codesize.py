# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.
import gzip
import json
import math
import os
import re
import shlex

import common
from common import (
  RunnerCore,
  compiler_for,
  create_file,
  read_binary,
  read_file,
  test_file,
)
from decorators import node_pthreads, parameterized

from tools import building, shared


def deminify_syms(names, minification_map):
  deminify_map = {}
  for line in read_file(minification_map).splitlines():
    minified_name, name = line.split(':')
    deminify_map[minified_name] = name
  # Include both the original name and the deminified name so that
  # changes to minification are also visible.
  return [f'{name} ({deminify_map[name]})' for name in names]


class codesize(RunnerCore):
  @parameterized({
    'audio_worklet': ('audio_worklet', False, True),
    'hello_world_wasm': ('hello_world', False, True),
    'hello_world_wasm2js': ('hello_world', True, True),
    'random_printf_wasm': ('random_printf', False),
    'random_printf_wasm2js': ('random_printf', True),
    'hello_webgl_wasm': ('hello_webgl', False),
    'hello_webgl_wasm2js': ('hello_webgl', True),
    'hello_webgl2_wasm_singlefile': ('hello_webgl2_wasm_singlefile', False),
    'hello_webgl2_wasm': ('hello_webgl2', False),
    'hello_webgl2_wasm2js': ('hello_webgl2', True),
    'math': ('math', False),
    'hello_wasm_worker': ('hello_wasm_worker', False, True),
    'hello_embind_val': ('embind_val', False),
    'hello_embind': ('embind_hello', False),
  })
  def test_minimal_runtime_code_size(self, test_name, wasm2js, compare_js_output=False):
    smallest_code_size_args = ['-sMINIMAL_RUNTIME=2',
                               '-sENVIRONMENT=web',
                               '-sTEXTDECODER=2',
                               '-sDYNAMIC_EXECUTION=0',
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
                               '--output-eol', 'linux',
                               '-Oz',
                               '--closure=1',
                               '-DNDEBUG',
                               '-ffast-math']

    math_sources = [test_file('codesize/math.c')]
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
    hello_webgl2_wasm_singlefile_sources = hello_webgl2_sources + ['-sSINGLE_FILE']
    hello_wasm_worker_sources = [test_file('wasm_worker/wasm_worker_code_size.c'), '-sWASM_WORKERS', '-sENVIRONMENT=web']
    audio_worklet_sources = [test_file('webaudio/audioworklet.c'), '-sWASM_WORKERS', '-sAUDIO_WORKLET', '-sENVIRONMENT=web', '-sTEXTDECODER=1']
    embind_hello_sources = [test_file('codesize/embind_hello_world.cpp'), '-lembind']
    embind_val_sources = [test_file('codesize/embind_val_hello_world.cpp'), '-lembind']

    sources = {
      'hello_world': hello_world_sources,
      'random_printf': random_printf_sources,
      'hello_webgl': hello_webgl_sources,
      'math': math_sources,
      'hello_webgl2': hello_webgl2_sources,
      'hello_webgl2_wasm_singlefile': hello_webgl2_wasm_singlefile_sources,
      'hello_wasm_worker': hello_wasm_worker_sources,
      'audio_worklet': audio_worklet_sources,
      'embind_val': embind_val_sources,
      'embind_hello': embind_hello_sources,
    }[test_name]

    outputs = ['a.html', 'a.js']
    args = smallest_code_size_args[:]

    if wasm2js:
      args += ['-sWASM=0']
      test_name += '_wasm2js'
    else:
      outputs += ['a.wasm']
      test_name += '_wasm'

    if '-sSINGLE_FILE' in sources:
      outputs = ['a.html']

    args = [compiler_for(sources[0]), '-o', 'a.html'] + args + sources
    print(shlex.join(args))
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
      js_out = test_file('codesize', test_name + '.expected.js')
      terser = shared.get_npm_cmd('terser')
      # N.b. this requires node in PATH, it does not run against NODE from
      # Emscripten config file. If you have this line fail, make sure 'node' is
      # visible in PATH.
      self.run_process(terser + ['-b', 'beautify=true', 'a.js', '-o', 'pretty.js'], env=shared.env_with_node_in_path())
      self.assertFileContents(js_out, read_file('pretty.js'))

    self.check_output_sizes(*outputs)

  def check_output_sizes(self, *outputs: str, metadata=None, skip_gz=False):
    test_name = self.id().split('.')[-1]
    results_file = test_file('codesize', test_name + '.json')

    expected_results: dict = {}
    try:
      expected_results = json.loads(read_file(results_file))
    except Exception:
      if not common.EMTEST_REBASELINE:
        raise

    obtained_results = {}

    def update_and_print_diff(key, actual, expected):
      obtained_results[key] = actual
      diff = actual - expected
      s = f'{key}: size={actual}, expected={expected}'
      if diff:
        s += f', delta={diff} ({diff * 100.0 / expected:+.2f}%)'
      print(s)
      return diff

    total_output_size = 0
    total_expected_size = 0
    total_output_size_gz = 0
    total_expected_size_gz = 0

    for f in outputs:
      contents = read_binary(f)

      size = len(contents)
      expected_size = expected_results.get(f, math.inf)
      if update_and_print_diff(f, size, expected_size) and common.EMTEST_VERBOSE and not common.EMTEST_REBASELINE and f.endswith(('.js', '.html')):
        print(f'Contents of {f}:')
        print(contents.decode('utf-8', errors='replace'))
      total_output_size += size
      total_expected_size += expected_size

      if not skip_gz:
        f_gz = f + '.gz'
        size_gz = len(gzip.compress(contents))
        expected_size_gz = expected_results.get(f_gz, math.inf)
        update_and_print_diff(f_gz, size_gz, expected_size_gz)
        total_output_size_gz += size_gz
        total_expected_size_gz += expected_size_gz

    if len(outputs) > 1:
      update_and_print_diff('total', total_output_size, total_expected_size)
      if not skip_gz:
        update_and_print_diff('total_gz', total_output_size_gz, total_expected_size_gz)

    if metadata:
      obtained_results.update(metadata)

    obtained_results_json = json.dumps(obtained_results, indent=2)
    expected_results_json = json.dumps(expected_results, indent=2)

    if common.EMTEST_REBASELINE:
      create_file(results_file, obtained_results_json + '\n', absolute=True)
    else:
      if total_output_size > total_expected_size:
        print(f'Oops, overall generated code size regressed by {total_output_size - total_expected_size} bytes!')
        print('If this is expected, rerun the test with --rebaseline to update the expected sizes')
      if total_output_size < total_expected_size:
        print(f'Hey amazing, overall generated code size was improved by {total_expected_size - total_output_size} bytes!')
        print('If this is expected, rerun the test with --rebaseline to update the expected sizes')
      self.assertTextDataIdentical(expected_results_json, obtained_results_json)

  def test_unoptimized_code_size(self):
    # We don't care too about unoptimized code size but we would like to keep it
    # under control to a certain extent.  This test allows us to track major
    # changes to the size of the unoptimized and unminified code size.
    # Run with `--rebase` when this test fails.
    self.build('hello_world.c', cflags=['-O0', '--output-eol=linux'])
    self.build('hello_world.c', cflags=['-O0', '--output-eol=linux', '-sASSERTIONS=0'], output_basename='no_asserts')
    self.build('hello_world.c', cflags=['-O0', '--output-eol=linux', '-sSTRICT'], output_basename='strict')

    self.check_output_sizes('hello_world.js', 'hello_world.wasm', 'no_asserts.js', 'no_asserts.wasm', 'strict.js', 'strict.wasm')

  def run_codesize_test(self, filename, cflags, check_funcs=True, check_full_js=False, skip_gz=False):
    # in -Os, -Oz, we remove imports wasm doesn't need
    print('Running codesize test: %s:' % filename, cflags, check_funcs, check_full_js)
    filename = test_file('codesize', filename)
    expected_basename = test_file('codesize', self.id().split('.')[-1])

    # Run once without closure and parse output to find wasmImports
    build_cmd = [compiler_for(filename), filename, '--output-eol=linux', '--emit-minification-map=minify.map'] + cflags + self.get_cflags()
    self.run_process(build_cmd + ['-g2'])
    # find the imports we send from JS
    # TODO(sbc): Find a way to do that that doesn't depend on internal details of
    # the generated code.
    js = read_file('a.out.js')
    if check_full_js:
      # Ignore absolute filenames in the generated code (they are likely /tmp files)
      js = re.sub(r'^// include: .*[/\\].*$', '// include: <FILENAME REPLACED>', js, flags=re.MULTILINE)
      js = re.sub(r'^// end include: .*[/\\].*$', '// end include: <FILENAME REPLACED>', js, flags=re.MULTILINE)
      self.assertFileContents(expected_basename + '.expected.js', js)
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

    outputs = ['a.out.js']
    info = {'sent': sent}

    if '-sSINGLE_FILE' not in cflags:
      # measure the wasm size without the name section
      building.strip('a.out.wasm', 'a.out.nodebug.wasm', sections=['name'])
      outputs.append('a.out.nodebug.wasm')

      imports, exports, funcs = self.parse_wasm('a.out.wasm')
      # Deminify the imports/export lists, if minification occured
      if os.path.exists('minify.map'):
        exports = deminify_syms(exports, 'minify.map')
        imports = [i.split('.', 1)[1] for i in imports]
        imports = deminify_syms(imports, 'minify.map')

      imports.sort()
      info['imports'] = imports

      exports.sort()
      info['exports'] = exports

      if check_funcs:
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

        funcs.sort()
        info['funcs'] = [strip_numeric_suffixes(f) for f in funcs]

    self.check_output_sizes(*outputs, metadata=info, skip_gz=skip_gz)

  @parameterized({
    'O0': ([], True),
    'O1': (['-O1'],),
    'O2': (['-O2'],),
    # in -O3, -Os and -Oz we metadce, and they shrink it down to the minimal output we want
    'O3': (['-O3'],), # noqa
    'Os': (['-Os'],), # noqa
    'Oz': (['-Oz'],), # noqa
    'Os_mr': (['-Os', '-sMINIMAL_RUNTIME'],),
    # EVAL_CTORS also removes the __wasm_call_ctors function
    'Oz-ctors': (['-Oz', '-sEVAL_CTORS'],),
    '64': (['-Oz', '-sMEMORY64'],),
    # WasmFS should not be fully linked into a minimal program.
    'wasmfs': (['-Oz', '-sWASMFS'],),
    'esm': (['-Oz', '-sEXPORT_ES6'],),
  })
  def test_codesize_minimal(self, args, check_full_js=False):
    self.set_setting('STRICT')
    self.cflags.append('--no-entry')
    self.run_codesize_test('minimal.c', args, check_full_js=check_full_js)

  @node_pthreads
  @parameterized({
    '': ([],),
    'memgrowth': (['-sALLOW_MEMORY_GROWTH'],),
  })
  def test_codesize_minimal_pthreads(self, args):
    self.run_codesize_test('minimal_main.c', ['-Oz', '-pthread', '-sPROXY_TO_PTHREAD', '-sSTRICT'] + args)

  @parameterized({
    'noexcept': (['-O2'],), # noqa
    # exceptions increases code size significantly
    'except':   (['-O2', '-fexceptions'],), # noqa
    # exceptions does not pull in demangling by default, which increases code size
    'mangle':   (['-O2', '-fexceptions', '-sEXPORTED_FUNCTIONS=_main,_free,___cxa_demangle', '-Wno-deprecated'],), # noqa
    # Wasm EH's code size increase is smaller than that of Emscripten EH
    'except_wasm': (['-O2', '-fwasm-exceptions', '-sWASM_LEGACY_EXCEPTIONS=0'],),
    'except_wasm_legacy': (['-O2', '-fwasm-exceptions', '-sWASM_LEGACY_EXCEPTIONS'],),
    # eval_ctors 1 can partially optimize, but runs into getenv() for locale
    # code. mode 2 ignores those and fully optimizes out the ctors
    'ctors1':    (['-O2', '-sEVAL_CTORS'],),
    'ctors2':    (['-O2', '-sEVAL_CTORS=2'],),
    'wasmfs':    (['-O2', '-sWASMFS'],),
    'lto':       (['-Oz', '-flto'],),
  })
  def test_codesize_cxx(self, args):
    # do not check functions in this test as there are a lot of libc++ functions
    # pulled in here, and small LLVM backend changes can affect their size and
    # lead to different inlining decisions which add or remove a function
    self.run_codesize_test('hello_libcxx.cpp', args, check_funcs=False)

  @parameterized({
    'O0': ([],),
    'O1': (['-O1'],),
    'O2': (['-O2'],),
    'O3': (['-O3'],), # in -O3, -Os and -Oz we metadce
    'Os': (['-Os'],),
    'Oz': (['-Oz'],),
    # finally, check what happens when we export nothing. wasm should be almost empty
    'export_nothing': (['-Os', '-sEXPORTED_FUNCTIONS=[]'],),
    # we don't metadce with linkable code! other modules may want stuff
    # TODO(sbc): Investivate why the number of exports is order of magnitude
    # larger for wasm backend.
    # This test seems to produce different results under gzip on macOS and Windows machines
    # so skip the gzip size reporting here.
    'dylink_all': (['-O3', '-sMAIN_MODULE'], {'skip_gz': True}),
    'dylink': (['-O3', '-sMAIN_MODULE=2'],),
    # WasmFS should not be fully linked into a hello world program.
    'wasmfs': (['-O3', '-sWASMFS'],),
    'single_file': (['-O3', '-sSINGLE_FILE'],),
  })
  def test_codesize_hello(self, args, kwargs={}): # noqa
    self.run_codesize_test('hello_world.c', args, **kwargs)

  @parameterized({
    'O3':                 ('mem.c', ['-O3']),
    # argc/argv support code etc. is in the wasm
    'O3_standalone':      ('mem.c', ['-O3', '-sSTANDALONE_WASM']),
    # without argc/argv, no support code for them is emitted
    'O3_standalone_narg': ('mem_no_argv.c', ['-O3', '-sSTANDALONE_WASM']),
    # without main, no support code for argc/argv is emitted either
    'O3_standalone_lib':  ('mem_no_main.c', ['-O3', '-sSTANDALONE_WASM', '--no-entry']),
    # Growth support code is in JS, no significant change in the wasm
    'O3_grow':            ('mem.c', ['-O3', '-sALLOW_MEMORY_GROWTH']),
    # Growth support code is in the wasm
    'O3_grow_standalone': ('mem.c', ['-O3', '-sALLOW_MEMORY_GROWTH', '-sSTANDALONE_WASM']),
    # without argc/argv, no support code for them is emitted, even with lto
    'O3_standalone_narg_flto':
                          ('mem_no_argv.c', ['-O3', '-sSTANDALONE_WASM', '-flto']),         # noqa
  })
  def test_codesize_mem(self, filename, args):
    self.run_codesize_test(filename, args)

  @parameterized({
    'O3':            (['-O3'],),
    # argc/argv support code etc. is in the wasm
    'O3_standalone': (['-O3', '-sSTANDALONE_WASM'],),
  })
  def test_codesize_libcxxabi_message(self, args):
    self.run_codesize_test('libcxxabi_message.cpp', args)

  @parameterized({
    'js_fs':  (['-O3', '-sNO_WASMFS'],), # noqa
    'wasmfs': (['-O3', '-sWASMFS'],), # noqa
  })
  def test_codesize_files(self, args):
    self.run_codesize_test('files.cpp', args)

  def test_codesize_file_preload(self):
    create_file('somefile.txt', 'hello')
    self.run_codesize_test('hello_world.c', cflags=['-sSTRICT', '-O3', '--preload-file=somefile.txt'], check_full_js=True)

  def test_small_js_flags(self):
    self.emcc('browser_test_hello_world.c', ['-O3', '--closure=1', '-sINCOMING_MODULE_JS_API=[]', '-sENVIRONMENT=web', '--output-eol=linux'])
    self.check_output_sizes('a.out.js')

  # This test verifies that gzipped binary-encoded a SINGLE_FILE build results in a smaller size
  # than gzipped base64-encoded version.
  def test_binary_encode_is_smaller_than_base64_encode(self):
    self.emcc('hello_world.c', ['-O2', '-sSINGLE_FILE', '-sSINGLE_FILE_BINARY_ENCODE'])
    size_binary_encode = len(gzip.compress(read_binary('a.out.js')))
    self.emcc('hello_world.c', ['-O2', '-sSINGLE_FILE', '-sSINGLE_FILE_BINARY_ENCODE=0'])
    size_base64 = len(gzip.compress(read_binary('a.out.js')))
    print(f'Binary encoded file size: {size_binary_encode}, base64 encoded file size: {size_base64}')
    self.assertLess(size_binary_encode, size_base64)
