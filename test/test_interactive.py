# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import json
import os
import shutil

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: test/runner.py interactive')

from common import parameterized
from common import BrowserCore, test_file, create_file, also_with_minimal_runtime
from tools.shared import WINDOWS


class interactive(BrowserCore):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    cls.browser_timeout = 60
    print()
    print('Running the interactive tests. Make sure the browser allows popups from localhost.')
    print()

  def test_html5_fullscreen(self):
    self.btest(test_file('test_html5_fullscreen.c'), expected='0', args=['-sDISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR', '-sEXPORTED_FUNCTIONS=_requestFullscreen,_enterSoftFullscreen,_main', '--shell-file', test_file('test_html5_fullscreen.html')])

  def test_html5_emscripten_exit_with_escape(self):
    self.btest('test_html5_emscripten_exit_fullscreen.c', expected='1', args=['-DEXIT_WITH_F'])

  def test_html5_emscripten_exit_fullscreen(self):
    self.btest('test_html5_emscripten_exit_fullscreen.c', expected='1')

  def test_html5_mouse(self):
    self.btest(test_file('test_html5_mouse.c'), expected='0', args=['-sDISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR'])

  def test_html5_pointerlockerror(self):
    self.btest(test_file('test_html5_pointerlockerror.c'), expected='0', args=['-sDISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR'])

  def test_sdl_mousewheel(self):
    self.btest_exit(test_file('test_sdl_mousewheel.c'))

  def test_sdl_touch(self):
    self.btest(test_file('sdl_touch.c'), args=['-O2', '-g1', '--closure=1'], expected='0')

  def test_sdl_wm_togglefullscreen(self):
    self.btest_exit('sdl_wm_togglefullscreen.c')

  def test_sdl_fullscreen_samecanvassize(self):
    self.btest_exit('sdl_fullscreen_samecanvassize.c')

  def test_sdl2_togglefullscreen(self):
    self.btest_exit('browser/test_sdl_togglefullscreen.c', args=['-sUSE_SDL=2'])

  def test_sdl_audio(self):
    shutil.copyfile(test_file('sounds', 'alarmvictory_1.ogg'), self.in_dir('sound.ogg'))
    shutil.copyfile(test_file('sounds', 'alarmcreatemiltaryfoot_1.wav'), self.in_dir('sound2.wav'))
    shutil.copyfile(test_file('sounds', 'noise.ogg'), self.in_dir('noise.ogg'))
    shutil.copyfile(test_file('sounds', 'the_entertainer.ogg'), self.in_dir('the_entertainer.ogg'))
    create_file('bad.ogg', 'I claim to be audio, but am lying')

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    self.btest_exit(test_file('sdl_audio.c'), args=['--preload-file', 'sound.ogg', '--preload-file', 'sound2.wav', '--embed-file', 'the_entertainer.ogg', '--preload-file', 'noise.ogg', '--preload-file', 'bad.ogg'])

    # print('SDL2')
    # check sdl2 as well
    # FIXME: restore this test once we have proper SDL2 audio (existing test
    #        depended on fragile SDL1/SDL2 mixing, which stopped working with
    #        7a5744d754e00bec4422405a1a94f60b8e53c8fc (which just uncovered
    #        the existing problem)
    # self.run_process([EMCC, '-O1', '--closure', '0', '--minify=0', self.in_dir('sdl_audio.c'), '--preload-file', 'sound.ogg', '--preload-file', 'sound2.wav', '--embed-file', 'the_entertainer.ogg', '--preload-file', 'noise.ogg', '--preload-file', 'bad.ogg', '-o', 'page.html', '-sEXPORTED_FUNCTIONS=[_main,_play,_play2', '-sUSE_SDL=2', '-DUSE_SDL2']).communicate()
    # self.run_browser('page.html', '', '/report_result?1')

  @parameterized({
    '': ([],),
    'wasmfs': (['-sWASMFS'],),
  })
  def test_sdl_audio_mix_channels(self, args):
    shutil.copyfile(test_file('sounds', 'noise.ogg'), self.in_dir('sound.ogg'))

    self.btest_exit('sdl_audio_mix_channels.c', args=['-O2', '--minify=0', '--preload-file', 'sound.ogg'] + args)

  @parameterized({
    '': ([],),
    'wasmfs': (['-sWASMFS'],),
  })
  def test_sdl_audio_mix(self, args):
    shutil.copyfile(test_file('sounds', 'pluck.ogg'), self.in_dir('sound.ogg'))
    shutil.copyfile(test_file('sounds', 'the_entertainer.ogg'), self.in_dir('music.ogg'))
    shutil.copyfile(test_file('sounds', 'noise.ogg'), self.in_dir('noise.ogg'))

    self.btest_exit('sdl_audio_mix.c', args=['-O2', '--minify=0', '--preload-file', 'sound.ogg', '--preload-file', 'music.ogg', '--preload-file', 'noise.ogg'] + args)

  def test_sdl_audio_panning(self):
    shutil.copyfile(test_file('sounds', 'the_entertainer.wav'), self.in_dir('the_entertainer.wav'))

    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    self.btest_exit('sdl_audio_panning.c', args=['-O2', '--closure=1', '--minify=0', '--preload-file', 'the_entertainer.wav', '-sEXPORTED_FUNCTIONS=_main,_play'])

  def test_sdl_audio_beeps(self):
    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    self.btest_exit(test_file('sdl_audio_beep.cpp'), args=['-O2', '--closure=1', '--minify=0', '-sDISABLE_EXCEPTION_CATCHING=0', '-o', 'page.html'])

  def test_sdl2_mixer_wav(self):
    shutil.copyfile(test_file('sounds', 'the_entertainer.wav'), 'sound.wav')
    self.btest_exit('browser/test_sdl2_mixer_wav.c', args=[
      '-O2',
      '-sUSE_SDL=2',
      '-sUSE_SDL_MIXER=2',
      '-sINITIAL_MEMORY=33554432',
      '--preload-file', 'sound.wav'
    ])

  @parameterized({
    'wav': ([],         '0',            'the_entertainer.wav'),
    'ogg': (['ogg'],    'MIX_INIT_OGG', 'alarmvictory_1.ogg'),
    'mp3': (['mp3'],    'MIX_INIT_MP3', 'pudinha.mp3'),
  })
  def test_sdl2_mixer_music(self, formats, flags, music_name):
    shutil.copyfile(test_file('sounds', music_name), music_name)
    self.btest('browser/test_sdl2_mixer_music.c', expected='exit:0', args=[
      '-O2',
      '--minify=0',
      '--preload-file', music_name,
      '-DSOUND_PATH=' + json.dumps(music_name),
      '-DFLAGS=' + flags,
      '-sUSE_SDL=2',
      '-sUSE_SDL_MIXER=2',
      '-sSDL2_MIXER_FORMATS=' + json.dumps(formats),
      '-sINITIAL_MEMORY=33554432'
    ])

  def test_sdl2_audio_beeps(self):
    # use closure to check for a possible bug with closure minifying away newer Audio() attributes
    # TODO: investigate why this does not pass
    self.btest_exit(test_file('browser/test_sdl2_audio_beep.cpp'), args=['-O2', '--closure=1', '--minify=0', '-sDISABLE_EXCEPTION_CATCHING=0', '-sUSE_SDL=2'])

  def test_openal_playback(self):
    shutil.copyfile(test_file('sounds', 'audio.wav'), self.in_dir('audio.wav'))

    for args in [[], ['-pthread', '-sPROXY_TO_PTHREAD']]:
      self.compile_btest(['-O2', test_file('openal_playback.cpp'), '--preload-file', 'audio.wav', '-o', 'page.html'] + args)
      self.run_browser('page.html', '/report_result?1')

  def test_openal_buffers(self):
    self.btest('openal_buffers.c', '0', args=['--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/'],)

  def test_openal_buffers_animated_pitch(self):
    self.btest('openal_buffers.c', '0', args=['-DTEST_ANIMATED_PITCH=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/'],)

  def test_openal_looped_pitched_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_looped_seek_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_LOOPED_SEEK_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_pitched_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_PITCHED_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_distance_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_DISTANCE_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_doppler_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_DOPPLER_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_panned_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_PANNED_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_animated_looped_relative_playback(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ANIMATED_LOOPED_RELATIVE_PLAYBACK=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_al_soft_loop_points(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_AL_SOFT_LOOP_POINTS=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_alc_soft_pause_device(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_ALC_SOFT_PAUSE_DEVICE=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_al_soft_source_spatialize(self):
    self.btest('openal_playback.cpp', '1', args=['-DTEST_AL_SOFT_SOURCE_SPATIALIZE=1', '-DTEST_LOOPED_PLAYBACK=1', '--preload-file', test_file('sounds', 'the_entertainer.wav') + '@/audio.wav'],)

  def test_openal_capture(self):
    self.btest('openal_capture.c', expected='0')

  def get_freealut_library(self):
    self.emcc_args += ['-Wno-pointer-sign']
    if WINDOWS and shutil.which('cmake'):
      return self.get_library(os.path.join('third_party', 'freealut'), 'libalut.a', configure=['cmake', '.'], configure_args=['-DBUILD_TESTS=ON'])
    else:
      return self.get_library(os.path.join('third_party', 'freealut'), os.path.join('src', '.libs', 'libalut.a'), configure_args=['--disable-shared'])

  def test_freealut(self):
    src = test_file('third_party', 'freealut', 'examples', 'hello_world.c')
    inc = test_file('third_party', 'freealut', 'include')
    self.compile_btest([src, '-O2', '-o', 'page.html', '-I' + inc] + self.get_freealut_library())
    self.run_browser('page.html', message='You should hear "Hello World!"')

  def test_glfw_cursor_disabled(self):
    self.btest_exit('interactive/test_glfw_cursor_disabled.c', args=['-sUSE_GLFW=3', '-lglfw', '-lGL'])

  def test_glfw_dropfile(self):
    self.btest_exit('interactive/test_glfw_dropfile.c', args=['-sUSE_GLFW=3', '-lglfw', '-lGL'])

  def test_glfw_fullscreen(self):
    self.btest_exit('interactive/test_glfw_fullscreen.c', args=['-sUSE_GLFW=3'])

  def test_glfw_get_key_stuck(self):
    self.btest_exit('interactive/test_glfw_get_key_stuck.c', args=['-sUSE_GLFW=3'])

  def test_glfw_joystick(self):
    self.btest_exit('interactive/test_glfw_joystick.c', args=['-sUSE_GLFW=3'])

  def test_glfw_pointerlock(self):
    self.btest_exit('interactive/test_glfw_pointerlock.c', args=['-sUSE_GLFW=3'])

  def test_glut_fullscreen(self):
    self.skipTest('looks broken')
    self.btest_exit('glut_fullscreen.c', args=['-lglut', '-lGL'])

  def test_cpuprofiler_memoryprofiler(self):
    self.btest('hello_world_gles.c', expected='0', args=['-DLONGTEST=1', '-DTEST_MEMORYPROFILER_ALLOCATIONS_MAP=1', '-O2', '--cpuprofiler', '--memoryprofiler'])

  def test_threadprofiler(self):
    args = ['-O2', '--threadprofiler',
            '-pthread',
            '-sASSERTIONS',
            '-DTEST_THREAD_PROFILING=1',
            '-sPTHREAD_POOL_SIZE=16',
            '-sINITIAL_MEMORY=64mb',
            '--shell-file', test_file('pthread', 'test_pthread_mandelbrot_shell.html')]
    self.btest_exit('pthread/test_pthread_mandelbrot.cpp', args=args)

  # Test that event backproxying works.
  def test_html5_callbacks_on_calling_thread(self):
    # TODO: Make this automatic by injecting mouse event in e.g. shell html file.
    for args in [[], ['-DTEST_SYNC_BLOCKING_LOOP=1']]:
      self.btest('html5_callbacks_on_calling_thread.c', expected='1', args=args + ['-sDISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR', '-pthread', '-sPROXY_TO_PTHREAD'])

  # Test that it is possible to register HTML5 event callbacks on either main browser thread, or application main thread,
  # and that the application can manually proxy the event from main browser thread to the application main thread, to
  # implement event suppression capabilities.
  def test_html5_callback_on_two_threads(self):
    # TODO: Make this automatic by injecting enter key press in e.g. shell html file.
    for args in [[], ['-pthread', '-sPROXY_TO_PTHREAD']]:
      self.btest('html5_event_callback_in_two_threads.c', expected='1', args=args)

  # Test that emscripten_hide_mouse() is callable from pthreads (and proxies to main thread to obtain the proper window.devicePixelRatio value).
  def test_emscripten_hide_mouse(self):
    for args in [[], ['-pthread']]:
      self.btest('emscripten_hide_mouse.c', expected='0', args=args)

  # Tests that WebGL can be run on another thread after first having run it on one thread (and that thread has exited). The intent of this is to stress graceful deinit semantics, so that it is not possible to "taint" a Canvas
  # to a bad state after a rendering thread in a program quits and restarts. (perhaps e.g. between level loads, or subsystem loads/restarts or something like that)
  def test_webgl_offscreen_canvas_in_two_pthreads(self):
    for args in [['-sOFFSCREENCANVAS_SUPPORT', '-DTEST_OFFSCREENCANVAS=1'], ['-sOFFSCREEN_FRAMEBUFFER']]:
      self.btest('gl_in_two_pthreads.cpp', expected='1', args=args + ['-pthread', '-lGL', '-sGL_DEBUG', '-sPROXY_TO_PTHREAD'])

  # Tests creating a Web Audio context using Emscripten library_webaudio.js feature.
  @also_with_minimal_runtime
  def test_web_audio(self):
    self.btest('webaudio/create_webaudio.c', expected='0', args=['-lwebaudio.js'])

  # Tests simple AudioWorklet noise generation
  @also_with_minimal_runtime
  def test_audio_worklet(self):
    self.btest('webaudio/audioworklet.c', expected='0', args=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '--preload-file', test_file('hello_world.c') + '@/'])
    self.btest('webaudio/audioworklet.c', expected='0', args=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-pthread'])

  # Tests AudioWorklet with emscripten_futex_wake().
  @also_with_minimal_runtime
  def test_audio_worklet_emscripten_futex_wake(self):
    self.btest('webaudio/audioworklet_emscripten_futex_wake.cpp', expected='0', args=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-pthread', '-sPTHREAD_POOL_SIZE=2'])

  # Tests a second AudioWorklet example: sine wave tone generator.
  def test_audio_worklet_tone_generator(self):
    self.btest('webaudio/tone_generator.c', expected='0', args=['-sAUDIO_WORKLET', '-sWASM_WORKERS'])

  # Tests that AUDIO_WORKLET+MINIMAL_RUNTIME+MODULARIZE combination works together.
  def test_audio_worklet_modularize(self):
    self.btest('webaudio/audioworklet.c', expected='0', args=['-sAUDIO_WORKLET', '-sWASM_WORKERS', '-sMINIMAL_RUNTIME', '-sMODULARIZE'])
