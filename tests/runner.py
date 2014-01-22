#!/usr/bin/env python2
# This Python file uses the following encoding: utf-8

'''
Simple test runner

These tests can be run in parallel using nose, for example

  nosetests --processes=4 -v -s tests/runner.py

will use 4 processes. To install nose do something like
|pip install nose| or |sudo apt-get install python-nose|.

Note however that emcc now uses multiple cores when optimizing,
so you may prefer to use fewer cores here.
'''

from subprocess import Popen, PIPE, STDOUT
import os, unittest, tempfile, shutil, time, inspect, sys, math, glob, re, difflib, webbrowser, hashlib, threading, platform, BaseHTTPServer, SimpleHTTPServer, multiprocessing, functools, stat, string

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root(''), path_from_root('third_party/websockify')]
import tools.shared
from tools.shared import *

# Sanity check for config

try:
  assert COMPILER_OPTS != None
except:
  raise Exception('Cannot find "COMPILER_OPTS" definition. Is %s set up properly? You may need to copy the template settings file into it.' % EM_CONFIG)

# Core test runner class, shared between normal tests and benchmarks
checked_sanity = False
test_modes = ['default', 'o1', 'o2', 'asm1', 'asm2', 'asm3', 'asm2f', 'asm2g', 'asm2x86', 's_0_0', 's_0_1']
test_index = 0

class RunnerCore(unittest.TestCase):
  emcc_args = None
  save_dir = os.environ.get('EM_SAVE_DIR')
  save_JS = 0
  stderr_redirect = STDOUT # This avoids cluttering the test runner output, which is stderr too, with compiler warnings etc.
                           # Change this to None to get stderr reporting, for debugging purposes

  env = {}

  def skipme(self): # used by tests we ask on the commandline to be skipped, see right before call to unittest.main
    return self.skip('requested to be skipped')

  def setUp(self):
    Settings.reset()
    self.banned_js_engines = []
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
      shutil.rmtree(self.get_dir())

      # Make sure we don't leave stuff around
      #if not self.has_prev_ll:
      #  for temp_file in os.listdir(TEMP_DIR):
      #    assert not temp_file.endswith('.ll'), temp_file
      #    # TODO assert not temp_file.startswith('emscripten_'), temp_file

  def skip(self, why):
    print >> sys.stderr, '<skipping: %s> ' % why,

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

    if self.emcc_args is None:
      Building.emscripten(filename, append_ext=True, extra_args=extra_emscripten_args)
      if post1:
        exec post1 in locals()
        shutil.copyfile(filename + '.o.js', filename + '.o.js.prepost.js')
        process(filename + '.o.js')
      if post2: post2(filename + '.o.js')
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
        transform_args = ['--js-transform', "%s %s" % (PYTHON, transform_filename)]
      Building.emcc(filename + '.o.ll', Settings.serialize() + self.emcc_args + transform_args + Building.COMPILER_TEST_OPTS, filename + '.o.js')
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
      output = Popen(args, stdout=PIPE, stderr=self.stderr_redirect if not DEBUG else None).communicate()[0]
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

    if self.emcc_args is not None and 'ASM_JS=1' in self.emcc_args:
      if '--memory-init-file' in self.emcc_args:
        memory_init_file = int(self.emcc_args[self.emcc_args.index('--memory-init-file')+1])
      else:
        memory_init_file = 0
      if memory_init_file:
        assert '/* memory initializer */' not in open(filename + '.o.js').read()
      else:
        assert 'memory initializer */' in open(filename + '.o.js').read()

  def validate_asmjs(self, err):
    if 'uccessfully compiled asm.js code' in err and 'asm.js link error' not in err:
      print >> sys.stderr, "[was asm.js'ified]"
    elif 'asm.js' in err: # if no asm.js error, then not an odin build
      raise Exception("did NOT asm.js'ify")
    err = '\n'.join(filter(lambda line: 'uccessfully compiled asm.js code' not in line, err.split('\n')))
    return err

  def run_generated_code(self, engine, filename, args=[], check_timeout=True, output_nicerizer=None):
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

  def get_library(self, name, generated_libs, configure=['sh', './configure'], configure_args=[], make=['make'], make_args='help', cache=True, env_init={}, cache_name_extra='', native=False):
    if make_args == 'help':
      make_args = ['-j', str(multiprocessing.cpu_count())]

    build_dir = self.get_build_dir()
    output_dir = self.get_dir()

    cache_name = name + str(Building.COMPILER_TEST_OPTS) + cache_name_extra + (self.env.get('EMCC_LLVM_TARGET') or '')

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
    if emcc_debug and not in_curr:
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
    if len(js_engines) == 0: return self.skip('No JS engine present to run this test with. Check %s and the paths therein.' % EM_CONFIG)
    for engine in js_engines:
      js_output = self.run_generated_code(engine, filename + '.o.js', args, output_nicerizer=output_nicerizer)
      self.assertContained(expected_output, js_output.replace('\r\n', '\n'))
      self.assertNotContained('ERROR', js_output)

    #shutil.rmtree(dirname) # TODO: leave no trace in memory. But for now nice for debugging

    if self.save_JS:
      global test_index
      self.hardcode_arguments(filename + '.o.js', args)
      shutil.copyfile(filename + '.o.js', os.path.join(TEMP_DIR, str(test_index) + '.js'))
      test_index += 1

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

  def run_browser(self, html_file, message, expectedResult=None):
    if expectedResult is not None:
      try:
        queue = multiprocessing.Queue()
        server = multiprocessing.Process(target=functools.partial(server_func, self.get_dir()), args=(queue,))
        server.start()
        self.harness_queue.put('http://localhost:8888/' + html_file)
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
    return r'''
      #if EMSCRIPTEN
      #include <emscripten.h>
      #define REPORT_RESULT_INTERNAL(sync) \
        char output[1000]; \
        sprintf(output, \
                "xhr = new XMLHttpRequest();" \
                "xhr.open('GET', 'http://localhost:8888/report_result?%d'%s);" \
                "xhr.send();", result, sync ? ", false" : ""); \
        emscripten_run_script(output); \
        emscripten_run_script("setTimeout(function() { window.close() }, 1000)"); // comment this out to keep the test runner window open to debug
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
      Module['preRun'].push(function() {
        setTimeout(doReftest, 1000); // if run() throws an exception and postRun is not called, this will kick in
      });
''' % basename)

  def btest(self, filename, expected=None, reference=None, force_c=False, reference_slack=0, manual_reference=False, post_build=None,
      args=[], outfile='test.html', message='.'): # TODO: use in all other tests
    if os.environ.get('EMCC_FAST_COMPILER') == '1' and 'LEGACY_GL_EMULATION=1' in args: return self.skip('no legacy gl emulation in fastcomp')
    # if we are provided the source and not a path, use that
    filename_is_src = '\n' in filename
    src = filename if filename_is_src else ''
    filepath = path_from_root('tests', filename) if not filename_is_src else ('main.c' if force_c else 'main.cpp')
    temp_filepath = os.path.join(self.get_dir(), os.path.basename(filepath))
    if filename_is_src:
      with open(temp_filepath, 'w') as f: f.write(src)
    if not reference:
      if not src:
        with open(filepath) as f: src = f.read()
      with open(temp_filepath, 'w') as f: f.write(self.with_report_result(src))
    else:
      expected = [str(i) for i in range(0, reference_slack+1)]
      shutil.copyfile(filepath, temp_filepath)
      self.reftest(path_from_root('tests', reference))
      if not manual_reference:
        args = args + ['--pre-js', 'reftest.js', '-s', 'GL_TESTING=1']
    Popen([PYTHON, EMCC, temp_filepath, '-o', outfile] + args).communicate()
    assert os.path.exists(outfile)
    if post_build: post_build()
    if type(expected) is str: expected = [expected]
    self.run_browser(outfile, message, ['/report_result?' + e for e in expected])

