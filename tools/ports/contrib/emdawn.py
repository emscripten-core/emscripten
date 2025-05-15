# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.


TAG = 'v20250514.194204'

EXTERNAL_PORT = f'https://github.com/google/dawn/releases/download/{TAG}/emdawnwebgpu_pkg-{TAG}.zip'
SHA512 = '6aedbaa6a19fe91fc36b1e2cb6ca369cf6a29a3c8213c39f3b3930bd36bb7186665301f090ad91e9a99ba5999c9f6d32404ef37fd0fbb7c017ea3d9151cd22ad'
PORT_FILE = 'emdawnwebgpu_pkg/emdawnwebgpu.port.py'

# contrib port information (required)
URL = 'https://dawn.googlesource.com/dawn'
DESCRIPTION = 'Dawn is an open-source and cross-platform implementation of the WebGPU standard'
LICENSE = 'BSD 3-Clause License'
