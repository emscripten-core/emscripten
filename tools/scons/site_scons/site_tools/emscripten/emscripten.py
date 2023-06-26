#!/usr/bin/env python2
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os


def generate(env, emscripten_path=None, **kw):
  """ SCons tool entry point """

  if emscripten_path is None:
    emscripten_path = os.environ.get('EMSCRIPTEN_ROOT')
    if not emscripten_path:
      raise 'Unable to find emscripten. Please set EMSCRIPTEN_ROOT'

  # SCons does not by default invoke the compiler with the
  # environment variabls from the parent calling process,
  # so manually route all environment variables referenced
  # by Emscripten to the child.
  for var in ['EM_CACHE', 'EMCC_DEBUG', 'EM_CONFIG',
              'EMMAKEN_JUST_CONFIGURE', 'EMCC_CFLAGS', 'EMCC_TEMP_DIR',
              'EMCC_AUTODEBUG', 'EM_COMPILER_WRAPPER',
              'MOZ_DISABLE_AUTO_SAFE_MODE', 'EMCC_STDERR_FILE',
              'EMSCRIPTEN_SUPPRESS_USAGE_WARNING', 'NODE_PATH', 'EMCC_JSOPT_MIN_CHUNK_SIZE',
              'EMCC_JSOPT_MAX_CHUNK_SIZE', 'EMCC_CORES', 'EMCC_NO_OPT_SORT',
              'EMCC_BUILD_DIR', 'EMCC_DEBUG_SAVE', 'EMCC_SKIP_SANITY_CHECK',
              'EM_PKG_CONFIG_PATH', 'EMCC_CLOSURE_ARGS', 'JAVA_HEAP_SIZE',
              'EMCC_FORCE_STDLIBS', 'EMCC_ONLY_FORCED_STDLIBS', 'EM_PORTS', 'IDL_CHECKS', 'IDL_VERBOSE']:
    if os.environ.get(var):
      env['ENV'][var] = os.environ.get(var)
  try:
    emscPath = emscripten_path.abspath
  except:
    emscPath = emscripten_path

  env.Replace(CC=os.path.join(emscPath, "emcc"))
  env.Replace(CXX=os.path.join(emscPath, "em++"))
  # LINK uses smark_link by default which will choose
  # either emcc or em++ depending on if there are any C++ sources
  # in the program, so no need to change that.
  # SHLINK and LDMODULE should use LINK so no
  # need to change them here

  env.Replace(AR=os.path.join(emscPath, "emar"))
  env.Replace(RANLIB=os.path.join(emscPath, "emranlib"))

  env.Replace(OBJSUFFIX=[".js", ".bc", ".o"][2])
  env.Replace(LIBSUFFIX=[".js", ".bc", ".o"][2])
  env.Replace(PROGSUFFIX=[".html", ".js"][1])


def exists(env):
  """ NOOP method required by SCons """
  return 1
