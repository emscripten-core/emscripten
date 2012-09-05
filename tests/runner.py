#!/usr/bin/env python
# This Python file uses the following encoding: utf-8

'''
Simple test runner

See settings.py file for options&params. Edit as needed.

These tests can be run in parallel using nose, for example

  nosetests --processes=4 -v -s tests/runner.py

will use 4 processes. To install nose do something like
|pip install nose| or |sudo apt-get install python-nose|.
'''

from subprocess import Popen, PIPE, STDOUT
import os, unittest, tempfile, shutil, time, inspect, sys, math, glob, tempfile, re, difflib, webbrowser, hashlib, threading, platform, BaseHTTPServer, multiprocessing, functools, stat


if len(sys.argv) == 1:
  print '''
==============================================================================
Running the main part of the test suite. Don't forget to run the other parts!

  sanity - tests for first run, etc., modifies ~/.emscripten
  benchmark - run before and after each set of changes before pushing to
              master, verify no regressions
  browser - runs pages in a web browser

To run one of those parts, do something like

  python tests/runner.py sanity

To run a specific set of tests, you can do things like

  python tests/runner.py o1

(that runs the o1 (-O1) tests). You can run individual tests with

  python tests/runner.py test_hello_world

Combinations work too, for example

  python tests/runner.py browser.test_sdl_image
==============================================================================

'''
  time.sleep(2)

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
import tools.shared
from tools.shared import *

# Sanity check for config

try:
  assert COMPILER_OPTS != None
except:
  raise Exception('Cannot find "COMPILER_OPTS" definition. Is %s set up properly? You may need to copy the template from settings.py into it.' % EM_CONFIG)

# Core test runner class, shared between normal tests and benchmarks

