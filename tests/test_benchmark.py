# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import math
import os
import re
import shutil
import sys
import time
import unittest
import zlib

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: tests/runner.py benchmark')

from runner import RunnerCore, chdir
from tools.shared import run_process, path_from_root, CLANG, Building, SPIDERMONKEY_ENGINE, LLVM_ROOT, CLOSURE_COMPILER, CLANG_CC, V8_ENGINE, PIPE, try_delete, PYTHON, EMCC
from tools import shared, jsrun

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

IGNORE_COMPILATION = 0

OPTIMIZATIONS = '-O3'

PROFILING = 0


class Benchmarker(object):
  def __init__(self, name):
    self.name = name

  def bench(self, args, output_parser=None, reps=TEST_REPS):
    self.times = []
    self.reps = reps
    for i in range(reps):
      start = time.time()
      output = self.run(args)
      if not output_parser or args == ['0']: # if arg is 0, we are not running code, and have no output to parse
        if IGNORE_COMPILATION:
          curr = float(re.search(r'took +([\d\.]+) milliseconds', output).group(1)) / 1000
        else:
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
      sorted_times = self.times[:]
      sorted_times.sort()
      median = sum(sorted_times[len(sorted_times) // 2 - 1:len(sorted_times) // 2 + 1]) / 2

      print('   %10s: mean: %4.3f (+-%4.3f) secs  median: %4.3f  range: %4.3f-%4.3f  (noise: %4.3f%%)  (%d runs)' % (self.name, mean, std, median, min(self.times), max(self.times), 100 * std / mean, self.reps), end=' ')

      if baseline:
        mean_baseline = sum(baseline.times) / len(baseline.times)
        final = mean / mean_baseline
        print('  Relative: %.2f X slower' % final)
      else:
        print()

    # size

    size = sum(os.path.getsize(f) for f in self.get_output_files())
    gzip_size = sum(len(zlib.compress(open(f).read())) for f in self.get_output_files())

    print('        size: %8s, compressed: %8s' % (size, gzip_size), end=' ')
    if self.get_size_text():
      print('  (' + self.get_size_text() + ')', end=' ')
    print()

  def get_size_text(self):
    return ''


class NativeBenchmarker(Benchmarker):
  def __init__(self, name, cc, cxx, args=[OPTIMIZATIONS]):
    self.name = name
    self.cc = cc
    self.cxx = cxx
    self.args = args[:]

  def build(self, parent, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser):
    self.parent = parent
    if lib_builder:
      native_args += lib_builder(self.name, native=True, env_init={'CC': self.cc, 'CXX': self.cxx, 'CXXFLAGS': "-Wno-c++11-narrowing"})
    if not native_exec:
      compiler = self.cxx if filename.endswith('cpp') else self.cc
      cmd = [
        compiler,
        '-fno-math-errno',
        filename,
        '-o', filename + '.native'
      ] + self.args + shared_args + native_args + shared.get_clang_native_args()
      # print(cmd)
      run_process(cmd, env=shared.get_clang_native_env())
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

  def cleanup(self):
    pass


def run_binaryen_opts(filename, opts):
  run_process([
    os.path.join(Building.get_binaryen_bin(), 'wasm-opt', '--all-features'),
    filename,
    '-o', filename
  ] + opts)


class EmscriptenBenchmarker(Benchmarker):
  def __init__(self, name, engine, extra_args=[], env={}, binaryen_opts=[]):
    self.name = name
    self.engine = engine
    self.extra_args = extra_args[:]
    self.env = os.environ.copy()
    for k, v in env.items():
      self.env[k] = v
    self.binaryen_opts = binaryen_opts[:]

  def build(self, parent, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser):
    self.filename = filename
    self.old_env = os.environ
    os.environ = self.env.copy()
    llvm_root = self.env.get('LLVM') or LLVM_ROOT
    if lib_builder:
      emcc_args = emcc_args + lib_builder('js_' + llvm_root, native=False, env_init=self.env.copy())
    final = os.path.dirname(filename) + os.path.sep + self.name + ('_' if self.name else '') + os.path.basename(filename) + '.js'
    final = final.replace('.cpp', '')
    try_delete(final)
    cmd = [
      PYTHON, EMCC, filename,
      OPTIMIZATIONS,
      '-s', 'TOTAL_MEMORY=256*1024*1024',
      '-s', 'FILESYSTEM=0',
      '--closure', '1',
      '-s', 'MINIMAL_RUNTIME=0',
      '-s', 'BENCHMARK=%d' % (1 if IGNORE_COMPILATION and not has_output_parser else 0),
      '-o', final
    ] + shared_args + emcc_args + self.extra_args
    if 'FORCE_FILESYSTEM=1' in cmd:
      cmd = [arg if arg != 'FILESYSTEM=0' else 'FILESYSTEM=1' for arg in cmd]
    if PROFILING:
      cmd += ['--profiling-funcs']
    self.cmd = cmd
    run_process(cmd, env=self.env)
    if self.binaryen_opts:
      run_binaryen_opts(final[:-3] + '.wasm', self.binaryen_opts)
    self.filename = final

  def run(self, args):
    return jsrun.run_js(self.filename, engine=self.engine, args=args, stderr=PIPE, full_output=True, assert_returncode=None)

  def get_output_files(self):
    ret = [self.filename]
    if 'WASM=0' in self.cmd:
      if 'MINIMAL_RUNTIME=1' in self.cmd:
        ret.append(self.filename[:-3] + '.asm.js')
        ret.append(self.filename[:-3] + '.mem')
      else:
        ret.append(self.filename + '.mem')
    else:
      ret.append(self.filename[:-3] + '.wasm')
    return ret

  def cleanup(self):
    os.environ = self.old_env
    Building.clear()


CHEERP_BIN = '/opt/cheerp/bin/'


class CheerpBenchmarker(Benchmarker):
  def __init__(self, name, engine, args=[OPTIMIZATIONS], binaryen_opts=[]):
    self.name = name
    self.engine = engine
    self.args = args[:]
    self.binaryen_opts = binaryen_opts[:]

  def build(self, parent, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser):
    suffix = filename.split('.')[-1]
    cheerp_temp = filename[:-len(suffix) - 1] + '.cheerp.cpp'
    code = open(filename).read()
    open(cheerp_temp, 'w').write('''
      %(code)s
#include <cheerp/client.h>
void webMain() {
  main();
}\n''' % {
      'code': code,
    })
    cheerp_args = [
      '-target', 'cheerp',
      '-fno-math-errno',
      '-Wno-c++11-narrowing',
      '-cheerp-mode=wasm'
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
        'RANLIB': CHEERP_BIN + 'llvm-ranlib',
        'CFLAGS': ' '.join(cheerp_args),
        'CXXFLAGS': ' '.join(cheerp_args),
        'CHEERP_PREFIX': CHEERP_BIN + '../',
      })
    if PROFILING:
      cheerp_args += ['-cheerp-pretty-code'] # get function names, like emcc --profiling
    final = os.path.dirname(filename) + os.path.sep + self.name + ('_' if self.name else '') + os.path.basename(filename) + '.js'
    final = final.replace('.cpp', '')
    try_delete(final)
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
        '-cheerp-wasm-loader=' + final,
        cheerp_temp,
        '-Wno-writable-strings', # for how we set up webMain
        '-o', final.replace('.js', '.wasm')
      ] + shared_args
      # print(' '.join(cmd))
      run_process(cmd, stdout=PIPE, stderr=PIPE)
      self.filename = final
      # Inject command line arguments
      run_process(['sed', '-i', 's/"use strict";/"use strict";var args=typeof(scriptArgs) !== "undefined" ? scriptArgs : arguments;/', self.filename])
      if self.binaryen_opts:
        run_binaryen_opts(final.replace('.js', '.wasm'), self.binaryen_opts)
    finally:
      for dir_ in dirs_to_delete:
        try_delete(dir_)

  def run(self, args):
    return jsrun.run_js(self.filename, engine=self.engine, args=args, stderr=PIPE, full_output=True, assert_returncode=None)

  def get_output_files(self):
    return [self.filename, self.filename.replace('.js', '.wasm')]

  def cleanup(self):
    pass


