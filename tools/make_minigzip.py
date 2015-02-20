import os, sys
from subprocess import Popen, PIPE, STDOUT

import shared

print 'Building zlib'

emscripten_temp_dir = shared.get_emscripten_temp_dir()
zlib = shared.Building.build_library('zlib', emscripten_temp_dir, emscripten_temp_dir, ['libz.a'], make_args=['libz.a'], copy_project=True, source_dir=shared.path_from_root('tests', 'zlib'))[0]

print 'Building minigzip'

Popen([shared.PYTHON, shared.EMCC, '-O2', shared.path_from_root('tests', 'zlib', 'minigzip.c'), zlib, '-o', shared.path_from_root('tools', 'minigzip.js')]).communicate()

