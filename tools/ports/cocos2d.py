# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging
import re

TAG = 'version_3_3'
HASH = 'd7b22660036c684f09754fcbbc7562984f02aa955eef2b76555270c63a717e6672c4fe695afb16280822e8b7c75d4b99ae21975a01a4ed51cad957f7783722cd'

deps = ['libpng', 'zlib']


def needed(settings):
  return settings.USE_COCOS2D == 3


def get(ports, settings, shared):
  ports.fetch_project(
    'cocos2d', 'https://github.com/emscripten-ports/Cocos2d/archive/' + TAG + '.zip', 'Cocos2d-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libcocos2d')

  def create():
    logging.info('building port: cocos2d v3')
    logging.warn('cocos2d: library is experimental, do not expect that it will work out of the box')

    cocos2d_src = os.path.join(ports.get_dir(), 'cocos2d')
    cocos2d_root = os.path.join(cocos2d_src, 'Cocos2d-' + TAG)
    cocos2dx_root = os.path.join(cocos2d_root, 'cocos2dx')
    cocos2dx_src = make_source_list(cocos2d_root, cocos2dx_root)
    cocos2dx_includes = make_includes(cocos2d_root)

    cocos2d_build = os.path.join(ports.get_build_dir(), 'cocos2d')
    shared.try_delete(os.path.join(cocos2d_build, 'samples'))
    shutil.copytree(os.path.join(cocos2d_root, 'samples', 'Cpp'),
                    os.path.join(cocos2d_build, 'samples'))

    commands = []
    o_s = []
    for src in cocos2dx_src:
      o = os.path.join(cocos2d_build, 'Cocos2d-' + TAG, 'build', src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))
      command = [shared.EMCC,
                 '-c', src,
                 '-Wno-overloaded-virtual',
                 '-Wno-deprecated-declarations',
                 '-D__CC_PLATFORM_FILEUTILS_CPP__',
                 '-DCC_ENABLE_CHIPMUNK_INTEGRATION',
                 '-DCC_KEYBOARD_SUPPORT',
                 '-DGL_ES=1',
                 '-DNDEBUG', # '-DCOCOS2D_DEBUG=1' 1 - error/warn, 2 - verbose
                 '-DCP_USE_DOUBLES=0',
                 '-O2',
                 '-s', 'USE_ZLIB=1',
                 '-s', 'USE_LIBPNG=1',
                 '-o', o, '-w']

      for include in cocos2dx_includes:
        command.append('-I' + include)

      commands.append(command)
      o_s.append(o)
    shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
    ports.run_commands(commands)
    final = os.path.join(cocos2d_build, libname)
    ports.create_lib(final, o_s)

    for dirname in cocos2dx_includes:
      target = os.path.join('cocos2d', os.path.relpath(dirname, cocos2d_root))
      ports.install_header_dir(dirname, target=target)

    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file(ports.get_lib_name('libcocos2d'))


def process_dependencies(settings):
  settings.USE_LIBPNG = 1
  settings.USE_ZLIB = 1


def process_args(ports):
  args = []
  for include in make_includes(os.path.join(ports.get_include_dir(), 'cocos2d')):
    args.append('-I' + include)
  return args


def show():
  return 'cocos2d'


def make_source_list(cocos2d_root, cocos2dx_root):
  sources = []

  def add_makefile(makefile):
    with open(makefile) as infile:
      add_next = False
      for line in infile:
        if line.startswith('SOURCES'):
          file = re.search(r'=\s*(.*?)(\s*\\$|\s*$)', line, re.IGNORECASE).group(1)
          absfile = os.path.abspath(os.path.join(os.path.dirname(makefile), file))
          sources.append(absfile)
          add_next = line.endswith('\\\n')
          continue
        if add_next:
          file = re.search(r'\s*(.*?)(\s*\\$|\s*$)', line, re.IGNORECASE).group(1)
          absfile = os.path.abspath(os.path.join(os.path.dirname(makefile), file))
          sources.append(absfile)
          add_next = line.endswith('\\\n')

  # core
  add_makefile(os.path.join(cocos2dx_root, 'proj.emscripten', 'Makefile'))
  # extensions
  add_makefile(os.path.join(cocos2d_root, 'extensions', 'proj.emscripten', 'Makefile'))
  # external
  add_makefile(os.path.join(cocos2d_root, 'external', 'Box2D', 'proj.emscripten', 'Makefile'))
  add_makefile(os.path.join(cocos2d_root, 'external', 'chipmunk', 'proj.emscripten', 'Makefile'))
  add_makefile(os.path.join(cocos2dx_root, 'platform', 'third_party', 'Makefile'))
  # misc
  sources.append(os.path.join(cocos2d_root, 'CocosDenshion', 'emscripten', 'SimpleAudioEngine.cpp'))
  sources.append(os.path.join(cocos2dx_root, 'CCDeprecated.cpp')) # subset of cocos2d v2
  return sources


def make_includes(root):
  return [os.path.join(root, 'CocosDenshion', 'include'),
          os.path.join(root, 'extensions'),
          os.path.join(root, 'extensions', 'AssetsManager'),
          os.path.join(root, 'extensions', 'CCArmature'),
          os.path.join(root, 'extensions', 'CCBReader'),
          os.path.join(root, 'extensions', 'GUI', 'CCControlExtension'),
          os.path.join(root, 'extensions', 'GUI', 'CCEditBox'),
          os.path.join(root, 'extensions', 'GUI', 'CCScrollView'),
          os.path.join(root, 'extensions', 'network'),
          os.path.join(root, 'extensions', 'Components'),
          os.path.join(root, 'extensions', 'LocalStorage'),
          os.path.join(root, 'extensions', 'physics_nodes'),
          os.path.join(root, 'extensions', 'spine'),
          os.path.join(root, 'external'),
          os.path.join(root, 'external', 'chipmunk', 'include', 'chipmunk'),
          os.path.join(root, 'cocos2dx'),
          os.path.join(root, 'cocos2dx', 'cocoa'),
          os.path.join(root, 'cocos2dx', 'include'),
          os.path.join(root, 'cocos2dx', 'kazmath', 'include'),
          os.path.join(root, 'cocos2dx', 'platform'),
          os.path.join(root, 'cocos2dx', 'platform', 'emscripten'),
          os.path.join(root, 'cocos2dx', 'platform', 'third_party', 'linux', 'libfreetype2'),
          os.path.join(root, 'cocos2dx', 'platform', 'third_party', 'common', 'etc'),
          os.path.join(root, 'cocos2dx', 'platform', 'third_party', 'emscripten', 'libtiff', 'include'),
          os.path.join(root, 'cocos2dx', 'platform', 'third_party', 'emscripten', 'libjpeg'),
          os.path.join(root, 'cocos2dx', 'platform', 'third_party', 'emscripten', 'libwebp')]
