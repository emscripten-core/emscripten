# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_7'
HASH = 'a921dab254f21cf5d397581c5efe58faf147c31527228b4fb34aed75164c736af4b3347092a8d9ec1249160230fa163309a87a20c2b9ceef8554566cc215de9d'


def needed(settings):
  return settings.USE_REGAL


def get_lib_name(ports, settings):
  return ports.get_lib_name('libregal' + ('-mt' if settings.USE_PTHREADS else ''))


def get(ports, settings, shared):
  ports.fetch_project('regal', 'https://github.com/emscripten-ports/regal/archive/' + TAG + '.zip',
                      'regal-' + TAG, sha512hash=HASH)

  def create():
    logging.info('building port: regal')
    ports.clear_project_build('regal')

    # copy sources
    # only what is needed is copied: regal, boost, lookup3
    source_path_src = os.path.join(ports.get_dir(), 'regal', 'regal-' + TAG, 'src')
    dest_path_src = os.path.join(ports.get_build_dir(), 'regal', 'src')

    source_path_regal = os.path.join(source_path_src, 'regal')
    source_path_boost = os.path.join(source_path_src, 'boost')
    source_path_lookup3 = os.path.join(source_path_src, 'lookup3')
    dest_path_regal = os.path.join(dest_path_src, 'regal')
    dest_path_boost = os.path.join(dest_path_src, 'boost')
    dest_path_lookup3 = os.path.join(dest_path_src, 'lookup3')

    shutil.rmtree(dest_path_src, ignore_errors=True)
    shutil.copytree(source_path_regal, dest_path_regal)
    shutil.copytree(source_path_boost, dest_path_boost)
    shutil.copytree(source_path_lookup3, dest_path_lookup3)

    # includes
    source_path_include = os.path.join(ports.get_dir(), 'regal', 'regal-' + TAG, 'include', 'GL')
    ports.install_header_dir(source_path_include)

    # build
    srcs_regal = ['regal/RegalShaderInstance.cpp',
                  'regal/RegalIff.cpp',
                  'regal/RegalQuads.cpp',
                  'regal/Regal.cpp',
                  'regal/RegalLog.cpp',
                  'regal/RegalInit.cpp',
                  'regal/RegalBreak.cpp',
                  'regal/RegalUtil.cpp',
                  'regal/RegalEmu.cpp',
                  'regal/RegalEmuInfo.cpp',
                  'regal/RegalFrame.cpp',
                  'regal/RegalHelper.cpp',
                  'regal/RegalMarker.cpp',
                  'regal/RegalTexC.cpp',
                  'regal/RegalCacheShader.cpp',
                  'regal/RegalCacheTexture.cpp',
                  'regal/RegalConfig.cpp',
                  'regal/RegalContext.cpp',
                  'regal/RegalContextInfo.cpp',
                  'regal/RegalDispatch.cpp',
                  'regal/RegalStatistics.cpp',
                  'regal/RegalLookup.cpp',
                  'regal/RegalPlugin.cpp',
                  'regal/RegalShader.cpp',
                  'regal/RegalToken.cpp',
                  'regal/RegalDispatchGlobal.cpp',
                  'regal/RegalDispatcher.cpp',
                  'regal/RegalDispatcherGL.cpp',
                  'regal/RegalDispatcherGlobal.cpp',
                  'regal/RegalDispatchEmu.cpp',
                  'regal/RegalDispatchGLX.cpp',
                  'regal/RegalDispatchLog.cpp',
                  'regal/RegalDispatchCode.cpp',
                  'regal/RegalDispatchCache.cpp',
                  'regal/RegalDispatchError.cpp',
                  'regal/RegalDispatchLoader.cpp',
                  'regal/RegalDispatchDebug.cpp',
                  'regal/RegalDispatchPpapi.cpp',
                  'regal/RegalDispatchStatistics.cpp',
                  'regal/RegalDispatchStaticES2.cpp',
                  'regal/RegalDispatchStaticEGL.cpp',
                  'regal/RegalDispatchTrace.cpp',
                  'regal/RegalDispatchMissing.cpp',
                  'regal/RegalPixelConversions.cpp',
                  'regal/RegalHttp.cpp',
                  'regal/RegalDispatchHttp.cpp',
                  'regal/RegalJson.cpp',
                  'regal/RegalFavicon.cpp',
                  'regal/RegalMac.cpp',
                  'regal/RegalSo.cpp',
                  'regal/RegalFilt.cpp',
                  'regal/RegalXfer.cpp',
                  'regal/RegalX11.cpp',
                  'regal/RegalDllMain.cpp']

    commands = []
    o_s = []

    for src in srcs_regal:
      c = os.path.join(dest_path_src, src)
      o = os.path.join(dest_path_src, src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))

      command = [shared.EMCC, '-c', c,
                 '-DNDEBUG',
                 '-DREGAL_LOG=0',  # Set to 1 if you need to have some logging info
                 '-DREGAL_MISSING=0',  # Set to 1 if you don't want to crash in case of missing GL implementation
                 '-fno-rtti',
                 '-fno-exceptions', # Disable exceptions (in STL containers mostly), as they are not used at all
                 '-O3',
                 '-o', o,
                 '-I' + dest_path_regal,
                 '-I' + dest_path_lookup3,
                 '-I' + dest_path_boost,
                 '-Wall',
                 '-Werror',
                 '-Wno-deprecated-register',
                 '-Wno-unused-parameter']
      if settings.USE_PTHREADS:
        command += ['-pthread']
      commands.append(command)

      o_s.append(o)

    ports.run_commands(commands)
    final = os.path.join(ports.get_build_dir(), 'regal', get_lib_name(ports, settings))
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get(get_lib_name(ports, settings), create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file(get_lib_name(ports, settings))


def process_dependencies(settings):
  settings.FULL_ES2 = 1


def process_args(ports):
  return []


def show():
  return 'regal (USE_REGAL=1; Regal license)'
