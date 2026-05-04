# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import argparse
import os
import shlex
import subprocess

from browser_common import BrowserCore, get_browser, has_browser
from common import EMRUN, RunnerCore, path_from_root, read_file, test_file
from decorators import also_with_pthreads

from tools.shared import EMCC, PIPE


class emrun(RunnerCore):
  def test_emrun_info(self):
    if not has_browser():
      self.skipTest('need a browser')
    result = self.run_process([EMRUN, '--system-info', '--browser_info'], stdout=PIPE).stdout
    assert 'CPU' in result
    assert 'Browser' in result
    assert 'Traceback' not in result

    result = self.run_process([EMRUN, '--list-browsers'], stdout=PIPE).stdout
    assert 'Traceback' not in result

  def test_no_browser(self):
    # Test --no-browser mode where we have to take care of launching the browser ourselves
    # and then killing emrun when we are done.
    if not has_browser():
      self.skipTest('need a browser')

    self.run_process([EMCC, test_file('test_emrun.c'), '--emrun', '-o', 'hello_world.html'])
    proc = subprocess.Popen([EMRUN, '--no-browser', '.', '--port=3333'], stdout=PIPE)
    try:
      if get_browser():
        url = 'http://localhost:3333/hello_world.html?argv0'
        print(f'Starting browser to {url}')
        BrowserCore.browser_open(url)
        try:
          while True:
            stdout = proc.stdout.read()
            if b'Dumping out file' in stdout:
              break
        finally:
          print('Terminating browser')
          BrowserCore.browser_terminate()
    finally:
      print('Terminating emrun server')
      proc.terminate()
      proc.wait()

  def test_program_arg_separator(self):
    # Verify that trying to pass argument to the page without the `--` separator will
    # generate an actionable error message
    err = self.expect_fail([EMRUN, '--foo'])
    self.assertContained('error: unrecognized arguments: --foo', err)
    self.assertContained('remember to add `--` between arguments', err)

  @also_with_pthreads
  def test_emrun(self):
    self.emcc('test_emrun.c', ['--emrun', '-o', 'test_emrun.html'])
    if not has_browser():
      self.skipTest('need a browser')

    # We cannot run emrun from the temp directory the suite will clean up afterwards, since the
    # browser that is launched will have that directory as startup directory, and the browser will
    # not close as part of the test, pinning down the cwd on Windows and it wouldn't be possible to
    # delete it. Therefore switch away from that directory before launching.
    os.chdir(path_from_root())

    # emrun tests may run in parallel processes, so each test case should use a unique port number
    # to avoid port address already in use errors.
    port = '6939'
    if '-pthread' in self.cflags:
      port = '6940'

    args_base = [EMRUN, '--timeout', '30', '--safe_firefox_profile',
                 '--kill-exit', '--port', port, '--verbose',
                 '--log-stdout', self.in_dir('stdout.txt'),
                 '--log-stderr', self.in_dir('stderr.txt')]

    if get_browser() is not None:
      # If EMTEST_BROWSER carried command line arguments to pass to the browser,
      # (e.g. "firefox -profile /path/to/foo") those can't be passed via emrun,
      # so strip them out.
      browser_cmd = shlex.split(get_browser())
      browser_path = browser_cmd[0]
      args_base += ['--browser', browser_path]
      if len(browser_cmd) > 1:
        browser_args = browser_cmd[1:]
        if 'firefox' in browser_path and ('-profile' in browser_args or '--profile' in browser_args):
          # emrun uses its own -profile, strip it out
          parser = argparse.ArgumentParser(add_help=False) # otherwise it throws with -headless
          parser.add_argument('-profile')
          parser.add_argument('--profile')
          browser_args = parser.parse_known_args(browser_args)[1]
        if browser_args:
          args_base += ['--browser_args', ' ' + ' '.join(browser_args)]

    for args in [
        [],
        ['--port', '0'],
        ['--private_browsing'],
        ['--dump_out_directory', 'other dir/multiple'],
        ['--dump_out_directory=foo_bar'],
    ]:
      args = args_base + args + [self.in_dir('test_emrun.html'), '--', '1', '2', '--3', 'escaped space', 'with_underscore']
      print(shlex.join(args))
      proc = self.run_process(args, check=False)
      self.assertEqual(proc.returncode, 100)
      dump_dir = 'dump_out'
      if '--dump_out_directory' in args:
        dump_dir = 'other dir/multiple'
      elif '--dump_out_directory=foo_bar' in args:
        dump_dir = 'foo_bar'
      self.assertExists(self.in_dir(f'{dump_dir}/test.dat'))
      self.assertExists(self.in_dir(f'{dump_dir}/heap.dat'))
      self.assertExists(self.in_dir(f'{dump_dir}/nested/with space.dat'))
      stdout = read_file(self.in_dir('stdout.txt'))
      stderr = read_file(self.in_dir('stderr.txt'))
      self.assertContained('argc: 6', stdout)
      self.assertContained('argv[3]: --3', stdout)
      self.assertContained('argv[4]: escaped space', stdout)
      self.assertContained('argv[5]: with_underscore', stdout)
      self.assertContained('Hello, world!', stdout)
      self.assertContained('Testing ASCII characters: !"$%&\'()*+,-./:;<=>?@[\\]^_`{|}~', stdout)
      self.assertContained('Testing char sequences: %20%21 &auml;', stdout)
      self.assertContained('hello, error stream!', stderr)
