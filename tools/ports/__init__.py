# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from . import binaryen, bullet, cocos2d, freetype, harfbuzz, icu, libpng, ogg, sdl, sdl_gfx, sdl_image, sdl_ttf, sdl_net, vorbis, zlib

# If port A depends on port B, then A should be _after_ B
ports = [icu, zlib, libpng, sdl, sdl_image, sdl_gfx, ogg, vorbis, bullet, freetype, harfbuzz, sdl_ttf, sdl_net, binaryen, cocos2d]

ports_by_name = {}
for port in ports:
  name = port.__name__.split('.')[-1]
  ports_by_name[name] = port

