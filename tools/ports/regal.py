# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_4'
HASH = 'db702ee677d6ee276663922560012fe8d28c5f414ba50a628320432212bdcb606c34bc71f7a533416f43bd0226ceb819f79af822f11518d53552a071a27fc841'


def get(ports, settings, shared):
  if settings.USE_REGAL != 1:
    return []

  ports.fetch_project('regal', 'https://github.com/emscripten-ports/regal/archive/' + TAG + '.zip',
                      'regal-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libregal')

  def create():
    logging.info('building port: regal')
    ports.clear_project_build('regal')

    # copy sources
    # only what is needed is copied: regal, md5, jsonsl, boost, lookup3,
    # Note: GLSL Optimizer is included (needed for headers) but not built
    source_path_src = os.path.join(ports.get_dir(), 'regal', 'regal-' + TAG, 'src')
    dest_path_src = os.path.join(ports.get_build_dir(), 'regal', 'src')

    source_path_regal = os.path.join(source_path_src, 'regal')
    source_path_md5 = os.path.join(source_path_src, 'md5')
    source_path_jsonsl = os.path.join(source_path_src, 'jsonsl')
    source_path_boost = os.path.join(source_path_src, 'boost')
    source_path_lookup3 = os.path.join(source_path_src, 'lookup3')
    source_path_glslopt = os.path.join(source_path_src, 'glsl')
    dest_path_regal = os.path.join(dest_path_src, 'regal')
    dest_path_md5 = os.path.join(dest_path_src, 'md5')
    dest_path_jsonsl = os.path.join(dest_path_src, 'jsonsl')
    dest_path_boost = os.path.join(dest_path_src, 'boost')
    dest_path_lookup3 = os.path.join(dest_path_src, 'lookup3')
    dest_path_glslopt = os.path.join(dest_path_src, 'glsl')

    shutil.rmtree(dest_path_src, ignore_errors=True)
    shutil.copytree(source_path_regal, dest_path_regal)
    shutil.copytree(source_path_md5, dest_path_md5)
    shutil.copytree(source_path_jsonsl, dest_path_jsonsl)
    shutil.copytree(source_path_boost, dest_path_boost)
    shutil.copytree(source_path_lookup3, dest_path_lookup3)
    shutil.copytree(source_path_glslopt, dest_path_glslopt)

    # includes
    source_path_include = os.path.join(ports.get_dir(), 'regal', 'regal-' + TAG, 'include')
    dest_path_include = os.path.join(ports.get_build_dir(), 'regal', 'include')
    shutil.copytree(source_path_include, dest_path_include)

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
                  'regal/RegalDllMain.cpp',
                  'md5/src/md5.c',
                  'jsonsl/jsonsl.c']

    commands = []
    o_s = []

    for src in srcs_regal:
      c = os.path.join(dest_path_src, src)
      o = os.path.join(dest_path_src, src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))
      commands.append([shared.PYTHON, shared.EMCC, c,
                       # specify the defined symbols as the Regal Makefiles does for Emscripten+Release
                       # the define logic for other symbols will be handled automatically by Regal headers (SYS_EMSCRIPTEN, SYS_EGL, SYS_ES2, etc.)
                       '-DNDEBUG',
                       '-DREGAL_NO_PNG=1',
                       '-DREGAL_LOG=0',
                       '-DREGAL_NO_TLS=1',
                       '-DREGAL_THREAD_LOCKING=0',
                       '-DREGAL_GLSL_OPTIMIZER=0',
                       '-fomit-frame-pointer',
                       '-Wno-constant-logical-operand',
                       '-fvisibility=hidden',
                       '-O2',
                       '-o', o,
                       '-I' + dest_path_include,
                       '-I' + dest_path_regal,
                       '-I' + os.path.join(dest_path_md5, 'include'),
                       '-I' + dest_path_lookup3,
                       '-I' + dest_path_jsonsl,
                       '-I' + dest_path_boost,
                       '-I' + os.path.join(dest_path_glslopt, 'include'),
                       '-I' + os.path.join(dest_path_glslopt, 'src', 'glsl'),
                       '-I' + os.path.join(dest_path_glslopt, 'src', 'mesa'),
                       '-w'])
      o_s.append(o)

    ports.run_commands(commands)
    final = os.path.join(ports.get_build_dir(), 'regal', libname)
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libregal'))


def process_dependencies(settings):
  if settings.USE_REGAL == 1:
    settings.FULL_ES2 = 1


def process_args(ports, args, settings, shared):
  if settings.USE_REGAL == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(ports.get_build_dir(), 'regal', 'include')]
  return args


def show():
  return 'regal (USE_REGAL=1; Regal license)'
