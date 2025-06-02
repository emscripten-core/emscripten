# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

r"""
# Emdawnwebgpu

Emdawnwebgpu is Dawn's implementation of webgpu.h for Emscripten (on top of the
WebGPU JS API). It is a fork of Emscripten's original `USE_WEBGPU` bindings,
and while it is maintained in Dawn, it works in any browser supporting WebGPU
(modulo individual feature support).

The package includes all of the necessary files to use
`<webgpu/webgpu.h>` and the Dawn-style `<webgpu/webgpu_cpp.h>` with Emscripten.

Find new versions of this package at <https://github.com/google/dawn/releases>.

If you find any issue with this release, please verify it in the latest release,
and then report it at <https://crbug.com/new?component=1570785&noWizard=True>.

## How to use this package

First, start with either:

- A "remote" port file `emdawnwebgpu-v*.remoteport.py` (requires Emscripten 4.0.10+).
- An `emdawnwebgpu_pkg` containing a local port file `emdawnwebgpu.port.py`.
  (Either from a pre-built zip release, or from a Dawn build output directory.)

## How to use this package (local or remote)

Pass the following flag to `emcc`, during both compile and link, to set the
include paths and link the implementation:

    --use-port=path/to/emdawnwebgpu_port_or_remoteport_file.py

If (and only if) using Emscripten before 4.0.7, also pass this flag during link:

    --closure-args=--externs=path/to/emdawnwebgpu_pkg/webgpu/src/webgpu-externs.js

## Port options

Options can be set by appending `:key1=value:key2=value` to `--use-port`.
For information about port options, run:

    emcc --use-port=emdawnwebgpu:help
    emcc --use-port=path/to/emdawnwebgpu.port.py:help
    emcc --use-port=path/to/emdawnwebgpu-*.remoteport.py:help

### C++ bindings

By default, C++ bindings are provided in the include path. Note that unlike
`webgpu.h`, these are not intended to be fully stable. If you don't want these
for any reason (you have custom bindings, you're using a pinned snapshot of
`webgpu_cpp.h`, etc.), you can set the option `cpp_bindings=false`:

    --use-port=emdawnwebgpu:cpp_bindings=false
    --use-port=path/to/emdawnwebgpu.port.py:cpp_bindings=false
    --use-port=path/to/emdawnwebgpu-*.remoteport.py:cpp_bindings=false

## Embuilder

If your build process needs a separate step to build the port before linking,
use Emscripten's `embuilder`.

Under `embuilder`, some options cannot be set automatically, so they must be
set manually. See `OPTIONS` in `emdawnwebgpu.port.py` for details.
"""

TAG = 'v20250531.224602'

EXTERNAL_PORT = f'https://github.com/google/dawn/releases/download/{TAG}/emdawnwebgpu_pkg-{TAG}.zip'
SHA512 = 'e3db5b2a4cc97cbb9b8a0c1f6fe7a740bf644322dfa4641a7b0902c192c58205d76acb9b359a0d6da1fdc9795e85eb1cf1927aa47f2a82814ee29868fef6670e'
PORT_FILE = 'emdawnwebgpu_pkg/emdawnwebgpu.port.py'

# Port information (required)
URL = 'https://dawn.googlesource.com/dawn/+/refs/heads/main/src/emdawnwebgpu/'
DESCRIPTION = "Emdawnwebgpu is a fork of Emscripten's original USE_WEBGPU, implementing a newer, more stable version of the standardized webgpu.h interface."
LICENSE = "Some files: BSD 3-Clause License. Other files: Emscripten's license (available under both MIT License and University of Illinois/NCSA Open Source License)"
