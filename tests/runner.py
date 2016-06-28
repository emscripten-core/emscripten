#!/usr/bin/env python2
# This Python file uses the following encoding: utf-8

'''
Simple test runner. Consider using parallel_test_core.py for faster iteration times.
'''

# XXX Use EM_ALL_ENGINES=1 in the env to test all engines!


from subprocess import Popen, PIPE, STDOUT
import os, unittest, tempfile, shutil, time, inspect, sys, math, glob, re, difflib, webbrowser, hashlib, threading, platform, BaseHTTPServer, SimpleHTTPServer, multiprocessing, functools, stat, string, random, operator, fnmatch, httplib
from urllib import unquote

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root(''), path_from_root('third_party/websockify')]
import tools.shared
from tools.shared import *
from tools.line_endings import check_line_endings

# Sanity check for config

try:
  assert COMPILER_OPTS != None
except:
  raise Exception('Cannot find "COMPILER_OPTS" definition. Is %s set up properly? You may need to copy the template settings file into it.' % EM_CONFIG)

HELP_TEXT = '''
==============================================================================
Running the main part of the test suite. Don't forget to run the other parts!
A recommended order is:

  sanity - tests for first run, etc., modifies ~/.emscripten
  (the main test suite)
  other - tests separate from the main suite
  browser - runs pages in a web browser
  interactive - runs interactive browser tests that need human verification, and could not be automated
  sockets - runs websocket networking tests
  benchmark - run before and after each set of changes before pushing to
              master, verify no regressions

To run one of those parts, do something like

  python tests/runner.py sanity

To run a specific set of tests, you can do things like

  python tests/runner.py asm2

(that runs the asm2 (asm.js, -O2) tests). You can run individual tests with

  python tests/runner.py test_hello_world

Combinations work too, for example

  python tests/runner.py browser.test_sdl_image

In the main test suite, you can run all variations (O0, O1, O2, etc.) of
an individual test with

  python tests/runner.py ALL.test_hello_world

You can run a random set of N tests with a command like

  python tests/runner.py random50

An individual test can be skipped by passing the "skip:" prefix. E.g.

  python tests/runner.py other skip:other.test_cmake

Passing a wildcard allows choosing a subset of tests in a suite, e.g.

  python tests/runner.py browser.test_pthread_*

will run all the pthreads related tests. Wildcards can also be passed in skip,
so

  python tests/runner.py browser skip:browser.test_pthread_*

will run the whole browser suite except for all the pthread tests in it.

Debugging: You can run

  EM_SAVE_DIR=1 python tests/runner.py ALL.test_hello_world

in order to save the test runner directory, in /tmp/emscripten_temp. All files
created by the test will be present there. You can also use EMCC_DEBUG to
further debug the compiler itself, see emcc.
==============================================================================

'''

# Core test runner class, shared between normal tests and benchmarks
checked_sanity = False
test_modes = [
  'default',
  'asm1',
  'asm2',
  'asm3',
  'asm2f',
  'asm2g',
  'asm2i',
  'asm2nn'
]
test_index = 0

use_all_engines = os.environ.get('EM_ALL_ENGINES') # generally js engines are equivalent, testing 1 is enough. set this
                                                   # to force testing on all js engines, good to find js engine bugs

