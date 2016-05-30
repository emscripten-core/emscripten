import BaseHTTPServer, multiprocessing, os, shutil, subprocess, unittest, zlib, webbrowser, time, shlex
from runner import BrowserCore, path_from_root
from tools.shared import *

# User can specify an environment variable EMSCRIPTEN_BROWSER to force the browser test suite to
# run using another browser command line than the default system browser.
emscripten_browser = os.environ.get('EMSCRIPTEN_BROWSER')
if emscripten_browser:
  cmd = shlex.split(emscripten_browser)
  def run_in_other_browser(url):
    Popen(cmd + [url])
  if EM_BUILD_VERBOSE_LEVEL >= 3:
    print >> sys.stderr, "using Emscripten browser: " + str(cmd)
  webbrowser.open_new = run_in_other_browser

def test_chunked_synchronous_xhr_server(support_byte_ranges, chunkSize, data, checksum):
  class ChunkedServerHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def sendheaders(s, extra=[], length=len(data)):
      s.send_response(200)
      s.send_header("Content-Length", str(length))
      s.send_header("Access-Control-Allow-Origin", "http://localhost:8888")
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
        (start, end) = s.headers.get("range").split("=")[1].split("-")
        start = int(start)
        end = int(end)
        end = min(len(data)-1, end)
        length = end-start+1
        s.sendheaders([],length)
        s.wfile.write(data[start:end+1])
      s.wfile.close()

  expectedConns = 11
  httpd = BaseHTTPServer.HTTPServer(('localhost', 11111), ChunkedServerHandler)
  for i in range(expectedConns+1):
    httpd.handle_request()

class browser(BrowserCore):
  @classmethod
  def setUpClass(self):
    super(browser, self).setUpClass()
    self.browser_timeout = 20
    print
    print 'Running the browser tests. Make sure the browser allows popups from localhost.'
    print

  def test_sdl1(self):
    self.btest('hello_world_sdl.cpp', reference='htmltest.png')
    self.btest('hello_world_sdl.cpp', reference='htmltest.png', args=['-s', 'USE_SDL=1']) # is the default anyhow

  def test_html_source_map(self):
    cpp_file = os.path.join(self.get_dir(), 'src.cpp')
    html_file = os.path.join(self.get_dir(), 'src.html')
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
    Popen([PYTHON, EMCC, 'src.cpp', '-o', 'src.html', '-g4'],
        cwd=self.get_dir()).communicate()
    assert os.path.exists(html_file)
    assert os.path.exists(html_file + '.map')
    webbrowser.open_new('file://' + html_file)
    time.sleep(1)
    print '''
If manually bisecting:
  Check that you see src.cpp among the page sources.
  Even better, add a breakpoint, e.g. on the printf, then reload, then step through and see the print (best to run with EM_SAVE_DIR=1 for the reload).
'''

  def test_emscripten_log(self):
    src = os.path.join(self.get_dir(), 'src.cpp')
    open(src, 'w').write(self.with_report_result(open(path_from_root('tests', 'emscripten_log', 'emscripten_log.cpp')).read()))

    Popen([PYTHON, EMCC, src, '--pre-js', path_from_root('src', 'emscripten-source-map.min.js'), '-g', '-o', 'page.html', '-s', 'DEMANGLE_SUPPORT=1']).communicate()
    self.run_browser('page.html', None, '/report_result?1')
  
  def build_native_lzma(self):
    lzma_native = path_from_root('third_party', 'lzma.js', 'lzma-native')
    if os.path.isfile(lzma_native) and os.access(lzma_native, os.X_OK): return

    cwd = os.getcwd()
    try:
      os.chdir(path_from_root('third_party', 'lzma.js'))
      if WINDOWS and Building.which('mingw32-make'): # On Windows prefer using MinGW make if it exists, otherwise fall back to hoping we have cygwin make.
        Popen(['doit.bat']).communicate()
      else:
        Popen(['sh', './doit.sh']).communicate()
    finally:
      os.chdir(cwd)

  def test_preload_file(self):
    absolute_src_path = os.path.join(self.get_dir(), 'somefile.txt').replace('\\', '/')
    open(absolute_src_path, 'w').write('''load me right before running the code please''')

    absolute_src_path2 = os.path.join(self.get_dir(), '.somefile.txt').replace('\\', '/')
    open(absolute_src_path2, 'w').write('''load me right before running the code please''')
    
    def make_main(path):
      print 'make main at', path
      path = path.replace('\\', '\\\\').replace('"', '\\"') # Escape tricky path name for use inside a C string.
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
      (absolute_src_path + "@/directory/file.txt", "directory/file.txt")]

    for test in test_cases:
      (srcpath, dstpath) = test
      print 'Testing', srcpath, dstpath
      make_main(dstpath)
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', srcpath, '-o', 'page.html']).communicate()
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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', tricky_filename.replace('@', '@@'), '--no-heap-copy', '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    # By absolute path

    make_main('somefile.txt') # absolute becomes relative
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', absolute_src_path, '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    # Test subdirectory handling with asset packaging.
    try_delete(self.in_dir('assets'))
    os.makedirs(os.path.join(self.get_dir(), 'assets/sub/asset1/').replace('\\', '/'))
    os.makedirs(os.path.join(self.get_dir(), 'assets/sub/asset1/.git').replace('\\', '/')) # Test adding directory that shouldn't exist.
    os.makedirs(os.path.join(self.get_dir(), 'assets/sub/asset2/').replace('\\', '/'))
    open(os.path.join(self.get_dir(), 'assets/sub/asset1/file1.txt'), 'w').write('''load me right before running the code please''')
    open(os.path.join(self.get_dir(), 'assets/sub/asset1/.git/shouldnt_be_embedded.txt'), 'w').write('''this file should not get embedded''')
    open(os.path.join(self.get_dir(), 'assets/sub/asset2/file2.txt'), 'w').write('''load me right before running the code please''')
    absolute_assets_src_path = os.path.join(self.get_dir(), 'assets').replace('\\', '/')
    def make_main_two_files(path1, path2, nonexistingpath):
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
          
          f = fopen("%s", "r");
          if (f == NULL)
            result = 0;
          fclose(f);
          
          f = fopen("%s", "r");
          if (f != NULL)
            result = 0;

          REPORT_RESULT();
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
      print srcpath
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', srcpath, '--exclude-file', '*/.*', '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')
      
    # Should still work with -o subdir/..

    make_main('somefile.txt') # absolute becomes relative
    try:
      os.mkdir(os.path.join(self.get_dir(), 'dirrey'))
    except:
      pass
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', absolute_src_path, '-o', 'dirrey/page.html']).communicate()
    self.run_browser('dirrey/page.html', 'You should see |load me right before|.', '/report_result?1')

    # With FS.preloadFile

    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      Module.preRun = function() {
        FS.createPreloadedFile('/', 'someotherfile.txt', 'somefile.txt', true, false); // we need --use-preload-plugins for this.
      };
    ''')
    make_main('someotherfile.txt')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '-o', 'page.html', '--use-preload-plugins']).communicate()
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

  def test_preload_caching(self):
    open(os.path.join(self.get_dir(), 'somefile.txt'), 'w').write('''load me right before running the code please''')
    def make_main(path):
      print path
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
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

          REPORT_RESULT();
          return 0;
        }
      ''' % path))

    open(os.path.join(self.get_dir(), 'test.js'), 'w').write('''
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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--use-preload-cache', '--js-library', os.path.join(self.get_dir(), 'test.js'), '--preload-file', 'somefile.txt', '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?2')

  def test_preload_caching_indexeddb_name(self):
    open(os.path.join(self.get_dir(), 'somefile.txt'), 'w').write('''load me right before running the code please''')
    def make_main(path):
      print path
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
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

          REPORT_RESULT();
          return 0;
        }
      ''' % path))

    open(os.path.join(self.get_dir(), 'test.js'), 'w').write('''
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
    Popen([PYTHON, FILE_PACKAGER, os.path.join(self.get_dir(), 'somefile.data'), '--use-preload-cache', '--indexedDB-name=testdb', '--preload', os.path.join(self.get_dir(), 'somefile.txt'), '--js-output=' + os.path.join(self.get_dir(), 'somefile.js')]).communicate()
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--js-library', os.path.join(self.get_dir(), 'test.js'), '--pre-js', 'somefile.js', '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?2')

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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', 'subdirr/data1.txt', '--preload-file', 'subdirr/moar/data2.txt', '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should see two cool numbers', '/report_result?1')
    os.remove('page.html')

    # by directory, and remove files to make sure
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', 'subdirr', '-o', 'page.html']).communicate()
    shutil.rmtree(os.path.join(self.get_dir(), 'subdirr'))
    self.run_browser('page.html', 'You should see two cool numbers', '/report_result?1')

  def test_custom_file_package_url(self):
    # a few files inside a directory
    self.clear()
    os.makedirs(os.path.join(self.get_dir(), 'subdirr'));
    os.makedirs(os.path.join(self.get_dir(), 'cdn'));
    open(os.path.join(self.get_dir(), 'subdirr', 'data1.txt'), 'w').write('''1214141516171819''')
    # change the file package base dir to look in a "cdn". note that normally you would add this in your own custom html file etc., and not by
    # modifying the existing shell in this manner
    open(self.in_dir('shell.html'), 'w').write(open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { filePackagePrefixURL: "cdn/", '))
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

        REPORT_RESULT();
        return 0;
      }
    '''))

    def test():
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--shell-file', 'shell.html', '--preload-file', 'subdirr/data1.txt', '-o', 'test.html']).communicate()
      shutil.move('test.data', os.path.join('cdn', 'test.data'))
      self.run_browser('test.html', '', '/report_result?1')

    test()

  def test_missing_data_throws_error(self):
    def setup(assetLocalization):
      self.clear()
      open(self.in_dir("data.txt"), "w").write('''data''');
      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
        #include <stdio.h>
        #include <string.h>
        #include <emscripten.h>
        int main() {
          // This code should never be executed in terms of missing required dependency file.
          int result = 0;
          REPORT_RESULT();
          return 0;
        }
      '''))
      open(os.path.join(self.get_dir(), 'on_window_error_shell.html'), 'w').write(r'''
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
              filePackagePrefixURL: "''' + assetLocalization + r'''",
              print: (function() {
                var element = document.getElementById('output');
                return function(text) { element.innerHTML += text.replace('\n', '<br>', 'g') + '<br>';};
              })(),
              canvas: document.getElementById('canvas')
            };
          </script>
          {{{ SCRIPT }}}
        </body>
      </html>'''
      )

    def test():
      # test test missing file should run xhr.onload with status different than 200, 304 or 206
      setup("");
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html']).communicate()
      shutil.move('test.data','missing.data');
      self.run_browser('test.html', '', '/report_result?1')
      
      # test unknown protocol should go through xhr.onerror
      setup("unknown_protocol://");
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html']).communicate()
      self.run_browser('test.html', '', '/report_result?1')
      
      # test wrong protocol and port
      setup("https://localhost:8800/");
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--shell-file', 'on_window_error_shell.html', '--preload-file', 'data.txt', '-o', 'test.html']).communicate()
      self.run_browser('test.html', '', '/report_result?1')

    test()


    # TODO: CORS, test using a full url for filePackagePrefixURL
    #open(self.in_dir('shell.html'), 'w').write(open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { filePackagePrefixURL: "http:/localhost:8888/cdn/", '))
    #test()

  def test_sdl_swsurface(self):
    self.btest('sdl_swsurface.c', expected='1')

  def test_sdl_surface_lock_opts(self):
    # Test Emscripten-specific extensions to optimize SDL_LockSurface and SDL_UnlockSurface.
    self.btest('hello_world_sdl.cpp', reference='htmltest.png', message='You should see "hello, world!" and a colored cube.', args=['-DTEST_SDL_LOCK_OPTS'])

  def test_sdl_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file, and memory-init
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpg'))
    open(os.path.join(self.get_dir(), 'sdl_image.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))

    for mem in [0, 1]:
      for dest, dirname, basename in [('screenshot.jpg',                        '/',       'screenshot.jpg'),
                                      ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
        Popen([
          PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_image.c'), '-o', 'page.html', '-O2', '--memory-init-file', str(mem),
          '--preload-file', dest, '-DSCREENSHOT_DIRNAME="' + dirname + '"', '-DSCREENSHOT_BASENAME="' + basename + '"', '--use-preload-plugins'
        ]).communicate()
        self.run_browser('page.html', '', '/report_result?600')

  def test_sdl_image_jpeg(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpeg'))
    open(os.path.join(self.get_dir(), 'sdl_image_jpeg.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))
    Popen([
      PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_image_jpeg.c'), '-o', 'page.html',
      '--preload-file', 'screenshot.jpeg', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"', '--use-preload-plugins'
    ]).communicate()
    self.run_browser('page.html', '', '/report_result?600')

  def test_sdl_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_image_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not'], also_proxied=True)

  def test_sdl_image_prepare_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_image_prepare_data.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not'])

  def test_sdl_image_must_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpg'))
    self.btest('sdl_image_must_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.jpg'])

  def test_sdl_stb_image(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_stb_image.c', reference='screenshot.jpg', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not'])

  def test_sdl_stb_image_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_stb_image_data.c', reference='screenshot.jpg', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not'])

  def test_sdl_canvas(self):
    self.clear()
    self.btest('sdl_canvas.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1'])
    # some extra coverage
    self.clear()
    self.btest('sdl_canvas.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-O0', '-s', 'SAFE_HEAP=1'])
    self.clear()
    self.btest('sdl_canvas.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1', '-O2', '-s', 'SAFE_HEAP=1'])

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
    open('test.html', 'w').write(html)

  def test_sdl_canvas_proxy(self):
    open('data.txt', 'w').write('datum')
    self.btest('sdl_canvas_proxy.c', reference='sdl_canvas_proxy.png', args=['--proxy-to-worker', '--preload-file', 'data.txt'], manual_reference=True, post_build=self.post_manual_reftest)

  def test_glgears_proxy(self):
    self.btest('hello_world_gles_proxy.c', reference='gears.png', args=['--proxy-to-worker', '-s', 'GL_TESTING=1', '-DSTATIC_GEARS=1'], manual_reference=True, post_build=self.post_manual_reftest)

    # test noProxy option applied at runtime

    # run normally (duplicates above test, but verifies we can run outside of the btest harness
    self.run_browser('test.html', None, ['/report_result?0'])

    # run with noProxy
    self.run_browser('test.html?noProxy', None, ['/report_result?0'])

    original = open('test.js').read()

    def copy(to, js_mod, html_mod = lambda x: x):
      open(to + '.html', 'w').write(html_mod(open('test.html').read().replace('test.js', to + '.js')))
      open(to + '.js', 'w').write(js_mod(open('test.js').read()))

    # run with noProxy, but make main thread fail
    copy('two', lambda original: original.replace('function _main($argc,$argv) {', 'function _main($argc,$argv) { if (ENVIRONMENT_IS_WEB) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:8888/report_result?999");xhr.send(); }'),
                lambda original: original.replace('function doReftest() {', 'function doReftest() { return; ')) # don't reftest on main thread, it would race
    self.run_browser('two.html?noProxy', None, ['/report_result?999'])
    copy('two', lambda original: original.replace('function _main($argc,$argv) {', 'function _main($argc,$argv) { if (ENVIRONMENT_IS_WEB) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:8888/report_result?999");xhr.send(); }'))
    self.run_browser('two.html', None, ['/report_result?0']) # this is still cool

    # run without noProxy, so proxy, but make worker fail
    copy('three', lambda original: original.replace('function _main($argc,$argv) {', 'function _main($argc,$argv) { if (ENVIRONMENT_IS_WORKER) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:8888/report_result?999");xhr.send(); }'),
                lambda original: original.replace('function doReftest() {', 'function doReftest() { return; ')) # don't reftest on main thread, it would race
    self.run_browser('three.html', None, ['/report_result?999'])
    copy('three', lambda original: original.replace('function _main($argc,$argv) {', 'function _main($argc,$argv) { if (ENVIRONMENT_IS_WORKER) { var xhr = new XMLHttpRequest(); xhr.open("GET", "http://localhost:8888/report_result?999");xhr.send(); }'))
    self.run_browser('three.html?noProxy', None, ['/report_result?0']) # this is still cool

  def test_glgears_proxy_jstarget(self):
    # test .js target with --proxy-worker; emits 2 js files, client and worker
    Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_gles_proxy.c'), '-o', 'test.js', '--proxy-to-worker', '-s', 'GL_TESTING=1']).communicate()
    open('test.html', 'w').write(open(path_from_root('src', 'shell_minimal.html')).read().replace('{{{ SCRIPT }}}', '<script src="test.js"></script>'))
    self.post_manual_reftest('gears.png')
    self.run_browser('test.html', None, '/report_result?0')

  def test_sdl_canvas_alpha(self):
    open(os.path.join(self.get_dir(), 'flag_0.js'), 'w').write('''
      Module['arguments'] = ['-0'];
    ''')

    self.btest('sdl_canvas_alpha.c', reference='sdl_canvas_alpha.png', reference_slack=12)
    self.btest('sdl_canvas_alpha.c', args=['--pre-js', 'flag_0.js'], reference='sdl_canvas_alpha_flag_0.png', reference_slack=12)


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
          print delay, defines, emterps
          open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
            function keydown(c) {
             %s
              //Module.print('push keydown');
              var event = document.createEvent("KeyboardEvent");
              event.initKeyEvent("keydown", true, true, window,
                                 0, 0, 0, 0,
                                 c, c);
              document.dispatchEvent(event);
             %s
            }

            function keyup(c) {
             %s
              //Module.print('push keyup');
              var event = document.createEvent("KeyboardEvent");
              event.initKeyEvent("keyup", true, true, window,
                                 0, 0, 0, 0,
                                 c, c);
              document.dispatchEvent(event);
             %s
            }
          ''' % ('setTimeout(function() {' if delay else '', '}, 1);' if delay else '', 'setTimeout(function() {' if delay else '', '}, 1);' if delay else ''))
          open(os.path.join(self.get_dir(), 'sdl_key.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_key.c')).read()))

          Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_key.c'), '-o', 'page.html'] + defines + emterps + ['--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main']''', '-s', 'NO_EXIT_RUNTIME=1']).communicate()
          self.run_browser('page.html', '', '/report_result?223092870')

  def test_sdl_key_proxy(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
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
  var event = document.createEvent("KeyboardEvent");
  event.initKeyEvent("keydown", true, true, window,
                     0, 0, 0, 0,
                     c, c);
  document.dispatchEvent(event);
}

