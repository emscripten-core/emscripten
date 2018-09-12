#!/usr/bin/env python
# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# emrun: Implements machinery that allows running a .html page as if it was a standard executable file.
# Usage: emrun <options> filename.html <args to program>
# See emrun --help for more information

from __future__ import print_function
import os, platform, argparse, logging, re, pprint, atexit, subprocess, sys, time, struct, socket, cgi, tempfile, stat, shutil, json, uuid, shlex
from operator import itemgetter
from threading import Thread, RLock

if sys.version_info.major == 2:
  import SocketServer as socketserver
  from BaseHTTPServer import HTTPServer
  from SimpleHTTPServer import SimpleHTTPRequestHandler
  from urllib import unquote
  from urlparse import urlsplit
  from Queue import PriorityQueue
else:
  import socketserver
  from http.server import HTTPServer, SimpleHTTPRequestHandler
  from urllib.parse import unquote, urlsplit
  from queue import PriorityQueue

# Populated from cmdline params
emrun_options = None

# Represents the process object handle to the browser we opened to run the html page.
browser_process = None

# If we have routed browser output to file with --log_stdout and/or --log_stderr,  
# these track the handles.
browser_stdout_handle = sys.stdout
browser_stderr_handle = sys.stderr

# This flag tracks whether the html page has sent any stdout messages back to us.
# Used to detect whether we might have gotten detached from the browser process we 
# spawned, in which case we are not able to detect when user closes the browser with
# the close button.
have_received_messages = False

# At startup print a warning message once if user did not build with --emrun.
emrun_not_enabled_nag_printed = False

# Stores the exit() code of the html page when/if it quits.
page_exit_code = 0

# If this is set to a non-empty string, all processes by this name will be killed at exit.
# This is used to clean up after browsers that spawn subprocesses to handle the actual
# browser launch. For example opera has a launcher.exe that runs the actual opera browser.
# So killing browser_process would just kill launcher.exe and not the opera browser itself.
processname_killed_atexit = ""

# If user does not specify a --hostname parameter, this hostname is used to launch the server.
default_webserver_hostname = "localhost"

# If user does not specify a --port parameter, this port is used to launch the server.
default_webserver_port = 6931

# Location of Android Debug Bridge executable
ADB = ''

# Host OS detection to autolocate browsers and other OS-specific support needs.
WINDOWS = False
LINUX = False
MACOS = False
if os.name == 'nt':
  WINDOWS = True
elif platform.system() == 'Linux':
  LINUX = True
elif platform.mac_ver()[0] != '':
  MACOS = True

  import plistlib

# If you are running on an OS that is not any of these, must add explicit support for it.
if not WINDOWS and not LINUX and not MACOS:
  raise Exception("Unknown OS!")

import_win32api_modules_warned_once = False

def import_win32api_modules():
  try:
    global win32api, winreg, GetObject
    import win32api
    from win32com.client import GetObject
    try:
      import winreg
    except ImportError:
      import _winreg as winreg

  except Exception as e:
    global import_win32api_modules_warned_once
    if not import_win32api_modules_warned_once:
      print(str(e), file=sys.stderr)
      print("Importing Python win32 modules failed! This most likely occurs if you do not have PyWin32 installed! Get it from http://sourceforge.net/projects/pywin32/", file=sys.stderr)
      import_win32api_modules_warned_once = True
    raise

# Returns wallclock time in seconds.
def tick():
  # Would like to return time.clock() since it's apparently better for
  # precision, but it is broken on macOS 10.10 and Python 2.7.8.
  return time.time()

# Absolute wallclock time in seconds specifying when the previous HTTP stdout message from
# the page was received.
last_message_time = tick()

# Absolute wallclock time in seconds telling when we launched emrun.
page_start_time = tick()

# Stores the time of most recent http page serve.
page_last_served_time = None

# Returns given log message formatted to be outputted on a HTML page.
def format_html(msg):
  if not msg.endswith('\n'):
    msg += '\n'
  msg = cgi.escape(msg)
  msg = msg.replace('\r\n', '<br />').replace('\n', '<br />')
  return msg

# HTTP requests are handled from separate threads - synchronize them to avoid race conditions
http_mutex = RLock()

# Prints a log message to 'info' stdout channel. Always printed.
def logi(msg):
  global last_message_time
  with http_mutex:
    if emrun_options.log_html:
      sys.stdout.write(format_html(msg))
    else:
      print(msg, file=sys.stdout)
    sys.stdout.flush()
    last_message_time = tick()

# Prints a verbose log message to stdout channel. Only shown if run with --verbose.
def logv(msg):
  global emrun_options, last_message_time
  with http_mutex:
    if emrun_options.verbose:
      if emrun_options.log_html:
        sys.stdout.write(format_html(msg))
      else:
        print(msg, file=sys.stdout)
      sys.stdout.flush()
      last_message_time = tick()

# Prints an error message to stderr channel.
def loge(msg):
  global last_message_time
  with http_mutex:
    if emrun_options.log_html:
      sys.stderr.write(format_html(msg))
    else:
      print(msg, file=sys.stderr)
    sys.stderr.flush()
    last_message_time = tick()

def format_eol(msg):
  if WINDOWS:
    msg = msg.replace('\r\n', '\n').replace('\n', '\r\n')
  return msg

# Prints a message to the browser stdout output stream.
def browser_logi(msg):
  global browser_stdout_handle, last_message_time
  msg = format_eol(msg)
  print(msg, file=browser_stdout_handle)
  browser_stdout_handle.flush()
  last_message_time = tick()

# Prints a message to the browser stderr output stream.
def browser_loge(msg):
  global browser_stderr_handle, last_message_time
  msg = format_eol(msg)
  print(msg, file=browser_stderr_handle)
  browser_stderr_handle.flush()
  last_message_time = tick()
  
# Unquotes a unicode string. (translates ascii-encoded utf string back to utf)
def unquote_u(source):
  result = unquote(source)
  if '%u' in result:
    result = result.replace('%u','\\u').decode('unicode_escape')
  return result

temp_firefox_profile_dir = None

# Deletes the temporary created Firefox profile (if one exists)
def delete_emrun_safe_firefox_profile():
  global temp_firefox_profile_dir
  if temp_firefox_profile_dir != None:
    logv('remove_tree("' + temp_firefox_profile_dir + '")')
    remove_tree(temp_firefox_profile_dir)
    temp_firefox_profile_dir = None

