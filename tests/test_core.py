# coding=utf-8

import glob, hashlib, os, re, shutil, subprocess, sys
import tools.shared
from tools.shared import *
from runner import RunnerCore, path_from_root, checked_sanity, test_modes, get_bullet_library

class T(RunnerCore): # Short name, to make it more fun to use manually on the commandline
  def is_le32(self):
    return not ('i386-pc-linux-gnu' in COMPILER_OPTS or self.env.get('EMCC_LLVM_TARGET') == 'i386-pc-linux-gnu')

  def test_hello_world(self):
      test_path = path_from_root('tests', 'core', 'test_hello_world')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

      assert 'EMSCRIPTEN_GENERATED_FUNCTIONS' not in open(self.in_dir('src.cpp.o.js')).read(), 'must not emit this unneeded internal thing'

  def test_intvars(self):
      if self.emcc_args == None: return self.skip('needs ta2')

      test_path = path_from_root('tests', 'core', 'test_intvars')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_sintvars(self):
      Settings.CORRECT_SIGNS = 1 # Relevant to this test
      Settings.CORRECT_OVERFLOWS = 0 # We should not need overflow correction to get this right

      test_path = path_from_root('tests', 'core', 'test_sintvars')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, force_c=True)

  def test_i64(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('i64 mode 1 requires ta2')

      src = '''
        #include <stdio.h>
        int main()
        {
          long long a = 0x2b00505c10;
          long long b = a >> 29;
          long long c = a >> 32;
          long long d = a >> 34;
          printf("*%Ld,%Ld,%Ld,%Ld*\\n", a, b, c, d);
          unsigned long long ua = 0x2b00505c10;
          unsigned long long ub = ua >> 29;
          unsigned long long uc = ua >> 32;
          unsigned long long ud = ua >> 34;
          printf("*%Ld,%Ld,%Ld,%Ld*\\n", ua, ub, uc, ud);

          long long x = 0x0000def123450789ULL; // any bigger than this, and we
          long long y = 0x00020ef123456089ULL; // start to run into the double precision limit!
          printf("*%Ld,%Ld,%Ld,%Ld,%Ld*\\n", x, y, x | y, x & y, x ^ y, x >> 2, y << 2);

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
          printf("m1: %Ld\n", t);
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
          printf("*%Ld\n%d,%d,%d,%d,%d\n%d,%d,%d,%d,%d*\n", x1, x1==x2, x1<x2, x1<=x2, x1>x2, x1>=x2, // note: some rounding in the printing!
                                                                x1==x3, x1<x3, x1<=x3, x1>x3, x1>=x3);
          printf("*%Ld*\n", returner1());
          printf("*%Ld*\n", returner2(30));

          uint64_t maxx = -1ULL;
          printf("*%Lu*\n*%Lu*\n", maxx, maxx >> 5);

          // Make sure params are not modified if they shouldn't be
          int64_t t = 123;
          modifier1(t);
          printf("*%Ld*\n", t);
          modifier2(t);
          printf("*%Ld*\n", t);

          // global structs with i64s
          printf("*%d,%Ld*\n*%d,%Ld*\n", iub[0].c, iub[0].d, iub[1].c, iub[1].d);

          // Bitshifts
          {
            int64_t a = -1;
            int64_t b = a >> 29;
            int64_t c = a >> 32;
            int64_t d = a >> 34;
            printf("*%Ld,%Ld,%Ld,%Ld*\n", a, b, c, d);
            uint64_t ua = -1;
            int64_t ub = ua >> 29;
            int64_t uc = ua >> 32;
            int64_t ud = ua >> 34;
            printf("*%Ld,%Ld,%Ld,%Ld*\n", ua, ub, uc, ud);
          }

          // Nonconstant bitshifts
          {
            int64_t a = -1;
            int64_t b = a >> (29 - argc + 1);
            int64_t c = a >> (32 - argc + 1);
            int64_t d = a >> (34 - argc + 1);
            printf("*%Ld,%Ld,%Ld,%Ld*\n", a, b, c, d);
            uint64_t ua = -1;
            int64_t ub = ua >> (29 - argc + 1);
            int64_t uc = ua >> (32 - argc + 1);
            int64_t ud = ua >> (34 - argc + 1);
            printf("*%Ld,%Ld,%Ld,%Ld*\n", ua, ub, uc, ud);
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
          printf("*%Ld,%Ld,%Ld,%Ld*\n",   (a+b)/5000, (a-b)/5000, (a*3)/5000, (a/5)/5000);

          a -= 17; if (truthy()) a += 5; // confuse optimizer
          b -= 17; if (truthy()) b += 121; // confuse optimizer
          printf("*%Lx,%Lx,%Lx,%Lx*\n", b - a, b - a/2, b/2 - a, b - 20);

          if (truthy()) a += 5/b; // confuse optimizer
          if (truthy()) b += 121*(3+a/b); // confuse optimizer
          printf("*%Lx,%Lx,%Lx,%Lx*\n", a - b, a - b/2, a/2 - b, a - 20);

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

      Settings.CORRECT_SIGNS = 1

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
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      test_path = path_from_root('tests', 'core', 'test_i64_b')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_cmp(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      test_path = path_from_root('tests', 'core', 'test_i64_cmp')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_cmp2(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      test_path = path_from_root('tests', 'core', 'test_i64_cmp2')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_double(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      test_path = path_from_root('tests', 'core', 'test_i64_double')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_umul(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      test_path = path_from_root('tests', 'core', 'test_i64_umul')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_i64_precise(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

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

      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      code = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
      assert 'goog.math.Long' not in code, 'i64 precise math should not have been included if not actually used'

      # But if we force it to be included, it is. First, a case where we don't need it
      Settings.PRECISE_I64_MATH = 2
      self.do_run(open(path_from_root('tests', 'hello_world.c')).read(), 'hello')
      code = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
      assert 'goog.math.Long' in code, 'i64 precise math should be included if forced'

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
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')
    Settings.PRECISE_I64_MATH = 2

    test_path = path_from_root('tests', 'core', 'test_i64_llabs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_zextneg(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

    test_path = path_from_root('tests', 'core', 'test_i64_zextneg')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_7z(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

    test_path = path_from_root('tests', 'core', 'test_i64_7z')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, ['hallo'])

  def test_i64_i16(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

    test_path = path_from_root('tests', 'core', 'test_i64_i16')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_qdouble(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

    test_path = path_from_root('tests', 'core', 'test_i64_qdouble')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i64_varargs(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

    test_path = path_from_root('tests', 'core', 'test_i64_varargs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, 'waka fleefl asdfasdfasdfasdf'.split(' '))

  def test_i32_mul_precise(self):
    if self.emcc_args == None: return self.skip('needs ta2')

    test_path = path_from_root('tests', 'core', 'test_i32_mul_precise')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i32_mul_semiprecise(self):
    if Settings.ASM_JS: return self.skip('asm is always fully precise')

    Settings.PRECISE_I32_MUL = 0 # we want semiprecise here

    test_path = path_from_root('tests', 'core', 'test_i32_mul_semiprecise')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_i16_emcc_intrinsic(self):
    Settings.CORRECT_SIGNS = 1 # Relevant to this test

    test_path = path_from_root('tests', 'core', 'test_i16_emcc_intrinsic')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_double_i64_conversion(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2')

    test_path = path_from_root('tests', 'core', 'test_double_i64_conversion')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_float32_precise(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    Settings.PRECISE_F32 = 1

    test_path = path_from_root('tests', 'core', 'test_float32_precise')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_negative_zero(self):
    test_path = path_from_root('tests', 'core', 'test_negative_zero')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_llvm_intrinsics(self):
    if self.emcc_args == None: return self.skip('needs ta2')

    Settings.PRECISE_I64_MATH = 2 # for bswap64

    test_path = path_from_root('tests', 'core', 'test_llvm_intrinsics')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_bswap64(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2')

    test_path = path_from_root('tests', 'core', 'test_bswap64')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sha1(self):
    if self.emcc_args == None: return self.skip('needs ta2')

    self.do_run(open(path_from_root('tests', 'sha1.c')).read(), 'SHA1=15dd99a1991e0b3826fede3deffc1feba42278e6')

  def test_cube2md5(self):
    if self.emcc_args == None: return self.skip('needs emcc')
    self.emcc_args += ['--embed-file', 'cube2md5.txt']
    shutil.copyfile(path_from_root('tests', 'cube2md5.txt'), os.path.join(self.get_dir(), 'cube2md5.txt'))
    self.do_run(open(path_from_root('tests', 'cube2md5.cpp')).read(), open(path_from_root('tests', 'cube2md5.ok')).read())

  def test_cube2hash(self):
    try:
      old_chunk_size = os.environ.get('EMSCRIPT_MAX_CHUNK_SIZE') or ''
      os.environ['EMSCRIPT_MAX_CHUNK_SIZE'] = '1' # test splitting out each function to a chunk in emscripten.py (21 functions here)

      # A good test of i64 math
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2 C-style memory aliasing')
      self.do_run('', 'Usage: hashstring <seed>',
                  libraries=self.get_library('cube2hash', ['cube2hash.bc'], configure=None),
                  includes=[path_from_root('tests', 'cube2hash')])

      for text, output in [('fleefl', '892BDB6FD3F62E863D63DA55851700FDE3ACF30204798CE9'),
                           ('fleefl2', 'AA2CC5F96FC9D540CA24FDAF1F71E2942753DB83E8A81B61'),
                           ('64bitisslow', '64D8470573635EC354FEE7B7F87C566FCAF1EFB491041670')]:
        self.do_run('', 'hash value: ' + output, [text], no_build=True)
    finally:
      os.environ['EMSCRIPT_MAX_CHUNK_SIZE'] = old_chunk_size

    assert 'asm1' in test_modes
    if self.run_name == 'asm1' and not os.environ.get('EMCC_FAST_COMPILER'):
      assert Settings.RELOOP
      generated = open('src.cpp.o.js').read()
      main = generated[generated.find('function _main'):]
      main = main[:main.find('\n}')]
      num_vars = 0
      for v in re.findall('var [^;]+;', main):
        num_vars += v.count(',') + 1
      assert num_vars == 10, 'no variable elimination should have been run, but seeing %d' % num_vars

  def test_unaligned(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip('No meaning to unaligned addresses in q1')

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

      if self.is_le32():
        return self.skip('LLVM marks the reads of s as fully aligned, making this test invalid')
      else:
        self.do_run(src, '*12 : 1 : 12\n328157500735811.0,23,416012775903557.0,99\n')

      return # TODO: continue to the next part here

      # Test for undefined behavior in C. This is not legitimate code, but does exist

      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('No meaning to unaligned addresses without t2')

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
          printf("*%Ld*\n", *t);
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

    if self.is_le32():
      self.do_run(src, '''16,32
0,8,8,8
16,24,24,24
0.00,10,123.46,0.00 : 0.00,10,123.46,0.00
''')
    else:
      self.do_run(src, '''12,28
0,4,4,4
12,16,16,16
0.00,10,123.46,0.00 : 0.00,10,123.46,0.00
''')

  def test_unsigned(self):
      Settings.CORRECT_SIGNS = 1 # We test for exactly this sort of thing here
      Settings.CHECK_SIGNS = 0
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

      # Now let's see some code that should just work in USE_TYPED_ARRAYS == 2, but requires
      # corrections otherwise
      if Settings.USE_TYPED_ARRAYS == 2:
        Settings.CORRECT_SIGNS = 0
        Settings.CHECK_SIGNS = 1 if not Settings.ASM_JS else 0
      else:
        Settings.CORRECT_SIGNS = 1
        Settings.CHECK_SIGNS = 0

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
      if self.emcc_args is None: Settings.SAFE_HEAP = 0 # bitfields do loads on invalid areas, by design

      test_path = path_from_root('tests', 'core', 'test_bitfields')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_floatvars(self):
    test_path = path_from_root('tests', 'core', 'test_floatvars')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_closebitcasts(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2')
    test_path = path_from_root('tests', 'core', 'closebitcasts')
    src, output = (test_path + s for s in ('.c', '.txt'))
    self.do_run_from_file(src, output)

  def test_fast_math(self):
    if self.emcc_args is None: return self.skip('requires emcc')
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
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2')

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

  def test_frexp(self):
      test_path = path_from_root('tests', 'core', 'test_frexp')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_rounding(self):
      test_path = path_from_root('tests', 'core', 'test_rounding')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_fcvt(self):
      if self.emcc_args is None: return self.skip('requires emcc')

      test_path = path_from_root('tests', 'core', 'test_fcvt')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_llrint(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2')

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

  def test_strings(self):
      test_path = path_from_root('tests', 'core', 'test_strings')
      src, output = (test_path + s for s in ('.in', '.out'))

      for named in (0, 1):
        print named

        if os.environ.get('EMCC_FAST_COMPILER') == '1' and named: continue # no named globals in fastcomp

        Settings.NAMED_GLOBALS = named
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
      if self.emcc_args is None: return self.skip('requires emcc')
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
      if Settings.QUANTUM_SIZE == 1:
        self.do_run(src, 'sizeofs:6,8\n*C___: 0,3,6,9<24*\n*Carr: 0,3,6,9<24*\n*C__w: 0,3,9,12<24*\n*Cp1_: 1,2,5,8<24*\n*Cp2_: 0,2,5,8<24*\n*Cint: 0,3,4,7<24*\n*C4__: 0,3,4,7<24*\n*C4_2: 0,3,5,8<20*\n*C__z: 0,3,5,8<28*')
      else:
        self.do_run(src, 'sizeofs:6,8\n*C___: 0,6,12,20<24*\n*Carr: 0,6,12,20<24*\n*C__w: 0,6,12,20<24*\n*Cp1_: 4,6,12,20<24*\n*Cp2_: 0,6,12,20<24*\n*Cint: 0,8,12,20<24*\n*C4__: 0,8,12,20<24*\n*C4_2: 0,6,10,16<20*\n*C__z: 0,8,16,24<28*')

  def test_assert(self):
    test_path = path_from_root('tests', 'core', 'test_assert')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_libcextra(self):
      if self.emcc_args is None: return self.skip('needs emcc for libcextra')

      test_path = path_from_root('tests', 'core', 'test_libcextra')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_regex(self):
      if self.emcc_args is None: return self.skip('needs emcc for libcextra')

      test_path = path_from_root('tests', 'core', 'test_regex')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_longjmp(self):
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      test_path = path_from_root('tests', 'core', 'test_longjmp')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_longjmp2(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_longjmp2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_longjmp3(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_longjmp3')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_longjmp4(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_longjmp4')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_longjmp_funcptr(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_longjmp_funcptr')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_longjmp_repeat(self):
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      Settings.MAX_SETJMPS = 1

      test_path = path_from_root('tests', 'core', 'test_longjmp_repeat')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_longjmp_stacked(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_longjmp_stacked')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)


  def test_longjmp_exc(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_longjmp_exc')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_setjmp_many(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    src = r'''
      #include <stdio.h>
      #include <setjmp.h>

      int main(int argc) {
        jmp_buf buf;
        for (int i = 0; i < NUM; i++) printf("%d\n", setjmp(buf));
        if (argc-- == 1131) longjmp(buf, 11);
        return 0;
      }
    '''
    for num in [Settings.MAX_SETJMPS, Settings.MAX_SETJMPS+1]:
      print num
      self.do_run(src.replace('NUM', str(num)), '0\n' * num if num <= Settings.MAX_SETJMPS or not Settings.ASM_JS else 'build with a higher value for MAX_SETJMPS')

  def test_exceptions(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip("we don't support libcxx in q1")
      if self.emcc_args is None: return self.skip('need emcc to add in libcxx properly')
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      Settings.EXCEPTION_DEBUG = 1

      Settings.DISABLE_EXCEPTION_CATCHING = 0
      if '-O2' in self.emcc_args:
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
              std::cout << "Catched...";
          }

          try
          {
              function2();
          }
          catch (MyException e)
          {
              std::cout << "Catched...";
          }

          return 0;
      }
      '''

      Settings.DISABLE_EXCEPTION_CATCHING = 0
      if '-O2' in self.emcc_args:
        self.emcc_args.pop() ; self.emcc_args.pop() # disable closure to work around a closure bug
      self.do_run(src, 'Throw...Construct...Catched...Destruct...Throw...Construct...Copy...Catched...Destruct...Destruct...')

  def test_exception_2(self):
    if self.emcc_args is None: return self.skip('need emcc to add in libcxx properly')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')
    Settings.DISABLE_EXCEPTION_CATCHING = 0

    test_path = path_from_root('tests', 'core', 'test_exception_2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_white_list_exception(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    Settings.DISABLE_EXCEPTION_CATCHING = 2
    Settings.EXCEPTION_CATCHING_WHITELIST = ["__Z12somefunctionv"]
    Settings.INLINING_LIMIT = 50 # otherwise it is inlined and not identified

    test_path = path_from_root('tests', 'core', 'test_white_list_exception')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

    Settings.DISABLE_EXCEPTION_CATCHING = 0
    Settings.EXCEPTION_CATCHING_WHITELIST = []

  def test_uncaught_exception(self):
      if self.emcc_args is None: return self.skip('no libcxx inclusion without emcc')
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

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

  def test_typed_exceptions(self):
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      Settings.DISABLE_EXCEPTION_CATCHING = 0
      Settings.SAFE_HEAP = 0  # Throwing null will cause an ignorable null pointer access.
      src = open(path_from_root('tests', 'exceptions', 'typed.cpp'), 'r').read()
      expected = open(path_from_root('tests', 'exceptions', 'output.txt'), 'r').read()
      self.do_run(src, expected)

  def test_multiexception(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    Settings.DISABLE_EXCEPTION_CATCHING = 0

    test_path = path_from_root('tests', 'core', 'test_multiexception')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_std_exception(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    self.emcc_args += ['-s', 'SAFE_HEAP=0']

    test_path = path_from_root('tests', 'core', 'test_std_exception')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_async_exit(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    open('main.c', 'w').write(r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include "emscripten.h"

      void main_loop() {
        exit(EXIT_SUCCESS);
      }

      int main() {
        emscripten_set_main_loop(main_loop, 60, 0);
        return 0;
      }
    ''')

    Popen([PYTHON, EMCC, 'main.c']).communicate()
    self.assertNotContained('Reached an unreachable!', run_js(self.in_dir('a.out.js'), stderr=STDOUT))

  def test_exit_stack(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if Settings.ASM_JS: return self.skip('uses report_stack without exporting')

    Settings.INLINING_LIMIT = 50

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      extern "C" {
        extern void report_stack(int x);
      }

      char moar() {
        char temp[125];
        for (int i = 0; i < 125; i++) temp[i] = i*i;
        for (int i = 1; i < 125; i++) temp[i] += temp[i-1]/2;
        if (temp[100] != 99) exit(1);
        return temp[120];
      }

      int main(int argc, char *argv[]) {
        report_stack((int)alloca(4));
        printf("*%d*\n", moar());
        return 0;
      }
    '''

    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      var initialStack = -1;
      var _report_stack = function(x) {
        Module.print('reported');
        initialStack = x;
      }
      var Module = {
        postRun: function() {
          Module.print('Exit Status: ' + EXITSTATUS);
          Module.print('postRun');
          assert(initialStack == STACKTOP, [initialStack, STACKTOP]);
          Module.print('ok.');
        }
      };
    ''')

    self.emcc_args += ['--pre-js', 'pre.js']
    self.do_run(src, '''reported\nExit Status: 1\npostRun\nok.\n''')

  def test_class(self):
    test_path = path_from_root('tests', 'core', 'test_class')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_inherit(self):
    test_path = path_from_root('tests', 'core', 'test_inherit')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_isdigit_l(self):
      if self.emcc_args is None: return self.skip('no libcxx inclusion without emcc')

      test_path = path_from_root('tests', 'core', 'test_isdigit_l')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_iswdigit(self):
      if self.emcc_args is None: return self.skip('no libcxx inclusion without emcc')

      test_path = path_from_root('tests', 'core', 'test_iswdigit')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_polymorph(self):
      if self.emcc_args is None: return self.skip('requires emcc')

      test_path = path_from_root('tests', 'core', 'test_polymorph')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_segfault(self):
    if self.emcc_args is None: return self.skip('SAFE_HEAP without ta2 means we check types too, which hide segfaults')
    if Settings.ASM_JS: return self.skip('asm does not support safe heap')

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

  def test_safe_dyncalls(self):
    if Settings.ASM_JS: return self.skip('asm does not support missing function stack traces')
    if Settings.SAFE_HEAP: return self.skip('safe heap warning will appear instead')
    if self.emcc_args is None: return self.skip('need libc')

    Settings.SAFE_DYNCALLS = 1

    for cond, body, work in [(True, True, False), (True, False, False), (False, True, True), (False, False, False)]:
      print cond, body, work
      src = r'''
        #include <stdio.h>

        struct Classey {
          virtual void doIt() = 0;
        };

        struct D1 : Classey {
          virtual void doIt() BODY;
        };

        int main(int argc, char **argv)
        {
          Classey *p = argc COND 100 ? new D1() : NULL;
          printf("%p\n", p);
          p->doIt();

          return 0;
        }
      '''.replace('COND', '==' if cond else '!=').replace('BODY', r'{ printf("all good\n"); }' if body else '')
      self.do_run(src, 'dyncall error: vi' if not work else 'all good')

  def test_dynamic_cast(self):
      if self.emcc_args is None: return self.skip('need libcxxabi')

      test_path = path_from_root('tests', 'core', 'test_dynamic_cast')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_dynamic_cast_b(self):
      if self.emcc_args is None: return self.skip('need libcxxabi')

      test_path = path_from_root('tests', 'core', 'test_dynamic_cast_b')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_dynamic_cast_2(self):
    if self.emcc_args is None: return self.skip('need libcxxabi')

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

  def test_funcptrfunc(self):
    test_path = path_from_root('tests', 'core', 'test_funcptrfunc')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_funcptr_namecollide(self):
    test_path = path_from_root('tests', 'core', 'test_funcptr_namecollide')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, force_c=True)

  def test_emptyclass(self):
      if self.emcc_args is None: return self.skip('requires emcc')

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
    if self.emcc_args is None: return # too slow in other modes

    test_path = path_from_root('tests', 'core', 'test_alloca_stack')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, force_c=True)

  def test_stack_byval(self):
    if self.emcc_args is None: return # too slow in other modes

    test_path = path_from_root('tests', 'core', 'test_stack_byval')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack_varargs(self):
    if self.emcc_args is None: return # too slow in other modes

    Settings.INLINING_LIMIT = 50
    Settings.TOTAL_STACK = 1024

    test_path = path_from_root('tests', 'core', 'test_stack_varargs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_stack_varargs2(self):
    if self.emcc_args is None: return # too slow in other modes
    Settings.TOTAL_STACK = 1024
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
    if self.emcc_args is None: return self.skip('need c99')
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
    if self.emcc_args is None: return self.skip('need ta2 to compress type data on zeroinitializers')

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
      if Settings.QUANTUM_SIZE == 1:
        # Compressed memory. Note that sizeof() does give the fat sizes, however!
        self.do_run(src, '*0,0,0,1,2,3,4,5*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*12,20,5*')
      else:
        if self.is_le32():
          self.do_run(src, '*0,0,0,4,8,16,20,24*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*16,24,24*')
        else:
          self.do_run(src, '*0,0,0,4,8,12,16,20*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*12,20,20*')

  def test_ptrtoint(self):
      if self.emcc_args is None: return self.skip('requires emcc')

      runner = self
      def check_warnings(output):
          runner.assertEquals(filter(lambda line: 'Warning' in line, output.split('\n')).__len__(), 4)

      test_path = path_from_root('tests', 'core', 'test_ptrtoint')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, output_processor=check_warnings)

  def test_sizeof(self):
      if self.emcc_args is None: return self.skip('requires emcc')
      # Has invalid writes between printouts
      Settings.SAFE_HEAP = 0

      test_path = path_from_root('tests', 'core', 'test_sizeof')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, [], lambda x, err: x.replace('\n', '*'))

  def test_llvm_used(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('pnacl kills llvm_used')

    Building.LLVM_OPTS = 3

    test_path = path_from_root('tests', 'core', 'test_llvm_used')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_emscripten_api(self):
      #if Settings.MICRO_OPTS or Settings.RELOOP or Building.LLVM_OPTS: return self.skip('FIXME')

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
      self.do_run_from_file(src, output, post_build=check)

  def test_emscripten_get_now(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2')

      if self.run_name == 'o2':
        self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage
      self.do_run(open(path_from_root('tests', 'emscripten_get_now.cpp')).read(), 'Timer resolution is good.')

  def test_inlinejs(self):
      if not self.is_le32(): return self.skip('le32 needed for inline js')
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      test_path = path_from_root('tests', 'core', 'test_inlinejs')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

      if self.emcc_args == []: # opts will eliminate the comments
        out = open('src.cpp.o.js').read()
        for i in range(1, 5): assert ('comment%d' % i) in out

  def test_inlinejs2(self):
      if not self.is_le32(): return self.skip('le32 needed for inline js')
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      test_path = path_from_root('tests', 'core', 'test_inlinejs2')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_inlinejs3(self):
      test_path = path_from_root('tests', 'core', 'test_inlinejs3')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_memorygrowth(self):
    if Settings.USE_TYPED_ARRAYS == 0: return self.skip('memory growth is only supported with typed arrays')
    if Settings.ASM_JS: return self.skip('asm does not support memory growth yet')

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
        char *buf3 = (char*)malloc(totalMemory+1);
        buf3[argc] = (int)buf2;
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
    Settings.ALLOW_MEMORY_GROWTH = 1
    self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')
    win = open('src.cpp.o.js').read()

    if self.emcc_args and '-O2' in self.emcc_args:
      # Make sure ALLOW_MEMORY_GROWTH generates different code (should be less optimized)
      code_start = 'var TOTAL_MEMORY = '
      fail = fail[fail.find(code_start):]
      win = win[win.find(code_start):]
      assert len(fail) < len(win), 'failing code - without memory growth on - is more optimized, and smaller'

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
      if Settings.QUANTUM_SIZE == 1:
        self.do_run(src, '''*4*\n0:22016,0,8,12\n1:22018,1,12,8\n''')
      else:
        self.do_run(src, '''*16*\n0:22016,0,32,48\n1:22018,1,48,32\n''')

  def test_tinyfuncstr(self):
      if self.emcc_args is None: return self.skip('requires emcc')

      test_path = path_from_root('tests', 'core', 'test_tinyfuncstr')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_llvmswitch(self):
      Settings.CORRECT_SIGNS = 1

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

  def test_bigswitch(self):
    if self.run_name != 'default': return self.skip('TODO: issue #781')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    src = open(path_from_root('tests', 'bigswitch.cpp')).read()
    self.do_run(src, '''34962: GL_ARRAY_BUFFER (0x8892)
26214: what?
35040: GL_STREAM_DRAW (0x88E0)
''', args=['34962', '26214', '35040'])

  def test_indirectbr(self):
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      Building.COMPILER_TEST_OPTS = filter(lambda x: x != '-g', Building.COMPILER_TEST_OPTS)

      test_path = path_from_root('tests', 'core', 'test_indirectbr')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_indirectbr_many(self):
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('blockaddr > 255 requires ta2')

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
      if Settings.QUANTUM_SIZE == 1:
        self.do_run(src, '*4,2,3*\n*6,2,3*')
      else:
        self.do_run(src, '*4,3,4*\n*6,4,6*')

  def test_varargs(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip('FIXME: Add support for this')
      if not self.is_le32(): return self.skip('we do not support all varargs stuff without le32')

      test_path = path_from_root('tests', 'core', 'test_varargs')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_varargs_byval(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('FIXME: Add support for this')
    if self.is_le32(): return self.skip('clang cannot compile this code with that target yet')

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

      if not (self.emcc_args is None or self.emcc_args == []): return # Optimized code is missing the warning comments

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
      if 'i386-pc-linux-gnu' in COMPILER_OPTS:
        assert 'Casting a function pointer type to a potentially incompatible one' in output[1], 'Missing expected warning'
      else:
        print >> sys.stderr, 'skipping C/C++ conventions warning check, since not i386-pc-linux-gnu'

  def test_stdlibs(self):
      if self.emcc_args is None: return self.skip('requires emcc')
      if Settings.USE_TYPED_ARRAYS == 2:
          # Typed arrays = 2 + safe heap prints a warning that messes up our output.
          Settings.SAFE_HEAP = 0
      src = '''
        #include <stdio.h>
        #include <stdlib.h>
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

          return 0;
        }
        '''

      self.do_run(src, '*1,2,3,5,5,6*\n*stdin==0:0*\n*%*\n*5*\n*66.0*\n*10*\n*0*\n*-10*\n*18*\n*10*\n*0*\n*4294967286*\n*malloc(0)!=0:1*\n*cleaned*')

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
    if self.emcc_args is None: return self.skip('requires emcc')

    # tests strtoll for hex strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_hex')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtoll_dec(self):
    if self.emcc_args is None: return self.skip('requires emcc')

    # tests strtoll for decimal strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_dec')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtoll_bin(self):
    if self.emcc_args is None: return self.skip('requires emcc')

    # tests strtoll for binary strings (0x...) 
    test_path = path_from_root('tests', 'core', 'test_strtoll_bin')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strtoll_oct(self):
    if self.emcc_args is None: return self.skip('requires emcc')

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
    if self.emcc_args is None: return self.skip('requires emcc')
    src = open(path_from_root('tests', 'pthread', 'specific.c'), 'r').read()
    expected = open(path_from_root('tests', 'pthread', 'specific.c.txt'), 'r').read()
    self.do_run(src, expected, force_c=True)

  def test_tcgetattr(self):
    src = open(path_from_root('tests', 'termios', 'test_tcgetattr.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_time(self):
    # XXX Not sure what the right output is here. Looks like the test started failing with daylight savings changes. Modified it to pass again.
    src = open(path_from_root('tests', 'time', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'time', 'output.txt'), 'r').read()
    expected2 = open(path_from_root('tests', 'time', 'output2.txt'), 'r').read()
    self.do_run(src, [expected, expected2],
                 extra_emscripten_args=['-H', 'libc/time.h'])
                 #extra_emscripten_args=['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/langinfo.h,libc/time.h'])

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
    if os.environ.get('EMCC_FAST_COMPILER') == '1' and self.run_name == 'default': return self.skip('todo in fastcomp in default')

    # Some programs intentionally segfault themselves, we should compile that into a throw
    src = r'''
      int main () {
        *(volatile char *)0 = 0;
        return *(volatile char *)0;
      }
      '''
    self.do_run(src, 'fault on write to 0' if not Settings.ASM_JS else 'abort()')

  def test_trickystring(self):
    test_path = path_from_root('tests', 'core', 'test_trickystring')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_statics(self):
      # static initializers save i16 but load i8 for some reason (or i64 and load i8)
      if Settings.SAFE_HEAP:
        Settings.SAFE_HEAP = 3
        Settings.SAFE_HEAP_LINES = ['src.cpp:19', 'src.cpp:26', 'src.cpp:28']

      test_path = path_from_root('tests', 'core', 'test_statics')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_copyop(self):
      if self.emcc_args is None: return self.skip('requires emcc')

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
        return hashlib.sha1(result).hexdigest()

      self.do_run_from_file(src, output, output_nicerizer = check)

  def test_memcpy2(self):
      test_path = path_from_root('tests', 'core', 'test_memcpy2')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output)

  def test_getopt(self):
      if self.emcc_args is None: return self.skip('needs emcc for libc')

      test_path = path_from_root('tests', 'core', 'test_getopt')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, args=['-t', '12', '-n', 'foobar'])

  def test_getopt_long(self):
      if self.emcc_args is None: return self.skip('needs emcc for libc')

      test_path = path_from_root('tests', 'core', 'test_getopt_long')
      src, output = (test_path + s for s in ('.in', '.out'))

      self.do_run_from_file(src, output, args=['--file', 'foobar', '-b'])

  def test_memmove(self):
    test_path = path_from_root('tests', 'core', 'test_memmove')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_memmove2(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('need ta2')

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
    if Settings.QUANTUM_SIZE == 1: return self.skip('Test cannot work with q1')

    test_path = path_from_root('tests', 'core', 'test_bsearch')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

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
      if Settings.QUANTUM_SIZE == 1:
        # Compressed memory. Note that sizeof() does give the fat sizes, however!
        self.do_run(src, '*16,0,1,2,2,3|20,0,1,1,2,3,3,4|24,0,5,0,1,1,2,3,3,4*\n*0,0,0,1,2,62,63,64,72*\n*2*')
      else:
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
    Settings.NAMED_GLOBALS = 1

    self.build(supp, self.get_dir(), self.in_dir('supp.cpp'))
    shutil.move(self.in_dir('supp.cpp.o.js'), self.in_dir('liblib.so'))
    Settings.BUILD_AS_SHARED_LIB = 0

    Settings.RUNTIME_LINKED_LIBS = ['liblib.so'];
    self.do_run(main, 'supp: 54,2\nmain: 56\nsupp see: 543\nmain see: 76\nok.')

  def can_dlfcn(self):
    if os.environ.get('EMCC_FAST_COMPILER') == '1':
      self.skip('todo in fastcomp')
      return False

    if self.emcc_args and '--memory-init-file' in self.emcc_args:
      for i in range(len(self.emcc_args)):
        if self.emcc_args[i] == '--memory-init-file':
          self.emcc_args = self.emcc_args[:i] + self.emcc_args[i+2:]
          break

    if Settings.ASM_JS:
      Settings.DLOPEN_SUPPORT = 1
    else:
      Settings.NAMED_GLOBALS = 1

    if not self.is_le32():
      self.skip('need le32 for dlfcn support')
      return False
    else:
      return True

  def prep_dlfcn_lib(self):
    if Settings.ASM_JS:
      Settings.MAIN_MODULE = 0
      Settings.SIDE_MODULE = 1
    else:
      Settings.BUILD_AS_SHARED_LIB = 1
      Settings.INCLUDE_FULL_LIBRARY = 0

  def prep_dlfcn_main(self):
    if Settings.ASM_JS:
      Settings.MAIN_MODULE = 1
      Settings.SIDE_MODULE = 0
    else:
      Settings.BUILD_AS_SHARED_LIB = 0
      Settings.INCLUDE_FULL_LIBRARY = 1

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

  def test_dlfcn_qsort(self):
    if not self.can_dlfcn(): return

    if Settings.USE_TYPED_ARRAYS == 2:
      Settings.CORRECT_SIGNS = 1 # Needed for unsafe optimizations

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

    if Settings.ASM_JS and os.path.exists(SPIDERMONKEY_ENGINE[0]):
      out = run_js('liblib.so', engine=SPIDERMONKEY_ENGINE, full_output=True, stderr=STDOUT)
      if 'asm' in out:
        self.validate_asmjs(out)

  def test_dlfcn_data_and_fptr(self):
    if Settings.ASM_JS: return self.skip('this is not a valid case - libraries should not be able to access their parents globals willy nilly')
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

  def test_dlfcn_alias(self):
    if Settings.ASM_JS: return self.skip('this is not a valid case - libraries should not be able to access their parents globals willy nilly')

    Settings.LINKABLE = 1
    Settings.NAMED_GLOBALS = 1

    if Building.LLVM_OPTS == 2: return self.skip('LLVM LTO will optimize away stuff we expect from the shared library')

    lib_src = r'''
      #include <stdio.h>
      extern int parent_global;
      extern "C" void func() {
        printf("Parent global: %d.\n", parent_global);
      }
      '''
    dirname = self.get_dir()
    filename = os.path.join(dirname, 'liblib.cpp')
    Settings.BUILD_AS_SHARED_LIB = 1
    Settings.EXPORTED_FUNCTIONS = ['_func']
    self.build(lib_src, dirname, filename)
    shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

    src = r'''
      #include <dlfcn.h>

      int parent_global = 123;

      int main() {
        void* lib_handle;
        void (*fptr)();

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        fptr = (void (*)())dlsym(lib_handle, "func");
        fptr();
        parent_global = 456;
        fptr();

        return 0;
      }
      '''
    Settings.BUILD_AS_SHARED_LIB = 0
    Settings.INCLUDE_FULL_LIBRARY = 1
    Settings.EXPORTED_FUNCTIONS = ['_main']
    self.do_run(src, 'Parent global: 123.*Parent global: 456.*',
                output_nicerizer=lambda x, err: x.replace('\n', '*'),
                post_build=self.dlfcn_post_build,
                extra_emscripten_args=['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/time.h,libc/langinfo.h'])
    Settings.INCLUDE_FULL_LIBRARY = 0

  def test_dlfcn_varargs(self):
    if Settings.ASM_JS: return self.skip('this is not a valid case - libraries should not be able to access their parents globals willy nilly')

    if not self.can_dlfcn(): return

    Settings.LINKABLE = 1

    if Building.LLVM_OPTS == 2: return self.skip('LLVM LTO will optimize things that prevent shared objects from working')
    if Settings.QUANTUM_SIZE == 1: return self.skip('FIXME: Add support for this')

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
    if Settings.USE_TYPED_ARRAYS == 1: return self.skip('Does not work with USE_TYPED_ARRAYS=1')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')
    Settings.DLOPEN_SUPPORT = 1

    def post(filename):
      with open(filename) as f:
        for line in f:
          if 'var SYMBOL_TABLE' in line:
            table = line
            break
        else:
          raise Exception('Could not find symbol table!')
      table = table[table.find('{'):table.rfind('}')+1]
      # ensure there aren't too many globals; we don't want unnamed_addr
      assert table.count(',') <= 4

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

      typedef int (*FUNCTYPE)(const char *);

      int main() {
        void *lib_handle;
        FUNCTYPE func_ptr;
        char str[128];

        snprintf(str, sizeof(str), "foobar");

        lib_handle = dlopen("liblib.so", RTLD_NOW);
        assert(lib_handle != NULL);

        func_ptr = (FUNCTYPE)dlsym(lib_handle, "myfunc");
        assert(func_ptr != NULL);
        assert(func_ptr(str) == 6);

        puts("success");

        return 0;
      }
      '''
    Settings.EXPORTED_FUNCTIONS = ['_main', '_malloc']
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
    if not Settings.ASM_JS: return self.skip('needs asm')

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

  def test_rand(self):
    return self.skip('rand() is now random') # FIXME

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main() {
        printf("%d\n", rand());
        printf("%d\n", rand());

        srand(123);
        printf("%d\n", rand());
        printf("%d\n", rand());
        srand(123);
        printf("%d\n", rand());
        printf("%d\n", rand());

        unsigned state = 0;
        int r;
        r = rand_r(&state);
        printf("%d, %u\n", r, state);
        r = rand_r(&state);
        printf("%d, %u\n", r, state);
        state = 0;
        r = rand_r(&state);
        printf("%d, %u\n", r, state);

        return 0;
      }
      '''
    expected = '''
      1250496027
      1116302336
      440917656
      1476150784
      440917656
      1476150784
      12345, 12345
      1406932606, 3554416254
      12345, 12345
      '''
    self.do_run(src, re.sub(r'(^|\n)\s+', r'\1', expected))

  def test_strtod(self):
    if self.emcc_args is None: return self.skip('needs emcc for libc')

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
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('i64 mode 1 requires ta2')
    if Settings.QUANTUM_SIZE == 1: return self.skip('Q1 and I64_1 do not mix well yet')
    src = open(path_from_root('tests', 'parseInt', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'parseInt', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_transtrcase(self):
    test_path = path_from_root('tests', 'core', 'test_transtrcase')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_printf(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('i64 mode 1 requires ta2')
    self.banned_js_engines = [NODE_JS, V8_ENGINE] # SpiderMonkey and V8 do different things to float64 typed arrays, un-NaNing, etc.
    src = open(path_from_root('tests', 'printf', 'test.c'), 'r').read()
    expected = [open(path_from_root('tests', 'printf', 'output.txt'), 'r').read(),
                open(path_from_root('tests', 'printf', 'output_i64_1.txt'), 'r').read()]
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
    if self.emcc_args is None: return self.skip('needs i64 math')

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
    if self.emcc_args is None: return self.skip('requires ta2')

    test_path = path_from_root('tests', 'core', 'test_atoX')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_strstr(self):
    test_path = path_from_root('tests', 'core', 'test_strstr')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf(self):
    if self.emcc_args is None: return self.skip('needs emcc for libc')

    test_path = path_from_root('tests', 'core', 'test_sscanf')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_sscanf_2(self):
    # doubles
    if Settings.USE_TYPED_ARRAYS == 2:
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
Pass: 123456.789063 123456.789063
Pass: 123456.789063 123456.789063
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
    # i64
    if not Settings.USE_TYPED_ARRAYS == 2: return self.skip('64-bit sscanf only supported in ta2')

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
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip("need ta2 for full i64")

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
    if self.emcc_args is not None and '-O2' in self.emcc_args:
      self.emcc_args += ['--closure', '1'] # Use closure here, to test we don't break FS stuff
      self.emcc_args = filter(lambda x: x != '-g', self.emcc_args) # ensure we test --closure 1 --memory-init-file 1 (-g would disable closure)
      self.emcc_args += ["-s", "CHECK_HEAP_ALIGN=0"] # disable heap align check here, it mixes poorly with closure

    Settings.CORRECT_SIGNS = 1 # Just so our output is what we expect. Can flip them both.
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
    try_delete(mem_file)
    self.do_run(src, ('size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\nok.\ntexte\n', 'size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\ntexte\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\nok.\n'),
                 post_build=post, extra_emscripten_args=['-H', 'libc/fcntl.h'])
    if self.emcc_args and '--memory-init-file' in self.emcc_args:
      assert os.path.exists(mem_file)

  def test_files_m(self):
    # Test for Module.stdin etc.

    Settings.CORRECT_SIGNS = 1

    post = '''
def process(filename):
  src = \'\'\'
    var data = [10, 20, 40, 30];
    var Module = {
      stdin: function() { return data.pop() || null },
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
    self.do_run(src, ('got: 35\ngot: 45\ngot: 25\ngot: 15\nisatty? 0,0,1\n', 'isatty? 0,0,1\ngot: 35\ngot: 45\ngot: 25\ngot: 15\n'), post_build=post)

  def test_fwrite_0(self):
    test_path = path_from_root('tests', 'core', 'test_fwrite_0')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_fgetc_ungetc(self):
    src = open(path_from_root('tests', 'stdio', 'test_fgetc_ungetc.c'), 'r').read()
    self.do_run(src, 'success', force_c=True)

  def test_fgetc_unsigned(self):
    if self.emcc_args is None: return self.skip('requires emcc')
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
    if self.emcc_args is None: return self.skip('requires emcc')
    src = r'''
      #include <stdio.h>
      char buf[32];
      int main()
      {
        char *r = "SUCCESS";
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
    if self.emcc_args is None: return self.skip('requires emcc')
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

  def test_readdir(self):
    src = open(path_from_root('tests', 'dirent', 'test_readdir.c'), 'r').read()
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
    if self.emcc_args is None: return self.skip('need libc for wcslen()')
    if not self.is_le32(): return self.skip('this test uses inline js, which requires le32')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    self.do_run(open(path_from_root('tests', 'utf32.cpp')).read(), 'OK.')
    self.do_run(open(path_from_root('tests', 'utf32.cpp')).read(), 'OK.', args=['-fshort-wchar'])

  def test_direct_string_constant_usage(self):
    if self.emcc_args is None: return self.skip('requires libcxx')

    test_path = path_from_root('tests', 'core', 'test_direct_string_constant_usage')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_std_cout_new(self):
    if self.emcc_args is None: return self.skip('requires emcc')

    test_path = path_from_root('tests', 'core', 'test_std_cout_new')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_istream(self):
    if self.emcc_args is None: return self.skip('requires libcxx')

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
    if self.emcc_args is None: return self.skip('requires emcc')
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    src = open(path_from_root('tests', 'fs', 'test_nodefs_rw.c'), 'r').read()
    self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])

  def test_unistd_access(self):
    self.clear()
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'access.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'access.out'), 'r').read()
      Building.COMPILER_TEST_OPTS += ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_curdir(self):
    if not self.is_le32(): return self.skip('le32 needed for inline js')
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
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'truncate.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'truncate.out'), 'r').read()
      Building.COMPILER_TEST_OPTS += ['-D' + fs]
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
    if self.emcc_args is None: return self.skip('requires emcc')
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'unlink.c'), 'r').read()
      Building.COMPILER_TEST_OPTS += ['-D' + fs]
      self.do_run(src, 'success', force_c=True, js_engines=[NODE_JS])

  def test_unistd_links(self):
    self.clear()
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    for fs in ['MEMFS', 'NODEFS']:
      if WINDOWS and fs == 'NODEFS':
        print >> sys.stderr, 'Skipping NODEFS part of this test for test_unistd_links on Windows, since it would require administrative privileges.'
        # Also, other detected discrepancies if you do end up running this test on NODEFS:
        # test expects /, but Windows gives \ as path slashes.
        # Calling readlink() on a non-link gives error 22 EINVAL on Unix, but simply error 0 OK on Windows.
        continue
      src = open(path_from_root('tests', 'unistd', 'links.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'links.out'), 'r').read()
      Building.COMPILER_TEST_OPTS += ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_sleep(self):
    src = open(path_from_root('tests', 'unistd', 'sleep.c'), 'r').read()
    expected = open(path_from_root('tests', 'unistd', 'sleep.out'), 'r').read()
    self.do_run(src, expected)

  def test_unistd_io(self):
    self.clear()
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    if self.run_name == 'o2': return self.skip('non-asm optimized builds can fail with inline js')
    if self.emcc_args is None: return self.skip('requires emcc')
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'io.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'io.out'), 'r').read()
      Building.COMPILER_TEST_OPTS += ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_unistd_misc(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if not self.is_le32(): return self.skip('le32 needed for inline js')
    for fs in ['MEMFS', 'NODEFS']:
      src = open(path_from_root('tests', 'unistd', 'misc.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'misc.out'), 'r').read()
      Building.COMPILER_TEST_OPTS += ['-D' + fs]
      self.do_run(src, expected, js_engines=[NODE_JS])

  def test_uname(self):
    test_path = path_from_root('tests', 'core', 'test_uname')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_env(self):
    src = open(path_from_root('tests', 'env', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'env', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_systypes(self):
    src = open(path_from_root('tests', 'systypes', 'src.c'), 'r').read()
    expected = open(path_from_root('tests', 'systypes', 'output.txt'), 'r').read()
    self.do_run(src, expected)

  def test_getloadavg(self):
    test_path = path_from_root('tests', 'core', 'test_getloadavg')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_799(self):
    src = open(path_from_root('tests', '799.cpp'), 'r').read()
    self.do_run(src, '''Set PORT family: 0, port: 3979
Get PORT family: 0
PORT: 3979
''')

  def test_ctype(self):
    # The bit fiddling done by the macros using __ctype_b_loc requires this.
    Settings.CORRECT_SIGNS = 1
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

  def test_phiundef(self):
    test_path = path_from_root('tests', 'core', 'test_phiundef')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  # libc++ tests

  def test_iostream(self):
    if Settings.QUANTUM_SIZE == 1: return self.skip("we don't support libcxx in q1")
    if self.emcc_args is None: return self.skip('needs ta2 and emcc')

    src = '''
      #include <iostream>

      int main()
      {
        std::cout << "hello world" << std::endl << 77 << "." << std::endl;
        return 0;
      }
    '''

    # FIXME: should not have so many newlines in output here
    self.do_run(src, 'hello world\n77.\n')

  def test_stdvec(self):
    if self.emcc_args is None: return self.skip('requires emcc')

    test_path = path_from_root('tests', 'core', 'test_stdvec')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_reinterpreted_ptrs(self):
    if self.emcc_args is None: return self.skip('needs emcc and libc')

    test_path = path_from_root('tests', 'core', 'test_reinterpreted_ptrs')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_jansson(self):
      return self.skip('currently broken')

      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2')
      if Settings.SAFE_HEAP: return self.skip('jansson is not safe-heap safe')

      src = '''
        #include <jansson.h>
        #include <stdio.h>
        #include <string.h>

        int main()
        {
          const char* jsonString = "{\\"key\\": \\"value\\",\\"array\\": [\\"array_item1\\",\\"array_item2\\",\\"array_item3\\"],\\"dict\\":{\\"number\\": 3,\\"float\\": 2.2}}";

          json_error_t error;
          json_t *root = json_loadb(jsonString, strlen(jsonString), 0, &error);

          if(!root) {
            printf("Node `root` is `null`.");
            return 0;
          }

          if(!json_is_object(root)) {
            printf("Node `root` is no object.");
            return 0;
          }

          printf("%s\\n", json_string_value(json_object_get(root, "key")));

          json_t *array = json_object_get(root, "array");
          if(!array) {
            printf("Node `array` is `null`.");
            return 0;
          }

          if(!json_is_array(array)) {
            printf("Node `array` is no array.");
            return 0;
          }

          for(size_t i=0; i<json_array_size(array); ++i)
          {
            json_t *arrayNode = json_array_get(array, i);
            if(!root || !json_is_string(arrayNode))
              return 0;
            printf("%s\\n", json_string_value(arrayNode));
          }

          json_t *dict = json_object_get(root, "dict");
          if(!dict || !json_is_object(dict))
            return 0;

          json_t *numberNode = json_object_get(dict, "number");
          json_t *floatNode = json_object_get(dict, "float");

          if(!numberNode || !json_is_number(numberNode) ||
             !floatNode || !json_is_real(floatNode))
            return 0;

          printf("%i\\n", json_integer_value(numberNode));
          printf("%.2f\\n", json_number_value(numberNode));
          printf("%.2f\\n", json_real_value(floatNode));

          json_t *invalidNode = json_object_get(dict, "invalidNode");
          if(invalidNode)
            return 0;

          printf("%i\\n", json_number_value(invalidNode));

          json_decref(root);

          if(!json_is_object(root))
            printf("jansson!\\n");

          return 0;
        }
      '''
      self.do_run(src, 'value\narray_item1\narray_item2\narray_item3\n3\n3.00\n2.20\nJansson: Node with ID `0` not found. Context has `10` nodes.\n0\nJansson: No JSON context.\njansson!')

  ### 'Medium' tests

  def test_fannkuch(self):
      results = [ (1,0), (2,1), (3,2), (4,4), (5,7), (6,10), (7, 16), (8,22) ]
      for i, j in results:
        src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read()
        self.do_run(src, 'Pfannkuchen(%d) = %d.' % (i,j), [str(i)], no_build=i>1)

  def test_raytrace(self):
      if self.emcc_args is None: return self.skip('requires emcc')
      if Settings.USE_TYPED_ARRAYS == 2: return self.skip('Relies on double value rounding, extremely sensitive')

      src = open(path_from_root('tests', 'raytrace.cpp'), 'r').read().replace('double', 'float')
      output = open(path_from_root('tests', 'raytrace.ppm'), 'r').read()
      self.do_run(src, output, ['3', '16'])#, build_ll_hook=self.do_autodebug)

  def test_fasta(self):
      if self.emcc_args is None: return self.skip('requires emcc')
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
    if not Settings.ASM_JS: return self.skip('mainly a test for asm validation here')
    self.do_run(open(path_from_root('tests', 'whets.cpp')).read(), 'Single Precision C Whetstone Benchmark')

  def test_dlmalloc(self):
    if self.emcc_args is None: self.emcc_args = [] # dlmalloc auto-inclusion is only done if we use emcc

    self.banned_js_engines = [NODE_JS] # slower, and fail on 64-bit
    Settings.CORRECT_SIGNS = 2
    Settings.CORRECT_SIGNS_LINES = ['src.cpp:' + str(i+4) for i in [4816, 4191, 4246, 4199, 4205, 4235, 4227]]
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
    if self.emcc_args is None: return self.skip('only emcc will link in dlmalloc')
    # present part of the symbols of dlmalloc, not all
    src = open(path_from_root('tests', 'new.cpp')).read().replace('{{{ NEW }}}', 'new int').replace('{{{ DELETE }}}', 'delete') + '''
void *
operator new(size_t size)
{
printf("new %d!\\n", size);
return malloc(size);
}
'''
    self.do_run(src, 'new 4!\n*1,0*')

  def test_dlmalloc_partial_2(self):
    if self.emcc_args is None or 'SAFE_HEAP' in str(self.emcc_args) or 'CHECK_HEAP_ALIGN' in str(self.emcc_args): return self.skip('only emcc will link in dlmalloc, and we do unsafe stuff')
    # present part of the symbols of dlmalloc, not all. malloc is harder to link than new which is weak.

    test_path = path_from_root('tests', 'core', 'test_dlmalloc_partial_2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_libcxx(self):
    if self.emcc_args is None: return self.skip('requires emcc')
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
    if self.emcc_args is None: Settings.SAFE_HEAP = 0 # LLVM mixes i64 and i8 in the guard check

    test_path = path_from_root('tests', 'core', 'test_static_variable')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_fakestat(self):
    test_path = path_from_root('tests', 'core', 'test_fakestat')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_mmap(self):
    if self.emcc_args is None: return self.skip('requires emcc')

    Settings.TOTAL_MEMORY = 128*1024*1024

    test_path = path_from_root('tests', 'core', 'test_mmap')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)
    self.do_run_from_file(src, output, force_c=True)

  def test_mmap_file(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    for extra_args in [[], ['--no-heap-copy']]:
      self.emcc_args += ['--embed-file', 'data.dat'] + extra_args

      open(self.in_dir('data.dat'), 'w').write('data from the file ' + ('.' * 9000))

      src = open(path_from_root('tests', 'mmap_file.c')).read()
      self.do_run(src, '*\ndata from the file .\nfrom the file ......\n*\n')

  def test_cubescript(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if self.run_name == 'o2':
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

    Building.COMPILER_TEST_OPTS = filter(lambda x: x != '-g', Building.COMPILER_TEST_OPTS) # remove -g, so we have one test without it by default
    if self.emcc_args is None: Settings.SAFE_HEAP = 0 # Has some actual loads of unwritten-to places, in the C++ code...

    # Overflows happen in hash loop
    Settings.CORRECT_OVERFLOWS = 1
    Settings.CHECK_OVERFLOWS = 0

    if Settings.USE_TYPED_ARRAYS == 2:
      Settings.CORRECT_SIGNS = 1

    self.do_run(path_from_root('tests', 'cubescript'), '*\nTemp is 33\n9\n5\nhello, everyone\n*', main_file='command.cpp')

    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('skipping extra parts in fastcomp')

    assert 'asm2g' in test_modes
    if self.run_name == 'asm2g':
      results = {}
      original = open('src.cpp.o.js').read()
      results[Settings.ALIASING_FUNCTION_POINTERS] = len(original)
      Settings.ALIASING_FUNCTION_POINTERS = 1 - Settings.ALIASING_FUNCTION_POINTERS
      self.do_run(path_from_root('tests', 'cubescript'), '*\nTemp is 33\n9\n5\nhello, everyone\n*', main_file='command.cpp')
      final = open('src.cpp.o.js').read()
      results[Settings.ALIASING_FUNCTION_POINTERS] = len(final)
      open('original.js', 'w').write(original)
      print results
      assert results[1] < 0.99*results[0]
      assert ' & 3]()' in original, 'small function table exists'
      assert ' & 3]()' not in final, 'small function table does not exist'
      assert ' & 255]()' not in original, 'big function table does not exist'
      assert ' & 255]()' in final, 'big function table exists'

    assert 'asm1' in test_modes
    if self.run_name == 'asm1':
      generated = open('src.cpp.o.js').read()
      main = generated[generated.find('function runPostSets'):]
      main = main[:main.find('\n}')]
      assert main.count('\n') == 7, 'must not emit too many postSets: %d' % main.count('\n')

  def test_simd(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2')
    if Settings.ASM_JS: Settings.ASM_JS = 2 # does not validate
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_simd')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_simd2(self):
    if Settings.ASM_JS: Settings.ASM_JS = 2 # does not validate
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_simd2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_simd3(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2')
    if Settings.ASM_JS: Settings.ASM_JS = 2 # does not validate
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    test_path = path_from_root('tests', 'core', 'test_simd3')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_gcc_unmangler(self):
    if os.environ.get('EMCC_FAST_COMPILER') != '1': Settings.NAMED_GLOBALS = 1 # test coverage for this

    Building.COMPILER_TEST_OPTS += ['-I' + path_from_root('third_party')]

    self.do_run(open(path_from_root('third_party', 'gcc_demangler.c')).read(), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'])

    #### Code snippet that is helpful to search for nonportable optimizations ####
    #global LLVM_OPT_OPTS
    #for opt in ['-aa-eval', '-adce', '-always-inline', '-argpromotion', '-basicaa', '-basiccg', '-block-placement', '-break-crit-edges', '-codegenprepare', '-constmerge', '-constprop', '-correlated-propagation', '-count-aa', '-dce', '-deadargelim', '-deadtypeelim', '-debug-aa', '-die', '-domfrontier', '-domtree', '-dse', '-extract-blocks', '-functionattrs', '-globaldce', '-globalopt', '-globalsmodref-aa', '-gvn', '-indvars', '-inline', '-insert-edge-profiling', '-insert-optimal-edge-profiling', '-instcombine', '-instcount', '-instnamer', '-internalize', '-intervals', '-ipconstprop', '-ipsccp', '-iv-users', '-jump-threading', '-lazy-value-info', '-lcssa', '-lda', '-libcall-aa', '-licm', '-lint', '-live-values', '-loop-deletion', '-loop-extract', '-loop-extract-single', '-loop-index-split', '-loop-reduce', '-loop-rotate', '-loop-unroll', '-loop-unswitch', '-loops', '-loopsimplify', '-loweratomic', '-lowerinvoke', '-lowersetjmp', '-lowerswitch', '-mem2reg', '-memcpyopt', '-memdep', '-mergefunc', '-mergereturn', '-module-debuginfo', '-no-aa', '-no-profile', '-partial-inliner', '-partialspecialization', '-pointertracking', '-postdomfrontier', '-postdomtree', '-preverify', '-prune-eh', '-reassociate', '-reg2mem', '-regions', '-scalar-evolution', '-scalarrepl', '-sccp', '-scev-aa', '-simplify-libcalls', '-simplify-libcalls-halfpowr', '-simplifycfg', '-sink', '-split-geps', '-sretpromotion', '-strip', '-strip-dead-debug-info', '-strip-dead-prototypes', '-strip-debug-declare', '-strip-nondebug', '-tailcallelim', '-tailduplicate', '-targetdata', '-tbaa']:
    #  LLVM_OPT_OPTS = [opt]
    #  try:
    #    self.do_run(path_from_root(['third_party']), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'], main_file='gcc_demangler.c')
    #    print opt, "ok"
    #  except:
    #    print opt, "FAIL"

  def test_lua(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if Settings.QUANTUM_SIZE == 1: return self.skip('TODO: make this work')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

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
    if self.emcc_args is None: return self.skip('requires emcc')
    if Settings.QUANTUM_SIZE == 1: return self.skip('TODO: Figure out and try to fix')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    assert 'asm2g' in test_modes
    if self.run_name == 'asm2g':
      Settings.ALIASING_FUNCTION_POINTERS = 1 - Settings.ALIASING_FUNCTION_POINTERS # flip for some more coverage here

    if Settings.CORRECT_SIGNS == 0: Settings.CORRECT_SIGNS = 1 # Not sure why, but needed

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
    if self.emcc_args is None: return self.skip('Very slow without ta2, and we would also need to include dlmalloc manually without emcc')
    if not self.is_le32(): return self.skip('fails on x86 due to a legalization issue on llvm 3.3')
    if Settings.QUANTUM_SIZE == 1: return self.skip('TODO FIXME')
    self.banned_js_engines = [NODE_JS] # OOM in older node

    Settings.CORRECT_SIGNS = 1
    Settings.CORRECT_OVERFLOWS = 0
    Settings.CORRECT_ROUNDINGS = 0
    if self.emcc_args is None: Settings.SAFE_HEAP = 0 # uses time.h to set random bytes, other stuff
    Settings.DISABLE_EXCEPTION_CATCHING = 1
    Settings.FAST_MEMORY = 4*1024*1024
    Settings.EXPORTED_FUNCTIONS += ['_sqlite3_open', '_sqlite3_close', '_sqlite3_exec', '_sqlite3_free', '_callback'];
    if Settings.ASM_JS == 1 and '-g' in self.emcc_args:
      print "disabling inlining" # without registerize (which -g disables), we generate huge amounts of code
      Settings.INLINING_LIMIT = 50

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
    if not Settings.USE_TYPED_ARRAYS == 2: return self.skip('works in general, but cached build will be optimized and fail, so disable this')

    if Settings.ASM_JS:
      self.banned_js_engines = [NODE_JS] # TODO investigate

    if self.emcc_args is not None and '-O2' in self.emcc_args and 'ASM_JS=0' not in self.emcc_args: # without asm, closure minifies Math.imul badly
      self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

    Settings.CORRECT_SIGNS = 1

    self.do_run(open(path_from_root('tests', 'zlib', 'example.c'), 'r').read(),
                 open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                 libraries=self.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a']),
                 includes=[path_from_root('tests', 'zlib')],
                 force_c=True)

  def test_the_bullet(self): # Called thus so it runs late in the alphabetical cycle... it is long
    if self.emcc_args is None: return self.skip('requires emcc')
    if Building.LLVM_OPTS and self.emcc_args is None: Settings.SAFE_HEAP = 0 # Optimizations make it so we do not have debug info on the line we need to ignore

    Settings.DEAD_FUNCTIONS = ['__ZSt9terminatev']

    # Note: this is also a good test of per-file and per-line changes (since we have multiple files, and correct specific lines)
    if Settings.SAFE_HEAP:
      # Ignore bitfield warnings
      Settings.SAFE_HEAP = 3
      Settings.SAFE_HEAP_LINES = ['btVoronoiSimplexSolver.h:40', 'btVoronoiSimplexSolver.h:41',
                                  'btVoronoiSimplexSolver.h:42', 'btVoronoiSimplexSolver.h:43']

    for use_cmake in [False, True]: # If false, use a configure script to configure Bullet build.
      print 'cmake', use_cmake
      # Windows cannot run configure sh scripts.
      if WINDOWS and not use_cmake:
        continue

      def test():
        self.do_run(open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'), 'r').read(),
                     [open(path_from_root('tests', 'bullet', 'output.txt'), 'r').read(), # different roundings
                      open(path_from_root('tests', 'bullet', 'output2.txt'), 'r').read(),
                      open(path_from_root('tests', 'bullet', 'output3.txt'), 'r').read()],
                     libraries=get_bullet_library(self, use_cmake),
                     includes=[path_from_root('tests', 'bullet', 'src')])
      test()

      assert 'asm2g' in test_modes
      if self.run_name == 'asm2g' and not use_cmake and os.environ.get('EMCC_FAST_COMPILER') != '1':
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
    if self.emcc_args is None: return self.skip('very slow, we only do this in emcc runs')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    Settings.CORRECT_OVERFLOWS = 1
    Settings.CORRECT_SIGNS = 1

    Building.COMPILER_TEST_OPTS += [
      '-I' + path_from_root('tests', 'freetype', 'include'),
      '-I' + path_from_root('tests', 'poppler', 'include'),
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
      Module.print("Data: " + JSON.stringify(FS.root.contents['filename-1.ppm'].contents.map(function(x) { return unSign(x, 8) })));
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

  def test_openjpeg(self):
    if self.emcc_args is None: return self.skip('needs libc for getopt')

    Building.COMPILER_TEST_OPTS = filter(lambda x: x != '-g', Building.COMPILER_TEST_OPTS) # remove -g, so we have one test without it by default

    if Settings.USE_TYPED_ARRAYS == 2:
      Settings.CORRECT_SIGNS = 1
    else:
      Settings.CORRECT_SIGNS = 2
      Settings.CORRECT_SIGNS_LINES = ["mqc.c:566", "mqc.c:317"]

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
    "Module.print('Data: ' + JSON.stringify(FS.analyzePath('image.raw').object.contents));"
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
    if self.emcc_args and '-O2' in self.emcc_args and 'EMCC_DEBUG' not in os.environ and '-g' in self.emcc_args:
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
        self.assertIdentical(clean(open('release.js').read()), clean(open('debug%d.js' % debug).read())) # EMCC_DEBUG=1 mode must not generate different code!
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

  def test_python(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if Settings.QUANTUM_SIZE == 1: return self.skip('TODO: make this work')
    if not self.is_le32(): return self.skip('fails on non-le32') # FIXME

    #Settings.EXPORTED_FUNCTIONS += ['_PyRun_SimpleStringFlags'] # for the demo

    if self.is_le32():
      bitcode = path_from_root('tests', 'python', 'python.le32.bc')
    else:
      bitcode = path_from_root('tests', 'python', 'python.small.bc')

    self.do_ll_run(bitcode,
                    'hello python world!\n[0, 2, 4, 6]\n5\n22\n5.470000',
                    args=['-S', '-c' '''print "hello python world!"; print [x*2 for x in range(4)]; t=2; print 10-3-t; print (lambda x: x*2)(11); print '%f' % 5.47'''])

  def test_lifetime(self):
    if self.emcc_args is None: return self.skip('test relies on emcc opts')

    self.do_ll_run(path_from_root('tests', 'lifetime.ll'), 'hello, world!\n')
    if '-O1' in self.emcc_args or '-O2' in self.emcc_args:
      assert 'a18' not in open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read(), 'lifetime stuff and their vars must be culled'

  # Test cases in separate files. Note that these files may contain invalid .ll!
  # They are only valid enough for us to read for test purposes, not for llvm-as
  # to process.
  def test_cases(self):
    if Building.LLVM_OPTS: return self.skip("Our code is not exactly 'normal' llvm assembly")

    emcc_args = self.emcc_args

    try:
      os.environ['EMCC_LEAVE_INPUTS_RAW'] = '1'
      Settings.CHECK_OVERFLOWS = 0

      for name in glob.glob(path_from_root('tests', 'cases', '*.ll')):
        shortname = name.replace('.ll', '')
        if '' not in shortname: continue
        if os.environ.get('EMCC_FAST_COMPILER') == '1' and os.path.basename(shortname) in [
          'structparam', 'uadd_overflow_ta2', 'extendedprecision', 'issue_39', 'emptystruct', 'phinonexist', 'quotedlabel', 'oob_ta2', 'phientryimplicit', 'phiself', 'invokebitcast', # invalid ir
          'structphiparam', 'callwithstructural_ta2', 'callwithstructural64_ta2', 'structinparam', # pnacl limitations in ExpandStructRegs
          '2xi40', # pnacl limitations in ExpandGetElementPtr
          'legalizer_ta2', '514_ta2', # pnacl limitation in not legalizing i104, i96, etc.
          'longjmp_tiny', 'longjmp_tiny_invoke', 'longjmp_tiny_phi', 'longjmp_tiny_phi2', 'longjmp_tiny_invoke_phi', 'indirectbrphi', 'ptrtoint_blockaddr', 'quoted', # current fastcomp limitations FIXME
          'sillyfuncast', 'sillyfuncast2', 'sillybitcast', 'atomicrmw_unaligned' # TODO XXX
        ]: continue
        if '_ta2' in shortname and not Settings.USE_TYPED_ARRAYS == 2:
          print self.skip('case "%s" only relevant for ta2' % shortname)
          continue
        if '_noasm' in shortname and Settings.ASM_JS:
          print self.skip('case "%s" not relevant for asm.js' % shortname)
          continue
        if '_le32' in shortname and not self.is_le32():
          print self.skip('case "%s" not relevant for non-le32 target' % shortname)
          continue
        self.emcc_args = emcc_args
        if os.path.exists(shortname + '.emcc'):
          if not self.emcc_args: continue
          self.emcc_args = self.emcc_args + json.loads(open(shortname + '.emcc').read())
        print >> sys.stderr, "Testing case '%s'..." % shortname
        output_file = path_from_root('tests', 'cases', shortname + '.txt')
        if Settings.QUANTUM_SIZE == 1:
          q1_output_file = path_from_root('tests', 'cases', shortname + '_q1.txt')
          if os.path.exists(q1_output_file):
            output_file = q1_output_file
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
      del os.environ['EMCC_LEAVE_INPUTS_RAW']
      self.emcc_args = emcc_args

  def test_fuzz(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2')

    Building.COMPILER_TEST_OPTS += ['-I' + path_from_root('tests', 'fuzz')]

    def run_all(x):
      print x
      for name in glob.glob(path_from_root('tests', 'fuzz', '*.c')):
        #if os.path.basename(name) != '4.c': continue
        if os.environ.get('EMCC_FAST_COMPILER') == '1' and os.path.basename(name) in ['17.c']: continue # pnacl limitation in not legalizing i104, i96, etc.

        print name
        self.do_run(open(path_from_root('tests', 'fuzz', name)).read(),
                    open(path_from_root('tests', 'fuzz', name + '.txt')).read(), force_c=True)

    run_all('normal')

    self.emcc_args += ['--llvm-lto', '1']

    run_all('lto')

  # Autodebug the code
  def do_autodebug(self, filename):
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
    self.test_structs()

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

  def test_corruption(self):
    if Settings.ASM_JS: return self.skip('cannot use corruption checks in asm')
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2 for actual test')

    Settings.CORRUPTION_CHECK = 1

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      int main(int argc, char **argv) {
        int size = 1024*argc;
        char *buffer = (char*)malloc(size);
      #if CORRUPT
        memset(buffer, argc, size+15);
      #else
        memset(buffer, argc, size);
      #endif
        for (int x = 0; x < size; x += argc*3) buffer[x] = x/3;
        int ret = 0;
        for (int x = 0; x < size; x++) ret += buffer[x];
        free(buffer);
        printf("All ok, %d\n", ret);
      }
    '''

    for corrupt in [1]:
      self.do_run(src.replace('CORRUPT', str(corrupt)), 'Heap corruption detected!' if corrupt else 'All ok, 4209')

  def test_corruption_2(self):
    if Settings.ASM_JS: return self.skip('cannot use corruption checks in asm')
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2 for actual test')

    Settings.SAFE_HEAP = 1
    Settings.CORRUPTION_CHECK = 1

    # test for free(0), malloc(0), etc.
    test_path = path_from_root('tests', 'core', 'test_corruption_2')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_corruption_3(self):
    if Settings.ASM_JS: return self.skip('cannot use corruption checks in asm')
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip('needs ta2 for actual test')

    Settings.CORRUPTION_CHECK = 1

    # realloc
    test_path = path_from_root('tests', 'core', 'test_corruption_3')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  ### Integration tests

  def test_ccall(self):
    if self.emcc_args is not None and '-O2' in self.emcc_args:
      self.emcc_args += ['--closure', '1'] # Use closure here, to test we export things right

    post = '''
def process(filename):
  src = \'\'\'
    var Module = { 'noInitialRun': true };
    \'\'\' + open(filename, 'r').read() + \'\'\'
    Module.addOnExit(function () {
      Module.print('*');
      var ret;
      ret = Module['ccall']('get_int', 'number'); Module.print([typeof ret, ret]);
      ret = ccall('get_float', 'number'); Module.print([typeof ret, ret.toFixed(2)]);
      ret = ccall('get_string', 'string'); Module.print([typeof ret, ret]);
      ret = ccall('print_int', null, ['number'], [12]); Module.print(typeof ret);
      ret = ccall('print_float', null, ['number'], [14.56]); Module.print(typeof ret);
      ret = ccall('print_string', null, ['string'], ["cheez"]); Module.print(typeof ret);
      ret = ccall('print_string', null, ['array'], [[97, 114, 114, 45, 97, 121, 0]]); Module.print(typeof ret);
      ret = ccall('multi', 'number', ['number', 'number', 'number', 'string'], [2, 1.4, 3, 'more']); Module.print([typeof ret, ret]);
      var p = ccall('malloc', 'pointer', ['number'], [4]);
      setValue(p, 650, 'i32');
      ret = ccall('pointer', 'pointer', ['pointer'], [p]); Module.print([typeof ret, getValue(ret, 'i32')]);
      Module.print('*');
      // part 2: cwrap
      var multi = Module['cwrap']('multi', 'number', ['number', 'number', 'number', 'string']);
      Module.print(multi(2, 1.4, 3, 'atr'));
      Module.print(multi(8, 5.4, 4, 'bret'));
      Module.print('*');
      // part 3: avoid stack explosion
      for (var i = 0; i < TOTAL_STACK/60; i++) {
        ccall('multi', 'number', ['number', 'number', 'number', 'string'], [0, 0, 0, '123456789012345678901234567890123456789012345678901234567890']);
      }
      Module.print('stack is ok.');
    });
    Module.callMain();
  \'\'\'
  open(filename, 'w').write(src)
'''

    Settings.EXPORTED_FUNCTIONS += ['_get_int', '_get_float', '_get_string', '_print_int', '_print_float', '_print_string', '_multi', '_pointer', '_malloc']

    test_path = path_from_root('tests', 'core', 'test_ccall')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output, post_build=post)

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
      test("*9*\n-s DEAD_FUNCTIONS='[\"_unused\"]'")
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

  def test_asm_pgo(self):
    if not Settings.ASM_JS: return self.skip('this is a test for PGO for asm (NB: not *in* asm)')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

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
    if self.emcc_args is None: return self.skip('requires emcc')

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      extern "C" {
        int other_function() { return 5; }
      }

      int main() {
        printf("waka!\n");
        return 0;
      }
    '''
    open('exps', 'w').write('["_main","_other_function"]')

    self.emcc_args += ['-s', 'EXPORTED_FUNCTIONS=@exps']
    self.do_run(src, '''waka!''')
    assert 'other_function' in open('src.cpp.o.js').read()

  def test_add_function(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    Settings.INVOKE_RUN = 0
    Settings.RESERVED_FUNCTION_POINTERS = 1

    src = r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main(int argc, char **argv) {
        int fp = atoi(argv[1]);
        printf("fp: %d\n", fp);
        void (*f)(int) = reinterpret_cast<void (*)(int)>(fp);
        f(7);
        return 0;
      }
    '''

    open(os.path.join(self.get_dir(), 'post.js'), 'w').write('''
      var newFuncPtr = Runtime.addFunction(function(num) {
        Module.print('Hello ' + num + ' from JS!');
      });
      Module.callMain([newFuncPtr.toString()]);
    ''')

    self.emcc_args += ['--post-js', 'post.js']
    self.do_run(src, '''Hello 7 from JS!''')

    if Settings.ASM_JS:
      Settings.RESERVED_FUNCTION_POINTERS = 0
      self.do_run(src, '''Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.''')
      generated = open('src.cpp.o.js').read()
      assert 'jsCall' not in generated
      Settings.RESERVED_FUNCTION_POINTERS = 1

      Settings.ALIASING_FUNCTION_POINTERS = 1 - Settings.ALIASING_FUNCTION_POINTERS # flip the test
      self.do_run(src, '''Hello 7 from JS!''')

  def test_demangle_stacks(self):
    if Settings.ASM_JS: return self.skip('spidermonkey has stack trace issues')

    test_path = path_from_root('tests', 'core', 'test_demangle_stacks')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

  def test_embind(self):
    if self.emcc_args is None: return self.skip('requires emcc')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')
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
    if self.emcc_args is None: return self.skip('requires emcc')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')
    Building.COMPILER_TEST_OPTS += ['--bind', '--post-js', 'post.js']
    open('post.js', 'w').write('''
      Module.print('lerp ' + Module.lerp(1, 2, 0.66) + '.');
    ''')
    src = r'''
      #include <stdio.h>
      #include <SDL/SDL.h>
      #include <emscripten/bind.h>
      using namespace emscripten;
      float lerp(float a, float b, float t) {
          return (1 - t) * a + t * b;
      }
      EMSCRIPTEN_BINDINGS(my_module) {
          function("lerp", &lerp);
      }
    '''
    self.do_run(src, 'lerp 1.66');

  def test_scriptaclass(self):
      if self.emcc_args is None: return self.skip('requires emcc')
      if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

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

      if self.emcc_args is not None and '-O2' in self.emcc_args and 'ASM_JS=0' not in self.emcc_args: # without asm, closure minifies Math.imul badly
        self.emcc_args += ['--closure', '1'] # Use closure here, to test we export things right

      # Way 2: use CppHeaderParser

      Settings.RUNTIME_TYPE_INFO = 1

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
      if self.emcc_args is None: return self.skip('requires emcc')

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

  def test_typeinfo(self):
    if self.emcc_args is not None and self.emcc_args != []: return self.skip('full LLVM opts optimize out all the code that uses the type')

    Settings.RUNTIME_TYPE_INFO = 1
    if Settings.QUANTUM_SIZE != 4: return self.skip('We assume normal sizes in the output here')

    src = '''
      #include<stdio.h>
      struct UserStruct {
        int x;
        char y;
        short z;
      };
      struct Encloser {
        short x;
        UserStruct us;
        int y;
      };
      int main() {
        Encloser e;
        e.us.y = 5;
        printf("*ok:%d*\\n", e.us.y);
        return 0;
      }
    '''

    post = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{POST_RUN_ADDITIONS}}',
    \'\'\'
      if (Runtime.typeInfo) {
        Module.print('|' + Runtime.typeInfo.UserStruct.fields + '|' + Runtime.typeInfo.UserStruct.flatIndexes + '|');
        var t = Runtime.generateStructInfo(['x', { us: ['x', 'y', 'z'] }, 'y'], 'Encloser')
        Module.print('|' + [t.x, t.us.x, t.us.y, t.us.z, t.y] + '|');
        Module.print('|' + JSON.stringify(Runtime.generateStructInfo(['x', 'y', 'z'], 'UserStruct')) + '|');
      } else {
        Module.print('No type info.');
      }
    \'\'\'
  )
  open(filename, 'w').write(src)
'''

    self.do_run(src,
                 '*ok:5*\n|i32,i8,i16|0,4,6|\n|0,4,8,10,12|\n|{"__size__":8,"x":0,"y":4,"z":6}|',
                 post_build=post)

    # Make sure that without the setting, we don't spam the .js with the type info
    Settings.RUNTIME_TYPE_INFO = 0
    self.do_run(src, 'No type info.', post_build=post)

  ### Tests for tools

  def test_safe_heap(self):
    if not Settings.SAFE_HEAP: return self.skip('We need SAFE_HEAP to test SAFE_HEAP')
    if Settings.USE_TYPED_ARRAYS == 2: return self.skip('It is ok to violate the load-store assumption with TA2')
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
      self.do_run(src, '*nothingatall*')
    except Exception, e:
      # This test *should* fail, by throwing this exception
      assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

    # And we should not fail if we disable checking on that line

    Settings.SAFE_HEAP = 3
    Settings.SAFE_HEAP_LINES = ["src.cpp:7"]

    self.do_run(src, '*ok*')

    # But if we disable the wrong lines, we still fail

    Settings.SAFE_HEAP_LINES = ["src.cpp:99"]

    try:
      self.do_run(src, '*nothingatall*')
    except Exception, e:
      # This test *should* fail, by throwing this exception
      assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

    # And reverse the checks with = 2

    Settings.SAFE_HEAP = 2
    Settings.SAFE_HEAP_LINES = ["src.cpp:99"]

    self.do_run(src, '*ok*')

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
      self.do_ll_run(all_name, '*nothingatall*')
    except Exception, e:
      # This test *should* fail, by throwing this exception
      assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

    # And we should not fail if we disable checking on those lines

    Settings.SAFE_HEAP = 3
    Settings.SAFE_HEAP_LINES = ["module.cpp:7", "main.cpp:9"]

    self.do_ll_run(all_name, '*ok*')

    # But we will fail if we do not disable exactly what we need to - any mistake leads to error

    for lines in [["module.cpp:22", "main.cpp:9"], ["module.cpp:7", "main.cpp:29"], ["module.cpp:127", "main.cpp:449"], ["module.cpp:7"], ["main.cpp:9"]]:
      Settings.SAFE_HEAP_LINES = lines
      try:
        self.do_ll_run(all_name, '*nothingatall*')
      except Exception, e:
        # This test *should* fail, by throwing this exception
        assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

  def test_debug(self):
    if '-g' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g')
    if self.emcc_args is not None:
      if '-O1' in self.emcc_args or '-O2' in self.emcc_args: return self.skip('optimizations remove LLVM debug info')

    src = '''
      #include <stdio.h>
      #include <assert.h>

      void checker(int x) {
        x += 20;
        assert(x < 15); // this is line 7!
      }

      int main() {
        checker(10);
        return 0;
      }
    '''
    try:
      self.do_run(src, '*nothingatall*')
    except Exception, e:
      # This test *should* fail
      assert 'Assertion failed: x < 15' in str(e), str(e)

    lines = open('src.cpp.o.js', 'r').readlines()
    lines = filter(lambda line: '___assert_fail(' in line or '___assert_func(' in line, lines)
    found_line_num = any(('//@line 7 "' in line) for line in lines)
    found_filename = any(('src.cpp"\n' in line) for line in lines)
    assert found_line_num, 'Must have debug info with the line number'
    assert found_filename, 'Must have debug info with the filename'

  def test_source_map(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip("doesn't pass without typed arrays")
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')
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
    no_maps_file = re.sub(' *//@.*$', '', no_maps_file, flags=re.MULTILINE)
    Building.COMPILER_TEST_OPTS.append('-g4')

    def build_and_check():
      import json
      Building.emcc(src_filename, Settings.serialize() + self.emcc_args +
          Building.COMPILER_TEST_OPTS, out_filename, stderr=PIPE)
      with open(out_filename) as f: out_file = f.read()
      # after removing the @line and @sourceMappingURL comments, the build
      # result should be identical to the non-source-mapped debug version.
      # this is worth checking because the parser AST swaps strings for token
      # objects when generating source maps, so we want to make sure the
      # optimizer can deal with both types.
      out_file = re.sub(' *//@.*$', '', out_file, flags=re.MULTILINE)
      def clean(code):
        code = code.replace('{\n}', '{}')
        return '\n'.join(sorted(code.split('\n')))
      self.assertIdentical(clean(no_maps_file), clean(out_file))
      map_filename = out_filename + '.map'
      data = json.load(open(map_filename, 'r'))
      self.assertPathsIdentical(out_filename, data['file'])
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

    # EMCC_DEBUG=2 causes lots of intermediate files to be written, and so
    # serves as a stress test for source maps because it needs to correlate
    # line numbers across all those files.
    old_emcc_debug = os.environ.get('EMCC_DEBUG', None)
    os.environ.pop('EMCC_DEBUG', None)
    try:
      build_and_check()
      os.environ['EMCC_DEBUG'] = '2'
      build_and_check()
    finally:
      if old_emcc_debug is not None:
        os.environ['EMCC_DEBUG'] = old_emcc_debug
      else:
        os.environ.pop('EMCC_DEBUG', None)

  def test_exception_source_map(self):
    if Settings.USE_TYPED_ARRAYS != 2: return self.skip("doesn't pass without typed arrays")
    if '-g4' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g4')
    if NODE_JS not in JS_ENGINES: return self.skip('sourcemapper requires Node to run')
    if os.environ.get('EMCC_FAST_COMPILER') == '1': return self.skip('todo in fastcomp')

    src = '''
      #include <stdio.h>

      __attribute__((noinline)) void foo(int i) {
          if (i < 10) throw i; // line 5
      }

      int main() {
        int i;
        scanf("%d", &i);
        foo(i);
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
        if m['originalLine'] == 5 and '__cxa_throw' in lines[m['generatedLine']]:
          return
      assert False, 'Must label throw statements with line numbers'

    dirname = self.get_dir()
    self.build(src, dirname, os.path.join(dirname, 'src.cpp'), post_build=(None, post))

  def test_emscripten_log(self):
    self.banned_js_engines = [SPIDERMONKEY_ENGINE] # XXX, emscripten_log is broken in spidermonkey currently, issue #1970
    if self.emcc_args is None: return self.skip('This test needs libc.')
    if '-g' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g')
    self.do_run('#define RUN_FROM_JS_SHELL\n' + open(path_from_root('tests', 'emscripten_log', 'emscripten_log.cpp')).read(), "Success!")

  def test_linespecific(self):
    if Settings.ASM_JS: return self.skip('asm always has corrections on')

    if '-g' not in Building.COMPILER_TEST_OPTS: Building.COMPILER_TEST_OPTS.append('-g')
    if self.emcc_args:
      self.emcc_args += ['--llvm-opts', '0'] # llvm full opts make the expected failures here not happen
      Building.COMPILER_TEST_OPTS += ['--llvm-opts', '0']

    Settings.CHECK_SIGNS = 0
    Settings.CHECK_OVERFLOWS = 0

    # Signs

    src = '''
      #include <stdio.h>
      #include <assert.h>

      int main()
      {
        int varey = 100;
        unsigned int MAXEY = -1;
        printf("*%d*\\n", varey >= MAXEY); // 100 >= -1? not in unsigned!
      }
    '''

    Settings.CORRECT_SIGNS = 0
    self.do_run(src, '*1*') # This is a fail - we expect 0

    Settings.CORRECT_SIGNS = 1
    self.do_run(src, '*0*') # Now it will work properly

    # And now let's fix just that one line
    Settings.CORRECT_SIGNS = 2
    Settings.CORRECT_SIGNS_LINES = ["src.cpp:9"]
    self.do_run(src, '*0*')

    # Fixing the wrong line should not work
    Settings.CORRECT_SIGNS = 2
    Settings.CORRECT_SIGNS_LINES = ["src.cpp:3"]
    self.do_run(src, '*1*')

    # And reverse the checks with = 2
    Settings.CORRECT_SIGNS = 3
    Settings.CORRECT_SIGNS_LINES = ["src.cpp:3"]
    self.do_run(src, '*0*')
    Settings.CORRECT_SIGNS = 3
    Settings.CORRECT_SIGNS_LINES = ["src.cpp:9"]
    self.do_run(src, '*1*')

    Settings.CORRECT_SIGNS = 0

    # Overflows

    src = '''
      #include<stdio.h>
      int main() {
        int t = 77;
        for (int i = 0; i < 30; i++) {
          t = t + t + t + t + t + 1;
        }
        printf("*%d,%d*\\n", t, t & 127);
        return 0;
      }
    '''

    correct = '*186854335,63*'
    Settings.CORRECT_OVERFLOWS = 0
    try:
      self.do_run(src, correct)
      raise Exception('UNEXPECTED-PASS')
    except Exception, e:
      assert 'UNEXPECTED' not in str(e), str(e)
      assert 'Expected to find' in str(e), str(e)

    Settings.CORRECT_OVERFLOWS = 1
    self.do_run(src, correct) # Now it will work properly

    # And now let's fix just that one line
    Settings.CORRECT_OVERFLOWS = 2
    Settings.CORRECT_OVERFLOWS_LINES = ["src.cpp:6"]
    self.do_run(src, correct)

    # Fixing the wrong line should not work
    Settings.CORRECT_OVERFLOWS = 2
    Settings.CORRECT_OVERFLOWS_LINES = ["src.cpp:3"]
    try:
      self.do_run(src, correct)
      raise Exception('UNEXPECTED-PASS')
    except Exception, e:
      assert 'UNEXPECTED' not in str(e), str(e)
      assert 'Expected to find' in str(e), str(e)

    # And reverse the checks with = 2
    Settings.CORRECT_OVERFLOWS = 3
    Settings.CORRECT_OVERFLOWS_LINES = ["src.cpp:3"]
    self.do_run(src, correct)
    Settings.CORRECT_OVERFLOWS = 3
    Settings.CORRECT_OVERFLOWS_LINES = ["src.cpp:6"]
    try:
      self.do_run(src, correct)
      raise Exception('UNEXPECTED-PASS')
    except Exception, e:
      assert 'UNEXPECTED' not in str(e), str(e)
      assert 'Expected to find' in str(e), str(e)

    Settings.CORRECT_OVERFLOWS = 0

    # Roundings

    src = '''
      #include <stdio.h>
      #include <assert.h>

      int main()
      {
        TYPE x = -5;
        printf("*%d*", x/2);
        x = 5;
        printf("*%d*", x/2);

        float y = -5.33;
        x = y;
        printf("*%d*", x);
        y = 5.33;
        x = y;
        printf("*%d*", x);

        printf("\\n");
      }
    '''

    if Settings.USE_TYPED_ARRAYS != 2: # the errors here are very specific to non-i64 mode 1
      Settings.CORRECT_ROUNDINGS = 0
      self.do_run(src.replace('TYPE', 'long long'), '*-3**2**-6**5*') # JS floor operations, always to the negative. This is an undetected error here!
      self.do_run(src.replace('TYPE', 'int'), '*-2**2**-5**5*') # We get these right, since they are 32-bit and we can shortcut using the |0 trick
      self.do_run(src.replace('TYPE', 'unsigned int'), '*-2**2**-6**5*')

    Settings.CORRECT_ROUNDINGS = 1
    Settings.CORRECT_SIGNS = 1 # To be correct here, we need sign corrections as well
    self.do_run(src.replace('TYPE', 'long long'), '*-2**2**-5**5*') # Correct
    self.do_run(src.replace('TYPE', 'int'), '*-2**2**-5**5*') # Correct
    self.do_run(src.replace('TYPE', 'unsigned int'), '*2147483645**2**-5**5*') # Correct
    Settings.CORRECT_SIGNS = 0

    if Settings.USE_TYPED_ARRAYS != 2: # the errors here are very specific to non-i64 mode 1
      Settings.CORRECT_ROUNDINGS = 2
      Settings.CORRECT_ROUNDINGS_LINES = ["src.cpp:13"] # Fix just the last mistake
      self.do_run(src.replace('TYPE', 'long long'), '*-3**2**-5**5*')
      self.do_run(src.replace('TYPE', 'int'), '*-2**2**-5**5*') # Here we are lucky and also get the first one right
      self.do_run(src.replace('TYPE', 'unsigned int'), '*-2**2**-5**5*')

    # And reverse the check with = 2
    if Settings.USE_TYPED_ARRAYS != 2: # the errors here are very specific to non-i64 mode 1
      Settings.CORRECT_ROUNDINGS = 3
      Settings.CORRECT_ROUNDINGS_LINES = ["src.cpp:999"]
      self.do_run(src.replace('TYPE', 'long long'), '*-2**2**-5**5*')
      self.do_run(src.replace('TYPE', 'int'), '*-2**2**-5**5*')
      Settings.CORRECT_SIGNS = 1 # To be correct here, we need sign corrections as well
      self.do_run(src.replace('TYPE', 'unsigned int'), '*2147483645**2**-5**5*')
      Settings.CORRECT_SIGNS = 0

  def test_exit_status(self):
    if self.emcc_args is None: return self.skip('need emcc')
    src = r'''
      #include <stdio.h>
      #include <stdlib.h>
      static void cleanup() {
        printf("cleanup\n");
      }

      int main() {
        atexit(cleanup); // this atexit should still be called
        printf("hello, world!\n");
        exit(118); // Unusual exit status to make sure it's working!
      }
    '''
    open('post.js', 'w').write('''
      Module.addOnExit(function () {
        Module.print('I see exit status: ' + EXITSTATUS);
      });
      Module.callMain();
    ''')
    self.emcc_args += ['-s', 'INVOKE_RUN=0', '--post-js', 'post.js']
    self.do_run(src, 'hello, world!\ncleanup\nI see exit status: 118')

  def test_gc(self):
    if self.emcc_args == None: return self.skip('needs ta2')
    if Settings.ASM_JS: return self.skip('asm cannot support generic function table')

    Settings.GC_SUPPORT = 1

    test_path = path_from_root('tests', 'core', 'test_gc')
    src, output = (test_path + s for s in ('.in', '.out'))

    self.do_run_from_file(src, output)

# Generate tests for everything
def make_run(fullname, name=-1, compiler=-1, embetter=0, quantum_size=0,
    typed_arrays=0, emcc_args=None, env=None):

  if env is None: env = {}

  TT = type(fullname, (T,), dict(run_name = fullname, env = env))

  def tearDown(self):
    super(TT, self).tearDown()

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

    Building.COMPILER_TEST_OPTS = ['-g']
    os.chdir(self.get_dir()) # Ensure the directory exists and go there
    Building.COMPILER = compiler

    self.emcc_args = None if emcc_args is None else emcc_args[:]
    if self.emcc_args is not None:
      Settings.load(self.emcc_args)
      Building.LLVM_OPTS = 0
      if '-O2' in self.emcc_args:
        Building.COMPILER_TEST_OPTS = [] # remove -g in -O2 tests, for more coverage
      #Building.COMPILER_TEST_OPTS += self.emcc_args
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

    # TODO: Move much of these to a init() function in shared.py, and reuse that
    Settings.USE_TYPED_ARRAYS = typed_arrays
    Settings.INVOKE_RUN = 1
    Settings.RELOOP = 0 # we only do them in the "o2" pass
    Settings.MICRO_OPTS = embetter
    Settings.QUANTUM_SIZE = quantum_size
    Settings.ASSERTIONS = 1-embetter
    Settings.SAFE_HEAP = 1-embetter
    Settings.CHECK_OVERFLOWS = 1-embetter
    Settings.CORRECT_OVERFLOWS = 1-embetter
    Settings.CORRECT_SIGNS = 0
    Settings.CORRECT_ROUNDINGS = 0
    Settings.CORRECT_OVERFLOWS_LINES = CORRECT_SIGNS_LINES = CORRECT_ROUNDINGS_LINES = SAFE_HEAP_LINES = []
    Settings.CHECK_SIGNS = 0 #1-embetter
    Settings.RUNTIME_TYPE_INFO = 0
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    Settings.INCLUDE_FULL_LIBRARY = 0
    Settings.BUILD_AS_SHARED_LIB = 0
    Settings.RUNTIME_LINKED_LIBS = []
    Settings.EMULATE_UNALIGNED_ACCESSES = int(Settings.USE_TYPED_ARRAYS == 2 and Building.LLVM_OPTS == 2)
    Settings.DOUBLE_MODE = 1 if Settings.USE_TYPED_ARRAYS and Building.LLVM_OPTS == 0 else 0
    Settings.PRECISE_I64_MATH = 0
    Settings.NAMED_GLOBALS = 0 if not embetter else 1

  TT.setUp = setUp

  return TT

# Make one run with the defaults
default = make_run("default", compiler=CLANG, emcc_args=[] if os.environ.get('EMCC_FAST_COMPILER') != '1' else ['-s', 'ASM_JS=1'])

# Make one run with -O1, with safe heap
o1 = make_run("o1", compiler=CLANG, emcc_args=["-O1", "-s", "ASM_JS=0", "-s", "SAFE_HEAP=1"])

# Make one run with -O2, but without closure (we enable closure in specific tests, otherwise on everything it is too slow)
o2 = make_run("o2", compiler=CLANG, emcc_args=["-O2", "-s", "ASM_JS=0", "-s", "JS_CHUNK_SIZE=1024"])

# asm.js
asm1 = make_run("asm1", compiler=CLANG, emcc_args=["-O1"])
asm2 = make_run("asm2", compiler=CLANG, emcc_args=["-O2"])
asm2f = make_run("asm2f", compiler=CLANG, emcc_args=["-O2", "-s", "PRECISE_F32=1"])
asm2g = make_run("asm2g", compiler=CLANG, emcc_args=["-O2", "-g", "-s", "ASSERTIONS=1", "--memory-init-file", "1", "-s", "CHECK_HEAP_ALIGN=1"])
asm2x86 = make_run("asm2x86", compiler=CLANG, emcc_args=["-O2", "-g", "-s", "CHECK_HEAP_ALIGN=1"], env={"EMCC_LLVM_TARGET": "i386-pc-linux-gnu"})

# Make custom runs with various options
for compiler, quantum, embetter, typed_arrays in [
  (CLANG, 4, 0, 0),
  (CLANG, 4, 1, 1),
]:
  fullname = 's_0_%d%s%s' % (
    embetter, '' if quantum == 4 else '_q' + str(quantum), '' if typed_arrays in [0, 1] else '_t' + str(typed_arrays)
  )
  locals()[fullname] = make_run(fullname, fullname, compiler, embetter, quantum, typed_arrays)

del T # T is just a shape for the specific subclasses, we don't test it itself
