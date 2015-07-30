#!/usr/bin/env python

# emrun: Implements machinery that allows running a .html page as if it was a standard executable file.
# Usage: emrun <options> filename.html <args to program>
# See emrun --help for more information

import os, platform, optparse, logging, re, pprint, atexit, urlparse, subprocess, sys, SocketServer, BaseHTTPServer, SimpleHTTPServer, time, string, struct, socket, cgi
from operator import itemgetter
from urllib import unquote
from Queue import PriorityQueue
from threading import Thread, RLock

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

# If user does not specify a --port parameter, this port is used to launch the server.
default_webserver_port = 6931

# Location of Android Debug Bridge executable
ADB = ''

# Host OS detection to autolocate browsers and other OS-specific support needs.
WINDOWS = False
LINUX = False
OSX = False
if os.name == 'nt':
  WINDOWS = True
  try:
    import shlex
    import win32api, _winreg
    from win32com.client import GetObject
    from win32api import GetFileVersionInfo, LOWORD, HIWORD
    from _winreg import HKEY_CURRENT_USER, OpenKey, QueryValue
  except Exception, e:
    print >> sys.stderr, str(e)
    print >> sys.stderr, "Importing Python win32 modules failed! This most likely occurs if you do not have PyWin32 installed! Get it from http://sourceforge.net/projects/pywin32/"
    sys.exit(1)
elif platform.system() == 'Linux':
  LINUX = True
elif platform.mac_ver()[0] != '':
  OSX = True

  import plistlib

# If you are running on an OS that is not any of these, must add explicit support for it.
if not WINDOWS and not LINUX and not OSX:
  raise Exception("Unknown OS!")

# Returns wallclock time in seconds.
def tick():
  # Would like to return time.clock() since it's apparently better for precision, but it is broken on OSX 10.10 and Python 2.7.8.
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
      print >> sys.stdout, msg
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
        print >> sys.stdout, msg
      sys.stdout.flush()
      last_message_time = tick()

# Prints an error message to stderr channel.
def loge(msg):
  global last_message_time
  with http_mutex:
    if emrun_options.log_html:
      sys.stderr.write(format_html(msg))
    else:
      print >> sys.stderr, msg
    sys.stderr.flush()
    last_message_time = tick()

def format_eol(msg):
  if WINDOWS:
    msg = msg.replace('\r\n', '\n').replace('\n', '\r\n')
  return msg

# Prints a message to the browser stdout output stream.
def browser_logi(msg):
  global browser_stdout_handle
  msg = format_eol(msg)
  print >> browser_stdout_handle, msg
  browser_stdout_handle.flush()
  last_message_time = tick()

# Prints a message to the browser stderr output stream.
def browser_loge(msg):
  global browser_stderr_handle
  msg = format_eol(msg)
  print >> browser_stderr_handle, msg
  browser_stderr_handle.flush()
  last_message_time = tick()
  
# Unquotes a unicode string. (translates ascii-encoded utf string back to utf)
def unquote_u(source):
  result = unquote(source)
  if '%u' in result:
    result = result.replace('%u','\\u').decode('unicode_escape')
  return result

# Returns whether the browser page we spawned is still running.
# (note, not perfect atm, in case we are running in detached mode)
def is_browser_process_alive():
  global browser_process
  return browser_process and browser_process.poll() == None
  
# Kills browser_process and processname_killed_atexit.
def kill_browser_process():
  global browser_process, processname_killed_atexit, emrun_options, ADB
  if browser_process:
    try:
      logv('Terminating browser process..')
      browser_process.kill()
    except Exception, e:
      logv('Failed with error ' + str(e) + '!')
    browser_process = None
    processname_killed_atexit = ''
    return
  if len(processname_killed_atexit) > 0:
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
        except OSError, e:
          try:
            subprocess.call(['killall', processname_killed_atexit])
          except OSError, e:
            loge('Both commands pkill and killall failed to clean up the spawned browser process. Perhaps neither of these utilities is available on your system?')
    # Clear the process name to represent that the browser is now dead.
    processname_killed_atexit = ''