# Firefox has a lot of default behavior that makes it unsuitable for automated/unattended run.
# This function creates a temporary profile directory that customized Firefox with various flags that enable
# automated runs.
def create_emrun_safe_firefox_profile():
  global temp_firefox_profile_dir
  temp_firefox_profile_dir = tempfile.mkdtemp(prefix='temp_emrun_firefox_profile_')
  f = open(os.path.join(temp_firefox_profile_dir, 'prefs.js'), 'w')
  f.write('''
// Lift the default max 20 workers limit to something higher to avoid hangs when page needs to spawn a lot of threads.
user_pref("dom.workers.maxPerDomain", 100);
// Always allow opening popups
user_pref("browser.popups.showPopupBlocker", false);
user_pref("dom.disable_open_during_load", false);
// Don't ask user if he wants to set Firefox as the default system browser
user_pref("browser.shell.checkDefaultBrowser", false);
user_pref("browser.shell.skipDefaultBrowserCheck", true);
// If automated runs crash, don't resume old tabs on the next run or show safe mode dialogs or anything else extra.
user_pref("browser.sessionstore.resume_from_crash", false);
user_pref("services.sync.prefs.sync.browser.sessionstore.restore_on_demand", false);
user_pref("browser.sessionstore.restore_on_demand", false);
user_pref("browser.sessionstore.max_resumed_crashes", -1);
user_pref("toolkip.startup.max_resumed_crashes", -1);
// Don't show the slow script dialog popup
user_pref("dom.max_script_run_time", 0);
user_pref("dom.max_chrome_script_run_time", 0);
// Don't open a home page at startup
user_pref("startup.homepage_override_url", "about:blank");
user_pref("startup.homepage_welcome_url", "about:blank");
user_pref("browser.startup.homepage", "about:blank");
// Don't try to perform browser (auto)update on the background
user_pref("app.update.auto", false);
user_pref("app.update.enabled", false);
user_pref("app.update.silent", false);
user_pref("app.update.mode", 0);
user_pref("app.update.service.enabled", false);
// Don't check compatibility with add-ons, or (auto)update them
user_pref("extensions.lastAppVersion", '');
user_pref("plugins.hide_infobar_for_outdated_plugin", true);
user_pref("plugins.update.url", '');
// Disable health reporter
user_pref("datareporting.healthreport.service.enabled", false);
// Disable crash reporter
user_pref("toolkit.crashreporter.enabled", false);
// Don't show WhatsNew on first run after every update
user_pref("browser.startup.homepage_override.mstone","ignore");
// Don't show 'know your rights' and a bunch of other nag windows at startup
user_pref("browser.rights.3.shown", true);
user_pref('devtools.devedition.promo.shown', true);
user_pref('extensions.shownSelectionUI', true);
user_pref('browser.newtabpage.introShown', true);
user_pref('browser.download.panel.shown', true);
user_pref('browser.customizemode.tip0.shown', true);
user_pref("browser.toolbarbuttons.introduced.pocket-button", true);
// Start in private browsing mode to not cache anything to disk (everything will be wiped anyway after this run)
user_pref("browser.privatebrowsing.autostart", true);
// Don't ask the user if he wants to close the browser when there are multiple tabs.
user_pref("browser.tabs.warnOnClose", false);
// Allow the launched script window to close itself, so that we don't need to kill the browser process in order to move on.
user_pref("dom.allow_scripts_to_close_windows", true);
// Set various update timers to a large value in the future in order to not
// trigger a large mass of update HTTP traffic on each Firefox run on the clean profile.
// 2147483647 seconds since Unix epoch is sometime in the year 2038, and this is the max integer accepted by Firefox.
user_pref("app.update.lastUpdateTime.addon-background-update-timer", 2147483647);
user_pref("app.update.lastUpdateTime.background-update-timer", 2147483647);
user_pref("app.update.lastUpdateTime.blocklist-background-update-timer", 2147483647);
user_pref("app.update.lastUpdateTime.browser-cleanup-thumbnails", 2147483647);
user_pref("app.update.lastUpdateTime.experiments-update-timer", 2147483647);
user_pref("app.update.lastUpdateTime.search-engine-update-timer", 2147483647);
user_pref("app.update.lastUpdateTime.xpi-signature-verification", 2147483647);
user_pref("extensions.getAddons.cache.lastUpdate", 2147483647);
user_pref("media.gmp-eme-adobe.lastUpdate", 2147483647);
user_pref("media.gmp-gmpopenh264.lastUpdate", 2147483647);
user_pref("datareporting.healthreport.nextDataSubmissionTime", "2147483647000");
// Detect directly when executing if asm.js does not validate by throwing an error.
user_pref("javascript.options.throw_on_asmjs_validation_failure", true);
// Sending Firefox Health Report Telemetry data is not desirable, since these are automated runs.
user_pref("datareporting.healthreport.uploadEnabled", false);
user_pref("datareporting.healthreport.service.enabled", false);
user_pref("datareporting.healthreport.service.firstRun", false);
user_pref("toolkit.telemetry.enabled", false);
user_pref("toolkit.telemetry.unified", false);
user_pref("datareporting.policy.dataSubmissionEnabled", false);
user_pref("datareporting.policy.dataSubmissionPolicyBypassNotification", true);
// Allow window.dump() to print directly to console
user_pref("browser.dom.window.dump.enabled", true);
// Disable background add-ons related update & information check pings
user_pref("extensions.update.enabled", false);
user_pref("extensions.getAddons.cache.enabled", false);
// Enable wasm
user_pref("javascript.options.wasm", true);
// Enable SharedArrayBuffer (this profile is for a testing environment, so Spectre/Meltdown don't apply)
user_pref("javascript.options.shared_memory", true);
''')
  f.close()
  logv('create_emrun_safe_firefox_profile: Created new Firefox profile "' + temp_firefox_profile_dir + '"')
  return temp_firefox_profile_dir

# Returns whether the browser page we spawned is still running.
# (note, not perfect atm, in case we are running in detached mode)
def is_browser_process_alive():
  global browser_process
  return browser_process and browser_process.poll() == None
  
