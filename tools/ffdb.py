#!/usr/bin/env python

import socket, json, sys, uuid, datetime, time, logging, cgi, zipfile, os, tempfile, atexit, subprocess, re, base64, struct, imghdr

WINDOWS = sys.platform == 'win32'
if WINDOWS:
  import ctypes
  stdout_handle = ctypes.windll.kernel32.GetStdHandle(-11)

LOG_FORMAT = 'short' # Either 'bare', 'short', or 'long'
ADB = 'adb'          # Path to the adb executable
LOG_VERBOSE = False  # Verbose printing enabled with --verbose
HOST = 'localhost'   # The remote host to connect to the B2G device
PORT = 6000          # The port on the host on which the B2G device listens on
b2g_socket = None    # Python socket object for the active connection to the B2G device
read_queue = ''      # Inbound queue of partial data read so far from the device

webappsActorName = None

def sizeof_fmt(num):
    for x in ['bytes','KB','MB','GB']:
        if num < 1024.0:
            return "%3.1f%s" % (num, x)
        num /= 1024.0
    return "%3.1f%s" % (num, 'TB')

def zipdir(path, zipfilename):
  try:
    import zlib
    zip_mode = zipfile.ZIP_DEFLATED
  except:
    zip_mode = zipfile.ZIP_STORED

  zipf = zipfile.ZipFile(zipfilename, 'w', zip_mode)
  files_to_compress = []
  for root, dirs, files in os.walk(path):
    for file in files:
      files_to_compress += [(root, file)]

  n = 1
  for tuple in files_to_compress:
    (root, file) = tuple
    filename = os.path.join(root, file)
    filesize = os.path.getsize(filename)
    path_in_archive = os.path.relpath(filename, path)
    print 'Compressing ' + str(n) + '/' + str(len(files_to_compress)) + ': "' + path_in_archive + '" (' + sizeof_fmt(filesize) + ')...'
    n += 1
    zipf.write(os.path.join(root, file), path_in_archive)
  zipf.close()
  print 'Done. '

# Returns given log message formatted to be outputted on a HTML page.
def format_html(msg):
  if not msg.endswith('\n'):
    msg += '\n'
  msg = cgi.escape(msg)
  msg = msg.replace('\r\n', '<br />').replace('\n', '<br />')
  return msg

# Prints a verbose log message to stdout channel. Only shown if run with --verbose.
def logv(msg):
  if LOG_VERBOSE:
    sys.stdout.write(format_html(msg) + '\n')
    sys.stdout.flush()

# Reads data from the socket, and tries to parse what we have got so far as a JSON message.
# The messages are of form "bytelength:{jsondict}", where bytelength tells how many bytes
# there are in the data that comes after the colon.
# Returns a JSON dictionary of the received message.
def read_b2g_response(print_errors_to_console = True):
  global read_queue, b2g_socket
  payload = ''
  while True:
    semicolon = float('Inf')
    payload_len = float('Inf')
    try:
      semicolon = read_queue.index(':')
      payload_len = int(read_queue[:semicolon])
    except:
      pass
    if semicolon+1+payload_len > len(read_queue):
      try:
        read_queue += b2g_socket.recv(4096)
      except socket.timeout, e: 
        pass # We simulate blocking sockets with looping over reads that time out, since on Windows, the user cannot press Ctrl-C to break on blocking sockets.
      except Exception, e:
        if e[0] == 57: # Socket is not connected
          print 'Error! Failed to receive data from the device: socket is not connected!'
          sys.exit(1)
        else:
          print 'Got exception ' + str(e)
          raise
      continue
    payload = read_queue[semicolon+1:semicolon+1+payload_len]
    read_queue = read_queue[semicolon+1+payload_len:]
    logv('Read a message of size ' + str(payload_len) + 'b from socket.')
    payload = json.loads(payload)
    # Log received errors immediately to console
    if print_errors_to_console and 'error' in payload:
      print >> sys.stderr, 'Received error "' + payload['error'] + '"! Reason: ' + payload['message']
    else:
      break
  return payload