class RunnerCore(unittest.TestCase):
  save_dir = os.environ.get('EM_SAVE_DIR')
  save_JS = 0
  stderr_redirect = STDOUT # This avoids cluttering the test runner output, which is stderr too, with compiler warnings etc.
                           # Change this to None to get stderr reporting, for debugging purposes

  def setUp(self):
    global Settings
    Settings.reset()
    Settings = tools.shared.Settings
    self.banned_js_engines = []
    if not self.save_dir:
      dirname = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=TEMP_DIR)
    else:
      dirname = CANONICAL_TEMP_DIR
    if not os.path.exists(dirname):
      os.makedirs(dirname)
    self.working_dir = dirname
    os.chdir(dirname)
    
  def tearDown(self):
    if self.save_JS:
      for name in os.listdir(self.get_dir()):
        if name.endswith(('.o.js', '.cc.js')):
          suff = '.'.join(name.split('.')[-2:])
          shutil.copy(os.path.join(self.get_dir(), name),
                      os.path.join(TEMP_DIR, self.id().replace('__main__.', '').replace('.test_', '.')+'.'+suff))
    if not self.save_dir:
      # rmtree() fails on Windows if the current working directory is inside the tree.
      os.chdir(os.path.join(self.get_dir(), '..'))
      shutil.rmtree(self.get_dir())

  def skip(self, why):
    print >> sys.stderr, '<skipping: %s> ' % why,

  def get_dir(self):
    return self.working_dir

  def get_shared_library_name(self, linux_name):
    if platform.system() == 'Linux':
      return linux_name
    elif platform.system() == 'Darwin':
      return linux_name.replace('.so', '') + '.dylib'
    else:
      print >> sys.stderr, 'get_shared_library_name needs to be implemented on %s' % platform.system()
      return linux_name

  def get_stdout_path(self):
    return os.path.join(self.get_dir(), 'stdout')

  def prep_ll_run(self, filename, ll_file, force_recompile=False, build_ll_hook=None):
    if ll_file.endswith(('.bc', '.o')):
      if ll_file != filename + '.o':
        shutil.copy(ll_file, filename + '.o')
      Building.llvm_dis(filename)
    else:
      shutil.copy(ll_file, filename + '.o.ll')

    #force_recompile = force_recompile or os.stat(filename + '.o.ll').st_size > 50000 # if the file is big, recompile just to get ll_opts # Recompiling just for dfe in ll_opts is too costly

    if Building.LLVM_OPTS or force_recompile or build_ll_hook:
      Building.ll_opts(filename)
      if build_ll_hook:
        need_post = build_ll_hook(filename)
      Building.llvm_as(filename)
      shutil.move(filename + '.o.ll', filename + '.o.ll.pre') # for comparisons later
      if Building.LLVM_OPTS:
        Building.llvm_opts(filename)
      Building.llvm_dis(filename)
      if build_ll_hook and need_post:
        build_ll_hook(filename)
        Building.llvm_as(filename)
        shutil.move(filename + '.o.ll', filename + '.o.ll.post') # for comparisons later
        Building.llvm_dis(filename)

  # Generate JS from ll, and optionally modify the generated JS with a post_build function. Note
  # that post_build is called on unoptimized JS, so we send it to emcc (otherwise, if run after
  # emcc, it would not apply on the optimized/minified JS)
  def ll_to_js(self, filename, extra_emscripten_args, post_build):
    if type(post_build) in (list, tuple):
      post1, post2 = post_build
    else:
      post1 = post_build
      post2 = None

    def run_post(post):
      if not post: return
      exec post in locals()
      shutil.copyfile(filename + '.o.js', filename + '.o.js.prepost.js')
      process(filename + '.o.js')

    if self.emcc_args is None:
      Building.emscripten(filename, append_ext=True, extra_args=extra_emscripten_args)
      run_post(post1)
      run_post(post2)
    else:
      transform_args = []
      if post1:
        transform_filename = os.path.join(self.get_dir(), 'transform.py')
        transform = open(transform_filename, 'w')
        transform.write('''
import sys
sys.path += [%r]
''' % path_from_root(''))
        transform.write(post1)
        transform.write('''
process(sys.argv[1])
''')
        transform.close()
        transform_args = ['--js-transform', "python %s" % transform_filename]
      Building.emcc(filename + '.o.ll', Settings.serialize() + self.emcc_args + transform_args, filename + '.o.js')
      run_post(post2)

  # Build JavaScript code from source code
  def build(self, src, dirname, filename, output_processor=None, main_file=None, additional_files=[], libraries=[], includes=[], build_ll_hook=None, extra_emscripten_args=[], post_build=None):

    Building.pick_llvm_opts(3) # pick llvm opts here, so we include changes to Settings in the test case code

    # Copy over necessary files for compiling the source
    if main_file is None:
      f = open(filename, 'w')
      f.write(src)
      f.close()
      final_additional_files = []
      for f in additional_files:
        final_additional_files.append(os.path.join(dirname, os.path.basename(f)))
        shutil.copyfile(f, final_additional_files[-1])
      additional_files = final_additional_files
    else:
      # copy whole directory, and use a specific main .cpp file
      # (rmtree() fails on Windows if the current working directory is inside the tree.)
      if os.getcwd().startswith(os.path.abspath(dirname)):
          os.chdir(os.path.join(dirname, '..'))
      shutil.rmtree(dirname)
      shutil.copytree(src, dirname)
      shutil.move(os.path.join(dirname, main_file), filename)
      # the additional files were copied; alter additional_files to point to their full paths now
      additional_files = map(lambda f: os.path.join(dirname, f), additional_files)
      os.chdir(self.get_dir())

    # C++ => LLVM binary

    for f in [filename] + additional_files:
      try:
        # Make sure we notice if compilation steps failed
        os.remove(f + '.o')
      except:
        pass
      args = [Building.COMPILER, '-emit-llvm'] + COMPILER_OPTS + Building.COMPILER_TEST_OPTS + \
             ['-I', dirname, '-I', os.path.join(dirname, 'include')] + \
             map(lambda include: '-I' + include, includes) + \
             ['-c', f, '-o', f + '.o']
      output = Popen(args, stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
      assert os.path.exists(f + '.o'), 'Source compilation error: ' + output

    # Link all files
    if len(additional_files) + len(libraries) > 0:
      shutil.move(filename + '.o', filename + '.o.alone')
      Building.link([filename + '.o.alone'] + map(lambda f: f + '.o', additional_files) + libraries,
               filename + '.o')
      if not os.path.exists(filename + '.o'):
        print "Failed to link LLVM binaries:\n\n", output
        raise Exception("Linkage error");

    # Finalize
    self.prep_ll_run(filename, filename + '.o', build_ll_hook=build_ll_hook)

    # BC => JS
    self.ll_to_js(filename, extra_emscripten_args, post_build)

    if output_processor is not None:
      output_processor(open(filename + '.o.js').read())

  def run_generated_code(self, engine, filename, args=[], check_timeout=True):
    stdout = os.path.join(self.get_dir(), 'stdout') # use files, as PIPE can get too full and hang us
    stderr = os.path.join(self.get_dir(), 'stderr')
    try:
      cwd = os.getcwd()
    except:
      cwd = None
    os.chdir(self.get_dir())
    run_js(filename, engine, args, check_timeout, stdout=open(stdout, 'w'), stderr=open(stderr, 'w'))
    if cwd is not None:
      os.chdir(cwd)
    ret = open(stdout, 'r').read() + open(stderr, 'r').read()
    assert 'strict warning:' not in ret, 'We should pass all strict mode checks: ' + ret
    return ret

  def build_native(self, filename):
    Popen([CLANG, '-O2', filename, '-o', filename+'.native'], stdout=PIPE).communicate()[0]

  def run_native(self, filename, args):
    Popen([filename+'.native'] + args, stdout=PIPE).communicate()[0]

  def assertIdentical(self, values, y):
    if type(values) not in [list, tuple]: values = [values]
    for x in values:
      if x == y: return # success
    raise Exception("Expected to have '%s' == '%s', diff:\n\n%s" % (
      limit_size(values[0]), limit_size(y),
      limit_size(''.join([a.rstrip()+'\n' for a in difflib.unified_diff(x.split('\n'), y.split('\n'), fromfile='expected', tofile='actual')]))
    ))

  def assertContained(self, values, string, additional_info=''):
    if type(values) not in [list, tuple]: values = [values]
    for value in values:
      if type(string) is not str: string = string()
      if value in string: return # success
    raise Exception("Expected to find '%s' in '%s', diff:\n\n%s\n%s" % (
      limit_size(values[0]), limit_size(string),
      limit_size(''.join([a.rstrip()+'\n' for a in difflib.unified_diff(values[0].split('\n'), string.split('\n'), fromfile='expected', tofile='actual')])),
      additional_info
    ))

  def assertNotContained(self, value, string):
    if type(value) is not str: value = value() # lazy loading
    if type(string) is not str: string = string()
    if value in string:
      raise Exception("Expected to NOT find '%s' in '%s', diff:\n\n%s" % (
        limit_size(value), limit_size(string),
        limit_size(''.join([a.rstrip()+'\n' for a in difflib.unified_diff(value.split('\n'), string.split('\n'), fromfile='expected', tofile='actual')]))
      ))

  library_cache = {}

  def get_build_dir(self):
    ret = os.path.join(self.get_dir(), 'building')
    if not os.path.exists(ret):
      os.makedirs(ret)
    return ret

  def get_library(self, name, generated_libs, configure=['sh', './configure'], configure_args=[], make=['make'], make_args=['-j', '2'], cache=True, env_init={}):
    build_dir = self.get_build_dir()
    output_dir = self.get_dir()

    cache_name = name + '|' + Building.COMPILER
    if self.library_cache is not None:
      if cache and self.library_cache.get(cache_name):
        print >> sys.stderr,  '<load build from cache> ',
        generated_libs = []
        for basename, contents in self.library_cache[cache_name]:
          bc_file = os.path.join(build_dir, basename)
          f = open(bc_file, 'wb')
          f.write(contents)
          f.close()
          generated_libs.append(bc_file)
        return generated_libs

    print >> sys.stderr, '<building and saving into cache> ',

    return Building.build_library(name, build_dir, output_dir, generated_libs, configure, configure_args, make, make_args, self.library_cache, cache_name,
                                  copy_project=True, env_init=env_init)

  def clear(self):
    for name in os.listdir(self.get_dir()):
      try_delete(name)
    emcc_debug = os.environ.get('EMCC_DEBUG')
    if emcc_debug:
      for name in os.listdir(EMSCRIPTEN_TEMP_DIR):
        try_delete(os.path.join(EMSCRIPTEN_TEMP_DIR, name))


###################################################################################################

sys.argv = map(lambda arg: arg if not arg.startswith('test_') else 'default.' + arg, sys.argv)

if 'benchmark' not in str(sys.argv) and 'sanity' not in str(sys.argv) and 'browser' not in str(sys.argv):
  # Tests

  print "Running Emscripten tests..."

  class T(RunnerCore): # Short name, to make it more fun to use manually on the commandline
    ## Does a complete test - builds, runs, checks output, etc.
    def do_run(self, src, expected_output, args=[], output_nicerizer=None, output_processor=None, no_build=False, main_file=None, additional_files=[], js_engines=None, post_build=None, basename='src.cpp', libraries=[], includes=[], force_c=False, build_ll_hook=None, extra_emscripten_args=[]):
        if force_c or (main_file is not None and main_file[-2:]) == '.c':
          basename = 'src.c'
          Building.COMPILER = to_cc(Building.COMPILER)

        dirname = self.get_dir()
        filename = os.path.join(dirname, basename)
        if not no_build:
          self.build(src, dirname, filename, main_file=main_file, additional_files=additional_files, libraries=libraries, includes=includes,
                     build_ll_hook=build_ll_hook, extra_emscripten_args=extra_emscripten_args, post_build=post_build)

        # Run in both JavaScript engines, if optimizing - significant differences there (typed arrays)
        if js_engines is None:
          js_engines = JS_ENGINES
        if Settings.USE_TYPED_ARRAYS:
          js_engines = filter(lambda engine: engine != V8_ENGINE, js_engines) # V8 issue 1822
        js_engines = filter(lambda engine: engine not in self.banned_js_engines, js_engines)
        if len(js_engines) == 0: return self.skip('No JS engine present to run this test with. Check %s and settings.py and the paths therein.' % EM_CONFIG)
        for engine in js_engines:
          js_output = self.run_generated_code(engine, filename + '.o.js', args)
          if output_nicerizer is not None:
              js_output = output_nicerizer(js_output)
          self.assertContained(expected_output, js_output)
          self.assertNotContained('ERROR', js_output)

        #shutil.rmtree(dirname) # TODO: leave no trace in memory. But for now nice for debugging

    # No building - just process an existing .ll file (or .bc, which we turn into .ll)
    def do_ll_run(self, ll_file, expected_output=None, args=[], js_engines=None, output_nicerizer=None, post_build=None, force_recompile=False, build_ll_hook=None, extra_emscripten_args=[]):
      filename = os.path.join(self.get_dir(), 'src.cpp')

      self.prep_ll_run(filename, ll_file, force_recompile, build_ll_hook)

      self.ll_to_js(filename, extra_emscripten_args, post_build)

      self.do_run(None,
                   expected_output,
                   args,
                   no_build=True,
                   js_engines=js_engines,
                   output_nicerizer=output_nicerizer,
                   post_build=None) # post_build was already done in ll_to_js, this do_run call is just to test the output

    def test_hello_world(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            printf("hello, world!\\n");
            return 0;
          }
        '''
        self.do_run(src, 'hello, world!')

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

            long hash = -1;
            size_t perturb;
            int ii = 0;
            for (perturb = hash; ; perturb >>= 5) {
              printf("%d:%d", ii, perturb);
              ii++;
              if (ii == 9) break;
              printf(",");
            }
            printf("*\\n");
            printf("*%.1d,%.2d*\\n", 56, 9);

            // Fixed-point math on 64-bit ints. Tricky to support since we have no 64-bit shifts in JS
            {
              struct Fixed {
                static int Mult(int a, int b) {
                  return ((long long)a * (long long)b) >> 16;
                }
              };
              printf("fixed:%d\\n", Fixed::Mult(150000, 140000));
            }

            printf("*%ld*%p\\n", (long)21, &hash); // The %p should not enter an infinite loop!
            return 0;
          }
        '''
        self.do_run(src, '*5,23,10,19,121,1,37,1,0*\n0:-1,1:134217727,2:4194303,3:131071,4:4095,5:127,6:3,7:0,8:0*\n*56,09*\nfixed:320434\n*21*')

    def test_sintvars(self):
        Settings.CORRECT_SIGNS = 1 # Relevant to this test
        src = '''
          #include <stdio.h>
          struct S {
            char *match_start;
            char *strstart;
          };
          int main()
          {
            struct S _s;
            struct S *s = &_s;
            unsigned short int sh;

            s->match_start = (char*)32522;
            s->strstart = (char*)(32780);
            printf("*%d,%d,%d*\\n", (int)s->strstart, (int)s->match_start, (int)(s->strstart - s->match_start));
            sh = s->strstart - s->match_start;
            printf("*%d,%d*\\n", sh, sh>>7);

            s->match_start = (char*)32999;
            s->strstart = (char*)(32780);
            printf("*%d,%d,%d*\\n", (int)s->strstart, (int)s->match_start, (int)(s->strstart - s->match_start));
            sh = s->strstart - s->match_start;
            printf("*%d,%d*\\n", sh, sh>>7);
          }
        '''
        output = '*32780,32522,258*\n*258,2*\n*32780,32999,-219*\n*65317,510*'
        Settings.CORRECT_OVERFLOWS = 0 # We should not need overflow correction to get this right
        self.do_run(src, output, force_c=True)

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
              printf("*prod:%llu*\n*%d,%d,%d*\n", c, (int)&a, (int)&b, (int)&c); // printing addresses prevents optimizations
            }

            // Basic (rounded, for now) math. Just check compilation.
            int64_t a = 0x1234def123450789ULL;
            a--; if (truthy()) a--; // confuse optimizer
            int64_t b = 0x1234000000450789ULL;
            b++; if (truthy()) b--; // confuse optimizer
            printf("*%Ld,%Ld,%Ld,%Ld*\n",   (a+b)/5000, (a-b)/5000, (a*3)/5000, (a/5)/5000);

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
                         '*prod:34*')

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

        src = r'''
          #include <stdio.h>
          #include <sys/time.h>

          typedef long long int64;

          #define PRMJ_USEC_PER_SEC       1000000L

          int main(int argc, char * argv[]) {
              int64 sec = 1329409675 + argc;
              int64 usec = 2329509675;
              int64 mul = int64(sec) * PRMJ_USEC_PER_SEC;
              int64 add = mul + int64(usec);
              int add_low = add;
              int add_high = add >> 32;
              printf("*%lld,%lld,%u,%u*\n", mul, add, add_low, add_high);
              return 0;
          }
        '''

        self.do_run(src, '*1329409676000000,1329412005509675,3663280683,309527*\n')

    def test_i64_cmp(self):
        if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

        src = r'''
          #include <stdio.h>

          typedef long long int64;

          bool compare(int64 val) {
            return val == -12;
          }

          bool compare2(int64 val) {
            return val < -12;
          }

          int main(int argc, char * argv[]) {
              printf("*%d,%d,%d,%d,%d,%d*\n", argc, compare(argc-1-12), compare(1000+argc), compare2(argc-1-10), compare2(argc-1-14), compare2(argc+1000));
              return 0;
          }
        '''

        self.do_run(src, '*1,1,0,0,1,0*\n')

    def test_i64_cmp2(self):
        if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')
        src = r'''
          #include <inttypes.h>
          #include <stdio.h>

          typedef int32_t INT32;
          typedef int64_t INT64;
          typedef uint8_t UINT8;

          void interface_clock_changed()
          {
	          UINT8 m_divshift;
	          INT32 m_divisor;

	          //INT64 attos = m_attoseconds_per_cycle;
	          INT64 attos = 279365114840;
	          m_divshift = 0;
	          while (attos >= (1UL << 31))
	          {
		          m_divshift++;
		          printf("m_divshift is %i, on %Ld >?= %lu\n", m_divshift, attos, 1UL << 31);
		          attos >>= 1;
	          }
	          m_divisor = attos;

	          printf("m_divisor is %i\n",m_divisor);
          }

          int main() {
	          interface_clock_changed();
	          return 0;
          }
        '''
        self.do_run(src, '''m_divshift is 1, on 279365114840 >?= 2147483648
m_divshift is 2, on 139682557420 >?= 2147483648
m_divshift is 3, on 69841278710 >?= 2147483648
m_divshift is 4, on 34920639355 >?= 2147483648
m_divshift is 5, on 17460319677 >?= 2147483648
m_divshift is 6, on 8730159838 >?= 2147483648
m_divshift is 7, on 4365079919 >?= 2147483648
m_divshift is 8, on 2182539959 >?= 2147483648
m_divisor is 1091269979
''')

    def test_i64_double(self):
        if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')
        src = r'''
          #include <stdio.h>

          typedef long long int64;
          #define JSDOUBLE_HI32_SIGNBIT   0x80000000

          bool JSDOUBLE_IS_NEGZERO(double d)
          {
            union {
              struct {
                unsigned int lo, hi;
              } s;
              double d;
            } x;
            if (d != 0)
              return false;
            x.d = d;
            return (x.s.hi & JSDOUBLE_HI32_SIGNBIT) != 0;
          }

          bool JSINT64_IS_NEGZERO(int64 l)
          {
            union {
              int64 i;
              double d;
            } x;
            if (l != 0)
              return false;
            x.i = l;
            return x.d == -0;
          }

          int main(int argc, char * argv[]) {
            printf("*%d,%d,%d,%d*\n", JSDOUBLE_IS_NEGZERO(0), JSDOUBLE_IS_NEGZERO(-0), JSDOUBLE_IS_NEGZERO(-1), JSDOUBLE_IS_NEGZERO(+1));
            printf("*%d,%d,%d,%d*\n", JSINT64_IS_NEGZERO(0), JSINT64_IS_NEGZERO(-0), JSINT64_IS_NEGZERO(-1), JSINT64_IS_NEGZERO(+1));
            return 0;
          }
        '''
        self.do_run(src, '*0,0,0,0*\n*1,1,0,0*\n') # same as gcc

    def test_i64_umul(self):
        if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')
        src = r'''
          #include <inttypes.h>
          #include <stdio.h>

          typedef uint32_t UINT32;
          typedef uint64_t UINT64;

          int main() {
          volatile UINT32 testu32a = 2375724032U;
          UINT32 bigu32 = 0xffffffffU;
          volatile UINT64 testu64a = 14746250828952703000U;

          while ((UINT64)testu32a * (UINT64)bigu32 < testu64a) {
	          printf("testu64a is %llu\n", testu64a);
	          testu64a /= 2;
          }

          return 0;
          }
        '''
        self.do_run(src, 'testu64a is 14746250828952703000\n')

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

        # Verify that without precision, we do not include the precision code
        Settings.PRECISE_I64_MATH = 0
        self.do_run(src, 'unsigned')
        code = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
        assert 'goog.math.Long' not in code and 'jsbn' not in code, 'i64 precise math should not have been included if not asked for'

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
        assert 'goog.math.Long' not in code and 'jsbn' not in code, 'i64 precise math should not have been included if not actually used'

    def test_i64_zextneg(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      src = r'''
        #include <stdint.h>
        #include <stdio.h>

        int main(int argc, char *argv[])
        {
            uint8_t byte = 0x80;
            uint16_t two = byte;
            uint32_t four = byte;
            uint64_t eight = byte;

            printf("value: %d,%d,%d,%lld.\n", byte, two, four, eight);

            return 0;
        }
      '''
      self.do_run(src, 'value: 128,128,128,128.')

    def test_i64_7z(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      src = r'''
        #include <stdint.h>
        #include <stdio.h>
        uint64_t a, b;
        int main(int argc, char *argv[])
        {
            a = argc;
            b = argv[1][0];
            if (a > a + b || a > a + b + 1) {
                printf("one %lld, %lld", a, b);
                return 0;
            }
            printf("zero %lld, %lld", a, b);
            return 0;
        }
      '''
      self.do_run(src, 'zero 2, 104', ['hallo'])

    def test_i64_i16(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('full i64 stuff only in ta2')

      src = r'''
        #include <stdio.h>
        int main(int argc, char ** argv){
            int y=-133;
            __int64_t x= ((__int64_t)((short)(y)))*(100 + argc);
            if(x>0)
                printf(">0\n");
            else
                printf("<=0\n");
        }
      '''
      self.do_run(src, '<=0')

    def test_i32_mul_precise(self):
      if self.emcc_args == None: return self.skip('needs ta2')

      self.emcc_args += ['-s', 'PRECISE_I32_MUL=1']
      src = r'''
        #include <stdio.h>

        int main(int argc, char **argv) {
          unsigned long d1 = 0x847c9b5d;
          unsigned long q =  0x549530e1;
          if (argc > 1000) { q += argc; d1 -= argc; } // confuse optimizer
          printf("%lu\n", d1*q);
          return 0;
        }
      '''
      self.do_run(src, '3217489085')

    def test_i16_emcc_intrinsic(self):
      Settings.CORRECT_SIGNS = 1 # Relevant to this test

      src = r'''
        #include <stdio.h>

        int test(unsigned short a, unsigned short b) {
            unsigned short result = a;
            result += b;
            if (result < b) printf("C!");
            return result;
        }

        int main(void) {
            printf(",%d,", test(0, 0));
            printf(",%d,", test(1, 1));
            printf(",%d,", test(65535, 1));
            printf(",%d,", test(1, 65535));
            printf(",%d,", test(32768, 32767));
            printf(",%d,", test(32768, 32768));
            return 0;
        }
      '''
      self.do_run(src, ',0,,2,C!,0,C!,0,,65535,C!,0,')

    def test_bswap(self):
      if self.emcc_args == None: return self.skip('needs ta2')

      src = r'''
        #include <stdio.h>

        extern "C" {
          extern unsigned short llvm_bswap_i16(unsigned short x);
          extern unsigned int llvm_bswap_i32(unsigned int x);
        }

        int main(void) {
            unsigned short x = 0xc8ef;
            printf("%x,%x\n", x&0xff, x >> 8);
            x = llvm_bswap_i16(x);
            printf("%x,%x\n", x&0xff, x >> 8);

            unsigned int y = 0xc5de158a;
            printf("%x,%x,%x,%x\n", y&0xff, (y>>8)&0xff, (y>>16)&0xff, (y>>24)&0xff);
            y = llvm_bswap_i32(y);
            printf("%x,%x,%x,%x\n", y&0xff, (y>>8)&0xff, (y>>16)&0xff, (y>>24)&0xff);
            return 0;
        }
      '''
      self.do_run(src, '''ef,c8
c8,ef
8a,15,de,c5
c5,de,15,8a
''')

    def test_sha1(self):
      if self.emcc_args == None: return self.skip('needs ta2')

      self.do_run(open(path_from_root('tests', 'sha1.c')).read(), 'SHA1=15dd99a1991e0b3826fede3deffc1feba42278e6')

    def test_cube2md5(self):
      if self.emcc_args == None: return self.skip('needs emcc')
      self.emcc_args += ['--embed-file', 'cube2md5.txt']
      shutil.copyfile(path_from_root('tests', 'cube2md5.txt'), os.path.join(self.get_dir(), 'cube2md5.txt'))
      self.do_run(open(path_from_root('tests', 'cube2md5.cpp')).read(), open(path_from_root('tests', 'cube2md5.ok')).read())

    def test_cube2hash(self):
      # A good test of i64 math
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('requires ta2 C-style memory aliasing')
      self.do_run('', 'Usage: hashstring <seed>',
                  libraries=self.get_library('cube2hash', ['cube2hash.bc'], configure=None),  
                  includes=[path_from_root('tests', 'cube2hash')])

      for text, output in [('fleefl', '892BDB6FD3F62E863D63DA55851700FDE3ACF30204798CE9'),
                           ('fleefl2', 'AA2CC5F96FC9D540CA24FDAF1F71E2942753DB83E8A81B61'),
                           ('64bitisslow', '64D8470573635EC354FEE7B7F87C566FCAF1EFB491041670')]:
        self.do_run('', 'hash value: ' + output, [text], no_build=True)

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

        Settings.EMULATE_UNALIGNED_ACCESSES = 0

        try:
          self.do_run(src, '*300:1*\n*515559*\n*42949672960*\n')
        except Exception, e:
          assert 'must be aligned' in str(e), e # expected to fail without emulation

        # XXX TODO Settings.EMULATE_UNALIGNED_ACCESSES = 1
        #self.do_run(src, '*300:1*\n*515559*\n*42949672960*\n') # but succeeds with it

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
          Settings.CHECK_SIGNS = 1
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
        src = '''
          #include <stdio.h>
          struct bitty {
            unsigned x : 1;
            unsigned y : 1;
            unsigned z : 1;
          };
          int main()
          {
            bitty b;
            printf("*");
            for (int i = 0; i <= 1; i++)
              for (int j = 0; j <= 1; j++)
                for (int k = 0; k <= 1; k++) {
                  b.x = i;
                  b.y = j;
                  b.z = k;
                  printf("%d,%d,%d,", b.x, b.y, b.z);
                }
            printf("*\\n");
            return 0;
          }
        '''
        self.do_run(src, '*0,0,0,0,0,1,0,1,0,0,1,1,1,0,0,1,0,1,1,1,0,1,1,1,*')

    def test_floatvars(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            float x = 1.234, y = 3.5, q = 0.00000001;
            y *= 3;
            int z = x < y;
            printf("*%d,%d,%.1f,%d,%.4f,%.2f*\\n", z, int(y), y, (int)x, x, q);

            /*
            // Rounding behavior
            float fs[6] = { -2.75, -2.50, -2.25, 2.25, 2.50, 2.75 };
            double ds[6] = { -2.75, -2.50, -2.25, 2.25, 2.50, 2.75 };
            for (int i = 0; i < 6; i++)
              printf("*int(%.2f)=%d,%d*\\n", fs[i], int(fs[i]), int(ds[i]));
            */

            return 0;
          }
        '''
        self.do_run(src, '*1,10,10.5,1,1.2340,0.00*')

    def test_globaldoubles(self):
        src = r'''
          #include <stdlib.h>
          #include <stdio.h>

          double      testVu,    testVv,    testWu,    testWv;

          void Test(double _testVu, double _testVv, double _testWu, double _testWv)
          {
              testVu = _testVu;
              testVv = _testVv;
              testWu = _testWu;
              testWv = _testWv;
              printf("BUG?\n");
              printf("Display: Vu=%f  Vv=%f  Wu=%f  Wv=%f\n", testVu, testVv, testWu, testWv);
          }

          int main(void)
          {
              double v1 = 465.1;
              double v2 = 465.2;
              double v3 = 160.3;
              double v4 = 111.4;
              Test(v1, v2, v3, v4);
              return 0;
          }
        '''
        self.do_run(src, 'BUG?\nDisplay: Vu=465.100000  Vv=465.200000  Wu=160.300000  Wv=111.400000')

    def test_math(self):
        src = '''
          #include <stdio.h>
          #include <stdlib.h>
          #include <cmath>
          int main()
          {
            printf("*%.2f,%.2f,%d", M_PI, -M_PI, (1/0.0) > 1e300); // could end up as infinity, or just a very very big number
            printf(",%d", finite(NAN) != 0);
            printf(",%d", finite(INFINITY) != 0);
            printf(",%d", finite(-INFINITY) != 0);
            printf(",%d", finite(12.3) != 0);
            printf(",%d", isinf(NAN) != 0);
            printf(",%d", isinf(INFINITY) != 0);
            printf(",%d", isinf(-INFINITY) != 0);
            printf(",%d", isinf(12.3) != 0);
            div_t div_result = div(23, 10);
            printf(",%d", div_result.quot);
            printf(",%d", div_result.rem);
            double sine = -1.0, cosine = -1.0;
            sincos(0.0, &sine, &cosine);
            printf(",%1.1lf", sine);
            printf(",%1.1lf", cosine);
            float fsine = -1.0f, fcosine = -1.0f;
            sincosf(0.0, &fsine, &fcosine);
            printf(",%1.1f", fsine);
            printf(",%1.1f", fcosine);
            printf("*\\n");
            return 0;
          }
        '''
        self.do_run(src, '*3.14,-3.14,1,0,0,0,1,0,1,1,0,2,3,0.0,1.0,0.0,1.0*')

    def test_math_hyperbolic(self):
        src = open(path_from_root('tests', 'hyperbolic', 'src.c'), 'r').read()
        expected = open(path_from_root('tests', 'hyperbolic', 'output.txt'), 'r').read()
        self.do_run(src, expected)

    def test_getgep(self):
        # Generated code includes getelementptr (getelementptr, 0, 1), i.e., GEP as the first param to GEP
        src = '''
          #include <stdio.h>
          struct {
            int y[10];
            int z[10];
          } commonblock;

          int main()
          {
            for (int i = 0; i < 10; ++i) {
              commonblock.y[i] = 1;
              commonblock.z[i] = 2;
            }
            printf("*%d %d*\\n", commonblock.y[0], commonblock.z[0]);
            return 0;
          }
        '''
        self.do_run(src, '*1 2*')

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
        self.do_run(src, '*yes*')

    def test_if_else(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            int x = 5;
            if (x > 10) {
              printf("*yes*\\n");
            } else {
              printf("*no*\\n");
            }
            return 0;
          }
        '''
        self.do_run(src, '*no*')

    def test_loop(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            int x = 5;
            for (int i = 0; i < 6; i++) {
              x += x*i;
              if (x > 1000) {
                if (x % 7 == 0) printf("cheez\\n");
                x /= 2;
                break;
              }
            }
            printf("*%d*\\n", x);
            return 0;
          }
        '''

        self.do_run(src, '*1800*')

        generated = open('src.cpp.o.js', 'r').read()
        assert '__label__ ==' not in generated, 'We should hoist into the loop'

    def test_stack(self):
        src = '''
          #include <stdio.h>
          int test(int i) {
            int x = 10;
            if (i > 0) {
              return test(i-1);
            }
            return int(&x); // both for the number, and forces x to not be nativized
          }
          int main()
          {
            // We should get the same value for the first and last - stack has unwound
            int x1 = test(0);
            int x2 = test(100);
            int x3 = test(0);
            printf("*%d,%d*\\n", x3-x1, x2 != x1);
            return 0;
          }
        '''
        self.do_run(src, '*0,1*')

    def test_strings(self):
        src = '''
          #include <stdio.h>
          #include <stdlib.h>
          #include <string.h>

          int main(int argc, char **argv)
          {
            int x = 5, y = 9, magic = 7; // fool compiler with magic
            memmove(&x, &y, magic-7); // 0 should not crash us

            int xx, yy, zz;
            char s[32];
            int cc = sscanf("abc_10.b1_xyz_543_defg", "abc_%d.%2x_xyz_%3d_%3s", &xx, &yy, &zz, s);
            printf("%d:%d,%d,%d,%s\\n", cc, xx, yy, zz, s);

            printf("%d\\n", argc);
            puts(argv[1]);
            puts(argv[2]);
            printf("%d\\n", atoi(argv[3])+2);
            const char *foolingthecompiler = "\\rabcd";
            printf("%d\\n", strlen(foolingthecompiler)); // Tests parsing /0D in llvm - should not be a 0 (end string) then a D!
            printf("%s\\n", NULL); // Should print '(null)', not the string at address 0, which is a real address for us!
            printf("/* a comment */\\n"); // Should not break the generated code!
            printf("// another\\n"); // Should not break the generated code!

            char* strdup_val = strdup("test");
            printf("%s\\n", strdup_val);
            free(strdup_val);

            return 0;
          }
        '''
        self.do_run(src, '4:10,177,543,def\n4\nwowie\ntoo\n76\n5\n(null)\n/* a comment */\n// another\ntest\n', ['wowie', 'too', '74'])

    def test_strcmp_uni(self):
      src = '''
        #include <stdio.h>
        #include <string.h>
        int main()
        {
          #define TEST(func) \
          { \
            char *word = "WORD"; \
            char *wordEntry = "Â"; \
            int cmp = func(word, wordEntry, 2); \
            printf("Compare value " #func " is %d\\n", cmp); \
          }
          TEST(strncmp);
          TEST(strncasecmp);
          TEST(memcmp);
        }
      '''
      self.do_run(src, 'Compare value strncmp is -1\nCompare value strncasecmp is -1\nCompare value memcmp is -1\n')

    def test_strndup(self):
        src = '''
          //---------------
          //- http://pubs.opengroup.org/onlinepubs/9699919799/functions/strndup.html
          //---------------

          #include <stdio.h>
          #include <stdlib.h>
          #include <string.h>

          int main(int argc, char **argv) {
            const char* source = "strndup - duplicate a specific number of bytes from a string";

            char* strdup_val = strndup(source, 0);
            printf("1:%s\\n", strdup_val);
            free(strdup_val);

            strdup_val = strndup(source, 7);
            printf("2:%s\\n", strdup_val);
            free(strdup_val);

            strdup_val = strndup(source, 1000);
            printf("3:%s\\n", strdup_val);
            free(strdup_val);

            strdup_val = strndup(source, 60);
            printf("4:%s\\n", strdup_val);
            free(strdup_val);

            strdup_val = strndup(source, 19);
            printf("5:%s\\n", strdup_val);
            free(strdup_val);

            strdup_val = strndup(source, -1);
            printf("6:%s\\n", strdup_val);
            free(strdup_val);

            return 0;
          }
        '''
        self.do_run(src, '1:\n2:strndup\n3:strndup - duplicate a specific number of bytes from a string\n4:strndup - duplicate a specific number of bytes from a string\n5:strndup - duplicate\n6:\n')

    def test_errar(self):
        src = r'''
          #include <stdio.h>
          #include <errno.h>
          #include <string.h>

          int main() {
            char* err;
            char buffer[200];

            err = strerror(EDOM);
            strerror_r(EWOULDBLOCK, buffer, 200);
            printf("<%s>\n", err);
            printf("<%s>\n", buffer);

            printf("<%d>\n", strerror_r(EWOULDBLOCK, buffer, 0));
            errno = 123;
            printf("<%d>\n", errno);

            return 0;
          }
          '''
        expected = '''
          <Numerical argument out of domain>
          <Resource temporarily unavailable>
          <34>
          <123>
          '''
        self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected))

    def test_mainenv(self):
        src = '''
          #include <stdio.h>
          int main(int argc, char **argv, char **envp)
          {
            printf("*%p*\\n", envp);
            return 0;
          }
        '''
        self.do_run(src, '*(nil)*')

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
        self.do_run(src, '*72,90*')

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
        self.do_run(src, '*10,6,101,7018,101,7018,101,7018*')

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
        self.do_run(src, '*7*')

    def test_globals(self):
        src = '''
          #include <stdio.h>

          char cache[256], *next = cache;

          int main()
          {
            cache[10] = 25;
            next[20] = 51;
            printf("*%d,%d*\\n", next[10], cache[20]);
            return 0;
          }
        '''
        self.do_run(src, '*25,51*')

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
        self.do_run(src, '*1410,0*')

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
            return 1;
          }
        '''
        if Settings.QUANTUM_SIZE == 1:
          self.do_run(src, 'sizeofs:6,8\n*C___: 0,3,6,9<24*\n*Carr: 0,3,6,9<24*\n*C__w: 0,3,9,12<24*\n*Cp1_: 1,2,5,8<24*\n*Cp2_: 0,2,5,8<24*\n*Cint: 0,3,4,7<24*\n*C4__: 0,3,4,7<24*\n*C4_2: 0,3,5,8<20*\n*C__z: 0,3,5,8<28*')
        else:
          self.do_run(src, 'sizeofs:6,8\n*C___: 0,6,12,20<24*\n*Carr: 0,6,12,20<24*\n*C__w: 0,6,12,20<24*\n*Cp1_: 4,6,12,20<24*\n*Cp2_: 0,6,12,20<24*\n*Cint: 0,8,12,20<24*\n*C4__: 0,8,12,20<24*\n*C4_2: 0,6,10,16<20*\n*C__z: 0,8,16,24<28*')

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
        self.do_run(src, 'Assertion failed: 1 == false')

    def test_longjmp(self):
        src = r'''
          #include <stdio.h>
          #include <setjmp.h>
           
          static jmp_buf buf;
           
          void second(void) {
              printf("second\n");         // prints
              longjmp(buf,1);             // jumps back to where setjmp was called - making setjmp now return 1
          }
           
          void first(void) {
              second();
              printf("first\n");          // does not print
          }
           
          int main() {
              int x = 0;
              if ( ! setjmp(buf) ) {
                  x++;
                  first();                // when executed, setjmp returns 0
              } else {                    // when longjmp jumps back, setjmp returns 1
                  printf("main: %d\n", x);       // prints
              }
           
              return 0;
          }
        '''
        # gcc -O0 and -O2 differ in what they do with the saved state of local vars - and we match that
        if self.emcc_args is None or ('-O1' not in self.emcc_args and '-O2' not in self.emcc_args):
          self.do_run(src, 'second\nmain: 1\n')
        else:
          self.do_run(src, 'second\nmain: 0\n')

    def test_exceptions(self):
        if Settings.QUANTUM_SIZE == 1: return self.skip("we don't support libcxx in q1")

        self.banned_js_engines = [NODE_JS] # node issue 1669, exception causes stdout not to be flushed
        Settings.DISABLE_EXCEPTION_CATCHING = 0
        if self.emcc_args is None:
          if Building.LLVM_OPTS: return self.skip('optimizing bitcode before emcc can confuse libcxx inclusion')
          self.emcc_args = [] # libc++ auto-inclusion is only done if we use emcc
        else:
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
            return 1;
          }
        '''
        self.do_run(src, '*throw...caught!infunc...done!*')

        Settings.DISABLE_EXCEPTION_CATCHING = 1
        self.do_run(src, 'Compiled code throwing an exception')
        
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
        self.do_run(src, 'Throw...Construct...Catched...Destruct...Throw...Construct...Copy...Catched...Destruct...Destruct...')

    def test_uncaught_exception(self):
        if self.emcc_args is None: return self.skip('no libcxx inclusion without emcc')
        if '-O2' in self.emcc_args:
          self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

        Settings.EXCEPTION_DEBUG = 0  # Messes up expected output.
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
        Settings.DISABLE_EXCEPTION_CATCHING = 0
        Settings.SAFE_HEAP = 0  # Throwing null will cause an ignorable null pointer access.
        Settings.EXCEPTION_DEBUG = 0  # Messes up expected output.
        src = open(path_from_root('tests', 'exceptions', 'typed.cpp'), 'r').read()
        expected = open(path_from_root('tests', 'exceptions', 'output.txt'), 'r').read()
        self.do_run(src, expected)

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
        self.do_run(src, '*0*')

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
        self.do_run(src, '*51,87,78,550,100,78,550*')

    def test_isdigit_l(self):
        if self.emcc_args is None: return self.skip('no libcxx inclusion without emcc')

        src = '''
          #include <iostream>
          int main() {
            using namespace std;
            use_facet<num_put<char> >(cout.getloc()).put(cout, cout, '0', 3.14159265);
          }
        '''
        self.do_run(src, '3.14159')

    def test_polymorph(self):
        src = '''
          #include <stdio.h>
          struct Pure {
            virtual int implme() = 0;
          };
          struct Parent : Pure {
            virtual int getit() { return 11; };
            int implme() { return 32; }
          };
          struct Child : Parent {
            int getit() { return 74; }
            int implme() { return 1012; }
          };

          struct Other {
            int one() { return 11; }
            int two() { return 22; }
          };

          int main()
          {
            Parent *x = new Parent();
            Parent *y = new Child();
            printf("*%d,%d,%d,%d*\\n", x->getit(), y->getit(), x->implme(), y->implme());

            Other *o = new Other;
            int (Other::*Ls)() = &Other::one;
            printf("*%d*\\n", (o->*(Ls))());
            Ls = &Other::two;
            printf("*%d*\\n", (o->*(Ls))());

            return 0;
          }
        '''
        self.do_run(src, '*11,74,32,1012*\n*11*\n*22*')

    def test_dynamic_cast(self):
        if self.emcc_args is None: return self.skip('need libcxxabi')

        src = r'''
          #include <stdio.h>

          struct Support {
            virtual void f() {
              printf("f()\n");
            }
          };

          struct Derived : Support {
          };

          int main() {
            Support * p = new Derived;
            dynamic_cast<Derived*>(p)->f();
          }
        '''
        self.do_run(src, 'f()\n')

        src = '''
          #include <stdio.h>

          class CBase { virtual void dummy() {} };
          class CDerived : public CBase { int a; };
          class CDerivedest : public CDerived { float b; };

          int main ()
          {
            CBase *pa = new CBase;
            CBase *pb = new CDerived;
            CBase *pc = new CDerivedest;

            printf("a1: %d\\n", dynamic_cast<CDerivedest*>(pa) != NULL);
            printf("a2: %d\\n", dynamic_cast<CDerived*>(pa) != NULL);
            printf("a3: %d\\n", dynamic_cast<CBase*>(pa) != NULL);

            printf("b1: %d\\n", dynamic_cast<CDerivedest*>(pb) != NULL);
            printf("b2: %d\\n", dynamic_cast<CDerived*>(pb) != NULL);
            printf("b3: %d\\n", dynamic_cast<CBase*>(pb) != NULL);

            printf("c1: %d\\n", dynamic_cast<CDerivedest*>(pc) != NULL);
            printf("c2: %d\\n", dynamic_cast<CDerived*>(pc) != NULL);
            printf("c3: %d\\n", dynamic_cast<CBase*>(pc) != NULL);

            return 0;
          }
        '''
        self.do_run(src, 'a1: 0\na2: 0\na3: 1\nb1: 0\nb2: 1\nb3: 1\nc1: 1\nc2: 1\nc3: 1\n')

    def test_funcptr(self):
        src = '''
          #include <stdio.h>
          int calc1() { return 26; }
          int calc2() { return 90; }
          typedef int (*fp_t)();

          fp_t globally1 = calc1;
          fp_t globally2 = calc2;

          int nothing(const char *str) { return 0; }

          int main()
          {
            fp_t fp = calc1;
            void *vp = (void*)fp;
            fp_t fpb = (fp_t)vp;
            fp_t fp2 = calc2;
            void *vp2 = (void*)fp2;
            fp_t fpb2 = (fp_t)vp2;
            printf("*%d,%d,%d,%d,%d,%d*\\n", fp(), fpb(), fp2(), fpb2(), globally1(), globally2());

            fp_t t = calc1;
            printf("*%d,%d", t == calc1, t == calc2);
            t = calc2;
            printf(",%d,%d*\\n", t == calc1, t == calc2);

            int (*other)(const char *str);
            other = nothing;
            other("*hello!*");
            other = puts;
            other("*goodbye!*");

            return 0;
          }
        '''
        self.do_run(src, '*26,26,90,90,26,90*\n*1,0,0,1*\n*goodbye!*')

    def test_mathfuncptr(self):
        src = '''
          #include <math.h>
          #include <stdio.h>

          int
          main(void) {
           float (*fn)(float) = &sqrtf;
           float (*fn2)(float) = &fabsf;
           printf("fn2(-5) = %d, fn(10) = %f\\n", (int)fn2(-5), fn(10));
           return 0;
          }
          '''
        self.do_run(src, 'fn2(-5) = 5, fn(10) = 3.16')

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
        self.do_run(src, '*zzcheezzz*')

    def test_alloca(self):
      src = '''
        #include <stdio.h>
        #include <stdlib.h>

        int main() {
          char *pc;
          pc = (char *)alloca(5);
          printf("z:%d*%d*\\n", pc > 0, (int)pc);
          return 0;
        }
      '''
      self.do_run(src, 'z:1*', force_c=True)

    def test_alloca_stack(self):
      if self.emcc_args is None: return # too slow in other modes

      # We should not blow up the stack with numerous allocas
      src = '''
        #include <stdio.h>
        #include <stdlib.h>

        func(int i) {
          char *pc = (char *)alloca(100);
          *pc = i;
          (*pc)++;
          return (*pc) % 10;
        }
        int main() {
          int total = 0;
          for (int i = 0; i < 1024*1024; i++)
            total += func(i);
          printf("ok:%d*\\n", total);
          return 0;
        }
      '''
      self.do_run(src, 'ok:-32768*', force_c=True)

    def test_stack_byval(self):
      if self.emcc_args is None: return # too slow in other modes

      # We should also not blow up the stack with byval arguments
      src = r'''
        #include<stdio.h>
        struct vec {
          int x, y, z;
          vec(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
          static vec add(vec a, vec b) {
            return vec(a.x+b.x, a.y+b.y, a.z+b.z);
          }
        };
        int main() {
          int total = 0;
          for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++) {
              vec c(i+i%10, j*2, i%255);
              vec d(j*2, j%255, i%120);
              vec f = vec::add(c, d);
              total += (f.x + f.y + f.z) % 100;
              total %= 10240;
            }
          }
          printf("sum:%d*\n", total);
          return 1;
        }
      '''
      self.do_run(src, 'sum:9780*')

    def test_stack_varargs(self):
      if self.emcc_args is None: return # too slow in other modes

      # We should not blow up the stack with numerous varargs
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>

        void func(int i) {
          printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                   i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
        }
        int main() {
          for (int i = 0; i < 1024; i++)
            func(i);
          printf("ok!\n");
          return 0;
        }
      '''
      Settings.TOTAL_STACK = 1024
      self.do_run(src, 'ok!')

    def test_stack_void(self):
      src = r'''
        #include <stdio.h>

        static char s[100]="aaaaa";
        static int func(void) {
          if(s[0]!='a') return 0; 
          printf("iso open %s\n", s, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001, 1.001);
          return 0;
        }
        int main(){
          int i;
          for(i=0;i<5000;i++)
            func();
          printf(".ok.\n");
        }
      '''
      self.do_run(src, '.ok.\n')

    def test_array2(self):
        src = '''
          #include <stdio.h>

          static const double grid[4][2] = {
           {-3/3.,-1/3.},{+1/3.,-3/3.},
           {-1/3.,+3/3.},{+3/3.,+1/3.}
          };

          int main() {
            for (int i = 0; i < 4; i++)
              printf("%d:%.2f,%.2f ", i, grid[i][0], grid[i][1]);
            printf("\\n");
            return 0;
          }
          '''
        self.do_run(src, '0:-1.00,-0.33 1:0.33,-1.00 2:-0.33,1.00 3:1.00,0.33')

    def test_array2b(self):
        src = '''
          #include <stdio.h>

          static const struct {
            unsigned char left;
            unsigned char right;
          } prioritah[] = {
             {6, 6}, {6, 6}, {7, 95}, {7, 7}
          };

          int main() {
            printf("*%d,%d\\n", prioritah[1].left, prioritah[1].right);
            printf("%d,%d*\\n", prioritah[2].left, prioritah[2].right);
            return 0;
          }
          '''
        self.do_run(src, '*6,6\n7,95*')


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

          const unsigned char faceedgesidx[6][4] =
          {
              { 4, 5, 8, 10 },
              { 6, 7, 9, 11 },
              { 0, 2, 8, 9  },
              { 1, 3, 10,11 },
              { 0, 1, 4, 6 },
              { 2, 3, 5, 7 },
          };

          int main( int argc, const char *argv[] ) {
             printf("*%d,%d,%d,%d*\\n", iub[0].c, int(iub[1].p*100), iub[2].pi, faceedgesidx[3][2]);
             return 0;
          }
          '''
        self.do_run(src, '*97,15,3,10*')

    def test_conststructs(self):
        src = '''
          #include <stdio.h>
          struct IUB {
             int c;
             double p;
             unsigned int pi;
          };

          int main( int argc, const char *argv[] ) {
             int before = 70;
             IUB iub[] = {
                { 'a', 0.3029549426680, 5 },
                { 'c', 0.15, 4 },
                { 'g', 0.12, 3 },
                { 't', 0.27, 2 },
             };
             int after = 90;
             printf("*%d,%d,%d,%d,%d,%d*\\n", before, iub[0].c, int(iub[1].p*100), iub[2].pi, int(iub[0].p*10000), after);
             return 0;
          }
          '''
        self.do_run(src, '*70,97,15,3,3029,90*')

    def test_mod_globalstruct(self):
        src = '''
          #include <stdio.h>

          struct malloc_params {
            size_t magic, page_size;
          };

          malloc_params mparams;

          #define SIZE_T_ONE ((size_t)1)
          #define page_align(S) (((S) + (mparams.page_size - SIZE_T_ONE)) & ~(mparams.page_size - SIZE_T_ONE))

          int main()
          {
            mparams.page_size = 4096;
            printf("*%d,%d,%d,%d*\\n", mparams.page_size, page_align(1000), page_align(6000), page_align(66474));
            return 0;
          }
        '''
        self.do_run(src, '*4096,4096,8192,69632*')

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
          self.do_run(src, '*0,0,0,4,8,12,16,20*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*12,20,20*')

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
        self.do_run(src, '*5*', output_processor=check_warnings)

    def test_sizeof(self):
        # Has invalid writes between printouts
        Settings.SAFE_HEAP = 0

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
            memcpy(a, c, 10*sizeof(int));
            printf("*%d,%d,%d,%d,%d*\\n", a[0], a[9], *b, c[0], c[9]);

            // Part 2
            A as[3] = { { 5, 12 }, { 6, 990 }, { 7, 2 } };
            memcpy(&as[0], &as[2], sizeof(A));

            printf("*%d,%d,%d,%d,%d,%d*\\n", as[0].x, as[0].y, as[1].x, as[1].y, as[2].x, as[2].y);
            return 0;
          }
          '''
        self.do_run(src, '*2,2,5,8,8***8,8,5,8,8***7,2,6,990,7,2*', [], lambda x: x.replace('\n', '*'))

    def test_emscripten_api(self):
        #if Settings.MICRO_OPTS or Settings.RELOOP or Building.LLVM_OPTS: return self.skip('FIXME')

        src = r'''
          #include <stdio.h>
          #include "emscripten.h"

          extern "C" {
            void EMSCRIPTEN_KEEPALIVE save_me_aimee() { printf("mann\n"); }
          }

          int main() {
            // EMSCRIPTEN_COMMENT("hello from the source");
            emscripten_run_script("Module.print('hello world' + '!')");
            printf("*%d*\n", emscripten_run_script_int("5*20"));
            emscripten_run_script("_save_me_aimee()");
            return 0;
          }
          '''

        check = '''
def process(filename):
  src = open(filename, 'r').read()
  # TODO: restore this (see comment in emscripten.h) assert '// hello from the source' in src
'''

        self.do_run(src, 'hello world!\n*100*\nmann\n', post_build=check)

    def test_inlinejs(self):
        src = r'''
          #include <stdio.h>

          int main() {
            asm("Module.print('Inline JS is very cool')");
            return 0;
          }
          '''

        self.do_run(src, 'Inline JS is very cool')

    def test_memorygrowth(self):
      # With typed arrays in particular, it is dangerous to use more memory than TOTAL_MEMORY,
      # since we then need to enlarge the heap(s).
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        #include <assert.h>
        #include "emscripten.h"

        int main()
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
      self.do_run(src, '*pre: hello,4.955*\n*hello,4.955*\n*hello,4.955*')

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
        src = '''
          #include <stdio.h>

          struct Class {
            static char *name1() { return "nameA"; }
                   char *name2() { return "nameB"; }
          };

          int main() {
            printf("*%s,%s*\\n", Class::name1(), (new Class())->name2());
            return 0;
          }
          '''
        self.do_run(src, '*nameA,nameB*')

    def test_llvmswitch(self):
        Settings.CORRECT_SIGNS = 1

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
              case 0xfffffff1:
                  return p+1;
            }
            return p;
          }

          int main( int argc, const char *argv[] ) {
            unsigned int x = 0xfffffff1;
            x >>= 0; // force it to be unsigned for purpose of checking our switch comparison in signed/unsigned
            printf("*%d,%d,%d,%d,%d,%d*\\n", switcher('a'), switcher('b'), switcher('c'), switcher(x), switcher(-15), switcher('e'));
            return 0;
          }
          '''
        self.do_run(src, '*96,97,98,-14,-14,101*')

    def test_indirectbr(self):
        src = '''
          #include <stdio.h>
          int main(void) {
            const void *addrs[2] = { &&FOO, &&BAR };

            // confuse the optimizer so it doesn't hardcode the jump and avoid generating an |indirectbr| instruction
            int which = 0;
            for (int x = 0; x < 1000; x++) which = (which + x*x) % 7;
            which = (which % 2) + 1;

            goto *addrs[which];

          FOO:
            printf("bad\\n");
            return 1;
          BAR:
            printf("good\\n");
            const void *addr = &&FOO;
            goto *addr;
          }
          '''
        self.do_run(src, 'good\nbad')

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

        src = '''
          #include <stdio.h>
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
            __va_copy(tempva, v);
            vsnprintf(d, 20, s, tempva);
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

          #define GETMAX(pref, type) \
            type getMax##pref(int num, ...) \
            { \
              va_list vv; \
              va_start(vv, num); \
              type maxx = va_arg(vv, type); \
              for (int i = 1; i < num; i++) \
              { \
                type curr = va_arg(vv, type); \
                maxx = curr > maxx ? curr : maxx; \
              } \
              va_end(vv); \
              return maxx; \
            }
          GETMAX(i, int);
          GETMAX(D, double);

          int main() {
            vary("*cheez: %d+%d*", 0, 24); // Also tests that '0' is not special as an array ender
            vary("*albeit*"); // Should not fail with no var args in vararg function
            vary2('Q', "%d*", 85);

            int maxxi = getMaxi(6,        2, 5, 21, 4, -10, 19);
            printf("maxxi:%d*\\n", maxxi);
            double maxxD = getMaxD(6,        (double)2.1, (double)5.1, (double)22.1, (double)4.1, (double)-10.1, (double)19.1);
            printf("maxxD:%.2f*\\n", (float)maxxD);

            // And, as a function pointer
            void (*vfp)(const char *s, ...) = vary;
            vfp("*vfp:%d,%d*", 22, 199);

            return 0;
          }
          '''
        self.do_run(src, '*cheez: 0+24*\n*cheez: 0+24*\n*albeit*\n*albeit*\nQ85*\nmaxxi:21*\nmaxxD:22.10*\n*vfp:22,199*\n*vfp:22,199*\n')

    def test_structbyval(self):
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
        output = Popen(['python', EMCC, all_name], stderr=PIPE).communicate()
        # Check for warning in the generated code
        generated = open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read()
        assert 'Casting a function pointer type to another with a different number of arguments' in output[1], 'Missing expected warning'

    def test_stdlibs(self):
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

    def test_atexit(self):
      # Confirms they are called in reverse order
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>
        
        static void cleanA() {
          printf("A");
        }
        static void cleanB() {
          printf("B");
        }
        
        int main() {
          atexit(cleanA);
          atexit(cleanB);
          return 0;
        }
        '''
      self.do_run(src, 'BA')

    def test_time(self):
      # XXX Not sure what the right output is here. Looks like the test started failing with daylight savings changes. Modified it to pass again.
      src = open(path_from_root('tests', 'time', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'time', 'output.txt'), 'r').read()
      self.do_run(src, expected,
                   extra_emscripten_args=['-H', 'libc/time.h'])
                   #extra_emscripten_args=['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/langinfo.h,libc/time.h'])

    def test_intentional_fault(self):
      # Some programs intentionally segfault themselves, we should compile that into a throw
      src = r'''
        int main () {
          *(volatile char *)0 = 0;
          return 0;
        }
        '''
      self.do_run(src, 'fault on write to 0')

    def test_trickystring(self):
      src = r'''
        #include <stdio.h>

        typedef struct
        {
	        int (*f)(void *);
	        void *d;
	        char s[16];
        } LMEXFunctionStruct;

        int f(void *user)
        {
	        return 0;
        }

        static LMEXFunctionStruct const a[] =
        {
	        {f, (void *)(int)'a', "aa"}
        };

        int main()
        {
          printf("ok\n");
	        return a[0].f(a[0].d);
        }
      '''
      self.do_run(src, 'ok\n')

    def test_statics(self):
        # static initializers save i16 but load i8 for some reason
        if Settings.SAFE_HEAP:
          Settings.SAFE_HEAP = 3
          Settings.SAFE_HEAP_LINES = ['src.cpp:19', 'src.cpp:26']

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

          struct XYZ {
            float x, y, z;
            XYZ(float a, float b, float c) : x(a), y(b), z(c) { }
            static const XYZ& getIdentity()
            {
              static XYZ iT(1,2,3);
              return iT;
            }
          };
          struct S {
            static const XYZ& getIdentity()
            {
              static const XYZ iT(XYZ::getIdentity());
              return iT;
            }
          };

          int main() {
            conoutfv("*staticccz*");
            printf("*%.2f,%.2f,%.2f*\\n", S::getIdentity().x, S::getIdentity().y, S::getIdentity().z);
            return 0;
          }
          '''
        self.do_run(src, '*staticccz*\n*1.00,2.00,3.00*')

    def test_copyop(self):
        # clang generated code is vulnerable to this, as it uses
        # memcpy for assignments, with hardcoded numbers of bytes
        # (llvm-gcc copies items one by one). See QUANTUM_SIZE in
        # settings.js.
        src = '''
          #include <stdio.h>
          #include <math.h>
          #include <string.h>

          struct vec {
            double x,y,z;
            vec() : x(0), y(0), z(0) { };
            vec(const double a, const double b, const double c) : x(a), y(b), z(c) { };
          };

          struct basis {
            vec a, b, c;
            basis(const vec& v) {
              a=v; // should not touch b!
              printf("*%.2f,%.2f,%.2f*\\n", b.x, b.y, b.z);
            }
          };

          int main() {
            basis B(vec(1,0,0));

            // Part 2: similar problem with memset and memmove
            int x = 1, y = 77, z = 2;
            memset((void*)&x, 0, sizeof(int));
            memset((void*)&z, 0, sizeof(int));
            printf("*%d,%d,%d*\\n", x, y, z);
            memcpy((void*)&x, (void*)&z, sizeof(int));
            memcpy((void*)&z, (void*)&x, sizeof(int));
            printf("*%d,%d,%d*\\n", x, y, z);
            memmove((void*)&x, (void*)&z, sizeof(int));
            memmove((void*)&z, (void*)&x, sizeof(int));
            printf("*%d,%d,%d*\\n", x, y, z);
            return 0;
          }
          '''
        self.do_run(src, '*0.00,0.00,0.00*\n*0,77,0*\n*0,77,0*\n*0,77,0*')

    def test_memcpy(self):
        src = '''
          #include <stdio.h>
          #include <string.h>
          #define MAXX 48
          void reset(unsigned char *buffer) {
            for (int i = 0; i < MAXX; i++) buffer[i] = i+1;
          }
          void dump(unsigned char *buffer) {
            for (int i = 0; i < MAXX-1; i++) printf("%2d,", buffer[i]);
            printf("%d\\n", buffer[MAXX-1]);
          }
          int main() {
            unsigned char buffer[MAXX];
            for (int i = MAXX/4; i < MAXX-MAXX/4; i++) {
              for (int j = MAXX/4; j < MAXX-MAXX/4; j++) {
                for (int k = 1; k < MAXX/4; k++) {
                  if (i == j) continue;
                  if (i < j && i+k > j) continue;
                  if (j < i && j+k > i) continue;
                  printf("[%d,%d,%d] ", i, j, k);
                  reset(buffer);
                  memcpy(buffer+i, buffer+j, k);
                  dump(buffer);
                }
              }
            }
            return 0;
          }
          '''
        def check(result):
          return hashlib.sha1(result).hexdigest()
        self.do_run(src, '6c9cdfe937383b79e52ca7a2cce83a21d9f5422c',
                    output_nicerizer = check)

    def test_memmove(self):
      src = '''
        #include <stdio.h>
        #include <string.h>
        int main() {
          char str[] = "memmove can be very useful....!";
          memmove (str+20, str+15, 11);
          puts(str);
          return 0;
        }
      '''
      self.do_run(src, 'memmove can be very very useful')

    def test_bsearch(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip('Test cannot work with q1')

      src = '''
          #include <stdlib.h>
          #include <stdio.h>

          int cmp(const void* key, const void* member) {
            return *(int *)key - *(int *)member;
          }

          void printResult(int* needle, int* haystack, unsigned int len) {
            void *result = bsearch(needle, haystack, len, sizeof(unsigned int), cmp);

            if (result == NULL) {
              printf("null\\n");
            } else {
              printf("%d\\n", *(unsigned int *)result);
            }
          }

          int main() {
            int a[] = { -2, -1, 0, 6, 7, 9 };
            int b[] = { 0, 1 };

            /* Find all keys that exist. */
            for(int i = 0; i < 6; i++) {
              int val = a[i];

              printResult(&val, a, 6);
            }

            /* Keys that are covered by the range of the array but aren't in
             * the array cannot be found.
             */
            int v1 = 3;
            int v2 = 8;
            printResult(&v1, a, 6);
            printResult(&v2, a, 6);

            /* Keys outside the range of the array cannot be found. */
            int v3 = -1;
            int v4 = 2;

            printResult(&v3, b, 2);
            printResult(&v4, b, 2);

            return 0;
          }
          '''

      self.do_run(src, '-2\n-1\n0\n6\n7\n9\nnull\nnull\nnull\nnull')

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
      if Building.LLVM_OPTS: return self.skip('LLVM opts will optimize printf into puts in the parent, and the child will still look for puts')

      Settings.LINKABLE = 1

      self.banned_js_engines = [NODE_JS] # node's global scope behaves differently than everything else, needs investigation FIXME

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

        extern void mainFunc(int x);
        extern int mainInt;

        void suppFunc(struct point &p) {
          printf("supp: %d,%d\n", p.x, p.y);
          mainFunc(p.x+p.y);
          printf("supp see: %d\n", mainInt);
        }

        int suppInt = 76;
      '''
      supp_name = os.path.join(self.get_dir(), 'supp.c')
      open(supp_name, 'w').write(supp)

      main = r'''
        #include <stdio.h>
        #include "header.h"

        extern void suppFunc(struct point &p);
        extern int suppInt;

        void mainFunc(int x) {
          printf("main: %d\n", x);
        }

        int mainInt = 543;

        int main( int argc, const char *argv[] ) {
          struct point p = { 54, 2 };
          suppFunc(p);
          printf("main see: %d\nok.\n", suppInt);
          return 0;
        }
      '''

      Settings.BUILD_AS_SHARED_LIB = 2
      dirname = self.get_dir()
      self.build(supp, dirname, supp_name)
      shutil.move(supp_name + '.o.js', os.path.join(dirname, 'liblib.so'))
      Settings.BUILD_AS_SHARED_LIB = 0

      Settings.RUNTIME_LINKED_LIBS = ['liblib.so'];
      self.do_run(main, 'supp: 54,2\nmain: 56\nsupp see: 543\nmain see: 76\nok.')

    def test_dlfcn_basic(self):
      Settings.LINKABLE = 1

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
      Settings.BUILD_AS_SHARED_LIB = 1
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

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
      Settings.BUILD_AS_SHARED_LIB = 0
      add_pre_run_and_checks = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createLazyFile('/', 'liblib.so', 'liblib.so', true, false);"
  )
  open(filename, 'w').write(src)