# Our custom HTTP web server that will server the target page to run via .html.
# This is used so that we can load the page via a http:// URL instead of a file:// URL, since those wouldn't work too well unless user allowed XHR without CORS rules.
# Also, the target page will route its stdout and stderr back to here via HTTP requests.
class HTTPWebServer(SocketServer.ThreadingMixIn, BaseHTTPServer.HTTPServer):
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
      if len(self.http_message_queue) > 0 and now - last_message_time > max_message_queue_time:
        self.print_next_message()
  
  # Skips to printing the next message in queue now, independent of whether there was missed messages in the sequence numbering.
  def print_next_message(self):
    with http_mutex:
      if len(self.http_message_queue) > 0:
        self.expected_http_seq_num = self.http_message_queue[0][0]
        self.print_messages_due()

  # Completely flushes all out-of-order messages in the queue.
  def print_all_messages(self):
    with http_mutex:
      while len(self.http_message_queue) > 0:
        self.print_next_message()

  # Prints any messages that are now due after we logged some other previous messages.
  def print_messages_due(self):
    with http_mutex:
      while len(self.http_message_queue) > 0:
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
        if not emrun_options.serve_after_close and browser_quit_code != None:
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
        logv('No activity in ' + str(emrun_options.silence_timeout) + ' seconds. Quitting web server with return code ' + str(emrun_options.timeout_returncode) + '. (--silence_timeout option)')
        page_exit_code = emrun_options.timeout_returncode
        emrun_options.kill_on_exit = True

      # If the page has been running too long as a whole, kill process.
      time_since_start = now - page_start_time
      if emrun_options.timeout != 0 and time_since_start > emrun_options.timeout:
        self.shutdown()
        logv('Page has not finished in ' + str(emrun_options.timeout) + ' seconds. Quitting web server with return code ' + str(emrun_options.timeout_returncode) + '. (--timeout option)')
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
    err = sys.exc_info()[1][0]
    # Filter out the useless '[Errno 10054] An existing connection was forcibly closed by the remote host' errors that occur when we 
    # forcibly kill the client.
    if err != 10054:
      SocketServer.BaseServer.handle_error(self, request, client_address)

  def shutdown(self):
    self.is_running = False
    self.print_all_messages()
    return 1

# Processes HTTP request back to the browser.
class HTTPHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
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
    ctype = self.guess_type(path)
    try:
      f = open(path, 'rb')
    except IOError:
      self.send_error(404, "File not found: " + path)
      return None
    self.send_response(200)
    self.send_header("Content-type", ctype)
    fs = os.fstat(f.fileno())
    self.send_header("Content-Length", str(fs[6]))
    self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
    self.send_header('Cache-Control','no-cache, must-revalidate')
    self.send_header('Connection','close')
    self.send_header('Expires','-1')
    self.end_headers()
    page_last_served_time = tick()
    return f

  def log_request(self, code):
    # Filter out 200 OK messages to remove noise.
    if code is not 200:
      SimpleHTTPServer.SimpleHTTPRequestHandler.log_request(self, code)

  def log_message(self, format, *args):
    msg = "%s - - [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format%args)
    if not 'favicon.ico' in msg: # Filter out 404 messages on favicon.ico not being found to remove noise.
      sys.stderr.write(msg)

  def do_POST(self):
    global page_exit_code, emrun_options, have_received_messages

    (_, _, path, query, _) = urlparse.urlsplit(self.path)
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
      print 'Wrote ' + str(len(data)) + ' bytes to file "' + filename + '".'
      have_received_messages = True
    else:
      data = self.rfile.read(int(self.headers['Content-Length']))
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
    self.wfile.write('OK')

