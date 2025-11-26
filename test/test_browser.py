# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import argparse
import os
import random
import re
import shlex
import shutil
import subprocess
import time
import zlib
from functools import wraps
from http.server import (
  BaseHTTPRequestHandler,
  HTTPServer,
  SimpleHTTPRequestHandler,
  ThreadingHTTPServer,
)
from pathlib import Path
from urllib.request import urlopen

import common
from browser_common import (
  CHROMIUM_BASED_BROWSERS,
  BrowserCore,
  HttpServerThread,
  Reporting,
  browser_should_skip_feature,
  find_browser_test_file,
  get_browser,
  get_safari_version,
  has_browser,
  is_chrome,
  is_firefox,
  is_safari,
)
from common import (
  EMRUN,
  WEBIDL_BINDER,
  RunnerCore,
  copytree,
  create_file,
  ensure_dir,
  path_from_root,
  read_file,
  test_file,
)
from decorators import (
  also_with_asan,
  also_with_fetch_streaming,
  also_with_minimal_runtime,
  also_with_wasm2js,
  also_with_wasmfs,
  disabled,
  flaky,
  no_2gb,
  no_4gb,
  no_wasm64,
  parameterize,
  parameterized,
  requires_dev_dependency,
  requires_wasm2js,
  skip_if,
  skip_if_simple,
  with_all_sjlj,
)

from tools import ports, shared
from tools.feature_matrix import Feature
from tools.shared import EMCC, FILE_PACKAGER, PIPE
from tools.utils import WINDOWS, delete_dir


def make_test_chunked_synchronous_xhr_server(support_byte_ranges, data, port):
  class ChunkedServerHandler(BaseHTTPRequestHandler):
    num_get_connections = 0

    def sendheaders(s, extra=None, length=None):
      length = length or len(data)
      s.send_response(200)
      s.send_header("Content-Length", str(length))
      s.send_header("Access-Control-Allow-Origin", "http://localhost:%s" % port)
      s.send_header('Cross-Origin-Resource-Policy', 'cross-origin')

      s.send_header('Cache-Control', 'no-cache, no-store, must-revalidate, private, max-age=0')
      s.send_header('Expires', '0')
      s.send_header('Pragma', 'no-cache')
      s.send_header('Vary', '*') # Safari insists on caching if this header is not present in addition to the above

      s.send_header("Access-Control-Expose-Headers", "Content-Length, Accept-Ranges")
      s.send_header("Content-type", "application/octet-stream")
      if support_byte_ranges:
        s.send_header("Accept-Ranges", "bytes")
      if extra:
        for key, value in extra:
          s.send_header(key, value)
      s.end_headers()

    def do_HEAD(s):
      s.sendheaders()

    def do_OPTIONS(s):
      s.sendheaders([("Access-Control-Allow-Headers", "Range")], 0)

    def do_GET(s):
      # CORS preflight makes OPTIONS requests which we need to account for.
      expectedConns = 22
      s.num_get_connections += 1
      assert(s.num_get_connections < expectedConns)

      if s.path == '/':
        s.sendheaders()
      elif not support_byte_ranges:
        s.sendheaders()
        s.wfile.write(data)
      else:
        start, end = s.headers.get("range").split("=")[1].split("-")
        start = int(start)
        end = int(end)
        end = min(len(data) - 1, end)
        length = end - start + 1
        s.sendheaders([], length)
        s.wfile.write(data[start:end + 1])

  return HTTPServer(('localhost', 11111), ChunkedServerHandler)


# This is similar to @core.no_wasmfs, but it disable WasmFS and runs the test
# normally. That is, in core we skip the test if we are in the wasmfs.* mode,
# while in browser we don't have such modes, so we force the test to run without
# WasmFS.
#
# When WasmFS is on by default, these annotations will still be needed. Only
# when we remove the old JS FS entirely would we remove them.
def no_wasmfs(note):
  assert not callable(note)

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      self.set_setting('WASMFS', 0)
      f(self, *args, **kwargs)
    return decorated
  return decorator


def shell_with_script(shell_file, output_file, replacement):
  shell = read_file(path_from_root('src', shell_file))
  create_file(output_file, shell.replace('{{{ SCRIPT }}}', replacement))


def is_swiftshader(_):
  return is_chrome() and '--use-gl=swiftshader' in get_browser()


no_swiftshader = skip_if_simple('not compatible with swiftshader', is_swiftshader)

no_chrome = skip_if('no_chrome', lambda _: is_chrome(), 'chrome is not supported')

no_firefox = skip_if('no_firefox', lambda _: is_firefox(), 'firefox is not supported')

no_safari = skip_if('no_safari', lambda _: is_safari(), 'safari is not supported')


def requires_version(name, version_getter):
  assert callable(version_getter)

  def decorator(min_required_version, note=''):
    return skip_if_simple(name, lambda _: version_getter() < min_required_version, f'{name} v{version_getter()} is not supported (need v{min_required_version} at minimum) {note}')

  return decorator


requires_safari_version = requires_version('safari', get_safari_version)


def is_jspi(args):
  return '-sASYNCIFY=2' in args


def also_with_threads(f):
  assert callable(f)

  @wraps(f)
  def decorated(self, threads, *args, **kwargs):
    if threads:
      self.cflags += ['-pthread']
    f(self, *args, **kwargs)

  parameterize(decorated, {'': (False,),
                           'pthreads': (True,)})

  return decorated


def skipIfFeatureNotAvailable(skip_env_var, feature, message):
  for env_var in skip_env_var if type(skip_env_var) == list else [skip_env_var]:
    should_skip = browser_should_skip_feature(env_var, feature)
    if should_skip:
      break

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      if should_skip == 'error':
        raise Exception(f'This test requires a browser that supports {feature.name} but your browser {get_browser()} does not support this. Run with {skip_env_var}=1 or EMTEST_AUTOSKIP=1 to skip this test automatically.')
      elif should_skip and bool(re.search(r"MIN_.*_VERSION", os.getenv('EMCC_CFLAGS', ''))):
        # should_skip=True, so we should skip running this test. However, user has specified a MIN_x_VERSION
        # directive in EMCC_CFLAGS, so we cannot even try to compile this test, or otherwise emcc can
        # error out on the MIN_x_VERSION being too old. So skip both compiling+running this test.
        self.skipTest(message)
      elif should_skip:
        # Skip running this test in a browser, but do test compiling it, to get partial coverage.
        self.skip_exec = message

      f(self, *args, **kwargs)

    return decorated

  return decorator


def webgl2_disabled():
  return browser_should_skip_feature('EMTEST_LACKS_WEBGL2', Feature.WEBGL2) or browser_should_skip_feature('EMTEST_LACKS_GRAPHICS_HARDWARE', Feature.WEBGL2)


requires_graphics_hardware = skipIfFeatureNotAvailable('EMTEST_LACKS_GRAPHICS_HARDWARE', None, 'This test requires graphics hardware')
requires_webgl2 = skipIfFeatureNotAvailable(['EMTEST_LACKS_WEBGL2', 'EMTEST_LACKS_GRAPHICS_HARDWARE'], Feature.WEBGL2, 'This test requires WebGL2 to be available')
requires_webgpu = skipIfFeatureNotAvailable(['EMTEST_LACKS_WEBGPU', 'EMTEST_LACKS_GRAPHICS_HARDWARE'], Feature.WEBGPU, 'This test requires WebGPU to be available')
requires_sound_hardware = skipIfFeatureNotAvailable('EMTEST_LACKS_SOUND_HARDWARE', None, 'This test requires sound hardware')
requires_microphone_access = skipIfFeatureNotAvailable('EMTEST_LACKS_MICROPHONE_ACCESS', None, 'This test accesses microphone, which may need accepting a user prompt to enable it.')
requires_offscreen_canvas = skipIfFeatureNotAvailable('EMTEST_LACKS_OFFSCREEN_CANVAS', Feature.OFFSCREENCANVAS_SUPPORT, 'This test requires a browser with OffscreenCanvas')
requires_es6_workers = skipIfFeatureNotAvailable('EMTEST_LACKS_ES6_WORKERS', Feature.WORKER_ES6_MODULES, 'This test requires a browser with ES6 Module Workers support')
requires_growable_arraybuffers = skipIfFeatureNotAvailable('EMTEST_LACKS_GROWABLE_ARRAYBUFFERS', Feature.GROWABLE_ARRAYBUFFERS, 'This test requires a browser that supports growable ArrayBuffers')
# N.b. not all SharedArrayBuffer requiring tests are annotated with this decorator, since at this point there are so many of such tests.
# As a middle ground, if a test has a name 'thread' or 'wasm_worker' in it, then it does not need decorating. To run all single-threaded tests in
# the suite, one can run "EMTEST_LACKS_SHARED_ARRAY_BUFFER=1 test/runner browser skip:browser.test_*thread* skip:browser.test_*wasm_worker* skip:browser.test_*audio_worklet*"
requires_shared_array_buffer = skipIfFeatureNotAvailable('EMTEST_LACKS_SHARED_ARRAY_BUFFER', Feature.THREADS, 'This test requires a browser with SharedArrayBuffer support')


