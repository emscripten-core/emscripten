'''
Simple test runner

See settings.py file for options&params. Edit as needed.
'''

from subprocess import Popen, PIPE, STDOUT
import os, unittest, tempfile, shutil, time

# Params

abspath = os.path.abspath(os.path.dirname(__file__))
def path_from_root(pathelems):
    return os.path.join(os.path.sep, *(abspath.split(os.sep)[:-1] + pathelems))

EMSCRIPTEN = path_from_root(['emscripten.py'])

exec(open(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'settings.py'), 'r').read())

def timeout_run(proc, timeout, note):
  start = time.time()
  while time.time() - start < timeout and proc.poll() is None:
    time.sleep(0.1)
  if proc.poll() is None:
    proc.kill()
    raise Exception("Timed out: " + note)
  return proc.communicate()[0]

class T(unittest.TestCase):
    def do_test(self, src, expected_output, args=[], output_nicerizer=None, output_processor=None, no_python=False, no_build=False, main_file=None, emscripten_settings=[]):
        global DEBUG
        dirname = TEMP_DIR + '/tmp' # tempfile.mkdtemp(dir=TEMP_DIR)
        if not os.path.exists(dirname):
          os.makedirs(dirname)
        filename = os.path.join(dirname, 'src.cpp')
        if not no_build:
          if main_file is None:
            f = open(filename, 'w')
            f.write(src)
            f.close()
          else:
            # copy whole directory, and use a specific main .cpp file
            for f in os.listdir(src):
              shutil.copy(os.path.join(src, f), dirname)
            shutil.move(os.path.join(dirname, main_file), filename)

          # Copy Emscripten C++ API
          shutil.copy(path_from_root(['src', 'include', 'emscripten.h']), dirname)

          # Begin

          if DEBUG: print "[[C++ => LLVM]]"
          try:
            os.remove(filename + '.o')
          except:
            pass
          os.chdir(dirname)
          cwd = os.getcwd()
          output = Popen([LLVM_GCC, '-DEMSCRIPTEN', '-emit-llvm', '-c', filename, '-o', filename + '.o'], stdout=PIPE, stderr=STDOUT).communicate()[0]
          os.chdir(cwd)
          if not os.path.exists(filename + '.o'):
            print "Failed to compile C/C++ source:\n\n", output
            raise Exception("Compilation error");
          if DEBUG: print output
          if DEBUG: print "[[C++ ==> LLVM]]"
          output = Popen([LLVM_DIS, filename + '.o', '-o=' + filename + '.o.llvm'], stdout=PIPE, stderr=STDOUT).communicate()[0]
          if DEBUG: print output
          # Run Emscripten
          if type(emscripten_settings) not in [list, tuple]:
            emscripten_settings = [emscripten_settings]
          Popen([EMSCRIPTEN, filename + '.o.llvm', PARSER_ENGINE] + emscripten_settings, stdout=open(filename + '.o.js', 'w'), stderr=STDOUT).communicate()
          output = open(filename + '.o.js').read()
          if output_processor is not None:
              output_processor(output)
          if output is not None and 'Traceback' in output: print output; assert (0) # 'generating JavaScript failed'
          if DEBUG: print "\nGenerated JavaScript:\n\n===\n\n%s\n\n===\n\n" % output
  #        if not DEBUG:
        js_output = timeout_run(Popen([JS_ENGINE] + JS_ENGINE_OPTS + [filename + '.o.js'] + args, stdout=PIPE, stderr=STDOUT), 20, 'Execution')
        if output_nicerizer is not None:
            js_output = output_nicerizer(js_output)
 #       else:
  #          print "[[JS output]]"
   #         ret = "Output shown on screen, test not actually run!"
    #        Popen([JS_ENGINE, filename + '.o.js'] + args, stderr=STDOUT).communicate()[0]
        self.assertContained(expected_output, js_output)
        self.assertNotContained('ERROR', js_output)
        return

        if not no_python:
          #DEBUG = True
          SPIDERMONKEY = True
          if SPIDERMONKEY:
            if DEBUG: print "[[RJS ==> SpiderMonkey parsed tree]]"
            args = [SPIDERMONKEY_SHELL, '-e', 'parse(snarf(\"%s\"))' % (filename + '.o.js')]
            output = Popen(args, stdout=PIPE, stderr=STDOUT).communicate()[0]
            f = open(filename + 'o.js.sm', 'w')
            f.write(output)
            f.close()
          else:
            if DEBUG: print "[[RJS ==> RPython]]"
            output = Popen(['python', RJS_RPYTHON, filename + '.o.js', filename + '.o.js.py'], stdout=PIPE, stderr=STDOUT).communicate()[0]
            if DEBUG: print output

            py_output = Popen(['python', filename + '.o.js.py'] + args, stdout=PIPE, stderr=STDOUT).communicate()[0]
            if output_nicerizer is not None:
                py_output = output_nicerizer(py_output)
            self.assertContained(expected_output, py_output)
            if js_output != py_output:
                print "WARNING: js and py outputs not identical (but each is similar enough to the expected_output)"

            PYPY = True
