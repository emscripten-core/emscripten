#!/usr/bin/env python2
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os


def path_to_bin(emscripten_root, bin_name):
  if emscripten_root is None:
    return bin_name
  else:
    return os.path.join(emscripten_root, bin_name)


def generate(env, **kw):
  """ SCons tool entry point """

  # SCons does not by default invoke the compiler with the
  # environment variables from the parent calling process,
  # so manually route all environment variables referenced
  # by Emscripten to the child.
  emscripten_env_vars = ['EM_CACHE', 'EMCC_DEBUG', 'EM_CONFIG',
    'EMMAKEN_JUST_CONFIGURE', 'EMCC_CFLAGS', 'EMCC_TEMP_DIR',
    'EMCC_AUTODEBUG', 'EM_COMPILER_WRAPPER', 'MOZ_DISABLE_AUTO_SAFE_MODE',
    'EMCC_STDERR_FILE', 'EMSCRIPTEN_SUPPRESS_USAGE_WARNING', 'NODE_PATH',
    'EMCC_JSOPT_MIN_CHUNK_SIZE', 'EMCC_JSOPT_MAX_CHUNK_SIZE',
    'EMCC_CORES', 'EMCC_NO_OPT_SORT', 'EMCC_BUILD_DIR',
    'EMCC_DEBUG_SAVE', 'EMCC_SKIP_SANITY_CHECK', 'EM_PKG_CONFIG_PATH',
    'EMCC_CLOSURE_ARGS', 'EMCC_FORCE_STDLIBS',
    'EMCC_ONLY_FORCED_STDLIBS', 'EM_PORTS', 'IDL_CHECKS', 'IDL_VERBOSE']

  pkg_config_vars = {
    'EMSCONS_PKG_CONFIG_LIBDIR': 'PKG_CONFIG_LIBDIR',
    'EMSCONS_PKG_CONFIG_PATH': 'PKG_CONFIG_PATH',
  }

  for var in emscripten_env_vars:
    if var in os.environ:
      env['ENV'][var] = os.environ[var]

  for var in pkg_config_vars:
    if var in os.environ:
      real_key = pkg_config_vars[var]
      env['ENV'][real_key] = os.environ[var]

  # Binary paths will be constructed from here if available.
  # Otherwise they are assumed to be in the PATH.
  emscripten_root = os.environ.get('EMSCRIPTEN_ROOT')

  env.Replace(CC=path_to_bin(emscripten_root, 'emcc'))
  env.Replace(CXX=path_to_bin(emscripten_root, 'em++'))
  # LINK uses smark_link by default which will choose
  # either emcc or em++ depending on if there are any C++ sources
  # in the program, so no need to change that.
  # SHLINK and LDMODULE should use LINK so no
  # need to change them here

  env.Replace(AR=path_to_bin(emscripten_root, 'emar'))
  env.Replace(RANLIB=path_to_bin(emscripten_root, 'emranlib'))

  env.Replace(PROGSUFFIX='.js')


def exists(env):
  """ NOOP method required by SCons """
  return 1