# From http://stackoverflow.com/questions/4842448/getting-processor-information-in-python
# Returns a string with something like "AMD64, Intel(R) Core(TM) i5-2557M CPU @ 1.70GHz, Intel64 Family 6 Model 42 Stepping 7, GenuineIntel"
def get_cpu_infoline():
  try:
    if WINDOWS:
      root_winmgmts = GetObject("winmgmts:root\cimv2")
      cpus = root_winmgmts.ExecQuery("Select * from Win32_Processor")
      cpu_name = cpus[0].Name + ', ' + platform.processor()
    elif OSX:
      cpu_name = subprocess.check_output(['sysctl', '-n', 'machdep.cpu.brand_string']).strip()
    elif LINUX:
      command = "cat /proc/cpuinfo"
      all_info = subprocess.check_output(command, shell=True).strip()
      for line in all_info.split("\n"):
        if "model name" in line:
          cpu_name = re.sub( ".*model name.*:", "", line,1).strip()
  except:
    return "Unknown"

  return platform.machine() + ', ' + cpu_name

def get_android_cpu_infoline():
  lines = subprocess.check_output([ADB, 'shell', 'cat', '/proc/cpuinfo']).split('\n')
  processor = ''
  hardware = ''
  for line in lines:
    if line.startswith('Processor'):
      processor = line[line.find(':')+1:].strip()
    elif line.startswith('Hardware'):
      hardware = line[line.find(':')+1:].strip()

  freq = int(subprocess.check_output([ADB, 'shell', 'cat', '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq']).strip())/1000
  return 'CPU: ' + processor + ', ' + hardware + ' @ ' + str(freq) + ' MHz'

def win_print_gpu_info():
  gpus = []
  gpu_memory = []

  for i in range(0, 16):
    try:
      hHardwareReg = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "HARDWARE")
      hDeviceMapReg = _winreg.OpenKey(hHardwareReg, "DEVICEMAP")
      hVideoReg = _winreg.OpenKey(hDeviceMapReg, "VIDEO")
      VideoCardString = _winreg.QueryValueEx(hVideoReg,"\Device\Video"+str(i))[0]
      #Get Rid of Registry/Machine from the string
      VideoCardStringSplit = VideoCardString.split("\\")
      ClearnVideoCardString = string.join(VideoCardStringSplit[3:], "\\")
      #Go up one level for detailed
      VideoCardStringRoot = string.join(VideoCardStringSplit[3:len(VideoCardStringSplit)-1], "\\")

      #Get the graphics card information
      hVideoCardReg = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, ClearnVideoCardString)
      try:
        VideoCardDescription  = _winreg.QueryValueEx(hVideoCardReg, "Device Description")[0]
      except WindowsError:
        VideoCardDescription  = _winreg.QueryValueEx(hVideoCardReg, "DriverDesc")[0]
      
      try:
        driverVersion = _winreg.QueryValueEx(hVideoCardReg, "DriverVersion")[0]
        VideoCardDescription += ', driver version ' + driverVersion
      except:
        pass

      try:
        driverDate = _winreg.QueryValueEx(hVideoCardReg, "DriverDate")[0]
        VideoCardDescription += ' (' + driverDate + ')'
      except:
        pass

      VideoCardMemorySize = _winreg.QueryValueEx(hVideoCardReg, "HardwareInformation.MemorySize")[0]
      try:
        vram = struct.unpack('l',VideoCardMemorySize)[0]
      except struct.error:
        vram = int(VideoCardMemorySize)
      if not VideoCardDescription in gpus:
        gpus += [VideoCardDescription]
        gpu_memory += [str(int(vram/1024/1024))]
    except WindowsError:
      pass

  if len(gpus) == 1:
    print "GPU: " + gpus[0] + " with " + gpu_memory[0] + " MB of VRAM"
  elif len(gpus) > 1:
    for i in range(0, len(gpus)):
      print "GPU"+str(i)+ ": " + gpus[i] + " with " + gpu_memory[i] + " MBs of VRAM"

