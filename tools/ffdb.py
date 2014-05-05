#!/usr/bin/env python

import socket, json, sys, uuid, datetime, time, logging, cgi, zipfile, os, tempfile, atexit, subprocess

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
  zipf = zipfile.ZipFile(zipfilename, 'w')
  files_to_compress = []
  for root, dirs, files in os.walk(path):
    for file in files:
      files_to_compress += [(root, file)]

  n = 1
  for tuple in files_to_compress:
    (root, file) = tuple
    filename = os.path.join(root, file)
    filesize = os.path.getsize(filename)
    print 'Compressing ' + str(n) + '/' + str(len(files_to_compress)) + ': "' + os.path.relpath(filename, path) + '" (' + sizeof_fmt(filesize) + ')...'
    n += 1
    zipf.write(os.path.join(root, file))
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
    sys.stdout.write(format_html(msg))
    sys.stdout.flush()

# Reads data from the socket, and tries to parse what we have got so far as a JSON message.
# The messages are of form "bytelength:{jsondict}", where bytelength tells how many bytes
# there are in the data that comes after the colon.
# Returns a JSON dictionary of the received message.
def read_b2g_response():
  global read_queue, b2g_socket
  read_queue += b2g_socket.recv(65536*2)
  while ':' in read_queue:
    semicolon = read_queue.index(':')
    payload_len = int(read_queue[:semicolon])
    if semicolon+1+payload_len > len(read_queue):
      read_queue += b2g_socket.recv(65536*2)
      continue
    payload = read_queue[semicolon+1:semicolon+1+payload_len]
    read_queue = read_queue[semicolon+1+payload_len:]
    logv('Read a message of size ' + str(payload_len) + 'b from socket.')
    payload = json.loads(payload)
  return payload

# Sends a command to the B2G device and waits for the response and returns it as a JSON dict.
def send_b2g_cmd(to, cmd, data = {}):
  global b2g_socket
  msg = { 'to': to, 'type': cmd}
  msg = dict(msg.items() + data.items())
  msg = json.dumps(msg, encoding='latin-1')
  msg = msg.replace('\\\\', '\\')
  msg = str(len(msg))+':'+msg
  logv('Sending cmd:' + cmd + ' to:' + to)
  b2g_socket.sendall(msg)
  return read_b2g_response()

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
  b2g_socket.sendall(message)

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

