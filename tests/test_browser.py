# coding=utf-8
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import argparse
import json
import multiprocessing
import os
import random
import re
import shlex
import shutil
import subprocess
import time
import unittest
import webbrowser
import zlib

from runner import BrowserCore, path_from_root, has_browser, EMTEST_BROWSER, no_wasm_backend, flaky, create_test_file
from tools import system_libs
from tools.shared import PYTHON, EMCC, WINDOWS, FILE_PACKAGER, PIPE, SPIDERMONKEY_ENGINE, JS_ENGINES
from tools.shared import try_delete, Building, run_process, run_js

try:
  from http.server import BaseHTTPRequestHandler, HTTPServer
except ImportError:
  # Python 2 compatibility
  from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer


def test_chunked_synchronous_xhr_server(support_byte_ranges, chunkSize, data, checksum, port):
  class ChunkedServerHandler(BaseHTTPRequestHandler):
    def sendheaders(s, extra=[], length=len(data)):
      s.send_response(200)
      s.send_header("Content-Length", str(length))
      s.send_header("Access-Control-Allow-Origin", "http://localhost:%s" % port)
      s.send_header("Access-Control-Expose-Headers", "Content-Length, Accept-Ranges")
      s.send_header("Content-type", "application/octet-stream")
      if support_byte_ranges:
        s.send_header("Accept-Ranges", "bytes")
      for i in extra:
        s.send_header(i[0], i[1])
      s.end_headers()

    def do_HEAD(s):
      s.sendheaders()

    def do_OPTIONS(s):
      s.sendheaders([("Access-Control-Allow-Headers", "Range")], 0)

    def do_GET(s):
      if not support_byte_ranges:
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

  # CORS preflight makes OPTIONS requests which we need to account for.
  expectedConns = 22
  httpd = HTTPServer(('localhost', 11111), ChunkedServerHandler)
  for i in range(expectedConns + 1):
    httpd.handle_request()


def shell_with_script(shell_file, output_file, replacement):
  with open(path_from_root('src', shell_file)) as input:
    with open(output_file, 'w') as output:
      output.write(input.read().replace('{{{ SCRIPT }}}', replacement))


def is_chrome():
  return EMTEST_BROWSER and 'chrom' in EMTEST_BROWSER.lower()


def no_chrome(note='chrome is not supported'):
  if is_chrome():
    return unittest.skip(note)
  return lambda f: f


def is_firefox():
  return EMTEST_BROWSER and 'firefox' in EMTEST_BROWSER.lower()


def no_firefox(note='firefox is not supported'):
  if is_firefox():
    return unittest.skip(note)
  return lambda f: f


def no_swiftshader(f):
  def decorated(self):
    if is_chrome() and '--use-gl=swiftshader' in EMTEST_BROWSER:
      self.skipTest('not compatible with swiftshader')
    return f(self)

  return decorated


def requires_threads(f):
  def decorated(self):
    if os.environ.get('EMTEST_LACKS_THREAD_SUPPORT'):
      self.skipTest('EMTEST_LACKS_THREAD_SUPPORT is set')
    # FIXME when the wasm backend gets threads
    if is_chrome() and self.is_wasm_backend():
      self.skipTest('wasm backend lacks threads')
    return f(self)

  return decorated


requires_graphics_hardware = unittest.skipIf(os.getenv('EMTEST_LACKS_GRAPHICS_HARDWARE'), "This test requires graphics hardware")
requires_sound_hardware = unittest.skipIf(os.getenv('EMTEST_LACKS_SOUND_HARDWARE'), "This test requires sound hardware")
requires_sync_compilation = unittest.skipIf(is_chrome(), "This test requires synchronous compilation, which does not work in Chrome (except for tiny wasms)")


