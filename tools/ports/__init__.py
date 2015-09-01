import bullet, freetype, libpng, ogg, sdl, sdl_image, sdl_ttf, vorbis, zlib

# If port A depends on port B, then A should be _after_ B
ports = [zlib, libpng, sdl, sdl_image, ogg, vorbis, bullet, freetype, sdl_ttf]
