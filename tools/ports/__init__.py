# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
from tools.shared import exit_with_error

ports = []

ports_by_name = {}

ports_dir = os.path.dirname(os.path.abspath(__file__))


def read_ports():
  expected_attrs = ['get', 'clear', 'process_args', 'show', 'needed']
  for filename in os.listdir(ports_dir):
    if not filename.endswith('.py') or filename == '__init__.py':
      continue
    filename = os.path.splitext(filename)[0]
    port = __import__(filename, globals(), level=1)
    ports.append(port)
    port.name = filename
    ports_by_name[port.name] = port
    for a in expected_attrs:
      assert hasattr(port, a), 'port %s is missing %s' % (port, a)
    if not hasattr(port, 'process_dependencies'):
      port.process_dependencies = lambda x: 0
    if not hasattr(port, 'deps'):
      port.deps = []

  for dep in port.deps:
    if dep not in ports_by_name:
      exit_with_error('unknown dependency in port: %s' % dep)


read_ports()