# Sends a command to the B2G device and waits for the response and returns it as a JSON dict.
def send_b2g_cmd(to, cmd, data = {}, print_errors_to_console = True):
  global b2g_socket
  msg = { 'to': to, 'type': cmd}
  msg = dict(msg.items() + data.items())
  msg = json.dumps(msg, encoding='latin-1')
  msg = msg.replace('\\\\', '\\')
  msg = str(len(msg))+':'+msg
  logv('Sending cmd:' + cmd + ' to:' + to)
  b2g_socket.sendall(msg)
  return read_b2g_response(print_errors_to_console)

def escape_bytes(b):
  return str(b)

# Sends a data fragment of a packaged app upload. This is a special-case version of the send_b2g_cmd
# command optimized for performance.
def send_b2g_data_chunk(to, data_blob):
  byte_str = []
  e = '\u0000'
  # '"' == 34
  # '\' == 92
  i = 0
  while i < len(data_blob):
    o = ord(data_blob[i])
#    if o == 34 or o == 92 or o >= 128 or o <= 32:#o <= 32 or o >= 36:# or o == ord('\\'):
    if o <= 34 or o >= 128 or o == 92:
      c = hex(o)[2:]
      byte_str += e[:-len(c)] + c
    else:
      byte_str += data_blob[i]
    i += 1
  message = '{"to":"'+to+'","type":"chunk","chunk":"' + ''.join(byte_str) + '"}'
  message = str(len(message)) + ':' + message
  logv('{"to":"'+to+'","type":"chunk","chunk":"<data>"}')
  b2g_socket.sendall(message)
  return read_b2g_response()

def send_b2g_bulk_data(to, data_blob):
  message = 'bulk ' + to + ' stream ' + str(len(data_blob)) + ':'
  logv(message)
  b2g_socket.sendall(message)
  b2g_socket.sendall(data_blob)
  # It seems that B2G doesn't send any response JSON back after a bulk transfer is finished, so no read_b2g_response() here.

# Queries the device for a list of all installed apps.
def b2g_get_appslist():
  global webappsActorName
  apps = send_b2g_cmd(webappsActorName, 'getAll')
  return apps['apps']

# Queries the device for a list of all currently running apps.
def b2g_get_runningapps():
  global webappsActorName
  apps = send_b2g_cmd(webappsActorName, 'listRunningApps')
  return apps['apps'] # Returns manifestURLs of all running apps

def print_applist(applist, running_app_manifests, print_removable):
  num_printed = 0
  for app in applist:
    if print_removable or app['removable']: # Print only removable apps unless --all is specified, skip the built-in apps that can't be uninstalled.
      if 'manifest' in app and 'version' in app['manifest']:
        version = " version '" + app['manifest']['version'] + "'"
      else:
        version = ''
      if app['manifestURL'] in running_app_manifests:
        version += '   RUNNING'
      print '   ' + str(app['localId']) + ': "' + app['name'] + '"' + version
      num_printed += 1
  return num_printed

def adb_devices():
  try:
    devices = subprocess.check_output([ADB, 'devices'])
    devices = devices.strip().split('\n')[1:]
    devices = map(lambda x: x.strip().split('\t'), devices)
    return devices
  except Exception, e:
    return []

def b2g_get_prefs_filename():
  return subprocess.check_output([ADB, 'shell', 'echo', '-n', '/data/b2g/mozilla/*.default/prefs.js'])

def b2g_get_prefs_data():
  return subprocess.check_output([ADB, 'shell', 'cat', '/data/b2g/mozilla/*.default/prefs.js'])

def b2g_get_pref(sub):
  prefs_data = b2g_get_prefs_data().split('\n')
  # Filter to find all prefs that have the substring 'sub' in them.
  r = re.compile('user_pref\w*\(\w*"([^"]*)"\w*,\w*([^\)]*)')
  for line in prefs_data:
    m = r.match(line)
    if m and (sub is None or sub in m.group(1)):
      print m.group(1) + ': ' + m.group(2).strip()

