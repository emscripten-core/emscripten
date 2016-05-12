import os, shutil, logging

TAG = 'version_11'

def get_with_configure(ports, settings, shared): # not currently used; no real need for configure on emscripten users' machines!
  if settings.USE_SDL == 2:
    ports.fetch_project('sdl2', 'https://github.com/emscripten-ports/SDL2/archive/' + TAG + '.zip', 'SDL2-' + TAG)
    def setup_includes():
      # copy includes to a location so they can be used as 'SDL2/'
      include_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2', 'include')
      shared.try_delete(os.path.join(include_path, 'SDL2'))
      shutil.copytree(include_path, os.path.join(include_path, 'SDL2'))
    ret = [ports.build_project('sdl2', 'SDL2-' + TAG,
                               ['sh', './configure', '--host=asmjs-unknown-emscripten', '--disable-assembly', '--disable-threads', '--enable-cpuinfo=false', 'CFLAGS=-O2'],
                               [os.path.join('build', '.libs', 'libSDL2.a')],
                               setup_includes)]
    return ret
  else:
    return []

def get(ports, settings, shared):
  if settings.USE_SDL == 2:
    ports.fetch_project('sdl2', 'https://github.com/emscripten-ports/SDL2/archive/' + TAG + '.zip', 'SDL2-' + TAG)
    def create():
      # we are rebuilding SDL, clear dependant projects so they copy in their includes to ours properly
      ports.clear_project_build('sdl2-image')
      # copy includes to a location so they can be used as 'SDL2/'
      source_include_path = os.path.join(ports.get_dir(), 'sdl2', 'SDL2-' + TAG + '', 'include')
      dest_include_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2', 'include')
      shared.try_delete(dest_include_path)
      shutil.copytree(source_include_path, dest_include_path)
      shutil.copytree(source_include_path, os.path.join(dest_include_path, 'SDL2'))
      # write out an SDL_config.h file, that configure would normally emit
      open(os.path.join(dest_include_path, 'SDL_config.h'), 'w').write(sdl_config_h)
      open(os.path.join(dest_include_path, 'SDL2', 'SDL_config.h'), 'w').write(sdl_config_h)
      # build
      srcs = 'SDL.c SDL_assert.c SDL_error.c SDL_hints.c SDL_log.c atomic/SDL_atomic.c atomic/SDL_spinlock.c audio/SDL_audio.c audio/SDL_audiocvt.c audio/SDL_audiodev.c audio/SDL_audiotypecvt.c audio/SDL_mixer.c audio/SDL_wave.c cpuinfo/SDL_cpuinfo.c dynapi/SDL_dynapi.c events/SDL_clipboardevents.c events/SDL_dropevents.c events/SDL_events.c events/SDL_gesture.c events/SDL_keyboard.c events/SDL_mouse.c events/SDL_quit.c events/SDL_touch.c events/SDL_windowevents.c file/SDL_rwops.c haptic/SDL_haptic.c joystick/SDL_gamecontroller.c joystick/SDL_joystick.c libm/e_atan2.c libm/e_log.c libm/e_pow.c libm/e_rem_pio2.c libm/e_sqrt.c libm/k_cos.c libm/k_rem_pio2.c libm/k_sin.c libm/k_tan.c libm/s_atan.c libm/s_copysign.c libm/s_cos.c libm/s_fabs.c libm/s_floor.c libm/s_scalbn.c libm/s_sin.c libm/s_tan.c power/SDL_power.c render/SDL_d3dmath.c render/SDL_render.c render/SDL_yuv_mmx.c render/SDL_yuv_sw.c render/direct3d/SDL_render_d3d.c render/direct3d11/SDL_render_d3d11.c render/opengl/SDL_render_gl.c render/opengl/SDL_shaders_gl.c render/opengles/SDL_render_gles.c render/opengles2/SDL_render_gles2.c render/opengles2/SDL_shaders_gles2.c render/psp/SDL_render_psp.c render/software/SDL_blendfillrect.c render/software/SDL_blendline.c render/software/SDL_blendpoint.c render/software/SDL_drawline.c render/software/SDL_drawpoint.c render/software/SDL_render_sw.c render/software/SDL_rotate.c stdlib/SDL_getenv.c stdlib/SDL_iconv.c stdlib/SDL_malloc.c stdlib/SDL_qsort.c stdlib/SDL_stdlib.c stdlib/SDL_string.c thread/SDL_thread.c timer/SDL_timer.c video/SDL_RLEaccel.c video/SDL_blit.c video/SDL_blit_0.c video/SDL_blit_1.c video/SDL_blit_A.c video/SDL_blit_N.c video/SDL_blit_auto.c video/SDL_blit_copy.c video/SDL_blit_slow.c video/SDL_bmp.c video/SDL_clipboard.c video/SDL_egl.c video/SDL_fillrect.c video/SDL_pixels.c video/SDL_rect.c video/SDL_shape.c video/SDL_stretch.c video/SDL_surface.c video/SDL_video.c video/emscripten/SDL_emscriptenevents.c video/emscripten/SDL_emscriptenframebuffer.c video/emscripten/SDL_emscriptenmouse.c video/emscripten/SDL_emscriptenopengles.c video/emscripten/SDL_emscriptenvideo.c audio/emscripten/SDL_emscriptenaudio.c video/dummy/SDL_nullevents.c video/dummy/SDL_nullframebuffer.c video/dummy/SDL_nullvideo.c audio/disk/SDL_diskaudio.c audio/dummy/SDL_dummyaudio.c loadso/dlopen/SDL_sysloadso.c power/emscripten/SDL_syspower.c joystick/emscripten/SDL_sysjoystick.c filesystem/emscripten/SDL_sysfilesystem.c timer/unix/SDL_systimer.c haptic/dummy/SDL_syshaptic.c thread/generic/SDL_syscond.c thread/generic/SDL_sysmutex.c thread/generic/SDL_syssem.c thread/generic/SDL_systhread.c thread/generic/SDL_systls.c main/dummy/SDL_dummy_main.c'.split(' ')
      commands = []
      o_s = []
      for src in srcs:
        o = os.path.join(ports.get_build_dir(), 'sdl2', 'src', src + '.o')
        shared.safe_ensure_dirs(os.path.dirname(o))
        commands.append([shared.PYTHON, shared.EMCC, os.path.join(ports.get_dir(), 'sdl2', 'SDL2-' + TAG, 'src', src), '-O2', '-o', o, '-I' + dest_include_path, '-O2', '-DUSING_GENERATED_CONFIG_H', '-w'])
        o_s.append(o)
      ports.run_commands(commands)
      final = os.path.join(ports.get_build_dir(), 'sdl2', 'libsdl2.bc')
      shared.Building.link(o_s, final)
      return final
    return [shared.Cache.get('sdl2', create, what='port')]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_SDL == 1:
      args += ['-Xclang', '-isystem' + shared.path_from_root('system', 'include', 'SDL')]
  elif settings.USE_SDL == 2:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2', 'include')]
  return args

