# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = '11022021'
HASH = 'f770031ad6c2152cbed8c8eab8edf2be1d27f9e74bc255a9930c17019944ee5fdda5308ea992c66a78af9fe1d8dca090f6c956910ce323f8728247c10e44036b'


def needed(settings):
  return settings.USE_MODPLUG


def get(ports, settings, shared):
  ports.fetch_project('libmodplug', f'https://github.com/jancc/libmodplug/archive/v{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('libmodplug', 'libmodplug-' + TAG)
    src_dir = os.path.join(source_path, 'src')
    libmodplug_path = os.path.join(src_dir, 'libmodplug')

    config_h = os.path.join(os.path.dirname(__file__), 'libmodplug/config.h')
    shutil.copyfile(config_h, os.path.join(source_path, 'config.h'))

    flags = [
      '-Wno-deprecated-register',
      '-DOPT_GENERIC',
      '-DREAL_IS_FLOAT',
      '-DHAVE_CONFIG_H',
      '-DSYM_VISIBILITY',
      '-std=gnu++14',
      '-fno-exceptions',
      '-ffast-math',
      '-fno-common',
      '-fvisibility=hidden',
      '-I' + source_path,
      '-I' + libmodplug_path,
    ]
    srcs = [
      os.path.join(src_dir, 'fastmix.cpp'),
      os.path.join(src_dir, 'load_669.cpp'),
      os.path.join(src_dir, 'load_abc.cpp'),
      os.path.join(src_dir, 'load_amf.cpp'),
      os.path.join(src_dir, 'load_ams.cpp'),
      os.path.join(src_dir, 'load_dbm.cpp'),
      os.path.join(src_dir, 'load_dmf.cpp'),
      os.path.join(src_dir, 'load_dsm.cpp'),
      os.path.join(src_dir, 'load_far.cpp'),
      os.path.join(src_dir, 'load_it.cpp'),
      os.path.join(src_dir, 'load_j2b.cpp'),
      os.path.join(src_dir, 'load_mdl.cpp'),
      os.path.join(src_dir, 'load_med.cpp'),
      os.path.join(src_dir, 'load_mid.cpp'),
      os.path.join(src_dir, 'load_mod.cpp'),
      os.path.join(src_dir, 'load_mt2.cpp'),
      os.path.join(src_dir, 'load_mtm.cpp'),
      os.path.join(src_dir, 'load_okt.cpp'),
      os.path.join(src_dir, 'load_pat.cpp'),
      os.path.join(src_dir, 'load_psm.cpp'),
      os.path.join(src_dir, 'load_ptm.cpp'),
      os.path.join(src_dir, 'load_s3m.cpp'),
      os.path.join(src_dir, 'load_stm.cpp'),
      os.path.join(src_dir, 'load_ult.cpp'),
      os.path.join(src_dir, 'load_umx.cpp'),
      os.path.join(src_dir, 'load_wav.cpp'),
      os.path.join(src_dir, 'load_xm.cpp'),
      os.path.join(src_dir, 'mmcmp.cpp'),
      os.path.join(src_dir, 'modplug.cpp'),
      os.path.join(src_dir, 'snd_dsp.cpp'),
      os.path.join(src_dir, 'sndfile.cpp'),
      os.path.join(src_dir, 'snd_flt.cpp'),
      os.path.join(src_dir, 'snd_fx.cpp'),
      os.path.join(src_dir, 'sndmix.cpp'),
    ]

    ports.build_port(source_path, final, 'libmodplug', flags=flags, srcs=srcs)

    ports.install_headers(libmodplug_path, pattern="*.h", target='libmodplug')
    ports.install_headers(src_dir, pattern="modplug.h", target='libmodplug')
    ports.make_pkg_config('libmodplug', TAG, '-sUSE_MODPLUG')

  return [shared.cache.get_lib('libmodplug.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libmodplug.a')


def show():
  return 'libmodplug (-sUSE_MODPLUG or --use-port=libmodplug; public domain)'
