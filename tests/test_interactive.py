# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os
import shutil

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: tests/runner.py interactive')

from runner import BrowserCore, path_from_root
from tools.shared import Popen, EMCC, PYTHON, WINDOWS, Building


class interactive(BrowserCore):
  @classmethod
  def setUpClass(cls):
    super(interactive, cls).setUpClass()
    cls.browser_timeout = 60
    print()
    print('Running the interactive tests. Make sure the browser allows popups from localhost.')
    print()

  def test_html5_fullscreen(self):
    self.btest(path_from_root('tests', 'test_html5_fullscreen.c'), expected='0', args=['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1', '-s', 'EXPORTED_FUNCTIONS=["_requestFullscreen","_enterSoftFullscreen","_main"]', '--shell-file', path_from_root('tests', 'test_html5_fullscreen.html')])

  def test_html5_mouse(self):
    self.btest(path_from_root('tests', 'test_html5_mouse.c'), expected='0', args=['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'])

  def test_html5_pointerlockerror(self):
    self.btest(path_from_root('tests', 'test_html5_pointerlockerror.c'), expected='0', args=['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1'])

  def test_sdl_mousewheel(self):
    self.btest(path_from_root('tests', 'test_sdl_mousewheel.c'), expected='0')

  def test_sdl_touch(self):
    self.btest(path_from_root('tests', 'sdl_touch.c'), args=['-O2', '-g1', '--closure', '1'], expected='0')

  def test_sdl_wm_togglefullscreen(self):
    self.btest('sdl_wm_togglefullscreen.c', expected='1')

  def test_sdl_fullscreen_samecanvassize(self):
    self.btest('sdl_fullscreen_samecanvassize.c', expected='1')

  def test_sdl2_togglefullscreen(self):
    self.btest('sdl_togglefullscreen.c', expected='1', args=['-s', 'USE_SDL=2'])

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

    # print('SDL2')
    # check sdl2 as well
    # FIXME: restore this test once we have proper SDL2 audio (existing test
    #        depended on fragile SDL1/SDL2 mixing, which stopped working with
    #        7a5744d754e00bec4422405a1a94f60b8e53c8fc (which just uncovered
    #        the existing problem)
    # Popen([PYTHON, EMCC, '-O1', '--closure', '0', '--minify', '0', os.path.join(self.get_dir(), 'sdl_audio.c'), '--preload-file', 'sound.ogg', '--preload-file', 'sound2.wav', '--embed-file', 'the_entertainer.ogg', '--preload-file', 'noise.ogg', '--preload-file', 'bad.ogg', '-o', 'page.html', '-s', 'EXPORTED_FUNCTIONS=["_main", "_play", "_play2"]', '-s', 'USE_SDL=2', '-DUSE_SDL2']).communicate()
    # self.run_browser('page.html', '', '/report_result?1')

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

  def test_sdl2_mixer(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'alarmvictory_1.ogg'), os.path.join(self.get_dir(), 'sound.ogg'))
    open(os.path.join(self.get_dir(), 'sdl2_mixer.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_mixer.c')).read()))

    Popen([PYTHON, EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl2_mixer.c'), '-s', 'USE_SDL=2', '-s', 'USE_SDL_MIXER=2', '-s', 'TOTAL_MEMORY=33554432', '--preload-file', 'sound.ogg', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_sdl2_mixer_wav(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'the_entertainer.wav'), os.path.join(self.get_dir(), 'sound.wav'))
    open(os.path.join(self.get_dir(), 'sdl2_mixer_wav.c'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_mixer_wav.c')).read()))

    Popen([PYTHON, EMCC, '-O2', '--minify', '0', os.path.join(self.get_dir(), 'sdl2_mixer_wav.c'), '-s', 'USE_SDL=2', '-s', 'USE_SDL_MIXER=2', '-s', 'TOTAL_MEMORY=33554432', '--preload-file', 'sound.wav', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def zzztest_sdl2_audio_beeps(self):
    open(os.path.join(self.get_dir(), 'sdl2_audio_beep.cpp'), 'w').write(self.with_report_result(open(path_from_root('tests', 'sdl2_audio_beep.cpp')).read()))

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    Popen([PYTHON, EMCC, '-O2', '--closure', '1', '--minify', '0', os.path.join(self.get_dir(), 'sdl2_audio_beep.cpp'), '-s', 'DISABLE_EXCEPTION_CATCHING=0', '-s', 'USE_SDL=2', '-o', 'page.html']).communicate()
    self.run_browser('page.html', '', '/report_result?1')

  def test_openal_playback(self):
    shutil.copyfile(path_from_root('tests', 'sounds', 'audio.wav'), os.path.join(self.get_dir(), 'audio.wav'))
    open(os.path.join(self.get_dir(), 'openal_playback.cpp'), 'w').write(self.with_report_result(open(path_from_root('tests', 'openal_playback.cpp')).read()))

    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'openal_playback.cpp'), '--preload-file', 'audio.wav', '-o', 'page.html'] + args).communicate()
      self.run_browser('page.html', '', '/report_result?1')

  def test_openal_buffers(self):
    self.btest('openal_buffers.c', '0', args=['--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/'],)

  def test_openal_buffers_animated_pitch(self):
    self.btest('openal_buffers.c', '0', args=['-DTEST_ANIMATED_PITCH=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/'],)

  def test_openal_looped_pitched_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_looped_seek_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_LOOPED_SEEK_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_pitched_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_PITCHED_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_distance_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_DISTANCE_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_doppler_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_DOPPLER_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_panned_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_PANNED_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_relative_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_RELATIVE_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_al_soft_loop_points(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_AL_SOFT_LOOP_POINTS=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_alc_soft_pause_device(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ALC_SOFT_PAUSE_DEVICE=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_al_soft_source_spatialize(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_AL_SOFT_SOURCE_SPATIALIZE=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', path_from_root('tests', 'sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_capture(self):
    self.btest('openal_capture.c', expected='0')

  def get_freealut_library(self):
    if WINDOWS and Building.which('cmake'):
      return self.get_library('freealut', os.path.join('hello_world.bc'), configure=['cmake', '.'], configure_args=['-DBUILD_TESTS=ON'])
    else:
      return self.get_library('freealut', [os.path.join('examples', '.libs', 'hello_world.bc'), os.path.join('src', '.libs', 'libalut.a')], make_args=['EXEEXT=.bc'])

  def test_freealut(self):
    Popen([PYTHON, EMCC, '-O2'] + self.get_freealut_library() + ['-o', 'page.html']).communicate()
    self.run_browser('page.html', 'You should hear "Hello World!"')

  def test_vr(self):
    self.btest(path_from_root('tests', 'test_vr.c'), expected='0')

  def test_glfw_cursor_disabled(self):
    self.btest('test_glfw_cursor_disabled.c', expected='1', args=['-s', 'USE_GLFW=3', '-lglfw', '-lGL'])

  def test_glfw_dropfile(self):
    self.btest('test_glfw_dropfile.c', expected='1', args=['-s', 'USE_GLFW=3', '-lglfw', '-lGL'])

  def test_glfw_fullscreen(self):
    self.btest('test_glfw_fullscreen.c', expected='1', args=['-s', 'USE_GLFW=3'])

  def test_glfw_get_key_stuck(self):
    self.btest('test_glfw_get_key_stuck.c', expected='1', args=['-s', 'USE_GLFW=3'])

  def test_glfw_joystick(self):
    self.btest('glfw_joystick.c', expected='1', args=['-s', 'USE_GLFW=3'])

  def test_glfw_pointerlock(self):
    self.btest('test_glfw_pointerlock.c', expected='1', args=['-s', 'USE_GLFW=3'])

  def test_glut_fullscreen(self):
    self.btest('glut_fullscreen.c', expected='1', args=['-lglut', '-lGL'])

  def test_cpuprofiler_memoryprofiler(self):
    self.btest('hello_world_gles.c', expected='0', args=['-DLONGTEST=1', '-DTEST_MEMORYPROFILER_ALLOCATIONS_MAP=1', '-O2', '--cpuprofiler', '--memoryprofiler'])

  def test_threadprofiler(self):
    self.btest('pthread/test_pthread_mandelbrot.cpp', expected='0', args=['-O2', '--threadprofiler', '-s', 'USE_PTHREADS=1', '-DTEST_THREAD_PROFILING=1', '-s', 'PTHREAD_POOL_SIZE=16', '--shell-file', path_from_root('tests', 'pthread', 'test_pthread_mandelbrot_shell.html')])

  # Test that event backproxying works.
  def test_html5_callbacks_on_calling_thread(self):
    # TODO: Make this automatic by injecting mouse event in e.g. shell html file.
    for args in [[], ['-DTEST_SYNC_BLOCKING_LOOP=1']]:
      self.btest('html5_callbacks_on_calling_thread.c', expected='1', args=args + ['-s', 'DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1', '-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1'])

  # Test that it is possible to register HTML5 event callbacks on either main browser thread, or application main thread,
  # and that the application can manually proxy the event from main browser thread to the application main thread, to
  # implement event suppression capabilities.
  def test_html5_callback_on_two_threads(self):
    # TODO: Make this automatic by injecting enter key press in e.g. shell html file.
    for args in [[], ['-s', 'USE_PTHREADS=1', '-s', 'PROXY_TO_PTHREAD=1']]:
      self.btest('html5_event_callback_in_two_threads.c', expected='1', args=args)

  # Test that emscripten_hide_mouse() is callable from pthreads (and proxies to main thread to obtain the proper window.devicePixelRatio value).
  def test_emscripten_hide_mouse(self):
    for args in [[], ['-s', 'USE_PTHREADS=1']]:
      self.btest('emscripten_hide_mouse.c', expected='0', args=args)

  # Tests that WebGL can be run on another thread after first having run it on one thread (and that thread has exited). The intent of this is to stress graceful deinit semantics, so that it is not possible to "taint" a Canvas
  # to a bad state after a rendering thread in a program quits and restarts. (perhaps e.g. between level loads, or subsystem loads/restarts or something like that)
  def test_webgl_offscreen_canvas_in_two_pthreads(self):
    for args in [['-s', 'OFFSCREENCANVAS_SUPPORT=1', '-DTEST_OFFSCREENCANVAS=1'], ['-s', 'OFFSCREEN_FRAMEBUFFER=1']]:
      self.btest('gl_in_two_pthreads.cpp', expected='1', args=args + ['-s', 'USE_PTHREADS=1', '-lGL', '-s', 'GL_DEBUG=1', '-s', 'PROXY_TO_PTHREAD=1'])
