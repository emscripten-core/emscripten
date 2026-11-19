# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

TAG = '3.5.1.20261109'

EXTERNAL_PORT = f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip'
SHA512 = 'e4b26b5d627476fb14ea900ebc3a4050c124e1f8fd5a600a887850ceed7d9ed8dd617ac16c7d1181839a26c553d8775093996d9c747e322d84a3236a9ca22791'
PORT_FILE = 'port/glfw3.py'

# contrib port information (required)
URL = 'https://github.com/pongasoft/emscripten-glfw'
DESCRIPTION = 'This project is an emscripten port of GLFW 3.5 written in C++ for the web/webassembly platform'
LICENSE = 'Apache 2.0 license'
