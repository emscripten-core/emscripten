# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '5.4.7'
HASH = '98c5c8978dfdf867e37e9eb3b3ec83dee92d199243b5119505da83895e33f10d43c841be6a7d3b106daba8a0b2bd25fe099ebff8f87831dcc55c79c78b97d8b8'
ZIP_URL = f'https://www.lua.org/ftp/lua-{TAG}.tar.gz'

# contrib port information (required)
URL = 'https://www.lua.org/'
DESCRIPTION = 'Lua is a powerful, efficient, lightweight, embeddable scripting language'
LICENSE = 'MIT License'

port_name = 'contrib.lua'


def get_lib_name(settings):
  return f'lib_{port_name}.a'


def get(ports, settings, shared):
  # get the port
  ports.fetch_project(port_name, ZIP_URL, sha512hash=HASH)

  def create(final):
    root_path = os.path.join(ports.get_dir(), port_name, f'lua-{TAG}')
    source_path = os.path.join(root_path, 'src')

    # install headers
    includes = ['lua.h', 'lua.hpp', 'luaconf.h', 'lauxlib.h', 'lualib.h']
    for f in includes:
      ports.install_headers(source_path, pattern=f)

    srcs = '''
       lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
       lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lcorolib.c
       ldblib.c liolib.c lmathlib.c loadlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c linit.c
       '''.split()

    flags = ['-DLUA_COMPAT_5_3']

    ports.build_port(source_path, final, port_name, srcs=srcs, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))
