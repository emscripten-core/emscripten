# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from . import binaryen, bullet, cocos2d, freetype, harfbuzz, icu, libpng, ogg, regal, sdl2, sdl2_gfx, sdl2_image, sdl2_mixer, sdl2_ttf, sdl2_net, vorbis, zlib

# If port A depends on port B, then A should be _after_ B
ports = [
    icu,
    zlib,
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
    binaryen,
    cocos2d,
    regal
]

ports_by_name = {}
for port in ports:
  name = port.__name__.split('.')[-1]
  ports_by_name[name] = port
