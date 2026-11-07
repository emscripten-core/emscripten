# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

TAG = '3.5.1.20261104'

EXTERNAL_PORT = f'https://github.com/pongasoft/emscripten-glfw/releases/download/v{TAG}/emscripten-glfw3-{TAG}.zip'
SHA512 = 'db6099a1a8fecae0c275cad23ffc2ff9de1b8e257997b75137cc3998667b364d5df10267e3b0bbdacc5db4e7bc2d7add4b946dc7c85aacf2f9bb373688aff7ac'
PORT_FILE = 'port/glfw3.py'

# contrib port information (required)
URL = 'https://github.com/pongasoft/emscripten-glfw'
DESCRIPTION = 'This project is an emscripten port of GLFW 3.5 written in C++ for the web/webassembly platform'
LICENSE = 'Apache 2.0 license'
