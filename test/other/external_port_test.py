# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
from typing import Dict, Optional

URL = 'https://github.com/emscripten-core/emscripten'
DESCRIPTION = 'External Ports Test'
LICENSE = 'MIT license'

OPTIONS = {
  'value1': 'Value for define TEST_VALUE_1',
  'value2': 'Value for define TEST_VALUE_2',
}

# user options (from --use-port)
opts: Dict[str, Optional[str]] = {
  'value1': None,
  'value2': None,
}

EXAMPLE_H = 'int external_port_test_fn(int);'
EXAMPLE_C = 'int external_port_test_fn(int value) { return value; }'



def get_lib_name(settings):
  return f'lib_{name}.a'


def get(ports, settings, shared):
  source_path = os.path.join(ports.get_dir(), name)
  os.makedirs(source_path, exist_ok=True)

  def create(final):
    ports.write_file(os.path.join(source_path, 'external_port_test.h'), EXAMPLE_H)
    ports.write_file(os.path.join(source_path, 'external_port_test.c'), EXAMPLE_C)
    ports.install_headers(source_path)
    print(f'about to build {source_path}')
    ports.build_port(source_path, final, name)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_args(ports):
  args = ['-isystem', ports.get_include_dir(name)]
  if opts['value1']:
    args.append(f'-DTEST_VALUE_1={opts["value1"]}')
  if opts['value2']:
    args.append(f'-DTEST_VALUE_2={opts["value2"]}')
  return args


def handle_options(options):
  opts.update(options)