class browser(BrowserCore):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    cls.browser_timeout = 60
    if get_browser() != 'node':
      print()
      print('Running the browser tests. Make sure the browser allows popups from localhost.')
      print()

  def require_jspi(self):
    if not is_chrome():
      self.skipTest(f'Current browser ({get_browser()}) does not support JSPI. Only chromium-based browsers ({CHROMIUM_BASED_BROWSERS}) support JSPI today.')
    super().require_jspi()

  def post_manual_reftest(self):
    assert os.path.exists('reftest.js')
    self.add_browser_reporting()
    html = read_file('test.html')
    html = html.replace('</body>', '''
<script src="browser_reporting.js"></script>
<script src="reftest.js"></script>
<script>
var windowClose = window.close;
window.close = () => {
  // wait for rafs to arrive and the screen to update before reftesting
  setTimeout(() => {
    doReftest();
    setTimeout(windowClose, 5000);
  }, 1000);
};
</script>
</body>''')
    create_file('test.html', html)

  def make_reftest(self, expected):
    # make sure the pngs used here have no color correction, using e.g.
    #   pngcrush -rem gAMA -rem cHRM -rem iCCP -rem sRGB infile outfile
    shutil.copy(expected, 'expected.png')
    create_file('reftest.js', f'''
      const reftestRebaseline = {common.EMTEST_REBASELINE};
    ''' + read_file(test_file('reftest.js')))

  def reftest(self, filename, reference, reference_slack=0, *args, **kwargs):
    """Special case of `btest` that uses reference image
    """
    reference = find_browser_test_file(reference)
    assert 'expected' not in kwargs
    expected = [str(i) for i in range(reference_slack + 1)]
    self.make_reftest(reference)
    kwargs.setdefault('cflags', [])
    kwargs['cflags'] += ['--pre-js', 'reftest.js', '-sGL_TESTING']

    try:
      return self.btest(filename, expected=expected, *args, **kwargs)
    finally:
      if common.EMTEST_REBASELINE and os.path.exists('actual.png'):
        print(f'overwriting expected image: {reference}')
        self.run_process('pngcrush -rem gAMA -rem cHRM -rem iCCP -rem sRGB actual.png'.split() + [reference])

  def test_sdl1_in_emscripten_nonstrict_mode(self):
    if 'EMCC_STRICT' in os.environ and int(os.environ['EMCC_STRICT']):
      self.skipTest('This test requires being run in non-strict mode (EMCC_STRICT env. variable unset)')
    # TODO: This test is verifying behavior that will be deprecated at some point in the future, remove this test once
    # system JS libraries are no longer automatically linked to anymore.
    self.reftest('hello_world_sdl.c', 'htmltest.png')

  def test_sdl1(self):
    self.reftest('hello_world_sdl.c', 'htmltest.png', cflags=['-lSDL', '-lGL'])
    self.reftest('hello_world_sdl.c', 'htmltest.png', cflags=['-sUSE_SDL', '-lGL']) # is the default anyhow

  def test_sdl1_es6(self):
    self.reftest('hello_world_sdl.c', 'htmltest.png', cflags=['-sUSE_SDL', '-lGL', '-sEXPORT_ES6'])

  @no_safari('TODO: Fails in NEW Safari 26.0.1 (21622.1.22.11.15), but not in OLD Safari 17.6 (17618.3.11.11.7, 17618) or Safari 18.5 (20621.2.5.11.8)')
  def test_emscripten_log(self):
    self.btest_exit('test_emscripten_log.cpp', cflags=['-Wno-deprecated-pragma', '-gsource-map', '-g2'])

  @also_with_wasmfs
  def test_preload_file(self):
    create_file('somefile.txt', 'load me right before running the code please')
    create_file('.somefile.txt', 'load me right before running the code please')
    create_file('some@file.txt', 'load me right before running the code please')

    absolute_src_path = os.path.abspath('somefile.txt')

    def make_main(path):
      print('make main at', path)
      path = path.replace('\\', '\\\\').replace('"', '\\"') # Escape tricky path name for use inside a C string.
      # TODO: change this when wasmfs supports relative paths.
      if self.get_setting('WASMFS'):
        path = "/" + path
      create_file('main.c', r'''
        #include <assert.h>
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

          assert(strcmp("load me right before", buf) == 0);
          return 0;
        }
        ''' % path)

    test_cases = [
      # (source preload-file string, file on target FS to load)
      ("somefile.txt", "somefile.txt"),
      (".somefile.txt@somefile.txt", "somefile.txt"),
      ("./somefile.txt", "somefile.txt"),
      ("somefile.txt@file.txt", "file.txt"),
      ("./somefile.txt@file.txt", "file.txt"),
      ("./somefile.txt@./file.txt", "file.txt"),
      ("somefile.txt@/file.txt", "file.txt"),
      ("somefile.txt@/", "somefile.txt"),
      (absolute_src_path + "@file.txt", "file.txt"),
      (absolute_src_path + "@/file.txt", "file.txt"),
      (absolute_src_path + "@/", "somefile.txt"),
      ("somefile.txt@/directory/file.txt", "/directory/file.txt"),
      ("somefile.txt@/directory/file.txt", "directory/file.txt"),
      (absolute_src_path + "@/directory/file.txt", "directory/file.txt"),
      ("some@@file.txt@other.txt", "other.txt"),
      ("some@@file.txt@some@@otherfile.txt", "some@otherfile.txt")]

    for srcpath, dstpath in test_cases:
      print('Testing', srcpath, dstpath)
      make_main(dstpath)
      self.btest_exit('main.c', cflags=['--preload-file', srcpath])
    if WINDOWS:
      # On Windows, the following non-alphanumeric non-control code ASCII characters are supported.
      # The characters <, >, ", |, ?, * are not allowed, because the Windows filesystem doesn't support those.
      tricky_filename = '!#$%&\'()+,-. ;=@[]^_`{}~.txt'
    else:
      # All 7-bit non-alphanumeric non-control code ASCII characters except /, : and \ are allowed.
      tricky_filename = '!#$%&\'()+,-. ;=@[]^_`{}~ "*<>?|.txt'
    create_file(tricky_filename, 'load me right before running the code please')
    make_main(tricky_filename)
    # As an Emscripten-specific feature, the character '@' must be escaped in the form '@@' to not confuse with the 'src@dst' notation.
    self.btest_exit('main.c', cflags=['--preload-file', tricky_filename.replace('@', '@@')])

    # TODO: WASMFS doesn't support the rest of this test yet. Exit early.
    if self.get_setting('WASMFS'):
      return

    # By absolute path

    make_main('somefile.txt') # absolute becomes relative
    self.btest_exit('main.c', cflags=['--preload-file', absolute_src_path])

    # Test subdirectory handling with asset packaging.
    delete_dir('assets')
    ensure_dir('assets/sub/asset1')
    ensure_dir('assets/sub/asset1/.git') # Test adding directory that shouldn't exist.
    ensure_dir('assets/sub/asset2')
    create_file('assets/sub/asset1/file1.txt', '''load me right before running the code please''')
    create_file('assets/sub/asset1/.git/shouldnt_be_embedded.txt', '''this file should not get embedded''')
    create_file('assets/sub/asset2/file2.txt', '''load me right before running the code please''')
    absolute_assets_src_path = 'assets'

    def make_main_two_files(path1, path2, nonexistingpath):
      create_file('main.c', r'''
        #include <stdio.h>
        #include <assert.h>
        #include <string.h>
        #include <emscripten.h>
        int main() {
          FILE *f = fopen("%s", "r");
          char buf[100];
          fread(buf, 1, 20, f);
          buf[20] = 0;
          fclose(f);
          printf("|%%s|\n", buf);

          assert(strcmp("load me right before", buf) == 0);

          f = fopen("%s", "r");
          assert(f != NULL);
          fclose(f);

          f = fopen("%s", "r");
          assert(f == NULL);

          return 0;
        }
      ''' % (path1, path2, nonexistingpath))

    test_cases = [
      # (source directory to embed, file1 on target FS to load, file2 on target FS to load, name of a file that *shouldn't* exist on VFS)
      ("assets", "assets/sub/asset1/file1.txt", "assets/sub/asset2/file2.txt", "assets/sub/asset1/.git/shouldnt_be_embedded.txt"),
      ("assets/", "assets/sub/asset1/file1.txt", "assets/sub/asset2/file2.txt", "assets/sub/asset1/.git/shouldnt_be_embedded.txt"),
      ("assets@/", "/sub/asset1/file1.txt", "/sub/asset2/file2.txt", "/sub/asset1/.git/shouldnt_be_embedded.txt"),
      ("assets/@/", "/sub/asset1/file1.txt", "/sub/asset2/file2.txt", "/sub/asset1/.git/shouldnt_be_embedded.txt"),
      ("assets@./", "/sub/asset1/file1.txt", "/sub/asset2/file2.txt", "/sub/asset1/.git/shouldnt_be_embedded.txt"),
      (absolute_assets_src_path + "@/", "/sub/asset1/file1.txt", "/sub/asset2/file2.txt", "/sub/asset1/.git/shouldnt_be_embedded.txt"),
      (absolute_assets_src_path + "@/assets", "/assets/sub/asset1/file1.txt", "/assets/sub/asset2/file2.txt", "assets/sub/asset1/.git/shouldnt_be_embedded.txt")]

    for test in test_cases:
      (srcpath, dstpath1, dstpath2, nonexistingpath) = test
      make_main_two_files(dstpath1, dstpath2, nonexistingpath)
      print(srcpath)
      self.btest_exit('main.c', cflags=['--preload-file', srcpath, '--exclude-file', '*/.*'])

    # Should still work with -o subdir/..

    make_main('somefile.txt') # absolute becomes relative
    ensure_dir('dirrey')
    self.compile_btest('main.c', ['--preload-file', absolute_src_path, '-o', 'dirrey/page.html'], reporting=Reporting.JS_ONLY)
    self.run_browser('dirrey/page.html', '/report_result?exit:0')

    # With FS.preloadFile

    create_file('pre.js', '''
      // we need --use-preload-plugins for this.
      Module.preRun = () => FS.createPreloadedFile('/', 'someotherfile.txt', 'somefile.txt', true, false);
    ''')
    make_main('someotherfile.txt')
    self.btest_exit('main.c', cflags=['--pre-js', 'pre.js', '--use-preload-plugins'])

  # Tests that user .html shell files can manually download .data files created with --preload-file cmdline.
  @parameterized({
    '': ([],),
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'],),
  })
  def test_preload_file_with_manual_data_download(self, args):
    create_file('file.txt', 'Hello!')

    self.compile_btest('browser/test_manual_download_data.c', ['-sEXIT_RUNTIME', '-o', 'out.js', '--preload-file', 'file.txt@/file.txt'] + args)
    shutil.copy(test_file('browser/test_manual_download_data.html'), '.')

    # Move .data file out of server root to ensure that getPreloadedPackage is actually used
    os.mkdir('test')
    shutil.move('out.js', 'test/test_manual_download_data.js')
    shutil.move('out.data', 'test/test_manual_download_data.data')

    self.run_browser('test_manual_download_data.html', '/report_result?exit:0')

  # Tests that if the output files have single or double quotes in them, that it will be handled by
  # correctly escaping the names.
  def test_output_file_escaping(self):
    self.set_setting('EXIT_RUNTIME')
    tricky_part = '\'' if WINDOWS else '\' and \"' # On Windows, files/directories may not contain a double quote character. On non-Windowses they can, so test that.

    d = 'dir with ' + tricky_part
    abs_d = os.path.abspath(d)
    ensure_dir(abs_d)
    txt = 'file with ' + tricky_part + '.txt'
    create_file(os.path.join(d, txt), 'load me right before')

    src = os.path.join(d, 'file with ' + tricky_part + '.c')
    create_file(src, r'''
      #include <assert.h>
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
        assert(strcmp("load me right before", buf) == 0);
        return 0;
      }
    ''' % (txt.replace('\'', '\\\'').replace('\"', '\\"')))

    data_file = os.path.join(abs_d, 'file with ' + tricky_part + '.data')
    data_js_file = os.path.join(abs_d, 'file with ' + tricky_part + '.js')
    abs_txt = os.path.join(abs_d, txt)
    self.run_process([FILE_PACKAGER, data_file, '--use-preload-cache', '--indexedDB-name=testdb', '--preload', abs_txt + '@' + txt, '--js-output=' + data_js_file])
    page_file = os.path.join(d, 'file with ' + tricky_part + '.html')
    abs_page_file = os.path.abspath(page_file)
    self.compile_btest(src, ['--pre-js', data_js_file, '-o', abs_page_file, '-sFORCE_FILESYSTEM'], reporting=Reporting.JS_ONLY)
    self.run_browser(page_file, '/report_result?exit:0')

  # Clear all IndexedDB databases. This gives us a fresh state for tests that
  # chech caching.
  def clear_indexed_db(self):
    self.add_browser_reporting()
    create_file('clear_indexed_db.html', '''
      <script src="browser_reporting.js"></script>
      <script>
        // Clear the cache, so that the next test starts from a clean slate.
        if (indexedDB.databases) {
          // If the tested browser supports IndexedDB 3.0 API, then enumerate all
          // available databases and delete them.
          indexedDB.databases().then(dbs => {
            Promise.all(dbs.map(db => {
              return indexedDB.deleteDatabase(db.name);
            })).then(() => {
              reportResultToServer("clear");
            });
          });
        } else {
          // Testing an old browser that does not support indexedDB.databases():
          // Delete the fixed database EM_PRELOAD_CACHE (this is hardcoded in
          // file packager)
          indexedDB.deleteDatabase('EM_PRELOAD_CACHE').onsuccess = () => {
            reportResultToServer("clear");
          };
        }
      </script>
    ''')
    self.run_browser('clear_indexed_db.html', '/report_result?clear')

  @parameterized({
    '0': (0,),
    '1mb': (1 * 1024 * 1024,),
    '100mb': (100 * 1024 * 1024,),
    '150mb': (150 * 1024 * 1024,),
  })
  def test_preload_caching(self, extra_size):
    self.clear_indexed_db()
    self.set_setting('EXIT_RUNTIME')
    create_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>
      #include <emscripten.h>

      extern int checkPreloadResults();

      int main(int argc, char** argv) {
        FILE *f = fopen("%s", "r");
        char buf[100];
        fread(buf, 1, 20, f);
        buf[20] = 0;
        fclose(f);
        printf("|%%s|\n", buf);

        assert(strcmp("load me right before", buf) == 0);
        return checkPreloadResults();
      }
    ''' % 'somefile.txt')

    create_file('test.js', '''
      addToLibrary({
        checkPreloadResults: function() {
          var cached = 0;
          var packages = Object.keys(Module['preloadResults']);
          packages.forEach(function(package) {
            var fromCache = Module['preloadResults'][package]['fromCache'];
            if (fromCache) {
              cached++;
            }
          });
          return cached;
        }
      });
    ''')

    # test caching of various sizes, including sizes higher than 128MB which is
    # chrome's limit on IndexedDB item sizes, see
    # https://cs.chromium.org/chromium/src/content/renderer/indexed_db/webidbdatabase_impl.cc?type=cs&q=%22The+serialized+value+is+too+large%22&sq=package:chromium&g=0&l=177
    # https://cs.chromium.org/chromium/src/out/Debug/gen/third_party/blink/public/mojom/indexeddb/indexeddb.mojom.h?type=cs&sq=package:chromium&g=0&l=60
    if is_chrome() and extra_size >= 100 * 1024 * 1024:
      self.skipTest('chrome bug')
    create_file('somefile.txt', '''load me right before running the code please''' + ('_' * extra_size))
    print('size:', os.path.getsize('somefile.txt'))
    args = ['--use-preload-cache', '--js-library', 'test.js', '--preload-file', 'somefile.txt', '-o', 'page.html', '-sALLOW_MEMORY_GROWTH']
    self.compile_btest('main.c', args, reporting=Reporting.JS_ONLY)
    self.run_browser('page.html', '/report_result?exit:0')
    self.run_browser('page.html', '/report_result?exit:1')

    # test with ENVIRONMENT=web, to check for problems with node.js support
    # (see #23059)
    self.clear_indexed_db()
    self.compile_btest('main.c', args + ['-sENVIRONMENT=web'], reporting=Reporting.JS_ONLY)
    self.run_browser('page.html', '/report_result?exit:0')

  def test_preload_caching_indexeddb_name(self):
    self.set_setting('EXIT_RUNTIME')
    create_file('somefile.txt', 'load me right before running the code please')

    def make_main(path):
      print(path)
      create_file('main.c', r'''
        #include <assert.h>
        #include <stdio.h>
        #include <string.h>
        #include <emscripten.h>

        extern int checkPreloadResults();

        int main(int argc, char** argv) {
          FILE *f = fopen("%s", "r");
          char buf[100];
          fread(buf, 1, 20, f);
          buf[20] = 0;
          fclose(f);
          printf("|%%s|\n", buf);

          int result = 0;

          assert(strcmp("load me right before", buf) == 0);
          int num_cached = checkPreloadResults();
          printf("got %%d preloadResults from cache\n", num_cached);
          return num_cached;
        }
      ''' % path)

    create_file('test.js', '''
      addToLibrary({
        checkPreloadResults: () => {
          var cached = 0;
          for (var result of Object.values(Module['preloadResults'])) {
            if (result['fromCache']) {
              cached++;
            }
          }
          return cached;
        }
      });
    ''')

    make_main('somefile.txt')
    self.run_process([FILE_PACKAGER, 'somefile.data', '--use-preload-cache', '--indexedDB-name=testdb', '--preload', 'somefile.txt', '--js-output=' + 'somefile.js'])
    self.compile_btest('main.c', ['--js-library', 'test.js', '--pre-js', 'somefile.js', '-o', 'page.html', '-sFORCE_FILESYSTEM'], reporting=Reporting.JS_ONLY)
    self.run_browser('page.html', '/report_result?exit:0')
    print("Re-running ..")
    self.run_browser('page.html', '/report_result?exit:1')

  def test_multifile(self):
    # a few files inside a directory
    ensure_dir('subdirr/moar')
    create_file('subdirr/data1.txt', '1214141516171819')
    create_file('subdirr/moar/data2.txt', '3.14159265358979')
    create_file('main.c', r'''
      #include <assert.h>
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
        assert(strcmp("1214141516171819", buf) == 0);

        FILE *f2 = fopen("subdirr/moar/data2.txt", "r");
        fread(buf, 1, 16, f2);
        buf[16] = 0;
        fclose(f2);
        printf("|%s|\n", buf);
        assert(strcmp("3.14159265358979", buf) == 0);

        return 0;
      }
    ''')

    # by individual files
    self.btest_exit('main.c', cflags=['--preload-file', 'subdirr/data1.txt', '--preload-file', 'subdirr/moar/data2.txt'])

    # by directory, and remove files to make sure
    self.set_setting('EXIT_RUNTIME')
    self.compile_btest('main.c', ['--preload-file', 'subdirr', '-o', 'page.html'], reporting=Reporting.JS_ONLY)
    shutil.rmtree('subdirr')
    self.run_browser('page.html', '/report_result?exit:0')

  def test_custom_file_package_url(self):
    # a few files inside a directory
    ensure_dir('subdirr')
    ensure_dir('cdn')
    create_file(Path('subdirr/data1.txt'), '1214141516171819')
    # change the file package base dir to look in a "cdn". note that normally
    # you would add this in your own custom html file etc., and not by
    # modifying the existing shell in this manner
    default_shell = read_file(path_from_root('src/shell.html'))
    create_file('shell.html', default_shell.replace('var Module = {', '''
    var Module = {
      locateFile: function(path, prefix) {
        if (path.endsWith(".wasm")) {
           return prefix + path;
        } else {
           return "cdn/" + path;
        }
      },
    '''))
    create_file('main.c', r'''
      #include <assert.h>
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
        assert(strcmp("1214141516171819", buf) == 0);

        return 0;
      }
    ''')

    self.set_setting('EXIT_RUNTIME')
    self.compile_btest('main.c', ['--shell-file', 'shell.html', '--preload-file', 'subdirr/data1.txt', '-o', 'test.html'], reporting=Reporting.JS_ONLY)
    shutil.move('test.data', Path('cdn/test.data'))
    self.run_browser('test.html', '/report_result?exit:0')

  def test_missing_data_throws_error(self):
    create_file('data.txt', 'data')
    create_file('main.c', r'''
      #include <stdio.h>
      #include <string.h>
      #include <emscripten.h>
      int main() {
        // This code should never be executed in terms of missing required dependency file.
        return 0;
      }
    ''')

    def setup(assetLocalization):
      create_file('on_window_error_shell.html', r'''
      <html>
        <body>
          <center><canvas id='canvas' width='256' height='256'></canvas></center>
          <hr><div id='output'></div><hr>
          <script type='text/javascript'>
            const errorHandler = async (event) => {
              if (globalThis.disableErrorReporting) return;
              event.stopImmediatePropagation();
              const error = String(event instanceof ErrorEvent ? event.message : (event.reason || event));
              globalThis.disableErrorReporting = true;
              window.onerror = null;
              var result = error.includes("test.data") ? 1 : 0;
              await fetch('/report_result?' + result);
              window.close();
            }
            window.addEventListener('error', errorHandler);
            window.addEventListener('unhandledrejection', errorHandler);
            const outputElem = document.getElementById('output');
            var Module = {
              locateFile: (path, prefix) => {
                if (path.endsWith('.wasm')) {
                  return prefix + path;
                } else {
                  return "''' + assetLocalization + r'''" + path;
                }
              },
              print: () => {
                outputElem.innerHTML += text.replace('\n', '<br>', 'g') + '<br>';
              },
              canvas: document.getElementById('canvas')
            };
          </script>
          {{{ SCRIPT }}}
        </body>
      </html>''')

    # test test missing file should run xhr.onload with status different than 200, 304 or 206
    setup("")
    self.compile_btest('main.c', ['--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html'])
    shutil.move('test.data', 'missing.data')
    self.run_browser('test.html', '/report_result?1')

    # test unknown protocol should go through xhr.onerror
    setup("unknown_protocol://")
    self.compile_btest('main.c', ['--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html'])
    self.run_browser('test.html', '/report_result?1')

    # test wrong protocol and port
    setup("https://localhost:8800/")
    self.compile_btest('main.c', ['--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html'])
    self.run_browser('test.html', '/report_result?1')

    # TODO: CORS, test using a full url for locateFile
    # create_file('shell.html', read_file(path_from_root('src/shell.html')).replace('var Module = {', 'var Module = { locateFile: function (path) {return "http:/localhost:8888/cdn/" + path;}, '))
    # test()

  @also_with_wasmfs
  def test_fs_dev_random(self):
    self.btest_exit('fs/test_fs_dev_random.c')

  def test_sdl_swsurface(self):
    self.btest_exit('test_sdl_swsurface.c', cflags=['-lSDL', '-lGL'])

  def test_sdl_surface_lock_opts(self):
    # Test Emscripten-specific extensions to optimize SDL_LockSurface and SDL_UnlockSurface.
    self.reftest('hello_world_sdl.c', 'htmltest.png', cflags=['-DTEST_SDL_LOCK_OPTS', '-lSDL', '-lGL'])

  @also_with_wasmfs
  def test_sdl_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file
    shutil.copy(test_file('screenshot.jpg'), '.')
    src = test_file('browser/test_sdl_image.c')
    for dest, dirname, basename in [('screenshot.jpg', '/', 'screenshot.jpg'),
                                    ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
      self.btest_exit(src, cflags=[
        '-O2', '-lSDL', '-lGL',
        '--preload-file', dest, '-DSCREENSHOT_DIRNAME="' + dirname + '"', '-DSCREENSHOT_BASENAME="' + basename + '"', '--use-preload-plugins',
      ])

  @also_with_wasmfs
  def test_sdl_image_jpeg(self):
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.jpeg')
    self.btest_exit('test_sdl_image.c', cflags=[
      '--preload-file', 'screenshot.jpeg',
      '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"', '--use-preload-plugins',
      '-lSDL', '-lGL',
    ])

  def test_sdl_image_webp(self):
    shutil.copy(test_file('screenshot.webp'), '.')
    self.btest_exit('test_sdl_image.c', cflags=[
      '--preload-file', 'screenshot.webp',
      '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.webp"', '--use-preload-plugins',
      '-lSDL', '-lGL',
    ])

  @also_with_wasmfs
  def test_sdl_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.reftest('test_sdl_image_prepare.c', 'screenshot.jpg', cflags=['--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  @parameterized({
    '': ([],),
    # add testing for closure on preloaded files + ENVIRONMENT=web (we must not
    # emit any node.js code here, see
    # https://github.com/emscripten-core/emscripten/issues/14486
    'closure_webonly': (['--closure', '1', '-sENVIRONMENT=web'],),
  })
  def test_sdl_image_prepare_data(self, args):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.reftest('test_sdl_image_prepare_data.c', 'screenshot.jpg', cflags=['--preload-file', 'screenshot.not', '-lSDL', '-lGL'] + args)

  def test_sdl_image_must_prepare(self):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.jpg')
    self.reftest('test_sdl_image_must_prepare.c', 'screenshot.jpg', cflags=['--preload-file', 'screenshot.jpg', '-lSDL', '-lGL'])

  def test_sdl_stb_image(self):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.reftest('test_sdl_stb_image.c', 'screenshot.jpg', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  def test_sdl_stb_image_bpp(self):
    # load grayscale image without alpha
    shutil.copy(test_file('browser/test_sdl-stb-bpp1.png'), 'screenshot.not')
    self.reftest('test_sdl_stb_image.c', 'test_sdl-stb-bpp1.png', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

    # load grayscale image with alpha
    self.clear()
    shutil.copy(test_file('browser/test_sdl-stb-bpp2.png'), 'screenshot.not')
    self.reftest('test_sdl_stb_image.c', 'test_sdl-stb-bpp2.png', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

    # load RGB image
    self.clear()
    shutil.copy(test_file('browser/test_sdl-stb-bpp3.png'), 'screenshot.not')
    self.reftest('test_sdl_stb_image.c', 'test_sdl-stb-bpp3.png', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

    # load RGBA image
    self.clear()
    shutil.copy(test_file('browser/test_sdl-stb-bpp4.png'), 'screenshot.not')
    self.reftest('test_sdl_stb_image.c', 'test_sdl-stb-bpp4.png', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  def test_sdl_stb_image_data(self):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.reftest('test_sdl_stb_image_data.c', 'screenshot.jpg', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  def test_sdl_stb_image_cleanup(self):
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.btest_exit('test_sdl_stb_image_cleanup.c', cflags=['-sSTB_IMAGE', '--preload-file', 'screenshot.not', '-lSDL', '-lGL', '--memoryprofiler'])

  @parameterized({
    '': ([],),
    'safe_heap': (['-sSAFE_HEAP'],),
    'safe_heap_O2': (['-sSAFE_HEAP', '-O2'],),
  })
  def test_sdl_canvas(self, args):
    self.btest_exit('test_sdl_canvas.c', cflags=['-sLEGACY_GL_EMULATION', '-lSDL', '-lGL'] + args)

  def test_sdl_canvas_alpha(self):
    # N.B. On Linux with Intel integrated graphics cards, this test needs Firefox 49 or newer.
    # See https://github.com/emscripten-core/emscripten/issues/4069.
    create_file('flag_0.js', "Module['arguments'] = ['-0'];")

    self.reftest('test_sdl_canvas_alpha.c', 'test_sdl_canvas_alpha.png', cflags=['-lSDL', '-lGL'], reference_slack=12)
    self.reftest('test_sdl_canvas_alpha.c', 'test_sdl_canvas_alpha_flag_0.png', cflags=['--pre-js', 'flag_0.js', '-lSDL', '-lGL'], reference_slack=12)

  @parameterized({
    '': ([],),
    'eventhandler': (['-DTEST_EMSCRIPTEN_SDL_SETEVENTHANDLER'],),
  })
  @parameterized({
    '': ([],),
    'asyncify': (['-DTEST_SLEEP', '-sASSERTIONS', '-sSAFE_HEAP', '-sASYNCIFY'],),
  })
  @parameterized({
    '': (False,),
    'delay': (True,),
  })
  def test_sdl_key(self, defines, async_, delay):
    if delay:
      settimeout_start = 'setTimeout(function() {'
      settimeout_end = '}, 1);'
    else:
      settimeout_start = ''
      settimeout_end = ''
    create_file('pre.js', '''
      function keydown(c) {
       %s
        simulateKeyDown(c);
       %s
      }

      function keyup(c) {
       %s
        simulateKeyUp(c);
       %s
      }
    ''' % (settimeout_start, settimeout_end, settimeout_start, settimeout_end))
    self.btest_exit('test_sdl_key.c', 223092870, cflags=defines + async_ + ['--pre-js', test_file('browser/fake_events.js'), '--pre-js=pre.js', '-lSDL', '-lGL'])

  def test_canvas_focus(self):
    self.btest_exit('test_canvas_focus.c', cflags=['--pre-js', test_file('browser/fake_events.js')])

  def test_sdl_text(self):
    create_file('pre.js', '''
      Module.postRun = () => {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }
    ''')

    self.btest_exit('test_sdl_text.c', cflags=['--pre-js', 'pre.js', '--pre-js', test_file('browser/fake_events.js'), '-lSDL', '-lGL'])

  def test_sdl_mouse(self):
    self.btest_exit('test_sdl_mouse.c', cflags=['-O2', '--minify=0', '--pre-js', test_file('browser/fake_events.js'), '-lSDL', '-lGL'])

  def test_sdl_mouse_offsets(self):
    create_file('page.html', '''
      <html>
        <head>
          <style type="text/css">
            html, body { margin: 0; padding: 0; }
            #container {
              position: absolute;
              left: 5px; right: 0;
              top: 5px; bottom: 0;
            }
            #canvas {
              position: absolute;
              left: 0; width: 600px;
              top: 0; height: 450px;
            }
            textarea {
              margin-top: 500px;
              margin-left: 5px;
              width: 600px;
            }
          </style>
        </head>
        <body>
          <div id="container">
            <canvas id="canvas"></canvas>
          </div>
          <textarea id="output" rows="8"></textarea>
          <script type="text/javascript">
            var Module = {
              canvas: document.getElementById('canvas'),
              print: (function() {
                var element = document.getElementById('output');
                element.value = ''; // clear browser cache
                return function(text) {
                  if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
                  element.value += text + "\\n";
                  element.scrollTop = element.scrollHeight; // focus on bottom
                };
              })()
            };
          </script>
          <script type="text/javascript" src="sdl_mouse.js"></script>
        </body>
      </html>
    ''')

    self.compile_btest('browser/test_sdl_mouse.c', ['-DTEST_SDL_MOUSE_OFFSETS', '-O2', '--minify=0', '-o', 'sdl_mouse.js', '--pre-js', test_file('browser/fake_events.js'), '-lSDL', '-lGL', '-sEXIT_RUNTIME'])
    self.run_browser('page.html', '', '/report_result?exit:0')

  def test_glut_touchevents(self):
    self.btest_exit('glut_touchevents.c', cflags=['-lglut'])

  def test_glut_wheelevents(self):
    self.btest_exit('glut_wheelevents.c', cflags=['-lglut'])

  @requires_graphics_hardware
  def test_glut_glutget_no_antialias(self):
    self.btest_exit('glut_glutget.c', cflags=['-lglut', '-lGL'])
    self.btest_exit('glut_glutget.c', cflags=['-lglut', '-lGL', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])

  # This test supersedes the one above, but it's skipped in the CI because anti-aliasing is not well supported by the Mesa software renderer.
  @requires_graphics_hardware
  def test_glut_glutget(self):
    self.btest_exit('glut_glutget.c', cflags=['-lglut', '-lGL'])
    self.btest_exit('glut_glutget.c', cflags=['-lglut', '-lGL', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])

  def test_glut_resize(self):
    self.btest_exit('test_glut_resize.c')

  def test_sdl_joystick_1(self):
    # Generates events corresponding to the Working Draft of the HTML5 Gamepad API.
    # http://www.w3.org/TR/2012/WD-gamepad-20120529/#gamepad-interface
    create_file('pre.js', '''
      var gamepads = [];
      // Spoof this function.
      navigator['getGamepads'] = () => gamepads;
      window['addNewGamepad'] = (id, numAxes, numButtons) => {
        var index = gamepads.length;
        gamepads.push({
          axes: new Array(numAxes),
          buttons: new Array(numButtons),
          id: id,
          index: index
        });
        var i;
        for (i = 0; i < numAxes; i++) gamepads[index].axes[i] = 0;
        for (i = 0; i < numButtons; i++) gamepads[index].buttons[i] = 0;
      };
      window['simulateGamepadButtonDown'] = (index, button) => {
        gamepads[index].buttons[button] = 1;
      };
      window['simulateGamepadButtonUp'] = (index, button) => {
        gamepads[index].buttons[button] = 0;
      };
      window['simulateAxisMotion'] = (index, axis, value) => {
        gamepads[index].axes[axis] = value;
      };
    ''')

    self.btest_exit('test_sdl_joystick.c', cflags=['-O2', '--minify=0', '-o', 'page.html', '--pre-js', 'pre.js', '-lSDL', '-lGL'])

  def test_sdl_joystick_2(self):
    # Generates events corresponding to the Editor's Draft of the HTML5 Gamepad API.
    # https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-Gamepad
    create_file('pre.js', '''
      var gamepads = [];
      // Spoof this function.
      navigator['getGamepads'] = () => gamepads;
      window['addNewGamepad'] = (id, numAxes, numButtons) => {
        var index = gamepads.length;
        gamepads.push({
          axes: new Array(numAxes),
          buttons: new Array(numButtons),
          id: id,
          index: index
        });
        var i;
        for (i = 0; i < numAxes; i++) gamepads[index].axes[i] = 0;
        // Buttons are objects
        for (i = 0; i < numButtons; i++) gamepads[index].buttons[i] = { pressed: false, value: 0 };
      };
      // FF mutates the original objects.
      window['simulateGamepadButtonDown'] = (index, button) => {
        gamepads[index].buttons[button].pressed = true;
        gamepads[index].buttons[button].value = 1;
      };
      window['simulateGamepadButtonUp'] = (index, button) => {
        gamepads[index].buttons[button].pressed = false;
        gamepads[index].buttons[button].value = 0;
      };
      window['simulateAxisMotion'] = (index, axis, value) => {
        gamepads[index].axes[axis] = value;
      };
    ''')

    self.btest_exit('test_sdl_joystick.c', cflags=['-O2', '--minify=0', '--pre-js', 'pre.js', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_glfw_joystick(self):
    # Generates events corresponding to the Editor's Draft of the HTML5 Gamepad API.
    # https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-Gamepad
    create_file('pre.js', '''
      var gamepads = [];
      // Spoof this function.
      navigator['getGamepads'] = () => gamepads;
      window['addNewGamepad'] = (id, numAxes, numButtons) => {
        var index = gamepads.length;
        var gamepad = {
          axes: new Array(numAxes),
          buttons: new Array(numButtons),
          id: id,
          index: index
        };
        gamepads.push(gamepad)
        var i;
        for (i = 0; i < numAxes; i++) gamepads[index].axes[i] = 0;
        // Buttons are objects
        for (i = 0; i < numButtons; i++) gamepads[index].buttons[i] = { pressed: false, value: 0 };

        // Dispatch event (required for glfw joystick; note not used in SDL test)
        var event = new Event('gamepadconnected');
        event.gamepad = gamepad;
        window.dispatchEvent(event);
      };
      // FF mutates the original objects.
      window['simulateGamepadButtonDown'] = (index, button) => {
        gamepads[index].buttons[button].pressed = true;
        gamepads[index].buttons[button].value = 1;
      };
      window['simulateGamepadButtonUp'] = (index, button) => {
        gamepads[index].buttons[button].pressed = false;
        gamepads[index].buttons[button].value = 0;
      };
      window['simulateAxisMotion'] = (index, axis, value) => {
        gamepads[index].axes[axis] = value;
      };
    ''')

    self.btest_exit('test_glfw_joystick.c', cflags=['-O2', '--minify=0', '-o', 'page.html', '--pre-js', 'pre.js', '-lGL', '-lglfw3', '-sUSE_GLFW=3'])

  @requires_graphics_hardware
  def test_webgl_context_attributes(self):
    # Javascript code to check the attributes support we want to test in the WebGL implementation
    # (request the attribute, create a context and check its value afterwards in the context attributes).
    # Tests will succeed when an attribute is not supported.
    create_file('check_webgl_attributes_support.js', '''
      addToLibrary({
        webglAntialiasSupported: function() {
          canvas = document.createElement('canvas');
          context = canvas.getContext('experimental-webgl', {antialias: true});
          attributes = context.getContextAttributes();
          return attributes.antialias;
        },
        webglDepthSupported: function() {
          canvas = document.createElement('canvas');
          context = canvas.getContext('experimental-webgl', {depth: true});
          attributes = context.getContextAttributes();
          return attributes.depth;
        },
        webglStencilSupported: function() {
          canvas = document.createElement('canvas');
          context = canvas.getContext('experimental-webgl', {stencil: true});
          attributes = context.getContextAttributes();
          return attributes.stencil;
        },
        webglAlphaSupported: function() {
          canvas = document.createElement('canvas');
          context = canvas.getContext('experimental-webgl', {alpha: true});
          attributes = context.getContextAttributes();
          return attributes.alpha;
        }
      });
    ''')

    # Copy common code file to temporary directory
    filepath = test_file('browser/test_webgl_context_attributes_common.c')
    temp_filepath = os.path.basename(filepath)
    shutil.copy(filepath, temp_filepath)

    # testAntiAliasing uses a window-sized buffer on the stack
    self.set_setting('STACK_SIZE', '1MB')

    # perform tests with attributes activated
    self.btest_exit('test_webgl_context_attributes_glut.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-lglut', '-lGLEW'])
    self.btest_exit('test_webgl_context_attributes_sdl.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-lSDL', '-lGLEW'])
    if not self.is_wasm64():
      self.btest_exit('test_webgl_context_attributes_sdl2.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-sUSE_SDL=2', '-lGLEW'])
    self.btest_exit('test_webgl_context_attributes_glfw.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-lglfw', '-lGLEW'])

    # perform tests with attributes desactivated
    self.btest_exit('test_webgl_context_attributes_glut.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-lGL', '-lglut', '-lGLEW'])
    self.btest_exit('test_webgl_context_attributes_sdl.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-lGL', '-lSDL', '-lGLEW'])
    self.btest_exit('test_webgl_context_attributes_glfw.c', cflags=['--js-library', 'check_webgl_attributes_support.js', '-lGL', '-lglfw', '-lGLEW'])

  @requires_graphics_hardware
  def test_webgl_no_double_error(self):
    self.btest_exit('webgl_error.c')

  @requires_graphics_hardware
  def test_webgl_parallel_shader_compile(self):
    self.btest_exit('webgl_parallel_shader_compile.cpp')

  @requires_webgl2
  def test_webgl_explicit_uniform_location(self):
    self.btest_exit('webgl_explicit_uniform_location.c', cflags=['-sGL_EXPLICIT_UNIFORM_LOCATION', '-sMIN_WEBGL_VERSION=2'])

  @requires_graphics_hardware
  def test_webgl_sampler_layout_binding(self):
    self.btest_exit('webgl_sampler_layout_binding.c', cflags=['-sGL_EXPLICIT_UNIFORM_BINDING'])

  @requires_webgl2
  def test_webgl2_ubo_layout_binding(self):
    self.btest_exit('webgl2_ubo_layout_binding.c', cflags=['-sGL_EXPLICIT_UNIFORM_BINDING', '-sMIN_WEBGL_VERSION=2'])

  # Test that -sGL_PREINITIALIZED_CONTEXT works and allows user to set Module['preinitializedWebGLContext'] to a preinitialized WebGL context.
  @requires_graphics_hardware
  def test_preinitialized_webgl_context(self):
    self.btest_exit('test_preinitialized_webgl_context.c', cflags=['-sGL_PREINITIALIZED_CONTEXT', '--shell-file', test_file('test_preinitialized_webgl_context.html')])

  @parameterized({
    '': ([],),
    'threads': (['-pthread'],),
    'closure': (['-sENVIRONMENT=web', '-O2', '--closure=1'],),
  })
  def test_emscripten_get_now(self, args):
    self.btest_exit('test_emscripten_get_now.c', cflags=args)

  def test_write_file_in_environment_web(self):
    self.btest_exit('write_file.c', cflags=['-sENVIRONMENT=web', '-Os', '--closure=1'])

  def test_fflush(self):
    self.btest('test_fflush.cpp', '0', cflags=['-sEXIT_RUNTIME', '--shell-file', test_file('test_fflush.html')], reporting=Reporting.NONE)

  @parameterized({
    '': ([],),
    'extra': (['-DEXTRA_WORK'],),
    'autopersist': (['-DIDBFS_AUTO_PERSIST'],),
  })
  def test_fs_idbfs_sync(self, args):
    secret = str(time.time())
    self.btest_exit('fs/test_idbfs_sync.c', cflags=['-lidbfs.js', f'-DSECRET="{secret}"', '-lidbfs.js'] + args + ['-DFIRST'])
    print('done first half')
    self.btest_exit('fs/test_idbfs_sync.c', cflags=['-lidbfs.js', f'-DSECRET="{secret}"', '-lidbfs.js'] + args)

  @parameterized({
    'open': ('TEST_CASE_OPEN', 2),
    'close': ('TEST_CASE_CLOSE', 3),
    'symlink': ('TEST_CASE_SYMLINK', 3),
    'unlink': ('TEST_CASE_UNLINK', 3),
    'rename': ('TEST_CASE_RENAME', 3),
    'mkdir': ('TEST_CASE_MKDIR', 2),
  })
  def test_fs_idbfs_autopersist(self, test_case, phase_count):
    self.cflags += ['-lidbfs.js', f'-DTEST_CASE={test_case}']
    for phase in range(phase_count):
      self.btest_exit('fs/test_idbfs_autopersist.c', cflags=[f'-DTEST_PHASE={phase + 1}'])

  def test_fs_idbfs_fsync(self):
    # sync from persisted state into memory before main()
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', '$ccall,$addRunDependency')
    create_file('pre.js', '''
      Module.preRun = () => {
        addRunDependency('syncfs');

        FS.mkdir('/working1');
        FS.mount(IDBFS, {}, '/working1');
        FS.syncfs(true, function (err) {
          if (err) throw err;
          removeRunDependency('syncfs');
        });
      };
    ''')

    args = ['--pre-js', 'pre.js', '-lidbfs.js', '-sEXIT_RUNTIME', '-sASYNCIFY']
    secret = str(time.time())
    self.btest('fs/test_idbfs_fsync.c', '1', cflags=args + ['-DFIRST', f'-DSECRET="{secret}"', '-lidbfs.js'])
    self.btest('fs/test_idbfs_fsync.c', '1', cflags=args + [f'-DSECRET="{secret}"', '-lidbfs.js'])

  def test_fs_memfs_fsync(self):
    args = ['-sASYNCIFY', '-sEXIT_RUNTIME']
    secret = str(time.time())
    self.btest_exit('fs/test_memfs_fsync.c', cflags=args + [f'-DSECRET="{secret}"'])

  def test_fs_workerfs(self):
    create_file('pre.js', '''
      Module.preRun = () => {
        var blob = new Blob(['hello blob']);
        var file = new File(['hello file'], 'file.txt');
        FS.mkdir('/work');
        FS.mount(WORKERFS, {
          blobs: [{ name: 'blob.txt', data: blob }],
          files: [file],
        }, '/work');
      };
    ''')
    self.btest_exit('fs/test_workerfs.c', cflags=['-lworkerfs.js', '--pre-js', 'pre.js'], run_in_worker=True)

  def test_fs_workerfs_package(self):
    create_file('file1.txt', 'first')
    ensure_dir('sub')
    create_file('sub/file2.txt', 'second')
    self.run_process([FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'sub/file2.txt', '--separate-metadata', '--js-output=files.js'])
    self.btest('fs/test_workerfs_package.c', '0', cflags=['-lworkerfs.js'], run_in_worker=True)

  def test_fs_lz4fs_package(self):
    # generate data
    ensure_dir('subdir')
    create_file('file1.txt', '0123456789' * (1024 * 128))
    create_file('subdir/file2.txt', '1234567890' * (1024 * 128))
    random_data = bytearray(random.randint(0, 255) for x in range(1024 * 128 * 10 + 1))
    random_data[17] = ord('X')
    create_file('file3.txt', random_data, binary=True)

    # compress in emcc, -sLZ4 tells it to tell the file packager
    print('emcc-normal')
    self.btest_exit('fs/test_lz4fs.c', 0, cflags=['-sLZ4', '--preload-file', 'file1.txt', '--preload-file', 'subdir/file2.txt', '--preload-file', 'file3.txt'])
    assert os.path.getsize('file1.txt') + os.path.getsize('subdir/file2.txt') + os.path.getsize('file3.txt') == 3 * 1024 * 128 * 10 + 1
    assert os.path.getsize('test.data') < (3 * 1024 * 128 * 10) / 2  # over half is gone
    print('    emcc-opts')
    self.btest_exit('fs/test_lz4fs.c', 0, cflags=['-sLZ4', '--preload-file', 'file1.txt', '--preload-file', 'subdir/file2.txt', '--preload-file', 'file3.txt', '-O2'])

    # compress in the file packager, on the server. the client receives compressed data and can just
    # use it. this is typical usage
    print('normal')
    out = subprocess.check_output([FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'subdir/file2.txt', 'file3.txt', '--lz4'])
    create_file('files.js', out, binary=True)
    self.btest_exit('fs/test_lz4fs.c', 0, cflags=['--pre-js', 'files.js', '-sLZ4', '-sFORCE_FILESYSTEM'])
    print('    opts')
    self.btest_exit('fs/test_lz4fs.c', 0, cflags=['--pre-js', 'files.js', '-sLZ4', '-sFORCE_FILESYSTEM', '-O2'])
    print('    modularize')
    self.compile_btest('fs/test_lz4fs.c', ['--pre-js', 'files.js', '-sLZ4', '-sFORCE_FILESYSTEM', '-sMODULARIZE', '-sEXIT_RUNTIME'])
    create_file('a.html', '''
      <script src="a.out.js"></script>
      <script>
        Module()
      </script>
    ''')
    self.run_browser('a.html', '/report_result?exit:0')

    # load the data into LZ4FS manually at runtime. This means we compress on the client. This is
    # generally not recommended
    print('manual')
    subprocess.check_output([FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'subdir/file2.txt', 'file3.txt', '--separate-metadata', '--js-output=files.js'])
    self.btest_exit('fs/test_lz4fs.c', 1, cflags=['-DLOAD_MANUALLY', '-sLZ4', '-sFORCE_FILESYSTEM'])
    print('    opts')
    self.btest_exit('fs/test_lz4fs.c', 1, cflags=['-DLOAD_MANUALLY', '-sLZ4', '-sFORCE_FILESYSTEM', '-O2'])
    print('    opts+closure')
    self.btest_exit('fs/test_lz4fs.c', 1, cflags=['-DLOAD_MANUALLY', '-sLZ4',
                                                  '-sFORCE_FILESYSTEM', '-O2',
                                                  '--closure=1', '-g1', '-Wno-closure'])

    # non-lz4 for comparison
    # try:
    #   os.mkdir('files')
    # except OSError:
    #   pass
    # shutil.copy('file1.txt', 'files/'))
    # shutil.copy('file2.txt', 'files/'))
    # shutil.copy('file3.txt', 'files/'))
    # out = subprocess.check_output([FILE_PACKAGER, 'files.data', '--preload', 'files/file1.txt', 'files/file2.txt', 'files/file3.txt'])
    # create_file('files.js', out, binary=True)

  def test_separate_metadata_later(self):
    # see issue #6654 - we need to handle separate-metadata both when we run before
    # the main program, and when we are run later

    create_file('data.dat', ' ')
    self.run_process([FILE_PACKAGER, 'more.data', '--preload', 'data.dat', '--separate-metadata', '--js-output=more.js'])
    self.btest(Path('browser/separate_metadata_later.cpp'), '1', cflags=['-sFORCE_FILESYSTEM'])

  @requires_safari_version(260001, 'TODO: Fails with "Assertion failed: false"') # Fails in Safari 18.5 (20621.2.5.11.8) with Intel x64 CPU only. Passes on Safari 18.5 (20621.2.5.11.8) with ARM M1, Safari 17.6 (17618.3.11.11.7, 17618)/x64 and Safari 26.0.1 (21622.1.22.11.15)/M4
  def test_idbstore(self):
    secret = str(time.time())
    for stage in (0, 1, 2, 3, 0, 1, 2, 0, 0, 1, 4, 2, 5, 0, 4, 6, 5):
      print(stage)
      self.btest_exit('test_idbstore.c',
                      cflags=['-lidbstore.js', f'-DSTAGE={stage}', f'-DSECRET="{secret}"'],
                      output_basename=f'idbstore_{stage}')

  @parameterized({
    'asyncify': (1,),
    'jspi': (2,),
  })
  def test_idbstore_sync(self, asyncify):
    if asyncify == 2:
      self.require_jspi()
    secret = str(time.time())
    self.btest('test_idbstore_sync.c', '8', cflags=['-sSTRICT', '-lidbstore.js', f'-DSECRET="{secret}"', '-O3', '--closure=1', f'-sASYNCIFY={asyncify}'])

  def test_force_exit(self):
    self.btest_exit('test_force_exit.c')

  def test_sdl_pumpevents(self):
    # key events should be detected using SDL_PumpEvents
    self.btest_exit('test_sdl_pumpevents.c', cflags=['--pre-js', test_file('browser/fake_events.js'), '-lSDL', '-lGL'])

  def test_sdl_canvas_size(self):
    self.btest_exit('test_sdl_canvas_size.c',
                    cflags=['-O2', '--minify=0', '--shell-file',
                               test_file('browser/test_sdl_canvas_size.html'), '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_gl_extensions(self):
    self.btest_exit('test_sdl_gl_extensions.c', cflags=['-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_gl_read(self):
    # SDL, OpenGL, readPixels
    self.btest_exit('test_sdl_gl_read.c', cflags=['-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_gl_mapbuffers(self):
    self.btest_exit('test_sdl_gl_mapbuffers.c', cflags=['-sFULL_ES3', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_ogl(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_ogl.c', 'screenshot-gray-purple.png', reference_slack=1,
                 cflags=['-O2', '--minify=0', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_ogl_regal(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_ogl.c', 'screenshot-gray-purple.png', reference_slack=1,
                 cflags=['-O2', '--minify=0', '--preload-file', 'screenshot.png', '-sUSE_REGAL', '-DUSE_REGAL', '--use-preload-plugins', '-lSDL', '-lGL', '-lc++', '-lc++abi'])

  @requires_graphics_hardware
  def test_sdl_ogl_defaultmatrixmode(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_ogl_defaultMatrixMode.c', 'screenshot-gray-purple.png', reference_slack=1,
                 cflags=['--minify=0', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_ogl_p(self):
    # Immediate mode with pointers
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_ogl_p.c', 'screenshot-gray.png', reference_slack=1,
                 cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_ogl_proc_alias(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_ogl_proc_alias.c', 'screenshot-gray-purple.png', reference_slack=1,
                 cflags=['-O2', '-g2', '-sINLINING_LIMIT', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '-sGL_ENABLE_GET_PROC_ADDRESS', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_fog_simple(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_fog_simple.c', 'screenshot-fog-simple.png',
                 cflags=['-O2', '--minify=0', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_fog_negative(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_fog_negative.c', 'screenshot-fog-negative.png',
                 cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_fog_density(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_fog_density.c', 'screenshot-fog-density.png',
                 cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_fog_exp2(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_fog_exp2.c', 'screenshot-fog-exp2.png',
                 cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_fog_linear(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_fog_linear.c', 'screenshot-fog-linear.png', reference_slack=1,
                 cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  @parameterized({
    '': (['-lglfw'],),
    's_flag': (['-sUSE_GLFW=2'],),
    'both_flags': (['-sUSE_GLFW=2', '-lglfw'],),
  })
  def test_glfw(self, args):
    self.btest_exit('test_glfw.c', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-sGL_ENABLE_GET_PROC_ADDRESS'] + args)

  @parameterized({
    '': ([],),
    's_flag': (['-sUSE_GLFW=2'],),
  })
  def test_glfw_minimal(self, args):
    self.btest_exit('test_glfw_minimal.c', cflags=['-lglfw', '-lGL'] + args)

  def test_glfw_time(self):
    self.btest_exit('test_glfw_time.c', cflags=['-sUSE_GLFW=3', '-lglfw', '-lGL'])

  @parameterized({
    '': ([],),
    'proxy_to_pthread': (['-pthread', '-sPROXY_TO_PTHREAD', '-sOFFSCREEN_FRAMEBUFFER'],),
  })
  @requires_graphics_hardware
  def test_egl(self, args):
    self.btest_exit('test_egl.c', cflags=['-O2', '-lEGL', '-lGL', '-sGL_ENABLE_GET_PROC_ADDRESS'] + args)

  @parameterized({
    '': ([],),
    'proxy_to_pthread': (['-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  def test_egl_width_height(self, args):
    self.btest_exit('test_egl_width_height.c', cflags=['-O2', '-lEGL', '-lGL'] + args)

  @requires_graphics_hardware
  def test_egl_createcontext_error(self):
    self.btest_exit('test_egl_createcontext_error.c', cflags=['-lEGL', '-lGL'])

  @parameterized({
    '': ([False],),
    'preload': ([True],),
  })
  def test_hello_world_worker(self, file_data):
    # Test running in a web worker
    create_file('file.dat', 'data for worker')
    create_file('main.html', '''
      <html>
      <body>
        Worker Test
        <script>
          var worker = new Worker('worker.js');
          worker.onmessage = async (event) => {
            await fetch('http://localhost:%s/report_result?' + event.data);
            window.close();
          };
        </script>
      </body>
      </html>
    ''' % self.PORT)

    cmd = [EMCC, test_file('hello_world_worker.c'), '-o', 'worker.js'] + self.get_cflags()
    if file_data:
      cmd += ['--preload-file', 'file.dat']
    self.run_process(cmd)
    self.assertExists('worker.js')
    self.run_browser('main.html', '/report_result?hello from worker, and :' + ('data for w' if file_data else '') + ':')

  @no_wasmfs('https://github.com/emscripten-core/emscripten/issues/19608')
  def test_mmap_lazyfile(self):
    create_file('lazydata.dat', 'hello world')
    create_file('pre.js', '''
      Module["preInit"] = () => {
        FS.createLazyFile('/', "lazy.txt", "lazydata.dat", true, false);
      }
    ''')
    self.btest_exit('test_mmap_lazyfile.c', cflags=['--pre-js=pre.js'], run_in_worker=True)

  @no_wasmfs('https://github.com/emscripten-core/emscripten/issues/19608')
  @no_firefox('keeps sending OPTIONS requests, and eventually errors')
  def test_chunked_synchronous_xhr(self):
    main = 'chunked_sync_xhr.html'
    worker_filename = "download_and_checksum_worker.js"

    create_file(main, r"""
      <!doctype html>
      <html>
      <head><meta charset="utf-8"><title>Chunked XHR</title></head>
      <body>
        Chunked XHR Web Worker Test
        <script>
          var worker = new Worker("%s");
          var buffer = [];
          worker.onmessage = async (event) => {
            if (event.data.channel === "stdout") {
              await fetch('http://localhost:%s/report_result?' + event.data.line);
              window.close();
            } else {
              if (event.data.trace) event.data.trace.split("\n").map(function(v) { console.error(v); });
              if (event.data.line) {
                console.error(event.data.line);
              } else {
                var v = event.data.char;
                if (v == 10) {
                  var line = buffer.splice(0);
                  console.error(line = line.map(function(charCode){return String.fromCharCode(charCode);}).join(''));
                } else {
                  buffer.push(v);
                }
              }
            }
          };
        </script>
      </body>
      </html>
    """ % (worker_filename, self.PORT))

    create_file('worker_prejs.js', r"""
      Module.arguments = ["/bigfile"];
      Module.preInit = () => {
        FS.createLazyFile('/', "bigfile", "http://localhost:11111/bogus_file_path", true, false);
      };
      var doTrace = true;
      Module.print = (s) => self.postMessage({channel: "stdout", line: s});
      Module.printErr = (s) => { self.postMessage({channel: "stderr", char: s, trace: ((doTrace && s === 10) ? new Error().stack : null)}); doTrace = false; };
    """)
    self.compile_btest('checksummer.c', ['-g', '-sSMALL_XHR_CHUNKS', '-o', worker_filename,
                                         '--pre-js', 'worker_prejs.js'])
    chunkSize = 1024
    data = os.urandom(10 * chunkSize + 1) # 10 full chunks and one 1 byte chunk
    checksum = zlib.adler32(data) & 0xffffffff # Python 2 compatibility: force bigint

    server = HttpServerThread(make_test_chunked_synchronous_xhr_server(True, data, self.PORT))
    server.start()

    # block until the server is actually ready
    for i in range(60):
      try:
        urlopen('http://localhost:11111')
        break
      except Exception as e:
        print('(sleep for server)')
        time.sleep(1)
        if i == 60:
          raise e

    try:
      self.run_browser(main, '/report_result?' + str(checksum))
    finally:
      server.stop()
      server.join()
    # Avoid race condition on cleanup, wait a bit so that processes have released file locks so that test tearDown won't
    # attempt to rmdir() files in use.
    if WINDOWS:
      time.sleep(2)

  @requires_graphics_hardware
  def test_glgears(self, extra_args=[]):  # noqa
    self.reftest('hello_world_gles.c', 'gears.png', reference_slack=3,
                 cflags=['-DHAVE_BUILTIN_SINCOS', '-lGL', '-lglut'] + extra_args)

  @requires_graphics_hardware
  def test_glgears_pthreads(self, extra_args=[]):  # noqa
    # test that a program that doesn't use pthreads still works with with pthreads enabled
    # (regression test for https://github.com/emscripten-core/emscripten/pull/8059#issuecomment-488105672)
    self.test_glgears(['-pthread'])

  @requires_graphics_hardware
  def test_glgears_long(self):
    args = ['-DHAVE_BUILTIN_SINCOS', '-DLONGTEST', '-lGL', '-lglut', '-DANIMATE']
    self.btest('hello_world_gles.c', expected='0', cflags=args)

  @requires_graphics_hardware
  @parameterized({
    '': ('hello_world_gles.c',),
    'full': ('hello_world_gles_full.c',),
    'full_944': ('hello_world_gles_full_944.c',),
  })
  @flaky('https://github.com/emscripten-core/emscripten/issues/25329')
  def test_glgears_animation(self, filename):
    shutil.copy(test_file('browser/fake_events.js'), '.')
    args = ['-o', 'something.html',
            '-DHAVE_BUILTIN_SINCOS', '-sGL_TESTING', '-lGL', '-lglut',
            '--shell-file', test_file('hello_world_gles_shell.html')]
    if 'full' in filename:
      args += ['-sFULL_ES2']
    self.compile_btest(filename, args)
    self.run_browser('something.html', '/report_gl_result?true')

  @requires_graphics_hardware
  def test_fulles2_sdlproc(self):
    self.btest_exit('full_es2_sdlproc.c', cflags=['-sGL_TESTING', '-DHAVE_BUILTIN_SINCOS', '-sFULL_ES2', '-lGL', '-lSDL', '-lglut', '-sGL_ENABLE_GET_PROC_ADDRESS', '-Wno-int-conversion'])

  @requires_graphics_hardware
  @flaky('https://github.com/emscripten-core/emscripten/issues/25329')
  def test_glgears_deriv(self):
    self.reftest('hello_world_gles_deriv.c', 'gears.png', reference_slack=2,
                 cflags=['-DHAVE_BUILTIN_SINCOS', '-lGL', '-lglut'])
    assert 'gl-matrix' not in read_file('test.html'), 'Should not include glMatrix when not needed'

  @requires_graphics_hardware
  @parameterized({
    'Hello_Triangle': ('CH02_HelloTriangle.o', [], []),
    'Simple_VertexShader': ('CH08_SimpleVertexShader.o', [], []),
    'Simple_Texture2D': ('CH09_SimpleTexture2D.o', [], []),
    'Simple_TextureCubemap': ('CH09_TextureCubemap.o', [], []),
    'TextureWrap': ('CH09_TextureWrap.o', [], []),
    'MultiTexture': ('CH10_MultiTexture.o', ['Chapter_10/MultiTexture/basemap.tga', 'Chapter_10/MultiTexture/lightmap.tga'], []),
    # run this individual test with optimizations and closure for more coverage
    'ParticleSystem': ('CH13_ParticleSystem.o', ['Chapter_13/ParticleSystem/smoke.tga'], ['-O2']),
  })
  def test_glbook(self, program, images, cflags):
    self.cflags.append('-Wno-int-conversion')
    self.cflags.append('-Wno-pointer-sign')

    libs = self.get_library('third_party/glbook', [
      'Chapter_2/Hello_Triangle/CH02_HelloTriangle.o',
      'Chapter_8/Simple_VertexShader/CH08_SimpleVertexShader.o',
      'Chapter_9/Simple_Texture2D/CH09_SimpleTexture2D.o',
      'Chapter_9/Simple_TextureCubemap/CH09_TextureCubemap.o',
      'Chapter_9/TextureWrap/CH09_TextureWrap.o',
      'Chapter_10/MultiTexture/CH10_MultiTexture.o',
      'Chapter_13/ParticleSystem/CH13_ParticleSystem.o',
    ], configure=None)

    def book_path(path):
      return test_file('third_party/glbook', path)

    cflags += ['-lGL', '-lEGL', '-lX11']
    for image in images:
      cflags += ['--preload-file', f'{book_path(image)}@{os.path.basename(image)}']

    lib = [l for l in libs if program in os.path.basename(l)][0]

    self.reftest(lib, book_path(program.replace('.o', '.png')), cflags=cflags)

  @requires_graphics_hardware
  @parameterized({
    'normal': (['-sFULL_ES2'],),
    # Enabling FULL_ES3 also enables ES2 automatically
    'full_es3': (['-sFULL_ES3'],),
  })
  def test_gles2_emulation(self, args):
    shutil.copy(test_file('third_party/glbook/Chapter_10/MultiTexture/basemap.tga'), '.')
    shutil.copy(test_file('third_party/glbook/Chapter_10/MultiTexture/lightmap.tga'), '.')
    shutil.copy(test_file('third_party/glbook/Chapter_13/ParticleSystem/smoke.tga'), '.')

    for source, reference in [
      ('third_party/glbook/Chapter_2/Hello_Triangle/Hello_Triangle_orig.c', 'third_party/glbook/CH02_HelloTriangle.png'),
      # ('third_party/glbook/Chapter_8/Simple_VertexShader/Simple_VertexShader_orig.c', 'third_party/glbook/CH08_SimpleVertexShader.png'), # XXX needs INT extension in WebGL
      ('third_party/glbook/Chapter_9/TextureWrap/TextureWrap_orig.c', 'third_party/glbook/CH09_TextureWrap.png'),
      # ('third_party/glbook/Chapter_9/Simple_TextureCubemap/Simple_TextureCubemap_orig.c', 'third_party/glbook/CH09_TextureCubemap.png'), # XXX needs INT extension in WebGL
      ('third_party/glbook/Chapter_9/Simple_Texture2D/Simple_Texture2D_orig.c', 'third_party/glbook/CH09_SimpleTexture2D.png'),
      ('third_party/glbook/Chapter_10/MultiTexture/MultiTexture_orig.c', 'third_party/glbook/CH10_MultiTexture.png'),
      ('third_party/glbook/Chapter_13/ParticleSystem/ParticleSystem_orig.c', 'third_party/glbook/CH13_ParticleSystem.png'),
    ]:
      print(source)
      self.reftest(source, reference,
                   cflags=['-I' + test_file('third_party/glbook/Common'),
                              test_file('third_party/glbook/Common/esUtil.c'),
                              test_file('third_party/glbook/Common/esShader.c'),
                              test_file('third_party/glbook/Common/esShapes.c'),
                              test_file('third_party/glbook/Common/esTransform.c'),
                              '-lGL', '-lEGL', '-lX11', '-Wno-int-conversion', '-Wno-pointer-sign',
                              '--preload-file', 'basemap.tga', '--preload-file', 'lightmap.tga', '--preload-file', 'smoke.tga'] + args)

  @requires_webgl2
  def test_clientside_vertex_arrays_es3(self):
    self.reftest('clientside_vertex_arrays_es3.c', 'gl_triangle.png', cflags=['-sFULL_ES3', '-sUSE_GLFW=3', '-lglfw', '-lGLESv2'])

  def test_emscripten_api(self):
    self.btest_exit('emscripten_api_browser.c', cflags=['-lSDL'])

  @also_with_wasmfs
  def test_emscripten_async_load_script(self):
    def setup():
      create_file('script1.js', '''
        Module._set(456);
      ''')
      create_file('file1.txt', 'first')
      create_file('file2.txt', 'second')

    setup()
    self.run_process([FILE_PACKAGER, 'test.data', '--preload', 'file1.txt', 'file2.txt'], stdout=open('script2.js', 'w'))
    self.btest_exit('test_emscripten_async_load_script.c', cflags=['-sFORCE_FILESYSTEM'])

    # check using file packager to another dir
    self.clear()
    setup()
    ensure_dir('sub')
    self.run_process([FILE_PACKAGER, 'sub/test.data', '--preload', 'file1.txt', 'file2.txt'], stdout=open('script2.js', 'w'))
    shutil.copy(Path('sub/test.data'), '.')
    self.btest_exit('test_emscripten_async_load_script.c', cflags=['-sFORCE_FILESYSTEM'])

  @also_with_wasmfs
  def test_emscripten_overlapped_package(self):
    # test that a program that loads multiple file_packager.py packages has a correctly initialized filesystem.
    # this exercises https://github.com/emscripten-core/emscripten/issues/23602 whose root cause was a difference
    # between JS FS and WASMFS behavior.
    def setup():
      ensure_dir('sub')
      create_file('sub/file1.txt', 'first')
      create_file('sub/file2.txt', 'second')

    setup()
    self.run_process([FILE_PACKAGER, 'test.data', '--preload', 'sub/file1.txt@/target/file1.txt'], stdout=open('script1.js', 'w'))
    self.run_process([FILE_PACKAGER, 'test2.data', '--preload', 'sub/file2.txt@/target/file2.txt'], stdout=open('script2.js', 'w'))
    self.btest_exit('test_emscripten_overlapped_package.c', cflags=['-sFORCE_FILESYSTEM'])
    self.clear()

  def test_emscripten_api_infloop(self):
    self.btest_exit('emscripten_api_browser_infloop.cpp')

  @parameterized({
    '': ([],),
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'],),
  })
  def test_emscripten_main_loop(self, args):
    self.btest_exit('test_emscripten_main_loop.c', cflags=args)

  @parameterized({
    '': ([],),
    # test pthreads + AUTO_JS_LIBRARIES mode as well
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD', '-sAUTO_JS_LIBRARIES=0'],),
  })
  def test_emscripten_main_loop_settimeout(self, args):
    self.btest_exit('test_emscripten_main_loop_settimeout.c', cflags=args)

  @parameterized({
    '': ([],),
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  def test_emscripten_main_loop_and_blocker(self, args):
    self.btest_exit('test_emscripten_main_loop_and_blocker.c', cflags=args)

  def test_emscripten_main_loop_and_blocker_exit(self):
    # Same as above but tests that EXIT_RUNTIME works with emscripten_main_loop.  The
    # app should still stay alive until the loop ends
    self.btest_exit('test_emscripten_main_loop_and_blocker.c')

  @parameterized({
    '': ([],),
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD'],),
    'strict': (['-sSTRICT'],),
  })
  def test_emscripten_main_loop_setimmediate(self, args):
    self.btest_exit('test_emscripten_main_loop_setimmediate.c', cflags=args)

  @parameterized({
    '': ([],),
    'O1': (['-O1'],),
  })
  def test_fs_after_main(self, args):
    self.btest_exit('test_fs_after_main.c', cflags=args)

  def test_sdl_quit(self):
    self.btest_exit('test_sdl_quit.c', cflags=['-lSDL', '-lGL'])

  def test_sdl_resize(self):
    # FIXME(https://github.com/emscripten-core/emscripten/issues/12978)
    self.cflags.append('-Wno-deprecated-declarations')
    self.btest_exit('test_sdl_resize.c', cflags=['-lSDL', '-lGL'])

  def test_glshaderinfo(self):
    self.btest_exit('test_glshaderinfo.c', cflags=['-lGL', '-lglut'])

  @requires_graphics_hardware
  def test_glgetattachedshaders(self):
    self.btest('glgetattachedshaders.c', '1', cflags=['-lGL', '-lEGL'])

  # Covered by dEQP text suite (we can remove it later if we add coverage for that).
  @requires_graphics_hardware
  def test_glframebufferattachmentinfo(self):
    self.btest('glframebufferattachmentinfo.c', '1', cflags=['-lGLESv2', '-lEGL'])

  @requires_graphics_hardware
  def test_sdl_glshader(self):
    self.reftest('test_sdl_glshader.c', 'test_sdl_glshader.png', cflags=['-O2', '--closure=1', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  @requires_graphics_hardware
  def test_sdl_glshader2(self):
    self.btest_exit('test_sdl_glshader2.c', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  @requires_graphics_hardware
  def test_gl_glteximage(self):
    self.btest('gl_teximage.c', '1', cflags=['-lGL', '-lSDL'])

  @parameterized({
    '': ([],),
    'pthreads': (['-pthread', '-sPROXY_TO_PTHREAD', '-sOFFSCREEN_FRAMEBUFFER'],),
    'pthreads_main_module': (['-pthread', '-sPROXY_TO_PTHREAD', '-sOFFSCREEN_FRAMEBUFFER', '-sMAIN_MODULE', '-Wno-experimental'],),
  })
  @requires_graphics_hardware
  def test_gl_textures(self, args):
    self.btest_exit('gl_textures.c', cflags=['-lGL', '-g', '-sSTACK_SIZE=1MB'] + args)

  @requires_graphics_hardware
  def test_gl_ps(self):
    # pointers and a shader
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('gl_ps.c', 'gl_ps.png', cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '--use-preload-plugins'], reference_slack=1)

  @requires_graphics_hardware
  def test_gl_ps_packed(self):
    # packed data that needs to be strided
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('gl_ps_packed.c', 'gl_ps.png', cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '--use-preload-plugins'], reference_slack=1)

  @requires_graphics_hardware
  def test_gl_ps_strides(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('gl_ps_strides.c', 'gl_ps_strides.png', cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_gl_ps_worker(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('gl_ps_worker.c', 'gl_ps.png', cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '--use-preload-plugins'], reference_slack=1)

  @requires_graphics_hardware
  def test_gl_renderers(self):
    self.reftest('gl_renderers.c', 'gl_renderers.png', cflags=['-sGL_UNSAFE_OPTS=0', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_2gb('render fails')
  @no_4gb('render fails')
  def test_gl_stride(self):
    self.reftest('gl_stride.c', 'gl_stride.png', cflags=['-sGL_UNSAFE_OPTS=0', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_gl_vertex_buffer_pre(self):
    self.reftest('gl_vertex_buffer_pre.c', 'gl_vertex_buffer_pre.png', cflags=['-sGL_UNSAFE_OPTS=0', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_gl_vertex_buffer(self):
    self.reftest('gl_vertex_buffer.c', 'gl_vertex_buffer.png', cflags=['-sGL_UNSAFE_OPTS=0', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'], reference_slack=1)

  @requires_graphics_hardware
  def test_gles2_uniform_arrays(self):
    self.btest_exit('test_gles2_uniform_arrays.c', cflags=['-sGL_ASSERTIONS', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_gles2_conformance(self):
    self.btest_exit('test_gles2_conformance.c', cflags=['-sGL_ASSERTIONS', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_matrix_identity(self):
    self.btest('gl_matrix_identity.c', expected=['-1882984448', '460451840', '1588195328', '2411982848'], cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre(self):
    self.reftest('third_party/cubegeom/cubegeom_pre.c', 'third_party/cubegeom/cubegeom_pre.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_regal(self):
    self.reftest('third_party/cubegeom/cubegeom_pre.c', 'third_party/cubegeom/cubegeom_pre.png', cflags=['-sUSE_REGAL', '-DUSE_REGAL', '-lGL', '-lSDL', '-lc++', '-lc++abi'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre2(self):
    self.reftest('third_party/cubegeom/cubegeom_pre2.c', 'third_party/cubegeom/cubegeom_pre2.png', cflags=['-sGL_DEBUG', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL']) # some coverage for GL_DEBUG not breaking the build

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre3(self):
    self.reftest('third_party/cubegeom/cubegeom_pre3.c', 'third_party/cubegeom/cubegeom_pre2.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @parameterized({
    '': ([],),
    'tracing': (['-sTRACE_WEBGL_CALLS'],),
  })
  @requires_graphics_hardware
  def test_cubegeom(self, args):
    self.reftest('third_party/cubegeom/cubegeom.c', 'third_party/cubegeom/cubegeom.png', cflags=['-O2', '-g', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'] + args)

  @requires_graphics_hardware
  def test_cubegeom_regal(self):
    self.reftest('third_party/cubegeom/cubegeom.c', 'third_party/cubegeom/cubegeom.png', cflags=['-O2', '-g', '-DUSE_REGAL', '-sUSE_REGAL', '-lGL', '-lSDL', '-lc++', '-lc++abi'])

  @requires_graphics_hardware
  def test_cubegeom_regal_pthread(self):
    self.reftest('third_party/cubegeom/cubegeom.c', 'third_party/cubegeom/cubegeom.png', cflags=['-O2', '-g', '-pthread', '-DUSE_REGAL', '-pthread', '-sUSE_REGAL', '-lGL', '-lSDL', '-lc++', '-lc++abi'])

  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'O1': (['-O1'],),
    # also test -Os in wasm, which uses meta-dce, which should not break
    # legacy gl emulation hacks
    'Os': (['-Os'],),
  })
  def test_cubegeom_proc(self, opts):
    create_file('side.c', r'''

