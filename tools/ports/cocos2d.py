# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging
import re

TAG = 'version_3_3'


def get(ports, settings, shared):
  if settings.USE_COCOS2D == 3:
    ports.fetch_project(
      'Cocos2d', 'https://github.com/emscripten-ports/Cocos2d/archive/' + TAG + '.zip', 'Cocos2d-' + TAG)

    def create():
      logging.info('building port: Cocos2d v3')
      logging.warn('Cocos2d: library is experimental, do not expect that it will work out of the box')

      cocos2d_build = os.path.join(ports.get_dir(), 'Cocos2d')
      cocos2d_root = os.path.join(cocos2d_build, 'Cocos2d-' + TAG)
      cocos2dx_root = os.path.join(cocos2d_root, 'cocos2dx')
      cocos2dx_src = make_source_list(cocos2d_root, cocos2dx_root)
      cocos2dx_includes = make_includes(cocos2d_root, cocos2dx_root)

      shutil.copytree(os.path.join(cocos2d_root, 'samples', 'Cpp'),
              os.path.join(ports.get_build_dir(), 'Cocos2d', 'samples'))

      commands = []
      o_s = []
      for src in cocos2dx_src:
        o = os.path.join(cocos2d_build, 'Cocos2d-' +
                 TAG, 'build', src + '.o')
        shared.safe_ensure_dirs(os.path.dirname(o))
        command = [shared.PYTHON,
               shared.EMCC,
               os.path.join(cocos2dx_root, 'proj.emscripten', src),
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

        if src.endswith('.cpp'):
          command.append('-std=c++11')

        commands.append(command)
        o_s.append(o)
      shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
      ports.run_commands(commands)
      final = os.path.join(ports.get_build_dir(), 'Cocos2d', 'libCocos2d.bc')
      shared.Building.link(o_s, final)
      return final
    return [shared.Cache.get('libCocos2d', create, what='port')]
  else:
    return []


def process_dependencies(settings):
  if settings.USE_COCOS2D == 3:
    settings.USE_LIBPNG = 1
    settings.USE_ZLIB = 1


def process_args(ports, args, settings, shared):
  if settings.USE_COCOS2D == 3:
    get(ports, settings, shared)
    cocos2d_build = os.path.join(ports.get_dir(), 'Cocos2d')
    cocos2d_root = os.path.join(cocos2d_build, 'Cocos2d-' + TAG)
    cocos2dx_root = os.path.join(cocos2d_root, 'cocos2dx')
    cocos2dx_includes = make_includes(cocos2d_root, cocos2dx_root)
    args += ['-Xclang']
    for include in cocos2dx_includes:
      args.append('-isystem' + include)
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
          file = re.search('=\s*(.*?)(\s*\\\\$|\s*$)', line, re.IGNORECASE).group(1)
          absfile = os.path.abspath(os.path.join(os.path.dirname(makefile), file))
          sources.append(absfile)
          add_next = line.endswith('\\\n')
          continue
        if add_next:
          file = re.search('\s*(.*?)(\s*\\\\$|\s*$)', line, re.IGNORECASE).group(1)
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


def make_includes(cocos2d_root, cocos2dx_root):
  return [os.path.join(cocos2d_root, 'CocosDenshion', 'include'),
      os.path.join(cocos2d_root, 'extensions'),
      os.path.join(cocos2d_root, 'extensions', 'AssetsManager'),
      os.path.join(cocos2d_root, 'extensions', 'CCArmature'),
      os.path.join(cocos2d_root, 'extensions', 'CCBReader'),
      os.path.join(cocos2d_root, 'extensions', 'GUI', 'CCControlExtension'),
      os.path.join(cocos2d_root, 'extensions', 'GUI', 'CCEditBox'),
      os.path.join(cocos2d_root, 'extensions', 'GUI', 'CCScrollView'),
      os.path.join(cocos2d_root, 'extensions', 'network'),
      os.path.join(cocos2d_root, 'extensions', 'Components'),
      os.path.join(cocos2d_root, 'extensions', 'LocalStorage'),
      os.path.join(cocos2d_root, 'extensions', 'physics_nodes'),
      os.path.join(cocos2d_root, 'extensions', 'spine'),
      os.path.join(cocos2d_root, 'external'),
      os.path.join(cocos2d_root, 'external', 'chipmunk', 'include', 'chipmunk'),
      cocos2dx_root,
      os.path.join(cocos2dx_root, 'cocoa'),
      os.path.join(cocos2dx_root, 'include'),
      os.path.join(cocos2dx_root, 'kazmath', 'include'),
      os.path.join(cocos2dx_root, 'platform'),
      os.path.join(cocos2dx_root, 'platform', 'emscripten'),
      os.path.join(cocos2dx_root, 'platform', 'third_party', 'linux', 'libfreetype2'),
      os.path.join(cocos2dx_root, 'platform', 'third_party', 'common', 'etc'),
      os.path.join(cocos2dx_root, 'platform', 'third_party', 'emscripten', 'libtiff', 'include'),
      os.path.join(cocos2dx_root, 'platform', 'third_party', 'emscripten', 'libjpeg'),
      os.path.join(cocos2dx_root, 'platform', 'third_party', 'emscripten', 'libwebp')]
