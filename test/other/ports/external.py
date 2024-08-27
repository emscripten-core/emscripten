# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
from typing import Dict, Optional

URL = 'https://emscripten.org'
DESCRIPTION = 'Test Description'
LICENSE = 'Test License'

OPTIONS = {
  'value1': 'Value for define TEST_VALUE_1',
  'value2': 'Value for define TEST_VALUE_2',
  'value3': 'String value',
  'dependency': 'A dependency',
}

# user options (from --use-port)
opts: Dict[str, Optional[str]] = {
  'value1': None,
  'value2': None,
  'value3': "v3",
  'dependency': None
}

deps = ['sdl2_image:formats=jpg']


def get_lib_name(settings):
  if opts['dependency']:
    return f'lib_external-{opts["dependency"]}.a'
  else:
    return 'lib_external.a'


def get(ports, settings, shared):
  # for simplicity in testing, the source is in the same folder as the port and not fetched as a tarball
  source_dir = os.path.dirname(__file__)

  def create(final):
    ports.install_headers(source_dir)
    print(f'about to build {source_dir}')
    ports.build_port(source_dir, final, 'external')

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_args(ports):
  args = ['-isystem', ports.get_include_dir('external')]
  if opts['value1']:
    args.append(f'-DTEST_VALUE_1={opts["value1"]}')
  if opts['value2']:
    args.append(f'-DTEST_VALUE_2={opts["value2"]}')
  if opts['dependency']:
    args.append(f'-DTEST_DEPENDENCY_{opts["dependency"].upper()}')
  args.append(f'-DTEST_VALUE_3="{opts["value3"]}"')
  return args


def process_dependencies(settings):
  if opts['dependency']:
    deps.append(opts['dependency'])


def handle_options(options, error_handler):
  opts.update(options)
