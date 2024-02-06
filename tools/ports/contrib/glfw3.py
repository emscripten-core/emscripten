# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
from urllib.parse import parse_qs

# contrib port information (required)
URL = 'https://github.com/pongasoft/emscripten-glfw'
DESCRIPTION = 'This project is an emscripten port of GLFW written in C++ for the web/webassembly platform'
LICENSE = 'Apache 2.0 license'

OPTIONS = {
  'tag': 'The git tag to use a different release',
  'hash': 'The sha512 of the release associated to the tag (can be omitted)',
  'disableWarning': 'Boolean to disable warnings emitted by the library',
  'disableJoystick': 'Boolean to disable support for joystick entirely',
  'disableMultiWindow': 'Boolean to disable multi window support which makes the code smaller and faster'
}

# user options (from --use-port)
opts = {
  'tag': '1.0.4',
  'hash': 'c3c96718e5d2b37df434a46c4a93ddfd9a768330d33f0d6ce2d08c139752894c2421cdd0fefb800fe41fafc2bbe58c8f22b8aa2849dc4fc6dde686037215cfad',
  'disableWarning': False,
  'disableJoystick': False,
  'disableMultiWindow': False
}


def get_lib_name(settings):
  return (f'lib_contrib.glfw3_{opts["tag"]}' +
          ('-nw' if opts['disableWarning'] else '') +
          ('-nj' if opts['disableJoystick'] else '') +
          ('-sw' if opts['disableMultiWindow'] else '') +
          '.a')


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('contrib.glfw3',
                      f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{opts["tag"]}/emscripten-glfw3-{opts["tag"]}.zip',
                      sha512hash=opts['hash'])

  def create(final):
    root_path = os.path.join(ports.get_dir(), 'contrib.glfw3')
    source_path = os.path.join(root_path, 'src', 'cpp')
    source_include_paths = [os.path.join(root_path, 'external', 'GLFW'), os.path.join(root_path, 'include', 'GLFW')]
    for source_include_path in source_include_paths:
      ports.install_headers(source_include_path, target='GLFW')

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


def handle_options(options):
  try:
    oqs = parse_qs(options, strict_parsing=True)
  except ValueError as error:
    return f'{options} is not valid: {error}. Available options are {OPTIONS}.'

  if not set(oqs.keys()).issubset(OPTIONS.keys()):
    return f'{options} is not valid. Available options are {OPTIONS}.'

  for option, values in oqs.items():
    value = values[-1]  # ignore multiple definitions (last one wins)
    if isinstance(opts[option], bool):
      if value.lower() in {'true', 'false'}:
        opts[option] = value.lower() == 'true'
      else:
        return f'{option} is expecting a boolean, got {value}'
    else:
      opts[option] = value

  # in the event that only 'tag' is provided, clear 'hash'
  if 'tag' in oqs and not 'hash' in oqs:
    opts['hash'] = None