#            PYPY = False
            if PYPY:
              pypy_source = filename.replace('.', '_') + '_o_js_py.py'
              if DEBUG: print "[[RPython ==> PyPy]]"
              output = Popen(['python', RJS_PYPY, filename + '.o.js.py', pypy_source], stdout=PIPE, stderr=STDOUT).communicate()[0]
              print output

#              # Python on pypy-ready source
 #             pypy_output = Popen(['python', pypy_source] + args, stdout=PIPE, stderr=STDOUT).communicate()[0]
  #            if output_nicerizer is not None:
   #               pypy_output = output_nicerizer(pypy_output)
    #          self.assertContained(expected_output, pypy_output)
     #         if js_output != pypy_output:
      #            print "WARNING: js and PYpy outputs not identical (but each is similar enough to the expected_output)"

            #   PyPy compilation of source to binary

#        shutil.rmtree(dirname)

    def assertContained(self, value, string):
        if value not in string:
            print "Expected to find '%s' in '%s'" % (value, string)
            self.assertTrue(value in string)

    def assertNotContained(self, value, string):
         if value in string:
             print "Expected to NOT find '%s' in '%s'" % (value, string)
             self.assertTrue(value not in string)

    def test_hello_world(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            printf("hello, world!\\n");
            return 0;
          }
        '''
        self.do_test(src, 'hello, world!')

    def test_intvars(self):
        src = '''
          #include <stdio.h>
          int global = 20;
          int *far;
          int main()
          {
            int x = 5;
            int y = x+17;
            int z = (y-1)/2; // Should stay an integer after division!
            y += 1;
            int w = x*3+4;
            int k = w < 15 ? 99 : 101;
            far = &k;
            *far += global;
            int i = k > 100; // Should be an int, not a bool!
            int j = i << 6;
            j >>= 1;
            j = j ^ 5;
            int h = 1;
            h |= 0;
            int p = h;
            p &= 0;
            printf("*%d,%d,%d,%d,%d,%d,%d,%d,%d*\\n", x, y, z, w, k, i, j, h, p);
            return 0;
          }
        '''
        self.do_test(src, '*5,23,10,19,121,1,37,1,0*')

    def test_floatvars(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            float x = 1.234, y = 3.5;
            y *= 3;
            int z = x < y;
            printf("*%d,%d,%f,%d,%f\\n", z, int(y), y, (int)x, x);
            return 0;
          }
        '''
        self.do_test(src, '*1,10,10.5,1,1.2339')

    def test_if(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            int x = 5;
            if (x > 3) {
              printf("*yes*\\n");
            }
            return 0;
          }
        '''
        self.do_test(src, '*yes*')

    def test_loop(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            int x = 5;
            for (int i = 0; i < 6; i++)
              x += x*i;
            printf("*%d*\\n", x);
            return 0;
          }
        '''
        self.do_test(src, '*3600*')

    def test_strings(self):
        src = '''
          #include <stdio.h>
          #include <stdlib.h>
          #include <string.h>

          int main(int argc, char **argv)
          {
            printf("*%d", argc);
            puts(argv[1]);
            puts(argv[2]);
            printf("%d", atoi(argv[3])+2);
            const char *foolingthecompiler = "\\rabcd";
            printf("%d", strlen(foolingthecompiler)); // Tests parsing /0D in llvm - should not be a 0 (end string) then a D!
            printf("%s*", NULL); // Should print '(null)', not the string at address 0, which is a real address for us!
            return 0;
          }
        '''
        self.do_test(src, '*4*wowie*too*76*5*(null)*', ['wowie', 'too', '74'], lambda x: x.replace('\n', '*'))

    def test_funcs(self):
        src = '''
          #include <stdio.h>
          int funcy(int x)
          {
            return x*9;
          }
          int main()
          {
            printf("*%d,%d*\\n", funcy(8), funcy(10));
            return 0;
          }
        '''
        self.do_test(src, '*72,90*')

    def test_structs(self):
        src = '''
          #include <stdio.h>
          struct S
          {
            int x, y;
          };
          int main()
          {
            S a, b;
            a.x = 5; a.y = 6;
            b.x = 101; b.y = 7009;
            S *c, *d;
            c = &a;
            c->x *= 2;
            c = &b;
            c->y -= 1;
            d = c;
            d->y += 10;
            printf("*%d,%d,%d,%d,%d,%d,%d,%d*\\n", a.x, a.y, b.x, b.y, c->x, c->y, d->x, d->y);
            return 0;
          }
        '''
        self.do_test(src, '*10,6,101,7018,101,7018,101,7018*')

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
        self.do_test(self.gen_struct_src.replace('{{gen_struct}}', '(S*)malloc(ES_SIZEOF(S))').replace('{{del_struct}}', 'free'), '*51,62*')

    def test_newstruct(self):
        self.do_test(self.gen_struct_src.replace('{{gen_struct}}', 'new S').replace('{{del_struct}}', 'delete'), '*51,62*')

    def test_addr_of_stacked(self):
        src = '''
          #include <stdio.h>
          void alter(int *y)
          {
            *y += 5;
          }
          int main()
          {
            int x = 2;
            alter(&x);
            printf("*%d*\\n", x);
            return 0;
          }
        '''
        self.do_test(src, '*7*')

    def test_linked_list(self):
        src = '''
          #include <stdio.h>
          struct worker_args {
            int value;
            struct worker_args *next;
          };
          int main()
          {
            worker_args a;
            worker_args b;
            a.value = 60;
            a.next = &b;
            b.value = 900;
            b.next = NULL;
            worker_args* c = &a;
            int total = 0;
            while (c) {
              total += c->value;
              c = c->next;
            }

            // Chunk of em
            worker_args chunk[10];
            for (int i = 0; i < 9; i++) {
              chunk[i].value = i*10;
              chunk[i].next = &chunk[i+1];
            }
            chunk[9].value = 90;
            chunk[9].next = &chunk[0];

            c = chunk;
            do {
              total += c->value;
              c = c->next;
            } while (c != chunk);
 
            printf("*%d,%d*\\n", total, b.next);
            // NULL *is* 0, in C/C++. No JS null! (null == 0 is false, etc.)

            return 0;
          }
        '''
        self.do_test(src, '*1410,0*')

    def test_assert(self):
        src = '''
          #include <stdio.h>
          #include <assert.h>
          int main() {
            assert(1 == true); // pass
            assert(1 == false); // fail
            return 1;
          }
        '''
        self.do_test(src, 'Assertion failed: 1 == false')

    def test_class(self):
        src = '''
          #include <stdio.h>
          struct Random {
             enum { IM = 139968, IA = 3877, IC = 29573 };
             Random() : last(42) {}
             float get( float max = 1.0f ) {
                last = ( last * IA + IC ) % IM;
                return max * last / IM;
             }
          protected:
             unsigned int last;
          } rng1;
          int main()
          {
            Random rng2;
            int count = 0;
            for (int i = 0; i < 100; i++) {
              float x1 = rng1.get();
              float x2 = rng2.get();
              printf("%f, %f\\n", x1, x2);
              if (x1 != x2) count += 1;
            }
            printf("*%d*\\n", count);
            return 0;
          }
        '''
        self.do_test(src, '*0*')

    def test_inherit(self):
        src = '''
          #include <stdio.h>
          struct Parent {
            int x1, x2;
          };
          struct Child : Parent {
            int y;
          };
          int main()
          {
            Parent a;
            a.x1 = 50;
            a.x2 = 87;
            Child b;
            b.x1 = 78;
            b.x2 = 550;
            b.y = 101;
            Child* c = (Child*)&a;
            c->x1 ++;
            c = &b;
            c->y --;
            printf("*%d,%d,%d,%d,%d,%d,%d*\\n", a.x1, a.x2, b.x1, b.x2, b.y, c->x1, c->x2);
            return 0;
          }
        '''
        self.do_test(src, '*51,87,78,550,100,78,550*')

    def test_polymorph(self):
        src = '''
          #include <stdio.h>
          struct Parent {
            virtual int getit() { return 11; };
          };
          struct Child : Parent {
            int getit() { return 74; }
          };
          int main()
          {
            Parent *x = new Parent();
            Parent *y = new Child();
            printf("*%d,%d*\\n", x->getit(), y->getit());
            return 0;
          }
        '''
        self.do_test(src, '*11,74*')

    def test_emptyclass(self):
        src = '''
        #include <stdio.h>

        struct Randomized {
          Randomized(int x) {
            printf("*zzcheezzz*\\n");
          }
        };

        int main( int argc, const char *argv[] ) {
          new Randomized(55);

          return 0;
        }
        '''
        self.do_test(src, '*zzcheezzz*')

    def test_constglobalstructs(self):
        src = '''
          #include <stdio.h>
          struct IUB {
             int c;
             double p;
             unsigned int pi;
          };

          IUB iub[] = {
             { 'a', 0.27, 5 },
             { 'c', 0.15, 4 },
             { 'g', 0.12, 3 },
             { 't', 0.27, 2 },
          };

          int main( int argc, const char *argv[] ) {
             printf("*%d,%d,%d*\\n", iub[0].c, int(iub[1].p*100), iub[2].pi);
             return 0;
          }
          '''
        self.do_test(src, '*97,15,3*')

    def test_conststructs(self):
        src = '''
          #include <stdio.h>
          struct IUB {
             int c;
             double p;
             unsigned int pi;
          };

          int main( int argc, const char *argv[] ) {
             IUB iub[] = {
                { 'a', 0.3029549426680, 5 },
                { 'c', 0.15, 4 },
                { 'g', 0.12, 3 },
                { 't', 0.27, 2 },
             };
             printf("*%d,%d,%d,%d*\\n", iub[0].c, int(iub[1].p*100), iub[2].pi, int(iub[0].p*10000));
//             printf("*%d*\\n", int(iub[1].p*100));
             return 0;
          }
          '''
        self.do_test(src, '*97,15,3,3029*')

    def test_ptrtoint(self):
        src = '''
          #include <stdio.h>

          int main( int argc, const char *argv[] ) {
            char *a = new char[10];
            char *a0 = a+0;
            char *a5 = a+5;
            int *b = new int[10];
            int *b0 = b+0;
            int *b5 = b+5;
            int c = (int)b5-(int)b0; // Emscripten should warn!
            int d = (int)b5-(int)b0; // Emscripten should warn!
            printf("*%d*\\n", (int)a5-(int)a0);
            return 0;
          }
          '''
        runner = self
        def check_warnings(output):
            runner.assertEquals(filter(lambda line: 'Warning' in line, output.split('\n')).__len__(), 4)
        self.do_test(src, '*5*', output_processor=check_warnings)

    def test_sizeof(self):
        src = '''
          #include <stdio.h>
          #include <string.h>
          #include "emscripten.h"

          struct A { int x, y; };

          int main( int argc, const char *argv[] ) {
            int *a = new int[10];
            int *b = new int[1];
            int *c = new int[10];
            for (int i = 0; i < 10; i++)
              a[i] = 2;
            *b = 5;
            for (int i = 0; i < 10; i++)
              c[i] = 8;
            printf("*%d,%d,%d,%d,%d*\\n", a[0], a[9], *b, c[0], c[9]);
            // Should overwrite a, but not touch b!
            memcpy(a, c, 10*ES_SIZEOF(int));
            printf("*%d,%d,%d,%d,%d*\\n", a[0], a[9], *b, c[0], c[9]);

            // Part 2
            A as[3] = { { 5, 12 }, { 6, 990 }, { 7, 2 } };
            memcpy(&as[0], &as[2], ES_SIZEOF(A));

            printf("*%d,%d,%d,%d,%d,%d*\\n", as[0].x, as[0].y, as[1].x, as[1].y, as[2].x, as[2].y);
            return 0;
          }
          '''
        self.do_test(src, '*2,2,5,8,8***8,8,5,8,8***7,2,6,990,7,2*', [], lambda x: x.replace('\n', '*'))

    def test_llvmswitch(self):
        src = '''
          #include <stdio.h>
          #include <string.h>

          int switcher(int p)
          {
            switch(p) {
              case 'a':
              case 'b':
              case 'c':
                  return p-1;
              case 'd':
                  return p+1;
            }
            return p;
          }

          int main( int argc, const char *argv[] ) {
            printf("*%d,%d,%d,%d,%d*\\n", switcher('a'), switcher('b'), switcher('c'), switcher('d'), switcher('e'));
            return 0;
          }
          '''
        self.do_test(src, '*96,97,98,101,101*')

    def test_varargs(self):
        src = '''
          #include <stdio.h>
          #include "stdarg.h"

          void vary(const char *s, ...)
          {
            va_list v;
            va_start(v, s);
            char d[20];
            vsnprintf(d, 20, s, v);
            puts(d);
            va_end(v);
          }

          void vary2(char color, const char *s, ...)
          {
            va_list v;
            va_start(v, s);
            char d[21];
            d[0] = color;
            vsnprintf(d+1, 20, s, v);
            puts(d);
            va_end(v);
          }

          int main() {
            vary("*cheez: %d+%d*", 0, 24); // Also tests that '0' is not special as an array ender
            vary2('Q', "%d*", 85);
            return 0;
          }
          '''
        self.do_test(src, '*cheez: 0+24*\nQ85*')

    def test_atexit(self):
        src = '''
          #include <stdio.h>
          #include <stdlib.h>

          void clean()
          {
            printf("*cleaned*\\n");
          }

          int main() {
            atexit(clean);
            return 0;
          }
          '''
        self.do_test(src, '*cleaned*')

    def test_statics(self):
        src = '''
          #include <stdio.h>
          #include <string.h>

          #define CONSTRLEN 32

          void conoutfv(const char *fmt)
          {
              static char buf[CONSTRLEN];
              strcpy(buf, fmt);
              puts(buf);
          }

          int main() {
            conoutfv("*staticccz*");
            return 0;
          }
          '''
        self.do_test(src, '*staticccz*')

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
                ES_SIZEOF(base),
                int(&(b->x)), int(&(b->y)), int(&(b->a)), int(&(b->b)), int(&(b->c)), 
                ES_SIZEOF(hashtableentry),
                int(&(e->key)), int(&(e->data)), int(&(e->data.x)), int(&(e->data.y)), int(&(e->data.a)), int(&(e->data.b)), int(&(e->data.c)), 
                ES_SIZEOF(hashset::chain),
                int(&(c->elem)), int(&(c->next)), int(&(c->elem.key)), int(&(c->elem.data)), int(&(c->elem.data.x)), int(&(c->elem.data.y)), int(&(c->elem.data.a)), int(&(c->elem.data.b)), int(&(c->elem.data.c))
              );
            }
          };

          int main() {
            hashtable t;
            return 0;
          }
          '''
        self.do_test(src, '*4,0,1,2,2,3|5,0,1,1,2,3,3,4|6,0,5,0,1,1,2,3,3,4*')

    def test_fannkuch(self):
        results = [ (1,0), (2,1), (3,2), (4,4), (5,7), (6,10), (7, 16), (8,22) ]
        for i, j in results:
          src = open(path_from_root(['tests', 'fannkuch.cpp']), 'r').read()
          self.do_test(src, 'Pfannkuchen(%d) = %d.' % (i,j), [str(i)], no_build=i>1)

    def test_fasta(self):
        results = [ (1,'''GG*ctt*tgagc*'''), (20,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg*tacgtgtagcctagtgtttgtgttgcgttatagtctatttgtggacacagtatggtcaaa*tgacgtcttttgatctgacggcgttaacaaagatactctg*'''),
