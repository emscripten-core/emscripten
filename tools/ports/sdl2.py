# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'release-2.0.20'
HASH = '67e1abe1183b04836b35d724fd495c83c9559b4530d4a5c9bcc89648af0ac7cc51c02f7055a1664fe5f5f90953d22a6c431fa8bc5cdd77c94a97f107c47e2d62'
SUBDIR = 'SDL-' + TAG


def needed(settings):
  return settings.USE_SDL == 2


def get_lib_name(settings):
  return 'libSDL2' + ('-mt' if settings.USE_PTHREADS else '') + '.a'


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('sdl2', 'https://github.com/libsdl-org/SDL/archive/' + TAG + '.zip', SUBDIR, sha512hash=HASH)

  def create(final):
    # copy includes to a location so they can be used as 'SDL2/'
    source_include_path = os.path.join(ports.get_dir(), 'sdl2', SUBDIR, 'include')
    ports.install_headers(source_include_path, target='SDL2')

    # build
    srcs = '''SDL.c SDL_assert.c SDL_dataqueue.c SDL_error.c SDL_hints.c SDL_log.c atomic/SDL_atomic.c
    atomic/SDL_spinlock.c audio/SDL_audio.c audio/SDL_audiocvt.c audio/SDL_audiodev.c
    audio/SDL_audiotypecvt.c audio/SDL_mixer.c audio/SDL_wave.c cpuinfo/SDL_cpuinfo.c
    dynapi/SDL_dynapi.c events/SDL_clipboardevents.c events/SDL_displayevents.c events/SDL_dropevents.c
    events/SDL_events.c events/SDL_gesture.c events/SDL_keyboard.c events/SDL_mouse.c events/SDL_quit.c
    events/SDL_touch.c events/SDL_windowevents.c file/SDL_rwops.c haptic/SDL_haptic.c
    joystick/SDL_gamecontroller.c joystick/SDL_joystick.c
    power/SDL_power.c render/SDL_d3dmath.c render/SDL_render.c
    render/SDL_yuv_sw.c render/direct3d/SDL_render_d3d.c render/direct3d11/SDL_render_d3d11.c
    render/opengl/SDL_render_gl.c render/opengl/SDL_shaders_gl.c render/opengles/SDL_render_gles.c
    render/opengles2/SDL_render_gles2.c render/opengles2/SDL_shaders_gles2.c
    render/psp/SDL_render_psp.c render/software/SDL_blendfillrect.c render/software/SDL_blendline.c
    render/software/SDL_blendpoint.c render/software/SDL_drawline.c render/software/SDL_drawpoint.c
    render/software/SDL_render_sw.c render/software/SDL_rotate.c render/software/SDL_triangle.c
    sensor/SDL_sensor.c sensor/dummy/SDL_dummysensor.c
    stdlib/SDL_crc32.c stdlib/SDL_getenv.c stdlib/SDL_iconv.c stdlib/SDL_malloc.c stdlib/SDL_qsort.c
    stdlib/SDL_stdlib.c stdlib/SDL_string.c stdlib/SDL_strtokr.c thread/SDL_thread.c timer/SDL_timer.c
    video/SDL_RLEaccel.c video/SDL_blit.c video/SDL_blit_0.c video/SDL_blit_1.c video/SDL_blit_A.c
    video/SDL_blit_N.c video/SDL_blit_auto.c video/SDL_blit_copy.c video/SDL_blit_slow.c
    video/SDL_bmp.c video/SDL_clipboard.c video/SDL_egl.c video/SDL_fillrect.c video/SDL_pixels.c
    video/SDL_rect.c video/SDL_shape.c video/SDL_stretch.c video/SDL_surface.c video/SDL_video.c
    video/SDL_yuv.c video/emscripten/SDL_emscriptenevents.c
    video/emscripten/SDL_emscriptenframebuffer.c video/emscripten/SDL_emscriptenmouse.c
    video/emscripten/SDL_emscriptenopengles.c video/emscripten/SDL_emscriptenvideo.c
    audio/emscripten/SDL_emscriptenaudio.c video/dummy/SDL_nullevents.c
    video/dummy/SDL_nullframebuffer.c video/dummy/SDL_nullvideo.c video/yuv2rgb/yuv_rgb.c
    audio/disk/SDL_diskaudio.c audio/dummy/SDL_dummyaudio.c loadso/dlopen/SDL_sysloadso.c
    power/emscripten/SDL_syspower.c joystick/emscripten/SDL_sysjoystick.c
    filesystem/emscripten/SDL_sysfilesystem.c timer/unix/SDL_systimer.c haptic/dummy/SDL_syshaptic.c
    main/dummy/SDL_dummy_main.c locale/SDL_locale.c locale/emscripten/SDL_syslocale.c misc/SDL_url.c
    misc/dummy/SDL_sysurl.c'''.split()
    thread_srcs = ['SDL_syscond.c', 'SDL_sysmutex.c', 'SDL_syssem.c', 'SDL_systhread.c', 'SDL_systls.c']
    thread_backend = 'generic' if not settings.USE_PTHREADS else 'pthread'
    srcs += ['thread/%s/%s' % (thread_backend, s) for s in thread_srcs]

    commands = []
    o_s = []
    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'sdl2', 'src', src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))
      command = [shared.EMCC,
                 '-c', os.path.join(ports.get_dir(), 'sdl2', SUBDIR, 'src', src),
                 '-o', o, '-I' + ports.get_include_dir('SDL2'),
                 '-O2', '-w']
      if settings.USE_PTHREADS:
        command += ['-sUSE_PTHREADS']
      commands.append(command)
      o_s.append(o)
    ports.run_commands(commands)
    ports.create_lib(final, o_s)

  return [shared.Cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_lib(get_lib_name(settings))


def linker_setup(ports, settings):
  settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$autoResumeAudioContext', '$dynCall']


def process_args(ports):
  # TODO(sbc): remove this
  return ['-Xclang', '-isystem' + os.path.join(ports.get_include_dir(), 'SDL2')]


def show():
  return 'SDL2 (USE_SDL=2; zlib license)'
