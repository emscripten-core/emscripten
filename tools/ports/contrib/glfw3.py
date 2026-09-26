# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

TAG = '3.5.1.20260809'

EXTERNAL_PORT = f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip'
SHA512 = '9efad6b2022885a39c92195540982027a0f13cf346272913ed89c7c67139d653c5835c799fe6325e83047afbb388532226174cbde476847ac3364500218e4efc'
PORT_FILE = 'port/glfw3.py'

# contrib port information (required)
URL = 'https://github.com/pongasoft/emscripten-glfw'
DESCRIPTION = 'This project is an emscripten port of GLFW 3.5 written in C++ for the web/webassembly platform'
LICENSE = 'Apache 2.0 license'