extern void* SDL_GL_GetProcAddress(const char *);

void *glBindBuffer = 0; // same name as the gl function, to check that the collision does not break us

void *getBindBuffer() {
  if (!glBindBuffer) glBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
  return glBindBuffer;
}
''')
    self.reftest('third_party/cubegeom/cubegeom_proc.c', 'third_party/cubegeom/cubegeom.png', cflags=opts + ['side.c', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  @also_with_wasmfs
  @requires_graphics_hardware
  def test_cubegeom_glew(self):
    self.reftest('third_party/cubegeom/cubegeom_glew.c', 'third_party/cubegeom/cubegeom.png', cflags=['-O2', '--closure=1', '-sLEGACY_GL_EMULATION', '-lGL', '-lGLEW', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_color(self):
    self.reftest('third_party/cubegeom/cubegeom_color.c', 'third_party/cubegeom/cubegeom_color.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal(self):
    self.reftest('third_party/cubegeom/cubegeom_normal.c', 'third_party/cubegeom/cubegeom_normal.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap(self): # draw is given a direct pointer to clientside memory, no element array buffer
    self.reftest('third_party/cubegeom/cubegeom_normal_dap.c', 'third_party/cubegeom/cubegeom_normal.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far(self): # indices do nto start from 0
    self.reftest('third_party/cubegeom/cubegeom_normal_dap_far.c', 'third_party/cubegeom/cubegeom_normal.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far_range(self): # glDrawRangeElements
    self.reftest('third_party/cubegeom/cubegeom_normal_dap_far_range.c', 'third_party/cubegeom/cubegeom_normal.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far_glda(self): # use glDrawArrays
    self.reftest('third_party/cubegeom/cubegeom_normal_dap_far_glda.c', 'third_party/cubegeom/cubegeom_normal_dap_far_glda.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_firefox('fails on CI but works locally')
  def test_cubegeom_normal_dap_far_glda_quad(self): # with quad
    self.reftest('third_party/cubegeom/cubegeom_normal_dap_far_glda_quad.c', 'third_party/cubegeom/cubegeom_normal_dap_far_glda_quad.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_mt(self):
    self.reftest('third_party/cubegeom/cubegeom_mt.c', 'third_party/cubegeom/cubegeom_mt.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL']) # multitexture

  @requires_graphics_hardware
  def test_cubegeom_color2(self):
    self.reftest('third_party/cubegeom/cubegeom_color2.c', 'third_party/cubegeom/cubegeom_color2.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_texturematrix(self):
    self.reftest('third_party/cubegeom/cubegeom_texturematrix.c', 'third_party/cubegeom/cubegeom_texturematrix.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_fog(self):
    self.reftest('third_party/cubegeom/cubegeom_fog.c', 'third_party/cubegeom/cubegeom_fog.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_vao(self):
    self.reftest('third_party/cubegeom/cubegeom_pre_vao.c', 'third_party/cubegeom/cubegeom_pre_vao.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_vao_regal(self):
    self.reftest('third_party/cubegeom/cubegeom_pre_vao.c', 'third_party/cubegeom/cubegeom_pre_vao.png', cflags=['-sUSE_REGAL', '-DUSE_REGAL', '-lGL', '-lSDL', '-lc++', '-lc++abi'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre2_vao(self):
    self.reftest('third_party/cubegeom/cubegeom_pre2_vao.c', 'third_party/cubegeom/cubegeom_pre_vao.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  @requires_graphics_hardware
  def test_cubegeom_pre2_vao2(self):
    self.reftest('third_party/cubegeom/cubegeom_pre2_vao2.c', 'third_party/cubegeom/cubegeom_pre2_vao2.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '-sGL_ENABLE_GET_PROC_ADDRESS'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_vao_es(self):
    self.reftest('third_party/cubegeom/cubegeom_pre_vao_es.c', 'third_party/cubegeom/cubegeom_pre_vao.png', cflags=['-sFULL_ES2', '-lGL', '-lSDL'])

  @requires_webgl2
  @no_swiftshader
  def test_cubegeom_row_length(self):
    self.reftest('third_party/cubegeom/cubegeom_pre_vao_es.c', 'third_party/cubegeom/cubegeom_pre_vao.png', cflags=['-sFULL_ES2', '-lGL', '-lSDL', '-DUSE_UNPACK_ROW_LENGTH', '-sMIN_WEBGL_VERSION=2'])

  @requires_graphics_hardware
  def test_cubegeom_u4fv_2(self):
    self.reftest('third_party/cubegeom/cubegeom_u4fv_2.c', 'third_party/cubegeom/cubegeom_u4fv_2.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cube_explosion(self):
    self.reftest('cube_explosion.c', 'cube_explosion.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_glgettexenv(self):
    self.btest('glgettexenv.c', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'], expected='1')

  def test_sdl_canvas_blank(self):
    self.reftest('test_sdl_canvas_blank.c', 'test_sdl_canvas_blank.png', cflags=['-lSDL', '-lGL'])

  def test_sdl_canvas_palette(self):
    self.reftest('test_sdl_canvas_palette.c', 'test_sdl_canvas_palette.png', cflags=['-lSDL', '-lGL'])

  def test_sdl_canvas_twice(self):
    self.reftest('test_sdl_canvas_twice.c', 'test_sdl_canvas_twice.png', cflags=['-lSDL', '-lGL'])

  def test_sdl_set_clip_rect(self):
    self.reftest('test_sdl_set_clip_rect.c', 'test_sdl_set_clip_rect.png', cflags=['-lSDL', '-lGL'])

  def test_sdl_maprgba(self):
    self.reftest('test_sdl_maprgba.c', 'test_sdl_maprgba.png', cflags=['-lSDL', '-lGL'], reference_slack=3)

  def test_sdl_create_rgb_surface_from(self):
    self.reftest('test_sdl_create_rgb_surface_from.c', 'test_sdl_create_rgb_surface_from.png', cflags=['-lSDL', '-lGL'])

  def test_sdl_rotozoom(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl_rotozoom.c', 'test_sdl_rotozoom.png', cflags=['--preload-file', 'screenshot.png', '--use-preload-plugins', '-lSDL', '-lGL'], reference_slack=3)

  def test_sdl_gfx_primitives(self):
    self.reftest('test_sdl_gfx_primitives.c', 'test_sdl_gfx_primitives.png', cflags=['-lSDL', '-lGL'], reference_slack=1)

  def test_sdl_canvas_palette_2(self):
    create_file('pre.js', '''
      Module['preRun'] = () => {
        SDL.defaults.copyOnLock = false;
      };
    ''')

    create_file('args-r.js', '''
      Module['arguments'] = ['-r'];
    ''')

    create_file('args-g.js', '''
      Module['arguments'] = ['-g'];
    ''')

    create_file('args-b.js', '''
      Module['arguments'] = ['-b'];
    ''')

    self.reftest('test_sdl_canvas_palette_2.c', 'test_sdl_canvas_palette_r.png', cflags=['--pre-js', 'pre.js', '--pre-js', 'args-r.js', '-lSDL', '-lGL'])
    self.reftest('test_sdl_canvas_palette_2.c', 'test_sdl_canvas_palette_g.png', cflags=['--pre-js', 'pre.js', '--pre-js', 'args-g.js', '-lSDL', '-lGL'])
    self.reftest('test_sdl_canvas_palette_2.c', 'test_sdl_canvas_palette_b.png', cflags=['--pre-js', 'pre.js', '--pre-js', 'args-b.js', '-lSDL', '-lGL'])

  def test_sdl_ttf_render_text_solid(self):
    self.reftest('test_sdl_ttf_render_text_solid.c', 'test_sdl_ttf_render_text_solid.png', cflags=['-O2', '-lSDL', '-lGL'])

  def test_sdl_alloctext(self):
    self.btest_exit('test_sdl_alloctext.c', cflags=['-lSDL', '-lGL'])

  def test_sdl_surface_refcount(self):
    self.btest_exit('test_sdl_surface_refcount.c', cflags=['-lSDL'])

  def test_sdl_free_screen(self):
    self.reftest('test_sdl_free_screen.c', 'htmltest.png', cflags=['-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_glbegin_points(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('glbegin_points.c', 'glbegin_points.png', cflags=['--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_s3tc(self):
    shutil.copy(test_file('screenshot.dds'), '.')
    self.reftest('s3tc.c', 's3tc.png', cflags=['--preload-file', 'screenshot.dds', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_s3tc_ffp_only(self):
    shutil.copy(test_file('screenshot.dds'), '.')
    self.reftest('s3tc.c', 's3tc.png', cflags=['--preload-file', 'screenshot.dds', '-sLEGACY_GL_EMULATION', '-sGL_FFP_ONLY', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'subimage': (['-DTEST_TEXSUBIMAGE'],),
  })
  def test_anisotropic(self, args):
    shutil.copy(test_file('browser/water.dds'), '.')
    self.reftest('test_anisotropic.c', 'test_anisotropic.png', reference_slack=2, cflags=['--preload-file', 'water.dds', '-sLEGACY_GL_EMULATION', '-lGL', '-lSDL', '-Wno-incompatible-pointer-types'] + args)

  @requires_graphics_hardware
  def test_tex_nonbyte(self):
    self.reftest('tex_nonbyte.c', 'tex_nonbyte.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_float_tex(self):
    self.reftest('float_tex.c', 'float_tex.png', cflags=['-lGL', '-lglut'])

  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'tracing': (['-sTRACE_WEBGL_CALLS'],),
    'es2': (['-sMIN_WEBGL_VERSION=2', '-sFULL_ES2', '-sWEBGL2_BACKWARDS_COMPATIBILITY_EMULATION'],),
    'es2_tracing': (['-sMIN_WEBGL_VERSION=2', '-sFULL_ES2', '-sWEBGL2_BACKWARDS_COMPATIBILITY_EMULATION', '-sTRACE_WEBGL_CALLS'],),
  })
  def test_subdata(self, args):
    if '-sMIN_WEBGL_VERSION=2' in args and webgl2_disabled():
      self.skipTest('This test requires WebGL2 to be available')
    if self.is_4gb() and '-sMIN_WEBGL_VERSION=2' in args:
      self.skipTest('texSubImage2D fails: https://crbug.com/325090165')
    self.reftest('gl_subdata.c', 'float_tex.png', cflags=['-lGL', '-lglut'] + args)

  @requires_graphics_hardware
  def test_perspective(self):
    self.reftest('perspective.c', 'perspective.png', cflags=['-sLEGACY_GL_EMULATION', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_glerror(self):
    self.btest('gl_error.c', expected='1', cflags=['-sLEGACY_GL_EMULATION', '-lGL'])

  @parameterized({
    '': ([],),
    'strict': (['-lopenal', '-sSTRICT'],),
    'closure': (['--closure=1'],),
  })
  def test_openal_error(self, args):
    self.btest_exit('openal/test_openal_error.c', cflags=args)

  @requires_microphone_access
  def test_openal_capture_sanity(self):
    self.btest_exit('openal/test_openal_capture_sanity.c')

  def test_openal_extensions(self):
    self.btest_exit('openal/test_openal_extensions.c')

  def test_openal_playback(self):
    shutil.copy(test_file('sounds/audio.wav'), '.')
    self.btest_exit('openal/test_openal_playback.c', cflags=['-O2', '--preload-file', 'audio.wav'])

  def test_openal_buffers(self):
    self.btest_exit('openal/test_openal_buffers.c', cflags=['--preload-file', test_file('sounds/the_entertainer.wav') + '@/'])

  def test_runtimelink(self):
    create_file('header.h', r'''
      struct point {
        int x, y;
      };
    ''')

    create_file('supp.c', r'''
      #include <stdio.h>
      #include "header.h"

      extern void mainFunc(int x);
      extern int mainInt;

      void suppFunc(struct point *p) {
        printf("supp: %d,%d\n", p->x, p->y);
        mainFunc(p->x + p->y);
        printf("supp see: %d\n", mainInt);
      }

      int suppInt = 76;
    ''')

    create_file('main.c', r'''
      #include <stdio.h>
      #include <assert.h>
      #include "header.h"

      extern void suppFunc(struct point *p);
      extern int suppInt;

      void mainFunc(int x) {
        printf("main: %d\n", x);
        assert(x == 56);
      }

      int mainInt = 543;

      int main( int argc, const char *argv[] ) {
        struct point p = { 54, 2 };
        suppFunc(&p);
        printf("main see: %d\nok.\n", suppInt);
        assert(suppInt == 76);
        return 0;
      }
    ''')
    self.run_process([EMCC, 'supp.c', '-o', 'supp.wasm', '-sSIDE_MODULE', '-O2'] + self.get_cflags())
    self.btest_exit('main.c', cflags=['-sMAIN_MODULE=2', '-O2', 'supp.wasm'])

  @also_with_wasm2js
  def test_pre_run_deps(self):
    # Adding a dependency in preRun will delay run
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', '$addRunDependency')
    create_file('pre.js', '''
      Module.preRun = () => {
        addRunDependency('foo');
        out('preRun called, added a dependency...');
        setTimeout(function() {
          Module.okk = 10;
          removeRunDependency('foo')
        }, 2000);
      };
    ''')

    self.btest('test_pre_run_deps.c', expected='10', cflags=['--pre-js', 'pre.js'])

  @also_with_wasm2js
  @parameterized({
    '': ([], '600'),
    'no_main': (['-DNO_MAIN', '--pre-js', 'pre_runtime.js'], '601'), # 601, because no main means we *do* run another call after exit()
  })
  def test_runtime_misuse(self, extra_args, second_code):
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', '$ccall,$cwrap')
    post_prep = '''
      var expected_ok = false;
      function doCcall(n) {
        ccall('note', 'string', ['number'], [n]);
      }
      var wrapped = cwrap('note', 'string', ['number']); // returns a string to suppress cwrap optimization
      function doCwrapCall(n) {
        var str = wrapped(n);
        out('got ' + str);
        assert(str === 'silly-string');
      }
      function doDirectCall(n) {
        Module['_note'](n);
      }
    '''
    post_test = '''
      var ok = false;
      try {
        doCcall(1);
        ok = true; // should fail and not reach here, runtime is not ready yet so ccall will abort
      } catch(e) {
        out('expected fail 1:', e);
        assert(e.toString().includes('Assertion failed')); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);

      ok = false;
      try {
        doCwrapCall(2);
        ok = true; // should fail and not reach here, runtime is not ready yet so cwrap call will abort
      } catch(e) {
        out('expected fail 2:', e);
        assert(e.toString().includes('Assertion failed')); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);

      ok = false;
      try {
        doDirectCall(3);
        ok = true; // should fail and not reach here, runtime is not ready yet so any code execution
      } catch(e) {
        out('expected fail 3:', e);
        assert(e.toString().includes('Assertion failed')); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);
    '''

    post_hook = r'''
      function myJSCallback() {
        // Run on the next event loop, as code may run in a postRun right after main().
        setTimeout(async () => {
          out('done timeout noted = ' + Module.noted);
          assert(Module.noted);
          await fetch('/report_result?' + HEAP32[Module.noted/4]);
          window.close();
        }, 0);
        // called from main, this is an ok time
        doCcall(100);
        doCwrapCall(200);
        doDirectCall(300);
      }
    '''

    create_file('pre_runtime.js', r'''
      Module.onRuntimeInitialized = myJSCallback;
    ''')

    print('mem init, so async, call too early')
    create_file('post.js', post_prep + post_test + post_hook)
    self.btest('test_runtime_misuse.c', expected='600', cflags=['--post-js', 'post.js', '-sEXIT_RUNTIME'] + extra_args, reporting=Reporting.NONE)

    print('sync startup, call too late')
    create_file('post.js', post_prep + 'Module.postRun = () => { ' + post_test + ' };' + post_hook)
    self.btest('test_runtime_misuse.c', expected=second_code, cflags=['--post-js', 'post.js', '-sEXIT_RUNTIME'] + extra_args, reporting=Reporting.NONE)

    print('sync, runtime still alive, so all good')
    create_file('post.js', post_prep + 'expected_ok = true; Module.postRun = () => { ' + post_test + ' };' + post_hook)
    self.btest('test_runtime_misuse.c', expected='606', cflags=['--post-js', 'post.js'] + extra_args, reporting=Reporting.NONE)

  def test_cwrap_early(self):
    self.btest('browser/test_cwrap_early.c', cflags=['-O2', '-sASSERTIONS', '--pre-js', test_file('browser/test_cwrap_early.js'), '-sEXPORTED_RUNTIME_METHODS=cwrap'], expected='0')

  @no_wasm64('TODO: wasm64 + BUILD_AS_WORKER')
  def test_worker_api(self):
    self.compile_btest('worker_api_worker.cpp', ['-o', 'worker.js', '-sBUILD_AS_WORKER', '-sEXPORTED_FUNCTIONS=_one'])
    self.btest('worker_api_main.cpp', expected='566')

  @no_wasm64('TODO: wasm64 + BUILD_AS_WORKER')
  def test_worker_api_2(self):
    self.compile_btest('worker_api_2_worker.cpp', ['-o', 'worker.js', '-sBUILD_AS_WORKER', '-O2', '--minify=0', '-sEXPORTED_FUNCTIONS=_one,_two,_three,_four', '--closure=1'])
    self.btest('worker_api_2_main.cpp', cflags=['-O2', '--minify=0'], expected='11')

  @no_wasm64('TODO: wasm64 + BUILD_AS_WORKER')
  def test_worker_api_3(self):
    self.compile_btest('worker_api_3_worker.cpp', ['-o', 'worker.js', '-sBUILD_AS_WORKER', '-sEXPORTED_FUNCTIONS=_one'])
    self.btest('worker_api_3_main.cpp', expected='5')

  @no_wasm64('TODO: wasm64 + BUILD_AS_WORKER')
  def test_worker_api_sleep(self):
    self.compile_btest('worker_api_worker_sleep.cpp', ['-o', 'worker.js', '-sBUILD_AS_WORKER', '-sEXPORTED_FUNCTIONS=_one', '-sASYNCIFY'])
    self.btest('worker_api_main.cpp', expected='566')

  @no_wasm64('TODO: wasm64 + BUILD_AS_WORKER')
  def test_worker_api_with_pthread_compilation_fails(self):
    self.run_process([EMCC, '-c', '-o', 'hello.o', test_file('hello_world.c')])
    expected = "pthreads + BUILD_AS_WORKER require separate modes that don't work together, see https://github.com/emscripten-core/emscripten/issues/8854"
    self.assert_fail([EMCC, 'hello.o', '-o', 'a.js', '-g', '--closure=1', '-pthread', '-sBUILD_AS_WORKER'], expected)

  @also_with_wasmfs
  def test_wget(self):
    create_file('test.txt', 'emscripten')
    self.btest_exit('test_wget.c', cflags=['-sASYNCIFY'])

  def test_wget_data(self):
    create_file('test.txt', 'emscripten')
    self.btest_exit('test_wget_data.c', cflags=['-O2', '-g2', '-sASYNCIFY'])

  @also_with_wasmfs
  @parameterized({
    '': ([],),
    'O2': (['-O2'],),
  })
  def test_emscripten_async_wget(self, args):
    shutil.copy(test_file('screenshot.png'), '.') # preloaded *after* run
    self.btest_exit('test_emscripten_async_wget.c', cflags=['-lSDL'] + args)

  @also_with_wasmfs
  def test_emscripten_async_wget2(self):
    self.btest_exit('test_emscripten_async_wget2.cpp')

  @disabled('https://github.com/emscripten-core/emscripten/issues/15818')
  def test_emscripten_async_wget2_data(self):
    create_file('hello.txt', 'Hello Emscripten!')
    self.btest('test_emscripten_async_wget2_data.cpp', expected='0')

  def test_emscripten_async_wget_side_module(self):
    self.emcc('test_emscripten_async_wget_side_module.c', ['-o', 'lib.wasm', '-O2', '-sSIDE_MODULE'])
    self.btest_exit('test_emscripten_async_wget_main_module.c', cflags=['-O2', '-sMAIN_MODULE=2'])

  @parameterized({
    '': ([],),
    'lz4': (['-sLZ4'],),
  })
  def test_preload_module(self, args):
    create_file('library.c', r'''
      #include <stdio.h>
      int library_func() {
        return 42;
      }
    ''')
    self.emcc('library.c', ['-sSIDE_MODULE', '-O2', '-o', 'library.so'])
    create_file('main.c', r'''
      #include <assert.h>
      #include <dlfcn.h>
      #include <stdio.h>
      #include <emscripten.h>
      int main() {
        int found = EM_ASM_INT(
          return preloadedWasm['/library.so'] !== undefined;
        );
        assert(found);
        void *lib_handle = dlopen("/library.so", RTLD_NOW);
        assert(lib_handle);
        typedef int (*voidfunc)();
        voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
        assert(x);
        assert(x() == 42);
        return 0;
      }
    ''')
    self.btest_exit(
      'main.c',
      cflags=['-sMAIN_MODULE=2', '--preload-file', '.@/', '--use-preload-plugins'] + args)

  # This does not actually verify anything except that --cpuprofiler and --memoryprofiler compiles.
  # Run interactive.test_cpuprofiler_memoryprofiler for interactive testing.
  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'modularized': (['-sMODULARIZE=1', '-sEXPORT_NAME=MyModule', '--shell-file', test_file('shell_that_launches_modularize.html')],),
  })
  def test_cpuprofiler_memoryprofiler(self, opts):
    self.btest_exit('hello_world_gles.c', cflags=['-DLONGTEST=1', '-DTEST_MEMORYPROFILER_ALLOCATIONS_MAP=1', '--cpuprofiler', '--memoryprofiler', '-lGL', '-lglut', '-DANIMATE'] + opts)

  def test_uuid(self):
    self.btest_exit('test_uuid.c', cflags=['-luuid'])

  @requires_graphics_hardware
  def test_glew(self):
    self.btest('glew.c', cflags=['-lGL', '-lSDL', '-lGLEW'], expected='1')
    self.btest('glew.c', cflags=['-lGL', '-lSDL', '-lGLEW', '-sLEGACY_GL_EMULATION'], expected='1')
    self.btest('glew.c', cflags=['-lGL', '-lSDL', '-lGLEW', '-DGLEW_MX'], expected='1')
    self.btest('glew.c', cflags=['-lGL', '-lSDL', '-lGLEW', '-sLEGACY_GL_EMULATION', '-DGLEW_MX'], expected='1')

  def test_doublestart_bug(self):
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', '$addRunDependency,$removeRunDependency')
    create_file('pre.js', r'''
