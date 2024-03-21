# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
from typing import Dict

TAG = '3.4.0.20240318'
HASH = 'fb90b43d74c234b5534c8936973c1f98e8843c16fae81ce4e1415ea947d1364aa7a167eed4cf263150f1df0f137319d0151e8fe39dc00d4eeb0c3ad48ede03f3'

# contrib port information (required)
URL = 'https://github.com/pongasoft/emscripten-glfw'
DESCRIPTION = 'This project is an emscripten port of GLFW written in C++ for the web/webassembly platform'
LICENSE = 'Apache 2.0 license'

OPTIONS = {
  'disableWarning': 'Boolean to disable warnings emitted by the library',
  'disableJoystick': 'Boolean to disable support for joystick entirely',
  'disableMultiWindow': 'Boolean to disable multi window support which makes the code smaller and faster'
}

# user options (from --use-port)
opts: Dict[str, bool] = {
  'disableWarning': False,
  'disableJoystick': False,
  'disableMultiWindow': False
}


def get_lib_name(settings):
  return ('lib_contrib.glfw3' +
          ('-nw' if opts['disableWarning'] else '') +
          ('-nj' if opts['disableJoystick'] else '') +
          ('-sw' if opts['disableMultiWindow'] else '') +
          '.a')


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('contrib.glfw3',
                      f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip',
                      sha512hash=HASH)

  def create(final):
    root_path = os.path.join(ports.get_dir(), 'contrib.glfw3')
    source_path = os.path.join(root_path, 'src', 'cpp')
    source_include_paths = [os.path.join(root_path, 'external'), os.path.join(root_path, 'include')]
    for source_include_path in source_include_paths:
      ports.install_headers(source_include_path, pattern="GLFW/*.h", target='contrib.glfw3')

    flags = []

    if opts['disableWarning']:
      flags += ['-DEMSCRIPTEN_GLFW3_DISABLE_WARNING']

    if opts['disableJoystick']:
      flags += ['-DEMSCRIPTEN_GLFW3_DISABLE_JOYSTICK']

    if opts['disableMultiWindow']:
      flags += ['-DEMSCRIPTEN_GLFW3_DISABLE_MULTI_WINDOW_SUPPORT']

    ports.build_port(source_path, final, 'contrib.glfw3', includes=source_include_paths, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def linker_setup(ports, settings):
  root_path = os.path.join(ports.get_dir(), 'contrib.glfw3')
  source_js_path = os.path.join(root_path, 'src', 'js', 'lib_emscripten_glfw3.js')
  settings.JS_LIBRARIES += [source_js_path]


# Using contrib.glfw3 to avoid installing headers into top level include path
# so that we don't conflict with the builtin GLFW headers that emscripten
# includes
def process_args(ports):
  return ['-isystem', ports.get_include_dir('contrib.glfw3')]


def handle_options(options, error_handler):
  for option, value in options.items():
    if value.lower() in {'true', 'false'}:
      opts[option] = value.lower() == 'true'
    else:
      error_handler(f'{option} is expecting a boolean, got {value}')