class RunnerCore(unittest.TestCase):
  emcc_args = None
  save_dir = os.environ.get('EM_SAVE_DIR')
  save_JS = 0
  stderr_redirect = STDOUT # This avoids cluttering the test runner output, which is stderr too, with compiler warnings etc.
                           # Change this to None to get stderr reporting, for debugging purposes

  env = {}

  EM_TESTRUNNER_DETECT_TEMPFILE_LEAKS = int(os.getenv('EM_TESTRUNNER_DETECT_TEMPFILE_LEAKS')) if os.getenv('EM_TESTRUNNER_DETECT_TEMPFILE_LEAKS') != None else 0

  temp_files_before_run = []

  def skipme(self): # used by tests we ask on the commandline to be skipped, see right before call to unittest.main
    return self.skip('requested to be skipped')

  def is_emterpreter(self):
    return False

  def uses_memory_init_file(self):
    if self.emcc_args is None:
      return None
    elif '--memory-init-file' in self.emcc_args:
      return int(self.emcc_args[self.emcc_args.index('--memory-init-file')+1])
    else:
      return ('-O2' in self.emcc_args or '-O3' in self.emcc_args or '-Oz' in self.emcc_args) and not Settings.SIDE_MODULE

  def setUp(self):
    Settings.reset()

    if self.EM_TESTRUNNER_DETECT_TEMPFILE_LEAKS:
      for root, dirnames, filenames in os.walk(TEMP_DIR):
        for dirname in dirnames: self.temp_files_before_run.append(os.path.normpath(os.path.join(root, dirname)))
        for filename in filenames: self.temp_files_before_run.append(os.path.normpath(os.path.join(root, filename)))

    self.banned_js_engines = []
    self.use_all_engines = use_all_engines
    if not self.save_dir:
      dirname = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=TEMP_DIR)
    else:
      dirname = CANONICAL_TEMP_DIR
    if not os.path.exists(dirname):
      os.makedirs(dirname)
    self.working_dir = dirname
    os.chdir(dirname)

    # Use emscripten root for node module lookup
    scriptdir = os.path.dirname(os.path.abspath(__file__))
    os.environ['NODE_PATH'] = os.path.join(scriptdir, '..', 'node_modules')

    if not self.save_dir:
      self.has_prev_ll = False
      for temp_file in os.listdir(TEMP_DIR):
        if temp_file.endswith('.ll'):
          self.has_prev_ll = True

  def tearDown(self):
    if not self.save_dir:
      # rmtree() fails on Windows if the current working directory is inside the tree.
      os.chdir(os.path.join(self.get_dir(), '..'))
      try_delete(self.get_dir())

      if self.EM_TESTRUNNER_DETECT_TEMPFILE_LEAKS and not os.environ.get('EMCC_DEBUG'):
        temp_files_after_run = []
        for root, dirnames, filenames in os.walk(TEMP_DIR):
          for dirname in dirnames: temp_files_after_run.append(os.path.normpath(os.path.join(root, dirname)))
          for filename in filenames: temp_files_after_run.append(os.path.normpath(os.path.join(root, filename)))

        left_over_files = list(set(temp_files_after_run) - set(self.temp_files_before_run))
        if len(left_over_files) > 0:
          print >> sys.stderr, 'ERROR: After running test, there are ' + str(len(left_over_files)) + ' new temporary files/directories left behind:'
          for f in left_over_files:
            print >> sys.stderr, 'leaked file: ' + f
          raise Exception('Test leaked ' + str(len(left_over_files)) + ' temporary files!')

      # Make sure we don't leave stuff around
      #if not self.has_prev_ll:
      #  for temp_file in os.listdir(TEMP_DIR):
      #    assert not temp_file.endswith('.ll'), temp_file
      #    # TODO assert not temp_file.startswith('emscripten_'), temp_file

  def skip(self, why):
    print >> sys.stderr, '<skipping: %s> ' % why,
    return False

  def get_dir(self):
    return self.working_dir

  def in_dir(self, *pathelems):
    return os.path.join(self.get_dir(), *pathelems)

  def get_stdout_path(self):
    return os.path.join(self.get_dir(), 'stdout')

  def hardcode_arguments(self, filename, args):
    # Hardcode in the arguments, so js is portable without manual commandlinearguments
    if not args: return
    js = open(filename).read()
    open(filename, 'w').write(js.replace('run();', 'run(%s + Module["arguments"]);' % str(args)))

  def prep_ll_run(self, filename, ll_file, force_recompile=False, build_ll_hook=None):
    #force_recompile = force_recompile or os.stat(filename + '.o.ll').st_size > 50000 # if the file is big, recompile just to get ll_opts # Recompiling just for dfe in ll_opts is too costly

    if Building.LLVM_OPTS or force_recompile or build_ll_hook:
      if ll_file.endswith(('.bc', '.o')):
        if ll_file != filename + '.o':
          shutil.copy(ll_file, filename + '.o')
        Building.llvm_dis(filename)
      else:
        shutil.copy(ll_file, filename + '.o.ll')

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

      Building.llvm_as(filename)
    else:
      if ll_file.endswith('.ll'):
        safe_copy(ll_file, filename + '.o.ll')
        Building.llvm_as(filename)
      else:
        safe_copy(ll_file, filename + '.o')

  # Generate JS from ll, and optionally modify the generated JS with a post_build function. Note
  # that post_build is called on unoptimized JS, so we send it to emcc (otherwise, if run after
  # emcc, it would not apply on the optimized/minified JS)
  def ll_to_js(self, filename, extra_emscripten_args, post_build):
    if type(post_build) in (list, tuple):
      post1, post2 = post_build
    else:
      post1 = post_build
      post2 = None

    emcc_args = self.emcc_args
    if emcc_args is None:
      emcc_args = []

    transform_args = []
    if post1:
      transform_filename = os.path.join(self.get_dir(), 'transform.py')
      transform = open(transform_filename, 'w')
      transform.write('\nimport sys\nsys.path += [%r]\n' % path_from_root(''))
      transform.write(post1)
      transform.write('\nprocess(sys.argv[1])\n')
      transform.close()
      transform_args = ['--js-transform', "%s %s" % (PYTHON, transform_filename)]
    Building.emcc(filename + '.o', Settings.serialize() + emcc_args + transform_args + Building.COMPILER_TEST_OPTS, filename + '.o.js')
    if post2: post2(filename + '.o.js')

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

    if build_ll_hook or post_build or extra_emscripten_args:
      # "slow", old path: build to bc, then build to JS

      # C++ => LLVM binary

      for f in [filename] + additional_files:
        try:
          # Make sure we notice if compilation steps failed
          os.remove(f + '.o')
        except:
          pass
        args = [PYTHON, EMCC] + Building.COMPILER_TEST_OPTS + Settings.serialize() + \
               ['-I', dirname, '-I', os.path.join(dirname, 'include')] + \
               map(lambda include: '-I' + include, includes) + \
               ['-c', f, '-o', f + '.o']
        output = subprocess.check_call(args, stderr=self.stderr_redirect if not DEBUG else None)
        assert os.path.exists(f + '.o')

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
    else:
      # "fast", new path: just call emcc and go straight to JS
      all_files = [filename] + additional_files + libraries
      for i in range(len(all_files)):
        if '.' not in all_files[i]:
          shutil.move(all_files[i], all_files[i] + '.bc')
          all_files[i] += '.bc'
      args = [PYTHON, EMCC] + Building.COMPILER_TEST_OPTS + Settings.serialize() + \
             self.emcc_args + \
             ['-I', dirname, '-I', os.path.join(dirname, 'include')] + \
             map(lambda include: '-I' + include, includes) + \
             all_files + \
             ['-o', filename + '.o.js']
      output = subprocess.check_call(args, stderr=self.stderr_redirect if not DEBUG else None)
      assert os.path.exists(filename + '.o.js')

    if output_processor is not None:
      output_processor(open(filename + '.o.js').read())

    if self.emcc_args is not None:
      src = open(filename + '.o.js').read()
      if self.uses_memory_init_file():
        # side memory init file, or an empty one in the js
        assert ('/* memory initializer */' not in src) or ('/* memory initializer */ allocate([]' in src)

  def validate_asmjs(self, err):
    m = re.search("asm.js type error: '(\w+)' is not a (standard|supported) SIMD type", err)
    if m:
      # Bug numbers for missing SIMD types:
      bugs = {
        'Int8x16'  : 1136226,
        'Int16x8'  : 1136226,
        'Uint8x16' : 1244117,
        'Uint16x8' : 1244117,
        'Uint32x4' : 1240796,
        'Float64x2': 1124205,
      }
      simd = m.group(1)
      if simd in bugs:
        print >> sys.stderr, ("\nWARNING: ignoring asm.js type error from {} due to implementation not yet available in SpiderMonkey." +
            " See https://bugzilla.mozilla.org/show_bug.cgi?id={}\n").format(simd, bugs[simd])
        err = err.replace(m.group(0), '')

    if 'uccessfully compiled asm.js code' in err and 'asm.js link error' not in err:
      print >> sys.stderr, "[was asm.js'ified]"
    elif 'asm.js' in err: # if no asm.js error, then not an odin build
      raise Exception("did NOT asm.js'ify: " + err)
    err = '\n'.join(filter(lambda line: 'uccessfully compiled asm.js code' not in line, err.split('\n')))
    return err

  def get_func(self, src, name):
    start = src.index('function ' + name + '(')
    t = start
    n = 0
    while True:
      if src[t] == '{': n += 1
      elif src[t] == '}':
        n -= 1
        if n == 0: return src[start:t+1]
      t += 1
      assert t < len(src)

  def count_funcs(self, javascript_file):
    num_funcs = 0
    start_tok = "// EMSCRIPTEN_START_FUNCS"
    end_tok = "// EMSCRIPTEN_END_FUNCS"
    start_off = 0
    end_off = 0

    with open (javascript_file, 'rt') as fin:
      blob = "".join(fin.readlines())
      start_off = blob.find(start_tok) + len(start_tok)
      end_off = blob.find(end_tok)
      asm_chunk = blob[start_off:end_off]
      num_funcs = asm_chunk.count('function ')

    return num_funcs

  def run_generated_code(self, engine, filename, args=[], check_timeout=True, output_nicerizer=None, assert_returncode=0):
    stdout = os.path.join(self.get_dir(), 'stdout') # use files, as PIPE can get too full and hang us
    stderr = os.path.join(self.get_dir(), 'stderr')
    try:
      cwd = os.getcwd()
    except:
      cwd = None
    os.chdir(self.get_dir())
    assert(check_line_endings(filename) == 0) # Make sure that we produced proper line endings to the .js file we are about to run.
    run_js(filename, engine, args, check_timeout, stdout=open(stdout, 'w'), stderr=open(stderr, 'w'), assert_returncode=assert_returncode)
    if cwd is not None:
      os.chdir(cwd)
    out = open(stdout, 'r').read()
    err = open(stderr, 'r').read()
    if engine == SPIDERMONKEY_ENGINE and Settings.ASM_JS == 1:
      err = self.validate_asmjs(err)
    if output_nicerizer:
      ret = output_nicerizer(out, err)
    else:
      ret = out + err
    assert 'strict warning:' not in ret, 'We should pass all strict mode checks: ' + ret
    return ret

  def build_native(self, filename, args=[]):
    compiler = CLANG if filename.endswith('cpp') else CLANG_CC
    process = Popen([compiler, '-O2', '-fno-math-errno', filename, '-o', filename+'.native'] + args, stdout=PIPE, stderr=self.stderr_redirect)
    output = process.communicate()
    if process.returncode is not 0:
      print >> sys.stderr, "Building native executable with command '%s' failed with a return code %d!" % (' '.join([CLANG, '-O2', filename, '-o', filename+'.native']), process.returncode)
      print "Output: " + output[0]

  def run_native(self, filename, args):
    process = Popen([filename+'.native'] + args, stdout=PIPE);
    output = process.communicate()
    if process.returncode is not 0:
      print >> sys.stderr, "Running native executable with command '%s' failed with a return code %d!" % (' '.join([filename+'.native'] + args), process.returncode)
      print "Output: " + output[0]
    return output[0]

  # Tests that the given two paths are identical, modulo path delimiters. E.g. "C:/foo" is equal to "C:\foo".
  def assertPathsIdentical(self, path1, path2):
    path1 = path1.replace('\\', '/')
    path2 = path2.replace('\\', '/')
    return self.assertIdentical(path1, path2)

  # Tests that the given two multiline text content are identical, modulo line ending differences (\r\n on Windows, \n on Unix).
  def assertTextDataIdentical(self, text1, text2):
    text1 = text1.replace('\r\n', '\n')
    text2 = text2.replace('\r\n', '\n')
    return self.assertIdentical(text1, text2)

  def assertIdentical(self, values, y):
    if type(values) not in [list, tuple]: values = [values]
    for x in values:
      if x == y: return # success
    raise Exception("Expected to have '%s' == '%s', diff:\n\n%s" % (
      limit_size(values[0]), limit_size(y),
      limit_size(''.join([a.rstrip()+'\n' for a in difflib.unified_diff(x.split('\n'), y.split('\n'), fromfile='expected', tofile='actual')]))
    ))

  def assertTextDataContained(self, text1, text2):
    text1 = text1.replace('\r\n', '\n')
    text2 = text2.replace('\r\n', '\n')
    return self.assertContained(text1, text2)

  def assertContained(self, values, string, additional_info=''):
    if type(values) not in [list, tuple]: values = [values]
    for value in values:
      if type(value) is unicode: string = string.decode('UTF-8') # If we have any non-ASCII chars in the expected string, treat the test string from ASCII as UTF8 as well.
      if type(string) is not str and type(string) is not unicode: string = string()
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

  def get_library(self, name, generated_libs, configure=['sh', './configure'], configure_args=[], make=['make'], make_args='help', cache=True, env_init={}, cache_name_extra='', native=False):
    if make_args == 'help':
      make_args = ['-j', str(multiprocessing.cpu_count())]

    build_dir = self.get_build_dir()
    output_dir = self.get_dir()

    cache_name = name + ','.join(filter(lambda opt: len(opt) < 10, Building.COMPILER_TEST_OPTS)) + '_' + hashlib.md5(str(Building.COMPILER_TEST_OPTS)).hexdigest() + cache_name_extra

    valid_chars = "_%s%s" % (string.ascii_letters, string.digits)
    cache_name = ''.join([(c if c in valid_chars else '_') for c in cache_name])

    if self.library_cache is not None:
      if cache and self.library_cache.get(cache_name):
        print >> sys.stderr,  '<load %s from cache> ' % cache_name
        generated_libs = []
        for basename, contents in self.library_cache[cache_name]:
          bc_file = os.path.join(build_dir, cache_name + '_' +  basename)
          f = open(bc_file, 'wb')
          f.write(contents)
          f.close()
          generated_libs.append(bc_file)
        return generated_libs

    print >> sys.stderr, '<building and saving %s into cache> ' % cache_name

    return Building.build_library(name, build_dir, output_dir, generated_libs, configure, configure_args, make, make_args, self.library_cache, cache_name,
                                  copy_project=True, env_init=env_init, native=native)

  def clear(self, in_curr=False):
    for name in os.listdir(self.get_dir()):
      try_delete(os.path.join(self.get_dir(), name) if not in_curr else name)
    emcc_debug = os.environ.get('EMCC_DEBUG')
    if emcc_debug and not in_curr and EMSCRIPTEN_TEMP_DIR:
      for name in os.listdir(EMSCRIPTEN_TEMP_DIR):
        try_delete(os.path.join(EMSCRIPTEN_TEMP_DIR, name))

  # Shared test code between main suite and others

  def setup_runtimelink_test(self):
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
    supp_name = os.path.join(self.get_dir(), 'supp.cpp')
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
        #ifdef BROWSER
          int result = suppInt;
          REPORT_RESULT();
        #endif
        return 0;
      }
    '''
    return (main, supp)

  def do_run_from_file(self, src, expected_output, args=[], output_nicerizer=None, output_processor=None, no_build=False, main_file=None, additional_files=[], js_engines=None, post_build=None, basename='src.cpp', libraries=[], includes=[], force_c=False, build_ll_hook=None, extra_emscripten_args=[]):
    self.do_run(open(src).read(), open(expected_output).read(),
                args, output_nicerizer, output_processor, no_build, main_file,
                additional_files, js_engines, post_build, basename, libraries,
                includes, force_c, build_ll_hook, extra_emscripten_args)

  ## Does a complete test - builds, runs, checks output, etc.
  def do_run(self, src, expected_output, args=[], output_nicerizer=None, output_processor=None, no_build=False, main_file=None, additional_files=[], js_engines=None, post_build=None, basename='src.cpp', libraries=[], includes=[], force_c=False, build_ll_hook=None, extra_emscripten_args=[], assert_returncode=None):
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
    for engine in js_engines: assert type(engine) == list
    for engine in self.banned_js_engines: assert type(engine) == list
    js_engines = filter(lambda engine: engine[0] not in map(lambda engine: engine[0], self.banned_js_engines), js_engines)
    if len(js_engines) == 0: return self.skip('No JS engine present to run this test with. Check %s and the paths therein.' % EM_CONFIG)
    if len(js_engines) > 1 and not self.use_all_engines:
      if SPIDERMONKEY_ENGINE in js_engines: # make sure to get asm.js validation checks, using sm
        js_engines = [SPIDERMONKEY_ENGINE]
      else:
        js_engines = js_engines[:1]
    for engine in js_engines:
      #print 'test in', engine
      js_output = self.run_generated_code(engine, filename + '.o.js', args, output_nicerizer=output_nicerizer, assert_returncode=assert_returncode)
      try:
        self.assertContained(expected_output, js_output.replace('\r\n', '\n'))
        self.assertNotContained('ERROR', js_output)
      except Exception, e:
        print '(test did not pass in JS engine: %s)' % engine
        raise e

    #shutil.rmtree(dirname) # TODO: leave no trace in memory. But for now nice for debugging

    if self.save_JS:
      global test_index
      self.hardcode_arguments(filename + '.o.js', args)
      shutil.copyfile(filename + '.o.js', os.path.join(TEMP_DIR, str(test_index) + '.js'))
      test_index += 1

  # No building - just process an existing .ll file (or .bc, which we turn into .ll)
  def do_ll_run(self, ll_file, expected_output=None, args=[], js_engines=None, output_nicerizer=None, post_build=None, force_recompile=False, build_ll_hook=None, extra_emscripten_args=[], assert_returncode=None):
    filename = os.path.join(self.get_dir(), 'src.cpp')

    self.prep_ll_run(filename, ll_file, force_recompile, build_ll_hook)

    self.ll_to_js(filename, extra_emscripten_args, post_build)

    self.do_run(None,
                expected_output,
                args,
                no_build=True,
                js_engines=js_engines,
                output_nicerizer=output_nicerizer,
                post_build=None,
                assert_returncode=assert_returncode) # post_build was already done in ll_to_js, this do_run call is just to test the output


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
    def log_request(code=0, size=0):
      # don't log; too noisy
      pass
  httpd = BaseHTTPServer.HTTPServer(('localhost', 9999), TestServerHandler)
  httpd.serve_forever() # test runner will kill us

def server_func(dir, q):
  class TestServerHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
      if 'report_' in self.path:
        print '[server response:', self.path, ']'
        q.put(self.path)
      else:
        # Use SimpleHTTPServer default file serving operation for GET.
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

    def log_request(code=0, size=0):
      # don't log; too noisy
      pass
  os.chdir(dir)
  httpd = BaseHTTPServer.HTTPServer(('localhost', 8888), TestServerHandler)
  httpd.serve_forever() # test runner will kill us

class BrowserCore(RunnerCore):
  def __init__(self, *args, **kwargs):
    super(BrowserCore, self).__init__(*args, **kwargs)

  @classmethod
  def setUpClass(self):
    super(BrowserCore, self).setUpClass()
    self.browser_timeout = 30
    self.harness_queue = multiprocessing.Queue()
    self.harness_server = multiprocessing.Process(target=harness_server_func, args=(self.harness_queue,))
    self.harness_server.start()
    print '[Browser harness server on process %d]' % self.harness_server.pid
    webbrowser.open_new('http://localhost:9999/run_harness')

  @classmethod
  def tearDownClass(self):
    super(BrowserCore, self).tearDownClass()
    self.harness_server.terminate()
    print '[Browser harness server terminated]'
    # On Windows, shutil.rmtree() in tearDown() raises this exception if we do not wait a bit:
    # WindowsError: [Error 32] The process cannot access the file because it is being used by another process.
    time.sleep(0.1)

  def run_browser(self, html_file, message, expectedResult=None, timeout=None):
    print '[browser launch:', html_file, ']'
    if expectedResult is not None:
      try:
        queue = multiprocessing.Queue()
        server = multiprocessing.Process(target=functools.partial(server_func, self.get_dir()), args=(queue,))
        server.start()
        # Starting the web page server above is an asynchronous procedure, so before we tell the browser below to navigate to
        # the test page, we need to know that the server has started up and is ready to process the site navigation.
        # Therefore block until we can make a connection to the server.
        for i in range(10):
          httpconn = httplib.HTTPConnection('localhost:8888', timeout=1)
          try:
            httpconn.connect()
            httpconn.close()
            break
          except:
            time.sleep(1)
        else:
          raise Exception('[Test harness server failed to start up in a timely manner]')
        self.harness_queue.put('http://localhost:8888/' + html_file)
        output = '[no http server activity]'
        start = time.time()
        if timeout is None: timeout = self.browser_timeout
        while time.time() - start < timeout:
          if not queue.empty():
            output = queue.get()
            break
          time.sleep(0.1)
        if output.startswith('/report_result?skipped:'):
          self.skip(unquote(output[len('/report_result?skipped:'):]).strip())
        else:
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
    return r'''
