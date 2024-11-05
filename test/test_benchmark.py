# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import math
import os
import re
import shutil
import sys
import time
import unittest
import json
import zlib
from pathlib import Path
from typing import List

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: test/runner.py benchmark')

import clang_native
import jsrun
import common
from tools.shared import CLANG_CC, CLANG_CXX
from common import test_file, read_file, read_binary
from tools.shared import run_process, PIPE, EMCC, config
from tools import building, utils, shared

# standard arguments for timing:
# 0: no runtime, just startup
# 1: very little runtime
# 2: 0.5 seconds
# 3: 1 second
# 4: 5 seconds
# 5: 10 seconds
DEFAULT_ARG = '4'

TEST_REPS = 5

# by default, run just core benchmarks
CORE_BENCHMARKS = True
# if a specific benchmark is requested, don't limit to core
if 'benchmark.' in str(sys.argv):
  CORE_BENCHMARKS = False

non_core = unittest.skipIf(CORE_BENCHMARKS, "only running core benchmarks")

OPTIMIZATIONS = '-O3'

PROFILING = 0

LLVM_FEATURE_FLAGS = ['-mnontrapping-fptoint']

# A comma separated list of benchmarkers to run during test_benchmark tests. See
# `named_benchmarkers` for what is available.
EMTEST_BENCHMARKERS = os.getenv('EMTEST_BENCHMARKERS', 'clang,v8,v8-lto,v8-ctors')


