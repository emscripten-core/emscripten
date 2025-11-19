# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import atexit
import logging
import os
import plistlib
import queue
import re
import shlex
import shutil
import subprocess
import threading
import time
import webbrowser
from enum import Enum
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, unquote, unquote_plus, urlparse

import common
import psutil
from common import (
  TEST_ROOT,
  RunnerCore,
  compiler_for,
  create_file,
  errlog,
  force_delete_dir,
  maybe_test_file,
  read_file,
  record_flaky_test,
  test_file,
)

from tools import feature_matrix, shared, utils
from tools.feature_matrix import UNSUPPORTED
from tools.shared import DEBUG, EMCC, exit_with_error
from tools.utils import MACOS, WINDOWS, memoize, path_from_root, read_binary

logger = logging.getLogger('common')

# User can specify an environment variable EMTEST_BROWSER to force the browser
# test suite to run using another browser command line than the default system
# browser. If only the path to the browser executable is given, the tests
# will run in headless mode with a temporary profile with the same options
# used in CI. To use a custom start command specify the executable and command
# line flags.
#
# Note that when specifying EMTEST_BROWSER to run tests on a Safari browser:
# the command line must point to the root of the app bundle, and not to the
# Safari executable inside the bundle. I.e. pass EMTEST_BROWSER=/Applications/Safari.app
# instead of EMTEST_BROWSER=/Applications/Safari.app/Contents/MacOS/Safari
#
# There are two special values that can be used here if running in an actual
# browser is not desired:
#  EMTEST_BROWSER=0 : This will disable the actual running of the test and simply
#                     verify that it compiles and links.
#  EMTEST_BROWSER=node : This will attempt to run the browser test under node.
#                        For most browser tests this does not work, but it can
#                        be useful for running pthread tests under node.
EMTEST_BROWSER = None
EMTEST_BROWSER_AUTO_CONFIG = None
EMTEST_HEADLESS = None
EMTEST_CAPTURE_STDIO = int(os.getenv('EMTEST_CAPTURE_STDIO', '0'))

# Triggers the browser to restart after every given number of tests.
# 0: Disabled (reuse the browser instance to run all tests. Default)
# 1: Restart a fresh browser instance for every browser test.
# 2,3,...: Restart a fresh browser instance after given number of tests have been run in it.
# Helps with e.g. https://bugzil.la/1992558
EMTEST_RESTART_BROWSER_EVERY_N_TESTS = int(os.getenv('EMTEST_RESTART_BROWSER_EVERY_N_TESTS', '0'))

DEFAULT_BROWSER_DATA_DIR = path_from_root('out/browser-profile')

browser_spawn_lock_filename = path_from_root('out/browser_spawn_lock')


class Reporting(Enum):
  """When running browser tests we normally automatically include support
  code for reporting results back to the browser.  This enum allows tests
  to decide what type of support code they need/want.
  """
  NONE = 0
  # Include the JS helpers for reporting results
  JS_ONLY = 1
  # Include C/C++ reporting code (REPORT_RESULT macros) as well as JS helpers
  FULL = 2


def list_processes_by_name(exe_name):
  pids = []
  if exe_name:
    for proc in psutil.process_iter():
      try:
        pinfo = proc.as_dict(attrs=['pid', 'name', 'exe'])
        if pinfo['exe'] and exe_name in pinfo['exe'].replace('\\', '/').split('/'):
          pids.append(psutil.Process(pinfo['pid']))
      except psutil.NoSuchProcess: # E.g. "process no longer exists (pid=13132)" (code raced to acquire the iterator and process it)
        pass

  return pids


def terminate_list_of_processes(proc_list):
  for proc in proc_list:
    try:
      proc.terminate()
      # If the browser doesn't shut down gracefully (in response to SIGTERM)
      # after 2 seconds kill it with force (SIGKILL).
      try:
        proc.wait(2)
      except (subprocess.TimeoutExpired, psutil.TimeoutExpired):
        logger.info('Browser did not respond to `terminate`.  Using `kill`')
        proc.kill()
        proc.wait()
    except (psutil.NoSuchProcess, ProcessLookupError):
      pass


def init(force_browser_process_termination):
  utils.delete_file(browser_spawn_lock_filename)
  utils.delete_file(f'{browser_spawn_lock_filename}_counter')
  if force_browser_process_termination or os.getenv('EMTEST_FORCE_BROWSER_PROCESS_TERMINATION'):
    config = get_browser_config()

    if config and hasattr(config, 'executable_name'):
      def terminate_all_browser_processes():
        procs = list_processes_by_name(config.executable_name)
        if len(procs) > 0:
          print(f'Terminating {len(procs)} stray browser processes.')
          terminate_list_of_processes(procs)

      atexit.register(terminate_all_browser_processes)
      terminate_all_browser_processes()