# Benchmarkers

benchmarkers = []

if CLANG_CC and CLANG:
  benchmarkers += [
    # NativeBenchmarker('clang', CLANG_CC, CLANG),
    # NativeBenchmarker('gcc',   'gcc',    'g++')
  ]
if SPIDERMONKEY_ENGINE and SPIDERMONKEY_ENGINE in shared.JS_ENGINES:
  benchmarkers += [
    # EmscriptenBenchmarker('sm', SPIDERMONKEY_ENGINE),
  ]
if V8_ENGINE and V8_ENGINE in shared.JS_ENGINES:
  benchmarkers += [
    EmscriptenBenchmarker('v8', V8_ENGINE),
  ]
if os.path.exists(CHEERP_BIN):
  benchmarkers += [
    # CheerpBenchmarker('cheerp-sm-wasm', SPIDERMONKEY_ENGINE + ['--no-wasm-baseline']),
    # CheerpBenchmarker('cheerp-v8-wasm', V8_ENGINE),
  ]


class benchmark(RunnerCore):
  save_dir = True

  @classmethod
  def setUpClass(self):
    super(benchmark, self).setUpClass()

    fingerprint = ['ignoring compilation' if IGNORE_COMPILATION else 'including compilation', time.asctime()]
    try:
      fingerprint.append('em: ' + run_process(['git', 'show'], stdout=PIPE).stdout.splitlines()[0])
    except:
      pass
    try:
      with chdir(os.path.expanduser('~/Dev/mozilla-central')):
        fingerprint.append('sm: ' + [line for line in run_process(['hg', 'tip'], stdout=PIPE).stdout.splitlines() if 'changeset' in line][0])
    except:
      pass
    fingerprint.append('llvm: ' + LLVM_ROOT)
    print('Running Emscripten benchmarks... [ %s ]' % ' | '.join(fingerprint))

    assert(os.path.exists(CLOSURE_COMPILER))

    Building.COMPILER = CLANG
    Building.COMPILER_TEST_OPTS = [OPTIMIZATIONS]

  # avoid depending on argument reception from the commandline
  def hardcode_arguments(self, code):
    if not code:
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

  def do_benchmark(self, name, src, expected_output='FAIL', args=[],
                   emcc_args=[], native_args=[], shared_args=[],
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
    with open(filename, 'w') as f:
      f.write(src)

    print()
    baseline = None
    for b in benchmarkers:
      if skip_native and isinstance(b, NativeBenchmarker):
        continue
      baseline = b
      print('Running benchmarker: ' + b.name)
      b.build(self, filename, args, shared_args, emcc_args, native_args, native_exec, lib_builder, has_output_parser=output_parser is not None)
      b.bench(args, output_parser, reps)
      b.display(baseline)
      b.cleanup()

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
    self.do_benchmark('primes' if check else 'primes-nocheck', src, 'lastprime:' if check else '', shared_args=['-DCHECK'] if check else [], emcc_args=['-s', 'MINIMAL_RUNTIME=0'])

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
    self.do_benchmark('memops', src, 'final:', emcc_args=['-s', 'MINIMAL_RUNTIME=0'])

  def zzztest_files(self):
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
    self.do_benchmark(src, 'ok')

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

        printf("ok\n");

        return sum;
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

        return x;
      }
    '''
    self.do_benchmark('conditionals', src, 'ok', reps=TEST_REPS, emcc_args=['-s', 'MINIMAL_RUNTIME=0'])

  def test_fannkuch(self):
    src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read().replace(
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

  def zzz_test_corrections64(self):
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

  def fasta(self, name, double_rep, emcc_args=[]):
    src = open(path_from_root('tests', 'fasta.cpp'), 'r').read().replace('double', double_rep)
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

  @non_core
  def test_fasta_double_full(self):
    self.fasta('fasta_double_full', 'double', emcc_args=['-s', 'DOUBLE_MODE=1'])

  def test_skinning(self):
    src = open(path_from_root('tests', 'skinning_test_no_simd.cpp'), 'r').read()
    self.do_benchmark('skinning', src, 'blah=0.000000')

  def test_havlak(self):
    src = open(path_from_root('tests', 'havlak.cpp'), 'r').read()
    self.do_benchmark('havlak', src, 'Found', shared_args=['-std=c++11'])

  def test_base64(self):
    src = open(path_from_root('tests', 'base64.cpp'), 'r').read()
    self.do_benchmark('base64', src, 'decode')

  @non_core
  def test_life(self):
    src = open(path_from_root('tests', 'life.c'), 'r').read()
    self.do_benchmark('life', src, '''--------------------------------''', shared_args=['-std=c99'], force_c=True)

  def test_linpack(self):
    def output_parser(output):
      mflops = re.search(r'Unrolled Double  Precision ([\d\.]+) Mflops', output).group(1)
      return 10000.0 / float(mflops)
    self.do_benchmark('linpack_double', open(path_from_root('tests', 'linpack2.c')).read(), '''Unrolled Double  Precision''', force_c=True, output_parser=output_parser)

  # Benchmarks the synthetic performance of calling native functions.
  @non_core
  def test_native_functions(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('native_functions', open(path_from_root('tests', 'benchmark_ffis.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  # Benchmarks the synthetic performance of calling function pointers.
  @non_core
  def test_native_function_pointers(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('native_functions', open(path_from_root('tests', 'benchmark_ffis.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DBENCHMARK_FUNCTION_POINTER=1', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  # Benchmarks the synthetic performance of calling "foreign" JavaScript functions.
  @non_core
  def test_foreign_functions(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('foreign_functions', open(path_from_root('tests', 'benchmark_ffis.cpp')).read(), 'Total time:', output_parser=output_parser, emcc_args=['--js-library', path_from_root('tests/benchmark_ffis.js')], shared_args=['-DBENCHMARK_FOREIGN_FUNCTION=1', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memcpy_128b(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_128b', open(path_from_root('tests', 'benchmark_memcpy.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMAX_COPY=128', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memcpy_4k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_4k', open(path_from_root('tests', 'benchmark_memcpy.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=128', '-DMAX_COPY=4096', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memcpy_16k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_16k', open(path_from_root('tests', 'benchmark_memcpy.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=4096', '-DMAX_COPY=16384', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memcpy_1mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_1mb', open(path_from_root('tests', 'benchmark_memcpy.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=16384', '-DMAX_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memcpy_16mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memcpy_16mb', open(path_from_root('tests', 'benchmark_memcpy.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memset_128b(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_128b', open(path_from_root('tests', 'benchmark_memset.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMAX_COPY=128', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memset_4k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_4k', open(path_from_root('tests', 'benchmark_memset.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=128', '-DMAX_COPY=4096', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memset_16k(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_16k', open(path_from_root('tests', 'benchmark_memset.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=4096', '-DMAX_COPY=16384', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memset_1mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_1mb', open(path_from_root('tests', 'benchmark_memset.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=16384', '-DMAX_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  @non_core
  def test_memset_16mb(self):
    def output_parser(output):
      return float(re.search(r'Total time: ([\d\.]+)', output).group(1))
    self.do_benchmark('memset_16mb', open(path_from_root('tests', 'benchmark_memset.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-DMIN_COPY=1048576', '-DBUILD_FOR_SHELL', '-I' + path_from_root('tests')])

  def test_matrix_multiply(self):
    def output_parser(output):
      return float(re.search(r'Total elapsed: ([\d\.]+)', output).group(1))
    self.do_benchmark('matrix_multiply', open(path_from_root('tests', 'matrix_multiply.cpp')).read(), 'Total time:', output_parser=output_parser, shared_args=['-I' + path_from_root('tests')])

  @non_core
  def test_zzz_java_nbody(self): # tests xmlvm compiled java, including bitcasts of doubles, i64 math, etc.
    args = [path_from_root('tests', 'nbody-java', x) for x in os.listdir(path_from_root('tests', 'nbody-java')) if x.endswith('.c')] + \
           ['-I' + path_from_root('tests', 'nbody-java')]
    self.do_benchmark('nbody_java', '', '''Time(s)''',
                      force_c=True, emcc_args=args + ['--llvm-lto', '2'], native_args=args + ['-lgc', '-std=c99', '-target', 'x86_64-pc-linux-gnu', '-lm'])

  def lua(self, benchmark, expected, output_parser=None, args_processor=None):
    shutil.copyfile(path_from_root('tests', 'lua', benchmark + '.lua'), benchmark + '.lua')

    def lib_builder(name, native, env_init):
      ret = self.get_library('lua_native' if native else 'lua', [os.path.join('src', 'lua'), os.path.join('src', 'liblua.a')], make=['make', 'generic'], configure=None, native=native, cache_name_extra=name, env_init=env_init)
      if native:
        return ret
      shutil.copyfile(ret[0], ret[0] + '.bc')
      ret[0] += '.bc'
      return ret

    self.do_benchmark('lua_' + benchmark, '', expected,
                      force_c=True, args=[benchmark + '.lua', DEFAULT_ARG],
                      emcc_args=['--embed-file', benchmark + '.lua', '-s', 'FORCE_FILESYSTEM=1'],
                      lib_builder=lib_builder, native_exec=os.path.join('building', 'lua_native', 'src', 'lua'),
                      output_parser=output_parser, args_processor=args_processor)

  def test_zzz_lua_scimark(self):
    def output_parser(output):
      return 100.0 / float(re.search(r'\nSciMark +([\d\.]+) ', output).group(1))

    self.lua('scimark', '[small problem sizes]', output_parser=output_parser)

  def test_zzz_lua_binarytrees(self):
    # js version: ['binarytrees.lua', {0: 0, 1: 9.5, 2: 11.99, 3: 12.85, 4: 14.72, 5: 15.82}[arguments[0]]]
    self.lua('binarytrees', 'long lived tree of depth')

  def test_zzz_zlib(self):
    src = open(path_from_root('tests', 'zlib', 'benchmark.c'), 'r').read()

    def lib_builder(name, native, env_init):
      return self.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a'], native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('zlib', src, '''ok.''',
                      force_c=True, shared_args=['-I' + path_from_root('tests', 'zlib')], lib_builder=lib_builder)

  def test_zzz_box2d(self): # Called thus so it runs late in the alphabetical cycle... it is long
    src = open(path_from_root('tests', 'box2d', 'Benchmark.cpp'), 'r').read()

    def lib_builder(name, native, env_init):
      return self.get_library('box2d', [os.path.join('box2d.a')], configure=None, native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('box2d', src, 'frame averages', shared_args=['-I' + path_from_root('tests', 'box2d')], lib_builder=lib_builder)

  def test_zzz_bullet(self): # Called thus so it runs late in the alphabetical cycle... it is long
    src = open(path_from_root('tests', 'bullet', 'Demos', 'Benchmarks', 'BenchmarkDemo.cpp'), 'r').read()
    src += open(path_from_root('tests', 'bullet', 'Demos', 'Benchmarks', 'main.cpp'), 'r').read()

    def lib_builder(name, native, env_init):
      return self.get_library('bullet', [os.path.join('src', '.libs', 'libBulletDynamics.a'),
                                         os.path.join('src', '.libs', 'libBulletCollision.a'),
                                         os.path.join('src', '.libs', 'libLinearMath.a')],
                              # The --host parameter is needed for 2 reasons:
                              # 1) bullet in it's configure.ac tries to do platform detection and will fail on unknown platforms
                              # 2) configure will try to compile and run a test file to check if the C compiler is sane. As Cheerp
                              #    will generate a wasm file (which cannot be run), configure will fail. Passing `--host` enables
                              #    cross compile mode, which lets configure complete happily.
                              configure_args=['--disable-demos', '--disable-dependency-tracking', '--host=i686-unknown-linux'], native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('bullet', src, '\nok.\n',
                      shared_args=['-I' + path_from_root('tests', 'bullet', 'src'), '-I' + path_from_root('tests', 'bullet', 'Demos', 'Benchmarks')],
                      lib_builder=lib_builder)

  def test_zzz_lzma(self):
    src = open(path_from_root('tests', 'lzma', 'benchmark.c'), 'r').read()

    def lib_builder(name, native, env_init):
      return self.get_library('lzma', [os.path.join('lzma.a')], configure=None, native=native, cache_name_extra=name, env_init=env_init)

    self.do_benchmark('lzma', src, 'ok.', shared_args=['-I' + path_from_root('tests', 'lzma')], lib_builder=lib_builder)

  def test_zzz_sqlite(self):
    src = open(path_from_root('tests', 'sqlite', 'sqlite3.c'), 'r').read() + open(path_from_root('tests', 'sqlite', 'speedtest1.c'), 'r').read()

    self.do_benchmark('sqlite', src, 'ok.', shared_args=['-I' + path_from_root('tests', 'sqlite')], emcc_args=['-s', 'FILESYSTEM=1'], force_c=True)

  def test_zzz_poppler(self):
    with open('pre.js', 'w') as f:
      f.write('''
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
          Module.arguments = ['-?'];
          Module.printErr = function(){};
        } else {
          // Add 'filename' after 'input.pdf' to write the output so it can be verified.
          Module.arguments = ['-scale-to', '1024', 'input.pdf',  '-f', '1', '-l', '' + benchmarkArgumentToPageCount[benchmarkArgument]];
          Module.postRun = function() {
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
              var data = MEMFS.getFileDataAsRegularArray(FS.root.contents[file]);
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
      return self.get_poppler_library()

    # TODO: Fix poppler native build and remove skip_native=True
    self.do_benchmark('poppler', '', 'hashed printout',
                      shared_args=['-I' + path_from_root('tests', 'poppler', 'include'), '-I' + path_from_root('tests', 'freetype', 'include')],
                      emcc_args=['-s', 'FILESYSTEM=1', '--pre-js', 'pre.js', '--embed-file', path_from_root('tests', 'poppler', 'emscripten_html5.pdf') + '@input.pdf', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=0'],
                      lib_builder=lib_builder, skip_native=True)
