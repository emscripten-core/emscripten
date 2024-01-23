# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '1.0.3'
HASH = '0ddb6385832b2bbf211fddc954f9d7e42712b6da940370900352b7a2d60de487643ce7c0b0c1aa93fd039176a2442a9fa4e1da4215338a111f13ea7eccfd35d1'

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

    ports.build_port(source_path, final, 'glfw3', includes=source_include_paths)

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
