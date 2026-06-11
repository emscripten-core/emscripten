import os
import sys
import subprocess
import tempfile
import shutil
from pathlib import Path

cwd = Path(__file__).parent
os.chdir(cwd)

def main():
  with tempfile.TemporaryDirectory() as tmp:
    src = os.path.abspath('modules')
    install_prefix = src + '/prefix'

    lib = 'lib/emscripten'
    share = 'share/libc++/v1'

    build_dir = 'out'
    dist = f'{tmp}/{build_dir}/dist/'
    build_lib = dist + lib
    build_share = dist + share

    vars = [
      ('LIBCXX_INSTALL_LIBRARY_DIR',  lib),
      ('LIBCXX_INSTALL_MODULES_DIR',  share),
      ('LIBCXX_LIBRARY_DIR',          build_lib),
      ('LIBCXX_GENERATED_MODULE_DIR', build_share),
    ]

    args = ''
    for var in vars:
      args += f' -D{var[0]}={var[1]}'

    with open(tmp + '/CMakeLists.txt', 'x', encoding='utf-8') as CMakeLists:
      CMakeLists.write(f'''\
        cmake_minimum_required(VERSION 3.10)
        project(libcxx-modules)
        add_subdirectory("{src}" libcxx)
      ''')

    subprocess.run(f'cmake -B {build_dir} -S {tmp} {args}', cwd=tmp, shell=True)
    subprocess.run(['cmake', '--build', build_dir], cwd=tmp)
    shutil.copytree(dist, install_prefix, dirs_exist_ok=True)  

  return 0

if __name__ == '__main__':
  try:
    sys.exit(main())
  except KeyboardInterrupt:
    sys.exit(1)  