(50,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA*TCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACAT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg*tactDtDagcctatttSVHtHttKtgtHMaSattgWaHKHttttagacatWatgtRgaaa*NtactMcSMtYtcMgRtacttctWBacgaa*agatactctgggcaacacacatacttctctcatgttgtttcttcggacctttcataacct*ttcctggcacatggttagctgcacatcacaggattgtaagggtctagtggttcagtgagc*ggaatatcattcgtcggtggtgttaatctatctcggtgtagcttataaatgcatccgtaa*gaatattatgtttatttgtcggtacgttcatggtagtggtgtcgccgatttagacgtaaa*ggcatgtatg*''') ]
        for i, j in results:
          src = open(path_from_root(['tests', 'fasta.cpp']), 'r').read()
          self.do_test(src, j, [str(i)], lambda x: x.replace('\n', '*'), no_python=True, no_build=i>1)

    def test_sauer(self):
      # XXX Warning: Running this in SpiderMonkey can lead to an extreme amount of memory being
      #              used, see Mozilla bug 593659.
      assert PARSER_ENGINE != SPIDERMONKEY_ENGINE
      # XXX RELOOP = 1 either is very very slow, or nonfinishing
      self.do_test(path_from_root(['tests', 'sauer']), '*Temp is 33\n9*', main_file='command.cpp', emscripten_settings='{"RELOOP": 0}')

if __name__ == '__main__':
    if DEBUG: print "LLVM_GCC:", LLVM_GCC
    if DEBUG: print "LLC:", LLC
    if DEBUG: print "PARSER:", PARSER
    if DEBUG: print "JS_ENGINE:", JS_ENGINE
    for cmd in [LLVM_GCC, JS_ENGINE]:
        if DEBUG: print "Checking for existence of", cmd
        assert(os.path.exists(cmd))
    unittest.main()