function keyup(c) {
  var event = document.createEvent("KeyboardEvent");
  event.initKeyEvent("keyup", true, true, window,
                     0, 0, 0, 0,
                     c, c);
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
      open('test.html', 'w').write(html)

    self.btest('sdl_key_proxy.c', '223092870', args=['--proxy-to-worker', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''', '-s', 'NO_EXIT_RUNTIME=1'], manual_reference=True, post_build=post)

  def test_keydown_preventdefault_proxy(self):
    def post():
      html = open('test.html').read()
      html = html.replace('</body>', '''
<script>
function keydown(c) {
  var event = document.createEvent("KeyboardEvent");
  event.initKeyEvent("keydown", true, true, window,
                     0, 0, 0, 0,
                     c, c);
  return document.dispatchEvent(event);
}

function keypress(c) {
  var event = document.createEvent("KeyboardEvent");
  event.initKeyEvent("keypress", true, true, window,
                     0, 0, 0, 0,
                     c, c);
  return document.dispatchEvent(event);
}

function keyup(c) {
  var event = document.createEvent("KeyboardEvent");
  event.initKeyEvent("keyup", true, true, window,
                     0, 0, 0, 0,
                     c, c);
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

      open('test.html', 'w').write(html)

    self.btest('keydown_preventdefault_proxy.cpp', '300', args=['--proxy-to-worker', '-s', '''EXPORTED_FUNCTIONS=['_main']''', '-s', 'NO_EXIT_RUNTIME=1'], manual_reference=True, post_build=post)

  def test_sdl_text(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      Module.postRun = function() {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }

      function simulateKeyEvent(charCode) {
        var event = document.createEvent("KeyboardEvent");
        event.initKeyEvent("keypress", true, true, window,
                           0, 0, 0, 0, 0, charCode);
        document.body.dispatchEvent(event);
      }
    ''')
    open(os.path.join(self.get_dir(), 'sdl_text.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_text.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_text.c'), '-o', 'page.html', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

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

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_mouse.c'), '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js']).communicate()
    self.run_browser('page.html', '', '/report_result?740')

  def test_sdl_mouse_offsets(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'page.html'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'sdl_mouse.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_mouse.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_mouse.c'), '-O2', '--minify', '0', '-o', 'sdl_mouse.js', '--pre-js', 'pre.js']).communicate()
    self.run_browser('page.html', '', '/report_result?600')

  def test_glut_touchevents(self):
    self.btest('glut_touchevents.c', '1')

  def test_glut_wheelevents(self):
    self.btest('glut_wheelevents.c', '1')

  def test_sdl_joystick_1(self):
    # Generates events corresponding to the Working Draft of the HTML5 Gamepad API.
    # http://www.w3.org/TR/2012/WD-gamepad-20120529/#gamepad-interface
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'sdl_joystick.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_joystick.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_joystick.c'), '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js']).communicate()
    self.run_browser('page.html', '', '/report_result?2')

  def test_sdl_joystick_2(self):
    # Generates events corresponding to the Editor's Draft of the HTML5 Gamepad API.
    # https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-Gamepad
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'sdl_joystick.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_joystick.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_joystick.c'), '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js']).communicate()
    self.run_browser('page.html', '', '/report_result?2')

  def test_webgl_context_attributes(self):
    # Javascript code to check the attributes support we want to test in the WebGL implementation 
    # (request the attribute, create a context and check its value afterwards in the context attributes).
    # Tests will succeed when an attribute is not supported.
    open(os.path.join(self.get_dir(), 'check_webgl_attributes_support.js'), 'w').write('''
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
    self.btest('test_webgl_context_attributes_glut.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])
    self.btest('test_webgl_context_attributes_sdl.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])
    self.btest('test_webgl_context_attributes_glfw.c', '1', args=['--js-library', 'check_webgl_attributes_support.js', '-DAA_ACTIVATED', '-DDEPTH_ACTIVATED', '-DSTENCIL_ACTIVATED', '-DALPHA_ACTIVATED'])
    
    # perform tests with attributes desactivated
    self.btest('test_webgl_context_attributes_glut.c', '1', args=['--js-library', 'check_webgl_attributes_support.js'])
    self.btest('test_webgl_context_attributes_sdl.c', '1', args=['--js-library', 'check_webgl_attributes_support.js'])
    self.btest('test_webgl_context_attributes_glfw.c', '1', args=['--js-library', 'check_webgl_attributes_support.js'])
    
  def test_emscripten_get_now(self):
    self.btest('emscripten_get_now.cpp', '1')

  def test_fflush(self):
    return self.skip('Skipping due to https://github.com/kripken/emscripten/issues/2770')
    self.btest('test_fflush.cpp', '0', args=['-s', 'NO_EXIT_RUNTIME=1', '--shell-file', path_from_root('tests', 'test_fflush.html')])

  def test_file_db(self):
    secret = str(time.time())
    open('moar.txt', 'w').write(secret)
    self.btest('file_db.cpp', '1', args=['--preload-file', 'moar.txt', '-DFIRST'])
    shutil.copyfile('test.html', 'first.html')
    self.btest('file_db.cpp', secret, args=['-s', 'FORCE_FILESYSTEM=1'])
    shutil.copyfile('test.html', 'second.html')
    open('moar.txt', 'w').write('aliantha')
    self.btest('file_db.cpp', secret, args=['--preload-file', 'moar.txt']) # even with a file there, we load over it
    shutil.move('test.html', 'third.html')

  def test_fs_idbfs_sync(self):
    for mode in [[], ['-s', 'MEMFS_APPEND_TO_TYPED_ARRAYS=1']]:
      for extra in [[], ['-DEXTRA_WORK']]:
        secret = str(time.time())
        self.btest(path_from_root('tests', 'fs', 'test_idbfs_sync.c'), '1', force_c=True, args=mode + ['-DFIRST', '-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_test', '_success']'''])
        self.btest(path_from_root('tests', 'fs', 'test_idbfs_sync.c'), '1', force_c=True, args=mode + ['-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_test', '_success']'''] + extra)

  def test_fs_idbfs_fsync(self):
    # sync from persisted state into memory before main()
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
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

    args = ['--pre-js', 'pre.js', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'];
    for mode in [[], ['-s', 'MEMFS_APPEND_TO_TYPED_ARRAYS=1']]:
      secret = str(time.time())
      self.btest(path_from_root('tests', 'fs', 'test_idbfs_fsync.c'), '1', force_c=True, args=args + mode + ['-DFIRST', '-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_success']'''])
      self.btest(path_from_root('tests', 'fs', 'test_idbfs_fsync.c'), '1', force_c=True, args=args + mode + ['-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main', '_success']'''])

  def test_fs_memfs_fsync(self):
    args = ['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'];
    for mode in [[], ['-s', 'MEMFS_APPEND_TO_TYPED_ARRAYS=1']]:
      secret = str(time.time())
      self.btest(path_from_root('tests', 'fs', 'test_memfs_fsync.c'), '1', force_c=True, args=args + mode + ['-DSECRET=\"' + secret + '\"', '-s', '''EXPORTED_FUNCTIONS=['_main']'''])

  def test_fs_workerfs_read(self):
    secret = 'a' * 10;
    secret2 = 'b' * 10;
    open(self.in_dir('pre.js'), 'w').write('''
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
    self.btest(path_from_root('tests', 'fs', 'test_workerfs_read.c'), '1', force_c=True, args=['--pre-js', 'pre.js', '-DSECRET=\"' + secret + '\"', '-DSECRET2=\"' + secret2 + '\"', '--proxy-to-worker'])

  def test_fs_workerfs_package(self):
    open('file1.txt', 'w').write('first')
    if not os.path.exists('sub'): os.makedirs('sub')
    open(os.path.join('sub', 'file2.txt'), 'w').write('second')
    Popen([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', os.path.join('sub', 'file2.txt'), '--separate-metadata', '--js-output=files.js']).communicate()
    self.btest(os.path.join('fs', 'test_workerfs_package.cpp'), '1', args=['--proxy-to-worker'])

  def test_fs_lz4fs_package(self):
    # generate data
    import random
    self.clear()
    os.mkdir('subdir')
    open('file1.txt', 'wb').write('0123456789' * (1024*128))
    open(os.path.join('subdir', 'file2.txt'), 'wb').write('1234567890' * (1024*128))
    random_data = [chr(random.randint(0,255)) for x in range(1024*128*10 + 1)]
    random_data[17] = 'X'
    open('file3.txt', 'wb').write(''.join(random_data))

    # compress in emcc,  -s LZ4=1  tells it to tell the file packager
    print 'emcc-normal'
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['-s', 'LZ4=1', '--preload-file', 'file1.txt', '--preload-file', 'subdir/file2.txt', '--preload-file', 'file3.txt'], timeout=60)
    assert os.stat('file1.txt').st_size + os.stat(os.path.join('subdir', 'file2.txt')).st_size + os.stat('file3.txt').st_size == 3*1024*128*10 + 1
    assert os.stat('test.data').st_size < (3*1024*128*10)/2 # over half is gone
    print '    emcc-opts'
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['-s', 'LZ4=1', '--preload-file', 'file1.txt', '--preload-file', 'subdir/file2.txt', '--preload-file', 'file3.txt', '-O2'], timeout=60)

    # compress in the file packager, on the server. the client receives compressed data and can just use it. this is typical usage
    print 'normal'
    out = subprocess.check_output([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'subdir/file2.txt', 'file3.txt', '--lz4'])
    open('files.js', 'wb').write(out)
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['--pre-js', 'files.js', '-s', 'LZ4=1'], timeout=60)
    print '    opts'
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '2', args=['--pre-js', 'files.js', '-s', 'LZ4=1', '-O2'], timeout=60)

    # load the data into LZ4FS manually at runtime. This means we compress on the client. This is generally not recommended
    print 'manual'
    subprocess.check_output([PYTHON, FILE_PACKAGER, 'files.data', '--preload', 'file1.txt', 'subdir/file2.txt', 'file3.txt', '--separate-metadata', '--js-output=files.js'])
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '1', args=['-DLOAD_MANUALLY', '-s', 'LZ4=1'], timeout=60)
    print '    opts'
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '1', args=['-DLOAD_MANUALLY', '-s', 'LZ4=1', '-O2'], timeout=60)
    print '    opts+closure'
    self.btest(os.path.join('fs', 'test_lz4fs.cpp'), '1', args=['-DLOAD_MANUALLY', '-s', 'LZ4=1', '-O2', '--closure', '1', '-g1'], timeout=60)

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

  def test_idbstore(self):
    secret = str(time.time())
    for stage in [0, 1, 2, 3, 0, 1, 2, 0, 0, 1, 4, 2, 5]:
      self.clear()
      self.btest(path_from_root('tests', 'idbstore.c'), str(stage), force_c=True, args=['-DSTAGE=' + str(stage), '-DSECRET=\"' + secret + '\"'])

  def test_idbstore_sync(self):
    secret = str(time.time())
    self.clear()
    self.btest(path_from_root('tests', 'idbstore_sync.c'), '6', force_c=True, args=['-DSECRET=\"' + secret + '\"', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '--memory-init-file', '1', '-O3', '-g2'])

  def test_idbstore_sync_worker(self):
    secret = str(time.time())
    self.clear()
    self.btest(path_from_root('tests', 'idbstore_sync_worker.c'), '6', force_c=True, args=['-DSECRET=\"' + secret + '\"', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '--memory-init-file', '1', '-O3', '-g2', '--proxy-to-worker', '-s', 'TOTAL_MEMORY=75000000'])

  def test_force_exit(self):
    self.btest('force_exit.c', force_c=True, expected='17')

  def test_sdl_pumpevents(self):
    # key events should be detected using SDL_PumpEvents
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      function keydown(c) {
        var event = document.createEvent("KeyboardEvent");
        event.initKeyEvent("keydown", true, true, window,
                           0, 0, 0, 0,
                           c, c);
        document.dispatchEvent(event);
      }
    ''')
    self.btest('sdl_pumpevents.c', expected='7', args=['--pre-js', 'pre.js'])

  def test_sdl_canvas_size(self):
    self.btest('sdl_canvas_size.c', expected='1',
      args=['-O2', '--minify', '0', '--shell-file', path_from_root('tests', 'sdl_canvas_size.html')])

  def test_sdl_gl_read(self):
    # SDL, OpenGL, readPixels
    open(os.path.join(self.get_dir(), 'sdl_gl_read.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_gl_read.c')).read()))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_gl_read.c'), '-o', 'something.html']).communicate()
    self.run_browser('something.html', '.', '/report_result?1')

  def test_sdl_gl_mapbuffers(self):
    self.btest('sdl_gl_mapbuffers.c', expected='1', args=['-s', 'FULL_ES3=1'],
               message='You should see a blue triangle.')

  def test_sdl_ogl(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl.c', reference='screenshot-gray-purple.png', reference_slack=1,
      args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with gray at the top.')

  def test_sdl_ogl_defaultmatrixmode(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl_defaultMatrixMode.c', reference='screenshot-gray-purple.png', reference_slack=1,
      args=['--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with gray at the top.')

  def test_sdl_ogl_p(self):
    # Immediate mode with pointers
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl_p.c', reference='screenshot-gray.png', reference_slack=1,
      args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with gray at the top.')

  def test_sdl_ogl_proc_alias(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl_proc_alias.c', reference='screenshot-gray-purple.png', reference_slack=1,
               args=['-O2', '-g2', '-s', 'INLINING_LIMIT=1', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'])

  def test_sdl_fog_simple(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_simple.c', reference='screenshot-fog-simple.png',
      args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl_fog_negative(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_negative.c', reference='screenshot-fog-negative.png',
      args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl_fog_density(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_density.c', reference='screenshot-fog-density.png',
      args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl_fog_exp2(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_exp2.c', reference='screenshot-fog-exp2.png',
      args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl_fog_linear(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_linear.c', reference='screenshot-fog-linear.png', reference_slack=1,
      args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_glfw(self):
    self.btest('glfw.c', '1', args=['-s', 'LEGACY_GL_EMULATION=1'])
    self.btest('glfw.c', '1', args=['-s', 'LEGACY_GL_EMULATION=1', '-s', 'USE_GLFW=2'])

  def test_glfw_minimal(self):
    self.btest('glfw_minimal.c', '1', args=[])
    self.btest('glfw_minimal.c', '1', args=['-s', 'USE_GLFW=2'])

  def test_egl(self):
    open(os.path.join(self.get_dir(), 'test_egl.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'test_egl.c')).read()))

    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'test_egl.c'), '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_egl_width_height(self):
    open(os.path.join(self.get_dir(), 'test_egl_width_height.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'test_egl_width_height.c')).read()))

    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'test_egl_width_height.c'), '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'Should print "(300, 150)" -- the size of the canvas in pixels', '/report_result?1')

  def test_worker(self):
    # Test running in a web worker
    open('file.dat', 'w').write('data for worker')
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

    # no file data
    for file_data in [0, 1]:
      print 'file data', file_data
      output = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_worker.cpp'), '-o', 'worker.js'] + (['--preload-file', 'file.dat'] if file_data else []) , stdout=PIPE, stderr=PIPE).communicate()
      assert len(output[0]) == 0, output[0]
      assert os.path.exists('worker.js'), output
      if not file_data: self.assertContained('you should not see this text when in a worker!', run_js('worker.js')) # code should run standalone
      self.run_browser('main.html', '', '/report_result?hello%20from%20worker,%20and%20|' + ('data%20for%20w' if file_data else '') + '|')

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
              xhr.open('GET', 'http://localhost:8888/report_result?' + event.data.line);
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
    """)
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
      Module["print"] =    function(s) { self.postMessage({channel: "stdout", line: s}); };
      Module["stderr"] =   function(s) { self.postMessage({channel: "stderr", char: s, trace: ((doTrace && s === 10) ? new Error().stack : null)}); doTrace = false; };
    """)
    prejs_file.close()
    # vs. os.path.join(self.get_dir(), filename)
    # vs. path_from_root('tests', 'hello_world_gles.c')
    Popen([PYTHON, EMCC, path_from_root('tests', c_source_filename), '-g', '-s', 'SMALL_CHUNKS=1', '-o', worker_filename,
                                         '--pre-js', prejs_filename]).communicate()

    chunkSize = 1024
    data = os.urandom(10*chunkSize+1) # 10 full chunks and one 1 byte chunk
    checksum = zlib.adler32(data)

    server = multiprocessing.Process(target=test_chunked_synchronous_xhr_server, args=(True,chunkSize,data,checksum,))
    server.start()
    self.run_browser(main, 'Chunked binary synchronous XHR in Web Workers!', '/report_result?' + str(checksum))
    server.terminate()
    # Avoid race condition on cleanup, wait a bit so that processes have released file locks so that test tearDown won't
    # attempt to rmdir() files in use.
    if WINDOWS:
      time.sleep(2)

  def test_glgears(self):
    self.btest('hello_world_gles.c', reference='gears.png', reference_slack=3,
        args=['-DHAVE_BUILTIN_SINCOS'], outfile='something.html',
        message='You should see animating gears.')

  def test_glgears_long(self):
    for proxy in [0, 1]:
      print 'proxy', proxy
      self.btest('hello_world_gles.c', expected=map(str, range(30, 500)), args=['-DHAVE_BUILTIN_SINCOS', '-DLONGTEST'] + (['--proxy-to-worker'] if proxy else []), timeout=30)

  def test_glgears_animation(self):
    es2_suffix = ['', '_full', '_full_944']
    for full_es2 in [0, 1, 2]:
      print full_es2
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_gles%s.c' % es2_suffix[full_es2]), '-o', 'something.html',
                                           '-DHAVE_BUILTIN_SINCOS', '-s', 'GL_TESTING=1',
                                           '--shell-file', path_from_root('tests', 'hello_world_gles_shell.html')] +
            (['-s', 'FULL_ES2=1'] if full_es2 else []),
            ).communicate()
      self.run_browser('something.html', 'You should see animating gears.', '/report_gl_result?true')

  def test_fulles2_sdlproc(self):
    self.btest('full_es2_sdlproc.c', '1', args=['-s', 'GL_TESTING=1', '-DHAVE_BUILTIN_SINCOS', '-s', 'FULL_ES2=1'])

  def test_glgears_deriv(self):
    self.btest('hello_world_gles_deriv.c', reference='gears.png', reference_slack=2,
        args=['-DHAVE_BUILTIN_SINCOS'], outfile='something.html',
        message='You should see animating gears.')
    with open('something.html') as f:
      assert 'gl-matrix' not in f.read(), 'Should not include glMatrix when not needed'

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
      print program
      basename = os.path.basename(program)
      args = []
      if basename == 'CH10_MultiTexture.bc':
        shutil.copyfile(book_path('Chapter_10', 'MultiTexture', 'basemap.tga'), os.path.join(self.get_dir(), 'basemap.tga'))
        shutil.copyfile(book_path('Chapter_10', 'MultiTexture', 'lightmap.tga'), os.path.join(self.get_dir(), 'lightmap.tga'))
        args = ['--preload-file', 'basemap.tga', '--preload-file', 'lightmap.tga']
      elif basename == 'CH13_ParticleSystem.bc':
        shutil.copyfile(book_path('Chapter_13', 'ParticleSystem', 'smoke.tga'), os.path.join(self.get_dir(), 'smoke.tga'))
        args = ['--preload-file', 'smoke.tga', '-O2'] # test optimizations and closure here as well for more coverage

      self.btest(program,
          reference=book_path(basename.replace('.bc', '.png')), args=args, timeout=30)

  def test_gles2_emulation(self):
    shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_10', 'MultiTexture', 'basemap.tga'), self.in_dir('basemap.tga'))
    shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_10', 'MultiTexture', 'lightmap.tga'), self.in_dir('lightmap.tga'))
    shutil.copyfile(path_from_root('tests', 'glbook', 'Chapter_13', 'ParticleSystem', 'smoke.tga'), self.in_dir('smoke.tga'))

    for source, reference in [
      (os.path.join('glbook', 'Chapter_2', 'Hello_Triangle', 'Hello_Triangle_orig.c'), path_from_root('tests', 'glbook', 'CH02_HelloTriangle.png')),
      #(os.path.join('glbook', 'Chapter_8', 'Simple_VertexShader', 'Simple_VertexShader_orig.c'), path_from_root('tests', 'glbook', 'CH08_SimpleVertexShader.png')), # XXX needs INT extension in WebGL
      (os.path.join('glbook', 'Chapter_9', 'TextureWrap', 'TextureWrap_orig.c'), path_from_root('tests', 'glbook', 'CH09_TextureWrap.png')),
      #(os.path.join('glbook', 'Chapter_9', 'Simple_TextureCubemap', 'Simple_TextureCubemap_orig.c'), path_from_root('tests', 'glbook', 'CH09_TextureCubemap.png')), # XXX needs INT extension in WebGL
      (os.path.join('glbook', 'Chapter_9', 'Simple_Texture2D', 'Simple_Texture2D_orig.c'), path_from_root('tests', 'glbook', 'CH09_SimpleTexture2D.png')),
      (os.path.join('glbook', 'Chapter_10', 'MultiTexture', 'MultiTexture_orig.c'), path_from_root('tests', 'glbook', 'CH10_MultiTexture.png')),
      (os.path.join('glbook', 'Chapter_13', 'ParticleSystem', 'ParticleSystem_orig.c'), path_from_root('tests', 'glbook', 'CH13_ParticleSystem.png')),
    ]:
      print source
      self.btest(source,
                 reference=reference,
                 args=['-I' + path_from_root('tests', 'glbook', 'Common'),
                       path_from_root('tests', 'glbook', 'Common', 'esUtil.c'),
                       path_from_root('tests', 'glbook', 'Common', 'esShader.c'),
                       path_from_root('tests', 'glbook', 'Common', 'esShapes.c'),
                       path_from_root('tests', 'glbook', 'Common', 'esTransform.c'),
                       '-s', 'FULL_ES2=1',
                       '--preload-file', 'basemap.tga', '--preload-file', 'lightmap.tga', '--preload-file', 'smoke.tga'])

  def test_emscripten_api(self):
    self.btest('emscripten_api_browser.cpp', '1', args=['-s', '''EXPORTED_FUNCTIONS=['_main', '_third']'''])

  def test_emscripten_api2(self):
    def setup():
      open('script1.js', 'w').write('''
        Module._set(456);
      ''')
      open('file1.txt', 'w').write('first');
      open('file2.txt', 'w').write('second');

    setup()
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'file1.txt', 'file2.txt'], stdout=open('script2.js', 'w')).communicate()
    self.btest('emscripten_api_browser2.cpp', '1', args=['-s', '''EXPORTED_FUNCTIONS=['_main', '_set']'''])

    # check using file packager to another dir
    self.clear()
    setup()
    os.mkdir('sub')
    Popen([PYTHON, FILE_PACKAGER, 'sub/test.data', '--preload', 'file1.txt', 'file2.txt'], stdout=open('script2.js', 'w')).communicate()
    shutil.copyfile(os.path.join('sub', 'test.data'), 'test.data')
    self.btest('emscripten_api_browser2.cpp', '1', args=['-s', '''EXPORTED_FUNCTIONS=['_main', '_set']'''])

  def test_emscripten_api_infloop(self):
    self.btest('emscripten_api_browser_infloop.cpp', '7')

  def test_emscripten_fs_api(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png')) # preloaded *after* run
    self.btest('emscripten_fs_api_browser.cpp', '1')

  def test_emscripten_fs_api2(self):
    self.btest('emscripten_fs_api_browser2.cpp', '1', args=['-s', "ASSERTIONS=0"])
    self.btest('emscripten_fs_api_browser2.cpp', '1', args=['-s', "ASSERTIONS=1"])

  def test_emscripten_main_loop(self):
    self.btest('emscripten_main_loop.cpp', '0')

  def test_emscripten_main_loop_settimeout(self):
    self.btest('emscripten_main_loop_settimeout.cpp', '1')

  def test_emscripten_main_loop_and_blocker(self):
    self.btest('emscripten_main_loop_and_blocker.cpp', '0')

  def test_sdl_quit(self):
    self.btest('sdl_quit.c', '1')

  def test_sdl_resize(self):
    self.btest('sdl_resize.c', '1')

  def test_glshaderinfo(self):
    self.btest('glshaderinfo.cpp', '1')

  def test_glgetattachedshaders(self):
    self.btest('glgetattachedshaders.c', '1')

  def test_sdlglshader(self):
    self.btest('sdlglshader.c', reference='sdlglshader.png', args=['-O2', '--closure', '1', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_sdlglshader2(self):
    self.btest('sdlglshader2.c', expected='1', args=['-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True)

  def test_gl_glteximage(self):
    self.btest('gl_teximage.c', '1')

  def test_gl_textures(self):
    self.btest('gl_textures.cpp', '0')

  def test_gl_ps(self):
    # pointers and a shader
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'], reference_slack=1)

  def test_gl_ps_packed(self):
    # packed data that needs to be strided
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps_packed.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'], reference_slack=1)

  def test_gl_ps_strides(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps_strides.c', reference='gl_ps_strides.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'])

  def test_gl_ps_worker(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps_worker.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'], reference_slack=1, also_proxied=True)

  def test_gl_renderers(self):
    self.btest('gl_renderers.c', reference='gl_renderers.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_gl_stride(self):
    self.btest('gl_stride.c', reference='gl_stride.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_gl_vertex_buffer_pre(self):
    self.btest('gl_vertex_buffer_pre.c', reference='gl_vertex_buffer_pre.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_gl_vertex_buffer(self):
    self.btest('gl_vertex_buffer.c', reference='gl_vertex_buffer.png', args=['-s', 'GL_UNSAFE_OPTS=0', '-s', 'LEGACY_GL_EMULATION=1'], reference_slack=1)

  def test_gles2_uniform_arrays(self):
    self.btest('gles2_uniform_arrays.cpp', args=['-s', 'GL_ASSERTIONS=1'], expected=['1'], also_proxied=True)

  def test_gles2_conformance(self):
    self.btest('gles2_conformance.cpp', args=['-s', 'GL_ASSERTIONS=1'], expected=['1'])

  def test_matrix_identity(self):
    self.btest('gl_matrix_identity.c', expected=['-1882984448', '460451840', '1588195328'], args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_pre(self):
    self.btest('cubegeom_pre.c', reference='cubegeom_pre.png', args=['-s', 'LEGACY_GL_EMULATION=1'])
    self.btest('cubegeom_pre.c', reference='cubegeom_pre.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-s', 'RELOCATABLE=1'])

  def test_cubegeom_pre2(self):
    self.btest('cubegeom_pre2.c', reference='cubegeom_pre2.png', args=['-s', 'GL_DEBUG=1', '-s', 'LEGACY_GL_EMULATION=1']) # some coverage for GL_DEBUG not breaking the build

  def test_cubegeom_pre3(self):
    self.btest('cubegeom_pre3.c', reference='cubegeom_pre2.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom(self):
    self.btest('cubegeom.c', reference='cubegeom.png', args=['-O2', '-g', '-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True)

  def test_cubegeom_proc(self):
    open('side.c', 'w').write(r'''

extern void* SDL_GL_GetProcAddress(const char *);

void *glBindBuffer = 0; // same name as the gl function, to check that the collision does not break us

void *getBindBuffer() {
  if (!glBindBuffer) glBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
  return glBindBuffer;
}
''')
    for opts in [0, 1]:
      self.btest('cubegeom_proc.c', reference='cubegeom.png', args=['-O' + str(opts), 'side.c', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_glew(self):
    self.btest('cubegeom_glew.c', reference='cubegeom.png', args=['-O2', '--closure', '1', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_color(self):
    self.btest('cubegeom_color.c', reference='cubegeom_color.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_normal(self):
    self.btest('cubegeom_normal.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True)

  def test_cubegeom_normal_dap(self): # draw is given a direct pointer to clientside memory, no element array buffer
    self.btest('cubegeom_normal_dap.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True)

  def test_cubegeom_normal_dap_far(self): # indices do nto start from 0
    self.btest('cubegeom_normal_dap_far.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_normal_dap_far_range(self): # glDrawRangeElements
    self.btest('cubegeom_normal_dap_far_range.c', reference='cubegeom_normal.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_normal_dap_far_glda(self): # use glDrawArrays
    self.btest('cubegeom_normal_dap_far_glda.c', reference='cubegeom_normal_dap_far_glda.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_normal_dap_far_glda_quad(self): # with quad
    self.btest('cubegeom_normal_dap_far_glda_quad.c', reference='cubegeom_normal_dap_far_glda_quad.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_mt(self):
    self.btest('cubegeom_mt.c', reference='cubegeom_mt.png', args=['-s', 'LEGACY_GL_EMULATION=1']) # multitexture

  def test_cubegeom_color2(self):
    self.btest('cubegeom_color2.c', reference='cubegeom_color2.png', args=['-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True)

  def test_cubegeom_texturematrix(self):
    self.btest('cubegeom_texturematrix.c', reference='cubegeom_texturematrix.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_fog(self):
    self.btest('cubegeom_fog.c', reference='cubegeom_fog.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_pre_vao(self):
    self.btest('cubegeom_pre_vao.c', reference='cubegeom_pre_vao.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_pre2_vao(self):
    self.btest('cubegeom_pre2_vao.c', reference='cubegeom_pre_vao.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_pre2_vao2(self):
    self.btest('cubegeom_pre2_vao2.c', reference='cubegeom_pre2_vao2.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_cubegeom_u4fv_2(self):
    self.btest('cubegeom_u4fv_2.c', reference='cubegeom_u4fv_2.png', args=['-s', 'LEGACY_GL_EMULATION=1'])
    self.btest('cubegeom_u4fv_2.c', reference='cubegeom_u4fv_2.png', args=['-s', 'LEGACY_GL_EMULATION=1', '-s', 'SPLIT_MEMORY=16777216']) # check for uniform4fv slice being valid in split memory

  def test_cube_explosion(self):
    self.btest('cube_explosion.c', reference='cube_explosion.png', args=['-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True)

  def test_glgettexenv(self):
    self.btest('glgettexenv.c', args=['-s', 'LEGACY_GL_EMULATION=1'], expected=['1'])

  def test_sdl_canvas_blank(self):
    self.btest('sdl_canvas_blank.c', reference='sdl_canvas_blank.png')

  def test_sdl_canvas_palette(self):
    self.btest('sdl_canvas_palette.c', reference='sdl_canvas_palette.png')

  def test_sdl_canvas_twice(self):
    self.btest('sdl_canvas_twice.c', reference='sdl_canvas_twice.png')

  def test_sdl_set_clip_rect(self):
    self.btest('sdl_set_clip_rect.c', reference='sdl_set_clip_rect.png')

  def test_sdl_maprgba(self):
    self.btest('sdl_maprgba.c', reference='sdl_maprgba.png', reference_slack=3)

  def test_sdl_create_rgb_surface_from(self):
    self.btest('sdl_create_rgb_surface_from.c', reference='sdl_create_rgb_surface_from.png')

  def test_sdl_rotozoom(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_rotozoom.c', reference='sdl_rotozoom.png', args=['--preload-file', 'screenshot.png', '--use-preload-plugins'], reference_slack=3)

  def test_sdl_gfx_primitives(self):
    self.btest('sdl_gfx_primitives.c', reference='sdl_gfx_primitives.png', reference_slack=1)

  def test_sdl_canvas_palette_2(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      Module['preRun'].push(function() {
        SDL.defaults.copyOnLock = false;
      });
    ''')

    open(os.path.join(self.get_dir(), 'args-r.js'), 'w').write('''
      Module['arguments'] = ['-r'];
    ''')

    open(os.path.join(self.get_dir(), 'args-g.js'), 'w').write('''
      Module['arguments'] = ['-g'];
    ''')

    open(os.path.join(self.get_dir(), 'args-b.js'), 'w').write('''
      Module['arguments'] = ['-b'];
    ''')

    self.btest('sdl_canvas_palette_2.c', reference='sdl_canvas_palette_r.png', args=['--pre-js', 'pre.js', '--pre-js', 'args-r.js'])
    self.btest('sdl_canvas_palette_2.c', reference='sdl_canvas_palette_g.png', args=['--pre-js', 'pre.js', '--pre-js', 'args-g.js'])
    self.btest('sdl_canvas_palette_2.c', reference='sdl_canvas_palette_b.png', args=['--pre-js', 'pre.js', '--pre-js', 'args-b.js'])

  def test_sdl_alloctext(self):
    self.btest('sdl_alloctext.c', expected='1', args=['-O2', '-s', 'TOTAL_MEMORY=' + str(1024*1024*8)])

  def test_sdl_surface_refcount(self):
    self.btest('sdl_surface_refcount.c', expected='1')

  def test_sdl_free_screen(self):
    self.btest('sdl_free_screen.cpp', reference='htmltest.png')

  def test_glbegin_points(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('glbegin_points.c', reference='glbegin_points.png', args=['--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'])

  def test_s3tc(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.dds'), os.path.join(self.get_dir(), 'screenshot.dds'))
    self.btest('s3tc.c', reference='s3tc.png', args=['--preload-file', 'screenshot.dds', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_s3tc_ffp_only(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.dds'), os.path.join(self.get_dir(), 'screenshot.dds'))
    self.btest('s3tc.c', reference='s3tc.png', args=['--preload-file', 'screenshot.dds', '-s', 'LEGACY_GL_EMULATION=1', '-s', 'GL_FFP_ONLY=1'])

  def test_s3tc_crunch(self):
    try:
      print 'Crunch is located at ' + CRUNCH
    except:
      return self.skip('Skipped: Crunch is not present on the current system. Please install it (manually or via emsdk) and make sure it is activated in the Emscripten configuration file.')
    def test(args):
      print args
      shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
      shutil.copyfile(path_from_root('tests', 'bloom.dds'), 'bloom.dds')
      shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
      open('text.txt', 'w').write('123')
      Popen([PYTHON, FILE_PACKAGER, 'test.data', '--crunch', '--preload', 'ship.dds', 'bloom.dds', 'water.dds'] + args, stdout=open('pre.js', 'w')).communicate()
      assert os.stat('test.data').st_size < 0.5*(os.stat('ship.dds').st_size+os.stat('bloom.dds').st_size+os.stat('water.dds').st_size), 'Compressed should be smaller than dds'
      shutil.move('ship.dds', 'ship.donotfindme.dds') # make sure we load from the compressed
      shutil.move('bloom.dds', 'bloom.donotfindme.dds') # make sure we load from the compressed
      shutil.move('water.dds', 'water.donotfindme.dds') # make sure we load from the compressed
      self.btest('s3tc_crunch.c', reference='s3tc_crunch.png', reference_slack=11, args=['--pre-js', 'pre.js', '-s', 'LEGACY_GL_EMULATION=1'])
    test([])
    test(['text.txt']) # also package a non-crunch file

  def test_s3tc_crunch_split(self): # load several datafiles/outputs of file packager
    try:
      print 'Crunch is located at ' + CRUNCH
    except:
      return self.skip('Skipped: Crunch is not present on the current system. Please install it (manually or via emsdk) and make sure it is activated in the Emscripten configuration file.')
    shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
    shutil.copyfile(path_from_root('tests', 'bloom.dds'), 'bloom.dds')
    shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
    Popen([PYTHON, FILE_PACKAGER, 'asset_a.data', '--crunch', '--preload', 'ship.dds', 'bloom.dds'], stdout=open('asset_a.js', 'w')).communicate()
    Popen([PYTHON, FILE_PACKAGER, 'asset_b.data', '--crunch', '--preload', 'water.dds'], stdout=open('asset_b.js', 'w')).communicate()
    shutil.move('ship.dds', 'ship.donotfindme.dds') # make sure we load from the compressed
    shutil.move('bloom.dds', 'bloom.donotfindme.dds') # make sure we load from the compressed
    shutil.move('water.dds', 'water.donotfindme.dds') # make sure we load from the compressed
    self.btest('s3tc_crunch.c', reference='s3tc_crunch.png', reference_slack=11, args=['--pre-js', 'asset_a.js', '--pre-js', 'asset_b.js', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_aniso(self):
    if SPIDERMONKEY_ENGINE in JS_ENGINES:
      # asm.js-ification check
      Popen([PYTHON, EMCC, path_from_root('tests', 'aniso.c'), '-O2', '-g2', '-s', 'LEGACY_GL_EMULATION=1']).communicate()
      Settings.ASM_JS = 1
      self.run_generated_code(SPIDERMONKEY_ENGINE, 'a.out.js', assert_returncode=None)
      print 'passed asm test'

    shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
    self.btest('aniso.c', reference='aniso.png', reference_slack=2, args=['--preload-file', 'water.dds', '-s', 'LEGACY_GL_EMULATION=1'])

  def test_tex_nonbyte(self):
    self.btest('tex_nonbyte.c', reference='tex_nonbyte.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_float_tex(self):
    self.btest('float_tex.cpp', reference='float_tex.png')

  def test_subdata(self):
    self.btest('gl_subdata.cpp', reference='float_tex.png')

  def test_perspective(self):
    self.btest('perspective.c', reference='perspective.png', args=['-s', 'LEGACY_GL_EMULATION=1'])

  def test_runtimelink(self):
    main, supp = self.setup_runtimelink_test()
    open('supp.cpp', 'w').write(supp)
    Popen([PYTHON, EMCC, 'supp.cpp', '-o', 'supp.js', '-s', 'SIDE_MODULE=1', '-O2']).communicate()
    self.btest(main, args=['-DBROWSER=1', '-s', 'MAIN_MODULE=1', '-O2', '-s', 'RUNTIME_LINKED_LIBS=["supp.js"]'], expected='76')

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

    for mem in [0, 1]:
      self.btest('pre_run_deps.cpp', expected='10', args=['--pre-js', 'pre.js', '--memory-init-file', str(mem)])

  def test_mem_init(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      function myJSCallback() { // called from main()
        Module._note(1);
      }
      Module.preRun = function() {
        addOnPreMain(function() {
          Module._note(2);
        });
      };
    ''')
    open(os.path.join(self.get_dir(), 'post.js'), 'w').write('''
      var assert = function(check, text) {
        if (!check) {
          var xhr = new XMLHttpRequest();
          xhr.open('GET', 'http://localhost:8888/report_result?9');
          xhr.onload = function() {
            window.close();
          };
          xhr.send();
        }
      }
      Module._note(4); // this happens too early! and is overwritten when the mem init arrives
    ''')

    # with assertions, we notice when memory was written to too early
    self.btest('mem_init.cpp', expected='9', args=['--pre-js', 'pre.js', '--post-js', 'post.js', '--memory-init-file', '1'])
    # otherwise, we just overwrite
    self.btest('mem_init.cpp', expected='3', args=['--pre-js', 'pre.js', '--post-js', 'post.js', '--memory-init-file', '1', '-s', 'ASSERTIONS=0'])

  def test_mem_init_request(self):
    def test(what, status):
      print what, status
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        var xhr = Module.memoryInitializerRequest = new XMLHttpRequest();
        xhr.open('GET', "''' + what + '''", true);
        xhr.responseType = 'arraybuffer';
        xhr.send(null);

        console.warn = function(x) {
          if (x.indexOf('a problem seems to have happened with Module.memoryInitializerRequest') >= 0) {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', 'http://localhost:8888/report_result?0');
            setTimeout(xhr.onload = function() {
              console.log('close!');
              window.close();
            }, 1000);
            xhr.send();
            throw 'halt';
          }
          console.log('WARNING: ' + x);
        };
      ''')
      self.btest('mem_init_request.cpp', expected=status, args=['--pre-js', 'pre.js', '--memory-init-file', '1'])

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
        Module.print('got ' + str);
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
        Module.print('expected fail 1');
        assert(e.toString().indexOf('assert') >= 0); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);

      ok = false;
      try {
        doCwrapCall(2);
        ok = true; // should fail and not reach here, runtime is not ready yet so cwrap call will abort
      } catch(e) {
        Module.print('expected fail 2');
        assert(e.toString().indexOf('assert') >= 0); // assertion, not something else
        ABORT = false; // hackish
      }
      assert(ok === expected_ok);

      ok = false;
      try {
        doDirectCall(3);
        ok = true; // should fail and not reach here, runtime is not ready yet so any code execution
      } catch(e) {
        Module.print('expected fail 3');
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
        xhr.open('GET', 'http://localhost:8888/report_result?' + HEAP32[Module.noted>>2]);
        xhr.send();
        setTimeout(function() { window.close() }, 1000);
      }, 1000);
    '''

    open('pre_main.js', 'w').write(r'''
      Module._main = function(){
        myJSCallback();
        return 0;
      };
    ''')

    open('pre_runtime.js', 'w').write(r'''
      Module.onRuntimeInitialized = function(){
        myJSCallback();
      };
    ''')

    for filename, extra_args, second_code in [
      ('runtime_misuse.cpp', [], 600),
      ('runtime_misuse_2.cpp', ['--pre-js', 'pre_main.js'], 600),
      ('runtime_misuse_2.cpp', ['--pre-js', 'pre_runtime.js'], 601) # 601, because no main means we *do* run another call after exit()
    ]:
      print '\n', filename, extra_args
      print 'mem init, so async, call too early'
      open(os.path.join(self.get_dir(), 'post.js'), 'w').write(post_prep + post_test + post_hook)
      self.btest(filename, expected='600', args=['--post-js', 'post.js', '--memory-init-file', '1'] + extra_args)
      print 'sync startup, call too late'
      open(os.path.join(self.get_dir(), 'post.js'), 'w').write(post_prep + 'Module.postRun.push(function() { ' + post_test + ' });' + post_hook);
      self.btest(filename, expected=str(second_code), args=['--post-js', 'post.js', '--memory-init-file', '0'] + extra_args)
      print 'sync, runtime still alive, so all good'
      open(os.path.join(self.get_dir(), 'post.js'), 'w').write(post_prep + 'expected_ok = true; Module.postRun.push(function() { ' + post_test + ' });' + post_hook);
      self.btest(filename, expected='606', args=['--post-js', 'post.js', '--memory-init-file', '0', '-s', 'NO_EXIT_RUNTIME=1'] + extra_args)

  def test_worker_api(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'worker_api_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]']).communicate()
    self.btest('worker_api_main.cpp', expected='566')

  def test_worker_api_2(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'worker_api_2_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-O2', '--minify', '0', '-s', 'EXPORTED_FUNCTIONS=["_one", "_two", "_three", "_four"]']).communicate()
    self.btest('worker_api_2_main.cpp', args=['-O2', '--minify', '0'], expected='11')

  def test_worker_api_3(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'worker_api_3_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]']).communicate()
    self.btest('worker_api_3_main.cpp', expected='5')

  def test_worker_api_sleep(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'worker_api_worker_sleep.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]', '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1']).communicate()
    self.btest('worker_api_main.cpp', expected='566')

  def test_emscripten_async_wget2(self):
    self.btest('http.cpp', expected='0', args=['-I' + path_from_root('tests')])

  # TODO: test only worked in non-fastcomp
  def test_module(self):
    return self.skip('non-fastcomp is deprecated and fails in 3.5')
    Popen([PYTHON, EMCC, path_from_root('tests', 'browser_module.cpp'), '-o', 'module.js', '-O2', '-s', 'SIDE_MODULE=1', '-s', 'DLOPEN_SUPPORT=1', '-s', 'EXPORTED_FUNCTIONS=["_one", "_two"]']).communicate()
    self.btest('browser_main.cpp', args=['-O2', '-s', 'MAIN_MODULE=1', '-s', 'DLOPEN_SUPPORT=1'], expected='8')

  def test_mmap_file(self):
    open(self.in_dir('data.dat'), 'w').write('data from the file ' + ('.' * 9000))
    for extra_args in [[], ['--no-heap-copy']]:
      self.btest(path_from_root('tests', 'mmap_file.c'), expected='1', args=['--preload-file', 'data.dat'] + extra_args)

  def test_emrun_info(self):
    result = subprocess.check_output([PYTHON, path_from_root('emrun'), '--system_info', '--browser_info'])
    assert 'CPU' in result
    assert 'Browser' in result
    assert 'Traceback' not in result

    result = subprocess.check_output([PYTHON, path_from_root('emrun'), '--list_browsers'])
    assert 'Traceback' not in result

  def test_emrun(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'test_emrun.c'), '--emrun', '-o', 'hello_world.html']).communicate()
    outdir = os.getcwd()
    # We cannot run emrun from the temp directory the suite will clean up afterwards, since the browser that is launched will have that directory as startup directory,
    # and the browser will not close as part of the test, pinning down the cwd on Windows and it wouldn't be possible to delete it. Therefore switch away from that directory
    # before launching.
    os.chdir(path_from_root())
    args = [PYTHON, path_from_root('emrun'), '--timeout', '30', '--safe_firefox_profile', '--verbose', '--log_stdout', os.path.join(outdir, 'stdout.txt'), '--log_stderr', os.path.join(outdir, 'stderr.txt')]
    if emscripten_browser is not None:
      # If EMSCRIPTEN_BROWSER carried command line arguments to pass to the browser, (e.g. "firefox -profile /path/to/foo") those can't be passed via emrun, so strip them out.
      browser_name = shlex.split(emscripten_browser)[0]
      args += ['--browser', browser_name]
    args += [os.path.join(outdir, 'hello_world.html'), '1', '2', '--3']
    process = subprocess.Popen(args)
    process.communicate()
    stdout = open(os.path.join(outdir, 'stdout.txt'), 'r').read()
    stderr = open(os.path.join(outdir, 'stderr.txt'), 'r').read()
    assert process.returncode == 100
    assert 'argc: 4' in stdout
    assert 'argv[3]: --3' in stdout
    assert 'hello, world!' in stdout
    assert 'Testing ASCII characters: !"$%&\'()*+,-./:;<=>?@[\\]^_`{|}~' in stdout
    assert 'Testing char sequences: %20%21 &auml;' in stdout
    assert 'hello, error stream!' in stderr

  # This does not actually verify anything except that --cpuprofiler and --memoryprofiler compiles.
  # Run interactive.test_cpuprofiler_memoryprofiler for interactive testing.
  def test_cpuprofiler_memoryprofiler(self):
    self.btest('hello_world_gles.c', expected='0', args=['-DLONGTEST=1', '-DTEST_MEMORYPROFILER_ALLOCATIONS_MAP=1', '-O2', '--cpuprofiler', '--memoryprofiler'], timeout=30)

  def test_uuid(self):
    # Run with ./runner.py browser.test_uuid
    # We run this test in Node/SPIDERMONKEY and browser environments because we try to make use of
    # high quality crypto random number generators such as crypto.getRandomValues or randomBytes (if available).

    # First run tests in Node and/or SPIDERMONKEY using run_js. Use closure compiler so we can check that
    # require('crypto').randomBytes and window.crypto.getRandomValues doesn't get minified out.
    Popen([PYTHON, EMCC, '-O2', '--closure', '1', path_from_root('tests', 'uuid', 'test.c'), '-o', 'test.js'], stdout=PIPE, stderr=PIPE).communicate()

    test_js_closure = open('test.js').read()

    # Check that test.js compiled with --closure 1 contains ").randomBytes" and "window.crypto.getRandomValues"
    assert ").randomBytes" in test_js_closure
    assert "window.crypto.getRandomValues" in test_js_closure

    out = run_js('test.js', full_output=True)
    print out

    # Tidy up files that might have been created by this test.
    try_delete(path_from_root('tests', 'uuid', 'test.js'))
    try_delete(path_from_root('tests', 'uuid', 'test.js.map'))

    # Now run test in browser
    self.btest(path_from_root('tests', 'uuid', 'test.c'), '1')

  def test_glew(self):
    self.btest(path_from_root('tests', 'glew.c'), expected='1')
    self.btest(path_from_root('tests', 'glew.c'), args=['-s', 'LEGACY_GL_EMULATION=1'], expected='1')
    self.btest(path_from_root('tests', 'glew.c'), args=['-DGLEW_MX'], expected='1')
    self.btest(path_from_root('tests', 'glew.c'), args=['-s', 'LEGACY_GL_EMULATION=1', '-DGLEW_MX'], expected='1')

  def test_doublestart_bug(self):
    open('pre.js', 'w').write(r'''
if (typeof Module === 'undefined') Module = eval('(function() { try { return Module || {} } catch(e) { return {} } })()');
if (!Module['preRun']) Module['preRun'] = [];
Module["preRun"].push(function () {
    Module['addRunDependency']('test_run_dependency');
    Module['removeRunDependency']('test_run_dependency');
});
''')

    self.btest('doublestart.c', args=['--pre-js', 'pre.js', '-o', 'test.html'], expected='1')

  def test_html5(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print opts
      self.btest(path_from_root('tests', 'test_html5.c'), args=opts, expected='0', timeout=20)

  def test_html5_webgl_create_context(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1'], ['-s', 'FULL_ES2=1']]:
      print opts
      self.btest(path_from_root('tests', 'webgl_create_context.cpp'), args=opts, expected='0', timeout=20)

  def test_html5_webgl_destroy_context(self):
    for opts in [[], ['-O2', '-g1'], ['-s', 'FULL_ES2=1']]:
      print opts
      self.btest(path_from_root('tests', 'webgl_destroy_context.cpp'), args=opts + ['--shell-file', path_from_root('tests/webgl_destroy_context_shell.html'), '-s', 'NO_EXIT_RUNTIME=1'], expected='0', timeout=20)

  def test_webgl2(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1'], ['-s', 'FULL_ES2=1']]:
      print opts
      self.btest(path_from_root('tests', 'webgl2.cpp'), args=['-s', 'USE_WEBGL2=1'] + opts, expected='0')

  def test_sdl_touch(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print opts
      self.btest(path_from_root('tests', 'sdl_touch.c'), args=opts + ['-DAUTOMATE_SUCCESS=1'], expected='0')

  def test_html5_mouse(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print opts
      self.btest(path_from_root('tests', 'test_html5_mouse.c'), args=opts + ['-DAUTOMATE_SUCCESS=1'], expected='0')

  def test_sdl_mousewheel(self):
    for opts in [[], ['-O2', '-g1', '--closure', '1']]:
      print opts
      self.btest(path_from_root('tests', 'test_sdl_mousewheel.c'), args=opts + ['-DAUTOMATE_SUCCESS=1'], expected='0')

  def test_codemods(self):
    for opt_level in [0, 2]:
      print 'opt level', opt_level
      opts = '-O' + str(opt_level)
      # sanity checks, building with and without precise float semantics generates different results
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='2', args=[opts])
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='1', args=[opts, '-s', 'PRECISE_F32=1'])
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='1', args=[opts, '-s', 'PRECISE_F32=2', '--separate-asm']) # empty polyfill, but browser has support, so semantics are like float

      # now use a shell to remove the browser's fround support
      open(self.in_dir('shell.html'), 'w').write(open(path_from_root('src', 'shell.html')).read().replace('var Module = {', '''
  Math.fround = null;
  var Module = {
  '''))
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='2', args=[opts, '--shell-file', 'shell.html'])
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='1', args=[opts, '--shell-file', 'shell.html', '-s', 'PRECISE_F32=1'])
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='2', args=[opts, '--shell-file', 'shell.html', '-s', 'PRECISE_F32=2', '--separate-asm']) # empty polyfill, no browser support, so semantics are like double

      # finally, remove fround, patch up fround as the code executes (after polyfilling etc.), to verify that we got rid of it entirely on the client side
      fixer = 'python fix.py'
      open('fix.py', 'w').write(r'''
import sys
filename = sys.argv[1]
js = open(filename).read()
replaced = js.replace("var Math_fround = Math.fround;", "var Math_fround = Math.fround = function(x) { return 0; }")
assert js != replaced
open(filename, 'w').write(replaced)
  ''')
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='2', args=[opts, '--shell-file', 'shell.html', '--js-transform', fixer]) # no fround anyhow
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='121378', args=[opts, '--shell-file', 'shell.html', '--js-transform', fixer, '-s', 'PRECISE_F32=1']) # proper polyfill was enstated, then it was replaced by the fix so 0 is returned all the time, hence a different result here
      self.btest(path_from_root('tests', 'codemods.cpp'), expected='2', args=[opts, '--shell-file', 'shell.html', '--js-transform', fixer, '-s', 'PRECISE_F32=2', '--separate-asm']) # we should remove the calls to the polyfill ENTIRELY here, on the clientside, so we should NOT see any calls to fround here, and result should be like double

  def test_wget(self):
    with open(os.path.join(self.get_dir(), 'test.txt'), 'w') as f:
      f.write('emscripten')
    self.btest(path_from_root('tests', 'test_wget.c'), expected='1', args=['-s', 'ASYNCIFY=1'])
    print 'asyncify+emterpreter'
    self.btest(path_from_root('tests', 'test_wget.c'), expected='1', args=['-s', 'ASYNCIFY=1', '-s', 'EMTERPRETIFY=1'])

  def test_wget_data(self):
    with open(os.path.join(self.get_dir(), 'test.txt'), 'w') as f:
      f.write('emscripten')
    self.btest(path_from_root('tests', 'test_wget_data.c'), expected='1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O2', '-g2'])
    self.btest(path_from_root('tests', 'test_wget_data.c'), expected='1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O2', '-g2', '-s', 'ASSERTIONS=1'])

  def test_locate_file(self):
    self.clear()
    open('src.cpp', 'w').write(self.with_report_result(r'''
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
        REPORT_RESULT();
        return 0;
      }
    '''))
    open('data.txt', 'w').write('load me right before...')
    open('pre.js', 'w').write('Module.locateFile = function(x) { return "sub/" + x };')
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'data.txt'], stdout=open('data.js', 'w')).communicate()
    # put pre.js first, then the file packager data, so locateFile is there for the file loading code
    Popen([PYTHON, EMCC, 'src.cpp', '-O2', '-g', '--pre-js', 'pre.js', '--pre-js', 'data.js', '-o', 'page.html']).communicate()
    os.mkdir('sub')
    shutil.move('page.html.mem', os.path.join('sub', 'page.html.mem'))
    shutil.move('test.data', os.path.join('sub', 'test.data'))
    self.run_browser('page.html', None, '/report_result?1')

    # alternatively, put locateFile in the HTML
    print 'in html'

    open('shell.html', 'w').write('''
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
      Popen([PYTHON, EMCC, 'src.cpp', '-O2', '-g', '--shell-file', 'shell.html', '--pre-js', 'data.js', '-o', 'page.html'] + args).communicate()
      self.run_browser('page.html', None, '/report_result?' + expected)

    in_html('1')

    # verify that the mem init request succeeded in the latter case
    open('src.cpp', 'w').write(self.with_report_result(r'''
#include<stdio.h>
#include<emscripten.h>

int main() {
  int result = EM_ASM_INT_V({
    return Module['memoryInitializerRequest'].status;
  });
  printf("memory init request: %d\n", result);
  REPORT_RESULT();
  return 0;
}
    '''))

    in_html('200', ['-s', 'FORCE_FILESYSTEM=1'])

  def test_glfw3(self):
    self.btest(path_from_root('tests', 'glfw3.c'), args=['-s', 'LEGACY_GL_EMULATION=1', '-s', 'USE_GLFW=3'], expected='1')

  def test_glfw_events(self):
    self.btest(path_from_root('tests', 'glfw_events.c'), args=['-s', 'USE_GLFW=2', "-DUSE_GLFW=2"], expected='1')
    self.btest(path_from_root('tests', 'glfw_events.c'), args=['-s', 'USE_GLFW=3', "-DUSE_GLFW=3"], expected='1')

  def test_asm_swapping(self):
    self.clear()
    open('run.js', 'w').write(r'''
Module['_main'] = function() {
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
      print opts
      opts += ['-s', 'NO_EXIT_RUNTIME=1', '--pre-js', 'run.js', '-s', 'SWAPPABLE_ASM_MODULE=1'] # important that both modules are built with the same opts
      open('second.cpp', 'w').write(self.with_report_result(open(path_from_root('tests', 'asm_swap2.cpp')).read()))
      Popen([PYTHON, EMCC, 'second.cpp'] + opts).communicate()
      Popen([PYTHON, path_from_root('tools', 'distill_asm.py'), 'a.out.js', 'second.js', 'swap-in']).communicate()
      assert os.path.exists('second.js')

      if isinstance(SPIDERMONKEY_ENGINE, list) and len(SPIDERMONKEY_ENGINE[0]) != 0:
        out = run_js('second.js', engine=SPIDERMONKEY_ENGINE, stderr=PIPE, full_output=True, assert_returncode=None)
        self.validate_asmjs(out)
      else:
        print 'Skipping asm validation check, spidermonkey is not configured'

      self.btest(path_from_root('tests', 'asm_swap.cpp'), args=opts, expected='999')

  def test_sdl2_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file, and memory-init
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpg'))
    open(os.path.join(self.get_dir(), 'sdl2_image.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_image.c')).read()))

    for mem in [0, 1]:
      for dest, dirname, basename in [('screenshot.jpg',                        '/',       'screenshot.jpg'),
                                      ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
        Popen([
          PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_image.c'), '-o', 'page.html', '-O2', '--memory-init-file', str(mem),
          '--preload-file', dest, '-DSCREENSHOT_DIRNAME="' + dirname + '"', '-DSCREENSHOT_BASENAME="' + basename + '"', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--use-preload-plugins'
        ]).communicate()
        self.run_browser('page.html', '', '/report_result?600')

  def test_sdl2_image_jpeg(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpeg'))
    open(os.path.join(self.get_dir(), 'sdl2_image_jpeg.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_image.c')).read()))
    Popen([
      PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_image_jpeg.c'), '-o', 'page.html',
      '--preload-file', 'screenshot.jpeg', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '--use-preload-plugins'
    ]).communicate()
    self.run_browser('page.html', '', '/report_result?600')

  def test_sdl2_image_formats(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl2_image.c', expected='512', args=['--preload-file', 'screenshot.png', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.png"',
                                                     '-DNO_PRELOADED','-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '-s', 'SDL2_IMAGE_FORMATS=["png"]'])

  def test_sdl2_key(self):
    for defines in [[]]:
      open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
        Module.postRun = function() {
          function doOne() {
            Module._one();
            setTimeout(doOne, 1000/60);
          }
          setTimeout(doOne, 1000/60);
        }

        function keydown(c) {
          var event = document.createEvent("KeyboardEvent");
          event.initKeyEvent("keydown", true, true, window,
                             0, 0, 0, 0,
                             c, c);
          var prevented = !document.dispatchEvent(event);

          //send keypress if not prevented
          if (!prevented) {
            event = document.createEvent("KeyboardEvent");
            event.initKeyEvent("keypress", true, true, window,
                               0, 0, 0, 0, 0, c);
            document.dispatchEvent(event);
          }
        }

        function keyup(c) {
          var event = document.createEvent("KeyboardEvent");
          event.initKeyEvent("keyup", true, true, window,
                             0, 0, 0, 0,
                             c, c);
          document.dispatchEvent(event);
        }
      ''')
      open(os.path.join(self.get_dir(), 'sdl2_key.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_key.c')).read()))

      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_key.c'), '-o', 'page.html'] + defines + ['-s', 'USE_SDL=2','--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''', '-s', 'NO_EXIT_RUNTIME=1']).communicate()
      self.run_browser('page.html', '', '/report_result?37182145')

  def test_sdl2_text(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      Module.postRun = function() {
        function doOne() {
          Module._one();
          setTimeout(doOne, 1000/60);
        }
        setTimeout(doOne, 1000/60);
      }

      function simulateKeyEvent(charCode) {
        var event = document.createEvent("KeyboardEvent");
        event.initKeyEvent("keypress", true, true, window,
                           0, 0, 0, 0, 0, charCode);
        document.body.dispatchEvent(event);
      }
    ''')
    open(os.path.join(self.get_dir(), 'sdl2_text.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_text.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_text.c'), '-o', 'page.html', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''', '-s', 'USE_SDL=2']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl2_mouse(self):
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
    open(os.path.join(self.get_dir(), 'sdl2_mouse.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_mouse.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_mouse.c'), '-O2', '--minify', '0', '-o', 'page.html', '--pre-js', 'pre.js', '-s', 'USE_SDL=2']).communicate()
    self.run_browser('page.html', '', '/report_result?712', timeout=30)

  def test_sdl2_mouse_offsets(self):
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'page.html'), 'w').write('''
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
    open(os.path.join(self.get_dir(), 'sdl2_mouse.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_mouse.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_mouse.c'), '-O2', '--minify', '0', '-o', 'sdl2_mouse.js', '--pre-js', 'pre.js', '-s', 'USE_SDL=2']).communicate()
    self.run_browser('page.html', '', '/report_result?572')

  def test_sdl2glshader(self):
    self.btest('sdl2glshader.c', reference='sdlglshader.png', args=['-s', 'USE_SDL=2', '-O2', '--closure', '1', '-s', 'LEGACY_GL_EMULATION=1'])
    self.btest('sdl2glshader.c', reference='sdlglshader.png', args=['-s', 'USE_SDL=2', '-O2', '-s', 'LEGACY_GL_EMULATION=1'], also_proxied=True) # XXX closure fails on proxy

  def test_sdl2_canvas_blank(self):
    self.btest('sdl2_canvas_blank.c', reference='sdl_canvas_blank.png', args=['-s', 'USE_SDL=2'])

  def test_sdl2_canvas_palette(self):
    self.btest('sdl2_canvas_palette.c', reference='sdl_canvas_palette.png', args=['-s', 'USE_SDL=2'])

  def test_sdl2_canvas_twice(self):
    self.btest('sdl2_canvas_twice.c', reference='sdl_canvas_twice.png', args=['-s', 'USE_SDL=2'])

  def zzztest_sdl2_gfx_primitives(self):
    self.btest('sdl2_gfx_primitives.c', args=['-s', 'USE_SDL=2', '-lSDL2_gfx'], reference='sdl_gfx_primitives.png', reference_slack=1)

  def test_sdl2_canvas_palette_2(self):
    open(os.path.join(self.get_dir(), 'args-r.js'), 'w').write('''
      Module['arguments'] = ['-r'];
    ''')

    open(os.path.join(self.get_dir(), 'args-g.js'), 'w').write('''
      Module['arguments'] = ['-g'];
    ''')

    open(os.path.join(self.get_dir(), 'args-b.js'), 'w').write('''
      Module['arguments'] = ['-b'];
    ''')

    self.btest('sdl2_canvas_palette_2.c', reference='sdl_canvas_palette_r.png', args=['-s', 'USE_SDL=2', '--pre-js', 'args-r.js'])
    self.btest('sdl2_canvas_palette_2.c', reference='sdl_canvas_palette_g.png', args=['-s', 'USE_SDL=2', '--pre-js', 'args-g.js'])
    self.btest('sdl2_canvas_palette_2.c', reference='sdl_canvas_palette_b.png', args=['-s', 'USE_SDL=2', '--pre-js', 'args-b.js'])

  def test_sdl2_swsurface(self):
    self.btest('sdl2_swsurface.c', expected='1', args=['-s', 'USE_SDL=2'])

  def test_sdl2_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl2_image_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not', '-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'])

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
      open('test.html', 'w').write(html)

    open('data.txt', 'w').write('datum')

    self.btest('sdl2_canvas_proxy.c', reference='sdl2_canvas.png', args=['-s', 'USE_SDL=2', '--proxy-to-worker', '--preload-file', 'data.txt', '-s', 'GL_TESTING=1'], manual_reference=True, post_build=post)

  def test_sdl2_pumpevents(self):
    # key events should be detected using SDL_PumpEvents
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      function keydown(c) {
        var event = document.createEvent("KeyboardEvent");
        event.initKeyEvent("keydown", true, true, window,
                           0, 0, 0, 0,
                           c, c);
        document.dispatchEvent(event);
      }
    ''')
    self.btest('sdl2_pumpevents.c', expected='7', args=['--pre-js', 'pre.js', '-s', 'USE_SDL=2'])

  def test_sdl2_timer(self):
    self.btest('sdl2_timer.c', expected='5', args=['-s', 'USE_SDL=2'])

  def test_sdl2_canvas_size(self):
    self.btest('sdl2_canvas_size.c', expected='1', args=['-s', 'USE_SDL=2'])

  def test_sdl2_gl_read(self):
    # SDL, OpenGL, readPixels
    open(os.path.join(self.get_dir(), 'sdl2_gl_read.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_gl_read.c')).read()))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl2_gl_read.c'), '-o', 'something.html', '-s', 'USE_SDL=2']).communicate()
    self.run_browser('something.html', '.', '/report_result?1')

  def test_sdl2_fog_simple(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl2_fog_simple.c', reference='screenshot-fog-simple.png',
      args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2','-O2', '--minify', '0', '--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl2_fog_negative(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl2_fog_negative.c', reference='screenshot-fog-negative.png',
      args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2','--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl2_fog_density(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl2_fog_density.c', reference='screenshot-fog-density.png',
      args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2','--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl2_fog_exp2(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl2_fog_exp2.c', reference='screenshot-fog-exp2.png',
      args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2','--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl2_fog_linear(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl2_fog_linear.c', reference='screenshot-fog-linear.png', reference_slack=1,
      args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2','--preload-file', 'screenshot.png', '-s', 'LEGACY_GL_EMULATION=1', '--use-preload-plugins'],
      message='You should see an image with fog.')

  def test_sdl2_unwasteful(self):
    self.btest('sdl2_unwasteful.cpp', expected='1', args=['-s', 'USE_SDL=2', '-O1'])

  def test_sdl2_canvas_write(self):
    self.btest('sdl2_canvas_write.cpp', expected='0', args=['-s', 'USE_SDL=2'])

  def test_sdl2_gl_frames_swap(self):
    def post_build(*args):
      self.post_manual_reftest(*args)
      html = open('test.html').read()
      html2 = html.replace('''Module['postRun'] = doReftest;''', '') # we don't want the very first frame
      assert html != html2
      open('test.html', 'w').write(html2)
    self.btest('sdl2_gl_frames_swap.c', reference='sdl2_gl_frames_swap.png', args=['--proxy-to-worker', '-s', 'GL_TESTING=1', '-s', 'USE_SDL=2'], manual_reference=True, post_build=post_build)

  def test_sdl2_ttf(self):
    shutil.copy2(path_from_root('tests', 'freetype', 'LiberationSansBold.ttf'), self.get_dir())
    self.btest('sdl2_ttf.c', reference='sdl2_ttf.png',
      args=['-O2', '-s', 'USE_SDL=2', '-s', 'USE_SDL_TTF=2', '--embed-file', 'LiberationSansBold.ttf'],
      message='You should see colorful "hello" and "world" in the window',
      timeout=30)

  def test_emterpreter_async(self):
    for opts in [0, 1, 2, 3]:
      print opts
      self.btest('emterpreter_async.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-g2'])

  def test_emterpreter_async_2(self):
    self.btest('emterpreter_async_2.cpp', '40', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O3'])

  def test_emterpreter_async_virtual(self):
    for opts in [0, 1, 2, 3]:
      print opts
      self.btest('emterpreter_async_virtual.cpp', '5', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-profiling'])

  def test_emterpreter_async_virtual_2(self):
    for opts in [0, 1, 2, 3]:
      print opts
      self.btest('emterpreter_async_virtual_2.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'ASSERTIONS=1', '-s', 'SAFE_HEAP=1', '-profiling'])

  def test_emterpreter_async_bad(self):
    for opts in [0, 1, 2, 3]:
      print opts
      self.btest('emterpreter_async_bad.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'EMTERPRETIFY_BLACKLIST=["_middle"]', '-s', 'ASSERTIONS=1'])

  def test_emterpreter_async_mainloop(self):
    for opts in [0, 1, 2, 3]:
      print opts
      self.btest('emterpreter_async_mainloop.cpp', '121', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts)], timeout=20)

  def test_emterpreter_async_with_manual(self):
    for opts in [0, 1, 2, 3]:
      print opts
      self.btest('emterpreter_async_with_manual.cpp', '121', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-O' + str(opts), '-s', 'EMTERPRETIFY_BLACKLIST=["_acall"]'], timeout=20)

  def test_emterpreter_async_sleep2(self):
    self.btest('emterpreter_async_sleep2.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Oz'])

  def test_emterpreter_async_sleep2_safeheap(self):
    # check that safe-heap machinery does not cause errors in async operations
    self.btest('emterpreter_async_sleep2_safeheap.cpp', '17', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Oz', '-profiling', '-s', 'SAFE_HEAP=1', '-s', 'ASSERTIONS=1', '-s', 'EMTERPRETIFY_WHITELIST=["_main","_callback","_fix"]'])

  def test_sdl_audio_beep_sleep(self):
    self.btest('sdl_audio_beep_sleep.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Os', '-s', 'ASSERTIONS=1', '-s', 'DISABLE_EXCEPTION_CATCHING=0', '-profiling', '-s', 'SAFE_HEAP=1'], timeout=60)

  def test_mainloop_reschedule(self):
    self.btest('mainloop_reschedule.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-Os'], timeout=30)

  def test_mainloop_infloop(self):
    self.btest('mainloop_infloop.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'], timeout=30)

  def test_emterpreter_async_iostream(self):
    self.btest('emterpreter_async_iostream.cpp', '1', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1'])

  def test_modularize(self):
    for opts in [[], ['-O1'], ['-O2', '-profiling'], ['-O2']]:
      for args, code in [
        ([], 'Module();'), # defaults
        (['-s', 'EXPORT_NAME="HelloWorld"'], '''
          if (typeof Module !== "undefined") throw "what?!"; // do not pollute the global scope, we are modularized!
          HelloWorld();
        '''), # use EXPORT_NAME
        (['-s', 'EXPORT_NAME="HelloWorld"'], '''
          var hello = HelloWorld({ noInitialRun: true, onRuntimeInitialized: function() {
            setTimeout(function() { hello._main(); }); // must be async, because onRuntimeInitialized may be called synchronously, so |hello| is not yet set!
          } });
        '''), # pass in a Module option (which prevents main(), which we then invoke ourselves)
        (['-s', 'EXPORT_NAME="HelloWorld"', '--memory-init-file', '0'], '''
          var hello = HelloWorld({ noInitialRun: true});
          hello._main();
        '''), # similar, but without a mem init file, everything is sync and simple
      ]:
        print 'test on', opts, args, code
        src = open(path_from_root('tests', 'browser_test_hello_world.c')).read()
        open('test.c', 'w').write(self.with_report_result(src))
        Popen([PYTHON, EMCC, 'test.c', '-s', 'MODULARIZE=1'] + args + opts).communicate()
        open('a.html', 'w').write('''
          <script src="a.out.js"></script>
          <script>
            %s
          </script>
        ''' % code)
        self.run_browser('a.html', '...', '/report_result?0')

  def test_webidl(self):
    # see original in test_core.py
    output = Popen([PYTHON, path_from_root('tools', 'webidl_binder.py'),
                            path_from_root('tests', 'webidl', 'test.idl'),
                            'glue']).communicate()[0]
    assert os.path.exists('glue.cpp')
    assert os.path.exists('glue.js')
    self.btest(os.path.join('webidl', 'test.cpp'), '1', args=['--post-js', 'glue.js', '-I' + path_from_root('tests', 'webidl'), '-DBROWSER'])

  def test_dynamic_link(self):
    open('pre.js', 'w').write('''
      Module.dynamicLibraries = ['side.js'];
  ''')
    open('main.cpp', 'w').write(r'''
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
          Module.realPrint = Module.print;
          Module.print = function(x) {
            if (!Module.printed) Module.printed = x;
            Module.realPrint(x);
          };
        });
        puts(ret);
        EM_ASM({ assert(Module.printed === 'hello through side', ['expected', Module.printed]); });
        int result = 2;
        REPORT_RESULT();
        return 0;
      }
    ''')
    open('side.cpp', 'w').write(r'''
      #include <stdlib.h>
      #include <string.h>
      char *side(const char *data);
      char *side(const char *data) {
        char *ret = (char*)malloc(strlen(data)+1);
        strcpy(ret, data);
        return ret;
      }
    ''')
    Popen([PYTHON, EMCC, 'side.cpp', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'side.js']).communicate()

    self.btest(self.in_dir('main.cpp'), '2', args=['-s', 'MAIN_MODULE=1', '-O2', '--pre-js', 'pre.js'])

  def test_dynamic_link_glemu(self):
    open('pre.js', 'w').write('''
      Module.dynamicLibraries = ['side.js'];
  ''')
    open('main.cpp', 'w').write(r'''
      #include <stdio.h>
      #include <string.h>
      #include <assert.h>
      const char *side();
      int main() {
        const char *exts = side();
        puts(side());
        assert(strstr(exts, "GL_EXT_texture_env_combine"));
        int result = 1;
        REPORT_RESULT();
        return 0;
      }
    ''')
    open('side.cpp', 'w').write(r'''
      #include "SDL/SDL.h"
      #include "SDL/SDL_opengl.h"
      const char *side() {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_SetVideoMode(600, 600, 16, SDL_OPENGL);
        return (const char *)glGetString(GL_EXTENSIONS);
      }
    ''')
    Popen([PYTHON, EMCC, 'side.cpp', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'side.js']).communicate()

    self.btest(self.in_dir('main.cpp'), '1', args=['-s', 'MAIN_MODULE=1', '-O2', '-s', 'LEGACY_GL_EMULATION=1', '--pre-js', 'pre.js'])

  def test_memory_growth_during_startup(self):
    open('data.dat', 'w').write('X' * (30*1024*1024))
    self.btest('browser_test_hello_world.c', '0', args=['-s', 'ASSERTIONS=1', '-s', 'ALLOW_MEMORY_GROWTH=1', '-s', 'TOTAL_MEMORY=10000', '-s', 'TOTAL_STACK=5000', '--preload-file', 'data.dat'])

  # pthreads tests

  def prep_no_SAB(self):
    open('html.html', 'w').write(open(path_from_root('src', 'shell_minimal.html')).read().replace('''<body>''', '''<body>
      <script>
        SharedArrayBuffer = undefined;
        Atomics = undefined;
      </script>
    '''))

  # Test that the emscripten_ atomics api functions work.
  def test_pthread_atomics(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_atomics.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=120) # extra time on first test, to be sure to build all libraries

  # Test 64-bit atomics.
  def test_pthread_64bit_atomics(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_64bit_atomics.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test 64-bit C++11 atomics.
  def test_pthread_64bit_cxx11_atomics(self):
    for opt in [['-O0'], ['-O3']]:
      for pthreads in [[], ['-s', 'USE_PTHREADS=1']]:
        self.btest(path_from_root('tests', 'pthread', 'test_pthread_64bit_cxx11_atomics.cpp'), expected='0', args=opt + pthreads + ['-std=c++11'], timeout=30)

  # Test the old GCC atomic __sync_fetch_and_op builtin operations.
  def test_pthread_gcc_atomic_fetch_and_op(self):
    # We need to resort to using regexes to optimize out SharedArrayBuffer when pthreads are not supported, which is brittle!
    # Therefore perform very extensive testing of different codegen modes to catch any problems.
    for opt in [[], ['-O1'], ['-O2'], ['-O3'], ['-O3', '-s', 'AGGRESSIVE_VARIABLE_ELIMINATION=1'], ['-Os'], ['-Oz']]:
      for debug in [[], ['-g1'], ['-g2'], ['-g4']]:
        for f32 in [[], ['-s', 'PRECISE_F32=1']]:
          print opt, debug, f32
          self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_atomic_fetch_and_op.cpp'), expected='0', args=opt+debug+f32+['-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=60)

  # 64 bit version of the above test.
  def test_pthread_gcc_64bit_atomic_fetch_and_op(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_64bit_atomic_fetch_and_op.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test the old GCC atomic __sync_op_and_fetch builtin operations.
  def test_pthread_gcc_atomic_op_and_fetch(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_atomic_op_and_fetch.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # 64 bit version of the above test.
  def test_pthread_gcc_64bit_atomic_op_and_fetch(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_64bit_atomic_op_and_fetch.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Tests the rest of the remaining GCC atomics after the two above tests.
  def test_pthread_gcc_atomics(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_atomics.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test the __sync_lock_test_and_set and __sync_lock_release primitives.
  def test_pthread_gcc_spinlock(self):
    for arg in [[], ['-DUSE_EMSCRIPTEN_INTRINSICS']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_gcc_spinlock.cpp'), expected='800', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'] + arg, timeout=30)

  # Test that basic thread creation works.
  def test_pthread_create(self):
    for opt in [['-O0'], ['-O3']]:
      for pthreads in [['-s', 'USE_PTHREADS=1'], ['-s', 'USE_PTHREADS=2', '--separate-asm']]:
        print str(opt) + ' ' + str(pthreads)
        self.btest(path_from_root('tests', 'pthread', 'test_pthread_create.cpp'), expected='0', args=opt + pthreads + ['-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

        if 'USE_PTHREADS=2' in pthreads:
          self.prep_no_SAB()
          self.btest(path_from_root('tests', 'pthread', 'test_pthread_create.cpp'), expected='0', args=opt + pthreads + ['-s', 'PTHREAD_POOL_SIZE=8', '--shell-file', 'html.html'], timeout=30)

  # Test that a pthread can spawn another pthread of its own.
  def test_pthread_create_pthread(self):
    for opt in [['-s', 'USE_PTHREADS=2', '--separate-asm'], ['-s', 'USE_PTHREADS=1', '--proxy-to-worker']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_create_pthread.cpp'), expected='1', args=opt + ['-O3', '-s', 'PTHREAD_POOL_SIZE=2', '-s', 'NO_EXIT_RUNTIME=1'], timeout=30)

  # Test another case of pthreads spawning pthreads, but this time the callers immediately join on the threads they created.
  def test_pthread_nested_spawns(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_nested_spawns.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=2'], timeout=30)

  # Test that main thread can wait for a pthread to finish via pthread_join().
  def test_pthread_join(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_join.cpp'), expected='6765', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test pthread_cancel() operation
  def test_pthread_cancel(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_cancel.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test pthread_kill() operation
  def test_pthread_kill(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_kill.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test that pthread cleanup stack (pthread_cleanup_push/_pop) works.
  def test_pthread_cleanup(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_cleanup.cpp'), expected='907640832', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Tests the pthread mutex api.
  def test_pthread_mutex(self):
    for arg in [[], ['-DSPINLOCK_TEST']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_mutex.cpp'), expected='50', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'] + arg, timeout=20)

  # Test that memory allocation is thread-safe.
  def test_pthread_malloc(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_malloc.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Stress test pthreads allocating memory that will call to sbrk(), and main thread has to free up the data.
  def test_pthread_malloc_free(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_malloc_free.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8', '-s', 'TOTAL_MEMORY=268435456'], timeout=30)

  # Test that the pthread_barrier API works ok.
  def test_pthread_barrier(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_barrier.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test the pthread_once() function.
  def test_pthread_once(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_once.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test against a certain thread exit time handling bug by spawning tons of threads.
  def test_pthread_spawns(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_spawns.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # It is common for code to flip volatile global vars for thread control. This is a bit lax, but nevertheless, test whether that
  # kind of scheme will work with Emscripten as well.
  def test_pthread_volatile(self):
    for arg in [[], ['-DUSE_C_VOLATILE']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_volatile.cpp'), expected='1', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'] + arg, timeout=30)

  # Test thread-specific data (TLS).
  def test_pthread_thread_local_storage(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_thread_local_storage.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test the pthread condition variable creation and waiting.
  def test_pthread_condition_variable(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_condition_variable.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8'], timeout=30)

  # Test that pthreads are able to do printf.
  def test_pthread_printf(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_printf.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=1'], timeout=30)

  # Test that pthreads are able to do cout. Failed due to https://bugzilla.mozilla.org/show_bug.cgi?id=1154858.
  def test_pthread_iostream(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_iostream.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=1'], timeout=30)

  # Test that the main thread is able to use pthread_set/getspecific.
  def test_pthread_setspecific_mainthread(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_setspecific_mainthread.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm'], timeout=30)

    self.prep_no_SAB()
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_setspecific_mainthread.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '--shell-file', 'html.html'], timeout=30)

  # Test the -s PTHREAD_HINT_NUM_CORES=x command line variable.
  def test_pthread_num_logical_cores(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_num_logical_cores.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_HINT_NUM_CORES=2'], timeout=30)

    self.prep_no_SAB()
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_num_logical_cores.cpp'), expected='0', args=['-O3', '-g', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_HINT_NUM_CORES=2', '--shell-file', 'html.html'], timeout=30)

  # Test that pthreads have access to filesystem.
  def test_pthread_file_io(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_file_io.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=1'], timeout=30)

  # Test that the pthread_create() function operates benignly in the case that threading is not supported.
  def test_pthread_supported(self):
    for args in [[], ['-s', 'USE_PTHREADS=2', '--separate-asm', '-s', 'PTHREAD_POOL_SIZE=8']]:
      self.btest(path_from_root('tests', 'pthread', 'test_pthread_supported.cpp'), expected='0', args=['-O3'] + args, timeout=30)

  def test_pthread_separate_asm_pthreads(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_atomics.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=8', '--separate-asm', '--profiling'], timeout=30)

  def test_pthread_custom_pthread_main_url(self):
    self.clear()
    os.makedirs(os.path.join(self.get_dir(), 'cdn'));
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(self.with_report_result(r'''
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
        REPORT_RESULT();
      }
    '''))

    # Test that it is possible to define "Module.pthreadMainPrefixURL" string to locate where pthread-main.js will be loaded from.
    open(self.in_dir('shell.html'), 'w').write(open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { pthreadMainPrefixURL: "cdn/", '))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--shell-file', 'shell.html', '-s', 'IN_TEST_HARNESS=1', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1', '-o', 'test.html']).communicate()
    shutil.move('pthread-main.js', os.path.join('cdn', 'pthread-main.js'))
    self.run_browser('test.html', '', '/report_result?1')

    # Test that it is possible to define "Module.locateFile(foo)" function to locate where pthread-main.js will be loaded from.
    open(self.in_dir('shell2.html'), 'w').write(open(path_from_root('src', 'shell.html')).read().replace('var Module = {', 'var Module = { locateFile: function(filename) { if (filename == "pthread-main.js") return "cdn/pthread-main.js"; else return filename; }, '))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--shell-file', 'shell2.html', '-s', 'IN_TEST_HARNESS=1', '-s', 'USE_PTHREADS=1', '-s', 'PTHREAD_POOL_SIZE=1', '-o', 'test2.html']).communicate()
    try_delete('pthread-main.js')
    self.run_browser('test2.html', '', '/report_result?1')

  # Test that if the main thread is performing a futex wait while a pthread needs it to do a proxied operation (before that pthread would wake up the main thread), that it's not a deadlock.
  def test_pthread_proxying_in_futex_wait(self):
    self.btest(path_from_root('tests', 'pthread', 'test_pthread_proxying_in_futex_wait.cpp'), expected='0', args=['-O3', '-s', 'USE_PTHREADS=2', '-s', 'PTHREAD_POOL_SIZE=1', '--separate-asm'], timeout=30)

  # test atomicrmw i64
  def test_atomicrmw_i64(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'atomicrmw_i64.ll'), '-s', 'USE_PTHREADS=1', '-s', 'IN_TEST_HARNESS=1', '-o', 'test.html']).communicate()
    self.run_browser('test.html', None, '/report_result?0')

  # Test that it is possible to send a signal via calling alarm(timeout), which in turn calls to the signal handler set by signal(SIGALRM, func);
  def test_sigalrm(self):
    self.btest(path_from_root('tests', 'sigalrm.cpp'), expected='0', args=['-O3'], timeout=30)

  def test_meminit_pairs(self):
    d = 'const char *data[] = {\n  "'
    d += '",\n  "'.join(''.join('\\x{:02x}\\x{:02x}'.format(i, j)
                                for j in range(256)) for i in range(256))
    with open(path_from_root('tests', 'meminit_pairs.c')) as f:
      d += '"\n};\n' + f.read()
    args = ["-O2", "--memory-init-file", "0", "-s", "MEM_INIT_METHOD=2", "-s", "ASSERTIONS=1"]
    self.btest(d, expected='0', args=args + ["--closure", "0"])
    self.btest(d, expected='0', args=args + ["--closure", "0", "-g"])
    self.btest(d, expected='0', args=args + ["--closure", "1"])

  def test_meminit_big(self):
    d = 'const char *data[] = {\n  "'
    d += '",\n  "'.join([''.join('\\x{:02x}\\x{:02x}'.format(i, j)
                                 for j in range(256)) for i in range(256)]*256)
    with open(path_from_root('tests', 'meminit_pairs.c')) as f:
      d += '"\n};\n' + f.read()
    assert len(d) > (1 << 27) # more than 32M memory initializer
    args = ["-O2", "--memory-init-file", "0", "-s", "MEM_INIT_METHOD=2", "-s", "ASSERTIONS=1"]
    self.btest(d, expected='0', args=args + ["--closure", "0"])
    self.btest(d, expected='0', args=args + ["--closure", "0", "-g"])
    self.btest(d, expected='0', args=args + ["--closure", "1"])

  def test_canvas_style_proxy(self):
    self.btest('canvas_style_proxy.c', expected='1', args=['--proxy-to-worker', '--shell-file', path_from_root('tests/canvas_style_proxy_shell.html'), '--pre-js', path_from_root('tests/canvas_style_proxy_pre.js')])

  def test_canvas_size_proxy(self):
    self.btest(path_from_root('tests', 'canvas_size_proxy.c'), expected='0', args=['--proxy-to-worker'])

  def test_custom_messages_proxy(self):
    self.btest(path_from_root('tests', 'custom_messages_proxy.c'), expected='1', args=['--proxy-to-worker', '--shell-file', path_from_root('tests', 'custom_messages_proxy_shell.html'), '--post-js', path_from_root('tests', 'custom_messages_proxy_postjs.js')])

  def test_separate_asm(self):
    for opts in [['-O0'], ['-O1'], ['-O2'], ['-O2', '--closure', '1']]:
      print opts
      open('src.cpp', 'w').write(self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))
      Popen([PYTHON, EMCC, 'src.cpp', '-o', 'test.html'] + opts).communicate()
      self.run_browser('test.html', None, '/report_result?0')

      open('one.html', 'w').write('<script src="test.js"></script>')
      self.run_browser('one.html', None, '/report_result?0')

      Popen([PYTHON, path_from_root('tools', 'separate_asm.py'), 'test.js', 'asm.js', 'rest.js']).communicate()
      open('two.html', 'w').write('''
        <script>
          var Module = {};
        </script>
        <script src="asm.js"></script>
        <script src="rest.js"></script>
      ''')
      self.run_browser('two.html', None, '/report_result?0')

      self.clear()
      assert not os.path.exists('tests.asm.js')
      self.btest('browser_test_hello_world.c', expected='0', args=opts + ['--separate-asm'])
      assert os.path.exists('test.asm.js')
      os.unlink('test.asm.js')
      self.run_browser('test.html', None, '[no http server activity]', timeout=5) # fail without the asm

  def test_emterpretify_file(self):
    open('shell.html', 'w').write('''
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
    try_delete('code.dat');
    self.btest('browser_test_hello_world.c', expected='0', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_FILE="code.dat"', '-O2', '-g', '--shell-file', 'shell.html', '-s', 'ASSERTIONS=1'])
    assert os.path.exists('code.dat')

    try_delete('code.dat');
    self.btest('browser_test_hello_world.c', expected='0', args=['-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_FILE="code.dat"', '-O2', '-g', '-s', 'ASSERTIONS=1'])
    assert os.path.exists('code.dat')

  def test_vanilla_html_when_proxying(self):
    for opts in [0, 1, 2]:
      print opts
      open('src.cpp', 'w').write(self.with_report_result(open(path_from_root('tests', 'browser_test_hello_world.c')).read()))
      Popen([PYTHON, EMCC, 'src.cpp', '-o', 'test.js', '-O' + str(opts), '--proxy-to-worker']).communicate()
      open('test.html', 'w').write('<script src="test.js"></script>')
      self.run_browser('test.html', None, '/report_result?0')

  def test_in_flight_memfile_request(self):
    for o in [0, 1, 2]:
      print o
      opts = ['-O' + str(o)]

      print 'plain html'
      open('src.cpp', 'w').write(self.with_report_result(open(path_from_root('tests', 'in_flight_memfile_request.c')).read()))
      Popen([PYTHON, EMCC, 'src.cpp', '-o', 'test.js'] + opts).communicate()
      open('test.html', 'w').write('<script src="test.js"></script>')
      self.run_browser('test.html', None, '/report_result?0') # never when we provide our own HTML like this.

      print 'default html'
      self.btest('in_flight_memfile_request.c', expected='0' if o < 2 else '1', args=opts) # should happen when there is a mem init file (-O2+)

  def test_split_memory_large_file(self):
    size = 2*1024*1024
    open('huge.dat', 'w').write(''.join([chr((x*x)&255) for x in range(size*2)])) # larger than a memory chunk
    self.btest('split_memory_large_file.cpp', expected='1', args=['-s', 'SPLIT_MEMORY=' + str(size), '-s', 'TOTAL_MEMORY=100000000', '-s', 'TOTAL_STACK=10240', '--preload-file', 'huge.dat'], timeout=60)

  def test_binaryen(self):
    self.btest('browser_test_hello_world.c', expected='0', args=['-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
    self.btest('browser_test_hello_world.c', expected='0', args=['-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"', '-O2'])