###################################################################################################

# Both test_core and test_other access the Bullet library, share the access here to avoid duplication.
def get_bullet_library(runner_core, use_cmake):
  if use_cmake:
    configure_commands = ['cmake', '.']
    configure_args = ['-DBUILD_DEMOS=OFF', '-DBUILD_EXTRAS=OFF']
    # Depending on whether 'configure' or 'cmake' is used to build, Bullet places output files in different directory structures.
    generated_libs = [os.path.join('src', 'BulletDynamics', 'libBulletDynamics.a'),
                      os.path.join('src', 'BulletCollision', 'libBulletCollision.a'),
                      os.path.join('src', 'LinearMath', 'libLinearMath.a')]
  else:
    configure_commands = ['sh', './configure']
    configure_args = ['--disable-demos','--disable-dependency-tracking']
    generated_libs = [os.path.join('src', '.libs', 'libBulletDynamics.a'),
                      os.path.join('src', '.libs', 'libBulletCollision.a'),
                      os.path.join('src', '.libs', 'libLinearMath.a')]

  return runner_core.get_library('bullet', generated_libs, configure=configure_commands, configure_args=configure_args, cache_name_extra=configure_commands[0])

if __name__ == '__main__':
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
  if len(sys.argv) == 2 and sys.argv[1].startswith('ALL.'):
    ignore, test = sys.argv[1].split('.')
    print 'Running all test modes on test "%s"' % test
    sys.argv = [sys.argv[0]] + map(lambda mode: mode+'.'+test, test_modes)

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

  # If no tests were specified, run the core suite
  if len(sys.argv) == 1:
    sys.argv = [sys.argv[0]] + map(lambda mode: mode, test_modes)
    print '''
==============================================================================
Running the main part of the test suite. Don't forget to run the other parts!
A recommended order is:

  sanity - tests for first run, etc., modifies ~/.emscripten
  (the main test suite)
  other - tests separate from the main suite
  browser - runs pages in a web browser
  sockets - runs websocket networking tests
  benchmark - run before and after each set of changes before pushing to
              master, verify no regressions

There are also commands to run specific subsets of the test suite:

  browser.audio - runs audio tests in a web browser (requires human verification)

To run one of those parts, do something like

  python tests/runner.py sanity

To run a specific set of tests, you can do things like

  python tests/runner.py o1

(that runs the o1 (-O1) tests). You can run individual tests with

  python tests/runner.py test_hello_world

Combinations work too, for example

  python tests/runner.py browser.test_sdl_image

In the main test suite, you can run all variations (O0, O1, O2, etc.) of
an individual test with

  python tests/runner.py ALL.test_hello_world

==============================================================================

'''
    time.sleep(2)

  # Filter and load tests from the discovered modules
  loader = unittest.TestLoader()
  names = sys.argv[1:]
  suites = []
  for m in modules:
    try:
      suites.append(loader.loadTestsFromNames(names, m))
    except:
      pass

  numFailures = 0 # Keep count of the total number of failing tests.

  # Run the discovered tests
  if not len(suites):
    print >> sys.stderr, 'No tests found for %s' % str(sys.argv[1:])
    numFailures = 1
  else:
    testRunner = unittest.TextTestRunner(verbosity=2)
    for suite in suites:
      results = testRunner.run(suite)
      numFailures += len(results.errors) + len(results.failures)

  # Return the number of failures as the process exit code for automating success/failure reporting.
  exit(numFailures)