Module["preRun"] = () => {
  addRunDependency('test_run_dependency');
  removeRunDependency('test_run_dependency');
};
''')

    self.btest('doublestart.c', cflags=['--pre-js', 'pre.js'], expected='1')

  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1', '-sHTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0'],),
    'pthread': (['-pthread'],),
    'proxy_to_pthread': (['-pthread', '-sPROXY_TO_PTHREAD'],),
    'legacy': (['-sMIN_FIREFOX_VERSION=0', '-sMIN_SAFARI_VERSION=0', '-sMIN_CHROME_VERSION=0', '-Wno-transpile'],),
  })
  def test_html5_core(self, opts):
    if self.is_wasm64() and '-sMIN_CHROME_VERSION=0' in opts:
      self.skipTest('wasm64 does not support older browsers')
    if '-sHTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0' in opts:
      # In this mode an exception can be thrown by the browser, and we don't
      # want the test to fail in that case so we override the error handling.
      create_file('pre.js', '''
      globalThis.disableErrorReporting = true;
      window.addEventListener('error', (event) => {
        if (!event.message.includes('exception:fullscreen error')) {
          reportTopLevelError(event);
        }
      });
      ''')
      self.cflags.append('--pre-js=pre.js')
    self.btest_exit('test_html5_core.c', cflags=opts)

  def test_html5_remove_event_listener(self):
    self.btest_exit('test_html5_remove_event_listener.c', cflags=[f'-DSAFARI_SUPPORT={int(not self.is_wasm64())}'])

  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1'],),
    'pthread': (['-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  def test_html5_gamepad(self, args):
    self.btest_exit('test_gamepad.c', cflags=args)

  def test_html5_unknown_event_target(self):
    self.btest_exit('test_html5_unknown_event_target.c')

  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1'],),
    'full_es2': (['-sFULL_ES2'],),
  })
  def test_html5_webgl_create_context_no_antialias(self, args):
    self.btest_exit('webgl_create_context.cpp', cflags=args + ['-DNO_ANTIALIAS', '-lGL'])

  # This test supersedes the one above, but it's skipped in the CI because anti-aliasing is not well supported by the Mesa software renderer.
  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1'],),
    'full_es2': (['-sFULL_ES2', '-DFULL_ES2', '-sGL_ASSERTIONS'],),
    'pthread': (['-pthread'],),
  })
  def test_html5_webgl_create_context(self, args):
    self.btest_exit('webgl_create_context.cpp', cflags=args + ['-lGL'])

  @requires_graphics_hardware
  # Verify bug https://github.com/emscripten-core/emscripten/issues/4556: creating a WebGL context to Module.canvas without an ID explicitly assigned to it.
  def test_html5_webgl_create_context2(self):
    self.btest_exit('webgl_create_context2.c')

  @requires_graphics_hardware
  @requires_offscreen_canvas
  # Verify bug https://github.com/emscripten-core/emscripten/issues/22943: creating a WebGL context with explicit swap control and offscreenCanvas
  @parameterized({
    'offscreencanvas': (['-sOFFSCREENCANVAS_SUPPORT'],),
    'offscreenframebuffer': (['-sOFFSCREEN_FRAMEBUFFER', '-DUSE_OFFSCREEN_FRAMEBUFFER'],),
  })
  def test_html5_webgl_create_context_swapcontrol(self, args):
    self.btest_exit('browser/webgl_create_context_swapcontrol.c', cflags=args)

  @requires_graphics_hardware
  # Verify bug https://github.com/emscripten-core/emscripten/issues/4556: creating a WebGL context to Module.canvas without an ID explicitly assigned to it.
  # (this only makes sense in the old deprecated -sDISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 mode)
  def test_html5_special_event_targets(self):
    self.btest_exit('html5_special_event_targets.cpp', cflags=['-lGL'])

  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'O2': (['-O2', '-g1'],),
    'full_es2': (['-sFULL_ES2'],),
  })
  def test_html5_webgl_destroy_context(self, args):
    self.btest_exit('webgl_destroy_context.c', cflags=args + ['--shell-file', test_file('browser/webgl_destroy_context_shell.html'), '-lGL'])

  @requires_graphics_hardware
  def test_webgl_context_params(self):
    self.btest_exit('webgl_color_buffer_readpixels.c', cflags=['-lGL'])

  # Test for PR#5373 (https://github.com/emscripten-core/emscripten/pull/5373)
  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'full_es2': (['-sFULL_ES2'],),
  })
  def test_webgl_shader_source_length(self, args):
    self.btest_exit('webgl_shader_source_length.c', cflags=args + ['-lGL'])

  # Tests calling glGetString(GL_UNMASKED_VENDOR_WEBGL).
  @requires_graphics_hardware
  def test_webgl_unmasked_vendor_webgl(self):
    self.btest_exit('webgl_unmasked_vendor_webgl.c', cflags=['-lGL'])

  @requires_graphics_hardware
  @parameterized({
    'legacy_browser': (['-sMIN_CHROME_VERSION=0', '-Wno-transpile'],),
    'closure': (['-O2', '-g1', '--closure=1'],),
    'full_es2': (['-sFULL_ES2'],),
  })
  def test_webgl2(self, args):
    if '-sMIN_CHROME_VERSION=0' in args and self.is_wasm64():
      self.skipTest('wasm64 not supported by legacy browsers')
    self.btest_exit('webgl2.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'] + args)

  # Tests the WebGL 2 glGetBufferSubData() functionality.
  @requires_webgl2
  @no_4gb('getBufferSubData fails: https://crbug.com/325090165')
  def test_webgl2_get_buffer_sub_data(self):
    self.btest_exit('webgl2_get_buffer_sub_data.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @requires_graphics_hardware
  def test_webgl2_pthreads(self):
    # test that a program can be compiled with pthreads and render WebGL2 properly on the main thread
    # (the testcase doesn't even use threads, but is compiled with thread support).
    self.btest_exit('webgl2.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL', '-pthread'])

  @requires_graphics_hardware
  def test_webgl2_objects(self):
    self.btest_exit('webgl2_objects.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @requires_webgl2
  @requires_offscreen_canvas
  @parameterized({
    '': ([],),
    'offscreencanvas_pthread': (['-sOFFSCREENCANVAS_SUPPORT', '-pthread', '-sPROXY_TO_PTHREAD'],),
    'offscreenframebuffer_pthread': (['-sOFFSCREEN_FRAMEBUFFER', '-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  def test_html5_webgl_api(self, args):
    if '-sOFFSCREENCANVAS_SUPPORT' in args and os.getenv('EMTEST_LACKS_OFFSCREEN_CANVAS'):
      return
    self.btest_exit('html5_webgl.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'] + args)

  @parameterized({
    'webgl1': (['-DWEBGL_VERSION=1'],),
    'webgl2': (['-sMAX_WEBGL_VERSION=2', '-DWEBGL_VERSION=2'],),
    'webgl1_extensions': (['-DWEBGL_VERSION=1', '-sGL_EXPLICIT_UNIFORM_LOCATION'],),
    'webgl2_extensions': (['-sMAX_WEBGL_VERSION=2', '-DWEBGL_VERSION=2', '-sGL_EXPLICIT_UNIFORM_LOCATION', '-sGL_EXPLICIT_UNIFORM_BINDING'],),
  })
  @requires_graphics_hardware
  def test_webgl_preprocessor_variables(self, opts):
    if '-DWEBGL_VERSION=2' in opts and webgl2_disabled():
      self.skipTest('This test requires WebGL2 to be available')
    self.btest_exit('webgl_preprocessor_variables.c', cflags=['-lGL'] + opts)

  @requires_graphics_hardware
  def test_webgl2_ubos(self):
    self.btest_exit('webgl2_ubos.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'webgl2': (['-sMAX_WEBGL_VERSION=2', '-DTEST_WEBGL2=1'],),
  })
  def test_webgl2_garbage_free_entrypoints(self, args):
    if '-DTEST_WEBGL2=1' in args and webgl2_disabled():
      self.skipTest('This test requires WebGL2 to be available')
    if args and self.is_4gb():
      self.skipTest('readPixels fails: https://crbug.com/324992397')
    self.btest_exit('webgl2_garbage_free_entrypoints.c', cflags=args)

  @requires_webgl2
  def test_webgl2_backwards_compatibility_emulation(self):
    self.btest_exit('webgl2_backwards_compatibility_emulation.c', cflags=['-sMAX_WEBGL_VERSION=2', '-sWEBGL2_BACKWARDS_COMPATIBILITY_EMULATION'])

  @requires_graphics_hardware
  def test_webgl2_runtime_no_context(self):
    # tests that if we support WebGL1 and 2, and WebGL2RenderingContext exists,
    # but context creation fails, that we can then manually try to create a
    # WebGL1 context and succeed.
    self.btest_exit('test_webgl2_runtime_no_context.cpp', cflags=['-sMAX_WEBGL_VERSION=2'])

  @requires_graphics_hardware
  def test_webgl_context_major_version(self):
    # testing that majorVersion accepts only valid values
    self.btest('test_webgl_context_major_version.c', expected='abort:Expected Error: Invalid WebGL version requested: 0', cflags=['-lGL', '-DWEBGL_CONTEXT_MAJOR_VERSION=0'])
    self.btest('test_webgl_context_major_version.c', expected='abort:Expected Error: Invalid WebGL version requested: 3', cflags=['-lGL', '-DWEBGL_CONTEXT_MAJOR_VERSION=3'])

    # no linker flag (equivalent to -sMIN_WEBGL_VERSION=1 -sMAX_WEBGL_VERSION=1) => only 1 allowed
    self.btest_exit('test_webgl_context_major_version.c', cflags=['-lGL', '-DWEBGL_CONTEXT_MAJOR_VERSION=1'])
    self.btest('test_webgl_context_major_version.c', expected='abort:Expected Error: WebGL 2 requested but only WebGL 1 is supported (set -sMAX_WEBGL_VERSION=2 to fix the problem)', cflags=['-lGL', '-DWEBGL_CONTEXT_MAJOR_VERSION=2'])

    # -sMIN_WEBGL_VERSION=2 => only 2 allowed
    self.btest('test_webgl_context_major_version.c', expected='abort:Expected Error: WebGL 1 requested but only WebGL 2 is supported (MIN_WEBGL_VERSION is 2)', cflags=['-lGL', '-sMIN_WEBGL_VERSION=2', '-DWEBGL_CONTEXT_MAJOR_VERSION=1'])

  @requires_webgl2
  def test_webgl_context_major_version_webgl2(self):
    self.btest_exit('test_webgl_context_major_version.c', cflags=['-lGL', '-sMIN_WEBGL_VERSION=2', '-DWEBGL_CONTEXT_MAJOR_VERSION=2'])

    # -sMAX_WEBGL_VERSION=2 => 1 and 2 are ok
    self.btest_exit('test_webgl_context_major_version.c', cflags=['-lGL', '-sMAX_WEBGL_VERSION=2', '-DWEBGL_CONTEXT_MAJOR_VERSION=1'])
    self.btest_exit('test_webgl_context_major_version.c', cflags=['-lGL', '-sMAX_WEBGL_VERSION=2', '-DWEBGL_CONTEXT_MAJOR_VERSION=2'])

  @requires_webgl2
  def test_webgl2_invalid_teximage2d_type(self):
    self.btest_exit('webgl2_invalid_teximage2d_type.c', cflags=['-sMAX_WEBGL_VERSION=2'])

  @requires_graphics_hardware
  def test_webgl_with_closure(self):
    self.btest_exit('webgl_with_closure.c', cflags=['-O2', '-sMAX_WEBGL_VERSION=2', '--closure=1', '-lGL'])

  # Tests that -sGL_ASSERTIONS and glVertexAttribPointer with packed types works
  @requires_webgl2
  def test_webgl2_packed_types(self):
    self.btest_exit('webgl2_draw_packed_triangle.c', cflags=['-lGL', '-sMAX_WEBGL_VERSION=2', '-sGL_ASSERTIONS'])

  @requires_graphics_hardware
  @no_4gb('compressedTexSubImage2D fails: https://crbug.com/324562920')
  def test_webgl2_pbo(self):
    self.btest_exit('webgl2_pbo.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @no_firefox('fails on CI likely due to GPU drivers there')
  @no_safari('TODO: Fails with report_result?5') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  @requires_graphics_hardware
  def test_webgl2_sokol_mipmap(self):
    self.reftest('third_party/sokol/mipmap-emsc.c', 'third_party/sokol/mipmap-emsc.png',
                 cflags=['-sMAX_WEBGL_VERSION=2', '-lGL', '-O1'], reference_slack=2)

  @no_firefox('fails on CI likely due to GPU drivers there')
  @no_4gb('fails to render')
  @requires_graphics_hardware
  def test_webgl2_sokol_mrt(self):
    self.reftest('third_party/sokol/mrt-emcc.c', 'third_party/sokol/mrt-emcc.png',
                 cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @requires_webgl2
  @no_4gb('fails to render')
  def test_webgl2_sokol_arraytex(self):
    self.reftest('third_party/sokol/arraytex-emsc.c', 'third_party/sokol/arraytex-emsc.png',
                 cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1'],),
  })
  def test_sdl_touch(self, opts):
    self.btest_exit('test_sdl_touch.c', cflags=opts + ['-DAUTOMATE_SUCCESS=1', '-lSDL', '-lGL'])

  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1'],),
  })
  def test_html5_mouse(self, opts):
    self.btest_exit('test_html5_mouse.c', cflags=opts + ['-DAUTOMATE_SUCCESS=1'])

  @parameterized({
    '': ([],),
    'closure': (['-O2', '-g1', '--closure=1'],),
  })
  def test_sdl_mousewheel(self, opts):
    self.btest_exit('test_sdl_mousewheel.c', cflags=opts + ['-DAUTOMATE_SUCCESS=1', '-lSDL', '-lGL'])

  @also_with_wasm2js
  @parameterized({
    '': ([],),
    'es6': (['-sEXPORT_ES6'],),
  })
  def test_locate_file(self, args):
    self.set_setting('EXIT_RUNTIME')
    create_file('src.c', r'''
      #include <stdio.h>
      #include <string.h>
      #include <assert.h>
      int main() {
        FILE *f = fopen("data.txt", "r");
        assert(f && "could not open file");
        char buf[100];
        int num = fread(buf, 1, 20, f);
        assert(num == 20 && "could not read 20 bytes");
        buf[20] = 0;
        fclose(f);
        printf("|%s|\n", buf);
        assert(strcmp("load me right before", buf) == 0);
        return 0;
      }
    ''')
    create_file('data.txt', 'load me right before...')
    create_file('pre.js', 'Module.locateFile = (x) => "sub/" + x;')
    self.run_process([FILE_PACKAGER, 'test.data', '--preload', 'data.txt'], stdout=open('data.js', 'w'))
    # put pre.js first, then the file packager data, so locateFile is there for the file loading code
    self.compile_btest('src.c', ['-O2', '-g', '--pre-js', 'pre.js', '--pre-js', 'data.js', '-o', 'page.html', '-sFORCE_FILESYSTEM'] + args, reporting=Reporting.JS_ONLY)
    ensure_dir('sub')
    if self.is_wasm():
      shutil.move('page.wasm', Path('sub/page.wasm'))
    shutil.move('test.data', Path('sub/test.data'))
    self.run_browser('page.html', '/report_result?exit:0')

    # alternatively, put locateFile in the HTML
    print('in html')

    create_file('shell.html', '''
      <body>
        <script>
          var Module = {
            locateFile: (x) => "sub/" + x,
          };
        </script>

        {{{ SCRIPT }}}
      </body>
    ''')

    def in_html(expected):
      self.compile_btest('src.c', ['-O2', '-g', '--shell-file', 'shell.html', '--pre-js', 'data.js', '-o', 'page.html', '-sSAFE_HEAP', '-sASSERTIONS', '-sFORCE_FILESYSTEM'] + args, reporting=Reporting.JS_ONLY)
      if self.is_wasm():
        shutil.move('page.wasm', Path('sub/page.wasm'))
      self.run_browser('page.html', '/report_result?exit:' + expected)

    in_html('0')

  @requires_graphics_hardware
  def test_glfw3_default_hints(self):
    self.btest_exit('test_glfw3_default_hints.c', cflags=['-sUSE_GLFW=3', '-lglfw', '-lGL'])

  @requires_graphics_hardware
  @parameterized({
    '': (['-DCLIENT_API=GLFW_OPENGL_ES_API', '-sGL_ENABLE_GET_PROC_ADDRESS'],),
    'no_gl': (['-DCLIENT_API=GLFW_NO_API'],),
  })
  @parameterized({
    '': ([],),
    'legacy': (['-sLEGACY_GL_EMULATION'],),
    'closure': (['-Os', '--closure=1'],),
  })
  def test_glfw3(self, args, opts):
    self.btest_exit('test_glfw3.c', cflags=['-sUSE_GLFW=3', '-lglfw', '-lGL'] + args + opts)

  @parameterized({
    '': (['-sUSE_GLFW=2', '-DUSE_GLFW=2'],),
    'glfw3': (['-sUSE_GLFW=3', '-DUSE_GLFW=3'],),
  })
  @requires_graphics_hardware
  def test_glfw_events(self, args):
    self.btest_exit('test_glfw_events.c', cflags=args + ['-lglfw', '-lGL', '--pre-js', test_file('browser/fake_events.js')])

  @requires_graphics_hardware
  def test_glfw3_hi_dpi_aware(self):
    self.btest_exit('test_glfw3_hi_dpi_aware.c', cflags=['-sUSE_GLFW=3', '-lGL'])

  @requires_graphics_hardware
  def test_glfw3_css_scaling(self):
    self.btest_exit('test_glfw3_css_scaling.c', cflags=['-sUSE_GLFW=3'])

  @requires_graphics_hardware
  @also_with_wasm2js
  def test_sdl2_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file
    shutil.copy(test_file('screenshot.jpg'), '.')

    for dest, dirname, basename in [('screenshot.jpg', '/', 'screenshot.jpg'),
                                    ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
      self.btest_exit('test_sdl2_image.c', 600, cflags=[
        '-O2',
        '--preload-file', dest,
        '-DSCREENSHOT_DIRNAME="' + dirname + '"',
        '-DSCREENSHOT_BASENAME="' + basename + '"',
        '-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '--use-preload-plugins',
      ])

  @requires_graphics_hardware
  def test_sdl2_image_jpeg(self):
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.jpeg')
    self.btest_exit('test_sdl2_image.c', 600, cflags=[
      '--preload-file', 'screenshot.jpeg',
      '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"',
      '-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '--use-preload-plugins',
    ])

  @also_with_wasmfs
  @requires_graphics_hardware
  @with_all_sjlj
  @requires_safari_version(170601, 'TODO: Test enables Wasm exceptions') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), passes in Safari 18.5 (20621.2.5.11.8) and Safari 26.0.1 (21622.1.22.11.15)
  def test_sdl2_image_formats(self):
    shutil.copy(test_file('screenshot.png'), '.')
    shutil.copy(test_file('screenshot.jpg'), '.')
    self.btest_exit('test_sdl2_image.c', 512, cflags=[
      '--preload-file', 'screenshot.png',
      '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.png"', '-DNO_PRELOADED',
      '-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '-sSDL2_IMAGE_FORMATS=png',
    ])
    self.btest_exit('test_sdl2_image.c', 600, cflags=[
      '--preload-file', 'screenshot.jpg',
      '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpg"', '-DBITSPERPIXEL=24', '-DNO_PRELOADED',
      '--use-port=sdl2', '--use-port=sdl2_image:formats=jpg',
    ])

  def test_sdl2_key(self):
    self.btest_exit('test_sdl2_key.c', 37182145, cflags=['-sUSE_SDL=2', '--pre-js', test_file('browser/fake_events.js')])

  def test_sdl2_text(self):
    create_file('pre.js', '''
      Module.postRun = () => {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }
    ''')

    self.btest_exit('test_sdl2_text.c', cflags=['--pre-js', 'pre.js', '--pre-js', test_file('browser/fake_events.js'), '-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_mouse(self):
    self.btest_exit('test_sdl2_mouse.c', cflags=['-O2', '--minify=0', '--pre-js', test_file('browser/fake_events.js'), '-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_mouse_offsets(self):
    create_file('page.html', '''
      <html>
        <head>
          <style type="text/css">
            html, body { margin: 0; padding: 0; }
            #container {
              position: absolute;
              left: 5px; right: 0;
              top: 5px; bottom: 0;
            }
            #canvas {
              position: absolute;
              left: 0; width: 600px;
              top: 0; height: 450px;
            }
            textarea {
              margin-top: 500px;
              margin-left: 5px;
              width: 600px;
            }
          </style>
        </head>
        <body>
          <div id="container">
            <canvas id="canvas"></canvas>
          </div>
          <textarea id="output" rows="8"></textarea>
          <script type="text/javascript">
            var Module = {
              canvas: document.getElementById('canvas'),
              print: (function() {
                var element = document.getElementById('output');
                element.value = ''; // clear browser cache
                return function(text) {
                  if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
                  element.value += text + "\\n";
                  element.scrollTop = element.scrollHeight; // focus on bottom
                };
              })()
            };
          </script>
          <script type="text/javascript" src="sdl2_mouse.js"></script>
        </body>
      </html>
    ''')

    self.compile_btest('browser/test_sdl2_mouse.c', ['-DTEST_SDL_MOUSE_OFFSETS', '-O2', '--minify=0', '-o', 'sdl2_mouse.js', '--pre-js', test_file('browser/fake_events.js'), '-sUSE_SDL=2', '-sEXIT_RUNTIME'])
    self.run_browser('page.html', '', '/report_result?exit:0')

  def test_sdl2_threads(self):
    self.btest_exit('test_sdl2_threads.c', cflags=['-pthread', '-sUSE_SDL=2', '-sPROXY_TO_PTHREAD'])

  @requires_graphics_hardware
  def test_sdl2_glshader(self):
    self.cflags += ['--closure=1', '-g1']
    self.reftest('test_sdl2_glshader.c', 'test_sdl_glshader.png', cflags=['-sUSE_SDL=2', '-sLEGACY_GL_EMULATION'])

  @requires_graphics_hardware
  def test_sdl2_canvas_blank(self):
    self.reftest('test_sdl2_canvas_blank.c', 'test_sdl_canvas_blank.png', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_canvas_palette(self):
    self.reftest('test_sdl2_canvas_palette.c', 'test_sdl_canvas_palette.png', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_canvas_twice(self):
    self.reftest('test_sdl2_canvas_twice.c', 'test_sdl_canvas_twice.png', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_gfx(self):
    self.reftest('test_sdl2_gfx.c', 'test_sdl2_gfx.png', cflags=['-sUSE_SDL=2', '-sUSE_SDL_GFX=2'], reference_slack=2)

  @requires_graphics_hardware
  def test_sdl2_canvas_palette_2(self):
    create_file('args-r.js', '''
      Module['arguments'] = ['-r'];
    ''')

    create_file('args-g.js', '''
      Module['arguments'] = ['-g'];
    ''')

    create_file('args-b.js', '''
      Module['arguments'] = ['-b'];
    ''')

    self.reftest('test_sdl2_canvas_palette_2.c', 'test_sdl_canvas_palette_r.png', cflags=['-sUSE_SDL=2', '--pre-js', 'args-r.js'])
    self.reftest('test_sdl2_canvas_palette_2.c', 'test_sdl_canvas_palette_g.png', cflags=['-sUSE_SDL=2', '--pre-js', 'args-g.js'])
    self.reftest('test_sdl2_canvas_palette_2.c', 'test_sdl_canvas_palette_b.png', cflags=['-sUSE_SDL=2', '--pre-js', 'args-b.js'])

  def test_sdl2_swsurface(self):
    self.btest_exit('test_sdl2_swsurface.c', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.reftest('test_sdl2_image_prepare.c', 'screenshot.jpg', cflags=['--preload-file', 'screenshot.not', '-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2'])

  @requires_graphics_hardware
  def test_sdl2_image_prepare_data(self):
    # load an image file, get pixel data.
    shutil.copy(test_file('screenshot.jpg'), 'screenshot.not')
    self.reftest('test_sdl2_image_prepare_data.c', 'screenshot.jpg', cflags=['--preload-file', 'screenshot.not', '-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2'])

  def test_sdl2_pumpevents(self):
    # key events should be detected using SDL_PumpEvents
    self.btest_exit('test_sdl2_pumpevents.c', cflags=['--pre-js', test_file('browser/fake_events.js'), '-sUSE_SDL=2'])

  def test_sdl2_timer(self):
    self.btest_exit('test_sdl2_timer.c', cflags=['-sUSE_SDL=2'])

  def test_sdl2_canvas_size(self):
    self.btest_exit('test_sdl2_canvas_size.c', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_gl_read(self):
    # SDL, OpenGL, readPixels
    self.btest_exit('test_sdl2_gl_read.c', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_glmatrixmode_texture(self):
    self.reftest('test_sdl2_glmatrixmode_texture.c', 'test_sdl2_glmatrixmode_texture.png',
                 cflags=['-sLEGACY_GL_EMULATION', '-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_gldrawelements(self):
    self.reftest('test_sdl2_gldrawelements.c', 'test_sdl2_gldrawelements.png',
                 cflags=['-sLEGACY_GL_EMULATION', '-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_glclipplane_gllighting(self):
    self.reftest('test_sdl2_glclipplane_gllighting.c', 'test_sdl2_glclipplane_gllighting.png',
                 cflags=['-sLEGACY_GL_EMULATION', '-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_glalphatest(self):
    self.reftest('test_sdl2_glalphatest.c', 'test_sdl2_glalphatest.png',
                 cflags=['-sLEGACY_GL_EMULATION', '-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_fog_simple(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl2_fog_simple.c', 'screenshot-fog-simple.png',
                 cflags=['-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '-O2', '--minify=0', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_sdl2_fog_negative(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl2_fog_negative.c', 'screenshot-fog-negative.png',
                 cflags=['-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_sdl2_fog_density(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl2_fog_density.c', 'screenshot-fog-density.png',
                 cflags=['-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_sdl2_fog_exp2(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl2_fog_exp2.c', 'screenshot-fog-exp2.png',
                 cflags=['-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_sdl2_fog_linear(self):
    shutil.copy(test_file('screenshot.png'), '.')
    self.reftest('test_sdl2_fog_linear.c', 'screenshot-fog-linear.png', reference_slack=1,
                 cflags=['-sUSE_SDL=2', '-sUSE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-sLEGACY_GL_EMULATION', '--use-preload-plugins'])

  def test_sdl2_unwasteful(self):
    self.btest_exit('test_sdl2_unwasteful.c', cflags=['-sUSE_SDL=2', '-O1'])

  def test_sdl2_canvas_write(self):
    self.btest_exit('test_sdl2_canvas_write.c', cflags=['-sUSE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_ttf(self):
    shutil.copy2(test_file('freetype/LiberationSansBold.ttf'), self.get_dir())
    self.reftest('test_sdl2_ttf.c', 'test_sdl2_ttf.png',
                 cflags=['-O2', '-sUSE_SDL=2', '-sUSE_SDL_TTF=2', '--embed-file', 'LiberationSansBold.ttf'])

  @requires_graphics_hardware
  def test_sdl2_ttf_rtl(self):
    shutil.copy2(test_file('third_party/notofont/NotoNaskhArabic-Regular.ttf'), self.get_dir())
    self.reftest('test_sdl2_ttf_rtl.c', 'test_sdl2_ttf_rtl.png',
                 cflags=['-O2', '-sUSE_SDL=2', '-sUSE_SDL_TTF=2', '--embed-file', 'NotoNaskhArabic-Regular.ttf'])

  def test_sdl2_custom_cursor(self):
    shutil.copy(test_file('cursor.bmp'), '.')
    self.btest_exit('test_sdl2_custom_cursor.c', cflags=['--preload-file', 'cursor.bmp', '-sUSE_SDL=2'])

  def test_sdl2_misc(self):
    self.btest_exit('test_sdl2_misc.c', cflags=['-sUSE_SDL=2'])

  def test_sdl2_misc_main_module(self):
    self.btest_exit('test_sdl2_misc.c', cflags=['-sUSE_SDL=2', '-sMAIN_MODULE'])

  def test_sdl2_misc_via_object(self):
    self.emcc('browser/test_sdl2_misc.c', ['-c', '-sUSE_SDL=2', '-o', 'test.o'])
    self.compile_btest('test.o', ['-sEXIT_RUNTIME', '-sUSE_SDL=2', '-o', 'test.html'])
    self.run_browser('test.html', '/report_result?exit:0')

  @parameterized({
    '': (['-sUSE_SDL=2', '-sUSE_SDL_MIXER=2'],),
    'dash_l': (['-lSDL2', '-lSDL2_mixer'],),
  })
  @requires_sound_hardware
  def test_sdl2_mixer_wav(self, flags):
    shutil.copy(test_file('sounds/the_entertainer.wav'), 'sound.wav')
    self.btest_exit('test_sdl2_mixer_wav.c', cflags=['--preload-file', 'sound.wav'] + flags)

  @parameterized({
    'wav': ([],         '0',            'the_entertainer.wav'),
    'ogg': (['ogg'],    'MIX_INIT_OGG', 'alarmvictory_1.ogg'),
    'mp3': (['mp3'],    'MIX_INIT_MP3', 'pudinha.mp3'),
    'mod': (['mod'],    'MIX_INIT_MOD', 'bleep.xm'),
    # TODO: need to source freepats.cfg and a midi file
    # 'mod': (['mid'],    'MIX_INIT_MID', 'midi.mid'),
  })
  @requires_sound_hardware
  def test_sdl2_mixer_music(self, formats, flags, music_name):
    shutil.copy(test_file('sounds', music_name), '.')
    args = [
      '--preload-file', music_name,
      '-DSOUND_PATH="%s"' % music_name,
      '-DFLAGS=' + flags,
      '-sUSE_SDL=2',
      '-sUSE_SDL_MIXER=2',
      '-sSDL2_MIXER_FORMATS=' + ','.join(formats),
    ]
    # libmodplug is written in C++ so we need to link in C++
    # libraries when using it.
    if 'mod' in formats:
      args += ['-lc++', '-lc++abi']
    self.btest_exit('test_sdl2_mixer_music.c', cflags=args)

  def test_sdl3_misc(self):
    self.cflags.append('-Wno-experimental')
    self.btest_exit('test_sdl3_misc.c', cflags=['-sUSE_SDL=3'])

  def test_sdl3_canvas_write(self):
    self.cflags.append('-Wno-experimental')
    self.btest_exit('test_sdl3_canvas_write.c', cflags=['-sUSE_SDL=3'])

  @requires_graphics_hardware
  @no_wasm64('cocos2d ports does not compile with wasm64')
  def test_cocos2d_hello(self):
    # cocos2d build contains a bunch of warnings about tiff symbols being missing at link time:
    # e.g. warning: undefined symbol: TIFFClientOpen
    cocos2d_root = os.path.join(ports.Ports.get_dir(), 'cocos2d', 'Cocos2d-version_3_3')
    preload_file = os.path.join(cocos2d_root, 'samples', 'Cpp', 'HelloCpp', 'Resources') + '@'
    self.reftest('cocos2d_hello.cpp', 'cocos2d_hello.png', reference_slack=1,
                 cflags=['-sUSE_COCOS2D=3', '-sERROR_ON_UNDEFINED_SYMBOLS=0',
                            # This line should really just be `-std=c++14` like we use to compile
                            # the cocos library itself, but that doesn't work in this case because
                            # btest adds browser_reporting.c to the command.
                            '-D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION',
                            '-Wno-js-compiler',
                            '-Wno-experimental',
                            '--preload-file', preload_file, '--use-preload-plugins',
                            '-Wno-inconsistent-missing-override',
                            '-Wno-deprecated-declarations'])

  @parameterized({
    'O0': ('-O0',),
    'O1': ('-O1',),
    'O2': ('-O2',),
    'O3': ('-O3',),
  })
  @parameterized({
    'asyncify': (['-sASYNCIFY=1'],),
    'asyncify_minimal_runtime': (['-sMINIMAL_RUNTIME', '-sASYNCIFY=1'],),
    'jspi': (['-sASYNCIFY=2', '-Wno-experimental'],),
    'jspi_wasm_bigint': (['-sASYNCIFY=2', '-sWASM_BIGINT', '-Wno-experimental'],),
    'jspi_wasm_bigint_minimal_runtime': (['-sMINIMAL_RUNTIME', '-sASYNCIFY=2', '-sWASM_BIGINT', '-Wno-experimental'],),
  })
  def test_async(self, opt, args):
    if is_jspi(args) and not is_chrome():
      self.skipTest(f'Current browser ({get_browser()}) does not support JSPI. Only chromium-based browsers ({CHROMIUM_BASED_BROWSERS}) support JSPI today.')

    self.btest_exit('test_async.c', cflags=[opt, '-g2'] + args)

  def test_asyncify_tricky_function_sig(self):
    self.btest('test_asyncify_tricky_function_sig.cpp', '85', cflags=['-sASYNCIFY_ONLY=[foo(char.const*?.int#),foo2(),main,__original_main]', '-sASYNCIFY'])

  def test_async_in_pthread(self):
    self.btest_exit('test_async.c', cflags=['-sASYNCIFY', '-pthread', '-sPROXY_TO_PTHREAD', '-g'])

  def test_async_2(self):
    # Error.stackTraceLimit default to 10 in chrome but this test relies on more
    # than 40 stack frames being reported.
    create_file('pre.js', 'Error.stackTraceLimit = 80;\n')
    self.btest_exit('test_async_2.c', cflags=['-O3', '--pre-js', 'pre.js', '-sASYNCIFY', '-sSTACK_SIZE=1MB'])

  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  def test_async_virtual(self, args):
    self.btest_exit('async_virtual.cpp', cflags=args + ['-profiling', '-sASYNCIFY'])

  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  def test_async_virtual_2(self, args):
    self.btest_exit('async_virtual_2.cpp', cflags=args + ['-sASSERTIONS', '-sSAFE_HEAP', '-profiling', '-sASYNCIFY'])

  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  def test_async_mainloop(self, args):
    self.btest_exit('test_async_mainloop.c', cflags=args + ['-sASYNCIFY'])

  @requires_sound_hardware
  @parameterized({
    '': ([],),
    'safeheap': (['-sSAFE_HEAP'],),
  })
  def test_sdl_audio_beep_sleep(self, args):
    self.btest_exit('test_sdl_audio_beep_sleep.cpp', cflags=['-Os', '-sASSERTIONS', '-sDISABLE_EXCEPTION_CATCHING=0', '-profiling', '-lSDL', '-sASYNCIFY'] + args, timeout=90)

  def test_mainloop_reschedule(self):
    self.btest('test_mainloop_reschedule.c', '1', cflags=['-Os', '-sASYNCIFY'])

  def test_mainloop_infloop(self):
    self.btest('test_mainloop_infloop.c', '1', cflags=['-sASYNCIFY'])

  def test_async_iostream(self):
    self.btest('async_iostream.cpp', '1', cflags=['-sASYNCIFY'])

  # Test an async return value. The value goes through a custom JS library
  # method that uses asyncify, and therefore it needs to be declared in
  # ASYNCIFY_IMPORTS.
  # To make the test more precise we also use ASYNCIFY_IGNORE_INDIRECT here.
  @parameterized({
    '': (['-sASYNCIFY_IMPORTS=sync_tunnel,sync_tunnel_bool'],), # noqa
    'pattern_imports': (['-sASYNCIFY_IMPORTS=[sync_tun*]'],), # noqa
    'response': (['-sASYNCIFY_IMPORTS=@filey.txt'],), # noqa
    'nothing': (['-DBAD'],), # noqa
    'empty_list': (['-DBAD', '-sASYNCIFY_IMPORTS=[]'],), # noqa
    'em_js_bad': (['-DBAD', '-DUSE_EM_JS'],), # noqa
  })
  def test_async_returnvalue(self, args):
    if '@' in str(args):
      create_file('filey.txt', 'sync_tunnel\nsync_tunnel_bool\n')
    self.btest('test_async_returnvalue.c', '0', cflags=['-sASSERTIONS', '-sASYNCIFY', '-sASYNCIFY_IGNORE_INDIRECT', '--js-library', test_file('browser/test_async_returnvalue.js')] + args)

  @no_safari('TODO: Never reports a result, so times out') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_async_bad_list(self):
    self.btest('test_async_bad_list.c', '0', cflags=['-sASYNCIFY', '-sASYNCIFY_ONLY=waka', '--profiling'])

  # Tests that when building with -sMINIMAL_RUNTIME, the build can use -sMODULARIZE as well.
  def test_minimal_runtime_modularize(self):
    self.btest_exit('browser_test_hello_world.c', cflags=['-sMODULARIZE', '-sMINIMAL_RUNTIME'])

  # Tests that when building with -sMINIMAL_RUNTIME, the build can use -sEXPORT_NAME=Foo as well.
  def test_minimal_runtime_export_name(self):
    self.btest_exit('browser_test_hello_world.c', cflags=['-sEXPORT_NAME=Foo', '-sMINIMAL_RUNTIME'])

  @parameterized({
    # defaults
    '': ([], '''
       let promise = Module();
       if (!promise instanceof Promise) throw new Error('Return value should be a promise');
    '''),
    # use EXPORT_NAME
    'export_name': (['-sEXPORT_NAME="HelloWorld"'], '''
       if (typeof Module !== "undefined") throw "what?!"; // do not pollute the global scope, we are modularized!
       HelloWorld.noInitialRun = true; // errorneous module capture will load this and cause timeout
       let promise = HelloWorld();
       if (!promise instanceof Promise) throw new Error('Return value should be a promise');
    '''),
    # pass in a Module option (which prevents main(), which we then invoke ourselves)
    'no_main': (['-sEXPORT_NAME="HelloWorld"'], '''
       HelloWorld({ noInitialRun: true }).then(hello => {
         hello._main();
       });
    '''),
  })
  @parameterized({
    '': ([],),
    'O1': (['-O1'],),
    'O2': (['-O2'],),
    'profiling': (['-O2', '-profiling'],),
    'closure': (['-O2', '--closure=1'],),
  })
  def test_modularize(self, args, code, opts):
    # this test is synchronous, so avoid async startup due to wasm features
    self.compile_btest('browser_test_hello_world.c', ['-sMODULARIZE', '-sSINGLE_FILE'] + args + opts)
    create_file('a.html', '''
      <!DOCTYPE html><html lang="en"><head><meta charset="utf-8"></head><body>
      <script src="a.out.js"></script>
      <script>
        %s
      </script>
      </body></html>
    ''' % code)
    self.run_browser('a.html', '/report_result?0')

  @no_firefox('source phase imports not implemented yet in firefox')
  @no_safari('TODO: croaks on line "import source wasmModule from \'./out.wasm\';"') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_source_phase_imports(self):
    self.compile_btest('browser_test_hello_world.c', ['-sEXPORT_ES6', '-sSOURCE_PHASE_IMPORTS', '-Wno-experimental', '-o', 'out.mjs'])
    create_file('a.html', '''
      <script type="module">
        import Module from "./out.mjs"
        const mod = await Module();
      </script>
    ''')
    self.run_browser('a.html', '/report_result?0')

  def test_modularize_network_error(self):
    self.compile_btest('browser_test_hello_world.c', ['-sMODULARIZE', '-sEXPORT_NAME=createModule'], reporting=Reporting.NONE)
    self.add_browser_reporting()
    create_file('a.html', '''
      <script src="browser_reporting.js"></script>
      <script src="a.out.js"></script>
      <script>
        createModule()
          .then(() => {
            reportResultToServer("Module creation succeeded when it should have failed");
          })
          .catch(err => {
            reportResultToServer(err.message);
          });
      </script>
    ''')
    print('Deleting a.out.wasm to cause a download error')
    os.remove('a.out.wasm')
    self.run_browser('a.html', '/report_result?Aborted(both async and sync fetching of the wasm failed)')

  def test_modularize_init_error(self):
    self.compile_btest('browser/test_modularize_init_error.cpp', ['-sMODULARIZE', '-sEXPORT_NAME=createModule'], reporting=Reporting.NONE)
    self.add_browser_reporting()
    create_file('a.html', '''
      <script src="browser_reporting.js"></script>
      <script src="a.out.js"></script>
      <script>
        createModule()
          .then(() => {
            reportResultToServer("Module creation succeeded when it should have failed");
          })
          .catch(err => {
            reportResultToServer(err);
          });
      </script>
    ''')
    self.run_browser('a.html', '/report_result?intentional error to test rejection')

  # test illustrating the regression on the modularize feature since commit c5af8f6
  # when compiling with the --preload-file option
  @requires_wasm2js
  @also_with_wasmfs
  @parameterized({
    '': ([],),
    'O1': (['-O1'],),
    'O2_profiling': (['-O2', '--profiling'],),
    'O2_closure': (['-O2', '--closure=1'],),
  })
  def test_modularize_and_preload_files(self, args):
    self.set_setting('EXIT_RUNTIME')
    # TODO(sbc): Fix closure warnings with MODULARIZE + WASM=0
    self.ldflags.append('-Wno-error=closure')
    # amount of memory different from the default one that will be allocated for the emscripten heap
    totalMemory = 33554432

    # the main function simply checks that the amount of allocated heap memory is correct
    create_file('test.c', r'''
      #include <stdio.h>
      #include <emscripten.h>
      int main() {
        EM_ASM({
          // use eval here in order for the test with closure compiler enabled to succeed
          var totalMemory = Module['INITIAL_MEMORY'];
          assert(totalMemory === %d, 'bad memory size');
        });
        return 0;
      }
    ''' % totalMemory)
    # generate a dummy file
    create_file('dummy_file', 'dummy')
    # compile the code with the modularize feature and the preload-file option enabled
    # no wasm, since this tests customizing total memory at runtime
    self.compile_btest('test.c', ['-sWASM=0', '-sIMPORTED_MEMORY', '-sMODULARIZE', '-sEXPORT_NAME=Foo', '--preload-file', 'dummy_file'] + args, reporting=Reporting.JS_ONLY)
    create_file('a.html', '''
      <script src="a.out.js"></script>
      <script>
        // instantiate the Foo module with custom INITIAL_MEMORY value
        var foo = Foo({ INITIAL_MEMORY: %d });
      </script>
    ''' % totalMemory)
    self.run_browser('a.html', '/report_result?exit:0')

  @parameterized({
    '': ([],),
    'O1': (['-O1'],),
    'O2': (['-O2'],),
  })
  def test_webidl(self, args):
    # see original in test_core.py
    self.run_process([WEBIDL_BINDER, test_file('webidl/test.idl'), 'glue'])
    self.assertExists('glue.cpp')
    self.assertExists('glue.js')
    self.btest('webidl/test.cpp', '1', cflags=['--post-js', 'glue.js', '-I.', '-DBROWSER'] + args)

  @no_wasm64('https://github.com/llvm/llvm-project/issues/98778')
  def test_dylink(self):
    create_file('main.c', r'''
      #include <assert.h>
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      char *side(const char *data);
      int main() {
        char *ret = side("hello through side\n");
        puts(ret);
        assert(strcmp(ret, "hello through side\n") == 0);
        return 0;
      }
    ''')
    create_file('side.c', r'''
      #include <string.h>
      char *side(const char *data) {
        return strdup(data);
      }
    ''')
    self.emcc('side.c', ['-sSIDE_MODULE', '-O2', '-o', 'side.wasm'])
    self.btest_exit('main.c', cflags=['-sMAIN_MODULE=2', '-O2', 'side.wasm'])

  def test_dlopen_async(self):
    create_file('side.c', 'int foo = 42;\n')
    self.emcc('side.c', ['-o', 'libside.so', '-sSIDE_MODULE'])
    self.btest_exit('other/test_dlopen_async.c', cflags=['-sMAIN_MODULE=2'])

  @requires_shared_array_buffer
  def test_dlopen_blocking(self):
    self.emcc('other/test_dlopen_blocking_side.c', ['-o', 'libside.so', '-sSIDE_MODULE', '-pthread', '-Wno-experimental'])
    # Attempt to use dlopen the side module (without preloading) should fail on the main thread
    # since the syncronous `readBinary` function does not exist.
    self.btest_exit('other/test_dlopen_blocking.c', assert_returncode=1, cflags=['-sMAIN_MODULE=2', '-sAUTOLOAD_DYLIBS=0', 'libside.so'])
    # But with PROXY_TO_PTHEAD it does work, since we can do blocking and sync XHR in a worker.
    self.btest_exit('other/test_dlopen_blocking.c', cflags=['-sMAIN_MODULE=2', '-sPROXY_TO_PTHREAD', '-pthread', '-Wno-experimental', '-sAUTOLOAD_DYLIBS=0', 'libside.so'])

  # verify that dynamic linking works in all kinds of in-browser environments.
  # don't mix different kinds in a single test.
  @parameterized({
    '': (False,),
    'inworker': (True,),
  })
  def test_dylink_dso_needed(self, inworker):
    if not inworker:
      self.skipTest('https://github.com/emscripten-core/emscripten/issues/25814')

    def do_run(src, expected_output, cflags):
      # XXX there is no infrastructure (yet ?) to retrieve stdout from browser in tests.
      # -> do the assert about expected output inside browser.
      #
      # we have to put the hook into post.js because in main it is too late
      # (in main we won't be able to catch what static constructors inside
      # linked dynlibs printed), and in pre.js it is too early (out is not yet
      # setup by the shell).
      create_file('post.js', r'''
          Module.realPrint = out;
          out = (x) => {
            if (!Module.printed) Module.printed = "";
            Module.printed += x + '\n'; // out is passed str without last \n
            Module.realPrint(x);
          };
        ''')
      create_file('test_dylink_dso_needed.c', src + r'''
        #include <emscripten/em_asm.h>

        int main() {
          int rtn = test_main();
          EM_ASM({
            var expected = %r;
            assert(Module.printed === expected, ['stdout expected:', expected]);
          });
          return rtn;
        }
      ''' % expected_output)
      self.btest_exit('test_dylink_dso_needed.c', cflags=['--post-js', 'post.js'] + cflags, run_in_worker=inworker)

    self._test_dylink_dso_needed(do_run)

  @requires_graphics_hardware
  @no_wasm64('https://github.com/llvm/llvm-project/issues/98778')
  def test_dylink_glemu(self):
    create_file('main.c', r'''
      #include <stdio.h>
      #include <string.h>
      #include <assert.h>
      const char *side();
      int main() {
        const char *exts = side();
        puts(side());
        assert(strstr(exts, "GL_EXT_texture_env_combine"));
        return 0;
      }
    ''')
    create_file('side.c', r'''
      #include "SDL/SDL.h"
      #include "SDL/SDL_opengl.h"
      const char *side() {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_SetVideoMode(600, 600, 16, SDL_OPENGL);
        return (const char *)glGetString(GL_EXTENSIONS);
      }
    ''')
    self.emcc('side.c', ['-sSIDE_MODULE', '-O2', '-o', 'side.wasm', '-lSDL'])

    self.btest_exit('main.c', cflags=['-sMAIN_MODULE=2', '-O2', '-sLEGACY_GL_EMULATION', '-lSDL', '-lGL', 'side.wasm'])

  def test_dylink_many(self):
    # test asynchronously loading two side modules during startup
    create_file('main.c', r'''
      #include <assert.h>
      int side1();
      int side2();
      int main() {
        assert(side1() == 1);
        assert(side2() == 2);
        return 0;
      }
    ''')
    create_file('side1.c', r'''
      int side1() { return 1; }
    ''')
    create_file('side2.c', r'''
      int side2() { return 2; }
    ''')
    self.emcc('side1.c', ['-sSIDE_MODULE', '-o', 'side1.wasm'])
    self.emcc('side2.c', ['-sSIDE_MODULE', '-o', 'side2.wasm'])
    self.btest_exit('main.c', cflags=['-sMAIN_MODULE=2', 'side1.wasm', 'side2.wasm'])

  def test_dylink_pthread_many(self):
    # Test asynchronously loading two side modules during startup
    # They should always load in the same order
    # Verify that function pointers in the browser's main thread
    # reffer to the same function as in a pthread worker.

    # The main thread function table is populated asynchronously
    # in the browser's main thread. However, it should still be
    # populated in the same order as in a pthread worker to
    # guarantee function pointer interop.
    create_file('main.cpp', r'''
      #include <cassert>
      #include <thread>
      #include <emscripten/emscripten.h>
      int side1();
      int side2();
      int main() {
        auto side1_ptr = &side1;
        auto side2_ptr = &side2;
        // Don't join the thread since this is running in the
        // browser's main thread.
        std::thread([=]{
          assert(side1_ptr == &side1);
          assert(side2_ptr == &side2);
          emscripten_force_exit(0);
        }).detach();
        emscripten_exit_with_live_runtime();
      }
    ''')

    # The browser will try to load side1 first.
    # Use a big payload in side1 so that it takes longer to load than side2
    create_file('side1.cpp', r'''
      char const * payload1 = "''' + str(list(range(1, int(1e5)))) + r'''";
      int side1() { return 1; }
    ''')
    create_file('side2.cpp', r'''
      char const * payload2 = "0";
      int side2() { return 2; }
    ''')
    self.emcc('side1.cpp', ['-Wno-experimental', '-pthread', '-sSIDE_MODULE', '-o', 'side1.wasm'])
    self.emcc('side2.cpp', ['-Wno-experimental', '-pthread', '-sSIDE_MODULE', '-o', 'side2.wasm'])
    self.btest_exit('main.cpp',
                    cflags=['-Wno-experimental', '-pthread', '-sMAIN_MODULE=2', 'side1.wasm', 'side2.wasm'])

  @no_2gb('uses INITIAL_MEMORY')
  @no_4gb('uses INITIAL_MEMORY')
  def test_memory_growth_during_startup(self):
    create_file('data.dat', 'X' * (30 * 1024 * 1024))
    self.btest_exit('browser_test_hello_world.c', cflags=['-sASSERTIONS', '-sALLOW_MEMORY_GROWTH', '-sINITIAL_MEMORY=16MB', '-sSTACK_SIZE=16384', '--preload-file', 'data.dat'])

  # pthreads tests

  def prep_no_SAB(self):
    create_file('html.html', read_file(path_from_root('src/shell_minimal.html')).replace('''<body>''', '''<body>
      <script>
        SharedArrayBuffer = undefined;
        Atomics = undefined;
      </script>
    '''))

  def test_pthread_c11_threads(self):
    self.btest_exit('pthread/test_pthread_c11_threads.c', cflags=['-gsource-map', '-pthread', '-sPROXY_TO_PTHREAD'])

  def test_pthread_pool_size_strict(self):
    # Check that it doesn't fail with sufficient number of threads in the pool.
    self.btest_exit('pthread/test_pthread_c11_threads.c', cflags=['-g2', '-pthread', '-sPTHREAD_POOL_SIZE=4', '-sPTHREAD_POOL_SIZE_STRICT=2'])
    # Check that it fails instead of deadlocking on insufficient number of threads in the pool.
    self.btest('pthread/test_pthread_c11_threads.c',
               expected='abort:Assertion failed: thrd_create(&t4, thread_main, NULL) == thrd_success',
               cflags=['-g2', '-pthread', '-sPTHREAD_POOL_SIZE=3', '-sPTHREAD_POOL_SIZE_STRICT=2'])

  def test_pthread_in_pthread_pool_size_strict(self):
    # Check that it fails when there's a pthread creating another pthread.
    self.btest_exit('pthread/test_pthread_create_pthread.c', cflags=['-g2', '-pthread', '-sPTHREAD_POOL_SIZE=2', '-sPTHREAD_POOL_SIZE_STRICT=2'])
    # Check that it fails when there's a pthread creating another pthread.
    self.btest_exit('pthread/test_pthread_create_pthread.c', cflags=['-g2', '-pthread', '-sPTHREAD_POOL_SIZE=1', '-sPTHREAD_POOL_SIZE_STRICT=2', '-DSMALL_POOL'])

  # Test that the emscripten_ atomics api functions work.
  @parameterized({
    '': ([],),
    'closure': (['--closure=1'],),
  })
  def test_pthread_atomics(self, args):
    self.btest_exit('pthread/test_pthread_atomics.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8', '-g1'] + args)

  # Test 64-bit atomics.
  def test_pthread_64bit_atomics(self):
    self.btest_exit('pthread/test_pthread_64bit_atomics.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test 64-bit C++11 atomics.
  @also_with_threads
  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  def test_pthread_64bit_cxx11_atomics(self, opt):
    self.btest_exit('pthread/test_pthread_64bit_cxx11_atomics.cpp', cflags=opt)

  # Test c++ std::thread::hardware_concurrency()
  def test_pthread_hardware_concurrency(self):
    self.btest_exit('pthread/test_pthread_hardware_concurrency.cpp', cflags=['-O2', '-pthread', '-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency'])

  # Test that we error if not ALLOW_BLOCKING_ON_MAIN_THREAD
  def test_pthread_main_thread_blocking_wait(self):
    self.btest('pthread/main_thread_wait.c', expected='abort:Blocking on the main thread is not allowed by default.', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE', '-sALLOW_BLOCKING_ON_MAIN_THREAD=0'])

  # Test that we error or warn depending on ALLOW_BLOCKING_ON_MAIN_THREAD or ASSERTIONS
  def test_pthread_main_thread_blocking_join(self):
    create_file('pre.js', '''
      Module['printErr'] = (x) => {
        if (x.includes('Blocking on the main thread is very dangerous')) {
          maybeReportResultToServer('got_warn');
        }
      };
    ''')
    # Test that we warn about blocking on the main thread in debug builds
    self.btest('pthread/main_thread_join.cpp', expected='got_warn', cflags=['-sEXIT_RUNTIME', '-sASSERTIONS', '--pre-js', 'pre.js', '-pthread', '-sPTHREAD_POOL_SIZE'])
    # Test that we do not warn about blocking on the main thread in release builds
    self.btest_exit('pthread/main_thread_join.cpp', cflags=['-O3', '--pre-js', 'pre.js', '-pthread', '-sPTHREAD_POOL_SIZE'])
    # Test that tryjoin is fine, even if not ALLOW_BLOCKING_ON_MAIN_THREAD
    self.btest_exit('pthread/main_thread_join.cpp', assert_returncode=2, cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE', '-g', '-DTRY_JOIN', '-sALLOW_BLOCKING_ON_MAIN_THREAD=0'])
    # Test that tryjoin is fine, even if not ALLOW_BLOCKING_ON_MAIN_THREAD, and even without a pool
    self.btest_exit('pthread/main_thread_join.cpp', assert_returncode=2, cflags=['-O3', '-pthread', '-g', '-DTRY_JOIN', '-sALLOW_BLOCKING_ON_MAIN_THREAD=0'])
    # Test that everything works ok when we are on a pthread
    self.btest_exit('pthread/main_thread_join.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE', '-sPROXY_TO_PTHREAD', '-sALLOW_BLOCKING_ON_MAIN_THREAD=0'])

  # Test the old GCC atomic __sync_fetch_and_op builtin operations.
  @parameterized({
    '': (['-g'],),
    'O1': (['-O1', '-g'],),
    'O2': (['-O2'],),
    'O3': (['-O3'],),
    'Os': (['-Os'],),
  })
  def test_pthread_gcc_atomic_fetch_and_op(self, args):
    self.cflags += ['-Wno-sync-fetch-and-nand-semantics-changed']
    self.btest_exit('pthread/test_pthread_gcc_atomic_fetch_and_op.c', cflags=args + ['-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # 64 bit version of the above test.
  @also_with_wasm2js
  def test_pthread_gcc_64bit_atomic_fetch_and_op(self):
    if self.is_wasm2js():
      self.skipTest('https://github.com/WebAssembly/binaryen/issues/4358')
    self.cflags += ['-Wno-sync-fetch-and-nand-semantics-changed']
    self.btest_exit('pthread/test_pthread_gcc_64bit_atomic_fetch_and_op.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test the old GCC atomic __sync_op_and_fetch builtin operations.
  @also_with_wasm2js
  @requires_safari_version(170601, 'TODO: browser.test_pthread_gcc_atomic_op_and_fetch_wasm2js fails with "abort:Assertion failed: nand_and_fetch_data == -1"') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), passes in Safari 18.5 (20621.2.5.11.8) and Safari 26.0.1 (21622.1.22.11.15)
  def test_pthread_gcc_atomic_op_and_fetch(self):
    self.cflags += ['-Wno-sync-fetch-and-nand-semantics-changed']
    self.btest_exit('pthread/test_pthread_gcc_atomic_op_and_fetch.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # 64 bit version of the above test.
  @also_with_wasm2js
  def test_pthread_gcc_64bit_atomic_op_and_fetch(self):
    if self.is_wasm2js():
      self.skipTest('https://github.com/WebAssembly/binaryen/issues/4358')
    self.cflags += ['-Wno-sync-fetch-and-nand-semantics-changed', '--profiling-funcs']
    self.btest_exit('pthread/test_pthread_gcc_64bit_atomic_op_and_fetch.c', cflags=['-pthread', '-O2', '-sPTHREAD_POOL_SIZE=8'])

  # Tests the rest of the remaining GCC atomics after the two above tests.
  @also_with_wasm2js
  def test_pthread_gcc_atomics(self):
    self.btest_exit('pthread/test_pthread_gcc_atomics.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test the __sync_lock_test_and_set and __sync_lock_release primitives.
  @also_with_wasm2js
  @parameterized({
    '': ([],),
    'em_instrinsics': (['-DUSE_EMSCRIPTEN_INTRINSICS'],),
  })
  def test_pthread_gcc_spinlock(self, args):
    self.btest_exit('pthread/test_pthread_gcc_spinlock.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'] + args)

  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
    'minimal_runtime': (['-sMINIMAL_RUNTIME'],),
    'single_file': (['-sSINGLE_FILE'],),
  })
  def test_pthread_create(self, args):
    self.btest_exit('pthread/test_pthread_create.c',
                    cflags=['-pthread', '-sPTHREAD_POOL_SIZE=8'] + args)
    files = os.listdir('.')
    if '-sSINGLE_FILE' in args:
      self.assertEqual(len(files), 1, files)
    else:
      self.assertEqual(len(files), 3, files)

  # Test that preallocating worker threads work.
  def test_pthread_preallocates_workers(self):
    self.btest_exit('pthread/test_pthread_preallocates_workers.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=4', '-sPTHREAD_POOL_DELAY_LOAD'])

  # Test that allocating a lot of threads doesn't regress. This needs to be checked manually!
  @no_2gb('uses INITIAL_MEMORY')
  @no_4gb('uses INITIAL_MEMORY')
  def test_pthread_large_pthread_allocation(self):
    self.btest_exit('pthread/test_large_pthread_allocation.c', cflags=['-sINITIAL_MEMORY=128MB', '-O3', '-pthread', '-sPTHREAD_POOL_SIZE=50'])

  # Tests the -sPROXY_TO_PTHREAD option.
  def test_pthread_proxy_to_pthread(self):
    self.btest_exit('pthread/test_pthread_proxy_to_pthread.c', cflags=['-O3', '-pthread', '-sPROXY_TO_PTHREAD'])

  # Test that a pthread can spawn another pthread of its own.
  @parameterized({
    '': ([],),
    'modularize': (['-sMODULARIZE', '-sEXPORT_NAME=MyModule', '--shell-file', test_file('shell_that_launches_modularize.html')],),
  })
  def test_pthread_create_pthread(self, args):
    self.btest_exit('pthread/test_pthread_create_pthread.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=2'] + args)

  # Test another case of pthreads spawning pthreads, but this time the callers immediately join on the threads they created.
  def test_pthread_nested_spawns(self):
    self.btest_exit('pthread/test_pthread_nested_spawns.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=2'])

  # Test that main thread can wait for a pthread to finish via pthread_join().
  def test_pthread_join(self):
    self.btest_exit('pthread/test_pthread_join.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test that threads can rejoin the pool once detached and finished
  def test_std_thread_detach(self):
    self.btest_exit('pthread/test_std_thread_detach.cpp', cflags=['-pthread'])

  # Test pthread_cancel() operation
  def test_pthread_cancel(self):
    self.btest_exit('pthread/test_pthread_cancel.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test that pthread_cancel() cancels pthread_cond_wait() operation
  def test_pthread_cancel_cond_wait(self):
    self.btest_exit('pthread/test_pthread_cancel_cond_wait.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test pthread_kill() operation
  @no_chrome('pthread_kill hangs chrome renderer, and keep subsequent tests from passing')
  def test_pthread_kill(self):
    self.btest_exit('pthread/test_pthread_kill.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test that pthread cleanup stack (pthread_cleanup_push/_pop) works.
  def test_pthread_cleanup(self):
    self.btest_exit('pthread/test_pthread_cleanup.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Tests the pthread mutex api.
  @parameterized({
    '': ([],),
    'spinlock': (['-DSPINLOCK_TEST'],),
  })
  @requires_safari_version(170601, 'TODO: browser.test_pthread_mutex and browser.test_pthread_mutex_spinlock both hang Safari') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), passes in Safari 18.5 (20621.2.5.11.8) and Safari 26.0.1 (21622.1.22.11.15)
  def test_pthread_mutex(self, args):
    self.btest_exit('pthread/test_pthread_mutex.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'] + args)

  def test_pthread_attr_getstack(self):
    self.btest_exit('pthread/test_pthread_attr_getstack.c', cflags=['-pthread', '-sPTHREAD_POOL_SIZE=2'])

  # Test that memory allocation is thread-safe.
  def test_pthread_malloc(self):
    self.btest_exit('pthread/test_pthread_malloc.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Stress test pthreads allocating memory that will call to sbrk(), and main thread has to free up the data.
  def test_pthread_malloc_free(self):
    self.btest_exit('pthread/test_pthread_malloc_free.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test that the pthread_barrier API works ok.
  def test_pthread_barrier(self):
    self.btest_exit('pthread/test_pthread_barrier.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test the pthread_once() function.
  def test_pthread_once(self):
    self.btest_exit('pthread/test_pthread_once.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test against a certain thread exit time handling bug by spawning tons of threads.
  def test_pthread_spawns(self):
    self.btest_exit('pthread/test_pthread_spawns.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8', '--closure=1', '-sENVIRONMENT=web'])

  # It is common for code to flip volatile global vars for thread control. This is a bit lax, but nevertheless, test whether that
  # kind of scheme will work with Emscripten as well.
  @parameterized({
    '': (['-DUSE_C_VOLATILE'],),
    'atomic': ([],),
  })
  def test_pthread_volatile(self, args):
    self.btest_exit('pthread/test_pthread_volatile.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'] + args)

  # Test thread-specific data (TLS).
  def test_pthread_thread_local_storage(self):
    self.btest_exit('pthread/test_pthread_thread_local_storage.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8', '-sASSERTIONS'])

  # Test the pthread condition variable creation and waiting.
  def test_pthread_condition_variable(self):
    self.btest_exit('pthread/test_pthread_condition_variable.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8'])

  # Test that pthreads are able to do printf.
  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
    'debug': (['-sLIBRARY_DEBUG'],),
  })
  def test_pthread_printf(self, args):
     self.btest_exit('pthread/test_pthread_printf.c', cflags=['-pthread', '-sPTHREAD_POOL_SIZE'] + args)

  # Test that pthreads are able to do cout. Failed due to https://bugzil.la/1154858.
  def test_pthread_iostream(self):
    self.btest_exit('pthread/test_pthread_iostream.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE'])

  def test_pthread_unistd_io_bigint(self):
    self.btest_exit('unistd/io.c', cflags=['-pthread', '-sPROXY_TO_PTHREAD', '-sWASM_BIGINT'])

  # Test that the main thread is able to use pthread_set/getspecific.
  @also_with_wasm2js
  def test_pthread_setspecific_mainthread(self):
    self.btest_exit('pthread/test_pthread_setspecific_mainthread.c', cflags=['-O3', '-pthread'])

  # Test that pthreads have access to filesystem.
  def test_pthread_file_io(self):
    self.btest_exit('pthread/test_pthread_file_io.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE'])

  # Test that the pthread_create() function operates benignly in the case that threading is not supported.
  @parameterized({
   '': ([],),
   'mt': (['-pthread', '-sPTHREAD_POOL_SIZE=8'],),
  })
  def test_pthread_supported(self, args):
    self.btest_exit('pthread/test_pthread_supported.cpp', cflags=['-O3'] + args)

  def test_pthread_dispatch_after_exit(self):
    self.btest_exit('pthread/test_pthread_dispatch_after_exit.c', cflags=['-pthread'])

  # Test that if the main thread is performing a futex wait while a pthread
  # needs it to do a proxied operation (before that pthread would wake up the
  # main thread), that it's not a deadlock.
  def test_pthread_proxying_in_futex_wait(self):
    self.btest_exit('pthread/test_pthread_proxying_in_futex_wait.cpp', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE'])

  # Test that sbrk() operates properly in multithreaded conditions
  @no_2gb('uses INITIAL_MEMORY')
  @no_4gb('uses INITIAL_MEMORY')
  @parameterized({
    '': (['-DABORTING_MALLOC=0', '-sABORTING_MALLOC=0'],),
    'aborting_malloc': (['-DABORTING_MALLOC=1'],),
  })
  def test_pthread_sbrk(self, args):
    # With aborting malloc = 1, test allocating memory in threads
    # With aborting malloc = 0, allocate so much memory in threads that some of the allocations fail.
    self.btest_exit('pthread/test_pthread_sbrk.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE=8', '-sINITIAL_MEMORY=128MB'] + args)

  # Test that -sABORTING_MALLOC=0 works in both pthreads and non-pthreads
  # builds. (sbrk fails gracefully)
  @also_with_threads
  @parameterized({
    '': ([],),
    'O2': (['-O2'],),
  })
  def test_gauge_available_memory(self, args):
    self.btest_exit('test_gauge_available_memory.c', cflags=['-sABORTING_MALLOC=0'] + args)

  # Test that the proxying operations of user code from pthreads to main thread
  # work
  def test_pthread_run_on_main_thread(self):
    self.btest_exit('pthread/test_pthread_run_on_main_thread.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE'])

  # Test how a lot of back-to-back called proxying operations behave.
  def test_pthread_run_on_main_thread_flood(self):
    self.btest_exit('pthread/test_pthread_run_on_main_thread_flood.c', cflags=['-O3', '-pthread', '-sPTHREAD_POOL_SIZE'])

  # Test that it is possible to asynchronously call a JavaScript function on the
  # main thread.
  def test_pthread_call_async(self):
    self.btest_exit('pthread/call_async.c', cflags=['-pthread'])

  # Test that it is possible to synchronously call a JavaScript function on the
  # main thread and get a return value back.
  def test_pthread_call_sync_on_main_thread(self):
    self.btest_exit('pthread/call_sync_on_main_thread.c', cflags=['-O3', '-pthread', '-sPROXY_TO_PTHREAD', '-DPROXY_TO_PTHREAD=1', '--js-library', test_file('pthread/call_sync_on_main_thread.js')])
    self.btest_exit('pthread/call_sync_on_main_thread.c', cflags=['-O3', '-pthread', '-DPROXY_TO_PTHREAD=0', '--js-library', test_file('pthread/call_sync_on_main_thread.js')])
    self.btest_exit('pthread/call_sync_on_main_thread.c', cflags=['-Oz', '-DPROXY_TO_PTHREAD=0', '--js-library', test_file('pthread/call_sync_on_main_thread.js')])

  # Test that it is possible to asynchronously call a JavaScript function on the
  # main thread.
  def test_pthread_call_async_on_main_thread(self):
    self.btest('pthread/call_async_on_main_thread.c', expected='7', cflags=['-O3', '-pthread', '-sPROXY_TO_PTHREAD', '-DPROXY_TO_PTHREAD=1', '--js-library', test_file('pthread/call_async_on_main_thread.js')])
    self.btest('pthread/call_async_on_main_thread.c', expected='7', cflags=['-O3', '-pthread', '-DPROXY_TO_PTHREAD=0', '--js-library', test_file('pthread/call_async_on_main_thread.js')])
    self.btest('pthread/call_async_on_main_thread.c', expected='7', cflags=['-Oz', '-DPROXY_TO_PTHREAD=0', '--js-library', test_file('pthread/call_async_on_main_thread.js')])

  # Tests that spawning a new thread does not cause a reinitialization of the
  # global data section of the application memory area.
  @parameterized({
    '': (['-O3'],),
    'modularize': (['-sMODULARIZE', '-sEXPORT_NAME=MyModule', '--shell-file', test_file('shell_that_launches_modularize.html')],),
  })
  def test_pthread_global_data_initialization(self, args):
    self.btest_exit('pthread/test_pthread_global_data_initialization.c', cflags=args + ['-pthread', '-sPROXY_TO_PTHREAD', '-sPTHREAD_POOL_SIZE'])

  @requires_wasm2js
  def test_pthread_global_data_initialization_in_sync_compilation_mode(self):
    self.btest_exit('pthread/test_pthread_global_data_initialization.c', cflags=['-sWASM_ASYNC_COMPILATION=0', '-pthread', '-sPROXY_TO_PTHREAD', '-sPTHREAD_POOL_SIZE'])

  # Test that emscripten_get_now() reports coherent wallclock times across all
  # pthreads, instead of each pthread independently reporting wallclock times
  # since the launch of that pthread.
  def test_pthread_clock_drift(self):
    self.btest_exit('pthread/test_pthread_clock_drift.c', cflags=['-O3', '-pthread', '-sPROXY_TO_PTHREAD'])

  def test_pthread_utf8_funcs(self):
    self.btest_exit('pthread/test_pthread_utf8_funcs.c', cflags=['-pthread', '-sPTHREAD_POOL_SIZE'])

  # Test the emscripten_futex_wake(addr, INT_MAX); functionality to wake all
  # waiters
  @also_with_wasm2js
  def test_pthread_wake_all(self):
    self.btest_exit('pthread/test_futex_wake_all.c', cflags=['-O3', '-pthread'])

  # Test that stack base and max correctly bound the stack on pthreads.
  def test_pthread_stack_bounds(self):
    self.btest_exit('pthread/test_pthread_stack_bounds.cpp', cflags=['-pthread'])

  # Test that real `thread_local` works.
  def test_pthread_tls(self):
    self.btest_exit('pthread/test_pthread_tls.c', cflags=['-sPROXY_TO_PTHREAD', '-pthread'])

  # Test that real `thread_local` works in main thread without PROXY_TO_PTHREAD.
  def test_pthread_tls_main(self):
    self.btest_exit('pthread/test_pthread_tls_main.cpp', cflags=['-pthread'])

  def test_pthread_safe_stack(self):
    # Note that as the test runs with PROXY_TO_PTHREAD, we set STACK_SIZE,
    # and not DEFAULT_PTHREAD_STACK_SIZE, as the pthread for main() gets the
    # same stack size as the main thread normally would.
    self.btest('core/test_safe_stack.c', expected='abort:stack overflow', cflags=['-pthread', '-sPROXY_TO_PTHREAD', '-sSTACK_OVERFLOW_CHECK=2', '-sSTACK_SIZE=64KB'])

  @no_wasm64('TODO: ASAN in memory64')
  @parameterized({
    'leak': ['test_pthread_lsan_leak', ['-gsource-map']],
    'no_leak': ['test_pthread_lsan_no_leak', []],
  })
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/15978')
  @no_safari('TODO: browser.test_pthread_lsan_leak fails with /report_result?0') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_pthread_lsan(self, name, args):
    self.btest(Path('pthread', name + '.cpp'), expected='1', cflags=['-fsanitize=leak', '-pthread', '-sPROXY_TO_PTHREAD', '--pre-js', test_file('pthread', name + '.js')] + args)

  @no_wasm64('TODO: ASAN in memory64')
  @no_2gb('ASAN + GLOBAL_BASE')
  @no_4gb('ASAN + GLOBAL_BASE')
  @parameterized({
    # Reusing the LSan test files for ASan.
    'leak': ['test_pthread_lsan_leak', ['-gsource-map']],
    'no_leak': ['test_pthread_lsan_no_leak', []],
  })
  @no_safari('TODO: browser.test_pthread_asan_leak fails with /report_result?0') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_pthread_asan(self, name, args):
    self.btest(Path('pthread', name + '.cpp'), expected='1', cflags=['-fsanitize=address', '-pthread', '-sPROXY_TO_PTHREAD', '--pre-js', test_file('pthread', name + '.js')] + args)

  @no_wasm64('TODO: ASAN in memory64')
  @no_2gb('ASAN + GLOBAL_BASE')
  @no_4gb('ASAN + GLOBAL_BASE')
  def test_pthread_asan_use_after_free(self):
    self.btest('pthread/test_pthread_asan_use_after_free.cpp', expected='1', cflags=['-fsanitize=address', '-pthread', '-sPROXY_TO_PTHREAD', '--pre-js', test_file('pthread/test_pthread_asan_use_after_free.js')])

  @no_wasm64('TODO: ASAN in memory64')
  @no_2gb('ASAN + GLOBAL_BASE')
  @no_4gb('ASAN + GLOBAL_BASE')
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/20006')
  @no_safari('TODO: Hangs') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  @also_with_wasmfs
  def test_pthread_asan_use_after_free_2(self):
    # similiar to test_pthread_asan_use_after_free, but using a pool instead
    # of proxy-to-pthread, and al, '-Wno-experimental'so the allocation happens on the pthread
    # (which tests that it can use the offset converter to get the stack
    # trace there)
    self.btest('pthread/test_pthread_asan_use_after_free_2.cpp', expected='1', cflags=['-fsanitize=address', '-pthread', '-sPTHREAD_POOL_SIZE=1', '--pre-js', test_file('pthread/test_pthread_asan_use_after_free_2.js')])

  def test_pthread_exit_process(self):
    args = ['-pthread',
            '-sPROXY_TO_PTHREAD',
            '-sPTHREAD_POOL_SIZE=2',
            '-sEXIT_RUNTIME',
            '-DEXIT_RUNTIME',
            '-O0']
    args += ['--pre-js', test_file('core/pthread/test_pthread_exit_runtime.pre.js')]
    self.btest('core/pthread/test_pthread_exit_runtime.c', expected='onExit status: 42', cflags=args)

  @no_safari('TODO: Fails with report_result?unexpected: [object ErrorEvent]') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_pthread_trap(self):
    create_file('pre.js', '''
    if (typeof window === 'object' && window) {
      window.addEventListener('error', function(e) {
        if (e.error && e.error.message.includes('unreachable'))
          maybeReportResultToServer("expected exception caught");
        else
          maybeReportResultToServer("unexpected: " + e);
      });
    }''')
    args = ['-pthread',
            '-sPROXY_TO_PTHREAD',
            '-sEXIT_RUNTIME',
            '--profiling-funcs',
            '--pre-js=pre.js']
    self.btest('pthread/test_pthread_trap.c', expected='expected exception caught', cflags=args)

  # Tests MAIN_THREAD_EM_ASM_INT() function call signatures.
  def test_main_thread_em_asm_signatures(self):
    self.btest_exit('core/test_em_asm_signatures.cpp')

  def test_main_thread_em_asm_signatures_pthreads(self):
    self.btest_exit('core/test_em_asm_signatures.cpp', cflags=['-O3', '-pthread', '-sPROXY_TO_PTHREAD', '-sASSERTIONS'])

  def test_main_thread_async_em_asm(self):
    self.btest_exit('core/test_main_thread_async_em_asm.cpp', cflags=['-O3', '-pthread', '-sPROXY_TO_PTHREAD', '-sASSERTIONS'])

  def test_main_thread_em_asm_blocking(self):
    shutil.copy(test_file('browser/test_em_asm_blocking.html'), 'page.html')

    self.compile_btest('browser/test_em_asm_blocking.cpp', ['-O2', '-o', 'wasm.js', '-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'])
    self.run_browser('page.html', '/report_result?exit:8')

  # Test that it is possible to send a signal via calling alarm(timeout), which in turn calls to the signal handler set by signal(SIGALRM, func);
  def test_sigalrm(self):
    self.btest_exit('test_sigalrm.c', cflags=['-O3'])

  @parameterized({
    '': ([], 1),
    'O1': (['-O1'], 1),
    'O2': (['-O2'], 1),
    'O3': (['-O3'], 1),
    # force it on
    'force': (['-sWASM_ASYNC_COMPILATION'], 1),
    'off': (['-sWASM_ASYNC_COMPILATION=0'], 0),
  })
  def test_async_compile(self, opts, returncode):
    # notice when we use async compilation
    create_file('pre.js', '''
      // note if we do async compilation
      var real_wasm_instantiate = WebAssembly.instantiate;
      var real_wasm_instantiateStreaming = WebAssembly.instantiateStreaming;
      if (typeof real_wasm_instantiateStreaming === 'function') {
        WebAssembly.instantiateStreaming = (a, b) => {
          err('instantiateStreaming called');
          console.log('instantiateStreaming called');
          Module.sawAsyncCompilation = true;
          return real_wasm_instantiateStreaming(a, b);
        };
      } else {
        WebAssembly.instantiate = (a, b) => {
          err('instantiate called');
          Module.sawAsyncCompilation = true;
          return real_wasm_instantiate(a, b);
        };
      }
    ''')
    self.cflags.append('--pre-js=pre.js')
    self.btest_exit('test_async_compile.c', assert_returncode=returncode, cflags=opts)
    # Ensure that compilation still works and is async without instantiateStreaming available
    create_file('pre0.js', 'WebAssembly.instantiateStreaming = undefined;')
    self.cflags.insert(0, '--pre-js=pre0.js')
    self.btest_exit('test_async_compile.c', assert_returncode=1)

  # Test that implementing Module.instantiateWasm() callback works.
  @also_with_asan
  def test_manual_wasm_instantiate(self):
    self.set_setting('EXIT_RUNTIME')
    self.compile_btest('test_manual_wasm_instantiate.c', ['-o', 'manual_wasm_instantiate.js'], reporting=Reporting.JS_ONLY)
    shutil.copy(test_file('test_manual_wasm_instantiate.html'), '.')
    self.run_browser('test_manual_wasm_instantiate.html', '/report_result?exit:0')

  def test_wasm_locate_file(self):
    # Test that it is possible to define "Module.locateFile(foo)" function to locate where worker.js will be loaded from.
    ensure_dir('cdn')
    shell = read_file(path_from_root('src/shell.html'))
    create_file('shell2.html', shell.replace('var Module = {', 'var Module = { locateFile: (filename) => (filename == "test.wasm") ? "cdn/test.wasm" : filename, '))
    self.compile_btest('browser_test_hello_world.c', ['--shell-file', 'shell2.html', '-o', 'test.html'])
    shutil.move('test.wasm', Path('cdn/test.wasm'))
    self.run_browser('test.html', '/report_result?0')

  @also_with_threads
  def test_utf8_textdecoder(self):
    self.btest_exit('benchmark/benchmark_utf8.c', 0, cflags=['--embed-file', test_file('utf8_corpus.txt') + '@/utf8_corpus.txt'])

  @also_with_threads
  def test_utf16_textdecoder(self):
    self.btest_exit('benchmark/benchmark_utf16.cpp', 0, cflags=['--embed-file', test_file('utf16_corpus.txt') + '@/utf16_corpus.txt', '-sEXPORTED_RUNTIME_METHODS=UTF16ToString,stringToUTF16,lengthBytesUTF16'])

  # Tests that it is possible to initialize and render WebGL content in a
  # pthread by using OffscreenCanvas.
  @no_chrome('https://crbug.com/961765')
  # The non-chained version suffers from browser priority inversion deadlock problem: offscreenCanvas.getContext("webgl2") does not make progress in a pthread until main thread yields to event loop.
  # The chained version of this test suffers from bug https://bugzil.la/1992576
  @no_firefox('https://bugzil.la/1972240 (priority inversion deadlock) + https://bugzil.la/1992576 (chained OffscreenCanvas transfer)')
  @parameterized({
    '': ([],),
    # -DTEST_CHAINED_WEBGL_CONTEXT_PASSING:
    # Tests that it is possible to transfer WebGL canvas in a chain from main
    # thread -> thread 1 -> thread 2 and then init and render WebGL content there.
    'chained': (['-DTEST_CHAINED_WEBGL_CONTEXT_PASSING'],),
  })
  @requires_offscreen_canvas
  @requires_graphics_hardware
  def test_webgl_offscreen_canvas_in_pthread(self, args):
    self.btest('gl_in_pthread.c', expected='1', cflags=args + ['-pthread', '-sPTHREAD_POOL_SIZE=2', '-sOFFSCREENCANVAS_SUPPORT', '-lGL'])

  # Tests that it is possible to render WebGL content on a <canvas> on the main
  # thread, after it has once been used to render WebGL content in a pthread
  # first -DTEST_MAIN_THREAD_EXPLICIT_COMMIT: Test the same (WebGL on main
  # thread after pthread), but by using explicit .commit() to swap on the main
  # thread instead of implicit "swap when rAF ends" logic
  @parameterized({
    '': ([],),
    'explicit': (['-DTEST_MAIN_THREAD_EXPLICIT_COMMIT'],),
  })
  @requires_offscreen_canvas
  @requires_graphics_hardware
  @disabled('This test is disabled because current OffscreenCanvas does not allow transfering it after a rendering context has been created for it.')
  def test_webgl_offscreen_canvas_in_mainthread_after_pthread(self, args):
    self.btest('gl_in_mainthread_after_pthread.c', expected='0', cflags=args + ['-pthread', '-sPTHREAD_POOL_SIZE=2', '-sOFFSCREENCANVAS_SUPPORT', '-lGL'])

  @requires_offscreen_canvas
  @requires_graphics_hardware
  def test_webgl_offscreen_canvas_only_in_pthread(self):
    self.btest_exit('gl_only_in_pthread.c', cflags=['-pthread', '-sPTHREAD_POOL_SIZE', '-sOFFSCREENCANVAS_SUPPORT', '-lGL', '-sOFFSCREEN_FRAMEBUFFER'])

  # Tests that rendering from client side memory without default-enabling extensions works.
  @requires_graphics_hardware
  def test_webgl_from_client_side_memory_without_default_enabled_extensions(self):
    self.btest_exit('webgl_draw_triangle.c', cflags=['-lGL', '-sOFFSCREEN_FRAMEBUFFER', '-DEXPLICIT_SWAP=1', '-DDRAW_FROM_CLIENT_MEMORY=1', '-sFULL_ES2'])

  # Tests for WEBGL_multi_draw extension
  # For testing WebGL draft extensions like this, if using chrome as the browser,
  # We might want to append the --enable-webgl-draft-extensions to the EMTEST_BROWSER env arg.
  @requires_graphics_hardware
  @no_2gb('https://crbug.com/324562920')
  @no_4gb('https://crbug.com/324562920')
  @parameterized({
    'arrays': (['-DMULTI_DRAW_ARRAYS'],),
    'arrays_instanced': (['-DMULTI_DRAW_ARRAYS_INSTANCED'],),
    'elements': (['-DMULTI_DRAW_ELEMENTS'],),
    'elements_instanced': (['-DMULTI_DRAW_ELEMENTS_INSTANCED'],),
  })
  def test_webgl_multi_draw(self, args):
    self.reftest('webgl_multi_draw_test.c', 'webgl_multi_draw.png',
                 cflags=['-lGL', '-sOFFSCREEN_FRAMEBUFFER', '-DEXPLICIT_SWAP'] + args)

  # Tests for base_vertex/base_instance extension
  # For testing WebGL draft extensions like this, if using chrome as the browser,
  # We might want to append the --enable-webgl-draft-extensions to the EMTEST_BROWSER env arg.
  # If testing on Mac, you also need --use-cmd-decoder=passthrough to get this extension.
  # Also there is a known bug with Mac Intel baseInstance which can fail producing the expected image result.
  @requires_webgl2
  @parameterized({
    '': (0,),
    'multidraw': (1,),
  })
  @parameterized({
    '': (0,),
    'drawelements': (1,),
  })
  def test_webgl_draw_base_vertex_base_instance(self, multi_draw, draw_elements):
    self.reftest('webgl_draw_base_vertex_base_instance_test.c', 'webgl_draw_instanced_base_vertex_base_instance.png',
                 cflags=['-lGL',
                            '-sMAX_WEBGL_VERSION=2',
                            '-sOFFSCREEN_FRAMEBUFFER',
                            '-DMULTI_DRAW=' + str(multi_draw),
                            '-DDRAW_ELEMENTS=' + str(draw_elements),
                            '-DEXPLICIT_SWAP=1',
                            '-DWEBGL_CONTEXT_VERSION=2'])

  @requires_graphics_hardware
  def test_webgl_sample_query(self):
    self.btest_exit('webgl_sample_query.c', cflags=['-sMAX_WEBGL_VERSION=2', '-lGL'])

  @requires_graphics_hardware
  @parameterized({
    # EXT query entrypoints on WebGL 1.0
    '': (['-sMAX_WEBGL_VERSION'],),
    # EXT query entrypoints on a WebGL 1.0 context while built for WebGL 2.0
    'v2': (['-sMAX_WEBGL_VERSION=2'],),
    # builtin query entrypoints on WebGL 2.0
    'v2api': (['-sMAX_WEBGL_VERSION=2', '-DTEST_WEBGL2'],),
  })
  def test_webgl_timer_query(self, args):
    self.btest_exit('webgl_timer_query.c', cflags=args + ['-lGL'])

  # Tests that -sOFFSCREEN_FRAMEBUFFER rendering works.
  @requires_graphics_hardware
  @parameterized({
    '': ([],),
    'threads': (['-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  @parameterized({
    '': ([],),
    'es2': (['-sFULL_ES2'],),
    'es3': (['-sFULL_ES3'],),
  })
  def test_webgl_offscreen_framebuffer(self, version, threads):
    # Tests all the different possible versions of libgl
    args = ['-lGL', '-sOFFSCREEN_FRAMEBUFFER', '-DEXPLICIT_SWAP=1'] + threads + version
    self.btest_exit('webgl_draw_triangle.c', cflags=args)

  # Tests that VAOs can be used even if WebGL enableExtensionsByDefault is set to 0.
  @requires_graphics_hardware
  def test_webgl_vao_without_automatic_extensions(self):
    self.btest_exit('test_webgl_no_auto_init_extensions.c', cflags=['-lGL', '-sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0'])

  # Tests that offscreen framebuffer state restoration works
  @requires_graphics_hardware
  @parameterized({
    # full state restoration path on WebGL 1.0
    'gl1_no_vao': (['-sMAX_WEBGL_VERSION=1', '-DTEST_DISABLE_VAO'],),
    # VAO path on WebGL 1.0
    'gl1': (['-sMAX_WEBGL_VERSION=1', '-DTEST_VERIFY_WEBGL1_VAO_SUPPORT=1'],),
    'gl1_max_gl2': (['-sMAX_WEBGL_VERSION=2'],),
    # VAO path on WebGL 2.0
    'gl2': (['-sMAX_WEBGL_VERSION=2', '-DTEST_WEBGL2=1', '-DTEST_ANTIALIAS=1'],),
    # full state restoration path on WebGL 2.0
    'gl2_no_vao': (['-sMAX_WEBGL_VERSION=2', '-DTEST_WEBGL2=1', '-DTEST_ANTIALIAS=1', '-DTEST_DISABLE_VAO'],),
    # blitFramebuffer path on WebGL 2.0 (falls back to VAO on Firefox < 67)
    'gl2_no_aa': (['-sMAX_WEBGL_VERSION=2', '-DTEST_WEBGL2=1', '-DTEST_ANTIALIAS=0'],),
  })
  def test_webgl_offscreen_framebuffer_state_restoration(self, args):
    if '-DTEST_WEBGL2=1' in args and webgl2_disabled():
      self.skipTest('This test requires WebGL2 to be available')
    base_args = ['-lGL', '-sOFFSCREEN_FRAMEBUFFER', '-DEXPLICIT_SWAP=1']
    self.btest_exit('webgl_offscreen_framebuffer_swap_with_bad_state.c', cflags=base_args + args)

  @parameterized({
    '': ([],),
    'es2': (['-sFULL_ES2'],),
    'es3': (['-sFULL_ES3'],),
  })
  @requires_graphics_hardware
  def test_webgl_draw_triangle_with_uniform_color(self, args):
    self.btest_exit('webgl_draw_triangle_with_uniform_color.c', cflags=args)

  # Tests that using an array of structs in GL uniforms works.
  @requires_webgl2
  def test_webgl_array_of_structs_uniform(self):
    self.reftest('webgl_array_of_structs_uniform.c', 'webgl_array_of_structs_uniform.png', cflags=['-lGL', '-sMAX_WEBGL_VERSION=2'])

  # Tests that if a WebGL context is created in a pthread on a canvas that has
  # not been transferred to that pthread, WebGL calls are then proxied to the
  # main thread -DTEST_OFFSCREEN_CANVAS=1: Tests that if a WebGL context is
  # created on a pthread that has the canvas transferred to it via using
  # Emscripten's EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES="#canvas", then
  # OffscreenCanvas is used -DTEST_OFFSCREEN_CANVAS=2: Tests that if a WebGL
  # context is created on a pthread that has the canvas transferred to it via
  # automatic transferring of Module.canvas when
  # EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES is not defined, then OffscreenCanvas
  # is also used
  @parameterized({
    '': ([False],),
    'asyncify': ([True],),
  })
  @requires_offscreen_canvas
  @requires_graphics_hardware
  def test_webgl_offscreen_canvas_in_proxied_pthread(self, asyncify):
    args = ['-pthread', '-sOFFSCREENCANVAS_SUPPORT', '-lGL', '-sGL_DEBUG', '-sPROXY_TO_PTHREAD']
    if asyncify:
      # given the synchronous render loop here, asyncify is needed to see intermediate frames and
      # the gradual color change
      args += ['-sASYNCIFY', '-DASYNCIFY']
    self.btest_exit('gl_in_proxy_pthread.c', cflags=args)

  @parameterized({
    '': ([],),
    'proxy': (['-sPROXY_TO_PTHREAD'],),
  })
  @parameterized({
    '': ([],),
    'blocking': (['-DTEST_SYNC_BLOCKING_LOOP=1'],),
  })
  @parameterized({
    '': ([],),
    'offscreen': (['-sOFFSCREENCANVAS_SUPPORT', '-sOFFSCREEN_FRAMEBUFFER'],),
  })
  @requires_graphics_hardware
  @requires_offscreen_canvas
  def test_webgl_resize_offscreencanvas_from_main_thread(self, args1, args2, args3):
    cmd = args1 + args2 + args3 + ['-pthread', '-lGL', '-sGL_DEBUG']

    if is_firefox() and '-sOFFSCREENCANVAS_SUPPORT' in cmd and '-sPROXY_TO_PTHREAD' in cmd:
      # Firefox is unable to transfer the same OffscreenCanvas multiple times across Workers
      # (in a chained fashion, e.g. main thread -> proxy-to-pthread main thread -> back to main thread -> user pthread)
      self.skipTest('https://bugzil.la/1992576')

    print(str(cmd))
    self.btest_exit('test_webgl_resize_offscreencanvas_from_main_thread.c', cflags=cmd)

  @requires_graphics_hardware
  @parameterized({
    'v1': (1,),
    'v2': (2,),
  })
  @parameterized({
    'enable': (1,),
    'disable': (0,),
  })
  def test_webgl_simple_extensions(self, webgl_version, simple_enable_extensions):
    if webgl_version == 2 and webgl2_disabled():
      self.skipTest('This test requires WebGL2 to be available')

    cmd = ['-DWEBGL_CONTEXT_VERSION=' + str(webgl_version),
           '-DWEBGL_SIMPLE_ENABLE_EXTENSION=' + str(simple_enable_extensions),
           '-sMAX_WEBGL_VERSION=2',
           '-sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=' + str(simple_enable_extensions),
           '-sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=' + str(simple_enable_extensions)]
    self.btest_exit('webgl2_simple_enable_extensions.c', cflags=cmd)

  @parameterized({
    '': ([],),
    'closure': (['-sASSERTIONS', '--closure=1'],),
    'closure_advanced': (['-sASSERTIONS', '--closure=1', '-O3'],),
    'main_module': (['-sMAIN_MODULE=1'],),
    'pthreads': (['-pthread', '-sOFFSCREENCANVAS_SUPPORT'],),
  })
  @requires_webgpu
  def test_webgpu_basic_rendering(self, args):
    self.btest_exit('webgpu_basic_rendering.cpp', cflags=['--use-port=emdawnwebgpu', '-sEXIT_RUNTIME'] + args)

  @requires_webgpu
  def test_webgpu_required_limits(self):
    self.set_setting('DEFAULT_TO_CXX')  # emdawnwebgpu uses C++ internally
    self.btest_exit('webgpu_required_limits.c', cflags=['--use-port=emdawnwebgpu'])

  # Tests the feature that shell html page can preallocate the typed array and place it
  # to Module.buffer before loading the script page.
  # In this build mode, the -sINITIAL_MEMORY=xxx option will be ignored.
  # Preallocating the buffer in this was is asm.js only (wasm needs a Memory).
  @requires_wasm2js
  def test_preallocated_heap(self):
    self.btest_exit('test_preallocated_heap.cpp', cflags=['-sWASM=0', '-sIMPORTED_MEMORY', '-sINITIAL_MEMORY=16MB', '-sABORTING_MALLOC=0', '--shell-file', test_file('test_preallocated_heap_shell.html')])

  # Tests emscripten_fetch() usage to XHR data directly to memory without persisting results to IndexedDB.
  @also_with_wasm2js
  @also_with_fetch_streaming
  def test_fetch_to_memory(self):
    # Test error reporting in the negative case when the file URL doesn't exist. (http 404)
    self.btest_exit('fetch/test_fetch_to_memory.cpp',
                    cflags=['-sFETCH_DEBUG', '-sFETCH', '-DFILE_DOES_NOT_EXIST'])

    # Test the positive case when the file URL exists. (http 200)
    shutil.copy(test_file('gears.png'), '.')
    for arg in ([], ['-sFETCH_SUPPORT_INDEXEDDB=0']):
      self.btest_exit('fetch/test_fetch_to_memory.cpp',
                      cflags=['-sFETCH_DEBUG', '-sFETCH'] + arg)

  @also_with_wasm2js
  @also_with_fetch_streaming
  @parameterized({
    '': ([],),
    'pthread_exit': (['-DDO_PTHREAD_EXIT'],),
  })
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/16868')
  def test_fetch_from_thread(self, args):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_from_thread.cpp',
                    cflags=args + ['-pthread', '-sPROXY_TO_PTHREAD', '-sFETCH_DEBUG', '-sFETCH', '-DFILE_DOES_NOT_EXIST'])

  @also_with_wasm2js
  @also_with_fetch_streaming
  def test_fetch_to_indexdb(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_to_indexeddb.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH'])

  # Tests emscripten_fetch() usage to persist an XHR into IndexedDB and subsequently load up from there.
  @also_with_wasm2js
  @also_with_fetch_streaming
  def test_fetch_cached_xhr(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_cached_xhr.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH'])

  # Tests that response headers get set on emscripten_fetch_t values.
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/16868')
  @also_with_wasm2js
  @also_with_fetch_streaming
  @parameterized({
    '': ([],),
    'sync': (['-DSYNC'],),
  })
  def test_fetch_response_headers(self, args):
    if self.get_setting('FETCH_STREAMING') and '-DSYNC' in args:
      self.skipTest('Fetch backend does not support sync fetch.')
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_response_headers.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH', '-pthread', '-sPROXY_TO_PTHREAD'] + args)

  def make_largefile(self):
    s = '12345678'
    for _ in range(14):
      s = s[::-1] + s # length of str will be 2^17=128KB
    with open('largefile.txt', 'w') as f:
      for _ in range(1024):
        f.write(s)

  # Test emscripten_fetch() usage to stream a fetch in to memory without storing the full file in memory
  # Streaming only works the fetch backend.
  @also_with_wasm2js
  def test_fetch_stream_file(self):
    # Strategy: create a large 128MB file, and compile with a small 16MB Emscripten heap, so that the tested file
    # won't fully fit in the heap. This verifies that streaming works properly.
    self.make_largefile()
    self.btest_exit('fetch/test_fetch_stream_file.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH', '-sFETCH_STREAMING'])

  @also_with_fetch_streaming
  def test_fetch_headers_received(self):
    create_file('myfile.dat', 'hello world\n')
    self.btest_exit('fetch/test_fetch_headers_received.c', cflags=['-sFETCH_DEBUG', '-sFETCH'])

  @also_with_fetch_streaming
  def test_fetch_xhr_abort(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_xhr_abort.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH'])

  # Tests emscripten_fetch() usage in synchronous mode when used from the main
  # thread proxied to a Worker with -sPROXY_TO_PTHREAD option.
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/16868')
  @also_with_wasm2js
  def test_fetch_sync_xhr(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_sync_xhr.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH', '-pthread', '-sPROXY_TO_PTHREAD'])

  # Tests synchronous emscripten_fetch() usage from pthread
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/16868')
  def test_fetch_sync(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_sync.c', cflags=['-sFETCH', '-pthread', '-sPROXY_TO_PTHREAD'])

  # Tests that the Fetch API works for synchronous XHRs when program is run in a worker
  @no_firefox('https://github.com/emscripten-core/emscripten/issues/16868')
  @also_with_wasm2js
  def test_fetch_sync_xhr_in_proxy_to_worker(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_sync_xhr.cpp', cflags=['-sFETCH_DEBUG', '-sFETCH'], run_in_worker=True)

  @disabled('https://github.com/emscripten-core/emscripten/issues/16746')
  def test_fetch_idb_store(self):
    self.btest_exit('fetch/test_fetch_idb_store.cpp', cflags=['-pthread', '-sFETCH', '-sPROXY_TO_PTHREAD'])

  @disabled('https://github.com/emscripten-core/emscripten/issues/16746')
  def test_fetch_idb_delete(self):
    shutil.copy(test_file('gears.png'), '.')
    self.btest_exit('fetch/test_fetch_idb_delete.cpp', cflags=['-pthread', '-sFETCH_DEBUG', '-sFETCH', '-sWASM=0', '-sPROXY_TO_PTHREAD'])

  @also_with_fetch_streaming
  def test_fetch_post(self):
    self.btest_exit('fetch/test_fetch_post.c', cflags=['-sFETCH'])

  @also_with_fetch_streaming
  def test_fetch_progress(self):
    create_file('myfile.dat', 'hello world\n' * 1000)
    self.btest_exit('fetch/test_fetch_progress.c', cflags=['-sFETCH'])

  @also_with_fetch_streaming
  def test_fetch_to_memory_async(self):
    create_file('myfile.dat', 'hello world\n' * 1000)
    self.btest_exit('fetch/test_fetch_to_memory_async.c', cflags=['-sFETCH'])

  @requires_shared_array_buffer
  def test_fetch_to_memory_sync(self):
    create_file('myfile.dat', 'hello world\n' * 1000)
    self.btest_exit('fetch/test_fetch_to_memory_sync.c', cflags=['-sFETCH', '-pthread', '-sPROXY_TO_PTHREAD'])

  # Streaming only works the fetch backend.
  def test_fetch_stream_async(self):
    create_file('myfile.dat', 'hello world\n' * 1000)
    self.btest_exit('fetch/test_fetch_stream_async.c', cflags=['-sFETCH', '-sFETCH_STREAMING'])

  @also_with_asan
  def test_fetch_stream_abort(self):
    self.make_largefile()
    self.btest_exit('fetch/test_fetch_stream_abort.cpp', cflags=['-sFETCH', '-sFETCH_STREAMING', '-sALLOW_MEMORY_GROWTH'])

  @also_with_fetch_streaming
  def test_fetch_persist(self):
    create_file('myfile.dat', 'hello world\n')
    self.btest_exit('fetch/test_fetch_persist.c', cflags=['-sFETCH'])

  @no_firefox('https://github.com/emscripten-core/emscripten/issues/16868')
  @also_with_fetch_streaming
  def test_fetch_redirect(self):
    self.btest_exit('fetch/test_fetch_redirect.c', cflags=['-sFETCH', '-pthread', '-sPROXY_TO_PTHREAD', f'-DSERVER="{self.SERVER_URL}"'])

  @parameterized({
    '': ([],),
    'mt': (['-pthread', '-sPTHREAD_POOL_SIZE=2'],),
  })
  def test_pthread_locale(self, args):
    self.cflags.append('-I' + path_from_root('system/lib/libc/musl/src/internal'))
    self.cflags.append('-I' + path_from_root('system/lib/pthread'))
    self.btest_exit('pthread/test_pthread_locale.c', cflags=args)

  # Tests the Emscripten HTML5 API emscripten_set_canvas_element_size() and
  # emscripten_get_canvas_element_size() functionality in singlethreaded programs.
  def test_emscripten_set_canvas_element_size(self):
    self.btest_exit('emscripten_set_canvas_element_size.c')

  # Test that emscripten_get_device_pixel_ratio() is callable from pthreads (and proxies to main
  # thread to obtain the proper window.devicePixelRatio value).
  @parameterized({
    '': ([],),
    'pthread': (['-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  def test_emscripten_get_device_pixel_ratio(self, args):
    self.btest_exit('emscripten_get_device_pixel_ratio.c', cflags=args)

  # Tests that emscripten_run_script() variants of functions work in pthreads.
  @parameterized({
    '': ([],),
    'pthread': (['-pthread', '-sPROXY_TO_PTHREAD'],),
  })
  def test_pthread_run_script(self, args):
    shutil.copy(test_file('pthread/foo.js'), '.')
    self.btest_exit('pthread/test_pthread_run_script.c', cflags=['-O3'] + args)

  # Tests emscripten_set_canvas_element_size() and OffscreenCanvas functionality in different build configurations.
  @requires_graphics_hardware
  @requires_offscreen_canvas
  @parameterized({
    '': ([], True),
    'offscreen': (['-sOFFSCREENCANVAS_SUPPORT'], True),
    'pthread': (['-sPROXY_TO_PTHREAD', '-pthread', '-sOFFSCREEN_FRAMEBUFFER'], True),
    'pthread_ofb_main_loop': (['-sPROXY_TO_PTHREAD', '-pthread', '-sOFFSCREEN_FRAMEBUFFER', '-DTEST_EXPLICIT_CONTEXT_SWAP=1'], True),
    'pthread_ofb': (['-sPROXY_TO_PTHREAD', '-pthread', '-sOFFSCREEN_FRAMEBUFFER', '-DTEST_EXPLICIT_CONTEXT_SWAP=1'], False),
    'manual_css': (['-sPROXY_TO_PTHREAD', '-pthread', '-sOFFSCREEN_FRAMEBUFFER', '-DTEST_EXPLICIT_CONTEXT_SWAP=1', '-DTEST_MANUALLY_SET_ELEMENT_CSS_SIZE=1'], False),
  })
  def test_emscripten_animate_canvas_element_size(self, args, main_loop):
    cmd = ['-lGL', '-O3', '-g2', '--shell-file', test_file('canvas_animate_resize_shell.html'), '-sGL_DEBUG', '-sASSERTIONS'] + args
    if not self.is_2gb() and not self.is_4gb():
      # Thread profiler does not yet work with large pointers.
      # https://github.com/emscripten-core/emscripten/issues/21229
      cmd.append('--threadprofiler')
    if main_loop:
      cmd.append('-DTEST_EMSCRIPTEN_SET_MAIN_LOOP=1')
    self.btest_exit('canvas_animate_resize.c', cflags=cmd)

  # Tests the absolute minimum pthread-enabled application.
  @parameterized({
    '': ([],),
    'modularize': (['-sMODULARIZE', '-sEXPORT_NAME=MyModule', '--shell-file',
                    test_file('shell_that_launches_modularize.html')],),
  })
  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  def test_pthread_hello_thread(self, opts, modularize):
    self.btest_exit('pthread/hello_thread.c', cflags=['-pthread'] + modularize + opts)

  # Tests that a pthreads build of -sMINIMAL_RUNTIME works well in different build modes
  @parameterized({
    '': ([],),
    'modularize': (['-sMODULARIZE', '-sEXPORT_NAME=MyModule'],),
    'O3': (['-O3'],),
    'O3_modularize': (['-O3', '-sMODULARIZE', '-sEXPORT_NAME=MyModule'],),
    'O3_modularize_MINIMAL_RUNTIME_2': (['-O3', '-sMODULARIZE', '-sEXPORT_NAME=MyModule', '-sMINIMAL_RUNTIME=2'],),
  })
  def test_minimal_runtime_hello_thread(self, opts):
    self.btest_exit('pthread/hello_thread.c', cflags=['--closure=1', '-sMINIMAL_RUNTIME', '-pthread'] + opts)

  # Tests memory growth in pthreads mode, but still on the main thread.
  @parameterized({
    '': ([], 1),
    'growable_arraybuffers': (['-sGROWABLE_ARRAYBUFFERS', '-Wno-experimental'], 1),
    'proxy': (['-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'], 2),
  })
  @no_2gb('uses INITIAL_MEMORY')
  @no_4gb('uses INITIAL_MEMORY')
  @requires_growable_arraybuffers
  def test_pthread_growth_mainthread(self, cflags, pthread_pool_size):
    self.set_setting('PTHREAD_POOL_SIZE', pthread_pool_size)
    if '-sGROWABLE_ARRAYBUFFERS' not in cflags:
      self.cflags.append('-Wno-pthreads-mem-growth')
    self.btest_exit('pthread/test_pthread_memory_growth_mainthread.c', cflags=['-pthread', '-sALLOW_MEMORY_GROWTH', '-sINITIAL_MEMORY=32MB', '-sMAXIMUM_MEMORY=256MB'] + cflags)

  # Tests memory growth in a pthread.
  @parameterized({
    '': ([],),
    'growable_arraybuffers': (['-sGROWABLE_ARRAYBUFFERS', '-Wno-experimental'],),
    'assert': (['-sASSERTIONS'],),
    'proxy': (['-sPROXY_TO_PTHREAD'], 2),
    'minimal': (['-sMINIMAL_RUNTIME', '-sMODULARIZE', '-sEXPORT_NAME=MyModule'],),
  })
  @no_2gb('uses INITIAL_MEMORY')
  @no_4gb('uses INITIAL_MEMORY')
  @requires_growable_arraybuffers
  def test_pthread_growth(self, cflags, pthread_pool_size = 1):
    self.set_setting('PTHREAD_POOL_SIZE', pthread_pool_size)
    if '-sGROWABLE_ARRAYBUFFERS' not in cflags:
      self.cflags.append('-Wno-pthreads-mem-growth')
    self.btest_exit('pthread/test_pthread_memory_growth.c', cflags=['-pthread', '-sALLOW_MEMORY_GROWTH', '-sINITIAL_MEMORY=32MB', '-sMAXIMUM_MEMORY=256MB'] + cflags)

  # Tests that time in a pthread is relative to the main thread, so measurements
  # on different threads are still monotonic, as if checking a single central
  # clock.
  def test_pthread_reltime(self):
    self.btest_exit('pthread/test_pthread_reltime.cpp', cflags=['-pthread', '-sPTHREAD_POOL_SIZE'])

  # Tests that it is possible to load the main .js file of the application manually via mainScriptUrlOrBlob,
  # and still use pthreads.
  @parameterized({
    'blob': (False, True),
    'url': (False, False),
    'blob_es6': (True, True),
    'url_es6': (True, False),
  })
  @requires_es6_workers
  def test_mainScriptUrlOrBlob(self, es6, use_blob):
    # TODO: enable this with wasm, currently pthreads/atomics have limitations
    self.set_setting('EXIT_RUNTIME')
    js_name = 'hello_thread_with_loader.%s' % ('mjs' if es6 else 'js')
    if es6:
      self.cflags += ['-sEXPORT_ES6']
    if es6 and use_blob:
      create_file('loader.mjs', '''
        Module['locateFile'] = (path,_prefix) => path;
        let blob = await (await fetch('hello_thread_with_loader.mjs')).blob();
        Module['mainScriptUrlOrBlob'] = blob;
        (await import(URL.createObjectURL(blob))).default(Module);
      ''')
    elif use_blob:
      create_file('loader.mjs', '''
        let blob = await (await fetch('hello_thread_with_loader.js')).blob();
        Module['mainScriptUrlOrBlob'] = blob;
        var script = document.createElement('script');
        script.src = URL.createObjectURL(blob);
        document.body.appendChild(script);
      ''')
    elif es6:
      create_file('loader.mjs', '''
        Module['mainScriptUrlOrBlob'] = 'hello_thread_with_loader.mjs';
        (await import('./hello_thread_with_loader.mjs')).default(Module);
      ''')
    else:
      create_file('loader.mjs', '''
        var script = document.createElement('script');
        Module['mainScriptUrlOrBlob'] = 'hello_thread_with_loader.js';
        script.src = Module['mainScriptUrlOrBlob'];
        document.body.appendChild(script);
      ''')

    self.compile_btest('pthread/hello_thread.c', ['-pthread', '-o', 'out.js'], reporting=Reporting.JS_ONLY)

    # Now run the test with the JS file renamed and with its content
    # stored in Module['mainScriptUrlOrBlob'].
    shutil.move('out.js', js_name)
    shutil.copy(test_file('pthread/main_js_with_loader.html'), 'hello_thread_with_loader.html')
    self.run_browser('hello_thread_with_loader.html', '/report_result?exit:0')

  # Tests that SINGLE_FILE works as intended in generated HTML (with and without Worker)
  def test_single_file_html(self):
    self.btest('single_file_static_initializer.cpp', '19', cflags=['-sSINGLE_FILE'])
    self.assertExists('test.html')
    self.assertNotExists('test.js')
    self.assertNotExists('test.worker.js')
    self.assertNotExists('test.wasm')

  # Tests that SINGLE_FILE works as intended in generated HTML with MINIMAL_RUNTIME
  @also_with_wasm2js
  @parameterized({
    '': ([],),
    'O3': (['-O3'],),
  })
  def test_minimal_runtime_single_file_html(self, opts):
    self.btest('single_file_static_initializer.cpp', '19', cflags=opts + ['-sMINIMAL_RUNTIME', '-sSINGLE_FILE'])
    self.assertExists('test.html')
    self.assertNotExists('test.js')
    self.assertNotExists('test.wasm')
    self.assertNotExists('test.asm.js')
    self.assertNotExists('test.js')
    self.assertNotExists('test.worker.js')

  # Tests that SINGLE_FILE works when built with ENVIRONMENT=web and Closure enabled (#7933)
  def test_single_file_in_web_environment_with_closure(self):
    self.btest_exit('minimal_hello.c', cflags=['-sSINGLE_FILE', '-sENVIRONMENT=web', '-O2', '--closure=1'])

  # Tests that SINGLE_FILE works as intended with locateFile
  @also_with_wasm2js
  def test_single_file_locate_file(self):
    self.compile_btest('browser_test_hello_world.c', ['-o', 'test.js', '-sSINGLE_FILE'])

    create_file('test.html', '''
      <script>
        var Module = {
          locateFile: function (path) {
            if (path.startsWith('data:')) {
              throw new Error('Unexpected data URI.');
            }

            return path;
          }
        };
      </script>
      <script src="test.js"></script>
    ''')

    self.run_browser('test.html', '/report_result?0')

  # Tests that SINGLE_FILE works as intended in a Worker in JS output
  def test_single_file_worker_js(self):
    self.btest_exit('browser_test_hello_world.c', cflags=['-sSINGLE_FILE'], run_in_worker=True)

  # Tests that pthreads code works as intended in a Worker. That is, a pthreads-using
  # program can run either on the main thread (normal tests) or when we start it in
  # a Worker in this test (in that case, both the main application thread and the worker
  # threads are all inside Web Workers).
  @parameterized({
    '': ([],),
    'limited_env': (['-sENVIRONMENT=worker'],),
  })
  def test_pthreads_started_in_worker(self, args):
    self.btest_exit('pthread/test_pthread_atomics.c', cflags=['-o', 'test.js', '-pthread', '-sPTHREAD_POOL_SIZE=8'] + args, run_in_worker=True)

  def test_access_file_after_heap_resize(self):
    create_file('test.txt', 'hello from file')
    self.btest_exit('access_file_after_heap_resize.c', cflags=['-sALLOW_MEMORY_GROWTH', '--preload-file', 'test.txt'])

    # with separate file packager invocation
    self.run_process([FILE_PACKAGER, 'data.data', '--preload', 'test.txt', '--js-output=' + 'data.js'])
    self.btest_exit('access_file_after_heap_resize.c', cflags=['-sALLOW_MEMORY_GROWTH', '--pre-js', 'data.js', '-sFORCE_FILESYSTEM'])

  def test_unicode_html_shell(self):
    create_file('main.c', r'''
      int main() {
        return 0;
      }
    ''')
    create_file('shell.html', read_file(path_from_root('src/shell.html')).replace('Emscripten-Generated Code', 'Emscripten-Generated Emoji 😅'))
    self.btest_exit('main.c', cflags=['--shell-file', 'shell.html'])

  # Tests the functionality of the emscripten_thread_sleep() function.
  def test_emscripten_thread_sleep(self):
    self.btest_exit('pthread/emscripten_thread_sleep.c', cflags=['-pthread'])

  # Tests that Emscripten-compiled applications can be run from a relative path in browser that is different than the address of the current page
  def test_browser_run_from_different_directory(self):
    self.compile_btest('browser_test_hello_world.c', ['-o', 'test.html', '-O3'])

    ensure_dir('subdir')
    shutil.move('test.js', Path('subdir/test.js'))
    shutil.move('test.wasm', Path('subdir/test.wasm'))
    src = read_file('test.html')
    # Make sure JS is loaded from subdirectory
    create_file('test-subdir.html', src.replace('test.js', 'subdir/test.js'))
    self.run_browser('test-subdir.html', '/report_result?0')

  # Similar to `test_browser_run_from_different_directory`, but asynchronous because of `-sMODULARIZE`
  def test_browser_run_from_different_directory_async(self):
    # compile the code with the modularize feature and the preload-file option enabled
    self.compile_btest('browser_test_hello_world.c', ['-o', 'test.js', '-O3', '-sMODULARIZE'])
    ensure_dir('subdir')
    shutil.move('test.js', Path('subdir/test.js'))
    shutil.move('test.wasm', Path('subdir/test.wasm'))
    # Make sure JS is loaded from subdirectory
    create_file('test-subdir.html', '''
      <script src="subdir/test.js"></script>
      <script>
        Module();
      </script>
    ''')
    self.run_browser('test-subdir.html', '/report_result?0')

  # Similar to `test_browser_run_from_different_directory`, but
  # also also we eval the initial code, so currentScript is not present. That prevents us
  # from finding the file in a subdir, but here we at least check we do not regress compared to the
  # normal case of finding in the current dir.
  # test both modularize (and creating an instance) and modularize-instance
  # (which creates by itself)
  @parameterized({
    '': ([], ['-sMODULARIZE'], 'Module();'),
    'subdir': (['subdir'], ['-sMODULARIZE'], 'Module();'),
  })
  def test_browser_modularize_no_current_script(self, path, args, creation):
    filesystem_path = os.path.join('.', *path)
    ensure_dir(filesystem_path)
    # compile the code with the modularize feature and the preload-file option enabled
    self.compile_btest('browser_test_hello_world.c', ['-o', 'test.js'] + args)
    shutil.move('test.js', Path(filesystem_path, 'test.js'))
    shutil.move('test.wasm', Path(filesystem_path, 'test.wasm'))
    create_file(Path(filesystem_path, 'test.html'), '''
      <script>
        setTimeout(async () => {
          let response = await fetch('test.js');
          let text = await response.text();
          eval(text);
          %s
        }, 1);
      </script>
    ''' % creation)
    self.run_browser('/'.join(path + ['test.html']), '/report_result?0')

  def test_emscripten_request_animation_frame(self):
    self.btest_exit('emscripten_request_animation_frame.c')

  def test_emscripten_request_animation_frame_loop(self):
    self.btest_exit('emscripten_request_animation_frame_loop.c')

  def test_request_animation_frame(self):
    self.btest_exit('test_request_animation_frame.c')

  @requires_shared_array_buffer
  def test_emscripten_set_timeout(self):
    self.btest_exit('emscripten_set_timeout.c', cflags=['-pthread', '-sPROXY_TO_PTHREAD'])

  @requires_shared_array_buffer
  def test_emscripten_set_timeout_loop(self):
    self.btest_exit('emscripten_set_timeout_loop.c', cflags=['-pthread', '-sPROXY_TO_PTHREAD'])

  def test_emscripten_set_immediate(self):
    self.btest_exit('emscripten_set_immediate.c')

  def test_emscripten_set_immediate_loop(self):
    self.btest_exit('emscripten_set_immediate_loop.c')

  @requires_shared_array_buffer
  def test_emscripten_set_interval(self):
    self.btest_exit('emscripten_set_interval.c', cflags=['-pthread', '-sPROXY_TO_PTHREAD'])

  # Test emscripten_performance_now() and emscripten_date_now()
  @requires_shared_array_buffer
  def test_emscripten_performance_now(self):
    self.btest('emscripten_performance_now.c', '0', cflags=['-pthread', '-sPROXY_TO_PTHREAD'])

  def test_embind_with_pthreads(self):
    self.btest_exit('embind/test_pthreads.cpp', cflags=['-lembind', '-pthread', '-sPTHREAD_POOL_SIZE=2'])

  @parameterized({
    'asyncify': (['-sASYNCIFY=1'],),
    'jspi': (['-sASYNCIFY=2', '-Wno-experimental'],),
  })
  def test_embind(self, args):
    if is_jspi(args) and not is_chrome():
      self.skipTest(f'Current browser ({get_browser()}) does not support JSPI. Only chromium-based browsers ({CHROMIUM_BASED_BROWSERS}) support JSPI today.')
    if is_jspi(args) and self.is_wasm64():
      self.skipTest('_emval_await fails')

    self.btest('embind_with_asyncify.cpp', '1', cflags=['-lembind'] + args)

  # Test emscripten_console_log(), emscripten_console_warn() and emscripten_console_error()
  def test_emscripten_console_log(self):
    self.btest_exit('emscripten_console_log.c', cflags=['--pre-js', test_file('emscripten_console_log_pre.js')])

  def test_emscripten_throw_number(self):
    self.btest('emscripten_throw_number.c', '0', cflags=['--pre-js', test_file('emscripten_throw_number_pre.js')])

  def test_emscripten_throw_string(self):
    self.btest('emscripten_throw_string.c', '0', cflags=['--pre-js', test_file('emscripten_throw_string_pre.js')])

  # Tests that Closure run in combination with -sENVIRONMENT=web mode works with a minimal console.log() application
  def test_closure_in_web_only_target_environment_console_log(self):
    self.btest_exit('minimal_hello.c', cflags=['-sENVIRONMENT=web', '-O3', '--closure=1'])

  # Tests that Closure run in combination with -sENVIRONMENT=web mode works with a small WebGL application
  @requires_graphics_hardware
  def test_closure_in_web_only_target_environment_webgl(self):
    self.btest_exit('webgl_draw_triangle.c', cflags=['-lGL', '-sENVIRONMENT=web', '-O3', '--closure=1'])

  @requires_wasm2js
  @parameterized({
    '': ([],),
    'minimal': (['-sMINIMAL_RUNTIME'],),
  })
  def test_no_declare_asm_module_exports_wasm2js(self, args):
    # TODO(sbc): Fix closure warnings with MODULARIZE + WASM=0
    self.ldflags.append('-Wno-error=closure')
    self.btest_exit('declare_asm_module_exports.c', cflags=['-sDECLARE_ASM_MODULE_EXPORTS=0', '-sENVIRONMENT=web', '-O3', '--closure=1', '-sWASM=0'] + args)

  @parameterized({
    '': ([],),
    'strict_js': (['-sSTRICT_JS'],),
    'minimal_runtime': (['-sMINIMAL_RUNTIME=1'],),
    'minimal_runtime_2': (['-sMINIMAL_RUNTIME=2'],),
  })
  def test_no_declare_asm_module_exports(self, args):
    self.btest_exit('declare_asm_module_exports.c', cflags=['-sDECLARE_ASM_MODULE_EXPORTS=0', '-sENVIRONMENT=web', '-O3', '--closure=1'] + args)

  # Tests that the different code paths in src/shell_minimal_runtime.html all work ok.
  @parameterized({
    '': ([],),
    'modularize': (['-sMODULARIZE'],),
  })
  @also_with_wasm2js
  def test_minimal_runtime_loader_shell(self, args):
    args = ['-sMINIMAL_RUNTIME=2']
    self.btest_exit('minimal_hello.c', cflags=args)

  # Tests that -sMINIMAL_RUNTIME works well in different build modes
  @parameterized({
    '': ([],),
    'streaming_compile': (['-sMINIMAL_RUNTIME_STREAMING_WASM_COMPILATION', '-sENVIRONMENT=web', '--closure=1'],),
    'streaming_inst': (['-sMINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION', '-sENVIRONMENT=web', '--closure=1'],),
  })
  def test_minimal_runtime_hello_world(self, args):
    self.btest_exit('small_hello_world.c', cflags=args + ['-sMINIMAL_RUNTIME'])

  # Tests emscripten_unwind_to_js_event_loop() behavior
  def test_emscripten_unwind_to_js_event_loop(self):
    self.btest_exit('test_emscripten_unwind_to_js_event_loop.c')

  @requires_wasm2js
  @parameterized({
    '': ([],),
    'minimal': (['-sMINIMAL_RUNTIME'],),
  })
  def test_wasm2js_fallback(self, args):
    self.set_setting('EXIT_RUNTIME')
    self.compile_btest('small_hello_world.c', ['-sWASM=2', '-o', 'test.html'] + args)

    # First run with WebAssembly support enabled
    # Move the Wasm2js fallback away to test it is not accidentally getting loaded.
    os.rename('test.wasm.js', 'test.wasm.js.unused')
    self.run_browser('test.html', '/report_result?exit:0')
    os.rename('test.wasm.js.unused', 'test.wasm.js')

    # Then disable WebAssembly support in VM, and try again.. Should still work with Wasm2JS fallback.
    html = read_file('test.html')
    html = html.replace('<body>', '<body><script>delete WebAssembly;</script>')
    create_file('test.html', html)
    os.remove('test.wasm') # Also delete the Wasm file to test that it is not attempted to be loaded.
    self.run_browser('test.html', '/report_result?exit:0')

  @requires_wasm2js
  @parameterized({
    '': ([],),
    'minimal': (['-sMINIMAL_RUNTIME'],),
  })
  def test_wasm2js_fallback_on_wasm_compilation_failure(self, args):
    self.set_setting('EXIT_RUNTIME')
    self.compile_btest('small_hello_world.c', ['-sWASM=2', '-o', 'test.html'] + args)

    # Run without the .wasm.js file present: with Wasm support, the page should still run
    os.rename('test.wasm.js', 'test.wasm.js.unused')
    self.run_browser('test.html', '/report_result?exit:0')

    # Restore the .wasm.js file, then corrupt the .wasm file, that should trigger the Wasm2js fallback to run
    os.rename('test.wasm.js.unused', 'test.wasm.js')
    shutil.copy('test.js', 'test.wasm')
    self.run_browser('test.html', '/report_result?exit:0')

  def test_system(self):
    self.btest_exit('test_system.c')

  # Tests the hello_wasm_worker.c documentation example code.
  @also_with_minimal_runtime
  def test_wasm_worker_hello(self):
    self.btest_exit('wasm_worker/hello_wasm_worker.c', cflags=['-sWASM_WORKERS', '-sENVIRONMENT=web'])

  @requires_es6_workers
  def test_wasm_worker_hello_export_es6(self):
    self.btest_exit('wasm_worker/hello_wasm_worker.c', cflags=['-sWASM_WORKERS', '-sENVIRONMENT=web', '-sEXPORT_ES6'])

  def test_wasm_worker_hello_minimal_runtime_2(self):
    self.btest_exit('wasm_worker/hello_wasm_worker.c', cflags=['-sWASM_WORKERS', '-sMINIMAL_RUNTIME=2'])

  # Tests Wasm Workers build in Wasm2JS mode.
  @requires_wasm2js
  @also_with_minimal_runtime
  def test_wasm_worker_hello_wasm2js(self):
    self.btest_exit('wasm_worker/hello_wasm_worker.c', cflags=['-sWASM_WORKERS', '-sWASM=0'])

  # Tests the WASM_WORKERS=2 build mode, which embeds the Wasm Worker bootstrap JS script file to the main JS file.
  @also_with_minimal_runtime
  def test_wasm_worker_hello_embedded(self):
    self.btest_exit('wasm_worker/hello_wasm_worker.c', cflags=['-sWASM_WORKERS=2'])

  # Tests that it is possible to call emscripten_futex_wait() in Wasm Workers.
  @parameterized({
    '': ([],),
    'pthread': (['-pthread'],),
  })
  def test_wasm_worker_futex_wait(self, args):
    self.btest('wasm_worker/wasm_worker_futex_wait.c', expected='0', cflags=['-sWASM_WORKERS=1', '-sASSERTIONS'] + args)

  # Tests Wasm Worker thread stack setup
  @also_with_minimal_runtime
  @parameterized({
    '0': (0,),
    '1': (1,),
    '2': (2,),
  })
  def test_wasm_worker_thread_stack(self, mode):
    self.btest('wasm_worker/thread_stack.c', expected='0', cflags=['-sWASM_WORKERS', f'-sSTACK_OVERFLOW_CHECK={mode}'])

  # Tests emscripten_malloc_wasm_worker() and emscripten_current_thread_is_wasm_worker() functions
  @also_with_minimal_runtime
  def test_wasm_worker_malloc(self):
    self.btest_exit('wasm_worker/malloc_wasm_worker.c', cflags=['-sWASM_WORKERS'])

  # Tests Wasm Worker+pthreads simultaneously
  @also_with_minimal_runtime
  def test_wasm_worker_and_pthreads(self):
    self.btest('wasm_worker/wasm_worker_and_pthread.c', expected='0', cflags=['-sWASM_WORKERS', '-pthread'])

  # Tests emscripten_wasm_worker_self_id() function
  @also_with_minimal_runtime
  def test_wasm_worker_self_id(self):
    self.btest('wasm_worker/wasm_worker_self_id.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests direct Wasm Assembly .S file based TLS variables in Wasm Workers
  @also_with_minimal_runtime
  def test_wasm_worker_tls_wasm_assembly(self):
    self.btest('wasm_worker/wasm_worker_tls_wasm_assembly.c',
               expected='42', cflags=['-sWASM_WORKERS', test_file('wasm_worker/wasm_worker_tls_wasm_assembly.S')])

  # Tests C++11 keyword thread_local for TLS in Wasm Workers
  @also_with_minimal_runtime
  def test_wasm_worker_cpp11_thread_local(self):
    self.btest('wasm_worker/cpp11_thread_local.cpp', expected='42', cflags=['-sWASM_WORKERS'])

  # Tests C11 keyword _Thread_local for TLS in Wasm Workers
  @also_with_minimal_runtime
  def test_wasm_worker_c11__Thread_local(self):
    self.btest('wasm_worker/c11__Thread_local.c', expected='42', cflags=['-sWASM_WORKERS'])

  # Tests GCC specific extension keyword __thread for TLS in Wasm Workers
  @also_with_minimal_runtime
  def test_wasm_worker_gcc___thread(self):
    self.btest('wasm_worker/gcc___Thread.c', expected='42', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_wasm_worker_sleep()
  @also_with_minimal_runtime
  def test_wasm_worker_sleep(self):
    self.btest('wasm_worker/wasm_worker_sleep.c', expected='1', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_terminate_wasm_worker()
  @also_with_minimal_runtime
  def test_wasm_worker_terminate(self):
    self.btest_exit('wasm_worker/terminate_wasm_worker.c', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_terminate_all_wasm_workers()
  @also_with_minimal_runtime
  def test_wasm_worker_terminate_all(self):
    self.set_setting('WASM_WORKERS')
    # Test uses the dynCall library function in its EM_ASM code
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', ['$dynCall'])
    self.btest('wasm_worker/terminate_all_wasm_workers.c', expected='0')

  # Tests emscripten_wasm_worker_post_function_*() API
  @also_with_minimal_runtime
  def test_wasm_worker_post_function(self):
    self.btest('wasm_worker/post_function.c', expected='8', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_wasm_worker_post_function_*() API and EMSCRIPTEN_WASM_WORKER_ID_PARENT
  # to send a message back from Worker to its parent thread.
  @also_with_minimal_runtime
  def test_wasm_worker_post_function_to_main_thread(self):
    self.btest('wasm_worker/post_function_to_main_thread.c', expected='10', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_navigator_hardware_concurrency() and emscripten_atomics_is_lock_free()
  @also_with_minimal_runtime
  def test_wasm_worker_hardware_concurrency_is_lock_free(self):
    self.btest('wasm_worker/hardware_concurrency_is_lock_free.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_atomic_wait_u32() and emscripten_atomic_notify() functions.
  @also_with_minimal_runtime
  def test_wasm_worker_wait32_notify(self):
    self.btest('atomic/test_wait32_notify.c', expected='3', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_atomic_wait_u64() and emscripten_atomic_notify() functions.
  @also_with_minimal_runtime
  def test_wasm_worker_wait64_notify(self):
    self.btest('atomic/test_wait64_notify.c', expected='3', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_atomic_wait_async() function.
  @also_with_minimal_runtime
  def test_wasm_worker_wait_async(self):
    self.btest_exit('atomic/test_wait_async.c', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_atomic_cancel_wait_async() function.
  @also_with_minimal_runtime
  def test_wasm_worker_cancel_wait_async(self):
    self.btest('wasm_worker/cancel_wait_async.c', expected='1', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_atomic_cancel_all_wait_asyncs() function.
  @also_with_minimal_runtime
  def test_wasm_worker_cancel_all_wait_asyncs(self):
    self.btest('wasm_worker/cancel_all_wait_asyncs.c', expected='1', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_atomic_cancel_all_wait_asyncs_at_address() function.
  @also_with_minimal_runtime
  def test_wasm_worker_cancel_all_wait_asyncs_at_address(self):
    self.btest('wasm_worker/cancel_all_wait_asyncs_at_address.c', expected='1', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_lock_init(), emscripten_lock_waitinf_acquire() and emscripten_lock_release()
  @also_with_minimal_runtime
  def test_wasm_worker_lock_waitinf(self):
    self.btest('wasm_worker/lock_waitinf_acquire.c', expected='4000', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_lock_wait_acquire() and emscripten_lock_try_acquire() in Worker.
  @also_with_minimal_runtime
  def test_wasm_worker_lock_wait(self):
    self.btest('wasm_worker/lock_wait_acquire.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_lock_wait_acquire() between two Wasm Workers.
  @also_with_minimal_runtime
  def test_wasm_worker_lock_wait2(self):
    self.btest('wasm_worker/lock_wait_acquire2.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_lock_async_acquire() function.
  @also_with_minimal_runtime
  @flaky('https://github.com/emscripten-core/emscripten/issues/25270')
  def test_wasm_worker_lock_async_acquire(self):
    self.btest_exit('wasm_worker/lock_async_acquire.c', cflags=['--closure=1', '-sWASM_WORKERS'])

  # Tests emscripten_lock_async_acquire() function when lock is acquired both synchronously and asynchronously.
  @also_with_minimal_runtime
  def test_wasm_worker_lock_async_and_sync_acquire(self):
    self.btest_exit('wasm_worker/lock_async_and_sync_acquire.c', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_lock_busyspin_wait_acquire() in Worker and main thread.
  @also_with_minimal_runtime
  def test_wasm_worker_lock_busyspin_wait(self):
    self.btest('wasm_worker/lock_busyspin_wait_acquire.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_lock_busyspin_waitinf_acquire() in Worker and main thread.
  @also_with_minimal_runtime
  def test_wasm_worker_lock_busyspin_waitinf(self):
    self.btest('wasm_worker/lock_busyspin_waitinf_acquire.c', expected='1', cflags=['-sWASM_WORKERS'])

  # Tests that proxied JS functions cannot be called from Wasm Workers
  @also_with_minimal_runtime
  def test_wasm_worker_no_proxied_js_functions(self):
    self.set_setting('WASM_WORKERS')
    self.set_setting('ASSERTIONS')
    # Test uses the dynCall library function in its EM_ASM code
    self.set_setting('DEFAULT_LIBRARY_FUNCS_TO_INCLUDE', ['$dynCall'])
    self.btest('wasm_worker/no_proxied_js_functions.c', expected='0',
               cflags=['--js-library', test_file('wasm_worker/no_proxied_js_functions.js')])

  # Tests emscripten_semaphore_init(), emscripten_semaphore_waitinf_acquire() and emscripten_semaphore_release()
  @also_with_minimal_runtime
  def test_wasm_worker_semaphore_waitinf_acquire(self):
    self.btest('wasm_worker/semaphore_waitinf_acquire.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests emscripten_semaphore_try_acquire() on the main thread
  @also_with_minimal_runtime
  def test_wasm_worker_semaphore_try_acquire(self):
    self.btest('wasm_worker/semaphore_try_acquire.c', expected='0', cflags=['-sWASM_WORKERS'])

  # Tests that calling any proxied function in a Wasm Worker will abort at runtime when ASSERTIONS are enabled.
  def test_wasm_worker_proxied_function(self):
    error_msg = "abort:Assertion failed: Attempted to call proxied function '_proxied_js_function' in a Wasm Worker, but in Wasm Worker enabled builds, proxied function architecture is not available!"
    # Test that program aborts in ASSERTIONS-enabled builds
    self.btest('wasm_worker/proxied_function.c', expected=error_msg, cflags=['--js-library', test_file('wasm_worker/proxied_function.js'), '-sWASM_WORKERS', '-sASSERTIONS'])
    # Test that code does not crash in ASSERTIONS-disabled builds
    self.btest('wasm_worker/proxied_function.c', expected='0', cflags=['--js-library', test_file('wasm_worker/proxied_function.js'), '-sWASM_WORKERS', '-sASSERTIONS=0'])

  @no_firefox('no 4GB support yet')
  @no_2gb('uses MAXIMUM_MEMORY')
  @no_4gb('uses MAXIMUM_MEMORY')
  def test_4gb(self):
    # TODO Convert to an actual browser test when it reaches stable.
    # For now, keep this in browser as this suite runs serially, which
    # means we don't compete for memory with anything else (and run it
    # at the very very end, to reduce the risk of it OOM-killing the
    # browser).

    # test that we can allocate in the 2-4GB range, if we enable growth and
    # set the max appropriately
    self.cflags += ['-O2', '-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=4GB']
    self.do_run_in_out_file_test('browser/test_4GB.cpp')

  # Tests that emmalloc supports up to 4GB Wasm heaps.
  @no_firefox('no 4GB support yet')
  @requires_safari_version(170601, 'Assertion failed: emscripten_get_heap_size() == MAX_HEAP') # Fails in Safari 17.6 (17618.3.11.11.7, 17618)
  @no_4gb('uses MAXIMUM_MEMORY')
  def test_emmalloc_4gb(self):
    # For now, keep this in browser as this suite runs serially, which
    # means we don't compete for memory with anything else (and run it
    # at the very very end, to reduce the risk of it OOM-killing the
    # browser).
    self.btest_exit('test_mem_growth.c', cflags=['-sMALLOC=emmalloc', '-sABORTING_MALLOC=0', '-sALLOW_MEMORY_GROWTH=1', '-sMAXIMUM_MEMORY=4GB'])

  # Test that it is possible to malloc() a huge 3GB memory block in 4GB mode using emmalloc.
  # Also test emmalloc-memvalidate and emmalloc-memvalidate-verbose build configurations.
  @no_firefox('no 4GB support yet')
  @no_2gb('not enough space to run in this mode')
  @parameterized({
    '': (['-sMALLOC=emmalloc'],),
    'debug': (['-sMALLOC=emmalloc-debug'],),
    'memvalidate': (['-sMALLOC=emmalloc-memvalidate'],),
    'memvalidate_verbose': (['-sMALLOC=emmalloc-memvalidate-verbose'],),
  })
  def test_emmalloc_3gb(self, args):
    if self.is_4gb():
      self.set_setting('MAXIMUM_MEMORY', '8GB')
    else:
      self.set_setting('MAXIMUM_MEMORY', '4GB')
    self.btest_exit('alloc_3gb.c', cflags=['-sALLOW_MEMORY_GROWTH=1'] + args)

  # Test that it is possible to malloc() a huge 3GB memory block in 4GB mode using dlmalloc.
  @no_firefox('no 4GB support yet')
  @no_2gb('not enough space tp run in this mode')
  def test_dlmalloc_3gb(self):
    if self.is_4gb():
      self.set_setting('MAXIMUM_MEMORY', '8GB')
    else:
      self.set_setting('MAXIMUM_MEMORY', '4GB')
    self.btest_exit('alloc_3gb.c', cflags=['-sMALLOC=dlmalloc', '-sALLOW_MEMORY_GROWTH=1'])

  @no_wasm64()
  @parameterized({
    # the fetch backend works even on the main thread: we proxy to a background
    # thread and busy-wait
    # this test requires one thread per fetch backend, so updates to the test
    # will require bumping this
    'main_thread': (['-sPTHREAD_POOL_SIZE=5'],),
    # using proxy_to_pthread also works, of course
    'proxy_to_pthread': (['-sPROXY_TO_PTHREAD', '-DPROXYING'],),
    # using BigInt support affects the ABI, and should not break things. (this
    # could be tested on either thread; do the main thread for simplicity)
    'bigint': (['-sPTHREAD_POOL_SIZE=5', '-sWASM_BIGINT'],),
  })
  def test_wasmfs_fetch_backend_threaded(self, args):
    create_file('data.dat', 'hello, fetch')
    create_file('small.dat', 'hello')
    create_file('test.txt', 'fetch 2')
    delete_dir('subdir')
    ensure_dir('subdir')
    create_file('subdir/backendfile', 'file 1')
    create_file('subdir/backendfile2', 'file 2')
    self.btest_exit('wasmfs/wasmfs_fetch.c',
                    cflags=['-sWASMFS', '-pthread', '-sPROXY_TO_PTHREAD',
                               '-sFORCE_FILESYSTEM', '-lfetchfs.js',
                               '--js-library', test_file('wasmfs/wasmfs_fetch.js')] + args)

  @no_firefox('no OPFS support yet')
  @no_wasm64()
  @parameterized({
    '': (['-pthread', '-sPROXY_TO_PTHREAD'],),
    'jspi': (['-Wno-experimental', '-sASYNCIFY=2'],),
    'jspi_wasm_bigint': (['-Wno-experimental', '-sASYNCIFY=2', '-sWASM_BIGINT'],),
  })
  @no_safari('TODO: Fails with abort:Assertion failed: err == 0') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_wasmfs_opfs(self, args):
    if '-sASYNCIFY=2' in args:
      self.require_jspi()
    test = test_file('wasmfs/wasmfs_opfs.c')
    args = ['-sWASMFS', '-O3'] + args
    self.btest_exit(test, cflags=args + ['-DWASMFS_SETUP'])
    self.btest_exit(test, cflags=args + ['-DWASMFS_RESUME'])

  @no_firefox('no OPFS support yet')
  @no_safari('TODO: Fails with exception:Did not get expected EIO when unlinking file') # Fails in Safari 17.6 (17618.3.11.11.7, 17618) and Safari 26.0.1 (21622.1.22.11.15)
  def test_wasmfs_opfs_errors(self):
    test = test_file('wasmfs/wasmfs_opfs_errors.c')
    postjs = test_file('wasmfs/wasmfs_opfs_errors_post.js')
    args = ['-sWASMFS', '-pthread', '-sPROXY_TO_PTHREAD', '--post-js', postjs]
    self.btest(test, cflags=args, expected='0')

  @no_firefox('no 4GB support yet')
  def test_emmalloc_memgrowth(self):
    if not self.is_4gb():
      self.set_setting('MAXIMUM_MEMORY', '4GB')
    self.btest_exit('emmalloc_memgrowth.cpp', cflags=['-sMALLOC=emmalloc', '-sALLOW_MEMORY_GROWTH=1', '-sABORTING_MALLOC=0', '-sASSERTIONS=2', '-sMINIMAL_RUNTIME=1'])

  @no_firefox('no 4GB support yet')
  @no_2gb('uses MAXIMUM_MEMORY')
  @no_4gb('uses MAXIMUM_MEMORY')
  def test_2gb_fail(self):
    # TODO Convert to an actual browser test when it reaches stable.
    #      For now, keep this in browser as this suite runs serially, which
    #      means we don't compete for memory with anything else (and run it
    #      at the very very end, to reduce the risk of it OOM-killing the
    #      browser).

    # test that growth doesn't go beyond 2GB without the max being set for that,
    # and that we can catch an allocation failure exception for that
    self.cflags += ['-O2', '-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=2GB']
    self.do_run_in_out_file_test('browser/test_2GB_fail.cpp')

  @no_firefox('no 4GB support yet')
  @no_2gb('uses MAXIMUM_MEMORY')
  @no_4gb('uses MAXIMUM_MEMORY')
  def test_4gb_fail(self):
    # TODO Convert to an actual browser test when it reaches stable.
    #      For now, keep this in browser as this suite runs serially, which
    #      means we don't compete for memory with anything else (and run it
    #      at the very very end, to reduce the risk of it OOM-killing the
    #      browser).

    # test that we properly report an allocation error that would overflow over
    # 4GB.
    self.set_setting('MAXIMUM_MEMORY', '4GB')
    self.cflags += ['-O2', '-sALLOW_MEMORY_GROWTH', '-sABORTING_MALLOC=0', '-sASSERTIONS']
    self.do_run_in_out_file_test('browser/test_4GB_fail.cpp')

  # Tests that Emscripten-compiled applications can be run when a slash in the URL query or fragment of the js file
  def test_browser_run_with_slash_in_query_and_hash(self):
    self.compile_btest('browser_test_hello_world.c', ['-o', 'test.html', '-O0'])
    src = open('test.html').read()
    # Slash in query
    create_file('test-query.html', src.replace('test.js', 'test.js?type=pass/fail'))
    self.run_browser('test-query.html', '/report_result?0')
    # Slash in fragment
    create_file('test-hash.html', src.replace('test.js', 'test.js#pass/fail'))
    self.run_browser('test-hash.html', '/report_result?0')
    # Slash in query and fragment
    create_file('test-query-hash.html', src.replace('test.js', 'test.js?type=pass/fail#pass/fail'))
    self.run_browser('test-query-hash.html', '/report_result?0')

  @disabled("only run this manually, to test for race conditions")
  @parameterized({
    'normal': ([],),
    'assertions': (['-sASSERTIONS'],),
  })
  def test_manual_pthread_proxy_hammer(self, args):
    # the specific symptom of the hang that was fixed is that the test hangs
    # at some point, using 0% CPU. often that occured in 0-200 iterations, but
    # you may want to adjust "ITERATIONS".
    self.btest_exit('pthread/test_pthread_proxy_hammer.cpp',
                    cflags=['-pthread', '-O2', '-sPROXY_TO_PTHREAD',
                               '-DITERATIONS=1024', '-g1'] + args,
                    timeout=10000)

  def test_assert_failure(self):
    self.btest('test_assert_failure.c', 'abort:Assertion failed: false && "this is a test"')

  @no_safari('TODO: Fails with report_result?exception:rejected!') # Fails in Safari 17.6 (17618.3.11.11.7, 17618), Safari 26.0.1 (21622.1.22.11.15)
  def test_pthread_unhandledrejection(self):
    # Check that an unhandled promise rejection is propagated to the main thread
    # as an error. This test is failing if it hangs!
    self.btest('pthread/test_pthread_unhandledrejection.c',
               cflags=['-pthread', '-sPROXY_TO_PTHREAD', '--post-js',
                          test_file('pthread/test_pthread_unhandledrejection.post.js')],
               # Firefox and Chrome report this slightly differently
               expected=['exception:Uncaught rejected!', 'exception:uncaught exception: rejected!'])

  def test_pthread_key_recreation(self):
    self.btest_exit('pthread/test_pthread_key_recreation.c', cflags=['-pthread', '-sPTHREAD_POOL_SIZE=1'])

  def test_full_js_library_strict(self):
    self.btest_exit('hello_world.c', cflags=['-sINCLUDE_FULL_LIBRARY', '-sSTRICT_JS'])

  # Tests the AudioWorklet demo
  @parameterized({
    '': ([],),
    'with_fs': (['--preload-file', test_file('hello_world.c') + '@/'],),
    'closure': (['--closure', '1', '-Oz'],),
    'asyncify': (['-sASYNCIFY'],),
    'pthreads': (['-pthread', '-sPTHREAD_POOL_SIZE=2'],),
    'pthreads_and_closure': (['-pthread', '--closure', '1', '-Oz'],),
    'minimal_runtime': (['-sMINIMAL_RUNTIME'],),
    'minimal_runtime_pthreads_and_closure': (['-sMINIMAL_RUNTIME', '-pthread', '--closure', '1', '-Oz'],),
    'pthreads_es6': (['-pthread', '-sPTHREAD_POOL_SIZE=2', '-sEXPORT_ES6'],),
    'es6': (['-sEXPORT_ES6'],),
    'strict': (['-sSTRICT'],),
    'audio_params_disabled': (['-sAUDIO_WORKLET_SUPPORT_AUDIO_PARAMS=0'],),
  })
  @requires_sound_hardware
  @requires_es6_workers
  def test_audio_worklet(self, args):
    self.btest_exit('webaudio/audioworklet.c', cflags=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-DTEST_AND_EXIT'] + args)

  # Tests that audioworklets and workers can be used at the same time
  # Note: doesn't need audio hardware (and has no AW code that tests 2GB or wasm64)
  def test_audio_worklet_worker(self):
    self.btest_exit('webaudio/audioworklet_worker.c', cflags=['-sAUDIO_WORKLET', '-sWASM_WORKERS'])

  # Tests that posting functions between the main thread and the audioworklet thread works
  @parameterized({
    '': ([],),
    'closure': (['--closure', '1', '-Oz'],),
  })
  # Note: doesn't need audio hardware (and has no AW code that tests 2GB or wasm64)
  def test_audio_worklet_post_function(self, args):
    self.btest_exit('webaudio/audioworklet_post_function.c', cflags=['-sAUDIO_WORKLET', '-sWASM_WORKERS'] + args)

  @parameterized({
    '': ([],),
    'closure': (['--closure', '1', '-Oz'],),
  })
  @requires_sound_hardware
  def test_audio_worklet_modularize(self, args):
    self.btest_exit('webaudio/audioworklet.c', cflags=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-sMODULARIZE=1', '-sEXPORT_NAME=MyModule', '--shell-file', test_file('shell_that_launches_modularize.html'), '-DTEST_AND_EXIT'] + args)

  # Tests an AudioWorklet with multiple stereo inputs mixing in the processor
  # via a varying parameter to a single stereo output (touching all of the API
  # copying from structs)
  @parameterized({
    '': ([],),
    'minimal_with_closure': (['-sMINIMAL_RUNTIME', '--closure=1', '-Oz'],),
  })
  @requires_sound_hardware
  def test_audio_worklet_params_mixing(self, args):
    os.mkdir('audio_files')
    shutil.copy(test_file('webaudio/audio_files/emscripten-beat.mp3'), 'audio_files/')
    shutil.copy(test_file('webaudio/audio_files/emscripten-bass.mp3'), 'audio_files/')
    self.btest_exit('webaudio/audioworklet_params_mixing.c', cflags=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-DTEST_AND_EXIT'] + args)

  # Tests AudioWorklet with emscripten_lock_busyspin_wait_acquire() and friends
  @requires_sound_hardware
  @also_with_minimal_runtime
  @flaky('https://github.com/emscripten-core/emscripten/issues/25245')
  def test_audio_worklet_emscripten_locks(self):
    self.btest_exit('webaudio/audioworklet_emscripten_locks.c', cflags=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-pthread'])

  # Verifies setting audio context sample rate, and that emscripten_audio_context_sample_rate() works.
  @requires_sound_hardware
  @also_with_minimal_runtime
  def test_web_audio_context_sample_rate(self):
    self.btest_exit('webaudio/audio_context_sample_rate.c', cflags=['-lwebaudio.js'])

  def test_error_reporting(self):
    # Test catching/reporting Error objects
    create_file('post.js', 'throw new Error("oops");')
    self.btest('hello_world.c', cflags=['--post-js=post.js'], expected='exception:oops')

    # Test catching/reporting non-Error objects
    create_file('post.js', 'throw "foo";')
    self.btest('hello_world.c', cflags=['--post-js=post.js'], expected='exception:foo')

  @also_with_threads
  @also_with_wasm2js
  @parameterized({
    '': ([],),
    'es6': (['-sEXPORT_ES6', '-pthread', '-sPTHREAD_POOL_SIZE=1'],),
  })
  @requires_dev_dependency('webpack')
  def test_webpack(self, args):
    if '-sEXPORT_ES6' in args:
      copytree(test_file('webpack_es6'), '.')
      outfile = 'src/hello.mjs'
    else:
      copytree(test_file('webpack'), '.')
      outfile = 'src/hello.js'
    self.compile_btest('hello_world.c', ['-sEXIT_RUNTIME', '-sMODULARIZE', '-sENVIRONMENT=web', '-o', outfile] + args)
    self.run_process(shared.get_npm_cmd('webpack') + ['--mode=development', '--no-devtool'])
    if not self.is_wasm2js():
      # Webpack doesn't bundle the wasm file by default so we need to copy it
      # TODO(sbc): Look into plugins that do bundling.
      shutil.copy('src/hello.wasm', 'dist/')
    self.run_browser('dist/index.html', '/report_result?exit:0')

  @also_with_threads
  @requires_dev_dependency('vite')
  def test_vite(self):
    copytree(test_file('vite'), '.')
    self.compile_btest('hello_world.c', ['-sEXIT_RUNTIME', '-sENVIRONMENT=web', '-o', 'hello.mjs'])
    self.run_process(shared.get_npm_cmd('vite') + ['build'])
    self.run_browser('dist/index.html', '/report_result?exit:0')

  @also_with_threads
  @requires_dev_dependency('rollup')
  def test_rollup(self):
    copytree(test_file('rollup'), '.')
    self.compile_btest('hello_world.c', ['-sEXIT_RUNTIME', '-sENVIRONMENT=web', '-o', 'hello.mjs'])
    self.run_process(shared.get_npm_cmd('rollup') + ['--config'])
    # Rollup doesn't bundle the wasm file by default so we need to copy it
    # TODO(sbc): Look into plugins that do bundling.
    shutil.copy('hello.wasm', 'dist/')
    self.run_browser('index.html', '/report_result?exit:0')

  # Use different ports for each parameterized version so they can be run in
  # parallel and not conflict.
  @parameterized({
    '': ([], 9998),
    'es6': (['-sEXPORT_ES6', '--extern-post-js', test_file('modularize_post_js.js')], 9999),
  })
  def test_cross_origin(self, args, port):
    if '-sEXPORT_ES6' in args and browser_should_skip_feature('EMTEST_LACKS_ES6_WORKERS', Feature.WORKER_ES6_MODULES):
      self.skipTest('This test requires a browser with ES6 Module Workers support')
    # Verifies that the emscripten-generated JS and Wasm can be hosted on a different origin.
    # This test creates a second HTTP server running on a different port that serves files from `subdir`.
    # The main html is served from the normal port 8888 server while the JS and Wasm are hosted
    # on the port specified above.
    os.mkdir('subdir')
    create_file('subdir/foo.txt', 'hello')
    self.compile_btest('hello_world.c', ['-o', 'subdir/hello.js', '-sRUNTIME_DEBUG', '-sCROSS_ORIGIN', '-sPROXY_TO_PTHREAD', '-pthread', '-sEXIT_RUNTIME'] + args)

    class MyReqestHandler(SimpleHTTPRequestHandler):
      def __init__(self, *args, **kwargs):
        super().__init__(*args, directory='subdir', **kwargs)

      # Add COOP, COEP, CORP, and no-caching headers
      def end_headers(self):
        self.send_header('Accept-Ranges', 'bytes')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')

        self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate, private, max-age=0')
        self.send_header('Expires', '0')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Vary', '*') # Safari insists on caching if this header is not present in addition to the above

        return SimpleHTTPRequestHandler.end_headers(self)

    if '-sEXPORT_ES6' in args:
      create_file('test.html', f'<script src="http://localhost:{port}/hello.js" type="module"></script>')
    else:
      create_file('test.html', f'<script src="http://localhost:{port}/hello.js"></script>')

    server = HttpServerThread(ThreadingHTTPServer(('localhost', port), MyReqestHandler))
    server.start()
    try:
      self.run_browser('test.html', '/report_result?exit:0')
    finally:
      server.stop()
      server.join()


class emrun(RunnerCore):
  def test_emrun_info(self):
    if not has_browser():
      self.skipTest('need a browser')
    result = self.run_process([EMRUN, '--system-info', '--browser_info'], stdout=PIPE).stdout
    assert 'CPU' in result
    assert 'Browser' in result
    assert 'Traceback' not in result

    result = self.run_process([EMRUN, '--list-browsers'], stdout=PIPE).stdout
    assert 'Traceback' not in result

  def test_no_browser(self):
    # Test --no-browser mode where we have to take care of launching the browser ourselves
    # and then killing emrun when we are done.
    if not has_browser():
      self.skipTest('need a browser')

    self.run_process([EMCC, test_file('test_emrun.c'), '--emrun', '-o', 'hello_world.html'])
    proc = subprocess.Popen([EMRUN, '--no-browser', '.', '--port=3333'], stdout=PIPE)
    try:
      if get_browser():
        print('Starting browser')
        browser_cmd = shlex.split(get_browser())
        browser = subprocess.Popen(browser_cmd + ['http://localhost:3333/hello_world.html'])
        try:
          while True:
            stdout = proc.stdout.read()
            if b'Dumping out file' in stdout:
              break
        finally:
          print('Terminating browser')
          browser.terminate()
          browser.wait()
    finally:
      print('Terminating emrun server')
      proc.terminate()
      proc.wait()

  def test_program_arg_separator(self):
    # Verify that trying to pass argument to the page without the `--` separator will
    # generate an actionable error message
    err = self.expect_fail([EMRUN, '--foo'])
    self.assertContained('error: unrecognized arguments: --foo', err)
    self.assertContained('remember to add `--` between arguments', err)

  def test_emrun(self):
    self.emcc('test_emrun.c', ['--emrun', '-o', 'test_emrun.html'])
    if not has_browser():
      self.skipTest('need a browser')

    # We cannot run emrun from the temp directory the suite will clean up afterwards, since the
    # browser that is launched will have that directory as startup directory, and the browser will
    # not close as part of the test, pinning down the cwd on Windows and it wouldn't be possible to
    # delete it. Therefore switch away from that directory before launching.
    os.chdir(path_from_root())

    args_base = [EMRUN, '--timeout', '30', '--safe_firefox_profile',
                 '--kill-exit', '--port', '6939', '--verbose',
                 '--log-stdout', self.in_dir('stdout.txt'),
                 '--log-stderr', self.in_dir('stderr.txt')]

    if get_browser() is not None:
      # If EMTEST_BROWSER carried command line arguments to pass to the browser,
      # (e.g. "firefox -profile /path/to/foo") those can't be passed via emrun,
      # so strip them out.
      browser_cmd = shlex.split(get_browser())
      browser_path = browser_cmd[0]
      args_base += ['--browser', browser_path]
      if len(browser_cmd) > 1:
        browser_args = browser_cmd[1:]
        if 'firefox' in browser_path and ('-profile' in browser_args or '--profile' in browser_args):
          # emrun uses its own -profile, strip it out
          parser = argparse.ArgumentParser(add_help=False) # otherwise it throws with -headless
          parser.add_argument('-profile')
          parser.add_argument('--profile')
          browser_args = parser.parse_known_args(browser_args)[1]
        if browser_args:
          args_base += ['--browser_args', ' ' + ' '.join(browser_args)]

    for args in [
        [],
        ['--port', '0'],
        ['--private_browsing', '--port', '6941'],
        ['--dump_out_directory', 'other dir/multiple', '--port', '6942'],
        ['--dump_out_directory=foo_bar', '--port', '6942'],
    ]:
      args = args_base + args + [self.in_dir('test_emrun.html'), '--', '1', '2', '--3', 'escaped space', 'with_underscore']
      print(shlex.join(args))
      proc = self.run_process(args, check=False)
      self.assertEqual(proc.returncode, 100)
      dump_dir = 'dump_out'
      if '--dump_out_directory' in args:
        dump_dir = 'other dir/multiple'
      elif '--dump_out_directory=foo_bar' in args:
        dump_dir = 'foo_bar'
      self.assertExists(self.in_dir(f'{dump_dir}/test.dat'))
      self.assertExists(self.in_dir(f'{dump_dir}/heap.dat'))
      self.assertExists(self.in_dir(f'{dump_dir}/nested/with space.dat'))
      stdout = read_file(self.in_dir('stdout.txt'))
      stderr = read_file(self.in_dir('stderr.txt'))
      self.assertContained('argc: 6', stdout)
      self.assertContained('argv[3]: --3', stdout)
      self.assertContained('argv[4]: escaped space', stdout)
      self.assertContained('argv[5]: with_underscore', stdout)
      self.assertContained('hello, world!', stdout)
      self.assertContained('Testing ASCII characters: !"$%&\'()*+,-./:;<=>?@[\\]^_`{|}~', stdout)
      self.assertContained('Testing char sequences: %20%21 &auml;', stdout)
      self.assertContained('hello, error stream!', stderr)


class browser64(browser):
  def setUp(self):
    super().setUp()
    self.set_setting('MEMORY64')
    self.require_wasm64()


class browser64_4gb(browser):
  def setUp(self):
    super().setUp()
    self.set_setting('MEMORY64')
    self.set_setting('INITIAL_MEMORY', '4200mb')
    self.set_setting('GLOBAL_BASE', '4gb')
    # Without this we get a warning about GLOBAL_BASE being ignored when used with SIDE_MODULE
    self.cflags.append('-Wno-unused-command-line-argument')
    self.require_wasm64()


class browser64_2gb(browser):
  def setUp(self):
    super().setUp()
    self.set_setting('MEMORY64')
    self.set_setting('INITIAL_MEMORY', '2200mb')
    self.set_setting('GLOBAL_BASE', '2gb')
    # Without this we get a warning about GLOBAL_BASE being ignored when used with SIDE_MODULE
    self.cflags.append('-Wno-unused-command-line-argument')
    self.require_wasm64()


class browser_2gb(browser):
  def setUp(self):
    super().setUp()
    self.set_setting('INITIAL_MEMORY', '2200mb')
    self.set_setting('GLOBAL_BASE', '2gb')
    # Without this we get a warning about GLOBAL_BASE being ignored when used with SIDE_MODULE
    self.cflags.append('-Wno-unused-command-line-argument')