def show():
  return 'SDL2 (USE_SDL=2; zlib license)'

sdl_config_h = r'''/* include/SDL_config.h.  Generated from SDL_config.h.in by configure.  */
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _SDL_config_h
#define _SDL_config_h

/**
 *  \file SDL_config.h.in
 *
 *  This is a set of defines to configure the SDL features
 */

/* General platform specific identifiers */
#include "SDL_platform.h"

/* Make sure that this isn't included by Visual C++ */
#ifdef _MSC_VER
#error You should run hg revert SDL_config.h 
#endif

/* C language features */
/* #undef const */
/* #undef inline */
/* #undef volatile */

/* C datatypes */
#ifdef __LP64__
#define SIZEOF_VOIDP 8
#else
#define SIZEOF_VOIDP 4
#endif
#define HAVE_GCC_ATOMICS 1
/* #undef HAVE_GCC_SYNC_LOCK_TEST_AND_SET */
/* #undef HAVE_PTHREAD_SPINLOCK */

/* #undef HAVE_DXGI_H */

/* Comment this if you want to build without any C library requirements */
#define HAVE_LIBC 1
#if HAVE_LIBC

/* Useful headers */
#define HAVE_ALLOCA_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_STDIO_H 1
#define STDC_HEADERS 1
#define HAVE_STDLIB_H 1
#define HAVE_STDARG_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MATH_H 1
#define HAVE_ICONV_H 1
#define HAVE_SIGNAL_H 1
/* #undef HAVE_ALTIVEC_H */
/* #undef HAVE_PTHREAD_NP_H */
/* #undef HAVE_LIBUDEV_H */
/* #undef HAVE_DBUS_DBUS_H */

/* C library functions */
#define HAVE_MALLOC 1
#define HAVE_CALLOC 1
#define HAVE_REALLOC 1
#define HAVE_FREE 1
#define HAVE_ALLOCA 1
#ifndef __WIN32__ /* Don't use C runtime versions of these on Windows */
#define HAVE_GETENV 1
#define HAVE_SETENV 1
#define HAVE_PUTENV 1
#define HAVE_UNSETENV 1
#endif
#define HAVE_QSORT 1
#define HAVE_ABS 1
#define HAVE_BCOPY 1
#define HAVE_MEMSET 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMCMP 1
#define HAVE_STRLEN 1
/* #undef HAVE_STRLCPY */
/* #undef HAVE_STRLCAT */
#define HAVE_STRDUP 1
/* #undef HAVE__STRREV */
/* #undef HAVE__STRUPR */
/* #undef HAVE__STRLWR */
/* #undef HAVE_INDEX */
/* #undef HAVE_RINDEX */
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
/* #undef HAVE_ITOA */
/* #undef HAVE__LTOA */
/* #undef HAVE__UITOA */
/* #undef HAVE__ULTOA */
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
/* #undef HAVE__I64TOA */
/* #undef HAVE__UI64TOA */
#define HAVE_STRTOLL 1
#define HAVE_STRTOULL 1
#define HAVE_STRTOD 1
#define HAVE_ATOI 1
#define HAVE_ATOF 1
#define HAVE_STRCMP 1
#define HAVE_STRNCMP 1
/* #undef HAVE__STRICMP */
#define HAVE_STRCASECMP 1
/* #undef HAVE__STRNICMP */
#define HAVE_STRNCASECMP 1
/* #undef HAVE_SSCANF */
#define HAVE_VSSCANF 1
/* #undef HAVE_SNPRINTF */
#define HAVE_VSNPRINTF 1
#define HAVE_M_PI /**/
#define HAVE_ATAN 1
#define HAVE_ATAN2 1
#define HAVE_ACOS 1
#define HAVE_ASIN 1
#define HAVE_CEIL 1
#define HAVE_COPYSIGN 1
#define HAVE_COS 1
#define HAVE_COSF 1
#define HAVE_FABS 1
#define HAVE_FLOOR 1
#define HAVE_LOG 1
#define HAVE_POW 1
#define HAVE_SCALBN 1
#define HAVE_SIN 1
#define HAVE_SINF 1
#define HAVE_SQRT 1
#define HAVE_SQRTF 1
#define HAVE_TAN 1
#define HAVE_TANF 1
#define HAVE_FSEEKO 1
#define HAVE_FSEEKO64 1
#define HAVE_SIGACTION 1
#define HAVE_SA_SIGACTION 1
#define HAVE_SETJMP 1
#define HAVE_NANOSLEEP 1
#define HAVE_SYSCONF 1
/* #undef HAVE_SYSCTLBYNAME */
#define HAVE_CLOCK_GETTIME 1
/* #undef HAVE_GETPAGESIZE */
#define HAVE_MPROTECT 1
#define HAVE_ICONV 1
/* #undef HAVE_PTHREAD_SETNAME_NP */
/* #undef HAVE_PTHREAD_SET_NAME_NP */
/* #undef HAVE_SEM_TIMEDWAIT */

#else
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#endif /* HAVE_LIBC */

/* SDL internal assertion support */
/* #undef SDL_DEFAULT_ASSERT_LEVEL */

/* Allow disabling of core subsystems */
/* #undef SDL_ATOMIC_DISABLED */
/* #undef SDL_AUDIO_DISABLED */
#define SDL_CPUINFO_DISABLED 1
/* #undef SDL_EVENTS_DISABLED */
/* #undef SDL_FILE_DISABLED */
/* #undef SDL_JOYSTICK_DISABLED */
#define SDL_HAPTIC_DISABLED 1
/* #undef SDL_LOADSO_DISABLED */
/* #undef SDL_RENDER_DISABLED */
#define SDL_THREADS_DISABLED 1
/* #undef SDL_TIMERS_DISABLED */
/* #undef SDL_VIDEO_DISABLED */
/* #undef SDL_POWER_DISABLED */
/* #undef SDL_FILESYSTEM_DISABLED */

/* Enable various audio drivers */
/* #undef SDL_AUDIO_DRIVER_ALSA */
/* #undef SDL_AUDIO_DRIVER_ALSA_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_ARTS */
/* #undef SDL_AUDIO_DRIVER_ARTS_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_PULSEAUDIO */
/* #undef SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_HAIKU */
/* #undef SDL_AUDIO_DRIVER_BSD */
/* #undef SDL_AUDIO_DRIVER_COREAUDIO */
#define SDL_AUDIO_DRIVER_DISK 1
#define SDL_AUDIO_DRIVER_DUMMY 1
/* #undef SDL_AUDIO_DRIVER_ANDROID */
/* #undef SDL_AUDIO_DRIVER_XAUDIO2 */
/* #undef SDL_AUDIO_DRIVER_DSOUND */
/* #undef SDL_AUDIO_DRIVER_ESD */
/* #undef SDL_AUDIO_DRIVER_ESD_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_NACL */
/* #undef SDL_AUDIO_DRIVER_NAS */
/* #undef SDL_AUDIO_DRIVER_NAS_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_SNDIO */
/* #undef SDL_AUDIO_DRIVER_SNDIO_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_OSS */
/* #undef SDL_AUDIO_DRIVER_OSS_SOUNDCARD_H */
/* #undef SDL_AUDIO_DRIVER_PAUDIO */
/* #undef SDL_AUDIO_DRIVER_QSA */
/* #undef SDL_AUDIO_DRIVER_SUNAUDIO */
/* #undef SDL_AUDIO_DRIVER_WINMM */
/* #undef SDL_AUDIO_DRIVER_FUSIONSOUND */
/* #undef SDL_AUDIO_DRIVER_FUSIONSOUND_DYNAMIC */
#define SDL_AUDIO_DRIVER_EMSCRIPTEN 1

/* Enable various input drivers */
/* #undef SDL_INPUT_LINUXEV */
/* #undef SDL_INPUT_LINUXKD */
/* #undef SDL_INPUT_TSLIB */
/* #undef SDL_JOYSTICK_HAIKU */
/* #undef SDL_JOYSTICK_DINPUT */
/* #undef SDL_JOYSTICK_DUMMY */
/* #undef SDL_JOYSTICK_IOKIT */
/* #undef SDL_JOYSTICK_LINUX */
/* #undef SDL_JOYSTICK_ANDROID */
/* #undef SDL_JOYSTICK_WINMM */
/* #undef SDL_JOYSTICK_USBHID */
/* #undef SDL_JOYSTICK_USBHID_MACHINE_JOYSTICK_H */
#define SDL_JOYSTICK_EMSCRIPTEN 1
/* #undef SDL_HAPTIC_DUMMY */
/* #undef SDL_HAPTIC_LINUX */
/* #undef SDL_HAPTIC_IOKIT */
/* #undef SDL_HAPTIC_DINPUT */

/* Enable various shared object loading systems */
/* #undef SDL_LOADSO_HAIKU */
#define SDL_LOADSO_DLOPEN 1
/* #undef SDL_LOADSO_DUMMY */
/* #undef SDL_LOADSO_LDG */
/* #undef SDL_LOADSO_WINDOWS */

/* Enable various threading systems */
/* #undef SDL_THREAD_PTHREAD */
/* #undef SDL_THREAD_PTHREAD_RECURSIVE_MUTEX */
/* #undef SDL_THREAD_PTHREAD_RECURSIVE_MUTEX_NP */
/* #undef SDL_THREAD_WINDOWS */

/* Enable various timer systems */
/* #undef SDL_TIMER_HAIKU */
/* #undef SDL_TIMER_DUMMY */
#define SDL_TIMER_UNIX 1
/* #undef SDL_TIMER_WINDOWS */

/* Enable various video drivers */
/* #undef SDL_VIDEO_DRIVER_HAIKU */
/* #undef SDL_VIDEO_DRIVER_COCOA */
/* #undef SDL_VIDEO_DRIVER_DIRECTFB */
/* #undef SDL_VIDEO_DRIVER_DIRECTFB_DYNAMIC */
#define SDL_VIDEO_DRIVER_DUMMY 1
/* #undef SDL_VIDEO_DRIVER_WINDOWS */
/* #undef SDL_VIDEO_DRIVER_WAYLAND */
/* #undef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */
/* #undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC */
/* #undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC_EGL */
/* #undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC_CURSOR */
/* #undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC_XKBCOMMON */
/* #undef SDL_VIDEO_DRIVER_MIR */
/* #undef SDL_VIDEO_DRIVER_MIR_DYNAMIC */
/* #undef SDL_VIDEO_DRIVER_MIR_DYNAMIC_XKBCOMMON */
/* #undef SDL_VIDEO_DRIVER_X11 */
/* #undef SDL_VIDEO_DRIVER_RPI */
/* #undef SDL_VIDEO_DRIVER_ANDROID */
#define SDL_VIDEO_DRIVER_EMSCRIPTEN 1
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XEXT */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XCURSOR */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XINERAMA */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XINPUT2 */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XRANDR */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XSS */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XVIDMODE */
/* #undef SDL_VIDEO_DRIVER_X11_XCURSOR */
/* #undef SDL_VIDEO_DRIVER_X11_XINERAMA */
/* #undef SDL_VIDEO_DRIVER_X11_XINPUT2 */
/* #undef SDL_VIDEO_DRIVER_X11_XINPUT2_SUPPORTS_MULTITOUCH */
/* #undef SDL_VIDEO_DRIVER_X11_XRANDR */
/* #undef SDL_VIDEO_DRIVER_X11_XSCRNSAVER */
/* #undef SDL_VIDEO_DRIVER_X11_XSHAPE */
/* #undef SDL_VIDEO_DRIVER_X11_XVIDMODE */
/* #undef SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS */
/* #undef SDL_VIDEO_DRIVER_X11_CONST_PARAM_XDATA32 */
/* #undef SDL_VIDEO_DRIVER_X11_CONST_PARAM_XEXTADDDISPLAY */
/* #undef SDL_VIDEO_DRIVER_X11_HAS_XKBKEYCODETOKEYSYM */
/* #undef SDL_VIDEO_DRIVER_NACL */

/* #undef SDL_VIDEO_RENDER_D3D */
/* #undef SDL_VIDEO_RENDER_D3D11 */
/* #undef SDL_VIDEO_RENDER_OGL */
/* #undef SDL_VIDEO_RENDER_OGL_ES */
#define SDL_VIDEO_RENDER_OGL_ES2 1
/* #undef SDL_VIDEO_RENDER_DIRECTFB */

/* Enable OpenGL support */
/* #undef SDL_VIDEO_OPENGL */
/* #undef SDL_VIDEO_OPENGL_ES */
#define SDL_VIDEO_OPENGL_ES2 1
/* #undef SDL_VIDEO_OPENGL_BGL */
/* #undef SDL_VIDEO_OPENGL_CGL */
#define SDL_VIDEO_OPENGL_EGL 1
/* #undef SDL_VIDEO_OPENGL_GLX */
/* #undef SDL_VIDEO_OPENGL_WGL */
/* #undef SDL_VIDEO_OPENGL_OSMESA */
/* #undef SDL_VIDEO_OPENGL_OSMESA_DYNAMIC */

/* Enable system power support */
/* #undef SDL_POWER_LINUX */
/* #undef SDL_POWER_WINDOWS */
/* #undef SDL_POWER_MACOSX */
/* #undef SDL_POWER_HAIKU */
/* #undef SDL_POWER_ANDROID */
#define SDL_POWER_EMSCRIPTEN 1
/* #undef SDL_POWER_HARDWIRED */

/* Enable system filesystem support */
/* #undef SDL_FILESYSTEM_HAIKU */
/* #undef SDL_FILESYSTEM_COCOA */
/* #undef SDL_FILESYSTEM_DUMMY */
/* #undef SDL_FILESYSTEM_UNIX */
/* #undef SDL_FILESYSTEM_WINDOWS */
/* #undef SDL_FILESYSTEM_NACL */
#define SDL_FILESYSTEM_EMSCRIPTEN 1

/* Enable assembly routines */
/* #undef SDL_ASSEMBLY_ROUTINES */
/* #undef SDL_ALTIVEC_BLITTERS */

#endif /* _SDL_config_h */
'''

