'''
Simple test runner

See settings.py file for options&params. Edit as needed.
'''

from subprocess import Popen, PIPE, STDOUT
import os, unittest, tempfile, shutil, time, inspect, sys, math, glob, tempfile, re

# Setup

abspath = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  return os.path.join(os.path.sep, *(abspath.split(os.sep)[:-1] + list(pathelems)))
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

# Sanity check for config

try:
  assert COMPILER_OPTS != None
except:
  raise Exception('Cannot find "COMPILER_OPTS" definition. Is ~/.emscripten set up properly? You may need to copy the template at ~/tests/settings.py into it.')

# Paths

EMSCRIPTEN = path_from_root('emscripten.py')
DEMANGLER = path_from_root('third_party', 'demangler.py')
NAMESPACER = path_from_root('tools', 'namespacer.py')
EMMAKEN = path_from_root('tools', 'emmaken.py')
AUTODEBUGGER = path_from_root('tools', 'autodebugger.py')
DFE = path_from_root('tools', 'dead_function_eliminator.py')

# Global cache for tests (we have multiple TestCase instances; this object lets them share data)

GlobalCache = {}

class Dummy: pass
Settings = Dummy()
Settings.saveJS = 0

# Core test runner class, shared between normal tests and benchmarks

class RunnerCore(unittest.TestCase):
  def tearDown(self):
    if Settings.saveJS:
      for name in os.listdir(self.get_dir()):
        if name.endswith(('.o.js', '.cc.js')):
          suff = '.'.join(name.split('.')[-2:])
          shutil.copy(os.path.join(self.get_dir(), name),
                      os.path.join(TEMP_DIR, self.id().replace('__main__.', '').replace('.test_', '.')+'.'+suff))

  def skip(self):
    print >> sys.stderr, '<skip> ',

  def get_dir(self):
    dirname = TEMP_DIR + '/tmp' # tempfile.mkdtemp(dir=TEMP_DIR)
    if not os.path.exists(dirname):
      os.makedirs(dirname)
    return dirname

  # Similar to LLVM::createStandardModulePasses()
  def pick_llvm_opts(self, optimization_level, optimize_size, allow_nonportable=False):
    global LLVM_OPT_OPTS
    LLVM_OPT_OPTS = []

    if optimization_level == 0: return

    if allow_nonportable:
      LLVM_OPT_OPTS.append('-O3')
      return

    # createStandardAliasAnalysisPasses
    #LLVM_OPT_OPTS.append('-tbaa')
    #LLVM_OPT_OPTS.append('-basicaa') # makes fannkuch slow but primes fast

    LLVM_OPT_OPTS.append('-globalopt')
    LLVM_OPT_OPTS.append('-ipsccp')
    LLVM_OPT_OPTS.append('-deadargelim')
    if allow_nonportable: LLVM_OPT_OPTS.append('-instcombine')
    LLVM_OPT_OPTS.append('-simplifycfg')

    LLVM_OPT_OPTS.append('-prune-eh')
    LLVM_OPT_OPTS.append('-inline')
    LLVM_OPT_OPTS.append('-functionattrs')
    if optimization_level > 2:
      LLVM_OPT_OPTS.append('-argpromotion')

    if allow_nonportable: LLVM_OPT_OPTS.append('-scalarrepl') # XXX Danger: Can turn a memcpy into something that violates the load-store
    #                                                         #             consistency hypothesis. See hashnum() in lua.
    #                                                         #             Note: this opt is of great importance for raytrace...
    if allow_nonportable: LLVM_OPT_OPTS.append('-early-cse') # ?
    LLVM_OPT_OPTS.append('-simplify-libcalls')
    LLVM_OPT_OPTS.append('-jump-threading')
    if allow_nonportable: LLVM_OPT_OPTS.append('-correlated-propagation') # ?
    LLVM_OPT_OPTS.append('-simplifycfg')
    if allow_nonportable: LLVM_OPT_OPTS.append('-instcombine')

    LLVM_OPT_OPTS.append('-tailcallelim')
    LLVM_OPT_OPTS.append('-simplifycfg')
    LLVM_OPT_OPTS.append('-reassociate')
    LLVM_OPT_OPTS.append('-loop-rotate')
    LLVM_OPT_OPTS.append('-licm')
    LLVM_OPT_OPTS.append('-loop-unswitch') # XXX should depend on optimize_size
    if allow_nonportable: LLVM_OPT_OPTS.append('-instcombine')
    LLVM_OPT_OPTS.append('-indvars')
    if allow_nonportable: LLVM_OPT_OPTS.append('-loop-idiom') # ?
    LLVM_OPT_OPTS.append('-loop-deletion')
    LLVM_OPT_OPTS.append('-loop-unroll')
    if allow_nonportable: LLVM_OPT_OPTS.append('-instcombine')
    if optimization_level > 1:
      if allow_nonportable: LLVM_OPT_OPTS.append('-gvn') # XXX Danger: Messes up Lua output for unknown reasons
                                                         #             Note: this opt is of minor importance for raytrace...
    LLVM_OPT_OPTS.append('-memcpyopt') # Danger?
    LLVM_OPT_OPTS.append('-sccp')

    if allow_nonportable: LLVM_OPT_OPTS.append('-instcombine')
    LLVM_OPT_OPTS.append('-jump-threading')
    LLVM_OPT_OPTS.append('-correlated-propagation')
    LLVM_OPT_OPTS.append('-dse')
    LLVM_OPT_OPTS.append('-adce')
    LLVM_OPT_OPTS.append('-simplifycfg')

    LLVM_OPT_OPTS.append('-strip-dead-prototypes')
    LLVM_OPT_OPTS.append('-deadtypeelim')

    if optimization_level > 2:
      LLVM_OPT_OPTS.append('-globaldce')
  
    if optimization_level > 1:
      LLVM_OPT_OPTS.append('-constmerge')

  # Emscripten optimizations that we run on the .ll file
  def do_ll_opts(self, filename):
    shutil.move(filename + '.o.ll', filename + '.o.ll.orig')
    output = Popen(['python', DFE, filename + '.o.ll.orig', filename + '.o.ll'], stdout=PIPE, stderr=STDOUT).communicate()[0]
    assert os.path.exists(filename + '.o.ll'), 'Failed to run ll optimizations'

  # Optional LLVM optimizations
  def do_llvm_opts(self, filename):
    if LLVM_OPTS:
      shutil.move(filename + '.o', filename + '.o.pre')
      output = Popen([LLVM_OPT, filename + '.o.pre'] + LLVM_OPT_OPTS + ['-o=' + filename + '.o'], stdout=PIPE, stderr=STDOUT).communicate()[0]

  def do_llvm_dis(self, filename):
    # LLVM binary ==> LLVM assembly
    try:
      os.remove(filename + '.o.ll')
    except:
      pass
    Popen([LLVM_DIS, filename + '.o'] + LLVM_DIS_OPTS + ['-o=' + filename + '.o.ll'], stdout=PIPE, stderr=STDOUT).communicate()[0]
    assert os.path.exists(filename + '.o.ll'), 'Could not create .ll file'

  def do_llvm_as(self, source, target):
    # LLVM assembly ==> LLVM binary
    try:
      os.remove(target)
    except:
      pass
    Popen([LLVM_AS, source, '-o=' + target], stdout=PIPE, stderr=STDOUT).communicate()[0]
    assert os.path.exists(target), 'Could not create bc file'

  def do_link(self, files, target):
    output = Popen([LLVM_LINK] + files + ['-o', target], stdout=PIPE, stderr=STDOUT).communicate()[0]
    assert output is None or 'Could not open input file' not in output, 'Linking error: ' + output

  def prep_ll_test(self, filename, ll_file, force_recompile=False, build_ll_hook=None):
    if ll_file.endswith(('.bc', '.o')):
      if ll_file != filename + '.o':
        shutil.copy(ll_file, filename + '.o')
      self.do_llvm_dis(filename)
    else:
      shutil.copy(ll_file, filename + '.o.ll')

    force_recompile = force_recompile or os.stat(filename + '.o.ll').st_size > 50000 # if the file is big, recompile just to get ll_opts

    if LLVM_OPTS or force_recompile or build_ll_hook:
      self.do_ll_opts(filename)
      if build_ll_hook:
        build_ll_hook(filename)
      shutil.move(filename + '.o.ll', filename + '.o.ll.pre')
      self.do_llvm_as(filename + '.o.ll.pre', filename + '.o')
      output = Popen([LLVM_AS, filename + '.o.ll.pre'] + ['-o=' + filename + '.o'], stdout=PIPE, stderr=STDOUT).communicate()[0]
      assert 'error:' not in output, 'Error in llvm-as: ' + output
      self.do_llvm_opts(filename)
      self.do_llvm_dis(filename)

  # Build JavaScript code from source code
  def build(self, src, dirname, filename, output_processor=None, main_file=None, additional_files=[], libraries=[], includes=[], build_ll_hook=None):
    # Copy over necessary files for compiling the source
    if main_file is None:
      f = open(filename, 'w')
      f.write(src)
      f.close()
      assert len(additional_files) == 0
    else:
      # copy whole directory, and use a specific main .cpp file
      shutil.rmtree(dirname)
      shutil.copytree(src, dirname)
      shutil.move(os.path.join(dirname, main_file), filename)
      # the additional files were copied; alter additional_files to point to their full paths now
      additional_files = map(lambda f: os.path.join(dirname, f), additional_files)

    # Copy Emscripten C++ API
    shutil.copy(path_from_root('src', 'include', 'emscripten.h'), dirname)

    # C++ => LLVM binary
    os.chdir(dirname)
    cwd = os.getcwd()
    for f in [filename] + additional_files:
      try:
        # Make sure we notice if compilation steps failed
        os.remove(f + '.o')
        os.remove(f + '.o.ll')
      except:
        pass
      output = Popen([COMPILER, '-DEMSCRIPTEN', '-emit-llvm'] + COMPILER_OPTS + COMPILER_TEST_OPTS +
                     ['-I', dirname, '-I', os.path.join(dirname, 'include')] +
                     map(lambda include: '-I' + include, includes) + 
                     ['-c', f, '-o', f + '.o'],
                     stdout=PIPE, stderr=STDOUT).communicate()[0]
      assert os.path.exists(f + '.o'), 'Source compilation error: ' + output

    os.chdir(cwd)

    # Link all files
    if len(additional_files) + len(libraries) > 0:
      shutil.move(filename + '.o', filename + '.o.alone')
      self.do_link([filename + '.o.alone'] + map(lambda f: f + '.o', additional_files) + libraries,
                   filename + '.o')
      if not os.path.exists(filename + '.o'):
        print "Failed to link LLVM binaries:\n\n", output
        raise Exception("Linkage error");

    # Finalize
    self.prep_ll_test(filename, filename + '.o', build_ll_hook=build_ll_hook)

    self.do_emscripten(filename, output_processor)

  def do_emscripten(self, filename, output_processor=None):
    # Run Emscripten
    exported_settings = {}
    for setting in ['QUANTUM_SIZE', 'RELOOP', 'OPTIMIZE', 'ASSERTIONS', 'USE_TYPED_ARRAYS', 'SAFE_HEAP', 'CHECK_OVERFLOWS', 'CORRECT_OVERFLOWS', 'CORRECT_SIGNS', 'CHECK_SIGNS', 'CORRECT_OVERFLOWS_LINES', 'CORRECT_SIGNS_LINES', 'CORRECT_ROUNDINGS', 'CORRECT_ROUNDINGS_LINES', 'INVOKE_RUN', 'SAFE_HEAP_LINES', 'INIT_STACK', 'AUTO_OPTIMIZE', 'EXPORTED_FUNCTIONS', 'EXPORTED_GLOBALS', 'BUILD_AS_SHARED_LIB', 'INCLUDE_FULL_LIBRARY']:
      try:
        value = eval(setting)
        exported_settings[setting] = value
      except:
        pass
    compiler_output = timeout_run(Popen([EMSCRIPTEN, filename + '.o.ll', str(exported_settings).replace("'", '"'), filename + '.o.js'], stdout=PIPE, stderr=STDOUT), TIMEOUT, 'Compiling')

    # Detect compilation crashes and errors
    if compiler_output is not None and 'Traceback' in compiler_output and 'in test_' in compiler_output: print compiler_output; assert 0

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
    run_js(engine, filename, args, check_timeout, stdout=open(stdout, 'w'), stderr=open(stderr, 'w'))
    if cwd is not None:
      os.chdir(cwd)
    ret = open(stdout, 'r').read() + open(stderr, 'r').read()
    assert 'strict warning:' not in ret, 'We should pass all strict mode checks: ' + ret
    return ret

  def run_llvm_interpreter(self, args):
    return Popen([LLVM_INTERPRETER] + args, stdout=PIPE, stderr=STDOUT).communicate()[0]

  def build_native(self, filename, compiler='g++'):
    Popen([compiler, '-O3', filename, '-o', filename+'.native'], stdout=PIPE, stderr=STDOUT).communicate()[0]

  def run_native(self, filename, args):
    Popen([filename+'.native'] + args, stdout=PIPE, stderr=STDOUT).communicate()[0]

  def assertContained(self, value, string):
    if type(value) is not str: value = value() # lazy loading
    if type(string) is not str: string = string()
    if value not in string:
      raise Exception("Expected to find '%s' in '%s'" % (limit_size(value), limit_size(string)))

  def assertNotContained(self, value, string):
    if type(value) is not str: value = value() # lazy loading
    if type(string) is not str: string = string()
    if value in string:
      raise Exception("Expected to NOT find '%s' in '%s'" % (limit_size(value), limit_size(string)))

