# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import glob
import hashlib
import json
import os
import random
import re
import shutil
import sys
import time
import unittest
from functools import wraps
from textwrap import dedent

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: tests/runner.py')

from tools.shared import try_delete, PIPE
from tools.shared import NODE_JS, V8_ENGINE, JS_ENGINES, SPIDERMONKEY_ENGINE, PYTHON, EMCC, EMAR, WINDOWS, MACOS, LLVM_ROOT
from tools import shared, building
from runner import RunnerCore, path_from_root, requires_native_clang
from runner import skip_if, no_wasm_backend, needs_dlfcn, no_windows, no_asmjs, is_slow_test, create_test_file, parameterized
from runner import js_engines_modify, wasm_engines_modify, env_modify, with_env_modify
from runner import NON_ZERO
import clang_native

# decorators for limiting which modes a test can run in


def wasm_simd(f):
  def decorated(self):
    if not V8_ENGINE or V8_ENGINE not in JS_ENGINES:
      self.skipTest('wasm simd only supported in d8 for now')
    if not self.is_wasm():
      self.skipTest('wasm2js only supports MVP for now')
    self.emcc_args.append('-msimd128')
    self.emcc_args.append('-fno-lax-vector-conversions')
    with js_engines_modify([V8_ENGINE + ['--experimental-wasm-simd']]):
      f(self)
  return decorated


def bleeding_edge_wasm_backend(f):
  def decorated(self):
    if not V8_ENGINE or V8_ENGINE not in JS_ENGINES:
      self.skipTest('only works in d8 for now')
    if not self.is_wasm():
      self.skipTest('wasm2js only supports MVP for now')
    with js_engines_modify([V8_ENGINE]):
      f(self)
  return decorated


def also_with_wasm_bigint(f):
  def decorated(self):
    self.set_setting('WASM_BIGINT', 0)
    f(self)
    if self.get_setting('WASM'):
      self.set_setting('WASM_BIGINT', 1)
      with js_engines_modify([NODE_JS + ['--experimental-wasm-bigint']]):
        f(self)
  return decorated


# without EMTEST_ALL_ENGINES set we only run tests in a single VM by
# default. in some tests we know that cross-VM differences may happen and
# so are worth testing, and they should be marked with this decorator
def all_engines(f):
  def decorated(self):
    old = self.use_all_engines
    self.use_all_engines = True
    try:
      f(self)
    finally:
      self.use_all_engines = old
  return decorated


# Tests exception handling in emscripten exception handling mode, and if
# possible, new wasm EH mode.
def with_both_exception_handling(f):
  def decorated(self):
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)
    f(self)
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 1)
    # Wasm EH is currently supported only in wasm backend and V8
    if V8_ENGINE and \
       V8_ENGINE in JS_ENGINES and self.get_setting('WASM'):
      self.emcc_args.append('-fwasm-exceptions')
      with js_engines_modify([V8_ENGINE + ['--experimental-wasm-eh']]):
        f(self)
  return decorated


def no_wasm(note=''):
  assert not callable(note)

  def decorated(f):
    return skip_if(f, 'is_wasm', note)
  return decorated


def no_wasm2js(note=''):
  assert not callable(note)

  def decorated(f):
    return skip_if(f, 'is_wasm2js', note)
  return decorated


# Async wasm compilation can't work in some tests, they are set up synchronously
def sync(f):
  assert callable(f)

  def decorated(self, *args, **kwargs):
    self.emcc_args += ['-s', 'WASM_ASYNC_COMPILATION=0'] # test is set up synchronously
    f(self, *args, **kwargs)
  return decorated


def also_with_noderawfs(func):
  def decorated(self):
    orig_args = self.emcc_args[:]
    func(self)
    print('noderawfs')
    self.emcc_args = orig_args + ['-s', 'NODERAWFS=1', '-DNODERAWFS']
    with js_engines_modify([NODE_JS]):
      func(self)
  return decorated


def can_do_standalone(self):
  return self.get_setting('WASM') and \
      self.get_setting('STACK_OVERFLOW_CHECK') < 2 and \
      not self.get_setting('MINIMAL_RUNTIME') and \
      '-fsanitize=address' not in self.emcc_args


# Impure means a test that cannot run in a wasm VM yet, as it is not 100%
# standalone. We can still run them with the JS code though.
def also_with_standalone_wasm(wasm2c=False, impure=False):
  def decorated(func):
    def metafunc(self, standalone):
      if not standalone:
        func(self)
      else:
        if can_do_standalone(self):
          self.set_setting('STANDALONE_WASM', 1)
          # we will not legalize the JS ffi interface, so we must use BigInt
          # support in order for JS to have a chance to run this without trapping
          # when it sees an i64 on the ffi.
          self.set_setting('WASM_BIGINT', 1)
          # if we are impure, disallow all wasm engines
          with wasm_engines_modify([] if impure else shared.WASM_ENGINES):
            with js_engines_modify([NODE_JS + ['--experimental-wasm-bigint']]):
              func(self)
              if wasm2c:
                print('wasm2c')
                self.set_setting('WASM2C', 1)
                with wasm_engines_modify([]):
                  func(self)

    metafunc._parameterize = {'': (False,),
                              'standalone': (True,)}
    return metafunc

  return decorated


def node_pthreads(f):
  def decorated(self):
    self.set_setting('USE_PTHREADS', 1)
    if '-fsanitize=address' in self.emcc_args:
      self.skipTest('asan ends up using atomics that are not yet supported in node 12')
    if self.get_setting('MINIMAL_RUNTIME'):
      self.skipTest('node pthreads not yet supported with MINIMAL_RUNTIME')
    with js_engines_modify([NODE_JS + ['--experimental-wasm-threads', '--experimental-wasm-bulk-memory']]):
      f(self)
  return decorated


# A simple check whether the compiler arguments cause optimization.
def is_optimizing(args):
  return '-O' in str(args) and '-O0' not in args


def no_optimize(note=''):
  assert not callable(note)

  def decorator(func):
    assert callable(func)

    def decorated(self):
      if is_optimizing(self.emcc_args):
        self.skipTest(note)
      func(self)
    return decorated
  return decorator


def needs_make(note=''):
  assert not callable(note)
  if WINDOWS:
    return unittest.skip('Tool not available on Windows bots (%s)' % note)
  return lambda f: f


def no_asan(note):
  assert not callable(note)

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      if '-fsanitize=address' in self.emcc_args:
        self.skipTest(note)
      f(self, *args, **kwargs)
    return decorated
  return decorator


def no_lsan(note):
  assert not callable(note)

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      if '-fsanitize=leak' in self.emcc_args:
        self.skipTest(note)
      f(self, *args, **kwargs)
    return decorated
  return decorator


def no_minimal_runtime(note):
  assert not callable(note)

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      if 'MINIMAL_RUNTIME=1' in self.emcc_args or self.get_setting('MINIMAL_RUNTIME'):
        self.skipTest(note)
      f(self, *args, **kwargs)
    return decorated
  return decorator


