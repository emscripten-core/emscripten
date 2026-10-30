#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Build CPython against this emscripten checkout and run its test suite.

This script is run in CI, but is also runnable locally. CPython is
shallow-cloned into `out/cpython` and synced to a pinned revision. Requires
Python >= 3.12.

  ./test/cpython_test.py                      # same as `all`: sync, build and test
  ./test/cpython_test.py sync                 # clone/checkout the pinned CPython revision
  ./test/cpython_test.py build                # build an existing checkout
  ./test/cpython_test.py test                 # run the tests on an existing build
  ./test/cpython_test.py test test_os test_io # run only specific tests

"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, __rootpath__)

from tools import config
from tools.utils import path_from_root

CPYTHON_REPO = 'https://github.com/python/cpython.git'

# The CPython revision to test.  This must be recent enough to not rely on
# `FS.filesystems`.  Once CPython 3.16.0a2 comes out we can switch to a tag.
CPYTHON_REVISION = '13ca348ddee6db1b5f2216c293295ed3c04200f5'

DEFAULT_CPYTHON_DIR = path_from_root('out', 'cpython')

# Minimum Python version required to run CPython's Platforms/emscripten build
# scripts
MIN_BUILD_PYTHON = (3, 12)


def run(cmd, **kwargs):
  print('+ ' + ' '.join(str(c) for c in cmd), flush=True)
  return subprocess.run([str(c) for c in cmd], check=True, **kwargs)

def node_path():
  # config.NODE_JS is always normalized to a list (see config.listify).
  if not config.NODE_JS:
    sys.exit('error: NODE_JS not set in emscripten config')
  return config.NODE_JS[0]


def git_head(cpython_dir):
  """Return the sha of HEAD, or None if the repo has no commits yet."""
  proc = subprocess.run(['git', 'rev-parse', '--verify', '-q', 'HEAD'], cwd=cpython_dir,
                        text=True, capture_output=True)
  return proc.stdout.strip() if proc.returncode == 0 else None


def sync(args):
  """Ensure a CPython checkout exists at `cpython_dir` and is at `revision`."""
  cpython_dir = Path(args.cpython_dir)
  revision = args.revision
  if not (cpython_dir / '.git').exists():
    print(f'Cloning CPython into {cpython_dir}')
    cpython_dir.mkdir(parents=True, exist_ok=True)
    run(['git', 'init', '-q', cpython_dir])
    run(['git', 'remote', 'add', 'origin', CPYTHON_REPO], cwd=cpython_dir)
  elif git_head(cpython_dir) == revision:
    print(f'CPython checkout at {cpython_dir} is already at {revision}')
    return 0

  print(f'Syncing CPython checkout at {cpython_dir} to {revision}')
  # Shallow fetch of a single commit rather than a full clone.  GitHub allows
  # fetching arbitrary commits by sha.
  run(['git', 'fetch', '-q', '--depth=1', 'origin', revision], cwd=cpython_dir)
  run(['git', 'checkout', '-q', '--detach', 'FETCH_HEAD'], cwd=cpython_dir)
  return 0


def build_env():
  """Environment for running the CPython build scripts and the built python.

  Ensures that emcc/emconfigure/etc from this checkout are used.
  """
  env = os.environ.copy()
  env['PATH'] = path_from_root() + os.pathsep + env.get('PATH', '')
  # The CPython build scripts can install their own emsdk if this is set;
  # we always want to use the local emscripten.
  env.pop('EMSDK_CACHE', None)
  if config.EM_CONFIG:
    env['EM_CONFIG'] = config.EM_CONFIG
  return env