###################################################################################################

if 'benchmark' not in sys.argv:
  # Tests

  print "Running Emscripten tests..."

  class T(RunnerCore): # Short name, to make it more fun to use manually on the commandline
    ## Does a complete test - builds, runs, checks output, etc.
    def do_test(self, src, expected_output=None, args=[], output_nicerizer=None, output_processor=None, no_build=False, main_file=None, additional_files=[], js_engines=None, post_build=None, basename='src.cpp', libraries=[], includes=[], force_c=False, build_ll_hook=None):
        #print 'Running test:', inspect.stack()[1][3].replace('test_', ''), '[%s,%s,%s]' % (COMPILER.split(os.sep)[-1], 'llvm-optimizations' if LLVM_OPTS else '', 'reloop&optimize' if RELOOP else '')
        if force_c or (main_file is not None and main_file[-2:]) == '.c':
          basename = 'src.c'
          global COMPILER
          COMPILER = to_cc(COMPILER)

        dirname = self.get_dir()
        filename = os.path.join(dirname, basename)
        if not no_build:
          self.build(src, dirname, filename, main_file=main_file, additional_files=additional_files, libraries=libraries, includes=includes,
                     build_ll_hook=build_ll_hook)

        if post_build is not None:
          post_build(filename + '.o.js')

        # If not provided with expected output, then generate it right now, using lli
        if expected_output is None:
          expected_output = self.run_llvm_interpreter([filename + '.o'])
          print '[autogenerated expected output: %20s]' % (expected_output[0:17].replace('\n', '')+'...')

        # Run in both JavaScript engines, if optimizing - significant differences there (typed arrays)
        if js_engines is None:
          js_engines = [SPIDERMONKEY_ENGINE, V8_ENGINE]
        if USE_TYPED_ARRAYS == 2:
          js_engines = [SPIDERMONKEY_ENGINE] # when oh when will v8 support typed arrays in the console
        for engine in js_engines:
          js_output = self.run_generated_code(engine, filename + '.o.js', args)
          if output_nicerizer is not None:
              js_output = output_nicerizer(js_output)
          self.assertContained(expected_output, js_output)
          self.assertNotContained('ERROR', js_output)

        #shutil.rmtree(dirname) # TODO: leave no trace in memory. But for now nice for debugging

    # No building - just process an existing .ll file (or .bc, which we turn into .ll)
    def do_ll_test(self, ll_file, expected_output=None, args=[], js_engines=None, output_nicerizer=None, post_build=None, force_recompile=False, build_ll_hook=None):
      if COMPILER != LLVM_GCC: return self.skip() # We use existing .ll, so which compiler is unimportant

      filename = os.path.join(self.get_dir(), 'src.cpp')

      self.prep_ll_test(filename, ll_file, force_recompile, build_ll_hook)
      self.do_emscripten(filename)
      self.do_test(None,
                   expected_output,
                   args,
                   no_build=True,
                   js_engines=js_engines,
                   output_nicerizer=output_nicerizer,
                   post_build=post_build)

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
        self.do_test(src, '*5,23,10,19,121,1,37,1,0*\n0:-1,1:134217727,2:4194303,3:131071,4:4095,5:127,6:3,7:0,8:0*\n*56,09*\nfixed:320434\n*21*')

    def test_sintvars(self):
        global CORRECT_SIGNS; CORRECT_SIGNS = 1 # Relevant to this test
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
        global CORRECT_OVERFLOWS; CORRECT_OVERFLOWS = 0 # We should not need overflow correction to get this right
        self.do_test(src, output, force_c=True)

    def test_bigint(self):
        src = '''
          #include <stdio.h>
          int main()
          {
            long long x = 0x0000def123450789ULL; // any bigger than this, and we
            long long y = 0x00020ef123456089ULL; // start to run into the double precision limit!
            printf("*%Ld,%Ld,%Ld,%Ld,%Ld*\\n", x, y, x | y, x & y, x ^ y, x >> 2, y << 2);
            return 0;
          }
        '''
        self.do_test(src, '*245127260211081,579378795077769,808077213656969,16428841631881,791648372025088*')

    def test_unsigned(self):
        global CORRECT_SIGNS; CORRECT_SIGNS = 1 # We test for exactly this sort of thing here
        global CHECK_SIGNS; CHECK_SIGNS = 0
        src = '''
          #include <stdio.h>
          const signed char cvals[2] = { -1, -2 }; // compiler can store this is a string, so -1 becomes \FF, and needs re-signing
          int main()
          {
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
        self.do_test(src, '*4294967295,0,4294967219*\n*-1,1,-1,1*\n*-2,1,-2,1*\n*246,296*\n*1,0*')

        # Now let's see some code that should just work in USE_TYPED_ARRAYS == 2, but requires
        # corrections otherwise
        if USE_TYPED_ARRAYS == 2:
          CORRECT_SIGNS = 0
          CHECK_SIGNS = 1
        else:
          CORRECT_SIGNS = 1
          CHECK_SIGNS = 0

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
        self.do_test(src, '*255*\n*65535*\n*-1*\n*-1*\n*-1*')

    def test_bitfields(self):
        global SAFE_HEAP; SAFE_HEAP = 0 # bitfields do loads on invalid areas, by design
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
        self.do_test(src, '*0,0,0,0,0,1,0,1,0,0,1,1,1,0,0,1,0,1,1,1,0,1,1,1,*')

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
        self.do_test(src, '*1,10,10.5,1,1.2340,0.00*')

    def test_math(self):
        src = '''
          #include <stdio.h>
          #include <cmath>
          int main()
          {
            printf("*%.2f,%.2f,%f,%f*\\n", M_PI, -M_PI, 1/0.0, -1/0.0);
            return 0;
          }
        '''
        self.do_test(src, '*3.14,-3.14,inf,-inf*')

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
        self.do_test(src, '*1 2*')

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
        self.do_test(src, '*no*')

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
        self.do_test(src, '*0,1*')

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
            int cc = sscanf("abc_10.b1_xyz_543", "abc_%d.%2x_xyz_%3d", &xx, &yy, &zz);
            printf("%d:%d,%d,%d\\n", cc, xx, yy, zz);

            printf("%d\\n", argc);
            puts(argv[1]);
            puts(argv[2]);
            printf("%d\\n", atoi(argv[3])+2);
            const char *foolingthecompiler = "\\rabcd";
            printf("%d\\n", strlen(foolingthecompiler)); // Tests parsing /0D in llvm - should not be a 0 (end string) then a D!
            printf("%s\\n", NULL); // Should print '(null)', not the string at address 0, which is a real address for us!
            printf("/* a comment */\\n"); // Should not break the generated code!
            printf("// another\\n"); // Should not break the generated code!
            return 0;
          }
        '''
        self.do_test(src, '3:10,177,543\n4\nwowie\ntoo\n76\n5\n(null)\n/* a comment */\n// another', ['wowie', 'too', '74'])

    def test_mainenv(self):
        src = '''
          #include <stdio.h>
          int main(int argc, char **argv, char **envp)
          {
            printf("*%p*\\n", envp);
            return 0;
          }
        '''
        self.do_test(src, '*0x0*')

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
        self.do_test(self.gen_struct_src.replace('{{gen_struct}}', '(S*)malloc(sizeof(S))').replace('{{del_struct}}', 'free'), '*51,62*')

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
        self.do_test(src, '*25,51*')

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
        if QUANTUM_SIZE == 1:
          self.do_test(src, 'sizeofs:6,8\n*C___: 0,3,6,9<24*\n*Carr: 0,3,6,9<24*\n*C__w: 0,3,9,12<24*\n*Cp1_: 1,2,5,8<24*\n*Cp2_: 0,2,5,8<24*\n*Cint: 0,3,4,7<24*\n*C4__: 0,3,4,7<24*\n*C4_2: 0,3,5,8<20*\n*C__z: 0,3,5,8<28*')
        else:
          self.do_test(src, 'sizeofs:6,8\n*C___: 0,6,12,20<24*\n*Carr: 0,6,12,20<24*\n*C__w: 0,6,12,20<24*\n*Cp1_: 4,6,12,20<24*\n*Cp2_: 0,6,12,20<24*\n*Cint: 0,8,12,20<24*\n*C4__: 0,8,12,20<24*\n*C4_2: 0,6,10,16<20*\n*C__z: 0,8,16,24<28*')

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

    def test_exceptions(self):
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
        self.do_test(src, '*throw...caught!infunc...done!*')

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
        self.do_test(src, '*11,74,32,1012*\n*11*\n*22*')

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
        self.do_test(src, '*26,26,90,90,26,90*\n*1,0,0,1*\n*goodbye!*')

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
        self.do_test(src, 'fn2(-5) = 5, fn(10) = 3.16')

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

    def test_alloca(self):
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g'] # This can mess up our parsing of [#uses=..]

      src = '''
        #include <stdio.h>

        int main() {
          char *pc;
          pc = (char *)alloca(5);
          printf("z:%d*%d*\\n", pc > 0, (int)pc);
          return 0;
        }
      '''
      self.do_test(src, 'z:1*', force_c=True)

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
        self.do_test(src, '0:-1.00,-0.33 1:0.33,-1.00 2:-0.33,1.00 3:1.00,0.33')

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
        self.do_test(src, '*6,6\n7,95*')


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
        self.do_test(src, '*97,15,3,10*')

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
        self.do_test(src, '*70,97,15,3,3029,90*')

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
        self.do_test(src, '*4096,4096,8192,69632*')

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
        if QUANTUM_SIZE == 1:
          # Compressed memory. Note that sizeof() does give the fat sizes, however!
          self.do_test(src, '*0,0,0,1,2,3,4,5*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*12,20,5*')
        else:
          self.do_test(src, '*0,0,0,4,8,12,16,20*\n*1,0,0*\n*0*\n0:1,1\n1:1,1\n2:1,1\n*12,20,20*')

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
        # Has invalid writes between printouts
        global SAFE_HEAP; SAFE_HEAP = 0

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
        self.do_test(src, '*2,2,5,8,8***8,8,5,8,8***7,2,6,990,7,2*', [], lambda x: x.replace('\n', '*'))

    def test_emscripten_api(self):
        src = '''
          #include <stdio.h>
          #include "emscripten.h"

          int main() {
            emscripten_run_script("print('hello world' + '!')");
            return 0;
          }
          '''
        self.do_test(src, 'hello world!')

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
        if QUANTUM_SIZE == 1:
          self.do_test(src, '''*4*\n0:22016,0,8,12\n1:22018,1,12,8\n''')
        else:
          self.do_test(src, '''*16*\n0:22016,0,32,48\n1:22018,1,48,32\n''')

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
        self.do_test(src, '*nameA,nameB*')

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
        if QUANTUM_SIZE == 1:
          self.do_test(src, '*4,2,3*\n*6,2,3*')
        else:
          self.do_test(src, '*4,3,4*\n*6,4,6*')

    def test_varargs(self):
        if QUANTUM_SIZE == 1: return self.skip() # FIXME: Add support for this

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

            return 0;
          }
          '''
        self.do_test(src, '*cheez: 0+24*\n*cheez: 0+24*\n*albeit*\n*albeit*\nQ85*\nmaxxi:21*\nmaxxD:22.10*\n')

    def test_stdlibs(self):
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

            return 0;
          }
          '''
        self.do_test(src, '*1,2,3,5,5,6*\n*stdin==0:0*\n*%*\n*5*\n*66.0*\n*cleaned*')

    def test_statics(self):
        # static initializers save i16 but load i8 for some reason
        global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']
        global SAFE_HEAP, SAFE_HEAP_LINES
        if SAFE_HEAP:
          SAFE_HEAP = 3
          SAFE_HEAP_LINES = ['src.cpp:19', 'src.cpp:26']

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
        self.do_test(src, '*staticccz*\n*1.00,2.00,3.00*')

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
        self.do_test(src, '*0.00,0.00,0.00*\n*0,77,0*\n*0,77,0*\n*0,77,0*')

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
        if QUANTUM_SIZE == 1:
          # Compressed memory. Note that sizeof() does give the fat sizes, however!
          self.do_test(src, '*16,0,1,2,2,3|20,0,1,1,2,3,3,4|24,0,5,0,1,1,2,3,3,4*\n*0,0,0,1,2,62,63,64,72*\n*2*')
        else:
          # Bloated memory; same layout as C/C++
          self.do_test(src, '*16,0,4,8,8,12|20,0,4,4,8,12,12,16|24,0,20,0,4,4,8,12,12,16*\n*0,0,0,1,2,64,68,69,72*\n*2*')

    def test_dlfcn_basic(self):
      global BUILD_AS_SHARED_LIB
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
      BUILD_AS_SHARED_LIB = 1
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so.js'))

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
      BUILD_AS_SHARED_LIB = 0
      self.do_test(src, 'Constructing main object.\nConstructing lib object.\n')

    def test_dlfcn_qsort(self):
      global BUILD_AS_SHARED_LIB, EXPORTED_FUNCTIONS
      lib_src = '''
        int lib_cmp(const void* left, const void* right) {
          const int* a = (const int*) left;
          const int* b = (const int*) right;
          if(*a > *b) return 1;
          else if(*a == *b) return  0;
          else return -1;
        }

        typedef int (*CMP_TYPE)(const void*, const void*);

        CMP_TYPE get_cmp() {
          return lib_cmp;
        }
        '''
      dirname = self.get_dir()
      filename = os.path.join(dirname, 'liblib.cpp')
      BUILD_AS_SHARED_LIB = 1
      EXPORTED_FUNCTIONS = ['__Z7get_cmpv']
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so.js'))

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
          getter_ptr = (CMP_TYPE (*)()) dlsym(lib_handle, "_Z7get_cmpv");
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
      BUILD_AS_SHARED_LIB = 0
      EXPORTED_FUNCTIONS = ['_main']
      self.do_test(src, 'Sort with main comparison: 5 4 3 2 1 *Sort with lib comparison: 1 2 3 4 5 *',
                   output_nicerizer=lambda x: x.replace('\n', '*'))

    def test_dlfcn_data_and_fptr(self):
      global BUILD_AS_SHARED_LIB, EXPORTED_FUNCTIONS, EXPORTED_GLOBALS
      lib_src = '''
        #include <stdio.h>

        int global = 42;

        void lib_fptr() {
          printf("Second calling lib_fptr from main.\\n");
        }

        void (*func(int x, void(*fptr)()))() {
          printf("In func: %d\\n", x);
          fptr();
          return lib_fptr;
        }
        '''
      dirname = self.get_dir()
      filename = os.path.join(dirname, 'liblib.cpp')
      BUILD_AS_SHARED_LIB = 1
      EXPORTED_FUNCTIONS = ['__Z4funciPFvvE']
      EXPORTED_GLOBALS = ['_global']
      self.build(lib_src, dirname, filename)
      shutil.move(filename + '.o.js', os.path.join(dirname, 'liblib.so.js'))

      src = '''
        #include <stdio.h>
        #include <dlfcn.h>

        typedef void (*FUNCTYPE(int, void(*)()))();

        FUNCTYPE func;

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
          func_fptr = (FUNCTYPE*) dlsym(lib_handle, "_Z4funciPFvvE");
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
      BUILD_AS_SHARED_LIB = 0
      EXPORTED_FUNCTIONS = ['_main']
      EXPORTED_GLOBALS = []
      self.do_test(src, 'In func: 13*First calling main_fptr from lib.*Second calling lib_fptr from main.*Var: 42*',
                   output_nicerizer=lambda x: x.replace('\n', '*'))

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
          printf("%g\n", strtod("1", &endptr));
          printf("%g\n", strtod("1.", &endptr));
          printf("%g\n", strtod("1.0", &endptr));
          printf("%g\n", strtod("123", &endptr));
          printf("%g\n", strtod("123.456", &endptr));
          printf("%g\n", strtod("1234567891234567890", &endptr));
          printf("%g\n", strtod("1234567891234567890e+50", &endptr));
          printf("%g\n", strtod("84e+220", &endptr));
          printf("%g\n", strtod("84e+420", &endptr));
          printf("%g\n", strtod("123e-50", &endptr));
          printf("%g\n", strtod("123e-250", &endptr));
          printf("%g\n", strtod("123e-450", &endptr));

          char str[] = "12.34e56end";
          strtod(str, &endptr);
          printf("%d\n", endptr - str);
          return 0;
        }
        '''
      expected = '''
        0
        0
        0
        1
        1
        1
        123
        123.456
        1.23457e+18
        1.23457e+68
        8.4e+221
        inf
        1.23e-48
        1.23e-248
        0
        8
        '''
      self.do_test(src, re.sub(r'\n\s+', '\n', expected))

    def test_printf(self):
      src = open(path_from_root('tests', 'printf', 'test.c'), 'r').read()
      expected = open(path_from_root('tests', 'printf', 'output.txt'), 'r').read()
      self.do_test(src, expected)

    def test_files(self):
      global CORRECT_SIGNS; CORRECT_SIGNS = 1 # Just so our output is what we expect. Can flip them both.
      def post(filename):
        src = open(filename, 'r').read().replace(
          '// {{PRE_RUN_ADDITIONS}}',
          '''
            STDIO.prepare('somefile.binary', [100, 200, 50, 25, 10, 77, 123]); // 200 becomes -56, since signed chars are used in memory
            Module.stdin = function(prompt) { return 'hi there!' };
          '''
        )
        open(filename, 'w').write(src)

      other = open(os.path.join(self.get_dir(), 'test.file'), 'w')
      other.write('some data');
      other.close()

      src = open(path_from_root('tests', 'files.cpp'), 'r').read()
      self.do_test(src, 'size: 7\ndata: 100,-56,50,25,10,77,123\ninput:hi there!\ntexto\ntexte\n5 : 10,30,20,11,88\nother=some data.\nseeked=me da.\nseeked=ata.\nseeked=ta.', post_build=post)

    ### 'Big' tests

    def test_fannkuch(self):
        results = [ (1,0), (2,1), (3,2), (4,4), (5,7), (6,10), (7, 16), (8,22) ]
        for i, j in results:
          src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read()
          self.do_test(src, 'Pfannkuchen(%d) = %d.' % (i,j), [str(i)], no_build=i>1)

    def test_raytrace(self):
        global USE_TYPED_ARRAYS
        if USE_TYPED_ARRAYS == 2: return self.skip() # relies on double values

        src = open(path_from_root('tests', 'raytrace.cpp'), 'r').read()
        output = open(path_from_root('tests', 'raytrace.ppm'), 'r').read()
        self.do_test(src, output, ['3', '16'])

    def test_fasta(self):
        results = [ (1,'''GG*ctt**tgagc*'''), (20,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tacgtgtagcctagtgtttgtgttgcgttatagtctatttgtggacacagtatggtcaaa**tgacgtcttttgatctgacggcgttaacaaagatactctg*'''),
(50,'''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA*TCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACAT*cttBtatcatatgctaKggNcataaaSatgtaaaDcDRtBggDtctttataattcBgtcg**tactDtDagcctatttSVHtHttKtgtHMaSattgWaHKHttttagacatWatgtRgaaa**NtactMcSMtYtcMgRtacttctWBacgaa**agatactctgggcaacacacatacttctctcatgttgtttcttcggacctttcataacct**ttcctggcacatggttagctgcacatcacaggattgtaagggtctagtggttcagtgagc**ggaatatcattcgtcggtggtgttaatctatctcggtgtagcttataaatgcatccgtaa**gaatattatgtttatttgtcggtacgttcatggtagtggtgtcgccgatttagacgtaaa**ggcatgtatg*''') ]
        for i, j in results:
          src = open(path_from_root('tests', 'fasta.cpp'), 'r').read()
          self.do_test(src, j, [str(i)], lambda x: x.replace('\n', '*'), no_build=i>1)

    def test_dlmalloc(self):
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']
      global CORRECT_SIGNS; CORRECT_SIGNS = 2
      global CORRECT_SIGNS_LINES; CORRECT_SIGNS_LINES = ['src.cpp:' + str(i) for i in [4816, 4191, 4246, 4199, 4205, 4235, 4227]]

      src = open(path_from_root('tests', 'dlmalloc.c'), 'r').read()
      self.do_test(src, '*1,0*', ['200', '1'])

    def zzztest_gl(self):
      # Switch to gcc from g++ - we don't compile properly otherwise (why?)
      global COMPILER
      if COMPILER != LLVM_GCC: return self.skip()
      COMPILER = LLVM_GCC.replace('g++', 'gcc')

      def post(filename):
        src = open(filename, 'r').read().replace(
          '// {{PRE_RUN_ADDITIONS}}',
          '''Module["__CANVAS__"] = {
               getContext: function() {},
             };'''
        )
        open(filename, 'w').write(src)
      self.do_test(path_from_root('tests', 'gl'), '*?*', main_file='sdl_ogl.c', post_build=post)

    def test_libcxx(self):
      self.do_test(path_from_root('tests', 'libcxx'),
                   'june -> 30\nPrevious (in alphabetical order) is july\nNext (in alphabetical order) is march',
                   main_file='main.cpp', additional_files=['hash.cpp'])

      # This will fail without using libcxx, as libstdc++ (gnu c++ lib) will use but not link in 
      # __ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_
      # So a way to avoid that problem is to include libcxx, as done here
      self.do_test('''
        #include <set>
        #include <stdio.h>
        int main() {
          std::set<int> *fetchOriginatorNums = new std::set<int>();
          fetchOriginatorNums->insert(171);
          printf("hello world\\n");
          return 1;
        }
        ''', 'hello world', includes=[path_from_root('tests', 'libcxx', 'include')]);

    def test_cubescript(self):
      # XXX Warning: Running this in SpiderMonkey can lead to an extreme amount of memory being
      #              used, see Mozilla bug 593659.
      global SAFE_HEAP; SAFE_HEAP = 0 # Has some actual loads of unwritten-to places, in the C++ code...

      # Overflows happen in hash loop
      global CORRECT_OVERFLOWS; CORRECT_OVERFLOWS = 1
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 0

      self.do_test(path_from_root('tests', 'cubescript'), '*\nTemp is 33\n9\n5\nhello, everyone\n*', main_file='command.cpp')

    def test_gcc_unmangler(self):
      self.do_test(path_from_root('third_party'), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'], main_file='gcc_demangler.c')

      #### Code snippet that is helpful to search for nonportable optimizations ####
      #global LLVM_OPT_OPTS
      #for opt in ['-aa-eval', '-adce', '-always-inline', '-argpromotion', '-basicaa', '-basiccg', '-block-placement', '-break-crit-edges', '-codegenprepare', '-constmerge', '-constprop', '-correlated-propagation', '-count-aa', '-dce', '-deadargelim', '-deadtypeelim', '-debug-aa', '-die', '-domfrontier', '-domtree', '-dse', '-extract-blocks', '-functionattrs', '-globaldce', '-globalopt', '-globalsmodref-aa', '-gvn', '-indvars', '-inline', '-insert-edge-profiling', '-insert-optimal-edge-profiling', '-instcombine', '-instcount', '-instnamer', '-internalize', '-intervals', '-ipconstprop', '-ipsccp', '-iv-users', '-jump-threading', '-lazy-value-info', '-lcssa', '-lda', '-libcall-aa', '-licm', '-lint', '-live-values', '-loop-deletion', '-loop-extract', '-loop-extract-single', '-loop-index-split', '-loop-reduce', '-loop-rotate', '-loop-unroll', '-loop-unswitch', '-loops', '-loopsimplify', '-loweratomic', '-lowerinvoke', '-lowersetjmp', '-lowerswitch', '-mem2reg', '-memcpyopt', '-memdep', '-mergefunc', '-mergereturn', '-module-debuginfo', '-no-aa', '-no-profile', '-partial-inliner', '-partialspecialization', '-pointertracking', '-postdomfrontier', '-postdomtree', '-preverify', '-prune-eh', '-reassociate', '-reg2mem', '-regions', '-scalar-evolution', '-scalarrepl', '-sccp', '-scev-aa', '-simplify-libcalls', '-simplify-libcalls-halfpowr', '-simplifycfg', '-sink', '-split-geps', '-sretpromotion', '-strip', '-strip-dead-debug-info', '-strip-dead-prototypes', '-strip-debug-declare', '-strip-nondebug', '-tailcallelim', '-tailduplicate', '-targetdata', '-tbaa']:
      #  LLVM_OPT_OPTS = [opt]
      #  try:
      #    self.do_test(path_from_root(['third_party']), '*d_demangle(char const*, int, unsigned int*)*', args=['_ZL10d_demanglePKciPj'], main_file='gcc_demangler.c')
      #    print opt, "ok"
      #  except:
      #    print opt, "FAIL"

    def test_lua(self):
      # Overflows in luaS_newlstr hash loop
      global SAFE_HEAP; SAFE_HEAP = 0 # Has various warnings, with copied HEAP_HISTORY values (fixed if we copy 'null' as the type)
      global CORRECT_OVERFLOWS; CORRECT_OVERFLOWS = 1
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 0
      global CORRECT_SIGNS; CORRECT_SIGNS = 1 # Not sure why, but needed
      global INIT_STACK; INIT_STACK = 1 # TODO: Investigate why this is necessary

      self.do_ll_test(path_from_root('tests', 'lua', 'lua.ll'),
                      'hello lua world!\n17\n1\n2\n3\n4\n7',
                      args=['-e', '''print("hello lua world!");print(17);for x = 1,4 do print(x) end;print(10-3)'''],
                      output_nicerizer=lambda string: string.replace('\n\n', '\n').replace('\n\n', '\n'))

    def get_building_dir(self):
      return os.path.join(self.get_dir(), 'building')

    # Build a library into a .bc file. We build the .bc file once and cache it for all our tests. (We cache in
    # memory since the test directory is destroyed and recreated for each test. Note that we cache separately
    # for different compilers)
    def get_library(self, name, generated_libs, configure=['./configure'], configure_args=[], make=['make'], make_args=['-j', '2'], cache=True):
      if type(generated_libs) is not list: generated_libs = [generated_libs]

      if GlobalCache is not None:
        cache_name = name + '|' + COMPILER
        if cache and GlobalCache.get(cache_name):
          print >> sys.stderr,  '<load build from cache> ',
          bc_file = os.path.join(self.get_dir(), 'lib' + name + '.bc')
          f = open(bc_file, 'wb')
          f.write(GlobalCache[cache_name])
          f.close()
          return bc_file

      temp_dir = self.get_building_dir()
      project_dir = os.path.join(temp_dir, name)
      shutil.copytree(path_from_root('tests', name), project_dir) # Useful in debugging sometimes to comment this out
      os.chdir(project_dir)
      env = os.environ.copy()
      env['RANLIB'] = env['AR'] = env['CXX'] = env['CC'] = env['LIBTOOL'] = EMMAKEN
      env['EMMAKEN_COMPILER'] = COMPILER
      env['EMSCRIPTEN_TOOLS'] = path_from_root('tools')
      env['CFLAGS'] = env['EMMAKEN_CFLAGS'] = ' '.join(COMPILER_OPTS + COMPILER_TEST_OPTS) # Normal CFLAGS is ignored by some configure's.
      if configure: # Useful in debugging sometimes to comment this out (and 2 lines below)
        Popen(configure + configure_args, stdout=PIPE, stderr=STDOUT, env=env).communicate()[0]
      Popen(make + make_args, stdout=PIPE, stderr=STDOUT, env=env).communicate()[0]
      bc_file = os.path.join(project_dir, 'bc.bc')
      self.do_link(map(lambda lib: os.path.join(project_dir, lib), generated_libs), bc_file)
      if cache and GlobalCache is not None:
        print >> sys.stderr, '<save build into cache> ',
        GlobalCache[cache_name] = open(bc_file, 'rb').read()
      return bc_file

    def get_freetype(self):
      global INIT_STACK; INIT_STACK = 1 # TODO: Investigate why this is necessary

      return self.get_library('freetype', os.path.join('objs', '.libs', 'libfreetype.so'))

    def test_freetype(self):
      if QUANTUM_SIZE == 1: return self.skip() # TODO: Figure out and try to fix

      if LLVM_OPTS or COMPILER == CLANG: global RELOOP; RELOOP = 0 # Too slow; we do care about typed arrays and OPTIMIZE though

      #global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']

      global CORRECT_SIGNS
      if CORRECT_SIGNS == 0: CORRECT_SIGNS = 1 # Not sure why, but needed

      def post(filename):
        # Embed the font into the document
        src = open(filename, 'r').read().replace(
          '// {{PRE_RUN_ADDITIONS}}',
          '''STDIO.prepare('font.ttf', %s);''' % str(
            map(ord, open(path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), 'rb').read())
          )
        )
        open(filename, 'w').write(src)

      # Main
      self.do_test(open(path_from_root('tests', 'freetype', 'main.c'), 'r').read(),
                   open(path_from_root('tests', 'freetype', 'ref.txt'), 'r').read(),
                   ['font.ttf', 'test!', '150', '120', '25'],
                   libraries=[self.get_freetype()],
                   includes=[path_from_root('tests', 'freetype', 'include')],
                   post_build=post)
                   #build_ll_hook=self.do_autodebug)

    def test_zlib(self):
      global CORRECT_SIGNS; CORRECT_SIGNS = 1

      self.do_test(open(path_from_root('tests', 'zlib', 'example.c'), 'r').read(),
                   open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                   libraries=[self.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a'])],
                   includes=[path_from_root('tests', 'zlib')],
                   force_c=True)

    def test_the_bullet(self): # Called thus so it runs late in the alphabetical cycle... it is long
      global SAFE_HEAP, SAFE_HEAP_LINES, COMPILER_TEST_OPTS, USE_TYPED_ARRAYS, LLVM_OPTS

      if LLVM_OPTS: SAFE_HEAP = 0 # Optimizations make it so we do not have debug info on the line we need to ignore
      if COMPILER == LLVM_GCC:
        global INIT_STACK; INIT_STACK = 1 # TODO: Investigate why this is necessary

      if USE_TYPED_ARRAYS == 2: return self.skip() # We have slightly different rounding here for some reason. TODO: activate this

      if SAFE_HEAP:
        # Ignore bitfield warnings
        SAFE_HEAP = 3
        SAFE_HEAP_LINES = ['btVoronoiSimplexSolver.h:40', 'btVoronoiSimplexSolver.h:41',
                           'btVoronoiSimplexSolver.h:42', 'btVoronoiSimplexSolver.h:43']
        COMPILER_TEST_OPTS = ['-g']

      self.do_test(open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'), 'r').read(),
                   open(path_from_root('tests', 'bullet', 'output.txt'), 'r').read(),
                   libraries=[self.get_library('bullet', [os.path.join('src', '.libs', 'libBulletCollision.a'),
                                                          os.path.join('src', '.libs', 'libBulletDynamics.a'),
                                                          os.path.join('src', '.libs', 'libLinearMath.a')],
                                               configure_args=['--disable-demos','--disable-dependency-tracking'])],
                   includes=[path_from_root('tests', 'bullet', 'src')],
                   js_engines=[SPIDERMONKEY_ENGINE]) # V8 issue 1407

    def test_poppler(self):
      if COMPILER != LLVM_GCC: return self.skip() # llvm-link failure when using clang, LLVM bug 9498
      if RELOOP or LLVM_OPTS: return self.skip() # TODO
      if QUANTUM_SIZE == 1: return self.skip() # TODO: Figure out and try to fix

      global USE_TYPED_ARRAYS; USE_TYPED_ARRAYS = 0 # XXX bug - we fail with this FIXME

      global SAFE_HEAP; SAFE_HEAP = 0 # Has variable object

      #global CORRECT_OVERFLOWS; CORRECT_OVERFLOWS = 1
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 0

      #global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 1
      #global CHECK_SIGNS; CHECK_SIGNS = 1

      global CORRECT_SIGNS; CORRECT_SIGNS = 1
      global CORRECT_SIGNS_LINES
      CORRECT_SIGNS_LINES = ['parseargs.cc:171', 'BuiltinFont.cc:64', 'NameToCharCode.cc:115', 'GooHash.cc:368',
                             'Stream.h:469', 'PDFDoc.cc:1064', 'Lexer.cc:201', 'Splash.cc:1130', 'XRef.cc:997',
                             'vector:714', 'Lexer.cc:259', 'Splash.cc:438', 'Splash.cc:532', 'GfxFont.cc:1152',
                             'Gfx.cc:3838', 'Splash.cc:3162', 'Splash.cc:3163', 'Splash.cc:3164', 'Splash.cc:3153',
                             'Splash.cc:3159', 'SplashBitmap.cc:80', 'SplashBitmap.cc:81', 'SplashBitmap.cc:82',
                             'Splash.cc:809', 'Splash.cc:805', 'GooHash.cc:379',
                             # FreeType
                             't1load.c:1850', 'psconv.c:104', 'psconv.c:185', 'psconv.c:366', 'psconv.c:399',
                             'ftcalc.c:308', 't1parse.c:405', 'psconv.c:431', 'ftcalc.c:555', 't1objs.c:458',
                             't1decode.c:595', 't1decode.c:606', 'pstables.h:4048', 'pstables.h:4055', 'pstables.h:4066',
                             'pshglob.c:166', 'ftobjs.c:2548', 'ftgrays.c:1190', 'psmodule.c:116', 'psmodule.c:119',
                             'psobjs.c:195', 'pshglob.c:165', 'ttload.c:694', 'ttmtx.c:195', 'sfobjs.c:957',
                             'sfobjs.c:958', 'ftstream.c:369', 'ftstream.c:372', 'ttobjs.c:1007'] # And many more...

      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-I' + path_from_root('tests', 'libcxx', 'include'), # Avoid libstdc++ linking issue, see libcxx test
                                                       '-g']

      global INVOKE_RUN; INVOKE_RUN = 0 # We append code that does run() ourselves

      # See post(), below
      input_file = open(os.path.join(self.get_dir(), 'paper.pdf.js'), 'w')
      input_file.write(str(map(ord, open(path_from_root('tests', 'poppler', 'paper.pdf'), 'rb').read())))
      input_file.close()

      def post(filename):
        # To avoid loading this large file to memory and altering it, we simply append to the end
        src = open(filename, 'a')
        src.write(
          '''
            STDIO.prepare('paper.pdf', eval(read('paper.pdf.js')));
            run();
            print("Data: " + JSON.stringify(STDIO.streams[STDIO.filenames['filename-1.ppm']].data));
          '''
        )
        src.close()

      #fontconfig = self.get_library('fontconfig', [os.path.join('src', '.libs', 'libfontconfig.a')]) # Used in file, but not needed, mostly

      freetype = self.get_freetype()

      poppler = self.get_library('poppler',
                                 [os.path.join('poppler', '.libs', 'libpoppler.so.13.0.0'),
                                  os.path.join('goo', '.libs', 'libgoo.a'),
                                  os.path.join('fofi', '.libs', 'libfofi.a'),
                                  os.path.join('splash', '.libs', 'libsplash.a'),
                                  os.path.join('utils', 'pdftoppm.o'),
                                  os.path.join('utils', 'parseargs.o')],
                                 configure_args=['--disable-libjpeg', '--disable-libpng', '--disable-poppler-qt', '--disable-poppler-qt4'])

      # Combine libraries

      combined = os.path.join(self.get_building_dir(), 'combined.bc')
      self.do_link([freetype, poppler], combined)

      self.do_ll_test(combined,
                      lambda: map(ord, open(path_from_root('tests', 'poppler', 'ref.ppm'), 'r').read()).__str__().replace(' ', ''),
                      args='-scale-to 512 paper.pdf filename'.split(' '),
                      post_build=post,
                      js_engines=[SPIDERMONKEY_ENGINE]) # V8 bug 1257
                      #, build_ll_hook=self.do_autodebug)

    def test_openjpeg(self):
      global USE_TYPED_ARRAYS
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']
      global CORRECT_SIGNS
      if USE_TYPED_ARRAYS == 2:
        CORRECT_SIGNS = 1
      else:
        CORRECT_SIGNS = 2
        global CORRECT_SIGNS_LINES
        if COMPILER == CLANG:
          CORRECT_SIGNS_LINES = ["mqc.c:566"]
        else:
          CORRECT_SIGNS_LINES = ["mqc.c:566", "mqc.c:317"]

      original_j2k = path_from_root('tests', 'openjpeg', 'syntensity_lobby_s.j2k')

      def post(filename):
        src = open(filename, 'r').read().replace(
          '// {{PRE_RUN_ADDITIONS}}',
          '''STDIO.prepare('image.j2k', %s);''' % line_splitter(str(
            map(ord, open(original_j2k, 'rb').read())
          ))
        ).replace(
          '// {{POST_RUN_ADDITIONS}}',
          '''print("Data: " + JSON.stringify(STDIO.streams[STDIO.filenames['image.raw']].data));'''
        )
        open(filename, 'w').write(src)

      lib = self.get_library('openjpeg',
                             [os.path.join('bin', 'libopenjpeg.so'),
                              os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/index.c.o'.split('/')),
                              os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/convert.c.o'.split('/')),
                              os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/__/common/color.c.o'.split('/')),
                              os.path.sep.join('codec/CMakeFiles/j2k_to_image.dir/__/common/getopt.c.o'.split('/'))],
                             configure=['cmake', '.'],
                             #configure_args=['--enable-tiff=no', '--enable-jp3d=no', '--enable-png=no'],
                             make_args=[], # no -j 2, since parallel builds can fail
                             cache=False) # We need opj_config.h and other generated files, so cannot cache just the .bc

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

      self.do_test(open(path_from_root('tests', 'openjpeg', 'codec', 'j2k_to_image.c'), 'r').read(),
                   'Successfully generated', # The real test for valid output is in image_compare
                   '-i image.j2k -o image.raw'.split(' '),
                   libraries=[lib],
                   includes=[path_from_root('tests', 'openjpeg', 'libopenjpeg'),
                             path_from_root('tests', 'openjpeg', 'codec'),
                             path_from_root('tests', 'openjpeg', 'common'),
                             os.path.join(self.get_building_dir(), 'openjpeg')],
                   force_c=True,
                   post_build=post,
                   output_nicerizer=image_compare)# build_ll_hook=self.do_autodebug)

    def test_python(self):
      # Overflows in string_hash
      global CORRECT_OVERFLOWS; CORRECT_OVERFLOWS = 1
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 0
      global RELOOP; RELOOP = 0 # Too slow; we do care about typed arrays and OPTIMIZE though
      global SAFE_HEAP; SAFE_HEAP = 0 # Has bitfields which are false positives. Also the PyFloat_Init tries to detect endianness.
      global CORRECT_SIGNS; CORRECT_SIGNS = 1 # Not sure why, but needed
      global EXPORTED_FUNCTIONS; EXPORTED_FUNCTIONS = ['_main', '_PyRun_SimpleStringFlags'] # for the demo

      self.do_ll_test(path_from_root('tests', 'python', 'python.ll'),
                      'hello python world!\n[0, 2, 4, 6]\n5\n22\n5.470',
                      args=['-S', '-c' '''print "hello python world!"; print [x*2 for x in range(4)]; t=2; print 10-3-t; print (lambda x: x*2)(11); print '%f' % 5.47'''])

    ### Test cases in separate files

    def test_cases(self):
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 0
      if LLVM_OPTS: return self.skip() # Our code is not exactly 'normal' llvm assembly
      for name in glob.glob(path_from_root('tests', 'cases', '*.ll')):
        shortname = name.replace('.ll', '')
        print "Testing case '%s'..." % shortname
        output_file = path_from_root('tests', 'cases', shortname + '.txt')
        if os.path.exists(output_file):
          output = open(output_file, 'r').read()
        else:
          output = 'hello, world!'
        self.do_ll_test(path_from_root('tests', 'cases', name), output)

    # Autodebug the code
    def do_autodebug(self, filename):
      output = Popen(['python', AUTODEBUGGER, filename+'.o.ll', filename+'.o.ll.ll'], stdout=PIPE, stderr=STDOUT).communicate()[0]
      assert 'Success.' in output, output
      self.prep_ll_test(filename, filename+'.o.ll.ll', force_recompile=True) # rebuild .bc

    def test_autodebug(self):
      if LLVM_OPTS: return self.skip() # They mess us up

      # Run a test that should work, generating some code
      self.test_structs()

      filename = os.path.join(self.get_dir(), 'src.cpp')
      self.do_autodebug(filename)

      # Compare to each other, and to expected output
      self.do_ll_test(path_from_root('tests', filename+'.o.ll.ll'))
      self.do_ll_test(path_from_root('tests', filename+'.o.ll.ll'), 'AD:34,10\nAD:43,7008\nAD:53,7018\n')

      # Test using build_ll_hook
      src = '''
          #include <stdio.h>

          char cache[256], *next = cache;

          int main()
          {
            cache[10] = 25;
            next[20] = 51;
            int x = cache[10];
            printf("*%d,%d*\\n", x, cache[20]);
            return 0;
          }
        '''
      self.do_test(src, build_ll_hook=self.do_autodebug)
      self.do_test(src, 'AD:', build_ll_hook=self.do_autodebug)

    def test_dfe(self):
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']

      def hook(filename):
        ll = open(filename + '.o.ll').read()
        assert 'unneeded' not in ll, 'DFE should remove the unneeded function'

      src = '''
          #include <stdio.h>

          void unneeded()
          {
            printf("some totally useless stuff\\n");
          }

          int main()
          {
            printf("*hello slim world*\\n");
            return 0;
          }
        '''
      # Using build_ll_hook forces a recompile, which leads to DFE being done even without opts
      self.do_test(src, '*hello slim world*', build_ll_hook=hook)

    ### Integration tests

    def test_scriptaclass(self):
        src = '''
          struct ScriptMe {
            int value;
            ScriptMe(int val);
            int getVal(); // XXX Sadly, inlining these will result in LLVM not
                          // producing any code for them (when just building
                          // as a library)
            void mulVal(int mul);
          };
          ScriptMe::ScriptMe(int val) : value(val) { }
          int ScriptMe::getVal() { return value; }
          void ScriptMe::mulVal(int mul) { value *= mul; }
        '''
        script_src = '''
          var sme = Module._.ScriptMe.__new__(83);          // malloc(sizeof(ScriptMe)), ScriptMe::ScriptMe(sme, 83) / new ScriptMe(83) (at addr sme)
          Module._.ScriptMe.mulVal(sme, 2);                 // ScriptMe::mulVal(sme, 2)       sme.mulVal(2)
          print('*' + Module._.ScriptMe.getVal(sme) + '*'); 
          _free(sme);
          print('*ok*');
        '''
        def post(filename):
          Popen(['python', DEMANGLER, filename], stdout=open(filename + '.tmp', 'w')).communicate()
          Popen(['python', NAMESPACER, filename, filename + '.tmp'], stdout=open(filename + '.tmp2', 'w')).communicate()
          src = open(filename, 'r').read().replace(
            '// {{MODULE_ADDITIONS}',
            'Module["_"] = ' + open(filename + '.tmp2', 'r').read().replace('var ModuleNames = ', '').rstrip() + ';\n\n' + script_src + '\n\n' +
              '// {{MODULE_ADDITIONS}'
          )
          open(filename, 'w').write(src)
        self.do_test(src, '*166*\n*ok*', post_build=post)

    ### Tests for tools

    def test_safe_heap(self):
      global SAFE_HEAP, SAFE_HEAP_LINES

      if not SAFE_HEAP: return self.skip()
      if LLVM_OPTS: return self.skip() # LLVM can optimize away the intermediate |x|...
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
        self.do_test(src, '*nothingatall*')
      except Exception, e:
        # This test *should* fail, by throwing this exception
        assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

      # And we should not fail if we disable checking on that line

      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']

      SAFE_HEAP = 3
      SAFE_HEAP_LINES = ["src.cpp:7"]

      self.do_test(src, '*ok*')

      # But if we disable the wrong lines, we still fail

      SAFE_HEAP_LINES = ["src.cpp:99"]

      try:
        self.do_test(src, '*nothingatall*')
      except Exception, e:
        # This test *should* fail, by throwing this exception
        assert 'Assertion failed: Load-store consistency assumption failure!' in str(e), str(e)

      # And reverse the checks with = 2

      SAFE_HEAP = 2
      SAFE_HEAP_LINES = ["src.cpp:99"]

      self.do_test(src, '*ok*')

    def test_check_overflow(self):
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 1
      global CORRECT_OVERFLOWS; CORRECT_OVERFLOWS = 0

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
        self.do_test(src, '*nothingatall*')
      except Exception, e:
        # This test *should* fail, by throwing this exception
        assert 'Too many corrections' in str(e), str(e)
        assert 'CHECK_OVERFLOW' in str(e), str(e)

    def test_debug(self):
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']
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
        def post(filename):
          lines = open(filename, 'r').readlines()
          line = filter(lambda line: '___assert_fail(' in line, lines)[0]
          assert '//@line 7 "' in line, 'Must have debug info with the line number'
          assert 'src.cpp"\n' in line, 'Must have debug info with the filename'
        self.do_test(src, '*nothingatall*', post_build=post)
      except Exception, e:
        # This test *should* fail
        assert 'Assertion failed' in str(e), str(e)

    def test_linespecific(self):
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']

      global CHECK_SIGNS; CHECK_SIGNS = 0
      global CHECK_OVERFLOWS; CHECK_OVERFLOWS = 0
      global CORRECT_SIGNS, CORRECT_OVERFLOWS, CORRECT_ROUNDINGS, CORRECT_SIGNS_LINES, CORRECT_OVERFLOWS_LINES, CORRECT_ROUNDINGS_LINES

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

      CORRECT_SIGNS = 0
      self.do_test(src, '*1*') # This is a fail - we expect 0

      CORRECT_SIGNS = 1
      self.do_test(src, '*0*') # Now it will work properly

      # And now let's fix just that one line
      CORRECT_SIGNS = 2
      CORRECT_SIGNS_LINES = ["src.cpp:9"]
      self.do_test(src, '*0*')

      # Fixing the wrong line should not work
      CORRECT_SIGNS = 2
      CORRECT_SIGNS_LINES = ["src.cpp:3"]
      self.do_test(src, '*1*')

      # And reverse the checks with = 2
      CORRECT_SIGNS = 3
      CORRECT_SIGNS_LINES = ["src.cpp:3"]
      self.do_test(src, '*0*')
      CORRECT_SIGNS = 3
      CORRECT_SIGNS_LINES = ["src.cpp:9"]
      self.do_test(src, '*1*')

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
      CORRECT_OVERFLOWS = 0
      try:
        self.do_test(src, correct)
        raise Exception('UNEXPECTED-PASS')
      except Exception, e:
        assert 'UNEXPECTED' not in str(e), str(e)
        assert 'Expected to find' in str(e), str(e)

      CORRECT_OVERFLOWS = 1
      self.do_test(src, correct) # Now it will work properly

      # And now let's fix just that one line
      CORRECT_OVERFLOWS = 2
      CORRECT_OVERFLOWS_LINES = ["src.cpp:6"]
      self.do_test(src, correct)

      # Fixing the wrong line should not work
      CORRECT_OVERFLOWS = 2
      CORRECT_OVERFLOWS_LINES = ["src.cpp:3"]
      try:
        self.do_test(src, correct)
        raise Exception('UNEXPECTED-PASS')
      except Exception, e:
        assert 'UNEXPECTED' not in str(e), str(e)
        assert 'Expected to find' in str(e), str(e)

      # And reverse the checks with = 2
      CORRECT_OVERFLOWS = 3
      CORRECT_OVERFLOWS_LINES = ["src.cpp:3"]
      self.do_test(src, correct)
      CORRECT_OVERFLOWS = 3
      CORRECT_OVERFLOWS_LINES = ["src.cpp:6"]
      try:
        self.do_test(src, correct)
        raise Exception('UNEXPECTED-PASS')
      except Exception, e:
        assert 'UNEXPECTED' not in str(e), str(e)
        assert 'Expected to find' in str(e), str(e)

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

      CORRECT_ROUNDINGS = 0
      self.do_test(src.replace('TYPE', 'long long'), '*-3**2**-6**5*') # JS floor operations, always to the negative. This is an undetected error here!
      self.do_test(src.replace('TYPE', 'int'), '*-2**2**-5**5*') # We get these right, since they are 32-bit and we can shortcut using the |0 trick

      CORRECT_ROUNDINGS = 1
      self.do_test(src.replace('TYPE', 'long long'), '*-2**2**-5**5*') # Correct
      self.do_test(src.replace('TYPE', 'int'), '*-2**2**-5**5*') # Correct

      CORRECT_ROUNDINGS = 2
      CORRECT_ROUNDINGS_LINES = ["src.cpp:13"] # Fix just the last mistake
      self.do_test(src.replace('TYPE', 'long long'), '*-3**2**-5**5*')
      self.do_test(src.replace('TYPE', 'int'), '*-2**2**-5**5*') # Here we are lucky and also get the first one right

      # And reverse the check with = 2
      CORRECT_ROUNDINGS = 3
      CORRECT_ROUNDINGS_LINES = ["src.cpp:999"]
      self.do_test(src.replace('TYPE', 'long long'), '*-2**2**-5**5*')
      self.do_test(src.replace('TYPE', 'int'), '*-2**2**-5**5*')

    def test_autooptimize(self):
      global CHECK_OVERFLOWS, CORRECT_OVERFLOWS, CHECK_SIGNS, CORRECT_SIGNS, AUTO_OPTIMIZE
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']

      AUTO_OPTIMIZE = CHECK_OVERFLOWS = CORRECT_OVERFLOWS = CHECK_SIGNS = CORRECT_SIGNS = 1

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
        assert 'Overflow|src.cpp:6 : 60 hits, %20 failures' in output, 'no indication of Overflow corrections'
        assert 'UnSign|src.cpp:13 : 6 hits, %16 failures' in output, 'no indication of Sign corrections'
        return output

      self.do_test(src, '*186854335,63*\n', output_nicerizer=check)


  # Generate tests for all our compilers
  def make_test(name, compiler, llvm_opts, embetter, quantum_size, typed_arrays):
    exec('''
class %s(T):
  def setUp(self):
    global COMPILER, QUANTUM_SIZE, RELOOP, OPTIMIZE, ASSERTIONS, USE_TYPED_ARRAYS, LLVM_OPTS, SAFE_HEAP, CHECK_OVERFLOWS, CORRECT_OVERFLOWS, CORRECT_OVERFLOWS_LINES, CORRECT_SIGNS, CORRECT_SIGNS_LINES, CHECK_SIGNS, COMPILER_TEST_OPTS, CORRECT_ROUNDINGS, CORRECT_ROUNDINGS_LINES, INVOKE_RUN, SAFE_HEAP_LINES, INIT_STACK, AUTO_OPTIMIZE

    COMPILER = '%s'
    llvm_opts = %d
    embetter = %d
    quantum_size = %d
    USE_TYPED_ARRAYS = %d
    INVOKE_RUN = 1
    RELOOP = OPTIMIZE = embetter
    if USE_TYPED_ARRAYS == 2: RELOOP = 0 # XXX Would be better to use this, but it isn't really what we test in this case, and is very slow
    QUANTUM_SIZE = quantum_size
    ASSERTIONS = 1-embetter
    SAFE_HEAP = 1-(embetter and llvm_opts)
    LLVM_OPTS = llvm_opts
    AUTO_OPTIMIZE = 0
    CHECK_OVERFLOWS = 1-(embetter or llvm_opts)
    CORRECT_OVERFLOWS = 1-(embetter and llvm_opts)
    CORRECT_SIGNS = 0
    CORRECT_ROUNDINGS = 0
    CORRECT_OVERFLOWS_LINES = CORRECT_SIGNS_LINES = CORRECT_ROUNDINGS_LINES = SAFE_HEAP_LINES = []
    CHECK_SIGNS = 0 #1-(embetter or llvm_opts)
    INIT_STACK = 0
    if LLVM_OPTS:
      self.pick_llvm_opts(3, True)
    COMPILER_TEST_OPTS = []
    shutil.rmtree(self.get_dir()) # Useful in debugging sometimes to comment this out
    self.get_dir() # make sure it exists
TT = %s
''' % (fullname, compiler, llvm_opts, embetter, quantum_size, typed_arrays, fullname))
    return TT

  for llvm_opts in [0,1]:
    for name, compiler, quantum, embetter, typed_arrays in [
      ('clang', CLANG, 1, 0, 0),
      ('clang', CLANG, 4, 0, 0),
      ('llvm_gcc', LLVM_GCC, 4, 0, 0),
      ('clang', CLANG, 1, 1, 1),
      ('clang', CLANG, 4, 1, 1),
      ('llvm_gcc', LLVM_GCC, 4, 1, 1),
      ('clang', CLANG, 4, 1, 2),
      #('llvm_gcc', LLVM_GCC, 4, 1, 2),
    ]:
      fullname = '%s_%d_%d%s%s' % (
        name, llvm_opts, embetter, '' if quantum == 4 else '_q' + str(quantum), '' if typed_arrays in [0, 1] else '_t' + str(typed_arrays)
      )
      exec('%s = make_test("%s","%s",%d,%d,%d,%d)' % (fullname, fullname, compiler, llvm_opts, embetter, quantum, typed_arrays))

  del T # T is just a shape for the specific subclasses, we don't test it itself

else:
  # Benchmarks

  print "Running Emscripten benchmarks..."

  sys.argv = filter(lambda x: x != 'benchmark', sys.argv)

  assert(os.path.exists(CLOSURE_COMPILER))

  USE_CLOSURE_COMPILER = 1

  if USE_CLOSURE_COMPILER:
    SPIDERMONKEY_ENGINE = filter(lambda x: x != '-s', SPIDERMONKEY_ENGINE) # closure generates non-strict

  COMPILER = CLANG
  JS_ENGINE = SPIDERMONKEY_ENGINE
  #JS_ENGINE = V8_ENGINE

  global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = []

  QUANTUM_SIZE = 1
  RELOOP = OPTIMIZE = 1
  USE_TYPED_ARRAYS = 0
  ASSERTIONS = SAFE_HEAP = CHECK_OVERFLOWS = CORRECT_OVERFLOWS = CHECK_SIGNS = INIT_STACK = AUTO_OPTIMIZE = 0
  INVOKE_RUN = 1
  CORRECT_SIGNS = 0
  CORRECT_ROUNDINGS = 0
  CORRECT_OVERFLOWS_LINES = CORRECT_SIGNS_LINES = CORRECT_ROUNDINGS_LINES = SAFE_HEAP_LINES = []
  LLVM_OPTS = 1

  TEST_REPS = 4
  TOTAL_TESTS = 6

  tests_done = 0
  total_times = map(lambda x: 0., range(TEST_REPS))
  total_native_times = map(lambda x: 0., range(TEST_REPS))

  class Benchmark(RunnerCore):
    def print_stats(self, times, native_times):
      mean = sum(times)/len(times)
      squared_times = map(lambda x: x*x, times)
      mean_of_squared = sum(squared_times)/len(times)
      std = math.sqrt(mean_of_squared - mean*mean)

      mean_native = sum(native_times)/len(native_times)
      squared_native_times = map(lambda x: x*x, native_times)
      mean_of_squared_native = sum(squared_native_times)/len(native_times)
      std_native = math.sqrt(mean_of_squared_native - mean_native*mean_native)

      print
      print '   JavaScript  : mean: %.3f (+-%.3f) seconds    (max: %.3f, min: %.3f, noise/signal: %.3f)     (%d runs)' % (mean, std, max(times), min(times), std/mean, TEST_REPS)
      print '   Native (gcc): mean: %.3f (+-%.3f) seconds    (max: %.3f, min: %.3f, noise/signal: %.3f)     JS is %.2f times slower' % (mean_native, std_native, max(native_times), min(native_times), std_native/mean_native, mean/mean_native)

    def do_benchmark(self, src, args=[], expected_output='FAIL', main_file=None):
      global USE_TYPED_ARRAYS
      self.pick_llvm_opts(3, True, USE_TYPED_ARRAYS == 2)

      dirname = self.get_dir()
      filename = os.path.join(dirname, 'src.cpp')
      self.build(src, dirname, filename, main_file=main_file)

      final_filename = filename + '.o.js'

      if USE_CLOSURE_COMPILER:
        # Optimize using closure compiler
        try:
          os.remove(filename + '.cc.js')
        except:
          pass
        # Something like this:
        #   java -jar CLOSURE_COMPILER --compilation_level ADVANCED_OPTIMIZATIONS --variable_map_output_file src.cpp.o.js.vars --js src.cpp.o.js --js_output_file src.cpp.o.cc.js

        cc_output = Popen(['java', '-jar', CLOSURE_COMPILER,
                           '--compilation_level', 'ADVANCED_OPTIMIZATIONS',
                           '--formatting', 'PRETTY_PRINT',
                           '--variable_map_output_file', filename + '.vars',
                           '--js', filename + '.o.js', '--js_output_file', filename + '.cc.js'], stdout=PIPE, stderr=STDOUT).communicate()[0]
        if 'ERROR' in cc_output:
          raise Exception('Error in cc output: ' + cc_output)

        final_filename = filename + '.cc.js'

      # Run JS
      global total_times
      times = []
      for i in range(TEST_REPS):
        start = time.time()
        js_output = self.run_generated_code(JS_ENGINE, final_filename, args, check_timeout=False)
        curr = time.time()-start
        times.append(curr)
        total_times[i] += curr
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
        total_native_times[i] += curr

      self.print_stats(times, native_times)

      global tests_done
      tests_done += 1
      if tests_done == TOTAL_TESTS:
        print
        print 'Total stats:'
        self.print_stats(total_times, total_native_times)

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
      # memcpy would also be interesting, however native code uses SSE/NEON/etc. and is much, much faster than JS can be
      src = '''
        #include<stdio.h>
        #include<string.h>
        #include<stdlib.h>
        int main() {
          int N = 4*1024*1024;
          int final = 0;
          char *buf = (char*)malloc(N);
          for (int t = 0; t < 20; t++) {
            for (int i = 0; i < N; i++)
              buf[i] = (i*i + final)%256;
            for (int i = 0; i < N; i++)
              final += buf[i] & 1;
          }
          printf("final: %d.\\n", final);
          return 1;
        }      
      '''
      self.do_benchmark(src, [], 'final: 41943040.')

    def test_fannkuch(self):
      src = open(path_from_root('tests', 'fannkuch.cpp'), 'r').read()
      self.do_benchmark(src, ['10'], 'Pfannkuchen(10) = 38.')

    def test_fasta(self):
      src = open(path_from_root('tests', 'fasta.cpp'), 'r').read()
      self.do_benchmark(src, ['2100000'], '''GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGA\nTCACCTGAGGTCAGGAGTTCGAGACCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACT\nAAAAATACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCAGCTACTCGGGAG\nGCTGAGGCAGGAGAATCGCTTGAACCCGGGAGGCGGAGGTTGCAGTGAGCCGAGATCGCG\nCCACTGCACTCCAGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAAGGCCGGGCGCGGT\nGGCTCACGCCTGTAATCCCAGCACTTTGGGAGGCCGAGGCGGGCGGATCACCTGAGGTCA\nGGAGTTCGAGACCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAATACAAAAA\nTTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCAGCTACTCGGGAGGCTGAGGCAGGAG\nAATCGCTTGAACCCGGGAGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCCA\nGCCTGGGCGA''')

    def test_raytrace(self):
      global QUANTUM_SIZE, USE_TYPED_ARRAYS
      old_quantum = QUANTUM_SIZE
      old_use_typed_arrays = USE_TYPED_ARRAYS
      QUANTUM_SIZE = 1
      USE_TYPED_ARRAYS = 0 # Rounding errors with TA2 are too big in this very rounding-sensitive code

      src = open(path_from_root('tests', 'raytrace.cpp'), 'r').read().replace('double', 'float') # benchmark with floats
      self.do_benchmark(src, ['7', '256'], '256 256')

      QUANTUM_SIZE = old_quantum
      USE_TYPED_ARRAYS = old_use_typed_arrays

    def test_dlmalloc(self):
      global COMPILER_TEST_OPTS; COMPILER_TEST_OPTS = ['-g']
      global CORRECT_SIGNS; CORRECT_SIGNS = 2
      global CORRECT_SIGNS_LINES; CORRECT_SIGNS_LINES = ['src.cpp:' + str(i) for i in [4816, 4191, 4246, 4199, 4205, 4235, 4227]]

      src = open(path_from_root('tests', 'dlmalloc.c'), 'r').read()
      self.do_benchmark(src, ['400', '400'], '*400,0*')

if __name__ == '__main__':
  sys.argv = [sys.argv[0]] + ['-v'] + sys.argv[1:] # Verbose output by default
  for cmd in [CLANG, LLVM_GCC, LLVM_DIS, SPIDERMONKEY_ENGINE[0], V8_ENGINE[0]]:
    if not os.path.exists(cmd):
      print 'WARNING: Cannot find', cmd
  unittest.main()