def find_browser_test_file(filename):
  """Looks for files in test/browser and then in test/
  """
  if not os.path.exists(filename):
    fullname = test_file('browser', filename)
    if not os.path.exists(fullname):
      fullname = test_file(filename)
    filename = fullname
  return filename


@memoize
def get_safari_version():
  if not is_safari():
    return UNSUPPORTED
  plist_path = os.path.join(EMTEST_BROWSER.strip(), 'Contents', 'version.plist')
  version_str = plistlib.load(open(plist_path, 'rb')).get('CFBundleShortVersionString')
  # Split into parts (major.minor.patch)
  parts = (version_str.split('.') + ['0', '0', '0'])[:3]
  # Convert each part into integers, discarding any trailing string, e.g. '13a' -> 13.
  parts = [int(re.match(r"\d+", s).group()) if re.match(r"\d+", s) else 0 for s in parts]
  # Return version as XXYYZZ
  return parts[0] * 10000 + parts[1] * 100 + parts[2]


@memoize
def get_firefox_version():
  if not is_firefox():
    return UNSUPPORTED
  exe_path = shlex.split(EMTEST_BROWSER)[0]
  ini_path = os.path.join(os.path.dirname(exe_path), '../Resources/platform.ini' if MACOS else 'platform.ini')
  # Extract the first numeric part before any dot (e.g. "Milestone=102.15.1" → 102)
  m = re.search(r"^Milestone=(.*)$", read_file(ini_path), re.MULTILINE)
  milestone = m.group(1).strip()
  version = int(re.match(r"(\d+)", milestone).group(1))
  # On Nightly and Beta, e.g. 145.0a1, pretend it to still mean version 144,
  # since it is a pre-release version
  if any(c in milestone for c in ('a', 'b')):
    version -= 1
  return version


def browser_should_skip_feature(skip_env_var, feature):
  # If an env. var. EMTEST_LACKS_x to skip the given test is set (to either
  # value 0 or 1), don't bother checking if current browser supports the feature
  # - just unconditionally run the test, or skip the test.
  if os.getenv(skip_env_var) is not None:
    return int(os.getenv(skip_env_var)) != 0

  # If there is no Feature object associated with this capability, then we
  # should run the test.
  if feature is None:
    return False

  # If EMTEST_AUTOSKIP=0, also never skip.
  if os.getenv('EMTEST_AUTOSKIP') == '0':
    return False

  # Otherwise EMTEST_AUTOSKIP=1 or EMTEST_AUTOSKIP is not set: check whether
  # the current browser supports the test or not.
  min_required = feature_matrix.min_browser_versions[feature]
  not_supported = get_firefox_version() < min_required['firefox'] or get_safari_version() < min_required['safari']

  # Current browser does not support the test, and EMTEST_AUTOSKIP is not set?
  # Then error out to have end user decide what to do in this situation.
  if not_supported and os.getenv('EMTEST_AUTOSKIP') is None:
    return 'error'

  # Report whether to skip the test based on browser support.
  return not_supported


# Default flags used to run browsers in CI testing:
class ChromeConfig:
  data_dir_flag = '--user-data-dir='
  default_flags = (
    # --no-sandbox because we are running as root and chrome requires
    # this flag for now: https://crbug.com/638180
    '--no-first-run -start-maximized --no-sandbox --enable-unsafe-swiftshader --use-gl=swiftshader --enable-experimental-web-platform-features --enable-features=JavaScriptSourcePhaseImports',
    '--enable-experimental-webassembly-features --js-flags="--experimental-wasm-type-reflection --experimental-wasm-rab-integration"',
    # The runners lack sound hardware so fallback to a dummy device (and
    # bypass the user gesture so audio tests work without interaction)
    '--use-fake-device-for-media-stream --autoplay-policy=no-user-gesture-required',
    # Cache options.
    '--disk-cache-size=1 --media-cache-size=1 --disable-application-cache',
    # Disable various background tasks downloads (e.g. updates).
    '--disable-background-networking',
    # Disable native password pop-ups
    '--password-store=basic',
    # Send console messages to browser stderr
    '--enable-logging=stderr',
  )
  headless_flags = '--headless=new --window-size=1024,768'

  @staticmethod
  def configure(data_dir):
    """Chrome has no special configuration step."""

  @staticmethod
  def open_url_args(url):
    return [url]