class browser(BrowserCore):
  @classmethod
  def setUpClass(self):
    super(browser, self).setUpClass()
    self.browser_timeout = 20
    print()
    print('Running the browser tests. Make sure the browser allows popups from localhost.')
    print()

  def test_sdl1_in_emscripten_nonstrict_mode(self):
    if 'EMCC_STRICT' in os.environ and int(os.environ['EMCC_STRICT']):
      self.skipTest('This test requires being run in non-strict mode (EMCC_STRICT env. variable unset)')
    # TODO: This test is verifying behavior that will be deprecated at some point in the future, remove this test once
    # system JS libraries are no longer automatically linked to anymore.
    self.btest('hello_world_sdl.cpp', reference='htmltest.png')

  def test_sdl1(self):
    self.btest('hello_world_sdl.cpp', reference='htmltest.png', args=['-lSDL', '-lGL'])
    self.btest('hello_world_sdl.cpp', reference='htmltest.png', args=['-s', 'USE_SDL=1', '-lGL']) # is the default anyhow

  # Deliberately named as test_zzz_* to make this test the last one
  # as this test may take the focus away from the main test window
  # by opening a new window and possibly not closing it.
  @no_wasm_backend('wasm source maps')
  def test_zzz_html_source_map(self):
    if not has_browser():
      self.skipTest('need a browser')
    cpp_file = 'src.cpp'
    html_file = 'src.html'
    # browsers will try to 'guess' the corresponding original line if a
    # generated line is unmapped, so if we want to make sure that our
    # numbering is correct, we need to provide a couple of 'possible wrong
    # answers'. thus, we add some printf calls so that the cpp file gets
    # multiple mapped lines. in other words, if the program consists of a
    # single 'throw' statement, browsers may just map any thrown exception to
    # that line, because it will be the only mapped line.
    with open(cpp_file, 'w') as f:
      f.write(r'''
      #include <cstdio>

      int main() {
        printf("Starting test\n");
        try {
          throw 42; // line 8
        } catch (int e) { }
        printf("done\n");
        return 0;
      }
      ''')
    # use relative paths when calling emcc, because file:// URIs can only load
    # sourceContent when the maps are relative paths
    try_delete(html_file)
    try_delete(html_file + '.map')
    self.compile_btest(['src.cpp', '-o', 'src.html', '-g4', '-s', 'WASM=0'])
    self.assertExists(html_file)
    self.assertExists(html_file + '.map')
    webbrowser.open_new('file://' + html_file)
    print('''
If manually bisecting:
  Check that you see src.cpp among the page sources.
  Even better, add a breakpoint, e.g. on the printf, then reload, then step
  through and see the print (best to run with EMTEST_SAVE_DIR=1 for the reload).
''')

  @no_wasm_backend('wasm source maps')
  def test_emscripten_log(self):
    # TODO: wasm support for source maps
    src = 'src.cpp'
    create_test_file(src, self.with_report_result(open(path_from_root('tests', 'emscripten_log', 'emscripten_log.cpp')).read()))

    self.compile_btest([src, '--pre-js', path_from_root('src', 'emscripten-source-map.min.js'), '-g', '-o', 'page.html', '-s', 'DEMANGLE_SUPPORT=1', '-s', 'WASM=0'])
    self.run_browser('page.html', None, '/report_result?1')

  def build_native_lzma(self):
    lzma_native = path_from_root('third_party', 'lzma.js', 'lzma-native')
    if os.path.isfile(lzma_native) and os.access(lzma_native, os.X_OK):
      return

    cwd = os.getcwd()
    try:
      os.chdir(path_from_root('third_party', 'lzma.js'))
      # On Windows prefer using MinGW make if it exists, otherwise fall back to hoping we have cygwin make.
      if WINDOWS and Building.which('mingw32-make'):
        run_process(['doit.bat'])
      else:
        run_process(['sh', './doit.sh'])
    finally:
      os.chdir(cwd)

  def test_preload_file(self):
    absolute_src_path = os.path.join(self.get_dir(), 'somefile.txt').replace('\\', '/')
    open(absolute_src_path, 'w').write('''load me right before running the code please''')

    absolute_src_path2 = os.path.join(self.get_dir(), '.somefile.txt').replace('\\', '/')
    open(absolute_src_path2, 'w').write('''load me right before running the code please''')

    absolute_src_path3 = os.path.join(self.get_dir(), 'some@file.txt').replace('\\', '/')
    open(absolute_src_path3, 'w').write('''load me right before running the code please''')

    def make_main(path):
      print('make main at', path)
      path = path.replace('\\', '\\\\').replace('"', '\\"') # Escape tricky path name for use inside a C string.
      create_test_file('main.cpp', self.with_report_result(r'''
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
          REPORT_RESULT(result);
          return 0;
        }
        ''' % path))

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

    for test in test_cases:
      (srcpath, dstpath) = test
      print('Testing', srcpath, dstpath)
      make_main(dstpath)
      self.compile_btest(['main.cpp', '--preload-file', srcpath, '-o', 'page.html'])
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')
    # Test that '--no-heap-copy' works.
    if WINDOWS:
      # On Windows, the following non-alphanumeric non-control code ASCII characters are supported.
      # The characters <, >, ", |, ?, * are not allowed, because the Windows filesystem doesn't support those.
      tricky_filename = '!#$%&\'()+,-. ;=@[]^_`{}~.txt'
    else:
      # All 7-bit non-alphanumeric non-control code ASCII characters except /, : and \ are allowed.
      tricky_filename = '!#$%&\'()+,-. ;=@[]^_`{}~ "*<>?|.txt'
    open(os.path.join(self.get_dir(), tricky_filename), 'w').write('''load me right before running the code please''')
    make_main(tricky_filename)
    # As an Emscripten-specific feature, the character '@' must be escaped in the form '@@' to not confuse with the 'src@dst' notation.
    self.compile_btest(['main.cpp', '--preload-file', tricky_filename.replace('@', '@@'), '--no-heap-copy', '-o', 'page.html'])
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    # By absolute path

    make_main('somefile.txt') # absolute becomes relative
    self.compile_btest(['main.cpp', '--preload-file', absolute_src_path, '-o', 'page.html'])
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    # Test subdirectory handling with asset packaging.
    try_delete('assets')
    os.makedirs('assets/sub/asset1/'.replace('\\', '/'))
    os.makedirs('assets/sub/asset1/.git'.replace('\\', '/')) # Test adding directory that shouldn't exist.
    os.makedirs('assets/sub/asset2/'.replace('\\', '/'))
    create_test_file('assets/sub/asset1/file1.txt', '''load me right before running the code please''')
    create_test_file('assets/sub/asset1/.git/shouldnt_be_embedded.txt', '''this file should not get embedded''')
    create_test_file('assets/sub/asset2/file2.txt', '''load me right before running the code please''')
    absolute_assets_src_path = 'assets'.replace('\\', '/')

    def make_main_two_files(path1, path2, nonexistingpath):
      create_test_file('main.cpp', self.with_report_result(r'''
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

          f = fopen("%s", "r");
          if (f == NULL)
            result = 0;
          fclose(f);

          f = fopen("%s", "r");
          if (f != NULL)
            result = 0;

          REPORT_RESULT(result);
          return 0;
        }
      ''' % (path1, path2, nonexistingpath)))

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
      self.compile_btest(['main.cpp', '--preload-file', srcpath, '--exclude-file', '*/.*', '-o', 'page.html'])
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    # Should still work with -o subdir/..

    make_main('somefile.txt') # absolute becomes relative
    try:
      os.mkdir('dirrey')
    except:
      pass
    self.compile_btest(['main.cpp', '--preload-file', absolute_src_path, '-o', 'dirrey/page.html'])
    self.run_browser('dirrey/page.html', 'You should see |load me right before|.', '/report_result?1')

    # With FS.preloadFile

    create_test_file('pre.js', '''
      Module.preRun = function() {
        FS.createPreloadedFile('/', 'someotherfile.txt', 'somefile.txt', true, false); // we need --use-preload-plugins for this.
      };
    ''')
    make_main('someotherfile.txt')
    self.compile_btest(['main.cpp', '--pre-js', 'pre.js', '-o', 'page.html', '--use-preload-plugins'])
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

  # Tests that user .html shell files can manually download .data files created with --preload-file cmdline.
  def test_preload_file_with_manual_data_download(self):
    create_test_file('src.cpp', self.with_report_result(open(os.path.join(path_from_root('tests/manual_download_data.cpp'))).read()))

    create_test_file('file.txt', '''Hello!''')

    self.compile_btest(['src.cpp', '-o', 'manual_download_data.js', '--preload-file', 'file.txt@/file.txt'])
    shutil.copyfile(path_from_root('tests', 'manual_download_data.html'), 'manual_download_data.html')
    self.run_browser('manual_download_data.html', 'Hello!', '/report_result?1')

  # Tests that if the output files have single or double quotes in them, that it will be handled by correctly escaping the names.
  def test_output_file_escaping(self):
    tricky_part = '\'' if WINDOWS else '\' and \"' # On Windows, files/directories may not contain a double quote character. On non-Windowses they can, so test that.

    d = 'dir with ' + tricky_part
    abs_d = os.path.join(self.get_dir(), d)
    try:
      os.mkdir(abs_d)
    except:
      pass
    txt = 'file with ' + tricky_part + '.txt'
    abs_txt = os.path.join(abs_d, txt)
    open(abs_txt, 'w').write('load me right before')

    cpp = os.path.join(d, 'file with ' + tricky_part + '.cpp')
    open(cpp, 'w').write(self.with_report_result(r'''
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
        int result = !strcmp("|load me right before|", buf);
        REPORT_RESULT(result);
        return 0;
      }
    ''' % (txt.replace('\'', '\\\'').replace('\"', '\\"'))))

    data_file = os.path.join(abs_d, 'file with ' + tricky_part + '.data')
    data_js_file = os.path.join(abs_d, 'file with ' + tricky_part + '.js')
    run_process([PYTHON, FILE_PACKAGER, data_file, '--use-preload-cache', '--indexedDB-name=testdb', '--preload', abs_txt + '@' + txt, '--js-output=' + data_js_file])
    page_file = os.path.join(d, 'file with ' + tricky_part + '.html')
    abs_page_file = os.path.join(self.get_dir(), page_file)
    self.compile_btest([cpp, '--pre-js', data_js_file, '-o', abs_page_file, '-s', 'FORCE_FILESYSTEM=1'])
    self.run_browser(page_file, '|load me right before|.', '/report_result?0')

  def test_preload_caching(self):
    create_test_file('main.cpp', self.with_report_result(r'''
      #include <stdio.h>
      #include <string.h>
      #include <emscripten.h>

      extern "C" {
        extern int checkPreloadResults();
      }

      int main(int argc, char** argv) {
        FILE *f = fopen("%s", "r");
        char buf[100];
        fread(buf, 1, 20, f);
        buf[20] = 0;
        fclose(f);
        printf("|%%s|\n", buf);

        int result = 0;

        result += !strcmp("load me right before", buf);
        result += checkPreloadResults();

        REPORT_RESULT(result);
        return 0;
      }
    ''' % 'somefile.txt'))

    create_test_file('test.js', '''
      mergeInto(LibraryManager.library, {
        checkPreloadResults: function() {
          var cached = 0;
          var packages = Object.keys(Module['preloadResults']);
          packages.forEach(function(package) {
            var fromCache = Module['preloadResults'][package]['fromCache'];
            if (fromCache)
              ++ cached;
          });
          return cached;
        }
      });
    ''')

    # test caching of various sizes, including sizes higher than 128MB which is
    # chrome's limit on IndexedDB item sizes, see
    # https://cs.chromium.org/chromium/src/content/renderer/indexed_db/webidbdatabase_impl.cc?type=cs&q=%22The+serialized+value+is+too+large%22&sq=package:chromium&g=0&l=177
    # https://cs.chromium.org/chromium/src/out/Debug/gen/third_party/blink/public/mojom/indexeddb/indexeddb.mojom.h?type=cs&sq=package:chromium&g=0&l=60
    for extra_size in (0, 1 * 1024 * 1024, 100 * 1024 * 1024, 150 * 1024 * 1024):
      if is_chrome() and extra_size >= 100 * 1024 * 1024:
        continue
      create_test_file('somefile.txt', '''load me right before running the code please''' + ('_' * extra_size))
      print('size:', os.path.getsize('somefile.txt'))
      self.compile_btest(['main.cpp', '--use-preload-cache', '--js-library', 'test.js', '--preload-file', 'somefile.txt', '-o', 'page.html', '-s', 'ALLOW_MEMORY_GROWTH=1'])
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?2')

  def test_preload_caching_indexeddb_name(self):
    create_test_file('somefile.txt', '''load me right before running the code please''')

    def make_main(path):
      print(path)
      create_test_file('main.cpp', self.with_report_result(r'''
        #include <stdio.h>
        #include <string.h>
        #include <emscripten.h>

        extern "C" {
          extern int checkPreloadResults();
        }

        int main(int argc, char** argv) {
          FILE *f = fopen("%s", "r");
          char buf[100];
          fread(buf, 1, 20, f);
          buf[20] = 0;
          fclose(f);
          printf("|%%s|\n", buf);

          int result = 0;

          result += !strcmp("load me right before", buf);
          result += checkPreloadResults();

          REPORT_RESULT(result);
          return 0;
        }
      ''' % path))

    create_test_file('test.js', '''
      mergeInto(LibraryManager.library, {
        checkPreloadResults: function() {
          var cached = 0;
          var packages = Object.keys(Module['preloadResults']);
          packages.forEach(function(package) {
            var fromCache = Module['preloadResults'][package]['fromCache'];
            if (fromCache)
              ++ cached;
          });
          return cached;
        }
      });
    ''')

    make_main('somefile.txt')
    run_process([PYTHON, FILE_PACKAGER, 'somefile.data', '--use-preload-cache', '--indexedDB-name=testdb', '--preload', 'somefile.txt', '--js-output=' + 'somefile.js'])
    self.compile_btest(['main.cpp', '--js-library', 'test.js', '--pre-js', 'somefile.js', '-o', 'page.html', '-s', 'FORCE_FILESYSTEM=1'])
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?2')

  def test_multifile(self):
    # a few files inside a directory
    os.makedirs(os.path.join('subdirr', 'moar'))
    create_test_file(os.path.join('subdirr', 'data1.txt'), '1214141516171819')
    create_test_file(os.path.join('subdirr', 'moar', 'data2.txt'), '3.14159265358979')
    create_test_file('main.cpp', self.with_report_result(r'''
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

        REPORT_RESULT(result);
        return 0;
      }
    '''))

    # by individual files
    self.compile_btest(['main.cpp', '--preload-file', 'subdirr/data1.txt', '--preload-file', 'subdirr/moar/data2.txt', '-o', 'page.html'])
    self.run_browser('page.html', 'You should see two cool numbers', '/report_result?1')
    os.remove('page.html')

    # by directory, and remove files to make sure
    self.compile_btest(['main.cpp', '--preload-file', 'subdirr', '-o', 'page.html'])
    shutil.rmtree('subdirr')
    self.run_browser('page.html', 'You should see two cool numbers', '/report_result?1')

  def test_custom_file_package_url(self):
    # a few files inside a directory
    self.clear()
    os.makedirs('subdirr')
    os.makedirs('cdn')
    create_test_file(os.path.join('subdirr', 'data1.txt'), '1214141516171819')
    # change the file package base dir to look in a "cdn". note that normally
    # you would add this in your own custom html file etc., and not by
    # modifying the existing shell in this manner
    create_test_file('shell.html', open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { locateFile: function (path, prefix) {if (path.endsWith(".wasm")) {return prefix + path;} else {return "cdn/" + path;}}, '))
    create_test_file('main.cpp', self.with_report_result(r'''
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

        REPORT_RESULT(result);
        return 0;
      }
    '''))

    self.compile_btest(['main.cpp', '--shell-file', 'shell.html', '--preload-file', 'subdirr/data1.txt', '-o', 'test.html'])
    shutil.move('test.data', os.path.join('cdn', 'test.data'))
    self.run_browser('test.html', '', '/report_result?1')

  def test_missing_data_throws_error(self):
    def setup(assetLocalization):
      self.clear()
      create_test_file('data.txt', 'data')
      create_test_file('main.cpp', self.with_report_result(r'''
        #include <stdio.h>
        #include <string.h>
        #include <emscripten.h>
        int main() {
          // This code should never be executed in terms of missing required dependency file.
          REPORT_RESULT(0);
          return 0;
        }
      '''))
      create_test_file('on_window_error_shell.html', r'''
      <html>
          <center><canvas id='canvas' width='256' height='256'></canvas></center>
          <hr><div id='output'></div><hr>
          <script type='text/javascript'>
            window.onerror = function(error) {
              window.onerror = null;
              var result = error.indexOf("test.data") >= 0 ? 1 : 0;
              var xhr = new XMLHttpRequest();
              xhr.open('GET', 'http://localhost:8888/report_result?' + result, true);
              xhr.send();
              setTimeout(function() { window.close() }, 1000);
            }
            var Module = {
              locateFile: function (path, prefix) {if (path.endsWith(".wasm")) {return prefix + path;} else {return "''' + assetLocalization + r'''" + path;}},
              print: (function() {
                var element = document.getElementById('output');
                return function(text) { element.innerHTML += text.replace('\n', '<br>', 'g') + '<br>';};
              })(),
              canvas: document.getElementById('canvas')
            };
          </script>
          {{{ SCRIPT }}}
        </body>
      </html>''')

    def test():
      # test test missing file should run xhr.onload with status different than 200, 304 or 206
      setup("")
      self.compile_btest(['main.cpp', '--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html'])
      shutil.move('test.data', 'missing.data')
      self.run_browser('test.html', '', '/report_result?1')

      # test unknown protocol should go through xhr.onerror
      setup("unknown_protocol://")
      self.compile_btest(['main.cpp', '--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html'])
      self.run_browser('test.html', '', '/report_result?1')

      # test wrong protocol and port
      setup("https://localhost:8800/")
      self.compile_btest(['main.cpp', '--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html'])
      self.run_browser('test.html', '', '/report_result?1')

    test()

    # TODO: CORS, test using a full url for locateFile
    # create_test_file('shell.html', open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { locateFile: function (path) {return "http:/localhost:8888/cdn/" + path;}, '))
    # test()

  def test_dev_random(self):
    self.btest(os.path.join('filesystem', 'dev_random.cpp'), expected='0')

  def test_sdl_swsurface(self):
    self.btest('sdl_swsurface.c', args=['-lSDL', '-lGL'], expected='1')

  def test_sdl_surface_lock_opts(self):
    # Test Emscripten-specific extensions to optimize SDL_LockSurface and SDL_UnlockSurface.
    self.btest('hello_world_sdl.cpp', reference='htmltest.png', message='You should see "hello, world!" and a colored cube.', args=['-DTEST_SDL_LOCK_OPTS', '-lSDL', '-lGL'])

  def test_sdl_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file, and memory-init
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.jpg')
    create_test_file('sdl_image.c', self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))

    for mem in [0, 1]:
      for dest, dirname, basename in [('screenshot.jpg', '/', 'screenshot.jpg'),
                                      ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
        self.compile_btest([
          'sdl_image.c', '-o', 'page.html', '-O2', '-lSDL', '-lGL', '--memory-init-file', str(mem),
          '--preload-file', dest, '-DSCREENSHOT_DIRNAME="' + dirname + '"', '-DSCREENSHOT_BASENAME="' + basename + '"', '--use-preload-plugins'
        ])
        self.run_browser('page.html', '', '/report_result?600')

  def test_sdl_image_jpeg(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.jpeg')
    create_test_file('sdl_image_jpeg.c', self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))
    self.compile_btest([
      'sdl_image_jpeg.c', '-o', 'page.html', '-lSDL', '-lGL',
      '--preload-file', 'screenshot.jpeg', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"', '--use-preload-plugins'
    ])
    self.run_browser('page.html', '', '/report_result?600')

  def test_sdl_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl_image_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not', '-lSDL', '-lGL'], also_proxied=True, manually_trigger_reftest=True)

  def test_sdl_image_prepare_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl_image_prepare_data.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not', '-lSDL', '-lGL'], manually_trigger_reftest=True)

  def test_sdl_image_must_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.jpg')
    self.btest('sdl_image_must_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.jpg', '-lSDL', '-lGL'])

  def test_sdl_stb_image(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl_stb_image.c', reference='screenshot.jpg', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  def test_sdl_stb_image_bpp(self):
    # load grayscale image without alpha
    self.clear()
    shutil.copyfile(path_from_root('tests', 'sdl-stb-bpp1.png'), 'screenshot.not')
    self.btest('sdl_stb_image.c', reference='sdl-stb-bpp1.png', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

    # load grayscale image with alpha
    self.clear()
    shutil.copyfile(path_from_root('tests', 'sdl-stb-bpp2.png'), 'screenshot.not')
    self.btest('sdl_stb_image.c', reference='sdl-stb-bpp2.png', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

    # load RGB image
    self.clear()
    shutil.copyfile(path_from_root('tests', 'sdl-stb-bpp3.png'), 'screenshot.not')
    self.btest('sdl_stb_image.c', reference='sdl-stb-bpp3.png', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

    # load RGBA image
    self.clear()
    shutil.copyfile(path_from_root('tests', 'sdl-stb-bpp4.png'), 'screenshot.not')
    self.btest('sdl_stb_image.c', reference='sdl-stb-bpp4.png', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  def test_sdl_stb_image_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl_stb_image_data.c', reference='screenshot.jpg', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL'])

  def test_sdl_stb_image_cleanup(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl_stb_image_cleanup.c', expected='0', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not', '-lSDL', '-lGL', '--memoryprofiler'])

  def test_sdl_canvas(self):
    self.clear()
    self.btest('sdl_canvas.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-lSDL', '-lGL'])
    # some extra coverage
    self.clear()
    self.btest('sdl_canvas.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-O0', '-s', 'SAFE_HEAP=1', '-lSDL', '-lGL'])
    self.clear()
    self.btest('sdl_canvas.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-O2', '-s', 'SAFE_HEAP=1', '-lSDL', '-lGL'])

  def post_manual_reftest(self, reference=None):
    self.reftest(path_from_root('tests', self.reference if reference is None else reference))

    html = open('test.html').read()
    html = html.replace('</body>', '''
<script>
function assert(x, y) { if (!x) throw 'assertion failed ' + y }

%s

var windowClose = window.close;
window.close = function() {
  // wait for rafs to arrive and the screen to update before reftesting
  setTimeout(function() {
    doReftest();
    setTimeout(windowClose, 5000);
  }, 1000);
};
</script>
</body>''' % open('reftest.js').read())
    create_test_file('test.html', html)

  def test_sdl_canvas_proxy(self):
    create_test_file('data.txt', 'datum')
    self.btest('sdl_canvas_proxy.c', reference='sdl_canvas_proxy.png', args=['--proxy-to-worker', '--preload-file', 'data.txt', '-lSDL', '-lGL'], manual_reference=True, post_build=self.post_manual_reftest)

  @no_chrome('see #7930')
  @requires_graphics_hardware
  def test_glgears_proxy(self):
    # we modify the asm.js, this is a non-wasm test
    self.btest('hello_world_gles_proxy.c', reference='gears.png', args=['--proxy-to-worker', '-s', 'GL_TESTING=1', '-DSTATIC_GEARS=1', '-lGL', '-lglut', '-s', 'WASM=0'], manual_reference=True, post_build=self.post_manual_reftest)

    # test noProxy option applied at runtime

    # run normally (duplicates above test, but verifies we can run outside of the btest harness
    self.run_browser('test.html', None, ['/report_result?0'])

    # run with noProxy
    self.run_browser('test.html?noProxy', None, ['/report_result?0'])

    def copy(to, js_mod, html_mod=lambda x: x):
      create_test_file(to + '.html', html_mod(open('test.html').read().replace('test.js', to + '.js')))
      create_test_file(to + '.js', js_mod(open('test.js').read()))

    # run with noProxy, but make main thread fail
    copy('two', lambda original: re.sub(r'function _main\(\$(.+),\$(.+)\) {', r'function _main($\1,$\2) { if (ENVIRONMENT_IS_WEB) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:%s/report_result?999");xhr.send(); return; }' % self.port, original),
         lambda original: original.replace('function doReftest() {', 'function doReftest() { return; ')) # don't reftest on main thread, it would race
    self.run_browser('two.html?noProxy', None, ['/report_result?999'])
    copy('two', lambda original: re.sub(r'function _main\(\$(.+),\$(.+)\) {', r'function _main($\1,$\2) { if (ENVIRONMENT_IS_WEB) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:%s/report_result?999");xhr.send(); return; }' % self.port, original))
    self.run_browser('two.html', None, ['/report_result?0']) # this is still cool

    # run without noProxy, so proxy, but make worker fail
    copy('three', lambda original: re.sub(r'function _main\(\$(.+),\$(.+)\) {', r'function _main($\1,$\2) { if (ENVIRONMENT_IS_WORKER) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:%s/report_result?999");xhr.send(); return; }' % self.port, original),
         lambda original: original.replace('function doReftest() {', 'function doReftest() { return; ')) # don't reftest on main thread, it would race
    self.run_browser('three.html', None, ['/report_result?999'])
    copy('three', lambda original: re.sub(r'function _main\(\$(.+),\$(.+)\) {', r'function _main($\1,$\2) { if (ENVIRONMENT_IS_WORKER) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:%s/report_result?999");xhr.send(); return; }' % self.port, original))
    self.run_browser('three.html?noProxy', None, ['/report_result?0']) # this is still cool

  @requires_graphics_hardware
  def test_glgears_proxy_jstarget(self):
    # test .js target with --proxy-worker; emits 2 js files, client and worker
    self.compile_btest([path_from_root('tests', 'hello_world_gles_proxy.c'), '-o', 'test.js', '--proxy-to-worker', '-s', 'GL_TESTING=1', '-lGL', '-lglut'])
    shell_with_script('shell_minimal.html', 'test.html', '<script src="test.js"></script>')
    self.post_manual_reftest('gears.png')
    self.run_browser('test.html', None, '/report_result?0')

  def test_sdl_canvas_alpha(self):
    # N.B. On Linux with Intel integrated graphics cards, this test needs Firefox 49 or newer.
    # See https://github.com/emscripten-core/emscripten/issues/4069.
    create_test_file('flag_0.js', '''
      Module['arguments'] = ['-0'];
    ''')

    self.btest('sdl_canvas_alpha.c', args=['-lSDL', '-lGL'], reference='sdl_canvas_alpha.png', reference_slack=12)
    self.btest('sdl_canvas_alpha.c', args=['--pre-js', 'flag_0.js', '-lSDL', '-lGL'], reference='sdl_canvas_alpha_flag_0.png', reference_slack=12)

  def test_sdl_key(self):
    for delay in [0, 1]:
      for defines in [
        [],
        ['-DTEST_EMSCRIPTEN_SDL_SETEVENTHANDLER']
      ]:
        for emterps in [
          [],
          ['-DTEST_SLEEP', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-s', 'ASSERTIONS=1', '-s', "SAFE_HEAP=1"]
        ]:
          print(delay, defines, emterps)

          if emterps and self.is_wasm_backend():
            return self.skipTest('no emterpretify with wasm backend')

          create_test_file('pre.js', '''
            function keydown(c) {
             %s
              var event = new KeyboardEvent("keydown", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
              document.dispatchEvent(event);
             %s
            }

            function keyup(c) {
             %s
              var event = new KeyboardEvent("keyup", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
              document.dispatchEvent(event);
             %s
            }
          ''' % ('setTimeout(function() {' if delay else '', '}, 1);' if delay else '', 'setTimeout(function() {' if delay else '', '}, 1);' if delay else ''))
          create_test_file('sdl_key.c', self.with_report_result(open(path_from_root('tests', 'sdl_key.c')).read()))

          self.compile_btest(['sdl_key.c', '-o', 'page.html'] + defines + emterps + ['--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main']''', '-lSDL', '-lGL'])
          self.run_browser('page.html', '', '/report_result?223092870')

  def test_sdl_key_proxy(self):
    create_test_file('pre.js', '''
      var Module = {};
      Module.postRun = function() {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }
    ''')

    def post():
      html = open('test.html').read()
      html = html.replace('</body>', '''
<script>
function keydown(c) {
  var event = new KeyboardEvent("keydown", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
  document.dispatchEvent(event);
}

function keyup(c) {
  var event = new KeyboardEvent("keyup", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
  document.dispatchEvent(event);
}

keydown(1250);keydown(38);keyup(38);keyup(1250); // alt, up
keydown(1248);keydown(1249);keydown(40);keyup(40);keyup(1249);keyup(1248); // ctrl, shift, down
keydown(37);keyup(37); // left
keydown(39);keyup(39); // right
keydown(65);keyup(65); // a
keydown(66);keyup(66); // b
keydown(100);keyup(100); // trigger the end

</script>
</body>''')
      create_test_file('test.html', html)

    self.btest('sdl_key_proxy.c', '223092870', args=['--proxy-to-worker', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''', '-lSDL', '-lGL'], manual_reference=True, post_build=post)

  def test_canvas_focus(self):
    self.btest('canvas_focus.c', '1')

  def test_keydown_preventdefault_proxy(self):
    def post():
      html = open('test.html').read()
      html = html.replace('</body>', '''
<script>
function keydown(c) {
  var event = new KeyboardEvent("keydown", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
  return document.dispatchEvent(event);
}

function keypress(c) {
  var event = new KeyboardEvent("keypress", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
  return document.dispatchEvent(event);
}

function keyup(c) {
  var event = new KeyboardEvent("keyup", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
  return document.dispatchEvent(event);
}

function sendKey(c) {
  // Simulate the sending of the keypress event when the
  // prior keydown event is not prevent defaulted.
  if (keydown(c) === false) {
    console.log('keydown prevent defaulted, NOT sending keypress!!!');
  } else {
    keypress(c);
  }
  keyup(c);
}

// Send 'a'. Simulate the sending of the keypress event when the
// prior keydown event is not prevent defaulted.
sendKey(65);

// Send backspace. Keypress should not be sent over as default handling of
// the Keydown event should be prevented.
sendKey(8);

keydown(100);keyup(100); // trigger the end
</script>
</body>''')

      create_test_file('test.html', html)

    self.btest('keydown_preventdefault_proxy.cpp', '300', args=['--proxy-to-worker', '-s', '''EXPORTED_FUNCTIONS=['_main']'''], manual_reference=True, post_build=post)

  def test_sdl_text(self):
    create_test_file('pre.js', '''
      Module.postRun = function() {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }

      function simulateKeyEvent(c) {
        var event = new KeyboardEvent("keypress", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
        document.body.dispatchEvent(event);
      }
    ''')
    create_test_file('sdl_text.c', self.with_report_result(open(path_from_root('tests', 'sdl_text.c')).read()))

    self.compile_btest(['sdl_text.c', '-o', 'page.html', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''', '-lSDL', '-lGL'])
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_mouse(self):
    create_test_file('pre.js', '''
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
    create_test_file('sdl_mouse.c', self.with_report_result(open(path_from_root('tests', 'sdl_mouse.c')).read()))

    self.compile_btest(['sdl_mouse.c', '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js', '-lSDL', '-lGL'])
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_mouse_offsets(self):
    create_test_file('pre.js', '''
      function simulateMouseEvent(x, y, button) {
        var event = document.createEvent("MouseEvents");
        if (button >= 0) {
          var event1 = document.createEvent("MouseEvents");
          event1.initMouseEvent('mousedown', true, true, window,
                     1, x, y, x, y,
                     0, 0, 0, 0,
                     button, null);
          Module['canvas'].dispatchEvent(event1);
          var event2 = document.createEvent("MouseEvents");
          event2.initMouseEvent('mouseup', true, true, window,
                     1, x, y, x, y,
                     0, 0, 0, 0,
                     button, null);
          Module['canvas'].dispatchEvent(event2);
        } else {
          var event1 = document.createEvent("MouseEvents");
          event1.initMouseEvent('mousemove', true, true, window,
                     0, x, y, x, y,
                     0, 0, 0, 0,
                     0, null);
          Module['canvas'].dispatchEvent(event1);
        }
      }
      window['simulateMouseEvent'] = simulateMouseEvent;
    ''')
    create_test_file('page.html', '''
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
    create_test_file('sdl_mouse.c', self.with_report_result(open(path_from_root('tests', 'sdl_mouse.c')).read()))

    self.compile_btest(['sdl_mouse.c', '-DTEST_SDL_MOUSE_OFFSETS', '-O2', '--minify', '0', '-o', 'sdl_mouse.js', '--pre-js', 'pre.js', '-lSDL', '-lGL'])
    self.run_browser('page.html', '', '/report_result?1')

  def test_glut_touchevents(self):
    self.btest('glut_touchevents.c', '1', args=['-lglut'])

  def test_glut_wheelevents(self):
    self.btest('glut_wheelevents.c', '1', args=['-lglut'])

  @requires_graphics_hardware
  def test_glut_glutget_no_antialias(self):
    self.btest('glut_glutget.c', '1', args=['-lglut', '-lGL'])
    self.btest('glut_glutget.c', '1', args=['-lglut', '-lGL', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])

  # This test supersedes the one above, but it's skipped in the CI because anti-aliasing is not well supported by the Mesa software renderer.
  @requires_graphics_hardware
  def test_glut_glutget(self):
    self.btest('glut_glutget.c', '1', args=['-lglut', '-lGL'])
    self.btest('glut_glutget.c', '1', args=['-lglut', '-lGL', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])

  def test_sdl_joystick_1(self):
    # Generates events corresponding to the Working Draft of the HTML5 Gamepad API.
    # http://www.w3.org/TR/2012/WD-gamepad-20120529/#gamepad-interface
    create_test_file('pre.js', '''
      var gamepads = [];
      // Spoof this function.
      navigator['getGamepads'] = function() {
        return gamepads;
      };
      window['addNewGamepad'] = function(id, numAxes, numButtons) {
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
      window['simulateGamepadButtonDown'] = function (index, button) {
        gamepads[index].buttons[button] = 1;
      };
      window['simulateGamepadButtonUp'] = function (index, button) {
        gamepads[index].buttons[button] = 0;
      };
      window['simulateAxisMotion'] = function (index, axis, value) {
        gamepads[index].axes[axis] = value;
      };
    ''')
    create_test_file('sdl_joystick.c', self.with_report_result(open(path_from_root('tests', 'sdl_joystick.c')).read()))

    self.compile_btest(['sdl_joystick.c', '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js', '-lSDL', '-lGL'])
    self.run_browser('page.html', '', '/report_result?2')

  def test_sdl_joystick_2(self):
    # Generates events corresponding to the Editor's Draft of the HTML5 Gamepad API.
    # https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-Gamepad
    create_test_file('pre.js', '''
      var gamepads = [];
      // Spoof this function.
      navigator['getGamepads'] = function() {
        return gamepads;
      };
      window['addNewGamepad'] = function(id, numAxes, numButtons) {
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
      window['simulateGamepadButtonDown'] = function (index, button) {
        gamepads[index].buttons[button].pressed = true;
        gamepads[index].buttons[button].value = 1;
      };
      window['simulateGamepadButtonUp'] = function (index, button) {
        gamepads[index].buttons[button].pressed = false;
        gamepads[index].buttons[button].value = 0;
      };
      window['simulateAxisMotion'] = function (index, axis, value) {
        gamepads[index].axes[axis] = value;
      };
    ''')
    create_test_file('sdl_joystick.c', self.with_report_result(open(path_from_root('tests', 'sdl_joystick.c')).read()))

    self.compile_btest(['sdl_joystick.c', '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js', '-lSDL', '-lGL'])
    self.run_browser('page.html', '', '/report_result?2')

  @requires_graphics_hardware
  def test_glfw_joystick(self):
    # Generates events corresponding to the Editor's Draft of the HTML5 Gamepad API.
    # https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-Gamepad
    create_test_file('pre.js', '''
      var gamepads = [];
      // Spoof this function.
      navigator['getGamepads'] = function() {
        return gamepads;
      };
      window['addNewGamepad'] = function(id, numAxes, numButtons) {
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
      window['simulateGamepadButtonDown'] = function (index, button) {
        gamepads[index].buttons[button].pressed = true;
        gamepads[index].buttons[button].value = 1;
      };
      window['simulateGamepadButtonUp'] = function (index, button) {
        gamepads[index].buttons[button].pressed = false;
        gamepads[index].buttons[button].value = 0;
      };
      window['simulateAxisMotion'] = function (index, axis, value) {
        gamepads[index].axes[axis] = value;
      };
    ''')
    create_test_file('test_glfw_joystick.c', self.with_report_result(open(path_from_root('tests', 'test_glfw_joystick.c')).read()))

    self.compile_btest(['test_glfw_joystick.c', '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js', '-lGL', '-lglfw3', '-s', 'USE_GLFW=3'])
    self.run_browser('page.html', '', '/report_result?2')

  @requires_graphics_hardware
  def test_webgl_context_attributes(self):
    # Javascript code to check the attributes support we want to test in the WebGL implementation
    # (request the attribute, create a context and check its value afterwards in the context attributes).
    # Tests will succeed when an attribute is not supported.
    create_test_file('check_webgl_attributes_support.js', '''
      mergeInto(LibraryManager.library, {
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
    filepath = path_from_root('tests/test_webgl_context_attributes_common.c')
    temp_filepath = os.path.join(self.get_dir(), os.path.basename(filepath))
    shutil.copyfile(filepath, temp_filepath)

    # perform tests with attributes activated
    self.btest('test_webgl_context_attributes_glut.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-lglut', '-lGLEW'])
    self.btest('test_webgl_context_attributes_sdl.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-lSDL', '-lGLEW'])
    self.btest('test_webgl_context_attributes_sdl2.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-s', 'USE_SDL=2', '-lGLEW'])
    self.btest('test_webgl_context_attributes_glfw.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED', '-lGL', '-lglfw', '-lGLEW'])

    # perform tests with attributes desactivated
    self.btest('test_webgl_context_attributes_glut.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-lGL', '-lglut', '-lGLEW'])
    self.btest('test_webgl_context_attributes_sdl.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-lGL', '-lSDL', '-lGLEW'])
    self.btest('test_webgl_context_attributes_glfw.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-lGL', '-lglfw', '-lGLEW'])

  # Test that -s GL_PREINITIALIZED_CONTEXT=1 works and allows user to set Module['preinitializedWebGLContext'] to a preinitialized WebGL context.
  @requires_graphics_hardware
  def test_preinitialized_webgl_context(self):
    self.btest('preinitialized_webgl_context.cpp', '5', args=['-s', 'GL_PREINITIALIZED_CONTEXT=1', '--shell-file', path_from_root('tests/preinitialized_webgl_context.html')])

  @requires_threads
  def test_emscripten_get_now(self):
    for args in [[], ['-s', 'USE_PTHREADS=1'], ['-s', 'ENVIRONMENT=web', '-O2', '--closure', '1']]:
      self.btest('emscripten_get_now.cpp', '1', args=args)

  def test_write_file_in_environment_web(self):
    self.btest('write_file.cpp', '0', args=['-s', 'ENVIRONMENT=web', '-Os', '--closure', '1'])

  @unittest.skip('Skipping due to https://github.com/emscripten-core/emscripten/issues/2770')
  def test_fflush(self):
    self.btest('test_fflush.cpp', '0', args=['--shell-file', path_from_root('tests', 'test_fflush.html')])

  def test_file_db(self):
    secret = str(time.time())
    create_test_file('moar.txt', secret)
    self.btest('file_db.cpp', '1', args=['--preload-file', 'moar.txt', '-DFIRST'])
    shutil.copyfile('test.html', 'first.html')
    self.btest('file_db.cpp', secret, args=['-s', 'FORCE_FILESYSTEM=1'])
    shutil.copyfile('test.html', 'second.html')
    create_test_file('moar.txt', 'aliantha')
    self.btest('file_db.cpp', secret, args=['--preload-file', 'moar.txt']) # even with a file there, we load over it
    shutil.move('test.html', 'third.html')

  def test_fs_idbfs_sync(self):
    for extra in [[], ['-DEXTRA_WORK']]:
      secret = str(time.time())
      self.btest(path_from_root('tests', 'fs', 'test_idbfs_sync.c'), '1', force_c=True, args=['-lidbfs.js', '-DFIRST', '-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_test', '_success']'''])
      self.btest(path_from_root('tests', 'fs', 'test_idbfs_sync.c'), '1', force_c=True, args=['-lidbfs.js', '-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_test', '_success']'''] + extra)

  @no_wasm_backend('emterpretify')
  def test_fs_idbfs_fsync(self):
    # sync from persisted state into memory before main()
    create_test_file('pre.js', '''
      Module.preRun = function() {
        addRunDependency('syncfs');

        FS.mkdir('/working1');
        FS.mount(IDBFS, {}, '/working1');
        FS.syncfs(true, function (err) {
          if (err) throw err;
          removeRunDependency('syncfs');
        });
      };
    ''')

    args = ['--pre-js', 'pre.js', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-lidbfs.js', '-s', 'EXIT_RUNTIME=1']
    secret = str(time.time())
    self.btest(path_from_root('tests', 'fs', 'test_idbfs_fsync.c'), '1', force_c=True, args=args + ['-DFIRST', '-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_success']'''])
    self.btest(path_from_root('tests', 'fs', 'test_idbfs_fsync.c'), '1', force_c=True, args=args + ['-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_success']'''])

  @no_wasm_backend('emterpretify')
  def test_fs_memfs_fsync(self):
    args = ['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-s', 'EXIT_RUNTIME=1']
    secret = str(time.time())
    self.btest(path_from_root('tests', 'fs', 'test_memfs_fsync.c'), '1', force_c=True, args=args + ['-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main']'''])

  def test_fs_workerfs_read(self):
    secret = 'a' * 10
    secret2 = 'b' * 10
    create_test_file('pre.js', '''
      var Module = {};
      Module.preRun = function() {
        var blob = new Blob(['%s']);
        var file = new File(['%s'], 'file.txt');
        FS.mkdir('/work');
        FS.mount(WORKERFS, {
          blobs: [{ name: 'blob.txt', data: blob }],
          files: [file],
        }, '/work');
      };
    ''' % (secret, secret2))
    self.btest(path_from_root('tests', 'fs', 'test_workerfs_read.c'), '1', force_c=True, args=['-lworkerfs.js', '--pre-js', 'pre.js', '-DSECRET=\"' + secret + '\"', '-DSECRET2=\"' + secret2 + '\"', '--proxy-to-worker'])

  def test_fs_workerfs_package(self):
    create_test_file('file1.txt', 'first')
    if not os.path.exists('sub'):
      os.makedirs('sub')
    open(os.path.join('sub', 'file2.txt'), 'w').write('second')
    run_process([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', os.path.join('sub', 'file2.txt'), '--separate-metadata', '--js-output=files.js'])
    self.btest(os.path.join('fs', 'test_workerfs_package.cpp'), '1', args=['-lworkerfs.js', '--proxy-to-worker'])

  def test_fs_lz4fs_package(self):
    # generate data
    self.clear()
    os.mkdir('subdir')
    create_test_file('file1.txt', '0123456789' * (1024 * 128))
    open(os.path.join('subdir', 'file2.txt'), 'w').write('1234567890' * (1024 * 128))
    random_data = bytearray(random.randint(0, 255) for x in range(1024 * 128 * 10 + 1))
    random_data[17] = ord('X')
    open('file3.txt', 'wb').write(random_data)

    # compress in emcc,  -s LZ4=1  tells it to tell the file packager
    print('emcc-normal')
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['-s', 'LZ4=1', '--preload-file', 'file1.txt', '--preload-file', 'subdir/file2.txt', '--preload-file', 'file3.txt'], timeout=60)
    assert os.path.getsize('file1.txt') + os.path.getsize(os.path.join('subdir', 'file2.txt')) + os.path.getsize('file3.txt') == 3 * 1024 * 128 * 10 + 1
    assert os.path.getsize('test.data') < (3 * 1024 * 128 * 10) / 2  # over half is gone
    print('    emcc-opts')
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['-s', 'LZ4=1', '--preload-file', 'file1.txt', '--preload-file', 'subdir/file2.txt', '--preload-file', 'file3.txt', '-O2'], timeout=60)

    # compress in the file packager, on the server. the client receives compressed data and can just use it. this is typical usage
    print('normal')
    out = subprocess.check_output([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'subdir/file2.txt', 'file3.txt', '--lz4'])
    open('files.js', 'wb').write(out)
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['--pre-js', 'files.js', '-s', 'LZ4=1', '-s', 'FORCE_FILESYSTEM=1'], timeout=60)
    print('    opts')
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['--pre-js', 'files.js', '-s', 'LZ4=1', '-s', 'FORCE_FILESYSTEM=1', '-O2'], timeout=60)

    # load the data into LZ4FS manually at runtime. This means we compress on the client. This is generally not recommended
    print('manual')
    subprocess.check_output([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'subdir/file2.txt', 'file3.txt', '--separate-metadata', '--js-output=files.js'])
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '1', args=['-DLOAD_MANUALLY', '-s', 'LZ4=1', '-s', 'FORCE_FILESYSTEM=1'], timeout=60)
    print('    opts')
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '1', args=['-DLOAD_MANUALLY', '-s', 'LZ4=1', '-s', 'FORCE_FILESYSTEM=1', '-O2'], timeout=60)
    print('    opts+closure')
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '1', args=['-DLOAD_MANUALLY', '-s', 'LZ4=1', '-s', 'FORCE_FILESYSTEM=1', '-O2', '--closure', '1', '-g1'], timeout=60)

    '''# non-lz4 for comparison
    try:
      os.mkdir('files')
    except:
      pass
    shutil.copyfile('file1.txt', os.path.join('files', 'file1.txt'))
    shutil.copyfile('file2.txt', os.path.join('files', 'file2.txt'))
    shutil.copyfile('file3.txt', os.path.join('files', 'file3.txt'))
    out = subprocess.check_output([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'files/file1.txt', 'files/file2.txt', 'files/file3.txt'])
    open('files.js', 'wb').write(out)
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['--pre-js', 'files.js'], timeout=60)'''

  def test_separate_metadata_later(self):
    # see issue #6654 - we need to handle separate-metadata both when we run before
    # the main program, and when we are run later

    create_test_file('data.dat', ' ')
    run_process([PYTHON, FILE_PACKAGER, 'more.data', '--preload', 'data.dat', '--separate-metadata', '--js-output=more.js'])
    self.btest(os.path.join('browser', 'separate_metadata_later.cpp'), '1', args=['-s', 'FORCE_FILESYSTEM=1'])

  def test_idbstore(self):
    secret = str(time.time())
    for stage in [0, 1, 2, 3, 0, 1, 2, 0, 0, 1, 4, 2, 5]:
      self.clear()
      self.btest(path_from_root('tests', 'idbstore.c'), str(stage), force_c=True, args=['-lidbstore.js', '-DSTAGE=' + str(stage), '-DSECRET=\"' + secret + '\"'])

  @no_wasm_backend('emterpretify')
  def test_idbstore_sync(self):
    secret = str(time.time())
    self.clear()
    self.btest(path_from_root('tests', 'idbstore_sync.c'), '6', force_c=True, args=['-lidbstore.js', '-DSECRET=\"' + secret + '\"', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '--memory-init-file', '1', '-O3', '-g2'])

  @no_wasm_backend('emterpretify')
  def test_idbstore_sync_worker(self):
    secret = str(time.time())
    self.clear()
    self.btest(path_from_root('tests', 'idbstore_sync_worker.c'), '6', force_c=True, args=['-lidbstore.js', '-DSECRET=\"' + secret + '\"', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '--memory-init-file', '1', '-O3', '-g2', '--proxy-to-worker', '-s', 'TOTAL_MEMORY=80MB'])

  def test_force_exit(self):
    self.btest('force_exit.c', force_c=True, expected='17', args=['-s', 'EXIT_RUNTIME=1'])

  def test_sdl_pumpevents(self):
    # key events should be detected using SDL_PumpEvents
    create_test_file('pre.js', '''
      function keydown(c) {
        var event = new KeyboardEvent("keydown", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
        document.dispatchEvent(event);
      }
    ''')
    self.btest('sdl_pumpevents.c', expected='7', args=['--pre-js', 'pre.js', '-lSDL', '-lGL'])

  def test_sdl_canvas_size(self):
    self.btest('sdl_canvas_size.c', expected='1',
               args=['-O2', '--minify', '0', '--shell-file',
                     path_from_root('tests', 'sdl_canvas_size.html'), '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_gl_read(self):
    # SDL, OpenGL, readPixels
    create_test_file('sdl_gl_read.c', self.with_report_result(open(path_from_root('tests', 'sdl_gl_read.c')).read()))
    self.compile_btest(['sdl_gl_read.c', '-o', 'something.html', '-lSDL', '-lGL'])
    self.run_browser('something.html', '.', '/report_result?1')

  @requires_graphics_hardware
  def test_sdl_gl_mapbuffers(self):
    self.btest('sdl_gl_mapbuffers.c', expected='1', args=['-s', 'FULL_ES3=1', '-lSDL', '-lGL'],
               message='You should see a blue triangle.')

  @requires_graphics_hardware
  def test_sdl_ogl(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_ogl.c', reference='screenshot-gray-purple.png', reference_slack=1,
               args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with gray at the top.')

  @requires_graphics_hardware
  def test_sdl_ogl_regal(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_ogl.c', reference='screenshot-gray-purple.png', reference_slack=1,
               args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'USE_REGAL=1', '-DUSE_REGAL', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with gray at the top.')

  @requires_graphics_hardware
  def test_sdl_ogl_defaultmatrixmode(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_ogl_defaultMatrixMode.c', reference='screenshot-gray-purple.png', reference_slack=1,
               args=['--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with gray at the top.')

  @requires_graphics_hardware
  def test_sdl_ogl_p(self):
    # Immediate mode with pointers
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_ogl_p.c', reference='screenshot-gray.png', reference_slack=1,
               args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with gray at the top.')

  @requires_graphics_hardware
  def test_sdl_ogl_proc_alias(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_ogl_proc_alias.c', reference='screenshot-gray-purple.png', reference_slack=1,
               args=['-O2', '-g2', '-s', 'INLINING_LIMIT=1', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'])

  @requires_graphics_hardware
  def test_sdl_fog_simple(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_fog_simple.c', reference='screenshot-fog-simple.png',
               args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl_fog_negative(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_fog_negative.c', reference='screenshot-fog-negative.png',
               args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl_fog_density(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_fog_density.c', reference='screenshot-fog-density.png',
               args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl_fog_exp2(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_fog_exp2.c', reference='screenshot-fog-exp2.png',
               args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl_fog_linear(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_fog_linear.c', reference='screenshot-fog-linear.png', reference_slack=1,
               args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins', '-lSDL', '-lGL'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_glfw(self):
    self.btest('glfw.c', '1', args=['-s', 'LEGACY_GL_EMULATION=1', '-lglfw', '-lGL'])
    self.btest('glfw.c', '1', args=['-s', 'LEGACY_GL_EMULATION=1', '-s', 'USE_GLFW=2', '-lglfw', '-lGL'])

  def test_glfw_minimal(self):
    self.btest('glfw_minimal.c', '1', args=['-lglfw', '-lGL'])
    self.btest('glfw_minimal.c', '1', args=['-s', 'USE_GLFW=2', '-lglfw', '-lGL'])

  def test_glfw_time(self):
    self.btest('test_glfw_time.c', '1', args=['-s', 'USE_GLFW=3', '-lglfw', '-lGL'])

  def _test_egl_base(self, *args):
    create_test_file('test_egl.c', self.with_report_result(open(path_from_root('tests', 'test_egl.c')).read()))

    self.compile_btest(['-O2', 'test_egl.c', '-o', 'page.html', '-lEGL', '-lGL'] + list(args))
    self.run_browser('page.html', '', '/report_result?1')

  @requires_graphics_hardware
  def test_egl(self):
    self._test_egl_base()

  @requires_threads
  @requires_graphics_hardware
  def test_egl_with_proxy_to_pthread(self):
    self._test_egl_base('-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1')

  def _test_egl_width_height_base(self, *args):
    create_test_file('test_egl_width_height.c', self.with_report_result(open(path_from_root('tests', 'test_egl_width_height.c')).read()))

    self.compile_btest(['-O2', 'test_egl_width_height.c', '-o', 'page.html', '-lEGL', '-lGL'] + list(args))
    self.run_browser('page.html', 'Should print "(300, 150)" -- the size of the canvas in pixels', '/report_result?1')

  def test_egl_width_height(self):
    self._test_egl_width_height_base()

  @requires_threads
  def test_egl_width_height_with_proxy_to_pthread(self):
    self._test_egl_width_height_base('-s', 'USE_PTHREADS', '-s', 'PROXY_TO_PTHREAD=1')

  def do_test_worker(self, args=[]):
    # Test running in a web worker
    create_test_file('file.dat', 'data for worker')
    html_file = open('main.html', 'w')
    html_file.write('''
      <html>
      <body>
        Worker Test
        <script>
          var worker = new Worker('worker.js');
          worker.onmessage = function(event) {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', 'http://localhost:%s/report_result?' + event.data);
            xhr.send();
            setTimeout(function() { window.close() }, 1000);
          };
        </script>
      </body>
      </html>
    ''' % self.port)
    html_file.close()

    for file_data in [1, 0]:
      cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world_worker.cpp'), '-o', 'worker.js'] + (['--preload-file', 'file.dat'] if file_data else []) + args
      print(cmd)
      subprocess.check_call(cmd)
      self.assertExists('worker.js')
      self.run_browser('main.html', '', '/report_result?hello%20from%20worker,%20and%20:' + ('data%20for%20w' if file_data else '') + ':')

  def test_worker(self):
    self.do_test_worker()
    self.assertContained('you should not see this text when in a worker!', run_js('worker.js')) # code should run standalone too

  def test_chunked_synchronous_xhr(self):
    main = 'chunked_sync_xhr.html'
    worker_filename = "download_and_checksum_worker.js"

    html_file = open(main, 'w')
    html_file.write(r"""
      <!doctype html>
      <html>
      <head><meta charset="utf-8"><title>Chunked XHR</title></head>
      <html>
      <body>
        Chunked XHR Web Worker Test
        <script>
          var worker = new Worker(""" + json.dumps(worker_filename) + r""");
          var buffer = [];
          worker.onmessage = function(event) {
            if (event.data.channel === "stdout") {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', 'http://localhost:%s/report_result?' + event.data.line);
              xhr.send();
              setTimeout(function() { window.close() }, 1000);
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
    """ % self.port)
    html_file.close()

    c_source_filename = "checksummer.c"

    prejs_filename = "worker_prejs.js"
    prejs_file = open(prejs_filename, 'w')
    prejs_file.write(r"""
      if (typeof(Module) === "undefined") Module = {};
      Module["arguments"] = ["/bigfile"];
      Module["preInit"] = function() {
          FS.createLazyFile('/', "bigfile", "http://localhost:11111/bogus_file_path", true, false);
      };
      var doTrace = true;
      Module["print"] = function(s) { self.postMessage({channel: "stdout", line: s}); };
      Module["printErr"] = function(s) { self.postMessage({channel: "stderr", char: s, trace: ((doTrace && s === 10) ? new Error().stack : null)}); doTrace = false; };
    """)
    prejs_file.close()
    # vs. os.path.join(self.get_dir(), filename)
    # vs. path_from_root('tests', 'hello_world_gles.c')
    self.compile_btest([path_from_root('tests', c_source_filename), '-g', '-s', 'SMALL_XHR_CHUNKS=1', '-o', worker_filename,
                        '--pre-js', prejs_filename])
    chunkSize = 1024
    data = os.urandom(10 * chunkSize + 1) # 10 full chunks and one 1 byte chunk
    checksum = zlib.adler32(data) & 0xffffffff # Python 2 compatibility: force bigint

    server = multiprocessing.Process(target=test_chunked_synchronous_xhr_server, args=(True, chunkSize, data, checksum, self.port))
    server.start()
    self.run_browser(main, 'Chunked binary synchronous XHR in Web Workers!', '/report_result?' + str(checksum))
    server.terminate()
    # Avoid race condition on cleanup, wait a bit so that processes have released file locks so that test tearDown won't
    # attempt to rmdir() files in use.
    if WINDOWS:
      time.sleep(2)

  @requires_graphics_hardware
  def test_glgears(self):
    self.btest('hello_world_gles.c', reference='gears.png', reference_slack=3,
               args=['-DHAVE_BUILTIN_SINCOS', '-lGL', '-lglut'], outfile='something.html',
               message='You should see animating gears.')

  @requires_graphics_hardware
  def test_glgears_long(self):
    for proxy in [0, 1]:
      print('proxy', proxy)
      self.btest('hello_world_gles.c', expected=list(map(str, range(30, 500))), args=['-DHAVE_BUILTIN_SINCOS', '-DLONGTEST', '-lGL', '-lglut'] + (['--proxy-to-worker'] if proxy else []), timeout=30)

  @requires_graphics_hardware
  def test_glgears_animation(self):
    es2_suffix = ['', '_full', '_full_944']
    for full_es2 in [0, 1, 2]:
      print(full_es2)
      self.compile_btest([path_from_root('tests', 'hello_world_gles%s.c' % es2_suffix[full_es2]), '-o', 'something.html',
                          '-DHAVE_BUILTIN_SINCOS', '-s', 'GL_TESTING=1', '-lGL', '-lglut',
                          '--shell-file', path_from_root('tests', 'hello_world_gles_shell.html')] +
                         (['-s', 'FULL_ES2=1'] if full_es2 else []))
      self.run_browser('something.html', 'You should see animating gears.', '/report_gl_result?true')

  @requires_graphics_hardware
  def test_fulles2_sdlproc(self):
    self.btest('full_es2_sdlproc.c', '1', args=['-s', 'GL_TESTING=1', '-DHAVE_BUILTIN_SINCOS', '-s', 'FULL_ES2=1', '-lGL', '-lSDL', '-lglut'])

  @requires_graphics_hardware
  def test_glgears_deriv(self):
    self.btest('hello_world_gles_deriv.c', reference='gears.png', reference_slack=2,
               args=['-DHAVE_BUILTIN_SINCOS', '-lGL', '-lglut'], outfile='something.html',
               message='You should see animating gears.')
    with open('something.html') as f:
      assert 'gl-matrix' not in f.read(), 'Should not include glMatrix when not needed'

  @requires_graphics_hardware
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

    def book_path(*pathelems):
      return path_from_root('tests', 'glbook', *pathelems)

    for program in programs:
      print(program)
      basename = os.path.basename(program)
      args = ['-lGL', '-lEGL', '-lX11']
      if basename == 'CH10_MultiTexture.bc':
        shutil.copyfile(book_path('Chapter_10', 'MultiTexture', 'basemap.tga'), 'basemap.tga')
        shutil.copyfile(book_path('Chapter_10', 'MultiTexture', 'lightmap.tga'), 'lightmap.tga')
        args += ['--preload-file', 'basemap.tga', '--preload-file', 'lightmap.tga']
      elif basename == 'CH13_ParticleSystem.bc':
        shutil.copyfile(book_path('Chapter_13', 'ParticleSystem', 'smoke.tga'), 'smoke.tga')
        args += ['--preload-file', 'smoke.tga', '-O2'] # test optimizations and closure here as well for more coverage

      self.btest(program,
                 reference=book_path(basename.replace('.bc', '.png')),
                 args=args,
                 timeout=30)

  @requires_graphics_hardware
  def test_gles2_emulation(self):
    shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_10', 'MultiTexture', 'basemap.tga'), 'basemap.tga')
    shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_10', 'MultiTexture', 'lightmap.tga'), 'lightmap.tga')
    shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_13', 'ParticleSystem', 'smoke.tga'), 'smoke.tga')

    for source, reference in [
      (os.path.join('glbook', 'Chapter_2', 'Hello_Triangle', 'Hello_Triangle_orig.c'), path_from_root('tests', 'glbook', 'CH02_HelloTriangle.png')),
      # (os.path.join('glbook', 'Chapter_8', 'Simple_VertexShader', 'Simple_VertexShader_orig.c'), path_from_root('tests', 'glbook', 'CH08_SimpleVertexShader.png')), # XXX needs INT extension in WebGL
      (os.path.join('glbook', 'Chapter_9', 'TextureWrap', 'TextureWrap_orig.c'), path_from_root('tests', 'glbook', 'CH09_TextureWrap.png')),
      # (os.path.join('glbook', 'Chapter_9', 'Simple_TextureCubemap', 'Simple_TextureCubemap_orig.c'), path_from_root('tests', 'glbook', 'CH09_TextureCubemap.png')), # XXX needs INT extension in WebGL
      (os.path.join('glbook', 'Chapter_9', 'Simple_Texture2D', 'Simple_Texture2D_orig.c'), path_from_root('tests', 'glbook', 'CH09_SimpleTexture2D.png')),
      (os.path.join('glbook', 'Chapter_10', 'MultiTexture', 'MultiTexture_orig.c'), path_from_root('tests', 'glbook', 'CH10_MultiTexture.png')),
      (os.path.join('glbook', 'Chapter_13', 'ParticleSystem', 'ParticleSystem_orig.c'), path_from_root('tests', 'glbook', 'CH13_ParticleSystem.png')),
    ]:
      print(source)
      self.btest(source,
                 reference=reference,
                 args=['-I' + path_from_root('tests', 'glbook', 'Common'),
                       path_from_root('tests', 'glbook', 'Common', 'esUtil.c'),
                       path_from_root('tests', 'glbook', 'Common', 'esShader.c'),
                       path_from_root('tests', 'glbook', 'Common', 'esShapes.c'),
                       path_from_root('tests', 'glbook', 'Common', 'esTransform.c'),
                       '-s', 'FULL_ES2=1', '-lGL', '-lEGL', '-lX11',
                       '--preload-file', 'basemap.tga', '--preload-file', 'lightmap.tga', '--preload-file', 'smoke.tga'])

  @requires_graphics_hardware
  def test_clientside_vertex_arrays_es3(self):
    # NOTE: Should FULL_ES3=1 imply client-side vertex arrays? The emulation needs FULL_ES2=1 for now.
    self.btest('clientside_vertex_arrays_es3.c', reference='gl_triangle.png', args=['-s', 'USE_WEBGL2=1', '-s', 'FULL_ES2=1', '-s', 'FULL_ES3=1', '-s', 'USE_GLFW=3', '-lglfw', '-lGLESv2'])

  def test_emscripten_api(self):
    self.btest('emscripten_api_browser.cpp', '1', args=['-s', '''EXPORTED_FUNCTIONS=['_main', '_third']''', '-lSDL'])

  def test_emscripten_api2(self):
    def setup():
      create_test_file('script1.js', '''
        Module._set(456);
      ''')
      create_test_file('file1.txt', 'first')
      create_test_file('file2.txt', 'second')

    setup()
    run_process([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'file1.txt', 'file2.txt'], stdout=open('script2.js', 'w'))
    self.btest('emscripten_api_browser2.cpp', '1', args=['-s', '''EXPORTED_FUNCTIONS=['_main', '_set']''', '-s', 'FORCE_FILESYSTEM=1'])

    # check using file packager to another dir
    self.clear()
    setup()
    os.mkdir('sub')
    run_process([PYTHON, FILE_PACKAGER, 'sub/test.data', '--preload', 'file1.txt', 'file2.txt'], stdout=open('script2.js', 'w'))
    shutil.copyfile(os.path.join('sub', 'test.data'), 'test.data')
    self.btest('emscripten_api_browser2.cpp', '1', args=['-s', '''EXPORTED_FUNCTIONS=['_main', '_set']''', '-s', 'FORCE_FILESYSTEM=1'])

  def test_emscripten_api_infloop(self):
    self.btest('emscripten_api_browser_infloop.cpp', '7')

  def test_emscripten_fs_api(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png') # preloaded *after* run
    self.btest('emscripten_fs_api_browser.cpp', '1', args=['-lSDL'])

  def test_emscripten_fs_api2(self):
    self.btest('emscripten_fs_api_browser2.cpp', '1', args=['-s', "ASSERTIONS=0"])
    self.btest('emscripten_fs_api_browser2.cpp', '1', args=['-s', "ASSERTIONS=1"])

  @requires_threads
  def test_emscripten_main_loop(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'EXIT_RUNTIME=1']]:
      self.btest('emscripten_main_loop.cpp', '0', args=args)

  @requires_threads
  def test_emscripten_main_loop_settimeout(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest('emscripten_main_loop_settimeout.cpp', '1', args=args)

  @requires_threads
  def test_emscripten_main_loop_and_blocker(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest('emscripten_main_loop_and_blocker.cpp', '0', args=args)

  @requires_threads
  def test_emscripten_main_loop_setimmediate(self):
    for args in [[], ['--proxy-to-worker'], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest('emscripten_main_loop_setimmediate.cpp', '1', args=args)

  def test_fs_after_main(self):
    for args in [[], ['-O1']]:
      self.btest('fs_after_main.cpp', '0', args=args)

  def test_sdl_quit(self):
    self.btest('sdl_quit.c', '1', args=['-lSDL', '-lGL'])

  def test_sdl_resize(self):
    self.btest('sdl_resize.c', '1', args=['-lSDL', '-lGL'])

  def test_glshaderinfo(self):
    self.btest('glshaderinfo.cpp', '1', args=['-lGL', '-lglut'])

  @requires_graphics_hardware
  def test_glgetattachedshaders(self):
    self.btest('glgetattachedshaders.c', '1', args=['-lGL', '-lEGL'])

  # Covered by dEQP text suite (we can remove it later if we add coverage for that).
  @requires_graphics_hardware
  def test_glframebufferattachmentinfo(self):
    self.btest('glframebufferattachmentinfo.c', '1', args=['-lGLESv2', '-lEGL'])

  @requires_graphics_hardware
  def test_sdlglshader(self):
    self.btest('sdlglshader.c', reference='sdlglshader.png', args=['-O2', '--closure', '1', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_sdlglshader2(self):
    self.btest('sdlglshader2.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_gl_glteximage(self):
    self.btest('gl_teximage.c', '1', args=['-lGL', '-lSDL'])

  @requires_graphics_hardware
  @requires_threads
  def test_gl_textures(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest('gl_textures.cpp', '0', args=['-lGL'])

  @requires_graphics_hardware
  def test_gl_ps(self):
    # pointers and a shader
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('gl_ps.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '--use-preload-plugins'], reference_slack=1)

  @requires_graphics_hardware
  def test_gl_ps_packed(self):
    # packed data that needs to be strided
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('gl_ps_packed.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '--use-preload-plugins'], reference_slack=1)

  @requires_graphics_hardware
  def test_gl_ps_strides(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('gl_ps_strides.c', reference='gl_ps_strides.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_gl_ps_worker(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('gl_ps_worker.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '--use-preload-plugins'], reference_slack=1, also_proxied=True)

  @requires_graphics_hardware
  def test_gl_renderers(self):
    self.btest('gl_renderers.c', reference='gl_renderers.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_gl_stride(self):
    self.btest('gl_stride.c', reference='gl_stride.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_gl_vertex_buffer_pre(self):
    self.btest('gl_vertex_buffer_pre.c', reference='gl_vertex_buffer_pre.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_gl_vertex_buffer(self):
    self.btest('gl_vertex_buffer.c', reference='gl_vertex_buffer.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], reference_slack=1)

  @requires_graphics_hardware
  def test_gles2_uniform_arrays(self):
    self.btest('gles2_uniform_arrays.cpp', args=['-s', 'GL_ASSERTIONS=1', '-lGL', '-lSDL'], expected=['1'], also_proxied=True)

  @requires_graphics_hardware
  def test_gles2_conformance(self):
    self.btest('gles2_conformance.cpp', args=['-s', 'GL_ASSERTIONS=1', '-lGL', '-lSDL'], expected=['1'])

  @requires_graphics_hardware
  def test_matrix_identity(self):
    self.btest('gl_matrix_identity.c', expected=['-1882984448', '460451840', '1588195328'], args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre(self):
    self.btest('cubegeom_pre.c', reference='cubegeom_pre.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_regal(self):
    self.btest('cubegeom_pre.c', reference='cubegeom_pre.png', args=['-s', 'USE_REGAL=1', '-DUSE_REGAL', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @requires_sync_compilation
  def test_cubegeom_pre_relocatable(self):
    self.btest('cubegeom_pre.c', reference='cubegeom_pre.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '-s', 'RELOCATABLE=1'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre2(self):
    self.btest('cubegeom_pre2.c', reference='cubegeom_pre2.png', args=['-s', 'GL_DEBUG=1', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL']) # some coverage for GL_DEBUG not breaking the build

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre3(self):
    self.btest('cubegeom_pre3.c', reference='cubegeom_pre2.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom(self):
    self.btest('cubegeom.c', reference='cubegeom.png', args=['-O2', '-g', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_cubegeom_regal(self):
    self.btest('cubegeom.c', reference='cubegeom.png', args=['-O2', '-g', '-DUSE_REGAL', '-s', 'USE_REGAL=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_cubegeom_proc(self):
    create_test_file('side.c', r'''

extern void* SDL_GL_GetProcAddress(const char *);

void *glBindBuffer = 0; // same name as the gl function, to check that the collision does not break us

void *getBindBuffer() {
  if (!glBindBuffer) glBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
  return glBindBuffer;
}
''')
    # also test -Os in wasm, which uses meta-dce, which should not break legacy gl emulation hacks
    for opts in [[], ['-O1'], ['-Os', '-s', 'WASM=1']]:
      self.btest('cubegeom_proc.c', reference='cubegeom.png', args=opts + ['side.c', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_glew(self):
    self.btest('cubegeom_glew.c', reference='cubegeom.png', args=['-O2', '--closure', '1', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lGLEW', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_color(self):
    self.btest('cubegeom_color.c', reference='cubegeom_color.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal(self):
    self.btest('cubegeom_normal.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_cubegeom_normal_dap(self): # draw is given a direct pointer to clientside memory, no element array buffer
    self.btest('cubegeom_normal_dap.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far(self): # indices do nto start from 0
    self.btest('cubegeom_normal_dap_far.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far_range(self): # glDrawRangeElements
    self.btest('cubegeom_normal_dap_far_range.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far_glda(self): # use glDrawArrays
    self.btest('cubegeom_normal_dap_far_glda.c', reference='cubegeom_normal_dap_far_glda.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_normal_dap_far_glda_quad(self): # with quad
    self.btest('cubegeom_normal_dap_far_glda_quad.c', reference='cubegeom_normal_dap_far_glda_quad.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_mt(self):
    self.btest('cubegeom_mt.c', reference='cubegeom_mt.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL']) # multitexture

  @requires_graphics_hardware
  def test_cubegeom_color2(self):
    self.btest('cubegeom_color2.c', reference='cubegeom_color2.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_cubegeom_texturematrix(self):
    self.btest('cubegeom_texturematrix.c', reference='cubegeom_texturematrix.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_fog(self):
    self.btest('cubegeom_fog.c', reference='cubegeom_fog.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_vao(self):
    self.btest('cubegeom_pre_vao.c', reference='cubegeom_pre_vao.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_vao_regal(self):
    self.btest('cubegeom_pre_vao.c', reference='cubegeom_pre_vao.png', args=['-s', 'USE_REGAL=1', '-DUSE_REGAL', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre2_vao(self):
    self.btest('cubegeom_pre2_vao.c', reference='cubegeom_pre_vao.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_pre2_vao2(self):
    self.btest('cubegeom_pre2_vao2.c', reference='cubegeom_pre2_vao2.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  @no_swiftshader
  def test_cubegeom_pre_vao_es(self):
    self.btest('cubegeom_pre_vao_es.c', reference='cubegeom_pre_vao.png', args=['-s', 'FULL_ES2=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cubegeom_u4fv_2(self):
    self.btest('cubegeom_u4fv_2.c', reference='cubegeom_u4fv_2.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_cube_explosion(self):
    self.btest('cube_explosion.c', reference='cube_explosion.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], also_proxied=True)

  @requires_graphics_hardware
  def test_glgettexenv(self):
    self.btest('glgettexenv.c', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'], expected=['1'])

  def test_sdl_canvas_blank(self):
    self.btest('sdl_canvas_blank.c', args=['-lSDL', '-lGL'], reference='sdl_canvas_blank.png')

  def test_sdl_canvas_palette(self):
    self.btest('sdl_canvas_palette.c', args=['-lSDL', '-lGL'], reference='sdl_canvas_palette.png')

  def test_sdl_canvas_twice(self):
    self.btest('sdl_canvas_twice.c', args=['-lSDL', '-lGL'], reference='sdl_canvas_twice.png')

  def test_sdl_set_clip_rect(self):
    self.btest('sdl_set_clip_rect.c', args=['-lSDL', '-lGL'], reference='sdl_set_clip_rect.png')

  def test_sdl_maprgba(self):
    self.btest('sdl_maprgba.c', args=['-lSDL', '-lGL'], reference='sdl_maprgba.png', reference_slack=3)

  def test_sdl_create_rgb_surface_from(self):
    self.btest('sdl_create_rgb_surface_from.c', args=['-lSDL', '-lGL'], reference='sdl_create_rgb_surface_from.png')

  def test_sdl_rotozoom(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl_rotozoom.c', reference='sdl_rotozoom.png', args=['--preload-file', 'screenshot.png', '--use-preload-plugins', '-lSDL', '-lGL'], reference_slack=3)

  def test_sdl_gfx_primitives(self):
    self.btest('sdl_gfx_primitives.c', args=['-lSDL', '-lGL'], reference='sdl_gfx_primitives.png', reference_slack=1)

  def test_sdl_canvas_palette_2(self):
    create_test_file('pre.js', '''
      Module['preRun'].push(function() {
        SDL.defaults.copyOnLock = false;
      });
    ''')

    create_test_file('args-r.js', '''
      Module['arguments'] = ['-r'];
    ''')

    create_test_file('args-g.js', '''
      Module['arguments'] = ['-g'];
    ''')

    create_test_file('args-b.js', '''
      Module['arguments'] = ['-b'];
    ''')

    self.btest('sdl_canvas_palette_2.c', reference='sdl_canvas_palette_r.png', args=['--pre-js', 'pre.js', '--pre-js', 'args-r.js', '-lSDL', '-lGL'])
    self.btest('sdl_canvas_palette_2.c', reference='sdl_canvas_palette_g.png', args=['--pre-js', 'pre.js', '--pre-js', 'args-g.js', '-lSDL', '-lGL'])
    self.btest('sdl_canvas_palette_2.c', reference='sdl_canvas_palette_b.png', args=['--pre-js', 'pre.js', '--pre-js', 'args-b.js', '-lSDL', '-lGL'])

  def test_sdl_ttf_render_text_solid(self):
    self.btest('sdl_ttf_render_text_solid.c', reference='sdl_ttf_render_text_solid.png', args=['-O2', '-s', 'TOTAL_MEMORY=16MB', '-lSDL', '-lGL'])

  def test_sdl_alloctext(self):
    self.btest('sdl_alloctext.c', expected='1', args=['-O2', '-s', 'TOTAL_MEMORY=16MB', '-lSDL', '-lGL'])

  def test_sdl_surface_refcount(self):
    self.btest('sdl_surface_refcount.c', args=['-lSDL'], expected='1')

  def test_sdl_free_screen(self):
    self.btest('sdl_free_screen.cpp', args=['-lSDL', '-lGL'], reference='htmltest.png')

  @requires_graphics_hardware
  def test_glbegin_points(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('glbegin_points.c', reference='glbegin_points.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '--use-preload-plugins'])

  @requires_graphics_hardware
  def test_s3tc(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.dds'), 'screenshot.dds')
    self.btest('s3tc.c', reference='s3tc.png', args=['--preload-file', 'screenshot.dds', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_s3tc_ffp_only(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.dds'), 'screenshot.dds')
    self.btest('s3tc.c', reference='s3tc.png', args=['--preload-file', 'screenshot.dds', '-s', 'LEGACY_GL_EMULATION=1', '-s', 'GL_FFP_ONLY=1', '-lGL', '-lSDL'])

  @no_chrome('see #7117')
  @requires_graphics_hardware
  def test_aniso(self):
    if SPIDERMONKEY_ENGINE in JS_ENGINES:
      # asm.js-ification check
      self.compile_btest([path_from_root('tests', 'aniso.c'), '-O2', '-g2', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '-Wno-incompatible-pointer-types'])
      self.set_setting('ASM_JS', 1)
      self.run_generated_code(SPIDERMONKEY_ENGINE, 'a.out.js', assert_returncode=None)
      print('passed asm test')

    shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
    self.btest('aniso.c', reference='aniso.png', reference_slack=2, args=['--preload-file', 'water.dds', '-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL', '-Wno-incompatible-pointer-types'])

  @requires_graphics_hardware
  def test_tex_nonbyte(self):
    self.btest('tex_nonbyte.c', reference='tex_nonbyte.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_float_tex(self):
    self.btest('float_tex.cpp', reference='float_tex.png', args=['-lGL', '-lglut'])

  @requires_graphics_hardware
  def test_subdata(self):
    self.btest('gl_subdata.cpp', reference='float_tex.png', args=['-lGL', '-lglut'])

  @requires_graphics_hardware
  def test_perspective(self):
    self.btest('perspective.c', reference='perspective.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL', '-lSDL'])

  @requires_graphics_hardware
  def test_glerror(self):
    self.btest('gl_error.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-lGL'])

  def test_openal_error(self):
    for args in [[], ['--closure', '1']]:
      print(args)
      self.btest('openal_error.c', expected='1', args=args)

  def test_openal_capture_sanity(self):
    self.btest('openal_capture_sanity.c', expected='0')

  @no_wasm_backend('dynamic linking')
  def test_runtimelink(self):
    for wasm in [0, 1]:
      print(wasm)
      main, supp = self.setup_runtimelink_test()
      create_test_file('supp.cpp', supp)
      self.compile_btest(['supp.cpp', '-o', 'supp.' + ('wasm' if wasm else 'js'), '-s', 'SIDE_MODULE=1', '-O2', '-s', 'WASM=%d' % wasm, '-s', 'EXPORT_ALL=1'])
      self.btest(main, args=['-DBROWSER=1', '-s', 'MAIN_MODULE=1', '-O2', '-s', 'WASM=%d' % wasm, '-s', 'RUNTIME_LINKED_LIBS=["supp.' + ('wasm' if wasm else 'js') + '"]', '-s', 'EXPORT_ALL=1'], expected='76')

  def test_pre_run_deps(self):
    # Adding a dependency in preRun will delay run
    create_test_file('pre.js', '''
      Module.preRun = function() {
        addRunDependency();
        out('preRun called, added a dependency...');
        setTimeout(function() {
          Module.okk = 10;
          removeRunDependency()
        }, 2000);
      };
    ''')

    for mem in [0, 1]:
      self.btest('pre_run_deps.cpp', expected='10', args=['--pre-js', 'pre.js', '--memory-init-file', str(mem)])

  @no_wasm_backend('mem init file')
  def test_mem_init(self):
    create_test_file('pre.js', '''
      function myJSCallback() { // called from main()
        Module._note(1);
      }
      Module.preRun = function() {
        addOnPreMain(function() {
          Module._note(2);
        });
      };
    ''')
    create_test_file('post.js', '''
      var assert = function(check, text) {
        if (!check) {
          console.log('assert failed: ' + text);
          maybeReportResultToServer(9);
        }
      }
      Module._note(4); // this happens too early! and is overwritten when the mem init arrives
    ''')

    # with assertions, we notice when memory was written to too early
    self.btest('mem_init.cpp', expected='9', args=['-s', 'WASM=0', '--pre-js', 'pre.js', '--post-js', 'post.js', '--memory-init-file', '1'])
    # otherwise, we just overwrite
    self.btest('mem_init.cpp', expected='3', args=['-s', 'WASM=0', '--pre-js', 'pre.js', '--post-js', 'post.js', '--memory-init-file', '1', '-s', 'ASSERTIONS=0'])

  @no_wasm_backend('mem init file')
  def test_mem_init_request(self):
    def test(what, status):
      print(what, status)
      create_test_file('pre.js', '''
        var xhr = Module.memoryInitializerRequest = new XMLHttpRequest();
        xhr.open('GET', "''' + what + '''", true);
        xhr.responseType = 'arraybuffer';
        xhr.send(null);

        console.warn = function(x) {
          if (x.indexOf('a problem seems to have happened with Module.memoryInitializerRequest') >= 0) {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', 'http://localhost:%s/report_result?0');
            setTimeout(xhr.onload = function() {
              console.log('close!');
              window.close();
            }, 1000);
            xhr.send();
            throw 'halt';
          }
          console.log('WARNING: ' + x);
        };
      ''' % self.port)
      self.btest('mem_init_request.cpp', expected=status, args=['-s', 'WASM=0', '--pre-js', 'pre.js', '--memory-init-file', '1'])

    test('test.html.mem', '1')
    test('nothing.nowhere', '0')

  def test_runtime_misuse(self):
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
        out('expected fail 1');
        assert(e.toString().indexOf('assert') >= 0); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);

      ok = false;
      try {
        doCwrapCall(2);
        ok = true; // should fail and not reach here, runtime is not ready yet so cwrap call will abort
      } catch(e) {
        out('expected fail 2');
        assert(e.toString().indexOf('assert') >= 0); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);

      ok = false;
      try {
        doDirectCall(3);
        ok = true; // should fail and not reach here, runtime is not ready yet so any code execution
      } catch(e) {
        out('expected fail 3');
        assert(e.toString().indexOf('assert') >= 0); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);
    '''

    post_hook = r'''
      function myJSCallback() {
        // called from main, this is an ok time
        doCcall(100);
        doCwrapCall(200);
        doDirectCall(300);
      }

      setTimeout(function() {
        var xhr = new XMLHttpRequest();
        assert(Module.noted);
        xhr.open('GET', 'http://localhost:%s/report_result?' + HEAP32[Module.noted>>2]);
        xhr.send();
        setTimeout(function() { window.close() }, 1000);
      }, 1000);
    ''' % self.port

    create_test_file('pre_runtime.js', r'''
      Module.onRuntimeInitialized = function(){
        myJSCallback();
      };
    ''')

    for filename, extra_args, second_code in [
      ('runtime_misuse.cpp', [], 600),
      ('runtime_misuse_2.cpp', ['--pre-js', 'pre_runtime.js'], 601) # 601, because no main means we *do* run another call after exit()
    ]:
      for mode in [['-s', 'WASM=0'], ['-s', 'WASM=1']]:
        if 'WASM=0' in mode and self.is_wasm_backend():
          continue
        print('\n', filename, extra_args, mode)
        print('mem init, so async, call too early')
        create_test_file('post.js', post_prep + post_test + post_hook)
        self.btest(filename, expected='600', args=['--post-js', 'post.js', '--memory-init-file', '1', '-s', 'EXIT_RUNTIME=1'] + extra_args + mode)
        print('sync startup, call too late')
        create_test_file('post.js', post_prep + 'Module.postRun.push(function() { ' + post_test + ' });' + post_hook)
        self.btest(filename, expected=str(second_code), args=['--post-js', 'post.js', '--memory-init-file', '0', '-s', 'EXIT_RUNTIME=1'] + extra_args + mode)
        print('sync, runtime still alive, so all good')
        create_test_file('post.js', post_prep + 'expected_ok = true; Module.postRun.push(function() { ' + post_test + ' });' + post_hook)
        self.btest(filename, expected='606', args=['--post-js', 'post.js', '--memory-init-file', '0'] + extra_args + mode)

  def test_cwrap_early(self):
    self.btest(os.path.join('browser', 'cwrap_early.cpp'), args=['-O2', '-s', 'ASSERTIONS=1', '--pre-js', path_from_root('tests', 'browser', 'cwrap_early.js'), '-s', 'EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap"]'], expected='0')

  def test_worker_api(self):
    self.compile_btest([path_from_root('tests', 'worker_api_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]'])
    self.btest('worker_api_main.cpp', expected='566')

  def test_worker_api_2(self):
    self.compile_btest([path_from_root('tests', 'worker_api_2_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-O2', '--minify', '0', '-s', 'EXPORTED_FUNCTIONS=["_one", "_two", "_three", "_four"]', '--closure', '1'])
    self.btest('worker_api_2_main.cpp', args=['-O2', '--minify', '0'], expected='11')

  def test_worker_api_3(self):
    self.compile_btest([path_from_root('tests', 'worker_api_3_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]'])
    self.btest('worker_api_3_main.cpp', expected='5')

  @no_wasm_backend('emterpretify')
  def test_worker_api_sleep(self):
    self.compile_btest([path_from_root('tests', 'worker_api_worker_sleep.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'])
    self.btest('worker_api_main.cpp', expected='566')

  def test_emscripten_async_wget2(self):
    self.btest('http.cpp', expected='0', args=['-I' + path_from_root('tests')])

  # TODO: test only worked in non-fastcomp
  @unittest.skip('non-fastcomp is deprecated and fails in 3.5')
  def test_module(self):
    self.compile_btest([path_from_root('tests', 'browser_module.cpp'), '-o', 'module.js', '-O2', '-s', 'SIDE_MODULE=1', '-s', 'DLOPEN_SUPPORT=1', '-s', 'EXPORTED_FUNCTIONS=["_one", "_two"]'])
    self.btest('browser_main.cpp', args=['-O2', '-s', 'MAIN_MODULE=1', '-s', 'DLOPEN_SUPPORT=1', '-s', 'EXPORT_ALL=1'], expected='8')

  @no_wasm_backend('dynamic linking')
  def test_preload_module(self):
    create_test_file('library.c', r'''
      #include <stdio.h>
      int library_func() {
        return 42;
      }
    ''')
    self.compile_btest(['library.c', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'library.wasm', '-s', 'WASM=1', '-s', 'EXPORT_ALL=1'])
    os.rename('library.wasm', 'library.so')
    main = r'''
      #include <dlfcn.h>
      #include <stdio.h>
      #include <emscripten.h>
      int main() {
        int found = EM_ASM_INT(
          return Module['preloadedWasm']['/library.so'] !== undefined;
        );
        if (!found) {
          REPORT_RESULT(1);
          return 1;
        }
        void *lib_handle = dlopen("/library.so", 0);
        if (!lib_handle) {
          REPORT_RESULT(2);
          return 2;
        }
        typedef int (*voidfunc)();
        voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
        if (!x || x() != 42) {
          REPORT_RESULT(3);
          return 3;
        }
        REPORT_RESULT(0);
        return 0;
      }
    '''
    self.btest(
      main,
      args=['-s', 'MAIN_MODULE=1', '--preload-file', '.@/', '-O2', '-s', 'WASM=1', '--use-preload-plugins', '-s', 'EXPORT_ALL=1'],
      expected='0')

  def test_mmap_file(self):
    create_test_file('data.dat', 'data from the file ' + ('.' * 9000))
    for extra_args in [[], ['--no-heap-copy']]:
      self.btest(path_from_root('tests', 'mmap_file.c'), expected='1', args=['--preload-file', 'data.dat'] + extra_args)

  def test_emrun_info(self):
    if not has_browser():
      self.skipTest('need a browser')
    result = run_process([PYTHON, path_from_root('emrun'), '--system_info', '--browser_info'], stdout=PIPE).stdout
    assert 'CPU' in result
    assert 'Browser' in result
    assert 'Traceback' not in result

    result = run_process([PYTHON, path_from_root('emrun'), '--list_browsers'], stdout=PIPE).stdout
    assert 'Traceback' not in result

  # Deliberately named as test_zzz_emrun to make this test the last one
  # as this test may take the focus away from the main test window
  # by opening a new window and possibly not closing it.
  def test_zzz_emrun(self):
    self.compile_btest([path_from_root('tests', 'test_emrun.c'), '--emrun', '-o', 'hello_world.html'])
    outdir = os.getcwd()
    if not has_browser():
      self.skipTest('need a browser')
    # We cannot run emrun from the temp directory the suite will clean up afterwards, since the browser that is launched will have that directory as startup directory,
    # and the browser will not close as part of the test, pinning down the cwd on Windows and it wouldn't be possible to delete it. Therefore switch away from that directory
    # before launching.
    os.chdir(path_from_root())
    args_base = [PYTHON, path_from_root('emrun'), '--timeout', '30', '--safe_firefox_profile', '--port', '6939', '--verbose', '--log_stdout', os.path.join(outdir, 'stdout.txt'), '--log_stderr', os.path.join(outdir, 'stderr.txt')]
    if EMTEST_BROWSER is not None:
      # If EMTEST_BROWSER carried command line arguments to pass to the browser,
      # (e.g. "firefox -profile /path/to/foo") those can't be passed via emrun,
      # so strip them out.
      browser_cmd = shlex.split(EMTEST_BROWSER)
      browser_path = browser_cmd[0]
      args_base += ['--browser', browser_path]
      if len(browser_cmd) > 1:
        browser_args = browser_cmd[1:]
        if 'firefox' in browser_path and '-profile' in browser_args:
          # emrun uses its own -profile, strip it out
          parser = argparse.ArgumentParser(add_help=False) # otherwise it throws with -headless
          parser.add_argument('-profile')
          browser_args = parser.parse_known_args(browser_args)[1]
        if browser_args:
          args_base += ['--browser_args', ' ' + ' '.join(browser_args)]
    for args in [
        args_base,
        args_base + ['--no_private_browsing']
    ]:
      args += [os.path.join(outdir, 'hello_world.html'), '1', '2', '--3']
      proc = run_process(args, check=False)
      stdout = open(os.path.join(outdir, 'stdout.txt'), 'r').read()
      stderr = open(os.path.join(outdir, 'stderr.txt'), 'r').read()
      assert proc.returncode == 100
      assert 'argc: 4' in stdout
      assert 'argv[3]: --3' in stdout
      assert 'hello, world!' in stdout
      assert 'Testing ASCII characters: !"$%&\'()*+,-./:;<=>?@[\\]^_`{|}~' in stdout
      assert 'Testing char sequences: %20%21 &auml;' in stdout
      assert 'hello, error stream!' in stderr

  # This does not actually verify anything except that --cpuprofiler and --memoryprofiler compiles.
  # Run interactive.test_cpuprofiler_memoryprofiler for interactive testing.
  @requires_graphics_hardware
  def test_cpuprofiler_memoryprofiler(self):
    self.btest('hello_world_gles.c', expected='0', args=['-DLONGTEST=1', '-DTEST_MEMORYPROFILER_ALLOCATIONS_MAP=1', '-O2', '--cpuprofiler', '--memoryprofiler', '-lGL', '-lglut'], timeout=30)

  def test_uuid(self):
    # Run with ./runner.py browser.test_uuid
    # We run this test in Node/SPIDERMONKEY and browser environments because we try to make use of
    # high quality crypto random number generators such as crypto.getRandomValues or randomBytes (if available).

    # First run tests in Node and/or SPIDERMONKEY using run_js. Use closure compiler so we can check that
    # require('crypto').randomBytes and window.crypto.getRandomValues doesn't get minified out.
    self.compile_btest(['-O2', '--closure', '1', path_from_root('tests', 'uuid', 'test.c'), '-o', 'test.js', '-luuid'])

    test_js_closure = open('test.js').read()

    # Check that test.js compiled with --closure 1 contains ").randomBytes" and "window.crypto.getRandomValues"
    assert ").randomBytes" in test_js_closure
    assert "window.crypto.getRandomValues" in test_js_closure

    out = run_js('test.js', full_output=True)
    print(out)

    # Tidy up files that might have been created by this test.
    try_delete(path_from_root('tests', 'uuid', 'test.js'))
    try_delete(path_from_root('tests', 'uuid', 'test.js.map'))

    # Now run test in browser
    self.btest(path_from_root('tests', 'uuid', 'test.c'), '1', args=['-luuid'])

  @requires_graphics_hardware
  def test_glew(self):
    self.btest(path_from_root('tests', 'glew.c'), args=['-lGL', '-lSDL', '-lGLEW'], expected='1')
    self.btest(path_from_root('tests', 'glew.c'), args=['-lGL', '-lSDL', '-lGLEW', '-s', 'LEGACY_GL_EMULATION=1'], expected='1')
    self.btest(path_from_root('tests', 'glew.c'), args=['-lGL', '-lSDL', '-lGLEW', '-DGLEW_MX'], expected='1')
    self.btest(path_from_root('tests', 'glew.c'), args=['-lGL', '-lSDL', '-lGLEW', '-s', 'LEGACY_GL_EMULATION=1', '-DGLEW_MX'], expected='1')

  def test_doublestart_bug(self):
    create_test_file('pre.js', r'''
if (!Module['preRun']) Module['preRun'] = [];
Module["preRun"].push(function () {
  addRunDependency('test_run_dependency');
  removeRunDependency('test_run_dependency');
});
''')

    self.btest('doublestart.c', args=['--pre-js', 'pre.js', '-o', 'test.html'], expected='1')

  @no_chrome('see #7930')
  @requires_threads
  def test_html5(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1'], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'test_html5.c'), args=['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'] + opts, expected='0', timeout=20)

  @requires_threads
  def test_html5_gamepad(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1'], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'test_gamepad.c'), args=['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'] + opts, expected='0', timeout=20)

  @requires_graphics_hardware
  def test_html5_webgl_create_context_no_antialias(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1'], ['-s', 'FULL_ES2=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'webgl_create_context.cpp'), args=opts + ['-DNO_ANTIALIAS', '-lGL'], expected='0', timeout=20)

  # This test supersedes the one above, but it's skipped in the CI because anti-aliasing is not well supported by the Mesa software renderer.
  @requires_threads
  @requires_graphics_hardware
  def test_html5_webgl_create_context(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1'], ['-s', 'FULL_ES2=1'], ['-s', 'USE_PTHREADS=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'webgl_create_context.cpp'), args=opts + ['-lGL'], expected='0', timeout=20)

  @requires_graphics_hardware
  # Verify bug https://github.com/emscripten-core/emscripten/issues/4556: creating a WebGL context to Module.canvas without an ID explicitly assigned to it.
  def test_html5_webgl_create_context2(self):
    self.btest(path_from_root('tests', 'webgl_create_context2.cpp'), args=['--shell-file', path_from_root('tests', 'webgl_create_context2_shell.html'), '-lGL'], expected='0', timeout=20)

  @requires_graphics_hardware
  def test_html5_webgl_destroy_context(self):
    for opts in [[], ['-O2', '-g1'], ['-s', 'FULL_ES2=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'webgl_destroy_context.cpp'), args=opts + ['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1', '--shell-file', path_from_root('tests/webgl_destroy_context_shell.html'), '-lGL'], expected='0', timeout=20)

  @no_chrome('see #7373')
  @requires_graphics_hardware
  def test_webgl_context_params(self):
    if WINDOWS:
      self.skipTest('SKIPPED due to bug https://bugzilla.mozilla.org/show_bug.cgi?id=1310005 - WebGL implementation advertises implementation defined GL_IMPLEMENTATION_COLOR_READ_TYPE/FORMAT pair that it cannot read with')
    self.btest(path_from_root('tests', 'webgl_color_buffer_readpixels.cpp'), args=['-lGL'], expected='0', timeout=20)

  # Test for PR#5373 (https://github.com/emscripten-core/emscripten/pull/5373)
  def test_webgl_shader_source_length(self):
    for opts in [[], ['-s', 'FULL_ES2=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'webgl_shader_source_length.cpp'), args=opts + ['-lGL'], expected='0', timeout=20)

  def test_webgl2(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1', '-s', 'WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG=1'], ['-s', 'FULL_ES2=1']]:
      print(opts)
      self.btest(path_from_root('tests', 'webgl2.cpp'), args=['-s', 'USE_WEBGL2=1', '-lGL'] + opts, expected='0')

  def test_webgl2_objects(self):
    self.btest(path_from_root('tests', 'webgl2_objects.cpp'), args=['-s', 'USE_WEBGL2=1', '-lGL'], expected='0')

  def test_webgl2_ubos(self):
    self.btest(path_from_root('tests', 'webgl2_ubos.cpp'), args=['-s', 'USE_WEBGL2=1', '-lGL'], expected='0')

  @requires_graphics_hardware
  def test_webgl2_garbage_free_entrypoints(self):
    self.btest(path_from_root('tests', 'webgl2_garbage_free_entrypoints.cpp'), args=['-s', 'USE_WEBGL2=1', '-DTEST_WEBGL2=1'], expected='1')
    self.btest(path_from_root('tests', 'webgl2_garbage_free_entrypoints.cpp'), expected='1')

  @requires_graphics_hardware
  def test_webgl2_backwards_compatibility_emulation(self):
    self.btest(path_from_root('tests', 'webgl2_backwards_compatibility_emulation.cpp'), args=['-s', 'USE_WEBGL2=1', '-s', 'WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1'], expected='0')

  @requires_graphics_hardware
  def test_webgl_with_closure(self):
    self.btest(path_from_root('tests', 'webgl_with_closure.cpp'), args=['-O2', '-s', 'USE_WEBGL2=1', '--closure', '1', '-lGL'], expected='0')

  # Tests that -s GL_ASSERTIONS=1 and glVertexAttribPointer with packed types works
  @requires_graphics_hardware
  def test_webgl2_packed_types(self):
    self.btest(path_from_root('tests', 'webgl2_draw_packed_triangle.c'), args=['-lGL', '-s', 'USE_WEBGL2=1', '-s', 'GL_ASSERTIONS=1'], expected='0')

  @requires_graphics_hardware
  def test_webgl2_pbo(self):
    self.btest(path_from_root('tests', 'webgl2_pbo.cpp'), args=['-s', 'USE_WEBGL2=1', '-lGL'], expected='0')

  def test_sdl_touch(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print(opts)
      self.btest(path_from_root('tests', 'sdl_touch.c'), args=opts + ['-DAUTOMATE_SUCCESS=1', '-lSDL', '-lGL'], expected='0')

  def test_html5_mouse(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print(opts)
      self.btest(path_from_root('tests', 'test_html5_mouse.c'), args=opts + ['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1', '-DAUTOMATE_SUCCESS=1'], expected='0')

  def test_sdl_mousewheel(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print(opts)
      self.btest(path_from_root('tests', 'test_sdl_mousewheel.c'), args=opts + ['-DAUTOMATE_SUCCESS=1', '-lSDL', '-lGL'], expected='0')

  @no_wasm_backend('asm.js-specific')
  def test_codemods(self):
    # tests asm.js client-side code modifications
    for opt_level in [0, 2]:
      print('opt level', opt_level)
      opts = ['-O' + str(opt_level), '-s', 'WASM=0']
      # sanity checks, building with and without precise float semantics generates different results
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='2', args=opts)
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='1', args=opts + ['-s', 'PRECISE_F32=1'])
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='1', args=opts + ['-s', 'PRECISE_F32=2', '--separate-asm']) # empty polyfill, but browser has support, so semantics are like float

  @no_wasm_backend('emterpretify')
  def test_wget(self):
    with open('test.txt', 'w') as f:
      f.write('emscripten')
    self.btest(path_from_root('tests', 'test_wget.c'), expected='1', args=['-s', 'ASYNCIFY=1'])
    print('asyncify+emterpreter')
    self.btest(path_from_root('tests', 'test_wget.c'), expected='1', args=['-s', 'ASYNCIFY=1', '-s', 'EMTERPRETIFY=1'])
    print('emterpreter by itself')
    self.btest(path_from_root('tests', 'test_wget.c'), expected='1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'])

  @no_wasm_backend('emterpretify')
  def test_wget_data(self):
    with open('test.txt', 'w') as f:
      f.write('emscripten')
    self.btest(path_from_root('tests', 'test_wget_data.c'), expected='1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O2', '-g2'])
    self.btest(path_from_root('tests', 'test_wget_data.c'), expected='1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O2', '-g2', '-s', 'ASSERTIONS=1'])

  def test_locate_file(self):
    for wasm in ([0, 1] if not self.is_wasm_backend() else [1]):
      print('wasm', wasm)
      self.clear()
      create_test_file('src.cpp', self.with_report_result(r'''
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
          int result = !strcmp("load me right before", buf);
          printf("|%s| : %d\n", buf, result);
          REPORT_RESULT(result);
          return 0;
        }
      '''))
      create_test_file('data.txt', 'load me right before...')
      create_test_file('pre.js', 'Module.locateFile = function(x) { return "sub/" + x };')
      run_process([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'data.txt'], stdout=open('data.js', 'w'))
      # put pre.js first, then the file packager data, so locateFile is there for the file loading code
      self.compile_btest(['src.cpp', '-O2', '-g', '--pre-js', 'pre.js', '--pre-js', 'data.js', '-o', 'page.html', '-s', 'FORCE_FILESYSTEM=1', '-s', 'WASM=' + str(wasm)])
      os.mkdir('sub')
      if wasm:
        shutil.move('page.wasm', os.path.join('sub', 'page.wasm'))
      else:
        shutil.move('page.html.mem', os.path.join('sub', 'page.html.mem'))
      shutil.move('test.data', os.path.join('sub', 'test.data'))
      self.run_browser('page.html', None, '/report_result?1')

      # alternatively, put locateFile in the HTML
      print('in html')

      create_test_file('shell.html', '''
        <body>
          <script>
            var Module = {
              locateFile: function(x) { return "sub/" + x }
            };
          </script>

          {{{ SCRIPT }}}
        </body>
      ''')

      def in_html(expected, args=[]):
        self.compile_btest(['src.cpp', '-O2', '-g', '--shell-file', 'shell.html', '--pre-js', 'data.js', '-o', 'page.html', '-s', 'SAFE_HEAP=1', '-s', 'ASSERTIONS=1', '-s', 'FORCE_FILESYSTEM=1', '-s', 'WASM=' + str(wasm)] + args)
        if wasm:
          shutil.move('page.wasm', os.path.join('sub', 'page.wasm'))
        else:
          shutil.move('page.html.mem', os.path.join('sub', 'page.html.mem'))
        self.run_browser('page.html', None, '/report_result?' + expected)

      in_html('1')

      # verify that the mem init request succeeded in the latter case
      if not wasm:
        create_test_file('src.cpp', self.with_report_result(r'''
  #include <stdio.h>
  #include <emscripten.h>

  int main() {
    int result = EM_ASM_INT({
      return Module['memoryInitializerRequest'].status;
    });
    printf("memory init request: %d\n", result);
    REPORT_RESULT(result);
    return 0;
  }
      '''))

        in_html('200')

  @requires_graphics_hardware
  def test_glfw3(self):
    for opts in [[], ['-Os', '--closure', '1']]:
      print(opts)
      self.btest(path_from_root('tests', 'glfw3.c'), args=['-s', 'LEGACY_GL_EMULATION=1', '-s', 'USE_GLFW=3', '-lglfw', '-lGL'] + opts, expected='1')

  @requires_graphics_hardware
  def test_glfw_events(self):
    self.btest(path_from_root('tests', 'glfw_events.c'), args=['-s', 'USE_GLFW=2', "-DUSE_GLFW=2", '-lglfw', '-lGL'], expected='1')
    self.btest(path_from_root('tests', 'glfw_events.c'), args=['-s', 'USE_GLFW=3', "-DUSE_GLFW=3", '-lglfw', '-lGL'], expected='1')

  @no_wasm_backend('asm.js')
  def test_asm_swapping(self):
    self.clear()
    create_test_file('run.js', r'''
Module['onRuntimeInitialized'] = function() {
  // test proper initial result
  var result = Module._func();
  console.log('first: ' + result);
  if (result !== 10) throw 'bad first result';

  // load second module to be swapped in
  var second = document.createElement('script');
  second.onload = function() { console.log('loaded second') };
  second.src = 'second.js';
  document.body.appendChild(second);
  console.log('second appended');

  Module['onAsmSwap'] = function() {
    console.log('swapped');
    // verify swapped-in result
    var result = Module._func();
    console.log('second: ' + result);
    if (result !== 22) throw 'bad second result';
    Module._report(999);
    console.log('reported');
  };
};
''')
    for opts in [[], ['-O1'], ['-O2', '-profiling'], ['-O2']]:
      print(opts)
      opts += ['-s', 'WASM=0', '--pre-js', 'run.js', '-s', 'SWAPPABLE_ASM_MODULE=1'] # important that both modules are built with the same opts
      create_test_file('second.cpp', self.with_report_result(open(path_from_root('tests', 'asm_swap2.cpp')).read()))
      self.compile_btest(['second.cpp'] + opts)
      run_process([PYTHON, path_from_root('tools', 'distill_asm.py'), 'a.out.js', 'second.js', 'swap-in'])
      self.assertExists('second.js')

      if SPIDERMONKEY_ENGINE in JS_ENGINES:
        out = run_js('second.js', engine=SPIDERMONKEY_ENGINE, stderr=PIPE, full_output=True, assert_returncode=None)
        self.validate_asmjs(out)
      else:
        print('Skipping asm validation check, spidermonkey is not configured')

      self.btest(path_from_root('tests', 'asm_swap.cpp'), args=opts, expected='999')

  @requires_graphics_hardware
  def test_sdl2_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file, and memory-init
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.jpg')
    create_test_file('sdl2_image.c', self.with_report_result(open(path_from_root('tests', 'sdl2_image.c')).read()))

    for mem in [0, 1]:
      for dest, dirname, basename in [('screenshot.jpg', '/', 'screenshot.jpg'),
                                      ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
        self.compile_btest([
          'sdl2_image.c', '-o', 'page.html', '-O2', '--memory-init-file', str(mem),
          '--preload-file', dest, '-DSCREENSHOT_DIRNAME="' + dirname + '"', '-DSCREENSHOT_BASENAME="' + basename + '"', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--use-preload-plugins'
        ])
        self.run_browser('page.html', '', '/report_result?600')

  @requires_graphics_hardware
  def test_sdl2_image_jpeg(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.jpeg')
    create_test_file('sdl2_image_jpeg.c', self.with_report_result(open(path_from_root('tests', 'sdl2_image.c')).read()))
    self.compile_btest([
      'sdl2_image_jpeg.c', '-o', 'page.html',
      '--preload-file', 'screenshot.jpeg', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--use-preload-plugins'
    ])
    self.run_browser('page.html', '', '/report_result?600')

  @requires_graphics_hardware
  def test_sdl2_image_formats(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl2_image.c', expected='512', args=['--preload-file', 'screenshot.png', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.png"',
                                                     '-DNO_PRELOADED', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '-s', 'SDL2_IMAGE_FORMATS=["png"]'])

  def test_sdl2_key(self):
    for defines in [[]]:
      create_test_file('pre.js', '''
        Module.postRun = function() {
          function doOne() {
            Module._one();
            setTimeout(doOne, 1000/60);
          }
          setTimeout(doOne, 1000/60);
        }

        function keydown(c) {
          var event = new KeyboardEvent("keydown", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
          var prevented = !document.dispatchEvent(event);

          //send keypress if not prevented
          if (!prevented) {
            var event = new KeyboardEvent("keypress", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
            document.dispatchEvent(event);
          }
        }

        function keyup(c) {
          var event = new KeyboardEvent("keyup", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
          document.dispatchEvent(event);
        }
      ''')
      create_test_file('sdl2_key.c', self.with_report_result(open(path_from_root('tests', 'sdl2_key.c')).read()))

      self.compile_btest(['sdl2_key.c', '-o', 'page.html'] + defines + ['-s', 'USE_SDL=2', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']'''])
      self.run_browser('page.html', '', '/report_result?37182145')

  def test_sdl2_text(self):
    create_test_file('pre.js', '''
      Module.postRun = function() {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }

      function simulateKeyEvent(c) {
        var event = new KeyboardEvent("keypress", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
        document.body.dispatchEvent(event);
      }
    ''')
    create_test_file('sdl2_text.c', self.with_report_result(open(path_from_root('tests', 'sdl2_text.c')).read()))

    self.compile_btest(['sdl2_text.c', '-o', 'page.html', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''', '-s', 'USE_SDL=2'])
    self.run_browser('page.html', '', '/report_result?1')

  @flaky
  @requires_graphics_hardware
  def test_sdl2_mouse(self):
    create_test_file('pre.js', '''
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
    create_test_file('sdl2_mouse.c', self.with_report_result(open(path_from_root('tests', 'sdl2_mouse.c')).read()))

    self.compile_btest(['sdl2_mouse.c', '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js', '-s', 'USE_SDL=2'])
    self.run_browser('page.html', '', '/report_result?1', timeout=30)

  @requires_graphics_hardware
  def test_sdl2_mouse_offsets(self):
    create_test_file('pre.js', '''
      function simulateMouseEvent(x, y, button) {
        var event = document.createEvent("MouseEvents");
        if (button >= 0) {
          var event1 = document.createEvent("MouseEvents");
          event1.initMouseEvent('mousedown', true, true, window,
                     1, x, y, x, y,
                     0, 0, 0, 0,
                     button, null);
          Module['canvas'].dispatchEvent(event1);
          var event2 = document.createEvent("MouseEvents");
          event2.initMouseEvent('mouseup', true, true, window,
                     1, x, y, x, y,
                     0, 0, 0, 0,
                     button, null);
          Module['canvas'].dispatchEvent(event2);
        } else {
          var event1 = document.createEvent("MouseEvents");
          event1.initMouseEvent('mousemove', true, true, window,
                     0, x, y, x, y,
                     0, 0, 0, 0,
                     0, null);
          Module['canvas'].dispatchEvent(event1);
        }
      }
      window['simulateMouseEvent'] = simulateMouseEvent;
    ''')
    create_test_file('page.html', '''
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
    create_test_file('sdl2_mouse.c', self.with_report_result(open(path_from_root('tests', 'sdl2_mouse.c')).read()))

    self.compile_btest(['sdl2_mouse.c', '-DTEST_SDL_MOUSE_OFFSETS=1', '-O2', '--minify', '0', '-o', 'sdl2_mouse.js', '--pre-js', 'pre.js', '-s', 'USE_SDL=2'])
    self.run_browser('page.html', '', '/report_result?1')

  @requires_threads
  def test_sdl2_threads(self):
      self.btest('sdl2_threads.c', expected='4', args=['-s', 'USE_PTHREADS=1', '-s', 'USE_SDL=2', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_graphics_hardware
  def test_sdl2glshader(self):
    self.btest('sdl2glshader.c', reference='sdlglshader.png', args=['-s', 'USE_SDL=2', '-O2', '--closure', '1', '-g1', '-s', 'LEGACY_GL_EMULATION=1'])
    self.btest('sdl2glshader.c', reference='sdlglshader.png', args=['-s', 'USE_SDL=2', '-O2', '-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True) # XXX closure fails on proxy

  @requires_graphics_hardware
  def test_sdl2_canvas_blank(self):
    self.btest('sdl2_canvas_blank.c', reference='sdl_canvas_blank.png', args=['-s', 'USE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_canvas_palette(self):
    self.btest('sdl2_canvas_palette.c', reference='sdl_canvas_palette.png', args=['-s', 'USE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_canvas_twice(self):
    self.btest('sdl2_canvas_twice.c', reference='sdl_canvas_twice.png', args=['-s', 'USE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_gfx(self):
    self.btest('sdl2_gfx.cpp', args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_GFX=2'], reference='sdl2_gfx.png', reference_slack=2)

  @requires_graphics_hardware
  def test_sdl2_canvas_palette_2(self):
    create_test_file('args-r.js', '''
      Module['arguments'] = ['-r'];
    ''')

    create_test_file('args-g.js', '''
      Module['arguments'] = ['-g'];
    ''')

    create_test_file('args-b.js', '''
      Module['arguments'] = ['-b'];
    ''')

    self.btest('sdl2_canvas_palette_2.c', reference='sdl_canvas_palette_r.png', args=['-s', 'USE_SDL=2', '--pre-js', 'args-r.js'])
    self.btest('sdl2_canvas_palette_2.c', reference='sdl_canvas_palette_g.png', args=['-s', 'USE_SDL=2', '--pre-js', 'args-g.js'])
    self.btest('sdl2_canvas_palette_2.c', reference='sdl_canvas_palette_b.png', args=['-s', 'USE_SDL=2', '--pre-js', 'args-b.js'])

  def test_sdl2_swsurface(self):
    self.btest('sdl2_swsurface.c', expected='1', args=['-s', 'USE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl2_image_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'], manually_trigger_reftest=True)

  @requires_graphics_hardware
  def test_sdl2_image_prepare_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), 'screenshot.not')
    self.btest('sdl2_image_prepare_data.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'], manually_trigger_reftest=True)

  @requires_graphics_hardware
  def test_sdl2_canvas_proxy(self):
    def post():
      html = open('test.html').read()
      html = html.replace('</body>', '''
<script>
function assert(x, y) { if (!x) throw 'assertion failed ' + y }

%s

var windowClose = window.close;
window.close = function() {
  // wait for rafs to arrive and the screen to update before reftesting
  setTimeout(function() {
    doReftest();
    setTimeout(windowClose, 5000);
  }, 1000);
};
</script>
</body>''' % open('reftest.js').read())
      create_test_file('test.html', html)

    create_test_file('data.txt', 'datum')

    self.btest('sdl2_canvas_proxy.c', reference='sdl2_canvas.png', args=['-s', 'USE_SDL=2', '--proxy-to-worker', '--preload-file', 'data.txt', '-s', 'GL_TESTING=1'], manual_reference=True, post_build=post)

  def test_sdl2_pumpevents(self):
    # key events should be detected using SDL_PumpEvents
    create_test_file('pre.js', '''
      function keydown(c) {
        var event = new KeyboardEvent("keydown", { 'keyCode': c, 'charCode': c, 'view': window, 'bubbles': true, 'cancelable': true });
        document.dispatchEvent(event);
      }
    ''')
    self.btest('sdl2_pumpevents.c', expected='7', args=['--pre-js', 'pre.js', '-s', 'USE_SDL=2'])

  def test_sdl2_timer(self):
    self.btest('sdl2_timer.c', expected='5', args=['-s', 'USE_SDL=2'])

  def test_sdl2_canvas_size(self):
    self.btest('sdl2_canvas_size.c', expected='1', args=['-s', 'USE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_gl_read(self):
    # SDL, OpenGL, readPixels
    create_test_file('sdl2_gl_read.c', self.with_report_result(open(path_from_root('tests', 'sdl2_gl_read.c')).read()))
    self.compile_btest(['sdl2_gl_read.c', '-o', 'something.html', '-s', 'USE_SDL=2'])
    self.run_browser('something.html', '.', '/report_result?1')

  @requires_graphics_hardware
  def test_sdl2_fog_simple(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl2_fog_simple.c', reference='screenshot-fog-simple.png',
               args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl2_fog_negative(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl2_fog_negative.c', reference='screenshot-fog-negative.png',
               args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl2_fog_density(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl2_fog_density.c', reference='screenshot-fog-density.png',
               args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl2_fog_exp2(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl2_fog_exp2.c', reference='screenshot-fog-exp2.png',
               args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
               message='You should see an image with fog.')

  @requires_graphics_hardware
  def test_sdl2_fog_linear(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), 'screenshot.png')
    self.btest('sdl2_fog_linear.c', reference='screenshot-fog-linear.png', reference_slack=1,
               args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
               message='You should see an image with fog.')

  def test_sdl2_unwasteful(self):
    self.btest('sdl2_unwasteful.cpp', expected='1', args=['-s', 'USE_SDL=2', '-O1'])

  def test_sdl2_canvas_write(self):
    self.btest('sdl2_canvas_write.cpp', expected='0', args=['-s', 'USE_SDL=2'])

  @requires_graphics_hardware
  def test_sdl2_gl_frames_swap(self):
    def post_build(*args):
      self.post_manual_reftest(*args)
      html = open('test.html').read()
      html2 = html.replace('''Module['postRun'] = doReftest;''', '') # we don't want the very first frame
      assert html != html2
      create_test_file('test.html', html2)
    self.btest('sdl2_gl_frames_swap.c', reference='sdl2_gl_frames_swap.png', args=['--proxy-to-worker', '-s', 'GL_TESTING=1', '-s', 'USE_SDL=2'], manual_reference=True, post_build=post_build)

  @requires_graphics_hardware
  def test_sdl2_ttf(self):
    shutil.copy2(path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), self.get_dir())
    self.btest('sdl2_ttf.c', reference='sdl2_ttf.png',
               args=['-O2', '-s', 'USE_SDL=2', '-s', 'USE_SDL_TTF=2', '--embed-file', 'LiberationSansBold.ttf'],
               message='You should see colorful "hello" and "world" in the window',
               timeout=30)

  def test_sdl2_custom_cursor(self):
    shutil.copyfile(path_from_root('tests', 'cursor.bmp'), 'cursor.bmp')
    self.btest('sdl2_custom_cursor.c', expected='1', args=['--preload-file', 'cursor.bmp', '-s', 'USE_SDL=2'])

  def test_sdl2_misc(self):
    self.btest('sdl2_misc.c', expected='1', args=['-s', 'USE_SDL=2'])
    print('also test building to object files first')
    src = open(path_from_root('tests', 'sdl2_misc.c')).read()
    create_test_file('test.c', self.with_report_result(src))
    run_process([PYTHON, EMCC, 'test.c', '-s', 'USE_SDL=2', '-o', 'test.o'])
    self.compile_btest(['test.o', '-s', 'USE_SDL=2', '-o', 'test.html'])
    self.run_browser('test.html', '...', '/report_result?1')

  @requires_sound_hardware
  def test_sdl2_mixer(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'alarmvictory_1.ogg'), 'sound.ogg')
    self.btest('sdl2_mixer.c', expected='1', args=['--preload-file', 'sound.ogg', '-s', 'USE_SDL=2', '-s', 'USE_SDL_MIXER=2', '-s', 'TOTAL_MEMORY=33554432'])

  @requires_sound_hardware
  def test_sdl2_mixer_wav(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.wav'), 'sound.wav')
    self.btest('sdl2_mixer_wav.c', expected='1', args=['--preload-file', 'sound.wav', '-s', 'USE_SDL=2', '-s', 'USE_SDL_MIXER=2', '-s', 'TOTAL_MEMORY=33554432'])

  @no_wasm_backend('cocos2d needs to be ported')
  @requires_graphics_hardware
  def test_cocos2d_hello(self):
    cocos2d_root = os.path.join(system_libs.Ports.get_build_dir(), 'cocos2d')
    preload_file = os.path.join(cocos2d_root, 'samples', 'HelloCpp', 'Resources') + '@'
    self.btest('cocos2d_hello.cpp', reference='cocos2d_hello.png', reference_slack=1,
               args=['-s', 'USE_COCOS2D=3', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=0', '--std=c++11', '--preload-file', preload_file, '--use-preload-plugins'],
               message='You should see Cocos2d logo',
               timeout=30)

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async(self):
    for opts in [0, 1, 2, 3]:
      print(opts)
      self.btest('emterpreter_async.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-g2'])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_2(self):
    # Error.stackTraceLimit default to 10 in chrome but this test relies on more
    # than 40 stack frames being reported.
    create_test_file('pre.js', 'Error.stackTraceLimit = 80;\n')
    self.btest('emterpreter_async_2.cpp', '40', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O3', '--pre-js', 'pre.js', ])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_virtual(self):
    for opts in [0, 1, 2, 3]:
      print(opts)
      self.btest('emterpreter_async_virtual.cpp', '5', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-profiling'])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_virtual_2(self):
    for opts in [0, 1, 2, 3]:
      print(opts)
      self.btest('emterpreter_async_virtual_2.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'ASSERTIONS=1', '-s', 'SAFE_HEAP=1', '-profiling'])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_bad(self):
    for opts in [0, 1, 2, 3]:
      print(opts)
      self.btest('emterpreter_async_bad.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'EMTERPRETIFY_BLACKLIST=["_middle"]', '-s', 'ASSERTIONS=1'])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_bad_2(self):
    for opts in [0, 1, 2, 3]:
      for assertions in [0, 1]:
        # without assertions, we end up continuing to run more non-emterpreted code in this testcase, returning 1
        # with assertions, we hit the emterpreter-async assertion on that, and report a  clear error
        expected = '2' if assertions else '1'
        print(opts, assertions, expected)
        self.btest('emterpreter_async_bad_2.cpp', expected, args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'EMTERPRETIFY_BLACKLIST=["_middle"]', '-s', 'ASSERTIONS=%s' % assertions, '-g'])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_mainloop(self):
    for opts in [0, 1, 2, 3]:
      print(opts)
      self.btest('emterpreter_async_mainloop.cpp', '121', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts)], timeout=20)

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_with_manual(self):
    for opts in [0, 1, 2, 3]:
      print(opts)
      self.btest('emterpreter_async_with_manual.cpp', '121', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'EMTERPRETIFY_BLACKLIST=["_acall"]'], timeout=20)

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_sleep2(self):
    self.btest('emterpreter_async_sleep2.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Oz'])

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_sleep2_safeheap(self):
    # check that safe-heap machinery does not cause errors in async operations
    self.btest('emterpreter_async_sleep2_safeheap.cpp', '17', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Oz', '-profiling', '-s', 'SAFE_HEAP=1', '-s', 'ASSERTIONS=1', '-s', 'EMTERPRETIFY_WHITELIST=["_main","_callback","_fix"]', '-s', 'EXIT_RUNTIME=1'])

  @no_wasm_backend('emterpretify')
  @requires_sound_hardware
  def test_sdl_audio_beep_sleep(self):
    self.btest('sdl_audio_beep_sleep.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Os', '-s', 'ASSERTIONS=1', '-s', 'DISABLE_EXCEPTION_CATCHING=0', '-profiling', '-s', 'SAFE_HEAP=1', '-lSDL'], timeout=90)

  @no_wasm_backend('emterpretify')
  def test_mainloop_reschedule(self):
    self.btest('mainloop_reschedule.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Os'], timeout=30)

  @no_wasm_backend('emterpretify')
  def test_mainloop_infloop(self):
    self.btest('mainloop_infloop.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'], timeout=30)

  @no_wasm_backend('emterpretify')
  def test_emterpreter_async_iostream(self):
    self.btest('emterpreter_async_iostream.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'])

  @requires_sync_compilation
  def test_modularize(self):
    for opts in [[], ['-O1'], ['-O2', '-profiling'], ['-O2'], ['-O2', '--closure', '1']]:
      for args, code in [
        ([], 'Module();'), # defaults
        # use EXPORT_NAME
        (['-s', 'EXPORT_NAME="HelloWorld"'], '''
          if (typeof Module !== "undefined") throw "what?!"; // do not pollute the global scope, we are modularized!
          HelloWorld.noInitialRun = true; // errorneous module capture will load this and cause timeout
          HelloWorld();
        '''),
        # pass in a Module option (which prevents main(), which we then invoke ourselves)
        (['-s', 'EXPORT_NAME="HelloWorld"'], '''
          var hello = HelloWorld({ noInitialRun: true, onRuntimeInitialized: function() {
            setTimeout(function() { hello._main(); }); // must be async, because onRuntimeInitialized may be called synchronously, so |hello| is not yet set!
          } });
        '''),
        # similar, but without a mem init file, everything is sync and simple
        (['-s', 'EXPORT_NAME="HelloWorld"', '--memory-init-file', '0'], '''
          var hello = HelloWorld({ noInitialRun: true});
          hello._main();
        '''),
        # use the then() API
        (['-s', 'EXPORT_NAME="HelloWorld"'], '''
          HelloWorld({ noInitialRun: true }).then(function(hello) {
            hello._main();
          });
        '''),
        # then() API, also note the returned value
        (['-s', 'EXPORT_NAME="HelloWorld"'], '''
          var helloOutside = HelloWorld({ noInitialRun: true }).then(function(hello) {
            setTimeout(function() {
              hello._main();
              if (hello !== helloOutside) throw 'helloOutside has not been set!'; // as we are async, helloOutside must have been set
            });
          });
        '''),
      ]:
        print('test on', opts, args, code)
        src = open(path_from_root('tests', 'browser_test_hello_world.c')).read()
        create_test_file('test.c', self.with_report_result(src))
        # this test is synchronous, so avoid async startup due to wasm features
        self.compile_btest(['test.c', '-s', 'MODULARIZE=1', '-s', 'BINARYEN_ASYNC_COMPILATION=0', '-s', 'SINGLE_FILE=1'] + args + opts)
        create_test_file('a.html', '''
          <script src="a.out.js"></script>
          <script>
            %s
          </script>
        ''' % code)
        self.run_browser('a.html', '...', '/report_result?0')

  # test illustrating the regression on the modularize feature since commit c5af8f6
  # when compiling with the --preload-file option
  @no_wasm_backend('cannot customize TOTAL_MEMORY in wasm at runtime')
  def test_modularize_and_preload_files(self):
    # amount of memory different from the default one that will be allocated for the emscripten heap
    totalMemory = 33554432
    for opts in [[], ['-O1'], ['-O2', '-profiling'], ['-O2'], ['-O2', '--closure', '1']]:
      # the main function simply checks that the amount of allocated heap memory is correct
      src = r'''
        #include <stdio.h>
        #include <emscripten.h>
        int main() {
          EM_ASM({
            // use eval here in order for the test with closure compiler enabled to succeed
            var totalMemory = Module['TOTAL_MEMORY'];
            assert(totalMemory === %d, 'bad memory size');
          });
          REPORT_RESULT(0);
          return 0;
        }
      ''' % totalMemory
      create_test_file('test.c', self.with_report_result(src))
      # generate a dummy file
      create_test_file('dummy_file', 'dummy')
      # compile the code with the modularize feature and the preload-file option enabled
      # no wasm, since this tests customizing total memory at runtime
      self.compile_btest(['test.c', '-s', 'WASM=0', '-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME="Foo"', '--preload-file', 'dummy_file'] + opts)
      create_test_file('a.html', '''
        <script src="a.out.js"></script>
        <script>
          // instantiate the Foo module with custom TOTAL_MEMORY value
          var foo = Foo({ TOTAL_MEMORY: %d });
        </script>
      ''' % totalMemory)
      self.run_browser('a.html', '...', '/report_result?0')

  def test_webidl(self):
    # see original in test_core.py
    run_process([PYTHON, path_from_root('tools', 'webidl_binder.py'),
                 path_from_root('tests', 'webidl', 'test.idl'),
                 'glue'])
    self.assertExists('glue.cpp')
    self.assertExists('glue.js')
    for opts in [[], ['-O1'], ['-O2']]:
      print(opts)
      self.btest(os.path.join('webidl', 'test.cpp'), '1', args=['--post-js', 'glue.js', '-I.', '-DBROWSER'] + opts)

  @no_wasm_backend('dynamic linking')
  @requires_sync_compilation
  def test_dynamic_link(self):
    create_test_file('pre.js', '''
      Module.dynamicLibraries = ['side.wasm'];
    ''')
    create_test_file('main.cpp', r'''
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      #include <emscripten.h>
      char *side(const char *data);
      int main() {
        char *temp = side("hello through side\n");
        char *ret = (char*)malloc(strlen(temp)+1);
        strcpy(ret, temp);
        temp[1] = 'x';
        EM_ASM({
          Module.realPrint = out;
          out = function(x) {
            if (!Module.printed) Module.printed = x;
            Module.realPrint(x);
          };
        });
        puts(ret);
        EM_ASM({ assert(Module.printed === 'hello through side', ['expected', Module.printed]); });
        REPORT_RESULT(2);
        return 0;
      }
    ''')
    create_test_file('side.cpp', r'''
      #include <stdlib.h>
      #include <string.h>
      char *side(const char *data);
      char *side(const char *data) {
        char *ret = (char*)malloc(strlen(data)+1);
        strcpy(ret, data);
        return ret;
      }
    ''')
    run_process([PYTHON, EMCC, 'side.cpp', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'side.wasm', '-s', 'EXPORT_ALL=1'])
    self.btest(self.in_dir('main.cpp'), '2', args=['-s', 'MAIN_MODULE=1', '-O2', '--pre-js', 'pre.js', '-s', 'EXPORT_ALL=1'])

    print('wasm in worker (we can read binary data synchronously there)')

    create_test_file('pre.js', '''
      var Module = { dynamicLibraries: ['side.wasm'] };
  ''')
    run_process([PYTHON, EMCC, 'side.cpp', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'side.wasm', '-s', 'WASM=1', '-s', 'EXPORT_ALL=1'])
    self.btest(self.in_dir('main.cpp'), '2', args=['-s', 'MAIN_MODULE=1', '-O2', '--pre-js', 'pre.js', '-s', 'WASM=1', '--proxy-to-worker', '-s', 'EXPORT_ALL=1'])

    print('wasm (will auto-preload since no sync binary reading)')

    create_test_file('pre.js', '''
      Module.dynamicLibraries = ['side.wasm'];
  ''')
    # same wasm side module works
    self.btest(self.in_dir('main.cpp'), '2', args=['-s', 'MAIN_MODULE=1', '-O2', '--pre-js', 'pre.js', '-s', 'WASM=1', '-s', 'EXPORT_ALL=1'])

  # verify that dynamic linking works in all kinds of in-browser environments.
  # don't mix different kinds in a single test.
  def test_dylink_dso_needed_wasm(self):
    self._test_dylink_dso_needed(1, 0)

  def test_dylink_dso_needed_wasm_inworker(self):
    self._test_dylink_dso_needed(1, 1)

  def test_dylink_dso_needed_asmjs(self):
    self._test_dylink_dso_needed(0, 0)

  def test_dylink_dso_needed_asmjs_inworker(self):
    self._test_dylink_dso_needed(0, 1)

  @requires_sync_compilation
  def _test_dylink_dso_needed(self, wasm, inworker):
    # here we reuse runner._test_dylink_dso_needed, but the code is run via browser.
    print('\n# wasm=%d inworker=%d' % (wasm, inworker))
    self.set_setting('WASM', wasm)
    self.emcc_args += ['-O2']

    def do_run(src, expected_output):
      # XXX there is no infrastructure (yet ?) to retrieve stdout from browser in tests.
      # -> do the assert about expected output inside browser.
      #
      # we have to put the hook into post.js because in main it is too late
      # (in main we won't be able to catch what static constructors inside
      # linked dynlibs printed), and in pre.js it is too early (out is not yet
      # setup by the shell).
      create_test_file('post.js', r'''
          Module.realPrint = out;
          out = function(x) {
            if (!Module.printed) Module.printed = "";
            Module.printed += x + '\n'; // out is passed str without last \n
            Module.realPrint(x);
          };
        ''')
      src += r'''
        int main() {
          _main();
          EM_ASM({
            var expected = %r;
            assert(Module.printed === expected, ['stdout expected:', expected]);
          });
          REPORT_RESULT(0);
        }
      ''' % (expected_output,)
      # --proxy-to-worker only on main
      if inworker:
        self.emcc_args += ['--proxy-to-worker']
      self.btest(src, '0', args=self.get_emcc_args() + ['--post-js', 'post.js'])

    super(browser, self)._test_dylink_dso_needed(do_run)

  @no_wasm_backend('dynamic linking')
  @requires_graphics_hardware
  @requires_sync_compilation
  def test_dynamic_link_glemu(self):
    create_test_file('pre.js', '''
      Module.dynamicLibraries = ['side.wasm'];
  ''')
    create_test_file('main.cpp', r'''
      #include <stdio.h>
      #include <string.h>
      #include <assert.h>
      const char *side();
      int main() {
        const char *exts = side();
        puts(side());
        assert(strstr(exts, "GL_EXT_texture_env_combine"));
        REPORT_RESULT(1);
        return 0;
      }
    ''')
    create_test_file('side.cpp', r'''
      #include "SDL/SDL.h"
      #include "SDL/SDL_opengl.h"
      const char *side() {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_SetVideoMode(600, 600, 16, SDL_OPENGL);
        return (const char *)glGetString(GL_EXTENSIONS);
      }
    ''')
    run_process([PYTHON, EMCC, 'side.cpp', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'side.wasm', '-lSDL', '-s', 'EXPORT_ALL=1'])

    self.btest(self.in_dir('main.cpp'), '1', args=['-s', 'MAIN_MODULE=1', '-O2', '-s', 'LEGACY_GL_EMULATION=1', '-lSDL', '-lGL', '--pre-js', 'pre.js', '-s', 'EXPORT_ALL=1'])

  def test_memory_growth_during_startup(self):
    create_test_file('data.dat', 'X' * (30 * 1024 * 1024))
    self.btest('browser_test_hello_world.c', '0', args=['-s', 'ASSERTIONS=1', '-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'TOTAL_MEMORY=16MB', '-s', 'TOTAL_STACK=16384', '--preload-file', 'data.dat'])

  # pthreads tests

  def prep_no_SAB(self):
    create_test_file('html.html', open(path_from_root('src', 'shell_minimal.html')).read().replace('''<body>''', '''<body>
      <script>
        SharedArrayBuffer = undefined;
        Atomics = undefined;
      </script>
    '''))

  # Test that the emscripten_ atomics api functions work.
  @requires_threads
  def test_pthread_atomics(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_atomics.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test 64-bit atomics.
  @requires_threads
  def test_pthread_64bit_atomics(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_64bit_atomics.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test 64-bit C++11 atomics.
  @requires_threads
  def test_pthread_64bit_cxx11_atomics(self):
    for opt in [['-O0'], ['-O3']]:
      for pthreads in [[], ['-s', 'USE_PTHREADS=1']]:
        self.btest(path_from_root('tests', 'pthread', 'test_pthread_64bit_cxx11_atomics.cpp'), expected='0', args=opt + pthreads + ['-std=c++11'])

  # Test the old GCC atomic __sync_fetch_and_op builtin operations.
  @requires_threads
  def test_pthread_gcc_atomic_fetch_and_op(self):
    # We need to resort to using regexes to optimize out SharedArrayBuffer when pthreads are not supported, which is brittle!
    # Therefore perform very extensive testing of different codegen modes to catch any problems.
    for opt in [[], ['-O1'], ['-O2'], ['-O3'], ['-O3', '-s', 'AGGRESSIVE_VARIABLE_ELIMINATION=1'], ['-Os'], ['-Oz']]:
      for debug in [[], ['-g1'], ['-g2'], ['-g4']]:
        for f32 in [[], ['-s', 'PRECISE_F32=1', '--separate-asm', '-s', 'WASM=0']]:
          print(opt, debug, f32)
          self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_atomic_fetch_and_op.cpp'), expected='0', args=opt + debug + f32 + ['-s', 'TOTAL_MEMORY=64MB', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # 64 bit version of the above test.
  @requires_threads
  def test_pthread_gcc_64bit_atomic_fetch_and_op(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_64bit_atomic_fetch_and_op.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'], also_asmjs=True)

  # Test the old GCC atomic __sync_op_and_fetch builtin operations.
  @requires_threads
  def test_pthread_gcc_atomic_op_and_fetch(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_atomic_op_and_fetch.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'], also_asmjs=True)

  # 64 bit version of the above test.
  @requires_threads
  def test_pthread_gcc_64bit_atomic_op_and_fetch(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_64bit_atomic_op_and_fetch.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'], also_asmjs=True)

  # Tests the rest of the remaining GCC atomics after the two above tests.
  @requires_threads
  def test_pthread_gcc_atomics(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_atomics.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test the __sync_lock_test_and_set and __sync_lock_release primitives.
  @requires_threads
  def test_pthread_gcc_spinlock(self):
    for arg in [[], ['-DUSE_EMSCRIPTEN_INTRINSICS']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_spinlock.cpp'), expected='800', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'] + arg, also_asmjs=True)

  # Test that basic thread creation works.
  @requires_threads
  def test_pthread_create(self):
    def test(args):
      print(args)
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_create.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'] + args)

    test([])
    test(['-O3'])
    test(['-s', 'MODULARIZE_INSTANCE=1'])

  # Tests the -s PROXY_TO_PTHREAD=1 option.
  @requires_threads
  def test_pthread_proxy_to_pthread(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_proxy_to_pthread.c'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'], timeout=30)

  # Test that a pthread can spawn another pthread of its own.
  @requires_threads
  def test_pthread_create_pthread(self):
    for modularize in [[], ['-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=MyModule', '--shell-file', path_from_root('tests', 'shell_that_launches_modularize.html')]]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_create_pthread.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2'] + modularize)

  # Test another case of pthreads spawning pthreads, but this time the callers immediately join on the threads they created.
  @requires_threads
  def test_pthread_nested_spawns(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_nested_spawns.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2'])

  # Test that main thread can wait for a pthread to finish via pthread_join().
  @requires_threads
  def test_pthread_join(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_join.cpp'), expected='6765', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test pthread_cancel() operation
  @requires_threads
  def test_pthread_cancel(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_cancel.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test pthread_kill() operation
  @no_chrome('pthread_kill hangs chrome renderer, and keep subsequent tests from passing')
  @requires_threads
  def test_pthread_kill(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_kill.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test that pthread cleanup stack (pthread_cleanup_push/_pop) works.
  @requires_threads
  def test_pthread_cleanup(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_cleanup.cpp'), expected='907640832', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Tests the pthread mutex api.
  @requires_threads
  def test_pthread_mutex(self):
    for arg in [[], ['-DSPINLOCK_TEST']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_mutex.cpp'), expected='50', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'] + arg)

  # Test that memory allocation is thread-safe.
  @requires_threads
  def test_pthread_malloc(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_malloc.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Stress test pthreads allocating memory that will call to sbrk(), and main thread has to free up the data.
  @requires_threads
  def test_pthread_malloc_free(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_malloc_free.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8', '-s', 'TOTAL_MEMORY=256MB'])

  # Test that the pthread_barrier API works ok.
  @requires_threads
  def test_pthread_barrier(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_barrier.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test the pthread_once() function.
  @requires_threads
  def test_pthread_once(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_once.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test against a certain thread exit time handling bug by spawning tons of threads.
  @requires_threads
  def test_pthread_spawns(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_spawns.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # It is common for code to flip volatile global vars for thread control. This is a bit lax, but nevertheless, test whether that
  # kind of scheme will work with Emscripten as well.
  @requires_threads
  def test_pthread_volatile(self):
    for arg in [[], ['-DUSE_C_VOLATILE']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_volatile.cpp'), expected='1', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'] + arg)

  # Test thread-specific data (TLS).
  @requires_threads
  def test_pthread_thread_local_storage(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_thread_local_storage.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test the pthread condition variable creation and waiting.
  @requires_threads
  def test_pthread_condition_variable(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_condition_variable.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8'])

  # Test that pthreads are able to do printf.
  @requires_threads
  def test_pthread_printf(self):
    def run(debug):
       self.btest(path_from_root('tests', 'pthread', 'test_pthread_printf.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1', '-s', 'LIBRARY_DEBUG=%d' % debug])

    run(debug=True)
    run(debug=False)

  # Test that pthreads are able to do cout. Failed due to https://bugzilla.mozilla.org/show_bug.cgi?id=1154858.
  @requires_threads
  def test_pthread_iostream(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_iostream.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Test that the main thread is able to use pthread_set/getspecific.
  @requires_threads
  def test_pthread_setspecific_mainthread(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_setspecific_mainthread.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1'], also_asmjs=True)

  # Test the -s PTHREAD_HINT_NUM_CORES=x command line variable.
  @requires_threads
  def test_pthread_num_logical_cores(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_num_logical_cores.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_HINT_NUM_CORES=2'], also_asmjs=True)

  # Test that pthreads have access to filesystem.
  @requires_threads
  def test_pthread_file_io(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_file_io.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Test that the pthread_create() function operates benignly in the case that threading is not supported.
  @requires_threads
  def test_pthread_supported(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_supported.cpp'), expected='0', args=['-O3'] + args)

  # Test that --separate-asm works with -s USE_PTHREADS=1.
  @no_wasm_backend('asm.js')
  @requires_threads
  def test_pthread_separate_asm_pthreads(self):
    for modularize in [[], ['-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=MyModule', '--shell-file', path_from_root('tests', 'shell_that_launches_modularize.html')]]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_atomics.cpp'), expected='0', args=['-s', 'TOTAL_MEMORY=64MB', '-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8', '--separate-asm', '--profiling'] + modularize)

  # Test the operation of Module.pthreadMainPrefixURL variable
  @requires_threads
  def test_pthread_custom_pthread_main_url(self):
    self.clear()
    os.makedirs('cdn')
    create_test_file('main.cpp', self.with_report_result(r'''
      #include <stdio.h>
      #include <string.h>
      #include <emscripten/emscripten.h>
      #include <emscripten/threading.h>
      #include <pthread.h>
      int result = 0;
      void *thread_main(void *arg) {
        emscripten_atomic_store_u32(&result, 1);
        pthread_exit(0);
      }

      int main() {
        pthread_t t;
        if (emscripten_has_threading_support()) {
          pthread_create(&t, 0, thread_main, 0);
          pthread_join(t, 0);
        } else {
          result = 1;
        }
        REPORT_RESULT(result);
      }
    '''))

    # Test that it is possible to define "Module.locateFile" string to locate where worker.js will be loaded from.
    create_test_file('shell.html', open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { locateFile: function (path, prefix) {if (path.endsWith(".wasm")) {return prefix + path;} else {return "cdn/" + path;}}, '))
    self.compile_btest(['main.cpp', '--shell-file', 'shell.html', '-s', 'WASM=0', '-s', 'IN_TEST_HARNESS=1', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1', '-o', 'test.html'])
    shutil.move('test.worker.js', os.path.join('cdn', 'test.worker.js'))
    self.run_browser('test.html', '', '/report_result?1')

    # Test that it is possible to define "Module.locateFile(foo)" function to locate where worker.js will be loaded from.
    create_test_file('shell2.html', open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { locateFile: function(filename) { if (filename == "test.worker.js") return "cdn/test.worker.js"; else return filename; }, '))
    self.compile_btest(['main.cpp', '--shell-file', 'shell2.html', '-s', 'WASM=0', '-s', 'IN_TEST_HARNESS=1', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1', '-o', 'test2.html'])
    try_delete('test.worker.js')
    self.run_browser('test2.html', '', '/report_result?1')

  # Test that if the main thread is performing a futex wait while a pthread needs it to do a proxied operation (before that pthread would wake up the main thread), that it's not a deadlock.
  @requires_threads
  def test_pthread_proxying_in_futex_wait(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_proxying_in_futex_wait.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Test that sbrk() operates properly in multithreaded conditions
  @requires_threads
  def test_pthread_sbrk(self):
    for aborting_malloc in [0, 1]:
      print('aborting malloc=' + str(aborting_malloc))
      # With aborting malloc = 1, test allocating memory in threads
      # With aborting malloc = 0, allocate so much memory in threads that some of the allocations fail.
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_sbrk.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8', '--separate-asm', '-s', 'ABORTING_MALLOC=' + str(aborting_malloc), '-DABORTING_MALLOC=' + str(aborting_malloc), '-s', 'TOTAL_MEMORY=128MB'])

  # Test that -s ABORTING_MALLOC=0 works in both pthreads and non-pthreads builds. (sbrk fails gracefully)
  @requires_threads
  def test_pthread_gauge_available_memory(self):
    for opts in [[], ['-O2']]:
      for args in [[], ['-s', 'USE_PTHREADS=1']]:
        self.btest(path_from_root('tests', 'gauge_available_memory.cpp'), expected='1', args=['-s', 'ABORTING_MALLOC=0'] + args + opts)

  # Test that the proxying operations of user code from pthreads to main thread work
  @requires_threads
  def test_pthread_run_on_main_thread(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_run_on_main_thread.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Test how a lot of back-to-back called proxying operations behave.
  @requires_threads
  def test_pthread_run_on_main_thread_flood(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_run_on_main_thread_flood.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Test that it is possible to synchronously call a JavaScript function on the main thread and get a return value back.
  @requires_threads
  def test_pthread_call_sync_on_main_thread(self):
    self.btest(path_from_root('tests', 'pthread', 'call_sync_on_main_thread.c'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1', '-DPROXY_TO_PTHREAD=1', '--js-library', path_from_root('tests', 'pthread', 'call_sync_on_main_thread.js')])
    self.btest(path_from_root('tests', 'pthread', 'call_sync_on_main_thread.c'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-DPROXY_TO_PTHREAD=0', '--js-library', path_from_root('tests', 'pthread', 'call_sync_on_main_thread.js')])
    self.btest(path_from_root('tests', 'pthread', 'call_sync_on_main_thread.c'), expected='1', args=['-Oz', '-DPROXY_TO_PTHREAD=0', '--js-library', path_from_root('tests', 'pthread', 'call_sync_on_main_thread.js')])

  # Test that it is possible to asynchronously call a JavaScript function on the main thread.
  @requires_threads
  def test_pthread_call_async_on_main_thread(self):
    self.btest(path_from_root('tests', 'pthread', 'call_async_on_main_thread.c'), expected='7', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1', '-DPROXY_TO_PTHREAD=1', '--js-library', path_from_root('tests', 'pthread', 'call_async_on_main_thread.js')])
    self.btest(path_from_root('tests', 'pthread', 'call_async_on_main_thread.c'), expected='7', args=['-O3', '-s', 'USE_PTHREADS=1', '-DPROXY_TO_PTHREAD=0', '--js-library', path_from_root('tests', 'pthread', 'call_async_on_main_thread.js')])
    self.btest(path_from_root('tests', 'pthread', 'call_async_on_main_thread.c'), expected='7', args=['-Oz', '-DPROXY_TO_PTHREAD=0', '--js-library', path_from_root('tests', 'pthread', 'call_async_on_main_thread.js')])

  # Tests that spawning a new thread does not cause a reinitialization of the global data section of the application memory area.
  @requires_threads
  def test_pthread_global_data_initialization(self):
    for mem_init_mode in [[], ['--memory-init-file', '0'], ['--memory-init-file', '1'], ['-s', 'MEM_INIT_METHOD=2', '-s', 'WASM=0']]:
      for args in [['-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=MyModule', '--shell-file', path_from_root('tests', 'shell_that_launches_modularize.html')], ['-O3']]:
        self.btest(path_from_root('tests', 'pthread', 'test_pthread_global_data_initialization.c'), expected='20', args=args + mem_init_mode + ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  @requires_threads
  @requires_sync_compilation
  def test_pthread_global_data_initialization_in_sync_compilation_mode(self):
    for mem_init_mode in [[], ['--memory-init-file', '0'], ['--memory-init-file', '1'], ['-s', 'MEM_INIT_METHOD=2', '-s', 'WASM=0']]:
      args = ['-s', 'BINARYEN_ASYNC_COMPILATION=0']
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_global_data_initialization.c'), expected='20', args=args + mem_init_mode + ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Test that emscripten_get_now() reports coherent wallclock times across all pthreads, instead of each pthread independently reporting wallclock times since the launch of that pthread.
  @requires_threads
  def test_pthread_clock_drift(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_clock_drift.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_pthread_utf8_funcs(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_utf8_funcs.cpp'), expected='0', args=['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1'])

  # Tests MAIN_THREAD_EM_ASM_INT() function call signatures.
  @no_wasm_backend('MAIN_THREAD_EM_ASM() not yet implemented in Wasm backend')
  def test_main_thread_em_asm_signatures(self):
    self.btest(path_from_root('tests', 'core', 'test_em_asm_signatures.cpp'), expected='121', args=[])

  @no_wasm_backend('MAIN_THREAD_EM_ASM() not yet implemented in Wasm backend')
  @requires_threads
  def test_main_thread_em_asm_signatures_pthreads(self):
    self.btest(path_from_root('tests', 'core', 'test_em_asm_signatures.cpp'), expected='121', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'ASSERTIONS=1'])

  # test atomicrmw i64
  @no_wasm_backend('uses an asm.js .ll file')
  @requires_threads
  def test_atomicrmw_i64(self):
    # TODO: enable this with wasm, currently pthreads/atomics have limitations
    self.compile_btest([path_from_root('tests', 'atomicrmw_i64.ll'), '-s', 'USE_PTHREADS=1', '-s', 'IN_TEST_HARNESS=1', '-o', 'test.html', '-s', 'WASM=0'])
    self.run_browser('test.html', None, '/report_result?0')

  # Test that it is possible to send a signal via calling alarm(timeout), which in turn calls to the signal handler set by signal(SIGALRM, func);
  def test_sigalrm(self):
    self.btest(path_from_root('tests', 'sigalrm.cpp'), expected='0', args=['-O3'], timeout=30)

  @no_wasm_backend('mem init file')
  def test_meminit_pairs(self):
    d = 'const char *data[] = {\n  "'
    d += '",\n  "'.join(''.join('\\x{:02x}\\x{:02x}'.format(i, j)
                                for j in range(256)) for i in range(256))
    with open(path_from_root('tests', 'meminit_pairs.c')) as f:
      d += '"\n};\n' + f.read()
    args = ["-O2", "--memory-init-file", "0", "-s", "MEM_INIT_METHOD=2", "-s", "ASSERTIONS=1", '-s', 'WASM=0']
    self.btest(d, expected='0', args=args + ["--closure", "0"])
    self.btest(d, expected='0', args=args + ["--closure", "0", "-g"])
    self.btest(d, expected='0', args=args + ["--closure", "1"])

  @no_wasm_backend('mem init file')
  def test_meminit_big(self):
    d = 'const char *data[] = {\n  "'
    d += '",\n  "'.join([''.join('\\x{:02x}\\x{:02x}'.format(i, j)
                                 for j in range(256)) for i in range(256)] * 256)
    with open(path_from_root('tests', 'meminit_pairs.c')) as f:
      d += '"\n};\n' + f.read()
    assert len(d) > (1 << 27) # more than 32M memory initializer
    args = ["-O2", "--memory-init-file", "0", "-s", "MEM_INIT_METHOD=2", "-s", "ASSERTIONS=1", '-s', 'WASM=0']
    self.btest(d, expected='0', args=args + ["--closure", "0"])
    self.btest(d, expected='0', args=args + ["--closure", "0", "-g"])
    self.btest(d, expected='0', args=args + ["--closure", "1"])

  def test_canvas_style_proxy(self):
    self.btest('canvas_style_proxy.c', expected='1', args=['--proxy-to-worker', '--shell-file', path_from_root('tests/canvas_style_proxy_shell.html'), '--pre-js', path_from_root('tests/canvas_style_proxy_pre.js')])

  def test_canvas_size_proxy(self):
    self.btest(path_from_root('tests', 'canvas_size_proxy.c'), expected='0', args=['--proxy-to-worker'])

  def test_custom_messages_proxy(self):
    self.btest(path_from_root('tests', 'custom_messages_proxy.c'), expected='1', args=['--proxy-to-worker', '--shell-file', path_from_root('tests', 'custom_messages_proxy_shell.html'), '--post-js', path_from_root('tests', 'custom_messages_proxy_postjs.js')])

  @no_wasm_backend('asm.js')
  def test_separate_asm(self):
    for opts in [['-O0'], ['-O1'], ['-O2'], ['-O2', '--closure', '1']]:
      print(opts)
      create_test_file('src.cpp', self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))
      self.compile_btest(['src.cpp', '-o', 'test.html', '-s', 'WASM=0'] + opts)
      self.run_browser('test.html', None, '/report_result?0')

      print('run one')
      create_test_file('one.html', '<script src="test.js"></script>')
      self.run_browser('one.html', None, '/report_result?0')

      print('run two')
      run_process([PYTHON, path_from_root('tools', 'separate_asm.py'), 'test.js', 'asm.js', 'rest.js'])
      create_test_file('two.html', '''
        <script>
          var Module = {};
        </script>
        <script src="asm.js"></script>
        <script src="rest.js"></script>
      ''')
      self.run_browser('two.html', None, '/report_result?0')

      print('run hello world')
      self.clear()
      assert not os.path.exists('tests.asm.js')
      self.btest('browser_test_hello_world.c', expected='0', args=opts + ['-s', 'WASM=0', '--separate-asm'])
      self.assertExists('test.asm.js')
      os.unlink('test.asm.js')

      print('see a fail')
      self.run_browser('test.html', None, '[no http server activity]', timeout=5) # fail without the asm

  @no_wasm_backend('emterpretify')
  def test_emterpretify_file(self):
    create_test_file('shell.html', '''
      <!--
        {{{ SCRIPT }}} // ignore this, we do it ourselves
      -->
      <script>
        var Module = {};
        var xhr = new XMLHttpRequest();
        xhr.open('GET', 'code.dat', true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = function() {
          Module.emterpreterFile = xhr.response;
          var script = document.createElement('script');
          script.src = "test.js";
          document.body.appendChild(script);
        };
        xhr.send(null);
      </script>
''')
    try_delete('code.dat')
    self.btest('browser_test_hello_world.c', expected='0', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_FILE="code.dat"', '-O2', '-g', '--shell-file', 'shell.html', '-s', 'ASSERTIONS=1'])
    self.assertExists('code.dat')

    try_delete('code.dat')
    self.btest('browser_test_hello_world.c', expected='0', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_FILE="code.dat"', '-O2', '-g', '-s', 'ASSERTIONS=1'])
    self.assertExists('code.dat')

  def test_vanilla_html_when_proxying(self):
    for opts in [0, 1, 2]:
      print(opts)
      create_test_file('src.cpp', self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))
      self.compile_btest(['src.cpp', '-o', 'test.js', '-O' + str(opts), '--proxy-to-worker'])
      create_test_file('test.html', '<script src="test.js"></script>')
      self.run_browser('test.html', None, '/report_result?0')

  @no_wasm_backend('mem init file')
  def test_in_flight_memfile_request(self):
    # test the XHR for an asm.js mem init file being in flight already
    for o in [0, 1, 2]:
      print(o)
      opts = ['-O' + str(o), '-s', 'WASM=0']

      print('plain html')
      create_test_file('src.cpp', self.with_report_result(open(path_from_root('tests', 'in_flight_memfile_request.c')).read()))
      self.compile_btest(['src.cpp', '-o', 'test.js'] + opts)
      create_test_file('test.html', '<script src="test.js"></script>')
      self.run_browser('test.html', None, '/report_result?0') # never when we provide our own HTML like this.

      print('default html')
      self.btest('in_flight_memfile_request.c', expected='0' if o < 2 else '1', args=opts) # should happen when there is a mem init file (-O2+)

  @requires_sync_compilation
  def test_binaryen_async(self):
    # notice when we use async compilation
    script = '''
    <script>
      // note if we do async compilation
      var real_wasm_instantiate = WebAssembly.instantiate;
      var real_wasm_instantiateStreaming = WebAssembly.instantiateStreaming;
      if (typeof real_wasm_instantiateStreaming === 'function') {
        WebAssembly.instantiateStreaming = function(a, b) {
          Module.sawAsyncCompilation = true;
          return real_wasm_instantiateStreaming(a, b);
        };
      } else {
        WebAssembly.instantiate = function(a, b) {
          Module.sawAsyncCompilation = true;
          return real_wasm_instantiate(a, b);
        };
      }
      // show stderr for the viewer's fun
      err = function(x) {
        out('<<< ' + x + ' >>>');
        console.log(x);
      };
    </script>
    {{{ SCRIPT }}}
'''
    shell_with_script('shell.html', 'shell.html', script)
    common_args = ['--shell-file', 'shell.html']
    for opts, expect in [
      ([], 1),
      (['-O1'], 1),
      (['-O2'], 1),
      (['-O3'], 1),
      (['-s', 'BINARYEN_ASYNC_COMPILATION=1'], 1), # force it on
      (['-O1', '-s', 'BINARYEN_ASYNC_COMPILATION=0'], 0), # force it off
    ]:
      print(opts, expect)
      self.btest('binaryen_async.c', expected=str(expect), args=common_args + opts)
    # Ensure that compilation still works and is async without instantiateStreaming available
    no_streaming = ' <script> WebAssembly.instantiateStreaming = undefined;</script>'
    shell_with_script('shell.html', 'shell.html', no_streaming + script)
    self.btest('binaryen_async.c', expected='1', args=common_args)

  # Test that implementing Module.instantiateWasm() callback works.
  def test_manual_wasm_instantiate(self):
    src = 'src.cpp'
    create_test_file(src, self.with_report_result(open(os.path.join(path_from_root('tests/manual_wasm_instantiate.cpp'))).read()))
    self.compile_btest(['src.cpp', '-o', 'manual_wasm_instantiate.js', '-s', 'BINARYEN=1'])
    shutil.copyfile(path_from_root('tests', 'manual_wasm_instantiate.html'), 'manual_wasm_instantiate.html')
    self.run_browser('manual_wasm_instantiate.html', 'wasm instantiation succeeded', '/report_result?1')

  def test_binaryen_worker(self):
    self.do_test_worker(['-s', 'WASM=1'])

  def test_wasm_locate_file(self):
    # Test that it is possible to define "Module.locateFile(foo)" function to locate where worker.js will be loaded from.
    self.clear()
    os.makedirs('cdn')
    create_test_file('shell2.html', open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { locateFile: function(filename) { if (filename == "test.wasm") return "cdn/test.wasm"; else return filename; }, '))
    create_test_file('src.cpp', self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))
    self.compile_btest(['src.cpp', '--shell-file', 'shell2.html', '-s', 'WASM=1', '-o', 'test.html'])
    shutil.move('test.wasm', os.path.join('cdn', 'test.wasm'))
    self.run_browser('test.html', '', '/report_result?0')

  def test_utf8_textdecoder(self):
    self.btest('benchmark_utf8.cpp', expected='0', args=['--embed-file', path_from_root('tests/utf8_corpus.txt') + '@/utf8_corpus.txt', '-s', 'EXTRA_EXPORTED_RUNTIME_METHODS=["UTF8ToString"]'])

  def test_utf16_textdecoder(self):
    self.btest('benchmark_utf16.cpp', expected='0', args=['--embed-file', path_from_root('tests/utf16_corpus.txt') + '@/utf16_corpus.txt', '-s', 'EXTRA_EXPORTED_RUNTIME_METHODS=["UTF16ToString","stringToUTF16","lengthBytesUTF16"]'])

  def test_TextDecoder(self):
    self.btest('browser_test_hello_world.c', '0', args=['-s', 'TEXTDECODER=0'])
    just_fallback = os.path.getsize('test.js')
    self.btest('browser_test_hello_world.c', '0')
    td_with_fallback = os.path.getsize('test.js')
    self.btest('browser_test_hello_world.c', '0', args=['-s', 'TEXTDECODER=2'])
    td_without_fallback = os.path.getsize('test.js')
    self.assertLess(td_without_fallback, just_fallback)
    self.assertLess(just_fallback, td_with_fallback)

  # Tests that it is possible to initialize and render WebGL content in a pthread by using OffscreenCanvas.
  # -DTEST_CHAINED_WEBGL_CONTEXT_PASSING: Tests that it is possible to transfer WebGL canvas in a chain from main thread -> thread 1 -> thread 2 and then init and render WebGL content there.
  @no_chrome('see #7374')
  @requires_threads
  def test_webgl_offscreen_canvas_in_pthread(self):
    for args in [[], ['-DTEST_CHAINED_WEBGL_CONTEXT_PASSING']]:
      self.btest('gl_in_pthread.cpp', expected='1', args=args + ['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2', '-s', 'OFFSCREENCANVAS_SUPPORT=1', '-lGL', '-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'])

  # Tests that it is possible to render WebGL content on a <canvas> on the main thread, after it has once been used to render WebGL content in a pthread first
  # -DTEST_MAIN_THREAD_EXPLICIT_COMMIT: Test the same (WebGL on main thread after pthread), but by using explicit .commit() to swap on the main thread instead of implicit "swap when rAF ends" logic
  @no_chrome('see #7374')
  @requires_threads
  def test_webgl_offscreen_canvas_in_mainthread_after_pthread(self):
    for args in [[], ['-DTEST_MAIN_THREAD_EXPLICIT_COMMIT']]:
      self.btest('gl_in_mainthread_after_pthread.cpp', expected='0', args=args + ['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2', '-s', 'OFFSCREENCANVAS_SUPPORT=1', '-lGL', '-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'])

  @no_chrome('see #7374')
  @requires_threads
  def test_webgl_offscreen_canvas_only_in_pthread(self):
    self.btest('gl_only_in_pthread.cpp', expected='0', args=['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1', '-s', 'OFFSCREENCANVAS_SUPPORT=1', '-lGL', '-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'])

  # Tests that rendering from client side memory without default-enabling extensions works.
  @requires_graphics_hardware
  def test_webgl_from_client_side_memory_without_default_enabled_extensions(self):
    self.btest('webgl_draw_triangle.c', '0', args=['-lGL', '-s', 'OFFSCREEN_FRAMEBUFFER=1', '-DEXPLICIT_SWAP=1', '-DDRAW_FROM_CLIENT_MEMORY=1', '-s', 'FULL_ES2=1'])

  # Tests that -s OFFSCREEN_FRAMEBUFFER=1 rendering works.
  @requires_graphics_hardware
  def test_webgl_offscreen_framebuffer(self):
    self.btest('webgl_draw_triangle.c', '0', args=['-lGL', '-s', 'OFFSCREEN_FRAMEBUFFER=1', '-DEXPLICIT_SWAP=1'])

  # Tests that offscreen framebuffer state restoration works
  @requires_graphics_hardware
  def test_webgl_offscreen_framebuffer_state_restoration(self):
    for args in [
        # full state restoration path on WebGL 1.0
        ['-s', 'USE_WEBGL2=0', '-s', 'OFFSCREEN_FRAMEBUFFER_FORBID_VAO_PATH=1'],
        # VAO path on WebGL 1.0
        ['-s', 'USE_WEBGL2=0'],
        ['-s', 'USE_WEBGL2=1', '-DTEST_WEBGL2=0'],
        # VAO path on WebGL 2.0
        ['-s', 'USE_WEBGL2=1', '-DTEST_WEBGL2=1', '-DTEST_ANTIALIAS=1', '-DTEST_REQUIRE_VAO=1'],
        # full state restoration path on WebGL 2.0
        ['-s', 'USE_WEBGL2=1', '-DTEST_WEBGL2=1', '-DTEST_ANTIALIAS=1', '-s', 'OFFSCREEN_FRAMEBUFFER_FORBID_VAO_PATH=1'],
        # blitFramebuffer path on WebGL 2.0 (falls back to VAO on Firefox < 67)
        ['-s', 'USE_WEBGL2=1', '-DTEST_WEBGL2=1', '-DTEST_ANTIALIAS=0'],
      ]:
      cmd = args + ['-lGL', '-s', 'OFFSCREEN_FRAMEBUFFER=1', '-DEXPLICIT_SWAP=1']
      self.btest('webgl_offscreen_framebuffer_swap_with_bad_state.c', '0', args=cmd)

  # Tests that -s WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG=1 rendering works.
  @requires_graphics_hardware
  def test_webgl_workaround_webgl_uniform_upload_bug(self):
    self.btest('webgl_draw_triangle_with_uniform_color.c', '0', args=['-lGL', '-s', 'WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG=1'])

  # Tests that using an array of structs in GL uniforms works.
  @requires_graphics_hardware
  def test_webgl_array_of_structs_uniform(self):
    self.btest('webgl_array_of_structs_uniform.c', args=['-lGL', '-s', 'USE_WEBGL2=1'], reference='webgl_array_of_structs_uniform.png')

  # Tests that if a WebGL context is created in a pthread on a canvas that has not been transferred to that pthread, WebGL calls are then proxied to the main thread
  # -DTEST_OFFSCREEN_CANVAS=1: Tests that if a WebGL context is created on a pthread that has the canvas transferred to it via using Emscripten's EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES="#canvas", then OffscreenCanvas is used
  # -DTEST_OFFSCREEN_CANVAS=2: Tests that if a WebGL context is created on a pthread that has the canvas transferred to it via automatic transferring of Module.canvas when EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES is not defined, then OffscreenCanvas is also used
  @no_chrome('see #7374')
  def test_webgl_offscreen_canvas_in_proxied_pthread(self):
    for args in [[], ['-DTEST_OFFSCREEN_CANVAS=1'], ['-DTEST_OFFSCREEN_CANVAS=2']]:
      cmd = args + ['-s', 'USE_PTHREADS=1', '-s', 'OFFSCREENCANVAS_SUPPORT=1', '-lGL', '-s', 'GL_DEBUG=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1']
      print(str(cmd))
      self.btest('gl_in_proxy_pthread.cpp', expected='1', args=cmd)

  @requires_threads
  @requires_graphics_hardware
  @no_chrome('see #7374')
  def test_webgl_resize_offscreencanvas_from_main_thread(self):
    for args1 in [[], ['-s', 'PROXY_TO_PTHREAD=1']]:
      for args2 in [[], ['-DTEST_SYNC_BLOCKING_LOOP=1']]:
        for args3 in [[], ['-s', 'OFFSCREENCANVAS_SUPPORT=1']]:
          cmd = args1 + args2 + args3 + ['-s', 'USE_PTHREADS=1', '-lGL', '-s', 'GL_DEBUG=1', '-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1']
          print(str(cmd))
          self.btest('resize_offscreencanvas_from_main_thread.cpp', expected='1', args=cmd)

  # Tests the feature that shell html page can preallocate the typed array and place it to Module.buffer before loading the script page.
  # In this build mode, the -s TOTAL_MEMORY=xxx option will be ignored.
  # Preallocating the buffer in this was is asm.js only (wasm needs a Memory).
  @no_wasm_backend('asm.js feature')
  def test_preallocated_heap(self):
    self.btest('test_preallocated_heap.cpp', expected='1', args=['-s', 'WASM=0', '-s', 'TOTAL_MEMORY=16MB', '-s', 'ABORTING_MALLOC=0', '--shell-file', path_from_root('tests', 'test_preallocated_heap_shell.html')])

  # Tests emscripten_fetch() usage to XHR data directly to memory without persisting results to IndexedDB.
  def test_fetch_to_memory(self):
    # Test error reporting in the negative case when the file URL doesn't exist. (http 404)
    self.btest('fetch/to_memory.cpp',
               expected='1',
               args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-DFILE_DOES_NOT_EXIST'],
               also_asmjs=True)

    # Test the positive case when the file URL exists. (http 200)
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    for arg in [[], ['-s', 'FETCH_SUPPORT_INDEXEDDB=0']]:
      self.btest('fetch/to_memory.cpp',
                 expected='1',
                 args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1'] + arg,
                 also_asmjs=True)

  def test_fetch_to_indexdb(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/to_indexeddb.cpp',
               expected='1',
               args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1'],
               also_asmjs=True)

  # Tests emscripten_fetch() usage to persist an XHR into IndexedDB and subsequently load up from there.
  def test_fetch_cached_xhr(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/cached_xhr.cpp',
               expected='1',
               args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1'],
               also_asmjs=True)

  # Tests that response headers get set on emscripten_fetch_t values.
  @requires_threads
  def test_fetch_response_headers(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/response_headers.cpp', expected='1', args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'], also_asmjs=True)

  # Test emscripten_fetch() usage to stream a XHR in to memory without storing the full file in memory
  @no_chrome('depends on moz-chunked-arraybuffer')
  def test_fetch_stream_file(self):
    # Strategy: create a large 128MB file, and compile with a small 16MB Emscripten heap, so that the tested file
    # won't fully fit in the heap. This verifies that streaming works properly.
    s = '12345678'
    for i in range(14):
      s = s[::-1] + s # length of str will be 2^17=128KB
    with open('largefile.txt', 'w') as f:
      for i in range(1024):
        f.write(s)
    self.btest('fetch/stream_file.cpp',
               expected='1',
               args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-s', 'TOTAL_MEMORY=536870912'],
               also_asmjs=True)

  # Tests emscripten_fetch() usage in synchronous mode when used from the main
  # thread proxied to a Worker with -s PROXY_TO_PTHREAD=1 option.
  @requires_threads
  def test_fetch_sync_xhr(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/sync_xhr.cpp', expected='1', args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  # Tests emscripten_fetch() usage when user passes none of the main 3 flags (append/replace/no_download).
  # In that case, in append is implicitly understood.
  @requires_threads
  def test_fetch_implicit_append(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/example_synchronous_fetch.cpp', expected='200', args=['-s', 'FETCH=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  # Tests synchronous emscripten_fetch() usage from wasm pthread in fastcomp.
  @no_wasm_backend("fetch API uses an asm.js based web worker to run synchronous XHRs and IDB operations")
  def test_fetch_sync_xhr_in_wasm(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/example_synchronous_fetch.cpp', expected='200', args=['-s', 'FETCH=1', '-s', 'WASM=1', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  # Tests that the Fetch API works for synchronous XHRs when used with --proxy-to-worker.
  @requires_threads
  def test_fetch_sync_xhr_in_proxy_to_worker(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/sync_xhr.cpp',
               expected='1',
               args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '--proxy-to-worker'],
               also_asmjs=True)

  # Tests waiting on EMSCRIPTEN_FETCH_WAITABLE request from a worker thread
  @requires_threads
  def test_fetch_sync_fetch_in_main_thread(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/sync_fetch_in_main_thread.cpp', expected='0', args=['--std=c++11', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_fetch_idb_store(self):
    self.btest('fetch/idb_store.cpp', expected='0', args=['-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-s', 'WASM=0', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_fetch_idb_delete(self):
    shutil.copyfile(path_from_root('tests', 'gears.png'), 'gears.png')
    self.btest('fetch/idb_delete.cpp', expected='0', args=['-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1', '-s', 'FETCH=1', '-s', 'WASM=0', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_asmfs_hello_file(self):
    # Test basic file loading and the valid character set for files.
    os.mkdir('dirrey')
    shutil.copyfile(path_from_root('tests', 'asmfs', 'hello_file.txt'), os.path.join(self.get_dir(), 'dirrey', 'hello file !#$%&\'()+,-.;=@[]^_`{}~ %%.txt'))
    self.btest('asmfs/hello_file.cpp', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_asmfs_read_file_twice(self):
    shutil.copyfile(path_from_root('tests', 'asmfs', 'hello_file.txt'), 'hello_file.txt')
    self.btest('asmfs/read_file_twice.cpp', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_asmfs_fopen_write(self):
    self.btest('asmfs/fopen_write.cpp', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_asmfs_mkdir_create_unlink_rmdir(self):
    self.btest('cstdio/test_remove.cpp', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_asmfs_dirent_test_readdir(self):
    self.btest('dirent/test_readdir.c', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_asmfs_dirent_test_readdir_empty(self):
    self.btest('dirent/test_readdir_empty.c', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_asmfs_unistd_close(self):
    self.btest('unistd/close.c', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_asmfs_unistd_access(self):
    self.btest('unistd/access.c', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_asmfs_unistd_unlink(self):
    # TODO: Once symlinks are supported, remove -DNO_SYMLINK=1
    self.btest('unistd/unlink.c', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1', '-DNO_SYMLINK=1'])

  @requires_threads
  def test_asmfs_test_fcntl_open(self):
    self.btest('fcntl-open/src.c', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_asmfs_relative_paths(self):
    self.btest('asmfs/relative_paths.cpp', expected='0', args=['-s', 'ASMFS=1', '-s', 'WASM=0', '-s', 'USE_PTHREADS=1', '-s', 'FETCH_DEBUG=1'])

  @requires_threads
  def test_pthread_locale(self):
    for args in [
        [],
        ['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2'],
        ['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2'],
    ]:
      print("Testing with: ", args)
      self.btest('pthread/test_pthread_locale.c', expected='1', args=args)

  # Tests the Emscripten HTML5 API emscripten_set_canvas_element_size() and emscripten_get_canvas_element_size() functionality in singlethreaded programs.
  def test_emscripten_set_canvas_element_size(self):
    self.btest('emscripten_set_canvas_element_size.c', expected='1')

  # Test that emscripten_get_device_pixel_ratio() is callable from pthreads (and proxies to main thread to obtain the proper window.devicePixelRatio value).
  @requires_threads
  def test_emscripten_get_device_pixel_ratio(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest('emscripten_get_device_pixel_ratio.c', expected='1', args=args)

  # Tests that emscripten_run_script() variants of functions work in pthreads.
  @requires_threads
  def test_pthread_run_script(self):
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_run_script.cpp'), expected='1', args=['-O3', '--separate-asm'] + args, timeout=30)

  # Tests emscripten_set_canvas_element_size() and OffscreenCanvas functionality in different build configurations.
  @requires_threads
  @requires_graphics_hardware
  def test_emscripten_animate_canvas_element_size(self):
    for args in [
      ['-DTEST_EMSCRIPTEN_SET_MAIN_LOOP=1'],
      ['-DTEST_EMSCRIPTEN_SET_MAIN_LOOP=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'USE_PTHREADS=1', '-s',   'OFFSCREEN_FRAMEBUFFER=1'],
      ['-DTEST_EMSCRIPTEN_SET_MAIN_LOOP=1', '-s', 'PROXY_TO_PTHREAD=1', '-s', 'USE_PTHREADS=1', '-s',   'OFFSCREEN_FRAMEBUFFER=1', '-DTEST_EXPLICIT_CONTEXT_SWAP=1'],
      ['-DTEST_EXPLICIT_CONTEXT_SWAP=1',    '-s', 'PROXY_TO_PTHREAD=1', '-s', 'USE_PTHREADS=1', '-s',   'OFFSCREEN_FRAMEBUFFER=1'],
      ['-DTEST_EXPLICIT_CONTEXT_SWAP=1',    '-s', 'PROXY_TO_PTHREAD=1', '-s', 'USE_PTHREADS=1', '-s',   'OFFSCREEN_FRAMEBUFFER=1', '-DTEST_MANUALLY_SET_ELEMENT_CSS_SIZE=1'],
    ]:
      cmd = ['-lGL', '-O3', '-g2', '--shell-file', path_from_root('tests', 'canvas_animate_resize_shell.html'), '--separate-asm', '-s', 'GL_DEBUG=1', '--threadprofiler'] + args
      print(' '.join(cmd))
      self.btest('canvas_animate_resize.cpp', expected='1', args=cmd)

  # Tests the absolute minimum pthread-enabled application.
  @requires_threads
  def test_pthread_hello_thread(self):
    for opts in [[], ['-O3']]:
      for modularize in [[], ['-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=MyModule', '--shell-file', path_from_root('tests', 'shell_that_launches_modularize.html')]]:
        self.btest(path_from_root('tests', 'pthread', 'hello_thread.c'), expected='1', args=['-s', 'USE_PTHREADS=1'] + modularize + opts)

  # Tests memory growth in pthreads mode, but still on the main thread.
  @no_chrome('https://bugs.chromium.org/p/v8/issues/detail?id=9062')
  @requires_threads
  def test_pthread_growth_mainthread(self):
    def run(emcc_args=[]):
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_memory_growth_mainthread.c'), expected='1', args=['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2', '-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'TOTAL_MEMORY=32MB', '-s', 'WASM_MEM_MAX=256MB'] + emcc_args, also_asmjs=False)

    run()
    run(['-s', 'MODULARIZE_INSTANCE=1'])
    run(['-s', 'PROXY_TO_PTHREAD=1'])

  # Tests memory growth in a pthread.
  @no_chrome('https://bugs.chromium.org/p/v8/issues/detail?id=9065')
  @requires_threads
  def test_pthread_growth(self):
    def run(emcc_args=[]):
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_memory_growth.c'), expected='1', args=['-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=2', '-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'TOTAL_MEMORY=32MB', '-s', 'WASM_MEM_MAX=256MB', '-g'] + emcc_args, also_asmjs=False)

    run()
    run(['-s', 'ASSERTIONS=1'])
    run(['-s', 'PROXY_TO_PTHREAD=1'])

  # Tests that it is possible to load the main .js file of the application manually via a Blob URL, and still use pthreads.
  @requires_threads
  def test_load_js_from_blob_with_pthreads(self):
    # TODO: enable this with wasm, currently pthreads/atomics have limitations
    src = 'src.c'
    create_test_file(src, self.with_report_result(open(path_from_root('tests', 'pthread', 'hello_thread.c')).read()))
    self.compile_btest(['src.c', '-s', 'USE_PTHREADS=1', '-o', 'hello_thread_with_blob_url.js', '-s', 'WASM=0'])
    shutil.copyfile(path_from_root('tests', 'pthread', 'main_js_as_blob_loader.html'), 'hello_thread_with_blob_url.html')
    self.run_browser('hello_thread_with_blob_url.html', 'hello from thread!', '/report_result?1')

  # Tests that base64 utils work in browser with no native atob function
  def test_base64_atob_fallback(self):
    opts = ['-s', 'SINGLE_FILE=1', '-s', 'WASM=1']
    src = r'''
      #include <stdio.h>
      #include <emscripten.h>
      int main() {
        REPORT_RESULT(0);
        return 0;
      }
    '''
    create_test_file('test.c', self.with_report_result(src))
    # generate a dummy file
    create_test_file('dummy_file', 'dummy')
    # compile the code with the modularize feature and the preload-file option enabled
    self.compile_btest(['test.c', '-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME="Foo"', '--preload-file', 'dummy_file'] + opts)
    create_test_file('a.html', '''
      <script>
        atob = undefined;
        fetch = undefined;
      </script>
      <script src="a.out.js"></script>
      <script>
        var foo = Foo();
      </script>
    ''')
    self.run_browser('a.html', '...', '/report_result?0')

  # Tests that SINGLE_FILE works as intended in generated HTML (with and without Worker)
  def test_single_file_html(self):
    self.btest('emscripten_main_loop_setimmediate.cpp', '1', args=['-s', 'SINGLE_FILE=1', '-s', 'WASM=1'], also_proxied=True)
    self.assertExists('test.html')
    self.assertNotExists('test.js')
    self.assertNotExists('test.worker.js')

  # Tests that SINGLE_FILE works when built with ENVIRONMENT=web and Closure enabled (#7933)
  def test_single_file_in_web_environment_with_closure(self):
    self.btest('minimal_hello.c', '0', args=['-s', 'SINGLE_FILE=1', '-s', 'ENVIRONMENT=web', '-O2', '--closure', '1'])

  # Tests that SINGLE_FILE works as intended with locateFile
  def test_single_file_locate_file(self):
    create_test_file('src.cpp', self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))

    for wasm_enabled in [True, False]:
      args = ['src.cpp', '-o', 'test.js', '-s', 'SINGLE_FILE=1']

      if wasm_enabled:
        args += ['-s', 'WASM=1']

      self.compile_btest(args)

      create_test_file('test.html', '''
        <script>
          var Module = {
            locateFile: function (path) {
              if (path.indexOf('data:') === 0) {
                throw new Error('Unexpected data URI.');
              }

              return path;
            }
          };
        </script>
        <script src="test.js"></script>
      ''')

      self.run_browser('test.html', None, '/report_result?0')

  # Tests that SINGLE_FILE works as intended in a Worker in JS output
  def test_single_file_worker_js(self):
    create_test_file('src.cpp', self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))
    self.compile_btest(['src.cpp', '-o', 'test.js', '--proxy-to-worker', '-s', 'SINGLE_FILE=1', '-s', 'WASM=1'])
    create_test_file('test.html', '<script src="test.js"></script>')
    self.run_browser('test.html', None, '/report_result?0')
    self.assertExists('test.js')
    self.assertNotExists('test.worker.js')

  def test_access_file_after_heap_resize(self):
    create_test_file('test.txt', 'hello from file')
    create_test_file('page.c', self.with_report_result(open(path_from_root('tests', 'access_file_after_heap_resize.c'), 'r').read()))
    self.compile_btest(['page.c', '-s', 'WASM=1', '-s', 'ALLOW_MEMORY_GROWTH=1', '--preload-file', 'test.txt', '-o', 'page.html'])
    self.run_browser('page.html', 'hello from file', '/report_result?15')

    # with separate file packager invocation, letting us affect heap copying
    # or lack thereof
    for file_packager_args in [[], ['--no-heap-copy']]:
      print(file_packager_args)
      run_process([PYTHON, FILE_PACKAGER, 'data.js', '--preload', 'test.txt', '--js-output=' + 'data.js'] + file_packager_args)
      self.compile_btest(['page.c', '-s', 'WASM=1', '-s', 'ALLOW_MEMORY_GROWTH=1', '--pre-js', 'data.js', '-o', 'page.html', '-s', 'FORCE_FILESYSTEM=1'])
      self.run_browser('page.html', 'hello from file', '/report_result?15')

  def test_unicode_html_shell(self):
    create_test_file('main.cpp', self.with_report_result(r'''
      int main() {
        REPORT_RESULT(0);
        return 0;
      }
    '''))
    create_test_file('shell.html', open(path_from_root('src', 'shell.html')).read().replace('Emscripten-Generated Code', 'Emscripten-Generated Emoji 😅'))
    self.compile_btest(['main.cpp', '--shell-file', 'shell.html', '-o', 'test.html'])
    self.run_browser('test.html', None, '/report_result?0')

  # Tests the functionality of the emscripten_thread_sleep() function.
  @requires_threads
  def test_emscripten_thread_sleep(self):
    self.btest(path_from_root('tests', 'pthread', 'emscripten_thread_sleep.c'), expected='1', args=['-s', 'USE_PTHREADS=1', '-s', 'EXTRA_EXPORTED_RUNTIME_METHODS=["print"]'])

  # Tests that Emscripten-compiled applications can be run from a relative path in browser that is different than the address of the current page
  def test_browser_run_from_different_directory(self):
    src = open(path_from_root('tests', 'browser_test_hello_world.c')).read()
    create_test_file('test.c', self.with_report_result(src))
    self.compile_btest(['test.c', '-o', 'test.html', '-O3'])

    if not os.path.exists('subdir'):
      os.mkdir('subdir')
    shutil.move('test.js', os.path.join('subdir', 'test.js'))
    shutil.move('test.wasm', os.path.join('subdir', 'test.wasm'))
    src = open('test.html').read()
    # Make sure JS is loaded from subdirectory
    create_test_file('test-subdir.html', src.replace('test.js', 'subdir/test.js'))
    self.run_browser('test-subdir.html', None, '/report_result?0')

  # Similar to `test_browser_run_from_different_directory`, but asynchronous because of `-s MODULARIZE=1`
  def test_browser_run_from_different_directory_async(self):
    src = open(path_from_root('tests', 'browser_test_hello_world.c')).read()
    create_test_file('test.c', self.with_report_result(src))
    for args, creations in [
      (['-s', 'MODULARIZE=1'], [
        'Module();',    # documented way for using modularize
        'new Module();' # not documented as working, but we support it
       ]),
      (['-s', 'MODULARIZE_INSTANCE=1'], ['']) # instance: no need to create anything
    ]:
      print(args)
      # compile the code with the modularize feature and the preload-file option enabled
      self.compile_btest(['test.c', '-o', 'test.js', '-O3'] + args)
      if not os.path.exists('subdir'):
        os.mkdir('subdir')
      shutil.move('test.js', os.path.join('subdir', 'test.js'))
      shutil.move('test.wasm', os.path.join('subdir', 'test.wasm'))
      for creation in creations:
        print(creation)
        # Make sure JS is loaded from subdirectory
        create_test_file('test-subdir.html', '''
          <script src="subdir/test.js"></script>
          <script>
            %s
          </script>
        ''' % creation)
        self.run_browser('test-subdir.html', None, '/report_result?0')

  # Similar to `test_browser_run_from_different_directory`, but
  # also also we eval the initial code, so currentScript is not present. That prevents us
  # from finding the file in a subdir, but here we at least check we do not regress compared to the
  # normal case of finding in the current dir.
  def test_browser_modularize_no_current_script(self):
    src = open(path_from_root('tests', 'browser_test_hello_world.c')).read()
    create_test_file('test.c', self.with_report_result(src))
    # test both modularize (and creating an instance) and modularize-instance
    # (which creates by itself)
    for path, args, creation in [
      ([], ['-s', 'MODULARIZE=1'], 'Module();'),
      ([], ['-s', 'MODULARIZE_INSTANCE=1'], ''),
      (['subdir'], ['-s', 'MODULARIZE=1'], 'Module();'),
      (['subdir'], ['-s', 'MODULARIZE_INSTANCE=1'], ''),
    ]:
      print(path, args, creation)
      filesystem_path = os.path.join('.', *path)
      if not os.path.exists(filesystem_path):
        os.makedirs(filesystem_path)
      # compile the code with the modularize feature and the preload-file option enabled
      self.compile_btest(['test.c', '-o', 'test.js'] + args)
      shutil.move('test.js', os.path.join(filesystem_path, 'test.js'))
      shutil.move('test.wasm', os.path.join(filesystem_path, 'test.wasm'))
      open(os.path.join(filesystem_path, 'test.html'), 'w').write('''
        <script>
          setTimeout(function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', 'test.js', false);
            xhr.send(null);
            eval(xhr.responseText);
            %s
          }, 1);
        </script>
      ''' % creation)
      self.run_browser('/'.join(path + ['test.html']), None, '/report_result?0')

  def test_modularize_Module_input(self):
    self.btest(path_from_root('tests', 'browser', 'modularize_Module_input.cpp'), '0', args=['--shell-file', path_from_root('tests', 'browser', 'modularize_Module_input.html'), '-s', 'MODULARIZE_INSTANCE=1'])

  def test_emscripten_request_animation_frame(self):
    self.btest(path_from_root('tests', 'emscripten_request_animation_frame.c'), '0')

  def test_emscripten_request_animation_frame_loop(self):
    self.btest(path_from_root('tests', 'emscripten_request_animation_frame_loop.c'), '0')

  @requires_threads
  def test_emscripten_set_timeout(self):
    self.btest(path_from_root('tests', 'emscripten_set_timeout.c'), '0', args=['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  @requires_threads
  def test_emscripten_set_timeout_loop(self):
    self.btest(path_from_root('tests', 'emscripten_set_timeout_loop.c'), '0', args=['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  def test_emscripten_set_immediate(self):
    self.btest(path_from_root('tests', 'emscripten_set_immediate.c'), '0')

  def test_emscripten_set_immediate_loop(self):
    self.btest(path_from_root('tests', 'emscripten_set_immediate_loop.c'), '0')

  @requires_threads
  def test_emscripten_set_interval(self):
    self.btest(path_from_root('tests', 'emscripten_set_interval.c'), '0', args=['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  # Test emscripten_performance_now() and emscripten_date_now()
  @requires_threads
  def test_emscripten_performance_now(self):
    self.btest(path_from_root('tests', 'emscripten_performance_now.c'), '0', args=['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  # Test emscripten_console_log(), emscripten_console_warn() and emscripten_console_error()
  def test_emscripten_console_log(self):
    self.btest(path_from_root('tests', 'emscripten_console_log.c'), '0', args=['--pre-js', path_from_root('tests', 'emscripten_console_log_pre.js')])

  def test_emscripten_throw_number(self):
    self.btest(path_from_root('tests', 'emscripten_throw_number.c'), '0', args=['--pre-js', path_from_root('tests', 'emscripten_throw_number_pre.js')])

  def test_emscripten_throw_string(self):
    self.btest(path_from_root('tests', 'emscripten_throw_string.c'), '0', args=['--pre-js', path_from_root('tests', 'emscripten_throw_string_pre.js')])

  # Tests that Closure run in combination with -s ENVIRONMENT=web mode works with a minimal console.log() application
  def test_closure_in_web_only_target_environment_console_log(self):
    self.btest('minimal_hello.c', '0', args=['-s', 'ENVIRONMENT=web', '-O3', '--closure', '1'])

  # Tests that Closure run in combination with -s ENVIRONMENT=web mode works with a small WebGL application
  @requires_graphics_hardware
  def test_closure_in_web_only_target_environment_webgl(self):
    self.btest('webgl_draw_triangle.c', '0', args=['-lGL', '-s', 'ENVIRONMENT=web', '-O3', '--closure', '1'])

  # Tests that it is possible to load two asm.js compiled programs to one page when both --separate-asm and MODULARIZE=1 is used, by assigning
  # the pages different asm module names to ensure they do not conflict when being XHRed in.
  @no_wasm_backend('this tests asm.js support')
  def test_two_separate_asm_files_on_same_page(self):
    html_file = open('main.html', 'w')
    html_file.write(open(path_from_root('tests', 'two_separate_asm_files.html')).read().replace('localhost:8888', 'localhost:%s' % self.port))
    html_file.close()

    cmd = [PYTHON, EMCC, path_from_root('tests', 'modularize_separate_asm.c'), '-o', 'page1.js', '-s', 'WASM=0', '--separate-asm', '-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=Module1', '-s', 'SEPARATE_ASM_MODULE_NAME=ModuleForPage1["asm"]']
    print(cmd)
    subprocess.check_call(cmd)

    cmd = [PYTHON, EMCC, path_from_root('tests', 'modularize_separate_asm.c'), '-o', 'page2.js', '-s', 'WASM=0', '--separate-asm', '-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=Module2', '-s', 'SEPARATE_ASM_MODULE_NAME=ModuleForPage2["asm"]']
    print(cmd)
    subprocess.check_call(cmd)

    self.run_browser('main.html', None, '/report_result?1')

  # Tests that it is possible to encapsulate asm.js compiled programs by using --separate-asm + MODULARIZE=1. See
  # encapsulated_asmjs_page_load.html for the example.
  @no_wasm_backend('this tests asm.js support')
  def test_encapsulated_asmjs_page_load(self):
    html_file = open('main.html', 'w')
    html_file.write(open(path_from_root('tests', 'encapsulated_asmjs_page_load.html')).read().replace('localhost:8888', 'localhost:%s' % self.port))
    html_file.close()

    cmd = [PYTHON, EMCC, path_from_root('tests', 'modularize_separate_asm.c'), '-o', 'a.js', '-s', 'WASM=0', '--separate-asm', '-s', 'MODULARIZE=1', '-s', 'EXPORT_NAME=EmscriptenCode', '-s', 'SEPARATE_ASM_MODULE_NAME="var EmscriptenCode"']
    print(cmd)
    subprocess.check_call(cmd)

    self.run_browser('main.html', None, '/report_result?1')

  @no_wasm_backend('MINIMAL_RUNTIME not yet available in Wasm backend')
  def test_no_declare_asm_module_exports_asmjs(self):
    for minimal_runtime in [[], ['-s', 'MINIMAL_RUNTIME=1']]:
      self.btest(path_from_root('tests', 'declare_asm_module_exports.cpp'), '1', args=['-s', 'DECLARE_ASM_MODULE_EXPORTS=0', '-s', 'ENVIRONMENT=web', '-O3', '--closure', '1', '-s', 'WASM=0'] + minimal_runtime)

  @no_wasm_backend('MINIMAL_RUNTIME not yet available in Wasm backend')
  def test_no_declare_asm_module_exports_wasm_minimal_runtime(self):
    self.btest(path_from_root('tests', 'declare_asm_module_exports.cpp'), '1', args=['-s', 'DECLARE_ASM_MODULE_EXPORTS=0', '-s', 'ENVIRONMENT=web', '-O3', '--closure', '1', '-s', 'MINIMAL_RUNTIME=1'])

  # Tests that the different code paths in src/shell_minimal_runtime.html all work ok.
  @no_wasm_backend('MINIMAL_RUNTIME not yet available in Wasm backend')
  def test_minimal_runtime_loader_shell(self):
    args = ['-s', 'MINIMAL_RUNTIME=2']
    for wasm in [[], ['-s', 'WASM=0', '--memory-init-file', '0'], ['-s', 'WASM=0', '--memory-init-file', '1']]:
      for modularize in [[], ['-s', 'MODULARIZE=1']]:
        print(str(args + wasm + modularize))
        self.btest('minimal_hello.c', '0', args=args + wasm + modularize)