def linux_print_gpu_info():
  try:
    glxinfo = subprocess.check_output('glxinfo')
    for line in glxinfo.split("\n"):
      if "OpenGL vendor string:" in line:
        gl_vendor = line[len("OpenGL vendor string:"):].strip()
      if "OpenGL version string:" in line:
        gl_version = line[len("OpenGL version string:"):].strip()
      if "OpenGL renderer string:" in line:
        gl_renderer = line[len("OpenGL renderer string:"):].strip()
    logi('GPU: ' + gl_vendor + ' ' + gl_renderer + ', GL version ' + gl_version)
  except:
    pass

def osx_print_gpu_info():
  try:
    info = subprocess.check_output(['system_profiler', 'SPDisplaysDataType'])
    gpu = re.search("Chipset Model: (.*)", info)
    if gpu:
      chipset = gpu.group(1)
    vram = re.search("VRAM \(Total\): (.*) MB", info)
    if vram:
      logi('GPU: ' + chipset + ' with ' + vram.group(1) + ' MBs of VRAM')
    else:
      logi('GPU: ' + chipset)
  except:
    pass

def print_gpu_infolines():
  if WINDOWS:
    win_print_gpu_info()
  elif LINUX:
    linux_print_gpu_info()
  elif OSX:
    osx_print_gpu_info()

def get_executable_version(filename):
  try:
    if WINDOWS:
      info = GetFileVersionInfo(filename, "\\")
      ms = info['FileVersionMS']
      ls = info['FileVersionLS']
      version = HIWORD (ms), LOWORD (ms), HIWORD (ls), LOWORD (ls)
      return '.'.join(map(str, version))
    elif OSX:
      plistfile = app_name[0:app_name.find('MacOS')] + 'Info.plist'
      info = plistlib.readPlist(plistfile)
      # Data in Info.plists is a bit odd, this check combo gives best information on each browser.
      if 'firefox' in app_name.lower():
        return info['CFBundleShortVersionString'] + ' 20' + info['CFBundleVersion'][2:].replace('.', '-')
      if 'opera' in app_name.lower():
        return info['CFBundleVersion']
      else:
        return info['CFBundleShortVersionString']
    elif LINUX:
      if 'firefox' in filename.lower():
        version = subprocess.check_output([filename, '-v'])
        version = version.replace('Mozilla Firefox ', '')
        return version.strip()
      else:
        return ""
  except:
    return ""

# http://stackoverflow.com/questions/580924/python-windows-file-version-attribute
def win_get_file_properties(fname):
  propNames = ('Comments', 'InternalName', 'ProductName',
    'CompanyName', 'LegalCopyright', 'ProductVersion',
    'FileDescription', 'LegalTrademarks', 'PrivateBuild',
    'FileVersion', 'OriginalFilename', 'SpecialBuild')

  props = {'FixedFileInfo': None, 'StringFileInfo': None, 'FileVersion': None}

  try:
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

def get_os_version():
  try:
    if WINDOWS:
      versionHandle = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")
      productName = _winreg.QueryValueEx(versionHandle, "ProductName")
      return productName[0]
    elif OSX:
      return 'Mac OS ' + platform.mac_ver()[0]
    elif LINUX:
      kernel_version = subprocess.check_output(['uname', '-r']).strip()
      return ' '.join(platform.linux_distribution()) + ', linux kernel ' + kernel_version + ' ' + platform.architecture()[0]
  except:
    return 'Unknown OS'

def get_system_memory():
  global emrun_options

  try:
    if LINUX or emrun_options.android:
      if emrun_options.android:
        lines = subprocess.check_output([ADB, 'shell', 'cat', '/proc/meminfo']).split('\n')
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
    elif OSX:
      return int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']).strip())
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
  # Manually find the default system browser on Windows without relying on 'start %1' since
  # that method has an issue, see comment below.

  # In Py3, this module is called winreg without the underscore

  with OpenKey(HKEY_CURRENT_USER, r"Software\Classes\http\shell\open\command") as key:
    cmd = QueryValue(key, None)
    if cmd:
      parts = shlex.split(cmd)
      if len(parts) > 0:
        return [parts[0]]

    # Fall back to 'start %1', which we have to treat as if user passed --serve_forever, since
    # for some reason, we are not able to detect when the browser closes when this is passed.
    return ['cmd', '/C', 'start']