'''
      self.do_run(src, 'Constructing main object.\nConstructing lib object.\n',
                  post_build=add_pre_run_and_checks)

    def test_dlfcn_qsort(self):
      Settings.LINKABLE = 1

      if Settings.USE_TYPED_ARRAYS == 2:
        Settings.CORRECT_SIGNS = 1 # Needed for unsafe optimizations

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
      Settings.BUILD_AS_SHARED_LIB = 1
      Settings.EXPORTED_FUNCTIONS = ['_get_cmp']
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

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

          qsort((void*)arr, 5, sizeof(int), main_cmp);
          printf("Sort with main comparison: ");
          for (int i = 0; i < 5; i++) {
            printf("%d ", arr[i]);
          }
          printf("\\n");

          qsort((void*)arr, 5, sizeof(int), lib_cmp_ptr);
          printf("Sort with lib comparison: ");
          for (int i = 0; i < 5; i++) {
            printf("%d ", arr[i]);
          }
          printf("\\n");

          return 0;
        }
        '''
      Settings.BUILD_AS_SHARED_LIB = 0
      Settings.EXPORTED_FUNCTIONS = ['_main']
      add_pre_run_and_checks = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createLazyFile('/', 'liblib.so', 'liblib.so', true, false);"
  )
  open(filename, 'w').write(src)
'''
      self.do_run(src, 'Sort with main comparison: 5 4 3 2 1 *Sort with lib comparison: 1 2 3 4 5 *',
                  output_nicerizer=lambda x: x.replace('\n', '*'),
                  post_build=add_pre_run_and_checks)

    def test_dlfcn_data_and_fptr(self):
      if Building.LLVM_OPTS: return self.skip('LLVM opts will optimize out parent_func')

      Settings.LINKABLE = 1

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
      Settings.BUILD_AS_SHARED_LIB = 1
      Settings.EXPORTED_FUNCTIONS = ['_func']
      Settings.EXPORTED_GLOBALS = ['_global']
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

      src = '''
        #include <stdio.h>
        #include <dlfcn.h>

        typedef void (*FUNCTYPE(int, void(*)()))();

        FUNCTYPE func;

        void parent_func() {
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
      Settings.BUILD_AS_SHARED_LIB = 0
      Settings.EXPORTED_FUNCTIONS = ['_main']
      Settings.EXPORTED_GLOBALS = []
      add_pre_run_and_checks = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createLazyFile('/', 'liblib.so', 'liblib.so', true, false);"
  )
  open(filename, 'w').write(src)
'''
      self.do_run(src, 'In func: 13*First calling main_fptr from lib.*Second calling lib_fptr from main.*parent_func called from child*parent_func called from child*Var: 42*',
                   output_nicerizer=lambda x: x.replace('\n', '*'),
                   post_build=add_pre_run_and_checks)

    def test_dlfcn_alias(self):
      Settings.LINKABLE = 1

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
      add_pre_run_and_checks = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createLazyFile('/', 'liblib.so', 'liblib.so', true, false);"
  )
  open(filename, 'w').write(src)
'''
      self.do_run(src, 'Parent global: 123.*Parent global: 456.*',
                  output_nicerizer=lambda x: x.replace('\n', '*'),
                  post_build=add_pre_run_and_checks,
                  extra_emscripten_args=['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/time.h,libc/langinfo.h'])
      Settings.INCLUDE_FULL_LIBRARY = 0

    def test_dlfcn_varargs(self):
      Settings.LINKABLE = 1

      if Building.LLVM_OPTS == 2: return self.skip('LLVM LTO will optimize things that prevent shared objects from working')
      if Settings.QUANTUM_SIZE == 1: return self.skip('FIXME: Add support for this')

      lib_src = r'''
        void print_ints(int n, ...);
        extern "C" void func() {
          print_ints(2, 13, 42);
        }
        '''
      dirname = self.get_dir()
      filename = os.path.join(dirname, 'liblib.cpp')
      Settings.BUILD_AS_SHARED_LIB = 1
      Settings.EXPORTED_FUNCTIONS = ['_func']
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so'))

      src = r'''
        #include <stdarg.h>
        #include <stdio.h>
        #include <dlfcn.h>

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
          fptr = (void (*)())dlsym(lib_handle, "func");
          fptr();

          return 0;
        }
        '''
      Settings.BUILD_AS_SHARED_LIB = 0
      Settings.EXPORTED_FUNCTIONS = ['_main']
      add_pre_run_and_checks = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createLazyFile('/', 'liblib.so', 'liblib.so', true, false);"
  )
  open(filename, 'w').write(src)
'''
      self.do_run(src, '100\n200\n13\n42\n',
                  post_build=add_pre_run_and_checks)

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
          return 0;
        }
        '''
      expected = '''
        0
        0
        0
        0
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
        '''

      self.do_run(src, re.sub(r'\n\s+', '\n', expected))
      self.do_run(src.replace('strtod', 'strtold'), re.sub(r'\n\s+', '\n', expected)) # XXX add real support for long double

    def test_strtok(self):
      src = r'''
        #include<stdio.h>
        #include<string.h>

        int main() {
          char test[80], blah[80];
          char *sep = "\\/:;=-";
          char *word, *phrase, *brkt, *brkb;

          strcpy(test, "This;is.a:test:of=the/string\\tokenizer-function.");

          for (word = strtok_r(test, sep, &brkt); word; word = strtok_r(NULL, sep, &brkt)) {
            strcpy(blah, "blah:blat:blab:blag");
            for (phrase = strtok_r(blah, sep, &brkb); phrase; phrase = strtok_r(NULL, sep, &brkb)) {
              printf("at %s:%s\n", word, phrase);
            }
          }
          return 1;
        }
      '''

      expected = '''at This:blah
at This:blat
at This:blab
at This:blag
at is.a:blah
at is.a:blat
at is.a:blab
at is.a:blag
at test:blah
at test:blat
at test:blab
at test:blag
at of:blah
at of:blat
at of:blab
at of:blag
at the:blah
at the:blat
at the:blab
at the:blag
at string:blah
at string:blat
at string:blab
at string:blag
at tokenizer:blah
at tokenizer:blat
at tokenizer:blab
at tokenizer:blag
at function.:blah
at function.:blat
at function.:blab
at function.:blag
'''
      self.do_run(src, expected)

    def test_parseInt(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('i64 mode 1 requires ta2')
      if Settings.QUANTUM_SIZE == 1: return self.skip('Q1 and I64_1 do not mix well yet')
      src = open(path_from_root('tests', 'parseInt', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'parseInt', 'output.txt'), 'r').read()
      self.do_run(src, expected)
      
    def test_transtrcase(self):
      src = '''
        #include <stdio.h>
        #include <string.h>
        int main()  {
          char szToupr[] = "hello, ";
          char szTolwr[] = "EMSCRIPTEN";
          strupr(szToupr);
          strlwr(szTolwr);
          printf(szToupr);
          printf(szTolwr);
          return 0;
        }
        '''
      self.do_run(src, 'HELLO, emscripten')

    def test_printf(self):
      if Settings.USE_TYPED_ARRAYS != 2: return self.skip('i64 mode 1 requires ta2')
      self.banned_js_engines = [NODE_JS, V8_ENGINE] # SpiderMonkey and V8 do different things to float64 typed arrays, un-NaNing, etc.
      src = open(path_from_root('tests', 'printf', 'test.c'), 'r').read()
      expected = [open(path_from_root('tests', 'printf', 'output.txt'), 'r').read(),
                  open(path_from_root('tests', 'printf', 'output_i64_1.txt'), 'r').read()]
      self.do_run(src, expected)

    def test_printf_types(self):
      src = r'''
        #include <stdio.h>

        int main() {
          char c = '1';
          short s = 2;
          int i = 3;
          long long l = 4;
          float f = 5.5;
          double d = 6.6;

          printf("%c,%hd,%d,%lld,%.1f,%.1llf\n", c, s, i, l, f, d);

          return 0;
        }
        '''
      self.do_run(src, '1,2,3,4,5.5,6.6\n')

    def test_vprintf(self):
      src = r'''
        #include <stdio.h>
        #include <stdarg.h>

        void print(char* format, ...) {
          va_list args;
          va_start (args, format);
          vprintf (format, args);
          va_end (args);
        }

        int main () {
           print("Call with %d variable argument.\n", 1);
           print("Call with %d variable %s.\n", 2, "arguments");

           return 0;
        }
        '''
      expected = '''
        Call with 1 variable argument.
        Call with 2 variable arguments.
        '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected))

    def test_atoX(self):
      if self.emcc_args is None: return self.skip('requires ta2')

      src = r'''
        #include <stdio.h>
        #include <stdlib.h>

        int main () {
          printf("%d*", atoi(""));
          printf("%d*", atoi("a"));
          printf("%d*", atoi(" b"));
          printf("%d*", atoi(" c "));
          printf("%d*", atoi("6"));
          printf("%d*", atoi(" 5"));
          printf("%d*", atoi("4 "));
          printf("%d*", atoi("3 6"));
          printf("%d*", atoi(" 3 7"));
          printf("%d*", atoi("9 d"));
          printf("%d\n", atoi(" 8 e"));
          printf("%d*", atol(""));
          printf("%d*", atol("a"));
          printf("%d*", atol(" b"));
          printf("%d*", atol(" c "));
          printf("%d*", atol("6"));
          printf("%d*", atol(" 5"));
          printf("%d*", atol("4 "));
          printf("%d*", atol("3 6"));
          printf("%d*", atol(" 3 7"));
          printf("%d*", atol("9 d"));
          printf("%d\n", atol(" 8 e"));
          printf("%lld*", atoll("6294967296"));
          printf("%lld*", atoll(""));
          printf("%lld*", atoll("a"));
          printf("%lld*", atoll(" b"));
          printf("%lld*", atoll(" c "));
          printf("%lld*", atoll("6"));
          printf("%lld*", atoll(" 5"));
          printf("%lld*", atoll("4 "));
          printf("%lld*", atoll("3 6"));
          printf("%lld*", atoll(" 3 7"));
          printf("%lld*", atoll("9 d"));
          printf("%lld\n", atoll(" 8 e"));
          return 0;
        }
        '''
      self.do_run(src, '0*0*0*0*6*5*4*3*3*9*8\n0*0*0*0*6*5*4*3*3*9*8\n6294967296*0*0*0*0*6*5*4*3*3*9*8\n')

    def test_strstr(self):
      src = r'''
        #include <stdio.h>
        #include <string.h>

        int main()
        {
          printf("%d\n", !!strstr("\\n", "\\n"));
          printf("%d\n", !!strstr("cheezy", "ez"));
          printf("%d\n", !!strstr("cheeezy", "ez"));
          printf("%d\n", !!strstr("cheeeeeeeeeezy", "ez"));
          printf("%d\n", !!strstr("cheeeeeeeeee1zy", "ez"));
          printf("%d\n", !!strstr("che1ezy", "ez"));
          printf("%d\n", !!strstr("che1ezy", "che"));
          printf("%d\n", !!strstr("ce1ezy", "che"));
          printf("%d\n", !!strstr("ce1ezy", "ezy"));
          printf("%d\n", !!strstr("ce1ezyt", "ezy"));
          printf("%d\n", !!strstr("ce1ez1y", "ezy"));
          printf("%d\n", !!strstr("cheezy", "a"));
          printf("%d\n", !!strstr("cheezy", "b"));
          printf("%d\n", !!strstr("cheezy", "c"));
          printf("%d\n", !!strstr("cheezy", "d"));
          printf("%d\n", !!strstr("cheezy", "g"));
          printf("%d\n", !!strstr("cheezy", "h"));
          printf("%d\n", !!strstr("cheezy", "i"));
          printf("%d\n", !!strstr("cheezy", "e"));
          printf("%d\n", !!strstr("cheezy", "x"));
          printf("%d\n", !!strstr("cheezy", "y"));
          printf("%d\n", !!strstr("cheezy", "z"));
          printf("%d\n", !!strstr("cheezy", "_"));

          const char *str = "a big string";
          printf("%d\n", strstr(str, "big") - str);
          return 0;
        }
      '''
      self.do_run(src, '''1
1
1
1
0
1
1
0
1
1
0
0
0
1
0
0
1
0
1
0
1
1
0
2
''')

    def test_sscanf(self):
      src = r'''
        #include <stdio.h>
        #include <string.h>
        #include <stdlib.h>

        int main () {
          #define CHECK(str) \
          { \
            char name[1000]; \
            memset(name, 0, 1000); \
            int prio = 99; \
            sscanf(str, "%s %d", name, &prio); \
            printf("%s : %d\n", name, prio); \
          }
          CHECK("en-us 2");
          CHECK("en-r");
          CHECK("en 3");

          printf("%f, %f\n", atof("1.234567"), atof("cheez"));

          float n = -1;
          sscanf(" 2.8208", "%f", &n);
          printf("%.4f\n", n);

          float a = -1;
          sscanf("-3.03", "%f", &a);
          printf("%.4f\n", a);

          char buffy[100];
          sscanf("cheez some thing moar 123\nyet more\n", "cheez %s", buffy);
          printf("|%s|\n", buffy);
          sscanf("cheez something\nmoar 123\nyet more\n", "cheez %s", buffy);
          printf("|%s|\n", buffy);
          sscanf("cheez somethingmoar\tyet more\n", "cheez %s", buffy);
          printf("|%s|\n", buffy);

          int numverts = -1;
          printf("%d\n", sscanf("	numverts 1499\n", " numverts %d", &numverts)); // white space is the same, even if tab vs space
          printf("%d\n", numverts);

          int index;
          float u, v;
          short start, count;
          printf("%d\n", sscanf("	vert 87 ( 0.481565 0.059481 ) 0 1\n", " vert %d ( %f %f ) %hu %hu", &index, &u, &v, &start, &count));
          printf("%d,%.6f,%.6f,%hu,%hu\n", index, u, v, start, count);

          int neg, neg2, neg3 = 0;
          printf("%d\n", sscanf("-123 -765 -34-6", "%d %u %d", &neg, &neg2, &neg3));
          printf("%d,%u,%d\n", neg, neg2, neg3);

          return 0;
        }
        '''
      self.do_run(src, 'en-us : 2\nen-r : 99\nen : 3\n1.234567, 0.000000\n2.8208\n-3.0300\n|some|\n|something|\n|somethingmoar|\n' +
                       '1\n1499\n' +
                       '5\n87,0.481565,0.059481,0,1\n' +
                       '3\n-123,4294966531,-34\n')

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

    def test_langinfo(self):
      src = open(path_from_root('tests', 'langinfo', 'test.c'), 'r').read()
      expected = open(path_from_root('tests', 'langinfo', 'output.txt'), 'r').read()
      self.do_run(src, expected, extra_emscripten_args=['-H', 'libc/langinfo.h'])

    def test_files(self):
      if self.emcc_args is not None and '-O2' in self.emcc_args:
        self.emcc_args += ['--closure', '1'] # Use closure here, to test we don't break FS stuff

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
      self.do_run(src, 'size: 7\ndata: 100,-56,50,25,10,77,123\nloop: 100 -56 50 25 10 77 123 \ninput:hi there!\ntexto\ntexte\n$\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.\nfscanfed: 10 - hello\n',
                   post_build=post, extra_emscripten_args=['-H', 'libc/fcntl.h'])

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
      self.do_run(src, 'isatty? 0,0,1\ngot: 35\ngot: 45\ngot: 25\ngot: 15\n', post_build=post)

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

    def test_folders(self):
      add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    \'\'\'
      FS.createFolder('/', 'test', true, false);
      FS.createPath('/', 'test/hello/world/', true, false);
      FS.createPath('/test', 'goodbye/world/', true, false);
      FS.createPath('/test/goodbye', 'noentry', false, false);
      FS.createDataFile('/test', 'freeforall.ext', 'abc', true, true);
      FS.createDataFile('/test', 'restricted.ext', 'def', false, false);
    \'\'\'
  )
  open(filename, 'w').write(src)
'''
      src = r'''
        #include <stdio.h>
        #include <dirent.h>
        #include <errno.h>

        int main() {
          struct dirent *e;

          // Basic correct behaviour.
          DIR* d = opendir("/test");
          printf("--E: %d\n", errno);
          while ((e = readdir(d))) puts(e->d_name);
          printf("--E: %d\n", errno);

          // Empty folder; tell/seek.
          puts("****");
          d = opendir("/test/hello/world/");
          e = readdir(d);
          puts(e->d_name);
          int pos = telldir(d);
          e = readdir(d);
          puts(e->d_name);
          seekdir(d, pos);
          e = readdir(d);
          puts(e->d_name);

          // Errors.
          puts("****");
          printf("--E: %d\n", errno);
          d = opendir("/test/goodbye/noentry");
          printf("--E: %d, D: %d\n", errno, d);
          d = opendir("/i/dont/exist");
          printf("--E: %d, D: %d\n", errno, d);
          d = opendir("/test/freeforall.ext");
          printf("--E: %d, D: %d\n", errno, d);
          while ((e = readdir(d))) puts(e->d_name);
          printf("--E: %d\n", errno);

          return 0;
        }
        '''
      expected = '''
        --E: 0
        .
        ..
        hello
        goodbye
        freeforall.ext
        restricted.ext
        --E: 0
        ****
        .
        ..
        ..
        ****
        --E: 0
        --E: 13, D: 0
        --E: 2, D: 0
        --E: 20, D: 0
        --E: 9
      '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected), post_build=add_pre_run)

    def test_stat(self):
      add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    \'\'\'
      var f1 = FS.createFolder('/', 'test', true, true);
      var f2 = FS.createDataFile(f1, 'file', 'abcdef', true, true);
      var f3 = FS.createLink(f1, 'link', 'file', true, true);
      var f4 = FS.createDevice(f1, 'device', function(){}, function(){});
      f1.timestamp = f2.timestamp = f3.timestamp = f4.timestamp = new Date(1200000000000);
    \'\'\'
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'stat', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'stat', 'output.txt'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run, extra_emscripten_args=['-H', 'libc/fcntl.h'])

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
      add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    \'\'\'
      FS.createDataFile('/', 'test-file', 'abcdef', true, true);
      FS.createFolder('/', 'test-folder', true, true);
    \'\'\'
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'fcntl-open', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'fcntl-open', 'output.txt'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run, extra_emscripten_args=['-H', 'libc/fcntl.h'])

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
      FS.createDataFile('/', 'file', 'abcdef', true, true);
      FS.createDevice('/', 'device', function() {}, function() {});
    \'\'\'
  )
  open(filename, 'w').write(src)
'''
      src = r'''
        #include <stdio.h>
        #include <errno.h>
        #include <fcntl.h>
        #include <poll.h>

        int main() {
          struct pollfd multi[5];
          multi[0].fd = open("/file", O_RDONLY, 0777);
          multi[1].fd = open("/device", O_RDONLY, 0777);
          multi[2].fd = 123;
          multi[3].fd = open("/file", O_RDONLY, 0777);
          multi[4].fd = open("/file", O_RDONLY, 0777);
          multi[0].events = POLLIN | POLLOUT | POLLNVAL | POLLERR;
          multi[1].events = POLLIN | POLLOUT | POLLNVAL | POLLERR;
          multi[2].events = POLLIN | POLLOUT | POLLNVAL | POLLERR;
          multi[3].events = 0x00;
          multi[4].events = POLLOUT | POLLNVAL | POLLERR;

          printf("ret: %d\n", poll(multi, 5, 123));
          printf("errno: %d\n", errno);
          printf("multi[0].revents: %d\n", multi[0].revents == (POLLIN | POLLOUT));
          printf("multi[1].revents: %d\n", multi[1].revents == (POLLIN | POLLOUT));
          printf("multi[2].revents: %d\n", multi[2].revents == POLLNVAL);
          printf("multi[3].revents: %d\n", multi[3].revents == 0);
          printf("multi[4].revents: %d\n", multi[4].revents == POLLOUT);

          return 0;
        }
        '''
      expected = r'''
        ret: 4
        errno: 0
        multi[0].revents: 1
        multi[1].revents: 1
        multi[2].revents: 1
        multi[3].revents: 1
        multi[4].revents: 1
        '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected), post_build=add_pre_run, extra_emscripten_args=['-H', 'libc/fcntl.h,poll.h'])

    def test_statvfs(self):
      src = r'''
        #include <stdio.h>
        #include <errno.h>
        #include <sys/statvfs.h>

        int main() {
          struct statvfs s;

          printf("result: %d\n", statvfs("/test", &s));
          printf("errno: %d\n", errno);

          printf("f_bsize: %lu\n", s.f_bsize);
          printf("f_frsize: %lu\n", s.f_frsize);
          printf("f_blocks: %lu\n", s.f_blocks);
          printf("f_bfree: %lu\n", s.f_bfree);
          printf("f_bavail: %lu\n", s.f_bavail);
          printf("f_files: %lu\n", s.f_files);
          printf("f_ffree: %lu\n", s.f_ffree);
          printf("f_favail: %lu\n", s.f_favail);
          printf("f_fsid: %lu\n", s.f_fsid);
          printf("f_flag: %lu\n", s.f_flag);
          printf("f_namemax: %lu\n", s.f_namemax);

          return 0;
        }
        '''
      expected = r'''
        result: 0
        errno: 0
        f_bsize: 4096
        f_frsize: 4096
        f_blocks: 1000000
        f_bfree: 500000
        f_bavail: 500000
        f_files: 10
        f_ffree: 1000000
        f_favail: 1000000
        f_fsid: 42
        f_flag: 2
        f_namemax: 255
        '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected))

    def test_libgen(self):
      src = r'''
        #include <stdio.h>
        #include <libgen.h>

        int main() {
          char p1[16] = "/usr/lib", p1x[16] = "/usr/lib";
          printf("%s -> ", p1);
          printf("%s : %s\n", dirname(p1x), basename(p1));

          char p2[16] = "/usr", p2x[16] = "/usr";
          printf("%s -> ", p2);
          printf("%s : %s\n", dirname(p2x), basename(p2));

          char p3[16] = "/usr/", p3x[16] = "/usr/";
          printf("%s -> ", p3);
          printf("%s : %s\n", dirname(p3x), basename(p3));

          char p4[16] = "/usr/lib///", p4x[16] = "/usr/lib///";
          printf("%s -> ", p4);
          printf("%s : %s\n", dirname(p4x), basename(p4));

          char p5[16] = "/", p5x[16] = "/";
          printf("%s -> ", p5);
          printf("%s : %s\n", dirname(p5x), basename(p5));

          char p6[16] = "///", p6x[16] = "///";
          printf("%s -> ", p6);
          printf("%s : %s\n", dirname(p6x), basename(p6));

          char p7[16] = "/usr/../lib/..", p7x[16] = "/usr/../lib/..";
          printf("%s -> ", p7);
          printf("%s : %s\n", dirname(p7x), basename(p7));

          char p8[16] = "", p8x[16] = "";
          printf("(empty) -> %s : %s\n", dirname(p8x), basename(p8));

          printf("(null) -> %s : %s\n", dirname(0), basename(0));

          return 0;
        }
        '''
      expected = '''
        /usr/lib -> /usr : lib
        /usr -> / : usr
        /usr/ -> / : usr
        /usr/lib/// -> /usr : lib
        / -> / : /
        /// -> / : /
        /usr/../lib/.. -> /usr/../lib : ..
        (empty) -> . : .
        (null) -> . : .
      '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected))

    def test_utime(self):
      add_pre_run_and_checks = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    \'\'\'
      var TEST_F1 = FS.createFolder('/', 'writeable', true, true);
      var TEST_F2 = FS.createFolder('/', 'unwriteable', true, false);
    \'\'\'
  ).replace(
    '// {{POST_RUN_ADDITIONS}}',
    \'\'\'
      Module.print('first changed: ' + (TEST_F1.timestamp == 1200000000000));
      Module.print('second changed: ' + (TEST_F2.timestamp == 1200000000000));
    \'\'\'
  )
  open(filename, 'w').write(src)
'''
      src = r'''
        #include <stdio.h>
        #include <errno.h>
        #include <utime.h>

        int main() {
          struct utimbuf t = {1000000000, 1200000000};
          char* writeable = "/writeable";
          char* unwriteable = "/unwriteable";

          utime(writeable, &t);
          printf("writeable errno: %d\n", errno);

          utime(unwriteable, &t);
          printf("unwriteable errno: %d\n", errno);

          return 0;
        }
        '''
      expected = '''
        writeable errno: 0
        unwriteable errno: 1
        first changed: true
        second changed: false
      '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected), post_build=add_pre_run_and_checks)

    def test_utf(self):
      self.banned_js_engines = [SPIDERMONKEY_ENGINE] # only node handles utf well
      src = r'''
        #include <stdio.h>
        #include <emscripten.h>

        int main() {
          char *c = "μ†ℱ ╋ℯ╳╋";
          printf("%d %d %d %d %s\n", c[0]&0xff, c[1]&0xff, c[2]&0xff, c[3]&0xff, c);
          emscripten_run_script("cheez = Module._malloc(100);"
                                "Module.writeStringToMemory(\"μ†ℱ ╋ℯ╳╋\", cheez);"
                                "Module.print([Pointer_stringify(cheez), Module.getValue(cheez, 'i8')&0xff, Module.getValue(cheez+1, 'i8')&0xff, Module.getValue(cheez+2, 'i8')&0xff, Module.getValue(cheez+3, 'i8')&0xff, ]);");
        }
      '''
      self.do_run(src, '206 188 226 128 μ†ℱ ╋ℯ╳╋\nμ†ℱ ╋ℯ╳╋,206,188,226,128\n');

    def test_direct_string_constant_usage(self):
      if self.emcc_args is None: return self.skip('requires libcxx')

      src = '''
        #include <iostream>
        template<int i>
        void printText( const char (&text)[ i ] )
        {
           std::cout << text;
        }
        int main()
        {
          printText( "some string constant" );
          return 0;
        }
      '''
      self.do_run(src, "some string constant")
      
    def test_istream(self):
      if self.emcc_args is None: return self.skip('requires libcxx')

      src = '''
        #include <string>
        #include <sstream>
        #include <iostream>
        
        int main()
        {
            std::string mystring("1 2 3");
            std::istringstream is(mystring);
            int one, two, three;
            
            is >> one >> two >> three;
            
            printf( "%i %i %i", one, two, three );
        }
      '''
      for linkable in [0, 1]:
        Settings.LINKABLE = linkable # regression check for issue #273
        self.do_run(src, "1 2 3")

    def test_readdir(self):
      add_pre_run = '''
def process(filename):
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    "FS.createFolder('', 'test', true, true);\\nFS.createLazyFile( 'test', 'some_file', 'http://localhost/some_file', true, false);\\nFS.createFolder('test', 'some_directory', true, true);"
  )
  open(filename, 'w').write(src)
