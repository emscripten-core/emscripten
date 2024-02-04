# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '1.0.4'
HASH = 'c3c96718e5d2b37df434a46c4a93ddfd9a768330d33f0d6ce2d08c139752894c2421cdd0fefb800fe41fafc2bbe58c8f22b8aa2849dc4fc6dde686037215cfad'

# contrib port information (required)
URL = 'https://github.com/pongasoft/emscripten-glfw'
DESCRIPTION = 'This project is an emscripten port of glfw written in C++ for the web/webassembly platform'
LICENSE = 'Apache 2.0 license'

# name is set when the port is read
name = ''


def get_lib_name(settings):
  return f'lib_{name}.a'


def get(ports, settings, shared):
  # get the port
  ports.fetch_project(name, f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip', sha512hash=HASH)

  def create(final):
    root_path = os.path.join(ports.get_dir(), name)
    source_path = os.path.join(root_path, 'src', 'cpp')
    source_include_paths = [os.path.join(root_path, 'external', 'GLFW'), os.path.join(root_path, 'include', 'GLFW')]
    for source_include_path in source_include_paths:
      ports.install_headers(source_include_path, target='GLFW')

    # this should be an option but better to disable for now...
    flags = ['-DEMSCRIPTEN_GLFW3_DISABLE_WARNING']

    ports.build_port(source_path, final, name, includes=source_include_paths, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def linker_setup(ports, settings):
  root_path = os.path.join(ports.get_dir(), name)
  source_js_path = os.path.join(root_path, 'src', 'js', 'lib_emscripten_glfw3.js')
  settings.JS_LIBRARIES += [source_js_path]


def process_args(ports):
  return ['-isystem', ports.get_include_dir(name)]
