# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging, subprocess, sys, stat

TAG = 'version_1'

def get(ports, settings, shared):
    if settings.USE_REGAL == 1:
        ports.fetch_project('regal', 'https://github.com/gabrielcuvillier/regal/archive/' + TAG + '.zip', 'Regal-' + TAG)
        def create():
            logging.info('building port: regal')
            ports.clear_project_build('regal')

            source_path = os.path.join(ports.get_dir(), 'regal', 'Regal-' + TAG)
            dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'regal')

            shutil.rmtree(dest_path, ignore_errors=True)
            shutil.copytree(source_path, dest_path)

            # build
            srcs = ['src/regal/RegalShaderInstance.cpp',
                    'src/regal/RegalIff.cpp',
                    'src/regal/RegalQuads.cpp',
                    'src/regal/Regal.cpp',
                    'src/regal/RegalLog.cpp',
                    'src/regal/RegalInit.cpp',
                    'src/regal/RegalBreak.cpp',
                    'src/regal/RegalUtil.cpp',
                    'src/regal/RegalEmu.cpp',
                    'src/regal/RegalEmuInfo.cpp',
                    'src/regal/RegalFrame.cpp',
                    'src/regal/RegalHelper.cpp',
                    'src/regal/RegalMarker.cpp',
                    'src/regal/RegalTexC.cpp',
                    'src/regal/RegalCacheShader.cpp',
                    'src/regal/RegalCacheTexture.cpp',
                    'src/regal/RegalConfig.cpp',
                    'src/regal/RegalContext.cpp',
                    'src/regal/RegalContextInfo.cpp',
                    'src/regal/RegalDispatch.cpp',
                    'src/regal/RegalStatistics.cpp',
                    'src/regal/RegalLookup.cpp',
                    'src/regal/RegalPlugin.cpp',
                    'src/regal/RegalShader.cpp',
                    'src/regal/RegalToken.cpp',
                    'src/regal/RegalDispatchGlobal.cpp',
                    'src/regal/RegalDispatcher.cpp',
                    'src/regal/RegalDispatcherGL.cpp',
                    'src/regal/RegalDispatcherGlobal.cpp',
                    'src/regal/RegalDispatchEmu.cpp',
                    'src/regal/RegalDispatchGLX.cpp',
                    'src/regal/RegalDispatchLog.cpp',
                    'src/regal/RegalDispatchCode.cpp',
                    'src/regal/RegalDispatchCache.cpp',
                    'src/regal/RegalDispatchError.cpp',
                    'src/regal/RegalDispatchLoader.cpp',
                    'src/regal/RegalDispatchDebug.cpp',
                    'src/regal/RegalDispatchPpapi.cpp',
                    'src/regal/RegalDispatchStatistics.cpp',
                    'src/regal/RegalDispatchStaticES2.cpp',
                    'src/regal/RegalDispatchStaticEGL.cpp',
                    'src/regal/RegalDispatchTrace.cpp',
                    'src/regal/RegalDispatchMissing.cpp',
                    'src/regal/RegalPixelConversions.cpp',
                    'src/regal/RegalHttp.cpp',
                    'src/regal/RegalDispatchHttp.cpp',
                    'src/regal/RegalJson.cpp',
                    'src/regal/RegalFavicon.cpp',
                    'src/regal/RegalMac.cpp',
                    'src/regal/RegalSo.cpp',
                    'src/regal/RegalFilt.cpp',
                    'src/regal/RegalXfer.cpp',
                    'src/regal/RegalX11.cpp',
                    'src/regal/RegalDllMain.cpp',
                    'src/md5/src/md5.c',
                    'src/jsonsl/jsonsl.c']
            commands = []
            o_s = []
            for src in srcs:
                o = os.path.join(ports.get_build_dir(), 'regal', src + '.o')
                shared.safe_ensure_dirs(os.path.dirname(o))
                commands.append([shared.PYTHON, shared.EMCC, os.path.join(dest_path, src),
                                 '-DNDEBUG',
                                 '-DREGAL_DECL_EXPORT=1',
                                 '-DREGAL_LOG_ALL=0',
                                 '-DREGAL_LOG_ONCE=0',
                                 '-DREGAL_LOG_JSON=0',
                                 '-DREGAL_HTTP=0',
                                 '-DREGAL_NO_ASSERT=1',
                                 '-DREGAL_NO_PNG=1',
                                 '-DREGAL_STATISTICS=0',
                                 '-DREGAL_CODE=0',
                                 '-DREGAL_SYS_EMSCRIPTEN=1',
                                 '-DREGAL_SYS_EGL=0',
                                 '-DREGAL_SYS_ES2=1',
                                 '-DREGAL_PLUGIN=0',
                                 '-DREGAL_TRACE=0',
                                 '-DREGAL_LOG=0',
                                 '-DREGAL_ERROR=0',
                                 '-DREGAL_CACHE=0',
                                 '-DREGAL_DEBUG=0',
                                 '-DREGAL_NO_TLS=1',
                                 '-fomit-frame-pointer',
                                 '-Wno-constant-logical-operand',
                                 '-fvisibility=hidden',
                                 '-O2',
                                 '-o', o,
                                 '-I' + dest_path + '/include',
                                 '-I' + dest_path + '/src/regal',
                                 '-I' + dest_path + '/src/md5/include',
                                 '-I' + dest_path + '/src/lookup3',
                                 '-I' + dest_path + '/src/jsonsl',
                                 '-I' + dest_path + '/src/boost',
                                 '-I' + dest_path + '/src/glsl/include',
                                 '-I' + dest_path + '/src/glsl/src/glsl',
                                 '-I' + dest_path + '/src/glsl/src/mesa',
                                 '-w',])
                o_s.append(o)

            ports.run_commands(commands)
            final = os.path.join(ports.get_build_dir(), 'regal', 'libregal.a')
            shared.try_delete(final)
            Popen([shared.LLVM_AR, 'rc', final] + o_s).communicate()
            assert os.path.exists(final)
            return final
        return [shared.Cache.get('regal', create, what='port')]
    else:
        return []

def process_args(ports, args, settings, shared):
    if settings.USE_REGAL == 1:
        get(ports, settings, shared)
        args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'regal', 'include')]
    return args

def show():
    return 'regal (USE_REGAL=1; Regal license)'