class FirefoxConfig:
  data_dir_flag = '-profile '
  default_flags = ('-new-instance', '-wait-for-browser')
  headless_flags = '-headless'
  executable_name = utils.exe_suffix('firefox')

  @staticmethod
  def configure(data_dir):
    shutil.copy(test_file('firefox_user.js'), os.path.join(data_dir, 'user.js'))

  @staticmethod
  def open_url_args(url):
    # Firefox is able to launch URLs by passing them as positional arguments,
    # but not when the -wait-for-browser flag is in use (which we need to be
    # able to track browser liveness). So explicitly use -url option parameter
    # to specify the page to launch. https://bugzil.la/1996614
    return ['-url', url]


class SafariConfig:
  default_flags = ('', )
  executable_name = 'Safari'
  # For the macOS 'open' command, pass
  #   --new: to make a new Safari app be launched, rather than add a tab to an existing Safari process/window
  #   --fresh: do not restore old tabs (e.g. if user had old navigated windows open)
  #   --background: Open the new Safari window behind the current Terminal window, to make following the test run more pleasing (this is for convenience only)
  #   -a <exe_name>: The path to the executable to open, in this case Safari
  launch_prefix = ('open', '--new', '--fresh', '--background', '-a')

  @staticmethod
  def configure(data_dir):
    """ Safari has no special configuration step."""

  @staticmethod
  def open_url_args(url):
    return [url]


# checks if browser testing is enabled
def has_browser():
  return EMTEST_BROWSER != '0'


def get_browser():
  return EMTEST_BROWSER


CHROMIUM_BASED_BROWSERS = ['chrom', 'edge', 'opera']


def is_chrome():
  return EMTEST_BROWSER and any(pattern in EMTEST_BROWSER.lower() for pattern in CHROMIUM_BASED_BROWSERS)


def is_firefox():
  return EMTEST_BROWSER and 'firefox' in EMTEST_BROWSER.lower()


def is_safari():
  return EMTEST_BROWSER and 'safari' in EMTEST_BROWSER.lower()


def get_browser_config():
  if is_chrome():
    return ChromeConfig()
  elif is_firefox():
    return FirefoxConfig()
  elif is_safari():
    return SafariConfig()
  return None


def configure_test_browser():
  global EMTEST_BROWSER

  if not has_browser():
    return

  if not EMTEST_BROWSER:
    EMTEST_BROWSER = 'google-chrome'

  if WINDOWS and '"' not in EMTEST_BROWSER and "'" not in EMTEST_BROWSER:
    # On Windows env. vars canonically use backslashes as directory delimiters, e.g.
    # set EMTEST_BROWSER=C:\Program Files\Mozilla Firefox\firefox.exe
    # and spaces are not escaped. But make sure to also support args, e.g.
    # set EMTEST_BROWSER="C:\Users\clb\AppData\Local\Google\Chrome SxS\Application\chrome.exe" --enable-unsafe-webgpu
    EMTEST_BROWSER = '"' + EMTEST_BROWSER.replace("\\", "\\\\") + '"'

  if EMTEST_BROWSER_AUTO_CONFIG:
    config = get_browser_config()
    if config:
      EMTEST_BROWSER += ' ' + ' '.join(config.default_flags)
      if EMTEST_HEADLESS == 1:
        EMTEST_BROWSER += f" {config.headless_flags}"


