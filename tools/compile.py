#!/usr/bin/env python3
# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""The functions in this file define the compiler flags that emcc passes to clang.

There are three different levels of flags, each one a superset of the next:

get_target_flags(): Defines just `-target` flags and should always be
used when calling clang, or any other llvm tool.

get_clang_flags(): In addition to the target flags this function returns all the
required compiler flags.

get_cflags(): In addition to compiler flags this function also returns pre-processor
flags. For example, include paths and macro defintions.
"""

import os

from . import building, cache, ports, shared
from .cmdline import SIMD_INTEL_FEATURE_TOWER, SIMD_NEON_FLAGS
from .settings import settings
from .utils import memoize


def get_target_flags():
  return ['-target', shared.get_llvm_target()]


def get_clang_flags(user_args):
  flags = get_target_flags()

  # if exception catching is disabled, we can prevent that code from being
  # generated in the frontend
  if settings.DISABLE_EXCEPTION_CATCHING and not settings.WASM_EXCEPTIONS:
    flags.append('-fignore-exceptions')

  if settings.INLINING_LIMIT:
    flags.append('-fno-inline-functions')

  if settings.PTHREADS:
    if '-pthread' not in user_args:
      flags.append('-pthread')
  elif settings.SHARED_MEMORY:
    if '-matomics' not in user_args:
      flags.append('-matomics')
    if '-mbulk-memory' not in user_args:
      flags.append('-mbulk-memory')

  if settings.RELOCATABLE and '-fPIC' not in user_args:
    flags.append('-fPIC')

  if settings.RELOCATABLE or settings.LINKABLE or '-fPIC' in user_args:
    if not any(a.startswith('-fvisibility') for a in user_args):
      # For relocatable code we default to visibility=default in emscripten even
      # though the upstream backend defaults visibility=hidden.  This matches the
      # expectations of C/C++ code in the wild which expects undecorated symbols
      # to be exported to other DSO's by default.
      flags.append('-fvisibility=default')

  if settings.LTO:
    if not any(a.startswith('-flto') for a in user_args):
      flags.append('-flto=' + settings.LTO)
    # setjmp/longjmp handling using Wasm EH
    # For non-LTO, '-mllvm -wasm-enable-eh' added in
    # building.llvm_backend_args() sets this feature in clang. But in LTO, the
    # argument is added to wasm-ld instead, so clang needs to know that EH is
    # enabled so that it can be added to the attributes in LLVM IR.
    if settings.SUPPORT_LONGJMP == 'wasm':
      flags.append('-mexception-handling')

  else:
    # In LTO mode these args get passed instead at link time when the backend runs.
    for a in building.llvm_backend_args():
      flags += ['-mllvm', a]

  return flags


@memoize
def get_cflags(user_args):
  # Flags we pass to the compiler when building C/C++ code
  # We add these to the user's flags (newargs), but not when building .s or .S assembly files
  cflags = get_clang_flags(user_args)
  cflags.append('--sysroot=' + cache.get_sysroot(absolute=True))

  if settings.EMSCRIPTEN_TRACING:
    cflags.append('-D__EMSCRIPTEN_TRACING__=1')

  if settings.SHARED_MEMORY:
    cflags.append('-D__EMSCRIPTEN_SHARED_MEMORY__=1')

  if settings.WASM_WORKERS:
    cflags.append('-D__EMSCRIPTEN_WASM_WORKERS__=1')

  if not settings.STRICT:
    # The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code
    # in strict mode. Code should use the define __EMSCRIPTEN__ instead.
    cflags.append('-DEMSCRIPTEN')

  ports.add_cflags(cflags, settings)

  def array_contains_any_of(hay, needles):
    for n in needles:
      if n in hay:
        return True

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER) or array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    if '-msimd128' not in user_args and '-mrelaxed-simd' not in user_args:
      shared.exit_with_error('passing any of ' + ', '.join(SIMD_INTEL_FEATURE_TOWER + SIMD_NEON_FLAGS) + ' flags also requires passing -msimd128 (or -mrelaxed-simd)!')
    cflags += ['-D__SSE__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[1:]):
    cflags += ['-D__SSE2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[2:]):
    cflags += ['-D__SSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[3:]):
    cflags += ['-D__SSSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[4:]):
    cflags += ['-D__SSE4_1__=1']

  # Handle both -msse4.2 and its alias -msse4.
  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[5:]):
    cflags += ['-D__SSE4_2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[7:]):
    cflags += ['-D__AVX__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[8:]):
    cflags += ['-D__AVX2__=1']

  if array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    cflags += ['-D__ARM_NEON__=1']

  if '-nostdinc' not in user_args:
    if not settings.USE_SDL:
      cflags += ['-Xclang', '-iwithsysroot' + os.path.join('/include', 'fakesdl')]
    cflags += ['-Xclang', '-iwithsysroot' + os.path.join('/include', 'compat')]

  return cflags
