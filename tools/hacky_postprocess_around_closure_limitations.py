# This file further processes an output JS file to optimize down to even smaller size that Closure is unable to do
import os
import re
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.append(__rootdir__)

from tools import building
from tools.utils import read_file, write_file

f = read_file(sys.argv[1])
orig_size = len(f)

f = f.strip()

# The following are introduced to the output after Closure has run, so need to manually optimize them out.
f = re.sub(r'\s*//\s*EMSCRIPTEN_START_ASM\s*', '', f)
f = re.sub(r'\s*//\s*EMSCRIPTEN_START_FUNCS\s*', '', f)
f = re.sub(r'\s*//\s*EMSCRIPTEN_END_ASM\s*', '', f)
f = re.sub(r'\s*//\s*EMSCRIPTEN_END_FUNCS\s*', '', f)
f = re.sub(r'\s*/\*\* @suppress {uselessCode} \*/\s*', '', f)

f = f.replace('Module["asm"] = (function(global,env,buffer) {', 'Module["asm"]=(function(global,env,buffer){')

# https://github.com/google/closure-compiler/issues/3185
f = re.sub(r';new Int8Array\(\w+\);', ';', f)
f = re.sub(r';new Int16Array\(\w+\);', ';', f)
f = re.sub(r';new Int32Array\(\w+\);', ';', f)
f = re.sub(r';new Uint8Array\(\w+\);', ';', f)
f = re.sub(r';new Uint16Array\(\w+\);', ';', f)
f = re.sub(r';new Uint32Array\(\w+\);', ';', f)
f = re.sub(r';new Float32Array\(\w+\);', ';', f)
f = re.sub(r';new Float64Array\(\w+\);', ';', f)
f = f.replace(';new TextDecoder("utf8");', ';')
f = f.replace('}new TextDecoder("utf8");', '}')
f = f.replace(';new TextDecoder("utf-16le");', ';')
f = f.replace('}new TextDecoder("utf-16le");', '}')

# var a;a||(a=Module)
# ->
# var a=Module;
f = re.sub(r'var (\w);\1\|\|\(\1=Module\);', r'var \1=Module;', f)

# var Module=function(Module){Module =Module || {};var a=Module;
# ->
# var Module=function(a){
f = re.sub(r'\s*function\s*\(Module\)\s*{\s*Module\s*=\s*Module\s*\|\|\s*{\s*}\s*;\s*var\s+(\w+)\s*=\s*Module\s*;', r'function(\1){', f)

# Same as above but for arrow function
f = re.sub(r'\s*\(Module\)\s*=>\s*{\s*Module\s*=\s*Module\s*\|\|\s*{\s*}\s*;\s*var\s+(\w+)\s*=\s*Module\s*;', r'(\1)=>{', f)

f = re.sub(r'\s+', ' ', f)
f = re.sub(r'[\n\s]+\n\s*', '\n', f)
f = re.sub(r'([;{}=,\+\-\*/\(\)\[\]])[\n]', r'\1', f)
f = re.sub(r'([;{}=,\*/\(\)\[\]])[\s]', r'\1', f)

# Finally, rerun minifier because the above changes may have left redundant whitespaces
write_file(sys.argv[1], f)
minified = building.acorn_optimizer(sys.argv[1], ['minifyWhitespace'], return_output=True)
write_file(sys.argv[1], minified)

# optimized_size = len(f)
# print('Further optimized ' + str(optimized_size - orig_size) + ' bytes (' + str(orig_size) + ' -> ' + str(optimized_size) + ' bytes, {0:.2f}'.format((optimized_size-orig_size)*100.0/orig_size) + '%)')
write_file(sys.argv[1], f)