# Create a server and a web page. When a test runs, we tell the server about it,
# which tells the web page, which then opens a window with the test. Doing
# it this way then allows the page to close() itself when done.
def make_test_server(in_queue, out_queue, port):
  class TestServerHandler(SimpleHTTPRequestHandler):
    # Request header handler for default do_GET() path in
    # SimpleHTTPRequestHandler.do_GET(self) below.
    def send_head(self):
      if self.headers.get('Range'):
        path = self.translate_path(self.path)
        try:
          fsize = os.path.getsize(path)
          f = open(path, 'rb')
        except OSError:
          self.send_error(404, f'File not found {path}')
          return None
        self.send_response(206)
        ctype = self.guess_type(path)
        self.send_header('Content-Type', ctype)
        pieces = self.headers.get('Range').split('=')[1].split('-')
        start = int(pieces[0]) if pieces[0] != '' else 0
        end = int(pieces[1]) if pieces[1] != '' else fsize - 1
        end = min(fsize - 1, end)
        length = end - start + 1
        self.send_header('Content-Range', f'bytes {start}-{end}/{fsize}')
        self.send_header('Content-Length', str(length))
        self.end_headers()
        return f
      else:
        return SimpleHTTPRequestHandler.send_head(self)

    # Add COOP, COEP, CORP, and no-caching headers
    def end_headers(self):
      self.send_header('Accept-Ranges', 'bytes')
      self.send_header('Access-Control-Allow-Origin', '*')
      self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
      self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
      self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')

      self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate, private, max-age=0')
      self.send_header('Expires', '0')
      self.send_header('Pragma', 'no-cache')
      self.send_header('Vary', '*') # Safari insists on caching if this header is not present in addition to the above

      return SimpleHTTPRequestHandler.end_headers(self)

    def do_POST(self):  # noqa: DC04
      urlinfo = urlparse(self.path)
      query = parse_qs(urlinfo.query)
      content_length = int(self.headers['Content-Length'])
      post_data = self.rfile.read(content_length)
      if urlinfo.path == '/log':
        # Logging reported by reportStdoutToServer / reportStderrToServer.
        #
        # To automatically capture stderr/stdout message from browser tests, modify
        # `captureStdoutStderr` in `test/browser_reporting.js`.
        filename = query['file'][0]
        print(f"[client {filename}: '{post_data.decode()}']")
        self.send_response(200)
        self.end_headers()
      elif urlinfo.path == '/upload':
        filename = query['file'][0]
        print(f'do_POST: got file: {filename}')
        create_file(filename, post_data, binary=True)
        self.send_response(200)
        self.end_headers()
      elif urlinfo.path.startswith('/status/'):
        code_str = urlinfo.path[len('/status/'):]
        code = int(code_str)
        if code in (301, 302, 303, 307, 308):
          self.send_response(code)
          self.send_header('Location', '/status/200')
          self.end_headers()
        elif code == 200:
          self.send_response(200)
          self.send_header('Content-type', 'text/plain')
          self.end_headers()
          self.wfile.write(b'OK')
        else:
          self.send_error(400, f'Not implemented for {code}')
      else:
        print(f'do_POST: unexpected POST: {urlinfo}')

    def do_GET(self):
      info = urlparse(self.path)
      if info.path == '/run_harness':
        if DEBUG:
          print('[server startup]')
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(read_binary(test_file('browser_harness.html')))
      elif info.path.startswith('/status/'):
        code_str = info.path[len('/status/'):]
        code = int(code_str)
        if code in (301, 302, 303, 307, 308):
          # Redirect to /status/200
          self.send_response(code)
          self.send_header('Location', '/status/200')
          self.end_headers()
        elif code == 200:
          self.send_response(200)
          self.send_header('Content-type', 'text/plain')
          self.end_headers()
          self.wfile.write(b'OK')
        else:
          self.send_error(400, f'Not implemented for {code}')
      elif 'report_' in self.path:
        # for debugging, tests may encode the result and their own url (window.location) as result|url
        if '|' in self.path:
          path, url = self.path.split('|', 1)
        else:
          path = self.path
          url = '?'
        if DEBUG:
          print('[server response:', path, url, ']')
        if out_queue.empty():
          out_queue.put(path)
        else:
          # a badly-behaving test may send multiple xhrs with reported results; we just care
          # about the first (if we queued the others, they might be read as responses for
          # later tests, or maybe the test sends more than one in a racy manner).
          # we place 'None' in the queue here so that the outside knows something went wrong
          # (none is not a valid value otherwise; and we need the outside to know because if we
          # raise an error in here, it is just swallowed in python's webserver code - we want
          # the test to actually fail, which a webserver response can't do).
          out_queue.put(None)
          raise Exception('browser harness error, excessive response to server - test must be fixed! "%s"' % self.path)
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.send_header('Connection', 'close')
        self.end_headers()
        self.wfile.write(b'OK')

      elif info.path == '/check':
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        if not in_queue.empty():
          # there is a new test ready to be served
          url, dir = in_queue.get()
          if DEBUG:
            print('[queue command:', url, dir, ']')
          assert in_queue.empty(), 'should not be any blockage - one test runs at a time'
          assert out_queue.empty(), 'the single response from the last test was read'
          # tell the browser to load the test
          self.wfile.write(b'COMMAND:' + url.encode('utf-8'))
        else:
          # the browser must keep polling
          self.wfile.write(b'(wait)')
      else:
        # Use SimpleHTTPServer default file serving operation for GET.
        if DEBUG:
          print('[simple HTTP serving:', unquote_plus(self.path), ']')
        if self.headers.get('Range'):
          self.send_response(206)
          path = self.translate_path(self.path)
          data = read_binary(path)
          ctype = self.guess_type(path)
          self.send_header('Content-type', ctype)
          pieces = self.headers.get('Range').split('=')[1].split('-')
          start = int(pieces[0]) if pieces[0] != '' else 0
          end = int(pieces[1]) if pieces[1] != '' else len(data) - 1
          end = min(len(data) - 1, end)
          length = end - start + 1
          self.send_header('Content-Length', str(length))
          self.send_header('Content-Range', f'bytes {start}-{end}/{len(data)}')
          self.end_headers()
          self.wfile.write(data[start:end + 1])
        else:
          SimpleHTTPRequestHandler.do_GET(self)

    def log_request(code=0, size=0):
      # don't log; too noisy
      pass

  # allows streaming compilation to work
  SimpleHTTPRequestHandler.extensions_map['.wasm'] = 'application/wasm'
  # Firefox browser security does not allow loading .mjs files if they
  # do not have the correct MIME type
  SimpleHTTPRequestHandler.extensions_map['.mjs'] = 'text/javascript'

  return ThreadingHTTPServer(('localhost', port), TestServerHandler)