def b2g_set_pref(pref, value):
  prefs_data = b2g_get_prefs_data().split('\n')
  # Remove any old value of this pref.
  r = re.compile('user_pref\w*\(\w*"([^"]*)"\w*,\w*([^\)]*)')
  new_prefs_data = []
  for line in prefs_data:
    m = r.match(line)
    if not m or m.group(1) != pref:
      new_prefs_data += [line]

  if value != None:
    print 'Setting pref "' + pref + '" = ' + value
    new_prefs_data += ['user_pref("' + pref + '", ' + value + ');']
  else:
    print 'Unsetting pref "' + pref + '"'
  (oshandle, tempfilename) = tempfile.mkstemp(suffix='.js', prefix='ffdb_temp_')
  os.write(oshandle, '\n'.join(new_prefs_data));

  # Write the new pref
  subprocess.check_output([ADB, 'shell', 'stop', 'b2g'])
  subprocess.check_output([ADB, 'push', tempfilename, b2g_get_prefs_filename()])
  subprocess.check_output([ADB, 'shell', 'start', 'b2g'])
  print 'Rebooting phone...'

  def delete_temp_file():
    os.remove(tempfilename)
  atexit.register(delete_temp_file)

def get_packaged_app_manifest(target_app_path):
  if os.path.isdir(target_app_path):
    manifest_file = os.path.join(target_app_path, 'manifest.webapp')
    if not os.path.isfile(manifest_file):
      print "Error: Failed to find FFOS packaged app manifest file '" + manifest_file + "'! That directory does not contain a packaged app?"
      sys.exit(1)
    return json.loads(open(manifest_file, 'r').read())
  elif target_app_path.endswith('.zip') and os.path.isfile(target_app_path):
    try:
      z = zipfile.ZipFile(target_app_path, "r")
      bytes = z.read('manifest.webapp')
    except Exception, e:
      print "Error: Failed to read FFOS packaged app manifest file 'manifest.webapp' in zip file '" + target_app_path + "'! Error: " + str(e)
      sys.exit(1)
      return None
    return json.loads(str(bytes))
  else:
      print "Error: Path '" + target_app_path + "' is neither a directory or a .zip file to represent the location of a FFOS packaged app!"
      sys.exit(1)
  return None

def b2g_install(target_app_path):
  if os.path.isdir(target_app_path):
    print 'Zipping up the contents of directory "' + target_app_path + '"...'
    (oshandle, tempzip) = tempfile.mkstemp(suffix='.zip', prefix='ffdb_temp_')
    zipdir(target_app_path, tempzip)
    target_app_path = tempzip
    # Remember to delete the temporary package after we quit.
    def delete_temp_file():
      os.remove(tempzip)
    atexit.register(delete_temp_file)

  print 'Uploading application package "' + target_app_path + '"...'
  print 'Size of compressed package: ' + sizeof_fmt(os.path.getsize(target_app_path)) + '.'
  app_file = open(target_app_path, 'rb')
  data = app_file.read()
  file_size = len(data)

  uploadResponse = send_b2g_cmd(webappsActorName, 'uploadPackage', { 'bulk': 'true'}, print_errors_to_console = False) # This may fail if on old device.
  start_time = time.time()
  if 'actor' in uploadResponse and 'BulkActor' in uploadResponse['actor']: # New B2G 2.0 hotness: binary data transfer
    packageUploadActor = uploadResponse['actor']
    send_b2g_bulk_data(packageUploadActor, data)
  else: # Old B2G 1.4 and older, serialize binary data in JSON text strings (SLOW!)
    print 'Bulk upload is not supported, uploading binary data with old slow format. Consider flashing your device to FFOS 2.0 or newer to enjoy faster upload speeds.'
    uploadResponse = send_b2g_cmd(webappsActorName, 'uploadPackage')
    packageUploadActor = uploadResponse['actor']
    chunk_size = 4*1024*1024
    i = 0
    while i < file_size:
      chunk = data[i:i+chunk_size]

      send_b2g_data_chunk(packageUploadActor, chunk)
      i += chunk_size
      bytes_uploaded = min(i, file_size)
      cur_time = time.time()
      secs_elapsed = cur_time - start_time
      percentage_done = bytes_uploaded * 1.0 / file_size
      total_time = secs_elapsed / percentage_done
      time_left = total_time - secs_elapsed
      print sizeof_fmt(bytes_uploaded) + " uploaded, {:5.1f} % done.".format(percentage_done*100.0) + ' Elapsed: ' + str(int(secs_elapsed)) + ' seconds. Time left: ' + str(datetime.timedelta(seconds=int(time_left))) + '. Data rate: {:5.2f} KB/second.'.format(bytes_uploaded / 1024.0 / secs_elapsed)

  app_local_id = str(uuid.uuid4())
  reply = send_b2g_cmd(webappsActorName, 'install', { 'appId': app_local_id, 'upload': packageUploadActor })
  cur_time = time.time()
  secs_elapsed = cur_time - start_time
  print 'Upload of ' + sizeof_fmt(file_size) + ' finished. Total time elapsed: ' + str(int(secs_elapsed)) + ' seconds. Data rate: {:5.2f} KB/second.'.format(file_size / 1024.0 / secs_elapsed)
  if not 'appId' in reply:
    print 'Error: Application install failed! ' + str(reply)
    sys.exit()
  return reply['appId']