'''

      src = '''
        #include <dirent.h>
        #include <stdio.h>
        
        int main()
        {
            DIR * dir;
            dirent * entity;
        
            dir = opendir( "test" );
        
            while( ( entity = readdir( dir ) ) )
            {
                printf( "%s is a %s\\n", entity->d_name, entity->d_type & DT_DIR ? "directory" : "file" );
            }
        
            return 0;
        }

      '''
      self.do_run(src, ". is a directory\n.. is a directory\nsome_file is a file\nsome_directory is a directory", post_build=add_pre_run)

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

    def test_unistd_access(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'access.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'access.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'access.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_curdir(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'curdir.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'curdir.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'curdir.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_close(self):
      src = open(path_from_root('tests', 'unistd', 'close.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'close.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_confstr(self):
      src = open(path_from_root('tests', 'unistd', 'confstr.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'confstr.out'), 'r').read()
      self.do_run(src, expected, extra_emscripten_args=['-H', 'libc/unistd.h'])

    def test_unistd_ttyname(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'ttyname.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'ttyname.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'ttyname.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_dup(self):
      src = open(path_from_root('tests', 'unistd', 'dup.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'dup.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_pathconf(self):
      src = open(path_from_root('tests', 'unistd', 'pathconf.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'pathconf.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_truncate(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'truncate.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'truncate.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'truncate.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_swab(self):
      src = open(path_from_root('tests', 'unistd', 'swab.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'swab.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_isatty(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'isatty.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'isatty.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'isatty.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_sysconf(self):
      src = open(path_from_root('tests', 'unistd', 'sysconf.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'sysconf.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_login(self):
      src = open(path_from_root('tests', 'unistd', 'login.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'login.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_unlink(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'unlink.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'unlink.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'unlink.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_links(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'links.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'links.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'links.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_sleep(self):
      src = open(path_from_root('tests', 'unistd', 'sleep.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'sleep.out'), 'r').read()
      self.do_run(src, expected)

    def test_unistd_io(self):
      add_pre_run = '''
def process(filename):
  import tools.shared as shared
  src = open(filename, 'r').read().replace(
    '// {{PRE_RUN_ADDITIONS}}',
    open(shared.path_from_root('tests', 'unistd', 'io.js'), 'r').read()
  )
  open(filename, 'w').write(src)
'''
      src = open(path_from_root('tests', 'unistd', 'io.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'io.out'), 'r').read()
      self.do_run(src, expected, post_build=add_pre_run)

    def test_unistd_misc(self):
      src = open(path_from_root('tests', 'unistd', 'misc.c'), 'r').read()
      expected = open(path_from_root('tests', 'unistd', 'misc.out'), 'r').read()
      self.do_run(src, expected)

    def test_uname(self):
      src = r'''
        #include <stdio.h>
        #include <sys/utsname.h>

        int main() {
          struct utsname u;
          printf("ret: %d\n", uname(&u));
          printf("sysname: %s\n", u.sysname);
          printf("nodename: %s\n", u.nodename);
          printf("release: %s\n", u.release);
          printf("version: %s\n", u.version);
          printf("machine: %s\n", u.machine);
          printf("invalid: %d\n", uname(0));
          return 0;
        }
        '''
      expected = '''
        ret: 0
        sysname: Emscripten
        nodename: emscripten
        release: 1.0
        version: #1
        machine: x86-JS
      '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected))

    def test_env(self):
      src = open(path_from_root('tests', 'env', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'env', 'output.txt'), 'r').read()
      self.do_run(src, expected)

    def test_systypes(self):
      src = open(path_from_root('tests', 'systypes', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'systypes', 'output.txt'), 'r').read()
      self.do_run(src, expected)

    def test_getloadavg(self):
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>

        int main() {
          double load[5] = {42.13, 42.13, 42.13, 42.13, 42.13};
          printf("ret: %d\n", getloadavg(load, 5));
          printf("load[0]: %.3lf\n", load[0]);
          printf("load[1]: %.3lf\n", load[1]);
          printf("load[2]: %.3lf\n", load[2]);
          printf("load[3]: %.3lf\n", load[3]);
          printf("load[4]: %.3lf\n", load[4]);
          return 0;
        }
        '''
      expected = '''
        ret: 3
        load[0]: 0.100
        load[1]: 0.100
        load[2]: 0.100
        load[3]: 42.130
        load[4]: 42.130
      '''
      self.do_run(src, re.sub('(^|\n)\s+', '\\1', expected))

    def test_inet(self):
      src = r'''
        #include <stdio.h>
        #include <arpa/inet.h>

        int main() {
          printf("*%x,%x,%x,%x*\n", htonl(0x12345678), htons(0xabcd), ntohl(0x43211234), ntohs(0xbeaf));
          return 0;
        }
      '''
      self.do_run(src, '*78563412,cdab,34122143,afbe*')

    def test_ctype(self):
      # The bit fiddling done by the macros using __ctype_b_loc requires this.
      Settings.CORRECT_SIGNS = 1
      src = open(path_from_root('tests', 'ctype', 'src.c'), 'r').read()
      expected = open(path_from_root('tests', 'ctype', 'output.txt'), 'r').read()
      self.do_run(src, expected)
      CORRECT_SIGNS = 0

    def test_atomic(self):
      src = '''
        #include <stdio.h>
        int main() {
          int x = 10;
          int y = __sync_add_and_fetch(&x, 5);
          printf("*%d,%d*\\n", x, y);
          x = 10;
          y = __sync_fetch_and_add(&x, 5);
          printf("*%d,%d*\\n", x, y);
          x = 10;
          y = __sync_lock_test_and_set(&x, 6);
          printf("*%d,%d*\\n", x, y);
          x = 10;
          y = __sync_bool_compare_and_swap(&x, 9, 7);
          printf("*%d,%d*\\n", x, y);
          y = __sync_bool_compare_and_swap(&x, 10, 7);
          printf("*%d,%d*\\n", x, y);
          return 0;
        }
      '''

      self.do_run(src, '*15,15*\n*15,10*\n*6,10*\n*10,0*\n*7,1*')

    def test_phiundef(self):
      src = r'''
#include <stdlib.h>
#include <stdio.h>

static int state;

struct my_struct {
  union {
    struct {
      unsigned char a;
      unsigned char b;
    } c;
    unsigned int d;
  } e;
  unsigned int f;
};

int main(int argc, char **argv) {
    struct my_struct r;

    state = 0;

    for (int i=0;i<argc+10;i++)
    {
        if (state % 2 == 0)
            r.e.c.a = 3;
        else
            printf("%d\n", r.e.c.a);
        state++;
    }
    return 0;
}
      '''

      self.do_run(src, '3\n3\n3\n3\n3\n')

    # libc++ tests

    def test_iostream(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip("we don't support libcxx in q1")

      if self.emcc_args is None:
        if Building.LLVM_OPTS: return self.skip('optimizing bitcode before emcc can confuse libcxx inclusion')
        self.emcc_args = [] # libc++ auto-inclusion is only done if we use emcc
        Settings.SAFE_HEAP = 0 # Some spurious warnings from libc++ internals

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
      src = '''
        #include <vector>
        #include <stdio.h>

        struct S {
            int a;
            float b;
        };

        void foo(int a, float b)
        {
          printf("%d:%.2f\\n", a, b);
        }

        int main ( int argc, char *argv[] )
        {
          std::vector<S> ar;  
          S s;

          s.a = 789;
          s.b = 123.456f;
          ar.push_back(s);

          s.a = 0;
          s.b = 100.1f;
          ar.push_back(s);

          foo(ar[0].a, ar[0].b);
          foo(ar[1].a, ar[1].b);
        }
      '''

      self.do_run(src, '789:123.46\n0:100.1')

    ### 'Medium' tests

    def test_fannkuch(self):
        results = [ (1,0), (2,1), (3,2), (4,4), (5,7), (6,10), (7, 16), (8,22) ]
        for i, j in results:
          src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read()
          self.do_run(src, 'Pfannkuchen(%d) = %d.' % (i,j), [str(i)], no_build=i>1)

    def test_raytrace(self):
        if Settings.USE_TYPED_ARRAYS == 2: return self.skip('Relies on double value rounding, extremely sensitive')

        src = open(path_from_root('tests', 'raytrace.cpp'), 'r').read().replace('double', 'float')
        output = open(path_from_root('tests', 'raytrace.ppm'), 'r').read()
        self.do_run(src, output, ['3', '16'])#, build_ll_hook=self.do_autodebug)

    def test_fasta(self):
        results = [ (1,'''GG*ctt**tgagc*'''), (20,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tacgtgtagcctagtgtttgtgttgcgttatagtctatttgtggacacagtatggtcaaa**tgacgtcttttgatctgacggcgttaacaaagatactctg*'''),