#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
  #define REPORT_RESULT_INTERNAL(sync) \
    EM_ASM_({ \
      var xhr = new XMLHttpRequest(); \
      xhr.open('GET', 'http://localhost:8888/report_result?' + $0, !$1); \
      xhr.send(); \
      setTimeout(function() { window.close() }, 1000); \
    }, result, sync);
  #define REPORT_RESULT() REPORT_RESULT_INTERNAL(0)
#endif
''' + code

  def reftest(self, expected):
    # make sure the pngs used here have no color correction, using e.g.
    #   pngcrush -rem gAMA -rem cHRM -rem iCCP -rem sRGB infile outfile
    basename = os.path.basename(expected)
    shutil.copyfile(expected, os.path.join(self.get_dir(), basename))
    open(os.path.join(self.get_dir(), 'reftest.js'), 'w').write('''
      var Module = eval('Module');
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
            /*
            document.body.appendChild(img); // for comparisons
            var div = document.createElement('div');
            div.innerHTML = '^=expected, v=actual';
            document.body.appendChild(div);
            document.body.appendChild(actualImage); // to grab it for creating the test reference
            */

            var actualCanvas = document.createElement('canvas');
            actualCanvas.width = actualImage.width;
            actualCanvas.height = actualImage.height;
            var actualCtx = actualCanvas.getContext('2d');
            actualCtx.drawImage(actualImage, 0, 0);
            var actual = actualCtx.getImageData(0, 0, actualImage.width, actualImage.height).data;

            var total = 0;
            var width = img.width;
            var height = img.height;
            for (var x = 0; x < width; x++) {
              for (var y = 0; y < height; y++) {
                total += Math.abs(expected[y*width*4 + x*4 + 0] - actual[y*width*4 + x*4 + 0]);
                total += Math.abs(expected[y*width*4 + x*4 + 1] - actual[y*width*4 + x*4 + 1]);
                total += Math.abs(expected[y*width*4 + x*4 + 2] - actual[y*width*4 + x*4 + 2]);
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

      if (typeof WebGLClient !== 'undefined') {
        // trigger reftest from RAF as well, needed for workers where there is no pre|postRun on the main thread
        var realRAF = window.requestAnimationFrame;
        window.requestAnimationFrame = function(func) {
          realRAF(function() {
            func();
            realRAF(doReftest);
          });
        };

        // trigger reftest from canvas render too, for workers not doing GL
        var realWOM = worker.onmessage;
        worker.onmessage = function(event) {
          realWOM(event);
          if (event.data.target === 'canvas' && event.data.op === 'render') {
            realRAF(doReftest);
          }
        };
      }

''' % basename)

  def btest(self, filename, expected=None, reference=None, force_c=False, reference_slack=0, manual_reference=False, post_build=None,
      args=[], outfile='test.html', message='.', also_proxied=False, url_suffix='', timeout=None): # TODO: use in all other tests
    # if we are provided the source and not a path, use that
    filename_is_src = '\n' in filename
    src = filename if filename_is_src else ''
    filepath = path_from_root('tests', filename) if not filename_is_src else ('main.c' if force_c else 'main.cpp')
    temp_filepath = os.path.join(self.get_dir(), os.path.basename(filepath))
    original_args = args[:]
    if filename_is_src:
      with open(temp_filepath, 'w') as f: f.write(src)
    if not reference:
      if not src:
        with open(filepath) as f: src = f.read()
      with open(temp_filepath, 'w') as f: f.write(self.with_report_result(src))
    else:
      self.reference = reference
      expected = [str(i) for i in range(0, reference_slack+1)]
      shutil.copyfile(filepath, temp_filepath)
      self.reftest(path_from_root('tests', reference))
      if not manual_reference:
        args = args + ['--pre-js', 'reftest.js', '-s', 'GL_TESTING=1']
    all_args = [PYTHON, EMCC, '-s', 'IN_TEST_HARNESS=1', temp_filepath, '-o', outfile] + args
    #print 'all args:', all_args
    try_delete(outfile)
    Popen(all_args).communicate()
    assert os.path.exists(outfile)
    if post_build: post_build()
    if type(expected) is str: expected = [expected]
    self.run_browser(outfile + url_suffix, message, ['/report_result?' + e for e in expected], timeout=timeout)
    if also_proxied:
      print 'proxied...'
      # save non-proxied
      if not os.path.exists('normal'):
        os.mkdir('normal')
      shutil.copyfile('test.html', os.path.join('normal', 'test.html'))
      shutil.copyfile('test.js', os.path.join('normal', 'test.js'))
      if reference:
        assert not manual_reference
        manual_reference = True
        assert not post_build
        post_build = self.post_manual_reftest
      # run proxied
      self.btest(filename, expected, reference, force_c, reference_slack, manual_reference, post_build, original_args + ['--proxy-to-worker', '-s', 'GL_TESTING=1'], outfile, message, timeout=timeout)

###################################################################################################

def get_zlib_library(runner_core):
  if WINDOWS:
    return runner_core.get_library('zlib', os.path.join('libz.a'), configure=['emconfigure.bat'], configure_args=['cmake', '.', '-DBUILD_SHARED_LIBS=OFF'], make=['mingw32-make'], make_args=[])
  else:
    return runner_core.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a'])

# Both test_core and test_other access the Bullet library, share the access here to avoid duplication.
def get_bullet_library(runner_core, use_cmake):
  if use_cmake:
    configure_commands = ['cmake', '.']
    configure_args = ['-DBUILD_DEMOS=OFF', '-DBUILD_EXTRAS=OFF', '-DUSE_GLUT=OFF']
    # Depending on whether 'configure' or 'cmake' is used to build, Bullet places output files in different directory structures.
    generated_libs = [os.path.join('src', 'BulletDynamics', 'libBulletDynamics.a'),
                      os.path.join('src', 'BulletCollision', 'libBulletCollision.a'),
                      os.path.join('src', 'LinearMath', 'libLinearMath.a')]
  else:
    configure_commands = ['sh', './configure']
    # Force a nondefault --host= so that the configure script will interpret that we are doing cross-compilation
    # and skip attempting to run the generated executable with './a.out', which would fail since we are building a .js file.
    configure_args = ['--host=i686-pc-linux-gnu', '--disable-demos','--disable-dependency-tracking']
    generated_libs = [os.path.join('src', '.libs', 'libBulletDynamics.a'),
                      os.path.join('src', '.libs', 'libBulletCollision.a'),
                      os.path.join('src', '.libs', 'libLinearMath.a')]

  return runner_core.get_library('bullet', generated_libs, configure=configure_commands, configure_args=configure_args, cache_name_extra=configure_commands[0])

if __name__ == '__main__':
  if len(sys.argv) == 2 and sys.argv[1] in ['--help', '-h']:
    print HELP_TEXT
    sys.exit(0)

  # If no tests were specified, run the core suite
  if len(sys.argv) == 1:
    sys.argv = [sys.argv[0]] + map(lambda mode: mode, test_modes)
    print HELP_TEXT
    time.sleep(2)

  if use_all_engines:
    print '(using ALL js engines)'
  else:
    logging.warning('use EM_ALL_ENGINES=1 in the env to run against all JS engines, which is slower but provides more coverage')

  # Sanity checks
  total_engines = len(JS_ENGINES)
  JS_ENGINES = filter(check_engine, JS_ENGINES)
  if len(JS_ENGINES) == 0:
    print 'WARNING: None of the JS engines in JS_ENGINES appears to work.'
  elif len(JS_ENGINES) < total_engines:
    print 'WARNING: Not all the JS engines in JS_ENGINES appears to work, ignoring those.'

  # Create a list of modules to load tests from
  modules = []
  for filename in glob.glob(os.path.join(os.path.dirname(__file__), 'test*.py')):
    module_dir, module_file = os.path.split(filename)
    module_name, module_ext = os.path.splitext(module_file)
    __import__(module_name)
    modules.append(sys.modules[module_name])

  # Extract the JS engine override from the arguments (used by benchmarks)
  for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if arg.isupper():
      print 'Interpreting all capital argument "%s" as JS_ENGINE override' % arg
      Building.JS_ENGINE_OVERRIDE = eval(arg)
      sys.argv[i] = None
  sys.argv = filter(lambda arg: arg is not None, sys.argv)

  # If an argument comes in as test_*, treat it as a test of the default suite
  sys.argv = map(lambda arg: arg if not arg.startswith('test_') else 'default.' + arg, sys.argv)

  # If a test (e.g. test_html) is specified as ALL.test_html, add an entry for each test_mode
  new_args = [sys.argv[0]]
  for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if arg.startswith('ALL.'):
      ignore, test = arg.split('.')
      print 'Running all test modes on test "%s"' % test
      new_args += map(lambda mode: mode+'.'+test, test_modes)
    else:
      new_args += [arg]
  sys.argv = new_args

  # Create a list of all known tests so that we can choose from them based on a wildcard search
  all_tests = []
  suites = test_modes + ['other', 'browser', 'sanity', 'sockets', 'interactive']
  for m in modules:
    for s in suites:
      if hasattr(m, s):
        tests = filter(lambda t: t.startswith('test_'), dir(getattr(m, s)))
        all_tests += map(lambda t: s + '.' + t, tests)

  # Process wildcards, e.g. "browser.test_pthread_*" should expand to list all pthread tests
  new_args = [sys.argv[0]]
  for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if '*' in arg:
      if arg.startswith('skip:'):
        arg = arg[5:]
        matching_tests = fnmatch.filter(all_tests, arg)
        new_args += map(lambda t: 'skip:' + t, matching_tests)
      else:
        new_args += fnmatch.filter(all_tests, arg)
    else:
      new_args += [arg]
  if len(new_args) == 1 and len(sys.argv) > 1:
    print 'No tests found to run in set ' + str(sys.argv[1:])
    sys.exit(0)
  sys.argv = new_args

  # Skip requested tests
  for i in range(len(sys.argv)):
    arg = sys.argv[i]
    if arg.startswith('skip:'):
      which = arg.split('skip:')[1]
      if which.startswith('ALL.'):
        ignore, test = which.split('.')
        which = map(lambda mode: mode+'.'+test, test_modes)
      else:
        which = [which]

      print >> sys.stderr, ','.join(which)
      for test in which:
        print >> sys.stderr, 'will skip "%s"' % test
        for m in modules:
          try:
            exec('m.' + test + ' = RunnerCore("skipme")')
            break
          except:
            pass
      sys.argv[i] = None
  sys.argv = filter(lambda arg: arg is not None, sys.argv)

  # If we were asked to run random tests, do that
  first = sys.argv[1]
  if first.startswith('random'):
    num = 1
    first = first[6:]
    base_module = 'default'
    relevant_modes = test_modes
    if len(first) > 0:
      if first.startswith('other'):
        base_module = 'other'
        relevant_modes = ['other']
        first = first.replace('other', '')
      elif first.startswith('browser'):
        base_module = 'browser'
        relevant_modes = ['browser']
        first = first.replace('browser', '')
      num = int(first)
    for m in modules:
      if hasattr(m, base_module):
        sys.argv = [sys.argv[0]]
        tests = filter(lambda t: t.startswith('test_'), dir(getattr(m, base_module)))
        print
        chosen = set()
        while len(chosen) < num:
          test = random.choice(tests)
          mode = random.choice(relevant_modes)
          new_test = mode + '.' + test
          before = len(chosen)
          chosen.add(new_test)
          if len(chosen) > before:
            print '* ' + new_test
          else:
            # we may have hit the limit
            if len(chosen) == len(tests)*len(relevant_modes):
              print '(all possible tests chosen! %d = %d*%d)' % (len(chosen), len(tests), len(relevant_modes))
              break
        sys.argv += list(chosen)
        std = 0.5/math.sqrt(num)
        print
        print 'running those %d randomly-selected tests. if they all pass, then there is a greater than 95%% chance that at least %.2f%% of the test suite will pass' % (num, 100.0-100.0*std)
        print

        import atexit
        def show():
          print 'if all tests passed then there is a greater than 95%% chance that at least %.2f%% of the test suite will pass' % (100.0-100.0*std)
        atexit.register(show)

  # Filter and load tests from the discovered modules
  loader = unittest.TestLoader()
  names = set(sys.argv[1:])
  suites = []
  for m in modules:
    mnames = []
    for name in list(names):
      try:
        operator.attrgetter(name)(m)
        mnames.append(name)
        names.remove(name)
      except AttributeError:
        pass
    if len(mnames) > 0:
      suites.append((m.__name__, loader.loadTestsFromNames(sorted(mnames), m)))

  resultMessages = []
  numFailures = 0

  if len(names) > 0:
    print 'WARNING: could not find the following tests: ' + ' '.join(names)
    numFailures += len(names)
    resultMessages.append('Could not find %s tests' % (len(names),))

  # Run the discovered tests
  testRunner = unittest.TextTestRunner(verbosity=2)
  for mod_name, suite in suites:
    res = testRunner.run(suite)
    msg = '%s: %s run, %s errors, %s failures, %s skipped' % (mod_name,
        res.testsRun, len(res.errors), len(res.failures), len(res.skipped)
    )
    numFailures += len(res.errors) + len(res.failures)
    resultMessages.append(msg)

  if len(resultMessages) > 1:
    print '===================='
    print
    print 'TEST SUMMARY'
    for msg in resultMessages:
      print '    ' + msg

  # Return the number of failures as the process exit code for automating success/failure reporting.
  exitcode = min(numFailures, 255)
  sys.exit(exitcode)