def b2g_app_command(app_command, app_name, print_errors_to_console = True):
  apps = b2g_get_appslist()
  for app in apps:
    if str(app['localId']) == app_name or app['name'] == app_name or app['manifestURL'] == app_name or app['id'] == app_name:
      send_b2g_cmd(webappsActorName, app_command, { 'manifestURL': app['manifestURL'] })
      return 0
  if print_errors_to_console:
    print 'Error! Application "' + app_name + '" was not found! Use the \'list\' command to find installed applications.'
  return 1

def b2g_memory(app_name):
  apps = b2g_get_appslist()
  appActor = ''
  for app in apps:
    if str(app['localId']) == app_name or app['name'] == app_name or app['manifestURL'] == app_name or app['id'] == app_name:
      appActor = send_b2g_cmd(webappsActorName, 'getAppActor', { 'manifestURL': app['manifestURL'] })
      break
  if 'actor' in appActor:
    memoryActor = appActor['actor']['memoryActor']
    measure = send_b2g_cmd(memoryActor, 'measure')
    for k,v in measure.items():
      if k != 'from':
        if k in ['otherSize', 'jsStringsSize', 'jsObjectsSize', 'styleSize', 'jsOtherSize', 'domSize', 'total']: # These are formatted in bytes
          print k + ': ' + sizeof_fmt(v)
        else:
          print k + ': ' + str(v)

def b2g_log(app_name, clear=False):
  global LOG_FORMAT
  apps = b2g_get_appslist()
  appActor = ''
  for app in apps:
    if str(app['localId']) == app_name or app['name'] == app_name or app['manifestURL'] == app_name or app['id'] == app_name:
      appActor = send_b2g_cmd(webappsActorName, 'getAppActor', { 'manifestURL': app['manifestURL'] })
      break
  if 'actor' in appActor:
    consoleActor = appActor['actor']['consoleActor']

    if clear:
      send_b2g_cmd(consoleActor, 'clearMessagesCache')
      print 'Cleared message log.'
      return 0

    msgs = send_b2g_cmd(consoleActor, 'startListeners', { 'listeners': ['PageError','ConsoleAPI','NetworkActivity','FileActivity'] })

    if WINDOWS:
      WARNING = 14 # FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
      FAIL = 12 # FOREGROUND_RED | FOREGROUND_INTENSITY
      INFO = 7 # FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
      ENDC = ''
      BOLD = ''
    else:
      WARNING = '\033[93m'
      FAIL = '\033[91m'
      INFO = ENDC = '\033[0m'
      BOLD = "\033[1m"

    def set_color(string, color):
      if WINDOWS:
        ctypes.windll.kernel32.SetConsoleTextAttribute(stdout_handle, color)
        return string
      else:
        return color + string + ENDC

    def reset_color():
      if WINDOWS:
        ctypes.windll.kernel32.SetConsoleTextAttribute(stdout_handle, INFO)

    def log_b2g_message(msg):
      msgs = []
      if 'type' in msg and msg['type'] == 'consoleAPICall':
        msgs = [msg['message']]
      elif 'messages' in msg:
        msgs = msg['messages']

      for m in msgs:
        args = m['arguments']

        for arg in args:
          if LOG_FORMAT == 'long':
            text = str(m['functionName']) + '@' + str(m['filename']) + ':' + str(m['lineNumber']) + ': ' + str(arg)
          elif LOG_FORMAT == 'bare':
            text = str(arg)
          else: # Default to 'short'
            text = str(m['functionName']) + '@' + os.path.basename(str(m['filename'])) + ':' + str(m['lineNumber']) + ': ' + str(arg)

          if m['level'] == 'log':
            color = INFO
            channel = 'I/'
          elif m['level'] == 'warn':
            color = WARNING
            channel = 'W/'
          elif m['level'] == 'error':
            color = FAIL
            channel = 'E/'
          else:
            color = INFO
            channel = m['level'] + '/'

          text = set_color(channel + text, color)
          print text
          reset_color()

    msgs = send_b2g_cmd(consoleActor, 'getCachedMessages', { 'messageTypes': ['PageError', 'ConsoleAPI'] })
    log_b2g_message(msgs)

    while True:
      msg = read_b2g_response()
      log_b2g_message(msg)
  else:
    print 'Application "' + sys.argv[2] + '" is not running!'

