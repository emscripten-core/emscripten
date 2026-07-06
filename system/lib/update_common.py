import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')


def get_llvm_dir():
  if len(sys.argv) > 1:
    llvm_dir = os.path.abspath(sys.argv[1])
  else:
    llvm_dir = default_llvm_dir
  if not os.path.isdir(llvm_dir):
    print(f'LLVM directory not found: {llvm_dir}', file=sys.stderr)
    print(f'Usage: {sys.argv[0]} [llvm_dir]', file=sys.stderr)
    sys.exit(1)
  return llvm_dir


def clean_dir(dirname, preserve_files=()):
  if not os.path.exists(dirname):
    return
  for f in os.listdir(dirname):
    if f in preserve_files or 'emscripten' in f:
      continue
    full = os.path.join(dirname, f)
    if os.path.isdir(full):
      shutil.rmtree(full)
    else:
      os.remove(full)


def copy_tree(upstream_dir, local_dir, excludes=()):
  if not os.path.exists(local_dir):
    os.makedirs(local_dir)
  for f in os.listdir(upstream_dir):
    full = os.path.join(upstream_dir, f)
    if os.path.isdir(full):
      shutil.copytree(full, os.path.join(local_dir, f))
    elif f not in excludes:
      shutil.copy2(full, os.path.join(local_dir, f))
  if excludes:
    for root, dirs, files in os.walk(local_dir):
      for f in files:
        if f in excludes:
          full = os.path.join(root, f)
          os.remove(full)