# Kills browser_process and processname_killed_atexit. Also removes the temporary Firefox profile that
# was created, if one exists.
def kill_browser_process():
  global browser_process, processname_killed_atexit, emrun_options, ADB
  if browser_process:
    try:
      logv('Terminating browser process..')
      browser_process.kill()
      delete_emrun_safe_firefox_profile()
    except Exception as e:
      logv('Failed with error ' + str(e) + '!')
    browser_process = None
    processname_killed_atexit = ''
    return
  if len(processname_killed_atexit):
    if emrun_options.android:
      logv("Terminating Android app '" + processname_killed_atexit + "'.")
      subprocess.call([ADB, 'shell', 'am', 'force-stop', processname_killed_atexit])
    else:
      logv("Terminating all processes that have string '" + processname_killed_atexit + "' in their name.")
      if WINDOWS:
        process_image = processname_killed_atexit if '.exe' in processname_killed_atexit else (processname_killed_atexit + '.exe')
        process = subprocess.Popen(['taskkill', '/F', '/IM', process_image, '/T'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        process.communicate()
      else:
        try:
          subprocess.call(['pkill', processname_killed_atexit])
        except OSError as e:
          try:
            subprocess.call(['killall', processname_killed_atexit])
          except OSError as e:
            loge('Both commands pkill and killall failed to clean up the spawned browser process. Perhaps neither of these utilities is available on your system?')
      delete_emrun_safe_firefox_profile()
    # Clear the process name to represent that the browser is now dead.
    processname_killed_atexit = ''

# Our custom HTTP web server that will server the target page to run via .html.
# This is used so that we can load the page via a http:// URL instead of a file:// URL, since those wouldn't work too well unless user allowed XHR without CORS rules.
# Also, the target page will route its stdout and stderr back to here via HTTP requests.
class HTTPWebServer(socketserver.ThreadingMixIn, HTTPServer):
  # Log messaging arriving via HTTP can come in out of sequence. Implement a sequencing mechanism to enforce ordered transmission.
  expected_http_seq_num = -1
  # Stores messages that have arrived out of order, pending for a send as soon as the missing message arrives.
  # Kept in sorted order, first element is the oldest message received.
  http_message_queue = []

  def handle_incoming_message(self, seq_num, log, data):
    global have_received_messages
    with http_mutex:
      have_received_messages = True

      if self.expected_http_seq_num == -1:
        self.expected_http_seq_num = seq_num+1
        log(data)
      elif seq_num == -1: # Message arrived without a sequence number? Just log immediately
        log(data)
      elif seq_num == self.expected_http_seq_num:
        log(data)
        self.expected_http_seq_num += 1
        self.print_messages_due()
      elif seq_num < self.expected_http_seq_num:
        log(data)
      else:
        self.http_message_queue += [(seq_num, data, log)]
        self.http_message_queue.sort(key=itemgetter(0))
        if len(self.http_message_queue) > 16:
          self.print_next_message()
  
  # If it's been too long since we we got a message, prints out the oldest queued message, ignoring the proper order.
  # This ensures that if any messages are actually lost, that the message queue will be orderly flushed.
  def print_timed_out_messages(self):
    global last_message_time
    with http_mutex:
      now = tick()
      max_message_queue_time = 5
      if len(self.http_message_queue) and now - last_message_time > max_message_queue_time:
        self.print_next_message()
  
  # Skips to printing the next message in queue now, independent of whether there was missed messages in the sequence numbering.
  def print_next_message(self):
    with http_mutex:
      if len(self.http_message_queue):
        self.expected_http_seq_num = self.http_message_queue[0][0]
        self.print_messages_due()

  # Completely flushes all out-of-order messages in the queue.
  def print_all_messages(self):
    with http_mutex:
      while len(self.http_message_queue):
        self.print_next_message()

  # Prints any messages that are now due after we logged some other previous messages.
  def print_messages_due(self):
    with http_mutex:
      while len(self.http_message_queue):
        msg = self.http_message_queue[0]
        if msg[0] == self.expected_http_seq_num:
          msg[2](msg[1])
          self.expected_http_seq_num += 1
          self.http_message_queue.pop(0)
        else:
          return

  def serve_forever(self, timeout=0.5):
    global emrun_options, last_message_time, page_exit_code, have_received_messages, emrun_not_enabled_nag_printed
    self.is_running = True
    self.timeout = timeout
    while self.is_running:
      now = tick()
      # Did user close browser?
      if browser_process:
        browser_quit_code = browser_process.poll()
        if browser_quit_code != None:
          delete_emrun_safe_firefox_profile()
          if not emrun_options.serve_after_close:
            if not have_received_messages:
              emrun_options.serve_after_close = True
              logv('Warning: emrun got detached from the target browser process (the process quit with code ' + str(browser_quit_code) + '). Cannot detect when user closes the browser. Behaving as if --serve_after_close was passed in.')
              if not emrun_options.browser:
                logv('Try passing the --browser=/path/to/browser option to avoid this from occurring. See https://github.com/kripken/emscripten/issues/3234 for more discussion.')
            else:
              self.shutdown()
              logv('Browser process has quit. Shutting down web server.. Pass --serve_after_close to keep serving the page even after the browser closes.')

      # Serve HTTP
      self.handle_request()
      # Process message log queue
      self.print_timed_out_messages()

      # If web page was silent for too long without printing anything, kill process.
      time_since_message = now - last_message_time
      if emrun_options.silence_timeout != 0 and time_since_message > emrun_options.silence_timeout:
        self.shutdown()
        logi('No activity in ' + str(emrun_options.silence_timeout) + ' seconds. Quitting web server with return code ' + str(emrun_options.timeout_returncode) + '. (--silence_timeout option)')
        page_exit_code = emrun_options.timeout_returncode
        emrun_options.kill_on_exit = True

      # If the page has been running too long as a whole, kill process.
      time_since_start = now - page_start_time
      if emrun_options.timeout != 0 and time_since_start > emrun_options.timeout:
        self.shutdown()
        logi('Page has not finished in ' + str(emrun_options.timeout) + ' seconds. Quitting web server with return code ' + str(emrun_options.timeout_returncode) + '. (--timeout option)')
        emrun_options.kill_on_exit = True
        page_exit_code = emrun_options.timeout_returncode

      # If we detect that the page is not running with emrun enabled, print a warning message.
      if not emrun_not_enabled_nag_printed and page_last_served_time is not None:
        time_since_page_serve = now - page_last_served_time
        if not have_received_messages and time_since_page_serve > 10:
          logi('The html page you are running is not emrun-capable. Stdout, stderr and exit(returncode) capture will not work. Recompile the application with the --emrun linker flag to enable this, or pass --no_emrun_detect to emrun to hide this check.')
          emrun_not_enabled_nag_printed = True

    # Clean up at quit, print any leftover messages in queue.
    self.print_all_messages()

  def handle_error(self, request, client_address):
    err = sys.exc_info()[1].args[0]
    # Filter out the useless '[Errno 10054] An existing connection was forcibly closed by the remote host' errors that occur when we
    # forcibly kill the client.
    if err != 10054:
      socketserver.BaseServer.handle_error(self, request, client_address)

  def shutdown(self):
    self.is_running = False
    self.print_all_messages()
    return 1

# Processes HTTP request back to the browser.
class HTTPHandler(SimpleHTTPRequestHandler):
  def send_head(self):
    global page_last_served_time
    path = self.translate_path(self.path)
    f = None
 
    if os.path.isdir(path):
      if not self.path.endswith('/'):
        self.send_response(301)
        self.send_header("Location", self.path + "/")
        self.end_headers()
        return None
      for index in "index.html", "index.htm":
        index = os.path.join(path, index)
        if os.path.isfile(index):
          path = index
          break
      else:
        # Manually implement directory listing support.
        return self.list_directory(path)

    try:
      f = open(path, 'rb')
    except IOError:
      self.send_error(404, "File not found: " + path)
      return None
    self.send_response(200)
    guess_file_type = path
    # All files of type x.gz are served as gzip-compressed, which means the browser will transparently decode the file before passing the uncompressed bytes to the JS page.
    # Note: In a slightly silly manner, detect files ending with "gz" and not ".gz", since both Unity and UE4 generate multiple files with .jsgz, .datagz, .memgz, .symbolsgz suffixes and so on,
    #       so everything goes.
    # Note 2: If the JS application would like to receive the actual bits of a gzipped file, instead of having the browser decompress it immediately, then it can't use the suffix .gz when using emrun.
    #         To work around, one can use the suffix .gzip instead.
    if 'Accept-Encoding' in self.headers and 'gzip' in self.headers['Accept-Encoding'] and path.lower().endswith('gz'):
      self.send_header('Content-Encoding', 'gzip')
      logv('Serving ' + path + ' as gzip-compressed.')
      guess_file_type = guess_file_type[:-2]
      if guess_file_type.endswith('.'): guess_file_type = guess_file_type[:-1]

    ctype = self.guess_type(guess_file_type)
    if guess_file_type.lower().endswith('.wasm'):
      ctype = 'application/wasm'
    self.send_header("Content-type", ctype)
    fs = os.fstat(f.fileno())
    self.send_header("Content-Length", str(fs[6]))
    self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
    self.send_header('Cache-Control','no-cache, must-revalidate')
    self.send_header('Connection','close')
    self.send_header('Expires','-1')
    self.send_header('Access-Control-Allow-Origin', '*')
    self.end_headers()
    page_last_served_time = tick()
    return f

  def log_request(self, code):
    # Filter out 200 OK messages to remove noise.
    if code is not 200:
      SimpleHTTPRequestHandler.log_request(self, code)

  def log_message(self, format, *args):
    msg = "%s - - [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format%args)
    if not 'favicon.ico' in msg: # Filter out 404 messages on favicon.ico not being found to remove noise.
      sys.stderr.write(msg)

  def do_POST(self):
    global page_exit_code, emrun_options, have_received_messages

    (_, _, path, query, _) = urlsplit(self.path)
    logv('POST: "' + self.path + '" (path: "' + path + '", query: "' + query + '")')
    if query.startswith('file='): # Binary file dump/upload handling. Requests to "stdio.html?file=filename" will write binary data to the given file.
      data = self.rfile.read(int(self.headers['Content-Length']))
      filename = query[len('file='):]
      dump_out_directory = 'dump_out'
      try:
        os.mkdir(dump_out_directory)
      except:
        pass
      filename = os.path.join(dump_out_directory, os.path.normpath(filename))
      open(filename, 'wb').write(data)
      print('Wrote ' + str(len(data)) + ' bytes to file "' + filename + '".')
      have_received_messages = True
    elif path == '/system_info':
      system_info = json.loads(get_system_info(format_json=True))
      try:
        browser_info = json.loads(get_browser_info(browser_exe, format_json=True))
      except:
        browser_info = ''
      data = { 'system': system_info, 'browser': browser_info }
      self.send_response(200)
      self.send_header("Content-type", "application/json")
      self.send_header('Cache-Control','no-cache, must-revalidate')
      self.send_header('Connection','close')
      self.send_header('Expires','-1')
      self.end_headers()
      self.wfile.write(json.dumps(data))
      return
    else:
      data = self.rfile.read(int(self.headers['Content-Length']))
      if str is not bytes and isinstance(data, bytes):
        data = data.decode('utf-8')
      data = data.replace("+", " ")
      data = unquote_u(data)

      # The user page sent a message with POST. Parse the message and log it to stdout/stderr.
      is_stdout = False
      is_stderr = False
      seq_num = -1
      # The html shell is expected to send messages of form ^out^(number)^(message) or ^err^(number)^(message).
      if data.startswith('^err^'):
        is_stderr = True
      elif data.startswith('^out^'):
        is_stdout = True
      if is_stderr or is_stdout:
        try:
          i = data.index('^', 5)
          seq_num = int(data[5:i])
          data = data[i+1:]
        except:
          pass

      is_exit = data.startswith('^exit^')

      if data == '^pageload^': # Browser is just notifying that it has successfully launched the page.
        have_received_messages = True
      elif not is_exit:
        log = browser_loge if is_stderr else browser_logi
        self.server.handle_incoming_message(seq_num, log, data)
      elif not emrun_options.serve_after_exit:
        page_exit_code = int(data[6:])
        logv('Web page has quit with a call to exit() with return code ' + str(page_exit_code) + '. Shutting down web server. Pass --serve_after_exit to keep serving even after the page terminates with exit().')
        self.server.shutdown()

    self.send_response(200)
    self.send_header("Content-type", "text/plain")
    self.send_header('Cache-Control','no-cache, must-revalidate')
    self.send_header('Connection','close')
    self.send_header('Expires','-1')
    self.end_headers()
    self.wfile.write(b'OK')

# Returns stdout by running command with universal_newlines=True
def check_output(cmd, universal_newlines=True, *args, **kwargs):
  if hasattr(subprocess, "run"):
    return subprocess.run(cmd, universal_newlines=universal_newlines, stdout=subprocess.PIPE, check=True, *args, **kwargs).stdout
  else:
    # check_output is considered as an old API so prefer subprocess.run if possible
    return subprocess.check_output(cmd, universal_newlines=universal_newlines, *args, **kwargs)

# From http://stackoverflow.com/questions/4842448/getting-processor-information-in-python
# Returns a string with something like "AMD64, Intel(R) Core(TM) i5-2557M CPU @ 1.70GHz, Intel64 Family 6 Model 42 Stepping 7, GenuineIntel"
def get_cpu_info():
  physical_cores = 1
  logical_cores = 1
  frequency = 0
  try:
    if WINDOWS:
      import_win32api_modules()
      root_winmgmts = GetObject("winmgmts:root\cimv2")
      cpus = root_winmgmts.ExecQuery("Select * from Win32_Processor")
      cpu_name = cpus[0].Name + ', ' + platform.processor()
      physical_cores = int(check_output(['wmic', 'cpu', 'get', 'NumberOfCores']).split('\n')[1].strip())
      logical_cores = int(check_output(['wmic', 'cpu', 'get', 'NumberOfLogicalProcessors']).split('\n')[1].strip())
      frequency = int(check_output(['wmic', 'cpu', 'get', 'MaxClockSpeed']).split('\n')[1].strip())
    elif MACOS:
      cpu_name = check_output(['sysctl', '-n', 'machdep.cpu.brand_string']).strip()
      physical_cores = int(check_output(['sysctl', '-n', 'machdep.cpu.core_count']).strip())
      logical_cores = int(check_output(['sysctl', '-n', 'machdep.cpu.thread_count']).strip())
      frequency = int(check_output(['sysctl', '-n', 'hw.cpufrequency']).strip()) // 1000000
    elif LINUX:
      all_info = check_output(['cat', '/proc/cpuinfo']).strip()
      for line in all_info.split("\n"):
        if "model name" in line:
          cpu_name = re.sub( ".*model name.*:", "", line, 1).strip()
      lscpu = check_output(['lscpu'])
      frequency = int(float(re.search('CPU MHz: (.*)', lscpu).group(1).strip()) + 0.5)
      sockets = int(re.search('Socket\(s\): (.*)', lscpu).group(1).strip())
      physical_cores = sockets * int(re.search('Core\(s\) per socket: (.*)', lscpu).group(1).strip())
      logical_cores = physical_cores * int(re.search('Thread\(s\) per core: (.*)', lscpu).group(1).strip())
  except Exception as e:
    import traceback
    print(traceback.format_exc())
    return { 'model': 'Unknown ("' + str(e) + '")',
      'physicalCores': 1,
      'logicalCores': 1,
      'frequency': 0
    }

  return { 'model': platform.machine() + ', ' + cpu_name,
    'physicalCores': physical_cores,
    'logicalCores': logical_cores,
    'frequency': frequency
    }

def get_android_cpu_infoline():
  lines = check_output([ADB, 'shell', 'cat', '/proc/cpuinfo']).split('\n')
  processor = ''
  hardware = ''
  for line in lines:
    if line.startswith('Processor'):
      processor = line[line.find(':')+1:].strip()
    elif line.startswith('Hardware'):
      hardware = line[line.find(':')+1:].strip()

  freq = int(check_output([ADB, 'shell', 'cat', '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq']).strip())//1000
  return 'CPU: ' + processor + ', ' + hardware + ' @ ' + str(freq) + ' MHz'

def win_get_gpu_info():
  gpus = []

  def find_gpu_model(model):
    for gpu in gpus:
      if gpu['model'] == model: return gpu
    return None

  try:
    import_win32api_modules()
  except:
    return []

  for i in range(0, 16):
    try:
      hHardwareReg = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "HARDWARE")
      hDeviceMapReg = winreg.OpenKey(hHardwareReg, "DEVICEMAP")
      hVideoReg = winreg.OpenKey(hDeviceMapReg, "VIDEO")
      VideoCardString = winreg.QueryValueEx(hVideoReg,"\Device\Video"+str(i))[0]
      #Get Rid of Registry/Machine from the string
      VideoCardStringSplit = VideoCardString.split("\\")
      ClearnVideoCardString = "\\".join(VideoCardStringSplit[3:])
      #Go up one level for detailed
      VideoCardStringRoot = "\\".join(VideoCardStringSplit[3:len(VideoCardStringSplit)-1])

      #Get the graphics card information
      hVideoCardReg = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, ClearnVideoCardString)
      try:
        VideoCardDescription  = winreg.QueryValueEx(hVideoCardReg, "Device Description")[0]
      except WindowsError:
        VideoCardDescription  = winreg.QueryValueEx(hVideoCardReg, "DriverDesc")[0]
      
      try:
        driverVersion = winreg.QueryValueEx(hVideoCardReg, "DriverVersion")[0]
        VideoCardDescription += ', driver version ' + driverVersion
      except:
        pass

      try:
        driverDate = winreg.QueryValueEx(hVideoCardReg, "DriverDate")[0]
        VideoCardDescription += ' (' + driverDate + ')'
      except:
        pass

      VideoCardMemorySize = winreg.QueryValueEx(hVideoCardReg, "HardwareInformation.MemorySize")[0]
      try:
        vram = struct.unpack('l',bytes(VideoCardMemorySize))[0]
      except struct.error:
        vram = int(VideoCardMemorySize)
      if not find_gpu_model(VideoCardDescription):
        gpus += [{ 'model': VideoCardDescription, 'ram': vram }]
    except WindowsError:
      pass
  return gpus