def find_browser(name):
  if WINDOWS and name == 'start':
    return win_get_default_browser()
  if OSX and name == 'open':
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
  if OSX:
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
  manufacturer = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.product.manufacturer']).strip()
  brand = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.product.brand']).strip()
  model = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.product.model']).strip()
  board = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.product.board']).strip()
  device = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.product.device']).strip()
  name = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.product.name']).strip()
  return manufacturer + ' ' + brand + ' ' + model + ' ' + board + ' ' + device + ' ' + name

def get_android_os_version():
  global ADB
  ver = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.build.version.release']).strip()
  apiLevel = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.build.version.sdk']).strip()
  if not apiLevel:
    apiLevel = subprocess.check_output([ADB, 'shell', 'getprop', 'ro.build.version.sdk_int']).strip()

  os = ''
  if ver:
    os += 'Android ' + ver + ' '
  if apiLevel:
    os += 'SDK API Level ' + apiLevel + ' '
  os += subprocess.check_output([ADB, 'shell', 'getprop', 'ro.build.description']).strip()
  return os

def list_android_browsers():
  global ADB
  apps = subprocess.check_output([ADB, 'shell', 'pm', 'list', 'packages', '-f']).replace('\r\n', '\n')
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
    product_name = win_get_file_properties(browser)['StringFileInfo']['ProductName'] if WINDOWS else 'firefox'
    if product_name.lower() != 'firefox':
      return 'Mozilla Firefox ' + product_name
    else:
      return 'Mozilla Firefox'
  if 'opera' in b:
    return 'Opera'
  if 'safari' in b:
    return 'Apple Safari'
  return browser