class Benchmarker():
  # Whether to record statistics. Set by SizeBenchmarker.
  record_stats = False

  # called when we init the object, which is during startup, even if we are
  # not running benchmarks
  def __init__(self, name):
    self.name = name

  # called when we actually start to run benchmarks
  def prepare(self):
    pass

  def bench(self, args, output_parser=None, reps=TEST_REPS, expected_output=None):
    self.times = []
    self.reps = reps
    for _ in range(reps):
      start = time.time()
      output = self.run(args)
      if expected_output is not None and expected_output not in output:
        raise ValueError('Incorrect benchmark output:\n' + output)

      if not output_parser or args == ['0']: # if arg is 0, we are not running code, and have no output to parse
        curr = time.time() - start
      else:
        try:
          curr = output_parser(output)
        except Exception as e:
          print(str(e))
          print('Parsing benchmark results failed, output was: ' + output)
      self.times.append(curr)

  def display(self, baseline=None):
    # speed

    if self.times:
      if baseline == self:
        baseline = None
      mean = sum(self.times) / len(self.times)
      squared_times = [x * x for x in self.times]
      mean_of_squared = sum(squared_times) / len(self.times)
      std = math.sqrt(mean_of_squared - mean * mean)
      sorted_times = sorted(self.times)
      count = len(sorted_times)
      if count % 2 == 0:
        median = sum(sorted_times[count // 2 - 1:count // 2 + 1]) / 2
      else:
        median = sorted_times[count // 2]

      print('   %10s: mean: %4.3f (+-%4.3f) secs  median: %4.3f  range: %4.3f-%4.3f  (noise: %4.3f%%)  (%d runs)' % (self.name, mean, std, median, min(self.times), max(self.times), 100 * std / mean, self.reps), end=' ')

      if baseline:
        mean_baseline = sum(baseline.times) / len(baseline.times)
        final = mean / mean_baseline
        print('  Relative: %.2f X slower' % final)
      else:
        print()

    # size

    recorded_stats = []

    def add_stat(name, size, gzip_size):
      recorded_stats.append({
        'value': name,
        'measurement': size,
      })
      recorded_stats.append({
        'value': name + ' (gzipped)',
        'measurement': gzip_size,
      })

    total_size = 0
    total_gzip_size = 0

    for file in self.get_output_files():
      size = os.path.getsize(file)
      gzip_size = len(zlib.compress(read_binary(file)))
      if self.record_stats:
        add_stat(utils.removeprefix(os.path.basename(file), 'size_'), size, gzip_size)
      total_size += size
      total_gzip_size += gzip_size

    if self.record_stats:
      add_stat('total', total_size, total_gzip_size)

    print('        size: %8s, compressed: %8s' % (total_size, total_gzip_size), end=' ')
    if self.get_size_text():
      print('  (' + self.get_size_text() + ')', end=' ')
    print()

    return recorded_stats

  def get_size_text(self):
    return ''


class NativeBenchmarker(Benchmarker):
  def __init__(self, name, cc, cxx, args=None):
    self.name = name
    self.cc = cc
    self.cxx = cxx
    self.args = args or [OPTIMIZATIONS]

  def build(self, parent, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser):
    native_args = native_args or []
    shared_args = shared_args or []
    self.parent = parent
    if lib_builder:
      env = {'CC': self.cc, 'CXX': self.cxx, 'CXXFLAGS': "-Wno-c++11-narrowing"}
      env.update(clang_native.get_clang_native_env())
      native_args = native_args + lib_builder(self.name, native=True, env_init=env)
    if not native_exec:
      compiler = self.cxx if filename.endswith('cpp') else self.cc
      cmd = compiler + [
        '-fno-math-errno',
        filename,
        '-o', filename + '.native'
      ] + self.args + shared_args + native_args + clang_native.get_clang_native_args()
      # print(cmd)
      run_process(cmd, env=clang_native.get_clang_native_env())
    else:
      shutil.copyfile(native_exec, filename + '.native')
      shutil.copymode(native_exec, filename + '.native')

    final = os.path.dirname(filename) + os.path.sep + self.name + '_' + os.path.basename(filename) + '.native'
    shutil.move(filename + '.native', final)
    self.filename = final

  def run(self, args):
    return run_process([self.filename] + args, stdout=PIPE, stderr=PIPE, check=False).stdout

  def get_output_files(self):
    return [self.filename]

  def get_size_text(self):
    return 'dynamically linked - libc etc. are not included!'


def run_binaryen_opts(filename, opts):
  run_process([
    os.path.join(building.get_binaryen_bin(), 'wasm-opt', '--all-features'),
    filename,
    '-o', filename
  ] + opts)


class EmscriptenBenchmarker(Benchmarker):
  def __init__(self, name, engine, extra_args=None, env=None, binaryen_opts=None):
    self.name = name
    self.engine = engine
    self.extra_args = extra_args or []
    self.env = os.environ.copy()
    if env:
      self.env.update(env)
    self.binaryen_opts = binaryen_opts or []

  def build(self, parent, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser):
    emcc_args = emcc_args or []
    self.filename = filename
    llvm_root = self.env.get('LLVM') or config.LLVM_ROOT
    if lib_builder:
      env_init = self.env.copy()
      # Note that we need to pass in all the flags here because some build
      # systems (like zlib) if they see a CFLAGS it will override all their
      # default flags, including optimizations.
      env_init['CFLAGS'] = ' '.join(LLVM_FEATURE_FLAGS + [OPTIMIZATIONS] + self.extra_args)
      # This shouldn't be 'emcc_args += ...', because emcc_args is passed in as
      # a parameter and changes will be visible to the caller.
      emcc_args = emcc_args + lib_builder('js_' + llvm_root, native=False, env_init=env_init)
    final = os.path.dirname(filename) + os.path.sep + self.name + ('_' if self.name else '') + os.path.basename(filename) + '.js'
    final = final.replace('.cpp', '')
    utils.delete_file(final)
    cmd = [
      EMCC, filename,
      OPTIMIZATIONS,
      '-sINITIAL_MEMORY=256MB',
      '-sENVIRONMENT=node,shell',
      '-o', final
    ] + LLVM_FEATURE_FLAGS
    if shared_args:
      cmd += shared_args
    if PROFILING:
      cmd += ['--profiling']
    else:
      cmd += ['--closure=1', '-sMINIMAL_RUNTIME']
    # add additional emcc args at the end, which may override other things
    # above, such as minimal runtime
    cmd += emcc_args + self.extra_args
    if '-sFILESYSTEM' not in cmd and '-sFORCE_FILESYSTEM' not in cmd:
      cmd += ['-sFILESYSTEM=0']
    self.cmd = cmd
    run_process(cmd, env=self.env)
    if self.binaryen_opts:
      run_binaryen_opts(shared.replace_suffix(final, '.wasm'), self.binaryen_opts)
    self.filename = final

  def run(self, args):
    return jsrun.run_js(self.filename, engine=self.engine, args=args, stderr=PIPE)

  def get_output_files(self):
    ret = [self.filename]
    if 'WASM=0' in self.cmd:
      if 'MINIMAL_RUNTIME=0' not in self.cmd:
        ret.append(shared.replace_suffix(self.filename, '.asm.js'))
        ret.append(shared.replace_suffix(self.filename, '.mem'))
      else:
        ret.append(self.filename + '.mem')
    else:
      ret.append(shared.replace_suffix(self.filename, '.wasm'))
    return ret


# This benchmarker will make a test benchmark build with Emscripten and record
# the file output sizes in out/test/stats.json. The file format is specified at
# https://skia.googlesource.com/buildbot/+/refs/heads/main/perf/FORMAT.md
# Running the benchmark will be skipped.
class SizeBenchmarker(EmscriptenBenchmarker):
  record_stats = True

  def __init__(self, name):
    # do not set an engine, as we will not run the code
    super().__init__(name, engine=None)

  # we will not actually run the benchmarks
  run = None


CHEERP_BIN = '/opt/cheerp/bin/'


class CheerpBenchmarker(Benchmarker):
  def __init__(self, name, engine, args=None, binaryen_opts=None):
    self.name = name
    self.engine = engine
    self.args = args or [OPTIMIZATIONS]
    self.binaryen_opts = binaryen_opts or []

  def build(self, parent, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser):
    cheerp_args = [
      '-fno-math-errno',
    ]
    cheerp_args += self.args
    self.parent = parent
    if lib_builder:
      # build as "native" (so no emcc env stuff), but with all the cheerp stuff
      # set in the env
      cheerp_args = cheerp_args + lib_builder(self.name, native=True, env_init={
        'CC': CHEERP_BIN + 'clang',
        'CXX': CHEERP_BIN + 'clang++',
        'AR': CHEERP_BIN + '../libexec/cheerp-unknown-none-ar',
        'LD': CHEERP_BIN + 'clang',
        'NM': CHEERP_BIN + 'llvm-nm',
        'LDSHARED': CHEERP_BIN + 'clang',
        'RANLIB': CHEERP_BIN + '../libexec/cheerp-unknown-none-ranlib',
        'CXXFLAGS': "-Wno-c++11-narrowing",
        'CHEERP_PREFIX': CHEERP_BIN + '../',
      })
    if PROFILING:
      cheerp_args += ['-cheerp-pretty-code'] # get function names, like emcc --profiling
    final = os.path.dirname(filename) + os.path.sep + self.name + ('_' if self.name else '') + os.path.basename(filename) + '.js'
    final = final.replace('.cpp', '')
    utils.delete_file(final)
    dirs_to_delete = []
    cheerp_args += ['-cheerp-preexecute']
    try:
      # print(cheerp_args)
      if filename.endswith('.c'):
        compiler = CHEERP_BIN + '/clang'
      else:
        compiler = CHEERP_BIN + '/clang++'
      cmd = [compiler] + cheerp_args + [
        '-cheerp-linear-heap-size=256',
        '-cheerp-secondary-output-file=' + final.replace('.js', '.wasm'),
        filename,
        '-o', final
      ] + shared_args
      # print(' '.join(cmd))
      run_process(cmd, stdout=PIPE, stderr=PIPE)
      self.filename = final
      if self.binaryen_opts:
        run_binaryen_opts(final.replace('.js', '.wasm'), self.binaryen_opts)
    finally:
      for dir_ in dirs_to_delete:
        utils.delete_dir(dir_)

  def run(self, args):
    return jsrun.run_js(self.filename, engine=self.engine, args=args, stderr=PIPE)

  def get_output_files(self):
    return [self.filename, shared.replace_suffix(self.filename, '.wasm')]


# Benchmarkers

benchmarkers: List[Benchmarker] = []

# avoid the baseline compiler running, because it adds a lot of noise
# (the nondeterministic time it takes to get to the full compiler ends up
# mattering as much as the actual benchmark)
aot_v8 = (config.V8_ENGINE if config.V8_ENGINE else []) + ['--no-liftoff']

named_benchmarkers = {
  'clang': NativeBenchmarker('clang', [CLANG_CC], [CLANG_CXX]),
  'gcc': NativeBenchmarker('gcc',   ['gcc', '-no-pie'],  ['g++', '-no-pie']),
  'size': SizeBenchmarker('size'),
  'v8': EmscriptenBenchmarker('v8', aot_v8),
  'v8-lto': EmscriptenBenchmarker('v8-lto', aot_v8, ['-flto']),
  'v8-ctors': EmscriptenBenchmarker('v8-ctors', aot_v8, ['-sEVAL_CTORS']),
  'v8-64': EmscriptenBenchmarker('v8-64', aot_v8, ['-sMEMORY64=2']),
  'node': EmscriptenBenchmarker('node', config.NODE_JS),
  'node-64': EmscriptenBenchmarker('node-64', config.NODE_JS, ['-sMEMORY64=2']),
  'cherp-v8': CheerpBenchmarker('cheerp-v8-wasm', aot_v8),
  # TODO: ensure no baseline compiler is used, see v8
  'sm': EmscriptenBenchmarker('sm', config.SPIDERMONKEY_ENGINE),
  'cherp-sm': CheerpBenchmarker('cheerp-sm-wasm', config.SPIDERMONKEY_ENGINE)
}

for name in EMTEST_BENCHMARKERS.split(','):
  if name not in named_benchmarkers:
    raise Exception('error, unknown benchmarker ' + name)
  benchmarkers.append(named_benchmarkers[name])


class benchmark(common.RunnerCore):
  save_dir = True
  stats = [] # type: ignore

  @classmethod
  def setUpClass(cls):
    super().setUpClass()

    for benchmarker in benchmarkers:
      benchmarker.prepare()

    fingerprint = ['including compilation', time.asctime()]
    try:
      fingerprint.append('em: ' + run_process(['git', 'show'], stdout=PIPE).stdout.splitlines()[0])
    except Exception:
      pass
    try:
      with common.chdir(os.path.expanduser('~/Dev/mozilla-central')):
        fingerprint.append('sm: ' + [line for line in run_process(['hg', 'tip'], stdout=PIPE).stdout.splitlines() if 'changeset' in line][0])
    except Exception:
      pass
    fingerprint.append('llvm: ' + config.LLVM_ROOT)
    print('Running Emscripten benchmarks... [ %s ]' % ' | '.join(fingerprint))

  @classmethod
  def tearDownClass(cls):
    super().tearDownClass()
    if cls.stats:
      output = {
        'version': 1,
        'git_hash': '',
        'results': cls.stats
      }
      utils.write_file('stats.json', json.dumps(output, indent=2) + '\n')

  # avoid depending on argument reception from the commandline
  def hardcode_arguments(self, code):
    if not code or 'int main()' in code:
      return code
    main_pattern = 'int main(int argc, char **argv)'
    assert main_pattern in code
    code = code.replace(main_pattern, 'int benchmark_main(int argc, char **argv)')
    code += '''
      int main() {
        int newArgc = 2;
        char* newArgv[] = { (char*)"./program.exe", (char*)"%s" };
        int ret = benchmark_main(newArgc, newArgv);
        return ret;
      }
    ''' % DEFAULT_ARG
    return code

  def do_benchmark(self, name, src, expected_output='FAIL', args=None,
                   emcc_args=None, native_args=None, shared_args=None,
                   force_c=False, reps=TEST_REPS, native_exec=None,
                   output_parser=None, args_processor=None, lib_builder=None,
                   skip_native=False):
    if not benchmarkers:
      raise Exception('error, no benchmarkers')

    args = args or [DEFAULT_ARG]
    if args_processor:
      args = args_processor(args)

    dirname = self.get_dir()
    filename = os.path.join(dirname, name + '.c' + ('' if force_c else 'pp'))
    src = self.hardcode_arguments(src)
    utils.write_file(filename, src)

    print()
    baseline = None
    for b in benchmarkers:
      if skip_native and isinstance(b, NativeBenchmarker):
        continue
      if not b.run:
        # If we won't run the benchmark, we don't need repetitions.
        reps = 0
      baseline = b
      print('Running benchmarker: %s: %s' % (b.__class__.__name__, b.name))
      b.build(self, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser=output_parser is not None)
      b.bench(args, output_parser, reps, expected_output)
      recorded_stats = b.display(baseline)
      if recorded_stats:
        self.add_stats(name, recorded_stats)

  def add_stats(self, name, stats):
    self.stats.append({
      'key': {
        'test': name,
        'units': 'bytes'
      },
      'measurements': {
        'stats': stats
      }
    })

  def test_primes(self, check=True):
    src = r'''
      #include <stdio.h>
      #include <math.h>
      int main(int argc, char **argv) {
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: arg = 33000; break;
          case 2: arg = 130000; break;
          case 3: arg = 220000; break;
          case 4: arg = 610000; break;
          case 5: arg = 1010000; break;
          default:
#ifdef CHECK
            printf("error: %d\\n", arg);
#endif
            return -1;
        }

        int primes = 0, curri = 2;
        while (primes < arg) {
          int ok = true;
          for (int j = 2; j < sqrtf(curri); j++) {
            if (curri % j == 0) {
              ok = false;
              break;
            }
          }
          if (ok) {
            primes++;
          }
          curri++;
        }
#ifdef CHECK
        printf("lastprime: %d.\n", curri-1);
#endif
        return 0;
      }
    '''
    self.do_benchmark('primes' if check else 'primes-nocheck', src, 'lastprime:' if check else '', shared_args=['-DCHECK'] if check else [])

  # Also interesting to test it without the printfs which allow checking the output. Without
  # printf, code size is dominated by the runtime itself (the compiled code is just a few lines).
  def test_primes_nocheck(self):
    self.test_primes(check=False)

  def test_memops(self):
    src = '''
      #include <stdio.h>
      #include <string.h>
      #include <stdlib.h>
      int main(int argc, char **argv) {
        int N, M;
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: N = 1024*1024; M = 55; break;
          case 2: N = 1024*1024; M = 400; break;
          case 3: N = 1024*1024; M = 800; break;
          case 4: N = 1024*1024; M = 4000; break;
          case 5: N = 1024*1024; M = 8000; break;
          default: printf("error: %d\\n", arg); return -1;
        }

        int final = 0;
        char *buf = (char*)malloc(N);
        for (int t = 0; t < M; t++) {
          for (int i = 0; i < N; i++)
            buf[i] = (i + final)%256;
          for (int i = 0; i < N; i++)
            final += buf[i] & 1;
          final = final % 1000;
        }
        printf("final: %d.\\n", final);
        return 0;
      }
    '''
    self.do_benchmark('memops', src, 'final:')

  @non_core
  def test_files(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <assert.h>
      #include <unistd.h>

      int main(int argc, char **argv) {
        int N = 100;
        int M = 1000;
        int K = 1000;
        unsigned char *k = (unsigned char*)malloc(K+1), *k2 = (unsigned char*)malloc(K+1);
        for (int i = 0; i < K; i++) {
          k[i] = (i % 250) + 1;
        }
        k[K] = 0;
        char buf[100];
        for (int i = 0; i < N; i++) {
          sprintf(buf, "/dev/shm/file-%d.dat", i);
          FILE *f = fopen(buf, "w");
          for (int j = 0; j < M; j++) {
            fwrite(k, 1, (j % K) + 1, f);
          }
          fclose(f);
        }
        for (int i = 0; i < N; i++) {
          sprintf(buf, "/dev/shm/file-%d.dat", i);
          FILE *f = fopen(buf, "r");
          for (int j = 0; j < M; j++) {
            fread(k2, 1, (j % K) + 1, f);
          }
          fclose(f);
          for (int j = 0; j < K; j++) {
            assert(k[j] == k2[j]);
          }
          unlink(buf);
        }
        printf("ok");
        return 0;
      }
    '''
    self.do_benchmark('files', src, 'ok', emcc_args=['-sFILESYSTEM', '-sMINIMAL_RUNTIME=0', '-sEXIT_RUNTIME'])

  def test_copy(self):
    src = r'''
      #include <stdio.h>
      struct vec {
        int x, y, z;
        int r, g, b;
        vec(int x_, int y_, int z_, int r_, int g_, int b_) : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}
        static vec add(vec a, vec b) {
          return vec(a.x+b.x, a.y+b.y, a.z+b.z, a.r+b.r, a.g+b.g, a.b+b.b);
        }
        void norm() {
          x %= 1024;
          y %= 1024;
          z %= 1024;
          r %= 1024;
          b %= 1024;
          g %= 1024;
        }
        int sum() { return x + y + z + r + g + b; }
      };
      int main(int argc, char **argv) {
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: arg = 75; break;
          case 2: arg = 625; break;
          case 3: arg = 1250; break;
          case 4: arg = 5*1250; break;
          case 5: arg = 10*1250; break;
          default: printf("error: %d\\n", arg); return -1;
        }

        int total = 0;
        for (int i = 0; i < arg; i++) {
          for (int j = 0; j < 50000; j++) {
            vec c(i, i+i%10, j*2, i%255, j%120, i%15);
            vec d(j+i%10, j*2, j%255, i%120, j%15, j);
            vec e = c;
            c.norm();
            d.norm();
            vec f = vec::add(c, d);
            f = vec::add(e, f);
            f.norm();
            f = vec::add(d, f);
            total += f.sum() % 100;
            total %= 10240;
          }
        }
        printf("sum:%d\n", total);
        return 0;
      }
    '''
    self.do_benchmark('copy', src, 'sum:')

  def test_ifs(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      volatile int x = 0;

      __attribute__ ((noinline)) int calc() {
        return (x++) & 16384;
      }

      int main(int argc, char **argv) {
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: arg = 5*75; break;
          case 2: arg = 5*625; break;
          case 3: arg = 5*1250; break;
          case 4: arg = 5*5*1250; break;
          case 5: arg = 5*10*1250; break;
          default: printf("error: %d\\n", arg); return -1;
        }

        int sum = 0;

        for (int j = 0; j < 27000; j++) {
          for (int i = 0; i < arg; i++) {
            if (calc() && calc()) {
              sum += 17;
            } else {
              sum += 19;
            }
            if (calc() || calc()) {
              sum += 23;
            }
          }
        }

        printf("ok %d\n", sum);

        return 0;
      }
    '''
    self.do_benchmark('ifs', src, 'ok')

  def test_conditionals(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main(int argc, char **argv) {
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: arg = 3*75; break;
          case 2: arg = 3*625; break;
          case 3: arg = 3*1250; break;
          case 4: arg = 3*5*1250; break;
          case 5: arg = 3*10*1250; break;
          default: printf("error: %d\\n", arg); return -1;
        }

        int x = 0;

        for (int j = 0; j < 27000; j++) {
          for (int i = 0; i < arg; i++) {
            if (((x*x+11) % 3 == 0) | ((x*(x+2)+17) % 5 == 0)) {
              x += 2;
            } else {
              x++;
            }
          }
        }

        printf("ok %d\n", x);

        return 0;
      }
    '''
    self.do_benchmark('conditionals', src, 'ok', reps=TEST_REPS)

  def test_fannkuch(self):
    src = read_file(test_file('fannkuch.cpp')).replace(
      'int n = argc > 1 ? atoi(argv[1]) : 0;',
      '''
        int n;
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: n = 9; break;
          case 2: n = 10; break;
          case 3: n = 11; break;
          case 4: n = 11; break;
          case 5: n = 12; break;
          default: printf("error: %d\\n", arg); return -1;
        }
      '''
    )
    assert 'switch(arg)' in src
    self.do_benchmark('fannkuch', src, 'Pfannkuchen(')

  def test_corrections(self):
    src = r'''
      #include <stdio.h>
      #include <math.h>
      int main(int argc, char **argv) {
        int N, M;
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: N = 20000; M = 550; break;
          case 2: N = 20000; M = 3500; break;
          case 3: N = 20000; M = 7000; break;
          case 4: N = 20000; M = 5*7000; break;
          case 5: N = 20000; M = 10*7000; break;
          default: printf("error: %d\\n", arg); return -1;
        }

        unsigned int f = 0;
        unsigned short s = 0;
        for (int t = 0; t < M; t++) {
          for (int i = 0; i < N; i++) {
            f += i / ((t % 5)+1);
            if (f > 1000) f /= (t % 3)+1;
            if (i % 4 == 0) f += i * (i % 8 == 0 ? 1 : -1);
            s += (short(f)*short(f)) % 256;
          }
        }
        printf("final: %d:%d.\n", f, s);
        return 0;
      }
    '''
    self.do_benchmark('corrections', src, 'final:')

  def test_corrections64(self):
    src = r'''
      #include <stdio.h>
      #include <math.h>
      #include <stdint.h>
      int main(int argc, char **argv) {
        int64_t N, M;
        int arg = argc > 1 ? argv[1][0] - '0' : 3;
        switch(arg) {
          case 0: return 0; break;
          case 1: N = 8000; M = 550; break;
          case 2: N = 8000; M = 3500; break;
          case 3: N = 8000; M = 7000; break;
          case 4: N = 8000; M = 5*7000; break;
          case 5: N = 8000; M = 10*7000; break;
          default: printf("error: %d\\n", arg); return -1;
        }

        uint64_t f = 0;
        uint32_t s = 0;
        for (int64_t t = 0; t < M; t++) {
          for (int64_t i = 0; i < N; i++) {
            f += i / ((t % 5)+1);
            if (f > 1000) f /= (t % 3)+1;
            if (i % 4 == 0) f += i * (i % 8 == 0 ? 1 : -1);
            s += (short(f)*short(f)) % 256;
          }
        }
        printf("final: %lld:%d.\n", f, s);
        return 0;
      }
    '''
    self.do_benchmark('corrections64', src, 'final:')

  def fasta(self, name, double_rep):
    src = read_file(test_file('fasta.cpp')).replace('double', double_rep)
    src = src.replace('   const size_t n = ( argc > 1 ) ? atoi( argv[1] ) : 512;', '''
      int n;
      int arg = argc > 1 ? argv[1][0] - '0' : 3;
      switch(arg) {
        case 0: return 0; break;
        case 1: n = 19000000/20; break;
        case 2: n = 19000000/2; break;
        case 3: n = 19000000; break;
        case 4: n = 19000000*5; break;
        case 5: n = 19000000*10; break;
        default: printf("error: %d\\n", arg); return -1;
      }
    ''')
    assert 'switch(arg)' in src
    self.do_benchmark('fasta', src, '')

  def test_fasta_float(self):
    self.fasta('fasta_float', 'float')

  @non_core
  def test_fasta_double(self):
    self.fasta('fasta_double', 'double')

  def test_skinning(self):
    src = read_file(test_file('skinning_test_no_simd.cpp'))
    self.do_benchmark('skinning', src, 'blah=0.000000')

  def test_havlak(self):
    src = read_file(test_file('havlak.cpp'))
    # This runs many recursive calls (DFS) and thus needs a larger stack
    self.do_benchmark('havlak', src, 'Found', shared_args=['-std=c++11'],
                      emcc_args=['-sSTACK_SIZE=1MB'])

  def test_base64(self):
    src = read_file(test_file('base64.c'))
    self.do_benchmark('base64', src, 'decode')

  @non_core
  def test_life(self):
    src = read_file(test_file('life.c'))
    self.do_benchmark('life', src, '''--------------------------------''', force_c=True)

  def test_zzz_linpack(self):
    def output_parser(output):
      mflops = re.search(r'Unrolled Double  Precision ([\d\.]+) Mflops', output).group(1)
      return 10000.0 / float(mflops)
    self.do_benchmark('linpack_double', read_file(test_file('benchmark/linpack2.c')), '''Unrolled Double  Precision''', force_c=True, output_parser=output_parser)

  # Benchmarks the synthetic performance of calling native functions.
  @non_core
  def test_native_functions(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('native_functions', read_file(test_file('benchmark/benchmark_ffis.cpp')), 'Total time:',
                      output_parser=output_parser,
                      # Not minimal because this uses functions in library_browsers.js
                      emcc_args=['-sMINIMAL_RUNTIME=0'],
                      shared_args=['-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  # Benchmarks the synthetic performance of calling function pointers.
  @non_core
  def test_native_function_pointers(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('native_functions', read_file(test_file('benchmark/benchmark_ffis.cpp')), 'Total time:',
                      output_parser=output_parser,
                      # Not minimal because this uses functions in library_browsers.js
                      emcc_args=['-sMINIMAL_RUNTIME=0'],
                      shared_args=['-DBENCHMARK_FUNCTION_POINTER=1', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  # Benchmarks the synthetic performance of calling "foreign" JavaScript functions.
  @non_core
  def test_foreign_functions(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('foreign_functions', read_file(test_file('benchmark/benchmark_ffis.cpp')), 'Total time:',
                      output_parser=output_parser,
                      # Not minimal because this uses functions in library_browsers.js
                      emcc_args=['--js-library', test_file('benchmark/benchmark_ffis.js'), '-sMINIMAL_RUNTIME=0'],
                      shared_args=['-DBENCHMARK_FOREIGN_FUNCTION=1', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memcpy_128b(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_128b', read_file(test_file('benchmark/benchmark_memcpy.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMAX_COPY=128', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memcpy_4k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_4k', read_file(test_file('benchmark/benchmark_memcpy.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=128', '-DMAX_COPY=4096', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memcpy_16k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_16k', read_file(test_file('benchmark/benchmark_memcpy.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=4096', '-DMAX_COPY=16384', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memcpy_1mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_1mb', read_file(test_file('benchmark/benchmark_memcpy.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=16384', '-DMAX_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memcpy_16mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_16mb', read_file(test_file('benchmark/benchmark_memcpy.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memset_128b(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_128b', read_file(test_file('benchmark/benchmark_memset.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMAX_COPY=128', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memset_4k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_4k', read_file(test_file('benchmark/benchmark_memset.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=128', '-DMAX_COPY=4096', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memset_16k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_16k', read_file(test_file('benchmark/benchmark_memset.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=4096', '-DMAX_COPY=16384', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memset_1mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_1mb', read_file(test_file('benchmark/benchmark_memset.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=16384', '-DMAX_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  @non_core
  def test_memset_16mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_16mb', read_file(test_file('benchmark/benchmark_memset.cpp')), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + test_file('benchmark')])

  def test_malloc_multithreading(self):
    # Multithreaded malloc test. For emcc we use mimalloc here.
    src = read_file(test_file('other/test_malloc_multithreading.cpp'))
    # TODO measure with different numbers of cores and not fixed 4
    self.do_benchmark('malloc_multithreading', src, 'Done.', shared_args=['-DWORKERS=4', '-pthread'], emcc_args=['-sEXIT_RUNTIME', '-sMALLOC=mimalloc'])

  def test_matrix_multiply(self):
    def output_parser(output):
      return float(re.search(r'Total elapsed: ([\d\.]+)', output).group(1))
    self.do_benchmark('matrix_multiply', read_file(test_file('matrix_multiply.cpp')), 'Total elapsed:', output_parser=output_parser, shared_args=['-I' + test_file('benchmark')])

  def lua(self, benchmark, expected, output_parser=None, args_processor=None):
    self.emcc_args.remove('-Werror')
    shutil.copyfile(test_file(f'third_party/lua/{benchmark}.lua'), benchmark + '.lua')

    def lib_builder(name, native, env_init):
      # We force recomputation for the native benchmarker because this benchmark
      # uses native_exec=True, so we need to copy the native executable
      return self.get_library(os.path.join('third_party', 'lua_native' if native else 'lua'), [os.path.join('src', 'lua.o'), os.path.join('src', 'liblua.a')], make=['make', 'generic'], configure=None, native=native, cache_name_extra=name, env_init=env_init, force_rebuild=native)

    self.do_benchmark('lua_' + benchmark, '', expected,
                      force_c=True, args=[benchmark + '.lua', DEFAULT_ARG],
                      emcc_args=['--embed-file', benchmark + '.lua', '-sFORCE_FILESYSTEM', '-sMINIMAL_RUNTIME=0'], # not minimal because of files
                      lib_builder=lib_builder, native_exec=os.path.join('building', 'third_party', 'lua_native', 'src', 'lua'),
                      output_parser=output_parser, args_processor=args_processor)

  def test_zzz_lua_scimark(self):
    def output_parser(output):
      return 100.0 / float(re.search(r'\nSciMark +([\d\.]+) ', output).group(1))

    self.lua('scimark', '[small problem sizes]', output_parser=output_parser)

  def test_zzz_lua_binarytrees(self):
    # js version: ['binarytrees.lua', {0: 0, 1: 9.5, 2: 11.99, 3: 12.85, 4: 14.72, 5: 15.82}[arguments[0]]]
    self.lua('binarytrees', 'long lived tree of depth')

  def test_zzz_zlib(self):
    self.emcc_args.remove('-Werror')
    src = read_file(test_file('benchmark/test_zlib_benchmark.c'))

    def lib_builder(name, native, env_init):
      return self.get_library(os.path.join('third_party', 'zlib'), os.path.join('libz.a'), make_args=['libz.a'], native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('zlib', src, 'ok.',
                      force_c=True, shared_args=['-I' + test_file('third_party/zlib')], lib_builder=lib_builder)

  def test_zzz_coremark(self):
    src = read_file(test_file('third_party/coremark/core_main.c'))

    def lib_builder(name, native, env_init):
      return self.get_library('third_party/coremark', [os.path.join('coremark.a')], configure=None, native=native, cache_name_extra=name, env_init=env_init)

    def output_parser(output):
      iters_sec = re.search(r'Iterations/Sec   : ([\d\.]+)', output).group(1)
      return 100000.0 / float(iters_sec)

    self.do_benchmark('coremark', src, 'Correct operation validated.', shared_args=['-I' + test_file('third_party/coremark')], lib_builder=lib_builder, output_parser=output_parser, force_c=True)

  def test_zzz_box2d(self):
    src = read_file(test_file('benchmark/test_box2d_benchmark.cpp'))

    def lib_builder(name, native, env_init):
      return self.get_library(os.path.join('third_party', 'box2d'), ['box2d.a'], configure=None, native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('box2d', src, 'frame averages', shared_args=['-I' + test_file('third_party/box2d')], lib_builder=lib_builder)

  def test_zzz_bullet(self):
    self.emcc_args.remove('-Werror')
    self.emcc_args += ['-Wno-c++11-narrowing', '-Wno-deprecated-register', '-Wno-writable-strings']
    src = read_file(test_file('third_party/bullet/Demos/Benchmarks/BenchmarkDemo.cpp'))
    src += read_file(test_file('third_party/bullet/Demos/Benchmarks/main.cpp'))

    def lib_builder(name, native, env_init):
      return self.get_library(str(Path('third_party/bullet')),
                              [Path('src/.libs/libBulletDynamics.a'),
                               Path('src/.libs/libBulletCollision.a'),
                               Path('src/.libs/libLinearMath.a')],
                              # The --host parameter is needed for 2 reasons:
                              # 1) bullet in it's configure.ac tries to do platform detection and will fail on unknown platforms
                              # 2) configure will try to compile and run a test file to check if the C compiler is sane. As Cheerp
                              #    will generate a wasm file (which cannot be run), configure will fail. Passing `--host` enables
                              #    cross compile mode, which lets configure complete happily.
                              configure_args=['--disable-demos', '--disable-dependency-tracking', '--host=i686-unknown-linux'], native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('bullet', src, '\nok.\n',
                      shared_args=['-I' + test_file('third_party/bullet/src'), '-I' + test_file('third_party/bullet/Demos/Benchmarks')],
                      lib_builder=lib_builder)

  def test_zzz_lzma(self):
    src = read_file(test_file('benchmark/test_lzma_benchmark.c'))

    def lib_builder(name, native, env_init):
      return self.get_library(os.path.join('third_party', 'lzma'), [os.path.join('lzma.a')], configure=None, native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('lzma', src, 'ok.', shared_args=['-I' + test_file('third_party/lzma')], lib_builder=lib_builder)

  def test_zzz_sqlite(self):
    src = read_file(test_file('third_party/sqlite/sqlite3.c')) + read_file(test_file('sqlite/speedtest1.c'))
    self.do_benchmark('sqlite', src, 'TOTAL...',
                      native_args=['-ldl', '-pthread'],
                      shared_args=['-I' + test_file('third_party/sqlite')],
                      # not minimal because of files
                      emcc_args=['-sFILESYSTEM', '-sMINIMAL_RUNTIME=0'],
                      force_c=True)

  def test_zzz_poppler(self):
    utils.write_file('pre.js', '''
      var benchmarkArgument = %s;
      var benchmarkArgumentToPageCount = {
        '0': 0,
        '1': 1,
        '2': 5,
        '3': 15,
        '4': 26,
        '5': 55,
      };
      if (benchmarkArgument === 0) {
        Module['arguments'] = ['-?'];
        Module['printErr'] = function(){};
      } else {
        // Add 'filename' after 'input.pdf' to write the output so it can be verified.
        Module['arguments'] = ['-scale-to', '1024', 'input.pdf',  '-f', '1', '-l', '' + benchmarkArgumentToPageCount[benchmarkArgument]];
        Module['postRun'] = function() {
          var files = [];
          for (var x in FS.root.contents) {
            if (x.startsWith('filename-')) {
              files.push(x);
            }
          }
          files.sort();
          var hash = 5381;
          var totalSize = 0;
          files.forEach(function(file) {
            var data = Array.from(MEMFS.getFileDataAsTypedArray(FS.root.contents[file]));
            for (var i = 0; i < data.length; i++) {
              hash = ((hash << 5) + hash) ^ (data[i] & 0xff);
            }
            totalSize += data.length;
          });
          out(files.length + ' files emitted, total output size: ' + totalSize + ', hashed printout: ' + hash);
        };
      }
    ''' % DEFAULT_ARG)

    def lib_builder(name, native, env_init):
      return self.get_poppler_library(env_init=env_init)

    # TODO: Fix poppler native build and remove skip_native=True
    self.do_benchmark('poppler', '', 'hashed printout',
                      shared_args=['-I' + test_file('poppler/include'),
                                   '-I' + test_file('freetype/include')],
                      emcc_args=['-sFILESYSTEM', '--pre-js=pre.js', '--embed-file',
                                 test_file('poppler/emscripten_html5.pdf') + '@input.pdf',
                                 '-sERROR_ON_UNDEFINED_SYMBOLS=0',
                                 '-sMINIMAL_RUNTIME=0'], # not minimal because of files
                      lib_builder=lib_builder, skip_native=True)
