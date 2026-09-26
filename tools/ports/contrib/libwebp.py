# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

URL = 'https://chromium.googlesource.com/webm/libwebp'
DESCRIPTION = 'libwebp is a library for encoding and decoding images in the WebP format, including animation, alpha and metadata (mux/demux) support'
LICENSE = 'BSD-3-Clause license'

TAG = '1.6.0'
HASH = '5c159d9760efcb92749092536daada22c0a73c20926c76097a5f0448ddbf874cf761324ca97925ca5f578b30477564b2b072b47667e504673797128b31cafcbf'

port_name = 'contrib.libwebp'

variants = {
  'contrib.libwebp-mt': {'PTHREADS': 1},
}


def get_lib_name(settings):
  return 'libwebp-mt.a' if settings.PTHREADS else 'libwebp.a'


def get(ports, settings, shared):
  ports.fetch_project(port_name, f'https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-{TAG}.tar.gz', sha512hash=HASH)

  def create(final):
    root_path = ports.get_dir(port_name, f'libwebp-{TAG}')

    ports.install_headers(os.path.join(root_path, 'src', 'webp'), target='webp')
    # More accurate and sharper RGB->YUV conversion
    sharpyuv_dir = os.path.join(root_path, 'sharpyuv')
    sharpyuv_target = os.path.join('webp', 'sharpyuv')
    ports.install_headers(sharpyuv_dir, pattern='sharpyuv.h', target=sharpyuv_target)
    ports.install_headers(sharpyuv_dir, pattern='sharpyuv_csp.h', target=sharpyuv_target)

    flags = []
    if settings.PTHREADS:
      flags += ['-pthread', '-DWEBP_USE_THREAD']

    ports.build_port(
      root_path, final, port_name,
      flags=flags,
      exclude_dirs=['cmake', 'doc', 'examples', 'extras', 'gradle', 'imageio', 'infra', 'm4', 'man', 'swig', 'tests', 'webp_js'],
    )

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))