def build_script_runner(args):
  """Return (cpython_dir, step) where step(*cmd) runs CPython's Platforms/emscripten script."""
  cpython_dir = Path(args.cpython_dir)
  if not (cpython_dir / 'configure').exists():
    sys.exit(f'error: no CPython checkout at {cpython_dir}; run `{sys.argv[0]} sync` first')
  build_script = cpython_dir / 'Platforms' / 'emscripten'
  if not build_script.exists():
    sys.exit(f'error: {build_script} not found; CPython checkout is too old')
  env = build_env()

  def step(*cmd):
    run([sys.executable, build_script, *cmd], cwd=cpython_dir, env=env)

  return cpython_dir, step


def build(args):
  cpython_dir, step = build_script_runner(args)
  print(f'Building CPython checkout at {cpython_dir} ({git_head(cpython_dir)})')

  configure_args = ['--config-cache']
  if args.pydebug:
    configure_args.append('--with-pydebug')
  step('configure-build-python', '--', *configure_args)
  step('make-build-python')
  step('make-dependencies')
  step('configure-host', '--host-runner', node_path(), '--', '--config-cache')
  step('make-host')
  step('pythoninfo-host')
  return 0


def test(args):
  cpython_dir, step = build_script_runner(args)
  python_sh = cpython_dir / 'cross-build' / 'wasm32-emscripten' / 'build' / 'python' / 'python.sh'
  if not python_sh.exists():
    sys.exit(f'error: {python_sh} not found; run `{sys.argv[0]} build` first')

  # `run --test` prepends the default test arguments from CPython's config.toml
  test_args = []
  if args.junit_xml:
    junit_xml = os.path.abspath(args.junit_xml)
    os.makedirs(os.path.dirname(junit_xml), exist_ok=True)
    test_args += ['--junit-xml', junit_xml]
  test_args += args.test_args
  step('run', '--test', '--', *test_args)
  return 0


def all_steps(args):
  sync(args)
  build(args)
  test(args)
  return 0


def main(argv):
  if sys.version_info < (3, 12):
    sys.exit("Requires Python version >= 3.12.0")

  parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
  subparsers = parser.add_subparsers(dest='subcommand')

  def add_common_args(p):
    p.add_argument('--cpython-dir', default=DEFAULT_CPYTHON_DIR,
                   help=f'CPython checkout to use (default: {DEFAULT_CPYTHON_DIR})')

  def add_sync_args(p):
    p.add_argument('--revision', default=CPYTHON_REVISION,
                   help=f'CPython git revision to sync to (default: {CPYTHON_REVISION})')

  def add_build_args(p):
    p.add_argument('--pydebug', action='store_true', help='Build CPython with --with-pydebug')

  def add_test_args(p):
    p.add_argument('--junit-xml', help='Write test results in junit xml format to this file')
    p.add_argument('test_args', nargs='*',
                   help='Extra arguments for `python -m test` (use `--` before any that start with `-`)')

  all_parser = subparsers.add_parser('all', help='Sync, build and test CPython (the default)')
  sync_parser = subparsers.add_parser('sync', help='Clone/checkout the pinned CPython revision')
  build_parser = subparsers.add_parser('build', help='Build an existing CPython checkout')
  test_parser = subparsers.add_parser('test', help='Run the test suite of an existing CPython build')
  for p in (all_parser, sync_parser, build_parser, test_parser):
    add_common_args(p)
  for p in (all_parser, sync_parser):
    add_sync_args(p)
  for p in (all_parser, build_parser):
    add_build_args(p)
  for p in (all_parser, test_parser):
    add_test_args(p)

  # Default to `all` if no subcommand is given.
  if not argv or (argv[0] not in subparsers.choices and argv[0] not in {'-h', '--help'}):
    argv = ['all', *argv]
  args = parser.parse_args(argv)

  dispatch = {
    'all': all_steps,
    'sync': sync,
    'build': build,
    'test': test,
  }
  try:
    return dispatch[args.subcommand](args)
  except subprocess.CalledProcessError as e:
    # The failing command has already printed its own output; avoid a noisy
    # traceback and just propagate its exit code.
    print(f'error: command failed with exit code {e.returncode}: {" ".join(e.cmd)}', file=sys.stderr)
    return e.returncode


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