def linux_get_gpu_info():
  glinfo = ''
  try:
    glxinfo = check_output('glxinfo')
    for line in glxinfo.split("\n"):
      if "OpenGL vendor string:" in line: gl_vendor = line[len("OpenGL vendor string:"):].strip()
      if "OpenGL version string:" in line: gl_version = line[len("OpenGL version string:"):].strip()
      if "OpenGL renderer string:" in line: gl_renderer = line[len("OpenGL renderer string:"):].strip()
    glinfo = gl_vendor + ' ' + gl_renderer + ', GL version ' + gl_version
  except Exception as e:
    logv(e)

  adapterinfo = ''
  try:
    vgainfo = check_output(['lshw', '-C', 'display'], stderr=subprocess.PIPE)
    vendor = re.search("vendor: (.*)", vgainfo).group(1).strip()
    product = re.search("product: (.*)", vgainfo).group(1).strip()
    description = re.search("description: (.*)", vgainfo).group(1).strip()
    clock = re.search("clock: (.*)", vgainfo).group(1).strip()
    adapterinfo = vendor + ' ' + product + ', ' + description + ' (' + clock + ')'
  except Exception as e:
    logv(e)

  ram = 0
  try:
    vgainfo = check_output('lspci -v -s $(lspci | grep VGA | cut -d " " -f 1)', shell=True, stderr=subprocess.PIPE)
    ram = int(re.search("\[size=([0-9]*)M\]", vgainfo).group(1)) * 1024 * 1024
  except Exception as e:
    logv(e)

  model = (adapterinfo + ' ' + glinfo).strip()
  if not model: model = 'Unknown'
  return [{'model': model, 'ram': ram}]

def macos_get_gpu_info():
  gpus = []
  try:
    info = check_output(['system_profiler', 'SPDisplaysDataType'])
    info = info.split("Chipset Model:")[1:]
    for gpu in info:
      model_name = gpu.split('\n')[0].strip()
      bus = re.search("Bus: (.*)", gpu).group(1).strip()
      memory = int(re.search("VRAM (.*?): (.*) MB", gpu).group(2).strip())
      gpus += [{'model': model_name + ' (' + bus + ')', 'ram': memory * 1024 * 1024}]
    return gpus
  except:
    pass

def get_gpu_info():
  if WINDOWS: return win_get_gpu_info()
  elif LINUX: return linux_get_gpu_info()
  elif MACOS: return macos_get_gpu_info()
  else: return []

def get_executable_version(filename):
  try:
    if WINDOWS:
      info = win32api.GetFileVersionInfo(filename, "\\")
      ms = info['FileVersionMS']
      ls = info['FileVersionLS']
      version = win32api.HIWORD(ms), win32api.LOWORD(ms), win32api.HIWORD(ls), win32api.LOWORD(ls)
      return '.'.join(map(str, version))
    elif MACOS:
      plistfile = filename[0:filename.find('MacOS')] + 'Info.plist'
      info = plistlib.readPlist(plistfile)
      # Data in Info.plists is a bit odd, this check combo gives best information on each browser.
      if 'firefox' in filename.lower():
        return info['CFBundleShortVersionString']
      if 'opera' in filename.lower():
        return info['CFBundleVersion']
      else:
        return info['CFBundleShortVersionString']
    elif LINUX:
      if 'firefox' in filename.lower():
        version = check_output([filename, '-v'])
        version = version.replace('Mozilla Firefox ', '')
        return version.strip()
      else:
        return ""
  except Exception as e:
    logv(e)
    return ""

def get_browser_build_date(filename):
  try:
    if MACOS:
      plistfile = filename[0:filename.find('MacOS')] + 'Info.plist'
      info = plistlib.readPlist(plistfile)
      # Data in Info.plists is a bit odd, this check combo gives best information on each browser.
      if 'firefox' in filename.lower():
        return '20' + '-'.join(map((lambda x: x.zfill(2)), info['CFBundleVersion'][2:].split('.')))
  except Exception as e:
    logv(e)

  # No exact information about the build date, so take the last modified date of the file.
  # This is not right, but assuming that one installed the browser shortly after the update was
  # available, it's shooting close.
  try:
    return time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(os.path.getmtime(filename)))
  except Exception as e:
    logv(e)
    return '(unknown)'

