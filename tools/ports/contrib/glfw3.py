# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '1.0.4'
HASH = 'c3c96718e5d2b37df434a46c4a93ddfd9a768330d33f0d6ce2d08c139752894c2421cdd0fefb800fe41fafc2bbe58c8f22b8aa2849dc4fc6dde686037215cfad'

options = {
  'contrib.glfw3:DISABLE_WARNING': 'Disable all warnings',
  'contrib.glfw3:DISABLE_JOYSTICK': 'Disable support for joystick (due to polling, it can help to disable joystick ' +
                                    'support if not needed)',
  'contrib.glfw3:DISABLE_MULTI_WINDOW_SUPPORT': 'Disable support for multiple windows if not needed',
}


def get_lib_name(settings):
  return ('libglfw3' +
          ('-nw' if 'contrib.glfw3:DISABLE_WARNING' in settings.PORT_OPTIONS else '') +
          ('-nj' if 'contrib.glfw3:DISABLE_JOYSTICK' in settings.PORT_OPTIONS else '') +
          ('-sw' if 'contrib.glfw3:DISABLE_MULTI_WINDOW_SUPPORT' in settings.PORT_OPTIONS else '') +
          '.a')


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('glfw3', f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip', sha512hash=HASH)

  def create(final):
    root_path = os.path.join(ports.get_dir(), 'glfw3')
    source_path = os.path.join(root_path, 'src', 'cpp')
    source_include_paths = [os.path.join(root_path, 'external', 'GLFW'), os.path.join(root_path, 'include', 'GLFW')]
    for source_include_path in source_include_paths:
      ports.install_headers(source_include_path, target='GLFW')

    flags = []

    if 'contrib.glfw3:DISABLE_WARNING' in settings.PORT_OPTIONS:
      flags += ['-DEMSCRIPTEN_GLFW3_DISABLE_WARNING']

    if 'contrib.glfw3:DISABLE_JOYSTICK' in settings.PORT_OPTIONS:
      flags += ['-DEMSCRIPTEN_GLFW3_DISABLE_JOYSTICK']

    if 'contrib.glfw3:DISABLE_MULTI_WINDOW_SUPPORT' in settings.PORT_OPTIONS:
      flags += ['-DEMSCRIPTEN_GLFW3_DISABLE_MULTI_WINDOW_SUPPORT']

    ports.build_port(source_path, final, 'glfw3', includes=source_include_paths, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def linker_setup(ports, settings):
  root_path = os.path.join(ports.get_dir(), 'glfw3')
  source_js_path = os.path.join(root_path, 'src', 'js', 'lib_emscripten_glfw3.js')
  settings.JS_LIBRARIES += [source_js_path]


def process_args(ports):
  return ['-isystem', ports.get_include_dir('glfw3')]


def project_url():
  return 'https://github.com/pongasoft/emscripten-glfw'


def project_description():
  return 'This project is an emscripten port of glfw written in C++ for the web/webassembly platform'


def project_license():
  return 'Apache 2.0 license'
