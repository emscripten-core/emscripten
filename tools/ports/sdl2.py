# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

VERSION = '2.32.10'
TAG = f'release-{VERSION}'
HASH = '001738b610b42a8f8badfd6af3402f0a1a8601034adef0b8c702dd2b1951dc1b71b733a6779d97499b6f7314d226ec0c8dcffeb753f35a5c51e995ca20bdd459'
SUBDIR = f'SDL-{TAG}'

variants = {'sdl2-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_SDL == 2


def get_lib_name(settings):
  return 'libSDL2' + ('-mt' if settings.PTHREADS else '') + '.a'


def process_dependencies(settings, cflags_only):
  if not cflags_only:
    # SDL2 includes an internal reference to Module['createContext']
    settings.EXPORTED_RUNTIME_METHODS.append('createContext')

    # SDL2 requires eglGetProcAddress() to work.
    # NOTE: if SDL2 is updated to not rely on eglGetProcAddress(), this can be removed
    settings.GL_ENABLE_GET_PROC_ADDRESS = 1


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('sdl2', f'https://github.com/libsdl-org/SDL/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    # copy includes to a location so they can be used as 'SDL2/'
    src_dir = ports.get_dir('sdl2', SUBDIR)
    source_include_path = os.path.join(src_dir, 'include')
    ports.install_headers(source_include_path, target='SDL2')
    ports.make_pkg_config('sdl2', VERSION, '-sUSE_SDL=2')

    # copy sdl2-config.cmake
    cmake_file = os.path.join(os.path.dirname(__file__), 'sdl2/sdl2-config.cmake')
    ports.install_file(cmake_file, 'lib/cmake/SDL2/sdl2-config.cmake')

    # build
    srcs = '''SDL.c SDL_assert.c SDL_dataqueue.c SDL_error.c SDL_guid.c SDL_hints.c SDL_list.c SDL_log.c
    SDL_utils.c atomic/SDL_atomic.c atomic/SDL_spinlock.c audio/SDL_audio.c audio/SDL_audiocvt.c
    audio/SDL_audiodev.c audio/SDL_audiotypecvt.c audio/SDL_mixer.c audio/SDL_wave.c cpuinfo/SDL_cpuinfo.c
    dynapi/SDL_dynapi.c events/SDL_clipboardevents.c events/SDL_displayevents.c events/SDL_dropevents.c
    events/SDL_events.c events/SDL_gesture.c events/SDL_keyboard.c events/SDL_keysym_to_scancode.c
    events/SDL_scancode_tables.c events/SDL_mouse.c events/SDL_quit.c
    events/SDL_touch.c events/SDL_windowevents.c file/SDL_rwops.c haptic/SDL_haptic.c
    joystick/controller_type.c joystick/SDL_gamecontroller.c joystick/SDL_joystick.c
    joystick/SDL_steam_virtual_gamepad.c
    power/SDL_power.c render/SDL_d3dmath.c render/SDL_render.c
    render/SDL_yuv_sw.c render/direct3d/SDL_render_d3d.c render/direct3d11/SDL_render_d3d11.c
    render/opengl/SDL_render_gl.c render/opengl/SDL_shaders_gl.c render/opengles/SDL_render_gles.c
    render/opengles2/SDL_render_gles2.c render/opengles2/SDL_shaders_gles2.c
    render/psp/SDL_render_psp.c render/software/SDL_blendfillrect.c render/software/SDL_blendline.c
    render/software/SDL_blendpoint.c render/software/SDL_drawline.c render/software/SDL_drawpoint.c
    render/software/SDL_render_sw.c render/software/SDL_rotate.c render/software/SDL_triangle.c
    sensor/SDL_sensor.c sensor/dummy/SDL_dummysensor.c
    stdlib/SDL_crc16.c stdlib/SDL_crc32.c stdlib/SDL_getenv.c stdlib/SDL_iconv.c stdlib/SDL_malloc.c
    stdlib/SDL_qsort.c stdlib/SDL_stdlib.c stdlib/SDL_string.c stdlib/SDL_strtokr.c
    thread/SDL_thread.c timer/SDL_timer.c
    video/SDL_RLEaccel.c video/SDL_blit.c video/SDL_blit_0.c video/SDL_blit_1.c video/SDL_blit_A.c
    video/SDL_blit_N.c video/SDL_blit_auto.c video/SDL_blit_copy.c video/SDL_blit_slow.c
    video/SDL_bmp.c video/SDL_clipboard.c video/SDL_egl.c video/SDL_fillrect.c video/SDL_pixels.c
    video/SDL_rect.c video/SDL_shape.c video/SDL_stretch.c video/SDL_surface.c video/SDL_video.c
    video/SDL_yuv.c video/emscripten/SDL_emscriptenevents.c
    video/emscripten/SDL_emscriptenframebuffer.c video/emscripten/SDL_emscriptenmouse.c
    video/emscripten/SDL_emscriptenopengles.c video/emscripten/SDL_emscriptenvideo.c
    audio/emscripten/SDL_emscriptenaudio.c video/dummy/SDL_nullevents.c
    video/dummy/SDL_nullframebuffer.c video/dummy/SDL_nullvideo.c video/yuv2rgb/yuv_rgb_std.c
    audio/disk/SDL_diskaudio.c audio/dummy/SDL_dummyaudio.c loadso/dlopen/SDL_sysloadso.c
    power/emscripten/SDL_syspower.c joystick/emscripten/SDL_sysjoystick.c
    filesystem/emscripten/SDL_sysfilesystem.c timer/unix/SDL_systimer.c haptic/dummy/SDL_syshaptic.c
    main/dummy/SDL_dummy_main.c locale/SDL_locale.c locale/emscripten/SDL_syslocale.c misc/SDL_url.c
    misc/emscripten/SDL_sysurl.c'''.split()
    thread_srcs = ['SDL_syscond.c', 'SDL_sysmutex.c', 'SDL_syssem.c', 'SDL_systhread.c', 'SDL_systls.c']
    thread_backend = 'generic' if not settings.PTHREADS else 'pthread'
    srcs += ['thread/%s/%s' % (thread_backend, s) for s in thread_srcs]

    srcs = [os.path.join(src_dir, 'src', s) for s in srcs]
    # TODO: Remove fwrapv when we update to a version which includes https://github.com/libsdl-org/SDL/pull/12581
    flags = ['-sUSE_SDL=0', '-fwrapv-pointer']
    includes = [ports.get_include_dir('SDL2')]
    if settings.PTHREADS:
      flags += ['-pthread']
    ports.build_port(src_dir, final, 'sdl2', srcs=srcs, includes=includes, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_args(ports):
  return ['-isystem', ports.get_include_dir('SDL2')]


def show():
  return 'sdl2 (-sUSE_SDL=2 or --use-port=sdl2; zlib license)'