def b2g_screenshot(filename):
  global deviceActorName
  data_reply = send_b2g_cmd(deviceActorName, 'screenshotToDataURL')
  data = data_reply['value']
  if not isinstance(data, basestring): # The device is sending the screenshot in multiple fragments since it's too long to fit in one message?
    data_get_actor = data['actor']
    data_len = int(data['length'])
    data = data['initial']
    chunk_size = 65000
    pos = len(data)
    # Pull and assemble individual screenshot fragments.
    while pos < data_len:
      bytes_to_read = min(data_len - pos, chunk_size)
      data_reply = send_b2g_cmd(data_get_actor, 'substring', { 'start': str(pos), 'end': str(pos + bytes_to_read) })
      if len(data_reply['substring']) != bytes_to_read:
        print >> sys.stderr, 'Error! Expected to receive ' + str(bytes_to_read) + ' bytes of image data, but got ' + str(len(data_reply['substring'])) + ' bytes instead!'
        sys.exit(1)
      data += data_reply['substring']
      pos += bytes_to_read
    send_b2g_cmd(data_get_actor, 'release') # We need to explicitly free the screenshot image string from the device, or the Devtools connection leaks resources!

  # Expected format is "data:image/png;base64,<base64data>"
  delim = re.search(",", data).start()
  data_format = data[:delim]
  if data_format != "data:image/png;base64":
    print >> sys.stderr, "Error: Received screenshot from device in an unexpected format '" + data_format + "'!"
    sys.exit(1)
  data = data[delim+1:]

  binary_data = base64.b64decode(data)
  open(filename, 'wb').write(binary_data)

  def get_png_image_size(filename):
    fhandle = open(filename, 'rb')
    head = fhandle.read(24)
    if len(head) != 24:
      return (-1, -1)
    check = struct.unpack('>i', head[4:8])[0]
    if check != 0x0d0a1a0a:
      return (-1, -1)
    return struct.unpack('>ii', head[16:24])

  width, height = get_png_image_size(filename)
  if width <= 0 or height <= 0:
    print >> sys.stderr, "Wrote " + sizeof_fmt(len(binary_data)) + " to file '" + filename + "', but the contents may be corrupted!"
  else:
    print "Wrote " + sizeof_fmt(len(binary_data)) + " to file '" + filename + "' (" + str(width) + 'x' + str(height) + ' pixels).'

def b2g_get_description(desc):
  global deviceActorName
  data_reply = send_b2g_cmd(deviceActorName, 'getDescription')
  # First try an exact match to requested desc
  if desc and desc in data_reply['value']:
    print desc + ': ' + str(data_reply['value'][desc])
  else: # Print all with case-insensitive substring search
    for k,v in data_reply['value'].items():
      if not desc or desc.lower() in k.lower():
        print k + ': ' + str(v)

