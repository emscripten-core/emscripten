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
  for var in ['EM_CACHE', 'EMCC_DEBUG', 'EMSCRIPTEN_NATIVE_OPTIMIZER', 'EMTEST_BROWSER',
              'EMMAKEN_JUST_CONFIGURE', 'EMCC_CFLAGS', 'EMCC_LEAVE_INPUTS_RAW', 'EMCC_TEMP_DIR',
              'EMCC_AUTODEBUG', 'EMMAKEN_JUST_CONFIGURE_RECURSE', 'EMCONFIGURE_JS', 'CONFIGURE_CC',
              'EMMAKEN_COMPILER', 'EMMAKEN_CFLAGS', 'EMCC_JSOPT_BLACKLIST',
              'MOZ_DISABLE_AUTO_SAFE_MODE', 'EMCC_STDERR_FILE',
              'EMSCRIPTEN_SUPPRESS_USAGE_WARNING', 'EM_SAVE_DIR', 'NODE_PATH', 'EMCC_JSOPT_MIN_CHUNK_SIZE',
              'EMCC_JSOPT_MAX_CHUNK_SIZE', 'EMCC_SAVE_OPT_TEMP', 'EMCC_CORES', 'EMCC_NO_OPT_SORT',
              'EMCC_BUILD_DIR', 'EM_POPEN_WORKAROUND', 'EMCC_DEBUG_SAVE', 'EMCC_SKIP_SANITY_CHECK',
              'EMMAKEN_NO_SDK', 'EM_PKG_CONFIG_PATH', 'EMCC_CLOSURE_ARGS', 'JAVA_HEAP_SIZE',
              'EMCC_FORCE_STDLIBS', 'EMCC_ONLY_FORCED_STDLIBS', 'EM_PORTS', 'IDL_CHECKS', 'IDL_VERBOSE',
              'EMTEST_SAVE_DIR']:
    if os.environ.get(var):
      env['ENV'][var] = os.environ.get(var)
  try:
    emscPath = emscripten_path.abspath
  except:
    emscPath = emscripten_path

  env.Replace(CC=os.path.join(emscPath, "emcc"))
  env.Replace(CXX=os.path.join(emscPath, "em++"))
  env.Replace(LINK=os.path.join(emscPath, "emcc"))
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