(50,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA*TCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACAT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tactDtDagcctatttSVHtHttKtgtHMaSattgWaHKHttttagacatWatgtRgaaa**NtactMcSMtYtcMgRtacttctWBacgaa**agatactctgggcaacacacatacttctctcatgttgtttcttcggacctttcataacct**ttcctggcacatggttagctgcacatcacaggattgtaagggtctagtggttcagtgagc**ggaatatcattcgtcggtggtgttaatctatctcggtgtagcttataaatgcatccgtaa**gaatattatgtttatttgtcggtacgttcatggtagtggtgtcgccgatttagacgtaaa**ggcatgtatg*''') ]
        for i, j in results:
          src = open(path_from_root('tests', 'fasta.cpp'), 'r').read()
          self.do_run(src, j, [str(i)], lambda x: x.replace('\n', '*'), no_build=i>1)

    def test_dlmalloc(self):
      if self.emcc_args is None: self.emcc_args = [] # dlmalloc auto-inclusion is only done if we use emcc

      Settings.CORRECT_SIGNS = 2
      Settings.CORRECT_SIGNS_LINES = ['src.cpp:' + str(i+4) for i in [4816, 4191, 4246, 4199, 4205, 4235, 4227]]
      Settings.TOTAL_MEMORY = 100*1024*1024 # needed with typed arrays

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
        output = Popen(['python', EMCC, path_from_root('tests', 'dlmalloc_test.c'),
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
          return 1;
        }
        ''', 'hello world');
        
    def test_static_variable(self):
      if self.emcc_args is None: Settings.SAFE_HEAP = 0 # LLVM mixes i64 and i8 in the guard check
      src = '''
        #include <stdio.h>

        struct DATA
        {
            int value;
    
            DATA()
            {
                value = 0;
            }
        };

        DATA & GetData()
        {
            static DATA data;
    
            return data;
        }

        int main()
        {
            GetData().value = 10;
            printf( "value:%i", GetData().value );
        }
      '''
      self.do_run(src, 'value:10')

    def test_fakestat(self):
      src = r'''
        #include <stdio.h>
        struct stat { int x, y; };
        int main() {
          stat s;
          s.x = 10;
          s.y = 22;
          printf("*%d,%d*\n", s.x, s.y);
        }
      '''
      self.do_run(src, '*10,22*')
      
    def test_mmap(self):
      src = '''
        #include <stdio.h>
        #include <sys/mman.h>
        #include <assert.h>
        
        int main(int argc, char *argv[]) {
            const int NUM_BYTES = 8 * 1024 * 1024;
            const int NUM_INTS = NUM_BYTES / sizeof(int);
        
            int* map = (int*)mmap(0, NUM_BYTES, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANON, -1, 0);
            assert(map != MAP_FAILED);
        
            int i;
        
            for (i = 0; i < NUM_INTS; i++) {
                map[i] = i;
            }
        
            for (i = 0; i < NUM_INTS; i++) {
                assert(map[i] == i);
            }
        
            assert(munmap(map, NUM_BYTES) == 0);
            
            printf("hello,world");
            return 0;
        }
      '''
      self.do_run(src, 'hello,world')
      self.do_run(src, 'hello,world', force_c=True)

    def test_cubescript(self):
      if self.emcc_args is not None and '-O2' in self.emcc_args:
        self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

      Building.COMPILER_TEST_OPTS = [] # remove -g, so we have one test without it by default
      if self.emcc_args is None: Settings.SAFE_HEAP = 0 # Has some actual loads of unwritten-to places, in the C++ code...

      # Overflows happen in hash loop
      Settings.CORRECT_OVERFLOWS = 1
      Settings.CHECK_OVERFLOWS = 0

      if Settings.USE_TYPED_ARRAYS == 2:
        Settings.CORRECT_SIGNS = 1

      self.do_run(path_from_root('tests', 'cubescript'), '*\nTemp is 33\n9\n5\nhello, everyone\n*', main_file='command.cpp')

    def test_gcc_unmangler(self):
      self.do_run(path_from_root('third_party'), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'], main_file='gcc_demangler.c')

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
      if self.emcc_args is None and Building.LLVM_OPTS: return self.skip('llvm 3.1 and safe llvm opts break lua')

      try:
        os.environ['EMCC_LEAVE_INPUTS_RAW'] = '1'

        if Settings.QUANTUM_SIZE == 1: return self.skip('TODO: make this work')

        # Overflows in luaS_newlstr hash loop
        if self.emcc_args is None: Settings.SAFE_HEAP = 0 # Has various warnings, with copied HEAP_HISTORY values (fixed if we copy 'null' as the type)
        Settings.CORRECT_OVERFLOWS = 1
        Settings.CHECK_OVERFLOWS = 0
        Settings.CORRECT_SIGNS = 1 # Not sure why, but needed
        Settings.INIT_STACK = 1 # TODO: Investigate why this is necessary

        self.do_ll_run(path_from_root('tests', 'lua', 'lua.ll'),
                        'hello lua world!\n17\n1\n2\n3\n4\n7',
                        args=['-e', '''print("hello lua world!");print(17);for x = 1,4 do print(x) end;print(10-3)'''],
                        output_nicerizer=lambda string: string.replace('\n\n', '\n').replace('\n\n', '\n'),
                        extra_emscripten_args=['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/langinfo.h,libc/time.h'])
      finally:
        del os.environ['EMCC_LEAVE_INPUTS_RAW']

    def get_freetype(self):
      Settings.INIT_STACK = 1 # TODO: Investigate why this is necessary

      return self.get_library('freetype', os.path.join('objs', '.libs', 'libfreetype.a'))

    def test_freetype(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip('TODO: Figure out and try to fix')

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
      self.do_run(open(path_from_root('tests', 'freetype', 'main.c'), 'r').read(),
                   open(path_from_root('tests', 'freetype', 'ref.txt'), 'r').read(),
                   ['font.ttf', 'test!', '150', '120', '25'],
                   libraries=self.get_freetype(),
                   includes=[path_from_root('tests', 'freetype', 'include')],
                   post_build=post)
                   #build_ll_hook=self.do_autodebug)

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
      if Settings.QUANTUM_SIZE == 1: return self.skip('TODO FIXME')

      pgo_data = read_pgo_data(path_from_root('tests', 'sqlite', 'sqlite-autooptimize.fails.txt'))

      Settings.CORRECT_SIGNS = 1 # XXX: in default, we fail with 2 here, even though the pgo_data should be correct (and works in s_0_0). Investigate this.
      Settings.CORRECT_SIGNS_LINES = pgo_data['signs_lines']
      Settings.CORRECT_OVERFLOWS = 0
      Settings.CORRECT_ROUNDINGS = 0
      if self.emcc_args is None: Settings.SAFE_HEAP = 0 # uses time.h to set random bytes, other stuff
      Settings.DISABLE_EXCEPTION_CATCHING = 1
      Settings.FAST_MEMORY = 4*1024*1024
      Settings.EXPORTED_FUNCTIONS = ['_main', '_sqlite3_open', '_sqlite3_close', '_sqlite3_exec', '_sqlite3_free', '_callback'];

      self.do_run(r'''
                        #define SQLITE_DISABLE_LFS
                        #define LONGDOUBLE_TYPE double
                        #define SQLITE_INT64_TYPE long long int
                        #define SQLITE_THREADSAFE 0
                   ''' + open(path_from_root('tests', 'sqlite', 'sqlite3.c'), 'r').read() +
                         open(path_from_root('tests', 'sqlite', 'benchmark.c'), 'r').read(),
                   open(path_from_root('tests', 'sqlite', 'benchmark.txt'), 'r').read(),
                   includes=[path_from_root('tests', 'sqlite')],
                   force_c=True,
                   js_engines=[SPIDERMONKEY_ENGINE]) # V8 is slow

    def test_zlib(self):
      if self.emcc_args is not None and '-O2' in self.emcc_args:
        self.emcc_args += ['--closure', '1'] # Use closure here for some additional coverage

      Settings.CORRECT_SIGNS = 1

      self.do_run(open(path_from_root('tests', 'zlib', 'example.c'), 'r').read(),
                   open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                   libraries=self.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a']),
                   includes=[path_from_root('tests', 'zlib')],
                   force_c=True)

    def test_the_bullet(self): # Called thus so it runs late in the alphabetical cycle... it is long
      if Building.LLVM_OPTS and self.emcc_args is None: Settings.SAFE_HEAP = 0 # Optimizations make it so we do not have debug info on the line we need to ignore

      # Note: this is also a good test of per-file and per-line changes (since we have multiple files, and correct specific lines)
      if Settings.SAFE_HEAP:
        # Ignore bitfield warnings
        Settings.SAFE_HEAP = 3
        Settings.SAFE_HEAP_LINES = ['btVoronoiSimplexSolver.h:40', 'btVoronoiSimplexSolver.h:41',
                                    'btVoronoiSimplexSolver.h:42', 'btVoronoiSimplexSolver.h:43']

      self.do_run(open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'), 'r').read(),
                   [open(path_from_root('tests', 'bullet', 'output.txt'), 'r').read(), # different roundings
                    open(path_from_root('tests', 'bullet', 'output2.txt'), 'r').read()],
                   libraries=self.get_library('bullet', [os.path.join('src', '.libs', 'libBulletDynamics.a'),
                                                          os.path.join('src', '.libs', 'libBulletCollision.a'),
                                                          os.path.join('src', '.libs', 'libLinearMath.a')],
                                               configure_args=['--disable-demos','--disable-dependency-tracking']),
                   includes=[path_from_root('tests', 'bullet', 'src')],
                   js_engines=[SPIDERMONKEY_ENGINE]) # V8 issue 1407

    def test_poppler(self):
      if self.emcc_args is None: return self.skip('very slow, we only do this in emcc runs')

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
      run();
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
                              os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/__/common/getopt.c.o'.split('/')),
                              os.path.join('bin', self.get_shared_library_name('libopenjpeg.so.1.4.0'))],
                             configure=['cmake', '.'],
                             #configure_args=['--enable-tiff=no', '--enable-jp3d=no', '--enable-png=no'],
                             make_args=[]) # no -j 2, since parallel builds can fail

      # We use doubles in JS, so we get slightly different values than native code. So we
      # check our output by comparing the average pixel difference
      def image_compare(output):
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

    def test_python(self):
      if Settings.QUANTUM_SIZE == 1: return self.skip('TODO: make this work')

      # Overflows in string_hash
      Settings.CORRECT_OVERFLOWS = 1
      Settings.CHECK_OVERFLOWS = 0
      if self.emcc_args is None: Settings.SAFE_HEAP = 0 # Has bitfields which are false positives. Also the PyFloat_Init tries to detect endianness.
      Settings.CORRECT_SIGNS = 1 # Not sure why, but needed
      Settings.EXPORTED_FUNCTIONS = ['_main', '_PyRun_SimpleStringFlags'] # for the demo

      self.do_ll_run(path_from_root('tests', 'python', 'python.small.bc'),
                      'hello python world!\n[0, 2, 4, 6]\n5\n22\n5.470000',
                      args=['-S', '-c' '''print "hello python world!"; print [x*2 for x in range(4)]; t=2; print 10-3-t; print (lambda x: x*2)(11); print '%f' % 5.47'''])

    def test_lifetime(self):
      if self.emcc_args is None: return self.skip('test relies on emcc opts')

      try:
        os.environ['EMCC_LEAVE_INPUTS_RAW'] = '1'

        self.do_ll_run(path_from_root('tests', 'lifetime.ll'), 'hello, world!\n')
        if '-O1' in self.emcc_args or '-O2' in self.emcc_args:
          assert 'a18' not in open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read(), 'lifetime stuff and their vars must be culled'
        else:
          assert 'a18' in open(os.path.join(self.get_dir(), 'src.cpp.o.js')).read(), "without opts, it's there"

      finally:
        del os.environ['EMCC_LEAVE_INPUTS_RAW']

    # Test cases in separate files. Note that these files may contain invalid .ll!
    # They are only valid enough for us to read for test purposes, not for llvm-as
    # to process.
    def test_cases(self):
      if Building.LLVM_OPTS: return self.skip("Our code is not exactly 'normal' llvm assembly")

      try:
        os.environ['EMCC_LEAVE_INPUTS_RAW'] = '1'
        self.banned_js_engines = [NODE_JS] # node issue 1669, exception causes stdout not to be flushed
        Settings.CHECK_OVERFLOWS = 0

        for name in glob.glob(path_from_root('tests', 'cases', '*.ll')):
          shortname = name.replace('.ll', '')
          if '' not in shortname: continue
          if '_ta2' in shortname and not Settings.USE_TYPED_ARRAYS == 2:
            print self.skip('case "%s" only relevant for ta2' % shortname)
            continue
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

    # Autodebug the code
    def do_autodebug(self, filename):
      output = Popen(['python', AUTODEBUGGER, filename+'.o.ll', filename+'.o.ll.ll'], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
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
      output = Popen(['python', AUTODEBUGGER, filename+'.o.ll', filename+'.o.ll.ll'], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
      assert 'Success.' in output, output
      shutil.copyfile(filename + '.o.ll.ll', filename + '.o.ll')
      Building.llvm_as(filename)
      Building.llvm_dis(filename)

    def test_autodebug(self):
      if Building.LLVM_OPTS: return self.skip('LLVM opts mess us up')

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

    def test_profiling(self):
      src = '''
          #include <emscripten.h>
          #include <unistd.h>

          int main()
          {
            EMSCRIPTEN_PROFILE_INIT(3);
            EMSCRIPTEN_PROFILE_BEGIN(0);
            usleep(10 * 1000);
            EMSCRIPTEN_PROFILE_END(0);
            EMSCRIPTEN_PROFILE_BEGIN(1);
            usleep(50 * 1000);
            EMSCRIPTEN_PROFILE_END(1);
            EMSCRIPTEN_PROFILE_BEGIN(2);
            usleep(250 * 1000);
            EMSCRIPTEN_PROFILE_END(2);
            return 0;
          }
        '''

      post1 = '''
def process(filename):
  src = open(filename, 'a')
  src.write(\'\'\'
    Profiling.dump();
  \'\'\')
  src.close()
'''

      self.do_run(src, '''Profiling data:
Block 0: ''', post_build=post1)

    ### Integration tests

    def test_ccall(self):
      if self.emcc_args is not None and '-O2' in self.emcc_args:
        self.emcc_args += ['--closure', '1'] # Use closure here, to test we export things right

      src = r'''
        #include <stdio.h>

        // Optimizations might wipe out our functions without this
        #define KEEPALIVE __attribute__((used))

        extern "C" {
          int KEEPALIVE get_int() { return 5; }
          float KEEPALIVE get_float() { return 3.14; }
          char * KEEPALIVE get_string() { return "hello world"; }
          void KEEPALIVE print_int(int x) { printf("%d\n", x); }
          void KEEPALIVE print_float(float x) { printf("%.2f\n", x); }
          void KEEPALIVE print_string(char *x) { printf("%s\n", x); }
          int KEEPALIVE multi(int x, float y, int z, char *str) { if (x) puts(str); return (x+y)*z; }
          int * KEEPALIVE pointer(int *in) { printf("%d\n", *in); static int ret = 21; return &ret; }
        }

        int main(int argc, char **argv) {
          // keep them alive
          if (argc == 10) return get_int();
          if (argc == 11) return get_float();
          if (argc == 12) return get_string()[0];
          if (argc == 13) print_int(argv[0][0]);
          if (argc == 14) print_float(argv[0][0]);
          if (argc == 15) print_string(argv[0]);
          if (argc == 16) pointer((int*)argv[0]);
          if (argc % 17 == 12) return multi(argc, float(argc)/2, argc+1, argv[0]);
          return 0;
        }
      '''

      post = '''
def process(filename):
  src = \'\'\'
    var Module = {
      'postRun': function() {
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
      }
    };
  \'\'\' + open(filename, 'r').read()
  open(filename, 'w').write(src)
'''

      Settings.EXPORTED_FUNCTIONS = ['_get_int', '_get_float', '_get_string', '_print_int', '_print_float', '_print_string', '_multi', '_pointer', '_malloc']

      self.do_run(src, '*\nnumber,5\nnumber,3.14\nstring,hello world\n12\nundefined\n14.56\nundefined\ncheez\nundefined\narr-ay\nundefined\nmore\nnumber,10\n650\nnumber,21\n*\natr\n10\nbret\n53\n*\nstack is ok.\n', post_build=post)

    def test_scriptaclass(self):
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
  Popen(['python', DEMANGLER, filename], stdout=open(filename + '.tmp', 'w')).communicate()
  Popen(['python', NAMESPACER, filename, filename + '.tmp'], stdout=open(filename + '.tmp2', 'w')).communicate()
  src = open(filename, 'r').read().replace(
    '// {{MODULE_ADDITIONS}',
    'Module["_"] = ' + open(filename + '.tmp2', 'r').read().replace('var ModuleNames = ', '').rstrip() + ';\n\n' + script_src + '\n\n' +
      '// {{MODULE_ADDITIONS}'
  )
  open(filename, 'w').write(src)
'''
        # XXX disable due to possible v8 bug -- self.do_run(src, '*166*\n*ok*', post_build=post)

        if self.emcc_args is not None and '-O2' in self.emcc_args:
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
        output = Popen(['python', BINDINGS_GENERATOR, basename, header_filename], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
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

        post3 = '''
def process(filename):
  script_src_2 = \'\'\'
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
        \'\'\'
  src = open(filename, 'a')
  src.write(script_src_2 + '\\n')
  src.close()
'''

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
*virtualf2*
Parent:9
Child2:9
*js virtualf replacement*
*js virtualf replacement*
*js virtualf2 replacement*
*virtualf*
*virtualf*
*virtualf2*
*ok*
''', post_build=[post2, post3])

    def test_scriptaclass_2(self):
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
        output = Popen(['python', BINDINGS_GENERATOR, basename, header_filename], stdout=PIPE, stderr=self.stderr_redirect).communicate()[0]
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
        int main() {
          int *x = new int;
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
        void callFunc() {
          int *x = new int;
          *x = 20;
          float *y = (float*)x;
          printf("%f\\n", *y);
        }
      '''
      module_name = os.path.join(self.get_dir(), 'module.cpp')
      open(module_name, 'w').write(module)

      main = '''
        #include<stdio.h>
        extern void callFunc();
        int main() {
          callFunc();
          int *x = new int;
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

    def test_check_overflow(self):
      Settings.CHECK_OVERFLOWS = 1
      Settings.CORRECT_OVERFLOWS = 0

      src = '''
          #include<stdio.h>
          int main() {
            int t = 77;
            for (int i = 0; i < 30; i++) {
              //t = (t << 2) + t + 1; // This would have worked, since << forces into 32-bit int...
              t = t*5 + 1; // Python lookdict_string has ~the above line, which turns into this one with optimizations...
              printf("%d,%d\\n", t, t & 127);
            }
            return 0;
          }
      '''
      try:
        self.do_run(src, '*nothingatall*')
      except Exception, e:
        # This test *should* fail, by throwing this exception
        assert 'Too many corrections' in str(e), str(e)

    def test_debug(self):
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
        post = r'''
def process(filename):
  lines = open(filename, 'r').readlines()
  lines = filter(lambda line: '___assert_fail(' in line or '___assert_func(' in line, lines)
  found_line_num = any(('//@line 7 "' in line) for line in lines)
  found_filename = any(('src.cpp"\n' in line) for line in lines)
  assert found_line_num, 'Must have debug info with the line number'
  assert found_filename, 'Must have debug info with the filename'
'''
        self.do_run(src, '*nothingatall*', post_build=post)
      except Exception, e:
        # This test *should* fail
        assert 'Assertion failed' in str(e), str(e)

    def test_linespecific(self):
      if self.emcc_args: self.emcc_args += ['--llvm-opts', '0'] # llvm full opts make the expected failures here not happen

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
            t = t*5 + 1;
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
        self.do_run(src.replace('TYPE', 'unsigned int'), '*-3**2**-6**5*') # We fail, since no fast shortcut for 32-bit unsigneds

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
        self.do_run(src.replace('TYPE', 'unsigned int'), '*-3**2**-5**5*') # No such luck here

      # And reverse the check with = 2
      if Settings.USE_TYPED_ARRAYS != 2: # the errors here are very specific to non-i64 mode 1
        Settings.CORRECT_ROUNDINGS = 3
        Settings.CORRECT_ROUNDINGS_LINES = ["src.cpp:999"]
        self.do_run(src.replace('TYPE', 'long long'), '*-2**2**-5**5*')
        self.do_run(src.replace('TYPE', 'int'), '*-2**2**-5**5*')
        Settings.CORRECT_SIGNS = 1 # To be correct here, we need sign corrections as well
        self.do_run(src.replace('TYPE', 'unsigned int'), '*2147483645**2**-5**5*')
        Settings.CORRECT_SIGNS = 0

    def test_pgo(self):
      Settings.PGO = Settings.CHECK_OVERFLOWS = Settings.CORRECT_OVERFLOWS = Settings.CHECK_SIGNS = Settings.CORRECT_SIGNS = 1

      src = '''
        #include<stdio.h>
        int main() {
          int t = 77;
          for (int i = 0; i < 30; i++) {
            t = t*5 + 1;
          }
          printf("*%d,%d*\\n", t, t & 127);

          int varey = 100;
          unsigned int MAXEY = -1;
          for (int j = 0; j < 2; j++) {
            printf("*%d*\\n", varey >= MAXEY); // 100 >= -1? not in unsigned!
            MAXEY = 1; // So we succeed the second time around
          }
          return 0;
        }
      '''

      def check(output):
        # TODO: check the line #
        if self.emcc_args is None or self.emcc_args == []: # LLVM full opts optimize out some corrections
          assert 'Overflow|src.cpp:6 : 60 hits, %20 failures' in output, 'no indication of Overflow corrections: ' + output
          assert 'UnSign|src.cpp:13 : 6 hits, %17 failures' in output, 'no indication of Sign corrections: ' + output
        return output

      self.do_run(src, '*186854335,63*\n', output_nicerizer=check)

      Settings.PGO = Settings.CHECK_OVERFLOWS = Settings.CORRECT_OVERFLOWS = Settings.CHECK_SIGNS = Settings.CORRECT_SIGNS = 0

      # Now, recompile with the PGO data, and it should work

      pgo_data = read_pgo_data(self.get_stdout_path())

      Settings.CORRECT_SIGNS = 2
      Settings.CORRECT_SIGNS_LINES = pgo_data['signs_lines']
      Settings.CORRECT_OVERFLOWS = 2
      Settings.CORRECT_OVERFLOWS_LINES = pgo_data['overflows_lines']

      self.do_run(src, '*186854335,63*\n')

      # Sanity check: Without PGO, we will fail

      try:
        self.do_run(src, '*186854335,63*\n')
      except:
        pass

    def test_exit_status(self):
      Settings.CATCH_EXIT_CODE = 1

      src = r'''
        #include <stdio.h>
        #include <stdlib.h>
        static void cleanup() {
          printf("cleanup\n");
        }

        int main()
        {
          atexit(cleanup); // this atexit should still be called
          printf("hello, world!\n");
          exit(118); // Unusual exit status to make sure it's working!
        }
      '''
      self.do_run(src, 'hello, world!\ncleanup\nExit Status: 118')

    def test_gc(self):
      if self.emcc_args == None: return self.skip('needs ta2')

      Settings.GC_SUPPORT = 1

      src = r'''
        #include <stdio.h>
        #include <gc.h>
        #include <assert.h>

        void *global;

        void finalizer(void *ptr, void *arg) {
          printf("finalizing %d (global == %d)\n", (int)arg, ptr == global);
        }

        void finalizer2(void *ptr, void *arg) {
          printf("finalizing2 %d (global == %d)\n", (int)arg, ptr == global);
        }

        int main() {
          GC_INIT();

          void *local, *local2, *local3, *local4;

          // Hold on to global, drop locals

          global = GC_MALLOC(1024); // rooted since in a static allocation
          GC_REGISTER_FINALIZER_NO_ORDER(global, finalizer, 0, 0, 0);
          printf("alloc %p\n", global);

          local = GC_MALLOC(1024); // not rooted since stack is not scanned
          GC_REGISTER_FINALIZER_NO_ORDER(local, finalizer, (void*)1, 0, 0);
          printf("alloc %p\n", local);

          assert((char*)local - (char*)global >= 1024 || (char*)global - (char*)local >= 1024);

          local2 = GC_MALLOC(1024); // no finalizer
          printf("alloc %p\n", local2);

          local3 = GC_MALLOC(1024); // with finalizable2
          GC_REGISTER_FINALIZER_NO_ORDER(local3, finalizer2, (void*)2, 0, 0);
          printf("alloc %p\n", local);

          local4 = GC_MALLOC(1024); // yet another
          GC_REGISTER_FINALIZER_NO_ORDER(local4, finalizer2, (void*)3, 0, 0);
          printf("alloc %p\n", local);

          printf("basic test\n");

          GC_FORCE_COLLECT();

          printf("*\n");

          GC_FREE(global); // force free will actually work

          // scanning inside objects

          global = GC_MALLOC(12);
          GC_REGISTER_FINALIZER_NO_ORDER(global, finalizer, 0, 0, 0);
          local = GC_MALLOC(12);
          GC_REGISTER_FINALIZER_NO_ORDER(local, finalizer, (void*)1, 0, 0);
          local2 = GC_MALLOC_ATOMIC(12);
          GC_REGISTER_FINALIZER_NO_ORDER(local2, finalizer, (void*)2, 0, 0);
          local3 = GC_MALLOC(12);
          GC_REGISTER_FINALIZER_NO_ORDER(local3, finalizer, (void*)3, 0, 0);
          local4 = GC_MALLOC(12);
          GC_REGISTER_FINALIZER_NO_ORDER(local4, finalizer, (void*)4, 0, 0);

          void **globalData = (void**)global;
          globalData[0] = local;
          globalData[1] = local2;

          void **localData = (void**)local;
          localData[0] = local3;

          void **local2Data = (void**)local2;
          local2Data[0] = local4; // actually ignored, because local2 is atomic, so 4 is freeable

          printf("object scan test test\n");

          GC_FORCE_COLLECT();

          printf("*\n");

          GC_FREE(global); // force free will actually work

          printf("*\n");

          GC_FORCE_COLLECT();

          printf(".\n");

          global = 0;

          return 0;
        }
      '''
      self.do_run(src, '''basic test
finalizing 1 (global == 0)
finalizing2 2 (global == 0)
finalizing2 3 (global == 0)
*
finalizing 0 (global == 1)
object scan test test
finalizing 4 (global == 0)
*
finalizing 0 (global == 1)
*
finalizing 1 (global == 0)
finalizing 2 (global == 0)
finalizing 3 (global == 0)
.
''')

  # Generate tests for everything
  def make_run(fullname, name=-1, compiler=-1, llvm_opts=0, embetter=0, quantum_size=0, typed_arrays=0, emcc_args=None):
    exec('''
class %s(T):
  def tearDown(self):
    super(%s, self).tearDown()
    
  def setUp(self):
    super(%s, self).setUp()

    Building.COMPILER_TEST_OPTS = ['-g']
    os.chdir(self.get_dir()) # Ensure the directory exists and go there
    Building.COMPILER = %r

    self.emcc_args = %s
    if self.emcc_args is not None:
      Settings.load(self.emcc_args)
      Building.LLVM_OPTS = 0
      return

    llvm_opts = %d # 1 is yes, 2 is yes and unsafe
    embetter = %d
    quantum_size = %d
    # TODO: Move much of these to a init() function in shared.py, and reuse that
    Settings.USE_TYPED_ARRAYS = %d
    Settings.INVOKE_RUN = 1
    Settings.RELOOP = 0 # we only do them in the "o2" pass
    Settings.MICRO_OPTS = embetter
    Settings.QUANTUM_SIZE = quantum_size
    Settings.ASSERTIONS = 1-embetter
    Settings.SAFE_HEAP = 1-(embetter and llvm_opts)
    Building.LLVM_OPTS = llvm_opts
    Settings.PGO = 0
    Settings.CHECK_OVERFLOWS = 1-(embetter or llvm_opts)
    Settings.CORRECT_OVERFLOWS = 1-(embetter and llvm_opts)
    Settings.CORRECT_SIGNS = 0
    Settings.CORRECT_ROUNDINGS = 0
    Settings.CORRECT_OVERFLOWS_LINES = CORRECT_SIGNS_LINES = CORRECT_ROUNDINGS_LINES = SAFE_HEAP_LINES = []
    Settings.CHECK_SIGNS = 0 #1-(embetter or llvm_opts)
    Settings.INIT_STACK = 0
    Settings.RUNTIME_TYPE_INFO = 0
    Settings.DISABLE_EXCEPTION_CATCHING = 0
    Settings.PROFILE = 0
    Settings.INCLUDE_FULL_LIBRARY = 0
    Settings.BUILD_AS_SHARED_LIB = 0
    Settings.RUNTIME_LINKED_LIBS = []
    Settings.CATCH_EXIT_CODE = 0
    Settings.EMULATE_UNALIGNED_ACCESSES = int(Settings.USE_TYPED_ARRAYS == 2 and Building.LLVM_OPTS == 2)
    Settings.DOUBLE_MODE = 1 if Settings.USE_TYPED_ARRAYS and Building.LLVM_OPTS == 0 else 0
    Settings.PRECISE_I64_MATH = 0

    Building.pick_llvm_opts(3)

TT = %s
''' % (fullname, fullname, fullname, compiler, str(emcc_args), llvm_opts, embetter, quantum_size, typed_arrays, fullname))
    return TT

  # Make one run with the defaults
  exec('default = make_run("default", compiler=CLANG, emcc_args=[])')

  # Make one run with -O1, with safe heap
  exec('o1 = make_run("o1", compiler=CLANG, emcc_args=["-O1", "-s", "SAFE_HEAP=1"])')

  # Make one run with -O2, but without closure (we enable closure in specific tests, otherwise on everything it is too slow)
  exec('o2 = make_run("o2", compiler=CLANG, emcc_args=["-O2", "--closure", "0"])')

  # Make custom runs with various options
  for compiler, quantum, embetter, typed_arrays, llvm_opts in [
    (CLANG, 1, 1, 0, 0),
    (CLANG, 1, 1, 1, 1),
    (CLANG, 4, 0, 0, 0),
    (CLANG, 4, 0, 0, 1),
    (CLANG, 4, 1, 1, 0),
    (CLANG, 4, 1, 1, 1),
  ]:
    fullname = 's_%d_%d%s%s' % (
      llvm_opts, embetter, '' if quantum == 4 else '_q' + str(quantum), '' if typed_arrays in [0, 1] else '_t' + str(typed_arrays)
    )
    exec('%s = make_run(fullname, %r,%r,%d,%d,%d,%d)' % (fullname, fullname, compiler, llvm_opts, embetter, quantum, typed_arrays))

  del T # T is just a shape for the specific subclasses, we don't test it itself

  class other(RunnerCore):
    def test_emcc(self):
      for compiler in [EMCC, EMXX]:
        shortcompiler = os.path.basename(compiler)
        suffix = '.c' if compiler == EMCC else '.cpp'

        # --version
        output = Popen(['python', compiler, '--version'], stdout=PIPE, stderr=PIPE).communicate()
        self.assertContained('''emcc (Emscripten GCC-like replacement) 2.0
Copyright (C) 2012 the Emscripten authors.
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
''', output[0].replace('\r', ''), output[1].replace('\r', ''))

        # -v, without input files
        output = Popen(['python', compiler, '-v'], stdout=PIPE, stderr=PIPE).communicate()
        self.assertContained('''clang version''', output[1].replace('\r', ''), output[1].replace('\r', ''))

        # --help
        output = Popen(['python', compiler, '--help'], stdout=PIPE, stderr=PIPE).communicate()
        self.assertContained('''%s [options] file...

Most normal gcc/g++ options will work, for example:
  --help                   Display this information
  --version                Display compiler version information

Options that are modified or new in %s include:
  -O0                      No optimizations (default)
''' % (shortcompiler, shortcompiler), output[0].replace('\r', ''), output[1].replace('\r', ''))

        # emcc src.cpp ==> writes a.out.js
        self.clear()
        output = Popen(['python', compiler, path_from_root('tests', 'hello_world' + suffix)], stdout=PIPE, stderr=PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        assert os.path.exists('a.out.js'), '\n'.join(output)
        self.assertContained('hello, world!', run_js('a.out.js'))

        # properly report source code errors, and stop there
        self.clear()
        assert not os.path.exists('a.out.js')
        output = Popen(['python', compiler, path_from_root('tests', 'hello_world_error' + suffix)], stdout=PIPE, stderr=PIPE).communicate()
        assert not os.path.exists('a.out.js'), 'compilation failed, so no output file is expected'
        assert len(output[0]) == 0, output[0]
        self.assertNotContained('IOError', output[1]) # no python stack
        self.assertNotContained('Traceback', output[1]) # no python stack
        self.assertContained('error: invalid preprocessing directive', output[1])
        self.assertContained("error: use of undeclared identifier 'cheez", output[1])
        self.assertContained('2 errors generated', output[1])
        assert output[1].split('2 errors generated.')[1].replace('\r', '').replace('\n', '') == 'emcc: compiler frontend failed to generate LLVM bitcode, halting'

        # emcc src.cpp -c    and   emcc src.cpp -o src.[o|bc] ==> should give a .bc file
        #      regression check: -o js should create "js", with bitcode content
        for args in [['-c'], ['-o', 'src.o'], ['-o', 'src.bc'], ['-o', 'src.so'], ['-o', 'js']]:
          target = args[1] if len(args) == 2 else 'hello_world.o'
          self.clear()
          Popen(['python', compiler, path_from_root('tests', 'hello_world' + suffix)] + args, stdout=PIPE, stderr=PIPE).communicate()
          syms = Building.llvm_nm(target)
          assert len(syms.defs) == 1 and 'main' in syms.defs, 'Failed to generate valid bitcode'
          if target == 'js': # make sure emcc can recognize the target as a bitcode file
            shutil.move(target, target + '.bc')
            target += '.bc'
          output = Popen(['python', compiler, target, '-o', target + '.js'], stdout = PIPE, stderr = PIPE).communicate()
          assert len(output[0]) == 0, output[0]
          assert os.path.exists(target + '.js'), 'Expected %s to exist since args are %s : %s' % (target + '.js', str(args), '\n'.join(output))
          self.assertContained('hello, world!', run_js(target + '.js'))

        # handle singleton archives
        self.clear()
        Popen(['python', compiler, path_from_root('tests', 'hello_world' + suffix), '-o', 'a.bc'], stdout=PIPE, stderr=PIPE).communicate()
        Popen([LLVM_AR, 'r', 'a.a', 'a.bc'], stdout=PIPE, stderr=PIPE).communicate()
        assert os.path.exists('a.a')
        output = Popen(['python', compiler, 'a.a']).communicate()
        assert os.path.exists('a.out.js'), output
        self.assertContained('hello, world!', run_js('a.out.js'))

        # emcc src.ll ==> generates .js
        self.clear()
        output = Popen(['python', compiler, path_from_root('tests', 'hello_world.ll')], stdout=PIPE, stderr=PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        assert os.path.exists('a.out.js'), '\n'.join(output)
        self.assertContained('hello, world!', run_js('a.out.js'))

        # emcc [..] -o [path] ==> should work with absolute paths
        try:
          os.mkdir('a_dir')
          os.chdir('a_dir')
          os.mkdir('b_dir')
          for path in [os.path.abspath(os.path.join('..', 'file1.js')), os.path.join('b_dir', 'file2.js')]:
            self.clear()
            output = Popen(['python', compiler, path_from_root('tests', 'hello_world.ll'), '-o', path], stdout=PIPE, stderr=PIPE).communicate()
            assert os.path.exists(path), path + ' does not exist; ' + '\n'.join(output)
            self.assertContained('hello, world!', run_js(path))
        finally:
          os.chdir(self.get_dir())
          try:
            shutil.rmtree('a_dir')
          except:
            pass

        # dlmalloc. dlmalloc is special in that it is the only part of libc that is (1) hard to write well, and
        # very speed-sensitive. So we do not implement it in JS in library.js, instead we compile it from source
        for source, has_malloc in [('hello_world' + suffix, False), ('hello_malloc.cpp', True)]:
          self.clear()
          output = Popen(['python', compiler, path_from_root('tests', source)], stdout=PIPE, stderr=PIPE).communicate()
          assert os.path.exists('a.out.js'), '\n'.join(output)
          self.assertContained('hello, world!', run_js('a.out.js'))
          generated = open('a.out.js').read()
          assert ('function _malloc(bytes) {' in generated) == (not has_malloc), 'If malloc is needed, it should be there, if not not'

        # Optimization: emcc src.cpp -o something.js [-Ox]. -O0 is the same as not specifying any optimization setting
        for params, opt_level, bc_params, closure, has_malloc in [ # bc params are used after compiling to bitcode
          (['-o', 'something.js'],                          0, None, 0, 1),
          (['-o', 'something.js', '-O0'],                   0, None, 0, 0),
          (['-o', 'something.js', '-O1'],                   1, None, 0, 0),
          (['-o', 'something.js', '-O1', '--closure', '1'], 1, None, 1, 0),
          (['-o', 'something.js', '-O2'],                   2, None, 1, 1),
          (['-o', 'something.js', '-O2', '--closure', '0'], 2, None, 0, 0),
          (['-o', 'something.js', '-O3'],                   3, None, 1, 1),
          (['-o', 'something.js', '-O3', '--closure', '0'], 3, None, 0, 0),
          # and, test compiling to bitcode first
          (['-o', 'something.bc'], 0, [],      0, 0),
          (['-o', 'something.bc'], 0, ['-O0'], 0, 0),
          (['-o', 'something.bc'], 1, ['-O1'], 0, 0),
          (['-o', 'something.bc'], 2, ['-O2'], 1, 0),
          (['-o', 'something.bc'], 3, ['-O3'], 1, 0),
          (['-O1', '-o', 'something.bc'], 0, [], 0, 0), # -Ox is ignored and warned about
        ]:
          #print params, opt_level, bc_params, closure
          self.clear()
          output = Popen(['python', compiler, path_from_root('tests', 'hello_world_loop' + ('_malloc' if has_malloc else '') + '.cpp')] + params,
                         stdout=PIPE, stderr=PIPE).communicate()
          assert len(output[0]) == 0, output[0]
          if bc_params is not None:
            if '-O1' in params and 'something.bc' in params:
              assert 'warning: -Ox flags ignored, since not generating JavaScript' in output[1]
            assert os.path.exists('something.bc'), output[1]
            output = Popen(['python', compiler, 'something.bc', '-o', 'something.js'] + bc_params, stdout=PIPE, stderr=PIPE).communicate()
          assert os.path.exists('something.js'), output[1]
          assert ('Warning: Applying some potentially unsafe optimizations!' in output[1]) == (opt_level >= 3), 'unsafe warning should appear in opt >= 3'
          self.assertContained('hello, world!', run_js('something.js'))

          # Verify optimization level etc. in the generated code
          # XXX these are quite sensitive, and will need updating when code generation changes
          generated = open('something.js').read() # TODO: parse out the _main function itself, not support code, if the tests below need that some day
          assert 'new Uint16Array' in generated and 'new Uint32Array' in generated, 'typed arrays 2 should be used by default'
          assert 'SAFE_HEAP' not in generated, 'safe heap should not be used by default'
          assert ': while(' not in generated, 'when relooping we also js-optimize, so there should be no labelled whiles'
          if closure:
            assert 'Module._main=' in generated, 'closure compiler should have been run (and output should be minified)'
          else:
            # closure has not been run, we can do some additional checks. TODO: figure out how to do these even with closure
            assert 'Module._main = ' not in generated, 'closure compiler should not have been run'
            # XXX find a way to test this: assert ('& 255' in generated or '&255' in generated) == (opt_level <= 2), 'corrections should be in opt <= 2'
            assert ('(__label__)' in generated) == (opt_level <= 1), 'relooping should be in opt >= 2'
            assert ('assert(STACKTOP < STACK_MAX' in generated) == (opt_level == 0), 'assertions should be in opt == 0'
            assert 'var $i;' in generated or 'var $i_01;' in generated or 'var $storemerge3;' in generated or 'var $storemerge4;' in generated or 'var $i_04;' in generated, 'micro opts should always be on'
            if opt_level >= 1:
              assert 'HEAP8[HEAP32[' in generated or 'HEAP8[$vla1 + (($storemerge4 | 0) / 2 & -1) | 0]' in generated or 'HEAP8[$vla1 + (($storemerge4 | 0) / 2 & -1) | 0]' in generated or 'HEAP8[$vla1 + (($i_04 | 0) / 2 & -1) | 0]' in generated or 'HEAP8[$vla1 + ($i_04 / 2 & -1)]' in generated or 'HEAP8[$1 + (($i_01 | 0) / 2 & -1) | 0]' in generated or 'HEAP8[$1 + (($i_01 | 0) / 2 & -1) | 0]' in generated or 'HEAP8[$1 + ($i_01 / 2 & -1)]' in generated, 'eliminator should create compound expressions, and fewer one-time vars'
            assert ('_puts(' in generated) == (opt_level >= 1), 'with opt >= 1, llvm opts are run and they should optimize printf to puts'
            assert ('function _malloc(bytes) {' in generated) == (not has_malloc), 'If malloc is needed, it should be there, if not not'
            assert 'function _main() {' in generated, 'Should be unminified, including whitespace'
            assert ('-O3' in (params+(bc_params or []))) or'function _dump' in generated, 'No inlining by default'

        # emcc -s RELOOP=1 src.cpp ==> should pass -s to emscripten.py. --typed-arrays is a convenient alias for -s USE_TYPED_ARRAYS
        for params, test, text in [
          (['-s', 'INLINING_LIMIT=0'], lambda generated: 'function _dump' in generated, 'no inlining without opts'),
          (['-O1', '-s', 'INLINING_LIMIT=0'], lambda generated: 'function _dump' not in generated, 'inlining'),
          (['-s', 'USE_TYPED_ARRAYS=0'], lambda generated: 'new Int32Array' not in generated, 'disable typed arrays'),
          (['-s', 'USE_TYPED_ARRAYS=1'], lambda generated: 'IHEAPU = ' in generated, 'typed arrays 1 selected'),
          ([], lambda generated: 'Module["_dump"]' not in generated, 'dump is not exported by default'),
          (['-s', 'EXPORTED_FUNCTIONS=["_main", "_dump"]'], lambda generated: 'Module["_dump"]' in generated, 'dump is now exported'),
          (['--typed-arrays', '0'], lambda generated: 'new Int32Array' not in generated, 'disable typed arrays'),
          (['--typed-arrays', '1'], lambda generated: 'IHEAPU = ' in generated, 'typed arrays 1 selected'),
          (['--typed-arrays', '2'], lambda generated: 'new Uint16Array' in generated and 'new Uint32Array' in generated, 'typed arrays 2 selected'),
          (['--llvm-opts', '1'], lambda generated: '_puts(' in generated, 'llvm opts requested'),
        ]:
          self.clear()
          output = Popen(['python', compiler, path_from_root('tests', 'hello_world_loop.cpp'), '-o', 'a.out.js'] + params, stdout=PIPE, stderr=PIPE).communicate()
          assert len(output[0]) == 0, output[0]
          assert os.path.exists('a.out.js'), '\n'.join(output)
          self.assertContained('hello, world!', run_js('a.out.js'))
          assert test(open('a.out.js').read()), text

        # Compiling two source files into a final JS.
        for args, target in [([], 'a.out.js'), (['-o', 'combined.js'], 'combined.js')]:
          self.clear()
          output = Popen(['python', compiler, path_from_root('tests', 'twopart_main.cpp'), path_from_root('tests', 'twopart_side.cpp')] + args,
                         stdout=PIPE, stderr=PIPE).communicate()
          assert len(output[0]) == 0, output[0]
          assert os.path.exists(target), '\n'.join(output)
          self.assertContained('side got: hello from main, over', run_js(target))

          # Compiling two files with -c will generate separate .bc files
          self.clear()
          output = Popen(['python', compiler, path_from_root('tests', 'twopart_main.cpp'), path_from_root('tests', 'twopart_side.cpp'), '-c'] + args,
                         stdout=PIPE, stderr=PIPE).communicate()
          if '-o' in args:
            # specifying -o and -c is an error
            assert 'fatal error' in output[1], output[1]
            continue

          assert os.path.exists('twopart_main.o'), '\n'.join(output)
          assert os.path.exists('twopart_side.o'), '\n'.join(output)
          assert not os.path.exists(target), 'We should only have created bitcode here: ' + '\n'.join(output)

          # Compiling one of them alone is expected to fail
          output = Popen(['python', compiler, 'twopart_main.o'] + args, stdout=PIPE, stderr=PIPE).communicate()
          assert os.path.exists(target), '\n'.join(output)
          #print '\n'.join(output)
          self.assertContained('is not a function', run_js(target, stderr=STDOUT))
          try_delete(target)

          # Combining those bc files into js should work
          output = Popen(['python', compiler, 'twopart_main.o', 'twopart_side.o'] + args, stdout=PIPE, stderr=PIPE).communicate()
          assert os.path.exists(target), '\n'.join(output)
          self.assertContained('side got: hello from main, over', run_js(target))

          # Combining bc files into another bc should also work
          try_delete(target)
          assert not os.path.exists(target)
          output = Popen(['python', compiler, 'twopart_main.o', 'twopart_side.o', '-o', 'combined.bc'] + args, stdout=PIPE, stderr=PIPE).communicate()
          syms = Building.llvm_nm('combined.bc')
          assert len(syms.defs) == 2 and 'main' in syms.defs, 'Failed to generate valid bitcode'
          output = Popen(['python', compiler, 'combined.bc', '-o', 'combined.bc.js'], stdout = PIPE, stderr = PIPE).communicate()
          assert len(output[0]) == 0, output[0]
          assert os.path.exists('combined.bc.js'), 'Expected %s to exist' % ('combined.bc.js')
          self.assertContained('side got: hello from main, over', run_js('combined.bc.js'))

        # --js-transform <transform>
        self.clear()
        trans = os.path.join(self.get_dir(), 't.py')
        trans_file = open(trans, 'w')
        trans_file.write('''
import sys
f = open(sys.argv[1], 'w')
f.write('transformed!')
f.close()
''')
        trans_file.close()
        output = Popen(['python', compiler, path_from_root('tests', 'hello_world' + suffix), '--js-transform', 'python t.py'], stdout=PIPE, stderr=PIPE).communicate()
        assert open('a.out.js').read() == 'transformed!', 'Transformed output must be as expected'

      # TODO: Add in files test a clear example of using disablePermissions, and link to it from the wiki
      # TODO: test normal project linking, static and dynamic: get_library should not need to be told what to link!
      # TODO: deprecate llvm optimizations, dlmalloc, etc. in emscripten.py.

    def test_catch_undef(self):
      open(os.path.join(self.get_dir(), 'test.cpp'), 'w').write(r'''
        #include <vector>
        #include <stdio.h>
         
        class Test {
        public:
          std::vector<int> vector;
        };
         
        Test globalInstance;
         
        int main() {
          printf("hello, world!\n");
          return 1;
        }
      ''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'test.cpp'), '-fcatch-undefined-behavior']).communicate()
      self.assertContained('hello, world!', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_l_link(self):
      # Linking with -lLIBNAME and -L/DIRNAME should work

      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        extern void printey();
        int main() {
          printey();
          return 0;
        }
      ''')

      try:
        os.makedirs(os.path.join(self.get_dir(), 'libdir'));
      except:
        pass

      open(os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), 'w').write('''
        #include <stdio.h>
        void printey() {
          printf("hello from lib\\n");
        }
      ''')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), '-c']).communicate()
      shutil.move(os.path.join(self.get_dir(), 'libfile.o'), os.path.join(self.get_dir(), 'libdir', 'libfile.so'))
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile']).communicate()
      self.assertContained('hello from lib', run_js(os.path.join(self.get_dir(), 'a.out.js')))
      assert not os.path.exists('a.out') and not os.path.exists('a.exe'), 'Must not leave unneeded linker stubs'

    def test_abspaths(self):
      # Includes with absolute paths are generally dangerous, things like -I/usr/.. will get to system local headers, not our portable ones.

      shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'main.c')

      for args, expected in [(['-I/usr/something'], True),
                             (['-L/usr/something'], True),
                             (['-Isubdir/something'], False),
                             (['-Lsubdir/something'], False),
                             ([], False)]:
        err = Popen(['python', EMCC, 'main.c'] + args, stderr=PIPE).communicate()[1]
        assert ('emcc: warning: -I or -L of an absolute path encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript)' in err) == expected, err

    def test_local_link(self):
      # Linking a local library directly, like /usr/lib/libsomething.so, cannot work of course since it
      # doesn't contain bitcode. However, when we see that we should look for a bitcode file for that
      # library in the -L paths and system/lib
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        extern void printey();
        int main() {
          printey();
          return 0;
        }
      ''')

      try:
        os.makedirs(os.path.join(self.get_dir(), 'subdir'));
      except:
        pass
      open(os.path.join(self.get_dir(), 'subdir', 'libfile.so'), 'w').write('this is not llvm bitcode!')

      open(os.path.join(self.get_dir(), 'libfile.cpp'), 'w').write('''
        #include <stdio.h>
        void printey() {
          printf("hello from lib\\n");
        }
      ''')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'libfile.cpp'), '-o', 'libfile.so']).communicate()
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), os.path.join(self.get_dir(), 'subdir', 'libfile.so'), '-L.']).communicate()
      self.assertContained('hello from lib', run_js(os.path.join(self.get_dir(), 'a.out.js')))

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

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--js-library', os.path.join(self.get_dir(), 'mylib1.js'),
                                                                       '--js-library', os.path.join(self.get_dir(), 'mylib2.js')]).communicate()
      self.assertContained('hello from lib!\n*32*\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_identical_basenames(self):
      # Issue 287: files in different dirs but with the same basename get confused as the same,
      # causing multiply defined symbol errors
      try:
        os.makedirs(os.path.join(self.get_dir(), 'foo'));
      except:
        pass
      try:
        os.makedirs(os.path.join(self.get_dir(), 'bar'));
      except:
        pass
      open(os.path.join(self.get_dir(), 'foo', 'main.cpp'), 'w').write('''
        extern void printey();
        int main() {
          printey();
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'bar', 'main.cpp'), 'w').write('''
        #include<stdio.h>
        void printey() { printf("hello there\\n"); }
      ''')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'foo', 'main.cpp'), os.path.join(self.get_dir(), 'bar', 'main.cpp')]).communicate()
      self.assertContained('hello there', run_js(os.path.join(self.get_dir(), 'a.out.js')))

      # ditto with first creating .o files
      try_delete(os.path.join(self.get_dir(), 'a.out.js'))
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'foo', 'main.cpp'), '-o', os.path.join(self.get_dir(), 'foo', 'main.o')]).communicate()
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'bar', 'main.cpp'), '-o', os.path.join(self.get_dir(), 'bar', 'main.o')]).communicate()
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'foo', 'main.o'), os.path.join(self.get_dir(), 'bar', 'main.o')]).communicate()
      self.assertContained('hello there', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_remove_duplicates(self):
      # can happen with .a files. we do a best-effort, removing dupes
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        #include<stdio.h>
        void printey() { printf("bye bye\\n"); }
        int main() {
          printey();
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'side.cpp'), 'w').write('''
        #include<stdio.h>
        void printey() { printf("bye bye\\n"); }
      ''')

      # without --remove-duplicates, we fail
      err = Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), os.path.join(self.get_dir(), 'side.cpp')], stderr=PIPE).communicate()[1]
      assert not os.path.exists('a.out.js')
      assert 'multiply' in err

      # with it, we succeed
      err = Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), os.path.join(self.get_dir(), 'side.cpp'), '--remove-duplicates'], stderr=PIPE).communicate()[1]
      self.assertContained('bye bye', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_main_a(self):
      # if main() is in a .a, we need to pull in that .a

      main_name = os.path.join(self.get_dir(), 'main.c')
      open(main_name, 'w').write(r'''
        #include <stdio.h>
        extern int f();
        int main() {
          printf("result: %d.\n", f());
          return 0;
        }
      ''')

      other_name = os.path.join(self.get_dir(), 'other.c')
      open(other_name, 'w').write(r'''
        #include <stdio.h>
        int f() { return 12346; }
      ''')

      Popen(['python', EMCC, main_name, '-c', '-o', main_name+'.bc']).communicate()
      Popen(['python', EMCC, other_name, '-c', '-o', other_name+'.bc']).communicate()

      Popen(['python', EMAR, 'cr', main_name+'.a', main_name+'.bc']).communicate()

      Popen(['python', EMCC, other_name+'.bc', main_name+'.a']).communicate()

      self.assertContained('result: 12346.', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_embed_file(self):
      open(os.path.join(self.get_dir(), 'somefile.txt'), 'w').write('''hello from a file with lots of data and stuff in it thank you very much''')
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
        #include <stdio.h>
        int main() {
          FILE *f = fopen("somefile.txt", "r");
          char buf[100];
          fread(buf, 1, 20, f);
          buf[20] = 0;
          fclose(f);
          printf("|%s|\n", buf);
          return 0;
        }
      ''')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--embed-file', 'somefile.txt']).communicate()
      self.assertContained('|hello from a file wi|', run_js(os.path.join(self.get_dir(), 'a.out.js')))

      # preload twice, should not err
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--embed-file', 'somefile.txt', '--embed-file', 'somefile.txt']).communicate()
      self.assertContained('|hello from a file wi|', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_multidynamic_link(self):
      # Linking the same dynamic library in will error, normally, since we statically link it, causing dupe symbols
      # A workaround is to use --ignore-dynamic-linking, see emcc --help for details

      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
        #include <stdio.h>
        extern void printey();
        extern void printother();
        int main() {
          printf("*");
          printey();
          printf("\n");
          printother();
          printf("\n");
          printf("*");
          return 0;
        }
      ''')

      try:
        os.makedirs(os.path.join(self.get_dir(), 'libdir'));
      except:
        pass

      open(os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), 'w').write('''
        #include <stdio.h>
        void printey() {
          printf("hello from lib");
        }
      ''')

      open(os.path.join(self.get_dir(), 'libdir', 'libother.cpp'), 'w').write('''
        #include <stdio.h>
        extern void printey();
        void printother() {
          printf("|");
          printey();
          printf("|");
        }
      ''')

      # This lets us link the same dynamic lib twice. We will need to link it in manually at the end.
      compiler = ['python', EMCC, '--ignore-dynamic-linking']

      # Build libfile normally into an .so
      Popen(compiler + [os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), '-o', os.path.join(self.get_dir(), 'libdir', 'libfile.so')]).communicate()
      # Build libother and dynamically link it to libfile - but add --ignore-dynamic-linking
      Popen(compiler + [os.path.join(self.get_dir(), 'libdir', 'libother.cpp'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile', '-o', os.path.join(self.get_dir(), 'libdir', 'libother.so')]).communicate()
      # Build the main file, linking in both the libs
      Popen(compiler + [os.path.join(self.get_dir(), 'main.cpp'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile', '-lother', '-c']).communicate()

      # The normal build system is over. We need to do an additional step to link in the dynamic libraries, since we ignored them before
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.o'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile', '-lother']).communicate()

      self.assertContained('*hello from lib\n|hello from lib|\n*', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_js_link(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        #include <stdio.h>
        int main() {
          printf("hello from main\\n");
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'before.js'), 'w').write('''
        var MESSAGE = 'hello from js';
        if (typeof Module != 'undefined') throw 'This code should run before anything else!';
      ''')
      open(os.path.join(self.get_dir(), 'after.js'), 'w').write('''
        Module.print(MESSAGE);
      ''')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'before.js', '--post-js', 'after.js']).communicate()
      self.assertContained('hello from main\nhello from js\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_sdl_endianness(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
        #include <stdio.h>
        #include <SDL/SDL.h>

        int main() {
          printf("%d, %d, %d\n", SDL_BYTEORDER, SDL_LIL_ENDIAN, SDL_BIG_ENDIAN);
          return 0;
        }
      ''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp')]).communicate()
      self.assertContained('1234, 1234, 4321\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_warn_undefined(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
        #include <stdio.h>

        extern "C" {
          void something();
        }

        int main() {
          something();
          return 0;
        }
      ''')
      output = Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-s', 'WARN_ON_UNDEFINED_SYMBOLS=1'], stderr=PIPE).communicate()
      self.assertContained('Unresolved symbol: _something\n', output[1])

      output = Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp')], stderr=PIPE).communicate()
      self.assertNotContained('Unresolved symbol: _something\n', output[1])

    def test_prepost(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        #include <stdio.h>
        int main() {
          printf("hello from main\\n");
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        var Module = {
          preRun: function() { Module.print('pre-run') },
          postRun: function() { Module.print('post-run') }
        };
      ''')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js']).communicate()
      self.assertContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

      # never run, so no preRun or postRun
      src = open(os.path.join(self.get_dir(), 'a.out.js')).read().replace('// {{PRE_RUN_ADDITIONS}}', 'addRunDependency()')
      open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
      self.assertNotContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

      # noInitialRun prevents run
      for no_initial_run in [0, 1]:
        Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp')]).communicate()
        src = 'var Module = { noInitialRun: %d };\n' % no_initial_run + open(os.path.join(self.get_dir(), 'a.out.js')).read()
        open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
        assert ('hello from main' in run_js(os.path.join(self.get_dir(), 'a.out.js'))) != no_initial_run, 'only run if no noInitialRun'

        if no_initial_run:
          # Calling main later should still work, filesystem etc. must be set up.
          src = open(os.path.join(self.get_dir(), 'a.out.js')).read() + '\n_main();\n';
          open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
          assert 'hello from main' in run_js(os.path.join(self.get_dir(), 'a.out.js')), 'main should print when called manually'

      # Use postInit
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        var Module = {
          preRun: function() { Module.print('pre-run') },
          postRun: function() { Module.print('post-run') },
          preInit: function() { Module.print('pre-init') }
        };
      ''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js']).communicate()
      self.assertContained('pre-init\npre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_prepost2(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        #include <stdio.h>
        int main() {
          printf("hello from main\\n");
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        var Module = {
          preRun: function() { Module.print('pre-run') },
        };
      ''')
      open(os.path.join(self.get_dir(), 'pre2.js'), 'w').write('''
        Module.postRun = function() { Module.print('post-run') };
      ''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '--pre-js', 'pre2.js']).communicate()
      self.assertContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_prepre(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
        #include <stdio.h>
        int main() {
          printf("hello from main\\n");
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        var Module = {
          preRun: [function() { Module.print('pre-run') }],
        };
      ''')
      open(os.path.join(self.get_dir(), 'pre2.js'), 'w').write('''
        Module.preRun.push(function() { Module.print('prepre') });
      ''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '--pre-js', 'pre2.js']).communicate()
      self.assertContained('prepre\npre-run\nhello from main\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    def test_eliminator(self):
      expected = open(path_from_root('tools', 'eliminator', 'eliminator-test-output.js')).read()
      output = Popen([NODE_JS, COFFEESCRIPT, VARIABLE_ELIMINATOR, path_from_root('tools', 'eliminator', 'eliminator-test.js')], stdout=PIPE).communicate()[0]
      self.assertIdentical(expected, output)

    def test_fix_closure(self):
      input = path_from_root('tests', 'test-fix-closure.js')
      expected = path_from_root('tests', 'test-fix-closure.out.js')
      Popen(['python', path_from_root('tools', 'fix_closure.py'), input, 'out.js']).communicate(input)
      output = open('out.js').read()
      assert '0,zzz_Q_39fa,0' in output
      assert 'function(a,c)' not in output # should be uninlined, so it gets a name
      assert run_js(input) == run_js('out.js')

    def test_js_optimizer(self):
      for input, expected, passes in [
        (path_from_root('tools', 'test-js-optimizer.js'), open(path_from_root('tools', 'test-js-optimizer-output.js')).read(),
         ['hoistMultiples', 'loopOptimizer', 'unGlobalize', 'removeAssignsToUndefined', 'simplifyExpressionsPre', 'simplifyExpressionsPost']),
        (path_from_root('tools', 'test-js-optimizer-t2c.js'), open(path_from_root('tools', 'test-js-optimizer-t2c-output.js')).read(),
         ['simplifyExpressionsPre', 'optimizeShiftsConservative']),
        (path_from_root('tools', 'test-js-optimizer-t2.js'), open(path_from_root('tools', 'test-js-optimizer-t2-output.js')).read(),
         ['simplifyExpressionsPre', 'optimizeShiftsAggressive']),
        # Make sure that optimizeShifts handles functions with shift statements.
        (path_from_root('tools', 'test-js-optimizer-t3.js'), open(path_from_root('tools', 'test-js-optimizer-t3.js')).read(),
         ['optimizeShiftsAggressive']),
        (path_from_root('tools', 'test-js-optimizer-regs.js'), open(path_from_root('tools', 'test-js-optimizer-regs-output.js')).read(),
         ['registerize']),
      ]:
        output = Popen([NODE_JS, JS_OPTIMIZER, input] + passes, stdin=PIPE, stdout=PIPE).communicate()[0]
        self.assertIdentical(expected, output.replace('\n\n', '\n'))

    def test_m_mm(self):
      open(os.path.join(self.get_dir(), 'foo.c'), 'w').write('''#include <emscripten.h>''')
      for opt in ['M', 'MM']:
        output, err = Popen(['python', EMCC, os.path.join(self.get_dir(), 'foo.c'), '-' + opt], stdout=PIPE, stderr=PIPE).communicate()
        assert 'foo.o: ' in output, '-%s failed to produce the right output: %s' % (opt, output)
        assert 'error' not in err, 'Unexpected stderr: ' + err

    def test_scons(self): # also incidentally tests c++11 integration in llvm 3.1
      try_delete(os.path.join(self.get_dir(), 'test'))
      shutil.copytree(path_from_root('tests', 'scons'), os.path.join(self.get_dir(), 'test'))
      shutil.copytree(path_from_root('tools', 'scons', 'site_scons'), os.path.join(self.get_dir(), 'test', 'site_scons'))
      os.chdir(os.path.join(self.get_dir(), 'test'))
      Popen(['scons']).communicate()
      output = run_js('scons_integration.js')
      assert 'If you see this - the world is all right!' in output

    def test_llvm_nativizer(self):
      # avoid impure_ptr problems etc.
      shutil.copyfile(path_from_root('tests', 'files.cpp'), os.path.join(self.get_dir(), 'files.cpp'))
      open(os.path.join(self.get_dir(), 'somefile.binary'), 'w').write('''waka waka############################''')
      open(os.path.join(self.get_dir(), 'test.file'), 'w').write('''ay file..............,,,,,,,,,,,,,,''')
      open(os.path.join(self.get_dir(), 'stdin'), 'w').write('''inter-active''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'files.cpp'), '-c']).communicate()
      Popen(['python', path_from_root('tools', 'nativize_llvm.py'), os.path.join(self.get_dir(), 'files.o')]).communicate(input)[0]
      output = Popen([os.path.join(self.get_dir(), 'files.o.run')], stdin=open(os.path.join(self.get_dir(), 'stdin')), stdout=PIPE, stderr=PIPE).communicate()
      self.assertIdentical('''size: 37
data: 119,97,107,97,32,119,97,107,97,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35
loop: 119 97 107 97 32 119 97 107 97 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 
input:inter-active
texto
$
5 : 10,30,20,11,88
other=ay file...
seeked= file.
seeked=e...
seeked=,,.
fscanfed: 10 - hello
''', output[0])
      self.assertIdentical('texte\n', output[1])

    def test_emconfig(self):
      output = Popen(['python', EMCONFIG, 'LLVM_ROOT'], stdout=PIPE, stderr=PIPE).communicate()[0]
      assert output == LLVM_ROOT + "\n"
      invalid = 'Usage: em-config VAR_NAME\n'
      # Don't accept variables that do not exist
      output = Popen(['python', EMCONFIG, 'VAR_WHICH_DOES_NOT_EXIST'], stdout=PIPE, stderr=PIPE).communicate()[0]
      assert output == invalid
      # Don't accept no arguments
      output = Popen(['python', EMCONFIG], stdout=PIPE, stderr=PIPE).communicate()[0]
      assert output == invalid
      # Don't accept more than one variable
      output = Popen(['python', EMCONFIG, 'LLVM_ROOT', 'EMCC'], stdout=PIPE, stderr=PIPE).communicate()[0]
      assert output == invalid
      # Don't accept arbitrary python code
      output = Popen(['python', EMCONFIG, 'sys.argv[1]'], stdout=PIPE, stderr=PIPE).communicate()[0]
      assert output == invalid

    def test_link_s(self):
      # -s OPT=VALUE can conflict with -s as a linker option. We warn and ignore
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
        extern "C" {
          void something();
        }

        int main() {
          something();
          return 0;
        }
      ''')
      open(os.path.join(self.get_dir(), 'supp.cpp'), 'w').write(r'''
        #include <stdio.h>

        extern "C" {
          void something() {
            printf("yello\n");
          }
        }
      ''')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-o', 'main.o']).communicate()
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'supp.cpp'), '-o', 'supp.o']).communicate()

      output = Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.o'), '-s', os.path.join(self.get_dir(), 'supp.o'), '-s', 'SAFE_HEAP=1'], stderr=PIPE).communicate()
      self.assertContained('emcc: warning: treating -s as linker option', output[1])
      output = run_js('a.out.js')
      assert 'yello' in output, 'code works'
      code = open('a.out.js').read()
      assert 'SAFE_HEAP' in code, 'valid -s option had an effect'

    def test_crunch(self):
      # crunch should not be run if a .crn exists that is more recent than the .dds
      shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
      time.sleep(0.1)
      Popen(['python', FILE_PACKAGER, 'test.data', '--pre-run', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
      assert os.stat('test.data').st_size < 0.25*os.stat('ship.dds').st_size, 'Compressed should be much smaller than dds'
      crunch_time = os.stat('ship.crn').st_mtime
      dds_time = os.stat('ship.dds').st_mtime
      assert crunch_time > dds_time, 'Crunch is more recent'
      # run again, should not recrunch!
      time.sleep(0.1)
      Popen(['python', FILE_PACKAGER, 'test.data', '--pre-run', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
      assert crunch_time == os.stat('ship.crn').st_mtime, 'Crunch is unchanged'
      # update dds, so should recrunch
      time.sleep(0.1)
      os.utime('ship.dds', None)
      Popen(['python', FILE_PACKAGER, 'test.data', '--pre-run', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
      assert crunch_time < os.stat('ship.crn').st_mtime, 'Crunch was changed'

elif 'browser' in str(sys.argv):
  # Browser tests.

  # Run a server and a web page. When a test runs, we tell the server about it,
  # which tells the web page, which then opens a window with the test. Doing
  # it this way then allows the page to close() itself when done.

  def harness_server_func(q):
    class TestServerHandler(BaseHTTPServer.BaseHTTPRequestHandler):
      def do_GET(s):
        s.send_response(200)
        s.send_header("Content-type", "text/html")
        s.end_headers()
        if s.path == '/run_harness':
          s.wfile.write(open(path_from_root('tests', 'browser_harness.html')).read())
        else:
          result = 'False'
          if not q.empty():
            result = q.get()
          s.wfile.write(result)
        s.wfile.close()
    httpd = BaseHTTPServer.HTTPServer(('localhost', 9999), TestServerHandler)
    httpd.serve_forever() # test runner will kill us

  def server_func(dir, q):
    class TestServerHandler(BaseHTTPServer.BaseHTTPRequestHandler):
      def do_GET(s):
        if 'report_' in s.path:
          q.put(s.path)
        else:
          filename = s.path[1:]
          if os.path.exists(filename):
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            s.wfile.write(open(filename).read())
            s.wfile.close()
          else:
            s.send_response(500)
            s.send_header("Content-type", "text/html")
            s.end_headers()
    os.chdir(dir)
    httpd = BaseHTTPServer.HTTPServer(('localhost', 8888), TestServerHandler)
    httpd.serve_forever() # test runner will kill us

  class browser(RunnerCore):
    def __init__(self, *args, **kwargs):
      super(browser, self).__init__(*args, **kwargs)

      if hasattr(browser, 'harness_server'): return
      browser.harness_queue = multiprocessing.Queue()
      browser.harness_server = multiprocessing.Process(target=harness_server_func, args=(browser.harness_queue,))
      browser.harness_server.start()
      print '[Browser harness server on process %d]' % browser.harness_server.pid
      webbrowser.open_new('http://localhost:9999/run_harness')

    def __del__(self):
      if not hasattr(browser, 'harness_server'): return

      browser.harness_server.terminate()
      delattr(browser, 'harness_server')
      print '[Browser harness server terminated]'
      # On Windows, shutil.rmtree() in tearDown() raises this exception if we do not wait a bit:
      # WindowsError: [Error 32] The process cannot access the file because it is being used by another process.
      time.sleep(0.1)
      
    def run_browser(self, html_file, message, expectedResult=None):
      if expectedResult is not None:
        try:
          queue = multiprocessing.Queue()
          server = multiprocessing.Process(target=functools.partial(server_func, self.get_dir()), args=(queue,))
          server.start()
          browser.harness_queue.put('http://localhost:8888/' + html_file)
          output = '[no http server activity]'
          start = time.time()
          while time.time() - start < 60:
            if not queue.empty():
              output = queue.get()
              break
            time.sleep(0.1)

          self.assertIdentical(expectedResult, output)
        finally:
          server.terminate()
          time.sleep(0.1) # see comment about Windows above
      else:
        webbrowser.open_new(os.path.abspath(html_file))
        print 'A web browser window should have opened a page containing the results of a part of this test.'
        print 'You need to manually look at the page to see that it works ok: ' + message
        print '(sleeping for a bit to keep the directory alive for the web browser..)'
        time.sleep(5)
        print '(moving on..)'

    def with_report_result(self, code):
      return code.replace('REPORT_RESULT();', '''
          char output[1000];
          sprintf(output, 
                  "xhr = new XMLHttpRequest();"
                  "xhr.open('GET', 'http://localhost:8888/report_result?%d');"
                  "xhr.send();", result);
          emscripten_run_script(output);
          emscripten_run_script("setTimeout(function() { window.close() }, 1000)");
''')

    def reftest(self, expected):
      basename = os.path.basename(expected)
      shutil.copyfile(expected, os.path.join(self.get_dir(), basename))
      open(os.path.join(self.get_dir(), 'reftest.js'), 'w').write('''
        function doReftest() {
          if (doReftest.done) return;
          doReftest.done = true;
          var img = new Image();
          img.onload = function() {
            assert(img.width == Module.canvas.width, 'Invalid width: ' + Module.canvas.width + ', should be ' + img.width);
            assert(img.height == Module.canvas.height, 'Invalid height: ' + Module.canvas.height + ', should be ' + img.height);

            var canvas = document.createElement('canvas');
            canvas.width = img.width;
            canvas.height = img.height;
            var ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            var expected = ctx.getImageData(0, 0, img.width, img.height).data;

            var actualUrl = Module.canvas.toDataURL();
            var actualImage = new Image();
            actualImage.onload = function() {
              var actualCanvas = document.createElement('canvas');
              actualCanvas.width = actualImage.width;
              actualCanvas.height = actualImage.height;
              var actualCtx = actualCanvas.getContext('2d');
              actualCtx.drawImage(actualImage, 0, 0);
              var actual = actualCtx.getImageData(0, 0, actualImage.width, actualImage.height).data;

              var total = 0;
              for (var x = 0; x < img.width; x++) {
                for (var y = 0; y < img.height; y++) {
                  total += Math.abs(expected[y*img.width*4 + x*4 + 0] - actual[y*img.width*4 + x*4 + 0]);
                  total += Math.abs(expected[y*img.width*4 + x*4 + 1] - actual[y*img.width*4 + x*4 + 1]);
                  total += Math.abs(expected[y*img.width*4 + x*4 + 2] - actual[y*img.width*4 + x*4 + 2]);
                }
              }
              var wrong = Math.floor(total / (img.width*img.height*3)); // floor, to allow some margin of error for antialiasing

              xhr = new XMLHttpRequest();
              xhr.open('GET', 'http://localhost:8888/report_result?' + wrong);
              xhr.send();
              setTimeout(function() { window.close() }, 1000);
            };
            actualImage.src = actualUrl;
          }
          img.src = '%s';
        };
        Module['postRun'] = doReftest;
        Module['preRun'].push(function() {
          setTimeout(doReftest, 1000); // if run() throws an exception and postRun is not called, this will kick in
        });
''' % basename)

    def test_html(self):
      # test HTML generation.
      self.reftest(path_from_root('tests', 'htmltest.png'))
      output = Popen(['python', EMCC, path_from_root('tests', 'hello_world_sdl.cpp'), '-o', 'something.html',  '--pre-js', 'reftest.js']).communicate()
      self.run_browser('something.html', 'You should see "hello, world!" and a colored cube.', '/report_result?0')

    def build_native_lzma(self):
      lzma_native = path_from_root('third_party', 'lzma.js', 'lzma-native')
      if os.path.isfile(lzma_native) and os.access(lzma_native, os.X_OK): return

      cwd = os.getcwd()
      try:
        os.chdir(path_from_root('third_party', 'lzma.js'))
        Popen(['sh', './doit.sh']).communicate()
      finally:
        os.chdir(cwd)

    def test_compression(self):
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
        #include <stdio.h>
        #include <emscripten.h>
        int main() {
          printf("hello compressed world\n");
          int result = 1;
          REPORT_RESULT();
          return 0;
        }
      '''))

      self.build_native_lzma()
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-o', 'page.html',
             '--compression', '%s,%s,%s' % (path_from_root('third_party', 'lzma.js', 'lzma-native'),
                                            path_from_root('third_party', 'lzma.js', 'lzma-decoder.js'),
                                            'LZMA.decompress')]).communicate()
      assert os.path.exists(os.path.join(self.get_dir(), 'page.js')), 'must be side js'
      assert os.path.exists(os.path.join(self.get_dir(), 'page.js.compress')), 'must be side compressed js'
      assert os.stat(os.path.join(self.get_dir(), 'page.js')).st_size > os.stat(os.path.join(self.get_dir(), 'page.js.compress')).st_size, 'compressed file must be smaller'
      shutil.move(os.path.join(self.get_dir(), 'page.js'), 'page.js.renamedsoitcannotbefound');
      self.run_browser('page.html', '', '/report_result?1')

    def test_preload_file(self):
      open(os.path.join(self.get_dir(), 'somefile.txt'), 'w').write('''load me right before running the code please''')
      def make_main(path):
        print path
        open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
          #include <stdio.h>
          #include <string.h>
          #include <emscripten.h>
          int main() {
            FILE *f = fopen("%s", "r");
            char buf[100];
            fread(buf, 1, 20, f);
            buf[20] = 0;
            fclose(f);
            printf("|%%s|\n", buf);

            int result = !strcmp("load me right before", buf);
            REPORT_RESULT();
            return 0;
          }
        ''' % path))

      make_main('somefile.txt')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', 'somefile.txt', '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

      # By absolute path

      make_main(os.path.join(self.get_dir(), 'somefile.txt'))
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', os.path.join(self.get_dir(), 'somefile.txt'), '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

      # Should still work with -o subdir/..

      make_main(os.path.join(self.get_dir(), 'somefile.txt'))
      try:
        os.mkdir(os.path.join(self.get_dir(), 'dirrey'))
      except:
        pass
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', os.path.join(self.get_dir(), 'somefile.txt'), '-o', 'dirrey/page.html']).communicate()
      self.run_browser('dirrey/page.html', 'You should see |load me right before|.', '/report_result?1')

      # With FS.preloadFile

      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        Module.preRun = function() {
          FS.createPreloadedFile('/', 'someotherfile.txt', 'somefile.txt', true, false);
        };
      ''')
      make_main('someotherfile.txt')
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    def test_multifile(self):
      # a few files inside a directory
      self.clear()
      os.makedirs(os.path.join(self.get_dir(), 'subdirr'));
      os.makedirs(os.path.join(self.get_dir(), 'subdirr', 'moar'));
      open(os.path.join(self.get_dir(), 'subdirr', 'data1.txt'), 'w').write('''1214141516171819''')
      open(os.path.join(self.get_dir(), 'subdirr', 'moar', 'data2.txt'), 'w').write('''3.14159265358979''')
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
        #include <stdio.h>
        #include <string.h>
        #include <emscripten.h>
        int main() {
          char buf[17];

          FILE *f = fopen("subdirr/data1.txt", "r");
          fread(buf, 1, 16, f);
          buf[16] = 0;
          fclose(f);
          printf("|%s|\n", buf);
          int result = !strcmp("1214141516171819", buf);

          FILE *f2 = fopen("subdirr/moar/data2.txt", "r");
          fread(buf, 1, 16, f2);
          buf[16] = 0;
          fclose(f2);
          printf("|%s|\n", buf);
          result = result && !strcmp("3.14159265358979", buf);

          REPORT_RESULT();
          return 0;
        }
      '''))

      # by individual files
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', 'subdirr/data1.txt', '--preload-file', 'subdirr/moar/data2.txt', '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should see two cool numbers', '/report_result?1')
      os.remove('page.html')

      # by directory, and remove files to make sure
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', 'subdirr', '-o', 'page.html']).communicate()
      shutil.rmtree(os.path.join(self.get_dir(), 'subdirr'))
      self.run_browser('page.html', 'You should see two cool numbers', '/report_result?1')

    def test_compressed_file(self):
      open(os.path.join(self.get_dir(), 'datafile.txt'), 'w').write('compress this please' + (2000*'.'))
      open(os.path.join(self.get_dir(), 'datafile2.txt'), 'w').write('moar' + (100*'!'))
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
        #include <stdio.h>
        #include <string.h>
        #include <emscripten.h>
        int main() {
          char buf[21];
          FILE *f = fopen("datafile.txt", "r");
          fread(buf, 1, 20, f);
          buf[20] = 0;
          fclose(f);
          printf("file says: |%s|\n", buf);
          int result = !strcmp("compress this please", buf);
          FILE *f2 = fopen("datafile2.txt", "r");
          fread(buf, 1, 5, f2);
          buf[5] = 0;
          fclose(f2);
          result = result && !strcmp("moar!", buf);
          printf("file 2 says: |%s|\n", buf);
          REPORT_RESULT();
          return 0;
        }
      '''))

      self.build_native_lzma()
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-o', 'page.html', '--preload-file', 'datafile.txt', '--preload-file', 'datafile2.txt',
             '--compression', '%s,%s,%s' % (path_from_root('third_party', 'lzma.js', 'lzma-native'),
                                            path_from_root('third_party', 'lzma.js', 'lzma-decoder.js'),
                                            'LZMA.decompress')]).communicate()
      assert os.path.exists(os.path.join(self.get_dir(), 'datafile.txt')), 'must be data file'
      assert os.path.exists(os.path.join(self.get_dir(), 'page.data.compress')), 'must be data file in compressed form'
      assert os.stat(os.path.join(self.get_dir(), 'page.js')).st_size != os.stat(os.path.join(self.get_dir(), 'page.js.compress')).st_size, 'compressed file must be different'
      shutil.move(os.path.join(self.get_dir(), 'datafile.txt'), 'datafile.txt.renamedsoitcannotbefound');
      self.run_browser('page.html', '', '/report_result?1')

    def test_sdl_image(self):
      # load an image file, get pixel data. Also O2 coverage for --preload-file
      shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpg'))
      open(os.path.join(self.get_dir(), 'sdl_image.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_image.c'), '-O2', '--preload-file', 'screenshot.jpg', '-o', 'page.html']).communicate()
      self.run_browser('page.html', '', '/report_result?600')

    def test_sdl_image_compressed(self):
      for image, width in [(path_from_root('tests', 'screenshot2.png'), 300),
                           (path_from_root('tests', 'screenshot.jpg'), 600)]:
        self.clear()
        print image

        basename = os.path.basename(image)
        shutil.copyfile(image, os.path.join(self.get_dir(), basename))
        open(os.path.join(self.get_dir(), 'sdl_image.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()).replace('screenshot.jpg', basename))

        self.build_native_lzma()
        Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_image.c'), '--preload-file', basename, '-o', 'page.html',
               '--compression', '%s,%s,%s' % (path_from_root('third_party', 'lzma.js', 'lzma-native'),
                                              path_from_root('third_party', 'lzma.js', 'lzma-decoder.js'),
                                              'LZMA.decompress')]).communicate()
        shutil.move(os.path.join(self.get_dir(), basename), basename + '.renamedsoitcannotbefound');
        self.run_browser('page.html', '', '/report_result?' + str(width))

    def test_sdl_canvas(self):
      open(os.path.join(self.get_dir(), 'sdl_canvas.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_canvas.c')).read()))

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_canvas.c'), '-o', 'page.html']).communicate()
      self.run_browser('page.html', '', '/report_result?1')

    def test_sdl_key(self):
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        Module.postRun = function() {
          function doOne() {
            _one();
            setTimeout(doOne, 1000/60);
          }
          setTimeout(doOne, 1000/60);
        }

        function simulateKeyEvent(c) {
          var event = document.createEvent("KeyboardEvent");
          event.initKeyEvent("keydown", true, true, window,
                             0, 0, 0, 0,
                             c, c);
          document.dispatchEvent(event);
          var event2 = document.createEvent("KeyboardEvent");
          event2.initKeyEvent("keyup", true, true, window,
                             0, 0, 0, 0,
                             c, c);
          document.dispatchEvent(event2);
        }
      ''')
      open(os.path.join(self.get_dir(), 'sdl_key.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_key.c')).read()))

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_key.c'), '-o', 'page.html', '--pre-js', 'pre.js']).communicate()
      self.run_browser('page.html', '', '/report_result?510510')

    def test_sdl_mouse(self):
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        function simulateMouseEvent(x, y, button) {
          var event = document.createEvent("MouseEvents");
          if (button >= 0) {
            var event1 = document.createEvent("MouseEvents");
            event1.initMouseEvent('mousedown', true, true, window,
                       1, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y,
                       0, 0, 0, 0,
                       button, null);
            Module['canvas'].dispatchEvent(event1);
            var event2 = document.createEvent("MouseEvents");
            event2.initMouseEvent('mouseup', true, true, window,
                       1, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y,
                       0, 0, 0, 0,
                       button, null);
            Module['canvas'].dispatchEvent(event2);
          } else {
            var event1 = document.createEvent("MouseEvents");
            event1.initMouseEvent('mousemove', true, true, window,
                       0, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y,
                       0, 0, 0, 0,
                       0, null);
            Module['canvas'].dispatchEvent(event1);
          }
        }
        window['simulateMouseEvent'] = simulateMouseEvent;
      ''')
      open(os.path.join(self.get_dir(), 'sdl_mouse.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_mouse.c')).read()))

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_mouse.c'), '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js']).communicate()
      self.run_browser('page.html', '', '/report_result?740')

    def test_sdl_audio(self):
      shutil.copyfile(path_from_root('tests', 'sounds', 'alarmvictory_1.ogg'), os.path.join(self.get_dir(), 'sound.ogg'))
      shutil.copyfile(path_from_root('tests', 'sounds', 'alarmcreatemiltaryfoot_1.wav'), os.path.join(self.get_dir(), 'sound2.wav'))
      open(os.path.join(self.get_dir(), 'bad.ogg'), 'w').write('I claim to be audio, but am lying')
      open(os.path.join(self.get_dir(), 'sdl_audio.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio.c')).read()))

      # use closure to check for a possible bug with closure minifying away newer Audio() attributes
      Popen(['python', EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio.c'), '--preload-file', 'sound.ogg', '--preload-file', 'sound2.wav', '--preload-file', 'bad.ogg', '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play", "_play2"]']).communicate()
      self.run_browser('page.html', '', '/report_result?1')

    def test_sdl_audio_quickload(self):
      open(os.path.join(self.get_dir(), 'sdl_audio_quickload.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio_quickload.c')).read()))

      # use closure to check for a possible bug with closure minifying away newer Audio() attributes
      Popen(['python', EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio_quickload.c'), '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play"]']).communicate()
      self.run_browser('page.html', '', '/report_result?1')

    def test_sdl_gl_read(self):
      # SDL, OpenGL, readPixels
      open(os.path.join(self.get_dir(), 'sdl_gl_read.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_gl_read.c')).read()))
      Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_gl_read.c'), '-o', 'something.html']).communicate()
      self.run_browser('something.html', '.', '/report_result?1')

    def test_sdl_ogl(self):
      # SDL, OpenGL, textures, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-gray-purple.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_ogl.c'), '-O2', '--minify', '0', '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with gray at the top.', '/report_result?0')

    def test_sdl_ogl_defaultmatrixmode(self):
      # SDL, OpenGL, textures, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-gray-purple.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_ogl_defaultMatrixMode.c'), '--minify', '0', '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with gray at the top.', '/report_result?0')

    def test_sdl_ogl_p(self):
      # Immediate mode with pointers
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-gray.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_ogl_p.c'), '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with gray at the top.', '/report_result?0')

    def test_sdl_fog_simple(self):
      # SDL, OpenGL, textures, fog, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-fog-simple.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_fog_simple.c'), '-O2', '--minify', '0', '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with fog.', '/report_result?0')

    def test_sdl_fog_negative(self):
      # SDL, OpenGL, textures, fog, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-fog-negative.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_fog_negative.c'), '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with fog.', '/report_result?0')

    def test_sdl_fog_density(self):
      # SDL, OpenGL, textures, fog, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-fog-density.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_fog_density.c'), '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with fog.', '/report_result?0')

    def test_sdl_fog_exp2(self):
      # SDL, OpenGL, textures, fog, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-fog-exp2.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_fog_exp2.c'), '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with fog.', '/report_result?0')

    def test_sdl_fog_linear(self):
      # SDL, OpenGL, textures, fog, immediate mode. Closure for more coverage
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.reftest(path_from_root('tests', 'screenshot-fog-linear.png'))
      Popen(['python', EMCC, path_from_root('tests', 'sdl_fog_linear.c'), '-o', 'something.html', '--pre-js', 'reftest.js', '--preload-file', 'screenshot.png']).communicate()
      self.run_browser('something.html', 'You should see an image with fog.', '/report_result?0')

    def test_worker(self):
      # Test running in a web worker
      output = Popen(['python', EMCC, path_from_root('tests', 'hello_world_worker.cpp'), '-o', 'worker.js'], stdout=PIPE, stderr=PIPE).communicate()
      assert len(output[0]) == 0, output[0]
      assert os.path.exists('worker.js'), output
      self.assertContained('you should not see this text when in a worker!', run_js('worker.js')) # code should run standalone
      html_file = open('main.html', 'w')
      html_file.write('''
        <html>
        <body>
          Worker Test
          <script>
            var worker = new Worker('worker.js');
            worker.onmessage = function(event) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', 'http://localhost:8888/report_result?' + event.data);
              xhr.send();
              setTimeout(function() { window.close() }, 1000);
            };
          </script>
        </body>
        </html>
      ''')
      html_file.close()
      self.run_browser('main.html', 'You should see that the worker was called, and said "hello from worker!"', '/report_result?hello%20from%20worker!')

    def test_glgears(self):
      self.reftest(path_from_root('tests', 'gears.png'))
      Popen(['python', EMCC, path_from_root('tests', 'hello_world_gles.c'), '-o', 'something.html',
                                           '-DHAVE_BUILTIN_SINCOS', '--pre-js', 'reftest.js']).communicate()
      self.run_browser('something.html', 'You should see animating gears.', '/report_result?0')

    def test_glgears_animation(self):
      Popen(['python', EMCC, path_from_root('tests', 'hello_world_gles.c'), '-o', 'something.html',
                                           '-DHAVE_BUILTIN_SINCOS',
                                           '--shell-file', path_from_root('tests', 'hello_world_gles_shell.html')]).communicate()
      self.run_browser('something.html', 'You should see animating gears.', '/report_gl_result?true')

    def test_glgears_bad(self):
      # Make sure that OpenGL ES is not available if typed arrays are not used
      Popen(['python', EMCC, path_from_root('tests', 'hello_world_gles.c'), '-o', 'something.html',
                                           '-DHAVE_BUILTIN_SINCOS',
                                           '-s', 'USE_TYPED_ARRAYS=0',
                                           '--shell-file', path_from_root('tests', 'hello_world_gles_shell.html')]).communicate()
      self.run_browser('something.html', 'You should not see animating gears.', '/report_gl_result?false')

    def test_glgears_deriv(self):
      self.reftest(path_from_root('tests', 'gears.png'))
      Popen(['python', EMCC, path_from_root('tests', 'hello_world_gles_deriv.c'), '-o', 'something.html',
                                           '-DHAVE_BUILTIN_SINCOS', '--pre-js', 'reftest.js']).communicate()
      self.run_browser('something.html', 'You should see animating gears.', '/report_result?0')
      src = open('something.html').read()
      assert 'gl-matrix' not in src, 'Should not include glMatrix when not needed'

    def test_glbook(self):
      programs = self.get_library('glbook', [
        os.path.join('Chapter_2', 'Hello_Triangle', 'CH02_HelloTriangle.bc'),
        os.path.join('Chapter_8', 'Simple_VertexShader', 'CH08_SimpleVertexShader.bc'),
        os.path.join('Chapter_9', 'Simple_Texture2D', 'CH09_SimpleTexture2D.bc'),
        os.path.join('Chapter_9', 'Simple_TextureCubemap', 'CH09_TextureCubemap.bc'),
        os.path.join('Chapter_9', 'TextureWrap', 'CH09_TextureWrap.bc'),
        os.path.join('Chapter_10', 'MultiTexture', 'CH10_MultiTexture.bc'),
        os.path.join('Chapter_13', 'ParticleSystem', 'CH13_ParticleSystem.bc'),
      ], configure=None)
      for program in programs:
        print program
        basename = os.path.basename(program)
        args = []
        if basename == 'CH10_MultiTexture.bc':
          shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_10', 'MultiTexture', 'basemap.tga'), os.path.join(self.get_dir(), 'basemap.tga'))
          shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_10', 'MultiTexture', 'lightmap.tga'), os.path.join(self.get_dir(), 'lightmap.tga'))
          args = ['--preload-file', 'basemap.tga', '--preload-file', 'lightmap.tga']
        elif basename == 'CH13_ParticleSystem.bc':
          shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_13', 'ParticleSystem', 'smoke.tga'), os.path.join(self.get_dir(), 'smoke.tga'))
          args = ['--preload-file', 'smoke.tga', '-O2'] # test optimizations and closure here as well for more coverage

        self.reftest(path_from_root('tests', 'glbook', basename.replace('.bc', '.png')))
        Popen(['python', EMCC, program, '-o', 'program.html', '--pre-js', 'reftest.js'] + args).communicate()
        self.run_browser('program.html', '', '/report_result?0')

    def btest(self, filename, expected=None, reference=None, reference_slack=0, args=[]): # TODO: use in all other tests
      if not reference:
        open(os.path.join(self.get_dir(), filename), 'w').write(self.with_report_result(open(path_from_root('tests', filename)).read()))
      else:
        expected = [str(i) for i in range(0, reference_slack+1)]
        shutil.copyfile(path_from_root('tests', filename), os.path.join(self.get_dir(), filename))
        self.reftest(path_from_root('tests', reference))
        args = args + ['--pre-js', 'reftest.js']
      Popen(['python', EMCC, os.path.join(self.get_dir(), filename), '-o', 'test.html'] + args).communicate()
      if type(expected) is str: expected = [expected]
      self.run_browser('test.html', '.', ['/report_result?' + e for e in expected])

    def test_emscripten_api(self):
      self.btest('emscripten_api_browser.cpp', '1')

    def test_emscripten_fs_api(self):
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png')) # preloaded *after* run
      self.btest('emscripten_fs_api_browser.cpp', '1')

    def test_gc(self):
      self.btest('browser_gc.cpp', '1')

    def test_glshaderinfo(self):
      self.btest('glshaderinfo.cpp', '1')

    def test_sdlglshader(self):
      self.btest('sdlglshader.c', reference='sdlglshader.png', args=['--closure', '1'])

    def test_gl_ps(self):
      # pointers and a shader
      shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
      self.btest('gl_ps.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png'])

    def test_matrix_identity(self):
      self.btest('gl_matrix_identity.c', expected=['-1882984448', '460451840'])

    def test_cubegeom_pre(self):
      self.btest('cubegeom_pre.c', expected='-1472804742')

    def test_cubegeom_pre2(self):
      self.btest('cubegeom_pre2.c', expected='-1472804742', args=['-s', 'GL_DEBUG=1']) # some coverage for GL_DEBUG not breaking the build

    def test_cubegeom_pre3(self):
      self.btest('cubegeom_pre3.c', expected='-1472804742')

    def test_cubegeom(self):
      self.btest('cubegeom.c', expected=['188641320', '1522377227'])

    def test_cubegeom_color(self):
      self.btest('cubegeom_color.c', expected='588472350')

    def test_cubegeom_normal(self):
      self.btest('cubegeom_normal.c', expected='752917084')

    def test_cubegeom_normal_dap(self): # draw is given a direct pointer to clientside memory, no element array buffer
      self.btest('cubegeom_normal_dap.c', expected='752917084')

    def test_cubegeom_normal_dap_far(self): # indices do nto start from 0
      self.btest('cubegeom_normal_dap_far.c', expected='752917084')

    def test_cubegeom_normal_dap_far_range(self): # glDrawRangeElements
      self.btest('cubegeom_normal_dap_far_range.c', expected='752917084')

    def test_cubegeom_normal_dap_far_glda(self): # use glDrawArrays
      self.btest('cubegeom_normal_dap_far_glda.c', expected='-218745386')

    def test_cubegeom_normal_dap_far_glda_quad(self): # with quad
      self.btest('cubegeom_normal_dap_far_glda_quad.c', expected='1757386625')

    def test_cubegeom_mt(self):
      self.btest('cubegeom_mt.c', expected='-457159152') # multitexture

    def test_cubegeom_color2(self):
      self.btest('cubegeom_color2.c', expected='1121999515')

    def test_cubegeom_texturematrix(self):
      self.btest('cubegeom_texturematrix.c', expected='1297500583')

    def test_cubegeom_fog(self):
      self.btest('cubegeom_fog.c', expected='1617140399')

    def test_cube_explosion(self):
      self.btest('cube_explosion.c', expected='667220544')

    def test_sdl_canvas_palette(self):
      self.btest('sdl_canvas_palette.c', reference='sdl_canvas_palette.png')

    def test_sdl_maprgba(self):
      self.btest('sdl_maprgba.c', reference='sdl_maprgba.png', reference_slack=3)

    def zzztest_sdl_canvas_palette_2(self): # XXX disabled until we have proper automation
      open(os.path.join(self.get_dir(), 'sdl_canvas_palette_2.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_canvas_palette_2.c')).read()))
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('Module[\'preRun\'] = function() { SDL.defaults.copyOnLock = false }')

      Popen(['python', EMCC, os.path.join(self.get_dir(), 'sdl_canvas_palette_2.c'), '-o', 'page.html', '--pre-js', 'pre.js']).communicate()
      self.run_browser('page.html', '')

    def test_s3tc(self):
      shutil.copyfile(path_from_root('tests', 'screenshot.dds'), os.path.join(self.get_dir(), 'screenshot.dds'))
      self.btest('s3tc.c', reference='s3tc.png', args=['--preload-file', 'screenshot.dds'])

    def test_s3tc_crunch(self):
      shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
      shutil.copyfile(path_from_root('tests', 'bloom.dds'), 'bloom.dds')
      shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
      Popen(['python', FILE_PACKAGER, 'test.data', '--pre-run', '--crunch', '--preload', 'ship.dds', 'bloom.dds', 'water.dds'], stdout=open('pre.js', 'w')).communicate()
      assert os.stat('test.data').st_size < 0.5*(os.stat('ship.dds').st_size+os.stat('bloom.dds').st_size+os.stat('water.dds').st_size), 'Compressed should be smaller than dds'
      shutil.move('ship.dds', 'ship.donotfindme.dds') # make sure we load from the compressed
      shutil.move('bloom.dds', 'bloom.donotfindme.dds') # make sure we load from the compressed
      shutil.move('water.dds', 'water.donotfindme.dds') # make sure we load from the compressed
      self.btest('s3tc_crunch.c', reference='s3tc_crunch.png', args=['--pre-js', 'pre.js'])

    def test_aniso(self):
      shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
      self.btest('aniso.c', reference='aniso.png', reference_slack=2, args=['--preload-file', 'water.dds'])

    def test_tex_nonbyte(self):
      self.btest('tex_nonbyte.c', reference='tex_nonbyte.png')

    def test_float_tex(self):
      self.btest('float_tex.cpp', reference='float_tex.png')

    def test_pre_run_deps(self):
      # Adding a dependency in preRun will delay run
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        Module.preRun = function() {
          addRunDependency();
          Module.print('preRun called, added a dependency...');
          setTimeout(function() {
            Module.okk = 10;
            removeRunDependency()
          }, 2000);
        };
      ''')
      self.btest('pre_run_deps.cpp', expected='10', args=['--pre-js', 'pre.js'])

