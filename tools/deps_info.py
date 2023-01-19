# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# deps_info is a mechanism that lets JS code depend on C functions. This
# needs special help because of how linking works:
#
#   1. Receive some input files (.o, .c, etc.) from the user.
#   2. Link them with system libraries.
#   3. Whatever symbols are still unresolved, look in JS libraries for them.
#
# This makes C->JS calls work in a natural way: if compiled code depends on
# a function foo() that is implemented in a JS library, it will be unresolved
# after stage 2, and therefore linked in at stage 3. The problem is the other
# direction: if a JS library function decides it needs some function from say
# libc, then at stage 3 it is too late to link in more libc code. That's
# where deps_info comes in.
#
# Specifically, before stage 2 (linking with system libraries) we look at what
# symbols are required by the input files. Imagine that js_func in a JS
# library depends on libc_func in libc. Then if deps_info tells us
#
#  "js_func": ["libc_func"]
#
# then if we see js_func is required (undefined) before stage 2, then we add
# a requirement to link in libc_func when linking in system libraries. All
# we do with deps_info is see if any of the keys are among the
# undefined symbols before stage 2, and if they are, add their values to the
# things we need to link in.
#
# This usually works the way you want, but note that it happens *before* stage
# 2 and not after it. That is, we look for js_func before linking in system
# libraries. If you have a situation where
#
#   user_code => other_libc_func => js_func => libc_func
#
# then the deps_info entry must contain
#
#  "other_libc_func": ["libc_func"]
#
# because that is what we see before stage 2: all we see is that
# other_libc_func is going to be linked in, and we don't know yet that it
# will end up calling js_func. But the presence of a call to other_libc_func
# indicates that we will need libc_func linked in as well, so that is what the
# deps_info entry should contain.
#
# TODO: Move all __deps from src/library*.js to deps_info, and use that single source of info
#       both here and in the JS compiler.

from tools.settings import settings

