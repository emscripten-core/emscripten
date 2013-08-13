import BaseHTTPServer, multiprocessing, os, shutil, subprocess, unittest
from runner import BrowserCore, path_from_root
from tools.shared import *

''' Enable this code to run in another browser than webbrowser detects as default
def run_in_other_browser(url):
  execute(['yourbrowser', url])
webbrowser.open_new = run_in_other_browser
'''

class browser(BrowserCore):
  @staticmethod
  def audio():
    print
    print 'Running the browser audio tests. Make sure to listen to hear the correct results!'
    print
    audio_test_cases = [
      'test_sdl_audio',
      'test_sdl_audio_mix_channels',
      'test_sdl_audio_mix',
      'test_sdl_audio_quickload',
      'test_openal_playback',
      'test_openal_buffers',
      'test_freealut'
    ]
    return unittest.TestSuite(map(browser, audio_test_cases))

  @classmethod
  def setUpClass(self):
    super(browser, self).setUpClass()
    print
    print 'Running the browser tests. Make sure the browser allows popups from localhost.'
    print

  def test_html(self):
    # test HTML generation.
    self.btest('hello_world_sdl.cpp', reference='htmltest.png',
        message='You should see "hello, world!" and a colored cube.')

  def test_html_source_map(self):
    if 'test_html_source_map' not in str(sys.argv): return self.skip('''This test
requires manual intervention; will not be run unless explicitly requested''')
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
    Popen([PYTHON, EMCC, 'src.cpp', '-o', 'src.html', '-g4'],
        cwd=self.get_dir()).communicate()
    webbrowser.open_new('file://' + html_file)
    print '''
Set the debugger to pause on exceptions
You should see an exception thrown at src.cpp:7.
Press any key to continue.'''
    raw_input()

  def build_native_lzma(self):
    lzma_native = path_from_root('third_party', 'lzma.js', 'lzma-native')
    if os.path.isfile(lzma_native) and os.access(lzma_native, os.X_OK): return

    cwd = os.getcwd()
    try:
      os.chdir(path_from_root('third_party', 'lzma.js'))
      Popen(['sh', './doit.sh']).communicate()
    finally:
      os.chdir(cwd)

  def test_split(self):
    # test HTML generation.
    self.reftest(path_from_root('tests', 'htmltest.png'))
    output = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_sdl.cpp'), '-o', 'something.js', '--split', '100', '--pre-js', 'reftest.js']).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'something.js')), 'must be main js file'
    assert os.path.exists(os.path.join(self.get_dir(), 'something_functions.js')), 'must be functions js file'
    assert os.path.exists(os.path.join(self.get_dir(), 'something.include.html')), 'must be js include file'

    open(os.path.join(self.get_dir(), 'something.html'), 'w').write('''

    <!doctype html>
    <html lang="en-us">
      <head>
        <meta charset="utf-8">
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
        <title>Emscripten-Generated Code</title>
        <style>
          .emscripten { padding-right: 0; margin-left: auto; margin-right: auto; display: block; }
          canvas.emscripten { border: 1px solid black; }
          textarea.emscripten { font-family: monospace; width: 80%; }
          div.emscripten { text-align: center; }
        </style>
      </head>
      <body>
        <hr/>
        <div class="emscripten" id="status">Downloading...</div>
        <div class="emscripten">
          <progress value="0" max="100" id="progress" hidden=1></progress>
        </div>
        <canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()"></canvas>
        <hr/>
        <div class="emscripten"><input type="button" value="fullscreen" onclick="Module.requestFullScreen()"></div>
        <hr/>
        <textarea class="emscripten" id="output" rows="8"></textarea>
        <hr>
        <script type='text/javascript'>
          // connect to canvas
          var Module = {
            preRun: [],
            postRun: [],
            print: (function() {
              var element = document.getElementById('output');
              element.value = ''; // clear browser cache
              return function(text) {
                // These replacements are necessary if you render to raw HTML
                //text = text.replace(/&/g, "&amp;");
                //text = text.replace(/</g, "&lt;");
                //text = text.replace(/>/g, "&gt;");
                //text = text.replace('\\n', '<br>', 'g');
                element.value += text + "\\n";
                element.scrollTop = 99999; // focus on bottom
              };
            })(),
            printErr: function(text) {
              if (0) { // XXX disabled for safety typeof dump == 'function') {
                dump(text + '\\n'); // fast, straight to the real console
              } else {
                console.log(text);
              }
            },
            canvas: document.getElementById('canvas'),
            setStatus: function(text) {
              if (Module.setStatus.interval) clearInterval(Module.setStatus.interval);
              var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
              var statusElement = document.getElementById('status');
              var progressElement = document.getElementById('progress');
              if (m) {
                text = m[1];
                progressElement.value = parseInt(m[2])*100;
                progressElement.max = parseInt(m[4])*100;
                progressElement.hidden = false;
              } else {
                progressElement.value = null;
                progressElement.max = null;
                progressElement.hidden = true;
              }
              statusElement.innerHTML = text;
            },
            totalDependencies: 0,
            monitorRunDependencies: function(left) {
              this.totalDependencies = Math.max(this.totalDependencies, left);
              Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
            }
          };
          Module.setStatus('Downloading...');
        </script>''' + open(os.path.join(self.get_dir(), 'something.include.html')).read() + '''
      </body>
    </html>
    ''')

    self.run_browser('something.html', 'You should see "hello, world!" and a colored cube.', '/report_result?0')

  def test_split_in_source_filenames(self):
    self.reftest(path_from_root('tests', 'htmltest.png'))
    output = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_sdl.cpp'), '-o', 'something.js', '-g', '--split', '100', '--pre-js', 'reftest.js']).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'something.js')), 'must be main js file'
    assert os.path.exists(self.get_dir() + '/something/' + path_from_root('tests', 'hello_world_sdl.cpp.js')), 'must be functions js file'
    assert os.path.exists(os.path.join(self.get_dir(), 'something.include.html')), 'must be js include file'

    open(os.path.join(self.get_dir(), 'something.html'), 'w').write('''

    <!doctype html>
    <html lang="en-us">
      <head>
        <meta charset="utf-8">
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
        <title>Emscripten-Generated Code</title>
        <style>
          .emscripten { padding-right: 0; margin-left: auto; margin-right: auto; display: block; }
          canvas.emscripten { border: 1px solid black; }
          textarea.emscripten { font-family: monospace; width: 80%; }
          div.emscripten { text-align: center; }
        </style>
      </head>
      <body>
        <hr/>
        <div class="emscripten" id="status">Downloading...</div>
        <div class="emscripten">
          <progress value="0" max="100" id="progress" hidden=1></progress>
        </div>
        <canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()"></canvas>
        <hr/>
        <div class="emscripten"><input type="button" value="fullscreen" onclick="Module.requestFullScreen()"></div>
        <hr/>
        <textarea class="emscripten" id="output" rows="8"></textarea>
        <hr>
        <script type='text/javascript'>
          // connect to canvas
          var Module = {
            preRun: [],
            postRun: [],
            print: (function() {
              var element = document.getElementById('output');
              element.value = ''; // clear browser cache
              return function(text) {
                // These replacements are necessary if you render to raw HTML
                //text = text.replace(/&/g, "&amp;");
                //text = text.replace(/</g, "&lt;");
                //text = text.replace(/>/g, "&gt;");
                //text = text.replace('\\n', '<br>', 'g');
                element.value += text + "\\n";
                element.scrollTop = 99999; // focus on bottom
              };
            })(),
            printErr: function(text) {
              if (0) { // XXX disabled for safety typeof dump == 'function') {
                dump(text + '\\n'); // fast, straight to the real console
              } else {
                console.log(text);
              }
            },
            canvas: document.getElementById('canvas'),
            setStatus: function(text) {
              if (Module.setStatus.interval) clearInterval(Module.setStatus.interval);
              var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
              var statusElement = document.getElementById('status');
              var progressElement = document.getElementById('progress');
              if (m) {
                text = m[1];
                progressElement.value = parseInt(m[2])*100;
                progressElement.max = parseInt(m[4])*100;
                progressElement.hidden = false;
              } else {
                progressElement.value = null;
                progressElement.max = null;
                progressElement.hidden = true;
              }
              statusElement.innerHTML = text;
            },
            totalDependencies: 0,
            monitorRunDependencies: function(left) {
              this.totalDependencies = Math.max(this.totalDependencies, left);
              Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
            }
          };
          Module.setStatus('Downloading...');
        </script>''' + open(os.path.join(self.get_dir(), 'something.include.html')).read() + '''
      </body>
    </html>
    ''')

    self.run_browser('something.html', 'You should see "hello, world!" and a colored cube.', '/report_result?0')

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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-o', 'page.html',
           '--compression', '%s,%s,%s' % (path_from_root('third_party', 'lzma.js', 'lzma-native'),
                                          path_from_root('third_party', 'lzma.js', 'lzma-decoder.js'),
                                          'LZMA.decompress')]).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'page.js')), 'must be side js'
    assert os.path.exists(os.path.join(self.get_dir(), 'page.js.compress')), 'must be side compressed js'
    assert os.stat(os.path.join(self.get_dir(), 'page.js')).st_size > os.stat(os.path.join(self.get_dir(), 'page.js.compress')).st_size, 'compressed file must be smaller'
    shutil.move(os.path.join(self.get_dir(), 'page.js'), 'page.js.renamedsoitcannotbefound');
    self.run_browser('page.html', '', '/report_result?1')

  def test_preload_file(self):
    absolute_src_path = os.path.join(self.get_dir(), 'somefile.txt').replace('\\', '/')
    open(absolute_src_path, 'w').write('''load me right before running the code please''')

    absolute_src_path2 = os.path.join(self.get_dir(), '.somefile.txt').replace('\\', '/')
    open(absolute_src_path2, 'w').write('''load me right before running the code please''')
    
    def make_main(path):
      print 'make main at', path
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

    # By absolute path

    make_main('somefile.txt') # absolute becomes relative
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', absolute_src_path, '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should see |load me right before|.', '/report_result?1')

    # Test subdirectory handling with asset packaging.
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
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--preload-file', srcpath, '-o', 'page.html']).communicate()
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
        FS.createPreloadedFile('/', 'someotherfile.txt', 'somefile.txt', true, false);
      };
    ''')
    make_main('someotherfile.txt')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '-o', 'page.html']).communicate()
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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-o', 'page.html', '--preload-file', 'datafile.txt', '--preload-file', 'datafile2.txt',
           '--compression', '%s,%s,%s' % (path_from_root('third_party', 'lzma.js', 'lzma-native'),
                                          path_from_root('third_party', 'lzma.js', 'lzma-decoder.js'),
                                          'LZMA.decompress')]).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'datafile.txt')), 'must be data file'
    assert os.path.exists(os.path.join(self.get_dir(), 'page.data.compress')), 'must be data file in compressed form'
    assert os.stat(os.path.join(self.get_dir(), 'page.js')).st_size != os.stat(os.path.join(self.get_dir(), 'page.js.compress')).st_size, 'compressed file must be different'
    shutil.move(os.path.join(self.get_dir(), 'datafile.txt'), 'datafile.txt.renamedsoitcannotbefound');
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_image(self):
    # load an image file, get pixel data. Also O2 coverage for --preload-file, and memory-init
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpg'))
    open(os.path.join(self.get_dir(), 'sdl_image.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))

    for mem in [0, 1]:
      for dest, dirname, basename in [('screenshot.jpg',                        '/',       'screenshot.jpg'),
                                      ('screenshot.jpg@/assets/screenshot.jpg', '/assets', 'screenshot.jpg')]:
        Popen([
          PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_image.c'), '-o', 'page.html', '-O2', '--memory-init-file', str(mem),
          '--preload-file', dest, '-DSCREENSHOT_DIRNAME="' + dirname + '"', '-DSCREENSHOT_BASENAME="' + basename + '"'
        ]).communicate()
        self.run_browser('page.html', '', '/report_result?600')

  def test_sdl_image_jpeg(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.jpeg'))
    open(os.path.join(self.get_dir(), 'sdl_image_jpeg.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))
    Popen([
      PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_image_jpeg.c'), '-o', 'page.html',
      '--preload-file', 'screenshot.jpeg', '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="screenshot.jpeg"'
    ]).communicate()
    self.run_browser('page.html', '', '/report_result?600')

  def test_sdl_image_compressed(self):
    for image, width in [(path_from_root('tests', 'screenshot2.png'), 300),
                         (path_from_root('tests', 'screenshot.jpg'), 600)]:
      self.clear()
      print image

      basename = os.path.basename(image)
      shutil.copyfile(image, os.path.join(self.get_dir(), basename))
      open(os.path.join(self.get_dir(), 'sdl_image.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_image.c')).read()))

      self.build_native_lzma()
      Popen([
        PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_image.c'), '-o', 'page.html',
        '--preload-file', basename, '-DSCREENSHOT_DIRNAME="/"', '-DSCREENSHOT_BASENAME="' + basename + '"',
        '--compression', '%s,%s,%s' % (path_from_root('third_party', 'lzma.js', 'lzma-native'),
                                       path_from_root('third_party', 'lzma.js', 'lzma-decoder.js'),
                                       'LZMA.decompress')
      ]).communicate()
      shutil.move(os.path.join(self.get_dir(), basename), basename + '.renamedsoitcannotbefound');
      self.run_browser('page.html', '', '/report_result?' + str(width))

  def test_sdl_image_prepare(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_image_prepare.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not'])

  def test_sdl_image_prepare_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_image_prepare_data.c', reference='screenshot.jpg', args=['--preload-file', 'screenshot.not'])

  def test_sdl_stb_image(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_stb_image.c', reference='screenshot.jpg', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not'])

  def test_sdl_stb_image_data(self):
    # load an image file, get pixel data.
    shutil.copyfile(path_from_root('tests', 'screenshot.jpg'), os.path.join(self.get_dir(), 'screenshot.not'))
    self.btest('sdl_stb_image_data.c', reference='screenshot.jpg', args=['-s', 'STB_IMAGE=1', '--preload-file', 'screenshot.not'])

  def test_sdl_canvas(self):
    open(os.path.join(self.get_dir(), 'sdl_canvas.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_canvas.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_canvas.c'), '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_key(self):
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
        document.dispatchEvent(event);
      }

      function keyup(c) {
        var event = document.createEvent("KeyboardEvent");
        event.initKeyEvent("keyup", true, true, window,
                           0, 0, 0, 0,
                           c, c);
        document.dispatchEvent(event);
      }
    ''')
    open(os.path.join(self.get_dir(), 'sdl_key.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_key.c')).read()))

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_key.c'), '-o', 'page.html', '--pre-js', 'pre.js', '-s', '''EXPORTED_FUNCTIONS=['_main', '_one']''']).communicate()
    self.run_browser('page.html', '', '/report_result?223092870')

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
                  text = Array.prototype.slice.call(arguments).join(' ');
                  element.value += text + "\\n";
                  element.scrollTop = 99999; // focus on bottom
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
    self.btest('sdl_pumpevents.c', expected='3', args=['--pre-js', 'pre.js'])

  def test_sdl_audio(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'alarmvictory_1.ogg'), os.path.join(self.get_dir(), 'sound.ogg'))
    shutil.copyfile(path_from_root('tests', 'sounds', 'alarmcreatemiltaryfoot_1.wav'), os.path.join(self.get_dir(), 'sound2.wav'))
    shutil.copyfile(path_from_root('tests', 'sounds', 'noise.ogg'), os.path.join(self.get_dir(), 'noise.ogg'))
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.ogg'), os.path.join(self.get_dir(), 'the_entertainer.ogg'))
    open(os.path.join(self.get_dir(), 'bad.ogg'), 'w').write('I claim to be audio, but am lying')
    open(os.path.join(self.get_dir(), 'sdl_audio.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio.c')).read()))

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    Popen([PYTHON, EMCC, '-O2', '--closure', '1', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio.c'), '--preload-file', 'sound.ogg', '--preload-file', 'sound2.wav', '--embed-file', 'the_entertainer.ogg', '--preload-file', 'noise.ogg', '--preload-file', 'bad.ogg', '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play", "_play2"]']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_audio_mix_channels(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'noise.ogg'), os.path.join(self.get_dir(), 'sound.ogg'))
    open(os.path.join(self.get_dir(), 'sdl_audio_mix_channels.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio_mix_channels.c')).read()))

    Popen([PYTHON, EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio_mix_channels.c'), '--preload-file', 'sound.ogg', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_audio_mix(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'pluck.ogg'), os.path.join(self.get_dir(), 'sound.ogg'))
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.ogg'), os.path.join(self.get_dir(), 'music.ogg'))
    shutil.copyfile(path_from_root('tests', 'sounds', 'noise.ogg'), os.path.join(self.get_dir(), 'noise.ogg'))
    open(os.path.join(self.get_dir(), 'sdl_audio_mix.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio_mix.c')).read()))

    Popen([PYTHON, EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio_mix.c'), '--preload-file', 'sound.ogg', '--preload-file', 'music.ogg', '--preload-file', 'noise.ogg', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_audio_quickload(self):
    open(os.path.join(self.get_dir(), 'sdl_audio_quickload.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio_quickload.c')).read()))

    Popen([PYTHON, EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio_quickload.c'), '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play"]']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_gl_read(self):
    # SDL, OpenGL, readPixels
    open(os.path.join(self.get_dir(), 'sdl_gl_read.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_gl_read.c')).read()))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'sdl_gl_read.c'), '-o', 'something.html']).communicate()
    self.run_browser('something.html', '.', '/report_result?1')

  def test_sdl_ogl(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl.c', reference='screenshot-gray-purple.png', reference_slack=1,
      args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png'],
      message='You should see an image with gray at the top.')

  def test_sdl_ogl_defaultmatrixmode(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl_defaultMatrixMode.c', reference='screenshot-gray-purple.png', reference_slack=1,
      args=['--minify', '0', '--preload-file', 'screenshot.png'],
      message='You should see an image with gray at the top.')

  def test_sdl_ogl_p(self):
    # Immediate mode with pointers
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_ogl_p.c', reference='screenshot-gray.png', reference_slack=1,
      args=['--preload-file', 'screenshot.png'],
      message='You should see an image with gray at the top.')

  def test_sdl_fog_simple(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_simple.c', reference='screenshot-fog-simple.png',
      args=['-O2', '--minify', '0', '--preload-file', 'screenshot.png'],
      message='You should see an image with fog.')

  def test_sdl_fog_negative(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_negative.c', reference='screenshot-fog-negative.png',
      args=['--preload-file', 'screenshot.png'],
      message='You should see an image with fog.')

  def test_sdl_fog_density(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_density.c', reference='screenshot-fog-density.png',
      args=['--preload-file', 'screenshot.png'],
      message='You should see an image with fog.')

  def test_sdl_fog_exp2(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_exp2.c', reference='screenshot-fog-exp2.png',
      args=['--preload-file', 'screenshot.png'],
      message='You should see an image with fog.')

  def test_sdl_fog_linear(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_fog_linear.c', reference='screenshot-fog-linear.png', reference_slack=1,
      args=['--preload-file', 'screenshot.png'],
      message='You should see an image with fog.')

  def test_openal_playback(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'audio.wav'), os.path.join(self.get_dir(), 'audio.wav'))
    open(os.path.join(self.get_dir(), 'openal_playback.cpp'), 'w').write(self.with_report_result(open(path_from_root('tests', 'openal_playback.cpp')).read()))

    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'openal_playback.cpp'), '--preload-file', 'audio.wav', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_openal_buffers(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.wav'), os.path.join(self.get_dir(), 'the_entertainer.wav'))
    self.btest('openal_buffers.c', '0', args=['--preload-file', 'the_entertainer.wav'],)

  def test_glfw(self):
    open(os.path.join(self.get_dir(), 'glfw.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'glfw.c')).read()))

    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'glfw.c'), '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_egl_width_height(self):
    open(os.path.join(self.get_dir(), 'test_egl_width_height.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'test_egl_width_height.c')).read()))

    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'test_egl_width_height.c'), '-o', 'page.html']).communicate()
    self.run_browser('page.html', 'Should print "(300, 150)" -- the size of the canvas in pixels', '/report_result?1')

  def test_freealut(self):
    programs = self.get_library('freealut', os.path.join('examples', '.libs', 'hello_world.bc'), make_args=['EXEEXT=.bc'])
    for program in programs:
      assert os.path.exists(program)
      Popen([PYTHON, EMCC, '-O2', program, '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should hear "Hello World!"')

  def test_worker(self):
    # Test running in a web worker
    output = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_worker.cpp'), '-o', 'worker.js'], stdout=PIPE, stderr=PIPE).communicate()
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
    expectedConns = 11
    import zlib
    checksum = zlib.adler32(data)

    def chunked_server(support_byte_ranges):
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
      httpd = BaseHTTPServer.HTTPServer(('localhost', 11111), ChunkedServerHandler)
      for i in range(expectedConns+1):
        httpd.handle_request()

    server = multiprocessing.Process(target=chunked_server, args=(True,))
    server.start()
    self.run_browser(main, 'Chunked binary synchronous XHR in Web Workers!', '/report_result?' + str(checksum))
    server.terminate()

  def test_glgears(self):
    self.btest('hello_world_gles.c', reference='gears.png', reference_slack=1,
        args=['-DHAVE_BUILTIN_SINCOS'], outfile='something.html',
        message='You should see animating gears.')

  def test_glgears_animation(self):
    es2_suffix = ['', '_full', '_full_944']
    for full_es2 in [0, 1, 2]:
      for emulation in [0, 1]:
        if full_es2 and emulation: continue
        print full_es2, emulation
        Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_gles%s.c' % es2_suffix[full_es2]), '-o', 'something.html',
                                             '-DHAVE_BUILTIN_SINCOS', '-s', 'GL_TESTING=1',
                                             '--shell-file', path_from_root('tests', 'hello_world_gles_shell.html')] +
              (['-s', 'FORCE_GL_EMULATION=1'] if emulation else []) +
              (['-s', 'FULL_ES2=1'] if full_es2 else []),
              ).communicate()
        self.run_browser('something.html', 'You should see animating gears.', '/report_gl_result?true')
        assert ('var GLEmulation' in open(self.in_dir('something.html')).read()) == emulation, "emulation code should be added when asked for"

  def test_fulles2_sdlproc(self):
    self.btest('full_es2_sdlproc.c', '1', args=['-s', 'GL_TESTING=1', '-DHAVE_BUILTIN_SINCOS', '-s', 'FULL_ES2=1'])

  def test_glgears_deriv(self):
    self.btest('hello_world_gles_deriv.c', reference='gears.png', reference_slack=1,
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
          reference=book_path(basename.replace('.bc', '.png')), args=args)

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

  def test_emscripten_api_infloop(self):
    self.btest('emscripten_api_browser_infloop.cpp', '7')

  def test_emscripten_fs_api(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png')) # preloaded *after* run
    self.btest('emscripten_fs_api_browser.cpp', '1')

  def test_sdl_quit(self):
    self.btest('sdl_quit.c', '1')

  def test_sdl_resize(self):
    self.btest('sdl_resize.c', '1')

  def test_gc(self):
    self.btest('browser_gc.cpp', '1')

  def test_glshaderinfo(self):
    self.btest('glshaderinfo.cpp', '1')

  def test_glgetattachedshaders(self):
    self.btest('glgetattachedshaders.c', '1')

  def test_sdlglshader(self):
    self.btest('sdlglshader.c', reference='sdlglshader.png', args=['-O2', '--closure', '1'])

  def test_gl_ps(self):
    # pointers and a shader
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png'], reference_slack=1)

  def test_gl_ps_packed(self):
    # packed data that needs to be strided
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps_packed.c', reference='gl_ps.png', args=['--preload-file', 'screenshot.png'], reference_slack=1)

  def test_gl_ps_strides(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('gl_ps_strides.c', reference='gl_ps_strides.png', args=['--preload-file', 'screenshot.png'])

  def test_gl_renderers(self):
    self.btest('gl_renderers.c', reference='gl_renderers.png', args=['-s', 'GL_UNSAFE_OPTS=0'])

  def test_gl_stride(self):
    self.btest('gl_stride.c', reference='gl_stride.png', args=['-s', 'GL_UNSAFE_OPTS=0'])

  def test_matrix_identity(self):
    self.btest('gl_matrix_identity.c', expected=['-1882984448', '460451840'])

  def test_cubegeom_pre(self):
    self.btest('cubegeom_pre.c', reference='cubegeom_pre.png')

  def test_cubegeom_pre2(self):
    self.btest('cubegeom_pre2.c', reference='cubegeom_pre2.png', args=['-s', 'GL_DEBUG=1']) # some coverage for GL_DEBUG not breaking the build

  def test_cubegeom_pre3(self):
    self.btest('cubegeom_pre3.c', reference='cubegeom_pre2.png')

  def test_cubegeom(self):
    self.btest('cubegeom.c', args=['-O2', '-g'], reference='cubegeom.png')

  def test_cubegeom_glew(self):
    self.btest('cubegeom_glew.c', args=['-O2', '--closure', '1'], reference='cubegeom.png')

  def test_cubegeom_color(self):
    self.btest('cubegeom_color.c', reference='cubegeom_color.png')

  def test_cubegeom_normal(self):
    self.btest('cubegeom_normal.c', reference='cubegeom_normal.png')

  def test_cubegeom_normal_dap(self): # draw is given a direct pointer to clientside memory, no element array buffer
    self.btest('cubegeom_normal_dap.c', reference='cubegeom_normal.png')

  def test_cubegeom_normal_dap_far(self): # indices do nto start from 0
    self.btest('cubegeom_normal_dap_far.c', reference='cubegeom_normal.png')

  def test_cubegeom_normal_dap_far_range(self): # glDrawRangeElements
    self.btest('cubegeom_normal_dap_far_range.c', reference='cubegeom_normal.png')

  def test_cubegeom_normal_dap_far_glda(self): # use glDrawArrays
    self.btest('cubegeom_normal_dap_far_glda.c', reference='cubegeom_normal_dap_far_glda.png')

  def test_cubegeom_normal_dap_far_glda_quad(self): # with quad
    self.btest('cubegeom_normal_dap_far_glda_quad.c', reference='cubegeom_normal_dap_far_glda_quad.png')

  def test_cubegeom_mt(self):
    self.btest('cubegeom_mt.c', reference='cubegeom_mt.png') # multitexture

  def test_cubegeom_color2(self):
    self.btest('cubegeom_color2.c', reference='cubegeom_color2.png')

  def test_cubegeom_texturematrix(self):
    self.btest('cubegeom_texturematrix.c', reference='cubegeom_texturematrix.png')

  def test_cubegeom_fog(self):
    self.btest('cubegeom_fog.c', reference='cubegeom_fog.png')

  def test_cubegeom_pre_vao(self):
    self.btest('cubegeom_pre_vao.c', reference='cubegeom_pre_vao.png')

  def test_cubegeom_pre2_vao(self):
    self.btest('cubegeom_pre2_vao.c', reference='cubegeom_pre_vao.png')

  def test_cubegeom_pre2_vao2(self):
    self.btest('cubegeom_pre2_vao2.c', reference='cubegeom_pre2_vao2.png')

  def test_cube_explosion(self):
    self.btest('cube_explosion.c', reference='cube_explosion.png')

  def test_sdl_canvas_blank(self):
    self.btest('sdl_canvas_blank.c', reference='sdl_canvas_blank.png')

  def test_sdl_canvas_palette(self):
    self.btest('sdl_canvas_palette.c', reference='sdl_canvas_palette.png')

  def test_sdl_canvas_twice(self):
    self.btest('sdl_canvas_twice.c', reference='sdl_canvas_twice.png')

  def test_sdl_maprgba(self):
    self.btest('sdl_maprgba.c', reference='sdl_maprgba.png', reference_slack=3)

  def test_sdl_rotozoom(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('sdl_rotozoom.c', reference='sdl_rotozoom.png', args=['--preload-file', 'screenshot.png'], reference_slack=5)

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

  def test_glbegin_points(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'screenshot.png'))
    self.btest('glbegin_points.c', reference='glbegin_points.png', args=['--preload-file', 'screenshot.png'])

  def test_s3tc(self):
    shutil.copyfile(path_from_root('tests', 'screenshot.dds'), os.path.join(self.get_dir(), 'screenshot.dds'))
    self.btest('s3tc.c', reference='s3tc.png', args=['--preload-file', 'screenshot.dds'])

  def test_s3tc_crunch(self):
    shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
    shutil.copyfile(path_from_root('tests', 'bloom.dds'), 'bloom.dds')
    shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--pre-run', '--crunch', '--preload', 'ship.dds', 'bloom.dds', 'water.dds'], stdout=open('pre.js', 'w')).communicate()
    assert os.stat('test.data').st_size < 0.5*(os.stat('ship.dds').st_size+os.stat('bloom.dds').st_size+os.stat('water.dds').st_size), 'Compressed should be smaller than dds'
    shutil.move('ship.dds', 'ship.donotfindme.dds') # make sure we load from the compressed
    shutil.move('bloom.dds', 'bloom.donotfindme.dds') # make sure we load from the compressed
    shutil.move('water.dds', 'water.donotfindme.dds') # make sure we load from the compressed
    self.btest('s3tc_crunch.c', reference='s3tc_crunch.png', reference_slack=11, args=['--pre-js', 'pre.js'])

  def test_s3tc_crunch_split(self): # load several datafiles/outputs of file packager
    shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
    shutil.copyfile(path_from_root('tests', 'bloom.dds'), 'bloom.dds')
    shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
    Popen([PYTHON, FILE_PACKAGER, 'asset_a.data', '--pre-run', '--crunch', '--preload', 'ship.dds', 'bloom.dds'], stdout=open('asset_a.js', 'w')).communicate()
    Popen([PYTHON, FILE_PACKAGER, 'asset_b.data', '--pre-run', '--crunch', '--preload', 'water.dds'], stdout=open('asset_b.js', 'w')).communicate()
    shutil.move('ship.dds', 'ship.donotfindme.dds') # make sure we load from the compressed
    shutil.move('bloom.dds', 'bloom.donotfindme.dds') # make sure we load from the compressed
    shutil.move('water.dds', 'water.donotfindme.dds') # make sure we load from the compressed
    self.btest('s3tc_crunch.c', reference='s3tc_crunch.png', reference_slack=11, args=['--pre-js', 'asset_a.js', '--pre-js', 'asset_b.js'])

  def test_aniso(self):
    shutil.copyfile(path_from_root('tests', 'water.dds'), 'water.dds')
    self.btest('aniso.c', reference='aniso.png', reference_slack=2, args=['--preload-file', 'water.dds'])

  def test_tex_nonbyte(self):
    self.btest('tex_nonbyte.c', reference='tex_nonbyte.png')

  def test_float_tex(self):
    self.btest('float_tex.cpp', reference='float_tex.png')

  def test_subdata(self):
    self.btest('gl_subdata.cpp', reference='float_tex.png')

  def test_perspective(self):
    self.btest('perspective.c', reference='perspective.png')

  def test_runtimelink(self):
    return self.skip('shared libs are deprecated')
    main, supp = self.setup_runtimelink_test()

    open(self.in_dir('supp.cpp'), 'w').write(supp)
    Popen([PYTHON, EMCC, self.in_dir('supp.cpp'), '-o', 'supp.js', '-s', 'LINKABLE=1', '-s', 'NAMED_GLOBALS=1', '-s', 'BUILD_AS_SHARED_LIB=2', '-O2', '-s', 'ASM_JS=0']).communicate()
    shutil.move(self.in_dir('supp.js'), self.in_dir('supp.so'))

    self.btest(main, args=['-s', 'LINKABLE=1', '-s', 'NAMED_GLOBALS=1', '-s', 'RUNTIME_LINKED_LIBS=["supp.so"]', '-DBROWSER=1', '-O2', '-s', 'ASM_JS=0'], expected='76')

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

  def test_worker_api(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'worker_api_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-s', 'EXPORTED_FUNCTIONS=["_one"]']).communicate()
    self.btest('worker_api_main.cpp', expected='566')

  def test_worker_api_2(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'worker_api_2_worker.cpp'), '-o', 'worker.js', '-s', 'BUILD_AS_WORKER=1', '-O2', '--minify', '0', '-s', 'EXPORTED_FUNCTIONS=["_one", "_two", "_three", "_four"]']).communicate()
    self.btest('worker_api_2_main.cpp', args=['-O2', '--minify', '0'], expected='11')

  def test_emscripten_async_wget2(self):
    self.btest('http.cpp', expected='0', args=['-I' + path_from_root('tests')])