def get_browser_info(filename, format_json):
  if format_json:
    return json.dumps({
      'name': browser_display_name(filename),
      'version': get_executable_version(filename),
      'buildDate': get_browser_build_date(filename)
    }, indent=2)
  else:
    return 'Browser: ' + browser_display_name(filename) + ' ' + get_executable_version(filename) + ', build ' + get_browser_build_date(filename)

# http://stackoverflow.com/questions/580924/python-windows-file-version-attribute
def win_get_file_properties(fname):
  propNames = ('Comments', 'InternalName', 'ProductName',
    'CompanyName', 'LegalCopyright', 'ProductVersion',
    'FileDescription', 'LegalTrademarks', 'PrivateBuild',
    'FileVersion', 'OriginalFilename', 'SpecialBuild')

  props = {'FixedFileInfo': None, 'StringFileInfo': None, 'FileVersion': None}

  try:
    import win32api
    # backslash as parm returns dictionary of numeric info corresponding to VS_FIXEDFILEINFO struc
    fixedInfo = win32api.GetFileVersionInfo(fname, '\\')
    props['FixedFileInfo'] = fixedInfo
    props['FileVersion'] = "%d.%d.%d.%d" % (fixedInfo['FileVersionMS'] / 65536,
        fixedInfo['FileVersionMS'] % 65536, fixedInfo['FileVersionLS'] / 65536,
        fixedInfo['FileVersionLS'] % 65536)

    # \VarFileInfo\Translation returns list of available (language, codepage)
    # pairs that can be used to retreive string info. We are using only the first pair.
    lang, codepage = win32api.GetFileVersionInfo(fname, '\\VarFileInfo\\Translation')[0]

    # any other must be of the form \StringfileInfo\%04X%04X\parm_name, middle
    # two are language/codepage pair returned from above

    strInfo = {}
    for propName in propNames:
      strInfoPath = u'\\StringFileInfo\\%04X%04X\\%s' % (lang, codepage, propName)
      ## print str_info
      strInfo[propName] = win32api.GetFileVersionInfo(fname, strInfoPath)

    props['StringFileInfo'] = strInfo
  except:
    pass

  return props

def get_computer_model():
  try:
    if MACOS:
      try:
        with open(os.path.join(os.getenv("HOME"), '.emrun.hwmodel.cached'), 'r') as f:
          model = f.read()
          return model
      except:
        pass

      try:
        # http://apple.stackexchange.com/questions/98080/can-a-macs-model-year-be-determined-via-terminal-command
        serial = check_output(['system_profiler', 'SPHardwareDataType'])
        serial = re.search("Serial Number (.*): (.*)", serial)
        serial = serial.group(2).strip()[-4:]
        cmd = ['curl', '-s', 'http://support-sp.apple.com/sp/product?cc=' + serial]
        logv(str(cmd))
        model = check_output(cmd)
        model = re.search('<configCode>(.*)</configCode>', model)
        model = model.group(1).strip()
        open(os.path.join(os.getenv("HOME"), '.emrun.hwmodel.cached'), 'w').write(model) # Cache the hardware model to disk
        return model
      except:
        hwmodel = check_output(['sysctl', 'hw.model'])
        hwmodel = re.search('hw.model: (.*)', hwmodel).group(1).strip()
        return hwmodel
    elif WINDOWS:
      manufacturer = check_output(['wmic', 'baseboard', 'get', 'manufacturer']).split('\n')[1].strip()
      version = check_output(['wmic', 'baseboard', 'get', 'version']).split('\n')[1].strip()
      product = check_output(['wmic', 'baseboard', 'get', 'product']).split('\n')[1].strip()
      if 'Apple' in manufacturer: return manufacturer + ' ' + version + ', ' + product
      else: return manufacturer + ' ' + product + ', ' + version
    elif LINUX:
      board_vendor = check_output(['cat', '/sys/devices/virtual/dmi/id/board_vendor']).strip()
      board_name = check_output(['cat', '/sys/devices/virtual/dmi/id/board_name']).strip()
      board_version = check_output(['cat', '/sys/devices/virtual/dmi/id/board_version']).strip()

      bios_vendor = check_output(['cat', '/sys/devices/virtual/dmi/id/bios_vendor']).strip()
      bios_version = check_output(['cat', '/sys/devices/virtual/dmi/id/bios_version']).strip()
      bios_date = check_output(['cat', '/sys/devices/virtual/dmi/id/bios_date']).strip()
      return board_vendor + ' ' + board_name + ' ' + board_version + ', ' + bios_vendor + ' ' + bios_version + ' (' + bios_date + ')'
  except Exception as e:
    logv(str(e))
  return 'Generic'

def get_os_version():
  bitness = ' (64bit)' if platform.machine() in ['AMD64', 'x86_64'] else ' (32bit)'
  try:
    if WINDOWS:
      import_win32api_modules()
      versionHandle = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")
      productName = winreg.QueryValueEx(versionHandle, "ProductName")

      version = ''
      try:
        version = ' ' + check_output(['wmic', 'os', 'get', 'version']).split('\n')[1].strip()
      except:
        pass
      return productName[0] + version + bitness
    elif MACOS:
      return 'macOS ' + platform.mac_ver()[0] + bitness
    elif LINUX:
      kernel_version = check_output(['uname', '-r']).strip()
      return ' '.join(platform.linux_distribution()) + ', linux kernel ' + kernel_version + ' ' + platform.architecture()[0] + bitness
  except:
    return 'Unknown OS'

def get_system_memory():
  global emrun_options

  try:
    if LINUX or emrun_options.android:
      if emrun_options.android:
        lines = check_output([ADB, 'shell', 'cat', '/proc/meminfo']).split('\n')
      else:
        mem = open('/proc/meminfo', 'r')
        lines = mem.readlines()
        mem.close()
      for i in lines:
        sline = i.split()
        if str(sline[0]) == 'MemTotal:':
          return int(sline[1]) * 1024
    elif WINDOWS:
      return win32api.GlobalMemoryStatusEx()['TotalPhys']
    elif MACOS:
      return int(check_output(['sysctl', '-n', 'hw.memsize']).strip())
  except:
    return -1

# Finds the given executable 'program' in PATH. Operates like the Unix tool 'which'.
def which(program):
  def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

  fpath, fname = os.path.split(program)
  if fpath:
    if is_exe(program):
      return program
  else:
    for path in os.environ["PATH"].split(os.pathsep):
      path = path.strip('"')
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file

      if WINDOWS and not '.' in fname:
        if is_exe(exe_file + '.exe'):
          return exe_file + '.exe'
        if is_exe(exe_file + '.cmd'):
          return exe_file + '.cmd'
        if is_exe(exe_file + '.bat'):
          return exe_file + '.bat'

  return None

def win_get_default_browser():
  import_win32api_modules()
  # Look in the registry for the default system browser on Windows without relying on
  # 'start %1' since that method has an issue, see comment below.
  try:
    with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\Classes\http\shell\open\command") as key:
      cmd = winreg.QueryValue(key, None)
      if cmd:
        parts = shlex.split(cmd)
        if len(parts):
          return [parts[0]]
  except WindowsError:
    logv("Unable to find default browser key in Windows registry. Trying fallback.")

  # Fall back to 'start %1', which we have to treat as if user passed --serve_forever, since
  # for some reason, we are not able to detect when the browser closes when this is passed.
  return ['cmd', '/C', 'start']

