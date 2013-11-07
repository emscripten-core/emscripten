import os, sys
from subprocess import Popen, PIPE, STDOUT

import shared

print 'Building zlib'

zlib = shared.Building.build_library('zlib', shared.EMSCRIPTEN_TEMP_DIR, shared.EMSCRIPTEN_TEMP_DIR, ['libz.a'], make_args=['libz.a'], copy_project=True, source_dir=shared.path_from_root('tests', 'zlib'))[0]

print 'Building minigzip'

Popen(['python2', shared.EMCC, '-O2', shared.path_from_root('tests', 'zlib', 'minigzip.c'), zlib, '-o', shared.path_from_root('tools', 'minigzip.js')]).communicate()