def main():
  global browser_process, processname_killed_atexit, emrun_options, emrun_not_enabled_nag_printed, ADB
  usage_str = "usage: emrun [emrun_options] filename.html [html_cmdline_options]\n\n   where emrun_options specifies command line options for emrun itself, whereas\n   html_cmdline_options specifies startup arguments to the program."
  parser = optparse.OptionParser(usage=usage_str)

  parser.add_option('--kill_start', dest='kill_on_start', action='store_true', default=False,
    help='If true, any previously running instances of the target browser are killed before starting.')

  parser.add_option('--kill_exit', dest='kill_on_exit', action='store_true', default=False,
    help='If true, the spawned browser process is forcibly killed when it calls exit(). Note: Using this option may require explicitly passing the option --browser=/path/to/browser, to avoid emrun being detached from the browser process it spawns.')

  parser.add_option('--no_server', dest='no_server', action='store_true', default=False,
    help='If specified, a HTTP web server is not launched to host the page to run.')

  parser.add_option('--no_browser', dest='no_browser', action='store_true', default=False,
    help='If specified, emrun will not launch a web browser to run the page.')

  parser.add_option('--no_emrun_detect', dest='no_emrun_detect', action='store_true', default=False,
    help='If specified, skips printing the warning message if html page is detected to not have been built with --emrun linker flag.')

  parser.add_option('--serve_after_close', dest='serve_after_close', action='store_true', default=False,
    help='If true, serves the web page even after the application quits by user closing the web page.')

  parser.add_option('--serve_after_exit', dest='serve_after_exit', action='store_true', default=False,
    help='If true, serves the web page even after the application quits by a call to exit().')

  parser.add_option('--serve_root', dest='serve_root', default='',
    help='If set, specifies the root path that the emrun web server serves. If not specified, the directory where the target .html page lives in is served.')

  parser.add_option('--verbose', dest='verbose', action='store_true', default=False,
    help='Enable verbose logging from emrun internal operation.')

  parser.add_option('--port', dest='port', default=default_webserver_port, type="int",
    help='Specifies the port the server runs in.')

  parser.add_option('--log_stdout', dest='log_stdout', default='',
    help='Specifies a log filename where the browser process stdout data will be appended to.')

  parser.add_option('--log_stderr', dest='log_stderr', default='',
    help='Specifies a log filename where the browser process stderr data will be appended to.')

  parser.add_option('--silence_timeout', dest='silence_timeout', type="int", default=0,
    help='If no activity is received in this many seconds, the browser process is assumed to be hung, and the web server is shut down and the target browser killed. Disabled by default.')

  parser.add_option('--timeout', dest='timeout', type="int", default=0,
    help='If the browser process does not quit or the page exit() in this many seconds, the browser is assumed to be hung, and the web server is shut down and the target browser killed. Disabled by default.')

  parser.add_option('--timeout_returncode', dest='timeout_returncode', type="int", default=99999,
    help='Sets the exit code that emrun reports back to caller in the case that a page timeout occurs. Default: 99999.')

  parser.add_option('--list_browsers', dest='list_browsers', action='store_true',
    help='Prints out all detected browser that emrun is able to use with the --browser command and exits.')

  parser.add_option('--browser', dest='browser', default='',
    help='Specifies the browser executable to run the web page in.')

  parser.add_option('--android', dest='android', action='store_true', default=False,
    help='Launches the page in a browser of an Android device connected to an USB on the local system. (via adb)')

  parser.add_option('--system_info', dest='system_info', action='store_true',
    help='Prints information about the current system at startup.')

  parser.add_option('--browser_info', dest='browser_info', action='store_true',
    help='Prints information about the target browser to launch at startup.')

  parser.add_option('--log_html', dest='log_html', action='store_true',
    help='If set, information lines are printed out an HTML-friendly format.')

  opts_with_param = ['--browser', '--timeout_returncode', '--timeout', '--silence_timeout', '--log_stderr', '--log_stdout', '--port', '--serve_root']

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

  (options, args) = parser.parse_args(sys.argv)
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
    elif OSX:
      options.browser = 'safari'

  if options.list_browsers:
    if options.android:
      list_android_browsers()
    else:
      list_pc_browsers()
    return

  if len(args) < 2 and (options.system_info or options.browser_info):
    options.no_server = options.no_browser = True # Don't run if only --system_info or --browser_info was passed.

  if len(args) < 2 and not (options.no_server == True and options.no_browser == True):
    logi(usage_str)
    logi('')
    logi('Type emrun --help for a detailed list of available options.')
    return

  file_to_serve = args[1] if len(args) > 1 else ''
  
  if options.serve_root:
    serve_dir = os.path.abspath(options.serve_root)
  else:
    if file_to_serve == '.': serve_dir = os.path.abspath(file_to_serve)
    else: serve_dir = os.path.dirname(os.path.abspath(file_to_serve))
  url = os.path.relpath(os.path.abspath(file_to_serve), serve_dir)
  if len(cmdlineparams) > 0:
    url += '?' + '&'.join(cmdlineparams)
  server_root = 'localhost'
  if options.android:
    server_root = socket.gethostbyname(socket.gethostname())
  url = 'http://' + server_root + ':' + str(options.port)+'/'+url
  
  os.chdir(serve_dir)
  logv('Web server root directory: ' + os.path.abspath('.'))

  if options.android:
    if not options.no_browser:
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
        browser_app = 'com.android.chrome/.Main'
      elif options.browser == 'chrome_beta' or options.browser == 'chrome_canary': # There is no Chrome Canary for Android, but Play store has 'Chrome Beta' instead.
        browser_app = 'com.chrome.beta/com.android.chrome.Main'
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

      if WINDOWS:
        url = url.replace('&', '\\&')
      browser = [ADB, 'shell', 'am', 'start', '-a', 'android.intent.action.VIEW', '-n', browser_app, '-d', url]
      processname_killed_atexit = browser_app[:browser_app.find('/')]
  else: #Launching a web page on local system.
    if options.browser:
      # Be resilient to quotes and whitespace
      options.browser = options.browser.strip()
      if (options.browser.startswith('"') and options.browser.endswith('"')) or (options.browser.startswith("'") and options.browser.endswith("'")):
        options.browser = options.browser[1:-1].strip()
    browser = find_browser(str(options.browser))
    if not browser:
      loge('Unable to find browser "' + str(options.browser) + '"! Check the correctness of the passed --browser=xxx parameter!')
      return 1
    browser_exe = browser[0]
    browser_args = []

    if 'safari' in browser_exe.lower():
      # Safari has a bug that a command line 'Safari http://page.com' does not launch that page,
      # but instead launches 'file:///http://page.com'. To remedy this, must use the open -a command
      # to run Safari, but unfortunately this will end up spawning Safari process detached from emrun.
      if OSX:
        browser = ['open', '-a', 'Safari'] + (browser[1:] if len(browser) > 1 else [])

      processname_killed_atexit = 'Safari'
    elif 'chrome' in browser_exe.lower():
      processname_killed_atexit = 'chrome'
      browser_args = ['--incognito', '--enable-nacl', '--enable-pnacl', '--disable-restore-session-state', '--enable-webgl', '--no-default-browser-check', '--no-first-run', '--allow-file-access-from-files']
  #    if options.no_server:
  #      browser_args += ['--disable-web-security']
    elif 'firefox' in browser_exe.lower():
      processname_killed_atexit = 'firefox'
    elif 'iexplore' in browser_exe.lower():
      processname_killed_atexit = 'iexplore'
      browser_args = ['-private']
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

  if options.system_info:
    logi('Time of run: ' + time.strftime("%x %X"))
    if options.android:
      logi('Model: ' + get_android_model())
      logi('OS: ' + get_android_os_version() + ' with ' + str(get_system_memory()/1024/1024) + ' MB of System RAM')
      logi('CPU: ' + get_android_cpu_infoline())
    else:
      logi('Computer name: ' + socket.gethostname()) # http://stackoverflow.com/questions/799767/getting-name-of-windows-computer-running-python-script
      logi('OS: ' + get_os_version() + ' with ' + str(get_system_memory()/1024/1024) + ' MB of System RAM')
      logi('CPU: ' + get_cpu_infoline())
      print_gpu_infolines()
  if options.browser_info:
    if options.android:
      logi('Browser: Android ' + browser_app)
    else:
      logi('Browser: ' + browser_display_name(browser[0]) + ' ' + get_executable_version(browser_exe))

  # Suppress run warning if requested.
  if options.no_emrun_detect:
    emrun_not_enabled_nag_printed = True

  global browser_stdout_handle, browser_stderr_handle
  if options.log_stdout:
    browser_stdout_handle = open(options.log_stdout, 'ab')
  if options.log_stderr:
    if options.log_stderr == options.log_stdout:
      browser_stderr_handle = browser_stdout_handle
    else:
      browser_stderr_handle = open(options.log_stderr, 'ab')

  if not options.no_server:
    logv('Starting web server in port ' + str(options.port))
    httpd = HTTPWebServer(('', options.port), HTTPHandler)

  if not options.no_browser:
    logv("Executing %s" % ' '.join(browser))
    if browser[0] == 'cmd':
      serve_forever = True # Workaround an issue where passing 'cmd /C start' is not able to detect when the user closes the page.
    browser_process = subprocess.Popen(browser)
    if options.kill_on_exit:
      atexit.register(kill_browser_process)
    # For Android automation, we execute adb, so this process does not represent a browser and no point killing it.
    if options.android:
      browser_process = None

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
    elif is_browser_process_alive():
      logv('Not terminating browser process, pass --kill_exit to terminate the browser when it calls exit().')

  return page_exit_code

if __name__ == '__main__':
  returncode = main()
  logv('emrun quitting with process exit code ' + str(returncode))
  sys.exit(returncode)