_deps_info = {
  'alarm': ['_emscripten_timeout'],
  'setitimer': ['_emscripten_timeout'],
  'Mix_LoadWAV_RW': ['fileno'],
  'SDL_CreateRGBSurface': ['malloc', 'free'],
  'SDL_GL_GetProcAddress': ['malloc'],
  'SDL_Init': ['malloc', 'free', 'memcpy'],
  'SDL_LockSurface': ['malloc', 'free'],
  'SDL_OpenAudio': ['malloc', 'free'],
  'SDL_PushEvent': ['malloc', 'free'],
  'SDL_free': ['free'],
  'SDL_malloc': ['malloc', 'free'],
  '_embind_register_class': ['free'],
  '_embind_register_enum_value': ['free'],
  '_embind_register_function': ['free'],
  '_embind_register_std_string': ['free'],
  '_embind_register_std_wstring': ['free'],
  'alGetString': ['malloc'],
  'alcGetString': ['malloc'],
  'bind': ['ntohs', 'htons'],
  'connect': ['ntohs', 'htons'],
  'ctime': ['malloc'],
  'gmtime': ['malloc'],
  'ctime_r': ['malloc'],
  'dladdr': ['malloc'],
  'dlopen': ['__dl_seterr', 'malloc'],
  'dlsym': ['__dl_seterr'],
  'eglGetProcAddress': ['malloc'],
  'eglQueryString': ['malloc'],
  'emscripten_GetProcAddress': ['malloc'],
  'emscripten_SDL_SetEventHandler': ['malloc', 'free'],
  'emscripten_alcGetStringiSOFT': ['malloc'],
  'emscripten_async_wget2_data': ['malloc', 'free'],
  'emscripten_async_wget_data': ['malloc', 'free'],
  'emscripten_create_worker': ['malloc', 'free'],
  'emscripten_get_compiler_setting': ['malloc'],
  'emscripten_get_preloaded_image_data': ['malloc'],
  'emscripten_get_preloaded_image_data_from_FILE': ['fileno'],
  'emscripten_get_window_title': ['malloc'],
  'emscripten_idb_async_load': ['malloc', 'free'],
  'emscripten_idb_load': ['malloc', 'free'],
  'emscripten_init_websocket_to_posix_socket_bridge': ['malloc', 'free'],
  # This list is the same as setjmp's dependencies. In non-LTO builds, setjmp
  # does not exist in the object files; it is converted into a code sequence
  # that includes several functions, one of which is emscripten_longjmp. This is
  # a trick to include these dependencies for setjmp even when setjmp does not
  # exist. Refer to setjmp's entry for more details.
  'emscripten_longjmp': ['malloc', 'free', 'saveSetjmp', 'setThrew'],
  'emscripten_pc_get_file': ['malloc', 'free'],
  'emscripten_pc_get_function': ['malloc', 'free'],
  'emscripten_run_preload_plugins_data': ['malloc'],
  'emscripten_run_script_string': ['malloc', 'free'],
  'emscripten_set_batterychargingchange_callback_on_thread': ['malloc'],
  'emscripten_set_batterylevelchange_callback_on_thread': ['malloc'],
  'emscripten_set_blur_callback_on_thread': ['malloc'],
  'emscripten_set_click_callback_on_thread': ['malloc'],
  'emscripten_set_dblclick_callback_on_thread': ['malloc'],
  'emscripten_set_devicemotion_callback_on_thread': ['malloc'],
  'emscripten_set_deviceorientation_callback_on_thread': ['malloc'],
  'emscripten_set_focus_callback_on_thread': ['malloc'],
  'emscripten_set_focusin_callback_on_thread': ['malloc'],
  'emscripten_set_focusout_callback_on_thread': ['malloc'],
  'emscripten_set_fullscreenchange_callback_on_thread': ['malloc'],
  'emscripten_set_gamepadconnected_callback_on_thread': ['malloc'],
  'emscripten_set_gamepaddisconnected_callback_on_thread': ['malloc'],
  'emscripten_set_keydown_callback_on_thread': ['malloc'],
  'emscripten_set_keypress_callback_on_thread': ['malloc'],
  'emscripten_set_keyup_callback_on_thread': ['malloc'],
  'emscripten_set_mousedown_callback_on_thread': ['malloc'],
  'emscripten_set_mouseenter_callback_on_thread': ['malloc'],
  'emscripten_set_mouseleave_callback_on_thread': ['malloc'],
  'emscripten_set_mousemove_callback_on_thread': ['malloc'],
  'emscripten_set_mouseout_callback_on_thread': ['malloc'],
  'emscripten_set_mouseover_callback_on_thread': ['malloc'],
  'emscripten_set_mouseup_callback_on_thread': ['malloc'],
  'emscripten_set_orientationchange_callback_on_thread': ['malloc'],
  'emscripten_set_pointerlockchange_callback_on_thread': ['malloc'],
  'emscripten_set_resize_callback_on_thread': ['malloc'],
  'emscripten_set_scroll_callback_on_thread': ['malloc'],
  'emscripten_set_touchcancel_callback_on_thread': ['malloc'],
  'emscripten_set_touchend_callback_on_thread': ['malloc'],
  'emscripten_set_touchmove_callback_on_thread': ['malloc'],
  'emscripten_set_touchstart_callback_on_thread': ['malloc'],
  'emscripten_set_visibilitychange_callback_on_thread': ['malloc'],
  'emscripten_set_wheel_callback_on_thread': ['malloc'],
  'emscripten_webgl_get_parameter_utf8': ['malloc'],
  'emscripten_webgl_get_program_info_log_utf8': ['malloc'],
  'emscripten_webgl_get_shader_info_log_utf8': ['malloc'],
  'emscripten_webgl_get_shader_source_utf8': ['malloc'],
  'emscripten_webgl_get_supported_extensions': ['malloc'],
  'emscripten_websocket_set_onclose_callback_on_thread': ['malloc'],
  'emscripten_websocket_set_onerror_callback_on_thread': ['malloc'],
  'emscripten_websocket_set_onmessage_callback_on_thread': ['malloc', 'free'],
  'emscripten_websocket_set_onopen_callback_on_thread': ['malloc'],
  'emscripten_wget_data': ['malloc', 'free'],
  'getaddrinfo': ['malloc', 'htonl', 'htons', 'ntohs'],
  'gethostbyaddr': ['malloc', 'htons'],
  'gethostbyname': ['malloc', 'htons'],
  'gethostbyname_r': ['malloc', 'free', 'htons', 'memcpy'],
  'getnameinfo': ['htons', 'ntohs'],
  'getpeername': ['htons'],
  'getsockname': ['htons'],
  'glGetString': ['malloc'],
  'glGetStringi': ['malloc'],
  'glMapBufferRange': ['malloc'],
  'glewInit': ['malloc'],
  'glfwGetProcAddress': ['malloc'],
  'glfwInit': ['malloc', 'free'],
  'glfwSleep': ['sleep'],
  'glfwGetMonitors': ['malloc'],
  'localtime': ['malloc'],
  'localtime_r': ['malloc'],
  'mktime': ['malloc'],
  'recv': ['htons'],
  'recvmsg': ['htons'],
  'accept': ['htons'],
  'recvfrom': ['htons'],
  'send': ['ntohs', 'htons'],
  'sendmsg': ['ntohs', 'htons'],
  # In WebAssemblyLowerEmscriptenEHSjLj pass in the LLVM backend, function calls
  # that exist in the same function with setjmp are converted to some code
  # sequence that includes invokes, malloc, free, saveSetjmp, and
  # emscripten_longjmp. setThrew is called from invokes, but there's no way to
  # directly include invokes in deps_info.py, so we list it as a setjmp's
  # dependency.
  'setjmp': ['malloc', 'free', 'saveSetjmp', 'setThrew'],
  'setprotoent': ['malloc'],
  'syslog': ['malloc', 'ntohs', 'htons'],
  'vsyslog': ['malloc', 'ntohs', 'htons'],
  'timegm': ['malloc'],
  'tzset': ['malloc'],
  'uuid_compare': ['memcmp'],
  'uuid_copy': ['memcpy'],
  'wgpuBufferGetMappedRange': ['malloc', 'free'],
  'wgpuBufferGetConstMappedRange': ['malloc', 'free'],
  'emscripten_glGetString': ['malloc'],
}


