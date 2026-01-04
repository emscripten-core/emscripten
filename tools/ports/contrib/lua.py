# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '5.5.0'
HASH = '3253d2cdc929da6438095a30d66ef16a1abdbb0ada8fee238705b3b38492f14be9553640fdca6b25661e01155ba5582032e0a2ef064e4c283e85efc0a128cabe'

# contrib port information (required)
URL = 'https://www.lua.org/'
DESCRIPTION = 'Lua is a powerful, efficient, lightweight, embeddable scripting language'
LICENSE = 'MIT License'

port_name = 'contrib.lua'
lib_name = 'liblua.a'


def get(ports, settings, shared):
  # get the port
  ports.fetch_project(port_name, f'https://www.lua.org/ftp/lua-{TAG}.tar.gz', sha512hash=HASH)

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

    ports.build_port(source_path, final, port_name, srcs=srcs)

  return [shared.cache.get_lib(lib_name, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(lib_name)
