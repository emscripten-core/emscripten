# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from . import boost_headers, bullet, cocos2d, freetype, harfbuzz, icu, libjpeg, libpng
from . import ogg, regal, sdl2, sdl2_gfx, sdl2_image, sdl2_mixer, sdl2_ttf
from . import sdl2_net, vorbis, zlib, bzip2

# If port A depends on port B, then A should be _after_ B
ports = [
    boost_headers,
    icu,
    zlib,
    bzip2,
    libjpeg,
    libpng,
    sdl2,
    sdl2_image,
    sdl2_gfx,
    ogg, vorbis,
    sdl2_mixer,
    bullet,
    freetype,
    harfbuzz,
    sdl2_ttf,
    sdl2_net,
    cocos2d,
    regal
]

ports_by_name = {}


def init():
  expected_attrs = ['get', 'clear', 'process_args', 'show']
  for port in ports:
    name = port.__name__.split('.')[-1]
    ports_by_name[name] = port
    for a in expected_attrs:
      assert hasattr(port, a), 'port %s is missing %s' % (port, a)


init()
