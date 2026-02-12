# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import platform
import sys

from tools.shared import CLANG_CC, CLANG_CXX, PIPE
from tools.utils import MACOS, WINDOWS, path_from_root, run_process

logger = logging.getLogger('clang_native')


def get_native_triple():
  # On Raspberry Pi 5, the target triple for native compilation must exactly
  # match 'aarch64-linux-gnu', or clang will not find the native sysroot.
  # Users on a Pi 5 can set environment variable
  # EMTEST_NATIVE_COMPILATION_TRIPLE=aarch64-linux-gnu
  # to be able to override the native triple for Pi 5 compilation.
  native_compilation_triple = os.getenv('EMTEST_NATIVE_COMPILATION_TRIPLE')
  if native_compilation_triple:
    return native_compilation_triple

  arch = {
      'aarch64': 'arm64',
      'arm64': 'arm64', # Python on Apple Silicon ARM64 reports lowercase arm64
      'ARM64': 'arm64', # Python on Windows-on-ARM reports uppercase ARM64
      'x86_64': 'x86_64',
      'AMD64': 'x86_64',
  }[platform.machine()]
  OS = {
      'linux': 'linux',
      'darwin': 'darwin',
      'win32': 'windows-msvc',
  }[sys.platform]
  return f'{arch}-{OS}'


# These extra args need to be passed to Clang when targeting a native host system executable
def get_clang_native_args():
  triple = ['--target=' + get_native_triple()]
  if MACOS:
    sysroot = run_process(['xcrun', '--show-sdk-path'], stdout=PIPE).stdout.strip()
    return triple + ['-isystem', path_from_root('system/include/libcxx'), f'--sysroot={sysroot}']
  elif os.name == 'nt':
    # TODO: If Windows.h et al. are needed, will need to add something like '-isystemC:/Program
    # Files (x86)/Microsoft SDKs/Windows/v7.1A/Include'.
    return triple + ['-DWIN32']
  else:
    return triple


# This environment needs to be present when targeting a native host system executable
CACHED_CLANG_NATIVE_ENV = None