def find_browser(name):
  if WINDOWS and name == 'start':
    return win_get_default_browser()
  if MACOS and name == 'open':
    return [name]

  if os.path.isfile(os.path.abspath(name)):
    return [name]
  if os.path.isfile(os.path.abspath(name)+'.exe'):
    return [os.path.abspath(name)+'.exe']
  if os.path.isfile(os.path.abspath(name)+'.cmd'):
    return [os.path.abspath(name)+'.cmd']
  if os.path.isfile(os.path.abspath(name)+'.bat'):
    return [os.path.abspath(name)+'.bat']

  path_lookup = which(name)
  if path_lookup != None:
    return [path_lookup]

  browser_locations = []
  if MACOS:
    # Note: by default Firefox beta installs as 'Firefox.app', you must manually rename it to
    # FirefoxBeta.app after installation.
    browser_locations = [ ('firefox', '/Applications/Firefox.app/Contents/MacOS/firefox'),
                          ('firefox_beta', '/Applications/FirefoxBeta.app/Contents/MacOS/firefox'),
                          ('firefox_aurora', '/Applications/FirefoxAurora.app/Contents/MacOS/firefox'),
                          ('firefox_nightly', '/Applications/FirefoxNightly.app/Contents/MacOS/firefox'),
                          ('safari', '/Applications/Safari.app/Contents/MacOS/Safari'),
                          ('opera', '/Applications/Opera.app/Contents/MacOS/Opera'),
                          ('chrome', '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'),
                          ('chrome_canary', '/Applications/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary') ]
  elif WINDOWS:
    pf_locations = ['ProgramFiles(x86)', 'ProgramFiles', 'ProgramW6432']

    for pf_env in pf_locations:
      if not pf_env in os.environ:
        continue
      program_files = os.environ[pf_env] if WINDOWS else ''

      browser_locations += [ ('chrome', os.path.join(program_files, 'Google/Chrome/Application/chrome.exe')),
                             ('chrome_canary', os.path.expanduser("~/AppData/Local/Google/Chrome SxS/Application/chrome.exe")),
                             ('firefox_nightly', os.path.join(program_files, 'Nightly/firefox.exe')),
                             ('firefox_aurora', os.path.join(program_files, 'Aurora/firefox.exe')),
                             ('firefox_beta', os.path.join(program_files, 'Beta/firefox.exe')),
                             ('firefox_beta', os.path.join(program_files, 'FirefoxBeta/firefox.exe')),
                             ('firefox_beta', os.path.join(program_files, 'Firefox Beta/firefox.exe')),
                             ('firefox', os.path.join(program_files, 'Mozilla Firefox/firefox.exe')),
                             ('iexplore', os.path.join(program_files, 'Internet Explorer/iexplore.exe')),
                             ('opera', os.path.join(program_files, 'Opera/launcher.exe')) ]

  elif LINUX:
    browser_locations = [ ('firefox', os.path.expanduser('~/firefox/firefox')),
                          ('firefox_beta', os.path.expanduser('~/firefox_beta/firefox')),
                          ('firefox_aurora', os.path.expanduser('~/firefox_aurora/firefox')),
                          ('firefox_nightly', os.path.expanduser('~/firefox_nightly/firefox')),
                          ('chrome', which('google-chrome-stable')),
                          ('chrome', which('google-chrome'))]

  for (alias, browser_exe) in browser_locations:
    if name == alias:
      if browser_exe is not None and os.path.isfile(browser_exe):
        return [browser_exe]
  
  return None # Could not find the browser

def get_android_model():
  global ADB
  manufacturer = check_output([ADB, 'shell', 'getprop', 'ro.product.manufacturer']).strip()
  brand = check_output([ADB, 'shell', 'getprop', 'ro.product.brand']).strip()
  model = check_output([ADB, 'shell', 'getprop', 'ro.product.model']).strip()
  board = check_output([ADB, 'shell', 'getprop', 'ro.product.board']).strip()
  device = check_output([ADB, 'shell', 'getprop', 'ro.product.device']).strip()
  name = check_output([ADB, 'shell', 'getprop', 'ro.product.name']).strip()
  return manufacturer + ' ' + brand + ' ' + model + ' ' + board + ' ' + device + ' ' + name

def get_android_os_version():
  global ADB
  ver = check_output([ADB, 'shell', 'getprop', 'ro.build.version.release']).strip()
  apiLevel = check_output([ADB, 'shell', 'getprop', 'ro.build.version.sdk']).strip()
  if not apiLevel:
    apiLevel = check_output([ADB, 'shell', 'getprop', 'ro.build.version.sdk_int']).strip()

  os = ''
  if ver:
    os += 'Android ' + ver + ' '
  if apiLevel:
    os += 'SDK API Level ' + apiLevel + ' '
  os += check_output([ADB, 'shell', 'getprop', 'ro.build.description']).strip()
  return os

def list_android_browsers():
  global ADB
  apps = check_output([ADB, 'shell', 'pm', 'list', 'packages', '-f']).replace('\r\n', '\n')
  browsers = []
  for line in apps.split('\n'):
    line = line.strip()
    if line.endswith('=org.mozilla.firefox'):
      browsers += ['firefox']
    if line.endswith('=org.mozilla.firefox_beta'):
      browsers += ['firefox_beta']
    if line.endswith('=org.mozilla.fennec_aurora'):
      browsers += ['firefox_aurora']
    if line.endswith('=org.mozilla.fennec'):
      browsers += ['firefox_nightly']
    if line.endswith('=com.android.chrome'):
      browsers += ['chrome']
    if line.endswith('=com.chrome.beta'):
      browsers += ['chrome_beta']
    if line.endswith('=com.chrome.dev'):
      browsers += ['chrome_dev']
    if line.endswith('=com.chrome.canary'):
      browsers += ['chrome_canary']
    if line.endswith('=com.opera.browser'):
      browsers += ['opera']
    if line.endswith('=com.opera.mini.android'):
      browsers += ['opera_mini']
    if line.endswith('=mobi.mgeek.TunnyBrowser'):
      browsers += ['dolphin']

  browsers.sort()
  logi('emrun has automatically found the following browsers on the connected Android device:')
  for browser in browsers:
    logi('  - ' + browser)

def list_pc_browsers():
  browsers = ['firefox', 'firefox_beta', 'firefox_aurora', 'firefox_nightly', 'chrome', 'chrome_canary', 'iexplore', 'safari', 'opera']  
  logi('emrun has automatically found the following browsers in the default install locations on the system:')
  logi('')
  for browser in browsers:
    browser_exe = find_browser(browser)
    if type(browser_exe) == list:
      browser_exe = browser_exe[0]
    if browser_exe:
      logi('  - ' + browser + ': ' + browser_display_name(browser_exe) + ' ' + get_executable_version(browser_exe))
  logi('')
  logi('You can pass the --browser <id> option to launch with the given browser above.')
  logi('Even if your browser was not detected, you can use --browser /path/to/browser/executable to launch with that browser.')

def browser_display_name(browser):
  b = browser.lower()
  if 'iexplore' in b:
    return 'Microsoft Internet Explorer'
  if 'chrome' in b:
    return 'Google Chrome'
  if 'firefox' in b:
    # Try to identify firefox flavor explicitly, to help show issues where emrun would launch the wrong browser.
    try:
      product_name = win_get_file_properties(browser)['StringFileInfo']['ProductName'] if WINDOWS else 'firefox'
      if product_name.lower() != 'firefox':
        return 'Mozilla Firefox ' + product_name
    except Exception as e:
      pass
    return 'Mozilla Firefox'
  if 'opera' in b:
    return 'Opera'
  if 'safari' in b:
    return 'Apple Safari'
  return browser

def subprocess_env():
  e = os.environ.copy()
  # https://bugzilla.mozilla.org/show_bug.cgi?id=745154
  e['MOZ_DISABLE_AUTO_SAFE_MODE'] = '1'
  e['MOZ_DISABLE_SAFE_MODE_KEY'] = '1' # https://bugzilla.mozilla.org/show_bug.cgi?id=653410#c9
  e['JIT_OPTION_asmJSAtomicsEnable'] = 'true' # https://bugzilla.mozilla.org/show_bug.cgi?id=1299359#c0
  return e

# Removes a directory tree even if it was readonly, and doesn't throw exception on failure.
def remove_tree(d):
  os.chmod(d, stat.S_IWRITE)
  try:
    def remove_readonly_and_try_again(func, path, exc_info):
      if not (os.stat(path).st_mode & stat.S_IWRITE):
        os.chmod(path, stat.S_IWRITE)
        func(path)
      else:
        raise
    shutil.rmtree(d, onerror=remove_readonly_and_try_again)
  except Exception as e:
    pass

