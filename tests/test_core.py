# coding=utf-8

import glob, hashlib, os, re, shutil, subprocess, sys, json, random
from textwrap import dedent
import tools.shared
from tools.shared import *
from tools.line_endings import check_line_endings
from runner import RunnerCore, path_from_root, checked_sanity, test_modes, get_zlib_library, get_bullet_library

# decorators for limiting which modes a test can run in

def SIMD(f):
  def decorated(self):
    if self.is_emterpreter(): return self.skip('simd not supported in emterpreter yet')
    if self.is_wasm(): return self.skip('wasm will not support SIMD in the MVP')
    self.use_all_engines = True # checks both native in spidermonkey and polyfill in others
    f(self)
  return decorated

def no_emterpreter(f):
  def decorated(self):
    if self.is_emterpreter(): return self.skip('todo')
    f(self)
  return decorated

def no_wasm(f):
  def decorated(self):
    if self.is_wasm(): return self.skip('todo')
    f(self)
  return decorated

class T(RunnerCore): # Short name, to make it more fun to use manually on the commandline
  def is_emterpreter(self):
    return 'EMTERPRETIFY=1' in self.emcc_args
  def is_split_memory(self):
    return 'SPLIT_MEMORY=' in str(self.emcc_args)
  def is_wasm(self):
    return 'BINARYEN' in str(self.emcc_args) or LLVM_TARGET == WASM_TARGET

  def test_hello_world(self):
      test_path = path_from_root('tests', 'core', 'test_hello_world')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

      src = open(self.in_dir('src.cpp.o.js')).read()
      assert 'EMSCRIPTEN_GENERATED_FUNCTIONS' not in src, 'must not emit this unneeded internal thing'

  def test_intvars(self):
      test_path = path_from_root('tests', 'core', 'test_intvars')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_sintvars(self):
      test_path = path_from_root('tests', 'core', 'test_sintvars')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, force_c=True)

  def test_i64(self):
      src = '''
        #include <stdio.h>
        int main()
        {
          long long a = 0x2b00505c10;
          long long b = a >> 29;
          long long c = a >> 32;
          long long d = a >> 34;
          printf("*%lld,%lld,%lld,%lld*\\n", a, b, c, d);
          unsigned long long ua = 0x2b00505c10;
          unsigned long long ub = ua >> 29;
          unsigned long long uc = ua >> 32;
          unsigned long long ud = ua >> 34;
          printf("*%lld,%lld,%lld,%lld*\\n", ua, ub, uc, ud);

          long long x = 0x0000def123450789ULL; // any bigger than this, and we
          long long y = 0x00020ef123456089ULL; // start to run into the double precision limit!
          printf("*%lld,%lld,%lld,%lld,%lld*\\n", x, y, x | y, x & y, x ^ y);

          printf("*");
          long long z = 13;
          int n = 0;
          while (z > 1) {
            printf("%.2f,", (float)z); // these must be integers!
            z = z >> 1;
            n++;
          }
          printf("*%d*\\n", n);
          return 0;
        }
      '''
      self.do_run(src, '*184688860176,344,43,10*\n*184688860176,344,43,10*\n*245127260211081,579378795077769,808077213656969,16428841631881,791648372025088*\n*13.00,6.00,3.00,*3*')

      src = r'''
        #include <time.h>
        #include <stdio.h>
        #include <stdint.h>

        int64_t returner1() { return 0x0000def123450789ULL; }
        int64_t returner2(int test) {
          while (test > 10) test /= 2; // confuse the compiler so it doesn't eliminate this function
          return test > 5 ? 0x0000def123450123ULL : 0ULL;
        }

        void modifier1(int64_t t) {
          t |= 12;
          printf("m1: %lld\n", t);
        }
        void modifier2(int64_t &t) {
          t |= 12;
        }

        int truthy() {
          int x = time(0);
          while (x > 10) {
            x |= 7;
            x /= 2;
          }
          return x < 3;
        }

        struct IUB {
           int c;
           long long d;
        };

        IUB iub[] = {
           { 55, 17179869201 },
           { 122, 25769803837 },
        };

        int main(int argc, char **argv)
        {
          int64_t x1 = 0x1234def123450789ULL;
          int64_t x2 = 0x1234def123450788ULL;
          int64_t x3 = 0x1234def123450789ULL;
          printf("*%lld\n%d,%d,%d,%d,%d\n%d,%d,%d,%d,%d*\n", x1, x1==x2, x1<x2, x1<=x2, x1>x2, x1>=x2, // note: some rounding in the printing!
                                                                x1==x3, x1<x3, x1<=x3, x1>x3, x1>=x3);
          printf("*%lld*\n", returner1());
          printf("*%lld*\n", returner2(30));

          uint64_t maxx = -1ULL;
          printf("*%llu*\n*%llu*\n", maxx, maxx >> 5);

          // Make sure params are not modified if they shouldn't be
          int64_t t = 123;
          modifier1(t);
          printf("*%lld*\n", t);
          modifier2(t);
          printf("*%lld*\n", t);

          // global structs with i64s
          printf("*%d,%lld*\n*%d,%lld*\n", iub[0].c, iub[0].d, iub[1].c, iub[1].d);

          // Bitshifts
          {
            int64_t a = -1;
            int64_t b = a >> 29;
            int64_t c = a >> 32;
            int64_t d = a >> 34;
            printf("*%lld,%lld,%lld,%lld*\n", a, b, c, d);
            uint64_t ua = -1;
            int64_t ub = ua >> 29;
            int64_t uc = ua >> 32;
            int64_t ud = ua >> 34;
            printf("*%lld,%lld,%lld,%lld*\n", ua, ub, uc, ud);
          }

          // Nonconstant bitshifts
          {
            int64_t a = -1;
            int64_t b = a >> (29 - argc + 1);
            int64_t c = a >> (32 - argc + 1);
            int64_t d = a >> (34 - argc + 1);
            printf("*%lld,%lld,%lld,%lld*\n", a, b, c, d);
            uint64_t ua = -1;
            int64_t ub = ua >> (29 - argc + 1);
            int64_t uc = ua >> (32 - argc + 1);
            int64_t ud = ua >> (34 - argc + 1);
            printf("*%lld,%lld,%lld,%lld*\n", ua, ub, uc, ud);
          }

          // Math mixtures with doubles
          {
            uint64_t a = 5;
            double b = 6.8;
            uint64_t c = a * b;
            if (truthy()) printf("*%d,%d,%d*\n", (int)&a, (int)&b, (int)&c); // printing addresses prevents optimizations
            printf("*prod:%llu*\n", c);
          }

          // Basic (rounded, for now) math. Just check compilation.
          int64_t a = 0x1234def123450789ULL;
          a--; if (truthy()) a--; // confuse optimizer
          int64_t b = 0x1234000000450789ULL;
          b++; if (truthy()) b--; // confuse optimizer
          printf("*%lld,%lld,%lld,%lld*\n",   (a+b)/5000, (a-b)/5000, (a*3)/5000, (a/5)/5000);

          a -= 17; if (truthy()) a += 5; // confuse optimizer
          b -= 17; if (truthy()) b += 121; // confuse optimizer
          printf("*%llx,%llx,%llx,%llx*\n", b - a, b - a/2, b/2 - a, b - 20);

          if (truthy()) a += 5/b; // confuse optimizer
          if (truthy()) b += 121*(3+a/b); // confuse optimizer
          printf("*%llx,%llx,%llx,%llx*\n", a - b, a - b/2, a/2 - b, a - 20);

          return 0;
        }
      '''
      self.do_run(src, '*1311918518731868041\n' +
                       '0,0,0,1,1\n' +
                       '1,0,1,0,1*\n' +
                       '*245127260211081*\n' +
                       '*245127260209443*\n' +
                       '*18446744073709551615*\n' +
                       '*576460752303423487*\n' +
                       'm1: 127\n' +
                       '*123*\n' +
                       '*127*\n' +
                       '*55,17179869201*\n' +
                       '*122,25769803837*\n' +
                       '*-1,-1,-1,-1*\n' +
                       '*-1,34359738367,4294967295,1073741823*\n' +
                       '*-1,-1,-1,-1*\n' +
                       '*-1,34359738367,4294967295,1073741823*\n' +
                       '*prod:34*\n' +
                       '*524718382041609,49025451137,787151111239120,52476740749274*\n' +
                       '*ffff210edd000002,91990876ea283be,f6e5210edcdd7c45,1234000000450765*\n' +
                       '*def122fffffe,91adef1232283bb,f6e66f78915d7c42,1234def123450763*\n')

      src = r'''
        #include <stdio.h>
        #include <limits>

        int main()
        {
            long long i,j,k;

            i = 0;
            j = -1,
            k = 1;

            printf( "*\n" );
            printf( "%s\n", i > j ? "Ok": "Fail" );
            printf( "%s\n", k > i ? "Ok": "Fail" );
            printf( "%s\n", k > j ? "Ok": "Fail" );
            printf( "%s\n", i < j ? "Fail": "Ok" );
            printf( "%s\n", k < i ? "Fail": "Ok" );
            printf( "%s\n", k < j ? "Fail": "Ok" );
            printf( "%s\n", (i-j) >= k ? "Ok": "Fail" );
            printf( "%s\n", (i-j) <= k ? "Ok": "Fail" );
            printf( "%s\n", i > std::numeric_limits<long long>::min() ? "Ok": "Fail" );
            printf( "%s\n", i < std::numeric_limits<long long>::max() ? "Ok": "Fail" );
            printf( "*\n" );
        }
      '''

      self.do_run(src, '*\nOk\nOk\nOk\nOk\nOk\nOk\nOk\nOk\nOk\nOk\n*')

      # stuff that also needs sign corrections

      src = r'''
        #include <stdio.h>
        #include <stdint.h>

        int main()
        {
          // i32 vs i64
          int32_t small = -1;
          int64_t large = -1;
          printf("*%d*\n", small == large);
          small++;
          printf("*%d*\n", small == large);
          uint32_t usmall = -1;
          uint64_t ularge = -1;
          printf("*%d*\n", usmall == ularge);
          return 0;
        }
      '''

      self.do_run(src, '*1*\n*0*\n*0*\n')

  def test_i64_b(self):
      test_path = path_from_root('tests', 'core', 'test_i64_b')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_cmp(self):
      test_path = path_from_root('tests', 'core', 'test_i64_cmp')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_cmp2(self):
      test_path = path_from_root('tests', 'core', 'test_i64_cmp2')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_double(self):
      test_path = path_from_root('tests', 'core', 'test_i64_double')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_umul(self):
      test_path = path_from_root('tests', 'core', 'test_i64_umul')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_precise(self):
      src = r'''
        #include <inttypes.h>
        #include <stdio.h>

        int main() {
          uint64_t x = 0, y = 0;
          for (int i = 0; i < 64; i++) {
            x += 1ULL << i;
            y += x;
            x /= 3;
            y *= 5;
            printf("unsigned %d: %llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", i, x, y, x+y, x-y, x*y, y ? x/y : 0, x ? y/x : 0, y ? x%y : 0, x ? y%x : 0);
          }
          int64_t x2 = 0, y2 = 0;
          for (int i = 0; i < 64; i++) {
            x2 += 1LL << i;
            y2 += x2;
            x2 /= 3 * (i % 7 ? -1 : 1);
            y2 *= 5 * (i % 2 ? -1 : 1);
            printf("signed %d: %lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", i, x2, y2, x2+y2, x2-y2, x2*y2, y2 ? x2/y2 : 0, x2 ? y2/x2 : 0, y2 ? x2%y2 : 0, x2 ? y2%x2 : 0);
          }
          return 0;
        }
      '''
      self.do_run(src, open(path_from_root('tests', 'i64_precise.txt')).read())

      # Verify that even if we ask for precision, if it is not needed it is not included
      Settings.PRECISE_I64_MATH = 1
      src = '''
        #include <inttypes.h>
        #include <stdio.h>

        int main(int argc, char **argv) {
          uint64_t x = 2125299906845564, y = 1225891506842664;
          if (argc == 12) {
            x = x >> 1;
            y = y >> 1;
          }
          x = x & 12ULL;
          y = y | 12ULL;
          x = x ^ y;
          x <<= 2;
          y >>= 3;
          printf("*%llu, %llu*\\n", x, y);
        }
      '''
      self.do_run(src, '*4903566027370624, 153236438355333*')

      code = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
      assert 'goog.math.Long' not in code, 'i64 precise math should never be included, musl does its own printfing'

      # and now one where we do
      self.do_run(r'''
        #include <stdio.h>

        int main( int argc, char ** argv )
        {
            unsigned long a = 0x60DD1695U;
            unsigned long b = 0xCA8C4E7BU;
            unsigned long long c = (unsigned long long)a * b;
            printf( "c = %016llx\n", c );

            return 0;
        }
      ''', 'c = 4ca38a6bd2973f97')

  def test_i64_llabs(self):
    Settings.PRECISE_I64_MATH = 2

    test_path = path_from_root('tests', 'core', 'test_i64_llabs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_zextneg(self):
    test_path = path_from_root('tests', 'core', 'test_i64_zextneg')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_7z(self):
    test_path = path_from_root('tests', 'core', 'test_i64_7z')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, ['hallo'])

  def test_i64_i16(self):
    test_path = path_from_root('tests', 'core', 'test_i64_i16')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_qdouble(self):
    test_path = path_from_root('tests', 'core', 'test_i64_qdouble')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_varargs(self):
    test_path = path_from_root('tests', 'core', 'test_i64_varargs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, 'waka fleefl asdfasdfasdfasdf'.split(' '))

  def test_llvm_fabs(self):
    Settings.PRECISE_F32 = 1
    test_path = path_from_root('tests', 'core', 'test_llvm_fabs')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_double_varargs(self):
    test_path = path_from_root('tests', 'core', 'test_double_varargs')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_struct_varargs(self):
    test_path = path_from_root('tests', 'core', 'test_struct_varargs')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def zzztest_nested_struct_varargs(self):
    test_path = path_from_root('tests', 'core', 'test_nested_struct_varargs')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_i32_mul_precise(self):
    test_path = path_from_root('tests', 'core', 'test_i32_mul_precise')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i16_emcc_intrinsic(self):
    test_path = path_from_root('tests', 'core', 'test_i16_emcc_intrinsic')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_double_i64_conversion(self):
    test_path = path_from_root('tests', 'core', 'test_double_i64_conversion')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_float32_precise(self):
    Settings.PRECISE_F32 = 1
    test_path = path_from_root('tests', 'core', 'test_float32_precise')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_negative_zero(self):
    test_path = path_from_root('tests', 'core', 'test_negative_zero')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_line_endings(self):
    self.build(open(path_from_root('tests', 'hello_world.cpp')).read(), self.get_dir(), self.in_dir('hello_world.cpp'))

  def test_literal_negative_zero(self):
    test_path = path_from_root('tests', 'core', 'test_literal_negative_zero')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_llvm_intrinsics(self):
    Settings.PRECISE_I64_MATH = 2 # for bswap64

    test_path = path_from_root('tests', 'core', 'test_llvm_intrinsics')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_bswap64(self):
    test_path = path_from_root('tests', 'core', 'test_bswap64')
    src, output = (test_path + s for s in ('.in', '.out'))

    # extra coverages
    for emulate_casts in [0, 1]:
      for emulate_fps in [0, 1, 2]:
        print emulate_casts, emulate_fps
        Settings.EMULATE_FUNCTION_POINTER_CASTS = emulate_casts
        Settings.EMULATED_FUNCTION_POINTERS = emulate_fps
        self.do_run_from_file(src, output)

  def test_sha1(self):
    self.do_run(open(path_from_root('tests', 'sha1.c')).read(), 'SHA1=15dd99a1991e0b3826fede3deffc1feba42278e6')

  def test_asmjs_unknown_emscripten(self):
    # No other configuration is supported, so always run this.
    self.do_run(open(path_from_root('tests', 'asmjs-unknown-emscripten.c')).read(), '')

  def test_cube2md5(self):
    self.emcc_args += ['--embed-file', 'cube2md5.txt']
    shutil.copyfile(path_from_root('tests', 'cube2md5.txt'), os.path.join(self.get_dir(), 'cube2md5.txt'))
    self.do_run(open(path_from_root('tests', 'cube2md5.cpp')).read(), open(path_from_root('tests', 'cube2md5.ok')).read())

  def test_cube2hash(self):
    # A good test of i64 math
    self.do_run('', 'Usage: hashstring <seed>',
                libraries=self.get_library('cube2hash', ['cube2hash.bc'], configure=None),
                includes=[path_from_root('tests', 'cube2hash')])

    for text, output in [('fleefl', '892BDB6FD3F62E863D63DA55851700FDE3ACF30204798CE9'),
                         ('fleefl2', 'AA2CC5F96FC9D540CA24FDAF1F71E2942753DB83E8A81B61'),
                         ('64bitisslow', '64D8470573635EC354FEE7B7F87C566FCAF1EFB491041670')]:
      self.do_run('', 'hash value: ' + output, [text], no_build=True)

  def test_unaligned(self):
      return self.skip('LLVM marks the reads of s as fully aligned, making this test invalid')
      src = r'''
        #include<stdio.h>

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
      except Exception, e:
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

  def test_align_moar(self):
    self.emcc_args = self.emcc_args + ['-msse']
    def test():
      self.do_run(r'''
#include <xmmintrin.h>
#include <stdio.h>

struct __attribute__((aligned(16))) float4x4
{
    union
    {
        float v[4][4];
        __m128 row[4];
    };
};

float4x4 v;
__m128 m;

int main()
{
    printf("Alignment: %d addr: 0x%x\n", ((int)&v) % 16, (int)&v);
    printf("Alignment: %d addr: 0x%x\n", ((int)&m) % 16, (int)&m);
}
    ''', ('Alignment: 0 addr: 0xa20\nAlignment: 0 addr: 0xa60\n', 'Alignment: 0 addr: 0xe10\nAlignment: 0 addr: 0xe50\n')) # hardcoded addresses, for 2 common global_base values. this tracks if this ever changes by surprise. will need normal updates.

    test()
    print 'relocatable'
    Settings.RELOCATABLE = 1
    test()

  def test_unsigned(self):
      src = '''
        #include <stdio.h>
        const signed char cvals[2] = { -1, -2 }; // compiler can store this is a string, so -1 becomes \FF, and needs re-signing
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
      test_path = path_from_root('tests', 'core', 'test_bitfields')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_floatvars(self):
    test_path = path_from_root('tests', 'core', 'test_floatvars')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_closebitcasts(self):
    test_path = path_from_root('tests', 'core', 'closebitcasts')
    src, output = (test_path + s for s in ('.c', '.txt'))
    self.do_run_from_file(src, output)

  def test_fast_math(self):
    Building.COMPILER_TEST_OPTS += ['-ffast-math']

    test_path = path_from_root('tests', 'core', 'test_fast_math')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, ['5', '6', '8'])

  def test_zerodiv(self):
    test_path = path_from_root('tests', 'core', 'test_zerodiv')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_zero_multiplication(self):
    test_path = path_from_root('tests', 'core', 'test_zero_multiplication')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_isnan(self):
    test_path = path_from_root('tests', 'core', 'test_isnan')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_globaldoubles(self):
    test_path = path_from_root('tests', 'core', 'test_globaldoubles')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_math(self):
      test_path = path_from_root('tests', 'core', 'test_math')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_erf(self):
      test_path = path_from_root('tests', 'core', 'test_erf')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_math_hyperbolic(self):
      src = open(path_from_root('tests', 'hyperbolic', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'hyperbolic', 'output.txt'), 'r').read()
      self.do_run(src, expected)

  def test_math_lgamma(self):
      test_path = path_from_root('tests', 'math', 'lgamma')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

      if Settings.ALLOW_MEMORY_GROWTH == 0 and not self.is_wasm():
        print 'main module'
        Settings.MAIN_MODULE = 1
        self.do_run_from_file(src, output)

  def test_frexp(self):
      test_path = path_from_root('tests', 'core', 'test_frexp')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_rounding(self):
      for precise_f32 in [0, 1]:
        print precise_f32
        Settings.PRECISE_F32 = precise_f32

        test_path = path_from_root('tests', 'core', 'test_rounding')
        src, output = (test_path + s for s in ('.in', '.out'))

        self.do_run_from_file(src, output)

  def test_fcvt(self):
      test_path = path_from_root('tests', 'core', 'test_fcvt')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_llrint(self):
    test_path = path_from_root('tests', 'core', 'test_llrint')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_getgep(self):
    # Generated code includes getelementptr (getelementptr, 0, 1), i.e., GEP as the first param to GEP
    test_path = path_from_root('tests', 'core', 'test_getgep')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_multiply_defined_symbols(self):
    a1 = "int f() { return 1; }"
    a1_name = os.path.join(self.get_dir(), 'a1.c')
    open(a1_name, 'w').write(a1)
    a2 = "void x() {}"
    a2_name = os.path.join(self.get_dir(), 'a2.c')
    open(a2_name, 'w').write(a2)
    b1 = "int f() { return 2; }"
    b1_name = os.path.join(self.get_dir(), 'b1.c')
    open(b1_name, 'w').write(b1)
    b2 = "void y() {}"
    b2_name = os.path.join(self.get_dir(), 'b2.c')
    open(b2_name, 'w').write(b2)
    main = r'''
      #include <stdio.h>
      int f();
      int main() {
        printf("result: %d\n", f());
        return 0;
      }
    '''
    main_name = os.path.join(self.get_dir(), 'main.c')
    open(main_name, 'w').write(main)

    Building.emcc(a1_name)
    Building.emcc(a2_name)
    Building.emcc(b1_name)
    Building.emcc(b2_name)
    Building.emcc(main_name)

    liba_name = os.path.join(self.get_dir(), 'liba.a')
    Building.emar('cr', liba_name, [a1_name + '.o', a2_name + '.o'])
    libb_name = os.path.join(self.get_dir(), 'libb.a')
    Building.emar('cr', libb_name, [b1_name + '.o', b2_name + '.o'])

    all_name = os.path.join(self.get_dir(), 'all.bc')
    Building.link([main_name + '.o', liba_name, libb_name], all_name)

    self.do_ll_run(all_name, 'result: 1')

  def test_if(self):
    test_path = path_from_root('tests', 'core', 'test_if')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_if_else(self):
    test_path = path_from_root('tests', 'core', 'test_if_else')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_loop(self):
    test_path = path_from_root('tests', 'core', 'test_loop')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack(self):
    Settings.INLINING_LIMIT = 50

    test_path = path_from_root('tests', 'core', 'test_stack')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack_align(self):
    Settings.INLINING_LIMIT = 50
    src = path_from_root('tests', 'core', 'test_stack_align.cpp')
    def test():
      self.do_run(open(src).read(), ['''align 4: 0
align 8: 0
align 16: 0
align 32: 0
base align: 0, 0, 0, 0'''])
    test()
    if '-O' in str(self.emcc_args):
      print 'outlining'
      Settings.OUTLINING_LIMIT = 60
      test()

  @no_emterpreter
  def test_stack_restore(self):
    if self.is_wasm(): return self.skip('generated code not available in wasm')
    self.emcc_args += ['-g3'] # to be able to find the generated code
    test_path = path_from_root('tests', 'core', 'test_stack_restore')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

    generated = open('src.cpp.o.js').read()

    def ensure_stack_restore_count(function_name, expected_count):
      code = generated[generated.find(function_name):]
      code = code[:code.find('\n}') + 2]
      actual_count = code.count('STACKTOP = sp')
      assert actual_count == expected_count, ('Expected %d stack restorations, got %d' % (expected_count, actual_count)) + ': ' + code

    ensure_stack_restore_count('function _no_stack_usage', 0)
    ensure_stack_restore_count('function _alloca_gets_restored', 1)
    ensure_stack_restore_count('function _stack_usage', 1)

  def test_strings(self):
      test_path = path_from_root('tests', 'core', 'test_strings')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, ['wowie', 'too', '74'])

      if self.emcc_args == []:
        gen = open(self.in_dir('src.cpp.o.js')).read()
        assert ('var __str1;' in gen) == named

  def test_strcmp_uni(self):
    test_path = path_from_root('tests', 'core', 'test_strcmp_uni')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strndup(self):
    test_path = path_from_root('tests', 'core', 'test_strndup')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_errar(self):
    test_path = path_from_root('tests', 'core', 'test_errar')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_mainenv(self):
    test_path = path_from_root('tests', 'core', 'test_mainenv')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_funcs(self):
    test_path = path_from_root('tests', 'core', 'test_funcs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_structs(self):
    test_path = path_from_root('tests', 'core', 'test_structs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

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

  def test_newstruct(self):
      self.do_run(self.gen_struct_src.replace('{{gen_struct}}', 'new S').replace('{{del_struct}}', 'delete'), '*51,62*')

  def test_addr_of_stacked(self):
    test_path = path_from_root('tests', 'core', 'test_addr_of_stacked')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_globals(self):
    test_path = path_from_root('tests', 'core', 'test_globals')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_linked_list(self):
    test_path = path_from_root('tests', 'core', 'test_linked_list')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

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

  def test_assert(self):
    test_path = path_from_root('tests', 'core', 'test_assert')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_libcextra(self):
      test_path = path_from_root('tests', 'core', 'test_libcextra')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_regex(self):
      test_path = path_from_root('tests', 'core', 'test_regex')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_longjmp(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp2(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp2')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp3(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp3')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp4(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp4')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp_funcptr(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp_funcptr')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp_repeat(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp_repeat')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp_stacked(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp_stacked')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp_exc(self):
    test_path = path_from_root('tests', 'core', 'test_longjmp_exc')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_longjmp_throw(self):
    for disable_throw in [0, 1]:
      print disable_throw
      Settings.DISABLE_EXCEPTION_CATCHING = disable_throw
      test_path = path_from_root('tests', 'core', 'test_longjmp_throw')
      src, output = (test_path + s for s in ('.cpp', '.out'))
      self.do_run_from_file(src, output)

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
      print num
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
  assert(m1.arena == m2.arena && m1.uordblks == m2.uordblks);
  printf("ok.\n");
}
'''

    self.do_run(src, r'''ok.''')

  def test_exceptions(self):
      Settings.EXCEPTION_DEBUG = 1

      Settings.DISABLE_EXCEPTION_CATCHING = 0
      if '-O2' in self.emcc_args and not self.is_wasm():
        self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

      src = '''
        #include <stdio.h>
        void thrower() {
          printf("infunc...");
          throw(99);
          printf("FAIL");
        }
        int main() {
          try {
            printf("*throw...");
            throw(1);
            printf("FAIL");
          } catch(...) {
            printf("caught!");
          }
          try {
            thrower();
          } catch(...) {
            printf("done!*\\n");
          }
          return 0;
        }
      '''
      self.do_run(src, '*throw...caught!infunc...done!*')

      Settings.DISABLE_EXCEPTION_CATCHING = 1
      self.do_run(src, 'Exception catching is disabled, this exception cannot be caught. Compile with -s DISABLE_EXCEPTION_CATCHING=0')

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

      Settings.DISABLE_EXCEPTION_CATCHING = 0
      self.do_run(src, 'Throw...Construct...Caught...Destruct...Throw...Construct...Copy...Caught...Destruct...Destruct...')

  def test_exceptions_2(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0

    for safe in [0,1]:
      print safe
      Settings.SAFE_HEAP = safe
      test_path = path_from_root('tests', 'core', 'test_exceptions_2')
      src, output = (test_path + s for s in ('.in', '.out'))
      self.do_run_from_file(src, output)

  def test_exceptions_3(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0

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

    print '0'
    self.do_run(src, 'Caught C string: a c string\nDone.', ['0'])
    print '1'
    self.do_run(src, 'Caught exception: std::exception\nDone.', ['1'], no_build=True)
    print '2'
    self.do_run(src, 'Caught exception: Hello\nDone.', ['2'], no_build=True)

  def test_exceptions_white_list(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 2
    Settings.EXCEPTION_CATCHING_WHITELIST = ["__Z12somefunctionv"]
    Settings.INLINING_LIMIT = 50 # otherwise it is inlined and not identified

    test_path = path_from_root('tests', 'core', 'test_exceptions_white_list')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)
    size = len(open('src.cpp.o.js').read())
    shutil.copyfile('src.cpp.o.js', 'orig.js')

    # check that an empty whitelist works properly (as in, same as exceptions disabled)
    empty_output = path_from_root('tests', 'core', 'test_exceptions_white_list_empty.out')

    Settings.EXCEPTION_CATCHING_WHITELIST = []
    self.do_run_from_file(src, empty_output)
    empty_size = len(open('src.cpp.o.js').read())
    shutil.copyfile('src.cpp.o.js', 'empty.js')

    Settings.EXCEPTION_CATCHING_WHITELIST = ['fake']
    self.do_run_from_file(src, empty_output)
    fake_size = len(open('src.cpp.o.js').read())
    shutil.copyfile('src.cpp.o.js', 'fake.js')

    Settings.DISABLE_EXCEPTION_CATCHING = 1
    self.do_run_from_file(src, empty_output)
    disabled_size = len(open('src.cpp.o.js').read())
    shutil.copyfile('src.cpp.o.js', 'disabled.js')

    if not self.is_wasm():
      print size, empty_size, fake_size, disabled_size

      assert size - empty_size > 0.0025*size, [empty_size, size] # big change when we disable entirely
      assert size - fake_size > 0.0025*size, [fake_size, size]
      assert abs(empty_size - fake_size) < 0.007*size, [empty_size, fake_size]
      assert empty_size - disabled_size < 0.007*size, [empty_size, disabled_size] # full disable removes a little bit more
      assert fake_size - disabled_size < 0.007*size, [disabled_size, fake_size]

  def test_exceptions_white_list_2(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 2
    Settings.EXCEPTION_CATCHING_WHITELIST = ["_main"]
    Settings.INLINING_LIMIT = 50 # otherwise it is inlined and not identified

    test_path = path_from_root('tests', 'core', 'test_exceptions_white_list_2')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_exceptions_uncaught(self):
      Settings.DISABLE_EXCEPTION_CATCHING = 0

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

  def test_exceptions_uncaught_2(self):
      Settings.DISABLE_EXCEPTION_CATCHING = 0

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

  def test_exceptions_typed(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    self.emcc_args += ['-s', 'SAFE_HEAP=0'] # Throwing null will cause an ignorable null pointer access.

    test_path = path_from_root('tests', 'core', 'test_exceptions_typed')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_exceptions_virtual_inheritance(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0

    test_path = path_from_root('tests', 'core', 'test_exceptions_virtual_inheritance')
    src, output = (test_path + s for s in ('.cpp', '.txt'))

    self.do_run_from_file(src, output)

  def test_exceptions_convert(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_convert')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_multi(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_multi')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_exceptions_std(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    Settings.ERROR_ON_UNDEFINED_SYMBOLS = 1
    self.emcc_args += ['-s', 'SAFE_HEAP=0']

    test_path = path_from_root('tests', 'core', 'test_exceptions_std')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_exceptions_alias(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_alias')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_exceptions_rethrow(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_rethrow')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_resume(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    Settings.EXCEPTION_DEBUG = 1
    test_path = path_from_root('tests', 'core', 'test_exceptions_resume')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_destroy_virtual(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_destroy_virtual')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_refcount(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_refcount')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_primary(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_primary')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_simplify_cfg(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_simplify_cfg')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_exceptions_libcxx(self):
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    test_path = path_from_root('tests', 'core', 'test_exceptions_libcxx')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.do_run_from_file(src, output)

  def test_bad_typeid(self):
    Settings.ERROR_ON_UNDEFINED_SYMBOLS = 1
    Settings.DISABLE_EXCEPTION_CATCHING = 0

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

  def test_iostream_ctors(self): # iostream stuff must be globally constructed before user global constructors, so iostream works in global constructors
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
''', "bugfree code")

  def test_class(self):
    test_path = path_from_root('tests', 'core', 'test_class')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_inherit(self):
    test_path = path_from_root('tests', 'core', 'test_inherit')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_isdigit_l(self):
      test_path = path_from_root('tests', 'core', 'test_isdigit_l')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_iswdigit(self):
      test_path = path_from_root('tests', 'core', 'test_iswdigit')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_polymorph(self):
      test_path = path_from_root('tests', 'core', 'test_polymorph')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_complex(self):
    self.do_run(r'''
#include <complex.h>
#include <stdio.h>

int main(int argc, char**argv)
{
   float complex z1 = 1.0 + 3.0 * I;
   printf("value = real %.2f imag %.2f\n",creal(z1),cimag(z1));
   float abs_value = cabsf(z1);
   printf("abs = %.2f\n",abs_value);
   float complex z2 =  conjf(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z2),cimag(z2));
   float complex z3 =  cexpf(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z3),cimag(z3));
   float complex z4 =  conj(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z4),cimag(z4));
   float complex z5 =  cargf(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z5),cimag(z5));
   float complex z6 = 0.5 + 0.5 * I;
   float complex z7 = 0.5 - 0.5 * I;
   float complex z8 = z6 * z7;
   float complex z9 = z6 / z7;
   printf("value = real %.2f imag %.2f\n",creal(z8),cimag(z8));
   printf("value = real %.2f imag %.2f\n",creal(z9),cimag(z9));
   return 0;
}
''', '''value = real 1.00 imag 3.00
abs = 3.16
value = real 1.00 imag -3.00
value = real -2.69 imag 0.38
value = real 1.00 imag -3.00
value = real 1.25 imag 0.00
value = real 0.50 imag 0.00
value = real 0.00 imag 1.00''', force_c=True)

  def test_segfault(self):
    Settings.SAFE_HEAP = 1

    for addr in ['0', 'new D2()']:
      print addr
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
      self.do_run(src, 'segmentation fault' if addr.isdigit() else 'marfoosh')

  def test_dynamic_cast(self):
      test_path = path_from_root('tests', 'core', 'test_dynamic_cast')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_dynamic_cast_b(self):
      test_path = path_from_root('tests', 'core', 'test_dynamic_cast_b')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_dynamic_cast_2(self):
    test_path = path_from_root('tests', 'core', 'test_dynamic_cast_2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_funcptr(self):
    test_path = path_from_root('tests', 'core', 'test_funcptr')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_mathfuncptr(self):
    test_path = path_from_root('tests', 'core', 'test_mathfuncptr')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

    if self.is_emterpreter():
      print 'emterpreter f32'
      Settings.PRECISE_F32 = 1
      self.do_run_from_file(src, output)

  def test_funcptrfunc(self):
    test_path = path_from_root('tests', 'core', 'test_funcptrfunc')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_funcptr_namecollide(self):
    test_path = path_from_root('tests', 'core', 'test_funcptr_namecollide')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, force_c=True)

  def test_emptyclass(self):
      test_path = path_from_root('tests', 'core', 'test_emptyclass')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_alloca(self):
    test_path = path_from_root('tests', 'core', 'test_alloca')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, force_c=True)

  def test_rename(self):
    src = open(path_from_root('tests', 'stdio', 'test_rename.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_alloca_stack(self):
    test_path = path_from_root('tests', 'core', 'test_alloca_stack')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, force_c=True)

  def test_stack_byval(self):
    test_path = path_from_root('tests', 'core', 'test_stack_byval')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack_varargs(self):
    Settings.INLINING_LIMIT = 50
    Settings.TOTAL_STACK = 2048

    test_path = path_from_root('tests', 'core', 'test_stack_varargs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack_varargs2(self):
    Settings.TOTAL_STACK = 1536
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      void func(int i) {
      }
      int main() {
        for (int i = 0; i < 1024; i++) {
          printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                   i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
        }
        printf("ok!\n");
        return 0;
      }
    '''
    self.do_run(src, 'ok!')

    print 'with return'

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main() {
        for (int i = 0; i < 1024; i++) {
          int j = printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                   i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
          printf(" (%d)\n", j);
        }
        printf("ok!\n");
        return 0;
      }
    '''
    self.do_run(src, 'ok!')

    print 'with definitely no return'

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
        for (int i = 0; i < 1024; i++) {
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
    Settings.INLINING_LIMIT = 50

    test_path = path_from_root('tests', 'core', 'test_stack_void')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_life(self):
    self.emcc_args += ['-std=c99']
    src = open(path_from_root('tests', 'life.c'), 'r').read()
    self.do_run(src, '''--------------------------------
[]                                    []                  [][][]
                    []  []    []    [][]  []            []  []  
[]                [][]  [][]              [][][]      []        
                  []    []      []      []  [][]    []        []
                  []  [][]    []        []    []  []    [][][][]
                    [][]      [][]  []    [][][]  []        []  
                                []  [][]  [][]    [][]  [][][]  
                                    [][]          [][][]  []  []
                                    [][]              [][]    []
                                                          [][][]
                                                            []  
                                                                
                                                                
                                                                
                                                                
                                        [][][]                  
                                      []      [][]      [][]    
                                      [][]      []  [][]  [][]  
                                                    [][]  [][]  
                                                      []        
                  [][]                                          
                  [][]                                        []
[]                                                      [][]  []
                                                  [][][]      []
                                                []      [][]    
[]                                                    []      []
                                                          []    
[]                                                        []  []
                                              [][][]            
                                                                
                                  []                            
                              [][][]                          []
--------------------------------
''', ['2'], force_c=True)

  def test_array2(self):
    test_path = path_from_root('tests', 'core', 'test_array2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_array2b(self):
    test_path = path_from_root('tests', 'core', 'test_array2b')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_constglobalstructs(self):
    test_path = path_from_root('tests', 'core', 'test_constglobalstructs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_conststructs(self):
    test_path = path_from_root('tests', 'core', 'test_conststructs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_bigarray(self):
    test_path = path_from_root('tests', 'core', 'test_bigarray')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_mod_globalstruct(self):
    test_path = path_from_root('tests', 'core', 'test_mod_globalstruct')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_pystruct(self):
      src = '''
        #include <stdio.h>

        // Based on CPython code
        union PyGC_Head {
            struct {
                union PyGC_Head *gc_next;
                union PyGC_Head *gc_prev;
                size_t gc_refs;
            } gc;
            long double dummy;  /* force worst-case alignment */
        } ;

        struct gc_generation {
            PyGC_Head head;
            int threshold; /* collection threshold */
            int count; /* count of allocations or collections of younger
                          generations */
        };

        #define NUM_GENERATIONS 3
        #define GEN_HEAD(n) (&generations[n].head)

        /* linked lists of container objects */
        static struct gc_generation generations[NUM_GENERATIONS] = {
            /* PyGC_Head,                               threshold,      count */
            {{{GEN_HEAD(0), GEN_HEAD(0), 0}},           700,            0},
            {{{GEN_HEAD(1), GEN_HEAD(1), 0}},           10,             0},
            {{{GEN_HEAD(2), GEN_HEAD(2), 0}},           10,             0},
        };

        int main()
        {
          gc_generation *n = NULL;
          printf("*%d,%d,%d,%d,%d,%d,%d,%d*\\n",
            (int)(&n[0]),
            (int)(&n[0].head),
            (int)(&n[0].head.gc.gc_next),
            (int)(&n[0].head.gc.gc_prev),
            (int)(&n[0].head.gc.gc_refs),
            (int)(&n[0].threshold), (int)(&n[0].count), (int)(&n[1])
          );
          printf("*%d,%d,%d*\\n",
            (int)(&generations[0]) ==
            (int)(&generations[0].head.gc.gc_next),
            (int)(&generations[0]) ==
            (int)(&generations[0].head.gc.gc_prev),
            (int)(&generations[0]) ==
            (int)(&generations[1])
          );
          int x1 = (int)(&generations[0]);
          int x2 = (int)(&generations[1]);
          printf("*%d*\\n", x1 == x2);
          for (int i = 0; i < NUM_GENERATIONS; i++) {
            PyGC_Head *list = GEN_HEAD(i);
            printf("%d:%d,%d\\n", i, (int)list == (int)(list->gc.gc_prev), (int)list ==(int)(list->gc.gc_next));
          }
          printf("*%d,%d,%d*\\n", sizeof(PyGC_Head), sizeof(gc_generation), int(GEN_HEAD(2)) - int(GEN_HEAD(1)));
        }
      '''
      def test():
        self.do_run(src, '*0,0,0,4,8,16,20,24*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*16,24,24*')

      test()

      print 'relocatable' # this tests recursive global structs => nontrivial postSets for relocation
      assert Settings.RELOCATABLE == Settings.EMULATED_FUNCTION_POINTERS == 0
      Settings.RELOCATABLE = Settings.EMULATED_FUNCTION_POINTERS = 1
      test()
      Settings.RELOCATABLE = Settings.EMULATED_FUNCTION_POINTERS = 0

  def test_ptrtoint(self):
      runner = self
      def check_warnings(output):
          runner.assertEquals(filter(lambda line: 'Warning' in line, output.split('\n')).__len__(), 4)

      test_path = path_from_root('tests', 'core', 'test_ptrtoint')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, output_processor=check_warnings)

  def test_sizeof(self):
      # Has invalid writes between printouts
      Settings.SAFE_HEAP = 0

      test_path = path_from_root('tests', 'core', 'test_sizeof')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, [], lambda x, err: x.replace('\n', '*'))

  def test_llvm_used(self):
    Building.LLVM_OPTS = 3

    test_path = path_from_root('tests', 'core', 'test_llvm_used')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_set_align(self):
    Settings.SAFE_HEAP = 1

    test_path = path_from_root('tests', 'core', 'test_set_align')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_emscripten_api(self):
      test_path = path_from_root('tests', 'core', 'test_emscripten_api')
      src, output = (test_path + s for s in ('.in', '.out'))

      check = '''
def process(filename):
  src = open(filename, 'r').read()
  # TODO: restore this (see comment in emscripten.h) assert '// hello from the source' in src
'''
      Settings.EXPORTED_FUNCTIONS = ['_main', '_save_me_aimee']
      self.do_run_from_file(src, output, post_build=check)

      # test EXPORT_ALL
      Settings.EXPORTED_FUNCTIONS = []
      Settings.EXPORT_ALL = 1
      Settings.LINKABLE = 1
      self.do_run_from_file(src, output, post_build=check)

  def test_emscripten_get_now(self):
    self.banned_js_engines = [V8_ENGINE] # timer limitations in v8 shell

    if self.run_name == 'asm2':
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage
    self.do_run(open(path_from_root('tests', 'emscripten_get_now.cpp')).read(), 'Timer resolution is good.')

  def test_emscripten_get_compiler_setting(self):
    test_path = path_from_root('tests', 'core', 'emscripten_get_compiler_setting')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run(open(src).read(), 'You must build with -s RETAIN_COMPILER_SETTINGS=1')
    Settings.RETAIN_COMPILER_SETTINGS = 1
    self.do_run(open(src).read(), open(output).read().replace('waka', EMSCRIPTEN_VERSION))

  # TODO: test only worked in non-fastcomp
  def test_inlinejs(self):
      return self.skip('non-fastcomp is deprecated and fails in 3.5') # only supports EM_ASM

      test_path = path_from_root('tests', 'core', 'test_inlinejs')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

      if self.emcc_args == []: # opts will eliminate the comments
        out = open('src.cpp.o.js').read()
        for i in range(1, 5): assert ('comment%d' % i) in out

  # TODO: test only worked in non-fastcomp
  def test_inlinejs2(self):
      return self.skip('non-fastcomp is deprecated and fails in 3.5') # only supports EM_ASM

      test_path = path_from_root('tests', 'core', 'test_inlinejs2')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_inlinejs3(self):
    if self.is_wasm(): return self.skip('wasm requires a proper asm module')

    test_path = path_from_root('tests', 'core', 'test_inlinejs3')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

    print 'no debugger, check validation'
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
''', 'false')

  def test_em_asm_unicode(self):
    self.do_run(r'''
#include <emscripten.h>

int main() {
  EM_ASM( Module.print("hello world…") );
}
''', 'hello world…')

  def test_em_asm_unused_arguments(self):
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>

      int main(int argc, char **argv) {
        int sum = EM_ASM_INT({
           return $0 + $2;
        }, 0, 1, 2);
        printf("0+2=%d\n", sum);
        return 0;
      }
    '''
    self.do_run(src, '''0+2=2''')

  # Verify that EM_ASM macros support getting called with multiple arities.
  # Maybe tests will later be joined into larger compilation units?
  # Then this must still be compiled separately from other code using EM_ASM
  # macros with arities 1-3. Otherwise this may incorrectly report a success.
  def test_em_asm_parameter_pack(self):
    Building.COMPILER_TEST_OPTS += ['-std=c++11']
    src = r'''
      #include <emscripten.h>

      template <typename... Args>
      int call(Args... args) {
        return(EM_ASM_INT(
          {
            console.log(Array.prototype.join.call(arguments, ','));
          },
          args...
        ));
      }

      int main(int argc, char **argv) {
        call(1);
        call(1, 2);
        call(1, 2, 3);
        return 0;
      }
    '''
    self.do_run(src, '''1\n1,2\n1,2,3''')

  def test_memorygrowth(self):
    if self.is_wasm(): return self.skip('wasm support for memory growth in the MVP is yet unclear')
    self.banned_js_engines = [V8_ENGINE] # stderr printing limitations in v8

    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=0'] # start with 0

    # With typed arrays in particular, it is dangerous to use more memory than TOTAL_MEMORY,
    # since we then need to enlarge the heap(s).
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      #include <assert.h>
      #include "emscripten.h"

      int main(int argc, char **argv)
      {
        char *buf1 = (char*)malloc(100);
        char *data1 = (char*)"hello";
        memcpy(buf1, data1, strlen(data1)+1);

        float *buf2 = (float*)malloc(100);
        float pie = 4.955;
        memcpy(buf2, &pie, sizeof(float));

        printf("*pre: %s,%.3f*\n", buf1, buf2[0]);

        int totalMemory = emscripten_run_script_int("TOTAL_MEMORY");
        char *buf3 = (char*)malloc(totalMemory+1);
        buf3[argc] = (int)buf2;
        if (argc % 7 == 6) printf("%d\n", (int)memcpy(buf3, buf1, argc));
        char *buf4 = (char*)malloc(100);
        float *buf5 = (float*)malloc(100);
        //printf("totalMemory: %d bufs: %d,%d,%d,%d,%d\n", totalMemory, buf1, buf2, buf3, buf4, buf5);
        assert((int)buf4 > (int)totalMemory && (int)buf5 > (int)totalMemory);

        printf("*%s,%.3f*\n", buf1, buf2[0]); // the old heap data should still be there

        memcpy(buf4, buf1, strlen(data1)+1);
        memcpy(buf5, buf2, sizeof(float));
        printf("*%s,%.3f*\n", buf4, buf5[0]); // and the new heap space should work too

        return 0;
      }
    '''

    # Fail without memory growth
    self.do_run(src, 'Cannot enlarge memory arrays.')
    fail = open('src.cpp.o.js').read()

    # Win with it
    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=1']
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')
    win = open('src.cpp.o.js').read()

    if '-O2' in self.emcc_args:
      # Make sure ALLOW_MEMORY_GROWTH generates different code (should be less optimized)
      code_start = 'var TOTAL_MEMORY'
      fail = fail[fail.find(code_start):]
      win = win[win.find(code_start):]
      assert len(fail) < len(win), 'failing code - without memory growth on - is more optimized, and smaller' + str([len(fail), len(win)])

    # Tracing of memory growths should work
    Settings.EMSCRIPTEN_TRACING = 1
    self.emcc_args += ['--tracing']
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')

  def test_memorygrowth_2(self):
    self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=0'] # start with 0

    emcc_args = self.emcc_args[:]

    def test():
      self.emcc_args = emcc_args[:]

      # With typed arrays in particular, it is dangerous to use more memory than TOTAL_MEMORY,
      # since we then need to enlarge the heap(s).
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        #include <assert.h>
        #include "emscripten.h"

        int main(int argc, char **argv)
        {
          char *buf1 = (char*)malloc(100);
          char *data1 = "hello";
          memcpy(buf1, data1, strlen(data1)+1);

          float *buf2 = (float*)malloc(100);
          float pie = 4.955;
          memcpy(buf2, &pie, sizeof(float));

          printf("*pre: %s,%.3f*\n", buf1, buf2[0]);

          int totalMemory = emscripten_run_script_int("TOTAL_MEMORY");
          int chunk = 1024*1024;
          char *buf3;
          for (int i = 0; i < (totalMemory/chunk)+1; i++) {
            buf3 = (char*)malloc(chunk);
            buf3[argc] = (int)buf2;
          }
          if (argc % 7 == 6) printf("%d\n", memcpy(buf3, buf1, argc));
          char *buf4 = (char*)malloc(100);
          float *buf5 = (float*)malloc(100);
          //printf("totalMemory: %d bufs: %d,%d,%d,%d,%d\n", totalMemory, buf1, buf2, buf3, buf4, buf5);
          assert((int)buf4 > (int)totalMemory && (int)buf5 > (int)totalMemory);

          printf("*%s,%.3f*\n", buf1, buf2[0]); // the old heap data should still be there

          memcpy(buf4, buf1, strlen(data1)+1);
          memcpy(buf5, buf2, sizeof(float));
          printf("*%s,%.3f*\n", buf4, buf5[0]); // and the new heap space should work too

          return 0;
        }
      '''

      # Fail without memory growth
      self.do_run(src, 'Cannot enlarge memory arrays.')
      fail = open('src.cpp.o.js').read()

      # Win with it
      self.emcc_args += ['-s', 'ALLOW_MEMORY_GROWTH=1']
      self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')
      win = open('src.cpp.o.js').read()

      if '-O2' in self.emcc_args:
        # Make sure ALLOW_MEMORY_GROWTH generates different code (should be less optimized)
        code_start = 'var TOTAL_MEMORY'
        fail = fail[fail.find(code_start):]
        win = win[win.find(code_start):]
        assert len(fail) < len(win), 'failing code - without memory growth on - is more optimized, and smaller' + str([len(fail), len(win)])

    test()

    if not self.is_wasm():
      print 'split memory'
      Settings.SPLIT_MEMORY = 16*1024*1024
      test()
      Settings.SPLIT_MEMORY = 0

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

        static opj_mqc_state_t mqc_states[2] = {
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
      test_path = path_from_root('tests', 'core', 'test_tinyfuncstr')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_llvmswitch(self):
      test_path = path_from_root('tests', 'core', 'test_llvmswitch')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  # By default, when user has not specified a -std flag, Emscripten should always build .cpp files using the C++03 standard,
  # i.e. as if "-std=c++03" had been passed on the command line. On Linux with Clang 3.2 this is the case, but on Windows
  # with Clang 3.2 -std=c++11 has been chosen as default, because of
  # < jrose> clb: it's deliberate, with the idea that for people who don't care about the standard, they should be using the "best" thing we can offer on that platform
  def test_cxx03_do_run(self):
    test_path = path_from_root('tests', 'core', 'test_cxx03_do_run')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  @no_emterpreter
  def test_bigswitch(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # bug 1174230
    src = open(path_from_root('tests', 'bigswitch.cpp')).read()
    self.do_run(src, '''34962: GL_ARRAY_BUFFER (0x8892)
26214: what?
35040: GL_STREAM_DRAW (0x88E0)
3060: what?
''', args=['34962', '26214', '35040', str(0xbf4)])

  @no_emterpreter
  def test_biggerswitch(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # bug 1174230
    num_cases = 20000
    switch_case, err = Popen([PYTHON, path_from_root('tests', 'gen_large_switchcase.py'), str(num_cases)], stdout=PIPE, stderr=PIPE).communicate()
    self.do_run(switch_case, '''58996: 589965899658996
59297: 592975929759297
59598: default
59899: 598995989959899
Success!''')

  def test_indirectbr(self):
      Building.COMPILER_TEST_OPTS = filter(lambda x: x != '-g', Building.COMPILER_TEST_OPTS)

      test_path = path_from_root('tests', 'core', 'test_indirectbr')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_indirectbr_many(self):
      test_path = path_from_root('tests', 'core', 'test_indirectbr_many')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

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
      test_path = path_from_root('tests', 'core', 'test_varargs')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_varargs_byval(self):
    return self.skip('clang cannot compile this code with that target yet')

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
    test_path = path_from_root('tests', 'core', 'test_functionpointer_libfunc_varargs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_structbyval(self):
      Settings.INLINING_LIMIT = 50

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

      # Check for lack of warning in the generated code (they should appear in part 2)
      generated = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
      assert 'Casting a function pointer type to another with a different number of arguments.' not in generated, 'Unexpected warning'

      # part 2: make sure we warn about mixing c and c++ calling conventions here

      if self.emcc_args != []: return # Optimized code is missing the warning comments

      header = r'''
        struct point
        {
          int x, y;
        };

      '''
      open(os.path.join(self.get_dir(), 'header.h'), 'w').write(header)

      supp = r'''
        #include <stdio.h>
        #include "header.h"

        void dump(struct point p) {
          p.x++; // should not modify
          p.y++; // anything in the caller!
          printf("dump: %d,%d\n", p.x, p.y);
        }
      '''
      supp_name = os.path.join(self.get_dir(), 'supp.c')
      open(supp_name, 'w').write(supp)

      main = r'''
        #include <stdio.h>
        #include "header.h"

        #ifdef __cplusplus
        extern "C" {
        #endif
          void dump(struct point p);
        #ifdef __cplusplus
        }
        #endif

        int main( int argc, const char *argv[] ) {
          struct point p = { 54, 2 };
          printf("pre:  %d,%d\n", p.x, p.y);
          dump(p);
          void (*dp)(struct point p) = dump; // And, as a function pointer
          dp(p);
          printf("post: %d,%d\n", p.x, p.y);
          return 0;
        }
      '''
      main_name = os.path.join(self.get_dir(), 'main.cpp')
      open(main_name, 'w').write(main)

      Building.emcc(supp_name)
      Building.emcc(main_name)
      all_name = os.path.join(self.get_dir(), 'all.bc')
      Building.link([supp_name + '.o', main_name + '.o'], all_name)

      # This will fail! See explanation near the warning we check for, in the compiler source code
      output = Popen([PYTHON, EMCC, all_name], stderr=PIPE).communicate()

      # Check for warning in the generated code
      generated = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
      print >> sys.stderr, 'skipping C/C++ conventions warning check, since not i386-pc-linux-gnu'

  def test_stdlibs(self):
      # safe heap prints a warning that messes up our output.
      Settings.SAFE_HEAP = 0
      src = '''
        #include <stdio.h>
        #include <stdlib.h>
        #include <ctype.h>
        #include <sys/time.h>

        void clean()
        {
          printf("*cleaned*\\n");
        }

        int comparer(const void *a, const void *b) {
          int aa = *((int*)a);
          int bb = *((int*)b);
          return aa - bb;
        }

        int main() {
          // timeofday
          timeval t;
          gettimeofday(&t, NULL);
          printf("*%d,%d\\n", int(t.tv_sec), int(t.tv_usec)); // should not crash

          // atexit
          atexit(clean);

          // qsort
          int values[6] = { 3, 2, 5, 1, 5, 6 };
          qsort(values, 5, sizeof(int), comparer);
          printf("*%d,%d,%d,%d,%d,%d*\\n", values[0], values[1], values[2], values[3], values[4], values[5]);

          printf("*stdin==0:%d*\\n", stdin == 0); // check that external values are at least not NULL
          printf("*%%*\\n");
          printf("*%.1ld*\\n", 5);

          printf("*%.1f*\\n", strtod("66", NULL)); // checks dependency system, as our strtod needs _isspace etc.

          printf("*%ld*\\n", strtol("10", NULL, 0));
          printf("*%ld*\\n", strtol("0", NULL, 0));
          printf("*%ld*\\n", strtol("-10", NULL, 0));
          printf("*%ld*\\n", strtol("12", NULL, 16));

          printf("*%lu*\\n", strtoul("10", NULL, 0));
          printf("*%lu*\\n", strtoul("0", NULL, 0));
          printf("*%lu*\\n", strtoul("-10", NULL, 0));

          printf("*malloc(0)!=0:%d*\\n", malloc(0) != 0); // We should not fail horribly

          printf("tolower_l: %c\\n", tolower_l('A', 0));

          return 0;
        }
        '''

      self.do_run(src, '*1,2,3,5,5,6*\n*stdin==0:0*\n*%*\n*5*\n*66.0*\n*10*\n*0*\n*-10*\n*18*\n*10*\n*0*\n*4294967286*\n*malloc(0)!=0:1*\ntolower_l: a\n*cleaned*')

      src = r'''
        #include <stdio.h>
        #include <stdbool.h>

        int main() {
          bool x = true;
          bool y = false;
          printf("*%d*\n", x != y);
          return 0;
        }
      '''

      self.do_run(src, '*1*', force_c=True)

  def test_strtoll_hex(self):
    # tests strtoll for hex strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_hex')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtoll_dec(self):
    # tests strtoll for decimal strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_dec')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtoll_bin(self):
    # tests strtoll for binary strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_bin')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtoll_oct(self):
    # tests strtoll for decimal strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_oct')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtol_hex(self):
    # tests strtoll for hex strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtol_hex')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtol_dec(self):
    # tests strtoll for decimal strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtol_dec')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtol_bin(self):
    # tests strtoll for binary strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtol_bin')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtol_oct(self):
    # tests strtoll for decimal strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtol_oct')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_atexit(self):
    # Confirms they are called in reverse order
    test_path = path_from_root('tests', 'core', 'test_atexit')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_pthread_specific(self):
    src = open(path_from_root('tests', 'pthread', 'specific.c'), 'r').read()
    expected = open(path_from_root('tests', 'pthread', 'specific.c.txt'), 'r').read()
    self.do_run(src, expected, force_c=True)

  def test_tcgetattr(self):
    src = open(path_from_root('tests', 'termios', 'test_tcgetattr.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_time(self):
    src = open(path_from_root('tests', 'time', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'time', 'output.txt'), 'r').read()
    self.do_run(src, expected);

  def test_timeb(self):
    # Confirms they are called in reverse order
    test_path = path_from_root('tests', 'core', 'test_timeb')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_time_c(self):
    test_path = path_from_root('tests', 'core', 'test_time_c')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_gmtime(self):
    test_path = path_from_root('tests', 'core', 'test_gmtime')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strptime_tm(self):
    test_path = path_from_root('tests', 'core', 'test_strptime_tm')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strptime_days(self):
    test_path = path_from_root('tests', 'core', 'test_strptime_days')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strptime_reentrant(self):
    test_path = path_from_root('tests', 'core', 'test_strptime_reentrant')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strftime(self):
    test_path = path_from_root('tests', 'core', 'test_strftime')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_intentional_fault(self):
    # Some programs intentionally segfault themselves, we should compile that into a throw
    src = r'''
      int main () {
        *(volatile char *)0 = 0;
        return *(volatile char *)0;
      }
      '''
    self.do_run(src, 'abort()' if self.run_name != 'asm2g' else 'abort("segmentation fault')

  def test_trickystring(self):
    test_path = path_from_root('tests', 'core', 'test_trickystring')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_statics(self):
      test_path = path_from_root('tests', 'core', 'test_statics')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_copyop(self):
      # clang generated code is vulnerable to this, as it uses
      # memcpy for assignments, with hardcoded numbers of bytes
      # (llvm-gcc copies items one by one). See QUANTUM_SIZE in
      # settings.js.
      test_path = path_from_root('tests', 'core', 'test_copyop')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_memcpy_memcmp(self):
      test_path = path_from_root('tests', 'core', 'test_memcpy_memcmp')
      src, output = (test_path + s for s in ('.in', '.out'))

      def check(result, err):
        result = result.replace('\n \n', '\n') # remove extra node output
        return hashlib.sha1(result).hexdigest()

      self.do_run_from_file(src, output, output_nicerizer = check)

  def test_memcpy2(self):
      test_path = path_from_root('tests', 'core', 'test_memcpy2')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_memcpy3(self):
    test_path = path_from_root('tests', 'core', 'test_memcpy3')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_memset(self):
    test_path = path_from_root('tests', 'core', 'test_memset')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_getopt(self):
      test_path = path_from_root('tests', 'core', 'test_getopt')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, args=['-t', '12', '-n', 'foobar'])

  def test_getopt_long(self):
      test_path = path_from_root('tests', 'core', 'test_getopt_long')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, args=['--file', 'foobar', '-b'])

  def test_memmove(self):
    test_path = path_from_root('tests', 'core', 'test_memmove')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_memmove2(self):
    test_path = path_from_root('tests', 'core', 'test_memmove2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_memmove3(self):
    test_path = path_from_root('tests', 'core', 'test_memmove3')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_flexarray_struct(self):
    test_path = path_from_root('tests', 'core', 'test_flexarray_struct')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_bsearch(self):
    test_path = path_from_root('tests', 'core', 'test_bsearch')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack_overflow(self):
    Settings.ASSERTIONS = 1
    self.do_run(open(path_from_root('tests', 'core', 'stack_overflow.cpp')).read(), 'abort()')

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

  def test_runtimelink(self):
    return self.skip('BUILD_AS_SHARED_LIB=2 is deprecated')
    if Building.LLVM_OPTS: return self.skip('LLVM opts will optimize printf into puts in the parent, and the child will still look for puts')
    if Settings.ASM_JS: return self.skip('asm does not support runtime linking')

    main, supp = self.setup_runtimelink_test()

    self.banned_js_engines = [NODE_JS] # node's global scope behaves differently than everything else, needs investigation FIXME
    Settings.LINKABLE = 1
    Settings.BUILD_AS_SHARED_LIB = 2

    self.build(supp, self.get_dir(), self.in_dir('supp.cpp'))
    shutil.move(self.in_dir('supp.cpp.o.js'), self.in_dir('liblib.so'))
    Settings.BUILD_AS_SHARED_LIB = 0

    Settings.RUNTIME_LINKED_LIBS = ['liblib.so'];
    self.do_run(main, 'supp: 54,2\nmain: 56\nsupp see: 543\nmain see: 76\nok.')

  def can_dlfcn(self):
    if Settings.ALLOW_MEMORY_GROWTH == 1: return self.skip('no dlfcn with memory growth yet')
    if self.is_wasm(): return self.skip('no shared modules in wasm')
    return True

  def prep_dlfcn_lib(self):
    Settings.MAIN_MODULE = 0
    Settings.SIDE_MODULE = 1

  def prep_dlfcn_main(self):
    Settings.MAIN_MODULE = 1
    Settings.SIDE_MODULE = 0

  dlfcn_post_build = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createLazyFile('/', 'liblib.so', 'liblib.so', true, false);"
  )
  open(filename, 'w').write(src)
'''

  def test_dlfcn_basic(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    lib_src = '''
      #include <cstdio>

      class Foo {
      public:
        Foo() {
          printf("Constructing lib object.\\n");
        }
      };

      Foo global;
      '''
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.cpp')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    src = '''
      #include <cstdio>
      #include <dlfcn.h>

      class Bar {
      public:
        Bar() {
          printf("Constructing main object.\\n");
        }
      };

      Bar global;

      int main() {
        dlopen("liblib.so", RTLD_NOW);
        return 0;
      }
      '''
    self.do_run(src, 'Constructing main object.\nConstructing lib object.\n',
                post_build=self.dlfcn_post_build)

  def test_dlfcn_i64(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    Settings.EXPORTED_FUNCTIONS = ['_foo']
    lib_src = '''
      int foo(int x) {
        return (long long)x / (long long)1234;
      }
      '''
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.c')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    Settings.EXPORTED_FUNCTIONS = ['_main']
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <dlfcn.h>

      typedef int (*intfunc)(int);

      void *p;

      int main() {
        p = malloc(1024);
        void *lib_handle = dlopen("liblib.so", 0);
        printf("load %p\n", lib_handle);
        intfunc x = (intfunc)dlsym(lib_handle, "foo");
        printf("foo func %p\n", x);
        if (p == 0) return 1;
        printf("|%d|\n", x(81234567));
        return 0;
      }
      '''
    self.do_run(src, '|65830|', post_build=self.dlfcn_post_build)

  def test_dlfcn_em_asm(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    lib_src = '''
      #include <emscripten.h>
      class Foo {
      public:
        Foo() {
          EM_ASM( Module.print("Constructing lib object.") );
        }
      };
      Foo global;
      '''
    filename = 'liblib.cpp'
    self.build(lib_src, self.get_dir(), filename)
    shutil.move(filename + '.o.js', 'liblib.so')

    self.prep_dlfcn_main()
    src = '''
      #include <emscripten.h>
      #include <dlfcn.h>
      class Bar {
      public:
        Bar() {
          EM_ASM( Module.print("Constructing main object.") );
        }
      };
      Bar global;
      int main() {
        dlopen("liblib.so", RTLD_NOW);
        EM_ASM( Module.print("All done.") );
        return 0;
      }
      '''
    self.do_run(src, 'Constructing main object.\nConstructing lib object.\nAll done.\n',
                post_build=self.dlfcn_post_build)

  def test_dlfcn_qsort(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    Settings.EXPORTED_FUNCTIONS = ['_get_cmp']
    lib_src = '''
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
      '''
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.cpp')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc']
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
                output_nicerizer=lambda x, err: x.replace('\n', '*'),
                post_build=self.dlfcn_post_build)

    if Settings.ASM_JS and SPIDERMONKEY_ENGINE and os.path.exists(SPIDERMONKEY_ENGINE[0]):
      out = run_js('liblib.so', engine=SPIDERMONKEY_ENGINE, full_output=True, stderr=STDOUT)
      if 'asm' in out:
        self.validate_asmjs(out)

  def test_dlfcn_data_and_fptr(self):
    if not self.can_dlfcn(): return

    if Building.LLVM_OPTS: return self.skip('LLVM opts will optimize out parent_func')

    self.prep_dlfcn_lib()
    lib_src = '''
      #include <stdio.h>

      int global = 42;

      extern void parent_func(); // a function that is defined in the parent

      void lib_fptr() {
        printf("Second calling lib_fptr from main.\\n");
        parent_func();
        // call it also through a pointer, to check indexizing
        void (*p_f)();
        p_f = parent_func;
        p_f();
      }

      extern "C" void (*func(int x, void(*fptr)()))() {
        printf("In func: %d\\n", x);
        fptr();
        return lib_fptr;
      }
      '''
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.cpp')
    Settings.EXPORTED_FUNCTIONS = ['_func']
    Settings.EXPORTED_GLOBALS = ['_global']
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    Settings.LINKABLE = 1
    src = '''
      #include <stdio.h>
      #include <dlfcn.h>
      #include <emscripten.h>

      typedef void (*FUNCTYPE(int, void(*)()))();

      FUNCTYPE func;

      void EMSCRIPTEN_KEEPALIVE parent_func() {
        printf("parent_func called from child\\n");
      }

      void main_fptr() {
        printf("First calling main_fptr from lib.\\n");
      }

      int main() {
        void* lib_handle;
        FUNCTYPE* func_fptr;

        // Test basic lib loading.
        lib_handle = dlopen("liblib.so", RTLD_NOW);
        if (lib_handle == NULL) {
          printf("Could not load lib.\\n");
          return 1;
        }

        // Test looked up function.
        func_fptr = (FUNCTYPE*) dlsym(lib_handle, "func");
        // Load twice to test cache.
        func_fptr = (FUNCTYPE*) dlsym(lib_handle, "func");
        if (func_fptr == NULL) {
          printf("Could not find func.\\n");
          return 1;
        }

        // Test passing function pointers across module bounds.
        void (*fptr)() = func_fptr(13, main_fptr);
        fptr();

        // Test global data.
        int* global = (int*) dlsym(lib_handle, "global");
        if (global == NULL) {
          printf("Could not find global.\\n");
          return 1;
        }

        printf("Var: %d\\n", *global);

        return 0;
      }
      '''
    Settings.EXPORTED_FUNCTIONS = ['_main']
    Settings.EXPORTED_GLOBALS = []
    self.do_run(src, 'In func: 13*First calling main_fptr from lib.*Second calling lib_fptr from main.*parent_func called from child*parent_func called from child*Var: 42*',
                 output_nicerizer=lambda x, err: x.replace('\n', '*'),
                 post_build=self.dlfcn_post_build)

  def test_dlfcn_varargs(self):
    # this test is not actually valid - it fails natively. the child should fail to be loaded, not load and successfully see the parent print_ints func
    if not self.can_dlfcn(): return

    Settings.LINKABLE = 1

    self.prep_dlfcn_lib()
    lib_src = r'''
      void print_ints(int n, ...);
      extern "C" void func() {
        print_ints(2, 13, 42);
      }
      '''
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.cpp')
    Settings.EXPORTED_FUNCTIONS = ['_func']
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

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
    Settings.EXPORTED_FUNCTIONS = ['_main']
    self.do_run(src, '100\n200\n13\n42\n',
                post_build=self.dlfcn_post_build)

  def test_dlfcn_self(self):
    if not self.can_dlfcn(): return
    self.prep_dlfcn_main()

    def post(filename):
      with open(filename) as f:
        for line in f:
          if 'var NAMED_GLOBALS' in line:
            table = line
            break
        else:
          raise Exception('Could not find symbol table!')
      table = table[table.find('{'):table.find('}')+1]
      # ensure there aren't too many globals; we don't want unnamed_addr
      assert table.count(',') <= 23, table.count(',')

    test_path = path_from_root('tests', 'core', 'test_dlfcn_self')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, post_build=(None, post))

  def test_dlfcn_unique_sig(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    lib_src = '''
      #include <stdio.h>

      int myfunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m) {
        return 13;
      }
      '''
    Settings.EXPORTED_FUNCTIONS = ['_myfunc']
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.c')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    src = '''
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
      '''
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc']
    self.do_run(src, 'success', force_c=True, post_build=self.dlfcn_post_build)

  def test_dlfcn_stacks(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    lib_src = '''
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
      '''
    Settings.EXPORTED_FUNCTIONS = ['_myfunc']
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.c')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    src = '''
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
      '''
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc', '_strcmp']
    self.do_run(src, 'success', force_c=True, post_build=self.dlfcn_post_build)

  def test_dlfcn_funcs(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    lib_src = r'''
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
      '''
    Settings.EXPORTED_FUNCTIONS = ['_callvoid', '_callint', '_getvoid', '_getint']
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.c')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    src = r'''
      #include <assert.h>
      #include <stdio.h>
      #include <dlfcn.h>

      typedef void (*voidfunc)();
      typedef void (*intfunc)(int);

      typedef void (*voidcaller)(voidfunc);
      typedef void (*intcaller)(intfunc, int);

      typedef voidfunc (*voidgetter)(int);
      typedef intfunc (*intgetter)(int);

      void void_main() { printf("main.\n"); }
      void int_main(int x) { printf("main %d\n", x); }

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
      '''
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc']
    self.do_run(src, '''go
main.
main 201
void 0
void 1
int 0 54
int 1 9000
ok
''', force_c=True, post_build=self.dlfcn_post_build)

  def test_dlfcn_mallocs(self):
    if not self.can_dlfcn(): return

    Settings.TOTAL_MEMORY = 64*1024*1024 # will be exhausted without functional malloc/free

    self.prep_dlfcn_lib()
    lib_src = r'''
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>
      #include <stdlib.h>

      void *mallocproxy(int n) { return malloc(n); }
      void freeproxy(void *p) { free(p); }
      '''
    Settings.EXPORTED_FUNCTIONS = ['_mallocproxy', '_freeproxy']
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.c')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    self.prep_dlfcn_main()
    src = open(path_from_root('tests', 'dlmalloc_proxy.c')).read()
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc', '_free']
    self.do_run(src, '''*294,153*''', force_c=True, post_build=self.dlfcn_post_build)

  def test_dlfcn_longjmp(self):
    if not self.can_dlfcn(): return

    self.prep_dlfcn_lib()
    lib_src = r'''
      #include <setjmp.h>
      #include <stdio.h>

      void jumpy(jmp_buf buf) {
        static int i = 0;
        i++;
        if (i == 10) longjmp(buf, i);
        printf("pre %d\n", i);
      }
      '''
    Settings.EXPORTED_FUNCTIONS = ['_jumpy']
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.c')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

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
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc', '_free']
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
''', post_build=self.dlfcn_post_build, force_c=True)

  def zzztest_dlfcn_exceptions(self): # TODO: make this work. need to forward tempRet0 across modules
    if not self.can_dlfcn(): return

    Settings.DISABLE_EXCEPTION_CATCHING = 0

    self.prep_dlfcn_lib()
    lib_src = r'''
      extern "C" {
      int ok() {
        return 65;
      }
      int fail() {
        throw 123;
      }
      }
      '''
    Settings.EXPORTED_FUNCTIONS = ['_ok', '_fail']
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.cpp')
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

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
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc', '_free']
    self.do_run(src, '''go!
ok: 65
int 123
ok
''', post_build=self.dlfcn_post_build)

  def dylink_test(self, main, side, expected, header=None, main_emcc_args=[], force_c=False, need_reverse=True, auto_load=True):
    if not self.can_dlfcn(): return

    if header:
      open('header.h', 'w').write(header)

    emcc_args = self.emcc_args[:]
    try:
      # general settings
      Settings.DISABLE_EXCEPTION_CATCHING = 1
      self.emcc_args += ['--memory-init-file', '0']

      # side settings
      Settings.MAIN_MODULE = 0
      Settings.SIDE_MODULE = 1
      if type(side) == str:
        base = 'liblib.cpp' if not force_c else 'liblib.c'
        try_delete(base + '.o.js')
        self.build(side, self.get_dir(), base)
        if force_c:
          shutil.move(base + '.o.js', 'liblib.cpp.o.js')
      else:
        # side is just a library
        try_delete('liblib.cpp.o.js')
        Popen([PYTHON, EMCC] + side + self.emcc_args + Settings.serialize() + ['-o', os.path.join(self.get_dir(), 'liblib.cpp.o.js')]).communicate()
      if SPIDERMONKEY_ENGINE and os.path.exists(SPIDERMONKEY_ENGINE[0]):
        out = run_js('liblib.cpp.o.js', engine=SPIDERMONKEY_ENGINE, full_output=True, stderr=STDOUT)
        if 'asm' in out:
          self.validate_asmjs(out)
      shutil.move('liblib.cpp.o.js', 'liblib.so')

      # main settings
      Settings.MAIN_MODULE = 1
      Settings.SIDE_MODULE = 0
      if auto_load:
        open('pre.js', 'w').write('''
var Module = {
  dynamicLibraries: ['liblib.so'],
};
  ''')
        self.emcc_args += ['--pre-js', 'pre.js'] + main_emcc_args

      if type(main) == str:
        self.do_run(main, expected, force_c=force_c)
      else:
        # main is just a library
        try_delete('src.cpp.o.js')
        Popen([PYTHON, EMCC] + main + self.emcc_args + Settings.serialize() + ['-o', os.path.join(self.get_dir(), 'src.cpp.o.js')]).communicate()
        self.do_run(None, expected, no_build=True)
    finally:
      self.emcc_args = emcc_args[:]

    if need_reverse:
      # test the reverse as well
      print 'flip'
      self.dylink_test(side, main, expected, header, main_emcc_args, force_c, need_reverse=False)

  def test_dylink_basics(self):
    self.dylink_test('''
      #include <stdio.h>
      extern int sidey();
      int main() {
        printf("other says %d.", sidey());
        return 0;
      }
    ''', '''
      int sidey() { return 11; }
    ''', 'other says 11.')

  def test_dylink_floats(self):
    self.dylink_test('''
      #include <stdio.h>
      extern float sidey();
      int main() {
        printf("other says %.2f.", sidey()+1);
        return 0;
      }
    ''', '''
      float sidey() { return 11.5; }
    ''', 'other says 12.50')

  def test_dylink_printfs(self):
    self.dylink_test(r'''
      #include <stdio.h>
      extern void sidey();
      int main() {
        printf("hello from main\n");
        sidey();
        return 0;
      }
    ''', r'''
      #include <stdio.h>
      void sidey() { printf("hello from side\n"); }
    ''', 'hello from main\nhello from side\n')

  def test_dylink_funcpointer(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include "header.h"
      voidfunc sidey(voidfunc f);
      void a() { printf("hello from funcptr\n"); }
      int main() {
        sidey(a)();
        return 0;
      }
    ''', '''
      #include "header.h"
      voidfunc sidey(voidfunc f) { return f; }
    ''', 'hello from funcptr\n', header='typedef void (*voidfunc)();')

  def test_dylink_funcpointers(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include "header.h"
      int sidey(voidfunc f);
      void areturn0() { printf("hello 0\n"); }
      void areturn1() { printf("hello 1\n"); }
      void areturn2() { printf("hello 2\n"); }
      int main(int argc, char **argv) {
        voidfunc table[3] = { areturn0, areturn1, areturn2 };
        table[sidey(NULL)]();
        return 0;
      }
    ''', '''
      #include "header.h"
      int sidey(voidfunc f) { if (f) f(); return 1; }
    ''', 'hello 1\n', header='typedef void (*voidfunc)();')

  def test_dylink_funcpointers2(self):
    self.dylink_test(r'''
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
          Runtime.alignFunctionTables();
          Module['FUNCTION_TABLE_v'].push(0, 0, 0, 0, 0);
          var newSize = Runtime.alignFunctionTables();
          //Module.print('new size of function tables: ' + newSize);
          // when masked, the two function pointers 1 and 2 should not happen to fall back to the right place
          assert(((newSize+1) & 3) !== 1 || ((newSize+2) & 3) !== 2);
          Runtime.loadDynamicLibrary('liblib.so');
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
    ''', r'''
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
    ''', 'main\nleft1\nleft2\nright1\nright2\nsecond\nleft1\nleft2\nright1\nright2\n', header='''
      #include <stdio.h>
      typedef void (*voidfunc)();
      typedef volatile voidfunc volatilevoidfunc;
      voidfunc getleft1();
      voidfunc getleft2();
      voidfunc getright1();
      voidfunc getright2();
      void second();
    ''', need_reverse=False, auto_load=False)

  def test_dylink_funcpointers_wrapper(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include "header.h"
      int main(int argc, char **argv) {
        volatile charfunc f = emscripten_run_script;
        f("Module.print('one')");
        f = get();
        f("Module.print('two')");
        return 0;
      }
    ''', '''
      #include "header.h"
      charfunc get() {
        return emscripten_run_script;
      }
    ''', 'one\ntwo\n', header='''
      #include <emscripten.h>
      typedef void (*charfunc)(const char*);
      extern charfunc get();
    ''')

  def test_dylink_funcpointers_float(self):
    self.dylink_test(r'''
      #include <stdio.h>
      #include "header.h"
      int sidey(floatfunc f);
      float areturn0(float f) { printf("hello 0: %f\n", f); return 0; }
      float areturn1(float f) { printf("hello 1: %f\n", f); return 1; }
      float areturn2(float f) { printf("hello 2: %f\n", f); return 2; }
      int main(int argc, char **argv) {
        volatile floatfunc table[3] = { areturn0, areturn1, areturn2 };
        printf("got: %d\n", (int)table[sidey(NULL)](12.34));
        return 0;
      }
    ''', '''
      #include "header.h"
      int sidey(floatfunc f) { if (f) f(56.78); return 1; }
    ''', 'hello 1: 12.340000\ngot: 1\n', header='typedef float (*floatfunc)(float);')

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

  def test_dylink_global_inits(self):
    if not self.can_dlfcn(): return
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

    if Settings.ASSERTIONS == 1:
      print 'check warnings'
      Settings.ASSERTIONS = 2
      test()
      full = run_js('src.cpp.o.js', engine=JS_ENGINES[0], full_output=True, stderr=STDOUT)
      self.assertNotContained("trying to dynamically load symbol '__ZN5ClassC2EPKc' (from 'liblib.so') that already exists", full)

  def test_dylink_i64(self):
    self.dylink_test('''
      #include <stdio.h>
      #include <stdint.h>
      extern int64_t sidey();
      int main() {
        printf("other says %llx.\\n", sidey());
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

  def test_dylink_i64_b(self):
    self.dylink_test('''
      #include <stdio.h>
      #include <stdint.h>
      extern int64_t sidey();
      int main() {
        printf("other says %lld.", sidey());
        return 0;
      }
    ''', '''
      #include <stdint.h>
      int64_t sidey() {
        volatile int64_t x = 0x12345678abcdef12LL;
        x += x % 17;
        x = 18 - x;
        return x;
      }
    ''', 'other says -1311768467750121224.')

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

  def test_dylink_mallocs(self):
    self.dylink_test(header=r'''
      #include <stdlib.h>
      #include <string.h>
      char *side(const char *data);
    ''', main=r'''
      #include <stdio.h>
      #include "header.h"
      int main() {
        char *temp = side("hello through side\n");
        char *ret = (char*)malloc(strlen(temp)+1);
        strcpy(ret, temp);
        temp[1] = 'x';
        puts(ret);
        return 0;
      }
    ''', side=r'''
      #include "header.h"
      char *side(const char *data) {
        char *ret = (char*)malloc(strlen(data)+1);
        strcpy(ret, data);
        return ret;
      }
    ''', expected=['hello through side\n'])

  def test_dylink_jslib(self):
    open('lib.js', 'w').write(r'''
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

  def test_dylink_global_var_jslib(self):
    open('lib.js', 'w').write(r'''
      mergeInto(LibraryManager.library, {
        jslib_x: 'allocate(1, "i32*", ALLOC_STATIC)',
        jslib_x__postset: 'HEAP32[_jslib_x>>2] = 148;',
      });
    ''')
    self.dylink_test(main=r'''
      #include <stdio.h>
      extern "C" int jslib_x;
      extern void call_side();
      int main() {
        printf("main: jslib_x is %d.\n", jslib_x);
        call_side();
        return 0;
      }
    ''', side=r'''
      #include <stdio.h>
      extern "C" int jslib_x;
      void call_side() {
        printf("side: jslib_x is %d.\n", jslib_x);
      }
    ''', expected=['main: jslib_x is 148.\nside: jslib_x is 148.\n'], main_emcc_args=['--js-library', 'lib.js'])

  def test_dylink_many_postSets(self):
    NUM = 1234
    self.dylink_test(header=r'''
      #include <stdio.h>
      typedef void (*voidfunc)();
      static void simple() {
        printf("simple.\n");
      }
      static volatile voidfunc funcs[''' + str(NUM) + '] = { ' + ','.join(['simple'] * NUM) + r''' };
      static void test() {
        volatile int i = ''' + str(NUM-1) + r''';
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

  def test_dylink_syslibs(self): # one module uses libcxx, need to force its inclusion when it isn't the main
    if not self.can_dlfcn(): return

    def test(syslibs, expect_pass=True, need_reverse=True):
      print 'syslibs', syslibs, Settings.ASSERTIONS
      passed = True
      try:
        os.environ['EMCC_FORCE_STDLIBS'] = syslibs
        self.dylink_test(main=r'''
          void side();
          int main() {
            side();
            return 0;
          }
        ''', side=r'''
          #include <iostream>
          void side() { std::cout << "cout hello from side"; }
        ''', expected=['cout hello from side\n'],
             need_reverse=need_reverse)
      except Exception, e:
        if expect_pass: raise e
        print '(seeing expected fail)'
        passed = False
        assertion = 'build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment'
        if Settings.ASSERTIONS:
          self.assertContained(assertion, str(e))
        else:
          self.assertNotContained(assertion, str(e))
      finally:
        del os.environ['EMCC_FORCE_STDLIBS']
      assert passed == expect_pass, ['saw', passed, 'but expected', expect_pass]

    test('libcxx')
    test('1')
    if 'ASSERTIONS=1' not in self.emcc_args:
      Settings.ASSERTIONS = 0
      test('', expect_pass=False, need_reverse=False)
    else:
      print '(skip ASSERTIONS == 0 part)'
    Settings.ASSERTIONS = 1
    test('', expect_pass=False, need_reverse=False)

  def test_dylink_iostream(self):
    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libcxx'
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
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']

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

  def test_dylink_hyper_dupe(self):
    if not self.can_dlfcn(): return

    Settings.TOTAL_MEMORY = 64*1024*1024

    if Settings.ASSERTIONS: self.emcc_args += ['-s', 'ASSERTIONS=2']

    # test hyper-dynamic linking, and test duplicate warnings
    open('third.cpp', 'w').write(r'''
      int sidef() { return 36; }
      int sideg = 49;
      int bsidef() { return 536; }
    ''')
    Popen([PYTHON, EMCC, 'third.cpp', '-s', 'SIDE_MODULE=1'] + Building.COMPILER_TEST_OPTS + self.emcc_args + ['-o', 'third.js']).communicate()

    self.dylink_test(main=r'''
      #include <stdio.h>
      #include <emscripten.h>
      extern int sidef();
      extern int sideg;
      extern int bsidef();
      extern int bsideg;
      int main() {
        EM_ASM({
          Runtime.loadDynamicLibrary('third.js'); // hyper-dynamic! works at least for functions (and consts not used in same block)
        });
        printf("sidef: %d, sideg: %d.\n", sidef(), sideg);
        printf("bsidef: %d.\n", bsidef());
      }
    ''', side=r'''
      int sidef() { return 10; } // third.js will try to override these, but fail!
      int sideg = 20;
    ''', expected=['sidef: 10, sideg: 20.\nbsidef: 536.\n'])

    if Settings.ASSERTIONS:
      print 'check warnings'
      full = run_js('src.cpp.o.js', engine=JS_ENGINES[0], full_output=True, stderr=STDOUT)
      #self.assertContained("warning: trying to dynamically load symbol '__Z5sidefv' (from 'third.js') that already exists", full)
      self.assertContained("warning: trying to dynamically load symbol '_sideg' (from 'third.js') that already exists", full)

  def test_dylink_dot_a(self):
    # .a linking must force all .o files inside it, when in a shared module
    open('third.cpp', 'w').write(r'''
      int sidef() { return 36; }
    ''')
    Popen([PYTHON, EMCC, 'third.cpp'] + Building.COMPILER_TEST_OPTS + self.emcc_args + ['-o', 'third.o', '-c']).communicate()

    open('fourth.cpp', 'w').write(r'''
      int sideg() { return 17; }
    ''')
    Popen([PYTHON, EMCC, 'fourth.cpp'] + Building.COMPILER_TEST_OPTS + self.emcc_args + ['-o', 'fourth.o', '-c']).communicate()

    Popen([PYTHON, EMAR, 'rc', 'libfourth.a', 'fourth.o']).communicate()

    self.dylink_test(main=r'''
      #include <stdio.h>
      #include <emscripten.h>
      extern int sidef();
      extern int sideg();
      int main() {
        printf("sidef: %d, sideg: %d.\n", sidef(), sideg());
      }
    ''', side=['libfourth.a', 'third.o'], # contents of libtwo.a must be included, even if they aren't referred to!
    expected=['sidef: 36, sideg: 17.\n'])

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

  def test_dylink_zlib(self):
    Building.COMPILER_TEST_OPTS += ['-I' + path_from_root('tests', 'zlib')]

    Popen([PYTHON, path_from_root('embuilder.py'), 'build' ,'zlib']).communicate()
    zlib = Cache.get_path(os.path.join('ports-builds', 'zlib', 'libz.a'))
    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libcextra'
      side = [zlib]
      self.dylink_test(main=open(path_from_root('tests', 'zlib', 'example.c'), 'r').read(),
                       side=side,
                       expected=open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                       force_c=True)
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']

  #def test_dylink_bullet(self):
  #  Building.COMPILER_TEST_OPTS += ['-I' + path_from_root('tests', 'bullet', 'src')]
  #  side = get_bullet_library(self, True)
  #  self.dylink_test(main=open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'), 'r').read(),
  #                   side=side,
  #                   expected=[open(path_from_root('tests', 'bullet', 'output.txt'), 'r').read(), # different roundings
  #                             open(path_from_root('tests', 'bullet', 'output2.txt'), 'r').read(),
  #                             open(path_from_root('tests', 'bullet', 'output3.txt'), 'r').read()])

  def test_random(self):
    src = r'''#include <stdlib.h>
#include <stdio.h>

int main()
{
    srandom(0xdeadbeef);
    printf("%ld", random());
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
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main() {
        char* endptr;

        printf("\n");
        printf("%g\n", strtod("0", &endptr));
        printf("%g\n", strtod("0.", &endptr));
        printf("%g\n", strtod("0.0", &endptr));
        printf("%g\n", strtod("-0.0", &endptr));
        printf("%g\n", strtod("1", &endptr));
        printf("%g\n", strtod("1.", &endptr));
        printf("%g\n", strtod("1.0", &endptr));
        printf("%g\n", strtod("z1.0", &endptr));
        printf("%g\n", strtod("0.5", &endptr));
        printf("%g\n", strtod(".5", &endptr));
        printf("%g\n", strtod(".a5", &endptr));
        printf("%g\n", strtod("123", &endptr));
        printf("%g\n", strtod("123.456", &endptr));
        printf("%g\n", strtod("-123.456", &endptr));
        printf("%g\n", strtod("1234567891234567890", &endptr));
        printf("%g\n", strtod("1234567891234567890e+50", &endptr));
        printf("%g\n", strtod("84e+220", &endptr));
        printf("%g\n", strtod("123e-50", &endptr));
        printf("%g\n", strtod("123e-250", &endptr));
        printf("%g\n", strtod("123e-450", &endptr));
        printf("%g\n", strtod("0x6", &endptr));
        printf("%g\n", strtod("-0x0p+0", &endptr));

        char str[] = "  12.34e56end";
        printf("%g\n", strtod(str, &endptr));
        printf("%d\n", endptr - str);
        printf("%g\n", strtod("84e+420", &endptr));

        printf("%.12f\n", strtod("1.2345678900000000e+08", NULL));

        return 0;
      }
      '''
    expected = '''
      0
      0
      0
      -0
      1
      1
      1
      0
      0.5
      0.5
      0
      123
      123.456
      -123.456
      1.23457e+18
      1.23457e+68
      8.4e+221
      1.23e-48
      1.23e-248
      0
      6
      -0
      1.234e+57
      10
      inf
      123456789.000000000000
      '''

    self.do_run(src, re.sub(r'\n\s+', '\n', expected))
    self.do_run(src.replace('strtod', 'strtold'), re.sub(r'\n\s+', '\n', expected)) # XXX add real support for long double

  def test_strtok(self):
    test_path = path_from_root('tests', 'core', 'test_strtok')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_parseInt(self):
    src = open(path_from_root('tests', 'parseInt', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'parseInt', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_transtrcase(self):
    test_path = path_from_root('tests', 'core', 'test_transtrcase')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_printf(self):
    self.banned_js_engines = [NODE_JS, V8_ENGINE] # SpiderMonkey and V8 do different things to float64 typed arrays, un-NaNing, etc.
    src = open(path_from_root('tests', 'printf', 'test.c'), 'r').read()
    expected = open(path_from_root('tests', 'printf', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_printf_2(self):
    test_path = path_from_root('tests', 'core', 'test_printf_2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_vprintf(self):
    test_path = path_from_root('tests', 'core', 'test_vprintf')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_vsnprintf(self):
    test_path = path_from_root('tests', 'core', 'test_vsnprintf')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_printf_more(self):
    test_path = path_from_root('tests', 'core', 'test_printf_more')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_perrar(self):
    test_path = path_from_root('tests', 'core', 'test_perrar')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_atoX(self):
    test_path = path_from_root('tests', 'core', 'test_atoX')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strstr(self):
    test_path = path_from_root('tests', 'core', 'test_strstr')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_fnmatch(self):
    # Run one test without assertions, for additional coverage
    #assert 'asm2m' in test_modes
    if self.run_name == 'asm2m':
      i = self.emcc_args.index('ASSERTIONS=1')
      assert i > 0 and self.emcc_args[i-1] == '-s'
      self.emcc_args[i] = 'ASSERTIONS=0'
      print 'flip assertions off'
    test_path = path_from_root('tests', 'core', 'fnmatch')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  def test_sscanf(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

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
    test_path = path_from_root('tests', 'core', 'test_sscanf_n')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_whitespace(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_whitespace')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_other_whitespace(self):
    Settings.SAFE_HEAP = 0 # use i16s in printf

    test_path = path_from_root('tests', 'core', 'test_sscanf_other_whitespace')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_3(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_3')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_4(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_4')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_5(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_5')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_6(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_6')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_sscanf_skip(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_skip')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_caps(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_caps')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_hex(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_hex')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_float(self):
    test_path = path_from_root('tests', 'core', 'test_sscanf_float')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_langinfo(self):
    src = open(path_from_root('tests', 'langinfo', 'test.c'), 'r').read()
    expected = open(path_from_root('tests', 'langinfo', 'output.txt'), 'r').read()
    self.do_run(src, expected, extra_emscripten_args=['-H', 'libc/langinfo.h'])

  def test_files(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # closure can generate variables called 'gc', which pick up js shell stuff
    if '-O2' in self.emcc_args and not self.is_wasm():
      self.emcc_args += ['--closure', '1'] # Use closure here, to test we don't break FS stuff
      self.emcc_args = filter(lambda x: x != '-g', self.emcc_args) # ensure we test --closure 1 --memory-init-file 1 (-g would disable closure)
    elif '-O3' in self.emcc_args and not self.is_wasm():
      print 'closure 2'
      self.emcc_args += ['--closure', '2'] # Use closure 2 here for some additional coverage

    print 'base', self.emcc_args

    post = '''
def process(filename):
  src = \'\'\'
    var Module = {
      'noFSInit': true,
      'preRun': function() {
        FS.createLazyFile('/', 'test.file', 'test.file', true, false);
        // Test FS_* exporting
        Module['FS_createDataFile']('/', 'somefile.binary', [100, 200, 50, 25, 10, 77, 123], true, false);  // 200 becomes -56, since signed chars are used in memory
        var test_files_input = 'hi there!';
        var test_files_input_index = 0;
        FS.init(function() {
          return test_files_input.charCodeAt(test_files_input_index++) || null;
        });
      }
    };
  \'\'\' + open(filename, 'r').read()
  open(filename, 'w').write(src)
'''
    other = open(os.path.join(self.get_dir(), 'test.file'), 'w')
    other.write('some data');
    other.close()

    src = open(path_from_root('tests', 'files.cpp'), 'r').read()

    mem_file = 'src.cpp.o.js.mem'
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'MEMFS_APPEND_TO_TYPED_ARRAYS=1'], ['-s', 'SYSCALL_DEBUG=1']]:
      print mode
      self.emcc_args = orig_args + mode
      try_delete(mem_file)

      def clean(out, err):
        return '\n'.join(filter(lambda line: 'binaryen' not in line, (out + err).split('\n')))

      self.do_run(src, map(lambda x: x if 'SYSCALL_DEBUG=1' not in mode else ('syscall! 146,SYS_writev' if self.run_name == 'default' else 'syscall! 146'), ('size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\n5 bytes to dev/null: 5\nok.\ntexte\n', 'size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\ntexte\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\n5 bytes to dev/null: 5\nok.\n')),
                  post_build=post, extra_emscripten_args=['-H', 'libc/fcntl.h'], output_nicerizer=clean)
      if self.uses_memory_init_file():
        assert os.path.exists(mem_file)

  def test_files_m(self):
    # Test for Module.stdin etc.

    post = '''
def process(filename):
  src = \'\'\'
    var Module = {
      data: [10, 20, 40, 30],
      stdin: function() { return Module.data.pop() || null },
      stdout: function(x) { Module.print('got: ' + x) }
    };
  \'\'\' + open(filename, 'r').read()
  open(filename, 'w').write(src)
'''
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
      return '\n'.join(filter(lambda line: 'warning' not in line and 'binaryen' not in line, (out + err).split('\n')))
    self.do_run(src, ('got: 35\ngot: 45\ngot: 25\ngot: 15\n \nisatty? 0,0,1\n', 'got: 35\ngot: 45\ngot: 25\ngot: 15\nisatty? 0,0,1\n', 'isatty? 0,0,1\ngot: 35\ngot: 45\ngot: 25\ngot: 15\n'), post_build=post, output_nicerizer=clean)

  def test_mount(self):
    Settings.FORCE_FILESYSTEM = 1
    src = open(path_from_root('tests', 'fs', 'test_mount.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_fwrite_0(self):
    test_path = path_from_root('tests', 'core', 'test_fwrite_0')
    src, output = (test_path + s for s in ('.in', '.out'))

    orig_args = self.emcc_args
    for mode in [[], ['-s', 'MEMFS_APPEND_TO_TYPED_ARRAYS=1']]:
      self.emcc_args = orig_args + mode
      self.do_run_from_file(src, output)

  def test_fgetc_ungetc(self):
    logging.warning('TODO: update this test once the musl ungetc-on-EOF-stream bug is fixed upstream and reaches us')
    Settings.SYSCALL_DEBUG = 1
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      print fs
      src = open(path_from_root('tests', 'stdio', 'test_fgetc_ungetc.c'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])

  def test_fgetc_unsigned(self):
    src = r'''
      #include <stdio.h>
      int main() {
        FILE *file = fopen("file_with_byte_234.txt", "rb");
        int c = fgetc(file);
        printf("*%d\n", c);
      }
    '''
    open('file_with_byte_234.txt', 'wb').write('\xea')
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
    open('eol.txt', 'wb').write('\n')
    self.emcc_args += ['--embed-file', 'eol.txt']
    self.do_run(src, 'SUCCESS\n')

  def test_fscanf(self):
    open(os.path.join(self.get_dir(), 'three_numbers.txt'), 'w').write('''-1 0.1 -.1''')
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
    open('a.txt', 'w').write('''1/2/3 4/5/6 7/8/9
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
    open(os.path.join(self.get_dir(), 'empty.txt'), 'w').write('')
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
    self.banned_js_engines = [V8_ENGINE] # stderr printing limitations in v8
    src = open(path_from_root('tests', 'dirent', 'test_readdir.c'), 'r').read()
    self.do_run(src, '''SIGILL: Illegal instruction
success
n: 8
name: tmp
name: proc
name: nocanread
name: home
name: foobar
name: dev
name: ..
name: .
''', force_c=True)

  def test_readdir_empty(self):
    src = open(path_from_root('tests', 'dirent', 'test_readdir_empty.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_stat(self):
    src = open(path_from_root('tests', 'stat', 'test_stat.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_stat_chmod(self):
    src = open(path_from_root('tests', 'stat', 'test_chmod.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_stat_mknod(self):
    src = open(path_from_root('tests', 'stat', 'test_mknod.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_fcntl(self):
    add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createDataFile('/', 'test', 'abcdef', true, true);"
  )
  open(filename, 'w').write(src)
'''
    src = open(path_from_root('tests', 'fcntl', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'fcntl', 'output.txt'), 'r').read()
    self.do_run(src, expected, post_build=add_pre_run, extra_emscripten_args=['-H', 'libc/fcntl.h'])

  def test_fcntl_open(self):
    src = open(path_from_root('tests', 'fcntl-open', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'fcntl-open', 'output.txt'), 'r').read()
    self.do_run(src, expected, force_c=True, extra_emscripten_args=['-H', 'libc/fcntl.h'])

  def test_fcntl_misc(self):
    add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createDataFile('/', 'test', 'abcdef', true, true);"
  )
  open(filename, 'w').write(src)
'''
    src = open(path_from_root('tests', 'fcntl-misc', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'fcntl-misc', 'output.txt'), 'r').read()
    self.do_run(src, expected, post_build=add_pre_run, extra_emscripten_args=['-H', 'libc/fcntl.h'])

  def test_poll(self):
    add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    \'\'\'
      var dummy_device = FS.makedev(64, 0);
      FS.registerDevice(dummy_device, {});

      FS.createDataFile('/', 'file', 'abcdef', true, true);
      FS.mkdev('/device', dummy_device);
    \'\'\'
  )
  open(filename, 'w').write(src)
'''
    test_path = path_from_root('tests', 'core', 'test_poll')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, post_build=add_pre_run, extra_emscripten_args=['-H', 'libc/fcntl.h,poll.h'])

  def test_statvfs(self):
    test_path = path_from_root('tests', 'core', 'test_statvfs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_libgen(self):
    test_path = path_from_root('tests', 'core', 'test_libgen')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_utime(self):
    src = open(path_from_root('tests', 'utime', 'test_utime.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_utf(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # only node handles utf well
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc']

    test_path = path_from_root('tests', 'core', 'test_utf')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_utf32(self):
    Settings.EXTRA_EXPORTED_RUNTIME_METHODS = ['UTF32ToString', 'stringToUTF32', 'lengthBytesUTF32']
    self.do_run(open(path_from_root('tests', 'utf32.cpp')).read(), 'OK.')
    self.do_run(open(path_from_root('tests', 'utf32.cpp')).read(), 'OK.', args=['-fshort-wchar'])

  def test_utf8(self):
    Building.COMPILER_TEST_OPTS += ['-std=c++11']
    self.do_run(open(path_from_root('tests', 'utf8.cpp')).read(), 'OK.')

  def test_wprintf(self):
    test_path = path_from_root('tests', 'core', 'test_wprintf')
    src, output = (test_path + s for s in ('.c', '.out'))
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'MEMFS_APPEND_TO_TYPED_ARRAYS=1']]:
      self.emcc_args = orig_args + mode
      self.do_run_from_file(src, output)

  def test_direct_string_constant_usage(self):
    test_path = path_from_root('tests', 'core', 'test_direct_string_constant_usage')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_std_cout_new(self):
    test_path = path_from_root('tests', 'core', 'test_std_cout_new')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_istream(self):
    test_path = path_from_root('tests', 'core', 'test_istream')
    src, output = (test_path + s for s in ('.in', '.out'))

    for linkable in [0]:#, 1]:
      print linkable
      Settings.LINKABLE = linkable # regression check for issue #273
      self.do_run_from_file(src, output)

  def test_fs_base(self):
    Settings.INCLUDE_FULL_LIBRARY = 1
    try:
      addJS = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace('FS.init();', '').replace( # Disable normal initialization, replace with ours
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'filesystem', 'src.js'), 'r').read())
  open(filename, 'w').write(src)
'''
      src = 'int main() {return 0;}\n'
      expected = open(path_from_root('tests', 'filesystem', 'output.txt'), 'r').read()
      self.do_run(src, expected, post_build=addJS, extra_emscripten_args=['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/langinfo.h,libc/time.h'])
    finally:
      Settings.INCLUDE_FULL_LIBRARY = 0

  def test_fs_nodefs_rw(self):
    Settings.SYSCALL_DEBUG = 1
    src = open(path_from_root('tests', 'fs', 'test_nodefs_rw.c'), 'r').read()
    self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])
    print 'closure'
    self.emcc_args += ['--closure', '1']
    self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])

  def test_fs_nodefs_cloexec(self):
    src = open(path_from_root('tests', 'fs', 'test_nodefs_cloexec.c'), 'r').read()
    self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])

  def test_fs_trackingdelegate(self):
    src = path_from_root('tests', 'fs', 'test_trackingdelegate.c')
    out = path_from_root('tests', 'fs', 'test_trackingdelegate.out')
    self.do_run_from_file(src, out)

  def test_fs_writeFile(self):
    self.emcc_args += ['-s', 'DISABLE_EXCEPTION_CATCHING=1'] # see issue 2334
    src = path_from_root('tests', 'fs', 'test_writeFile.cc')
    out = path_from_root('tests', 'fs', 'test_writeFile.out')
    self.do_run_from_file(src, out)

  def test_fs_emptyPath(self):
    src = path_from_root('tests', 'fs', 'test_emptyPath.c')
    out = path_from_root('tests', 'fs', 'test_emptyPath.out')
    self.do_run_from_file(src, out)

  def test_fs_append(self):
    src = open(path_from_root('tests', 'fs', 'test_append.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_fs_mmap(self):
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS']:
      src = path_from_root('tests', 'fs', 'test_mmap.c')
      out = path_from_root('tests', 'fs', 'test_mmap.out')
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run_from_file(src, out)

  def test_unistd_access(self):
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'access.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'access.out'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_curdir(self):
    src = open(path_from_root('tests', 'unistd', 'curdir.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'curdir.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_close(self):
    src = open(path_from_root('tests', 'unistd', 'close.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'close.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_confstr(self):
    src = open(path_from_root('tests', 'unistd', 'confstr.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'confstr.out'), 'r').read()
    self.do_run(src, expected, extra_emscripten_args=['-H', 'libc/unistd.h'])

  def test_unistd_ttyname(self):
    src = open(path_from_root('tests', 'unistd', 'ttyname.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_unistd_dup(self):
    src = open(path_from_root('tests', 'unistd', 'dup.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'dup.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_pathconf(self):
    src = open(path_from_root('tests', 'unistd', 'pathconf.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'pathconf.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_truncate(self):
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'truncate.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'truncate.out'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_swab(self):
    src = open(path_from_root('tests', 'unistd', 'swab.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'swab.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_isatty(self):
    src = open(path_from_root('tests', 'unistd', 'isatty.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_unistd_sysconf(self):
    src = open(path_from_root('tests', 'unistd', 'sysconf.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'sysconf.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_login(self):
    src = open(path_from_root('tests', 'unistd', 'login.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'login.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_unlink(self):
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'unlink.c'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      # symlinks on node.js on Windows require administrative privileges, so skip testing those bits on that combination.
      if WINDOWS and fs == 'NODEFS': Building.COMPILER_TEST_OPTS += ['-DNO_SYMLINK=1']
      self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])

  def test_unistd_links(self):
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      if WINDOWS and fs == 'NODEFS':
        print >> sys.stderr, 'Skipping NODEFS part of this test for test_unistd_links on Windows, since it would require administrative privileges.'
        # Also, other detected discrepancies if you do end up running this test on NODEFS:
        # test expects /, but Windows gives \ as path slashes.
        # Calling readlink() on a non-link gives error 22 EINVAL on Unix, but simply error 0 OK on Windows.
        continue
      src = open(path_from_root('tests', 'unistd', 'links.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'links.out'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_symlink_on_nodefs(self):
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['NODEFS']:
      if WINDOWS and fs == 'NODEFS':
        print >> sys.stderr, 'Skipping NODEFS part of this test for test_unistd_symlink_on_nodefs on Windows, since it would require administrative privileges.'
        # Also, other detected discrepancies if you do end up running this test on NODEFS:
        # test expects /, but Windows gives \ as path slashes.
        # Calling readlink() on a non-link gives error 22 EINVAL on Unix, but simply error 0 OK on Windows.
        continue
      src = open(path_from_root('tests', 'unistd', 'symlink_on_nodefs.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'symlink_on_nodefs.out'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_sleep(self):
    src = open(path_from_root('tests', 'unistd', 'sleep.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'sleep.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_io(self):
    self.clear()
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'io.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'io.out'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_misc(self):
    orig_compiler_opts = Building.COMPILER_TEST_OPTS[:]
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'misc.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'misc.out'), 'r').read()
      Building.COMPILER_TEST_OPTS = orig_compiler_opts + ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_posixtime(self):
    test_path = path_from_root('tests', 'core', 'test_posixtime')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.banned_js_engines = [V8_ENGINE] # v8 lacks monotonic time
    self.do_run_from_file(src, output)

    if V8_ENGINE in JS_ENGINES:
      self.banned_js_engines = filter(lambda engine: engine != V8_ENGINE, JS_ENGINES)
      self.do_run_from_file(src, test_path + '_no_monotonic.out')
    else:
      print '(no v8, skipping no-monotonic case)'

  def test_uname(self):
    test_path = path_from_root('tests', 'core', 'test_uname')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_env(self):
    src = open(path_from_root('tests', 'env', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'env', 'output.txt'), 'r').read()
    self.do_run(src, [
      expected.replace('{{{ THIS_PROGRAM }}}', os.path.join(self.get_dir(), 'src.cpp.o.js').replace('\\', '/')), # node, can find itself properly
      expected.replace('{{{ THIS_PROGRAM }}}', './this.program') # spidermonkey, v8
    ])

  def test_environ(self):
    src = open(path_from_root('tests', 'env', 'src-mini.c'), 'r').read()
    expected = open(path_from_root('tests', 'env', 'output-mini.txt'), 'r').read()
    self.do_run(src, [
      expected.replace('{{{ THIS_PROGRAM }}}', os.path.join(self.get_dir(), 'src.cpp.o.js').replace('\\', '/')), # node, can find itself properly
      expected.replace('{{{ THIS_PROGRAM }}}', './this.program') # spidermonkey, v8
    ])

  def test_systypes(self):
    src = open(path_from_root('tests', 'systypes', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'systypes', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_getloadavg(self):
    test_path = path_from_root('tests', 'core', 'test_getloadavg')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_nl_types(self):
    test_path = path_from_root('tests', 'core', 'test_nl_types')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_799(self):
    src = open(path_from_root('tests', '799.cpp'), 'r').read()
    self.do_run(src, '''Set PORT family: 0, port: 3979
Get PORT family: 0
PORT: 3979
''')

  def test_ctype(self):
    src = open(path_from_root('tests', 'ctype', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'ctype', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_strcasecmp(self):
    test_path = path_from_root('tests', 'core', 'test_strcasecmp')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_atomic(self):
    test_path = path_from_root('tests', 'core', 'test_atomic')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  def test_atomic_cxx(self):
    test_path = path_from_root('tests', 'core', 'test_atomic_cxx')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    Building.COMPILER_TEST_OPTS += ['-std=c++11']
    self.do_run_from_file(src, output)

    if Settings.ALLOW_MEMORY_GROWTH == 0 and not self.is_wasm():
      print 'main module'
      Settings.MAIN_MODULE = 1
      self.do_run_from_file(src, output)

  def test_phiundef(self):
    test_path = path_from_root('tests', 'core', 'test_phiundef')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_netinet_in(self):
    src = open(path_from_root('tests', 'netinet', 'in.cpp'), 'r').read()
    expected = open(path_from_root('tests', 'netinet', 'in.out'), 'r').read()
    self.do_run(src, expected)

  # libc++ tests

  def test_iostream_and_determinism(self):
    src = '''
      #include <iostream>

      int main()
      {
        std::cout << "hello world" << std::endl << 77 << "." << std::endl;
        return 0;
      }
    '''
    num = 5
    def test():
      print '(iteration)'
      time.sleep(random.random()/(10*num)) # add some timing nondeterminism here, not that we need it, but whatever
      self.do_run(src, 'hello world\n77.\n')
      return open('src.cpp.o.js').read()
    builds = map(lambda i: test(), range(num))
    print map(len, builds)
    uniques = set(builds)
    if len(uniques) != 1:
      i = 0
      for unique in uniques:
        open('unique_' + str(i) + '.js', 'w').write(unique)
        i += 1
      assert 0, 'builds must be deterministic, see unique_X.js'

  def test_stdvec(self):
    test_path = path_from_root('tests', 'core', 'test_stdvec')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_random_device(self):
    Building.COMPILER_TEST_OPTS += ['-std=c++11']

    test_path = path_from_root('tests', 'core', 'test_random_device')
    src, output = (test_path + s for s in ('.cpp', '.txt'))

    self.do_run_from_file(src, output)

  def test_reinterpreted_ptrs(self):
    test_path = path_from_root('tests', 'core', 'test_reinterpreted_ptrs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_js_libraries(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'mylib1.js'), 'w').write('''
      mergeInto(LibraryManager.library, {
        printey: function() {
          Module.print('hello from lib!');
        }
      });
    ''')
    open(os.path.join(self.get_dir(), 'mylib2.js'), 'w').write('''
      mergeInto(LibraryManager.library, {
        calcey: function(x, y) {
          return x + y;
        }
      });
    ''')

    self.emcc_args += ['--js-library', os.path.join(self.get_dir(), 'mylib1.js'), '--js-library', os.path.join(self.get_dir(), 'mylib2.js')]
    self.do_run(open(os.path.join(self.get_dir(), 'main.cpp'), 'r').read(), 'hello from lib!\n*32*\n')

  def test_unicode_js_library(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
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
    self.do_run(open(os.path.join(self.get_dir(), 'main.cpp'), 'r').read(), u'Unicode snowman \u2603 says hello!')

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
    results = [ (1,0), (2,1), (3,2), (4,4), (5,7), (6,10), (7, 16), (8,22) ]
    for i, j in results:
      src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read()
      self.do_run(src, 'Pfannkuchen(%d) = %d.' % (i,j), [str(i)], no_build=i>1)

  def test_raytrace(self):
      # TODO: Should we remove this test?
      return self.skip('Relies on double value rounding, extremely sensitive')

      src = open(path_from_root('tests', 'raytrace.cpp'), 'r').read().replace('double', 'float')
      output = open(path_from_root('tests', 'raytrace.ppm'), 'r').read()
      self.do_run(src, output, ['3', '16'])#, build_ll_hook=self.do_autodebug)

  def test_fasta(self):
      results = [ (1,'''GG*ctt**tgagc*'''), (20,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tacgtgtagcctagtgtttgtgttgcgttatagtctatttgtggacacagtatggtcaaa**tgacgtcttttgatctgacggcgttaacaaagatactctg*'''),
(50,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA*TCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACAT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tactDtDagcctatttSVHtHttKtgtHMaSattgWaHKHttttagacatWatgtRgaaa**NtactMcSMtYtcMgRtacttctWBacgaa**agatactctgggcaacacacatacttctctcatgttgtttcttcggacctttcataacct**ttcctggcacatggttagctgcacatcacaggattgtaagggtctagtggttcagtgagc**ggaatatcattcgtcggtggtgttaatctatctcggtgtagcttataaatgcatccgtaa**gaatattatgtttatttgtcggtacgttcatggtagtggtgtcgccgatttagacgtaaa**ggcatgtatg*''') ]
      for precision in [0, 1, 2]:
        Settings.PRECISE_F32 = precision
        for t in ['float', 'double']:
          print precision, t
          src = open(path_from_root('tests', 'fasta.cpp'), 'r').read().replace('double', t)
          for i, j in results:
            self.do_run(src, j, [str(i)], lambda x, err: x.replace('\n', '*'), no_build=i>1)
          shutil.copyfile('src.cpp.o.js', '%d_%s.js' % (precision, t))

  def test_whets(self):
    self.do_run(open(path_from_root('tests', 'whets.cpp')).read(), 'Single Precision C Whetstone Benchmark')

  def test_dlmalloc(self):
    self.banned_js_engines = [NODE_JS] # slower, and fail on 64-bit
    Settings.TOTAL_MEMORY = 128*1024*1024 # needed with typed arrays

    src = open(path_from_root('system', 'lib', 'dlmalloc.c'), 'r').read() + '\n\n\n' + open(path_from_root('tests', 'dlmalloc_test.c'), 'r').read()
    self.do_run(src, '*1,0*', ['200', '1'])
    self.do_run(src, '*400,0*', ['400', '400'], no_build=True)

    # Linked version
    src = open(path_from_root('tests', 'dlmalloc_test.c'), 'r').read()
    self.do_run(src, '*1,0*', ['200', '1'], extra_emscripten_args=['-m'])
    self.do_run(src, '*400,0*', ['400', '400'], extra_emscripten_args=['-m'], no_build=True)

    if self.emcc_args == []: # TODO: do this in other passes too, passing their opts into emcc
      # emcc should build in dlmalloc automatically, and do all the sign correction etc. for it

      try_delete(os.path.join(self.get_dir(), 'src.cpp.o.js'))
      output = Popen([PYTHON, EMCC, path_from_root('tests', 'dlmalloc_test.c'), '-s', 'TOTAL_MEMORY=' + str(128*1024*1024),
                      '-o', os.path.join(self.get_dir(), 'src.cpp.o.js')], stdout=PIPE, stderr=self.stderr_redirect).communicate()

      self.do_run('x', '*1,0*', ['200', '1'], no_build=True)
      self.do_run('x', '*400,0*', ['400', '400'], no_build=True)

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

  def test_dlmalloc_partial_2(self):
    if 'SAFE_HEAP' in str(self.emcc_args): return self.skip('we do unsafe stuff here')
    # present part of the symbols of dlmalloc, not all. malloc is harder to link than new which is weak.

    Settings.NO_EXIT_RUNTIME = 1 # if we exit, then we flush streams, but the bad malloc we install here messes that up

    test_path = path_from_root('tests', 'core', 'test_dlmalloc_partial_2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_libcxx(self):
    self.do_run(open(path_from_root('tests', 'hashtest.cpp')).read(),
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
      ''', 'hello world');

  def test_typeid(self):
    test_path = path_from_root('tests', 'core', 'test_typeid')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_static_variable(self):
    test_path = path_from_root('tests', 'core', 'test_static_variable')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_fakestat(self):
    test_path = path_from_root('tests', 'core', 'test_fakestat')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_mmap(self):
    Settings.TOTAL_MEMORY = 128*1024*1024

    test_path = path_from_root('tests', 'core', 'test_mmap')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)
    self.do_run_from_file(src, output, force_c=True)

  def test_mmap_file(self):
    for extra_args in [[], ['--no-heap-copy']]:
      self.emcc_args += ['--embed-file', 'data.dat'] + extra_args
      x = 'data from the file........'
      s = ''
      while len(s) < 9000:
        if len(s) + len(x) < 9000:
          s += x
          continue
        s += '.'
      assert len(s) == 9000
      open(self.in_dir('data.dat'), 'w').write(s)
      src = open(path_from_root('tests', 'mmap_file.c')).read()
      self.do_run(src, '*\n' + s[0:20] + '\n' + s[4096:4096+20] + '\n*\n')

  def test_cubescript(self):
    assert 'asm3' in test_modes
    if self.run_name == 'asm3':
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

    Building.COMPILER_TEST_OPTS = filter(lambda x: x != '-g', Building.COMPILER_TEST_OPTS) # remove -g, so we have one test without it by default

    def test():
      self.do_run(path_from_root('tests', 'cubescript'), '*\nTemp is 33\n9\n5\nhello, everyone\n*', main_file='command.cpp')

    test()

    assert 'asm1' in test_modes
    if self.run_name == 'asm1':
      print 'verifing postsets'
      generated = open('src.cpp.o.js').read()
      generated = re.sub(r'\n+[ \n]*\n+', '\n', generated)
      main = generated[generated.find('function runPostSets'):]
      main = main[:main.find('\n}')]
      assert main.count('\n') <= 7, ('must not emit too many postSets: %d' % main.count('\n')) + ' : ' + main

    print 'relocatable'
    assert Settings.RELOCATABLE == Settings.EMULATED_FUNCTION_POINTERS == 0
    Settings.RELOCATABLE = Settings.EMULATED_FUNCTION_POINTERS = 1
    test()
    Settings.RELOCATABLE = Settings.EMULATED_FUNCTION_POINTERS = 0

    if self.is_emterpreter():
      print 'emterpreter/async/assertions' # extra coverage
      self.emcc_args += ['-s', 'EMTERPRETIFY_ASYNC=1', '-s', 'ASSERTIONS=1']
      test()
      print 'emterpreter/async/assertions/whitelist'
      self.emcc_args += ['-s', 'EMTERPRETIFY_WHITELIST=["_frexpl"]'] # test double call assertions
      test()

    if not self.is_wasm():
      print 'split memory'
      Settings.SPLIT_MEMORY = 8*1024*1024
      test()
      Settings.SPLIT_MEMORY = 0

  @SIMD
  def test_sse1(self):
    if 'SAFE_HEAP=1' in self.emcc_args and SPIDERMONKEY_ENGINE in JS_ENGINES:
      self.banned_js_engines += [SPIDERMONKEY_ENGINE]
      print 'Skipping test_sse1 with SAFE_HEAP=1 on SpiderMonkey, since it fails due to NaN canonicalization.'
    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround

    orig_args = self.emcc_args
    for mode in [[], ['-s', 'SIMD=1']]:
      self.emcc_args = orig_args + mode + ['-msse']
      self.do_run(open(path_from_root('tests', 'test_sse1.cpp'), 'r').read(), 'Success!')

  # Tests the full SSE1 API.
  @SIMD
  def test_sse1_full(self):
    Popen([CLANG, path_from_root('tests', 'test_sse1_full.cpp'), '-o', 'test_sse1_full', '-D_CRT_SECURE_NO_WARNINGS=1'] + get_clang_native_args(), env=get_clang_native_env(), stdout=PIPE).communicate()
    native_result, err = Popen('./test_sse1_full', stdout=PIPE).communicate()
    native_result = native_result.replace('\r\n', '\n') # Windows line endings fix

    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'SIMD=1']]:
      self.emcc_args = orig_args + mode + ['-I' + path_from_root('tests'), '-msse']
      self.do_run(open(path_from_root('tests', 'test_sse1_full.cpp'), 'r').read(), native_result)

  # Tests the full SSE2 API.
  @SIMD
  def test_sse2_full(self):
    if self.run_name == 'asm1': return self.skip("some i64 thing we can't legalize yet. possible hint: optimize with -O0 or -O2+, and not -O1");
    import platform
    is_64bits = platform.architecture()[0] == '64bit'
    if not is_64bits: return self.skip('This test requires 64-bit system, since it tests SSE2 intrinsics only available in 64-bit mode!')

    args = []
    if '-O0' in self.emcc_args: args += ['-D_DEBUG=1']
    Popen([CLANG, path_from_root('tests', 'test_sse2_full.cpp'), '-o', 'test_sse2_full', '-D_CRT_SECURE_NO_WARNINGS=1'] + args + get_clang_native_args(), env=get_clang_native_env(), stdout=PIPE).communicate()
    native_result, err = Popen('./test_sse2_full', stdout=PIPE).communicate()
    native_result = native_result.replace('\r\n', '\n') # Windows line endings fix

    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'SIMD=1']]:
      self.emcc_args = orig_args + mode + ['-I' + path_from_root('tests'), '-msse2'] + args
      self.do_run(open(path_from_root('tests', 'test_sse2_full.cpp'), 'r').read(), native_result)

  # Tests the full SSE3 API.
  @SIMD
  def test_sse3_full(self):
    args = []
    if '-O0' in self.emcc_args: args += ['-D_DEBUG=1']
    Popen([CLANG, path_from_root('tests', 'test_sse3_full.cpp'), '-o', 'test_sse3_full', '-D_CRT_SECURE_NO_WARNINGS=1', '-msse3'] + args + get_clang_native_args(), env=get_clang_native_env(), stdout=PIPE).communicate()
    native_result, err = Popen('./test_sse3_full', stdout=PIPE).communicate()
    native_result = native_result.replace('\r\n', '\n') # Windows line endings fix

    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'SIMD=1']]:
      self.emcc_args = orig_args + mode + ['-I' + path_from_root('tests'), '-msse3'] + args
      self.do_run(open(path_from_root('tests', 'test_sse3_full.cpp'), 'r').read(), native_result)

  @SIMD
  def test_ssse3_full(self):
    args = []
    if '-O0' in self.emcc_args: args += ['-D_DEBUG=1']
    Popen([CLANG, path_from_root('tests', 'test_ssse3_full.cpp'), '-o', 'test_ssse3_full', '-D_CRT_SECURE_NO_WARNINGS=1', '-mssse3'] + args + get_clang_native_args(), env=get_clang_native_env(), stdout=PIPE).communicate()
    native_result, err = Popen('./test_ssse3_full', stdout=PIPE).communicate()
    native_result = native_result.replace('\r\n', '\n') # Windows line endings fix

    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'SIMD=1']]:
      self.emcc_args = orig_args + mode + ['-I' + path_from_root('tests'), '-mssse3'] + args
      self.do_run(open(path_from_root('tests', 'test_ssse3_full.cpp'), 'r').read(), native_result)

  @SIMD
  def test_sse4_1_full(self):
    args = []
    if '-O0' in self.emcc_args: args += ['-D_DEBUG=1']
    Popen([CLANG, path_from_root('tests', 'test_sse4_1_full.cpp'), '-o', 'test_sse4_1_full', '-D_CRT_SECURE_NO_WARNINGS=1', '-msse4.1'] + args + get_clang_native_args(), env=get_clang_native_env(), stdout=PIPE).communicate()
    native_result, err = Popen('./test_sse4_1_full', stdout=PIPE).communicate()
    native_result = native_result.replace('\r\n', '\n') # Windows line endings fix

    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround
    orig_args = self.emcc_args
    for mode in [[], ['-s', 'SIMD=1']]:
      self.emcc_args = orig_args + mode + ['-I' + path_from_root('tests'), '-msse4.1'] + args
      self.do_run(open(path_from_root('tests', 'test_sse4_1_full.cpp'), 'r').read(), native_result)

  @SIMD
  def test_simd(self):
    test_path = path_from_root('tests', 'core', 'test_simd')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd2(self):
    test_path = path_from_root('tests', 'core', 'test_simd2')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd3(self):
    Settings.PRECISE_F32 = 1 # SIMD currently requires Math.fround
    test_path = path_from_root('tests', 'core', 'test_simd3')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse2']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd4(self):
    # test_simd4 is to test phi node handling of SIMD path
    test_path = path_from_root('tests', 'core', 'test_simd4')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd5(self):
    # test_simd5 is to test shufflevector of SIMD path
    test_path = path_from_root('tests', 'core', 'test_simd5')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  @SIMD
  def test_simd6(self):
    # test_simd6 is to test x86 min and max intrinsics on NaN and -0.0
    test_path = path_from_root('tests', 'core', 'test_simd6')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd7(self):
    # test_simd7 is to test negative zero handling: https://github.com/kripken/emscripten/issues/2791
    test_path = path_from_root('tests', 'core', 'test_simd7')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd8(self):
    # test_simd8 is to test unaligned load and store
    test_path = path_from_root('tests', 'core', 'test_simd8')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd9(self):
    # test_simd9 is to test a bug where _mm_set_ps(0.f) would generate an expression that did not validate as asm.js
    test_path = path_from_root('tests', 'core', 'test_simd9')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd10(self):
    # test_simd10 is to test that loading and storing arbitrary bit patterns works in SSE1.
    test_path = path_from_root('tests', 'core', 'test_simd10')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd11(self):
    # test_simd11 is to test that _mm_movemask_ps works correctly when handling input floats with 0xFFFFFFFF NaN bit patterns.
    test_path = path_from_root('tests', 'core', 'test_simd11')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse2']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd12(self):
    test_path = path_from_root('tests', 'core', 'test_simd12')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd13(self):
    test_path = path_from_root('tests', 'core', 'test_simd13')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd14(self):
    test_path = path_from_root('tests', 'core', 'test_simd14')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.emcc_args = self.emcc_args + ['-msse', '-msse2']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd15(self):
    if self.run_name == 'asm1': return self.skip('legalizing -O1 output is much harder, and not worth it - we work on -O0 and -O2+')
    test_path = path_from_root('tests', 'core', 'test_simd15')
    src, output = (test_path + s for s in ('.c', '.out'))
    self.emcc_args = self.emcc_args + ['-msse', '-msse2']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd16(self):
    src, output = (path_from_root('tests', 'core', 'test_simd16') + s for s in ('.c', '.out'))
    self.emcc_args = self.emcc_args + ['-msse', '-msse2']
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd_float64x2(self):
    src, output = (path_from_root('tests', 'core', 'test_simd_float64x2') + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd_float32x4(self):
    src, output = (path_from_root('tests', 'core', 'test_simd_float32x4') + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd_int32x4(self):
    src, output = (path_from_root('tests', 'core', 'test_simd_int32x4') + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd_int16x8(self):
    src, output = (path_from_root('tests', 'core', 'test_simd_int16x8') + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd_int8x16(self):
    src, output = (path_from_root('tests', 'core', 'test_simd_int8x16') + s for s in ('.c', '.out'))
    self.do_run_from_file(src, output)

  @SIMD
  def test_simd_dyncall(self):
    test_path = path_from_root('tests', 'core', 'test_simd_dyncall')
    src, output = (test_path + s for s in ('.cpp', '.txt'))
    self.emcc_args = self.emcc_args + ['-msse']
    self.do_run_from_file(src, output)

  def test_gcc_unmangler(self):
    Building.COMPILER_TEST_OPTS += ['-I' + path_from_root('third_party')]

    self.do_run(open(path_from_root('third_party', 'gcc_demangler.c')).read(), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'])

  def test_lua(self):
    if self.emcc_args: self.emcc_args = ['-g1'] + self.emcc_args

    total_memory = Settings.TOTAL_MEMORY

    if self.is_emterpreter():
      Settings.PRECISE_F32 = 1

    for aggro in ([0, 1] if Settings.ASM_JS and '-O2' in self.emcc_args else [0]):
      Settings.AGGRESSIVE_VARIABLE_ELIMINATION = aggro
      Settings.TOTAL_MEMORY = total_memory
      print aggro
      self.do_run('',
                  'hello lua world!\n17\n1\n2\n3\n4\n7',
                  args=['-e', '''print("hello lua world!");print(17);for x = 1,4 do print(x) end;print(10-3)'''],
                  libraries=self.get_library('lua', [os.path.join('src', 'lua'), os.path.join('src', 'liblua.a')], make=['make', 'generic'], configure=None),
                  includes=[path_from_root('tests', 'lua')],
                  output_nicerizer=lambda string, err: (string + err).replace('\n\n', '\n').replace('\n\n', '\n'))

  def get_freetype(self):
    Settings.DEAD_FUNCTIONS += ['_inflateEnd', '_inflate', '_inflateReset', '_inflateInit2_']

    return self.get_library('freetype',
                            os.path.join('objs', '.libs', 'libfreetype.a'))

  def test_freetype(self):
    if WINDOWS: return self.skip('test_freetype uses a ./configure script to build and therefore currently only runs on Linux and OS X.')
    assert 'asm2g' in test_modes
    if self.run_name == 'asm2g':
      Settings.ALIASING_FUNCTION_POINTERS = 1 - Settings.ALIASING_FUNCTION_POINTERS # flip for some more coverage here

    post = '''
def process(filename):
  import tools.shared as shared
  # Embed the font into the document
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createDataFile('/', 'font.ttf', %s, true, false);" % str(
      map(ord, open(shared.path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), 'rb').read())
    )
  )
  open(filename, 'w').write(src)
'''

    # Not needed for js, but useful for debugging
    shutil.copyfile(path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), os.path.join(self.get_dir(), 'font.ttf'))

    # Main
    for outlining in [0, 5000]:
      Settings.OUTLINING_LIMIT = outlining
      print >> sys.stderr, 'outlining:', outlining
      self.do_run(open(path_from_root('tests', 'freetype', 'main.c'), 'r').read(),
                   open(path_from_root('tests', 'freetype', 'ref.txt'), 'r').read(),
                   ['font.ttf', 'test!', '150', '120', '25'],
                   libraries=self.get_freetype(),
                   includes=[path_from_root('tests', 'freetype', 'include')],
                   post_build=post)

    # github issue 324
    print '[issue 324]'
    self.do_run(open(path_from_root('tests', 'freetype', 'main_2.c'), 'r').read(),
                 open(path_from_root('tests', 'freetype', 'ref_2.txt'), 'r').read(),
                 ['font.ttf', 'w', '32', '32', '25'],
                 libraries=self.get_freetype(),
                 includes=[path_from_root('tests', 'freetype', 'include')],
                 post_build=post)

    print '[issue 324 case 2]'
    self.do_run(open(path_from_root('tests', 'freetype', 'main_3.c'), 'r').read(),
                 open(path_from_root('tests', 'freetype', 'ref_3.txt'), 'r').read(),
                 ['font.ttf', 'W', '32', '32', '0'],
                 libraries=self.get_freetype(),
                 includes=[path_from_root('tests', 'freetype', 'include')],
                 post_build=post)

    print '[issue 324 case 3]'
    self.do_run('',
                 open(path_from_root('tests', 'freetype', 'ref_4.txt'), 'r').read(),
                 ['font.ttf', 'ea', '40', '32', '0'],
                 no_build=True)

  def test_sqlite(self):
    # gcc -O3 -I/home/alon/Dev/emscripten/tests/sqlite -ldl src.c
    self.banned_js_engines = [NODE_JS] # OOM in older node
    if '-O' not in str(self.emcc_args):
      self.banned_js_engines += [SPIDERMONKEY_ENGINE] # SM bug 1066759
    if self.is_split_memory(): return self.skip('SM bug 1205121')

    Settings.DISABLE_EXCEPTION_CATCHING = 1
    Settings.EXPORTED_FUNCTIONS += ['_sqlite3_open', '_sqlite3_close', '_sqlite3_exec', '_sqlite3_free', '_callback'];
    if Settings.ASM_JS == 1 and '-g' in self.emcc_args:
      print "disabling inlining" # without registerize (which -g disables), we generate huge amounts of code
      Settings.INLINING_LIMIT = 50

    #Settings.OUTLINING_LIMIT = 60000

    self.do_run(r'''
                      #define SQLITE_DISABLE_LFS
                      #define LONGDOUBLE_TYPE double
                      #define SQLITE_INT64_TYPE long long int
                      #define SQLITE_THREADSAFE 0
                 ''' + open(path_from_root('tests', 'sqlite', 'sqlite3.c'), 'r').read() +
                       open(path_from_root('tests', 'sqlite', 'benchmark.c'), 'r').read(),
                 open(path_from_root('tests', 'sqlite', 'benchmark.txt'), 'r').read(),
                 includes=[path_from_root('tests', 'sqlite')],
                 force_c=True)

  def test_zlib(self):
    if '-O2' in self.emcc_args and 'ASM_JS=0' not in self.emcc_args and not self.is_wasm(): # without asm, closure minifies Math.imul badly
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

    assert 'asm2g' in test_modes
    if self.run_name == 'asm2g':
      self.emcc_args += ['-g4'] # more source maps coverage

    use_cmake_configure = WINDOWS
    if use_cmake_configure:
      make_args = []
      configure = [PYTHON, path_from_root('emcmake'), 'cmake', '.', '-DBUILD_SHARED_LIBS=OFF']
    else:
      make_args = ['libz.a']
      configure = ['sh', './configure']

    self.do_run(open(path_from_root('tests', 'zlib', 'example.c'), 'r').read(),
                 open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                 libraries=self.get_library('zlib', os.path.join('libz.a'), make_args=make_args, configure=configure),
                 includes=[path_from_root('tests', 'zlib'), os.path.join(self.get_dir(), 'building', 'zlib')],
                 force_c=True)

  def test_the_bullet(self): # Called thus so it runs late in the alphabetical cycle... it is long
    Settings.DEAD_FUNCTIONS = ['__ZSt9terminatev']

    asserts = Settings.ASSERTIONS

    for use_cmake in [False, True]: # If false, use a configure script to configure Bullet build.
      print 'cmake', use_cmake
      # Windows cannot run configure sh scripts.
      if WINDOWS and not use_cmake:
        continue

      Settings.ASSERTIONS = 2 if use_cmake else asserts # extra testing for ASSERTIONS == 2

      def test():
        self.do_run(open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'), 'r').read(),
                     [open(path_from_root('tests', 'bullet', 'output.txt'), 'r').read(), # different roundings
                      open(path_from_root('tests', 'bullet', 'output2.txt'), 'r').read(),
                      open(path_from_root('tests', 'bullet', 'output3.txt'), 'r').read(),
                      open(path_from_root('tests', 'bullet', 'output4.txt'), 'r').read()],
                     libraries=get_bullet_library(self, use_cmake),
                     includes=[path_from_root('tests', 'bullet', 'src')])
      test()

      # TODO: test only worked in non-fastcomp (well, this section)
      continue
      assert 'asm2g' in test_modes
      if self.run_name == 'asm2g' and not use_cmake:
        # Test forced alignment
        print >> sys.stderr, 'testing FORCE_ALIGNED_MEMORY'
        old = open('src.cpp.o.js').read()
        Settings.FORCE_ALIGNED_MEMORY = 1
        test()
        new = open('src.cpp.o.js').read()
        print len(old), len(new), old.count('tempBigInt'), new.count('tempBigInt')
        assert len(old) > len(new)
        assert old.count('tempBigInt') > new.count('tempBigInt')

  def test_poppler(self):
    if WINDOWS: return self.skip('test_poppler depends on freetype, which uses a ./configure script to build and therefore currently only runs on Linux and OS X.')

    def test():
      Settings.NO_EXIT_RUNTIME = 1

      Building.COMPILER_TEST_OPTS += [
        '-I' + path_from_root('tests', 'freetype', 'include'),
        '-I' + path_from_root('tests', 'poppler', 'include')
      ]

      Settings.INVOKE_RUN = 0 # We append code that does run() ourselves

      # See post(), below
      input_file = open(os.path.join(self.get_dir(), 'paper.pdf.js'), 'w')
      input_file.write(str(map(ord, open(path_from_root('tests', 'poppler', 'paper.pdf'), 'rb').read())))
      input_file.close()

      post = '''
def process(filename):
  # To avoid loading this large file to memory and altering it, we simply append to the end
  src = open(filename, 'a')
  src.write(
    \'\'\'
      FS.createDataFile('/', 'paper.pdf', eval(Module.read('paper.pdf.js')), true, false);
      Module.callMain(Module.arguments);
      Module.print("Data: " + JSON.stringify(MEMFS.getFileDataAsRegularArray(FS.root.contents['filename-1.ppm']).map(function(x) { return unSign(x, 8) })));
    \'\'\'
  )
  src.close()
'''
 
 #fontconfig = self.get_library('fontconfig', [os.path.join('src', '.libs', 'libfontconfig.a')]) # Used in file, but not needed, mostly

      freetype = self.get_freetype()

      poppler = self.get_library('poppler',
                                 [os.path.join('utils', 'pdftoppm.o'),
                                  os.path.join('utils', 'parseargs.o'),
                                  os.path.join('poppler', '.libs', 'libpoppler.a')],
                                 env_init={ 'FONTCONFIG_CFLAGS': ' ', 'FONTCONFIG_LIBS': ' ' },
                                 configure_args=['--disable-libjpeg', '--disable-libpng', '--disable-poppler-qt', '--disable-poppler-qt4', '--disable-cms', '--disable-cairo-output', '--disable-abiword-output', '--enable-shared=no'])

      # Combine libraries

      combined = os.path.join(self.get_dir(), 'poppler-combined.bc')
      Building.link(poppler + freetype, combined)

      self.do_ll_run(combined,
                     map(ord, open(path_from_root('tests', 'poppler', 'ref.ppm'), 'r').read()).__str__().replace(' ', ''),
                     args='-scale-to 512 paper.pdf filename'.split(' '),
                     post_build=post)
                     #, build_ll_hook=self.do_autodebug)

    test()
    num_original_funcs = self.count_funcs('src.cpp.o.js')

    # Run with duplicate function elimination turned on
    dfe_supported_opt_levels = ['-O2', '-O3', '-Oz', '-Os']

    for opt_level in dfe_supported_opt_levels:
      if opt_level in self.emcc_args:
        print >> sys.stderr, "Testing poppler with ELIMINATE_DUPLICATE_FUNCTIONS set to 1"
        Settings.ELIMINATE_DUPLICATE_FUNCTIONS = 1
        test()

        # Make sure that DFE ends up eliminating more than 200 functions (if we can view source)
        if not self.is_wasm():
          assert (num_original_funcs - self.count_funcs('src.cpp.o.js')) > 200
        break

  def test_openjpeg(self):
    Building.COMPILER_TEST_OPTS = filter(lambda x: x != '-g', Building.COMPILER_TEST_OPTS) # remove -g, so we have one test without it by default

    post = '''
def process(filename):
  import tools.shared as shared
  original_j2k = shared.path_from_root('tests', 'openjpeg', 'syntensity_lobby_s.j2k')
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createDataFile('/', 'image.j2k', %s, true, false);" % shared.line_splitter(str(
      map(ord, open(original_j2k, 'rb').read())
    ))
  ).replace(
    '// {{POST_RUN_ADDITIONS}}',
    "Module.print('Data: ' + JSON.stringify(MEMFS.getFileDataAsRegularArray(FS.analyzePath('image.raw').object)));"
  )
  open(filename, 'w').write(src)
'''

    shutil.copy(path_from_root('tests', 'openjpeg', 'opj_config.h'), self.get_dir())

    lib = self.get_library('openjpeg',
                           [os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/index.c.o'.split('/')),
                            os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/convert.c.o'.split('/')),
                            os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/__/common/color.c.o'.split('/')),
                            os.path.join('bin', 'libopenjpeg.so.1.4.0')],
                           configure=['cmake', '.'],
                           #configure_args=['--enable-tiff=no', '--enable-jp3d=no', '--enable-png=no'],
                           make_args=[]) # no -j 2, since parallel builds can fail

    # We use doubles in JS, so we get slightly different values than native code. So we
    # check our output by comparing the average pixel difference
    def image_compare(output, err):
      # Get the image generated by JS, from the JSON.stringify'd array
      m = re.search('\[[\d, -]*\]', output)
      try:
        js_data = eval(m.group(0))
      except AttributeError:
        print 'Failed to find proper image output in: ' + output
        raise

      js_data = map(lambda x: x if x >= 0 else 256+x, js_data) # Our output may be signed, so unsign it

      # Get the correct output
      true_data = open(path_from_root('tests', 'openjpeg', 'syntensity_lobby_s.raw'), 'rb').read()

      # Compare them
      assert(len(js_data) == len(true_data))
      num = len(js_data)
      diff_total = js_total = true_total = 0
      for i in range(num):
        js_total += js_data[i]
        true_total += ord(true_data[i])
        diff_total += abs(js_data[i] - ord(true_data[i]))
      js_mean = js_total/float(num)
      true_mean = true_total/float(num)
      diff_mean = diff_total/float(num)

      image_mean = 83.265
      #print '[image stats:', js_mean, image_mean, true_mean, diff_mean, num, ']'
      assert abs(js_mean - image_mean) < 0.01
      assert abs(true_mean - image_mean) < 0.01
      assert diff_mean < 0.01

      return output

    self.emcc_args += ['--minify', '0'] # to compare the versions

    Settings.NO_EXIT_RUNTIME = 1

    def do_test():
      self.do_run(open(path_from_root('tests', 'openjpeg', 'codec', 'j2k_to_image.c'), 'r').read(),
                   'Successfully generated', # The real test for valid output is in image_compare
                   '-i image.j2k -o image.raw'.split(' '),
                   libraries=lib,
                   includes=[path_from_root('tests', 'openjpeg', 'libopenjpeg'),
                             path_from_root('tests', 'openjpeg', 'codec'),
                             path_from_root('tests', 'openjpeg', 'common'),
                             os.path.join(self.get_build_dir(), 'openjpeg')],
                   force_c=True,
                   post_build=post,
                   output_nicerizer=image_compare)#, build_ll_hook=self.do_autodebug)

    do_test()

    # some test coverage for EMCC_DEBUG 1 and 2
    assert 'asm2g' in test_modes
    if self.run_name == 'asm2g':
      shutil.copyfile('src.c.o.js', 'release.js')
      try:
        os.environ['EMCC_DEBUG'] = '1'
        print '2'
        do_test()
        shutil.copyfile('src.c.o.js', 'debug1.js')
        os.environ['EMCC_DEBUG'] = '2'
        print '3'
        do_test()
        shutil.copyfile('src.c.o.js', 'debug2.js')
      finally:
        del os.environ['EMCC_DEBUG']
      for debug in [1,2]:
        def clean(text):
          text = text.replace('\n\n', '\n').replace('\n\n', '\n').replace('\n\n', '\n').replace('\n\n', '\n').replace('\n\n', '\n').replace('{\n}', '{}')
          return '\n'.join(sorted(text.split('\n')))
        sizes = len(open('release.js').read()), len(open('debug%d.js' % debug).read())
        print >> sys.stderr, debug, 'sizes', sizes
        assert abs(sizes[0] - sizes[1]) < 0.001*sizes[0], sizes # we can't check on identical output, compilation is not 100% deterministic (order of switch elements, etc.), but size should be ~identical
        print >> sys.stderr, 'debug check %d passed too' % debug

      try:
        os.environ['EMCC_FORCE_STDLIBS'] = '1'
        print 'EMCC_FORCE_STDLIBS'
        do_test()
      finally:
        del os.environ['EMCC_FORCE_STDLIBS']
      print >> sys.stderr, 'EMCC_FORCE_STDLIBS ok'

      try_delete(CANONICAL_TEMP_DIR)
    else:
      print >> sys.stderr, 'not doing debug check'

    if Settings.ALLOW_MEMORY_GROWTH == 1: # extra testing
      print >> sys.stderr, 'no memory growth'
      Settings.ALLOW_MEMORY_GROWTH = 0
      do_test()

  def test_python(self):
    Settings.EMULATE_FUNCTION_POINTER_CASTS = 1

    bitcode = path_from_root('tests', 'python', 'python.bc')
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
      print 'lto:', lto
      if lto == 1: self.emcc_args += ['--llvm-lto', '1']
      self.do_ll_run(bitcode, pyoutput, args=['-S', '-c', pyscript])

  def test_lifetime(self):
    self.do_ll_run(path_from_root('tests', 'lifetime.ll'), 'hello, world!\n')
    if '-O1' in self.emcc_args or '-O2' in self.emcc_args:
      assert 'a18' not in open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read(), 'lifetime stuff and their vars must be culled'

  # Test cases in separate files. Note that these files may contain invalid .ll!
  # They are only valid enough for us to read for test purposes, not for llvm-as
  # to process.
  def test_cases(self):
    if Building.LLVM_OPTS: return self.skip("Our code is not exactly 'normal' llvm assembly")

    emcc_args = self.emcc_args

    # The following tests link to libc, and must be run with EMCC_LEAVE_INPUTS_RAW = 0
    need_no_leave_inputs_raw = ['muli33_ta2', 'philoop_ta2', 'uadd_overflow_64_ta2', 'i64toi8star', 'legalizer_ta2', 'quotedlabel', 'alignedunaligned', 'sillybitcast', 'invokeundef', 'loadbitcastgep', 'sillybitcast2', 'legalizer_b_ta2', 'emptystruct', 'entry3', 'atomicrmw_i64', 'atomicrmw_b_i64']

    try:
      import random
      names = glob.glob(path_from_root('tests', 'cases', '*.ll'))
      #random.shuffle(names)
      for name in names:
        shortname = name.replace('.ll', '')
        if '' not in shortname: continue
        # TODO: test only worked in non-fastcomp (well, these cases)
        if os.path.basename(shortname) in [
          'aliasbitcast', 'structparam', 'issue_39', 'phinonexist', 'oob_ta2', 'phiself', 'invokebitcast', # invalid ir
          'structphiparam', 'callwithstructural_ta2', 'callwithstructural64_ta2', 'structinparam', # pnacl limitations in ExpandStructRegs
          '2xi40', # pnacl limitations in ExpandGetElementPtr
          'quoted', # current fastcomp limitations FIXME
          'atomicrmw_unaligned', # TODO XXX
        ]: continue
        if self.is_emterpreter() and os.path.basename(shortname) in [
          'funcptr', # test writes to memory we store out bytecode! test is invalid
          'i1282vecnback', # uses simd
        ]:
          continue
        if self.is_wasm() and os.path.basename(shortname) in [
          'i1282vecnback', # uses simd
        ]:
          continue
        if os.path.basename(shortname) in need_no_leave_inputs_raw:
          if 'EMCC_LEAVE_INPUTS_RAW' in os.environ: del os.environ['EMCC_LEAVE_INPUTS_RAW']
          Settings.NO_FILESYSTEM = 0
        else:
          os.environ['EMCC_LEAVE_INPUTS_RAW'] = '1'
          Settings.NO_FILESYSTEM = 1 # no libc is linked in; with NO_FILESYSTEM we have a chance at printfing anyhow

        if '_noasm' in shortname and Settings.ASM_JS:
          print self.skip('case "%s" not relevant for asm.js' % shortname)
          continue
        self.emcc_args = emcc_args
        if os.path.exists(shortname + '.emcc'):
          if not self.emcc_args: continue
          self.emcc_args = self.emcc_args + json.loads(open(shortname + '.emcc').read())
        print >> sys.stderr, "Testing case '%s'..." % shortname
        output_file = path_from_root('tests', 'cases', shortname + '.txt')
        if os.path.exists(output_file):
          output = open(output_file, 'r').read()
        else:
          output = 'hello, world!'
        if output.rstrip() != 'skip':
          self.do_ll_run(path_from_root('tests', 'cases', name), output)
        # Optional source checking, a python script that gets a global generated with the source
        src_checker = path_from_root('tests', 'cases', shortname + '.py')
        if os.path.exists(src_checker):
          generated = open('src.cpp.o.js').read()
          exec(open(src_checker).read())

    finally:
      if 'EMCC_LEAVE_INPUTS_RAW' in os.environ: del os.environ['EMCC_LEAVE_INPUTS_RAW']
      self.emcc_args = emcc_args

  def test_fuzz(self):
    Building.COMPILER_TEST_OPTS += ['-I' + path_from_root('tests', 'fuzz', 'include'), '-w']

    def run_all(x):
      print x
      for name in glob.glob(path_from_root('tests', 'fuzz', '*.c')) + glob.glob(path_from_root('tests', 'fuzz', '*.cpp')):
        #if os.path.basename(name) != '4.c': continue
        if 'newfail' in name: continue
        if os.path.basename(name).startswith('temp_fuzzcode'): continue
        if x == 'lto' and self.run_name in ['default', 'asm2f'] and os.path.basename(name) in [
          '8.c' # pnacl legalization issue, see https://code.google.com/p/nativeclient/issues/detail?id=4027
        ]: continue
        if x == 'lto' and self.run_name == 'default' and os.path.basename(name) in [
          '19.c', '18.cpp', # LLVM LTO bug
          '23.cpp', # puts exists before LTO, but is not used; LTO cleans it out, but then creates uses to it (printf=>puts) XXX https://llvm.org/bugs/show_bug.cgi?id=23814
        ]: continue
        if x == 'lto' and os.path.basename(name) in [
          '21.c'
        ]: continue # LLVM LTO bug

        print name
        self.do_run(open(path_from_root('tests', 'fuzz', name)).read(),
                    open(path_from_root('tests', 'fuzz', name + '.txt')).read(), force_c=name.endswith('.c'))

    run_all('normal')

    self.emcc_args += ['--llvm-lto', '1']

    run_all('lto')

  # Autodebug the code
  def do_autodebug(self, filename):
    Building.llvm_dis(filename)
    output = Popen([PYTHON, AUTODEBUGGER, filename+'.o.ll', filename+'.o.ll.ll'], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
    assert 'Success.' in output, output
    self.prep_ll_run(filename, filename+'.o.ll.ll', force_recompile=True) # rebuild .bc # TODO: use code in do_autodebug_post for this

  # Autodebug the code, after LLVM opts. Will only work once!
  def do_autodebug_post(self, filename):
    if not hasattr(self, 'post'):
      print 'Asking for post re-call'
      self.post = True
      return True
    print 'Autodebugging during post time'
    delattr(self, 'post')
    output = Popen([PYTHON, AUTODEBUGGER, filename+'.o.ll', filename+'.o.ll.ll'], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
    assert 'Success.' in output, output
    shutil.copyfile(filename + '.o.ll.ll', filename + '.o.ll')
    Building.llvm_as(filename)
    Building.llvm_dis(filename)

  def test_autodebug(self):
    if Building.LLVM_OPTS: return self.skip('LLVM opts mess us up')
    Building.COMPILER_TEST_OPTS += ['--llvm-opts', '0']

    # Run a test that should work, generating some code
    test_path = path_from_root('tests', 'core', 'test_structs')
    src, output = (test_path + s for s in ('.in', '.out'))
    self.do_run_from_file(src, output, build_ll_hook=lambda x: False) # add an ll hook, to force ll generation

    filename = os.path.join(self.get_dir(), 'src.cpp')
    self.do_autodebug(filename)

    # Compare to each other, and to expected output
    self.do_ll_run(path_from_root('tests', filename+'.o.ll.ll'), '''AD:-1,1''')
    assert open('stdout').read().startswith('AD:-1'), 'We must note when we enter functions'

    # Test using build_ll_hook
    src = '''
        #include <stdio.h>

        char cache[256], *next = cache;

        int main()
        {
          cache[10] = 25;
          next[20] = 51;
          int x = cache[10];
          double y = 11.52;
          printf("*%d,%d,%.2f*\\n", x, cache[20], y);
          return 0;
        }
      '''
    self.do_run(src, '''AD:-1,1''', build_ll_hook=self.do_autodebug)

  ### Integration tests

  def test_ccall(self):
    post = '''
def process(filename):
  src = open(filename, 'r').read() + \'\'\'
      Module.print('*');
      var ret;
      ret = Module['ccall']('get_int', 'number'); Module.print([typeof ret, ret].join(','));
      ret = ccall('get_float', 'number'); Module.print([typeof ret, ret.toFixed(2)].join(','));
      ret = ccall('get_string', 'string'); Module.print([typeof ret, ret].join(','));
      ret = ccall('print_int', null, ['number'], [12]); Module.print(typeof ret);
      ret = ccall('print_float', null, ['number'], [14.56]); Module.print(typeof ret);
      ret = ccall('print_string', null, ['string'], ["cheez"]); Module.print(typeof ret);
      ret = ccall('print_string', null, ['array'], [[97, 114, 114, 45, 97, 121, 0]]); Module.print(typeof ret);
      ret = ccall('multi', 'number', ['number', 'number', 'number', 'string'], [2, 1.4, 3, 'more']); Module.print([typeof ret, ret].join(','));
      var p = ccall('malloc', 'pointer', ['number'], [4]);
      setValue(p, 650, 'i32');
      ret = ccall('pointer', 'pointer', ['pointer'], [p]); Module.print([typeof ret, getValue(ret, 'i32')].join(','));
      Module.print('*');
      // part 2: cwrap
      var noThirdParam = Module['cwrap']('get_int', 'number');
      Module.print(noThirdParam());
      var multi = Module['cwrap']('multi', 'number', ['number', 'number', 'number', 'string']);
      Module.print(multi(2, 1.4, 3, 'atr'));
      Module.print(multi(8, 5.4, 4, 'bret'));
      Module.print('*');
      // part 3: avoid stack explosion and check it's restored correctly
      for (var i = 0; i < TOTAL_STACK/60; i++) {
        ccall('multi', 'number', ['number', 'number', 'number', 'string'], [0, 0, 0, '123456789012345678901234567890123456789012345678901234567890']);
      }
      Module.print('stack is ok.');
      ccall('call_ccall_again', null);
  \'\'\'
  open(filename, 'w').write(src)
'''

    Settings.EXPORTED_FUNCTIONS += ['_get_int', '_get_float', '_get_string', '_print_int', '_print_float', '_print_string', '_multi', '_pointer', '_call_ccall_again', '_malloc']
    test_path = path_from_root('tests', 'core', 'test_ccall')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, post_build=post)

    if '-O2' in self.emcc_args or self.is_emterpreter():
      print 'with closure'
      self.emcc_args += ['--closure', '1']
      self.do_run_from_file(src, output, post_build=post)

  def test_dead_functions(self):
    src = r'''
      #include <stdio.h>
      extern "C" {
      __attribute__((noinline)) int unused(int x) {
        return x;
      }
      }
      int main(int argc, char **argv) {
        printf("*%d*\n", argc > 1 ? unused(1) : 2);
        return 0;
      }
    '''
    def test(expected, args=[], no_build=False):
      self.do_run(src, expected, args=args, no_build=no_build)
      return open(self.in_dir('src.cpp.o.js')).read()

    # Sanity check that it works and the dead function is emitted
    js = test('*1*', ['x'])
    test('*2*', no_build=True)
    if self.run_name in ['default', 'asm1', 'asm2g']: assert 'function _unused($' in js

    # Kill off the dead function, and check a code path using it aborts
    Settings.DEAD_FUNCTIONS = ['_unused']
    test('*2*')
    test('abort(-1) at', args=['x'], no_build=True)

    # Kill off a library function, check code aborts
    Settings.DEAD_FUNCTIONS = ['_printf']
    test('abort(-1) at')
    test('abort(-1) at', args=['x'], no_build=True)

  def test_pgo(self):
    if Settings.ASM_JS: return self.skip('PGO does not work in asm mode')

    def run_all(name, src):
      print name
      def test(expected, args=[], no_build=False):
        self.do_run(src, expected, args=args, no_build=no_build)
        return open(self.in_dir('src.cpp.o.js')).read()

      # Sanity check that it works and the dead function is emitted
      js = test('*9*')
      assert 'function _unused(' in js

      # Run with PGO, see that unused is true to its name
      Settings.PGO = 1
      test("*9*\n-s DEAD_FUNCTIONS='[\"_free\",\"_unused\"]'")
      Settings.PGO = 0

      # Kill off the dead function, still works and it is not emitted
      Settings.DEAD_FUNCTIONS = ['_unused']
      js = test('*9*')
      assert 'function _unused($' not in js # no compiled code
      assert 'function _unused(' in js # lib-generated stub
      Settings.DEAD_FUNCTIONS = []

      # Run the same code with argc that uses the dead function, see abort
      test(('dead function: unused'), args=['a', 'b'], no_build=True)

    # Normal stuff
    run_all('normal', r'''
      #include <stdio.h>
      extern "C" {
      int used(int x) {
        if (x == 0) return -1;
        return used(x/3) + used(x/17) + x%5;
      }
      int unused(int x) {
        if (x == 0) return -1;
        return unused(x/4) + unused(x/23) + x%7;
      }
      }
      int main(int argc, char **argv) {
        printf("*%d*\n", argc == 3 ? unused(argv[0][0] + 1024) : used(argc + 1555));
        return 0;
      }
    ''')

    # Call by function pointer
    run_all('function pointers', r'''
      #include <stdio.h>
      extern "C" {
      int used(int x) {
        if (x == 0) return -1;
        return used(x/3) + used(x/17) + x%5;
      }
      int unused(int x) {
        if (x == 0) return -1;
        return unused(x/4) + unused(x/23) + x%7;
      }
      }
      typedef int (*ii)(int);
      int main(int argc, char **argv) {
        ii pointers[256];
        for (int i = 0; i < 256; i++) {
          pointers[i] = (i == 3) ? unused : used;
        }
        printf("*%d*\n", pointers[argc](argc + 1555));
        return 0;
      }
    ''')

  # TODO: test only worked in non-fastcomp
  def test_asm_pgo(self):
    return self.skip('non-fastcomp is deprecated and fails in 3.5')

    src = open(path_from_root('tests', 'hello_libcxx.cpp')).read()
    output = 'hello, world!'

    self.do_run(src, output)
    shutil.move(self.in_dir('src.cpp.o.js'), self.in_dir('normal.js'))

    Settings.ASM_JS = 0
    Settings.PGO = 1
    self.do_run(src, output)
    Settings.ASM_JS = 1
    Settings.PGO = 0

    shutil.move(self.in_dir('src.cpp.o.js'), self.in_dir('pgo.js'))
    pgo_output = run_js(self.in_dir('pgo.js')).split('\n')[1]
    open('pgo_data.rsp', 'w').write(pgo_output)

    # with response file

    self.emcc_args += ['@pgo_data.rsp']
    self.do_run(src, output)
    self.emcc_args.pop()
    shutil.move(self.in_dir('src.cpp.o.js'), self.in_dir('pgoed.js'))

    before = len(open('normal.js').read())
    after = len(open('pgoed.js').read())
    assert after < 0.90 * before, [before, after] # expect a size reduction

    # with response in settings element itself

    open('dead_funcs', 'w').write(pgo_output[pgo_output.find('['):-1])
    self.emcc_args += ['-s', 'DEAD_FUNCTIONS=@' + self.in_dir('dead_funcs')]
    self.do_run(src, output)
    self.emcc_args.pop()
    self.emcc_args.pop()
    shutil.move(self.in_dir('src.cpp.o.js'), self.in_dir('pgoed2.js'))
    assert open('pgoed.js').read() == open('pgoed2.js').read()

    # with relative response in settings element itself

    open('dead_funcs', 'w').write(pgo_output[pgo_output.find('['):-1])
    self.emcc_args += ['-s', 'DEAD_FUNCTIONS=@dead_funcs']
    self.do_run(src, output)
    self.emcc_args.pop()
    self.emcc_args.pop()
    shutil.move(self.in_dir('src.cpp.o.js'), self.in_dir('pgoed2.js'))
    assert open('pgoed.js').read() == open('pgoed2.js').read()

  def test_exported_response(self):
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <emscripten.h>

      extern "C" {
        int other_function() { return 5; }
      }

      int main() {
        int x = EM_ASM_INT_V({ return Module._other_function() });
        emscripten_run_script_string(""); // Add a reference to a symbol that exists in src/deps_info.json to uncover issue #2836 in the test suite.
        printf("waka %d!\n", x);
        return 0;
      }
    '''
    open('exps', 'w').write('["_main","_other_function"]')

    self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=@exps']
    self.do_run(src, '''waka 5!''')
    assert 'other_function' in open('src.cpp.o.js').read()

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
        int x = EM_ASM_INT_V({ return Module._exported_func_from_response_file_4999() });
        emscripten_run_script_string(""); // Add a reference to a symbol that exists in src/deps_info.json to uncover issue #2836 in the test suite.
        printf("waka %d!\n", x);
        return 0;
      }
    '''

    js_funcs.append('_main')
    exported_func_json_file = os.path.join(self.get_dir(), 'large_exported_response.json')
    open(exported_func_json_file, 'wb').write(json.dumps(js_funcs))

    self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=@' + exported_func_json_file]
    self.do_run(src, '''waka 4999!''')
    assert '_exported_func_from_response_file_1' in open('src.cpp.o.js').read()

  def test_add_function(self):
    Settings.INVOKE_RUN = 0
    Settings.RESERVED_FUNCTION_POINTERS = 1

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <emscripten.h>

      int main(int argc, char **argv) {
        int fp = atoi(argv[1]);
        printf("fp: %d\n", fp);
        void (*f)(int) = reinterpret_cast<void (*)(int)>(fp);
        f(7);
        EM_ASM_(Module['Runtime']['removeFunction']($0), f);
        printf("ok\n");
        return 0;
      }
    '''

    open(os.path.join(self.get_dir(), 'post.js'), 'w').write('''
      var newFuncPtr = Runtime.addFunction(function(num) {
        Module.print('Hello ' + num + ' from JS!');
      });
      Module.callMain([newFuncPtr.toString()]);
    ''')

    expected = '''Hello 7 from JS!\nok\n'''
    self.emcc_args += ['--post-js', 'post.js']
    self.do_run(src, expected)

    if Settings.ASM_JS:
      Settings.RESERVED_FUNCTION_POINTERS = 0
      self.do_run(src, '''Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.''')
      generated = open('src.cpp.o.js').read()
      assert 'jsCall' not in generated
      Settings.RESERVED_FUNCTION_POINTERS = 1

      Settings.ALIASING_FUNCTION_POINTERS = 1 - Settings.ALIASING_FUNCTION_POINTERS # flip the test
      self.do_run(src, expected)

    assert 'asm2' in test_modes
    if self.run_name == 'asm2':
      print 'closure'
      self.banned_js_engines = [NODE_JS] # weird global handling in node
      self.emcc_args += ['--closure', '1']
      self.do_run(src, expected)

    print 'function pointer emulation'
    Settings.RESERVED_FUNCTION_POINTERS = 0
    Settings.EMULATED_FUNCTION_POINTERS = 1 # with emulation, we don't need to reserve
    self.do_run(src, expected)

  def test_getFuncWrapper_sig_alias(self):
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
      EM_ASM_INT({
        Runtime.getFuncWrapper($0, 'vi')(0);
        Runtime.getFuncWrapper($1, 'vii')(0, 0);
      }, func1, func2);
      return 0;
    }
    '''
    self.do_run(src, 'func1\nfunc2\n')

  def test_emulate_function_pointer_casts(self):
    Settings.EMULATE_FUNCTION_POINTER_CASTS = 1

    src = r'''
    #include <stdio.h>
    #include <math.h>
    
    // We have to use a proxy function 'acos_test' here because the updated libc++ library provides a set of overloads to acos,
    // this has the result that we can't take the function pointer to acos anymore due to failed overload resolution.
    // This proxy function has no overloads so it's allowed to take the function pointer directly.
    double acos_test(double x) {
      return acos(x);
    }

    typedef double (*ddd)(double x, double unused);
    typedef int    (*iii)(int x,    int unused);

    int main() {
      volatile ddd d = (ddd)acos_test;
      volatile iii i = (iii)acos_test;
      printf("|%.3f,%d|\n", d(0.3, 0.6), i(0, 0));
      return 0;
    }
    '''
    self.do_run(src, '|1.266,1|\n')

  def test_demangle_stacks(self):
    Settings.DEMANGLE_SUPPORT = 1
    if '-O' in str(self.emcc_args):
      self.emcc_args += ['--profiling-funcs', '--llvm-opts', '0']

    test_path = path_from_root('tests', 'core', 'test_demangle_stacks')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_tracing(self):
    Building.COMPILER_TEST_OPTS += ['--tracing']

    test_path = path_from_root('tests', 'core', 'test_tracing')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_eval_ctors(self):
    if '-O2' not in str(self.emcc_args) or '-O1' in str(self.emcc_args): return self.skip('need js optimizations')

    orig_args = self.emcc_args[:] + ['-s', 'EVAL_CTORS=0']

    print 'leave printf in ctor'
    self.emcc_args = orig_args + ['-s', 'EVAL_CTORS=1']
    self.do_run(r'''
      #include <stdio.h>
      struct C {
        C() { printf("constructing!\n"); } // don't remove this!
      };
      C c;
      int main() {}
    ''', "constructing!\n");

    def do_test(test):
      self.emcc_args = orig_args + ['-s', 'EVAL_CTORS=1']
      test()
      ec_js_size = os.stat('src.cpp.o.js').st_size
      ec_mem_size = os.stat('src.cpp.o.js.mem').st_size
      self.emcc_args = orig_args[:]
      test()
      js_size = os.stat('src.cpp.o.js').st_size
      mem_size = os.stat('src.cpp.o.js.mem').st_size
      print js_size, ' => ', ec_js_size
      print mem_size, ' => ', ec_mem_size
      assert ec_js_size < js_size
      assert ec_mem_size > mem_size

    print 'remove ctor of just assigns to memory'
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
      ''', "x: 11\n");
    do_test(test1)

    print 'libcxx'

    src = open(path_from_root('tests', 'hello_libcxx.cpp')).read()
    output = 'hello, world!'
    self.do_run(src, output)
    js_size = os.stat('src.cpp.o.js').st_size
    if self.uses_memory_init_file():
      mem_size = os.stat('src.cpp.o.js.mem').st_size
    self.emcc_args += ['-s', 'EVAL_CTORS=1']
    self.do_run(src, output)
    ec_js_size = os.stat('src.cpp.o.js').st_size
    if self.uses_memory_init_file():
      ec_mem_size = os.stat('src.cpp.o.js.mem').st_size
    print js_size, ' => ', ec_js_size
    print mem_size, ' => ', ec_mem_size
    assert ec_js_size < js_size
    assert ec_mem_size > mem_size

    print 'assertions too'
    Settings.ASSERTIONS = 1
    self.do_run(src, output)

  def test_embind(self):
    Building.COMPILER_TEST_OPTS += ['--bind']

    src = r'''
      #include<stdio.h>
      #include<emscripten/val.h>

      using namespace emscripten;

      int main() {
        val Math = val::global("Math");

        // two ways to call Math.abs
        printf("abs(-10): %d\n", Math.call<int>("abs", -10));
        printf("abs(-11): %d\n", Math["abs"](-11).as<int>());

        return 0;
      }
    '''
    self.do_run(src, 'abs(-10): 10\nabs(-11): 11');

  def test_embind_2(self):
    Settings.NO_EXIT_RUNTIME = 1 # we emit some post.js that we need to see
    Building.COMPILER_TEST_OPTS += ['--bind', '--post-js', 'post.js']
    open('post.js', 'w').write('''
      Module.print('lerp ' + Module.lerp(100, 200, 66) + '.');
    ''')
    src = r'''
      #include <stdio.h>
      #include <emscripten/bind.h>
      using namespace emscripten;
      int lerp(int a, int b, int t) {
          return (100 - t) * a + t * b;
      }
      EMSCRIPTEN_BINDINGS(my_module) {
          function("lerp", &lerp);
      }
      int main(int argc, char **argv) {
          return 0;
      }
    '''
    self.do_run(src, 'lerp 166');

  def test_embind_3(self):
    Settings.NO_EXIT_RUNTIME = 1 # we emit some post.js that we need to see
    Building.COMPILER_TEST_OPTS += ['--bind', '--post-js', 'post.js']
    open('post.js', 'w').write('''
      function ready() {
        try {
          Module.compute(new Uint8Array([1,2,3]));
        } catch(e) {
          Module.print(e);
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
    self.do_run(src, 'UnboundTypeError: Cannot call compute due to unbound types: Pi');

  def test_scriptaclass(self):
      Settings.EXPORT_BINDINGS = 1

      header_filename = os.path.join(self.get_dir(), 'header.h')
      header = '''
        struct ScriptMe {
          int value;
          ScriptMe(int val);
          int getVal(); // XXX Sadly, inlining these will result in LLVM not
                        // producing any code for them (when just building
                        // as a library)
          void mulVal(int mul);
        };
      '''
      h = open(header_filename, 'w')
      h.write(header)
      h.close()

      src = '''
        #include "header.h"

        ScriptMe::ScriptMe(int val) : value(val) { }
        int ScriptMe::getVal() { return value; }
        void ScriptMe::mulVal(int mul) { value *= mul; }
      '''

      # Way 1: use demangler and namespacer

      script_src = '''
        var sme = Module._.ScriptMe.__new__(83);          // malloc(sizeof(ScriptMe)), ScriptMe::ScriptMe(sme, 83) / new ScriptMe(83) (at addr sme)
        Module._.ScriptMe.mulVal(sme, 2);                 // ScriptMe::mulVal(sme, 2)       sme.mulVal(2)
        Module.print('*' + Module._.ScriptMe.getVal(sme) + '*');
        _free(sme);
        Module.print('*ok*');
      '''
      post = '''
def process(filename):
  Popen([PYTHON, DEMANGLER, filename], stdout=open(filename + '.tmp', 'w')).communicate()
  Popen([PYTHON, NAMESPACER, filename, filename + '.tmp'], stdout=open(filename + '.tmp2', 'w')).communicate()
  src = open(filename, 'r').read().replace(
    '// {{MODULE_ADDITIONS}',
    'Module["_"] = ' + open(filename + '.tmp2', 'r').read().replace('var ModuleNames = ', '').rstrip() + ';\n\n' + script_src + '\n\n' +
      '// {{MODULE_ADDITIONS}'
  )
  open(filename, 'w').write(src)
'''
      # XXX disable due to possible v8 bug -- self.do_run(src, '*166*\n*ok*', post_build=post)

      if '-O2' in self.emcc_args and 'ASM_JS=0' not in self.emcc_args and not self.is_wasm(): # without asm, closure minifies Math.imul badly
        self.emcc_args += ['--closure', '1'] # Use closure here, to test we export things right

      # Way 2: use CppHeaderParser

      header = '''
        #include <stdio.h>

        class Parent {
        protected:
          int value;
        public:
          Parent(int val);
          Parent(Parent *p, Parent *q); // overload constructor
          int getVal() { return value; }; // inline should work just fine here, unlike Way 1 before
          void mulVal(int mul);
        };

        class Child1 : public Parent {
        public:
          Child1() : Parent(7) { printf("Child1:%d\\n", value); };
          Child1(int val) : Parent(val*2) { value -= 1; printf("Child1:%d\\n", value); };
          int getValSqr() { return value*value; }
          int getValSqr(int more) { return value*value*more; }
          int getValTimes(int times=1) { return value*times; }
        };

        class Child2 : public Parent {
        public:
          Child2() : Parent(9) { printf("Child2:%d\\n", value); };
          int getValCube() { return value*value*value; }
          static void printStatic() { printf("*static*\\n"); }

          virtual void virtualFunc() { printf("*virtualf*\\n"); }
          virtual void virtualFunc2() { printf("*virtualf2*\\n"); }
          static void runVirtualFunc(Child2 *self) { self->virtualFunc(); };
        private:
          void doSomethingSecret() { printf("security breached!\\n"); }; // we should not be able to do this
        };
      '''
      open(header_filename, 'w').write(header)

      basename = os.path.join(self.get_dir(), 'bindingtest')
      output = Popen([PYTHON, BINDINGS_GENERATOR, basename, header_filename], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
      #print output
      assert 'Traceback' not in output, 'Failure in binding generation: ' + output

      src = '''
        #include "header.h"

        Parent::Parent(int val) : value(val) { printf("Parent:%d\\n", val); }
        Parent::Parent(Parent *p, Parent *q) : value(p->value + q->value) { printf("Parent:%d\\n", value); }
        void Parent::mulVal(int mul) { value *= mul; }

        #include "bindingtest.cpp"
      '''

      post2 = '''
def process(filename):
  src = open(filename, 'a')
  src.write(open('bindingtest.js').read() + '\\n\\n')
  src.close()
'''

      def post3(filename):
        script_src_2 = '''
          var sme = new Module.Parent(42);
          sme.mulVal(2);
          Module.print('*')
          Module.print(sme.getVal());

          Module.print('c1');

          var c1 = new Module.Child1();
          Module.print(c1.getVal());
          c1.mulVal(2);
          Module.print(c1.getVal());
          Module.print(c1.getValSqr());
          Module.print(c1.getValSqr(3));
          Module.print(c1.getValTimes()); // default argument should be 1
          Module.print(c1.getValTimes(2));

          Module.print('c1 v2');

          c1 = new Module.Child1(8); // now with a parameter, we should handle the overloading automatically and properly and use constructor #2
          Module.print(c1.getVal());
          c1.mulVal(2);
          Module.print(c1.getVal());
          Module.print(c1.getValSqr());
          Module.print(c1.getValSqr(3));

          Module.print('c2')

          var c2 = new Module.Child2();
          Module.print(c2.getVal());
          c2.mulVal(2);
          Module.print(c2.getVal());
          Module.print(c2.getValCube());
          var succeeded;
          try {
            succeeded = 0;
            Module.print(c2.doSomethingSecret()); // should fail since private
            succeeded = 1;
          } catch(e) {}
          Module.print(succeeded);
          try {
            succeeded = 0;
            Module.print(c2.getValSqr()); // function from the other class
            succeeded = 1;
          } catch(e) {}
          Module.print(succeeded);
          try {
            succeeded = 0;
            c2.getValCube(); // sanity
            succeeded = 1;
          } catch(e) {}
          Module.print(succeeded);

          Module.Child2.prototype.printStatic(); // static calls go through the prototype

          // virtual function
          c2.virtualFunc();
          Module.Child2.prototype.runVirtualFunc(c2);
          c2.virtualFunc2();

          // extend the class from JS
          var c3 = new Module.Child2;
          Module.customizeVTable(c3, [{
            original: Module.Child2.prototype.virtualFunc,
            replacement: function() {
              Module.print('*js virtualf replacement*');
            }
          }, {
            original: Module.Child2.prototype.virtualFunc2,
            replacement: function() {
              Module.print('*js virtualf2 replacement*');
            }
          }]);
          c3.virtualFunc();
          Module.Child2.prototype.runVirtualFunc(c3);
          c3.virtualFunc2();

          c2.virtualFunc(); // original should remain the same
          Module.Child2.prototype.runVirtualFunc(c2);
          c2.virtualFunc2();
          Module.print('*ok*');
        '''
        code = open(filename).read()
        src = open(filename, 'w')
        src.write('var Module = {};\n') # name Module
        src.write(code)
        src.write(script_src_2 + '\n')
        src.close()

      Settings.RESERVED_FUNCTION_POINTERS = 20

      self.do_run(src, '''*
84
c1
Parent:7
Child1:7
7
14
196
588
14
28
c1 v2
Parent:16
Child1:15
15
30
900
2700
c2
Parent:9
Child2:9
9
18
5832
0
0
1
*static*
*virtualf*
*virtualf*
*virtualf2*''' + ('''
Parent:9
Child2:9
*js virtualf replacement*
*js virtualf replacement*
*js virtualf2 replacement*
*virtualf*
*virtualf*
*virtualf2*''') + '''
*ok*
''', post_build=(post2, post3))

  def test_scriptaclass_2(self):
      Settings.EXPORT_BINDINGS = 1

      header_filename = os.path.join(self.get_dir(), 'header.h')
      header = '''
        #include <stdio.h>
        #include <string.h>

        class StringUser {
          char *s;
          int i;
        public:
          StringUser(char *string, int integer) : s(strdup(string)), i(integer) {}
          void Print(int anotherInteger, char *anotherString) {
            printf("|%s|%d|%s|%d|\\n", s, i, anotherString, anotherInteger);
          }
          void CallOther(StringUser *fr) { fr->Print(i, s); }
        };
      '''
      open(header_filename, 'w').write(header)

      basename = os.path.join(self.get_dir(), 'bindingtest')
      output = Popen([PYTHON, BINDINGS_GENERATOR, basename, header_filename], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
      #print output
      assert 'Traceback' not in output, 'Failure in binding generation: ' + output

      src = '''
        #include "header.h"

        #include "bindingtest.cpp"
      '''

      post = '''
def process(filename):
  src = open(filename, 'a')
  src.write(open('bindingtest.js').read() + '\\n\\n')
  src.write(\'\'\'
          var user = new Module.StringUser("hello", 43);
          user.Print(41, "world");
            \'\'\')
  src.close()
'''
      self.do_run(src, '|hello|43|world|41|', post_build=post)

  def test_webidl(self):
    assert 'asm2' in test_modes
    if self.run_name == 'asm2':
      self.emcc_args += ['--closure', '1', '-g1'] # extra testing
      Settings.MODULARIZE = 1 # avoid closure minified names competing with our test code in the global name space

    def do_test_in_mode(mode):
      print 'testing mode', mode
      # Force IDL checks mode
      os.environ['IDL_CHECKS'] = mode

      output = Popen([PYTHON, path_from_root('tools', 'webidl_binder.py'),
                              path_from_root('tests', 'webidl', 'test.idl'),
                              'glue']).communicate()[0]
      assert os.path.exists('glue.cpp')
      assert os.path.exists('glue.js')

      # Export things on "TheModule". This matches the typical use pattern of the bound library
      # being used as Box2D.* or Ammo.*, and we cannot rely on "Module" being always present (closure may remove it).
      open('export.js', 'w').write('''
// test purposes: remove printErr output, whose order is unpredictable when compared to print
Module.printErr = Module['printErr'] = function(){};
''')
      self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=["_malloc"]', '--post-js', 'glue.js', '--post-js', 'export.js']
      shutil.copyfile(path_from_root('tests', 'webidl', 'test.h'), self.in_dir('test.h'))
      shutil.copyfile(path_from_root('tests', 'webidl', 'test.cpp'), self.in_dir('test.cpp'))
      src = open('test.cpp').read()
      def post(filename):
        src = open(filename, 'a')
        src.write('\n\n')
        if self.run_name == 'asm2':
          src.write('var TheModule = Module();\n')
        else:
          src.write('var TheModule = Module;\n')
        src.write('\n\n')
        src.write(open(path_from_root('tests', 'webidl', 'post.js')).read())
        src.write('\n\n')
        src.close()
      self.do_run(src, open(path_from_root('tests', 'webidl', "output_%s.txt" % mode)).read(), post_build=(None, post),
        output_nicerizer=(lambda out, err: out))

    do_test_in_mode('ALL')
    do_test_in_mode('FAST')
    do_test_in_mode('DEFAULT')

  ### Tests for tools

  def test_safe_heap(self):
    if not Settings.SAFE_HEAP: return self.skip('We need SAFE_HEAP to test SAFE_HEAP')
    # TODO: Should we remove this test?
    return self.skip('It is ok to violate the load-store assumption with TA2')
    if Building.LLVM_OPTS: return self.skip('LLVM can optimize away the intermediate |x|')

    src = '''
      #include<stdio.h>
      #include<stdlib.h>
      int main() { int *x = (int*)malloc(sizeof(int));
        *x = 20;
        float *y = (float*)x;
        printf("%f\\n", *y);
        printf("*ok*\\n");
        return 0;
      }
    '''

    try:
      self.do_run(src, '*nothingatall*', assert_returncode=None)
    except Exception, e:
      # This test *should* fail, by throwing this exception
      assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

    Settings.SAFE_HEAP = 1

    # Linking multiple files should work too

    module = '''
      #include<stdio.h>
      #include<stdlib.h>
      void callFunc() { int *x = (int*)malloc(sizeof(int));
        *x = 20;
        float *y = (float*)x;
        printf("%f\\n", *y);
      }
    '''
    module_name = os.path.join(self.get_dir(), 'module.cpp')
    open(module_name, 'w').write(module)

    main = '''
      #include<stdio.h>
      #include<stdlib.h>
      extern void callFunc();
      int main() { callFunc();
        int *x = (int*)malloc(sizeof(int));
        *x = 20;
        float *y = (float*)x;
        printf("%f\\n", *y);
        printf("*ok*\\n");
        return 0;
      }
    '''
    main_name = os.path.join(self.get_dir(), 'main.cpp')
    open(main_name, 'w').write(main)

    Building.emcc(module_name, ['-g'])
    Building.emcc(main_name, ['-g'])
    all_name = os.path.join(self.get_dir(), 'all.bc')
    Building.link([module_name + '.o', main_name + '.o'], all_name)

    try:
      self.do_ll_run(all_name, '*nothingatall*', assert_returncode=None)
    except Exception, e:
      # This test *should* fail, by throwing this exception
      assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

  @no_emterpreter
  def test_source_map(self):
    if NODE_JS not in JS_ENGINES: return self.skip('sourcemapper requires Node to run')
    if '-g' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g')

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

    dirname = self.get_dir()
    src_filename = os.path.join(dirname, 'src.cpp')
    out_filename = os.path.join(dirname, 'a.out.js')
    no_maps_filename = os.path.join(dirname, 'no-maps.out.js')

    with open(src_filename, 'w') as f: f.write(src)
    assert '-g4' not in Building.COMPILER_TEST_OPTS
    Building.emcc(src_filename, Settings.serialize() + self.emcc_args +
        Building.COMPILER_TEST_OPTS, out_filename)
    # the file name may find its way into the generated code, so make sure we
    # can do an apples-to-apples comparison by compiling with the same file name
    shutil.move(out_filename, no_maps_filename)
    with open(no_maps_filename) as f: no_maps_file = f.read()
    no_maps_file = re.sub(' *//[@#].*$', '', no_maps_file, flags=re.MULTILINE)
    Building.COMPILER_TEST_OPTS.append('-g4')

    def build_and_check():
      import json
      Building.emcc(src_filename, Settings.serialize() + self.emcc_args +
          Building.COMPILER_TEST_OPTS, out_filename, stderr=PIPE)
      # after removing the @line and @sourceMappingURL comments, the build
      # result should be identical to the non-source-mapped debug version.
      # this is worth checking because the parser AST swaps strings for token
      # objects when generating source maps, so we want to make sure the
      # optimizer can deal with both types.
      map_filename = out_filename + '.map'
      data = json.load(open(map_filename, 'r'))
      self.assertPathsIdentical(out_filename, data['file'])
      assert len(data['sources']) == 1, data['sources']
      self.assertPathsIdentical(src_filename, data['sources'][0])
      self.assertTextDataIdentical(src, data['sourcesContent'][0])
      mappings = json.loads(jsrun.run_js(
        path_from_root('tools', 'source-maps', 'sourcemap2json.js'),
        tools.shared.NODE_JS, [map_filename]))
      seen_lines = set()
      for m in mappings:
        self.assertPathsIdentical(src_filename, m['source'])
        seen_lines.add(m['originalLine'])
      # ensure that all the 'meaningful' lines in the original code get mapped
      assert seen_lines.issuperset([6, 7, 11, 12])

    build_and_check()

    assert 'asm2g' in test_modes
    if self.run_name == 'asm2g':
      # EMCC_DEBUG=2 causes lots of intermediate files to be written, and so
      # serves as a stress test for source maps because it needs to correlate
      # line numbers across all those files.
      old_emcc_debug = os.environ.get('EMCC_DEBUG', None)
      os.environ.pop('EMCC_DEBUG', None)
      try:
        os.environ['EMCC_DEBUG'] = '2'
        build_and_check()
      finally:
        if old_emcc_debug is not None:
          os.environ['EMCC_DEBUG'] = old_emcc_debug
        else:
          os.environ.pop('EMCC_DEBUG', None)

  @no_emterpreter
  def test_exception_source_map(self):
    if self.is_wasm(): return self.skip('wasmifying destroys debug info and stack tracability')
    if '-g4' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g4')
    if NODE_JS not in JS_ENGINES: return self.skip('sourcemapper requires Node to run')

    src = '''
      #include <stdio.h>

      __attribute__((noinline)) void foo(int i) {
          if (i < 10) throw i; // line 5
      }

      #include <iostream>
      #include <string>

      int main() {
        std::string x = "ok"; // add libc++ stuff to make this big, test for #2410
        int i;
        scanf("%d", &i);
        foo(i);
        std::cout << x << std::endl;
        return 0;
      }
    '''

    def post(filename):
      import json
      map_filename = filename + '.map'
      mappings = json.loads(jsrun.run_js(
        path_from_root('tools', 'source-maps', 'sourcemap2json.js'),
        tools.shared.NODE_JS, [map_filename]))
      with open(filename) as f: lines = f.readlines()
      for m in mappings:
        if m['originalLine'] == 5 and '__cxa_throw' in lines[m['generatedLine']-1]: # -1 to fix 0-start vs 1-start
          return
      assert False, 'Must label throw statements with line numbers'

    dirname = self.get_dir()
    self.build(src, dirname, os.path.join(dirname, 'src.cpp'), post_build=(None, post))

  def test_emscripten_log(self):
    if self.is_wasm(): return self.skip('wasmifying destroys debug info and stack tracability')
    self.emcc_args += ['-s', 'DEMANGLE_SUPPORT=1']
    if self.is_emterpreter():
      self.emcc_args += ['--profiling-funcs'] # without this, stack traces are not useful (we jump emterpret=>emterpret)
      Building.COMPILER_TEST_OPTS += ['-DEMTERPRETER'] # even so, we get extra emterpret() calls on the stack
    if Settings.ASM_JS:
      # XXX Does not work in SpiderMonkey since callstacks cannot be captured when running in asm.js, see https://bugzilla.mozilla.org/show_bug.cgi?id=947996
      self.banned_js_engines = [SPIDERMONKEY_ENGINE] 
    if '-g' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g')
    Building.COMPILER_TEST_OPTS += ['-DRUN_FROM_JS_SHELL']
    self.do_run(open(path_from_root('tests', 'emscripten_log', 'emscripten_log.cpp')).read(), '''test print 123

12.345679 9.123457 1.353180

12345678 9123456 1353179

12.345679 9123456 1353179

12345678 9.123457 1353179

12345678 9123456 1.353180

12345678 9.123457 1.353180

12.345679 9123456 1.353180

12.345679 9.123457 1353179

Success!
''')

  def test_float_literals(self):
    self.do_run_from_file(path_from_root('tests', 'test_float_literals.cpp'), path_from_root('tests', 'test_float_literals.out'))

  def test_exit_status(self):
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
    open('post.js', 'w').write('''
      Module.addOnExit(function () {
        Module.print('I see exit status: ' + EXITSTATUS);
      });
      Module.callMain();
    ''')
    self.emcc_args += ['-s', 'INVOKE_RUN=0', '--post-js', 'post.js']
    self.do_run(src.replace('CAPITAL_EXIT', '0'), 'hello, world!\ncleanup\nI see exit status: 118')
    self.do_run(src.replace('CAPITAL_EXIT', '1'), 'hello, world!\ncleanup\nI see exit status: 118')

  def test_noexitruntime(self):
    src = r'''
      #include <emscripten.h>
      #include <stdio.h>
      static int testPre = TEST_PRE;
      struct Global {
        Global() {
          printf("in Global()\n");
          if (testPre) { EM_ASM(Module['noExitRuntime'] = true;); }
        }
        ~Global() { printf("ERROR: in ~Global()\n"); }
      } global;
      int main() {
        if (!testPre) { EM_ASM(Module['noExitRuntime'] = true;); }
        printf("in main()\n");
      }
    '''
    self.do_run(src.replace('TEST_PRE', '0'), 'in Global()\nin main()')
    self.do_run(src.replace('TEST_PRE', '1'), 'in Global()\nin main()')

  def test_minmax(self):
    self.do_run(open(path_from_root('tests', 'test_minmax.c')).read(), 'NAN != NAN\nSuccess!')

  def test_locale(self):
    self.do_run_from_file(path_from_root('tests', 'test_locale.c'), path_from_root('tests', 'test_locale.out'))

  def test_async(self):
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
  emscripten_%s(100);
  printf("%%d\n", i);
}
''' % ('sleep_with_yield' if self.is_emterpreter() else 'sleep')

    if not self.is_emterpreter():
      Settings.ASYNCIFY = 1
    else:
      Settings.EMTERPRETIFY_ASYNC = 1

    self.do_run(src, 'HelloWorld!99');

    if self.is_emterpreter():
      print 'check bad ccall use'
      src = r'''
#include <stdio.h>
#include <emscripten.h>
int main() {
  printf("Hello");
  emscripten_sleep(100);
  printf("World\n");
}
'''
      Settings.ASSERTIONS = 1
      Settings.INVOKE_RUN = 0
      open('post.js', 'w').write('''
try {
  Module['ccall']('main', 'number', ['number', 'string'], [2, 'waka']);
  var never = true;
} catch(e) {
  Module.print(e);
  assert(!never);
}
''')
      self.emcc_args += ['--post-js', 'post.js']
      self.do_run(src, 'cannot start async op with normal JS');

      print 'check reasonable ccall use'
      src = r'''
#include <stdio.h>
#include <emscripten.h>
int main() {
  printf("Hello");
  emscripten_sleep(100);
  printf("World\n");
}
'''
      open('post.js', 'w').write('''
Module['ccall']('main', null, ['number', 'string'], [2, 'waka'], { async: true });
''')
      self.do_run(src, 'HelloWorld');

  def test_async_returnvalue(self):
    if not self.is_emterpreter(): return self.skip('emterpreter-only test')

    Settings.EMTERPRETIFY_ASYNC = 1
    self.banned_js_engines = [SPIDERMONKEY_ENGINE, V8_ENGINE] # needs setTimeout which only node has

    open('lib.js', 'w').write(r'''
mergeInto(LibraryManager.library, {
  sleep_with_return__deps: ['$EmterpreterAsync'],
  sleep_with_return: function(ms) {
    return EmterpreterAsync.handle(function(resume) {
      var startTime = Date.now();
      setTimeout(function() {
        if (ABORT) return; // do this manually; we can't call into Browser.safeSetTimeout, because that is paused/resumed!
        resume(function() {
          return Date.now() - startTime;
        });
      }, ms);
    });
  }
});
''')

    src = r'''
#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

extern "C" {
extern int sleep_with_return(int ms);
}

int main() {
  int ms = sleep_with_return(1000);
  assert(ms >= 900);
  printf("napped for %d ms\n", ms);
}
'''
    self.emcc_args += ['--js-library', 'lib.js']

    self.do_run(src, 'napped');

  def test_async_exit(self):
    if not self.is_emterpreter(): return self.skip('emterpreter-only test')

    Settings.EMTERPRETIFY_ASYNC = 1
    self.banned_js_engines = [SPIDERMONKEY_ENGINE, V8_ENGINE] # needs setTimeout which only node has

    self.do_run(r'''
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

void f()
{
    printf("f\n");
    emscripten_sleep(1);
    printf("hello\n");
    static int i = 0;
    i++;
    if(i == 5) {
        printf("exit\n");
        exit(0);
        printf("world\n");
        i = 0;
    }
}

int main() {
    while(1) {
        f();
    }
    return 0;
}
''', 'f\nhello\nf\nhello\nf\nhello\nf\nhello\nf\nhello\nexit\n')

  def test_coroutine(self):
    src = r'''
#include <stdio.h>
#include <emscripten.h>
void fib(void * arg) {
    int * p = (int*)arg;
    int cur = 1;
    int next = 1;
    for(int i = 0; i < 9; ++i) {
        *p = cur;
        emscripten_yield();
        int next2 = cur + next;
        cur = next;
        next = next2;
    }
}
void f(void * arg) {
    int * p = (int*)arg;
    *p = 0;
    emscripten_yield();
    fib(arg); // emscripten_yield in fib() can `pass through` f() back to main(), and then we can assume inside fib()
}
void g(void * arg) {
    int * p = (int*)arg;
    for(int i = 0; i < 10; ++i) {
        *p = 100+i;
        emscripten_yield();
    }
}
int main(int argc, char **argv) {
    int i;
    emscripten_coroutine co = emscripten_coroutine_create(f, (void*)&i, 0);
    emscripten_coroutine co2 = emscripten_coroutine_create(g, (void*)&i, 0);
    printf("*");
    while(emscripten_coroutine_next(co)) {
        printf("%d-", i);
        emscripten_coroutine_next(co2);
        printf("%d-", i);
    }
    printf("*");
    return 0;
}
'''
    Settings.ASYNCIFY = 1;
    self.do_run(src, '*0-100-1-101-1-102-2-103-3-104-5-105-8-106-13-107-21-108-34-109-*');

  def test_cxx_self_assign(self):
    # See https://github.com/kripken/emscripten/pull/2688 and http://llvm.org/bugs/show_bug.cgi?id=18735
    open('src.cpp', 'w').write(r'''
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
      ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('ok.', run_js('a.out.js', args=['C']))

  def test_memprof_requirements(self):
    # This test checks for the global variables required to run the memory
    # profiler.  It would fail if these variables were made no longer global
    # or if their identifiers were changed.
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      extern "C" {
        void check_memprof_requirements();
      }
      int main() {
        check_memprof_requirements();
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'lib.js'), 'w').write('''
      mergeInto(LibraryManager.library, {
        check_memprof_requirements: function() {
          if (typeof TOTAL_MEMORY === 'number' &&
              typeof STATIC_BASE === 'number' &&
              typeof STATICTOP === 'number' &&
              typeof STACK_BASE === 'number' &&
              typeof STACK_MAX === 'number' &&
              typeof STACKTOP === 'number' &&
              typeof DYNAMIC_BASE === 'number' &&
              typeof DYNAMICTOP === 'number') {
             Module.print('able to run memprof');
           } else {
             Module.print('missing the required variables to run memprof');
           }
        }
      });
    ''')
    self.emcc_args += ['--js-library', os.path.join(self.get_dir(), 'lib.js')]
    self.do_run(open(os.path.join(self.get_dir(), 'main.cpp'), 'r').read(), 'able to run memprof')

  def test_fs_dict(self):
      Settings.FORCE_FILESYSTEM = 1
      open(self.in_dir('pre.js'), 'w').write('''
        var Module = {};
        Module['preRun'] = function() {
            console.log(typeof FS.filesystems['MEMFS']);
            console.log(typeof FS.filesystems['IDBFS']);
            console.log(typeof FS.filesystems['NODEFS']);
        };
      ''')
      self.emcc_args += ['--pre-js', 'pre.js']
      self.do_run('', 'object\nobject\nobject')

  def test_stack_overflow_check(self):
    args = self.emcc_args + ['-s', 'TOTAL_STACK=1048576']
    self.emcc_args = args + ['-s', 'STACK_OVERFLOW_CHECK=1', '-s', 'ASSERTIONS=0']
    self.do_run(open(path_from_root('tests', 'stack_overflow.cpp'), 'r').read(), 'Stack overflow! Stack cookie has been overwritten' if not Settings.SAFE_HEAP else 'segmentation fault')

    self.emcc_args = args + ['-s', 'STACK_OVERFLOW_CHECK=2', '-s', 'ASSERTIONS=0']
    self.do_run(open(path_from_root('tests', 'stack_overflow.cpp'), 'r').read(), 'Stack overflow! Attempted to allocate')

    self.emcc_args = args + ['-s', 'ASSERTIONS=1']
    self.do_run(open(path_from_root('tests', 'stack_overflow.cpp'), 'r').read(), 'Stack overflow! Attempted to allocate')

  @no_wasm
  @no_emterpreter
  def test_binaryen(self):
    self.emcc_args += ['-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"']
    self.do_run(open(path_from_root('tests', 'hello_world.c')).read(), 'hello, world!')

# Generate tests for everything
def make_run(fullname, name=-1, compiler=-1, embetter=0, quantum_size=0,
    typed_arrays=0, emcc_args=None, env=None):

  if env is None: env = {}

  TT = type(fullname, (T,), dict(run_name = fullname, env = env))

  def tearDown(self):
    try:
      super(TT, self).tearDown()
    finally:
      for k, v in self.env.iteritems():
        del os.environ[k]

      # clear global changes to Building
      Building.COMPILER_TEST_OPTS = []
      Building.COMPILER = CLANG
      Building.LLVM_OPTS = 0

  TT.tearDown = tearDown

  def setUp(self):
    super(TT, self).setUp()
    for k, v in self.env.iteritems():
      assert k not in os.environ, k + ' should not be in environment'
      os.environ[k] = v

    global checked_sanity
    if not checked_sanity:
      print '(checking sanity from test runner)' # do this after we set env stuff
      check_sanity(force=True)
      checked_sanity = True

    os.chdir(self.get_dir()) # Ensure the directory exists and go there
    Building.COMPILER = compiler

    assert emcc_args is not None
    self.emcc_args = emcc_args[:]
    Settings.load(self.emcc_args)
    Building.LLVM_OPTS = 0

    Building.COMPILER_TEST_OPTS += ['-Wno-dynamic-class-memaccess', '-Wno-format', '-Wno-format-extra-args', '-Wno-format-security', '-Wno-pointer-bool-conversion', '-Wno-unused-volatile-lvalue', '-Wno-c++11-compat-deprecated-writable-strings', '-Wno-invalid-pp-token']

    for arg in self.emcc_args:
      if arg.startswith('-O'):
        Building.COMPILER_TEST_OPTS.append(arg) # so bitcode is optimized too, this is for cpp to ll
      else:
        try:
          key, value = arg.split('=')
          Settings[key] = value # forward  -s K=V
        except:
          pass
      return

  TT.setUp = setUp

  return TT

# Main test modes
default = make_run("default", compiler=CLANG, emcc_args=["-s", "ASM_JS=2"])
asm1 = make_run("asm1", compiler=CLANG, emcc_args=["-O1"])
asm2 = make_run("asm2", compiler=CLANG, emcc_args=["-O2"])
asm3 = make_run("asm3", compiler=CLANG, emcc_args=["-O3"])
asm2f = make_run("asm2f", compiler=CLANG, emcc_args=["-Oz", "-s", "PRECISE_F32=1", "-s", "ALLOW_MEMORY_GROWTH=1"])
asm2g = make_run("asm2g", compiler=CLANG, emcc_args=["-O2", "-g", "-s", "ASSERTIONS=1", "-s", "SAFE_HEAP=1"])
asm2i = make_run("asm2i", compiler=CLANG, emcc_args=["-O2", '-s', 'EMTERPRETIFY=1'])
#asm2m = make_run("asm2m", compiler=CLANG, emcc_args=["-O2", "--memory-init-file", "0", "-s", "MEM_INIT_METHOD=2", "-s", "ASSERTIONS=1"])
binaryen0 = make_run("binaryen0", compiler=CLANG, emcc_args=['-O0', '-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
binaryen1 = make_run("binaryen1", compiler=CLANG, emcc_args=['-O1', '-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
binaryen2 = make_run("binaryen2", compiler=CLANG, emcc_args=['-O2', '-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
binaryen3 = make_run("binaryen3", compiler=CLANG, emcc_args=['-O3', '-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
#normalyen = make_run("normalyen", compiler=CLANG, emcc_args=['-O0', '-s', 'GLOBAL_BASE=1024']) # useful comparison to binaryen
#spidaryen = make_run("binaryen", compiler=CLANG, emcc_args=['-O0', '-s', 'BINARYEN=1', '-s', 'BINARYEN_SCRIPTS="spidermonkify.py"'])

# Legacy test modes - 
asm2nn = make_run("asm2nn", compiler=CLANG, emcc_args=["-O2"], env={"EMCC_NATIVE_OPTIMIZER": "0"})

del T # T is just a shape for the specific subclasses, we don't test it itself
