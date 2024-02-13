# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
from typing import Dict, Optional

OPTIONS = {
  'value1': 'Value for define TEST_VALUE_1',
  'value2': 'Value for define TEST_VALUE_2',
  'dependency': 'A dependency'
}

# user options (from --use-port)
opts: Dict[str, Optional[str]] = {
  'value1': None,
  'value2': None,
  'dependency': None
}

deps = []


def get_lib_name(settings):
  return 'lib_external.a'


def get(ports, settings, shared):
  # for simplicity in testing, the source is in the same folder as the port and not fetched as a tarball
  source_dir = os.path.dirname(os.path.abspath(__file__))

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
  return args


def process_dependencies(settings):
  if opts['dependency']:
    deps.append(opts['dependency'])


def handle_options(options):
  opts.update(options)
