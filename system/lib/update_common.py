import os
import sys
import shutil
import subprocess
import re
import argparse

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')


def parse_args(default_dir, dir_name='llvm_dir'):
  parser = argparse.ArgumentParser(description=f'Update library from {dir_name}')
  parser.add_argument('-f', '--force', action='store_true', help='Force update even if the Emscripten tree is not clean')
  parser.add_argument('src_dir', nargs='?', default=default_dir, help=f'Path to {dir_name}')
  args = parser.parse_args()

  src_dir = os.path.abspath(args.src_dir)
  if not os.path.isdir(src_dir):
    print(f'{dir_name} directory not found: {src_dir}', file=sys.stderr)
    parser.print_help(sys.stderr)
    sys.exit(1)

  return src_dir, args.force


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


def check_clean(force=False):
  if force:
    return
  try:
    status = subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=emscripten_root, stderr=subprocess.PIPE).decode('utf-8').strip()
    if status:
      print('Emscripten tree is not clean (has uncommitted changes).', file=sys.stderr)
      sys.exit(1)
  except subprocess.CalledProcessError as e:
    print('Emscripten tree is not a valid git repository or git failed.', file=sys.stderr)
    if e.stderr:
      print(e.stderr.decode('utf-8').strip(), file=sys.stderr)
    sys.exit(1)


def get_llvm_version(upstream_dir):
  cmake_file = os.path.join(upstream_dir, 'cmake', 'Modules', 'LLVMVersion.cmake')
  if not os.path.exists(cmake_file):
    return None, None
  with open(cmake_file, 'r') as f:
    content = f.read()
  major = re.search(r'set\(LLVM_VERSION_MAJOR\s+(\d+)\)', content)
  minor = re.search(r'set\(LLVM_VERSION_MINOR\s+(\d+)\)', content)
  patch = re.search(r'set\(LLVM_VERSION_PATCH\s+(\d+)\)', content)
  if major and minor and patch:
    return major.group(1), f'{major.group(1)}.{minor.group(1)}.{patch.group(1)}'
  return None, None


def update_readme(local_dir, llvm_dir):
  readme_path = os.path.join(local_dir, 'readme.txt')
  if not os.path.exists(readme_path):
    readme_path = os.path.join(local_dir, 'README.txt')
    if not os.path.exists(readme_path):
      return

  with open(readme_path, 'r') as f:
    content = f.read()

  major, full_version = get_llvm_version(llvm_dir)
  if major and full_version:
    # Find full version numbers in the form of x.y.z and update them
    content = re.sub(r'\b\d+\.\d+\.\d+\b', full_version, content)
    # Find 'emscripten-libs-NN' strings and update them with the major version
    content = re.sub(r'emscripten-libs-\d+', f'emscripten-libs-{major}', content)

  with open(readme_path, 'w') as f:
    f.write(content)