def get_deps_info():
  if not settings.WASM_EXCEPTIONS and settings.LINK_AS_CXX:
    _deps_info['__cxa_begin_catch'] = ['__cxa_is_pointer_type', '__cxa_free_exception']
    _deps_info['__cxa_throw'] = ['__cxa_is_pointer_type']
    _deps_info['__cxa_find_matching_catch'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_1'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_2'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_3'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_4'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_5'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_6'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_7'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_8'] = ['__cxa_can_catch', 'setTempRet0']
    _deps_info['__cxa_find_matching_catch_9'] = ['__cxa_can_catch', 'setTempRet0']
  if settings.USE_PTHREADS and settings.OFFSCREENCANVAS_SUPPORT:
    _deps_info['pthread_create'] = ['malloc']
  if settings.FILESYSTEM and settings.SYSCALLS_REQUIRE_FILESYSTEM:
    _deps_info['mmap'] = ['emscripten_builtin_memalign']
  if settings.USE_PTHREADS:
    _deps_info['glutCreateWindow'] = ['malloc']
    _deps_info['emscripten_webgl_create_context'] = ['malloc']
    _deps_info['emscripten_webgl_destroy_context'] = ['free']
    _deps_info['emscripten_set_canvas_element_size_calling_thread'] = ['emscripten_dispatch_to_thread_']
    if settings.OFFSCREEN_FRAMEBUFFER:
      # When OFFSCREEN_FRAMEBUFFER is defined these functions are defined in native code,
      # otherwise they are defined in src/library_html5_webgl.js.
      _deps_info['emscripten_webgl_destroy_context'] += ['emscripten_webgl_make_context_current', 'emscripten_webgl_get_current_context']
    if settings.OFFSCREENCANVAS_SUPPORT:
      _deps_info['emscripten_set_offscreencanvas_size_on_target_thread'] = ['emscripten_dispatch_to_thread_', 'malloc', 'free']
      _deps_info['emscripten_set_offscreencanvas_size_on_target_thread_js'] = ['malloc']

  return _deps_info
