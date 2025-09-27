# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# https://dawn.googlesource.com/dawn/+/faa7054b5b65c3ce3774151952a68aa7668aa20b/src/emdawnwebgpu/pkg/README.md
r"""
This "remote port" instructs Emscripten (4.0.10+) how to automatically download
the actual port for Emdawnwebgpu. See README below for instructions.

# Emdawnwebgpu

Emdawnwebgpu is Dawn's implementation of webgpu.h for Emscripten (on top of the
WebGPU JS API). It is a fork of Emscripten's original `USE_WEBGPU` bindings,
and while it is maintained in Dawn, it works in any browser supporting WebGPU
(modulo individual feature support).

Emdawnwebgpu provides everything necessary to use `<webgpu/webgpu.h>` and the
Dawn-style `<webgpu/webgpu_cpp.h>` with Emscripten.

<!-- TODO(crbug.com/430616385): Link to a sample project. -->

If you find an issue in this release, please download the latest Emdawnwebgpu
release (see below) and latest Emscripten and verify the bug, then report it at
<https://crbug.com/new?component=1570785&noWizard=True>.

## API Stability

Core parts of `webgpu.h` (defined in
<https://github.com/webgpu-native/webgpu-headers>) are considered stable APIs
and should not change, except for bugfixes (though guarantees are not made).
Dawn/Emscripten-specific parts, and all of `webgpu_cpp.h`, are **NOT**
considered stable, and may change.

## How to use Emdawnwebgpu

Emdawnwebgpu is distributed in several ways. Choose the one that works for you.

In all cases, it is important to enable Closure to reduce code size in release
builds. Pass the following flag to `emcc` during linking:

    --closure=1

### Targeting Web only

#### Easiest: "Remote" port built into Emscripten

Recent releases of Emscripten vendor a copy of a "remote" port which
automatically downloads a pinned version of Emdawnwebgpu and configures it.

Pass the following flag to `emcc` during both compilation and linking:

    --use-port=emdawnwebgpu

#### Latest: "Remote" port from Dawn release

This is the same as the built-in port, but you can download a newer version if
you need recent bugfixes or features in Emdawnwebgpu that haven't been rolled
into Emscripten yet. **Requires Emscripten 4.0.10+.**

Download and extract the `emdawnwebgpu-*.remoteport.py` file from
<https://github.com/google/dawn/releases>.

Pass the following flag to `emcc` during both compilation and linking:

    --use-port=path/to/emdawnwebgpu_remoteport_file.py

#### Latest, without automatic downloading: "Local" port from Dawn release

Use this method if your build system requires sources to be local (e.g. checked
into your repository) instead of automatically downloaded, or if you use
Emscripten before 4.0.10.
**Note that Emdawnwebgpu may not work with older Emscripten releases.**

Download and extract the `emdawnwebgpu_pkg-*.zip` package from
<https://github.com/google/dawn/releases>.
(Note the package is text-only and does not contain any binaries, but see below
if you need to build the package from the original source.)

Pass the following flag to `emcc` during both compilation and linking:

    --use-port=path/to/emdawnwebgpu_pkg/emdawnwebgpu.port.py

If (and only if) using Emscripten before 4.0.7, pass this flag during linking:

    --closure-args=--externs=path/to/emdawnwebgpu_pkg/webgpu/src/webgpu-externs.js

#### Without using a port file (**Unsupported!**)

It is possible to integrate the Emdawnwebgpu sources directly into your build
process, which may be necessary for certain build systems, but this is not
officially supported. Using a port file instead is strongly recommended.

If you do this, the port files or Dawn's GN or CMake files can serve as a
reference for the steps needed. Note that in all cases, the sources include both
C++ and JS code. While it is possible to precompile the C++ code to `.a`, the JS
code cannot be precompiled and must be provided at the final link step.

### Cross-targeting Web/Native

#### Using CMake

Use this method if your project uses CMake and targets both Emscripten and
native platforms.

<https://developer.chrome.com/docs/web-platform/webgpu/build-app>

#### Building the package locally to use with a non-CMake project

If your project already has Dawn source, or you otherwise want to cross-target
Web and native with your non-CMake project, you can use CMake to build
`emdawnwebgpu_pkg` locally (similar to how you would build binary libraries
to link with in native), then use the "Local" port instructions above.

<https://dawn.googlesource.com/dawn/+/refs/heads/main/src/emdawnwebgpu/README.md>

## Port options

Options can be set by appending `:key1=value:key2=value` to `--use-port`.
For information about port options, run:

    emcc --use-port=emdawnwebgpu:help
    emcc --use-port=path/to/emdawnwebgpu_remoteport_file.py:help
    emcc --use-port=path/to/emdawnwebgpu_pkg/emdawnwebgpu.port.py:help

### C++ bindings

By default, C++ bindings are provided in the include path. Note that unlike
`webgpu.h`, these are not intended to be fully stable. If you don't want these
for any reason (you have custom bindings, you're using a pinned snapshot of
`webgpu_cpp.h`, etc.), you can set the option `cpp_bindings=false`:

    --use-port=emdawnwebgpu:cpp_bindings=false
    --use-port=path/to/emdawnwebgpu_remoteport_file.py:cpp_bindings=false
    --use-port=path/to/emdawnwebgpu_pkg/emdawnwebgpu.port.py:cpp_bindings=false

## Embuilder

If your build process needs a separate step to build the port before linking,
use Emscripten's `embuilder`.

Under `embuilder`, some options cannot be set automatically, so they must be
set manually. For details, see `OPTIONS` in `emdawnwebgpu.port.py` (in the
package zip).
"""

import sys

if __name__ == '__main__':
    print('Please see documentation inside this file for details on how to use this port.')
    sys.exit(1)

_VERSION = 'v20250926.144300'

# Remote-specific port information

# - Where to download the port
EXTERNAL_PORT = f'https://github.com/google/dawn/releases/download/{_VERSION}/emdawnwebgpu_pkg-{_VERSION}.zip'
# - Hash to verify the download integrity
SHA512 = 'a186cf7f33266c9dfeca7d99ffac769a91b2129e34054f1e857cd82e8b033896da34cf758088bbdeeb128aa713df9953851f83ba6d677c438a929d245a789948'
# - Path of the port inside the zip file
PORT_FILE = 'emdawnwebgpu_pkg/emdawnwebgpu.port.py'

# General port information

# - Visible in emcc --show-ports and emcc --use-port=emdawnwebgpu:help
LICENSE = "Some files: BSD 3-Clause License. Other files: Emscripten's license (available under both MIT License and University of Illinois/NCSA Open Source License)"

# - Visible in emcc --use-port=emdawnwebgpu:help
DESCRIPTION = "Emdawnwebgpu implements webgpu.h on WebGPU, replacing -sUSE_WEBGPU. **For info on usage and filing feedback, see link below.**"
URL = 'https://dawn.googlesource.com/dawn/+/faa7054b5b65c3ce3774151952a68aa7668aa20b/src/emdawnwebgpu/pkg/README.md'


# Emscripten <4.0.10 won't notice EXTERNAL_PORT and will try to use this.
def get(ports, settings, shared):
    raise Exception('Remote ports require Emscripten 4.0.10+.')


def clear(ports, settings, shared):
    pass