class HttpServerThread(threading.Thread):
  """A generic thread class to create and run an http server."""
  def __init__(self, server):
    super().__init__()
    self.server = server

  def stop(self):
    """Shuts down the server if it is running."""
    self.server.shutdown()

  def run(self):
    """Creates the server instance and serves forever until stop() is called."""
    # Start the server's main loop (this blocks until shutdown() is called)
    self.server.serve_forever()


# This will hold the ID for each worker process if running in parallel mode,
# otherwise None if running in non-parallel mode.
worker_id = None


def init_worker(counter, lock):
  """ Initializer function for each worker.
  It acquires a lock, gets a unique ID from the shared counter,
  and stores it in a global variable specific to this worker process.
  """
  global worker_id
  with lock:
    # Get the next available ID
    worker_id = counter.value
    # Increment the counter for the next worker
    counter.value += 1


def move_browser_window(pid, x, y):
  """Utility function to move the top-level window owned by given process to
  (x,y) coordinate. Used to ensure each browser window has some visible area."""
  import win32con
  import win32gui
  import win32process

  def enum_windows_callback(hwnd, _unused):
    _, win_pid = win32process.GetWindowThreadProcessId(hwnd)
    if win_pid == pid and win32gui.IsWindowVisible(hwnd):
      # If the browser window is maximized, it won't react to MoveWindow, so
      # un-maximize the window first to show it in windowed mode.
      if win32gui.GetWindowPlacement(hwnd)[1] == win32con.SW_SHOWMAXIMIZED:
        win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)

      # Then cascade the window, but also resize the window size to cover a
      # smaller area of the desktop, in case the original size was full screen.
      win32gui.MoveWindow(hwnd, x, y, 800, 600, True)
    return True

  win32gui.EnumWindows(enum_windows_callback, None)


def increment_suffix_number(str_with_maybe_suffix):
  match = re.match(r"^(.*?)(?:_(\d+))?$", str_with_maybe_suffix)
  if match:
    base, number = match.groups()
    if number:
      return f'{base}_{int(number) + 1}'

  return f'{str_with_maybe_suffix}_1'


class FileLock:
  """Implements a filesystem-based mutex, with an additional feature that it
  returns an integer counter denoting how many times the lock has been locked
  before (during the current python test run instance)"""
  def __init__(self, path):
    self.path = path
    self.counter = 0

  def __enter__(self):
    # Acquire the lock
    while True:
      try:
        self.fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        break
      except FileExistsError:
        time.sleep(0.1)
    # Return the locking count number
    try:
      self.counter = int(open(f'{self.path}_counter').read())
    except Exception:
      pass
    return self.counter

  def __exit__(self, *a):
    # Increment locking count number before releasing the lock
    with open(f'{self.path}_counter', 'w') as f:
      f.write(str(self.counter + 1))
    # And release the lock
    os.close(self.fd)
    try:
      os.remove(self.path)
    except Exception:
      pass # Another process has raced to acquire the lock, and will delete it.


