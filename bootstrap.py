#!/usr/bin/env python3
"""Bootstrap script for emscripten developers / git users.

After checking out emscripten there are certain steps that need to be
taken before it can be used.  This script enumerates and automates
these steps and is able to run just the steps that are needed based
on the timestamps of various input files (kind of like a dumb version
of a Makefile).
"""
import argparse
import os
import shutil
import subprocess
import sys

__rootdir__ = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, __rootdir__)

STAMP_DIR = os.path.join(__rootdir__, 'out')

# N.b. This script bootstrap.py cannot use 'from tools import shared',
# because shared.py requires that a valid .emscripten config is already
# created. Bootstrap.py needs to be run before an .emscripten config exists.
from tools import utils


def get_stamp_file(action_name):
  return os.path.join(STAMP_DIR, action_name.replace(' ', '_') + '.stamp')


def check_deps(name, deps):
  stamp_file = get_stamp_file(name)
  if not os.path.exists(stamp_file):
    return False
  for dep in deps:
    dep = utils.path_from_root(dep)
    if os.path.getmtime(dep) > os.path.getmtime(stamp_file):
      return False
  return True


def check():
  for name, deps, _ in actions:
    if not check_deps(name, deps):
      utils.exit_with_error(f'emscripten setup is not complete ("{name}" is out-of-date). Run `bootstrap.py` to update')


def ask_yes_no(question):
  while True:
    try:
      reply = input(f"{question} (y/n): ").strip().lower()
    except EOFError:
      return False

    if reply in {'y', 'yes'}:
      return True
    if reply in {'n', 'no'}:
      return False

    print("Invalid input. Please enter 'y' or 'n'.")


def maybe_install_hooks():
  if os.path.exists('.git/hooks/pre-push'):
    print('git hooks already installed; skipping')
    return
  if os.environ.get('CI') or not sys.stdin.isatty():
    # Do nothing when running in CI or non-interactive shell
    return
  if ask_yes_no('Install emscripten git hooks (see tools/maint/git-hooks)?'):
    install_hooks()


def install_hooks():
  if not os.path.exists(utils.path_from_root('.git')):
    print('--install-git-hooks requires a git checkout')
    return 1

  dst = utils.path_from_root('.git/hooks')
  if not os.path.exists(dst):
    os.mkdir(dst)

  for src in ('tools/maint/git-hooks/post-checkout', 'tools/maint/git-hooks/pre-push'):
    shutil.copy(utils.path_from_root(src), dst)
  return 0


def run_cmd(cmd):
  orig_exe = cmd[0]
  if not os.path.isabs(orig_exe):
    cmd[0] = shutil.which(orig_exe)
    if not cmd[0]:
      utils.exit_with_error(f'command not found: {orig_exe}')
  print(' -> %s' % ' '.join(cmd))
  subprocess.run(cmd, check=True, text=True, encoding='utf-8', cwd=utils.path_from_root())


def build_emcc_native():
  build_dir = utils.path_from_root('out/build_emcc_native')
  source_dir = utils.path_from_root('tools/emcc_native')
  cmd = ['cmake', '-B', build_dir, source_dir, '-DCMAKE_BUILD_TYPE=Release']
  if not utils.WINDOWS and shutil.which('ninja'):
    cmd.extend(['-G', 'Ninja'])
  run_cmd(cmd)
  run_cmd(['cmake', '--build', build_dir, '--config', 'Release'])
  run_cmd(['cmake', '--install', build_dir, '--config', 'Release'])


actions = [
  ('npm packages', [
     'package.json',
     'package-lock.json',
   ], ['npm', 'ci']),
  ('git submodules', [
     'test/third_party/posixtestsuite/',
     'test/third_party/googletest',
     'test/third_party/wasi-test-suite',
   ], ['git', 'submodule', 'update', '--init']),
  ('install hooks', [
     'tools/maint/git-hooks/post-checkout',
     'tools/maint/git-hooks/pre-push',
   ], maybe_install_hooks),
]

if os.environ.get('EMCC_NATIVE') == '0':
  actions.append(('legacy entry points', [
     'tools/maint/create_entry_points.py',
     'tools/maint/run_python.sh',
     'tools/maint/run_python.bat',
     'tools/maint/run_python.ps1',
     'tools/maint/run_python_compiler.sh',
     'tools/maint/run_python_compiler.bat',
     'tools/maint/run_python_compiler.ps1',
   ], [sys.executable, utils.path_from_root('tools/maint/create_entry_points.py')]))
else:
  actions.append(('build emcc_native', [
     'tools/emcc_native/CMakeLists.txt',
     'tools/emcc_native/main.cpp',
     'tools/emcc_native/diagnostics.cpp',
     'tools/emcc_native/diagnostics.h',
     'tools/emcc_native/driver.cpp',
     'tools/emcc_native/driver.h',
     'tools/emcc_native/exec.cpp',
     'tools/emcc_native/exec.h',
     'tools/emcc_native/config.cpp',
     'tools/emcc_native/config.h',
   ], build_emcc_native))


def main(args):
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-v', '--verbose', action='store_true', help='verbose', default=False)
  parser.add_argument('-f', '--force', action='store_true', help='force all actions to run', default=False)
  parser.add_argument('-n', '--dry-run', action='store_true', help='dry run', default=False)
  parser.add_argument('-i', '--install-git-hooks', action='store_true', help='install emscripten git hooks', default=False)
  args = parser.parse_args()

  if args.install_git_hooks:
    return install_hooks()

  for name, deps, action in actions:
    if not args.force:
      if check_deps(name, deps):
        print('Up-to-date: %s' % name)
        continue
      print('Out-of-date: %s' % name)
    if args.dry_run:
      if type(action) == list:
        action_str = ' '.join(action)
      else:
        action_str = action.__name__
      print(f' (skipping: dry run) -> {action_str}')
      continue
    if type(action) == list:
      run_cmd(action)
    else:
      action()
    utils.safe_ensure_dirs(STAMP_DIR)
    stamp_file = get_stamp_file(name)
    utils.write_file(stamp_file, 'Timestamp file created by bootstrap.py')
  return 0


if __name__ == '__main__':
  main(sys.argv[1:])
