# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.


# The purpose of this contrib port is to have 2 contrib ports in order to have a test with
# 2 ports since the syntax is different. Once there are 2 contrib ports available, this
# example can be removed and the test changed to use the 2 available ports...


import os


def get_lib_name(settings):
  return 'libcontrib_example.a'


def get(ports, settings, shared):

  def create(final):
    source_path = ports.get_dir()
    ports.write_file(os.path.join(source_path, 'contrib_example.h'), example_h)
    ports.write_file(os.path.join(source_path, 'contrib_example.c'), example_c)
    ports.install_headers(source_path)
    ports.build_port(source_path, final, 'contrib_example')

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_args(ports):
  return ['-isystem', ports.get_include_dir('contrib_example')]


def project_url():
  return 'https://github.com/emscripten-core/emscripten'


def project_description():
  return 'Port Contrib Example'


def project_license():
  return 'MIT license'


example_h = 'int contrib_example();'


example_c = r'''
int contrib_example() {
  return 12;
}
'''
