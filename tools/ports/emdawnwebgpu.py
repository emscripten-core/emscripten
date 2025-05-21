# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.


TAG = 'v20250516.124039'

EXTERNAL_PORT = f'https://github.com/google/dawn/releases/download/{TAG}/emdawnwebgpu_pkg-{TAG}.zip'
SHA512 = '994eac4be5f69d8ec83838af9c7b4cc87f15fa22bede589517c169320dd69ab5cf164528f7bd6ec6503b1ef178da3d87df0565d16445dac2a69f98450083dd8f'
PORT_FILE = 'emdawnwebgpu_pkg/emdawnwebgpu.port.py'

# Port information (required)
URL = 'https://dawn.googlesource.com/dawn'
DESCRIPTION = "Emdawnwebgpu is a fork of Emscripten's original USE_WEBGPU, implementing a newer, more stable version of the standardized webgpu.h interface. If you find issues, verify in the latest nightly release (https://github.com/google/dawn/releases) and file feedback with Dawn. (Emdawnwebgpu is maintained as part of Dawn, the open-source WebGPU implementation used by Chromium, but it is still cross-browser.)"
LICENSE = 'Some files: BSD 3-Clause License. Other files: Emscripten\'s license (available under both MIT License and University of Illinois/NCSA Open Source License)'
