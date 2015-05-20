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

class interactive(BrowserCore):
  @classmethod
  def setUpClass(self):
    super(interactive, self).setUpClass()
    print
    print 'Running the browser tests. Make sure the browser allows popups from localhost.'
    print

  def test_html5_fullscreen(self):
    self.btest(path_from_root('tests', 'test_html5_fullscreen.c'), expected='0', args=['-s', 'EXPORTED_FUNCTIONS=["_requestFullscreen","_enterSoftFullscreen","_main"]', '--shell-file', path_from_root('tests', 'test_html5_fullscreen.html')])

  def test_html5_mouse(self):
    self.btest(path_from_root('tests', 'test_html5_mouse.c'), expected='0')

  def test_sdl_mousewheel(self):
    self.btest(path_from_root('tests', 'test_sdl_mousewheel.c'), expected='0')

  def test_sdl_touch(self):
    self.btest(path_from_root('tests', 'sdl_touch.c'), args=['-O2', '-g1', '--closure', '1'], expected='0')

  def test_sdl_wm_togglefullscreen(self):
    self.btest('sdl_wm_togglefullscreen.c', expected='1', args=['-s', 'NO_EXIT_RUNTIME=1'])

  def test_sdl2_togglefullscreen(self):
    self.btest('sdl_togglefullscreen.c', expected='1', args=['-s', 'USE_SDL=2', '-s', 'NO_EXIT_RUNTIME=1'])

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

    print 'SDL2'

    # check sdl2 as well
    Popen([PYTHON, EMCC, '-O1', '--closure', '0', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio.c'), '--preload-file', 'sound.ogg', '--preload-file', 'sound2.wav', '--embed-file', 'the_entertainer.ogg', '--preload-file', 'noise.ogg', '--preload-file', 'bad.ogg', '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play", "_play2"]', '-s', 'USE_SDL=2', '-DUSE_SDL2']).communicate()
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

  def test_sdl_audio_panning(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.wav'), os.path.join(self.get_dir(), 'the_entertainer.wav'))
    open(os.path.join(self.get_dir(), 'sdl_audio_panning.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio_panning.c')).read()))

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    Popen([PYTHON, EMCC, '-O2', '--closure', '1', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio_panning.c'), '--preload-file', 'the_entertainer.wav', '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play"]']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl_audio_beeps(self):
    open(os.path.join(self.get_dir(), 'sdl_audio_beep.cpp'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl_audio_beep.cpp')).read()))

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    Popen([PYTHON, EMCC, '-O2', '--closure', '1', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio_beep.cpp'), '-s', 'DISABLE_EXCEPTION_CATCHING=0', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def zzztest_sdl2_audio_beeps(self):
    open(os.path.join(self.get_dir(), 'sdl2_audio_beep.cpp'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_audio_beep.cpp')).read()))

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    Popen([PYTHON, EMCC, '-O2', '--closure', '1', '--minify', '0', os.path.join(self.get_dir(), 'sdl2_audio_beep.cpp'), '-s', 'DISABLE_EXCEPTION_CATCHING=0', '-s', 'USE_SDL=2', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_openal_playback(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'audio.wav'), os.path.join(self.get_dir(), 'audio.wav'))
    open(os.path.join(self.get_dir(), 'openal_playback.cpp'), 'w').write(self.with_report_result(open(path_from_root('tests', 'openal_playback.cpp')).read()))

    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'openal_playback.cpp'), '--preload-file', 'audio.wav', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_openal_buffers(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.wav'), os.path.join(self.get_dir(), 'the_entertainer.wav'))
    self.btest('openal_buffers.c', '0', args=['--preload-file', 'the_entertainer.wav'],)

  def get_freealut_library(self):
    if WINDOWS and Building.which('cmake'):
      return self.get_library('freealut', os.path.join('hello_world.bc'), configure=['cmake', '.'], configure_args=['-DBUILD_TESTS=ON'])
    else:
      return self.get_library('freealut', os.path.join('examples', 'hello_world.bc'), make_args=['EXEEXT=.bc'])

  def test_freealut(self):
    programs = self.get_freealut_library()
    for program in programs:
      assert os.path.exists(program)
      Popen([PYTHON, EMCC, '-O2', program, '-o', 'page.html']).communicate()
      self.run_browser('page.html', 'You should hear "Hello World!"')

  def test_vr(self):
    self.btest(path_from_root('tests', 'test_vr.c'), expected='0')
