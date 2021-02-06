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

deps_info = {
  '$getTypeName': ['free'],
  'Mix_LoadWAV_RW': ['fileno'],
  'SDL_CreateRGBSurface': ['malloc', 'free'],
  'SDL_GL_GetProcAddress': ['emscripten_GetProcAddress'],
  'SDL_Init': ['malloc', 'free', 'memset', 'memcpy'],
  'SDL_LockSurface': ['malloc', 'free'],
  'SDL_OpenAudio': ['malloc', 'free'],
  'SDL_PushEvent': ['malloc', 'free'],
  'SDL_free': ['free'],
  'SDL_getenv': ['malloc', 'free'],
  'SDL_malloc': ['malloc', 'free'],
  '__ctype_b_loc': ['malloc', 'free'],
  '__ctype_tolower_loc': ['malloc', 'free'],
  '__ctype_toupper_loc': ['malloc', 'free'],
  '__cxa_allocate_exception': ['malloc', 'free', 'setThrew'],
  '__cxa_begin_catch': ['_ZSt18uncaught_exceptionv', 'setThrew'],
  '__cxa_end_catch': ['free'],
  '__cxa_find_matching_catch': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_0': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_1': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_2': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_3': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_4': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_5': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_6': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_7': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_8': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_find_matching_catch_9': ['__cxa_is_pointer_type', '__cxa_can_catch'],
  '__cxa_free_exception': ['free'],
  '__cxa_throw': ['setThrew'],
  '__gxx_personality_v0': ['_ZSt18uncaught_exceptionv', '__cxa_find_matching_catch'],
  '_embind_register_class': ['free'],
  '_embind_register_enum_value': ['free'],
  '_embind_register_function': ['free'],
  '_embind_register_std_string': ['free'],
  '_embind_register_std_wstring': ['free'],
  '_inet_ntop6_raw': ['ntohs'],
  'alGetProcAddress': ['emscripten_GetAlProcAddress'],
  'alGetString': ['malloc', 'free'],
  'alcGetProcAddress': ['emscripten_GetAlcProcAddress'],
  'alcGetString': ['malloc', 'free'],
  'bind': ['htonl', 'htons', 'ntohs'],
  'calloc': ['malloc', 'free'],
  'connect': ['htonl', 'htons', 'ntohs'],
  'ctime': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'ctime_r': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'dladdr': ['malloc', 'free'],
  'dlerror': ['malloc', 'free'],
  'eglGetProcAddress': ['emscripten_GetProcAddress'],
  'eglQueryString': ['malloc', 'free'],
  'emscripten_GetAlProcAddress': ['strcmp'],
  'emscripten_GetAlcProcAddress': ['strcmp'],
  'emscripten_GetProcAddress': ['strstr'],
  'emscripten_SDL_SetEventHandler': ['malloc', 'free'],
  'emscripten_alcGetStringiSOFT': ['malloc', 'free'],
  'emscripten_async_wget2_data': ['malloc', 'free'],
  'emscripten_async_wget_data': ['malloc', 'free'],
  'emscripten_create_worker': ['malloc', 'free'],
  'emscripten_fetch': ['emscripten_is_main_browser_thread'],
  'emscripten_get_compiler_setting': ['malloc', 'free'],
  'emscripten_get_preloaded_image_data': ['malloc', 'free'],
  'emscripten_get_preloaded_image_data_from_FILE': ['fileno'],
  'emscripten_get_window_title': ['malloc', 'free'],
  'emscripten_idb_async_load': ['malloc', 'free'],
  'emscripten_idb_load': ['malloc', 'free'],
  'emscripten_init_websocket_to_posix_socket_bridge': ['malloc', 'free'],
  'emscripten_log': ['strlen'],
  'emscripten_longjmp': ['setThrew', 'realloc', 'testSetjmp', 'saveSetjmp'],
  'emscripten_pc_get_file': ['malloc', 'free'],
  'emscripten_pc_get_function': ['malloc', 'free'],
  'emscripten_run_preload_plugins_data': ['malloc', 'free'],
  'emscripten_run_script_string': ['malloc', 'free'],
  'emscripten_set_batterychargingchange_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_batterylevelchange_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_beforeunload_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_blur_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_canvas_element_size_calling_thread': ['_emscripten_call_on_thread'],
  'emscripten_set_click_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_dblclick_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_devicemotion_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_deviceorientation_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_focus_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_focusin_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_focusout_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_fullscreenchange_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_gamepadconnected_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_gamepaddisconnected_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_keydown_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_keypress_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_keyup_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mousedown_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mouseenter_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mouseleave_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mousemove_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mouseout_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mouseover_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_mouseup_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_offscreencanvas_size_on_target_thread': ['_emscripten_call_on_thread', 'malloc', 'free'],
  'emscripten_set_offscreencanvas_size_on_target_thread_js': ['malloc', 'free'],
  'emscripten_set_orientationchange_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_pointerlockchange_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_pointerlockerror_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_resize_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_scroll_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_touchcancel_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_touchend_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_touchmove_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_touchstart_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_visibilitychange_callback_on_thread': ['malloc', 'free'],
  'emscripten_set_wheel_callback_on_thread': ['malloc', 'free'],
  'emscripten_start_fetch': ['emscripten_is_main_browser_thread'],
  'emscripten_webgl_create_context': ['malloc', 'free'],
  'emscripten_webgl_destroy_context': ['emscripten_webgl_make_context_current', 'emscripten_webgl_get_current_context'],
  'emscripten_webgl_get_parameter_utf8': ['malloc', 'free'],
  'emscripten_webgl_get_program_info_log_utf8': ['malloc', 'free'],
  'emscripten_webgl_get_shader_info_log_utf8': ['malloc', 'free'],
  'emscripten_webgl_get_shader_source_utf8': ['malloc', 'free'],
  'emscripten_webgl_get_supported_extensions': ['malloc', 'free'],
  'emscripten_websocket_new': ['malloc', 'free'],
  'emscripten_websocket_set_onclose_callback_on_thread': ['malloc', 'free'],
  'emscripten_websocket_set_onerror_callback_on_thread': ['malloc', 'free'],
  'emscripten_websocket_set_onmessage_callback_on_thread': ['malloc', 'free'],
  'emscripten_websocket_set_onopen_callback_on_thread': ['malloc', 'free'],
  'emscripten_wget_data': ['malloc', 'free'],
  'formatString': ['strlen'],
  'freeaddrinfo': ['free'],
  'freelocale': ['free'],
  'freopen': ['free'],
  'gai_strerror': ['malloc', 'free'],
  'getaddrinfo': ['malloc', 'free', 'htonl', 'htons', 'ntohs'],
  'getenv': ['malloc', 'free'],
  'gethostbyname': ['malloc', 'free', 'htons', 'ntohs', 'memcpy'],
  'gethostbyname_r': ['malloc', 'free', 'htons', 'ntohs', 'memcpy'],
  'getlogin': ['malloc', 'free'],
  'getnameinfo': ['htons', 'ntohs'],
  'getpeername': ['htons', 'ntohs'],
  'getrusage': ['memset'],
  'glBegin': ['malloc', 'free'],
  'glGetString': ['malloc', 'free'],
  'glGetStringi': ['malloc', 'free'],
  'glMapBufferRange': ['malloc', 'free'],
  'glewInit': ['malloc', 'free'],
  'glfwGetProcAddress': ['emscripten_GetProcAddress'],
  'glfwInit': ['malloc', 'free'],
  'glfwSleep': ['sleep'],
  'gmtime':  ['malloc', 'free'],
  'gmtime_r':  ['malloc', 'free'],
  'inet_ntoa': ['malloc', 'free'],
  'localtime': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'localtime_r': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'longjmp': ['setThrew', 'realloc', 'testSetjmp', 'saveSetjmp'],
  'mktime': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'mmap': ['memalign', 'memset', 'malloc', 'free'],
  'munmap': ['memalign', 'memset', 'malloc', 'free'],
  'newlocale': ['malloc', 'free'],
  'nl_langinfo': ['malloc', 'free'],
  'pthread_create': ['malloc', 'free', 'emscripten_main_thread_process_queued_calls'],
  'readdir': ['malloc', 'free'],
  'realloc': ['malloc', 'free'],
  'realpath': ['malloc', 'free'],
  'recv': ['htons', 'ntohs'],
  'send': ['htons', 'ntohs'],
  'setjmp': ['setThrew', 'realloc', 'testSetjmp', 'saveSetjmp'],
  'setprotoent': ['malloc', 'free'],
  'siglongjmp': ['setThrew', 'realloc', 'testSetjmp', 'saveSetjmp'],
  'sleep': ['usleep'],
  'socket': ['htonl', 'htons', 'ntohs'],
  'socketpair': ['htons', 'ntohs'],
  'strerror': ['malloc', 'free'],
  'stringToNewUTF8': ['malloc', 'free'],
  'syslog': ['malloc', 'htons', 'ntohs'],
  'timegm': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'times': ['memset'],
  'tmpnam': ['malloc', 'free'],
  'ttyname': ['malloc', 'free'],
  'tzset': ['_get_tzname', '_get_daylight', '_get_timezone', 'malloc', 'free'],
  'uuid_clear': ['memset'],
  'uuid_compare': ['memcmp', 'memcpy', 'memset'],
  'uuid_copy': ['memcpy'],
  'wgpuDeviceCreateBuffer': ['malloc', 'free'],
}