def get_system_info(format_json):
  if emrun_options.android:
    if format_json:
      return json.dumps({
          'model': get_android_model(),
          'os': get_android_os_version(),
          'ram': get_system_memory(),
          'cpu': get_android_cpu_infoline()
          }, indent=2)
    else:
      info = 'Model: ' + get_android_model() + '\n'
      info += 'OS: ' + get_android_os_version() + ' with ' + str(get_system_memory()//1024//1024) + ' MB of System RAM\n'
      info += 'CPU: ' + get_android_cpu_infoline() + '\n'
      return info.strip()
  else:
    try:
      unique_system_id = open(os.path.expanduser('~/.emrun.generated.guid'), 'r').read().strip()
    except:
      import uuid
      unique_system_id = str(uuid.uuid4())
      try:
        open(os.path.expanduser('~/.emrun.generated.guid'), 'w').write(unique_system_id)
      except Exception as e:
        logv(e)

    if format_json:
      return json.dumps({
          'name': socket.gethostname(),
          'model': get_computer_model(),
          'os': get_os_version(),
          'ram': get_system_memory(),
          'cpu': get_cpu_info(),
          'gpu': get_gpu_info(),
          'uuid': unique_system_id
          }, indent=2)
    else:
      cpu = get_cpu_info()
      gpus = get_gpu_info()
      info = 'Computer name: ' + socket.gethostname() + '\n' # http://stackoverflow.com/questions/799767/getting-name-of-windows-computer-running-python-script
      info += 'Model: ' + get_computer_model() + '\n'
      info += 'OS: ' + get_os_version() + ' with ' + str(get_system_memory()//1024//1024) + ' MB of System RAM\n'
      info += 'CPU: ' + cpu['model'] + ', ' + str(cpu['frequency']) + ' MHz, ' + str(cpu['physicalCores']) + ' physical cores, ' + str(cpu['logicalCores']) + ' logical cores\n';
      if len(gpus) == 1:
        info += "GPU: " + gpus[0]['model'] + " with " + str(gpus[0]['ram']//1024//1024) + " MB of VRAM\n"
      elif len(gpus) > 1:
        for i in range(0, len(gpus)):
          info += "GPU"+str(i)+ ": " + gpus[i]['model'] + " with " + str(gpus[i]['ram']//1024//1024) + " MBs of VRAM\n"
      info += 'UUID: ' + unique_system_id
      return info.strip()

# Be resilient to quotes and whitespace
def unwrap(s):
  s = s.strip()
  if (s.startswith('"') and s.endswith('"')) or (s.startswith("'") and s.endswith("'")):
    s = s[1:-1].strip()
  return s

def run():
  global browser_process, browser_exe, processname_killed_atexit, emrun_options, emrun_not_enabled_nag_printed, ADB
  usage_str = "emrun [emrun_options] filename.html [html_cmdline_options]\n\n   where emrun_options specifies command line options for emrun itself, whereas\n   html_cmdline_options specifies startup arguments to the program."
  parser = argparse.ArgumentParser(usage=usage_str)

  parser.add_argument('--kill_start', dest='kill_on_start', action='store_true', default=False,
    help='If true, any previously running instances of the target browser are killed before starting.')

  parser.add_argument('--kill_exit', dest='kill_on_exit', action='store_true', default=False,
    help='If true, the spawned browser process is forcibly killed when it calls exit(). Note: Using this option may require explicitly passing the option --browser=/path/to/browser, to avoid emrun being detached from the browser process it spawns.')

  parser.add_argument('--no_server', dest='no_server', action='store_true', default=False,
    help='If specified, a HTTP web server is not launched to host the page to run.')

  parser.add_argument('--no_browser', dest='no_browser', action='store_true', default=False,
    help='If specified, emrun will not launch a web browser to run the page.')

  parser.add_argument('--no_emrun_detect', dest='no_emrun_detect', action='store_true', default=False,
    help='If specified, skips printing the warning message if html page is detected to not have been built with --emrun linker flag.')

  parser.add_argument('--serve_after_close', dest='serve_after_close', action='store_true', default=False,
    help='If true, serves the web page even after the application quits by user closing the web page.')

  parser.add_argument('--serve_after_exit', dest='serve_after_exit', action='store_true', default=False,
    help='If true, serves the web page even after the application quits by a call to exit().')

  parser.add_argument('--serve_root', dest='serve_root', default='',
    help='If set, specifies the root path that the emrun web server serves. If not specified, the directory where the target .html page lives in is served.')

  parser.add_argument('--verbose', dest='verbose', action='store_true', default=False,
    help='Enable verbose logging from emrun internal operation.')

  parser.add_argument('--hostname', dest='hostname', default=default_webserver_hostname,
    help='Specifies the hostname the server runs in.')

  parser.add_argument('--port', dest='port', default=default_webserver_port, type=int,
    help='Specifies the port the server runs in.')

  parser.add_argument('--log_stdout', dest='log_stdout', default='',
    help='Specifies a log filename where the browser process stdout data will be appended to.')

  parser.add_argument('--log_stderr', dest='log_stderr', default='',
    help='Specifies a log filename where the browser process stderr data will be appended to.')

  parser.add_argument('--silence_timeout', dest='silence_timeout', type=int, default=0,
    help='If no activity is received in this many seconds, the browser process is assumed to be hung, and the web server is shut down and the target browser killed. Disabled by default.')

  parser.add_argument('--timeout', dest='timeout', type=int, default=0,
    help='If the browser process does not quit or the page exit() in this many seconds, the browser is assumed to be hung, and the web server is shut down and the target browser killed. Disabled by default.')

  parser.add_argument('--timeout_returncode', dest='timeout_returncode', type=int, default=99999,
    help='Sets the exit code that emrun reports back to caller in the case that a page timeout occurs. Default: 99999.')

  parser.add_argument('--list_browsers', dest='list_browsers', action='store_true',
    help='Prints out all detected browser that emrun is able to use with the --browser command and exits.')

  parser.add_argument('--browser', dest='browser', default='',
    help='Specifies the browser executable to run the web page in.')

  parser.add_argument('--browser_args', dest='browser_args', default='',
    help='Specifies the arguments to the browser executable.')

  parser.add_argument('--android', dest='android', action='store_true', default=False,
    help='Launches the page in a browser of an Android device connected to an USB on the local system. (via adb)')

  parser.add_argument('--system_info', dest='system_info', action='store_true',
    help='Prints information about the current system at startup.')

  parser.add_argument('--browser_info', dest='browser_info', action='store_true',
    help='Prints information about the target browser to launch at startup.')

  parser.add_argument('--json', dest='json', action='store_true',
    help='If specified, --system_info and --browser_info are outputted in JSON format.')

  parser.add_argument('--safe_firefox_profile', dest='safe_firefox_profile', action='store_true',
    help='If true, the browser is launched into a new clean Firefox profile that is suitable for unattended automated runs. (If target browser != Firefox, this parameter is ignored)')

  parser.add_argument('--log_html', dest='log_html', action='store_true',
    help='If set, information lines are printed out an HTML-friendly format.')

  parser.add_argument('serve', nargs='*')

  opts_with_param = ['--browser', '--browser_args', '--timeout_returncode', '--timeout', '--silence_timeout', '--log_stderr', '--log_stdout', '--hostname', '--port', '--serve_root']

  cmdlineparams = []
  # Split the startup arguments to two parts, delimited by the first (unbound) positional argument.
  # The first set is args intended for emrun, and the second set is the cmdline args to program.
  i = 1
  while i < len(sys.argv):
    if sys.argv[i] in opts_with_param:
      i += 1 # Skip next one, it's the value for this opt.
    elif not sys.argv[i].startswith('-'):
      cmdlineparams = sys.argv[i+1:]
      sys.argv = sys.argv[:i+1]
      break
    i += 1

  options = parser.parse_args(sys.argv[1:])
  args = options.serve
  emrun_options = options

  if options.android:
    ADB = which('adb')
    if not ADB:
      loge("Could not find the adb tool. Install Android SDK and add the directory of adb to PATH.")
      return 1

  if not options.browser and not options.android:
    if WINDOWS:
      options.browser = 'start'
    elif LINUX:
      options.browser = which('xdg-open')
      if not options.browser:
        options.browser = 'firefox'
    elif MACOS:
      options.browser = 'safari'

  if options.list_browsers:
    if options.android:
      list_android_browsers()
    else:
      list_pc_browsers()
    return

  if len(args) < 1 and (options.system_info or options.browser_info):
    options.no_server = options.no_browser = True # Don't run if only --system_info or --browser_info was passed.

  if len(args) < 1 and not (options.no_server == True and options.no_browser == True):
    logi(usage_str)
    logi('')
    logi('Type emrun --help for a detailed list of available options.')
    return

  file_to_serve = args[0] if len(args) else '.'
  file_to_serve_is_url = file_to_serve.startswith('file://') or file_to_serve.startswith('http://') or file_to_serve.startswith('https://')
  
  if options.serve_root:
    serve_dir = os.path.abspath(options.serve_root)
  else:
    if file_to_serve == '.' or file_to_serve_is_url: serve_dir = os.path.abspath('.')
    else: serve_dir = os.path.dirname(os.path.abspath(file_to_serve))
  if file_to_serve_is_url:
    url = file_to_serve
  else:
    url = os.path.relpath(os.path.abspath(file_to_serve), serve_dir)
    if len(cmdlineparams):
      url += '?' + '&'.join(cmdlineparams)
    hostname = socket.gethostbyname(socket.gethostname()) if options.android else options.hostname
    url = 'http://' + hostname + ':' + str(options.port)+'/'+url

  os.chdir(serve_dir)
  if not options.no_server:
    if options.no_browser:
      logi('Web server root directory: ' + os.path.abspath('.'))
    else:
      logv('Web server root directory: ' + os.path.abspath('.'))

  if options.android:
    if not options.no_browser or options.browser_info:
      if not options.browser:
        loge("Running on Android requires that you explicitly specify the browser to run with --browser <id>. Run emrun --android --list_browsers to obtain a list of installed browsers you can use.")
        return 1
      elif options.browser == 'firefox':
        browser_app = 'org.mozilla.firefox/.App'
      elif options.browser == 'firefox_beta':
        browser_app = 'org.mozilla.firefox_beta/.App'
      elif options.browser == 'firefox_aurora' or options.browser == 'fennec_aurora':
        browser_app = 'org.mozilla.fennec_aurora/.App'
      elif options.browser == 'firefox_nightly' or options.browser == 'fennec':
        browser_app = 'org.mozilla.fennec/.App'
      elif options.browser == 'chrome':
        browser_app = 'com.android.chrome/com.google.android.apps.chrome.Main'
      elif options.browser == 'chrome_beta':
        browser_app = 'com.chrome.beta/com.google.android.apps.chrome.Main'
      elif options.browser == 'chrome_dev':
        browser_app = 'com.chrome.dev/com.google.android.apps.chrome.Main'
      elif options.browser == 'chrome_canary':
        browser_app = 'com.chrome.canary/com.google.android.apps.chrome.Main'
      elif options.browser == 'opera':
        browser_app = 'com.opera.browser/com.opera.Opera'
      elif options.browser == 'opera_mini': # Launching the URL works, but page seems to never load (Fails with 'Network problem' even when other browsers work)
        browser_app = 'com.opera.mini.android/.Browser'
      elif options.browser =='dolphin': # Current stable Dolphin as of 12/2013 does not have WebGL support. 
        browser_app = 'mobi.mgeek.TunnyBrowser/.BrowserActivity'
      else:
        loge("Don't know how to launch browser " + options.browser + ' on Android!')
        return 1
      # To add support for a new Android browser in the list above:
      # 1. Install the browser to Android phone, connect it via adb to PC.
      # 2. Type 'adb shell pm list packages -f' to locate the package name of that application.
      # 3. Type 'adb pull <packagename>.apk' to copy the apk of that application to PC.
      # 4. Type 'aapt d xmltree <packagename>.apk AndroidManifest.xml > manifest.txt' to extract the manifest from the package.
      # 5. Locate the name of the main activity for the browser in manifest.txt and add an entry to above list in form 'appname/mainactivityname'

      url = url.replace('&', '\\&')
      browser = [ADB, 'shell', 'am', 'start', '-a', 'android.intent.action.VIEW', '-n', browser_app, '-d', url]
      print(str(browser))
      processname_killed_atexit = browser_app[:browser_app.find('/')]
  else: #Launching a web page on local system.
    if options.browser:
      options.browser = unwrap(options.browser)

    if not options.no_browser or options.browser_info:
      browser = find_browser(str(options.browser))
      if not browser:
        loge('Unable to find browser "' + str(options.browser) + '"! Check the correctness of the passed --browser=xxx parameter!')
        return 1
      browser_exe = browser[0]
      browser_args = shlex.split(unwrap(options.browser_args))

      if 'safari' in browser_exe.lower():
        # Safari has a bug that a command line 'Safari http://page.com' does not launch that page,
        # but instead launches 'file:///http://page.com'. To remedy this, must use the open -a command
        # to run Safari, but unfortunately this will end up spawning Safari process detached from emrun.
        if MACOS:
          browser = ['open', '-a', 'Safari'] + (browser[1:] if len(browser) > 1 else [])

        processname_killed_atexit = 'Safari'
      elif 'chrome' in browser_exe.lower():
        processname_killed_atexit = 'chrome'
        browser_args += ['--incognito', '--enable-nacl', '--enable-pnacl', '--disable-restore-session-state', '--enable-webgl', '--no-default-browser-check', '--no-first-run', '--allow-file-access-from-files']
    #    if options.no_server:
    #      browser_args += ['--disable-web-security']
      elif 'firefox' in browser_exe.lower():
        processname_killed_atexit = 'firefox'
      elif 'iexplore' in browser_exe.lower():
        processname_killed_atexit = 'iexplore'
        browser_args += ['-private']
      elif 'opera' in browser_exe.lower():
        processname_killed_atexit = 'opera'

      # In Windows cmdline, & character delimits multiple commmands, so must use ^ to escape them.
      if browser_exe == 'cmd':
        url = url.replace('&', '^&')
      browser += browser_args + [url]

  if options.kill_on_start:
    pname = processname_killed_atexit
    kill_browser_process()
    processname_killed_atexit = pname


  # Copy the profile over to Android.
  if options.android and options.safe_firefox_profile:
    profile_dir = create_emrun_safe_firefox_profile()
    def run(cmd):
      print(str(cmd))
      subprocess.call(cmd)

    run(['adb', 'shell', 'rm', '-rf', '/mnt/sdcard/safe_firefox_profile'])
    run(['adb', 'shell', 'mkdir', '/mnt/sdcard/safe_firefox_profile'])
    run(['adb', 'push', os.path.join(profile_dir, 'prefs.js'), '/mnt/sdcard/safe_firefox_profile/prefs.js'])
    browser += ['--es', 'args', '"--profile /mnt/sdcard/safe_firefox_profile"']

  # Create temporary Firefox profile to run the page with. This is important to run after kill_browser_process()/kill_on_start op above, since that
  # cleans up the temporary profile if one exists.
  if processname_killed_atexit == 'firefox' and options.safe_firefox_profile and not options.no_browser and not options.android:
    profile_dir = create_emrun_safe_firefox_profile()

    browser += ['-no-remote', '-profile', profile_dir.replace('\\', '/')]

  if options.system_info:
    logi('Time of run: ' + time.strftime("%x %X"))
    print(get_system_info(format_json=options.json))

  if options.browser_info:
    if options.android:
      if options.json:
        print(json.dumps({
          'browser': 'Android ' + browser_app
          }, indent=2))
      else:
        logi('Browser: Android ' + browser_app)
    else:
      print(get_browser_info(browser_exe, format_json=options.json))

  # Suppress run warning if requested.
  if options.no_emrun_detect:
    emrun_not_enabled_nag_printed = True

  global browser_stdout_handle, browser_stderr_handle
  if options.log_stdout:
    browser_stdout_handle = open(options.log_stdout, 'a')
  if options.log_stderr:
    if options.log_stderr == options.log_stdout:
      browser_stderr_handle = browser_stdout_handle
    else:
      browser_stderr_handle = open(options.log_stderr, 'a')

  if not options.no_server:
    logv('Starting web server: http://%s:%i/' % (options.hostname, options.port))
    httpd = HTTPWebServer((options.hostname, options.port), HTTPHandler)

  if not options.no_browser:
    logi("Executing %s" % ' '.join(browser))
    if browser[0] == 'cmd':
      serve_forever = True # Workaround an issue where passing 'cmd /C start' is not able to detect when the user closes the page.
    browser_process = subprocess.Popen(browser, env=subprocess_env())
    if options.kill_on_exit:
      atexit.register(kill_browser_process)
    # For Android automation, we execute adb, so this process does not represent a browser and no point killing it.
    if options.android:
      browser_process = None
  elif not options.no_server:
    logi('Now listening at http://%s:%i/' % (options.hostname, options.port))

  if browser_process:
    premature_quit_code = browser_process.poll()
    if premature_quit_code != None:
      options.serve_after_close = True
      logv('Warning: emrun got immediately detached from the target browser process (the process quit with exit code ' + str(premature_quit_code) + '). Cannot detect when user closes the browser. Behaving as if --serve_after_close was passed in.')
      if not options.browser:
        logv('Try passing the --browser=/path/to/browser option to avoid this from occurring. See https://github.com/kripken/emscripten/issues/3234 for more discussion.')
  
  if not options.no_server:
    try:
      httpd.serve_forever()
    except KeyboardInterrupt:
      pass
    httpd.server_close()

    logv('Closed web server.')

  if not options.no_browser:
    if options.kill_on_exit:
      kill_browser_process()
    else:
      if is_browser_process_alive():
        logv('Not terminating browser process, pass --kill_exit to terminate the browser when it calls exit().')
      # If we have created a temporary Firefox profile, we would really really like to wait until the browser closes,
      # or otherwise we'll just have to litter temp files and keep the temporary profile alive. It is possible here
      # that the browser is cooperatively shutting down, but has not yet had time to do so, so wait for a short while.
      if temp_firefox_profile_dir != None: time.sleep(3)

    if not is_browser_process_alive():
      # Browser is no longer running, make sure to clean up the temp Firefox profile, if we created one.
      delete_emrun_safe_firefox_profile()

  return page_exit_code

def main():
  returncode = run()
  logv('emrun quitting with process exit code ' + str(returncode))
  if temp_firefox_profile_dir != None:
    logi('Warning: Had to leave behind a temporary Firefox profile directory ' + temp_firefox_profile_dir + ' because --safe_firefox_profile was set and the browser did not quit before emrun did.')
  return returncode

if __name__ == '__main__':
  sys.exit(main())