def main():
  global b2g_socket, webappsActorName
  if len(sys.argv) < 2 or '--help' in sys.argv or 'help' in sys.argv or '-v' in sys.argv:
    print '''Firefox OS Debug Bridge, a tool for automating FFOS device tasks from the command line.

    Usage: ffdb.py <command>, where command is one of:

    list [--running] [--all]: Prints out the user applications installed on the device.
                              If --running is passed, only the currently opened apps are shown.
                              If --all is specified, then also uninstallable system applications are listed.
    launch <app>: Starts the given application. If already running, brings to front.
    close <app>: Terminates the execution of the given application.
    uninstall <app>: Removes the given application from the device.
    install <path>: Uploads and installs a packaged app that resides in the given local directory.
                    <path> may either refer to a directory containing a packaged app, or to a prepackaged zip file.
    log <app> [--clear]: Starts a persistent log listener that reads web console messages from the given application.
                         If --clear is passed, the message log for that application is cleared instead.
    navigate <url>: Opens the given web page in the B2G browser.

  In the above, whenever a command requires an <app> to be specified, either the human-readable name, 
  localId or manifestURL of the application can be used.'''

    sys.exit(0)

  b2g_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  try:
    b2g_socket.connect((HOST, PORT))
  except Exception, e:
    if e[0] == 61: # Connection refused
      if HOST == 'localhost' or HOST == '127.0.0.1':
        cmd = ['adb', 'forward', 'tcp:'+str(PORT), 'localfilesystem:/data/local/debugger-socket']
        print 'Connection to ' + HOST + ':' + str(PORT) + ' refused, attempting to forward device debugger-socket to local address by calling ' + str(cmd) + ':'
      else:
        print 'Error! Failed to connect to B2G device debugger socket at address ' + HOST + ':' + str(PORT) + '!'
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
  deviceActorName = data['deviceActor']
  logv('deviceActor: ' + deviceActorName)
  webappsActorName = data['webappsActor']
  logv('webappsActor: ' + webappsActorName)

  send_b2g_cmd(deviceActorName, 'getDescription')
  send_b2g_cmd(deviceActorName, 'getRawPermissionsTable')

  apps = b2g_get_appslist()

  if sys.argv[1] == 'list':
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
    for app in apps:
      if str(app['localId']) == sys.argv[2] or app['name'] == sys.argv[2] or app['manifestURL'] == sys.argv[2]:
        send_b2g_cmd(webappsActorName, sys.argv[1], { 'manifestURL': app['manifestURL'] })
        return 0
    print 'Error! Application "' + sys.argv[2] + '" was not found! Use the \'list\' command to find installed applications.'
    return 1
  elif sys.argv[1] == 'install':
    if len(sys.argv) < 3:
      print 'Error! No application path given! Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' <path>'
      return 1
    target_app_path = sys.argv[2]
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
    uploadResponse = send_b2g_cmd(webappsActorName, 'uploadPackage')
    packageUploadActor = uploadResponse['actor']
    app_file = open(target_app_path, 'rb')
    data = app_file.read()
    file_size = len(data)
    chunk_size = 4*1024*1024
    i = 0
    start_time = time.time()
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
    send_b2g_cmd(webappsActorName, 'install', { 'appId': str(uuid.uuid4()), 'upload': packageUploadActor })

    cur_time = time.time()
    secs_elapsed = cur_time - start_time
    print 'Upload of ' + sizeof_fmt(file_size) + ' finished. Total time elapsed: ' + str(int(secs_elapsed)) + ' seconds. Data rate: {:5.2f} KB/second.'.format(file_size / 1024.0 / secs_elapsed)
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
    appActor = ''
    for app in apps:
      if str(app['localId']) == sys.argv[2] or app['name'] == sys.argv[2] or app['manifestURL'] == sys.argv[2]:
        appActor = send_b2g_cmd(webappsActorName, 'getAppActor', { 'manifestURL': app['manifestURL'] })
        break
    if 'actor' in appActor:
      consoleActor = appActor['actor']['consoleActor']

      if '-c' in sys.argv or '-clear' in sys.argv or '--clear' in sys.argv:
        send_b2g_cmd(consoleActor, 'clearMessagesCache')
        print 'Cleared message log.'
        sys.exit(0)

      msgs = send_b2g_cmd(consoleActor, 'startListeners', { 'listeners': ['PageError','ConsoleAPI','NetworkActivity','FileActivity'] })

      def log_b2g_message(msg):
        WARNING = '\033[93m'
        FAIL = '\033[91m'
        ENDC = '\033[0m'
        BOLD = "\033[1m"
        msgs = []
        if 'type' in msg and msg['type'] == 'consoleAPICall':
          msgs = [msg['message']]
        elif 'messages' in msg:
          msgs = msg['messages']

        for m in msgs:
          args = m['arguments']

          for arg in args:
            if m['level'] == 'log':
              color = 'I/'
            elif m['level'] == 'warn':
              color = WARNING + 'W/'
            elif m['level'] == 'error':
              color = FAIL + 'E/'
            else:
              color = m['level'] + '/'

            print color + str(m['functionName']) + '@' + str(m['filename']) + ':' + str(m['lineNumber']) + ': ' + str(arg) + ENDC

      msgs = send_b2g_cmd(consoleActor, 'getCachedMessages', { 'messageTypes': ['PageError', 'ConsoleAPI'] })
      log_b2g_message(msgs)

      while True:
        msg = read_b2g_response()
        log_b2g_message(msg)
    else:
      print 'Application "' + sys.argv[2] + '" is not running!'
  else:
    print "Unknown command '" + sys.argv[1] + "'! Pass --help for instructions."

  b2g_socket.close()
  return 0

if __name__ == '__main__':
  returncode = main()
  logv('ffdb.py quitting with process exit code ' + str(returncode))
  sys.exit(returncode)
