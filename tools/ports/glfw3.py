# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '1.0.4'
HASH = 'c3c96718e5d2b37df434a46c4a93ddfd9a768330d33f0d6ce2d08c139752894c2421cdd0fefb800fe41fafc2bbe58c8f22b8aa2849dc4fc6dde686037215cfad'


def needed(settings):
  return settings.USE_GLFW_PORT == 3


lib_name = 'libglfw3.a'


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('glfw3', f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip', sha512hash=HASH)

  def create(final):
    root_path = os.path.join(ports.get_dir(), 'glfw3')
    source_path = os.path.join(root_path, 'src', 'cpp')
    source_include_paths = [os.path.join(root_path, 'external', 'GLFW'), os.path.join(root_path, 'include', 'GLFW')]
    for source_include_path in source_include_paths:
      ports.install_headers(source_include_path, target='GLFW')

    # It would be great if the user could configure the build of the port, but I am
    # not sure if it is possible, so removing warnings
    flags = [ '-DEMSCRIPTEN_GLFW3_DISABLE_WARNING' ]

    ports.build_port(source_path, final, 'glfw3', includes=source_include_paths, flags=flags)

  return [shared.cache.get_lib(lib_name, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(lib_name)


def linker_setup(ports, settings):
  root_path = os.path.join(ports.get_dir(), 'glfw3')
  source_js_path = os.path.join(root_path, 'src', 'js', 'lib_emscripten_glfw3.js')
  settings.JS_LIBRARIES += [source_js_path]


def process_args(ports):
  return ['-isystem', ports.get_include_dir('glfw3')]


def show():
  return 'GLFW3 (USE_GLFW=3X; Apache 2.0 license)'