def main():
  global b2g_socket, webappsActorName, deviceActorName, HOST, PORT, VERBOSE, ADB
  if len(sys.argv) < 2 or '--help' in sys.argv or 'help' in sys.argv or '-v' in sys.argv:
    print '''Firefox OS Debug Bridge, a tool for automating FFOS device tasks from the command line.

    Usage: ffdb.py <command>, where command is one of:

    list [--running] [--all]: Prints out the user applications installed on the device.
                              If --running is passed, only the currently opened apps are shown.
                              If --all is specified, then also uninstallable system applications are listed.
    launch <app> [--log]: Starts the given application. If already running, brings to front. If the --log option is passed, ffdb will
                          start persistently logging the execution of the given application.
    close <app>: Terminates the execution of the given application.
    uninstall <app>: Removes the given application from the device.
    install <path> [--run] [--log]: Uploads and installs a packaged app that resides in the given local directory.
                    <path> may either refer to a directory containing a packaged app, or to a prepackaged zip file.
                    If the --run option is passed, the given application is immediately launched after the installation finishes.
                    If the --log option is passed, ffdb will start persistently logging the execution of the installed application.
    log <app> [--clear]: Starts a persistent log listener that reads web console messages from the given application.
                         If --clear is passed, the message log for that application is cleared instead.
    memory <app>: Dumps a memory usage summary for the given application.
    navigate <url>: Opens the given web page in the B2G browser.
    screenshot [filename.png]: Takes a screenshot of the current contents displayed on the device. If an optional
                               filename is specified, the screenshot is saved to that file. Otherwise the filename
                               will be autogenerated.
    get [pref]: Fetches the value of the given developer pref option from the FFOS device and prints it to console. The parameter pref
                is optional and may either be the full name of a pref, or a substring to search for. All matching prefs will be printed.
                If no pref parameter is given, all prefs are printed.
                NOTE: This function (currently at least) only reports prefs that have been explicitly set and don't have their default value.
    set <pref> <value>: Writes the given pref option to the FFOS device and restarts the B2G process on it for the change to take effect.
    unset <pref>: Removes the given pref option from the FFOS device and restarts the B2G process on it for the change to take effect.

    hide-prompt: Permanently removes the remote debugging connection dialog from showing up, and reboots the phone. This command is
                 provided for conveniency, and is the same as calling './ffdb.py set devtools.debugger.prompt-connection false'
    restore-prompt: Restores the remote debugging connection dialog prompt to its default state.

    desc [desc]: Fetches the value of the given device description field. These fields are read-only and describe the current system.
                    If the optional desc parameter is omitted, all device descriptions are printed. Otherwise the given description is
                    printed if it is an exact match, or all descriptions containing desc as the substring are printed.

    Options: Additionally, the following options may be passed to control FFDB execution:

      --host <hostname>: Specifies the target network address to connect to. Default: 'localhost'.
      --port <number>: Specifies the network port to connect to. Default: 6000.
      --verbose: Enables verbose printing, mostly useful for debugging.
      --simulator: Signal that we will be connecting to a FFOS simulator and not a real device.

  In the above, whenever a command requires an <app> to be specified, either the human-readable name, 
  localId or manifestURL of the application can be used.'''

    sys.exit(0)

  connect_to_simulator = False

  options_with_value = ['--host', '--port']
  options = options_with_value + ['--verbose', '--simulator']
  # Process options
  for i in range(0, len(sys.argv)):
    if sys.argv[i] in options_with_value:
      if i+1 >= sys.argv or sys.argv[i+1].startswith('-'):
        print >> sys.stderr, "Missing value for option " + sys.argv[i] +'!'
        sys.exit(1)
    if sys.argv[i] == '--host':
      HOST = sys.argv[i+1]
    elif sys.argv[i] == '--port':
      PORT = int(sys.argv[i+1])
    elif sys.argv[i] == '--verbose':
      VERBOSE = True
    elif sys.argv[i] == '--simulator':
      connect_to_simulator = True

    # Clear the processed options so that parsing the commands below won't trip up on these.
    if sys.argv[i] in options: sys.argv[i] = ''
    if sys.argv[i] in options_with_value: sys.argv[i+1] = ''

  sys.argv = filter(lambda x: len(x) > 0, sys.argv)

  # Double-check that the device is found via adb:
  if (HOST == 'localhost' or HOST == '127.0.0.1') and not connect_to_simulator:
    devices = adb_devices()
    if len(devices) == 0:
      print 'Error! Failed to connect to B2G device debugger socket at address ' + HOST + ':' + str(PORT) + ' and no devices were detected via adb. Please double-check the following and try again: '
      print ' 1) The device is powered on and connected to the computer with an USB cable.'
      print ' 2) ADB and DevTools debugging is enabled on the device. (Settings -> Developer -> Debugging via USB: "ADB and DevTools"'
      print ' 3) The device is listed when you run "adb devices" on the command line.'
      print ' 4) When launching ffdb, remember to acknowledge the "incoming debug connection" dialog if it pops up on the device.'
      sys.exit(1)
  b2g_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  if WINDOWS:
    # Python Windows issue: user cannot press Ctrl-C to abort from a socket .recv() Therefore simulate blocking sockets with looping over reads that time out.
    b2g_socket.settimeout(0.5)
  try:
    b2g_socket.connect((HOST, PORT))
  except Exception, e:
    if e[0] == 61 or e[0] == 107 or e[0] == 111: # 61 == Connection refused and 107+111 == Transport endpoint is not connected
      if (HOST == 'localhost' or HOST == '127.0.0.1') and not connect_to_simulator:
        cmd = [ADB, 'forward', 'tcp:'+str(PORT), 'localfilesystem:/data/local/debugger-socket']
        print 'Connection to ' + HOST + ':' + str(PORT) + ' refused, attempting to forward device debugger-socket to local address by calling ' + str(cmd) + ':'
      else:
        print 'Error! Failed to connect to B2G ' + ('simulator' if connect_to_simulator else 'device') + ' debugger socket at address ' + HOST + ':' + str(PORT) + '!'
        sys.exit(1)
      try:
        retcode = subprocess.check_call(cmd)
      except Exception, e:
        print 'Error! Failed to execute adb: ' + str(e)
        print "Check that the device is connected properly, call 'adb devices' to list the detected devices."
        sys.exit(1)
      if retcode is not 0:
        print 'Error! Failed to connect to B2G device and executing adb failed with return code ' + retcode + '!'
        sys.exit(1)
      time.sleep(3)
      # Try again:
      try:
        b2g_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        b2g_socket.connect((HOST, PORT))
      except Exception, e:
        print 'Error! Failed to connect to B2G device debugger socket at address ' + HOST + ':' + str(PORT) + '!'
        sys.exit(1)

  handshake = read_b2g_response()
  logv('Connected. Handshake: ' + str(handshake))

  data = send_b2g_cmd('root', 'listTabs')
  if not 'deviceActor' in data:
    print 'Error! Debugging connection was not available. Make sure that the "Remote debugging" developer option on the device is set to "ADB and Devtools".'
    sys.exit(1)
  deviceActorName = data['deviceActor']
  logv('deviceActor: ' + deviceActorName)
  webappsActorName = data['webappsActor']
  logv('webappsActor: ' + webappsActorName)

  send_b2g_cmd(deviceActorName, 'getDescription')
  send_b2g_cmd(deviceActorName, 'getRawPermissionsTable')

  if sys.argv[1] == 'list':
    apps = b2g_get_appslist()
    running_app_manifests = b2g_get_runningapps()
    printed_apps = apps
    print_only_running = '--running' in sys.argv and not '--all' in sys.argv
    if print_only_running: # Print running apps only?
      print 'Running applications by id:'
      printed_apps = filter(lambda x: x['manifestURL'] in running_app_manifests, apps)
    else:
      print 'Installed applications by id:'
    num_printed = print_applist(printed_apps, running_app_manifests, '--all' in sys.argv or print_only_running)
    if num_printed == 0:
      if print_only_running:
        print '   No applications running.'
      else:
        print '   No applications installed.'
    if not '--all' in sys.argv and not print_only_running:
      print 'Not showing built-in apps that cannot be uninstalled. Pass --all to include those in the listing.'
  elif sys.argv[1] == 'launch' or sys.argv[1] == 'close' or sys.argv[1] == 'uninstall' or sys.argv[1] == 'getAppActor':
    if len(sys.argv) < 3:
      print 'Error! No application name given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <app>'
      return 1
    ret = b2g_app_command(sys.argv[1], sys.argv[2])
    if ret == 0 and '--log' in sys.argv:
      b2g_log(sys.argv[2])
  elif sys.argv[1] == 'install':
    if len(sys.argv) < 3:
      print 'Error! No application path given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <path>'
      return 1
    target_app_path = sys.argv[2]
    # Kill and uninstall old running app execution before starting.
    if '--run' in sys.argv:
      app_manifest = get_packaged_app_manifest(target_app_path)
      b2g_app_command('close', app_manifest['name'], print_errors_to_console=False)
      b2g_app_command('uninstall', app_manifest['name'], print_errors_to_console=False)
    # Upload package
    app_id = b2g_install(target_app_path)
    # Launch it immediately if requested.
    if '--run' in sys.argv:
      b2g_app_command('launch', app_id)
    # Don't quit, but keep logging the app if requested.
    if '--log' in sys.argv:
      b2g_log(app_id)
  elif sys.argv[1] == 'navigate':
    if len(sys.argv) < 3:
      print 'Error! No URL given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <url>'
      return 1
    browserActor = ''
    for app in apps:
      if app['name'] == 'Browser':
        browserActor = send_b2g_cmd(webappsActorName, 'getAppActor', { 'manifestURL': app['manifestURL'] })
        break
    if 'actor' in browserActor:
      browserActor = browserActor['actor']['actor']
      send_b2g_cmd(browserActor, 'navigateTo', { 'url': sys.argv[2]})
    else:
      print 'Web browser is not running!'
  elif sys.argv[1] == 'log':
    if len(sys.argv) < 3:
      print 'Error! No application name given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <app>'
      return 1
    clear = '-c' in sys.argv or '-clear' in sys.argv or '--clear' in sys.argv
    b2g_log(sys.argv[2], clear)
  elif sys.argv[1] == 'memory':
    b2g_memory(sys.argv[2])
  elif sys.argv[1] == 'screenshot':
    if len(sys.argv) >= 3:
      filename = sys.argv[2]
      if not filename.endswith('.png'):
        print >> sys.stderr, "Writing screenshots only to .png files are supported!"
        sys.exit(1)
    else:
      filename = time.strftime("screen_%Y%m%d_%H%M%S.png", time.gmtime())

    b2g_screenshot(filename)
  elif sys.argv[1] == 'get':
    b2g_get_pref(sys.argv[2] if len(sys.argv) >= 3 else None)
  elif sys.argv[1] == 'set':
    if len(sys.argv) < 3:
      print 'Error! No pref name to set given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <pref> <value>'
      sys.exit(1)
    if len(sys.argv) < 4:
      print 'Error! No value given to set! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <pref> <value>'
      sys.exit(1)
    if len(sys.argv) > 4:
      print 'Error! Too many arguments given (' + str(sys.argv) + '), need exactly four! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <pref> <value>'
      sys.exit(1)
    b2g_set_pref(sys.argv[2], sys.argv[3])
  elif sys.argv[1] == 'unset':
    if len(sys.argv) < 3:
      print 'Error! No pref name given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <pref>'
      sys.exit(1)
    b2g_set_pref(sys.argv[2], None)
  elif sys.argv[1] == 'hide-prompt':
    b2g_set_pref('devtools.debugger.prompt-connection', 'false')
  elif sys.argv[1] == 'restore-prompt':
    b2g_set_pref('devtools.debugger.prompt-connection', None)
  elif sys.argv[1] == 'desc':
    b2g_get_description(sys.argv[2] if len(sys.argv) >= 3 else None)
  else:
    print "Unknown command '" + sys.argv[1] + "'! Pass --help for instructions."

  b2g_socket.close()
  return 0

if __name__ == '__main__':
  try:
    returncode = main()
    logv('ffdb.py quitting with process exit code ' + str(returncode))
    sys.exit(returncode)
  except KeyboardInterrupt:
    print ('^C' if WINDOWS else '') + ' Aborted by user'
    sys.exit(1)