elif 'benchmark' in str(sys.argv):
  # Benchmarks. Run them with argument |benchmark|. To run a specific test, do
  # |benchmark.test_X|.

  fingerprint = [time.asctime()]
  try:
    fingerprint.append('em: ' + Popen(['git', 'show'], stdout=PIPE).communicate()[0].split('\n')[0])
  except:
    pass
  try:
    d = os.getcwd()
    os.chdir(os.path.expanduser('~/Dev/mozilla-central'))
    fingerprint.append('sm: ' + filter(lambda line: 'changeset' in line, 
                                       Popen(['hg', 'tip'], stdout=PIPE).communicate()[0].split('\n'))[0])
  except:
    pass
  finally:
    os.chdir(d)
  fingerprint.append('llvm: ' + LLVM_ROOT)
  print 'Running Emscripten benchmarks... [ %s ]' % ' | '.join(fingerprint)

  sys.argv = filter(lambda x: x != 'benchmark', sys.argv)

  assert(os.path.exists(CLOSURE_COMPILER))

  try:
    index = SPIDERMONKEY_ENGINE.index("options('strict')")
    SPIDERMONKEY_ENGINE = SPIDERMONKEY_ENGINE[:index-1] + SPIDERMONKEY_ENGINE[index+1:] # closure generates non-strict
  except:
    pass

  Building.COMPILER = CLANG

  # Pick the JS engine to benchmark. If you specify one, it will be picked. For example, python tests/runner.py benchmark SPIDERMONKEY_ENGINE
  JS_ENGINE = JS_ENGINES[0]
  for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if not arg.startswith('benchmark.test_'):
      JS_ENGINE = eval(arg)
      sys.argv[i] = None
  sys.argv = filter(lambda arg: arg is not None, sys.argv)
  print 'Benchmarking JS engine:', JS_ENGINE

  Building.COMPILER_TEST_OPTS = []

  TEST_REPS = 10
  TOTAL_TESTS = 9

  tests_done = 0
  total_times = map(lambda x: 0., range(TOTAL_TESTS))
  total_native_times = map(lambda x: 0., range(TOTAL_TESTS))

  class benchmark(RunnerCore):
    def print_stats(self, times, native_times, last=False):
      mean = sum(times)/len(times)
      squared_times = map(lambda x: x*x, times)
      mean_of_squared = sum(squared_times)/len(times)
      std = math.sqrt(mean_of_squared - mean*mean)
      sorted_times = times[:]
      sorted_times.sort()
      median = sum(sorted_times[len(sorted_times)/2 - 1:len(sorted_times)/2 + 1])/2

      mean_native = sum(native_times)/len(native_times)
      squared_native_times = map(lambda x: x*x, native_times)
      mean_of_squared_native = sum(squared_native_times)/len(native_times)
      std_native = math.sqrt(mean_of_squared_native - mean_native*mean_native)
      sorted_native_times = native_times[:]
      sorted_native_times.sort()
      median_native = sum(sorted_native_times[len(sorted_native_times)/2 - 1:len(sorted_native_times)/2 + 1])/2

      final = mean / mean_native

      if last:
        norm = 0
        for i in range(len(times)):
          norm += times[i]/native_times[i]
        norm /= len(times)
        print
        print '  JavaScript: %.3f    Native: %.3f   Ratio:  %.3f  Normalized ratio: %.3f' % (mean, mean_native, final, norm)
        return

      print
      print '   JavaScript: mean: %.3f (+-%.3f) secs  median: %.3f  range: %.3f-%.3f  (noise: %3.3f%%)  (%d runs)' % (mean, std, median, min(times), max(times), 100*std/mean, TEST_REPS)
      print '   Native    : mean: %.3f (+-%.3f) secs  median: %.3f  range: %.3f-%.3f  (noise: %3.3f%%)  JS is %.2f X slower' % (mean_native, std_native, median_native, min(native_times), max(native_times), 100*std_native/mean_native, final)

    def do_benchmark(self, src, args=[], expected_output='FAIL', emcc_args=[]):
      dirname = self.get_dir()
      filename = os.path.join(dirname, 'src.cpp')
      f = open(filename, 'w')
      f.write(src)
      f.close()
      final_filename = os.path.join(dirname, 'src.js')

      try_delete(final_filename)
      output = Popen(['python', EMCC, filename, '-O3',
                      #'-O2', '-s', 'INLINING_LIMIT=0', '-s', 'DOUBLE_MODE=0', '-s', 'PRECISE_I64_MATH=0',
                      '-s', 'TOTAL_MEMORY=100*1024*1024', '-s', 'FAST_MEMORY=10*1024*1024',
                      '-o', final_filename] + emcc_args, stdout=PIPE, stderr=self.stderr_redirect).communicate()
      assert os.path.exists(final_filename), 'Failed to compile file: ' + output[0]

      # Run JS
      global total_times, tests_done
      times = []
      for i in range(TEST_REPS):
        start = time.time()
        js_output = self.run_generated_code(JS_ENGINE, final_filename, args, check_timeout=False)
        curr = time.time()-start
        times.append(curr)
        total_times[tests_done] += curr
        if i == 0:
          # Sanity check on output
          self.assertContained(expected_output, js_output)

      # Run natively
      self.build_native(filename)
      global total_native_times
      native_times = []
      for i in range(TEST_REPS):
        start = time.time()
        self.run_native(filename, args)
        curr = time.time()-start
        native_times.append(curr)
        total_native_times[tests_done] += curr

      self.print_stats(times, native_times)

      tests_done += 1
      if tests_done == TOTAL_TESTS:
        print 'Total stats:',
        self.print_stats(total_times, total_native_times, last=True)

    def test_primes(self):
      src = '''
        #include<stdio.h>
        #include<math.h>
        int main() {
          int primes = 0, curri = 2;
          while (primes < 100000) {
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
          printf("lastprime: %d.\\n", curri-1);
          return 1;
        }
      '''
      self.do_benchmark(src, [], 'lastprime: 1297001.')

    def test_memops(self):
      src = '''
        #include<stdio.h>
        #include<string.h>
        #include<stdlib.h>
        int main() {
          int N = 1024*1024;
          int M = 190;
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
          return 1;
        }      
      '''
      self.do_benchmark(src, [], 'final: 720.')

    def zzztest_files(self):
      src = r'''
        #include<stdio.h>
        #include<stdlib.h>
        #include<assert.h>
        #include <unistd.h>

        int main() {
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
          return 1;
        }      
      '''
      self.do_benchmark(src, [], 'ok')

    def test_copy(self):
      src = r'''
        #include<stdio.h>
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
        int main() {
          int total = 0;
          for (int i = 0; i < 1250; i++) {
            for (int j = 0; j < 1000; j++) {
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
          return 1;
        }      
      '''
      self.do_benchmark(src, [], 'sum:9928\n', emcc_args=['-s', 'QUANTUM_SIZE=4', '-s', 'USE_TYPED_ARRAYS=2'])

    def test_fannkuch(self):
      src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read()
      self.do_benchmark(src, ['10'], 'Pfannkuchen(10) = 38.')

    def test_corrections(self):
      src = r'''
        #include<stdio.h>
        #include<math.h>
        int main() {
          int N = 4100;
          int M = 4100;
          unsigned int f = 0;
          unsigned short s = 0;
          for (int t = 0; t < M; t++) {
            for (int i = 0; i < N; i++) {
              f += i / ((t % 5)+1);
              if (f > 1000) f /= (t % 3)+1;
              if (i % 4 == 0) f += sqrtf(i) * (i % 8 == 0 ? 1 : -1);
              s += (short(f)*short(f)) % 256;
            }
          }
          printf("final: %d:%d.\n", f, s);
          return 1;
        }      
      '''
      self.do_benchmark(src, [], 'final: 826:14324.', emcc_args=['-s', 'CORRECT_SIGNS=1', '-s', 'CORRECT_OVERFLOWS=1', '-s', 'CORRECT_ROUNDINGS=1'])

    def fasta(self, double_rep):
      src = open(path_from_root('tests', 'fasta.cpp'), 'r').read().replace('double', double_rep)
      self.do_benchmark(src, ['2100000'], '''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA\nTCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACT\nAAAAATACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCAGCTACTCGGGAG\nGCTGAGGCAGGAGAATCGCTTGAACCCGGGAGGCGGAGGTTGCAGTGAGCCGAGATCGCG\nCCACTGCACTCCAGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAAGGCCGGGCGCGGT\nGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGATCACCTGAGGTCA\nGGAGTTCGAGACCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAATACAAAAA\nTTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCAGCTACTCGGGAGGCTGAGGCAGGAG\nAATCGCTTGAACCCGGGAGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCCA\nGCCTGGGCGA''')

    def test_fasta_float(self):
      self.fasta('float')

    def zzztest_fasta_double(self):
      self.fasta('double')

    def test_skinning(self):
      src = open(path_from_root('tests', 'skinning_test_no_simd.cpp'), 'r').read()
      self.do_benchmark(src, ['10000', '1000'], 'blah=0.000000')

    def test_dlmalloc(self):
      # XXX This seems to have regressed slightly with emcc. Are -g and the signs lines passed properly?
      src = open(path_from_root('system', 'lib', 'dlmalloc.c'), 'r').read() + '\n\n\n' + open(path_from_root('tests', 'dlmalloc_test.c'), 'r').read()
      self.do_benchmark(src, ['400', '400'], '*400,0*', emcc_args=['-g', '-s', 'CORRECT_SIGNS=2', '-s', 'CORRECT_SIGNS_LINES=[4820, 4195, 4250, 4203, 4209, 4239, 4231]'])

