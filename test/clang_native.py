# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import platform
import sys
from tools.shared import PIPE, run_process, CLANG_CC, CLANG_CXX
from tools.utils import MACOS, WINDOWS, path_from_root

logger = logging.getLogger('clang_native')


def get_native_triple():
  arch = {
      'aarch64': 'arm64',
      'arm64': 'arm64',
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
    return triple + ['-isystem', path_from_root('system/include/libcxx')]
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

    # Guess where VS2015 is installed (VSINSTALLDIR env. var in VS2015 X64 Command Prompt)
    if 'VSINSTALLDIR' in env:
      visual_studio_path = env['VSINSTALLDIR']
    elif 'VS140COMNTOOLS' in env:
      visual_studio_path = os.path.normpath(os.path.join(env['VS140COMNTOOLS'], '../..'))
    elif 'ProgramFiles(x86)' in env:
      visual_studio_path = os.path.normpath(os.path.join(env['ProgramFiles(x86)'], 'Microsoft Visual Studio 14.0'))
    elif 'ProgramFiles' in env:
      visual_studio_path = os.path.normpath(os.path.join(env['ProgramFiles'], 'Microsoft Visual Studio 14.0'))
    else:
      visual_studio_path = 'C:\\Program Files (x86)\\Microsoft Visual Studio 14.0'
    if not os.path.isdir(visual_studio_path):
      raise Exception('Visual Studio 2015 was not found in "' + visual_studio_path + '"! Run in Visual Studio X64 command prompt to avoid the need to autoguess this location (or set VSINSTALLDIR env var).')

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

    # Guess where Windows 8.1 SDK is located
    if 'WindowsSdkDir' in env:
      windows8_sdk_dir = env['WindowsSdkDir']
    else:
      windows8_sdk_dir = os.path.join(prog_files_x86, 'Windows Kits', '8.1')
    if not os.path.isdir(windows8_sdk_dir):
      raise Exception('Windows 8.1 SDK was not found in "' + windows8_sdk_dir + '"! Run in Visual Studio command prompt to avoid the need to autoguess this location (or set WindowsSdkDir env var).')

    # Guess where Windows 10 SDK is located
    if os.path.isdir(os.path.join(prog_files_x86, 'Windows Kits', '10')):
      windows10_sdk_dir = os.path.join(prog_files_x86, 'Windows Kits', '10')
    if not os.path.isdir(windows10_sdk_dir):
      raise Exception('Windows 10 SDK was not found in "' + windows10_sdk_dir + '"! Run in Visual Studio command prompt to avoid the need to autoguess this location.')

    env.setdefault('VSINSTALLDIR', visual_studio_path)
    env.setdefault('VCINSTALLDIR', os.path.join(visual_studio_path, 'VC'))

    windows10sdk_kits_include_dir = os.path.join(windows10_sdk_dir, 'Include')
    windows10sdk_kit_version_name = [x for x in os.listdir(windows10sdk_kits_include_dir) if os.path.isdir(os.path.join(windows10sdk_kits_include_dir, x))][0] # e.g. "10.0.10150.0" or "10.0.10240.0"

    def append_item(key, item):
      if key not in env or len(env[key].strip()) == 0:
        env[key] = item
      else:
        env[key] = env[key] + ';' + item

    append_item('INCLUDE', os.path.join(env['VCINSTALLDIR'], 'INCLUDE'))
    append_item('INCLUDE', os.path.join(env['VCINSTALLDIR'], 'ATLMFC', 'INCLUDE'))
    append_item('INCLUDE', os.path.join(windows10_sdk_dir, 'include', windows10sdk_kit_version_name, 'ucrt'))
    #   append_item('INCLUDE', 'C:\\Program Files (x86)\\Windows Kits\\NETFXSDK\\4.6.1\\include\\um') # VS2015 X64 command prompt has this, but not needed for Emscripten
    append_item('INCLUDE', os.path.join(env['VCINSTALLDIR'], 'ATLMFC', 'INCLUDE'))
    append_item('INCLUDE', os.path.join(windows8_sdk_dir, 'include', 'shared'))
    append_item('INCLUDE', os.path.join(windows8_sdk_dir, 'include', 'um'))
    append_item('INCLUDE', os.path.join(windows8_sdk_dir, 'include', 'winrt'))
    logger.debug('VS2015 native build INCLUDE: ' + env['INCLUDE'])

    append_item('LIB', os.path.join(env['VCINSTALLDIR'], 'LIB', 'amd64'))
    append_item('LIB', os.path.join(env['VCINSTALLDIR'], 'ATLMFC', 'LIB', 'amd64'))
    append_item('LIB', os.path.join(windows10_sdk_dir, 'lib', windows10sdk_kit_version_name, 'ucrt', 'x64'))
    #   append_item('LIB', 'C:\\Program Files (x86)\\Windows Kits\\NETFXSDK\\4.6.1\\lib\\um\\x64') # VS2015 X64 command prompt has this, but not needed for Emscripten
    append_item('LIB', os.path.join(windows8_sdk_dir, 'lib', 'winv6.3', 'um', 'x64'))
    logger.debug('VS2015 native build LIB: ' + env['LIB'])

    env['PATH'] = env['PATH'] + ';' + os.path.join(env['VCINSTALLDIR'], 'BIN')
    logger.debug('VS2015 native build PATH: ' + env['PATH'])

  # Current configuration above is all Visual Studio -specific, so on non-Windowses, no action needed.

  CACHED_CLANG_NATIVE_ENV = env
  return env