class TestCoreBase(RunnerCore):
  def is_wasm2js(self):
    return not self.get_setting('WASM')

  # Use closure in some tests for some additional coverage
  def maybe_closure(self):
    if '-g' not in self.emcc_args and ('-O2' in self.emcc_args or '-Os' in self.emcc_args):
      self.emcc_args += ['--closure', '1']
      return True
    return False

  def assertStartswith(self, output, prefix):
    self.assertEqual(prefix, output[:len(prefix)])

  def verify_in_strict_mode(self, filename):
    with open(filename) as infile:
      js = infile.read()
    filename += '.strict.js'
    with open(filename, 'w') as outfile:
      outfile.write('"use strict";\n' + js)
    self.run_js(filename)

  def get_bullet_library(self, use_cmake):
    if use_cmake:
      configure_commands = ['cmake', '.']
      configure_args = ['-DBUILD_DEMOS=OFF', '-DBUILD_EXTRAS=OFF', '-DUSE_GLUT=OFF']
      # Depending on whether 'configure' or 'cmake' is used to build, Bullet
      # places output files in different directory structures.
      generated_libs = [os.path.join('src', 'BulletDynamics', 'libBulletDynamics.a'),
                        os.path.join('src', 'BulletCollision', 'libBulletCollision.a'),
                        os.path.join('src', 'LinearMath', 'libLinearMath.a')]
    else:
      configure_commands = ['sh', './configure']
      # Force a nondefault --host= so that the configure script will interpret
      # that we are doing cross-compilation
      # and skip attempting to run the generated executable with './a.out',
      # which would fail since we are building a .js file.
      configure_args = ['--disable-shared', '--host=i686-pc-linux-gnu',
                        '--disable-demos', '--disable-dependency-tracking']
      generated_libs = [os.path.join('src', '.libs', 'libBulletDynamics.a'),
                        os.path.join('src', '.libs', 'libBulletCollision.a'),
                        os.path.join('src', '.libs', 'libLinearMath.a')]

    return self.get_library(os.path.join('third_party', 'bullet'), generated_libs,
                            configure=configure_commands,
                            configure_args=configure_args,
                            cache_name_extra=configure_commands[0])

  @also_with_standalone_wasm()
  def test_hello_world(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')
    # must not emit this unneeded internal thing
    self.assertNotContained('EMSCRIPTEN_GENERATED_FUNCTIONS', open('test_hello_world.js').read())

  @sync
  def test_wasm_synchronous_compilation(self):
    self.set_setting('STRICT_JS')
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')

  @also_with_standalone_wasm()
  def test_hello_argc(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_argc.c')

  def test_intvars(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_intvars.cpp')

  def test_sintvars(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sintvars.c')

  def test_int53(self):
    self.emcc_args += ['-s', 'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[$convertI32PairToI53,$convertU32PairToI53,$readI53FromU64,$readI53FromI64,$writeI53ToI64,$writeI53ToI64Clamped,$writeI53ToU64Clamped,$writeI53ToI64Signaling,$writeI53ToU64Signaling]']
    self.do_run_in_out_file_test('tests', 'core', 'test_int53.c')

  def test_i64(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64.c')

  def test_i64_2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_2.cpp')

  def test_i64_3(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_3.cpp')

  def test_i64_4(self):
    # stuff that also needs sign corrections

    self.do_run_in_out_file_test('tests', 'core', 'test_i64_4.c')

  def test_i64_b(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_b.cpp')

  def test_i64_cmp(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_cmp.cpp')

  def test_i64_cmp2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_cmp2.c')

  def test_i64_double(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_double.cpp')

  def test_i64_umul(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_umul.c')

  @also_with_standalone_wasm()
  def test_i64_precise(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_precise.c')

  def test_i64_precise_needed(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_precise_needed.c')

  def test_i64_llabs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_llabs.c')

  def test_i64_zextneg(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_zextneg.c')

  def test_i64_7z(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_7z.c',
                                 args=['hallo'])

  def test_i64_i16(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_i16.c')

  def test_i64_qdouble(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_qdouble.c')

  def test_i64_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_varargs.c',
                                 args='waka fleefl asdfasdfasdfasdf'
                                      .split(' '))

  @no_wasm2js('wasm_bigint')
  def test_i64_invoke_bigint(self):
    self.set_setting('WASM_BIGINT', 1)
    self.emcc_args += ['-fexceptions']
    self.do_run_in_out_file_test('tests', 'core', 'test_i64_invoke_bigint.cpp',
                                 js_engines=[NODE_JS + ['--experimental-wasm-bigint']])

  def test_vararg_copy(self):
    self.do_run_in_out_file_test('tests', 'va_arg', 'test_va_copy.c')

  def test_llvm_fabs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_llvm_fabs.c')

  def test_double_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_double_varargs.c')

  def test_trivial_struct_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_trivial_struct_varargs.c')

  def test_struct_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_struct_varargs.c')

  def test_zero_struct_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_zero_struct_varargs.c')

  def zzztest_nested_struct_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_nested_struct_varargs.c')

  def test_i32_mul_precise(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_i32_mul_precise.c')

  def test_i16_emcc_intrinsic(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_i16_emcc_intrinsic.c')

  def test_double_i64_conversion(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_double_i64_conversion.c')

  def test_float32_precise(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_float32_precise.c')

  def test_negative_zero(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_negative_zero.c')

  def test_literal_negative_zero(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_literal_negative_zero.c')

  @no_wasm_backend('test uses calls to expected js imports, rather than using llvm intrinsics directly')
  def test_llvm_intrinsics(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_llvm_intrinsics.cpp')

  @no_wasm_backend('test looks for js impls of intrinsics')
  def test_lower_intrinsics(self):
    self.emcc_args += ['-g1']
    self.do_run_in_out_file_test('tests', 'core', 'test_lower_intrinsics.c')
    # intrinsics should be lowered out
    js = open('src.js').read()
    assert ('llvm_' not in js) == is_optimizing(self.emcc_args) or not self.is_wasm(), 'intrinsics must be lowered when optimizing'

  @also_with_standalone_wasm()
  def test_bswap64(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_bswap64.cpp')

  def test_sha1(self):
    self.do_runf(path_from_root('tests', 'sha1.c'), 'SHA1=15dd99a1991e0b3826fede3deffc1feba42278e6')

  @no_wasm_backend('test checks that __asmjs__ is #defined')
  def test_asmjs_unknown_emscripten(self):
    # No other configuration is supported, so always run this.
    self.do_runf(path_from_root('tests', 'asmjs-unknown-emscripten.c'), '')

  def test_cube2md5(self):
    self.emcc_args += ['--embed-file', 'cube2md5.txt']
    shutil.copyfile(path_from_root('tests', 'cube2md5.txt'), 'cube2md5.txt')
    self.do_run_from_file(path_from_root('tests', 'cube2md5.cpp'), path_from_root('tests', 'cube2md5.ok'), assert_returncode=NON_ZERO)

  @also_with_standalone_wasm(wasm2c=True)
  @needs_make('make')
  def test_cube2hash(self):
    # A good test of i64 math
    self.do_run('// empty file', 'Usage: hashstring <seed>',
                libraries=self.get_library('cube2hash', ['cube2hash.bc'], configure=None),
                includes=[path_from_root('tests', 'cube2hash')], assert_returncode=NON_ZERO)

    for text, output in [('fleefl', '892BDB6FD3F62E863D63DA55851700FDE3ACF30204798CE9'),
                         ('fleefl2', 'AA2CC5F96FC9D540CA24FDAF1F71E2942753DB83E8A81B61'),
                         ('64bitisslow', '64D8470573635EC354FEE7B7F87C566FCAF1EFB491041670')]:
      self.do_run('src.js', 'hash value: ' + output, args=[text], no_build=True)

  def test_unaligned(self):
    self.skipTest('LLVM marks the reads of s as fully aligned, making this test invalid')
    src = r'''
      #include <stdio.h>

      struct S {
        double x;
        int y;
      };

      int main() {
        // the 64-bit value here will not be 8-byte aligned
        S s0[3] = { {0x12a751f430142, 22}, {0x17a5c85bad144, 98}, {1, 1}};
        char buffer[10*sizeof(S)];
        int b = int(buffer);
        S *s = (S*)(b + 4-b%8);
        s[0] = s0[0];
        s[1] = s0[1];
        s[2] = s0[2];

        printf("*%d : %d : %d\n", sizeof(S), ((unsigned int)&s[0]) % 8 != ((unsigned int)&s[1]) % 8,
                                             ((unsigned int)&s[1]) - ((unsigned int)&s[0]));
        s[0].x++;
        s[0].y++;
        s[1].x++;
        s[1].y++;
        printf("%.1f,%d,%.1f,%d\n", s[0].x, s[0].y, s[1].x, s[1].y);
        return 0;
      }
      '''

    # TODO: A version of this with int64s as well

    self.do_run(src, '*12 : 1 : 12\n328157500735811.0,23,416012775903557.0,99\n')

    return # TODO: continue to the next part here

    # Test for undefined behavior in C. This is not legitimate code, but does exist

    src = r'''
      #include <stdio.h>

      int main()
      {
        int x[10];
        char *p = (char*)&x[0];
        p++;
        short *q = (short*)p;
        *q = 300;
        printf("*%d:%d*\n", *q, ((int)q)%2);
        int *r = (int*)p;
        *r = 515559;
        printf("*%d*\n", *r);
        long long *t = (long long*)p;
        *t = 42949672960;
        printf("*%lld*\n", *t);
        return 0;
      }
    '''

    try:
      self.do_run(src, '*300:1*\n*515559*\n*42949672960*\n')
    except Exception as e:
      assert 'must be aligned' in str(e), e # expected to fail without emulation

  def test_align64(self):
    src = r'''
      #include <stdio.h>

      // inspired by poppler

      enum Type {
        A = 10,
        B = 20
      };

      struct Object {
        Type type;
        union {
          int intg;
          double real;
          char *name;
        };
      };

      struct Principal {
        double x;
        Object a;
        double y;
      };

      int main(int argc, char **argv)
      {
        int base = argc-1;
        Object *o = NULL;
        printf("%d,%d\n", sizeof(Object), sizeof(Principal));
        printf("%d,%d,%d,%d\n", (int)&o[base].type, (int)&o[base].intg, (int)&o[base].real, (int)&o[base].name);
        printf("%d,%d,%d,%d\n", (int)&o[base+1].type, (int)&o[base+1].intg, (int)&o[base+1].real, (int)&o[base+1].name);
        Principal p, q;
        p.x = p.y = q.x = q.y = 0;
        p.a.type = A;
        p.a.real = 123.456;
        *(&q.a) = p.a;
        printf("%.2f,%d,%.2f,%.2f : %.2f,%d,%.2f,%.2f\n", p.x, p.a.type, p.a.real, p.y, q.x, q.a.type, q.a.real, q.y);
        return 0;
      }
    '''

    self.do_run(src, '''16,32
0,8,8,8
16,24,24,24
0.00,10,123.46,0.00 : 0.00,10,123.46,0.00
''')

  @no_asan('asan errors on corner cases we check')
  def test_aligned_alloc(self):
    self.do_runf(path_from_root('tests', 'test_aligned_alloc.c'), '')

  def test_unsigned(self):
    src = '''
      #include <stdio.h>
      const signed char cvals[2] = { -1, -2 }; // compiler can store this is a string, so -1 becomes \\FF, and needs re-signing
      int main()
      {
        {
          unsigned char x = 200;
          printf("*%d*\\n", x);
          unsigned char y = -22;
          printf("*%d*\\n", y);
        }

        int varey = 100;
        unsigned int MAXEY = -1, MAXEY2 = -77;
        printf("*%u,%d,%u*\\n", MAXEY, varey >= MAXEY, MAXEY2); // 100 >= -1? not in unsigned!

        int y = cvals[0];
        printf("*%d,%d,%d,%d*\\n", cvals[0], cvals[0] < 0, y, y < 0);
        y = cvals[1];
        printf("*%d,%d,%d,%d*\\n", cvals[1], cvals[1] < 0, y, y < 0);

        // zext issue - see mathop in jsifier
        unsigned char x8 = -10;
        unsigned long hold = 0;
        hold += x8;
        int y32 = hold+50;
        printf("*%u,%u*\\n", hold, y32);

        // Comparisons
        x8 = 0;
        for (int i = 0; i < 254; i++) x8++; // make it an actual 254 in JS - not a -2
        printf("*%d,%d*\\n", x8+1 == 0xff, x8+1 != 0xff); // 0xff may be '-1' in the bitcode

        return 0;
      }
    '''
    self.do_run(src, '*4294967295,0,4294967219*\n*-1,1,-1,1*\n*-2,1,-2,1*\n*246,296*\n*1,0*')

    self.emcc_args.append('-Wno-constant-conversion')
    src = '''
      #include <stdio.h>
      int main()
      {
        {
          unsigned char x;
          unsigned char *y = &x;
          *y = -1;
          printf("*%d*\\n", x);
        }
        {
          unsigned short x;
          unsigned short *y = &x;
          *y = -1;
          printf("*%d*\\n", x);
        }
        /*{ // This case is not checked. The hint for unsignedness is just the %u in printf, and we do not analyze that
          unsigned int x;
          unsigned int *y = &x;
          *y = -1;
          printf("*%u*\\n", x);
        }*/
        {
          char x;
          char *y = &x;
          *y = 255;
          printf("*%d*\\n", x);
        }
        {
          char x;
          char *y = &x;
          *y = 65535;
          printf("*%d*\\n", x);
        }
        {
          char x;
          char *y = &x;
          *y = 0xffffffff;
          printf("*%d*\\n", x);
        }
        return 0;
      }
    '''
    self.do_run(src, '*255*\n*65535*\n*-1*\n*-1*\n*-1*')

  def test_bitfields(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_bitfields.c')

  def test_floatvars(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_floatvars.cpp')

  def test_closebitcasts(self):
    self.do_run_in_out_file_test('tests', 'core', 'closebitcasts.c')

  def test_fast_math(self):
    self.emcc_args += ['-ffast-math']

    self.do_run_in_out_file_test('tests', 'core', 'test_fast_math.c',
                                 args=['5', '6', '8'])

  def test_zerodiv(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_zerodiv.c')

  def test_zero_multiplication(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_zero_multiplication.c')

  def test_isnan(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_isnan.c')

  def test_globaldoubles(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_globaldoubles.c')

  def test_math(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_math.cpp')

  def test_erf(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_erf.c')

  def test_math_hyperbolic(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_math_hyperbolic.c')

  def test_math_lgamma(self):
    self.do_run_in_out_file_test('tests', 'math', 'lgamma.c', assert_returncode=NON_ZERO)

  def test_math_fmodf(self):
    self.do_run_in_out_file_test('tests', 'math', 'fmodf.c')

  def test_frexp(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_frexp.c')

  def test_rounding(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_rounding.c')

  def test_fcvt(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_fcvt.cpp')

  def test_llrint(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_llrint.c')

  def test_getgep(self):
    # Generated code includes getelementptr (getelementptr, 0, 1), i.e., GEP as the first param to GEP
    self.do_run_in_out_file_test('tests', 'core', 'test_getgep.c')

  # No compiling from C/C++ - just process an existing .o/.ll/.bc file.
  def do_run_object(self, obj_file, expected_output=None, **kwargs):
    js_file = os.path.basename(obj_file) + '.js'
    building.emcc(obj_file, self.get_emcc_args(), js_file)
    self.do_run(js_file, expected_output, no_build=True, **kwargs)

  def test_multiply_defined_symbols(self):
    create_test_file('a1.c', 'int f() { return 1; }')
    create_test_file('a2.c', 'void x() {}')
    create_test_file('b1.c', 'int f() { return 2; }')
    create_test_file('b2.c', 'void y() {}')
    create_test_file('main.c', r'''
      #include <stdio.h>
      int f();
      int main() {
        printf("result: %d\n", f());
        return 0;
      }
    ''')

    building.emcc('a1.c', ['-c'])
    building.emcc('a2.c', ['-c'])
    building.emcc('b1.c', ['-c'])
    building.emcc('b2.c', ['-c'])
    building.emcc('main.c', ['-c'])

    building.emar('cr', 'liba.a', ['a1.c.o', 'a2.c.o'])
    building.emar('cr', 'libb.a', ['b1.c.o', 'b2.c.o'])

    building.link_to_object(['main.c.o', 'liba.a', 'libb.a'], 'all.o')

    self.do_run_object('all.o', 'result: 1')

  def test_if(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_if.c')

  def test_if_else(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_if_else.c')

  def test_loop(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_loop.c')

  def test_stack(self):
    self.set_setting('INLINING_LIMIT', 50)
    # some extra coverage in all test suites for stack checks
    self.set_setting('STACK_OVERFLOW_CHECK', 2)

    self.do_run_in_out_file_test('tests', 'core', 'test_stack.c')

  def test_stack_align(self):
    src = path_from_root('tests', 'core', 'test_stack_align.cpp')

    def test():
      self.do_runf(src, ['''align 4: 0
align 8: 0
align 16: 0
align 32: 0
base align: 0, 0, 0, 0'''])

    test()

  @no_asan('stack size is too low for asan to work properly')
  def test_stack_placement(self):
    self.set_setting('TOTAL_STACK', 1024)
    self.do_run_in_out_file_test('tests', 'core', 'test_stack_placement.c')
    self.set_setting('GLOBAL_BASE', 102400)
    self.do_run_in_out_file_test('tests', 'core', 'test_stack_placement.c')

  @no_asan('asan does not support main modules')
  @no_wasm2js('MAIN_MODULE support')
  def test_stack_placement_pic(self):
    self.set_setting('TOTAL_STACK', 1024)
    self.set_setting('MAIN_MODULE')
    self.do_run_in_out_file_test('tests', 'core', 'test_stack_placement.c')
    self.set_setting('GLOBAL_BASE', 102400)
    self.do_run_in_out_file_test('tests', 'core', 'test_stack_placement.c')

  @no_wasm_backend('generated code not available in wasm')
  def test_stack_restore(self):
    self.emcc_args += ['-g3'] # to be able to find the generated code

    self.do_run_in_out_file_test('tests', 'core', 'test_stack_restore.c')

    generated = open('src.js').read()

    def ensure_stack_restore_count(function_name, expected_count):
      code = generated[generated.find(function_name):]
      code = code[:code.find('\n}') + 2]
      actual_count = code.count('STACKTOP = sp')
      assert actual_count == expected_count, ('Expected %d stack restorations, got %d' % (expected_count, actual_count)) + ': ' + code

    ensure_stack_restore_count('function _no_stack_usage', 0)
    ensure_stack_restore_count('function _alloca_gets_restored', 1)
    ensure_stack_restore_count('function _stack_usage', 1)

  def test_strings(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strings.c', args=['wowie', 'too', '74'])

  def test_strcmp_uni(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strcmp_uni.c')

  def test_strndup(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strndup.c')

  def test_errar(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_errar.c')

  def test_mainenv(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_mainenv.c')

  def test_funcs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_funcs.c')

  def test_structs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_structs.c')

  gen_struct_src = '''
        #include <stdio.h>
        #include <stdlib.h>
        #include "emscripten.h"

        struct S
        {
          int x, y;
        };
        int main()
        {
          S* a = {{gen_struct}};
          a->x = 51; a->y = 62;
          printf("*%d,%d*\\n", a->x, a->y);
          {{del_struct}}(a);
          return 0;
        }
  '''

  def test_mallocstruct(self):
    self.do_run(self.gen_struct_src.replace('{{gen_struct}}', '(S*)malloc(sizeof(S))').replace('{{del_struct}}', 'free'), '*51,62*')

  @no_asan('ASan does not support custom memory allocators')
  @no_lsan('LSan does not support custom memory allocators')
  @parameterized({
    'normal': [],
    'debug': ['-DEMMALLOC_DEBUG'],
    'debug_log': ['-DEMMALLOC_DEBUG', '-DEMMALLOC_DEBUG_LOG', '-DRANDOM_ITERS=130'],
  })
  def test_emmalloc(self, *args):
    # in newer clang+llvm, the internal calls to malloc in emmalloc may be optimized under
    # the assumption that they are external, so like in system_libs.py where we build
    # malloc, we need to disable builtin here too
    self.set_setting('MALLOC', 'none')
    self.emcc_args += ['-fno-builtin'] + list(args)

    self.do_run(open(path_from_root('system', 'lib', 'emmalloc.cpp')).read() +
                open(path_from_root('system', 'lib', 'sbrk.c')).read() +
                open(path_from_root('tests', 'core', 'test_emmalloc.cpp')).read(),
                open(path_from_root('tests', 'core', 'test_emmalloc.out')).read())

  @no_asan('ASan does not support custom memory allocators')
  @no_lsan('LSan does not support custom memory allocators')
  def test_emmalloc_usable_size(self, *args):
    self.set_setting('MALLOC', 'emmalloc')
    self.emcc_args += list(args)

    self.do_run_in_out_file_test('tests', 'core', 'test_malloc_usable_size.c')

  @no_optimize('output is sensitive to optimization flags, so only test unoptimized builds')
  @no_asan('ASan does not support custom memory allocators')
  @no_lsan('LSan does not support custom memory allocators')
  def test_emmalloc_memory_statistics(self, *args):

    self.set_setting('MALLOC', 'emmalloc')
    self.emcc_args += ['-s', 'INITIAL_MEMORY=128MB', '-g'] + list(args)

    self.do_run_in_out_file_test('tests', 'core', 'test_emmalloc_memory_statistics.cpp')

  @no_optimize('output is sensitive to optimization flags, so only test unoptimized builds')
  @no_asan('ASan does not support custom memory allocators')
  @no_lsan('LSan does not support custom memory allocators')
  def test_emmalloc_trim(self, *args):
    self.set_setting('MALLOC', 'emmalloc')
    self.emcc_args += ['-s', 'INITIAL_MEMORY=128MB', '-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'MAXIMUM_MEMORY=2147418112'] + list(args)

    self.do_run_in_out_file_test('tests', 'core', 'test_emmalloc_trim.cpp')

  # Test case against https://github.com/emscripten-core/emscripten/issues/10363
  def test_emmalloc_memalign_corruption(self, *args):
    self.set_setting('MALLOC', 'emmalloc')
    self.do_run_in_out_file_test('tests', 'core', 'emmalloc_memalign_corruption.cpp')

  def test_newstruct(self):
    self.do_run(self.gen_struct_src.replace('{{gen_struct}}', 'new S').replace('{{del_struct}}', 'delete'), '*51,62*')

  def test_addr_of_stacked(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_addr_of_stacked.c')

  def test_globals(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_globals.c')

  def test_linked_list(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_linked_list.c')

  def test_sup(self):
      src = '''
        #include <stdio.h>

        struct S4   { int x;          }; // size: 4
        struct S4_2 { short x, y;     }; // size: 4, but for alignment purposes, 2
        struct S6   { short x, y, z;  }; // size: 6
        struct S6w  { char x[6];      }; // size: 6 also
        struct S6z  { int x; short y; }; // size: 8, since we align to a multiple of the biggest - 4

        struct C___  { S6 a, b, c; int later; };
        struct Carr  { S6 a[3]; int later; }; // essentially the same, but differently defined
        struct C__w  { S6 a; S6w b; S6 c; int later; }; // same size, different struct
        struct Cp1_  { int pre; short a; S6 b, c; int later; }; // fillers for a
        struct Cp2_  { int a; short pre; S6 b, c; int later; }; // fillers for a (get addr of the other filler)
        struct Cint  { S6 a; int  b; S6 c; int later; }; // An int (different size) for b
        struct C4__  { S6 a; S4   b; S6 c; int later; }; // Same size as int from before, but a struct
        struct C4_2  { S6 a; S4_2 b; S6 c; int later; }; // Same size as int from before, but a struct with max element size 2
        struct C__z  { S6 a; S6z  b; S6 c; int later; }; // different size, 8 instead of 6

        int main()
        {
          #define TEST(struc) \\
          { \\
            struc *s = 0; \\
            printf("*%s: %d,%d,%d,%d<%d*\\n", #struc, (int)&(s->a), (int)&(s->b), (int)&(s->c), (int)&(s->later), sizeof(struc)); \\
          }
          #define TEST_ARR(struc) \\
          { \\
            struc *s = 0; \\
            printf("*%s: %d,%d,%d,%d<%d*\\n", #struc, (int)&(s->a[0]), (int)&(s->a[1]), (int)&(s->a[2]), (int)&(s->later), sizeof(struc)); \\
          }
          printf("sizeofs:%d,%d\\n", sizeof(S6), sizeof(S6z));
          TEST(C___);
          TEST_ARR(Carr);
          TEST(C__w);
          TEST(Cp1_);
          TEST(Cp2_);
          TEST(Cint);
          TEST(C4__);
          TEST(C4_2);
          TEST(C__z);
          return 0;
        }
      '''
      self.do_run(src, 'sizeofs:6,8\n*C___: 0,6,12,20<24*\n*Carr: 0,6,12,20<24*\n*C__w: 0,6,12,20<24*\n*Cp1_: 4,6,12,20<24*\n*Cp2_: 0,6,12,20<24*\n*Cint: 0,8,12,20<24*\n*C4__: 0,8,12,20<24*\n*C4_2: 0,6,10,16<20*\n*C__z: 0,8,16,24<28*')

  @also_with_standalone_wasm()
  def test_assert(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_assert.cpp', assert_returncode=NON_ZERO)

  def test_wcslen(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_wcslen.c')

  def test_regex(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_regex.c')

  @also_with_standalone_wasm(wasm2c=True, impure=True)
  def test_longjmp(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp.c')

  def test_longjmp2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp2.c')

  @needs_dlfcn
  def test_longjmp2_main_module(self):
    # Test for binaryen regression:
    # https://github.com/WebAssembly/binaryen/issues/2180
    self.set_setting('MAIN_MODULE')
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp2.c')

  def test_longjmp3(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp3.c')

  def test_longjmp4(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp4.c')

  def test_longjmp_funcptr(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_funcptr.c')

  def test_longjmp_repeat(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_repeat.c')

  def test_longjmp_stacked(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_stacked.c', assert_returncode=NON_ZERO)

  def test_longjmp_exc(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_exc.c', assert_returncode=NON_ZERO)

  def test_longjmp_throw(self):
    for disable_throw in [0, 1]:
      print(disable_throw)
      self.set_setting('DISABLE_EXCEPTION_CATCHING', disable_throw)
      self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_throw.cpp')

  def test_longjmp_unwind(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_unwind.c', assert_returncode=NON_ZERO)

  def test_longjmp_i64(self):
    self.emcc_args += ['-g']
    self.do_run_in_out_file_test('tests', 'core', 'test_longjmp_i64.c', assert_returncode=NON_ZERO)

  def test_siglongjmp(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_siglongjmp.c')

  def test_setjmp_many(self):
    src = r'''
      #include <stdio.h>
      #include <setjmp.h>

      int main(int argc, char** argv) {
        jmp_buf buf;
        for (int i = 0; i < NUM; i++) printf("%d\n", setjmp(buf));
        if (argc-- == 1131) longjmp(buf, 11);
        return 0;
      }
    '''
    for num in [1, 5, 20, 1000]:
      print('NUM=%d' % num)
      self.do_run(src.replace('NUM', str(num)), '0\n' * num)

  def test_setjmp_many_2(self):
    src = r'''
#include <setjmp.h>
#include <stdio.h>

jmp_buf env;

void luaWork(int d){
    int x;
    printf("d is at %d\n", d);

    longjmp(env, 1);
}

int main()
{
    const int ITERATIONS=25;
    for(int i = 0; i < ITERATIONS; i++){
        if(!setjmp(env)){
            luaWork(i);
        }
    }
    return 0;
}
'''

    self.do_run(src, r'''d is at 24''')

  def test_setjmp_noleak(self):
    src = r'''
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>

jmp_buf env;

void luaWork(int d){
  int x;
  printf("d is at %d\n", d);

  longjmp(env, 1);
}

#include <malloc.h>
#include <stdlib.h>

void dump() {
  struct mallinfo m = mallinfo();
  printf("dump: %d , %d\n", m.arena, m.uordblks);
}

void work(int n)
{
  printf("work %d\n", n);
  dump();

  if(!setjmp(env)){
    luaWork(n);
  }

  if (n > 0) work(n-1);
}

int main() {
  struct mallinfo m1 = mallinfo();
  dump();
  work(10);
  dump();
  struct mallinfo m2 = mallinfo();
  assert(m1.uordblks == m2.uordblks);
  printf("ok.\n");
}
'''
    self.do_run(src, r'''ok.''')

  @with_both_exception_handling
  def test_exceptions(self):
    self.set_setting('EXCEPTION_DEBUG', 1)
    self.maybe_closure()
    for support_longjmp in [0, 1]:
      self.set_setting('SUPPORT_LONGJMP', support_longjmp)
      self.do_run_from_file(path_from_root('tests', 'core', 'test_exceptions.cpp'), path_from_root('tests', 'core', 'test_exceptions_caught.out'))

  def test_exceptions_off(self):
    for support_longjmp in [0, 1]:
      self.set_setting('DISABLE_EXCEPTION_CATCHING', 1)
      self.do_run_from_file(path_from_root('tests', 'core', 'test_exceptions.cpp'), path_from_root('tests', 'core', 'test_exceptions_uncaught.out'), assert_returncode=NON_ZERO)

  def test_exceptions_minimal_runtime(self):
    self.set_setting('EXCEPTION_DEBUG', 1)
    self.set_setting('EXIT_RUNTIME', 1)
    self.maybe_closure()
    self.set_setting('MINIMAL_RUNTIME', 1)
    for support_longjmp in [0, 1]:
      self.set_setting('SUPPORT_LONGJMP', support_longjmp)

      self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)
      self.do_run_from_file(path_from_root('tests', 'core', 'test_exceptions.cpp'), path_from_root('tests', 'core', 'test_exceptions_caught.out'))

      self.set_setting('DISABLE_EXCEPTION_CATCHING', 1)
      # TODO: Node currently returns 0 for unhandled promise rejections.
      # Switch this to True when they change their default
      expect_fail = False
      if self.get_setting('WASM') == 0:
        expect_fail = True
      self.do_run_from_file(path_from_root('tests', 'core', 'test_exceptions.cpp'), path_from_root('tests', 'core', 'test_exceptions_uncaught.out'), assert_returncode=NON_ZERO if expect_fail else 0)

  @with_both_exception_handling
  def test_exceptions_custom(self):
    self.set_setting('EXCEPTION_DEBUG', 1)
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.maybe_closure()
    src = '''
    #include <iostream>

    class MyException
    {
    public:
        MyException(){ std::cout << "Construct..."; }
        MyException( const MyException & ) { std::cout << "Copy..."; }
        ~MyException(){ std::cout << "Destruct..."; }
    };

    int function()
    {
        std::cout << "Throw...";
        throw MyException();
    }

    int function2()
    {
        return function();
    }

    int main()
    {
        try
        {
            function2();
        }
        catch (MyException & e)
        {
            std::cout << "Caught...";
        }

        try
        {
            function2();
        }
        catch (MyException e)
        {
            std::cout << "Caught...";
        }

        return 0;
    }
    '''

    self.do_run(src, 'Throw...Construct...Caught...Destruct...Throw...Construct...Copy...Caught...Destruct...Destruct...')

  @with_both_exception_handling
  def test_exceptions_2(self):
    for safe in [0, 1]:
      print(safe)
      if safe and '-fsanitize=address' in self.emcc_args:
        # Can't use safe heap with ASan
        continue
      self.set_setting('SAFE_HEAP', safe)
      self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_2.cpp')

  @with_both_exception_handling
  def test_exceptions_3(self):
    src = r'''
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
  if (argc != 2) {
    std::cout << "need an arg" << std::endl;
    return 1;
  }

  int arg = argv[1][0] - '0';
  try {
    if (arg == 0) throw "a c string";
    if (arg == 1) throw std::exception();
    if (arg == 2) throw std::runtime_error("Hello");
  } catch(const char * ex) {
    std::cout << "Caught C string: " << ex << std::endl;
  } catch(const std::exception &ex) {
    std::cout << "Caught exception: " << ex.what() << std::endl;
  } catch(...) {
    std::cout << "Caught something else" << std::endl;
  }

  std::cout << "Done.\n";
}
'''

    print('0')
    self.do_run(src, 'Caught C string: a c string\nDone.', args=['0'])
    print('1')
    self.do_run('src.js', 'Caught exception: std::exception\nDone.', args=['1'], no_build=True)
    print('2')
    self.do_run('src.js', 'Caught exception: Hello\nDone.', args=['2'], no_build=True)

  def test_exceptions_allowed(self):
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 2)
    # Wasm does not add an underscore to function names. For wasm, the
    # mismatches are fixed in fixImports() function in JS glue code.
    self.set_setting('EXCEPTION_CATCHING_ALLOWED', ["_Z12somefunctionv"])
    # otherwise it is inlined and not identified
    self.set_setting('INLINING_LIMIT', 50)

    src = path_from_root('tests', 'core', 'test_exceptions_allowed.cpp')
    output = shared.unsuffixed(src) + '.out'
    self.do_run_from_file(src, output)
    size = len(open('test_exceptions_allowed.js').read())
    shutil.copyfile('test_exceptions_allowed.js', 'orig.js')

    # check that an empty whitelist works properly (as in, same as exceptions disabled)
    empty_output = path_from_root('tests', 'core', 'test_exceptions_allowed_empty.out')

    self.set_setting('EXCEPTION_CATCHING_ALLOWED', [])
    self.do_run_from_file(src, empty_output, assert_returncode=NON_ZERO)
    empty_size = len(open('test_exceptions_allowed.js').read())
    shutil.copyfile('test_exceptions_allowed.js', 'empty.js')

    self.set_setting('EXCEPTION_CATCHING_ALLOWED', ['fake'])
    self.do_run_from_file(src, empty_output, assert_returncode=NON_ZERO)
    fake_size = len(open('test_exceptions_allowed.js').read())
    shutil.copyfile('test_exceptions_allowed.js', 'fake.js')

    self.set_setting('DISABLE_EXCEPTION_CATCHING', 1)
    self.do_run_from_file(src, empty_output, assert_returncode=NON_ZERO)
    disabled_size = len(open('test_exceptions_allowed.js').read())
    shutil.copyfile('test_exceptions_allowed.js', 'disabled.js')

    if not self.is_wasm():
      print(size, empty_size, fake_size, disabled_size)
      assert empty_size == fake_size, [empty_size, fake_size]
      # big change when we disable exception catching of the function
      assert size - empty_size > 0.01 * size, [empty_size, size]
      # full disable can remove a little bit more
      assert empty_size >= disabled_size, [empty_size, disabled_size]

  def test_exceptions_allowed_2(self):
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 2)
    # Wasm does not add an underscore to function names. For wasm, the
    # mismatches are fixed in fixImports() function in JS glue code.
    self.set_setting('EXCEPTION_CATCHING_ALLOWED', ["main"])
    # otherwise it is inlined and not identified
    self.set_setting('INLINING_LIMIT', 1)

    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_allowed_2.cpp')

  def test_exceptions_allowed_uncaught(self):
    self.emcc_args += ['-std=c++11']
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 2)
    # Wasm does not add an underscore to function names. For wasm, the
    # mismatches are fixed in fixImports() function in JS glue code.
    self.set_setting('EXCEPTION_CATCHING_ALLOWED', ["_Z4testv"])
    # otherwise it is inlined and not identified
    self.set_setting('INLINING_LIMIT', 1)

    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_allowed_uncaught.cpp')

  @with_both_exception_handling
  def test_exceptions_uncaught(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    src = r'''
      #include <stdio.h>
      #include <exception>
      struct X {
        ~X() {
          printf("exception? %s\n", std::uncaught_exception() ? "yes" : "no");
        }
      };
      int main() {
        printf("exception? %s\n", std::uncaught_exception() ? "yes" : "no");
        try {
          X x;
          throw 1;
        } catch(...) {
          printf("exception? %s\n", std::uncaught_exception() ? "yes" : "no");
        }
        printf("exception? %s\n", std::uncaught_exception() ? "yes" : "no");
        return 0;
      }
    '''
    self.do_run(src, 'exception? no\nexception? yes\nexception? no\nexception? no\n')

    src = r'''
      #include <fstream>
      #include <iostream>
      int main() {
        std::ofstream os("test");
        os << std::unitbuf << "foo"; // trigger a call to std::uncaught_exception from
                                     // std::basic_ostream::sentry::~sentry
        std::cout << "success";
      }
    '''
    self.do_run(src, 'success')

  @with_both_exception_handling
  def test_exceptions_uncaught_2(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    src = r'''
      #include <iostream>
      #include <exception>

      int main() {
        try {
            throw std::exception();
        } catch(std::exception) {
          try {
            throw;
          } catch(std::exception) {}
        }

        if (std::uncaught_exception())
          std::cout << "ERROR: uncaught_exception still set.";
        else
          std::cout << "OK";
      }
    '''
    self.do_run(src, 'OK\n')

  @with_both_exception_handling
  def test_exceptions_typed(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.emcc_args += ['-s', 'SAFE_HEAP=0'] # Throwing null will cause an ignorable null pointer access.

    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_typed.cpp')

  @with_both_exception_handling
  def test_exceptions_virtual_inheritance(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_virtual_inheritance.cpp')

  @with_both_exception_handling
  def test_exceptions_convert(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_convert.cpp')

  # TODO Make setjmp-longjmp also use Wasm exception handling
  @with_both_exception_handling
  def test_exceptions_multi(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_multi.cpp')

  @with_both_exception_handling
  def test_exceptions_std(self):
    self.emcc_args += ['-s', 'SAFE_HEAP=0']

    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_std.cpp')

  @with_both_exception_handling
  def test_exceptions_alias(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_alias.cpp')

  @with_both_exception_handling
  def test_exceptions_rethrow(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_rethrow.cpp')

  @with_both_exception_handling
  def test_exceptions_resume(self):
    self.set_setting('EXCEPTION_DEBUG', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_resume.cpp')

  @with_both_exception_handling
  def test_exceptions_destroy_virtual(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_destroy_virtual.cpp')

  @with_both_exception_handling
  def test_exceptions_refcount(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_refcount.cpp')

  @with_both_exception_handling
  def test_exceptions_primary(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_primary.cpp')

  @with_both_exception_handling
  def test_exceptions_simplify_cfg(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_simplify_cfg.cpp')

  @with_both_exception_handling
  def test_exceptions_libcxx(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_libcxx.cpp')

  @with_both_exception_handling
  def test_exceptions_multiple_inherit(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_multiple_inherit.cpp')

  @with_both_exception_handling
  def test_exceptions_multiple_inherit_rethrow(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_exceptions_multiple_inherit_rethrow.cpp')

  @with_both_exception_handling
  def test_bad_typeid(self):
    self.do_run(r'''
// exception example
#include <iostream>       // std::cerr
#include <typeinfo>       // operator typeid
#include <exception>      // std::exception

class Polymorphic {virtual void member(){}};

int main () {
  try
  {
    Polymorphic * pb = 0;
    const std::type_info& ti = typeid(*pb);  // throws a bad_typeid exception
  }
  catch (std::exception& e)
  {
    std::cerr << "exception caught: " << e.what() << '\n';
  }
  return 0;
}
''', 'exception caught: std::bad_typeid')

  def test_iostream_ctors(self):
    # iostream stuff must be globally constructed before user global
    # constructors, so iostream works in global constructors
    self.do_run(r'''
#include <iostream>

struct A {
  A() { std::cout << "bug"; }
};
A a;

int main() {
  std::cout << "free code" << std::endl;
  return 0;
}
''', 'bugfree code')

  # Marked as impure since the WASI reactor modules (modules without main)
  # are not yet suppored by the wasm engines we test against.
  @also_with_standalone_wasm(impure=True)
  def test_ctors_no_main(self):
    self.emcc_args.append('--no-entry')
    self.do_run_in_out_file_test('tests', 'core', 'test_ctors_no_main.cpp')

  def test_class(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_class.cpp')

  def test_inherit(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_inherit.cpp')

  def test_isdigit_l(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_isdigit_l.cpp')

  def test_iswdigit(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_iswdigit.cpp')

  def test_polymorph(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_polymorph.cpp')

  def test_complex(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_complex.c')

  def test_float_builtins(self):
    # tests wasm_libc_rt
    self.do_run_in_out_file_test('tests', 'core', 'test_float_builtins.c')

  @no_asan('SAFE_HEAP cannot be used with ASan')
  def test_segfault(self):
    self.set_setting('SAFE_HEAP', 1)

    for addr in ['0', 'new D2()']:
      print(addr)
      src = r'''
        #include <stdio.h>

        struct Classey {
          virtual void doIt() = 0;
        };

        struct D1 : Classey {
          virtual void doIt() { printf("fleefl\n"); }
        };

        struct D2 : Classey {
          virtual void doIt() { printf("marfoosh\n"); }
        };

        int main(int argc, char **argv)
        {
          Classey *p = argc == 100 ? new D1() : (Classey*)%s;

          p->doIt();

          return 0;
        }
      ''' % addr
      if addr.isdigit():
        self.do_run(src, 'segmentation fault', assert_returncode=NON_ZERO)
      else:
        self.do_run(src, 'marfoosh')

  def test_dynamic_cast(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_dynamic_cast.cpp')

  def test_dynamic_cast_b(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_dynamic_cast_b.cpp')

  def test_dynamic_cast_2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_dynamic_cast_2.cpp')

  def test_funcptr(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_funcptr.c')

  def test_mathfuncptr(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_mathfuncptr.c')

  def test_funcptrfunc(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_funcptrfunc.c')

  def test_funcptr_namecollide(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_funcptr_namecollide.c')

  def test_emptyclass(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_emptyclass.cpp')

  def test_alloca(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_alloca.c')

  def test_rename(self):
    self.do_run_in_out_file_test('tests', 'stdio', 'test_rename.c')

  def test_remove(self):
   # needs to flush stdio streams
   self.set_setting('EXIT_RUNTIME', 1)
   self.do_run_in_out_file_test('tests', 'cstdio', 'test_remove.cpp')

  def test_alloca_stack(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_alloca_stack.c')

  def test_stack_byval(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_stack_byval.cpp')

  def test_stack_varargs(self):
    # in node.js we allocate argv[0] on the stack, which means the  length
    # of the program directory influences how much stack we need, and so
    # long random temp dir names can lead to random failures. The stack
    # size was increased here to avoid that.
    self.set_setting('INLINING_LIMIT', 50)
    self.set_setting('TOTAL_STACK', 8 * 1024)

    self.do_run_in_out_file_test('tests', 'core', 'test_stack_varargs.c')

  def test_stack_varargs2(self):
    # in node.js we allocate argv[0] on the stack, which means the  length
    # of the program directory influences how much stack we need, and so
    # long random temp dir names can lead to random failures. The stack
    # size was increased here to avoid that.
    self.set_setting('TOTAL_STACK', 8 * 1024)
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      void func(int i) {
      }
      int main() {
        for (int i = 0; i < 7000; i++) {
          printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                   i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
        }
        printf("ok!\n");
        return 0;
      }
    '''
    self.do_run(src, 'ok!')

    print('with return')

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main() {
        for (int i = 0; i < 7000; i++) {
          int j = printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                   i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
          printf(" (%d)\n", j);
        }
        printf("ok!\n");
        return 0;
      }
    '''
    self.do_run(src, 'ok!')

    print('with definitely no return')

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <stdarg.h>

      void vary(const char *s, ...)
      {
        va_list v;
        va_start(v, s);
        char d[20];
        vsnprintf(d, 20, s, v);
        puts(d);

        // Try it with copying
        va_list tempva;
        va_copy(tempva, v);
        vsnprintf(d, 20, s, tempva);
        puts(d);

        va_end(v);
      }

      int main() {
        for (int i = 0; i < 7000; i++) {
          int j = printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                   i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
          printf(" (%d)\n", j);
          vary("*cheez: %d+%d*", 99, 24);
          vary("*albeit*");
        }
        printf("ok!\n");
        return 0;
      }
    '''
    self.do_run(src, 'ok!')

  def test_stack_void(self):
    self.set_setting('INLINING_LIMIT', 50)

    self.do_run_in_out_file_test('tests', 'core', 'test_stack_void.c')

  def test_life(self):
    self.emcc_args += ['-std=c99']
    self.do_run_in_out_file_test('tests', 'life.c', args=['2'])

  def test_array2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_array2.c')

  def test_array2b(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_array2b.c')

  def test_constglobalstructs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_constglobalstructs.c')

  def test_conststructs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_conststructs.c')

  def test_bigarray(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_bigarray.c')

  def test_mod_globalstruct(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_mod_globalstruct.c')

  def test_sizeof(self):
      # Has invalid writes between printouts
      self.set_setting('SAFE_HEAP', 0)

      self.do_run_in_out_file_test('tests', 'core', 'test_sizeof.cpp')

  def test_llvm_used(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_llvm_used.c')

  @no_asan('SAFE_HEAP cannot be used with ASan')
  def test_set_align(self):
    self.set_setting('SAFE_HEAP', 1)

    self.do_run_in_out_file_test('tests', 'core', 'test_set_align.c')

  def test_emscripten_api(self):
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_save_me_aimee'])
    self.do_run_in_out_file_test('tests', 'core', 'test_emscripten_api.cpp')

    if '-fsanitize=address' not in self.emcc_args:
      # test EXPORT_ALL (this is not compatible with asan, which doesn't
      # support dynamic linking at all or the LINKING flag)
      self.set_setting('EXPORTED_FUNCTIONS', [])
      self.set_setting('EXPORT_ALL', 1)
      self.set_setting('LINKABLE', 1)
      self.do_run_in_out_file_test('tests', 'core', 'test_emscripten_api.cpp')

  def test_emscripten_run_script_string_int(self):
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>

      int main() {
        const char *str = emscripten_run_script_string("1+1");
        printf("got string: %s\n", str);
        return 0;
      }
    '''
    self.do_run(src, '''got string: 2''')

  def test_emscripten_run_script_string_utf8(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      #include <emscripten.h>

      int main() {
        const char *str = emscripten_run_script_string("'\\u2603 \\u2603 \\u2603 Hello!'");
        printf("length of returned string: %d. Position of substring 'Hello': %d\n", strlen(str), strstr(str, "Hello")-str);
        return 0;
      }
    '''
    self.do_run(src, '''length of returned string: 18. Position of substring 'Hello': 12''')

  def test_emscripten_run_script_string_null(self):
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>

      int main() {
        const char *str = emscripten_run_script_string("void(0)");
        if (str) {
          printf("got string: %s\n", str);
        } else {
          puts("got null");
        }
        return 0;
      }
    '''
    self.do_run(src, 'got null')

  def test_emscripten_get_now(self):
    self.banned_js_engines = [V8_ENGINE] # timer limitations in v8 shell
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)

    if self.run_name == 'asm2':
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage
    self.do_runf(path_from_root('tests', 'emscripten_get_now.cpp'), 'Timer resolution is good')

  def test_emscripten_get_compiler_setting(self):
    src = path_from_root('tests', 'core', 'emscripten_get_compiler_setting.c')
    output = shared.unsuffixed(src) + '.out'
    old = self.get_setting('ASSERTIONS')
    # with assertions, a nice message is shown
    self.set_setting('ASSERTIONS', 1)
    self.do_runf(src, 'You must build with -s RETAIN_COMPILER_SETTINGS=1', assert_returncode=NON_ZERO)
    self.set_setting('ASSERTIONS', old)
    self.set_setting('RETAIN_COMPILER_SETTINGS', 1)
    self.do_runf(src, open(output).read().replace('waka', shared.EMSCRIPTEN_VERSION))

  def test_emscripten_has_asyncify(self):
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>

      int main() {
        printf("%d\n", emscripten_has_asyncify());
        return 0;
      }
    '''
    self.set_setting('ASYNCIFY', 0)
    self.do_run(src, '0')
    self.set_setting('ASYNCIFY', 1)
    self.do_run(src, '1')

  # TODO: test only worked in non-fastcomp
  def test_inlinejs(self):
    self.skipTest('non-fastcomp is deprecated and fails in 3.5') # only supports EM_ASM

    self.do_run_in_out_file_test('tests', 'core', 'test_inlinejs.c')

    if self.emcc_args == []:
      # opts will eliminate the comments
      out = open('src.js').read()
      for i in range(1, 5):
        assert ('comment%d' % i) in out

  # TODO: test only worked in non-fastcomp
  def test_inlinejs2(self):
    self.skipTest('non-fastcomp is deprecated and fails in 3.5') # only supports EM_ASM

    self.do_run_in_out_file_test('tests', 'core', 'test_inlinejs2.c')

  def test_inlinejs3(self):
    if self.is_wasm():
      self.skipTest('wasm requires a proper asm module')

    self.emcc_args.append('-Wno-almost-asm')
    src = path_from_root('tests', 'core', 'test_inlinejs3.c')
    output = shared.unsuffixed(src) + '.out'

    self.do_run_in_out_file_test('tests', 'core', 'test_inlinejs3.c')

    print('no debugger, check validation')
    src = open(src).read().replace('emscripten_debugger();', '')
    self.do_run(src, open(output).read())

  def test_inlinejs4(self):
    self.do_run(r'''
#include <emscripten.h>

#define TO_STRING_INNER(x) #x
#define TO_STRING(x) TO_STRING_INNER(x)
#define assert_msg(msg, file, line) EM_ASM( throw 'Assert (' + msg + ') failed in ' + file + ':' + line + '!'; )
#define assert(expr) { \
  if (!(expr)) { \
    assert_msg(#expr, TO_STRING(__FILE__), TO_STRING(__LINE__)); \
  } \
}

int main(int argc, char **argv) {
  assert(argc != 17);
  assert(false);
  return 0;
}
''', 'false', assert_returncode=NON_ZERO)

  def test_em_asm(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm.cpp', force_c=True)

  # Tests various different ways to invoke the EM_ASM(), EM_ASM_INT() and EM_ASM_DOUBLE() macros.
  @no_asan('Cannot use ASan: test depends exactly on heap size')
  def test_em_asm_2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_2.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_2.cpp', force_c=True)

  # Tests various different ways to invoke the MAIN_THREAD_EM_ASM(), MAIN_THREAD_EM_ASM_INT() and MAIN_THREAD_EM_ASM_DOUBLE() macros.
  # This test is identical to test_em_asm_2, just search-replaces EM_ASM to MAIN_THREAD_EM_ASM on the test file. That way if new
  # test cases are added to test_em_asm_2.cpp for EM_ASM, they will also get tested in MAIN_THREAD_EM_ASM form.
  @no_asan('Cannot use ASan: test depends exactly on heap size')
  def test_main_thread_em_asm(self):
    src = open(path_from_root('tests', 'core', 'test_em_asm_2.cpp')).read()
    create_test_file('src.cpp', src.replace('EM_ASM', 'MAIN_THREAD_EM_ASM'))

    expected_result = open(path_from_root('tests', 'core', 'test_em_asm_2.out')).read()
    create_test_file('result.out', expected_result.replace('EM_ASM', 'MAIN_THREAD_EM_ASM'))

    self.do_run_from_file('src.cpp', 'result.out')
    self.do_run_from_file('src.cpp', 'result.out', force_c=True)

  def test_main_thread_async_em_asm(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_main_thread_async_em_asm.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_main_thread_async_em_asm.cpp', force_c=True)

  # Tests MAIN_THREAD_EM_ASM_INT() function call with different signatures.
  def test_main_thread_em_asm_signatures(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_signatures.cpp', assert_returncode=NON_ZERO)

  def test_em_asm_unicode(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_unicode.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_unicode.cpp', force_c=True)

  def test_em_asm_types(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_types.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_types.cpp', force_c=True)

  def test_em_asm_unused_arguments(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_unused_arguments.cpp')

  # Verify that EM_ASM macros support getting called with multiple arities.
  # Maybe tests will later be joined into larger compilation units?
  # Then this must still be compiled separately from other code using EM_ASM
  # macros with arities 1-3. Otherwise this may incorrectly report a success.
  def test_em_asm_parameter_pack(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_parameter_pack.cpp')

  def test_em_asm_arguments_side_effects(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_arguments_side_effects.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_em_asm_arguments_side_effects.cpp', force_c=True)

  @parameterized({
    'normal': ([],),
    'linked': (['-s', 'MAIN_MODULE'],),
  })
  def test_em_js(self, args):
    if 'MAIN_MODULE' in args and self.get_setting('WASM') == 0:
      self.skipTest('main module support for non-wasm')
    if '-fsanitize=address' in self.emcc_args:
      self.skipTest('no dynamic library support in asan yet')
    self.emcc_args += args + ['-s', 'EXPORTED_FUNCTIONS=["_main","_malloc"]']
    self.do_run_in_out_file_test('tests', 'core', 'test_em_js.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_em_js.cpp', force_c=True)

  def test_runtime_stacksave(self):
    src = open(path_from_root('tests', 'core', 'test_runtime_stacksave.c')).read()
    self.do_run(src, 'success')

  # Tests that -s MINIMAL_RUNTIME=1 builds can utilize -s ALLOW_MEMORY_GROWTH=1 option.
  def test_minimal_runtime_memorygrowth(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')
    self.set_setting('MINIMAL_RUNTIME', 1)
    src = open(path_from_root('tests', 'core', 'test_memorygrowth.c')).read()
    # Fail without memory growth
    expect_fail = False
    if self.get_setting('WASM') == 0:
      expect_fail = True
    self.do_run(src, 'OOM', assert_returncode=NON_ZERO if expect_fail else 0)
    # Win with it
    self.emcc_args += ['-Wno-almost-asm', '-s', 'ALLOW_MEMORY_GROWTH']
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')

  def test_memorygrowth(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')
    if self.maybe_closure():
      # verify NO_DYNAMIC_EXECUTION is compatible with closure
      self.set_setting('DYNAMIC_EXECUTION', 0)
    # With typed arrays in particular, it is dangerous to use more memory than INITIAL_MEMORY,
    # since we then need to enlarge the heap(s).
    src = open(path_from_root('tests', 'core', 'test_memorygrowth.c')).read()

    # Fail without memory growth
    self.do_run(src, 'OOM', assert_returncode=NON_ZERO)
    fail = open('src.js').read()

    # Win with it
    self.emcc_args += ['-Wno-almost-asm', '-s', 'ALLOW_MEMORY_GROWTH']
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')
    win = open('src.js').read()

    if '-O2' in self.emcc_args and not self.is_wasm():
      # Make sure ALLOW_MEMORY_GROWTH generates different code (should be less optimized)
      possible_starts = ['// EMSCRIPTEN_START_FUNCS', 'var TOTAL_STACK']
      code_start = None
      for s in possible_starts:
        if fail.find(s) >= 0:
          code_start = s
          break
      assert code_start is not None, 'Generated code must contain one of ' + str(possible_starts)

      fail = fail[fail.find(code_start):]
      win = win[win.find(code_start):]
      assert len(fail) < len(win), 'failing code - without memory growth on - is more optimized, and smaller' + str([len(fail), len(win)])

    # Tracing of memory growths should work
    self.set_setting('EMSCRIPTEN_TRACING', 1)
    self.emcc_args += ['--tracing']
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')

  def test_memorygrowth_2(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')

    # With typed arrays in particular, it is dangerous to use more memory than INITIAL_MEMORY,
    # since we then need to enlarge the heap(s).
    src = open(path_from_root('tests', 'core', 'test_memorygrowth_2.c')).read()

    # Fail without memory growth
    self.do_run(src, 'OOM', assert_returncode=NON_ZERO)
    fail = open('src.js').read()

    # Win with it
    self.emcc_args += ['-Wno-almost-asm', '-s', 'ALLOW_MEMORY_GROWTH']
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')
    win = open('src.js').read()

    if '-O2' in self.emcc_args and not self.is_wasm():
      # Make sure ALLOW_MEMORY_GROWTH generates different code (should be less optimized)
      assert len(fail) < len(win), 'failing code - without memory growth on - is more optimized, and smaller' + str([len(fail), len(win)])

  def test_memorygrowth_3(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')

    # checks handling of malloc failure properly
    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=0', '-s', 'ABORTING_MALLOC=0', '-s', 'SAFE_HEAP']
    self.do_run_in_out_file_test('tests', 'core', 'test_memorygrowth_3.c')

  @also_with_standalone_wasm(impure=True)
  def test_memorygrowth_MAXIMUM_MEMORY(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')
    if not self.is_wasm():
      self.skipTest('wasm memory specific test')

    # check that memory growth does not exceed the wasm mem max limit
    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'INITIAL_MEMORY=64Mb', '-s', 'MAXIMUM_MEMORY=100Mb']
    self.do_run_in_out_file_test('tests', 'core', 'test_memorygrowth_wasm_mem_max.c')

  def test_memorygrowth_linear_step(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')
    if not self.is_wasm():
      self.skipTest('wasm memory specific test')

    # check that memory growth does not exceed the wasm mem max limit and is exactly or one step below the wasm mem max
    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'TOTAL_STACK=1Mb', '-s', 'INITIAL_MEMORY=64Mb', '-s', 'MAXIMUM_MEMORY=130Mb', '-s', 'MEMORY_GROWTH_LINEAR_STEP=1Mb']
    self.do_run_in_out_file_test('tests', 'core', 'test_memorygrowth_memory_growth_step.c')

  def test_memorygrowth_geometric_step(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')
    if not self.is_wasm():
      self.skipTest('wasm memory specific test')

    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'MEMORY_GROWTH_GEOMETRIC_STEP=15', '-s', 'MEMORY_GROWTH_GEOMETRIC_CAP=0']
    self.do_run_in_out_file_test('tests', 'core', 'test_memorygrowth_geometric_step.c')

  def test_memorygrowth_3_force_fail_reallocBuffer(self):
    if self.has_changed_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('test needs to modify memory growth')

    self.emcc_args += ['-Wno-almost-asm', '-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'TEST_MEMORY_GROWTH_FAILS=1']
    self.do_run_in_out_file_test('tests', 'core', 'test_memorygrowth_3.c')

  @parameterized({
    'nogrow': (['-s', 'ALLOW_MEMORY_GROWTH=0'],),
    'grow': (['-s', 'ALLOW_MEMORY_GROWTH=1'],)
  })
  @no_asan('requires more memory when growing')
  def test_aborting_new(self, args):
    # test that C++ new properly errors if we fail to malloc when growth is
    # enabled, with or without growth
    self.emcc_args += ['-Wno-almost-asm', '-s', 'MAXIMUM_MEMORY=18MB'] + args
    self.do_run_in_out_file_test('tests', 'core', 'test_aborting_new.cpp')

  @no_asmjs()
  @no_wasm2js('no WebAssembly.Memory()')
  @no_asan('ASan alters the memory size')
  def test_module_wasm_memory(self):
    self.emcc_args += ['--pre-js', path_from_root('tests', 'core', 'test_module_wasm_memory.js')]
    self.do_runf(path_from_root('tests', 'core', 'test_module_wasm_memory.c'), 'success')

  def test_ssr(self): # struct self-ref
      src = '''
        #include <stdio.h>

        // see related things in openjpeg
        typedef struct opj_mqc_state {
          unsigned int qeval;
          int mps;
          struct opj_mqc_state *nmps;
          struct opj_mqc_state *nlps;
        } opj_mqc_state_t;

        static opj_mqc_state_t mqc_states[4] = {
          {0x5600, 0, &mqc_states[2], &mqc_states[3]},
          {0x5602, 1, &mqc_states[3], &mqc_states[2]},
        };

        int main() {
          printf("*%d*\\n", (int)(mqc_states+1)-(int)mqc_states);
          for (int i = 0; i < 2; i++)
            printf("%d:%d,%d,%d,%d\\n", i, mqc_states[i].qeval, mqc_states[i].mps,
                   (int)mqc_states[i].nmps-(int)mqc_states, (int)mqc_states[i].nlps-(int)mqc_states);
          return 0;
        }
        '''
      self.do_run(src, '''*16*\n0:22016,0,32,48\n1:22018,1,48,32\n''')

  def test_tinyfuncstr(self):
      self.do_run_in_out_file_test('tests', 'core', 'test_tinyfuncstr.cpp')

  def test_llvmswitch(self):
      self.do_run_in_out_file_test('tests', 'core', 'test_llvmswitch.c')

  def test_cxx_version(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_cxx_version.cpp')

  @no_wasm2js('massive switches can break js engines')
  def test_bigswitch(self):
    src = open(path_from_root('tests', 'bigswitch.cpp')).read()
    self.do_run(src, '''34962: GL_ARRAY_BUFFER (0x8892)
26214: what?
35040: GL_STREAM_DRAW (0x88E0)
3060: what?
''', args=['34962', '26214', '35040', str(0xbf4)])

  @no_wasm2js('massive switches can break js engines')
  @is_slow_test
  def test_biggerswitch(self):
    if not is_optimizing(self.emcc_args):
      self.skipTest('nodejs takes >6GB to compile this if the wasm is not optimized, which OOMs, see https://github.com/emscripten-core/emscripten/issues/7928#issuecomment-458308453')
    if '-Os' in self.emcc_args:
      self.skipTest('hangs in recent upstream clang, see https://bugs.llvm.org/show_bug.cgi?id=43468')
    num_cases = 20000
    switch_case = self.run_process([PYTHON, path_from_root('tests', 'gen_large_switchcase.py'), str(num_cases)], stdout=PIPE, stderr=PIPE).stdout
    self.do_run(switch_case, '''58996: 589965899658996
59297: 592975929759297
59598: default
59899: 598995989959899
Success!''')

  def test_indirectbr(self):
      self.emcc_args = [x for x in self.emcc_args if x != '-g']

      self.do_run_in_out_file_test('tests', 'core', 'test_indirectbr.c')

  @no_asan('local count too large for VMs')
  @no_wasm2js('extremely deep nesting, hits stack limit on some VMs')
  def test_indirectbr_many(self):
      self.do_run_in_out_file_test('tests', 'core', 'test_indirectbr_many.c')

  def test_pack(self):
      src = '''
        #include <stdio.h>
        #include <string.h>

        #pragma pack(push,1)
        typedef struct header
        {
            unsigned char  id;
            unsigned short colour;
            unsigned char  desc;
        } header;
        #pragma pack(pop)

        typedef struct fatheader
        {
            unsigned char  id;
            unsigned short colour;
            unsigned char  desc;
        } fatheader;

        int main( int argc, const char *argv[] ) {
          header h, *ph = 0;
          fatheader fh, *pfh = 0;
          printf("*%d,%d,%d*\\n", sizeof(header), (int)((int)&h.desc - (int)&h.id), (int)(&ph[1])-(int)(&ph[0]));
          printf("*%d,%d,%d*\\n", sizeof(fatheader), (int)((int)&fh.desc - (int)&fh.id), (int)(&pfh[1])-(int)(&pfh[0]));
          return 0;
        }
        '''
      self.do_run(src, '*4,3,4*\n*6,4,6*')

  def test_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_varargs.c')

  @no_wasm_backend('Calling varargs across function calls is undefined behavior in C,'
                   ' and asmjs and wasm implement it differently.')
  def test_varargs_multi(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_varargs_multi.c')

  @unittest.skip('clang cannot compile this code with that target yet')
  def test_varargs_byval(self):
    src = r'''
      #include <stdio.h>
      #include <stdarg.h>

      typedef struct type_a {
        union {
          double f;
          void *p;
          int i;
          short sym;
        } value;
      } type_a;

      enum mrb_vtype {
        MRB_TT_FALSE = 0,   /*   0 */
        MRB_TT_CLASS = 9    /*   9 */
      };

      typedef struct type_b {
        enum mrb_vtype tt:8;
      } type_b;

      void print_type_a(int argc, ...);
      void print_type_b(int argc, ...);

      int main(int argc, char *argv[])
      {
        type_a a;
        type_b b;
        a.value.p = (void*) 0x12345678;
        b.tt = MRB_TT_CLASS;

        printf("The original address of a is: %p\n", a.value.p);
        printf("The original type of b is: %d\n", b.tt);

        print_type_a(1, a);
        print_type_b(1, b);

        return 0;
      }

      void print_type_a(int argc, ...) {
        va_list ap;
        type_a a;

        va_start(ap, argc);
        a = va_arg(ap, type_a);
        va_end(ap);

        printf("The current address of a is: %p\n", a.value.p);
      }

      void print_type_b(int argc, ...) {
        va_list ap;
        type_b b;

        va_start(ap, argc);
        b = va_arg(ap, type_b);
        va_end(ap);

        printf("The current type of b is: %d\n", b.tt);
      }
      '''
    self.do_run(src, '''The original address of a is: 0x12345678
The original type of b is: 9
The current address of a is: 0x12345678
The current type of b is: 9
''')

  def test_functionpointer_libfunc_varargs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_functionpointer_libfunc_varargs.c')

  def test_structbyval(self):
    self.set_setting('INLINING_LIMIT', 50)

    # part 1: make sure that normally, passing structs by value works

    src = r'''
      #include <stdio.h>

      struct point
      {
        int x, y;
      };

      void dump(struct point p) {
        p.x++; // should not modify
        p.y++; // anything in the caller!
        printf("dump: %d,%d\n", p.x, p.y);
      }

      void dumpmod(struct point *p) {
        p->x++; // should not modify
        p->y++; // anything in the caller!
        printf("dump: %d,%d\n", p->x, p->y);
      }

      int main( int argc, const char *argv[] ) {
        point p = { 54, 2 };
        printf("pre:  %d,%d\n", p.x, p.y);
        dump(p);
        void (*dp)(point p) = dump; // And, as a function pointer
        dp(p);
        printf("post: %d,%d\n", p.x, p.y);
        dumpmod(&p);
        dumpmod(&p);
        printf("last: %d,%d\n", p.x, p.y);
        return 0;
      }
    '''
    self.do_run(src, 'pre:  54,2\ndump: 55,3\ndump: 55,3\npost: 54,2\ndump: 55,3\ndump: 56,4\nlast: 56,4')

  def test_stdlibs(self):
    # safe heap prints a warning that messes up our output.
    self.set_setting('SAFE_HEAP', 0)
    # needs atexit
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_stdlibs.c')

  def test_stdbool(self):
    create_test_file('test_stdbool.c', r'''
        #include <stdio.h>
        #include <stdbool.h>

        int main() {
          bool x = true;
          bool y = false;
          printf("*%d*\n", x != y);
          return 0;
        }
      ''')

    self.do_runf('test_stdbool.c', '*1*')

  def test_strtoll_hex(self):
    # tests strtoll for hex strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtoll_hex.c')

  def test_strtoll_dec(self):
    # tests strtoll for decimal strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtoll_dec.c')

  def test_strtoll_bin(self):
    # tests strtoll for binary strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtoll_bin.c')

  def test_strtoll_oct(self):
    # tests strtoll for decimal strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtoll_oct.c')

  def test_strtol_hex(self):
    # tests strtoll for hex strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtol_hex.c')

  def test_strtol_dec(self):
    # tests strtoll for decimal strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtol_dec.c')

  def test_strtol_bin(self):
    # tests strtoll for binary strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtol_bin.c')

  def test_strtol_oct(self):
    # tests strtoll for decimal strings (0x...)
    self.do_run_in_out_file_test('tests', 'core', 'test_strtol_oct.c')

  @also_with_standalone_wasm()
  def test_atexit(self):
    # Confirms they are called in the proper reverse order
    if not self.get_setting('STANDALONE_WASM'):
      # STANDALONE_WASM mode always sets EXIT_RUNTIME if main exists
      self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_atexit.c')

  def test_atexit_threads(self):
    # also tests thread exit (__cxa_thread_atexit)
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_atexit_threads.c')

  @no_asan('test relies on null pointer reads')
  def test_pthread_specific(self):
    self.do_run_in_out_file_test('tests', 'pthread', 'specific.c')

  def test_pthread_equal(self):
    self.do_run_in_out_file_test('tests', 'pthread', 'test_pthread_equal.cpp')

  def test_tcgetattr(self):
    self.do_runf(path_from_root('tests', 'termios', 'test_tcgetattr.c'), 'success')

  def test_time(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_time.cpp')
    for tz in ['EST+05EDT', 'UTC+0']:
      print('extra tz test:', tz)
      with env_modify({'TZ': tz}):
        # Run the test with different time zone settings if
        # possible. It seems that the TZ environment variable does not
        # work all the time (at least it's not well respected by
        # Node.js on Windows), but it does no harm either.
        self.do_run_in_out_file_test('tests', 'core', 'test_time.cpp')

  def test_timeb(self):
    # Confirms they are called in reverse order
    self.do_run_in_out_file_test('tests', 'core', 'test_timeb.c')

  def test_time_c(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_time_c.c')

  def test_gmtime(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_gmtime.c')

  def test_strptime_tm(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strptime_tm.c')

  def test_strptime_days(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strptime_days.c')

  def test_strptime_reentrant(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_strptime_reentrant.c')

  def test_strftime(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strftime.cpp')

  @no_wasm_backend("wasm backend doesn't compile intentional segfault into an abort() call. "
                   "It also doesn't segfault.")
  def test_intentional_fault(self):
    # Some programs intentionally segfault themselves, we should compile that into a throw
    src = open(path_from_root('tests', 'core', 'test_intentional_fault.c')).read()
    self.do_run(src, 'abort(' if self.run_name != 'asm2g' else 'abort(segmentation fault', assert_returncode=NON_ZERO)

  def test_trickystring(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_trickystring.c')

  def test_statics(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_statics.cpp')

  def test_copyop(self):
    # clang generated code is vulnerable to this, as it uses
    # memcpy for assignments, with hardcoded numbers of bytes
    # (llvm-gcc copies items one by one).
    self.do_run_in_out_file_test('tests', 'core', 'test_copyop.cpp')

  def test_memcpy_memcmp(self):
    self.banned_js_engines = [V8_ENGINE] # Currently broken under V8_ENGINE but not node

    def check(result, err):
      result = result.replace('\n \n', '\n') # remove extra node output
      return hashlib.sha1(result.encode('utf-8')).hexdigest()

    self.do_run_in_out_file_test('tests', 'core', 'test_memcpy_memcmp.c', output_nicerizer=check)

  def test_memcpy2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_memcpy2.c')

  def test_memcpy3(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_memcpy3.c')

  @also_with_standalone_wasm()
  def test_memcpy_alignment(self):
    self.do_runf(path_from_root('tests', 'test_memcpy_alignment.cpp'), 'OK.')

  def test_memset_alignment(self):
    self.do_runf(path_from_root('tests', 'test_memset_alignment.cpp'), 'OK.')

  def test_memset(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_memset.c')

  def test_getopt(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_getopt.c', args=['-t', '12', '-n', 'foobar'])

  def test_getopt_long(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_getopt_long.c', args=['--file', 'foobar', '-b'])

  def test_memmove(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_memmove.c')

  def test_memmove2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_memmove2.c')

  def test_memmove3(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_memmove3.c')

  def test_flexarray_struct(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_flexarray_struct.c')

  def test_bsearch(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_bsearch.c')

  @no_wasm_backend("https://github.com/emscripten-core/emscripten/issues/9039")
  def test_stack_overflow(self):
    self.set_setting('ASSERTIONS', 1)
    self.do_runf(path_from_root('tests', 'core', 'stack_overflow.cpp'), 'Stack overflow!', assert_returncode=NON_ZERO)

  def test_stackAlloc(self):
    self.do_run_in_out_file_test('tests', 'core', 'stackAlloc.cpp')

  def test_nestedstructs(self):
      src = '''
        #include <stdio.h>
        #include "emscripten.h"

        struct base {
          int x;
          float y;
          union {
            int a;
            float b;
          };
          char c;
        };

        struct hashtableentry {
          int key;
          base data;
        };

        struct hashset {
          typedef hashtableentry entry;
          struct chain { entry elem; chain *next; };
        //  struct chainchunk { chain chains[100]; chainchunk *next; };
        };

        struct hashtable : hashset {
          hashtable() {
            base *b = NULL;
            entry *e = NULL;
            chain *c = NULL;
            printf("*%d,%d,%d,%d,%d,%d|%d,%d,%d,%d,%d,%d,%d,%d|%d,%d,%d,%d,%d,%d,%d,%d,%d,%d*\\n",
              sizeof(base),
              int(&(b->x)), int(&(b->y)), int(&(b->a)), int(&(b->b)), int(&(b->c)),
              sizeof(hashtableentry),
              int(&(e->key)), int(&(e->data)), int(&(e->data.x)), int(&(e->data.y)), int(&(e->data.a)), int(&(e->data.b)), int(&(e->data.c)),
              sizeof(hashset::chain),
              int(&(c->elem)), int(&(c->next)), int(&(c->elem.key)), int(&(c->elem.data)), int(&(c->elem.data.x)), int(&(c->elem.data.y)), int(&(c->elem.data.a)), int(&(c->elem.data.b)), int(&(c->elem.data.c))
            );
          }
        };

        struct B { char buffer[62]; int last; char laster; char laster2; };

        struct Bits {
          unsigned short A : 1;
          unsigned short B : 1;
          unsigned short C : 1;
          unsigned short D : 1;
          unsigned short x1 : 1;
          unsigned short x2 : 1;
          unsigned short x3 : 1;
          unsigned short x4 : 1;
        };

        int main() {
          hashtable t;

          // Part 2 - the char[] should be compressed, BUT have a padding space at the end so the next
          // one is aligned properly. Also handle char; char; etc. properly.
          B *b = NULL;
          printf("*%d,%d,%d,%d,%d,%d,%d,%d,%d*\\n", int(b), int(&(b->buffer)), int(&(b->buffer[0])), int(&(b->buffer[1])), int(&(b->buffer[2])),
                                                    int(&(b->last)), int(&(b->laster)), int(&(b->laster2)), sizeof(B));

          // Part 3 - bitfields, and small structures
          Bits *b2 = NULL;
          printf("*%d*\\n", sizeof(Bits));

          return 0;
        }
        '''
      # Bloated memory; same layout as C/C++
      self.do_run(src, '*16,0,4,8,8,12|20,0,4,4,8,12,12,16|24,0,20,0,4,4,8,12,12,16*\n*0,0,0,1,2,64,68,69,72*\n*2*')

  def prep_dlfcn_lib(self):
    self.clear_setting('MAIN_MODULE')
    self.set_setting('SIDE_MODULE')

  def prep_dlfcn_main(self):
    self.set_setting('MAIN_MODULE')
    self.clear_setting('SIDE_MODULE')

    create_test_file('lib_so_pre.js', '''
    if (!Module['preRun']) Module['preRun'] = [];
    Module['preRun'].push(function() { FS.createDataFile('/', 'liblib.so', %s, true, false, false); });
''' % str(list(bytearray(open('liblib.so', 'rb').read()))))
    self.emcc_args += ['--pre-js', 'lib_so_pre.js']

  def build_dlfcn_lib(self, filename):
    if self.get_setting('WASM'):
      # emcc emits a wasm in this case
      self.build(filename, js_outfile=False)
      shutil.move(shared.unsuffixed(filename) + '.wasm', 'liblib.so')
    else:
      self.build(filename)
      shutil.move(shared.unsuffixed(filename) + '.js', 'liblib.so')

  @needs_dlfcn
  def test_dlfcn_missing(self):
    self.set_setting('MAIN_MODULE', 1)
    if self.has_changed_setting('ASSERTIONS'):
      self.skipTest('test needs to customize ASSERTIONS')
    self.set_setting('ASSERTIONS', 1)
    src = r'''
      #include <dlfcn.h>
      #include <stdio.h>
      #include <assert.h>

      int main() {
        void* lib_handle = dlopen("libfoo.so", RTLD_NOW);
        assert(!lib_handle);
        printf("error: %s\n", dlerror());
        return 0;
      }
      '''
    self.do_run(src, 'error: Could not load dynamic lib: libfoo.so\nError: No such file or directory')
    print('without assertions, the error is less clear')
    self.set_setting('ASSERTIONS', 0)
    self.do_run(src, 'error: Could not load dynamic lib: libfoo.so\nError: FS error')

  @needs_dlfcn
  def test_dlfcn_basic(self):
    self.prep_dlfcn_lib()
    create_test_file('liblib.cpp', '''
      #include <cstdio>

      class Foo {
      public:
        Foo() {
          puts("Constructing lib object.");
        }
      };

      Foo global;
      ''')
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    src = '''
      #include <cstdio>
      #include <dlfcn.h>

      class Bar {
      public:
        Bar() {
          puts("Constructing main object.");
        }
      };

      Bar global;

      int main() {
        dlopen("liblib.so", RTLD_NOW);
        return 0;
      }
      '''
    self.do_run(src, 'Constructing main object.\nConstructing lib object.\n')

  @needs_dlfcn
  def test_dlfcn_i64(self):
    self.prep_dlfcn_lib()
    self.set_setting('EXPORTED_FUNCTIONS', ['_foo'])
    create_test_file('liblib.c', '''
      int foo(int x) {
        return (long long)x / (long long)1234;
      }
      ''')
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    self.clear_setting('EXPORTED_FUNCTIONS')
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <dlfcn.h>

      typedef int (*intfunc)(int);

      void *p;

      int main() {
        p = malloc(1024);
        void *lib_handle = dlopen("liblib.so", 0);
        if (!lib_handle) {
          puts(dlerror());
          abort();
        }
        printf("dll handle: %p\n", lib_handle);
        intfunc x = (intfunc)dlsym(lib_handle, "foo");
        printf("foo func handle: %p\n", x);
        if (p == 0) return 1;
        if (!x) {
          printf("dlsym failed: %s\n", dlerror());
          return 1;
        }
        printf("|%d|\n", x(81234567));
        return 0;
      }
      '''
    self.do_run(src, '|65830|')

  @needs_dlfcn
  @no_wasm('EM_ASM in shared wasm modules, stored inside the wasm somehow')
  def test_dlfcn_em_asm(self):
    self.prep_dlfcn_lib()
    create_test_file('liblib.cpp', '''
      #include <emscripten.h>
      class Foo {
      public:
        Foo() {
          EM_ASM( out("Constructing lib object.") );
        }
      };
      Foo global;
      ''')
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    src = '''
      #include <emscripten.h>
      #include <dlfcn.h>
      class Bar {
      public:
        Bar() {
          EM_ASM( out("Constructing main object.") );
        }
      };
      Bar global;
      int main() {
        dlopen("liblib.so", RTLD_NOW);
        EM_ASM( out("All done.") );
        return 0;
      }
      '''
    self.do_run(src, 'Constructing main object.\nConstructing lib object.\nAll done.\n')

  @needs_dlfcn
  def test_dlfcn_qsort(self):
    self.prep_dlfcn_lib()
    self.set_setting('EXPORTED_FUNCTIONS', ['_get_cmp'])
    create_test_file('liblib.cpp', '''
      int lib_cmp(const void* left, const void* right) {
        const int* a = (const int*) left;
        const int* b = (const int*) right;
        if(*a > *b) return 1;
        else if(*a == *b) return  0;
        else return -1;
      }

      typedef int (*CMP_TYPE)(const void*, const void*);

      extern "C" CMP_TYPE get_cmp() {
        return lib_cmp;
      }
      ''')
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc'])
    src = '''
      #include <stdio.h>
      #include <stdlib.h>
      #include <dlfcn.h>

      typedef int (*CMP_TYPE)(const void*, const void*);

      int main_cmp(const void* left, const void* right) {
        const int* a = (const int*) left;
        const int* b = (const int*) right;
        if(*a < *b) return 1;
        else if(*a == *b) return  0;
        else return -1;
      }

      int main() {
        void* lib_handle;
        CMP_TYPE (*getter_ptr)();
        CMP_TYPE lib_cmp_ptr;
        int arr[5] = {4, 2, 5, 1, 3};

        qsort((void*)arr, 5, sizeof(int), main_cmp);
        printf("Sort with main comparison: ");
        for (int i = 0; i < 5; i++) {
          printf("%d ", arr[i]);
        }
        printf("\\n");

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        if (lib_handle == NULL) {
          printf("Could not load lib.\\n");
          return 1;
        }
        getter_ptr = (CMP_TYPE (*)()) dlsym(lib_handle, "get_cmp");
        if (getter_ptr == NULL) {
          printf("Could not find func.\\n");
          return 1;
        }
        lib_cmp_ptr = getter_ptr();
        qsort((void*)arr, 5, sizeof(int), lib_cmp_ptr);
        printf("Sort with lib comparison: ");
        for (int i = 0; i < 5; i++) {
          printf("%d ", arr[i]);
        }
        printf("\\n");

        return 0;
      }
      '''
    self.do_run(src, 'Sort with main comparison: 5 4 3 2 1 *Sort with lib comparison: 1 2 3 4 5 *',
                output_nicerizer=lambda x, err: x.replace('\n', '*'))

  @needs_dlfcn
  def test_dlfcn_data_and_fptr(self):
    # Failing under v8 since: https://chromium-review.googlesource.com/712595
    if self.is_wasm():
      self.banned_js_engines = [V8_ENGINE]

    self.prep_dlfcn_lib()
    create_test_file('liblib.cpp', r'''
      #include <stdio.h>

      int theglobal = 42;

      extern void parent_func(); // a function that is defined in the parent

      int* lib_get_global_addr() {
        return &theglobal;
      }

      void lib_fptr() {
        printf("Second calling lib_fptr from main.\n");
        parent_func();
        // call it also through a pointer, to check indexizing
        void (*p_f)();
        p_f = parent_func;
        p_f();
      }

      extern "C" void (*func(int x, void(*fptr)()))() {
        printf("In func: %d\n", x);
        fptr();
        return lib_fptr;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_func'])
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    src = r'''
      #include <stdio.h>
      #include <dlfcn.h>
      #include <emscripten.h>

      typedef void (*FUNCTYPE(int, void(*)()))();

      FUNCTYPE func;

      void EMSCRIPTEN_KEEPALIVE parent_func() {
        printf("parent_func called from child\n");
      }

      void main_fptr() {
        printf("First calling main_fptr from lib.\n");
      }

      int main() {
        void* lib_handle;
        FUNCTYPE* func_fptr;

        // Test basic lib loading.
        lib_handle = dlopen("liblib.so", RTLD_NOW);
        if (lib_handle == NULL) {
          printf("Could not load lib.\n");
          return 1;
        }

        // Test looked up function.
        func_fptr = (FUNCTYPE*) dlsym(lib_handle, "func");
        // Load twice to test cache.
        func_fptr = (FUNCTYPE*) dlsym(lib_handle, "func");
        if (func_fptr == NULL) {
          printf("Could not find func.\n");
          return 1;
        }

        // Test passing function pointers across module bounds.
        void (*fptr)() = func_fptr(13, main_fptr);
        fptr();

        // Test global data.
        int* globaladdr = (int*) dlsym(lib_handle, "theglobal");
        if (globaladdr == NULL) {
          printf("Could not find global.\n");
          return 1;
        }

        printf("Var: %d\n", *globaladdr);

        return 0;
      }
      '''
    self.set_setting('EXPORTED_FUNCTIONS', ['_main'])
    self.do_run(src, '''\
In func: 13
First calling main_fptr from lib.
Second calling lib_fptr from main.
parent_func called from child
parent_func called from child
Var: 42
''')

  @needs_dlfcn
  def test_dlfcn_varargs(self):
    # this test is not actually valid - it fails natively. the child should fail
    # to be loaded, not load and successfully see the parent print_ints func

    self.prep_dlfcn_lib()
    create_test_file('liblib.cpp', r'''
      void print_ints(int n, ...);
      extern "C" void func() {
        print_ints(2, 13, 42);
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_func'])
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    src = r'''
      #include <stdarg.h>
      #include <stdio.h>
      #include <dlfcn.h>
      #include <assert.h>

      void print_ints(int n, ...) {
        va_list args;
        va_start(args, n);
        for (int i = 0; i < n; i++) {
          printf("%d\n", va_arg(args, int));
        }
        va_end(args);
      }

      int main() {
        void* lib_handle;
        void (*fptr)();

        print_ints(2, 100, 200);

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle);
        fptr = (void (*)())dlsym(lib_handle, "func");
        fptr();

        return 0;
      }
      '''
    self.set_setting('EXPORTED_FUNCTIONS', ['_main'])
    self.do_run(src, '100\n200\n13\n42\n')

  @needs_dlfcn
  def test_dlfcn_alignment_and_zeroing(self):
    self.prep_dlfcn_lib()
    self.set_setting('INITIAL_MEMORY', 16 * 1024 * 1024)
    create_test_file('liblib.cpp', r'''
      extern "C" {
        int prezero = 0;
        __attribute__((aligned(1024))) int superAligned = 12345;
        int postzero = 0;
      }
      ''')
    self.build_dlfcn_lib('liblib.cpp')
    for i in range(10):
      curr = '%d.so' % i
      shutil.copyfile('liblib.so', curr)
      self.emcc_args += ['--embed-file', curr]

    self.prep_dlfcn_main()
    self.set_setting('INITIAL_MEMORY', 128 * 1024 * 1024)
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      #include <dlfcn.h>
      #include <assert.h>
      #include <emscripten.h>

      int main() {
        printf("'prepare' memory with non-zero inited stuff\n");
        int num = 120 * 1024 * 1024; // total is 128; we'll use 5*5 = 25 at least, so allocate pretty much all of it
        void* mem = malloc(num);
        assert(mem);
        printf("setting this range to non-zero: %d - %d\n", int(mem), int(mem) + num);
        memset(mem, 1, num);
        EM_ASM({
          var value = HEAP8[64*1024*1024];
          out('verify middle of memory is non-zero: ' + value);
          assert(value === 1);
        });
        free(mem);
        for (int i = 0; i < 10; i++) {
          char curr[] = "?.so";
          curr[0] = '0' + i;
          printf("loading %s\n", curr);
          void* lib_handle = dlopen(curr, RTLD_NOW);
          if (!lib_handle) {
            puts(dlerror());
            assert(0);
          }
          printf("getting superAligned\n");
          int* superAligned = (int*)dlsym(lib_handle, "superAligned");
          assert(superAligned);
          assert(int(superAligned) % 1024 == 0); // alignment
          printf("checking value of superAligned, at %d\n", superAligned);
          assert(*superAligned == 12345); // value
          printf("getting prezero\n");
          int* prezero = (int*)dlsym(lib_handle, "prezero");
          assert(prezero);
          printf("checking value of prezero, at %d\n", prezero);
          assert(*prezero == 0);
          *prezero = 1;
          assert(*prezero != 0);
          printf("getting postzero\n");
          int* postzero = (int*)dlsym(lib_handle, "postzero");
          printf("checking value of postzero, at %d\n", postzero);
          assert(postzero);
          printf("checking value of postzero\n");
          assert(*postzero == 0);
          *postzero = 1;
          assert(*postzero != 0);
        }
        printf("success.\n");
        return 0;
      }
      '''
    self.do_run(src, 'success.\n')

  @needs_dlfcn
  def test_dlfcn_self(self):
    self.set_setting('MAIN_MODULE')
    self.set_setting('EXPORT_ALL')
    # TODO(https://github.com/emscripten-core/emscripten/issues/11121)
    # We link with C++ stdlibs, even when linking with emcc for historical reasons.  We can remove
    # this if this issues is fixed.
    self.emcc_args.append('-nostdlib++')

    def post(filename):
      js = open(filename).read()
      start = js.find('var NAMED_GLOBALS')
      first = js.find('{', start)
      last = js.find('}', start)
      exports = js[first + 1:last]
      exports = exports.split(',')
      # ensure there aren't too many globals; we don't want unnamed_addr
      exports = [e.split(':')[0].strip('"') for e in exports]
      exports.sort()
      self.assertGreater(len(exports), 20)
      # wasm backend includes alias in NAMED_GLOBALS
      self.assertLess(len(exports), 56)

    self.do_run_in_out_file_test('tests', 'core', 'test_dlfcn_self.c', post_build=post)

  @needs_dlfcn
  def test_dlfcn_unique_sig(self):
    self.prep_dlfcn_lib()
    create_test_file('liblib.c', r'''
      #include <stdio.h>

      int myfunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m) {
        return 13;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_myfunc'])
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    create_test_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>

      typedef int (*FUNCTYPE)(int, int, int, int, int, int, int, int, int, int, int, int, int);

      int main() {
        void *lib_handle;
        FUNCTYPE func_ptr;

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        func_ptr = (FUNCTYPE)dlsym(lib_handle, "myfunc");
        assert(func_ptr != NULL);
        assert(func_ptr(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 13);

        puts("success");

        return 0;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc'])
    self.do_runf('main.c', 'success')

  @needs_dlfcn
  def test_dlfcn_info(self):

    self.prep_dlfcn_lib()
    create_test_file('liblib.c', r'''
      #include <stdio.h>

      int myfunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m) {
        return 13;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_myfunc'])
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    create_test_file('main.c', '''
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>
      #include <dlfcn.h>

      typedef int (*FUNCTYPE)(int, int, int, int, int, int, int, int, int, int, int, int, int);

      int main() {
        void *lib_handle;
        FUNCTYPE func_ptr;

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        func_ptr = (FUNCTYPE)dlsym(lib_handle, "myfunc");
        assert(func_ptr != NULL);
        assert(func_ptr(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 13);

        /* Verify that we don't corrupt func_ptr when calling dladdr.  */
        Dl_info info;
        memset(&info, 0, sizeof(info));
        dladdr(func_ptr, &info);

        assert(func_ptr != NULL);
        assert(func_ptr(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 13);

        /* Verify something useful lives in info.  */
        assert(info.dli_fname != NULL);
        assert(info.dli_fbase == NULL);
        assert(info.dli_sname == NULL);
        assert(info.dli_saddr == NULL);

        puts("success");

        return 0;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc'])
    self.do_runf('main.c', 'success')

  @needs_dlfcn
  def test_dlfcn_stacks(self):
    self.prep_dlfcn_lib()
    create_test_file('liblib.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>

      int myfunc(const char *input) {
        char bigstack[1024] = { 0 };

        // make sure we didn't just trample the stack!
        assert(!strcmp(input, "foobar"));

        snprintf(bigstack, sizeof(bigstack), input);
        return strlen(bigstack);
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_myfunc'])
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    create_test_file('main.c', '''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>
      #include <string.h>

      typedef int (*FUNCTYPE)(const char *);

      int main() {
        void *lib_handle;
        FUNCTYPE func_ptr;
        char str[128];

        snprintf(str, sizeof(str), "foobar");

        // HACK: Use strcmp in the main executable so that it doesn't get optimized out and the dynamic library
        //       is able to use it.
        assert(!strcmp(str, "foobar"));

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        func_ptr = (FUNCTYPE)dlsym(lib_handle, "myfunc");
        assert(func_ptr != NULL);
        assert(func_ptr(str) == 6);

        puts("success");

        return 0;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc', '_strcmp'])
    self.do_runf('main.c', 'success')

  @needs_dlfcn
  def test_dlfcn_funcs(self):
    self.prep_dlfcn_lib()
    create_test_file('liblib.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>

      typedef void (*voidfunc)();
      typedef void (*intfunc)(int);

      void callvoid(voidfunc f) { f(); }
      void callint(voidfunc f, int x) { f(x); }

      void void_0() { printf("void 0\n"); }
      void void_1() { printf("void 1\n"); }
      voidfunc getvoid(int i) {
        switch(i) {
          case 0: return void_0;
          case 1: return void_1;
          default: return NULL;
        }
      }

      void int_0(int x) { printf("int 0 %d\n", x); }
      void int_1(int x) { printf("int 1 %d\n", x); }
      intfunc getint(int i) {
        switch(i) {
          case 0: return int_0;
          case 1: return int_1;
          default: return NULL;
        }
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_callvoid', '_callint', '_getvoid', '_getint'])
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    create_test_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>

      typedef void (*voidfunc)();
      typedef void (*intfunc)(int);

      typedef void (*voidcaller)(voidfunc);
      typedef void (*intcaller)(intfunc, int);

      typedef voidfunc (*voidgetter)(int);
      typedef intfunc (*intgetter)(int);

      void void_main() { printf("void_main.\n"); }
      void int_main(int x) { printf("int_main %d\n", x); }

      int main() {
        printf("go\n");
        void *lib_handle;
        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        voidcaller callvoid = (voidcaller)dlsym(lib_handle, "callvoid");
        assert(callvoid != NULL);
        callvoid(void_main);

        intcaller callint = (intcaller)dlsym(lib_handle, "callint");
        assert(callint != NULL);
        callint(int_main, 201);

        voidgetter getvoid = (voidgetter)dlsym(lib_handle, "getvoid");
        assert(getvoid != NULL);
        callvoid(getvoid(0));
        callvoid(getvoid(1));

        intgetter getint = (intgetter)dlsym(lib_handle, "getint");
        assert(getint != NULL);
        callint(getint(0), 54);
        callint(getint(1), 9000);

        assert(getint(1000) == NULL);

        puts("ok");
        return 0;
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc'])
    self.do_runf('main.c', '''go
void_main.
int_main 201
void 0
void 1
int 0 54
int 1 9000
ok
''')

  @needs_dlfcn
  def test_dlfcn_mallocs(self):
    # will be exhausted without functional malloc/free
    self.set_setting('INITIAL_MEMORY', 64 * 1024 * 1024)

    self.prep_dlfcn_lib()
    create_test_file('liblib.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>
      #include <stdlib.h>

      void *mallocproxy(int n) { return malloc(n); }
      void freeproxy(void *p) { free(p); }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_mallocproxy', '_freeproxy'])
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc', '_free'])
    self.do_runf(path_from_root('tests', 'dlmalloc_proxy.c'), '*294,153*')

  @needs_dlfcn
  def test_dlfcn_longjmp(self):
    self.prep_dlfcn_lib()
    create_test_file('liblib.c', r'''
      #include <setjmp.h>
      #include <stdio.h>

      void jumpy(jmp_buf buf) {
        static int i = 0;
        i++;
        if (i == 10) longjmp(buf, i);
        printf("pre %d\n", i);
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_jumpy'])
    self.build_dlfcn_lib('liblib.c')

    self.prep_dlfcn_main()
    src = r'''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>
      #include <setjmp.h>

      typedef void (*jumpfunc)(jmp_buf);

      int main() {
        printf("go!\n");

        void *lib_handle;
        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        jumpfunc jumpy = (jumpfunc)dlsym(lib_handle, "jumpy");
        assert(jumpy);

        jmp_buf buf;
        int jmpval = setjmp(buf);
        if (jmpval == 0) {
          while (1) jumpy(buf);
        } else {
          printf("out!\n");
        }

        return 0;
      }
      '''
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc', '_free'])
    self.do_run(src, '''go!
pre 1
pre 2
pre 3
pre 4
pre 5
pre 6
pre 7
pre 8
pre 9
out!
''', force_c=True)

  # TODO: make this work. need to forward tempRet0 across modules
  # TODO Enable @with_both_exception_handling (the test is not working now)
  @needs_dlfcn
  def zzztest_dlfcn_exceptions(self):
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)

    self.prep_dlfcn_lib()
    create_test_file('liblib.cpp', r'''
      extern "C" {
      int ok() {
        return 65;
      }
      int fail() {
        throw 123;
      }
      }
      ''')
    self.set_setting('EXPORTED_FUNCTIONS', ['_ok', '_fail'])
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    src = r'''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>

      typedef int (*intfunc)();

      int main() {
        printf("go!\n");

        void *lib_handle;
        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        intfunc okk = (intfunc)dlsym(lib_handle, "ok");
        intfunc faill = (intfunc)dlsym(lib_handle, "fail");
        assert(okk && faill);

        try {
          printf("ok: %d\n", okk());
        } catch(...) {
          printf("wha\n");
        }

        try {
          printf("fail: %d\n", faill());
        } catch(int x) {
          printf("int %d\n", x);
        }

        try {
          printf("fail: %d\n", faill());
        } catch(double x) {
          printf("caught %f\n", x);
        }

        return 0;
      }
      '''
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc', '_free'])
    self.do_run(src, '''go!
ok: 65
int 123
ok
''')

  @needs_dlfcn
  def test_dlfcn_handle_alloc(self):
    # verify that dlopen does not allocate already used handles
    dirname = self.get_dir()

    def indir(name):
      return os.path.join(dirname, name)

    create_test_file('a.cpp', r'''
      #include <stdio.h>

      static struct a {
        a() {
          puts("a: loaded");
        }
      } _;
    ''')

    create_test_file('b.cpp', r'''
      #include <stdio.h>

      static struct b {
        b() {
          puts("b: loaded");
        }
      } _;
    ''')

    self.prep_dlfcn_lib()
    self.build_dlfcn_lib('a.cpp')
    shutil.move(indir('liblib.so'), indir('liba.so'))
    self.build_dlfcn_lib('b.cpp')
    shutil.move(indir('liblib.so'), indir('libb.so'))

    self.set_setting('MAIN_MODULE')
    self.clear_setting('SIDE_MODULE')
    self.set_setting('EXPORT_ALL')
    self.emcc_args += ['--embed-file', '.@/']

    # XXX in wasm each lib load currently takes 5MB; default INITIAL_MEMORY=16MB is thus not enough
    self.set_setting('INITIAL_MEMORY', 32 * 1024 * 1024)

    src = r'''
      #include <dlfcn.h>
      #include <assert.h>
      #include <stddef.h>

      int main() {
        void *liba, *libb, *liba2;
        int err;

        liba = dlopen("liba.so", RTLD_NOW);
        assert(liba != NULL);
        libb = dlopen("libb.so", RTLD_NOW);
        assert(liba != NULL);

        err = dlclose(liba);
        assert(!err);

        liba2 = dlopen("liba.so", RTLD_NOW);
        assert(liba2 != libb);

        return 0;
      }
      '''
    self.do_run(src, 'a: loaded\nb: loaded\na: loaded\n')

  @needs_dlfcn
  @bleeding_edge_wasm_backend
  def test_dlfcn_feature_in_lib(self):
    self.emcc_args.append('-mnontrapping-fptoint')

    self.prep_dlfcn_lib()
    create_test_file('liblib.cpp', r'''
        extern "C" int magic(float x) {
          return __builtin_wasm_trunc_saturate_s_i32_f32(x);
        }
      ''')
    self.build_dlfcn_lib('liblib.cpp')

    self.prep_dlfcn_main()
    src = r'''
      #include <dlfcn.h>
      #include <stdio.h>
      #include <stdlib.h>

      typedef int (*fi)(float);

      int main() {
        void *lib_handle = dlopen("liblib.so", 0);
        if (!lib_handle) {
          puts(dlerror());
          abort();
        }
        fi x = (fi)dlsym(lib_handle, "magic");
        if (!x) {
          puts(dlerror());
          abort();
        }
        printf("float: %d.\n", x(42.99));
        return 0;
      }
      '''
    self.do_run(src, 'float: 42.\n')

  def dylink_test(self, main, side, expected=None, header=None, main_emcc_args=[], force_c=False, need_reverse=True, auto_load=True, **kwargs):
    if header:
      create_test_file('header.h', header)

    old_args = self.emcc_args[:]

    # side settings
    self.clear_setting('MAIN_MODULE')
    self.set_setting('SIDE_MODULE')
    side_suffix = 'wasm' if self.is_wasm() else 'js'
    out_file = 'liblib.' + side_suffix
    if isinstance(side, list):
      # side is just a library
      try_delete(out_file)
      self.run_process([EMCC] + side + self.get_emcc_args() + ['-o', out_file])
    else:
      filename = 'liblib.cpp' if not force_c else 'liblib.c'
      try_delete(out_file)
      with open(filename, 'w') as f:
        f.write(side)
      self.build(filename, js_outfile=(side_suffix == 'js'))
    shutil.move(out_file, 'liblib.so')

    # main settings
    self.set_setting('MAIN_MODULE')
    self.clear_setting('SIDE_MODULE')
    if auto_load:
      self.set_setting('RUNTIME_LINKED_LIBS', ['liblib.so'])
      self.emcc_args += main_emcc_args

    if isinstance(main, list):
      # main is just a library
      try_delete('src.js')
      self.run_process([EMCC] + main + self.emcc_args + self.serialize_settings() + ['-o', 'src.js'])
      self.do_run('src.js', expected, no_build=True, **kwargs)
    else:
      self.do_run(main, expected, force_c=force_c, **kwargs)

    self.emcc_args = old_args

    if need_reverse:
      # test the reverse as well
      print('flip')
      self.dylink_test(side, main, expected, header, main_emcc_args + ['--no-entry'], force_c, need_reverse=False, **kwargs)

  def do_basic_dylink_test(self, need_reverse=True):
    self.dylink_test(r'''
      #include <stdio.h>
      #include "header.h"

      int main() {
        printf("other says %d.\n", sidey());
        return 0;
      }
    ''', '''
      #include "header.h"

      int sidey() {
        return 11;
      }
    ''', 'other says 11.', 'extern "C" int sidey();', need_reverse=need_reverse)

  @needs_dlfcn
  def test_dylink_basics(self):
    self.do_basic_dylink_test()

  @needs_dlfcn
  def test_dylink_no_export(self):
    self.set_setting('NO_DECLARE_ASM_MODULE_EXPORTS')
    self.do_basic_dylink_test()

  @needs_dlfcn
  def test_dylink_memory_growth(self):
    if not self.is_wasm():
      self.skipTest('wasm only')
    self.set_setting('ALLOW_MEMORY_GROWTH', 1)
    self.do_basic_dylink_test()

  @needs_dlfcn
  def test_dylink_safe_heap(self):
    self.set_setting('SAFE_HEAP', 1)
    self.do_basic_dylink_test()

  @needs_dlfcn
  def test_dylink_function_pointer_equality(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include "header.h"

      int main() {
        void* puts_side = get_address();
        printf("main module address %p.\n", &puts);
        printf("side module address address %p.\n", puts_side);
        if (&puts == puts_side)
          printf("success\n");
        else
          printf("failure\n");
        return 0;
      }
    ''', '''
      #include <stdio.h>
      #include "header.h"

      void* get_address() {
        return (void*)&puts;
      }
    ''', 'success', header='extern "C" void* get_address();')

  @needs_dlfcn
  def test_dylink_floats(self):
    self.dylink_test(r'''
      #include <stdio.h>
      extern float sidey();
      int main() {
        printf("other says %.2f.\n", sidey()+1);
        return 0;
      }
    ''', '''
      float sidey() { return 11.5; }
    ''', 'other says 12.50')

  @needs_dlfcn
  def test_dylink_printfs(self):
    self.dylink_test(r'''
      #include <stdio.h>
      extern "C" void sidey();
      int main() {
        printf("hello from main\n");
        sidey();
        return 0;
      }
    ''', r'''
      #include <stdio.h>
      extern "C" void sidey() {
        printf("hello from side\n");
      }
    ''', 'hello from main\nhello from side\n')

  # Verify that a function pointer can be passed back and forth and invoked
  # on both sides.
  @needs_dlfcn
  def test_dylink_funcpointer(self):
    self.dylink_test(
      main=r'''
      #include <stdio.h>
      #include <assert.h>
      #include "header.h"
      intfunc sidey(intfunc f);
      void a(int arg) { printf("hello from funcptr: %d\n", arg); }
      int main() {
        intfunc b = sidey(a);
        assert(a == b);
        b(0);
        return 0;
      }
      ''',
      side='''
      #include "header.h"
      intfunc sidey(intfunc f) { f(1); return f; }
      ''',
      expected='hello from funcptr: 1\nhello from funcptr: 0\n',
      header='typedef void (*intfunc)(int );')

  @needs_dlfcn
  # test dynamic linking of a module with multiple function pointers, stored
  # statically
  def test_dylink_static_funcpointers(self):
    self.dylink_test(
      main=r'''
      #include <stdio.h>
      #include "header.h"
      void areturn0() { printf("hello 0\n"); }
      void areturn1() { printf("hello 1\n"); }
      void areturn2() { printf("hello 2\n"); }
      voidfunc func_ptrs[3] = { areturn0, areturn1, areturn2 };
      int main(int argc, char **argv) {
        sidey(func_ptrs[0]);
        sidey(func_ptrs[1]);
        sidey(func_ptrs[2]);
        return 0;
      }
      ''',
      side='''
      #include "header.h"
      void sidey(voidfunc f) { f(); }
      ''',
      expected='hello 0\nhello 1\nhello 2\n',
      header='typedef void (*voidfunc)(); void sidey(voidfunc f);')

  @no_wasm('uses function tables in an asm.js specific way')
  @needs_dlfcn
  def test_dylink_asmjs_funcpointers(self):
    self.dylink_test(
      main=r'''
      #include "header.h"
      #include <emscripten.h>
      void left1() { printf("left1\n"); }
      void left2() { printf("left2\n"); }
      voidfunc getleft1() { return left1; }
      voidfunc getleft2() { return left2; }
      int main(int argc, char **argv) {
        printf("main\n");
        EM_ASM({
          // make the function table sizes a non-power-of-two
          var newSize = alignFunctionTables();
          //out('old size of function tables: ' + newSize);
          while ((newSize & 3) !== 3) {
            Module['FUNCTION_TABLE_v'].push(0);
            newSize = alignFunctionTables();
          }
          //out('new size of function tables: ' + newSize);
          // when masked, the two function pointers 1 and 2 should not happen to fall back to the right place
          assert(((newSize+1) & 3) !== 1 || ((newSize+2) & 3) !== 2);
          loadDynamicLibrary('liblib.so');
        });
        volatilevoidfunc f;
        f = (volatilevoidfunc)left1;
        f();
        f = (volatilevoidfunc)left2;
        f();
        f = (volatilevoidfunc)getright1();
        f();
        f = (volatilevoidfunc)getright2();
        f();
        second();
        return 0;
      }
      ''',
      side=r'''
      #include "header.h"
      void right1() { printf("right1\n"); }
      void right2() { printf("right2\n"); }
      voidfunc getright1() { return right1; }
      voidfunc getright2() { return right2; }
      void second() {
        printf("second\n");
        volatilevoidfunc f;
        f = (volatilevoidfunc)getleft1();
        f();
        f = (volatilevoidfunc)getleft2();
        f();
        f = (volatilevoidfunc)right1;
        f();
        f = (volatilevoidfunc)right2;
        f();
      }
      ''',
      expected='main\nleft1\nleft2\nright1\nright2\nsecond\nleft1\nleft2\nright1\nright2\n',
      header='''
      #include <stdio.h>
      typedef void (*voidfunc)();
      typedef volatile voidfunc volatilevoidfunc;
      voidfunc getleft1();
      voidfunc getleft2();
      voidfunc getright1();
      voidfunc getright2();
      void second();
      ''', need_reverse=False, auto_load=False)

  @needs_dlfcn
  def test_dylink_funcpointers_wrapper(self):
    self.dylink_test(
      main=r'''\
      #include <stdio.h>
      #include "header.h"
      int main(int argc, char **argv) {
        charfunc f1 = emscripten_run_script;
        f1("out('one')");
        charfunc f2 = get();
        f2("out('two')");
        return 0;
      }
      ''',
      side='''\
      #include "header.h"
      charfunc get() {
        return emscripten_run_script;
      }
      ''',
      expected='one\ntwo\n',
      header='''\
      #include <emscripten.h>
      typedef void (*charfunc)(const char*);
      extern charfunc get();
      ''')

  @needs_dlfcn
  def test_dylink_static_funcpointer_float(self):
    self.dylink_test(
      main=r'''\
      #include <stdio.h>
      #include "header.h"
      int sidey(floatfunc f);
      float func1(float f) { printf("hello 1: %f\n", f); return 0; }
      floatfunc f1 = &func1;
      int main(int argc, char **argv) {
        printf("got: %d\n", sidey(f1));
        f1(12.34);
        return 0;
      }
      ''',
      side='''\
      #include "header.h"
      int sidey(floatfunc f) { f(56.78); return 1; }
      ''',
      expected='hello 1: 56.779999\ngot: 1\nhello 1: 12.340000\n',
      header='typedef float (*floatfunc)(float);')

  @needs_dlfcn
  def test_dylink_global_init(self):
    self.dylink_test(r'''
      #include <stdio.h>
      struct Class {
        Class() { printf("a new Class\n"); }
      };
      static Class c;
      int main() {
        return 0;
      }
    ''', r'''
      void nothing() {}
    ''', 'a new Class\n')

  @needs_dlfcn
  def test_dylink_global_inits(self):
    def test():
      self.dylink_test(header=r'''
        #include <stdio.h>
        struct Class {
          Class(const char *name) { printf("new %s\n", name); }
        };
      ''', main=r'''
        #include "header.h"
        static Class c("main");
        int main() {
          return 0;
        }
      ''', side=r'''
        #include "header.h"
        static Class c("side");
      ''', expected=['new main\nnew side\n', 'new side\nnew main\n'])
    test()

    # TODO: this in wasm
    if self.get_setting('ASSERTIONS') == 1 and not self.is_wasm():
      print('check warnings')
      self.set_setting('ASSERTIONS', 2)
      test()
      full = self.run_js('src.js')
      self.assertNotContained("trying to dynamically load symbol '__ZN5ClassC2EPKc' (from 'liblib.so') that already exists", full)

  @needs_dlfcn
  def test_dylink_i64(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include <stdint.h>
      extern int64_t sidey();
      int main() {
        printf("other says %llx.\n", sidey());
        return 0;
      }
    ''', '''
      #include <stdint.h>
      int64_t sidey() {
        volatile int64_t x = 11;
        x = x * x * x * x;
        x += x % 17;
        x += (x * (1 << 30));
        x -= 96;
        x = (x + 1000) / ((x % 5) + 1);
        volatile uint64_t y = x / 2;
        x = y / 3;
        y = y * y * y * y;
        y += y % 17;
        y += (y * (1 << 30));
        y -= 121;
        y = (y + 1000) / ((y % 5) + 1);
        x += y;
        return x;
      }
    ''', 'other says 175a1ddee82b8c31.')

  @all_engines
  @needs_dlfcn
  def test_dylink_i64_b(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include <stdint.h>
      extern int64_t sidey();
      int64_t testAdd(int64_t a) {
        return a + 1;
      }
      int64_t testAddB(int a) {
        return a + 1;
      }
      typedef int64_t (*testAddHandler)(int64_t);
      testAddHandler h = &testAdd;
      typedef int64_t (*testAddBHandler)(int);
      testAddBHandler hb = &testAddB;
      int main() {
        printf("other says %lld.\n", sidey());
        int64_t r = h(42);
        printf("my fp says: %lld.\n", r);
        int64_t rb = hb(42);
        printf("my second fp says: %lld.\n", r);
      }
    ''', '''
      #include <stdint.h>
      int64_t sidey() {
        volatile int64_t x = 0x12345678abcdef12LL;
        x += x % 17;
        x = 18 - x;
        return x;
      }
    ''', 'other says -1311768467750121224.\nmy fp says: 43.\nmy second fp says: 43.')

  @needs_dlfcn
  @also_with_wasm_bigint
  def test_dylink_i64_c(self):
    self.dylink_test(r'''
      #include <cstdio>
      #include <cinttypes>
      #include "header.h"

      typedef int32_t (*fp_type_32)(int32_t, int32_t, int32_t);
      typedef int64_t (*fp_type_64)(int32_t, int32_t, int32_t);

      int32_t internal_function_ret_32(int32_t i, int32_t j, int32_t k) {
          return 32;
      }
      int64_t internal_function_ret_64(int32_t i, int32_t j, int32_t k) {
          return 64;
      }

      int main() {
          fp_type_32 fp32_internal = &internal_function_ret_32;
          fp_type_32 fp32_external = &function_ret_32;
          fp_type_64 fp64_external = &function_ret_64;
          fp_type_64 fp64_internal = &internal_function_ret_64;
          int32_t ires32 = fp32_internal(0,0,0);
          printf("res32 - internal %d\n",ires32);
          int32_t eres32 = fp32_external(0,0,0);
          printf("res32 - external %d\n",eres32);

          int64_t ires64 = fp64_internal(0,0,0);
          printf("res64 - internal %" PRId64 "\n",ires64);
          int64_t eres64 = fp64_external(0,0,0);
          printf("res64 - external %" PRId64 "\n",eres64);
          return 0;
      }
    ''', '''
      #include "header.h"
      int32_t function_ret_32(int32_t i, int32_t j, int32_t k) {
          return 32;
      }
      int64_t function_ret_64(int32_t i, int32_t j, int32_t k) {
          return 64;
      }
    ''', '''res32 - internal 32
res32 - external 32
res64 - internal 64
res64 - external 64\n''', header='''
      #include <emscripten.h>
      #include <cstdint>
      EMSCRIPTEN_KEEPALIVE int32_t function_ret_32(int32_t i, int32_t j, int32_t k);
      EMSCRIPTEN_KEEPALIVE int64_t function_ret_64(int32_t i, int32_t j, int32_t k);
    ''')

  @needs_dlfcn
  @also_with_wasm_bigint
  def test_dylink_i64_invoke(self):
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)
    self.dylink_test(r'''\
    #include <stdio.h>
    #include <stdint.h>

    extern "C" int64_t sidey(int64_t arg);

    int main(int argc, char *argv[]) {
        int64_t temp = 42;
        printf("got %lld\n", sidey(temp));
        return 0;
    }''', r'''\
    #include <stdint.h>
    #include <stdio.h>
    #include <emscripten.h>

    extern "C" {

    EMSCRIPTEN_KEEPALIVE int64_t do_call(int64_t arg) {
        if (arg == 0) {
            throw;
        }
        return 2 * arg;
    }
    int64_t sidey(int64_t arg) {
        try {
            return do_call(arg);
        } catch(...) {
            return 0;
        }
    }
    }''', 'got 84', need_reverse=False)

  @needs_dlfcn
  def test_dylink_class(self):
    self.dylink_test(header=r'''
      #include <stdio.h>
      struct Class {
        Class(const char *name);
      };
    ''', main=r'''
      #include "header.h"
      int main() {
        Class c("main");
        return 0;
      }
    ''', side=r'''
      #include "header.h"
      Class::Class(const char *name) { printf("new %s\n", name); }
    ''', expected=['new main\n'])

  @needs_dlfcn
  def test_dylink_global_var(self):
    self.dylink_test(main=r'''
      #include <stdio.h>
      extern int x;
      int main() {
        printf("extern is %d.\n", x);
        return 0;
      }
    ''', side=r'''
      int x = 123;
    ''', expected=['extern is 123.\n'])

  @needs_dlfcn
  def test_dylink_global_var_modded(self):
    self.dylink_test(main=r'''
      #include <stdio.h>
      extern int x;
      int main() {
        printf("extern is %d.\n", x);
        return 0;
      }
    ''', side=r'''
      int x = 123;
      struct Initter {
        Initter() { x = 456; }
      };
      Initter initter;
    ''', expected=['extern is 456.\n'])

  @needs_dlfcn
  def test_dylink_stdlib(self):
    self.dylink_test(header=r'''
      #include <math.h>
      #include <stdlib.h>
      #include <string.h>
      char *side(const char *data);
      double pow_two(double x);
    ''', main=r'''
      #include <stdio.h>
      #include "header.h"
      int main() {
        char *temp = side("hello through side\n");
        char *ret = (char*)malloc(strlen(temp)+1);
        strcpy(ret, temp);
        temp[1] = 'x';
        puts(ret);
        printf("pow_two: %d.\n", int(pow_two(5.9)));
        return 0;
      }
    ''', side=r'''
      #include "header.h"
      char *side(const char *data) {
        char *ret = (char*)malloc(strlen(data)+1);
        strcpy(ret, data);
        return ret;
      }
      double pow_two(double x) {
        return pow(2, x);
      }
    ''', expected=['hello through side\n\npow_two: 59.'])

  @needs_dlfcn
  def test_dylink_jslib(self):
    create_test_file('lib.js', r'''
      mergeInto(LibraryManager.library, {
        test_lib_func: function(x) {
          return x + 17.2;
        }
      });
    ''')
    self.dylink_test(header=r'''
      extern "C" { extern double test_lib_func(int input); }
    ''', main=r'''
      #include <stdio.h>
      #include "header.h"
      extern double sidey();
      int main2() { return 11; }
      int main() {
        int input = sidey();
        double temp = test_lib_func(input);
        printf("other says %.2f\n", temp);
        printf("more: %.5f, %d\n", temp, input);
        return 0;
      }
    ''', side=r'''
      #include <stdio.h>
      #include "header.h"
      extern int main2();
      double sidey() {
        int temp = main2();
        printf("main2 sed: %d\n", temp);
        printf("main2 sed: %u, %c\n", temp, temp/2);
        return test_lib_func(temp);
      }
    ''', expected='other says 45.2', main_emcc_args=['--js-library', 'lib.js'])

  @needs_dlfcn
  def test_dylink_many_postsets(self):
    NUM = 1234
    self.dylink_test(header=r'''
      #include <stdio.h>
      typedef void (*voidfunc)();
      static void simple() {
        printf("simple.\n");
      }
      static volatile voidfunc funcs[''' + str(NUM) + '] = { ' + ','.join(['simple'] * NUM) + r''' };
      static void test() {
        volatile int i = ''' + str(NUM - 1) + r''';
        funcs[i]();
        i = 0;
        funcs[i]();
      }
      extern void more();
    ''', main=r'''
      #include "header.h"
      int main() {
        test();
        more();
        return 0;
      }
    ''', side=r'''
      #include "header.h"
      void more() {
        test();
      }
    ''', expected=['simple.\nsimple.\nsimple.\nsimple.\n'])

  @needs_dlfcn
  def test_dylink_postsets_chunking(self):
    self.dylink_test(header=r'''
      extern int global_var;
    ''', main=r'''
      #include <stdio.h>
      #include "header.h"

      // prepare 99 global variable with local initializer
      static int p = 1;
      #define P(x) __attribute__((used)) int *padding##x = &p;
      P(01) P(02) P(03) P(04) P(05) P(06) P(07) P(08) P(09) P(10)
      P(11) P(12) P(13) P(14) P(15) P(16) P(17) P(18) P(19) P(20)
      P(21) P(22) P(23) P(24) P(25) P(26) P(27) P(28) P(29) P(30)
      P(31) P(32) P(33) P(34) P(35) P(36) P(37) P(38) P(39) P(40)
      P(41) P(42) P(43) P(44) P(45) P(46) P(47) P(48) P(49) P(50)
      P(51) P(52) P(53) P(54) P(55) P(56) P(57) P(58) P(59) P(60)
      P(61) P(62) P(63) P(64) P(65) P(66) P(67) P(68) P(69) P(70)
      P(71) P(72) P(73) P(74) P(75) P(76) P(77) P(78) P(79) P(80)
      P(81) P(82) P(83) P(84) P(85) P(86) P(87) P(88) P(89) P(90)
      P(91) P(92) P(93) P(94) P(95) P(96) P(97) P(98) P(99)

      // prepare global variable with global initializer
      int *ptr = &global_var;

      int main(int argc, char *argv[]) {
        printf("%d\n", *ptr);
      }
    ''', side=r'''
      #include "header.h"

      int global_var = 12345;
    ''', expected=['12345\n'])

  @needs_dlfcn
  def test_dylink_syslibs(self): # one module uses libcxx, need to force its inclusion when it isn't the main
    # https://github.com/emscripten-core/emscripten/issues/10571
    return self.skipTest('Currently not working due to duplicate symbol errors in wasm-ld')

    def test(syslibs, expect_pass=True, need_reverse=True):
      print('syslibs', syslibs, self.get_setting('ASSERTIONS'))
      passed = True
      try:
        with env_modify({'EMCC_FORCE_STDLIBS': syslibs}):
          self.dylink_test(main=r'''
            void side();
            int main() {
              side();
              return 0;
            }
          ''', side=r'''
            #include <iostream>
            void side() { std::cout << "cout hello from side\n"; }
          ''', expected=['cout hello from side\n'], need_reverse=need_reverse, assert_returncode=NON_ZERO)
      except Exception as e:
        if expect_pass:
          raise
        print('(seeing expected fail)')
        passed = False
        assertion = 'build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment'
        if self.get_setting('ASSERTIONS'):
          self.assertContained(assertion, str(e))
        else:
          self.assertNotContained(assertion, str(e))
      assert passed == expect_pass, ['saw', passed, 'but expected', expect_pass]

    test('libc++')
    test('1')
    if not self.has_changed_setting('ASSERTIONS'):
      self.set_setting('ASSERTIONS', 0)
      test('', expect_pass=False, need_reverse=False)
      self.set_setting('ASSERTIONS', 1)
      test('', expect_pass=False, need_reverse=False)

  @needs_dlfcn
  @with_env_modify({'EMCC_FORCE_STDLIBS': 'libc++'})
  def test_dylink_iostream(self):
    self.dylink_test(header=r'''
      #include <iostream>
      #include <string>
      std::string side();
    ''', main=r'''
      #include "header.h"
      int main() {
        std::cout << "hello from main " << side() << std::endl;
        return 0;
      }
    ''', side=r'''
      #include "header.h"
      std::string side() { return "and hello from side"; }
    ''', expected=['hello from main and hello from side\n'])

  @needs_dlfcn
  def test_dylink_dynamic_cast(self): # issue 3465
    self.dylink_test(header=r'''
      class Base {
      public:
          virtual void printName();
      };

      class Derived : public Base {
      public:
          void printName();
      };
    ''', main=r'''
      #include "header.h"
      #include <iostream>

      using namespace std;

      int main() {
        cout << "starting main" << endl;

        Base *base = new Base();
        Base *derived = new Derived();
        base->printName();
        derived->printName();

        if (dynamic_cast<Derived*>(derived)) {
          cout << "OK" << endl;
        } else {
          cout << "KO" << endl;
        }

        return 0;
      }
    ''', side=r'''
      #include "header.h"
      #include <iostream>

      using namespace std;

      void Base::printName() {
          cout << "Base" << endl;
      }

      void Derived::printName() {
          cout << "Derived" << endl;
      }
    ''', expected=['starting main\nBase\nDerived\nOK'])

  @needs_dlfcn
  @with_both_exception_handling
  def test_dylink_raii_exceptions(self):
    self.dylink_test(main=r'''
      #include <stdio.h>
      extern int side();
      int main() {
        printf("from side: %d.\n", side());
      }
    ''', side=r'''
      #include <stdio.h>
      typedef int (*ifdi)(float, double, int);
      int func_with_special_sig(float a, double b, int c) {
        printf("special %f %f %d\n", a, b, c);
        return 1337;
      }
      struct DestructorCaller {
        ~DestructorCaller() { printf("destroy\n"); }
      };
      int side() {
        // d has a destructor that must be called on function
        // exit, which means an invoke will be used for the
        // indirect call here - and the signature of that call
        // is special and not present in the main module, so
        // it must be generated for the side module.
        DestructorCaller d;
        volatile ifdi p = func_with_special_sig;
        return p(2.18281, 3.14159, 42);
      }
    ''', expected=['special 2.182810 3.141590 42\ndestroy\nfrom side: 1337.\n'])

  @needs_dlfcn
  @no_wasm_backend('wasm backend resolves symbols greedily on startup')
  def test_dylink_hyper_dupe(self):
    self.set_setting('INITIAL_MEMORY', 64 * 1024 * 1024)
    if not self.has_changed_setting('ASSERTIONS'):
      self.set_setting('ASSERTIONS', 2)

    # test hyper-dynamic linking, and test duplicate warnings
    create_test_file('third.cpp', r'''
      #include <stdio.h>
      int sidef() { return 36; }
      int sideg = 49;
      int bsidef() { return 536; }
      extern void only_in_second_1(int x);
      extern int second_to_third;
      int third_to_second = 1337;
      void only_in_third_0() {
        // note we access our own globals directly, so
        // it doesn't matter that overriding failed
        printf("only_in_third_0: %d, %d, %d\n", sidef(), sideg, second_to_third);
        only_in_second_1(2112);
      }
      void only_in_third_1(int x) {
        printf("only_in_third_1: %d, %d, %d, %d\n", sidef(), sideg, second_to_third, x);
      }
    ''')
    if self.is_wasm():
      libname = 'third.wasm'
    else:
      libname = 'third.js'
    self.run_process([EMCC, 'third.cpp', '-o', libname, '-s', 'SIDE_MODULE', '-s', 'EXPORT_ALL'] + self.get_emcc_args())

    self.dylink_test(main=r'''
      #include <stdio.h>
      #include <emscripten.h>
      extern int sidef();
      extern int sideg;
      extern int bsidef();
      extern int bsideg;
      extern void only_in_second_0();
      extern void only_in_third_0();
      int main() {
        EM_ASM({
          loadDynamicLibrary('%s'); // hyper-dynamic! works at least for functions (and consts not used in same block)
        });
        printf("sidef: %%d, sideg: %%d.\n", sidef(), sideg);
        printf("bsidef: %%d.\n", bsidef());
        only_in_second_0();
        only_in_third_0();
      }
    ''' % libname,
                     side=r'''
      #include <stdio.h>
      int sidef() { return 10; } // third will try to override these, but fail!
      int sideg = 20;
      extern void only_in_third_1(int x);
      int second_to_third = 500;
      extern int third_to_second;
      void only_in_second_0() {
        printf("only_in_second_0: %d, %d, %d\n", sidef(), sideg, third_to_second);
        only_in_third_1(1221);
      }
      void only_in_second_1(int x) {
        printf("only_in_second_1: %d, %d, %d, %d\n", sidef(), sideg, third_to_second, x);
      }
    ''',
                     expected=['sidef: 10, sideg: 20.\nbsidef: 536.\nonly_in_second_0: 10, 20, 1337\nonly_in_third_1: 36, 49, 500, 1221\nonly_in_third_0: 36, 49, 500\nonly_in_second_1: 10, 20, 1337, 2112\n'],
                     need_reverse=not self.is_wasm()) # in wasm, we can't flip as the side would have an EM_ASM, which we don't support yet TODO

    if not self.has_changed_setting('ASSERTIONS'):
      print('check warnings')
      full = self.run_js('src.js')
      self.assertContained("warning: symbol '_sideg' from '%s' already exists" % libname, full)

  @needs_dlfcn
  @no_wasm_backend('possible https://github.com/emscripten-core/emscripten/issues/9038')
  def test_dylink_dso_needed(self):
    def do_run(src, expected_output):
      self.do_run(src + 'int main() { return test_main(); }', expected_output)
    self._test_dylink_dso_needed(do_run)

  @needs_dlfcn
  def test_dylink_dot_a(self):
    # .a linking must force all .o files inside it, when in a shared module
    create_test_file('third.cpp', 'extern "C" int sidef() { return 36; }')
    create_test_file('fourth.cpp', 'extern "C" int sideg() { return 17; }')

    self.run_process([EMCC, '-fPIC', '-c', 'third.cpp', '-o', 'third.o'] + self.get_emcc_args())
    self.run_process([EMCC, '-fPIC', '-c', 'fourth.cpp', '-o', 'fourth.o'] + self.get_emcc_args())
    self.run_process([EMAR, 'rc', 'libfourth.a', 'fourth.o'])

    self.dylink_test(main=r'''
      #include <stdio.h>
      #include <emscripten.h>
      extern "C" int sidef();
      extern "C" int sideg();
      int main() {
        printf("sidef: %d, sideg: %d.\n", sidef(), sideg());
      }
    ''',
                     # contents of libfourth.a must be included, even if they aren't referred to!
                     side=['libfourth.a', 'third.o'],
                     expected=['sidef: 36, sideg: 17.\n'])

  @needs_dlfcn
  def test_dylink_spaghetti(self):
    self.dylink_test(main=r'''
      #include <stdio.h>
      int main_x = 72;
      extern int side_x;
      int adjust = side_x + 10;
      int *ptr = &side_x;
      struct Class {
        Class() {
          printf("main init sees %d, %d, %d.\n", adjust, *ptr, main_x);
        }
      };
      Class cm;
      int main() {
        printf("main main sees %d, %d, %d.\n", adjust, *ptr, main_x);
        return 0;
      }
    ''', side=r'''
      #include <stdio.h>
      extern int main_x;
      int side_x = -534;
      int adjust2 = main_x + 10;
      int *ptr2 = &main_x;
      struct Class {
        Class() {
          printf("side init sees %d, %d, %d.\n", adjust2, *ptr2, side_x);
        }
      };
      Class cs;
    ''', expected=['side init sees 82, 72, -534.\nmain init sees -524, -534, 72.\nmain main sees -524, -534, 72.',
                   'main init sees -524, -534, 72.\nside init sees 82, 72, -534.\nmain main sees -524, -534, 72.'])

  @needs_make('mingw32-make')
  @needs_dlfcn
  def test_dylink_zlib(self):
    self.emcc_args += ['-I' + path_from_root('tests', 'third_party', 'zlib'), '-s', 'RELOCATABLE']
    zlib_archive = self.get_zlib_library()
    self.dylink_test(main=open(path_from_root('tests', 'third_party', 'zlib', 'example.c')).read(),
                     side=zlib_archive,
                     expected=open(path_from_root('tests', 'core', 'test_zlib.out')).read(),
                     force_c=True)

  # @needs_dlfcn
  # def test_dylink_bullet(self):
  #   self.emcc_args += ['-I' + path_from_root('tests', 'bullet', 'src')]
  #   side = self.get_bullet_library(self, True)
  #   self.dylink_test(main=open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp')).read(),
  #                    side=side,
  #                    expected=[open(path_from_root('tests', 'bullet', 'output.txt')).read(), # different roundings
  #                              open(path_from_root('tests', 'bullet', 'output2.txt')).read(),
  #                              open(path_from_root('tests', 'bullet', 'output3.txt')).read()])

  @needs_dlfcn
  def test_dylink_rtti(self):
    # Verify that objects created in one module and be dynamic_cast<> correctly
    # in the another module.
    # Each module will define its own copy of certain COMDAT symbols such as
    # each classs's typeinfo, but at runtime they should both use the same one.
    header = '''
    #include <cstddef>

    class Foo {
    public:
      virtual ~Foo() {}
    };

    class Bar : public Foo {
    public:
      virtual ~Bar() {}
    };

    bool is_bar(Foo* foo);
    '''

    main = '''
    #include <stdio.h>
    #include "header.h"

    int main() {
      Bar bar;
      if (!is_bar(&bar)) {
        puts("failure");
        return 1;
      }
      puts("success");
      return 0;
    }
    '''

    side = '''
    #include "header.h"

    bool is_bar(Foo* foo) {
      return dynamic_cast<Bar*>(foo) != nullptr;
    }
    '''

    self.dylink_test(main=main,
                     side=side,
                     header=header,
                     expected='success')

  def test_random(self):
    src = r'''#include <stdlib.h>
#include <stdio.h>

int main()
{
    srandom(0xdeadbeef);
    printf("%ld\n", random());
}
'''
    self.do_run(src, '956867869')

  def test_rand(self):
    src = r'''#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
int main()
{
    // we need RAND_MAX to be a bitmask (power of 2 minus 1). this assertions guarantees
    // if RAND_MAX changes the test failure will focus attention on that issue here.
    assert(RAND_MAX == 0x7fffffff);

    srand(0xdeadbeef);
    for(int i = 0; i < 10; ++i)
        printf("%d\n", rand());

    unsigned int seed = 0xdeadbeef;
    for(int i = 0; i < 10; ++i)
        printf("%d\n", rand_r(&seed));

    bool haveEvenAndOdd = true;
    for(int i = 1; i <= 30; ++i)
    {
        int mask = 1 << i;
        if (mask > RAND_MAX) break;
        bool haveEven = false;
        bool haveOdd = false;
        for(int j = 0; j < 1000 && (!haveEven || !haveOdd); ++j)
        {
            if ((rand() & mask) == 0)
                haveEven = true;
            else
                haveOdd = true;
        }
        haveEvenAndOdd = haveEvenAndOdd && haveEven && haveOdd;
    }
    if (haveEvenAndOdd)
        printf("Have even and odd!\n");

    return 0;
}
'''
    expected = '''490242850
2074599277
1480056542
1912638067
931112055
2110392489
2053422194
1614832492
216117595
174823244
760368382
602359081
1121118963
1291018924
1608306807
352705809
958258461
1182561381
114276303
1481323674
Have even and odd!
'''
    self.do_run(src, expected)

  def test_strtod(self):
    src = open(path_from_root('tests', 'core', 'test_strtod.c')).read()
    expected = open(path_from_root('tests', 'core', 'test_strtod.out')).read()
    self.do_run(src, expected)

  def test_strtold(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strtold.c')

  def test_strtok(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strtok.c')

  def test_parseInt(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_parseInt.c')

  def test_transtrcase(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_transtrcase.c')

  @no_wasm2js('very slow to compile')
  def test_printf(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'printf', 'test.c')

  def test_printf_2(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_printf_2.c')

  def test_printf_float(self):
    self.do_run_in_out_file_test('tests', 'printf', 'test_float.c')

  def test_printf_octal(self):
    self.do_run_in_out_file_test('tests', 'printf', 'test_octal.c')

  def test_vprintf(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_vprintf.c')

  def test_vsnprintf(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_vsnprintf.c')

  def test_printf_more(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_printf_more.c')

  def test_perrar(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_perrar.c')

  def test_atoX(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_atoX.c')

  def test_strstr(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strstr.c')

  def test_fnmatch(self):
    # Run one test without assertions, for additional coverage
    if self.run_name == 'asm2m':
      i = self.emcc_args.index('ASSERTIONS=1')
      assert i > 0 and self.emcc_args[i - 1] == '-s'
      self.emcc_args[i] = 'ASSERTIONS=0'
      print('flip assertions off')
    self.do_run_in_out_file_test('tests', 'core', 'test_fnmatch.cpp')

  def test_sscanf(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf.c')

  def test_sscanf_2(self):
    # doubles
    for ftype in ['float', 'double']:
      src = r'''
          #include <stdio.h>

          int main(){
              char strval1[] = "1.2345678901";
              char strval2[] = "1.23456789e5";
              char strval3[] = "1.23456789E5";
              char strval4[] = "1.2345678e-5";
              char strval5[] = "1.2345678E-5";
              double dblval = 1.2345678901;
              double tstval;

              sscanf(strval1, "%lf", &tstval);
              if(dblval != tstval) printf("FAIL: Values are not equal: %lf %lf\n", dblval, tstval);
              else printf("Pass: %lf %lf\n", tstval, dblval);

              sscanf(strval2, "%lf", &tstval);
              dblval = 123456.789;
              if(dblval != tstval) printf("FAIL: Values are not equal: %lf %lf\n", dblval, tstval);
              else printf("Pass: %lf %lf\n", tstval, dblval);

              sscanf(strval3, "%lf", &tstval);
              dblval = 123456.789;
              if(dblval != tstval) printf("FAIL: Values are not equal: %lf %lf\n", dblval, tstval);
              else printf("Pass: %lf %lf\n", tstval, dblval);

              sscanf(strval4, "%lf", &tstval);
              dblval = 0.000012345678;
              if(dblval != tstval) printf("FAIL: Values are not equal: %lf %lf\n", dblval, tstval);
              else printf("Pass: %lf %lf\n", tstval, dblval);

              sscanf(strval5, "%lf", &tstval);
              dblval = 0.000012345678;
              if(dblval != tstval) printf("FAIL: Values are not equal: %lf %lf\n", dblval, tstval);
              else printf("Pass: %lf %lf\n", tstval, dblval);

              return 0;
          }
        '''
      if ftype == 'float':
        self.do_run(src.replace('%lf', '%f').replace('double', 'float'), '''Pass: 1.234568 1.234568
Pass: 123456.789062 123456.789062
Pass: 123456.789062 123456.789062
Pass: 0.000012 0.000012
Pass: 0.000012 0.000012''')
      else:
        self.do_run(src, '''Pass: 1.234568 1.234568
Pass: 123456.789000 123456.789000
Pass: 123456.789000 123456.789000
Pass: 0.000012 0.000012
Pass: 0.000012 0.000012''')

  def test_sscanf_n(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_n.c')

  def test_sscanf_whitespace(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_whitespace.c')

  def test_sscanf_other_whitespace(self):
    # use i16s in printf
    self.set_setting('SAFE_HEAP', 0)
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_other_whitespace.c')

  def test_sscanf_3(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_3.c')

  def test_sscanf_4(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_4.c')

  def test_sscanf_5(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_5.c')

  def test_sscanf_6(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_6.c')

  def test_sscanf_skip(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_skip.c')

  def test_sscanf_caps(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_caps.c')

  def test_sscanf_hex(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_hex.cpp')

  def test_sscanf_float(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_sscanf_float.c')

  def test_langinfo(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_langinfo.c')

  def test_files(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # closure can generate variables called 'gc', which pick up js shell stuff
    if self.maybe_closure(): # Use closure here, to test we don't break FS stuff
      self.emcc_args = [x for x in self.emcc_args if x != '-g'] # ensure we test --closure 1 --memory-init-file 1 (-g would disable closure)
    elif '-O3' in self.emcc_args and not self.is_wasm():
      print('closure 2')
      self.emcc_args += ['--closure', '2', '-Wno-almost-asm'] # Use closure 2 here for some additional coverage
      return self.skipTest('TODO: currently skipped because CI runs out of memory running Closure in this test!')

    self.emcc_args += ['-s', 'FORCE_FILESYSTEM=1', '--pre-js', 'pre.js']

    print('base', self.emcc_args)

    create_test_file('pre.js', '''
/** @suppress{checkTypes}*/
Module = {
  'noFSInit': true,
  'preRun': function() {
    FS.createLazyFile('/', 'test.file', 'test.file', true, false);
    // Test FS_* exporting
    Module['FS_createDataFile']('/', 'somefile.binary', [100, 200, 50, 25, 10, 77, 123], true, false, false);  // 200 becomes -56, since signed chars are used in memory
    var test_files_input = 'hi there!';
    var test_files_input_index = 0;
    FS.init(function() {
      return test_files_input.charCodeAt(test_files_input_index++) || null;
    });
  }
};
''')

    create_test_file('test.file', 'some data')

    src = open(path_from_root('tests', 'files.cpp')).read()

    mem_file = 'src.js.mem'
    try_delete(mem_file)

    def clean(out, err):
      return '\n'.join([line for line in (out + err).split('\n') if 'binaryen' not in line and 'wasm' not in line and 'so not running' not in line])

    self.do_run(src, ('size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\n5 bytes to dev/null: 5\nok.\ntexte\n', 'size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\ntexte\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\n5 bytes to dev/null: 5\nok.\n'),
                output_nicerizer=clean)

    if self.uses_memory_init_file():
      self.assertExists(mem_file)

  def test_files_m(self):
    # Test for Module.stdin etc.
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)

    create_test_file('pre.js', '''
    Module = {
      data: [10, 20, 40, 30],
      stdin: function() { return Module.data.pop() || null },
      stdout: function(x) { out('got: ' + x) }
    };
''')
    self.emcc_args += ['--pre-js', 'pre.js']

    src = r'''
      #include <stdio.h>
      #include <unistd.h>

      int main () {
        char c;
        fprintf(stderr, "isatty? %d,%d,%d\n", isatty(fileno(stdin)), isatty(fileno(stdout)), isatty(fileno(stderr)));
        while ((c = fgetc(stdin)) != EOF) {
          putc(c+5, stdout);
        }
        return 0;
      }
      '''

    def clean(out, err):
      return '\n'.join(l for l in (out + err).splitlines() if 'warning' not in l and 'binaryen' not in l)

    self.do_run(src, ('got: 35\ngot: 45\ngot: 25\ngot: 15\nisatty? 0,0,1\n', 'got: 35\ngot: 45\ngot: 25\ngot: 15\nisatty? 0,0,1', 'isatty? 0,0,1\ngot: 35\ngot: 45\ngot: 25\ngot: 15'), output_nicerizer=clean)

  def test_mount(self):
    self.set_setting('FORCE_FILESYSTEM', 1)
    self.do_runf(path_from_root('tests', 'fs', 'test_mount.c'), 'success')

  def test_getdents64(self):
    self.do_runf(path_from_root('tests', 'fs', 'test_getdents64.cpp'), '..')

  def test_getdents64_special_cases(self):
    self.banned_js_engines = [V8_ENGINE] # https://bugs.chromium.org/p/v8/issues/detail?id=6881
    src = path_from_root('tests', 'fs', 'test_getdents64_special_cases.cpp')
    out = path_from_root('tests', 'fs', 'test_getdents64_special_cases.out')
    self.do_run_from_file(src, out, assert_identical=True)

  def test_getcwd_with_non_ascii_name(self):
    self.banned_js_engines = [V8_ENGINE] # https://bugs.chromium.org/p/v8/issues/detail?id=6881
    src = path_from_root('tests', 'fs', 'test_getcwd_with_non_ascii_name.cpp')
    out = path_from_root('tests', 'fs', 'test_getcwd_with_non_ascii_name.out')
    self.do_run_from_file(src, out, assert_identical=True)

  def test_fwrite_0(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_fwrite_0.c')

  def test_fgetc_ungetc(self):
    print('TODO: update this test once the musl ungetc-on-EOF-stream bug is fixed upstream and reaches us')
    self.set_setting('SYSCALL_DEBUG', 1)
    self.clear()
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      print(fs)
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_runf(path_from_root('tests', 'stdio', 'test_fgetc_ungetc.c'), 'success', js_engines=[NODE_JS])

  def test_fgetc_unsigned(self):
    src = r'''
      #include <stdio.h>
      int main() {
        FILE *file = fopen("file_with_byte_234.txt", "rb");
        int c = fgetc(file);
        printf("*%d\n", c);
      }
    '''
    create_test_file('file_with_byte_234.txt', b'\xea', binary=True)
    self.emcc_args += ['--embed-file', 'file_with_byte_234.txt']
    self.do_run(src, '*234\n')

  def test_fgets_eol(self):
    src = r'''
      #include <stdio.h>
      char buf[32];
      int main()
      {
        const char *r = "SUCCESS";
        FILE *f = fopen("eol.txt", "r");
        while (fgets(buf, 32, f) != NULL) {
          if (buf[0] == '\0') {
            r = "FAIL";
            break;
          }
        }
        printf("%s\n", r);
        fclose(f);
        return 0;
      }
    '''
    open('eol.txt', 'wb').write(b'\n')
    self.emcc_args += ['--embed-file', 'eol.txt']
    self.do_run(src, 'SUCCESS\n')

  def test_fscanf(self):
    create_test_file('three_numbers.txt', '-1 0.1 -.1')
    src = r'''
      #include <stdio.h>
      #include <assert.h>
      #include <float.h>
      int main()
      {
          float x = FLT_MAX, y = FLT_MAX, z = FLT_MAX;

          FILE* fp = fopen("three_numbers.txt", "r");
          if (fp) {
              int match = fscanf(fp, " %f %f %f ", &x, &y, &z);
              printf("match = %d\n", match);
              printf("x = %0.1f, y = %0.1f, z = %0.1f\n", x, y, z);
          } else {
              printf("failed to open three_numbers.txt\n");
          }
          return 0;
      }
    '''
    self.emcc_args += ['--embed-file', 'three_numbers.txt']
    self.do_run(src, 'match = 3\nx = -1.0, y = 0.1, z = -0.1\n')

  def test_fscanf_2(self):
    create_test_file('a.txt', '''1/2/3 4/5/6 7/8/9
''')
    self.emcc_args += ['--embed-file', 'a.txt']
    self.do_run(r'''#include <cstdio>
#include <iostream>

using namespace std;

int
main( int argv, char ** argc ) {
    cout << "fscanf test" << endl;

    FILE * file;
    file = fopen("a.txt", "rb");
    int vertexIndex[4];
    int normalIndex[4];
    int uvIndex[4];

    int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex    [1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3]);

    cout << matches << endl;

    return 0;
}
''', 'fscanf test\n9\n')

  def test_fileno(self):
    create_test_file('empty.txt', '')
    src = r'''
      #include <stdio.h>
      #include <unistd.h>
      int main()
      {
          FILE* fp = fopen("empty.txt", "r");
          if (fp) {
              printf("%d\n", fileno(fp));
          } else {
              printf("failed to open empty.txt\n");
          }
          return 0;
      }
    '''
    self.emcc_args += ['--embed-file', 'empty.txt']
    self.do_run(src, '3\n')

  def test_readdir(self):
    self.do_run_in_out_file_test('tests', 'dirent', 'test_readdir.c')

  def test_readdir_empty(self):
    self.do_run_in_out_file_test('tests', 'dirent', 'test_readdir_empty.c')

  def test_stat(self):
    self.do_runf(path_from_root('tests', 'stat', 'test_stat.c'), 'success')
    self.verify_in_strict_mode('test_stat.js')

  def test_stat_chmod(self):
    self.do_runf(path_from_root('tests', 'stat', 'test_chmod.c'), 'success')

  def test_stat_mknod(self):
    self.do_runf(path_from_root('tests', 'stat', 'test_mknod.c'), 'success')

  def test_fcntl(self):
    self.add_pre_run("FS.createDataFile('/', 'test', 'abcdef', true, true, false);")
    self.do_run_in_out_file_test('tests', 'fcntl', 'test_fcntl.c')

  def test_fcntl_open(self):
    self.do_run_in_out_file_test('tests', 'fcntl', 'test_fcntl_open.c')

  @also_with_wasm_bigint
  def test_fcntl_misc(self):
    self.add_pre_run("FS.createDataFile('/', 'test', 'abcdef', true, true, false);")
    self.do_run_in_out_file_test('tests', 'fcntl', 'test_fcntl_misc.c')

  def test_poll(self):
    self.add_pre_run('''
      var dummy_device = FS.makedev(64, 0);
      FS.registerDevice(dummy_device, {});

      FS.createDataFile('/', 'file', 'abcdef', true, true, false);
      FS.mkdev('/device', dummy_device);
    ''')
    self.do_run_in_out_file_test('tests', 'core', 'test_poll.c')

  def test_statvfs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_statvfs.c')

  def test_libgen(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_libgen.c')

  def test_utime(self):
    self.do_runf(path_from_root('tests', 'utime', 'test_utime.c'), 'success')

  @no_minimal_runtime('MINIMAL_RUNTIME does not have getValue() and setValue() (TODO add it to a JS library function to get it in)')
  def test_utf(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # only node handles utf well
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_malloc'])
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['getValue', 'setValue', 'UTF8ToString', 'stringToUTF8'])
    self.do_run_in_out_file_test('tests', 'core', 'test_utf.c')

  def test_utf32(self):
    if self.get_setting('MINIMAL_RUNTIME'):
      self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', ['$UTF32ToString', '$stringToUTF32', '$lengthBytesUTF32'])
    else:
      self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['UTF32ToString', 'stringToUTF32', 'lengthBytesUTF32'])
    self.do_runf(path_from_root('tests', 'utf32.cpp'), 'OK.')
    self.do_runf(path_from_root('tests', 'utf32.cpp'), 'OK.', args=['-fshort-wchar'])

  def test_utf8(self):
    if self.get_setting('MINIMAL_RUNTIME'):
      self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', ['$AsciiToString', '$stringToAscii', '$writeAsciiToMemory'])
    else:
      self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS',
                       ['UTF8ToString', 'stringToUTF8', 'AsciiToString', 'stringToAscii'])
    self.do_runf(path_from_root('tests', 'utf8.cpp'), 'OK.')

  @also_with_wasm_bigint
  def test_utf8_textdecoder(self):
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['UTF8ToString', 'stringToUTF8'])
    self.emcc_args += ['--embed-file', path_from_root('tests/utf8_corpus.txt') + '@/utf8_corpus.txt']
    self.do_runf(path_from_root('tests', 'benchmark_utf8.cpp'), 'OK.')

  # Test that invalid character in UTF8 does not cause decoding to crash.
  def test_utf8_invalid(self):
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['UTF8ToString', 'stringToUTF8'])
    for decoder_mode in [[], ['-s', 'TEXTDECODER=1']]:
      self.emcc_args += decoder_mode
      print(str(decoder_mode))
      self.do_runf(path_from_root('tests', 'utf8_invalid.cpp'), 'OK.')

  # Test that invalid character in UTF8 does not cause decoding to crash.
  def test_minimal_runtime_utf8_invalid(self):
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['UTF8ToString', 'stringToUTF8'])
    for decoder_mode in [[], ['-s', 'TEXTDECODER=1']]:
      self.emcc_args += ['-s', 'MINIMAL_RUNTIME=1'] + decoder_mode
      print(str(decoder_mode))
      self.do_runf(path_from_root('tests', 'utf8_invalid.cpp'), 'OK.')

  def test_utf16_textdecoder(self):
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['UTF16ToString', 'stringToUTF16', 'lengthBytesUTF16'])
    self.emcc_args += ['--embed-file', path_from_root('tests/utf16_corpus.txt') + '@/utf16_corpus.txt']
    self.do_runf(path_from_root('tests', 'benchmark_utf16.cpp'), 'OK.')

  def test_wprintf(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_wprintf.cpp')

  def test_write_stdout_fileno(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_write_stdout_fileno.c')
    self.do_run_in_out_file_test('tests', 'core', 'test_write_stdout_fileno.c', args=['-s', 'FILESYSTEM=0'])

  def test_direct_string_constant_usage(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_direct_string_constant_usage.cpp')

  def test_std_cout_new(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_std_cout_new.cpp')

  def test_std_function_incomplete_return(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_std_function_incomplete_return.cpp')

  def test_istream(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)

    for linkable in [0]: # , 1]:
      print(linkable)
      # regression check for issue #273
      self.set_setting('LINKABLE', linkable)
      self.do_run_in_out_file_test('tests', 'core', 'test_istream.cpp')

  def test_fs_base(self):
    self.uses_es6 = True
    # TODO(sbc): It seems that INCLUDE_FULL_LIBRARY will generally generate
    # undefined symbols at link time so perhaps have it imply this setting?
    self.set_setting('WARN_ON_UNDEFINED_SYMBOLS', 0)
    self.set_setting('INCLUDE_FULL_LIBRARY', 1)
    self.add_pre_run(open(path_from_root('tests', 'filesystem', 'src.js')).read())
    src = 'int main() {return 0;}\n'
    expected = open(path_from_root('tests', 'filesystem', 'output.txt')).read()
    self.do_run(src, expected)

  @also_with_noderawfs
  @is_slow_test
  def test_fs_nodefs_rw(self):
    self.emcc_args += ['-lnodefs.js']
    self.set_setting('SYSCALL_DEBUG', 1)
    self.do_runf(path_from_root('tests', 'fs', 'test_nodefs_rw.c'), 'success')
    if '-g' not in self.emcc_args:
      print('closure')
      self.emcc_args += ['--closure', '1']
      self.do_runf(path_from_root('tests', 'fs', 'test_nodefs_rw.c'), 'success')

  @also_with_noderawfs
  def test_fs_nodefs_cloexec(self):
    self.emcc_args += ['-lnodefs.js']
    self.do_runf(path_from_root('tests', 'fs', 'test_nodefs_cloexec.c'), 'success')

  def test_fs_nodefs_home(self):
    self.set_setting('FORCE_FILESYSTEM', 1)
    self.emcc_args += ['-lnodefs.js']
    self.do_runf(path_from_root('tests', 'fs', 'test_nodefs_home.c'), 'success', js_engines=[NODE_JS])

  def test_fs_nodefs_nofollow(self):
    self.emcc_args += ['-lnodefs.js']
    self.do_runf(path_from_root('tests', 'fs', 'test_nodefs_nofollow.c'), 'success', js_engines=[NODE_JS])

  def test_fs_trackingdelegate(self):
    src = path_from_root('tests', 'fs', 'test_trackingdelegate.c')
    out = path_from_root('tests', 'fs', 'test_trackingdelegate.out')
    self.do_run_from_file(src, out)

  @also_with_noderawfs
  def test_fs_writeFile(self):
    self.emcc_args += ['-s', 'DISABLE_EXCEPTION_CATCHING=1'] # see issue 2334
    src = path_from_root('tests', 'fs', 'test_writeFile.cpp')
    out = path_from_root('tests', 'fs', 'test_writeFile.out')
    self.do_run_from_file(src, out)

  def test_fs_write(self):
    src = path_from_root('tests', 'fs', 'test_write.cpp')
    out = path_from_root('tests', 'fs', 'test_write.out')
    self.do_run_from_file(src, out)

  @also_with_noderawfs
  def test_fs_emptyPath(self):
    src = path_from_root('tests', 'fs', 'test_emptyPath.c')
    out = path_from_root('tests', 'fs', 'test_emptyPath.out')
    self.do_run_from_file(src, out)

  @also_with_noderawfs
  def test_fs_append(self):
    self.do_runf(path_from_root('tests', 'fs', 'test_append.c'), 'success')

  def test_fs_mmap(self):
    self.uses_es6 = True
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      src = path_from_root('tests', 'fs', 'test_mmap.c')
      out = path_from_root('tests', 'fs', 'test_mmap.out')
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_run_from_file(src, out)

  @also_with_noderawfs
  def test_fs_errorstack(self):
    # Enables strict mode, which may catch some strict-mode-only errors
    # so that users can safely work with strict JavaScript if enabled.
    create_test_file('pre.js', '"use strict";')
    self.emcc_args += ['--pre-js', 'pre.js']

    self.set_setting('FORCE_FILESYSTEM', 1)
    self.set_setting('ASSERTIONS', 1)
    self.do_run(r'''
      #include <emscripten.h>
      #include <iostream>
      int main(void) {
        std::cout << "hello world\n"; // should work with strict mode
        EM_ASM(
          try {
            FS.readFile('/dummy.txt');
          } catch (err) {
            err.stack = err.stack; // should be writable
            throw err;
          }
        );
        return 0;
      }
    ''', 'at Object.readFile', assert_returncode=NON_ZERO) # engines has different error stack format

  @also_with_noderawfs
  def test_fs_llseek(self):
    self.set_setting('FORCE_FILESYSTEM', 1)
    self.do_runf(path_from_root('tests', 'fs', 'test_llseek.c'), 'success')

  def test_fs_64bit(self):
    self.do_runf(path_from_root('tests', 'fs', 'test_64bit.c'), 'success')

  def test_sigalrm(self):
    self.do_runf(path_from_root('tests', 'sigalrm.cpp'), '')

  @no_windows('https://github.com/emscripten-core/emscripten/issues/8882')
  def test_unistd_access(self):
    self.uses_es6 = True
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_run_in_out_file_test('tests', 'unistd', 'access.c', js_engines=[NODE_JS])
    # Node.js fs.chmod is nearly no-op on Windows
    if not WINDOWS:
      self.emcc_args = orig_compiler_opts
      self.emcc_args += ['-s', 'NODERAWFS=1']
      self.do_run_in_out_file_test('tests', 'unistd', 'access.c', js_engines=[NODE_JS])

  def test_unistd_curdir(self):
    self.uses_es6 = True
    self.do_run_in_out_file_test('tests', 'unistd', 'curdir.c')

  @also_with_noderawfs
  def test_unistd_close(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'close.c')

  def test_unistd_confstr(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'confstr.c')

  def test_unistd_ttyname(self):
    self.do_runf(path_from_root('tests', 'unistd', 'ttyname.c'), 'success')

  @also_with_noderawfs
  def test_unistd_pipe(self):
    self.do_runf(path_from_root('tests', 'unistd', 'pipe.c'), 'success')

  @also_with_noderawfs
  def test_unistd_dup(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'dup.c')

  def test_unistd_pathconf(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'pathconf.c')

  def test_unistd_truncate(self):
    self.uses_es6 = True
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_run_in_out_file_test('tests', 'unistd', 'truncate.c', js_engines=[NODE_JS])

  @no_windows("Windows throws EPERM rather than EACCES or EINVAL")
  @unittest.skipIf(WINDOWS or os.geteuid() == 0, "Root access invalidates this test by being able to write on readonly files")
  def test_unistd_truncate_noderawfs(self):
    # FIXME
    self.skipTest('fails on some node versions and OSes, e.g. 10.13.0 on linux')

    self.emcc_args += ['-s', 'NODERAWFS=1']
    self.do_run_in_out_file_test('tests', 'unistd', 'truncate.c', js_engines=[NODE_JS])

  def test_unistd_swab(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'swab.c')

  def test_unistd_isatty(self):
    self.do_runf(path_from_root('tests', 'unistd', 'isatty.c'), 'success')

  @also_with_standalone_wasm()
  def test_unistd_sysconf(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'sysconf.c')

  @no_asan('ASan alters memory layout')
  def test_unistd_sysconf_phys_pages(self):
    filename = path_from_root('tests', 'unistd', 'sysconf_phys_pages.c')
    if self.get_setting('ALLOW_MEMORY_GROWTH'):
      expected = (2 * 1024 * 1024 * 1024) // 16384
    else:
      expected = 16 * 1024 * 1024 // 16384
    self.do_runf(filename, str(expected) + ', errno: 0')

  def test_unistd_login(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'login.c')

  @no_windows('https://github.com/emscripten-core/emscripten/issues/8882')
  def test_unistd_unlink(self):
    self.clear()
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      # symlinks on node.js on non-linux behave differently (e.g. on Windows they require administrative privileges)
      # so skip testing those bits on that combination.
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
        if WINDOWS:
          self.emcc_args += ['-DNO_SYMLINK=1']
        if MACOS:
          continue
      self.do_runf(path_from_root('tests', 'unistd', 'unlink.c'), 'success', js_engines=[NODE_JS])
    # Several differences/bugs on non-linux including https://github.com/nodejs/node/issues/18014
    if not WINDOWS and not MACOS:
      self.emcc_args = orig_compiler_opts + ['-DNODERAWFS']
      # 0 if root user
      if os.geteuid() == 0:
        self.emcc_args += ['-DSKIP_ACCESS_TESTS']
      self.emcc_args += ['-s', 'NODERAWFS=1']
      self.do_runf(path_from_root('tests', 'unistd', 'unlink.c'), 'success', js_engines=[NODE_JS])

  def test_unistd_links(self):
    self.clear()
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      if WINDOWS and fs == 'NODEFS':
        print('Skipping NODEFS part of this test for test_unistd_links on Windows, since it would require administrative privileges.', file=sys.stderr)
        # Also, other detected discrepancies if you do end up running this test on NODEFS:
        # test expects /, but Windows gives \ as path slashes.
        # Calling readlink() on a non-link gives error 22 EINVAL on Unix, but simply error 0 OK on Windows.
        continue
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_run_in_out_file_test('tests', 'unistd', 'links.c', js_engines=[NODE_JS])

  @no_windows('Skipping NODEFS test, since it would require administrative privileges.')
  def test_unistd_symlink_on_nodefs(self):
    # Also, other detected discrepancies if you do end up running this test on NODEFS:
    # test expects /, but Windows gives \ as path slashes.
    # Calling readlink() on a non-link gives error 22 EINVAL on Unix, but simply error 0 OK on Windows.
    self.emcc_args += ['-lnodefs.js']
    self.do_run_in_out_file_test('tests', 'unistd', 'symlink_on_nodefs.c', js_engines=[NODE_JS])

  def test_unistd_sleep(self):
    self.do_run_in_out_file_test('tests', 'unistd', 'sleep.c')

  @also_with_wasm_bigint
  def test_unistd_io(self):
    self.set_setting('INCLUDE_FULL_LIBRARY', 1) # uses constants from ERRNO_CODES
    self.set_setting('ERROR_ON_UNDEFINED_SYMBOLS', 0) # avoid errors when linking in full library
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      self.clear()
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_run_in_out_file_test('tests', 'unistd', 'io.c')

  @no_windows('https://github.com/emscripten-core/emscripten/issues/8882')
  def test_unistd_misc(self):
    orig_compiler_opts = self.emcc_args[:]
    for fs in ['MEMFS', 'NODEFS']:
      self.emcc_args = orig_compiler_opts + ['-D' + fs]
      if fs == 'NODEFS':
        self.emcc_args += ['-lnodefs.js']
      self.do_run_in_out_file_test('tests', 'unistd', 'misc.c', js_engines=[NODE_JS])

  # i64s in the API, which we'd need to legalize for JS, so in standalone mode
  # all we can test is wasm VMs
  @also_with_standalone_wasm(wasm2c=True)
  def test_posixtime(self):
    self.banned_js_engines = [V8_ENGINE] # v8 lacks monotonic time
    self.do_run_in_out_file_test('tests', 'core', 'test_posixtime.c')

  def test_uname(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_uname.c')

  def test_unary_literal(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_unary_literal.cpp')

  def test_env(self):
    src = path_from_root('tests', 'env', 'src.c')
    expected = open(path_from_root('tests', 'env', 'output.txt')).read()
    self.do_runf(src, [
      expected.replace('{{{ THIS_PROGRAM }}}', self.in_dir('src.js')).replace('\\', '/'), # node, can find itself properly
      expected.replace('{{{ THIS_PROGRAM }}}', './this.program') # spidermonkey, v8
    ])

  def test_environ(self):
    src = open(path_from_root('tests', 'env', 'src-mini.c')).read()
    expected = open(path_from_root('tests', 'env', 'output-mini.txt')).read()
    self.do_run(src, [
      expected.replace('{{{ THIS_PROGRAM }}}', self.in_dir('src.js')).replace('\\', '/'), # node, can find itself properly
      expected.replace('{{{ THIS_PROGRAM }}}', './this.program') # spidermonkey, v8
    ])

  def test_systypes(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_systypes.c')

  def test_stddef(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_stddef.cpp')
    self.do_run_in_out_file_test('tests', 'core', 'test_stddef.cpp', force_c=True)

  def test_getloadavg(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_getloadavg.c')

  def test_nl_types(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_nl_types.c')

  def test_799(self):
    src = path_from_root('tests', '799.cpp')
    self.do_runf(src, '''Set PORT family: 0, port: 3979
Get PORT family: 0
PORT: 3979
''')

  def test_ctype(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_ctype.c')

  def test_strcasecmp(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_strcasecmp.c')

  def test_atomic(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_atomic.c')

  def test_atomic_cxx(self):
    # the wasm backend has lock-free atomics, but not asm.js or asm2wasm
    self.emcc_args += ['-DIS_64BIT_LOCK_FREE=1']
    self.do_run_in_out_file_test('tests', 'core', 'test_atomic_cxx.cpp')
    # TODO: test with USE_PTHREADS in wasm backend as well

  def test_phiundef(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_phiundef.c')

  def test_netinet_in(self):
    self.do_run_in_out_file_test('tests', 'netinet', 'in.cpp')

  @needs_dlfcn
  def test_main_module_static_align(self):
    if self.get_setting('ALLOW_MEMORY_GROWTH'):
      self.skipTest('no shared modules with memory growth')
    self.set_setting('MAIN_MODULE', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_main_module_static_align.cpp')

  # libc++ tests

  def test_iostream_and_determinism(self):
    create_test_file('src.cpp', '''
      #include <iostream>

      int main()
      {
        std::cout << "hello world" << std::endl << 77 << "." << std::endl;
        return 0;
      }
    ''')

    num = 5
    for i in range(num):
      print('(iteration %d)' % i)

      # add some timing nondeterminism here, not that we need it, but whatever
      time.sleep(random.random() / (10 * num))
      self.do_runf('src.cpp', 'hello world\n77.\n')

      # Verify that this build is identical to the previous one
      if os.path.exists('src.js.previous'):
        self.assertBinaryEqual('src.js', 'src.js.previous')
      shutil.copy2('src.js', 'src.js.previous')

      # Same but for the wasm file.
      if self.get_setting('WASM') and not self.get_setting('WASM2JS'):
        if os.path.exists('src.wasm.previous'):
          self.assertBinaryEqual('src.wasm', 'src.wasm.previous')
        shutil.copy2('src.wasm', 'src.wasm.previous')

  def test_stdvec(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_stdvec.cpp')

  def test_random_device(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_random_device.cpp')

  def test_reinterpreted_ptrs(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_reinterpreted_ptrs.cpp')

  def test_js_libraries(self):
    create_test_file('main.cpp', '''
      #include <stdio.h>
      extern "C" {
        extern void printey();
        extern int calcey(int x, int y);
      }
      int main() {
        printey();
        printf("*%d*\\n", calcey(10, 22));
        return 0;
      }
    ''')
    create_test_file('mylib1.js', '''
      mergeInto(LibraryManager.library, {
        printey: function() {
          out('hello from lib!');
        }
      });
    ''')
    create_test_file('mylib2.js', '''
      mergeInto(LibraryManager.library, {
        calcey: function(x, y) {
          return x + y;
        }
      });
    ''')

    self.emcc_args += ['--js-library', 'mylib1.js', '--js-library', 'mylib2.js']
    self.do_runf('main.cpp', 'hello from lib!\n*32*\n')

  def test_unicode_js_library(self):
    create_test_file('main.cpp', '''
      #include <stdio.h>
      extern "C" {
        extern void printey();
      }
      int main() {
        printey();
        return 0;
      }
    ''')
    self.emcc_args += ['--js-library', path_from_root('tests', 'unicode_library.js')]
    self.do_runf('main.cpp', u'Unicode snowman \u2603 says hello!')

  def test_funcptr_import_type(self):
    self.emcc_args += ['--js-library', path_from_root('tests', 'core', 'test_funcptr_import_type.js')]
    self.do_run_in_out_file_test('tests', 'core', 'test_funcptr_import_type.cpp')

  @no_asan('ASan does not work with EXPORT_ALL')
  def test_constglobalunion(self):
    self.emcc_args += ['-s', 'EXPORT_ALL=1']

    self.do_run(r'''
#include <stdio.h>

struct one_const {
  long a;
};

struct two_consts {
  long a;
  long b;
};

union some_consts {
  struct one_const one;
  struct two_consts two;
};

union some_consts my_consts = {{
  1
}};

struct one_const addr_of_my_consts = {
  (long)(&my_consts)
};

int main(void) {
  printf("%li\n", (long)!!addr_of_my_consts.a);
  return 0;
}
    ''', '1')

  ### 'Medium' tests

  def test_fannkuch(self):
    results = [(1, 0), (2, 1), (3, 2), (4, 4), (5, 7), (6, 10), (7, 16), (8, 22)]
    self.build(path_from_root('tests', 'fannkuch.cpp'))
    for i, j in results:
      print(i, j)
      self.do_run('fannkuch.js', 'Pfannkuchen(%d) = %d.' % (i, j), args=[str(i)], no_build=True)

  def test_raytrace(self):
    # TODO: Should we remove this test?
    self.skipTest('Relies on double value rounding, extremely sensitive')

    src = open(path_from_root('tests', 'raytrace.cpp')).read().replace('double', 'float')
    output = open(path_from_root('tests', 'raytrace.ppm')).read()
    self.do_run(src, output, args=['3', '16'])

  def test_fasta(self):
    results = [(1, '''GG*ctt**tgagc*'''),
               (20, '''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tacgtgtagcctagtgtttgtgttgcgttatagtctatttgtggacacagtatggtcaaa**tgacgtcttttgatctgacggcgttaacaaagatactctg*'''),
               (50, '''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA*TCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACAT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tactDtDagcctatttSVHtHttKtgtHMaSattgWaHKHttttagacatWatgtRgaaa**NtactMcSMtYtcMgRtacttctWBacgaa**agatactctgggcaacacacatacttctctcatgttgtttcttcggacctttcataacct**ttcctggcacatggttagctgcacatcacaggattgtaagggtctagtggttcagtgagc**ggaatatcattcgtcggtggtgttaatctatctcggtgtagcttataaatgcatccgtaa**gaatattatgtttatttgtcggtacgttcatggtagtggtgtcgccgatttagacgtaaa**ggcatgtatg*''')]

    old = self.emcc_args
    orig_src = open(path_from_root('tests', 'fasta.cpp')).read()

    def test(extra_args):
      self.emcc_args = old + extra_args
      for t in ['float', 'double']:
        print(t)
        src = orig_src.replace('double', t)
        with open('fasta.cpp', 'w') as f:
          f.write(src)
        self.build('fasta.cpp')
        for arg, output in results:
          self.do_run('fasta.js', output, args=[str(arg)], output_nicerizer=lambda x, err: x.replace('\n', '*'), no_build=True)
        shutil.copyfile('fasta.js', '%s.js' % t)

    test([])

  @bleeding_edge_wasm_backend
  def test_fasta_nontrapping(self):
    self.emcc_args += ['-mnontrapping-fptoint']
    self.test_fasta()

  def test_whets(self):
    self.do_runf(path_from_root('tests', 'whets.cpp'), 'Single Precision C Whetstone Benchmark')

  def test_dlmalloc_inline(self):
    self.banned_js_engines = [NODE_JS] # slower, and fail on 64-bit
    # needed with typed arrays
    self.set_setting('INITIAL_MEMORY', 128 * 1024 * 1024)

    src = open(path_from_root('system', 'lib', 'dlmalloc.c')).read() + '\n\n\n' + open(path_from_root('tests', 'dlmalloc_test.c')).read()
    self.do_run(src, '*1,0*', args=['200', '1'], force_c=True)
    self.do_run('src.js', '*400,0*', args=['400', '400'], force_c=True, no_build=True)

  def test_dlmalloc(self):
    self.banned_js_engines = [NODE_JS] # slower, and fail on 64-bit
    # needed with typed arrays
    self.set_setting('INITIAL_MEMORY', 128 * 1024 * 1024)

    # Linked version
    src = open(path_from_root('tests', 'dlmalloc_test.c')).read()
    self.do_run(src, '*1,0*', args=['200', '1'])
    self.do_run('src.js', '*400,0*', args=['400', '400'], no_build=True)

    # TODO: do this in other passes too, passing their opts into emcc
    if self.emcc_args == []:
      # emcc should build in dlmalloc automatically, and do all the sign correction etc. for it

      try_delete('src.js')
      self.run_process([EMCC, path_from_root('tests', 'dlmalloc_test.c'), '-s', 'INITIAL_MEMORY=128MB', '-o', 'src.js'], stdout=PIPE, stderr=self.stderr_redirect)

      self.do_run(None, '*1,0*', ['200', '1'], no_build=True)
      self.do_run(None, '*400,0*', ['400', '400'], no_build=True)

      # The same for new and all its variants
      src = open(path_from_root('tests', 'new.cpp')).read()
      for new, delete in [
        ('malloc(100)', 'free'),
        ('new char[100]', 'delete[]'),
        ('new Structy', 'delete'),
        ('new int', 'delete'),
        ('new Structy[10]', 'delete[]'),
      ]:
        self.do_run(src.replace('{{{ NEW }}}', new).replace('{{{ DELETE }}}', delete), '*1,0*')

  @no_asan('asan also changes malloc, and that ends up linking in new twice')
  def test_dlmalloc_partial(self):
    # present part of the symbols of dlmalloc, not all
    src = open(path_from_root('tests', 'new.cpp')).read().replace('{{{ NEW }}}', 'new int').replace('{{{ DELETE }}}', 'delete') + '''
#include <new>

void *
operator new(size_t size) throw(std::bad_alloc)
{
printf("new %d!\\n", size);
return malloc(size);
}
'''
    self.do_run(src, 'new 4!\n*1,0*')

  @no_asan('asan also changes malloc, and that ends up linking in new twice')
  def test_dlmalloc_partial_2(self):
    if 'SAFE_HEAP' in str(self.emcc_args):
      self.skipTest('we do unsafe stuff here')
    # present part of the symbols of dlmalloc, not all. malloc is harder to link than new which is weak.
    self.do_run_in_out_file_test('tests', 'core', 'test_dlmalloc_partial_2.c', assert_returncode=NON_ZERO)

  def test_libcxx(self):
    self.do_runf(path_from_root('tests', 'hashtest.cpp'),
                 'june -> 30\nPrevious (in alphabetical order) is july\nNext (in alphabetical order) is march')

    self.do_run('''
      #include <set>
      #include <stdio.h>
      int main() {
        std::set<int> *fetchOriginatorNums = new std::set<int>();
        fetchOriginatorNums->insert(171);
        printf("hello world\\n");
        return 0;
      }
      ''', 'hello world')

  def test_typeid(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_typeid.cpp')

  def test_static_variable(self):
    # needs atexit
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_static_variable.cpp')

  def test_fakestat(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_fakestat.c')

  def test_mmap(self):
    self.set_setting('INITIAL_MEMORY', 128 * 1024 * 1024)
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_mmap.c')

  def test_mmap_file(self):
    for extra_args in [[]]:
      self.emcc_args += ['--embed-file', 'data.dat'] + extra_args
      x = 'data from the file........'
      s = ''
      while len(s) < 9000:
        if len(s) + len(x) < 9000:
          s += x
          continue
        s += '.'
      assert len(s) == 9000
      create_test_file('data.dat', s)
      src = open(path_from_root('tests', 'mmap_file.c')).read()
      self.do_run(src, '*\n' + s[0:20] + '\n' + s[4096:4096 + 20] + '\n*\n')

  def test_cubescript(self):
    # uses register keyword
    self.emcc_args.append('-std=c++03')
    if self.run_name == 'asm3':
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

    self.emcc_args = [x for x in self.emcc_args if x != '-g'] # remove -g, so we have one test without it by default

    self.emcc_args += ['-I', path_from_root('tests', 'cubescript')]

    def test():
      src = path_from_root('tests', 'cubescript', 'command.cpp')
      self.do_runf(src, '*\nTemp is 33\n9\n5\nhello, everyone\n*')

    test()

    print('asyncify') # extra coverage
    self.emcc_args += ['-s', 'ASYNCIFY=1']
    test()

  @needs_dlfcn
  def test_relocatable_void_function(self):
    self.set_setting('RELOCATABLE', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_relocatable_void_function.c')

  @wasm_simd
  def test_wasm_builtin_simd(self):
    # Improves test readability
    self.emcc_args.append('-Wno-c++11-narrowing')
    self.do_runf(path_from_root('tests', 'test_wasm_builtin_simd.cpp'), 'Success!')
    self.emcc_args.append('-munimplemented-simd128')
    self.build(path_from_root('tests', 'test_wasm_builtin_simd.cpp'))

  @wasm_simd
  def test_wasm_intrinsics_simd(self):
    def run():
      self.do_runf(path_from_root('tests', 'test_wasm_intrinsics_simd.c'), 'Success!')
    # Improves test readability
    self.emcc_args.append('-Wno-c++11-narrowing')
    self.emcc_args.extend(['-Wpedantic', '-Werror', '-Wall', '-xc++'])
    run()
    self.emcc_args.append('-funsigned-char')
    run()
    self.emcc_args.extend(['-munimplemented-simd128', '-xc', '-std=c99'])
    self.build(path_from_root('tests', 'test_wasm_intrinsics_simd.c'))

  # Tests invoking the NEON SIMD API via arm_neon.h header
  @wasm_simd
  def test_neon_wasm_simd(self):
    self.emcc_args.append('-Wno-c++11-narrowing')
    self.emcc_args.append('-mfpu=neon')
    self.emcc_args.append('-msimd128')
    self.do_runf(path_from_root('tests', 'neon', 'test_neon_wasm_simd.cpp'), 'Success!')

  # Tests invoking the SIMD API via x86 SSE1 xmmintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_sse1(self):
    src = path_from_root('tests', 'sse', 'test_sse1.cpp')
    self.run_process([shared.CLANG_CXX, src, '-msse', '-o', 'test_sse1', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_sse1', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-msse']
    self.maybe_closure()

    self.do_runf(src, native_result)

  # Tests invoking the SIMD API via x86 SSE2 emmintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_sse2(self):
    src = path_from_root('tests', 'sse', 'test_sse2.cpp')
    self.run_process([shared.CLANG_CXX, src, '-msse2', '-Wno-argument-outside-range', '-o', 'test_sse2', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_sse2', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-msse2', '-Wno-argument-outside-range']
    self.maybe_closure()
    self.do_runf(src, native_result)

  # Tests invoking the SIMD API via x86 SSE3 pmmintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_sse3(self):
    src = path_from_root('tests', 'sse', 'test_sse3.cpp')
    self.run_process([shared.CLANG_CXX, src, '-msse3', '-Wno-argument-outside-range', '-o', 'test_sse3', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_sse3', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-msse3', '-Wno-argument-outside-range']
    self.maybe_closure()
    self.do_runf(src, native_result)

  # Tests invoking the SIMD API via x86 SSSE3 tmmintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_ssse3(self):
    src = path_from_root('tests', 'sse', 'test_ssse3.cpp')
    self.run_process([shared.CLANG_CXX, src, '-mssse3', '-Wno-argument-outside-range', '-o', 'test_ssse3', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_ssse3', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-mssse3', '-Wno-argument-outside-range']
    self.maybe_closure()
    self.do_runf(src, native_result)

  # Tests invoking the SIMD API via x86 SSE4.1 smmintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_sse4_1(self):
    src = path_from_root('tests', 'sse', 'test_sse4_1.cpp')
    self.run_process([shared.CLANG_CXX, src, '-msse4.1', '-Wno-argument-outside-range', '-o', 'test_sse4_1', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_sse4_1', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-msse4.1', '-Wno-argument-outside-range']
    self.maybe_closure()
    self.do_runf(src, native_result)

  # Tests invoking the SIMD API via x86 SSE4.2 nmmintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_sse4_2(self):
    src = path_from_root('tests', 'sse', 'test_sse4_2.cpp')
    self.run_process([shared.CLANG_CXX, src, '-msse4.2', '-Wno-argument-outside-range', '-o', 'test_sse4_2', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_sse4_2', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-msse4.2', '-Wno-argument-outside-range']
    self.maybe_closure()
    self.do_runf(src, native_result)

  # Tests invoking the SIMD API via x86 AVX avxintrin.h header (_mm_x() functions)
  @wasm_simd
  @requires_native_clang
  def test_avx(self):
    src = path_from_root('tests', 'sse', 'test_avx.cpp')
    self.run_process([shared.CLANG_CXX, src, '-mavx', '-Wno-argument-outside-range', '-o', 'test_avx', '-D_CRT_SECURE_NO_WARNINGS=1'] + clang_native.get_clang_native_args(), stdout=PIPE)
    native_result = self.run_process('./test_avx', stdout=PIPE).stdout

    orig_args = self.emcc_args
    self.emcc_args = orig_args + ['-I' + path_from_root('tests', 'sse'), '-mavx', '-Wno-argument-outside-range']
    self.maybe_closure()
    self.do_runf(src, native_result)

  @no_asan('call stack exceeded on some versions of node')
  def test_gcc_unmangler(self):
    self.emcc_args += ['-I' + path_from_root('third_party')]

    self.do_runf(path_from_root('third_party', 'gcc_demangler.c'), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'])

  @needs_make('make')
  def test_lua(self):
    self.emcc_args.remove('-Werror')

    self.do_run('',
                'hello lua world!\n17\n1\n2\n3\n4\n7',
                args=['-e', '''print("hello lua world!");print(17);for x = 1,4 do print(x) end;print(10-3)'''],
                libraries=self.get_library(os.path.join('third_party', 'lua'), [os.path.join('src', 'lua.o'), os.path.join('src', 'liblua.a')], make=['make', 'generic'], configure=None),
                includes=[path_from_root('tests', 'lua')],
                output_nicerizer=lambda string, err: (string + err).replace('\n\n', '\n').replace('\n\n', '\n'))

  @no_asan('issues with freetype itself')
  @needs_make('configure script')
  @is_slow_test
  def test_freetype(self):
    self.add_pre_run("FS.createDataFile('/', 'font.ttf', %s, true, false, false);" % str(
      list(bytearray(open(path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), 'rb').read()))
    ))

    # Not needed for js, but useful for debugging
    shutil.copyfile(path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), 'font.ttf')

    # Main
    self.do_run_from_file(path_from_root('tests', 'freetype', 'main.c'),
                          path_from_root('tests', 'freetype', 'ref.txt'),
                          args=['font.ttf', 'test!', '150', '120', '25'],
                          libraries=self.get_freetype_library(),
                          includes=[path_from_root('tests', 'third_party', 'freetype', 'include')])

    # github issue 324
    print('[issue 324]')
    self.do_run_from_file(path_from_root('tests', 'freetype', 'main_2.c'),
                          path_from_root('tests', 'freetype', 'ref_2.txt'),
                          args=['font.ttf', 'w', '32', '32', '25'],
                          libraries=self.get_freetype_library(),
                          includes=[path_from_root('tests', 'third_party', 'freetype', 'include')])

    print('[issue 324 case 2]')
    self.do_run_from_file(path_from_root('tests', 'freetype', 'main_3.c'),
                          path_from_root('tests', 'freetype', 'ref_3.txt'),
                          args=['font.ttf', 'W', '32', '32', '0'],
                          libraries=self.get_freetype_library(),
                          includes=[path_from_root('tests', 'third_party', 'freetype', 'include')])

    print('[issue 324 case 3]')
    self.do_run('main_3.js',
                open(path_from_root('tests', 'freetype', 'ref_4.txt')).read(),
                args=['font.ttf', 'ea', '40', '32', '0'],
                no_build=True)

  @no_asan('local count too large for VMs')
  def test_sqlite(self):
    self.set_setting('DISABLE_EXCEPTION_CATCHING', 1)
    self.set_setting('EXPORTED_FUNCTIONS', ['_main', '_sqlite3_open', '_sqlite3_close', '_sqlite3_exec', '_sqlite3_free'])
    if '-g' in self.emcc_args:
      print("disabling inlining") # without registerize (which -g disables), we generate huge amounts of code
      self.set_setting('INLINING_LIMIT', 50)

    # newer clang has a warning for implicit conversions that lose information,
    # which happens in sqlite (see #9138)
    self.emcc_args += ['-Wno-implicit-int-float-conversion']
    # newer clang warns about "suspicious concatenation of string literals in an
    # array initialization; did you mean to separate the elements with a comma?"
    self.emcc_args += ['-Wno-string-concatenation']
    # ignore unknown flags, which lets the above flags be used on github CI
    # before the LLVM change rolls in (the same LLVM change that adds the
    # warning also starts to warn on it)
    self.emcc_args += ['-Wno-unknown-warning-option']

    self.emcc_args += ['-I' + path_from_root('tests', 'third_party', 'sqlite')]

    src = '''
       #define SQLITE_DISABLE_LFS
       #define LONGDOUBLE_TYPE double
       #define SQLITE_INT64_TYPE long long int
       #define SQLITE_THREADSAFE 0
    '''
    src += open(path_from_root('tests', 'third_party', 'sqlite', 'sqlite3.c')).read()
    src += open(path_from_root('tests', 'sqlite', 'benchmark.c')).read()
    self.do_run(src,
                open(path_from_root('tests', 'sqlite', 'benchmark.txt')).read(),
                includes=[path_from_root('tests', 'sqlite')],
                force_c=True)

  @needs_make('mingw32-make')
  @is_slow_test
  @parameterized({
    'cmake': (True,),
    'configure': (False,)
  })
  def test_zlib(self, use_cmake):
    if WINDOWS and not use_cmake:
      self.skipTest("Windows cannot run configure sh scripts")

    self.maybe_closure()

    if self.run_name == 'asm2g':
      self.emcc_args += ['-g4'] # more source maps coverage

    if use_cmake:
      make_args = []
      configure = [path_from_root('emcmake'), 'cmake', '.']
    else:
      make_args = ['libz.a']
      configure = ['sh', './configure']

    self.do_run_from_file(
        path_from_root('tests', 'third_party', 'zlib', 'example.c'),
        path_from_root('tests', 'core', 'test_zlib.out'),
        libraries=self.get_library(os.path.join('third_party', 'zlib'), 'libz.a', make_args=make_args, configure=configure),
        includes=[path_from_root('tests', 'third_party', 'zlib'), 'building', 'zlib'])

  @needs_make('make')
  @is_slow_test
  @parameterized({
    'cmake': (True,),
    'autoconf': (False,)
  })
  # Called thus so it runs late in the alphabetical cycle... it is long
  def test_bullet(self, use_cmake):
    if WINDOWS and not use_cmake:
      self.skipTest("Windows cannot run configure sh scripts")

    self.emcc_args += ['-Wno-c++11-narrowing', '-Wno-deprecated-register', '-Wno-writable-strings']
    asserts = self.get_setting('ASSERTIONS')

    # extra testing for ASSERTIONS == 2
    self.set_setting('ASSERTIONS', 2 if use_cmake else asserts)

    self.do_runf(path_from_root('tests', 'third_party', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'),
                 [open(path_from_root('tests', 'bullet', 'output.txt')).read(), # different roundings
                  open(path_from_root('tests', 'bullet', 'output2.txt')).read(),
                  open(path_from_root('tests', 'bullet', 'output3.txt')).read(),
                  open(path_from_root('tests', 'bullet', 'output4.txt')).read()],
                 libraries=self.get_bullet_library(use_cmake),
                 includes=[path_from_root('tests', 'third_party', 'bullet', 'src')])

  @no_asan('issues with freetype itself')
  @needs_make('depends on freetype')
  @is_slow_test
  def test_poppler(self):
    pdf_data = open(path_from_root('tests', 'poppler', 'paper.pdf'), 'rb').read()
    create_test_file('paper.pdf.js', str(list(bytearray(pdf_data))))

    create_test_file('pre.js', '''
    Module.preRun = function() {
      FS.createDataFile('/', 'paper.pdf', eval(read_('paper.pdf.js')), true, false, false);
    };
    Module.postRun = function() {
      var FileData = MEMFS.getFileDataAsRegularArray(FS.root.contents['filename-1.ppm']);
      out("Data: " + JSON.stringify(FileData.map(function(x) { return unSign(x, 8) })));
    };
    ''')
    self.emcc_args += ['--pre-js', 'pre.js', '-s', 'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[$unSign]']

    ppm_data = str(list(bytearray(open(path_from_root('tests', 'poppler', 'ref.ppm'), 'rb').read())))
    self.do_run('', ppm_data.replace(' ', ''),
                libraries=self.get_poppler_library(),
                args=['-scale-to', '512', 'paper.pdf', 'filename'])

  @needs_make('make')
  @is_slow_test
  def test_openjpeg(self):
    if '-fsanitize=address' in self.emcc_args:
      self.set_setting('INITIAL_MEMORY', 128 * 1024 * 1024)

    def line_splitter(data):
      out = ''
      counter = 0

      for ch in data:
        out += ch
        if ch == ' ' and counter > 60:
          out += '\n'
          counter = 0
        else:
          counter += 1

      return out

    # remove -g, so we have one test without it by default
    self.emcc_args = [x for x in self.emcc_args if x != '-g']

    original_j2k = path_from_root('tests', 'openjpeg', 'syntensity_lobby_s.j2k')
    image_bytes = list(bytearray(open(original_j2k, 'rb').read()))
    create_test_file('pre.js', """
      Module.preRun = function() { FS.createDataFile('/', 'image.j2k', %s, true, false, false); };
      Module.postRun = function() {
        out('Data: ' + JSON.stringify(MEMFS.getFileDataAsRegularArray(FS.analyzePath('image.raw').object)));
      };
      """ % line_splitter(str(image_bytes)))

    shutil.copy(path_from_root('tests', 'third_party', 'openjpeg', 'opj_config.h'), self.get_dir())

    lib = self.get_library(os.path.join('third_party', 'openjpeg'),
                           [os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/index.c.o'.split('/')),
                            os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/convert.c.o'.split('/')),
                            os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/__/common/color.c.o'.split('/')),
                            os.path.join('bin', 'libopenjpeg.a')],
                           configure=['cmake', '.'],
                           # configure_args=['--enable-tiff=no', '--enable-jp3d=no', '--enable-png=no'],
                           make_args=[]) # no -j 2, since parallel builds can fail

    # We use doubles in JS, so we get slightly different values than native code. So we
    # check our output by comparing the average pixel difference
    def image_compare(output, err):
      # Get the image generated by JS, from the JSON.stringify'd array
      m = re.search(r'\[[\d, -]*\]', output)
      self.assertIsNotNone(m, 'Failed to find proper image output in: ' + output)
      # Evaluate the output as a python array
      js_data = eval(m.group(0))

      js_data = [x if x >= 0 else 256 + x for x in js_data] # Our output may be signed, so unsign it

      # Get the correct output
      true_data = bytearray(open(path_from_root('tests', 'openjpeg', 'syntensity_lobby_s.raw'), 'rb').read())

      # Compare them
      assert(len(js_data) == len(true_data))
      num = len(js_data)
      diff_total = js_total = true_total = 0
      for i in range(num):
        js_total += js_data[i]
        true_total += true_data[i]
        diff_total += abs(js_data[i] - true_data[i])
      js_mean = js_total / float(num)
      true_mean = true_total / float(num)
      diff_mean = diff_total / float(num)

      image_mean = 83.265
      # print '[image stats:', js_mean, image_mean, true_mean, diff_mean, num, ']'
      assert abs(js_mean - image_mean) < 0.01, [js_mean, image_mean]
      assert abs(true_mean - image_mean) < 0.01, [true_mean, image_mean]
      assert diff_mean < 0.01, diff_mean

      return output

    self.emcc_args += ['--minify', '0'] # to compare the versions
    self.emcc_args += ['--pre-js', 'pre.js']

    def do_test():
      self.do_runf(path_from_root('tests', 'third_party', 'openjpeg', 'codec', 'j2k_to_image.c'),
                   'Successfully generated', # The real test for valid output is in image_compare
                   args='-i image.j2k -o image.raw'.split(),
                   libraries=lib,
                   includes=[path_from_root('tests', 'third_party', 'openjpeg', 'libopenjpeg'),
                             path_from_root('tests', 'third_party', 'openjpeg', 'codec'),
                             path_from_root('tests', 'third_party', 'openjpeg', 'common'),
                             os.path.join(self.get_build_dir(), 'openjpeg')],
                   output_nicerizer=image_compare)

    do_test()

    # extra testing
    if self.get_setting('ALLOW_MEMORY_GROWTH') == 1:
      print('no memory growth', file=sys.stderr)
      self.set_setting('ALLOW_MEMORY_GROWTH', 0)
      do_test()

  @no_wasm_backend("uses bitcode compiled with asmjs, and we don't have unified triples")
  def test_python(self):
    self.set_setting('EMULATE_FUNCTION_POINTER_CASTS', 1)
    # The python build contains several undefined symbols
    self.set_setting('ERROR_ON_UNDEFINED_SYMBOLS', 0)

    bitcode = path_from_root('tests', 'third_party', 'python', 'python.bc')
    pyscript = dedent('''\
      print '***'
      print "hello python world!"
      print [x*2 for x in range(4)]
      t=2
      print 10-3-t
      print (lambda x: x*2)(11)
      print '%f' % 5.47
      print {1: 2}.keys()
      print '***'
      ''')
    pyoutput = '***\nhello python world!\n[0, 2, 4, 6]\n5\n22\n5.470000\n[1]\n***'

    for lto in [0, 1]:
      print('lto:', lto)
      if lto:
        self.emcc_args += ['-flto']
      self.do_run_object(bitcode, pyoutput, args=['-S', '-c', pyscript])

  @no_asan('call stack exceeded on some versions of node')
  @is_slow_test
  def test_fuzz(self):
    self.emcc_args += ['-I' + path_from_root('tests', 'fuzz', 'include'), '-w']

    skip_lto_tests = [
      # LLVM LTO bug
      '19.c', '18.cpp',
      # puts exists before LTO, but is not used; LTO cleans it out, but then creates uses to it (printf=>puts) XXX https://llvm.org/bugs/show_bug.cgi?id=23814
      '23.cpp'
    ]

    def run_all(x):
      print(x)
      for name in sorted(glob.glob(path_from_root('tests', 'fuzz', '*.c')) + glob.glob(path_from_root('tests', 'fuzz', '*.cpp'))):
        # if os.path.basename(name) != '4.c':
        #   continue
        if 'newfail' in name:
          continue
        if os.path.basename(name).startswith('temp_fuzzcode'):
          continue
        # pnacl legalization issue, see https://code.google.com/p/nativeclient/issues/detail?id=4027
        if x == 'lto' and self.run_name in ['default'] and os.path.basename(name) in ['8.c']:
          continue
        if x == 'lto' and self.run_name == 'default' and os.path.basename(name) in skip_lto_tests:
          continue
        if x == 'lto' and os.path.basename(name) in ['21.c']:
          continue # LLVM LTO bug

        print(name)
        if name.endswith('.cpp'):
          self.emcc_args.append('-std=c++03')
        self.do_runf(path_from_root('tests', 'fuzz', name),
                     open(path_from_root('tests', 'fuzz', name + '.txt')).read())
        if name.endswith('.cpp'):
          self.emcc_args.remove('-std=c++03')

    run_all('normal')

    self.emcc_args += ['-flto']

    run_all('lto')

  @also_with_standalone_wasm(wasm2c=True, impure=True)
  @no_asan('autodebug logging interferes with asan')
  @with_env_modify({'EMCC_AUTODEBUG': '1'})
  def test_autodebug_wasm(self):
    # Autodebug does not work with too much shadow memory.
    # Memory consumed by autodebug depends on the size of the WASM linear memory.
    # With a large shadow memory, the JS engine runs out of memory.
    if '-fsanitize=address' in self.emcc_args:
      self.set_setting('ASAN_SHADOW_SIZE', 16 * 1024 * 1024)

    # test that the program both works and also emits some of the logging
    # (but without the specific output, as it is logging the actual locals
    # used and so forth, which will change between opt modes and updates of
    # llvm etc.)
    def check(out, err):
      for msg in ['log_execution', 'get_i32', 'set_i32', 'load_ptr', 'load_val', 'store_ptr', 'store_val']:
        self.assertIn(msg, out)
      return out + err

    self.do_runf(path_from_root('tests', 'core', 'test_autodebug.c'),
                 'success', output_nicerizer=check)

  ### Integration tests

  @sync
  def test_ccall(self):
    self.emcc_args.append('-Wno-return-stack-address')
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['ccall', 'cwrap'])
    create_test_file('post.js', '''
      out('*');
      var ret;
      ret = Module['ccall']('get_int', 'number'); out([typeof ret, ret].join(','));
      ret = ccall('get_float', 'number'); out([typeof ret, ret.toFixed(2)].join(','));
      ret = ccall('get_bool', 'boolean'); out([typeof ret, ret].join(','));
      ret = ccall('get_string', 'string'); out([typeof ret, ret].join(','));
      ret = ccall('print_int', null, ['number'], [12]); out(typeof ret);
      ret = ccall('print_float', null, ['number'], [14.56]); out(typeof ret);
      ret = ccall('print_bool', null, ['boolean'], [true]); out(typeof ret);
      ret = ccall('print_string', null, ['string'], ["cheez"]); out(typeof ret);
      ret = ccall('print_string', null, ['array'], [[97, 114, 114, 45, 97, 121, 0]]); out(typeof ret); // JS array
      ret = ccall('print_string', null, ['array'], [new Uint8Array([97, 114, 114, 45, 97, 121, 0])]); out(typeof ret); // typed array
      ret = ccall('multi', 'number', ['number', 'number', 'number', 'string'], [2, 1.4, 3, 'more']); out([typeof ret, ret].join(','));
      var p = ccall('malloc', 'pointer', ['number'], [4]);
      setValue(p, 650, 'i32');
      ret = ccall('pointer', 'pointer', ['pointer'], [p]); out([typeof ret, getValue(ret, 'i32')].join(','));
      out('*');
      // part 2: cwrap
      var noThirdParam = Module['cwrap']('get_int', 'number');
      out(noThirdParam());
      var multi = Module['cwrap']('multi', 'number', ['number', 'number', 'number', 'string']);
      out(multi(2, 1.4, 3, 'atr'));
      out(multi(8, 5.4, 4, 'bret'));
      out('*');
      // part 3: avoid stack explosion and check it's restored correctly
      for (var i = 0; i < TOTAL_STACK/60; i++) {
        ccall('multi', 'number', ['number', 'number', 'number', 'string'], [0, 0, 0, '123456789012345678901234567890123456789012345678901234567890']);
      }
      out('stack is ok.');
      ccall('call_ccall_again', null);
      ''')
    self.emcc_args += ['--post-js', 'post.js']

    self.set_setting('EXPORTED_FUNCTIONS', ['_get_int', '_get_float', '_get_bool', '_get_string', '_print_int', '_print_float', '_print_bool', '_print_string', '_multi', '_pointer', '_call_ccall_again', '_malloc'])
    self.do_run_in_out_file_test('tests', 'core', 'test_ccall.cpp')

    if '-O2' in self.emcc_args and '-g' not in self.emcc_args:
      print('with closure')
      self.emcc_args += ['--closure', '1']
      self.do_run_in_out_file_test('tests', 'core', 'test_ccall.cpp')

  def test_EXTRA_EXPORTED_RUNTIME_METHODS(self):
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', ['$dynCall'])
    self.do_run_in_out_file_test('tests', 'core', 'EXTRA_EXPORTED_RUNTIME_METHODS.c')
    # test dyncall (and other runtime methods in support.js) can be exported
    self.emcc_args += ['-DEXPORTED']
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['dynCall', 'addFunction', 'lengthBytesUTF8', 'getTempRet0', 'setTempRet0'])
    self.do_run_in_out_file_test('tests', 'core', 'EXTRA_EXPORTED_RUNTIME_METHODS.c')

  @no_minimal_runtime('MINIMAL_RUNTIME does not blindly export all symbols to Module to save code size')
  def test_dyncall_specific(self):
    emcc_args = self.emcc_args[:]
    for which, exported_runtime_methods in [
        ('DIRECT', []),
        ('EXPORTED', []),
        ('FROM_OUTSIDE', ['dynCall_viji'])
      ]:
      print(which)
      self.emcc_args = emcc_args + ['-D' + which]
      self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', exported_runtime_methods)
      self.do_run_in_out_file_test('tests', 'core', 'dyncall_specific.c')

  def test_getValue_setValue(self):
    # these used to be exported, but no longer are by default
    def test(output_prefix='', args=[], assert_returncode=0):
      old = self.emcc_args[:]
      self.emcc_args += args
      src = path_from_root('tests', 'core', 'getValue_setValue.cpp')
      expected = path_from_root('tests', 'core', 'getValue_setValue' + output_prefix + '.out')
      self.do_run_from_file(src, expected, assert_returncode=assert_returncode)
      self.emcc_args = old

    # see that direct usage (not on module) works. we don't export, but the use
    # keeps it alive through JSDCE
    test(args=['-DDIRECT'])
    # see that with assertions, we get a nice error message
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', [])
    self.set_setting('ASSERTIONS', 1)
    test('_assert', assert_returncode=NON_ZERO)
    self.set_setting('ASSERTIONS', 0)
    # see that when we export them, things work on the module
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['getValue', 'setValue'])
    test()

  def test_FS_exports(self):
    # these used to be exported, but no longer are by default
    for use_files in (0, 1):
      print(use_files)

      def test(output_prefix='', args=[], assert_returncode=0):
        if use_files:
          args += ['-DUSE_FILES']
        print(args)
        old = self.emcc_args[:]
        self.emcc_args += args
        self.do_runf(path_from_root('tests', 'core', 'FS_exports.cpp'),
                     (open(path_from_root('tests', 'core', 'FS_exports' + output_prefix + '.out')).read(),
                      open(path_from_root('tests', 'core', 'FS_exports' + output_prefix + '_2.out')).read()),
                     assert_returncode=assert_returncode)
        self.emcc_args = old

      # see that direct usage (not on module) works. we don't export, but the use
      # keeps it alive through JSDCE
      test(args=['-DDIRECT', '-s', 'FORCE_FILESYSTEM=1'])
      # see that with assertions, we get a nice error message
      self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', [])
      self.set_setting('ASSERTIONS', 1)
      test('_assert', assert_returncode=NON_ZERO)
      self.set_setting('ASSERTIONS', 0)
      # see that when we export them, things work on the module
      self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['FS_createDataFile'])
      test(args=['-s', 'FORCE_FILESYSTEM=1'])

  def test_legacy_exported_runtime_numbers(self):
    # these used to be exported, but no longer are by default
    def test(output_prefix='', args=[], assert_returncode=0):
      old = self.emcc_args[:]
      self.emcc_args += args
      src = path_from_root('tests', 'core', 'legacy_exported_runtime_numbers.cpp')
      expected = path_from_root('tests', 'core', 'legacy_exported_runtime_numbers%s.out' % output_prefix)
      self.do_run_from_file(src, expected, assert_returncode=assert_returncode)
      self.emcc_args = old

    # see that direct usage (not on module) works. we don't export, but the use
    # keeps it alive through JSDCE
    test(args=['-DDIRECT'])
    # see that with assertions, we get a nice error message
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', [])
    self.set_setting('ASSERTIONS', 1)
    test('_assert', assert_returncode=NON_ZERO)
    self.set_setting('ASSERTIONS', 0)
    # see that when we export them, things work on the module
    self.set_setting('EXTRA_EXPORTED_RUNTIME_METHODS', ['ALLOC_STACK'])
    test()

  def test_response_file(self):
    response_data = '-o %s/response_file.js %s' % (self.get_dir(), path_from_root('tests', 'hello_world.cpp'))
    create_test_file('rsp_file', response_data.replace('\\', '\\\\'))
    self.run_process([EMCC, "@rsp_file"] + self.get_emcc_args())
    self.do_run('response_file.js', 'hello, world', no_build=True)

    self.assertContained('response file not found: foo.txt', self.expect_fail([EMCC, '@foo.txt']))

  def test_linker_response_file(self):
    objfile = 'response_file.o'
    self.run_process([EMCC, '-c', path_from_root('tests', 'hello_world.cpp'), '-o', objfile] + self.get_emcc_args())
    # This should expand into -Wl,--start-group <objfile> -Wl,--end-group
    response_data = '--start-group ' + objfile + ' --end-group'
    create_test_file('rsp_file', response_data.replace('\\', '\\\\'))
    self.run_process([EMCC, "-Wl,@rsp_file", '-o', 'response_file.o.js'] + self.get_emcc_args())
    self.do_run('response_file.o.js', 'hello, world', no_build=True)

  def test_exported_response(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <emscripten.h>

      extern "C" {
        int other_function() { return 5; }
      }

      int main() {
        int x = EM_ASM_INT({ return Module._other_function() });
        emscripten_run_script_string(""); // Add a reference to a symbol that exists in src/deps_info.json to uncover issue #2836 in the test suite.
        printf("waka %d!\n", x);
        return 0;
      }
    '''
    create_test_file('exps', '["_main","_other_function"]')

    self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=@exps']
    self.do_run(src, '''waka 5!''')
    assert 'other_function' in open('src.js').read()

  def test_large_exported_response(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <emscripten.h>

      extern "C" {
      '''

    js_funcs = []
    num_exports = 5000
    count = 0
    while count < num_exports:
        src += 'int exported_func_from_response_file_%d () { return %d;}\n' % (count, count)
        js_funcs.append('_exported_func_from_response_file_%d' % count)
        count += 1

    src += r'''
      }

      int main() {
        int x = EM_ASM_INT({ return Module._exported_func_from_response_file_4999() });
        emscripten_run_script_string(""); // Add a reference to a symbol that exists in src/deps_info.json to uncover issue #2836 in the test suite.
        printf("waka %d!\n", x);
        return 0;
      }
    '''

    js_funcs.append('_main')
    exported_func_json_file = 'large_exported_response.json'
    create_test_file(exported_func_json_file, json.dumps(js_funcs))

    self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=@' + exported_func_json_file]
    self.do_run(src, '''waka 4999!''')
    assert '_exported_func_from_response_file_1' in open('src.js').read()

  @sync
  def test_add_function(self):
    self.set_setting('INVOKE_RUN', 0)
    self.set_setting('RESERVED_FUNCTION_POINTERS', 1)
    self.set_setting('EXPORTED_RUNTIME_METHODS', ['callMain'])
    src = path_from_root('tests', 'interop', 'test_add_function.cpp')
    post_js = path_from_root('tests', 'interop', 'test_add_function_post.js')
    self.emcc_args += ['--post-js', post_js]

    print('basics')
    self.do_run_in_out_file_test('tests', 'interop', 'test_add_function.cpp')

    print('with RESERVED_FUNCTION_POINTERS=0')
    self.set_setting('RESERVED_FUNCTION_POINTERS', 0)
    expected = 'Unable to grow wasm table'
    if self.is_wasm2js() and is_optimizing(self.emcc_args):
      # in wasm2js the error message doesn't come from the VM, but from our
      # emulation code. when ASSERTIONS are enabled we show a clear message, but
      # in optimized builds we don't waste code size on that, and the JS engine
      # shows a generic error.
      expected = 'table.grow is not a function'
    self.do_runf(src, expected, assert_returncode=NON_ZERO)

    print('- with table growth')
    self.set_setting('ALLOW_TABLE_GROWTH', 1)
    self.emcc_args += ['-DGROWTH']
    # enable costly assertions to verify correct table behavior
    self.set_setting('ASSERTIONS', 2)
    self.do_run_in_out_file_test('tests', 'interop', 'test_add_function.cpp')

  def test_getFuncWrapper_sig_alias(self):
    self.emcc_args += ['-s', 'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[$getFuncWrapper]']
    src = r'''
    #include <stdio.h>
    #include <emscripten.h>

    void func1(int a) {
      printf("func1\n");
    }
    void func2(int a, int b) {
      printf("func2\n");
    }

    int main() {
      EM_ASM({
        getFuncWrapper($0, 'vi')(0);
        getFuncWrapper($1, 'vii')(0, 0);
      }, func1, func2);
      return 0;
    }
    '''
    self.do_run(src, 'func1\nfunc2\n')

  def test_emulate_function_pointer_casts(self):
    self.set_setting('EMULATE_FUNCTION_POINTER_CASTS', 1)

    self.do_runf(path_from_root('tests', 'core', 'test_emulate_function_pointer_casts.cpp'),
                 ('|1.266,1|',                 # asm.js, double <-> int
                  '|1.266,1413754136|')) # wasm, reinterpret the bits

  @no_wasm2js('TODO: nicely printed names in wasm2js')
  @parameterized({
    'normal': ([],),
    'noexcept': (['-fno-exceptions'],)
  })
  def test_demangle_stacks(self, extra_args):
    self.emcc_args += extra_args
    self.set_setting('DEMANGLE_SUPPORT', 1)
    self.set_setting('ASSERTIONS', 1)
    # ensure function names are preserved
    self.emcc_args += ['--profiling-funcs', '--llvm-opts', '0']
    self.do_run_in_out_file_test('tests', 'core', 'test_demangle_stacks.cpp', assert_returncode=NON_ZERO)
    if not self.has_changed_setting('ASSERTIONS'):
      print('without assertions, the stack is not printed, but a message suggesting assertions is')
      self.set_setting('ASSERTIONS', 0)
      self.do_run_in_out_file_test('tests', 'core', 'test_demangle_stacks_noassert.cpp', assert_returncode=NON_ZERO)

  def test_demangle_stacks_symbol_map(self):
    self.set_setting('DEMANGLE_SUPPORT', 1)
    if '-O' in str(self.emcc_args) and '-O0' not in self.emcc_args and '-O1' not in self.emcc_args and '-g' not in self.emcc_args:
      self.emcc_args += ['--llvm-opts', '0']
    else:
      self.skipTest("without opts, we don't emit a symbol map")
    self.emcc_args += ['--emit-symbol-map']
    self.do_runf(path_from_root('tests', 'core', 'test_demangle_stacks.cpp'), 'abort', assert_returncode=NON_ZERO)
    # make sure the shortened name is the right one
    full_aborter = None
    short_aborter = None
    for line in open('test_demangle_stacks.js.symbols').readlines():
      if ':' not in line:
        continue
      # split by the first ':' (wasm backend demangling may include more :'s later on)
      short, full = line.split(':', 1)
      if 'Aborter' in full:
        short_aborter = short
        full_aborter = full
    self.assertIsNotNone(full_aborter)
    self.assertIsNotNone(short_aborter)
    print('full:', full_aborter, 'short:', short_aborter)
    if SPIDERMONKEY_ENGINE and os.path.exists(SPIDERMONKEY_ENGINE[0]):
      output = self.run_js('test_demangle_stacks.js', engine=SPIDERMONKEY_ENGINE, assert_returncode=NON_ZERO)
      # we may see the full one, if -g, or the short one if not
      if ' ' + short_aborter + ' ' not in output and ' ' + full_aborter + ' ' not in output:
        # stack traces may also be ' name ' or 'name@' etc
        if '\n' + short_aborter + ' ' not in output and '\n' + full_aborter + ' ' not in output and 'wasm-function[' + short_aborter + ']' not in output:
          if '\n' + short_aborter + '@' not in output and '\n' + full_aborter + '@' not in output:
            self.assertContained(' ' + short_aborter + ' ' + '\n' + ' ' + full_aborter + ' ', output)

  def test_tracing(self):
    self.emcc_args += ['--tracing']
    self.do_run_in_out_file_test('tests', 'core', 'test_tracing.c')

  @no_wasm_backend('https://github.com/emscripten-core/emscripten/issues/9527')
  def test_eval_ctors(self):
    if '-O2' not in str(self.emcc_args) or '-O1' in str(self.emcc_args):
      self.skipTest('need js optimizations')
    if not self.get_setting('WASM'):
      self.skipTest('this test uses wasm binaries')

    orig_args = self.emcc_args

    print('leave printf in ctor')
    self.emcc_args = orig_args + ['-s', 'EVAL_CTORS=1']
    self.do_run(r'''
      #include <stdio.h>
      struct C {
        C() { printf("constructing!\n"); } // don't remove this!
      };
      C c;
      int main() {}
    ''', "constructing!\n")

    def get_code_size():
      if self.is_wasm():
        # Use number of functions as a for code size
        return self.count_wasm_contents('src.wasm', 'funcs')
      else:
        return os.path.getsize('src.js')

    def get_mem_size():
      if self.is_wasm():
        # Use number of functions as a for code size
        return self.count_wasm_contents('src.wasm', 'memory-data')
      if self.uses_memory_init_file():
        return os.path.getsize('src.js.mem')

      # otherwise we ignore memory size
      return 0

    def do_test(test):
      self.emcc_args = orig_args + ['-s', 'EVAL_CTORS=1']
      test()
      ec_code_size = get_code_size()
      ec_mem_size = get_mem_size()
      self.emcc_args = orig_args
      test()
      code_size = get_code_size()
      mem_size = get_mem_size()
      if mem_size:
        print('mem: ', mem_size, '=>', ec_mem_size)
        self.assertGreater(ec_mem_size, mem_size)
      print('code:', code_size, '=>', ec_code_size)
      self.assertLess(ec_code_size, code_size)

    print('remove ctor of just assigns to memory')

    def test1():
      self.do_run(r'''
        #include <stdio.h>
        struct C {
          int x;
          C() {
            volatile int y = 10;
            y++;
            x = y;
          }
        };
        C c;
        int main() {
          printf("x: %d\n", c.x);
        }
      ''', "x: 11\n")

    do_test(test1)

    # The wasm backend currently exports a single initalizer so the ctor
    # evaluation is all or nothing.  As well as that it doesn't currently
    # do DCE of libcxx symbols (because the are marked as visibility(defaault)
    # and because of that we end up not being able to eval ctors unless all
    # libcxx constrcutors can be eval'd

    print('libcxx - remove 2 ctors from iostream code')
    src = open(path_from_root('tests', 'hello_libcxx.cpp')).read()
    output = 'hello, world!'

    def test2():
      self.do_run(src, output)
    do_test(test2)

    print('assertions too')
    self.set_setting('ASSERTIONS', 1)
    self.do_run(src, output)
    self.set_setting('ASSERTIONS', 0)

    print('remove just some, leave others')

    def test3():
      self.do_run(r'''
#include <iostream>
#include <string>

class std_string {
public:
  std_string(): ptr(nullptr) { std::cout << "std_string()\n"; }
  std_string(const char* s): ptr(s) { std::cout << "std_string(const char* s)" << std::endl; }
  std_string(const std_string& s): ptr(s.ptr) { std::cout << "std_string(const std_string& s) " << std::endl; }
  const char* data() const { return ptr; }
private:
  const char* ptr;
};

const std_string txtTestString("212121\0");
const std::string s2text("someweirdtext");

int main() {
  std::cout << s2text << std::endl;
  std::cout << txtTestString.data() << std::endl;
  std::cout << txtTestString.data() << std::endl;
  return 0;
}
      ''', '''std_string(const char* s)
someweirdtext
212121
212121
''') # noqa
    do_test(test3)

  def test_embind(self):
    self.emcc_args += ['--bind']

    src = r'''
      #include <stdio.h>
      #include <emscripten/val.h>

      using namespace emscripten;

      int main() {
        val Math = val::global("Math");

        // two ways to call Math.abs
        printf("abs(-10): %d\n", Math.call<int>("abs", -10));
        printf("abs(-11): %d\n", Math["abs"](-11).as<int>());

        return 0;
      }
    '''
    self.do_run(src, 'abs(-10): 10\nabs(-11): 11')

  def test_embind_2(self):
    self.emcc_args += ['--bind', '--post-js', 'post.js']
    create_test_file('post.js', '''
      function printLerp() {
          out('lerp ' + Module.lerp(100, 200, 66) + '.');
      }
    ''')
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>
      #include <emscripten/bind.h>
      using namespace emscripten;
      int lerp(int a, int b, int t) {
          return (100 - t) * a + t * b;
      }
      EMSCRIPTEN_BINDINGS(my_module) {
          function("lerp", &lerp);
      }
      int main(int argc, char **argv) {
          EM_ASM(printLerp());
          return 0;
      }
    '''
    self.do_run(src, 'lerp 166')

  def test_embind_3(self):
    self.emcc_args += ['--bind', '--post-js', 'post.js']
    create_test_file('post.js', '''
      function ready() {
        try {
          Module.compute(new Uint8Array([1,2,3]));
        } catch(e) {
          out(e);
        }
      }
    ''')
    src = r'''
      #include <emscripten.h>
      #include <emscripten/bind.h>
      using namespace emscripten;
      int compute(int array[]) {
          return 0;
      }
      EMSCRIPTEN_BINDINGS(my_module) {
          function("compute", &compute, allow_raw_pointers());
      }
      int main(int argc, char **argv) {
          EM_ASM(ready());
          return 0;
      }
    '''
    self.do_run(src, 'UnboundTypeError: Cannot call compute due to unbound types: Pi')

  @no_wasm_backend('long doubles are f128s in wasm backend')
  def test_embind_4(self):
    self.emcc_args += ['--bind', '--post-js', 'post.js']
    create_test_file('post.js', '''
      function printFirstElement() {
        out(Module.getBufferView()[0]);
      }
    ''')
    src = r'''
      #include <emscripten.h>
      #include <emscripten/bind.h>
      #include <emscripten/val.h>
      #include <stdio.h>
      using namespace emscripten;

      const size_t kBufferSize = 1024;
      long double buffer[kBufferSize];
      val getBufferView(void) {
          val v = val(typed_memory_view(kBufferSize, buffer));
          return v;
      }
      EMSCRIPTEN_BINDINGS(my_module) {
          function("getBufferView", &getBufferView);
      }

      int main(int argc, char **argv) {
        buffer[0] = 107;
        EM_ASM(printFirstElement());
        return 0;
      }
    '''
    self.do_run(src, '107')

  def test_embind_5(self):
    self.emcc_args += ['--bind', '-s', 'EXIT_RUNTIME=1']
    self.do_run_in_out_file_test('tests', 'core', 'test_embind_5.cpp')

  def test_embind_custom_marshal(self):
    self.emcc_args += ['--bind', '--pre-js', path_from_root('tests', 'embind', 'test_custom_marshal.js')]
    self.do_run_in_out_file_test('tests', 'embind', 'test_custom_marshal.cpp', assert_identical=True)

  def test_embind_float_constants(self):
    self.emcc_args += ['--bind']
    self.do_run_from_file(path_from_root('tests', 'embind', 'test_float_constants.cpp'),
                          path_from_root('tests', 'embind', 'test_float_constants.out'))

  def test_embind_negative_constants(self):
    self.emcc_args += ['--bind']
    self.do_run_from_file(path_from_root('tests', 'embind', 'test_negative_constants.cpp'),
                          path_from_root('tests', 'embind', 'test_negative_constants.out'))

  @also_with_wasm_bigint
  def test_embind_unsigned(self):
    self.emcc_args += ['--bind']
    self.do_run_from_file(path_from_root('tests', 'embind', 'test_unsigned.cpp'), path_from_root('tests', 'embind', 'test_unsigned.out'))

  def test_embind_val(self):
    self.emcc_args += ['--bind']
    self.do_run_from_file(path_from_root('tests', 'embind', 'test_val.cpp'), path_from_root('tests', 'embind', 'test_val.out'))

  def test_embind_no_rtti(self):
    src = r'''
      #include <emscripten.h>
      #include <emscripten/bind.h>
      #include <emscripten/val.h>
      #include <stdio.h>

      EM_JS(void, calltest, (), {
        console.log("dotest returned: " + Module.dotest());
      });

      int main(int argc, char** argv){
        printf("418\n");
        calltest();
        return 0;
      }

      int test() {
        return 42;
      }

      EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("dotest", &test);
      }
    '''
    self.emcc_args += ['--bind', '-fno-rtti', '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0']
    self.do_run(src, '418\ndotest returned: 42\n')

  def test_embind_polymorphic_class_no_rtti(self):
    self.emcc_args += ['--bind', '-fno-rtti', '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0']
    self.do_run_in_out_file_test('tests', 'core', 'test_embind_polymorphic_class_no_rtti.cpp')

  def test_embind_no_rtti_followed_by_rtti(self):
    src = r'''
      #include <emscripten.h>
      #include <emscripten/bind.h>
      #include <emscripten/val.h>
      #include <stdio.h>

      EM_JS(void, calltest, (), {
        console.log("dotest returned: " + Module.dotest());
      });

      int main(int argc, char** argv){
        printf("418\n");
        calltest();
        return 0;
      }

      int test() {
        return 42;
      }

      EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("dotest", &test);
      }
    '''
    self.emcc_args += ['--bind', '-fno-rtti', '-frtti']
    self.do_run(src, '418\ndotest returned: 42\n')

  @parameterized({
    'all': ('ALL', False),
    'fast': ('FAST', False),
    'default': ('DEFAULT', False),
    'all_growth': ('ALL', True),
  })
  @sync
  def test_webidl(self, mode, allow_memory_growth):
    if self.run_name == 'asm2':
      self.emcc_args += ['--closure', '1', '-g1'] # extra testing
      # avoid closure minified names competing with our test code in the global name space
      self.set_setting('MODULARIZE', 1)

    # Force IDL checks mode
    os.environ['IDL_CHECKS'] = mode

    self.run_process([PYTHON, path_from_root('tools', 'webidl_binder.py'),
                     path_from_root('tests', 'webidl', 'test.idl'),
                     'glue'])
    self.assertExists('glue.cpp')
    self.assertExists('glue.js')

    # Export things on "TheModule". This matches the typical use pattern of the bound library
    # being used as Box2D.* or Ammo.*, and we cannot rely on "Module" being always present (closure may remove it).
    self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=["_malloc","_free"]', '--post-js', 'glue.js']
    if allow_memory_growth:
      self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH', '-Wno-almost-asm']

    def post(filename):
      with open(filename, 'a') as f:
        f.write('\n\n')
        if self.run_name == 'asm2':
          f.write('var TheModule = Module();\n')
        else:
          f.write('var TheModule = Module;\n')
        f.write('\n\n')
        if allow_memory_growth:
          f.write("var isMemoryGrowthAllowed = true;")
        else:
          f.write("var isMemoryGrowthAllowed = false;")
        f.write(open(path_from_root('tests', 'webidl', 'post.js')).read())
        f.write('\n\n')

    output = path_from_root('tests', 'webidl', "output_%s.txt" % mode)
    self.do_run_from_file(path_from_root('tests', 'webidl', 'test.cpp'), output, post_build=post)

  ### Tests for tools

  @no_wasm2js('TODO: source maps in wasm2js')
  @parameterized({
    '': ([],),
    'minimal_runtime': (['-s', 'MINIMAL_RUNTIME'],),
  })
  def test_source_map(self, args):
    if '-g' not in self.emcc_args:
      self.emcc_args.append('-g')

    self.emcc_args += args

    src = '''
      #include <stdio.h>
      #include <assert.h>

      __attribute__((noinline)) int foo() {
        printf("hi"); // line 6
        return 1; // line 7
      }

      int main() {
        printf("%d", foo()); // line 11
        return 0; // line 12
      }
    '''
    create_test_file('src.cpp', src)

    out_filename = 'a.out.js'
    wasm_filename = 'a.out.wasm'
    no_maps_filename = 'no-maps.out.js'

    assert '-g4' not in self.emcc_args
    building.emcc('src.cpp',
                  self.serialize_settings() + self.emcc_args + self.emcc_args,
                  out_filename)
    # the file name may find its way into the generated code, so make sure we
    # can do an apples-to-apples comparison by compiling with the same file name
    shutil.move(out_filename, no_maps_filename)
    with open(no_maps_filename) as f:
      no_maps_file = f.read()
    no_maps_file = re.sub(' *//[@#].*$', '', no_maps_file, flags=re.MULTILINE)
    self.emcc_args.append('-g4')

    building.emcc(os.path.abspath('src.cpp'),
                  self.serialize_settings() + self.emcc_args + self.emcc_args,
                  out_filename,
                  stderr=PIPE)
    map_referent = out_filename if not self.get_setting('WASM') else wasm_filename
    # after removing the @line and @sourceMappingURL comments, the build
    # result should be identical to the non-source-mapped debug version.
    # this is worth checking because the parser AST swaps strings for token
    # objects when generating source maps, so we want to make sure the
    # optimizer can deal with both types.
    map_filename = map_referent + '.map'

    def encode_utf8(data):
      if isinstance(data, dict):
        for key in data:
          data[key] = encode_utf8(data[key])
        return data
      elif isinstance(data, list):
        for i in range(len(data)):
          data[i] = encode_utf8(data[i])
        return data
      elif isinstance(data, type(u'')):
        return data.encode('utf8')
      else:
        return data

    data = json.load(open(map_filename))
    if str is bytes:
      # Python 2 compatibility
      data = encode_utf8(data)
    if hasattr(data, 'file'):
      # the file attribute is optional, but if it is present it needs to refer
      # the output file.
      self.assertPathsIdentical(map_referent, data['file'])
    assert len(data['sources']) == 1, data['sources']
    self.assertPathsIdentical('src.cpp', data['sources'][0])
    if hasattr(data, 'sourcesContent'):
      # the sourcesContent attribute is optional, but if it is present it
      # needs to containt valid source text.
      self.assertTextDataIdentical(src, data['sourcesContent'][0])
    mappings = json.loads(self.run_js(
      path_from_root('tools', 'source-maps', 'sourcemap2json.js'),
      args=[map_filename]))
    if str is bytes:
      # Python 2 compatibility
      mappings = encode_utf8(mappings)
    seen_lines = set()
    for m in mappings:
      self.assertPathsIdentical('src.cpp', m['source'])
      seen_lines.add(m['originalLine'])
    # ensure that all the 'meaningful' lines in the original code get mapped
    # when optimizing, the binaryen optimizer may remove some of them (by inlining, etc.)
    if is_optimizing(self.emcc_args):
      assert seen_lines.issuperset([11, 12]), seen_lines
    else:
      assert seen_lines.issuperset([6, 7, 11, 12]), seen_lines

  @no_wasm2js('TODO: source maps in wasm2js')
  def test_dwarf(self):
    self.emcc_args.append('-g')

    create_test_file('src.cpp', '''
      #include <emscripten.h>
      EM_JS(int, out_to_js, (int x), {})
      void foo() {
        out_to_js(0); // line 5
        out_to_js(1); // line 6
        out_to_js(2); // line 7
        // A silly possible recursion to avoid binaryen doing any inlining.
        if (out_to_js(3)) foo();
      }
      int main() {
        foo();
      }
    ''')

    js_filename = 'a.out.js'
    wasm_filename = 'a.out.wasm'

    building.emcc('src.cpp',
                  self.serialize_settings() + self.emcc_args,
                  js_filename)

    LLVM_DWARFDUMP = os.path.join(LLVM_ROOT, 'llvm-dwarfdump')
    out = self.run_process([LLVM_DWARFDUMP, wasm_filename, '-all'], stdout=PIPE).stdout

    # parse the sections
    sections = {}
    curr_section_name = ''
    curr_section_body = ''

    def add_section():
      if curr_section_name:
        sections[curr_section_name] = curr_section_body

    for line in out.splitlines():
      if ' contents:' in line:
        # a new section, a line like ".debug_str contents:"
        add_section()
        curr_section_name = line.split(' ')[0]
        curr_section_body = ''
      else:
        # possibly a line in a section
        if curr_section_name:
          curr_section_body += line + '\n'
    add_section()

    # make sure the right sections exist
    self.assertIn('.debug_abbrev', sections)
    self.assertIn('.debug_info', sections)
    self.assertIn('.debug_line', sections)
    self.assertIn('.debug_str', sections)
    self.assertIn('.debug_ranges', sections)

    # verify some content in the sections
    self.assertIn('"src.cpp"', sections['.debug_info'])
    # the line section looks like this:
    # Address            Line   Column File   ISA Discriminator Flags
    # ------------------ ------ ------ ------ --- ------------- -------------
    # 0x000000000000000b      5      0      3   0             0  is_stmt
    src_to_addr = {}
    for line in sections['.debug_line'].splitlines():
      if line.startswith('0x'):
        while '  ' in line:
          line = line.replace('  ', ' ')
        addr, line, col = line.split(' ')[:3]
        key = (int(line), int(col))
        src_to_addr.setdefault(key, []).append(addr)

    # each of the calls must remain in the binary, and be mapped
    self.assertIn((5, 9), src_to_addr)
    self.assertIn((6, 9), src_to_addr)
    self.assertIn((7, 9), src_to_addr)

    def get_dwarf_addr(line, col):
      addrs = src_to_addr[(line, col)]
      assert len(addrs) == 1, 'we assume the simple calls have one address'
      return int(addrs[0], 0)

    # the lines must appear in sequence (as calls to JS, the optimizer cannot
    # reorder them)
    self.assertLess(get_dwarf_addr(5, 9), get_dwarf_addr(6, 9))
    self.assertLess(get_dwarf_addr(6, 9), get_dwarf_addr(7, 9))

    # get the wat, printing with -g which has binary offsets
    wat = self.run_process([os.path.join(building.get_binaryen_bin(), 'wasm-opt'),
                           wasm_filename, '-g', '--print'], stdout=PIPE).stdout

    # we expect to see a pattern like this, as in both debug and opt builds
    # there isn't much that can change with such calls to JS (they can't be
    # reordered or anything else):
    #
    #   ;; code offset: 0x?
    #   (drop
    #    ;; code offset: 0x?
    #    (call $out_to_js
    #     ;; code offset: 0x?
    #     (local.get ?) or (i32.const ?)
    #    )
    #   )
    #
    # In stacky stream of instructions form, it is
    #   local.get or i32.const
    #   call $out_to_js
    #   drop

    # get_wat_addr gets the address of one of the 3 interesting calls, by its
    # index (0,1,2).
    def get_wat_addr(call_index):
      # find the call_index-th call
      call_loc = -1
      for i in range(call_index + 1):
        call_loc = wat.find('call $out_to_js', call_loc + 1)
        assert call_loc > 0
      # the call begins with the local.get/i32.const printed below it, which is
      # the first instruction in the stream, so it has the lowest address
      start_addr_loc = wat.find('0x', call_loc)
      assert start_addr_loc > 0
      start_addr_loc_end = wat.find('\n', start_addr_loc)
      start_addr = int(wat[start_addr_loc:start_addr_loc_end], 0)
      # the call ends with the drop, which is the last in the stream, at the
      # highest address
      end_addr_loc = wat.rfind('drop', 0, call_loc)
      assert end_addr_loc > 0
      end_addr_loc = wat.rfind('0x', 0, end_addr_loc)
      assert end_addr_loc > 0
      end_addr_loc_end = wat.find('\n', end_addr_loc)
      assert end_addr_loc_end > 0
      end_addr = int(wat[end_addr_loc:end_addr_loc_end], 0)
      return (start_addr, end_addr)

    # match up the DWARF and the wat
    for i in range(3):
      dwarf_addr = get_dwarf_addr(5 + i, 9)
      start_wat_addr, end_wat_addr = get_wat_addr(i)
      # the dwarf may match any of the 3 instructions that form the stream of
      # of instructions implementing the call in the source code, in theory
      self.assertLessEqual(start_wat_addr, dwarf_addr)
      self.assertLessEqual(dwarf_addr, end_wat_addr)

  def test_modularize_closure_pre(self):
    # test that the combination of modularize + closure + pre-js works. in that mode,
    # closure should not minify the Module object in a way that the pre-js cannot use it.
    self.emcc_args += [
      '--pre-js', path_from_root('tests', 'core', 'modularize_closure_pre.js'),
      '--closure', '1',
      '-g1',
      '-s',
      'MODULARIZE=1',
    ]

    def post(filename):
      with open(filename, 'a') as f:
        f.write('\n\n')
        f.write('var TheModule = Module();\n')

    self.do_run_in_out_file_test('tests', 'core', 'modularize_closure_pre.c', post_build=post)

  @no_wasm('wasmifying destroys debug info and stack tracability')
  @no_wasm2js('source maps support')
  def test_emscripten_log(self):
    self.banned_js_engines = [V8_ENGINE] # v8 doesn't support console.log
    self.emcc_args += ['-s', 'DEMANGLE_SUPPORT=1']
    if '-g' not in self.emcc_args:
      self.emcc_args.append('-g')
    self.emcc_args += ['-DRUN_FROM_JS_SHELL']
    self.do_run_in_out_file_test('tests', 'emscripten_log', 'emscripten_log.cpp')
    # test closure compiler as well
    if self.run_name == 'asm2':
      print('closure')
      self.emcc_args += ['--closure', '1', '-g1'] # extra testing
      self.do_run_in_out_file_test('tests', 'emscripten_log', 'emscripten_log_with_closure')

  def test_float_literals(self):
    self.do_run_in_out_file_test('tests', 'test_float_literals.cpp')

  def test_exit_status(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      static void cleanup() {
        printf("cleanup\n");
      }

      int main() {
        atexit(cleanup); // this atexit should still be called
        printf("hello, world!\n");
        // Unusual exit status to make sure it's working!
        if (CAPITAL_EXIT) {
          _Exit(118);
        } else {
          exit(118);
        }
      }
    '''
    create_test_file('pre.js', '''
      Module.preInit = function() {
        addOnExit(function () {
          out('I see exit status: ' + EXITSTATUS);
        });
      }
      ''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_run(src.replace('CAPITAL_EXIT', '0'), 'hello, world!\ncleanup\nI see exit status: 118', assert_returncode=118)
    self.do_run(src.replace('CAPITAL_EXIT', '1'), 'hello, world!\ncleanup\nI see exit status: 118', assert_returncode=118)

  def test_noexitruntime(self):
    src = r'''
      #include <emscripten.h>
      #include <stdio.h>
      static int testPre = TEST_PRE;
      struct Global {
        Global() {
          printf("in Global()\n");
          if (testPre) { EM_ASM(noExitRuntime = true;); }
        }
        ~Global() { printf("ERROR: in ~Global()\n"); }
      } global;
      int main() {
        if (!testPre) { EM_ASM(noExitRuntime = true;); }
        printf("in main()\n");
      }
    '''
    self.do_run(src.replace('TEST_PRE', '0'), 'in Global()\nin main()')
    self.do_run(src.replace('TEST_PRE', '1'), 'in Global()\nin main()')

  def test_minmax(self):
    self.do_runf(path_from_root('tests', 'test_minmax.c'), 'NAN != NAN\nSuccess!')

  def test_locale(self):
    self.do_run_from_file(path_from_root('tests', 'test_locale.c'), path_from_root('tests', 'test_locale.out'))

  def test_vswprintf_utf8(self):
    self.do_run_from_file(path_from_root('tests', 'vswprintf_utf8.c'), path_from_root('tests', 'vswprintf_utf8.out'))

  @no_asan('asan is not compatible with asyncify stack operations; may also need to not instrument asan_c_load_4, TODO')
  def test_async(self):
    # needs to flush stdio streams
    self.set_setting('EXIT_RUNTIME', 1)
    self.set_setting('ASYNCIFY', 1)
    self.banned_js_engines = [SPIDERMONKEY_ENGINE, V8_ENGINE] # needs setTimeout which only node has

    src = r'''
#include <stdio.h>
#include <emscripten.h>
void f(void *p) {
  *(int*)p = 99;
  printf("!");
}
int main() {
  int i = 0;
  printf("Hello");
  emscripten_async_call(f, &i, 1);
  printf("World");
  emscripten_sleep(100);
  printf("%d\n", i);
}
'''

    self.do_run(src, 'HelloWorld!99')

    print('check bad ccall use')
    src = r'''
#include <stdio.h>
#include <emscripten.h>
int main() {
  printf("Hello");
  emscripten_sleep(100);
  printf("World\n");
}
'''
    self.set_setting('ASSERTIONS', 1)
    self.set_setting('INVOKE_RUN', 0)
    create_test_file('pre.js', '''
Module['onRuntimeInitialized'] = function() {
  try {
    ccall('main', 'number', ['number', 'string'], [2, 'waka']);
    var never = true;
  } catch(e) {
    out(e);
    assert(!never);
  }
};
''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_run(src, 'The call to main is running asynchronously.')

    print('check reasonable ccall use')
    src = r'''
#include <stdio.h>
#include <emscripten.h>
int main() {
  printf("Hello");
  emscripten_sleep(100);
  printf("World\n");
}
'''
    create_test_file('pre.js', '''
Module['onRuntimeInitialized'] = function() {
  ccall('main', null, ['number', 'string'], [2, 'waka'], { async: true });
};
''')
    self.do_run(src, 'HelloWorld')

    print('check ccall promise')
    self.set_setting('EXPORTED_FUNCTIONS', ['_stringf', '_floatf'])
    src = r'''
#include <stdio.h>
#include <emscripten.h>
extern "C" {
  const char* stringf(char* param) {
    emscripten_sleep(20);
    printf(param);
    return "second";
  }
  double floatf() {
    emscripten_sleep(20);
    emscripten_sleep(20);
    return 6.4;
  }
}
'''
    create_test_file('pre.js', r'''
Module['onRuntimeInitialized'] = function() {
  ccall('stringf', 'string', ['string'], ['first\n'], { async: true })
    .then(function(val) {
      console.log(val);
      ccall('floatf', 'number', null, null, { async: true }).then(console.log);
    });
};
''')
    self.do_run(src, 'first\nsecond\n6.4')

  @no_asan('asyncify stack operations confuse asan')
  def test_fibers_asyncify(self):
    self.set_setting('ASYNCIFY', 1)
    src = open(path_from_root('tests', 'test_fibers.cpp')).read()
    self.do_run(src, '*leaf-0-100-1-101-1-102-2-103-3-104-5-105-8-106-13-107-21-108-34-109-*')

  def test_asyncify_unused(self):
    # test a program not using asyncify, but the pref is set
    self.set_setting('ASYNCIFY', 1)
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')

  @parameterized({
    'normal': ([], True),
    'removelist_a': (['-s', 'ASYNCIFY_REMOVE=["foo(int, double)"]'], False),
    'removelist_b': (['-s', 'ASYNCIFY_REMOVE=["bar()"]'], True),
    'removelist_c': (['-s', 'ASYNCIFY_REMOVE=["baz()"]'], False),
    'onlylist_a': (['-s', 'ASYNCIFY_ONLY=["main","__original_main","foo(int, double)","baz()","c_baz","Structy::funcy()","bar()"]'], True),
    'onlylist_b': (['-s', 'ASYNCIFY_ONLY=["main","__original_main","foo(int, double)","baz()","c_baz","Structy::funcy()"]'], True),
    'onlylist_c': (['-s', 'ASYNCIFY_ONLY=["main","__original_main","foo(int, double)","baz()","c_baz"]'], False),
    'onlylist_d': (['-s', 'ASYNCIFY_ONLY=["foo(int, double)","baz()","c_baz","Structy::funcy()"]'], False),
    'onlylist_b_response': ([], True,  '["main","__original_main","foo(int, double)","baz()","c_baz","Structy::funcy()"]'),
    'onlylist_c_response': ([], False, '["main","__original_main","foo(int, double)","baz()","c_baz"]'),
  })
  @no_asan('asan is not compatible with asyncify stack operations; may also need to not instrument asan_c_load_4, TODO')
  def test_asyncify_lists(self, args, should_pass, response=None):
    if response is not None:
      create_test_file('response.file', response)
      self.emcc_args += ['-s', 'ASYNCIFY_ONLY=@response.file']
    self.set_setting('ASYNCIFY', 1)
    self.emcc_args += args
    try:
      self.do_run_in_out_file_test('tests', 'core', 'test_asyncify_lists.cpp', assert_identical=True)
      if not should_pass:
        should_pass = True
        raise Exception('should not have passed')
    except Exception:
      if should_pass:
        raise

  @parameterized({
    'normal': ([], True),
    'ignoreindirect': (['-s', 'ASYNCIFY_IGNORE_INDIRECT'], False),
    'add': (['-s', 'ASYNCIFY_IGNORE_INDIRECT', '-s', 'ASYNCIFY_ADD=["__original_main","main","virt()"]'], True),
  })
  @no_asan('asan is not compatible with asyncify stack operations; may also need to not instrument asan_c_load_4, TODO')
  def test_asyncify_indirect_lists(self, args, should_pass):
    self.set_setting('ASYNCIFY', 1)
    self.emcc_args += args
    try:
      self.do_run_in_out_file_test('tests', 'core', 'test_asyncify_indirect_lists.cpp', assert_identical=True)
      if not should_pass:
        should_pass = True
        raise Exception('should not have passed')
    except Exception:
      if should_pass:
        raise

  @no_asan('asyncify stack operations confuse asan')
  def test_emscripten_scan_registers(self):
    self.set_setting('ASYNCIFY', 1)
    self.do_run_in_out_file_test('tests', 'core', 'emscripten_scan_registers.cpp')

  def test_asyncify_assertions(self):
    self.set_setting('ASYNCIFY', 1)
    self.set_setting('ASYNCIFY_IMPORTS', ['suspend'])
    self.set_setting('ASSERTIONS', 1)
    self.do_run_in_out_file_test('tests', 'core', 'asyncify_assertions.cpp')

  @no_asan('asyncify stack operations confuse asan')
  @no_wasm2js('TODO: lazy loading in wasm2js')
  @parameterized({
    'conditional': (True,),
    'unconditional': (False,),
  })
  def test_emscripten_lazy_load_code(self, conditional):
    self.set_setting('ASYNCIFY', 1)
    self.set_setting('ASYNCIFY_LAZY_LOAD_CODE', 1)
    self.set_setting('ASYNCIFY_IGNORE_INDIRECT', 1)
    self.set_setting('MALLOC', 'emmalloc')
    self.emcc_args += ['--profiling-funcs'] # so that we can find the functions for the changes below
    if conditional:
      self.emcc_args += ['-DCONDITIONAL']
    self.do_run_in_out_file_test('tests', 'core', 'emscripten_lazy_load_code.cpp', args=['0'])

    first_size = os.path.getsize('emscripten_lazy_load_code.wasm')
    second_size = os.path.getsize('emscripten_lazy_load_code.wasm.lazy.wasm')
    print('first wasm size', first_size)
    print('second wasm size', second_size)
    if not conditional and is_optimizing(self.emcc_args):
      # If the call to lazy-load is unconditional, then the optimizer can dce
      # out more than half
      self.assertLess(first_size, 0.5 * second_size)

    with open('emscripten_lazy_load_code.wasm', 'rb') as f:
      with open('emscripten_lazy_load_code.wasm.lazy.wasm', 'rb') as g:
        self.assertNotEqual(f.read(), g.read())

    # attempts to "break" the wasm by adding an unreachable in $foo_end. returns whether we found it.
    def break_wasm(name):
      wat = self.run_process([os.path.join(building.get_binaryen_bin(), 'wasm-dis'), name], stdout=PIPE).stdout
      lines = wat.splitlines()
      wat = None
      for i in range(len(lines)):
        if '(func $foo_end ' in lines[i]:
          j = i + 1
          while '(local ' in lines[j]:
            j += 1
          # we found the first line after the local defs
          lines[j] = '(unreachable)' + lines[j]
          wat = '\n'.join(lines)
          break
      if wat is None:
        # $foo_end is not present in the wasm, nothing to break
        shutil.copyfile(name, name + '.orig')
        return False
      with open('wat.wat', 'w') as f:
        f.write(wat)
      shutil.move(name, name + '.orig')
      self.run_process([os.path.join(building.get_binaryen_bin(), 'wasm-as'), 'wat.wat', '-o', name, '-g'])
      return True

    def verify_working(args=['0']):
      self.assertContained('foo_end\n', self.run_js('emscripten_lazy_load_code.js', args=args))

    def verify_broken(args=['0']):
      self.assertNotContained('foo_end\n', self.run_js('emscripten_lazy_load_code.js', args=args, assert_returncode=NON_ZERO))

    # the first-loaded wasm will not reach the second call, since we call it after lazy-loading.
    # verify that by changing the first wasm to throw in that function
    found_foo_end = break_wasm('emscripten_lazy_load_code.wasm')
    if not conditional and is_optimizing(self.emcc_args):
      self.assertFalse(found_foo_end, 'should have optimizd out $foo_end')
    verify_working()
    # but breaking the second wasm actually breaks us
    break_wasm('emscripten_lazy_load_code.wasm.lazy.wasm')
    verify_broken()

    # restore
    shutil.copyfile('emscripten_lazy_load_code.wasm.orig', 'emscripten_lazy_load_code.wasm')
    shutil.copyfile('emscripten_lazy_load_code.wasm.lazy.wasm.orig', 'emscripten_lazy_load_code.wasm.lazy.wasm')
    verify_working()

    if conditional:
      # if we do not call the lazy load function, then we do not need the lazy wasm,
      # and we do the second call in the first wasm
      os.remove('emscripten_lazy_load_code.wasm.lazy.wasm')
      verify_broken()
      verify_working(['42'])
      break_wasm('emscripten_lazy_load_code.wasm')
      verify_broken()

  # Test basic wasm2js functionality in all core compilation modes.
  @no_asan('no wasm2js support yet in asan')
  def test_wasm2js(self):
    if self.get_setting('WASM') == 0:
      self.skipTest('redundant to test wasm2js in wasm2js* mode')
    self.set_setting('WASM', 0)
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')
    # a mem init file is emitted just like with JS
    expect_memory_init_file = self.uses_memory_init_file()
    if expect_memory_init_file:
      self.assertExists('test_hello_world.js.mem')
      with open('test_hello_world.js.mem', 'rb') as f:
        self.assertTrue(f.read()[-1] != b'\0')
    else:
      self.assertNotExists('test_hello_world.js.mem')

  @no_asan('no wasm2js support yet in asan')
  def test_maybe_wasm2js(self):
    if self.get_setting('WASM') == 0:
      self.skipTest('redundant to test wasm2js in wasm2js* mode')
    self.set_setting('MAYBE_WASM2JS', 1)
    # see that running as wasm works
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')
    # run wasm2js, bundle the code, and use the wasm2js path
    cmd = [PYTHON, path_from_root('tools', 'maybe_wasm2js.py'), 'test_hello_world.js', 'test_hello_world.wasm']
    if is_optimizing(self.emcc_args):
      cmd += ['-O2']
    self.run_process(cmd, stdout=open('do_wasm2js.js', 'w')).stdout
    # remove the wasm to make sure we never use it again
    os.remove('test_hello_world.wasm')
    # verify that it runs
    self.assertContained('hello, world!', self.run_js('do_wasm2js.js'))

  @no_asan('no wasm2js support yet in asan')
  @parameterized({
    '': ([],),
    'minimal_runtime': (['-s', 'MINIMAL_RUNTIME'],),
  })
  def test_wasm2js_fallback(self, args):
    if self.get_setting('WASM') == 0:
      self.skipTest('redundant to test wasm2js in wasm2js* mode')

    cmd = [EMCC, path_from_root('tests', 'small_hello_world.c'), '-s', 'WASM=2'] + args
    self.run_process(cmd)

    # First run with WebAssembly support enabled
    # Move the Wasm2js fallback away to test it is not accidentally getting loaded.
    os.rename('a.out.wasm.js', 'a.out.wasm.js.unused')
    self.assertContained('hello!', self.run_js('a.out.js'))
    os.rename('a.out.wasm.js.unused', 'a.out.wasm.js')

    # Then disable WebAssembly support in VM, and try again.. Should still work with Wasm2JS fallback.
    open('b.out.js', 'w').write('WebAssembly = undefined;\n' + open('a.out.js', 'r').read())
    os.remove('a.out.wasm') # Also delete the Wasm file to test that it is not attempted to be loaded.
    self.assertContained('hello!', self.run_js('b.out.js'))

  def test_cxx_self_assign(self):
    # See https://github.com/emscripten-core/emscripten/pull/2688 and http://llvm.org/bugs/show_bug.cgi?id=18735
    self.do_run(r'''
      #include <map>
      #include <stdio.h>

      int main() {
        std::map<int, int> m;
        m[0] = 1;
        m = m;
        // size should still be one after self assignment
        if (m.size() == 1) {
          printf("ok.\n");
        }
      }
    ''', 'ok.')

  def test_memprof_requirements(self):
    # This test checks for the global variables required to run the memory
    # profiler.  It would fail if these variables were made no longer global
    # or if their identifiers were changed.
    create_test_file('main.cpp', '''
      extern "C" {
        void check_memprof_requirements();
      }
      int main() {
        check_memprof_requirements();
        return 0;
      }
    ''')
    create_test_file('lib.js', '''
      mergeInto(LibraryManager.library, {
        check_memprof_requirements: function() {
          if (typeof STACK_BASE === 'number' &&
              typeof STACK_MAX === 'number' &&
              typeof STACKTOP === 'number' &&
              typeof Module['___heap_base'] === 'number') {
             out('able to run memprof');
           } else {
             out('missing the required variables to run memprof');
           }
        }
      });
    ''')
    self.emcc_args += ['--memoryprofiler', '--js-library', 'lib.js']
    self.do_runf('main.cpp', 'able to run memprof')

  def test_fs_dict(self):
    self.set_setting('FORCE_FILESYSTEM', 1)
    self.emcc_args += ['-lidbfs.js']
    self.emcc_args += ['-lnodefs.js']
    create_test_file('pre.js', '''
      Module = {};
      Module['preRun'] = function() {
        out(typeof FS.filesystems['MEMFS']);
        out(typeof FS.filesystems['IDBFS']);
        out(typeof FS.filesystems['NODEFS']);
        // Globals
        console.log(typeof MEMFS);
        console.log(typeof IDBFS);
        console.log(typeof NODEFS);
      };
    ''')
    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_run('int main() { return 0; }', 'object\nobject\nobject\nobject\nobject\nobject')

  def test_fs_dict_none(self):
    # if IDBFS and NODEFS are not enabled, they are not present.
    self.set_setting('FORCE_FILESYSTEM', 1)
    self.set_setting('ASSERTIONS', 1)
    create_test_file('pre.js', '''
      Module = {};
      Module['preRun'] = function() {
        out(typeof FS.filesystems['MEMFS']);
        out(typeof FS.filesystems['IDBFS']);
        out(typeof FS.filesystems['NODEFS']);
        // Globals
        if (ASSERTIONS) {
          console.log(typeof MEMFS);
          console.log(IDBFS);
          console.log(NODEFS);
          FS.mkdir('/working1');
          try {
            FS.mount(IDBFS, {}, '/working1');
          } catch (e) {
            console.log('|' + e + '|');
          }
        }
      };
    ''')
    self.emcc_args += ['--pre-js', 'pre.js']
    expected = '''\
object
undefined
undefined
object
IDBFS is no longer included by default; build with -lidbfs.js
NODEFS is no longer included by default; build with -lnodefs.js
|IDBFS is no longer included by default; build with -lidbfs.js|'''
    self.do_run('int main() { return 0; }', expected)

  @sync
  def test_stack_overflow_check(self):
    self.set_setting('TOTAL_STACK', 1048576)
    self.set_setting('STACK_OVERFLOW_CHECK', 2)
    self.do_runf(path_from_root('tests', 'stack_overflow.cpp'), 'stack overflow', assert_returncode=NON_ZERO)

    self.emcc_args += ['-DONE_BIG_STRING']
    self.do_runf(path_from_root('tests', 'stack_overflow.cpp'), 'stack overflow', assert_returncode=NON_ZERO)

  @node_pthreads
  def test_binaryen_2170_emscripten_atomic_cas_u8(self):
    self.emcc_args += ['-s', 'USE_PTHREADS=1']
    self.do_run_in_out_file_test('tests', 'binaryen_2170_emscripten_atomic_cas_u8.cpp')

  @also_with_standalone_wasm()
  def test_sbrk(self):
    self.do_runf(path_from_root('tests', 'sbrk_brk.cpp'), 'OK.')

  def test_brk(self):
    self.emcc_args += ['-DTEST_BRK=1']
    self.do_runf(path_from_root('tests', 'sbrk_brk.cpp'), 'OK.')

  # Tests that we can use the dlmalloc mallinfo() function to obtain information
  # about malloc()ed blocks and compute how much memory is used/freed.
  @no_asan('mallinfo is not part of ASan malloc')
  def test_mallinfo(self):
    self.do_runf(path_from_root('tests', 'mallinfo.cpp'), 'OK.')

  @no_asan('cannot replace malloc/free with ASan')
  def test_wrap_malloc(self):
    self.do_runf(path_from_root('tests', 'wrap_malloc.cpp'), 'OK.')

  def test_environment(self):
    self.set_setting('ASSERTIONS', 1)

    def test(assert_returncode=0):
      self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c', assert_returncode=assert_returncode)
      js = open('test_hello_world.js').read()
      assert ('require(' in js) == ('node' in self.get_setting('ENVIRONMENT')), 'we should have require() calls only if node js specified'

    for engine in JS_ENGINES:
      print(engine)
      # set us to test in just this engine
      self.banned_js_engines = [e for e in JS_ENGINES if e != engine]
      # tell the compiler to build with just that engine
      if engine == NODE_JS:
        right = 'node'
        wrong = 'shell'
      else:
        right = 'shell'
        wrong = 'node'
      # test with the right env
      self.set_setting('ENVIRONMENT', right)
      print('ENVIRONMENT =', self.get_setting('ENVIRONMENT'))
      test()
      # test with the wrong env
      self.set_setting('ENVIRONMENT', wrong)
      print('ENVIRONMENT =', self.get_setting('ENVIRONMENT'))
      try:
        test(assert_returncode=NON_ZERO)
        raise Exception('unexpected success')
      except Exception as e:
        self.assertContained('not compiled for this environment', str(e))
      # test with a combined env
      self.set_setting('ENVIRONMENT', right + ',' + wrong)
      print('ENVIRONMENT =', self.get_setting('ENVIRONMENT'))
      test()

  def test_postrun_exception(self):
    # verify that an exception thrown in postRun() will not trigger the
    # compilation failed handler, and will be printed to stderr.
    self.add_post_run('ThisFunctionDoesNotExist()')
    self.build(path_from_root('tests', 'core', 'test_hello_world.c'))
    output = self.run_js('test_hello_world.js', assert_returncode=NON_ZERO)
    self.assertStartswith(output, 'hello, world!')
    self.assertContained('ThisFunctionDoesNotExist is not defined', output)

  # Tests that building with -s DECLARE_ASM_MODULE_EXPORTS=0 works
  def test_no_declare_asm_module_exports(self):
    self.set_setting('DECLARE_ASM_MODULE_EXPORTS', 0)
    self.set_setting('WASM_ASYNC_COMPILATION', 0)
    self.maybe_closure()
    self.do_runf(path_from_root('tests', 'declare_asm_module_exports.cpp'), 'jsFunction: 1')
    js = open('declare_asm_module_exports.js').read()
    occurances = js.count('cFunction')
    if is_optimizing(self.emcc_args) and '-g' not in self.emcc_args:
      # In optimized builds only the single reference cFunction that exists in the EM_ASM should exist
      if self.is_wasm():
        self.assertEqual(occurances, 1)
      else:
        # With js the asm module itself also contains a reference for the cFunction name
        self.assertEqual(occurances, 2)
    else:
      print(occurances)

  # Tests that building with -s DECLARE_ASM_MODULE_EXPORTS=0 works
  def test_minimal_runtime_no_declare_asm_module_exports(self):
    self.set_setting('DECLARE_ASM_MODULE_EXPORTS', 0)
    self.set_setting('WASM_ASYNC_COMPILATION', 0)
    self.maybe_closure()
    self.set_setting('MINIMAL_RUNTIME', 1)
    self.do_runf(path_from_root('tests', 'declare_asm_module_exports.cpp'), 'jsFunction: 1')

  # Tests that -s MINIMAL_RUNTIME=1 works well in different build modes
  @parameterized({
    'default': ([],),
    'streaming': (['-s', 'MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION=1'],),
    'streaming_inst': (['-s', 'MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION=1'],),
    'no_export': (['-s', 'DECLARE_ASM_MODULE_EXPORTS=0'],)
  })
  def test_minimal_runtime_hello_world(self, args):
    # TODO: Support for non-Node.js shells has not yet been added to MINIMAL_RUNTIME
    self.banned_js_engines = [V8_ENGINE, SPIDERMONKEY_ENGINE]
    self.emcc_args = ['-s', 'MINIMAL_RUNTIME=1'] + args
    self.set_setting('MINIMAL_RUNTIME', 1)
    self.maybe_closure()
    self.do_runf(path_from_root('tests', 'small_hello_world.c'), 'hello')

  # Test that printf() works in MINIMAL_RUNTIME=1
  @parameterized({
    'fs': (['-s', 'FORCE_FILESYSTEM=1'],),
    'nofs': (['-s', 'NO_FILESYSTEM=1'],),
  })
  def test_minimal_runtime_hello_printf(self, args):
    self.emcc_args = ['-s', 'MINIMAL_RUNTIME=1'] + args
    self.maybe_closure()
    self.do_runf(path_from_root('tests', 'hello_world.c'), 'hello, world!')

  # Tests that -s MINIMAL_RUNTIME=1 works well with SAFE_HEAP
  def test_minimal_runtime_safe_heap(self):
    self.emcc_args = ['-s', 'MINIMAL_RUNTIME=1', '-s', 'SAFE_HEAP=1']
    self.maybe_closure()
    self.do_runf(path_from_root('tests', 'small_hello_world.c'), 'hello')

  # Tests global initializer with -s MINIMAL_RUNTIME=1
  def test_minimal_runtime_global_initializer(self):
    self.set_setting('MINIMAL_RUNTIME', 1)
    self.maybe_closure()
    self.do_runf(path_from_root('tests', 'test_global_initializer.cpp'), 't1 > t0: 1')

  @no_wasm2js('wasm2js wasm emulation does not include custom sections')
  @no_optimize('return address test cannot work with optimizations')
  def test_return_address(self):
    self.emcc_args += ['-s', 'USE_OFFSET_CONVERTER']
    self.do_runf(path_from_root('tests', 'core', 'test_return_address.cpp'), 'passed')

  @no_wasm2js('TODO: sanitizers in wasm2js')
  @no_asan('-fsanitize-minimal-runtime cannot be used with ASan')
  def test_ubsan_minimal_too_many_errors(self):
    self.emcc_args += ['-fsanitize=undefined', '-fsanitize-minimal-runtime']
    if self.get_setting('WASM') == 0:
      if is_optimizing(self.emcc_args):
        self.skipTest('test can only be run without optimizations on asm.js')
      # Need to use `-g` to get proper line numbers in asm.js
      self.emcc_args += ['-g']
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_minimal_too_many_errors.c'),
                 expected_output='ubsan: add-overflow\n' * 20 + 'ubsan: too many errors\n')

  @no_wasm2js('TODO: sanitizers in wasm2js')
  @no_asan('-fsanitize-minimal-runtime cannot be used with ASan')
  def test_ubsan_minimal_errors_same_place(self):
    self.emcc_args += ['-fsanitize=undefined', '-fsanitize-minimal-runtime']
    if self.get_setting('WASM') == 0:
      if is_optimizing(self.emcc_args):
        self.skipTest('test can only be run without optimizations on asm.js')
      # Need to use `-g` to get proper line numbers in asm.js
      self.emcc_args += ['-g']
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_minimal_errors_same_place.c'),
                 expected_output='ubsan: add-overflow\n' * 5)

  @parameterized({
    'fsanitize_undefined': (['-fsanitize=undefined'],),
    'fsanitize_integer': (['-fsanitize=integer'],),
    'fsanitize_overflow': (['-fsanitize=signed-integer-overflow'],),
  })
  @no_wasm2js('TODO: sanitizers in wasm2js')
  def test_ubsan_full_overflow(self, args):
    self.emcc_args += args
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_full_overflow.c'),
                 assert_all=True, expected_output=[
      ".c:3:5: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'",
      ".c:7:7: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'",
    ])

  @parameterized({
    'fsanitize_undefined': (['-fsanitize=undefined'],),
    'fsanitize_return': (['-fsanitize=return'],),
  })
  @no_wasm2js('TODO: sanitizers in wasm2js')
  def test_ubsan_full_no_return(self, args):
    self.emcc_args += ['-Wno-return-type'] + args
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_full_no_return.cpp'),
                 expected_output='.cpp:1:5: runtime error: execution reached the end of a value-returning function without returning a value', assert_returncode=NON_ZERO)

  @parameterized({
    'fsanitize_undefined': (['-fsanitize=undefined'],),
    'fsanitize_integer': (['-fsanitize=integer'],),
    'fsanitize_shift': (['-fsanitize=shift'],),
  })
  @no_wasm2js('TODO: sanitizers in wasm2js')
  def test_ubsan_full_left_shift(self, args):
    self.emcc_args += args
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_full_left_shift.c'),
                 assert_all=True, expected_output=[
      '.c:3:5: runtime error: left shift of negative value -1',
      ".c:7:5: runtime error: left shift of 16 by 29 places cannot be represented in type 'int'"
    ])

  @parameterized({
    'fsanitize_undefined': (['-fsanitize=undefined'],),
    'fsanitize_null': (['-fsanitize=null'],),
  })
  @no_wasm2js('TODO: sanitizers in wasm2js')
  def test_ubsan_full_null_ref(self, args):
    self.emcc_args += args
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_full_null_ref.cpp'),
                 assert_all=True, expected_output=[
      ".cpp:3:12: runtime error: reference binding to null pointer of type 'int'",
      ".cpp:4:13: runtime error: reference binding to null pointer of type 'int'",
      ".cpp:5:14: runtime error: reference binding to null pointer of type 'int'",
    ])

  @parameterized({
    'fsanitize_undefined': (['-fsanitize=undefined'],),
    'fsanitize_vptr': (['-fsanitize=vptr'],),
  })
  @no_wasm2js('TODO: sanitizers in wasm2js')
  def test_ubsan_full_static_cast(self, args):
    self.emcc_args += args
    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_full_static_cast.cpp'),
                 assert_all=True, expected_output=[
      ".cpp:18:10: runtime error: downcast of address",
      "which does not point to an object of type 'R'",
    ])

  @parameterized({
    'g': ('-g', [
      ".cpp:3:12: runtime error: reference binding to null pointer of type 'int'",
      'in main',
    ]),
    'g4': ('-g4', [
      ".cpp:3:12: runtime error: reference binding to null pointer of type 'int'",
      'in main ',
      '.cpp:3:8'
    ]),
  })
  @no_wasm2js('TODO: sanitizers in wasm2js')
  def test_ubsan_full_stack_trace(self, g_flag, expected_output):
    self.emcc_args += ['-fsanitize=null', g_flag, '-s', 'ALLOW_MEMORY_GROWTH=1']

    if g_flag == '-g4':
      if not self.get_setting('WASM'):
        self.skipTest('wasm2js has no source map support')
      elif '-Oz' in self.emcc_args:
        self.skipTest('-Oz breaks stack traces')

    def modify_env(filename):
      with open(filename) as f:
        contents = f.read()
      contents = 'Module = {UBSAN_OPTIONS: "print_stacktrace=1"};' + contents
      with open(filename, 'w') as f:
        f.write(contents)

    self.do_runf(path_from_root('tests', 'core', 'test_ubsan_full_null_ref.cpp'),
                 post_build=modify_env, assert_all=True, expected_output=expected_output)

  def test_template_class_deduction(self):
    self.emcc_args += ['-std=c++17']
    self.do_run_in_out_file_test('tests', 'core', 'test_template_class_deduction.cpp')

  @parameterized({
    'c': ['test_asan_no_error.c'],
    'cpp': ['test_asan_no_error.cpp'],
  })
  def test_asan_no_error(self, name):
    self.emcc_args += ['-fsanitize=address', '-s', 'ALLOW_MEMORY_GROWTH=1']
    self.do_runf(path_from_root('tests', 'core', name), '', assert_returncode=NON_ZERO)

  # note: these tests have things like -fno-builtin-memset in order to avoid
  # clang optimizing things away. for example, a memset might be optimized into
  # stores, and then the stores identified as dead, which leaves nothing for
  # asan to test. here we want to test asan itself, so we work around that.
  @parameterized({
    'use_after_free_c': ('test_asan_use_after_free.c', [
      'AddressSanitizer: heap-use-after-free on address',
    ]),
    'use_after_free_cpp': ('test_asan_use_after_free.cpp', [
      'AddressSanitizer: heap-use-after-free on address',
    ]),
    'use_after_return': ('test_asan_use_after_return.c', [
      'AddressSanitizer: stack-use-after-return on address',
    ], ['-Wno-return-stack-address']),
    'static_buffer_overflow': ('test_asan_static_buffer_overflow.c', [
      'AddressSanitizer: global-buffer-overflow on address',
    ], ['-fno-builtin-memset']),
    'heap_buffer_overflow_c': ('test_asan_heap_buffer_overflow.c', [
      'AddressSanitizer: heap-buffer-overflow on address',
    ], ['-fno-builtin-memset']),
    'heap_buffer_overflow_cpp': ('test_asan_heap_buffer_overflow.cpp', [
      'AddressSanitizer: heap-buffer-overflow on address',
    ], ['-fno-builtin-memset']),
    'stack_buffer_overflow': ('test_asan_stack_buffer_overflow.c', [
      'AddressSanitizer: stack-buffer-overflow'
    ], ['-fno-builtin-memset']),
    'stack_buffer_overflow_js': ('test_asan_stack_buffer_overflow_js.c', [
      'AddressSanitizer: stack-buffer-overflow'
    ], ['-fno-builtin-memset']),
    'bitfield_unround_size': ('test_asan_bitfield_unround_size.c', [
      'AddressSanitizer: stack-buffer-overflow'
    ], ['-fno-builtin-memset']),
    'bitfield_unround_offset': ('test_asan_bitfield_unround_offset.c', [
      'AddressSanitizer: stack-buffer-overflow'
    ], ['-fno-builtin-memset']),
    'bitfield_round': ('test_asan_bitfield_round.c', [
      'AddressSanitizer: stack-buffer-overflow'
    ], ['-fno-builtin-memset']),
    'memset_null': ('test_asan_memset_null.c', [
      'AddressSanitizer: null-pointer-dereference on address 0x00000001'
    ], ['-fno-builtin-memset']),
    'memset_freed': ('test_asan_memset_freed.c', [
      'AddressSanitizer: heap-use-after-free on address'
    ], ['-fno-builtin-memset']),
    'strcpy': ('test_asan_strcpy.c', [
      'AddressSanitizer: heap-buffer-overflow on address'
    ], ['-fno-builtin-strcpy']),
    'memcpy': ('test_asan_memcpy.c', [
      'AddressSanitizer: heap-buffer-overflow on address'
    ], ['-fno-builtin-memcpy']),
    'memchr': ('test_asan_memchr.c', [
      'AddressSanitizer: global-buffer-overflow on address'
    ], ['-fno-builtin-memchr']),
    'vector': ('test_asan_vector.cpp', [
      'AddressSanitizer: container-overflow on address'
    ]),
  })
  def test_asan(self, name, expected_output, cflags=None):
    if '-Oz' in self.emcc_args:
      self.skipTest('-Oz breaks source maps')

    if not self.get_setting('WASM'):
      self.skipTest('wasm2js has no ASan support')

    self.emcc_args += ['-fsanitize=address', '-s', 'ALLOW_MEMORY_GROWTH=1']
    if cflags:
      self.emcc_args += cflags
    self.do_runf(path_from_root('tests', 'core', name),
                 expected_output=expected_output, assert_all=True,
                 check_for_error=False, assert_returncode=NON_ZERO)

  @no_wasm2js('TODO: ASAN in wasm2js')
  def test_asan_js_stack_op(self):
    self.emcc_args += ['-fsanitize=address', '-s', 'ALLOW_MEMORY_GROWTH=1']
    self.do_runf(path_from_root('tests', 'core', 'test_asan_js_stack_op.c'),
                 expected_output='Hello, World!')

  @no_wasm2js('TODO: ASAN in wasm2js')
  def test_asan_api(self):
    self.emcc_args.append('-fsanitize=address')
    self.set_setting('ALLOW_MEMORY_GROWTH')
    self.do_run_in_out_file_test('tests', 'core', 'test_asan_api.c')

  @no_wasm2js('TODO: ASAN in wasm2js')
  def test_asan_modularized_with_closure(self):
    self.emcc_args.append('-sMODULARIZE=1')
    self.emcc_args.append('-sEXPORT_NAME="createModule"')
    self.emcc_args.append('-sUSE_CLOSURE_COMPILER=1')
    self.emcc_args.append('-fsanitize=address')
    self.emcc_args.append('-sALLOW_MEMORY_GROWTH=1')

    def post(filename):
      with open(filename, 'a') as f:
        f.write('\n\n')
        # the bug is that createModule() returns undefined, instead of the
        # proper Promise object.
        f.write('if (!(createModule() instanceof Promise)) throw "Promise was not returned :(";\n')

    self.do_runf(path_from_root('tests', 'hello_world.c'),
                 post_build=post,
                 expected_output='hello, world!')

  def test_safe_stack(self):
    self.set_setting('STACK_OVERFLOW_CHECK', 2)
    self.set_setting('TOTAL_STACK', 65536)
    self.do_runf(path_from_root('tests', 'core', 'test_safe_stack.c'),
                 expected_output=['abort(stack overflow)', '__handle_stack_overflow'], assert_returncode=NON_ZERO)

  def test_safe_stack_alloca(self):
    self.set_setting('STACK_OVERFLOW_CHECK', 2)
    self.set_setting('TOTAL_STACK', 65536)
    self.do_runf(path_from_root('tests', 'core', 'test_safe_stack_alloca.c'),
                 expected_output=['abort(stack overflow)', '__handle_stack_overflow'], assert_returncode=NON_ZERO)

  @needs_dlfcn
  def test_safe_stack_dylink(self):
    self.set_setting('STACK_OVERFLOW_CHECK', 2)
    self.set_setting('TOTAL_STACK', 65536)
    self.dylink_test(r'''
      #include <stdio.h>
      extern void sidey();
      int main() {
        sidey();
      }
    ''', '''
      #include <string.h>

      int f(int *b) {
        int a[64];
        memset(b, 0, 2048 * sizeof(int));
        return f(a);
      }

      void sidey() {
        int a[2048];
        f(a);
      }
    ''', ['abort(stack overflow)', '__handle_stack_overflow'], assert_returncode=NON_ZERO)

  def test_fpic_static(self):
    self.emcc_args.append('-fPIC')
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')

  @node_pthreads
  def test_pthread_create(self):
    self.set_setting('-lbrowser.js')

    def test():
      self.do_run_in_out_file_test('tests', 'core', 'pthread', 'create.cpp')
    test()

    print('with pool')
    # with a pool, we can synchronously depend on workers being available
    self.set_setting('PTHREAD_POOL_SIZE', '2')
    self.emcc_args += ['-DPOOL']
    test()

  def test_emscripten_atomics_stub(self):
    self.do_run_in_out_file_test('tests', 'core', 'pthread', 'emscripten_atomics.c')

  @no_asan('incompatibility with atomics')
  @node_pthreads
  def test_emscripten_atomics(self):
    self.set_setting('USE_PTHREADS', '1')
    self.do_run_in_out_file_test('tests', 'core', 'pthread', 'emscripten_atomics.c')

  @no_asan('incompatibility with atomics')
  @node_pthreads
  def test_emscripten_futexes(self):
    self.set_setting('USE_PTHREADS', '1')
    self.do_run_in_out_file_test('tests', 'core', 'pthread', 'emscripten_futexes.c')

  # Tests the emscripten_get_exported_function() API.
  def test_emscripten_get_exported_function(self):
    # Could also test with -s ALLOW_TABLE_GROWTH=1
    self.set_setting('RESERVED_FUNCTION_POINTERS', 2)
    self.emcc_args += ['-lexports.js']
    self.do_run_in_out_file_test('tests', 'core', 'test_get_exported_function.cpp')

  # Tests the emscripten_get_exported_function() API.
  def test_minimal_runtime_emscripten_get_exported_function(self):
    # Could also test with -s ALLOW_TABLE_GROWTH=1
    self.set_setting('RESERVED_FUNCTION_POINTERS', 2)
    self.emcc_args += ['-lexports.js', '-s', 'MINIMAL_RUNTIME=1']
    self.do_run_in_out_file_test('tests', 'core', 'test_get_exported_function.cpp')

  # Marked as impure since the WASI reactor modules (modules without main)
  # are not yet suppored by the wasm engines we test against.
  @also_with_standalone_wasm(impure=True)
  def test_undefined_main(self):
    if self.get_setting('STANDALONE_WASM'):
      # In standalone we don't support implicitly building without main.  The user has to explicitly
      # opt out (see below).
      err = self.expect_fail([EMCC, path_from_root('tests', 'core', 'test_ctors_no_main.cpp')] + self.get_emcc_args())
      self.assertContained('error: undefined symbol: main (referenced by top-level compiled C/C++ code)', err)
      self.assertContained('warning: To build in STANDALONE_WASM mode without a main(), use emcc --no-entry', err)
    elif not self.get_setting('LLD_REPORT_UNDEFINED') and not self.get_setting('STRICT'):
      # Traditionally in emscripten we allow main to be implicitly undefined.  This allows programs
      # with a main and libraries without a main to be compiled identically.
      # However we are trying to move away from that model to a more explicit opt-out model. See:
      # https://github.com/emscripten-core/emscripten/issues/9640
      self.do_run_in_out_file_test('tests', 'core', 'test_ctors_no_main.cpp')

      # Disabling IGNORE_MISSING_MAIN should cause link to fail due to missing main
      self.set_setting('IGNORE_MISSING_MAIN', 0)
      err = self.expect_fail([EMCC, path_from_root('tests', 'core', 'test_ctors_no_main.cpp')] + self.get_emcc_args())
      self.assertContained('error: entry symbol not defined (pass --no-entry to suppress): main', err)

      # In non-standalone mode exporting an empty list of functions signal that we don't
      # have a main and so should not generate an error.
      self.set_setting('EXPORTED_FUNCTIONS', [])
      self.do_run_in_out_file_test('tests', 'core', 'test_ctors_no_main.cpp')
      self.clear_setting('EXPORTED_FUNCTIONS')

    # If we pass --no-entry or set EXPORTED_FUNCTIONS to empty should never see any errors
    self.emcc_args.append('--no-entry')
    self.do_run_in_out_file_test('tests', 'core', 'test_ctors_no_main.cpp')

  def test_export_start(self):
    if not can_do_standalone(self):
      self.skipTest('standalone mode only')
    self.set_setting('STANDALONE_WASM', 1)
    self.set_setting('EXPORTED_FUNCTIONS', ['__start'])
    self.do_run_in_out_file_test('tests', 'core', 'test_hello_world.c')

  # Tests the operation of API found in #include <emscripten/math.h>
  def test_emscripten_math(self):
    self.do_run_in_out_file_test('tests', 'core', 'test_emscripten_math.c')

  # Tests that users can pass custom JS options from command line using
  # the -jsDfoo=val syntax:
  # See https://github.com/emscripten-core/emscripten/issues/10580.
  def test_custom_js_options(self):
    self.emcc_args += ['--js-library', path_from_root('tests', 'core', 'test_custom_js_settings.js'), '-jsDCUSTOM_JS_OPTION=1']
    self.do_run_in_out_file_test('tests', 'core', 'test_custom_js_settings.c')

    self.assertContained('cannot change built-in settings values with a -jsD directive', self.expect_fail([EMCC, '-jsDWASM=0']))

  # Tests <emscripten/stack.h> API
  @no_asan('stack allocation sizes are no longer predictable')
  def test_emscripten_stack(self):
    self.emcc_args += ['-lstack.js']
    self.set_setting('TOTAL_STACK', 4 * 1024 * 1024)
    self.do_run_in_out_file_test('tests', 'core', 'test_stack_get_free.c')

  # Tests Settings.ABORT_ON_WASM_EXCEPTIONS
  def test_abort_on_exceptions(self):
    self.set_setting('ABORT_ON_WASM_EXCEPTIONS', 1)
    self.emcc_args += ['--bind', '--post-js', 'post.js']
    create_test_file('post.js', '''
      addOnPostRun(function() {
        try {
          // Crash the program
          _crash();
        }
        catch(e) {
          // Catch the abort
          out(true);
        }

        out("again");

        try {
          // Try executing some function again
          _crash();
        }
        catch(e) {
          // Make sure it failed with the expected exception
          out(e === "program has already aborted!");
        }
      });
    ''')
    self.do_run_in_out_file_test('tests', 'core', 'test_abort_on_exception.c')


# Generate tests for everything
def make_run(name, emcc_args, settings=None, env=None):
  if env is None:
    env = {}
  if settings is None:
    settings = {}

  TT = type(name, (TestCoreBase,), dict(run_name=name, env=env, __module__=__name__))  # noqa

  def tearDown(self):
    try:
      super(TT, self).tearDown()
    finally:
      for k, v in self.env.items():
        del os.environ[k]

  TT.tearDown = tearDown

  def setUp(self):
    super(TT, self).setUp()
    for k, v in self.env.items():
      assert k not in os.environ, k + ' should not be in environment'
      os.environ[k] = v

    os.chdir(self.get_dir()) # Ensure the directory exists and go there

    for k, v in settings.items():
      self.set_setting(k, v)

    self.emcc_args += emcc_args
    # avoid various compiler warnings in our test output
    self.emcc_args += [
      '-Wno-dynamic-class-memaccess', '-Wno-format',
      '-Wno-format-extra-args', '-Wno-format-security',
      '-Wno-pointer-bool-conversion', '-Wno-unused-volatile-lvalue',
      '-Wno-c++11-compat-deprecated-writable-strings',
      '-Wno-invalid-pp-token', '-Wno-shift-negative-value'
    ]

  TT.setUp = setUp

  return TT


# Main wasm test modes
wasm0 = make_run('wasm0', emcc_args=['-O0'])
wasm0g = make_run('wasm0g', emcc_args=['-O0', '-g'])
wasm1 = make_run('wasm1', emcc_args=['-O1'])
wasm2 = make_run('wasm2', emcc_args=['-O2'])
wasm2g = make_run('wasm2g', emcc_args=['-O2', '-g'])
wasm3 = make_run('wasm3', emcc_args=['-O3'])
wasms = make_run('wasms', emcc_args=['-Os'])
wasmz = make_run('wasmz', emcc_args=['-Oz'])

wasmlto0 = make_run('wasmlto0', emcc_args=['-flto', '-O0'])
wasmlto1 = make_run('wasmlto1', emcc_args=['-flto', '-O1'])
wasmlto2 = make_run('wasmlto2', emcc_args=['-flto', '-O2'])
wasmlto3 = make_run('wasmlto3', emcc_args=['-flto', '-O3'])
wasmltos = make_run('wasmltos', emcc_args=['-flto', '-Os'])
wasmltoz = make_run('wasmltoz', emcc_args=['-flto', '-Oz'])

wasm2js0 = make_run('wasm2js0', emcc_args=['-O0'], settings={'WASM': 0})
wasm2js1 = make_run('wasm2js1', emcc_args=['-O1'], settings={'WASM': 0})
wasm2js2 = make_run('wasm2js2', emcc_args=['-O2'], settings={'WASM': 0})
wasm2js3 = make_run('wasm2js3', emcc_args=['-O3'], settings={'WASM': 0})
wasm2jss = make_run('wasm2jss', emcc_args=['-Os'], settings={'WASM': 0})
wasm2jsz = make_run('wasm2jsz', emcc_args=['-Oz'], settings={'WASM': 0})

# Secondary test modes - run directly when there is a specific need

# features

simd2 = make_run('simd2', emcc_args=['-O2', '-msimd128'])
bulkmem2 = make_run('bulkmem2', emcc_args=['-O2', '-mbulk-memory'])

# wasm
wasm2s = make_run('wasm2s', emcc_args=['-O2'], settings={'SAFE_HEAP': 1})
wasm2ss = make_run('wasm2ss', emcc_args=['-O2'], settings={'STACK_OVERFLOW_CHECK': 2})
# Add DEFAULT_TO_CXX=0
strict = make_run('strict', emcc_args=[], settings={'STRICT': 1})

lsan = make_run('lsan', emcc_args=['-fsanitize=leak'], settings={'ALLOW_MEMORY_GROWTH': 1})
asan = make_run('asan', emcc_args=['-fsanitize=address'], settings={'ALLOW_MEMORY_GROWTH': 1, 'ASAN_SHADOW_SIZE': 128 * 1024 * 1024})
asani = make_run('asani', emcc_args=['-fsanitize=address', '--pre-js', os.path.join(os.path.dirname(__file__), 'asan-no-leak.js')],
                 settings={'ALLOW_MEMORY_GROWTH': 1})

# Experimental modes (not tested by CI)
lld = make_run('lld', emcc_args=[], settings={'LLD_REPORT_UNDEFINED': 1})
minimal0 = make_run('minimal', emcc_args=['-g'], settings={'MINIMAL_RUNTIME': 1})

# TestCoreBase is just a shape for the specific subclasses, we don't test it itself
del TestCoreBase # noqa
