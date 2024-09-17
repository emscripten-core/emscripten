# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_7'
HASH = 'a921dab254f21cf5d397581c5efe58faf147c31527228b4fb34aed75164c736af4b3347092a8d9ec1249160230fa163309a87a20c2b9ceef8554566cc215de9d'

variants = {'regal-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_REGAL


def get_lib_name(settings):
  return 'libregal' + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  ports.fetch_project('regal', f'https://github.com/emscripten-ports/regal/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('regal', 'regal-' + TAG)

    # copy sources
    # only what is needed is copied: regal, boost, lookup3
    source_path_src = os.path.join(source_path, 'src')

    source_path_regal = os.path.join(source_path_src, 'regal')
    source_path_boost = os.path.join(source_path_src, 'boost')
    source_path_lookup3 = os.path.join(source_path_src, 'lookup3')

    # includes
    source_path_include = os.path.join(source_path, 'include', 'GL')
    ports.install_headers(source_path_include, target='GL')

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

    srcs_regal = [os.path.join(source_path_src, s) for s in srcs_regal]

    flags = [
      '-DNDEBUG',
      '-DREGAL_LOG=0',  # Set to 1 if you need to have some logging info
      '-DREGAL_MISSING=0',  # Set to 1 if you don't want to crash in case of missing GL implementation
      '-std=gnu++14',
      '-fno-rtti',
      '-fno-exceptions', # Disable exceptions (in STL containers mostly), as they are not used at all
      '-O3',
      '-I' + source_path_regal,
      '-I' + source_path_lookup3,
      '-I' + source_path_boost,
      '-Wno-deprecated-register',
      '-Wno-unused-parameter',
      '-fdelayed-template-parsing',
    ]
    if settings.PTHREADS:
      flags += ['-pthread']

    ports.build_port(source_path_src, final, 'regal', srcs=srcs_regal, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def linker_setup(ports, settings):
  settings.FULL_ES2 = 1


def show():
  return 'regal (-sUSE_REGAL=1 or --use-port=regal; Regal license)'