class BrowserCore(RunnerCore):
  # note how many tests hang / do not send an output. if many of these
  # happen, likely something is broken and it is best to abort the test
  # suite early, as otherwise we will wait for the timeout on every
  # single test (hundreds of minutes)
  MAX_UNRESPONSIVE_TESTS = 10
  BROWSER_TIMEOUT = 60

  unresponsive_tests = 0
  num_tests_ran = 0

  def __init__(self, *args, **kwargs):
    self.capture_stdio = EMTEST_CAPTURE_STDIO
    super().__init__(*args, **kwargs)

  @classmethod
  def browser_terminate(cls):
    terminate_list_of_processes(cls.browser_procs)

  @classmethod
  def browser_restart(cls):
    # Kill existing browser
    assert has_browser()
    logger.info('Restarting browser process')
    cls.browser_terminate()
    cls.browser_open(cls.HARNESS_URL)
    BrowserCore.num_tests_ran = 0

  @classmethod
  def browser_open(cls, url):
    assert has_browser()
    browser_args = EMTEST_BROWSER
    parallel_harness = worker_id is not None

    config = get_browser_config()
    if not config and EMTEST_BROWSER_AUTO_CONFIG:
      exit_with_error(f'EMTEST_BROWSER_AUTO_CONFIG only currently works with firefox, chrome and safari. EMTEST_BROWSER was "{EMTEST_BROWSER}"')

    # Prepare the browser data directory, if it uses one.
    if EMTEST_BROWSER_AUTO_CONFIG and config and hasattr(config, 'data_dir_flag'):
      logger.info('Using default CI configuration.')
      browser_data_dir = DEFAULT_BROWSER_DATA_DIR
      if parallel_harness:
        # Running in parallel mode, give each browser its own profile dir.
        browser_data_dir += '-' + str(worker_id)

      # Delete old browser data directory.
      if WINDOWS:
        # If we cannot (the data dir is in use on Windows), switch to another dir.
        while not force_delete_dir(browser_data_dir):
          browser_data_dir = increment_suffix_number(browser_data_dir)
      else:
        force_delete_dir(browser_data_dir)

      # Recreate the new data directory.
      os.mkdir(browser_data_dir)

      if WINDOWS:
        # Escape directory delimiter backslashes for shlex.split.
        browser_data_dir = browser_data_dir.replace('\\', '\\\\')
      config.configure(browser_data_dir)
      browser_args += f' {config.data_dir_flag}"{browser_data_dir}"'

    browser_args = shlex.split(browser_args)
    if hasattr(config, 'launch_prefix'):
      browser_args = list(config.launch_prefix) + browser_args

    logger.info('Launching browser: %s', str(browser_args))

    if (WINDOWS and is_firefox()) or is_safari():
      cls.launch_browser_harness_with_proc_snapshot_workaround(parallel_harness, config, browser_args, url)
    else:
      cls.browser_procs = [subprocess.Popen(browser_args + config.open_url_args(url))]

  @classmethod
  def launch_browser_harness_with_proc_snapshot_workaround(cls, parallel_harness, config, browser_args, url):
    ''' Dedicated function for launching browser harness in scenarios where
    we need to identify the launched browser processes via a before-after
    subprocess snapshotting delta workaround.'''

    # In order for this to work, each browser needs to be launched one at a time
    # so that we know which process belongs to which browser.
    with FileLock(browser_spawn_lock_filename) as count:
      # Take a snapshot before spawning the browser to find which processes
      # existed before launching the browser.
      if parallel_harness or is_safari():
        procs_before = list_processes_by_name(config.executable_name)

      # Browser launch
      cls.browser_procs = [subprocess.Popen(browser_args + config.open_url_args(url))]

      # Give the browser time to spawn its subprocesses. Use an increasing
      # timeout as a crude way to account for system load.
      if parallel_harness or is_safari():
        time.sleep(min(5 + count * 0.3, 10))
        procs_after = list_processes_by_name(config.executable_name)

        # Take a snapshot again to find which processes exist after launching
        # the browser. Then the newly launched browser processes are determined
        # by the delta before->after.
        cls.browser_procs += list(set(procs_after).difference(set(procs_before)))
        if len(cls.browser_procs) == 0:
          exit_with_error('Could not detect the launched browser subprocesses. The test harness will not be able to close the browser after testing is done, so aborting the test run here.')

      # Firefox on Windows quirk:
      # Make sure that each browser window is visible on the desktop. Otherwise
      # browser might decide that the tab is backgrounded, and not load a test,
      # or it might not tick rAF()s forward, causing tests to hang.
      if WINDOWS and parallel_harness and not EMTEST_HEADLESS:
        # Wrap window positions on a Full HD desktop area modulo primes.
        for proc in cls.browser_procs:
          move_browser_window(proc.pid, (300 + count * 47) % 1901, (10 + count * 37) % 997)

  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    cls.PORT = 8888 + (0 if worker_id is None else worker_id)
    cls.SERVER_URL = f'http://localhost:{cls.PORT}'
    cls.HARNESS_URL = f'{cls.SERVER_URL}/run_harness'

    if not has_browser() or EMTEST_BROWSER == 'node':
      errlog(f'[Skipping browser launch (EMTEST_BROWSER={EMTEST_BROWSER})]')
      return

    cls.harness_in_queue = queue.Queue()
    cls.harness_out_queue = queue.Queue()
    cls.harness_server = HttpServerThread(make_test_server(cls.harness_in_queue, cls.harness_out_queue, cls.PORT))
    cls.harness_server.start()

    errlog(f'[Browser harness server on thread {cls.harness_server.name}]')
    cls.browser_open(cls.HARNESS_URL)

  @classmethod
  def tearDownClass(cls):
    super().tearDownClass()
    if not has_browser() or EMTEST_BROWSER == 'node':
      return
    cls.harness_server.stop()
    cls.harness_server.join()
    cls.browser_terminate()

    if WINDOWS:
      # On Windows, shutil.rmtree() in tearDown() raises this exception if we do not wait a bit:
      # WindowsError: [Error 32] The process cannot access the file because it is being used by another process.
      time.sleep(0.1)

  def is_browser_test(self):
    return True

  def add_browser_reporting(self):
    contents = read_file(test_file('browser_reporting.js'))
    contents = contents.replace('{{{REPORTING_URL}}}', self.SERVER_URL)
    create_file('browser_reporting.js', contents)

  def check_browser_feature(self, env_var, feature, message):
    skip = browser_should_skip_feature(env_var, feature)
    if skip == 'error':
      self.fail(message)
    elif skip:
      self.skipTest(message)

  def assert_out_queue_empty(self, who):
    if not self.harness_out_queue.empty():
      responses = []
      while not self.harness_out_queue.empty():
        responses += [self.harness_out_queue.get()]
      raise Exception('excessive responses from %s: %s' % (who, '\n'.join(responses)))

  # @param extra_tries: how many more times to try this test, if it fails. browser tests have
  #                     many more causes of flakiness (in particular, they do not run
  #                     synchronously, so we have a timeout, which can be hit if the VM
  #                     we run on stalls temporarily).
  def run_browser(self, html_file, expected=None, message=None, timeout=None, extra_tries=None):
    if not has_browser():
      return
    assert '?' not in html_file, 'URL params not supported'
    if extra_tries is None:
      extra_tries = common.EMTEST_RETRY_FLAKY if self.flaky else 0
    url = html_file
    if self.capture_stdio:
      url += '?capture_stdio'
    if self.skip_exec:
      self.skipTest('skipping test execution: ' + self.skip_exec)
    if BrowserCore.unresponsive_tests >= BrowserCore.MAX_UNRESPONSIVE_TESTS:
      self.skipTest('too many unresponsive tests, skipping remaining tests')

    if EMTEST_RESTART_BROWSER_EVERY_N_TESTS and BrowserCore.num_tests_ran >= EMTEST_RESTART_BROWSER_EVERY_N_TESTS:
      logger.warning(f'[EMTEST_RESTART_BROWSER_EVERY_N_TESTS={EMTEST_RESTART_BROWSER_EVERY_N_TESTS} workaround: restarting browser]')
      self.browser_restart()
    BrowserCore.num_tests_ran += 1

    self.assert_out_queue_empty('previous test')
    if DEBUG:
      print('[browser launch:', html_file, ']')
    assert not (message and expected), 'run_browser expects `expected` or `message`, but not both'

    if expected is not None:
      try:
        self.harness_in_queue.put((
          'http://localhost:%s/%s' % (self.PORT, url),
          self.get_dir(),
        ))
        if timeout is None:
          timeout = self.BROWSER_TIMEOUT
        try:
          output = self.harness_out_queue.get(block=True, timeout=timeout)
        except queue.Empty:
          BrowserCore.unresponsive_tests += 1
          print(f'[unresponsive test: {self.id()} total unresponsive={str(BrowserCore.unresponsive_tests)}]')
          self.browser_restart()
          # Rather than fail the test here, let fail on the `assertContained` so
          # that the test can be retried via `extra_tries`
          output = '[no http server activity]'
        if output is None:
          # the browser harness reported an error already, and sent a None to tell
          # us to also fail the test
          self.fail('browser harness error')
        output = unquote(output)
        if output.startswith('/report_result?skipped:'):
          self.skipTest(unquote(output[len('/report_result?skipped:'):]).strip())
        else:
          # verify the result, and try again if we should do so
          try:
            self.assertContained(expected, output)
          except self.failureException as e:
            if extra_tries > 0:
              record_flaky_test(self.id(), common.EMTEST_RETRY_FLAKY - extra_tries, common.EMTEST_RETRY_FLAKY, e)
              if not self.capture_stdio:
                print('[enabling stdio/stderr reporting]')
                self.capture_stdio = True
              return self.run_browser(html_file, expected, message, timeout, extra_tries - 1)
            else:
              raise e
      finally:
        time.sleep(0.1) # see comment about Windows above
      self.assert_out_queue_empty('this test')
    else:
      webbrowser.open_new(os.path.abspath(html_file))
      print('A web browser window should have opened a page containing the results of a part of this test.')
      print('You need to manually look at the page to see that it works ok: ' + message)
      print('(sleeping for a bit to keep the directory alive for the web browser..)')
      time.sleep(5)
      print('(moving on..)')

  def compile_btest(self, filename, cflags, reporting=Reporting.FULL):
    # Inject support code for reporting results. This adds an include a header so testcases can
    # use REPORT_RESULT, and also adds a cpp file to be compiled alongside the testcase, which
    # contains the implementation of REPORT_RESULT (we can't just include that implementation in
    # the header as there may be multiple files being compiled here).
    if reporting != Reporting.NONE:
      # For basic reporting we inject JS helper funtions to report result back to server.
      self.add_browser_reporting()
      cflags += ['--pre-js', 'browser_reporting.js']
      if reporting == Reporting.FULL:
        # If C reporting (i.e. the REPORT_RESULT macro) is required we
        # also include report_result.c and force-include report_result.h
        self.run_process([EMCC, '-c', '-I' + TEST_ROOT,
                          test_file('report_result.c')] + self.get_cflags(compile_only=True) + (['-fPIC'] if '-fPIC' in cflags else []))
        cflags += ['report_result.o', '-include', test_file('report_result.h')]
    if EMTEST_BROWSER == 'node':
      cflags.append('-DEMTEST_NODE')
    filename = maybe_test_file(filename)
    self.run_process([compiler_for(filename), filename] + self.get_cflags() + cflags)
    # Remove the file since some tests have assertions for how many files are in
    # the output directory.
    utils.delete_file('browser_reporting.js')

  def btest_exit(self, filename, assert_returncode=0, *args, **kwargs):
    """Special case of `btest` that reports its result solely via exiting
    with a given result code.

    In this case we set EXIT_RUNTIME and we don't need to provide the
    REPORT_RESULT macro to the C code.
    """
    self.set_setting('EXIT_RUNTIME')
    assert 'reporting' not in kwargs
    assert 'expected' not in kwargs
    kwargs['reporting'] = Reporting.JS_ONLY
    kwargs['expected'] = 'exit:%d' % assert_returncode
    return self.btest(filename, *args, **kwargs)

  def btest(self, filename, expected=None,
            post_build=None,
            cflags=None,
            timeout=None,
            reporting=Reporting.FULL,
            run_in_worker=False,
            output_basename='test'):
    assert expected, 'a btest must have an expected output'
    if cflags is None:
      cflags = []
    cflags = cflags.copy()
    filename = find_browser_test_file(filename)
    if run_in_worker:
      outfile = output_basename + '.js'
    else:
      outfile = output_basename + '.html'
    cflags += ['-o', outfile]
    # print('cflags:', cflags)
    utils.delete_file(outfile)
    self.compile_btest(filename, cflags, reporting=reporting)
    self.assertExists(outfile)
    if post_build:
      post_build()
    if not isinstance(expected, list):
      expected = [expected]
    if EMTEST_BROWSER == 'node':
      nodejs = self.require_node()
      self.node_args += shared.node_pthread_flags(nodejs)
      output = self.run_js(f'{output_basename}.js')
      self.assertContained('RESULT: ' + expected[0], output)
    else:
      html_file = outfile
      if run_in_worker:
        create_file('run_worker.html', f'''\
          <script>
            new Worker('{output_basename}.js');
          </script>
          ''')
        html_file = 'run_worker.html'
      self.run_browser(html_file, expected=['/report_result?' + e for e in expected], timeout=timeout)