def get_clang_native_env():
  global CACHED_CLANG_NATIVE_ENV
  if CACHED_CLANG_NATIVE_ENV is not None:
    return CACHED_CLANG_NATIVE_ENV
  env = os.environ.copy()

  env['CC'] = CLANG_CC
  env['CXX'] = CLANG_CXX
  env['LD'] = CLANG_CXX

  if MACOS:
    path = run_process(['xcrun', '--show-sdk-path'], stdout=PIPE).stdout.strip()
    logger.debug('Using MacOS SDKROOT: ' + path)
    env['SDKROOT'] = path
  elif WINDOWS:
    # If already running in Visual Studio Command Prompt manually, no need to
    # add anything here, so just return.
    if 'VSINSTALLDIR' in env and 'INCLUDE' in env and 'LIB' in env:
      CACHED_CLANG_NATIVE_ENV = env
      return env

    # VSINSTALLDIR is not in environment, so the user is not running in Visual Studio
    # Command Prompt. Attempt to autopopulate INCLUDE and LIB directives.

    # Guess where Visual Studio is installed (VSINSTALLDIR env. var in VS X64 Command Prompt)
    if 'VSINSTALLDIR' in env:
      visual_studio_path = env['VSINSTALLDIR']
    elif 'VS170COMNTOOLS' in env:
      visual_studio_path = os.path.normpath(os.path.join(env['VS170COMNTOOLS'], '../..'))
    elif 'ProgramFiles(x86)' in env:
      visual_studio_path = os.path.normpath(os.path.join(env['ProgramFiles(x86)'], 'Microsoft Visual Studio', '2022', 'Community'))
    elif 'ProgramFiles' in env:
      visual_studio_path = os.path.normpath(os.path.join(env['ProgramFiles'], 'Microsoft Visual Studio', '2022', 'Community'))
    else:
      visual_studio_path = 'C:\\Program Files\\Microsoft Visual Studio\\2022\\Community'
    if not os.path.isdir(visual_studio_path):
      raise Exception('Visual Studio was not found in "' + visual_studio_path + '"! Run in Visual Studio X64 command prompt to avoid the need to autoguess this location (or set VSINSTALLDIR env var).')

    # Guess where Program Files (x86) is located
    if 'ProgramFiles(x86)' in env:
      prog_files_x86 = env['ProgramFiles(x86)']
    elif 'ProgramFiles' in env:
      prog_files_x86 = env['ProgramFiles']
    elif os.path.isdir('C:\\Program Files (x86)'):
      prog_files_x86 = 'C:\\Program Files (x86)'
    elif os.path.isdir('C:\\Program Files'):
      prog_files_x86 = 'C:\\Program Files'
    else:
      raise Exception('Unable to detect Program files directory for native Visual Studio build!')

    # Find include directory root
    def highest_version_subdir(path):
      candidates = []
      for entry in os.listdir(path):
        full = os.path.join(path, entry)
        if os.path.isdir(full):
          try:
            candidates.append((tuple(int(p) for p in entry.split('.')), full))
          except ValueError:
            continue
      if len(candidates) == 0:
        return None
      return max(candidates, key=lambda x: x[0])[1]

    vc_root = os.path.join(visual_studio_path, 'VC')
    vc_code_root = highest_version_subdir(os.path.join(vc_root, 'Tools', 'MSVC'))
    if not vc_code_root:
      raise Exception('Unable to find Visual Studio INCLUDE root directory. Run in Visual Studio command prompt to avoid the need to autoguess this location.')

    windows_sdk_dir = highest_version_subdir(os.path.join(prog_files_x86, 'Windows Kits'))
    if not windows_sdk_dir:
      raise Exception('Unable to find Windows SDK root directory. Run in Visual Studio command prompt to avoid the need to autoguess this location.')

    env.setdefault('VSINSTALLDIR', visual_studio_path)
    env.setdefault('VCINSTALLDIR', os.path.join(visual_studio_path, 'VC'))

    windows_sdk_include_dir = highest_version_subdir(os.path.join(windows_sdk_dir, 'include'))
    windows_sdk_lib_dir = highest_version_subdir(os.path.join(windows_sdk_dir, 'lib'))

    def append_path_item(key, path):
      if not os.path.isdir(path):
        logger.warning(f'VS path {path} does not exist')
        return

      if key not in env or len(env[key].strip()) == 0:
        env[key] = path
      else:
        env[key] = env[key] + ';' + path

    append_path_item('INCLUDE', os.path.join(vc_code_root, 'include'))
    append_path_item('INCLUDE', os.path.join(vc_code_root, 'ATLMFC', 'include'))
    append_path_item('INCLUDE', os.path.join(vc_root, 'Auxiliary', 'VS', 'include'))
    for d in ['ucrt', 'um', 'shared', 'winrt', 'cppwinrt']:
      append_path_item('INCLUDE', os.path.join(windows_sdk_include_dir, d))
    logger.debug('Visual Studio native build INCLUDE: ' + env['INCLUDE'])

    append_path_item('LIB', os.path.join(vc_code_root, 'ATLMFC', 'lib', 'x64'))
    append_path_item('LIB', os.path.join(vc_code_root, 'lib', 'x64'))
    append_path_item('LIB', os.path.join(windows_sdk_lib_dir, 'ucrt', 'x64'))
    append_path_item('LIB', os.path.join(windows_sdk_lib_dir, 'um', 'x64'))
    logger.debug('Visual Studio native build LIB: ' + env['LIB'])

    env['PATH'] = env['PATH'] + ';' + os.path.join(env['VCINSTALLDIR'], 'BIN')
    logger.debug('Visual Studio native build PATH: ' + env['PATH'])

  CACHED_CLANG_NATIVE_ENV = env
  return env