elif 'sanity' in str(sys.argv):

  # Run some sanity checks on the test runner and emcc.

  sys.argv = filter(lambda x: x != 'sanity', sys.argv)

  print
  print 'Running sanity checks.'
  print 'WARNING: This will modify %s, and in theory can break it although it should be restored properly. A backup will be saved in %s_backup' % (EM_CONFIG, EM_CONFIG)
  print

  assert os.path.exists(CONFIG_FILE), 'To run these tests, we need a (working!) %s file to already exist' % EM_CONFIG

  shutil.copyfile(CONFIG_FILE, CONFIG_FILE + '_backup')
  def restore():
    shutil.copyfile(CONFIG_FILE + '_backup', CONFIG_FILE)

  SANITY_FILE = CONFIG_FILE + '_sanity'

  def wipe():
    try_delete(CONFIG_FILE)
    try_delete(SANITY_FILE)

  commands = [[EMCC], ['python', path_from_root('tests', 'runner.py'), 'blahblah']]

  def mtime(filename):
    return os.stat(filename).st_mtime

  class sanity(RunnerCore):
    def setUp(self):
      wipe()

    def tearDown(self):
      restore()

    def do(self, command):
      if type(command) is not list:
        command = [command]
      if command[0] == EMCC:
        command = ['python'] + command

      return Popen(command, stdout=PIPE, stderr=STDOUT).communicate()[0]

    def check_working(self, command, expected=None):
      if type(command) is not list:
        command = [command]
      if expected is None:
        if command[0] == EMCC:
          expected = 'no input files'
        else:
          expected = "has no attribute 'blahblah'"

      output = self.do(command)
      self.assertContained(expected, output)
      return output

    def test_aaa_normal(self): # this should be the very first thing that runs. if this fails, everything else is irrelevant!
      for command in commands:
        # Your existing EM_CONFIG should work!
        restore()
        self.check_working(command)

    def test_firstrun(self):
      for command in commands:
        wipe()
        output = self.do(command)

        self.assertContained('Welcome to Emscripten!', output)
        self.assertContained('This is the first time any of the Emscripten tools has been run.', output)
        self.assertContained('A settings file has been copied to %s, at absolute path: %s' % (EM_CONFIG, CONFIG_FILE), output)
        self.assertContained('Please edit that file and change the paths to fit your system', output)
        self.assertContained('make sure LLVM_ROOT and NODE_JS are correct', output)
        self.assertContained('This command will now exit. When you are done editing those paths, re-run it.', output)
        assert output.split()[-1].endswith('===='), 'We should have stopped: ' + output
        assert (open(CONFIG_FILE).read() == open(path_from_root('settings.py')).read()), 'Settings should be copied from settings.py'

        # Second run, with bad EM_CONFIG
        for settings in ['blah', 'LLVM_ROOT="blah"; JS_ENGINES=[]; COMPILER_ENGINE=NODE_JS=SPIDERMONKEY_ENGINE=[]']:
          f = open(CONFIG_FILE, 'w')
          f.write(settings)
          f.close()
          output = self.do(command)

          if 'LLVM_ROOT' not in settings:
            self.assertContained('Error in evaluating %s' % EM_CONFIG, output)
          else:
            self.assertContained('FATAL', output) # sanity check should fail

    def test_closure_compiler(self):
      CLOSURE_FATAL = 'fatal: Closure compiler'
      CLOSURE_WARNING = 'WARNING: Closure compiler'

      # Sanity check should find closure
      restore()
      output = self.check_working(EMCC)
      self.assertNotContained(CLOSURE_FATAL, output)
      self.assertNotContained(CLOSURE_WARNING, output)

      # Append a bad path for closure, will warn
      f = open(CONFIG_FILE, 'a')
      f.write('CLOSURE_COMPILER = "/tmp/nowhere/nothingtoseehere/kjadsfkjwelkjsdfkqgas/nonexistent.txt"\n')
      f.close()
      output = self.check_working(EMCC, CLOSURE_WARNING)

      # And if you actually try to use the bad path, will be fatal
      f = open(CONFIG_FILE, 'a')
      f.write('CLOSURE_COMPILER = "/tmp/nowhere/nothingtoseehere/kjadsfkjwelkjsdfkqgas/nonexistent.txt"\n')
      f.close()
      output = self.check_working([EMCC, '-O2', 'tests/hello_world.cpp'], CLOSURE_FATAL)

      # With a working path, all is well
      restore()
      try_delete('a.out.js')
      output = self.check_working([EMCC, '-O2', 'tests/hello_world.cpp'], '')
      assert os.path.exists('a.out.js')

    def test_llvm(self):
      LLVM_WARNING = 'warning: LLVM version appears incorrect'

      restore()

      # Clang should report the version number we expect, and emcc should not warn
      assert ('clang version ' + '.'.join(map(str, EXPECTED_LLVM_VERSION))) in Popen([CLANG, '-v'], stderr=PIPE).communicate()[1]
      output = self.check_working(EMCC)
      assert LLVM_WARNING not in output, output

      # Fake a different llvm version
      restore()
      f = open(CONFIG_FILE, 'a')
      f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake') + '"')
      f.close()

      if not os.path.exists(path_from_root('tests', 'fake')):
        os.makedirs(path_from_root('tests', 'fake'))

      try:
        os.environ['EM_IGNORE_SANITY'] = '1'
        for x in range(-2, 3):
          for y in range(-2, 3):
            f = open(path_from_root('tests', 'fake', 'clang'), 'w')
            f.write('#!/bin/sh\n')
            f.write('echo "clang version %d.%d" 1>&2\n' % (EXPECTED_LLVM_VERSION[0] + x, EXPECTED_LLVM_VERSION[1] + y))
            f.close()
            shutil.copyfile(path_from_root('tests', 'fake', 'clang'), path_from_root('tests', 'fake', 'clang++'))
            os.chmod(path_from_root('tests', 'fake', 'clang'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
            os.chmod(path_from_root('tests', 'fake', 'clang++'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
            if x != 0 or y != 0:
              output = self.check_working(EMCC, LLVM_WARNING)
            else:
              output = self.check_working(EMCC)
              assert LLVM_WARNING not in output, output
      finally:
        del os.environ['EM_IGNORE_SANITY']

    def test_emcc(self):
      SANITY_MESSAGE = 'Emscripten: Running sanity checks'
      SANITY_FAIL_MESSAGE = 'sanity check failed to run'

      # emcc should check sanity if no ${EM_CONFIG}_sanity
      restore()
      time.sleep(0.1)
      assert not os.path.exists(SANITY_FILE) # restore is just the settings, not the sanity
      output = self.check_working(EMCC)
      self.assertContained(SANITY_MESSAGE, output)
      assert os.path.exists(SANITY_FILE) # EMCC should have checked sanity successfully
      assert mtime(SANITY_FILE) >= mtime(CONFIG_FILE)
      self.assertNotContained(SANITY_FAIL_MESSAGE, output)

      # emcc run again should not sanity check, because the sanity file is newer
      output = self.check_working(EMCC)
      self.assertNotContained(SANITY_MESSAGE, output)
      self.assertNotContained(SANITY_FAIL_MESSAGE, output)

      # But the test runner should
      output = self.check_working(commands[1])
      self.assertContained(SANITY_MESSAGE, output)
      self.assertNotContained(SANITY_FAIL_MESSAGE, output)

      # Make sure the test runner didn't do anything to the setup
      output = self.check_working(EMCC)
      self.assertNotContained(SANITY_MESSAGE, output)
      self.assertNotContained(SANITY_FAIL_MESSAGE, output)

      # emcc should also check sanity if the file is outdated
      time.sleep(0.1)
      restore()
      assert mtime(SANITY_FILE) < mtime(CONFIG_FILE)
      output = self.check_working(EMCC)
      self.assertContained(SANITY_MESSAGE, output)
      assert mtime(SANITY_FILE) >= mtime(CONFIG_FILE)
      self.assertNotContained(SANITY_FAIL_MESSAGE, output)

      # emcc should be configurable directly from EM_CONFIG without any config file
      restore()
      config = open(CONFIG_FILE, 'r').read()
      os.environ['EM_CONFIG'] = config
      wipe()
      dirname = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=TEMP_DIR)
      open(os.path.join(dirname, 'main.cpp'), 'w').write('''
        #include <stdio.h>
        int main() {
          printf("hello from emcc with no config file\\n");
          return 0;
        }
      ''')
      Popen(['python', EMCC, os.path.join(dirname, 'main.cpp'), '-o', os.path.join(dirname, 'a.out.js')]).communicate()
      self.assertContained('hello from emcc with no config file', run_js(os.path.join(dirname, 'a.out.js')))
      del os.environ['EM_CONFIG']
      shutil.rmtree(dirname)

    def test_emcc_caching(self):
      INCLUDING_MESSAGE = 'emcc: including X'
      BUILDING_MESSAGE = 'emcc: building X for cache'

      EMCC_CACHE = Cache.dirname

      restore()

      Cache.erase()
      assert not os.path.exists(EMCC_CACHE)

      try:
        emcc_debug = os.environ.get('EMCC_DEBUG')
        os.environ['EMCC_DEBUG'] ='1'

        # Building a file that doesn't need cached stuff should not trigger cache generation
        output = self.do([EMCC, path_from_root('tests', 'hello_world.cpp')])
        assert INCLUDING_MESSAGE.replace('X', 'dlmalloc') not in output
        assert BUILDING_MESSAGE.replace('X', 'dlmalloc') not in output
        self.assertContained('hello, world!', run_js('a.out.js'))
        assert not os.path.exists(EMCC_CACHE)
        try_delete('a.out.js')

        basebc_name = os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-0-basebc.bc')
        dcebc_name = os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-1-dce.bc')

        # Building a file that *does* need dlmalloc *should* trigger cache generation, but only the first time
        for filename, libname in [('hello_malloc.cpp', 'dlmalloc'), ('hello_libcxx.cpp', 'libcxx')]:
          for i in range(3):
            try_delete(basebc_name) # we might need to check this file later
            try_delete(dcebc_name) # we might need to check this file later
            output = self.do([EMCC, path_from_root('tests', filename)])
            assert INCLUDING_MESSAGE.replace('X', libname) in output
            if libname == 'dlmalloc':
              assert INCLUDING_MESSAGE.replace('X', 'libcxx') not in output # we don't need libcxx in this code
            else:
              assert INCLUDING_MESSAGE.replace('X', 'dlmalloc') in output # libcxx always forces inclusion of dlmalloc
            assert (BUILDING_MESSAGE.replace('X', libname) in output) == (i == 0), 'Must only build the first time'
            self.assertContained('hello, world!', run_js('a.out.js'))
            assert os.path.exists(EMCC_CACHE)
            assert os.path.exists(os.path.join(EMCC_CACHE, libname + '.bc'))
            if libname == 'libcxx':
              print os.stat(os.path.join(EMCC_CACHE, libname + '.bc')).st_size, os.stat(basebc_name).st_size, os.stat(dcebc_name).st_size
              assert os.stat(os.path.join(EMCC_CACHE, libname + '.bc')).st_size > 2000000, 'libc++ is big'
              assert os.stat(basebc_name).st_size > 2000000, 'libc++ is indeed big'
              assert os.stat(dcebc_name).st_size < 1500000, 'Dead code elimination must remove most of libc++'
      finally:
        if emcc_debug:
          os.environ['EMCC_DEBUG'] = emcc_debug

else:
  raise Exception('Test runner is confused: ' + str(sys.argv))

if __name__ == '__main__':
  sys.argv = [sys.argv[0]] + ['-v'] + sys.argv[1:] # Verbose output by default

  # Sanity checks

  check_sanity(force=True)

  total_engines = len(JS_ENGINES)
  JS_ENGINES = filter(check_engine, JS_ENGINES)
  if len(JS_ENGINES) == 0:
    print 'WARNING: None of the JS engines in JS_ENGINES appears to work.'
  elif len(JS_ENGINES) < total_engines:
    print 'WARNING: Not all the JS engines in JS_ENGINES appears to work, ignoring those.'

  # Go

  unittest.main()

