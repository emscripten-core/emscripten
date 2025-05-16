# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

VERSION = '2.8.0'
HASH = '10b99739167bbcf3fec55446f0c54b4ddf04ad1403b52aeca26de937f95615d6f1ea880b031a4457559c345ea3c0bcff9af87f27b9f8cea98dc5444e8146962a'
PROJECT = 'utf8proc'


def needed(settings):
  return settings.USE_UTF8PROC


def get(ports, settings, shared):
    ports.fetch_project(
        PROJECT,
        f'https://github.com/JuliaStrings/utf8proc/archive/refs/tags/v{VERSION}.zip',
        sha512hash=HASH)

    def create(final):
        logging.info(f'building port: {PROJECT}')
        ports.clear_project_build(PROJECT)

        source_path = os.path.join(ports.get_dir(), PROJECT, f'{PROJECT}-{VERSION}')
        dest_path = os.path.join(ports.get_build_dir(), PROJECT)

        shutil.rmtree(dest_path, ignore_errors=True)
        shutil.copytree(source_path, dest_path)

        ports.install_headers(dest_path)
        ports.build_port(dest_path, final, PROJECT, srcs=['utf8proc.c'])

    return [shared.cache.get_lib(f'lib{PROJECT}.a', create, what='port')]


def clear(ports, settings, shared):
    shared.cache.erase_lib(f'lib{PROJECT}.a')


def process_args(ports):
    return []


def show():
    return f'{PROJECT} (USE_UTF8PROC=1; MIT license)'
