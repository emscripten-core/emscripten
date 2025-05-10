# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.


TAG = 'v20250509.171557'

EXTERNAL_PORT = f'https://github.com/google/dawn/releases/download/{TAG}/emdawnwebgpu_pkg-{TAG}.zip'
SHA512 = '4b66bf0f64b9616a6420abdad636b3ecefab892dde8f67cd941147bfddf7920f5523ff10160c9a563ef377a0f88b2dfc033527591b2d0753d531de5cbbabde59'
PORT_FILE = 'emdawnwebgpu.port.py'

# contrib port information (required)
URL = 'https://dawn.googlesource.com/dawn'
DESCRIPTION = 'Dawn is an open-source and cross-platform implementation of the WebGPU standard'
LICENSE = 'BSD 3-Clause License'
