import shutil, time, os, sys, json, tempfile, copy, shlex, atexit, subprocess, hashlib, cPickle, re, errno
from subprocess import Popen, PIPE, STDOUT
from tempfile import mkstemp
from distutils.spawn import find_executable
import jsrun, cache, tempfiles
import response_file
import logging, platform, multiprocessing

# Temp file utilities
from tempfiles import try_delete

# On Windows python suffers from a particularly nasty bug if python is spawning new processes while python itself is spawned from some other non-console process.
# Use a custom replacement for Popen on Windows to avoid the "WindowsError: [Error 6] The handle is invalid" errors when emcc is driven through cmake or mingw32-make. 
# See http://bugs.python.org/issue3905
class WindowsPopen:
  def __init__(self, args, bufsize=0, executable=None, stdin=None, stdout=None, stderr=None, preexec_fn=None, close_fds=False, 
               shell=False, cwd=None, env=None, universal_newlines=False, startupinfo=None, creationflags=0):
    self.stdin = stdin
    self.stdout = stdout
    self.stderr = stderr
    
    # (stdin, stdout, stderr) store what the caller originally wanted to be done with the streams.
    # (stdin_, stdout_, stderr_) will store the fixed set of streams that workaround the bug.
    self.stdin_ = stdin
    self.stdout_ = stdout
    self.stderr_ = stderr
    
    # If the caller wants one of these PIPEd, we must PIPE them all to avoid the 'handle is invalid' bug.
    if self.stdin_ == PIPE or self.stdout_ == PIPE or self.stderr_ == PIPE:
      if self.stdin_ == None:
        self.stdin_ = PIPE
      if self.stdout_ == None:
        self.stdout_ = PIPE
      if self.stderr_ == None:
        self.stderr_ = PIPE

    # emscripten.py supports reading args from a response file instead of cmdline.
    # Use .rsp to avoid cmdline length limitations on Windows.
    if len(args) >= 2 and args[1].endswith("emscripten.py"):
      response_filename = response_file.create_response_file(args[2:], TEMP_DIR)
      args = args[0:2] + ['@' + response_filename]
      
    try:
      # Call the process with fixed streams.
      self.process = subprocess.Popen(args, bufsize, executable, self.stdin_, self.stdout_, self.stderr_, preexec_fn, close_fds, shell, cwd, env, universal_newlines, startupinfo, creationflags)
      self.pid = self.process.pid
    except Exception, e:
      logging.error('\nsubprocess.Popen(args=%s) failed! Exception %s\n' % (' '.join(args), str(e)))
      raise e

  def communicate(self, input=None):
    output = self.process.communicate(input)
    self.returncode = self.process.returncode

    # If caller never wanted to PIPE stdout or stderr, route the output back to screen to avoid swallowing output.
    if self.stdout == None and self.stdout_ == PIPE and len(output[0].strip()) > 0:
      print >> sys.stdout, output[0]
    if self.stderr == None and self.stderr_ == PIPE and len(output[1].strip()) > 0:
      print >> sys.stderr, output[1]

    # Return a mock object to the caller. This works as long as all emscripten code immediately .communicate()s the result, and doesn't
    # leave the process object around for longer/more exotic uses.
    if self.stdout == None and self.stderr == None:
      return (None, None)
    if self.stdout == None:
      return (None, output[1])
    if self.stderr == None:
      return (output[0], None)
    return (output[0], output[1])

  def poll(self):
    return self.process.poll()

  def kill(self):
    return self.process.kill()
  
__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

def add_coloring_to_emit_windows(fn):
  def _out_handle(self):
    import ctypes
    return ctypes.windll.kernel32.GetStdHandle(self.STD_OUTPUT_HANDLE)
  out_handle = property(_out_handle)

  def _set_color(self, code):
    import ctypes
    # Constants from the Windows API
    self.STD_OUTPUT_HANDLE = -11
    hdl = ctypes.windll.kernel32.GetStdHandle(self.STD_OUTPUT_HANDLE)
    ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, code)

  setattr(logging.StreamHandler, '_set_color', _set_color)

  def new(*args):
    FOREGROUND_BLUE      = 0x0001 # text color contains blue.
    FOREGROUND_GREEN     = 0x0002 # text color contains green.
    FOREGROUND_RED       = 0x0004 # text color contains red.
    FOREGROUND_INTENSITY = 0x0008 # text color is intensified.
    FOREGROUND_WHITE     = FOREGROUND_BLUE|FOREGROUND_GREEN |FOREGROUND_RED
    # winbase.h
    STD_INPUT_HANDLE = -10
    STD_OUTPUT_HANDLE = -11
    STD_ERROR_HANDLE = -12

    # wincon.h
    FOREGROUND_BLACK     = 0x0000
    FOREGROUND_BLUE      = 0x0001
    FOREGROUND_GREEN     = 0x0002
    FOREGROUND_CYAN      = 0x0003
    FOREGROUND_RED       = 0x0004
    FOREGROUND_MAGENTA   = 0x0005
    FOREGROUND_YELLOW    = 0x0006
    FOREGROUND_GREY      = 0x0007
    FOREGROUND_INTENSITY = 0x0008 # foreground color is intensified.

    BACKGROUND_BLACK     = 0x0000
    BACKGROUND_BLUE      = 0x0010
    BACKGROUND_GREEN     = 0x0020
    BACKGROUND_CYAN      = 0x0030
    BACKGROUND_RED       = 0x0040
    BACKGROUND_MAGENTA   = 0x0050
    BACKGROUND_YELLOW    = 0x0060
    BACKGROUND_GREY      = 0x0070
    BACKGROUND_INTENSITY = 0x0080 # background color is intensified.
    levelno = args[1].levelno
    if(levelno >= 50):
        color = BACKGROUND_YELLOW | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY
    elif(levelno >= 40):
        color = FOREGROUND_RED | FOREGROUND_INTENSITY
    elif(levelno >= 30):
        color = FOREGROUND_YELLOW | FOREGROUND_INTENSITY
    elif(levelno >= 20):
        color = FOREGROUND_GREEN
    elif(levelno >= 10):
        color = FOREGROUND_MAGENTA
    else:
        color =  FOREGROUND_WHITE
    args[0]._set_color(color)
    ret = fn(*args)
    args[0]._set_color( FOREGROUND_WHITE )
    #print "after"
    return ret
  return new

def add_coloring_to_emit_ansi(fn):
  # add methods we need to the class
  def new(*args):
    levelno = args[1].levelno
    if(levelno >= 50):
      color = '\x1b[31m' # red
    elif(levelno >= 40):
      color = '\x1b[31m' # red
    elif(levelno >= 30):
      color = '\x1b[33m' # yellow
    elif(levelno >= 20):
      color = '\x1b[32m' # green
    elif(levelno >= 10):
      color = '\x1b[35m' # pink
    else:
      color = '\x1b[0m' # normal
    args[1].msg = color + args[1].msg +  '\x1b[0m'  # normal
    #print "after"
    return fn(*args)
  return new

WINDOWS = sys.platform.startswith('win')

if WINDOWS:
  logging.StreamHandler.emit = add_coloring_to_emit_windows(logging.StreamHandler.emit)
else:
  logging.StreamHandler.emit = add_coloring_to_emit_ansi(logging.StreamHandler.emit)

# Emscripten configuration is done through the --em-config command line option or
# the EM_CONFIG environment variable. If the specified string value contains newline
# or semicolon-separated definitions, then these definitions will be used to configure
# Emscripten.  Otherwise, the string is understood to be a path to a settings
# file that contains the required definitions.

try:
  EM_CONFIG = sys.argv[sys.argv.index('--em-config')+1]
  # And now remove it from sys.argv
  skip = False
  newargs = []
  for arg in sys.argv:
    if not skip and arg != '--em-config':
      newargs += [arg]
    elif arg == '--em-config':
      skip = True
    elif skip:
      skip = False
  sys.argv = newargs
  # Emscripten compiler spawns other processes, which can reimport shared.py, so make sure that
  # those child processes get the same configuration file by setting it to the currently active environment.
  os.environ['EM_CONFIG'] = EM_CONFIG
except:
  EM_CONFIG = os.environ.get('EM_CONFIG')

if EM_CONFIG and not os.path.isfile(EM_CONFIG):
  if EM_CONFIG.startswith('-'):
    raise Exception('Passed --em-config without an argument. Usage: --em-config /path/to/.emscripten or --em-config EMSCRIPTEN_ROOT=/path/;LLVM_ROOT=/path;...')
  if not '=' in EM_CONFIG:
    raise Exception('File ' + EM_CONFIG + ' passed to --em-config does not exist!')
  else:
    EM_CONFIG = EM_CONFIG.replace(';', '\n') + '\n'

if not EM_CONFIG:
  EM_CONFIG = '~/.emscripten'
if '\n' in EM_CONFIG:
  CONFIG_FILE = None
else:
  CONFIG_FILE = os.path.expanduser(EM_CONFIG)
  if not os.path.exists(CONFIG_FILE):
    # Note: repr is used to ensure the paths are escaped correctly on Windows.
    # The full string is replaced so that the template stays valid Python.
    config_file = open(path_from_root('tools', 'settings_template_readonly.py')).read().split('\n')
    config_file = config_file[1:] # remove "this file will be copied..."
    config_file = '\n'.join(config_file)
    # autodetect some default paths
    config_file = config_file.replace('\'{{{ EMSCRIPTEN_ROOT }}}\'', repr(__rootpath__))
    llvm_root = os.path.dirname(find_executable('llvm-dis') or '/usr/bin/llvm-dis')
    config_file = config_file.replace('\'{{{ LLVM_ROOT }}}\'', repr(llvm_root))
    node = find_executable('node') or find_executable('nodejs') or 'node'
    config_file = config_file.replace('\'{{{ NODE }}}\'', repr(node))
    if WINDOWS:
      tempdir = os.environ.get('TEMP') or os.environ.get('TMP') or 'c:\\temp'
    else:
      tempdir = '/tmp'
    config_file = config_file.replace('\'{{{ TEMP }}}\'', repr(tempdir))

    # write
    open(CONFIG_FILE, 'w').write(config_file)
    print >> sys.stderr, '''
==============================================================================
Welcome to Emscripten!

This is the first time any of the Emscripten tools has been run.

A settings file has been copied to %s, at absolute path: %s

It contains our best guesses for the important paths, which are:

  LLVM_ROOT       = %s
  NODE_JS         = %s
  EMSCRIPTEN_ROOT = %s

Please edit the file if any of those are incorrect.

This command will now exit. When you are done editing those paths, re-run it.
==============================================================================
''' % (EM_CONFIG, CONFIG_FILE, llvm_root, node, __rootpath__)
    sys.exit(0)

try:
  config_text = open(CONFIG_FILE, 'r').read() if CONFIG_FILE else EM_CONFIG
  exec(config_text)
except Exception, e:
  logging.error('Error in evaluating %s (at %s): %s, text: %s' % (EM_CONFIG, CONFIG_FILE, str(e), config_text))
  sys.exit(1)

def listify(x):
  if type(x) is not list: return [x]
  return x

def fix_js_engine(old, new):
  global JS_ENGINES
  JS_ENGINES = map(lambda x: new if x == old else x, JS_ENGINES)
  return new

try:
  SPIDERMONKEY_ENGINE = fix_js_engine(SPIDERMONKEY_ENGINE, listify(SPIDERMONKEY_ENGINE))
except:
  pass
try:
  NODE_JS = fix_js_engine(NODE_JS, listify(NODE_JS))
except:
  pass
try:
  V8_ENGINE = fix_js_engine(V8_ENGINE, listify(V8_ENGINE))
except:
  pass

COMPILER_ENGINE = listify(COMPILER_ENGINE)
JS_ENGINES = [listify(ENGINE) for ENGINE in JS_ENGINES]

try:
  EM_POPEN_WORKAROUND
except:
  EM_POPEN_WORKAROUND = os.environ.get('EM_POPEN_WORKAROUND')

# Install our replacement Popen handler if we are running on Windows to avoid python spawn process function.
# nb. This is by default disabled since it has the adverse effect of buffering up all logging messages, which makes
# builds look unresponsive (messages are printed only after the whole build finishes). Whether this workaround is needed 
# seems to depend on how the host application that invokes emcc has set up its stdout and stderr.
if EM_POPEN_WORKAROUND and os.name == 'nt':
  logging.debug('Installing Popen workaround handler to avoid bug http://bugs.python.org/issue3905')
  Popen = WindowsPopen

# Verbosity level control for any intermediate subprocess spawns from the compiler. Useful for internal debugging.
# 0: disabled.
# 1: Log stderr of subprocess spawns.
# 2: Log stdout and stderr of subprocess spawns. Print out subprocess commands that were executed.
# 3: Log stdout and stderr, and pass VERBOSE=1 to CMake configure steps.
EM_BUILD_VERBOSE_LEVEL = int(os.getenv('EM_BUILD_VERBOSE')) if os.getenv('EM_BUILD_VERBOSE') != None else 0

# Expectations

EXPECTED_LLVM_VERSION = (3,5)

actual_clang_version = None

def get_clang_version():
  global actual_clang_version
  if actual_clang_version is None:
    response = Popen([CLANG, '-v'], stderr=PIPE).communicate()[1]
    m = re.search(r'[Vv]ersion\s+(\d+\.\d+)', response)
    actual_clang_version = m and m.group(1)
  return actual_clang_version

def check_clang_version():
  expected = '.'.join(map(str, EXPECTED_LLVM_VERSION))
  actual = get_clang_version()
  if expected in actual:
    return True
  logging.warning('LLVM version appears incorrect (seeing "%s", expected "%s")' % (actual, expected))
  return False

def check_llvm_version():
  try:
    check_clang_version()
  except Exception, e:
    logging.warning('Could not verify LLVM version: %s' % str(e))

# look for emscripten-version.txt files under or alongside the llvm source dir
def get_fastcomp_src_dir():
  d = LLVM_ROOT
  emroot = path_from_root() # already abspath
  # look for version file in llvm repo, making sure not to mistake the emscripten repo for it
  while d != os.path.dirname(d):
    d = os.path.abspath(d)
    # when the build directory lives below the source directory
    if os.path.exists(os.path.join(d, 'emscripten-version.txt')) and not d == emroot:
      return d
    # when the build directory lives alongside the source directory
    elif os.path.exists(os.path.join(d, 'src', 'emscripten-version.txt')) and not os.path.join(d, 'src') == emroot:
      return os.path.join(d, 'src')
    else:
      d = os.path.dirname(d)
  return None

def check_fastcomp():
  try:
    llc_version_info = Popen([LLVM_COMPILER, '--version'], stdout=PIPE).communicate()[0]
    pre, targets = llc_version_info.split('Registered Targets:')
    if 'js' not in targets or 'JavaScript (asm.js, emscripten) backend' not in targets:
      logging.critical('fastcomp in use, but LLVM has not been built with the JavaScript backend as a target, llc reports:')
      print >> sys.stderr, '==========================================================================='
      print >> sys.stderr, llc_version_info,
      print >> sys.stderr, '==========================================================================='
      logging.critical('you can fall back to the older (pre-fastcomp) compiler core, although that is not recommended, see https://github.com/kripken/emscripten/wiki/LLVM-Backend')
      return False

    d = get_fastcomp_src_dir()
    if d is not None:
      llvm_version = open(os.path.join(d, 'emscripten-version.txt')).read().strip()
      if os.path.exists(os.path.join(d, 'tools', 'clang', 'emscripten-version.txt')):
        clang_version = open(os.path.join(d, 'tools', 'clang', 'emscripten-version.txt')).read().strip()
      elif os.path.exists(os.path.join(d, 'tools', 'clang')):
        clang_version = '?' # Looks like the LLVM compiler tree has an old checkout from the time before it contained a version.txt: Should update!
      else:
        clang_version = llvm_version # This LLVM compiler tree does not have a tools/clang, so it's probably an out-of-source build directory. No need for separate versioning.
      if EMSCRIPTEN_VERSION != llvm_version or EMSCRIPTEN_VERSION != clang_version:
        logging.error('Emscripten, llvm and clang versions do not match, this is dangerous (%s, %s, %s)', EMSCRIPTEN_VERSION, llvm_version, clang_version)
        logging.error('Make sure to use the same branch in each repo, and to be up-to-date on each. See https://github.com/kripken/emscripten/wiki/LLVM-Backend')
    else:
      logging.warning('did not see a source tree above or next to the LLVM root directory (guessing based on directory of %s), could not verify version numbers match' % LLVM_COMPILER)
    return True
  except Exception, e:
    logging.warning('could not check fastcomp: %s' % str(e))
    return True

EXPECTED_NODE_VERSION = (0,8,0)

def check_node_version():
  try:
    actual = Popen(NODE_JS + ['--version'], stdout=PIPE).communicate()[0].strip()
    version = tuple(map(int, actual.replace('v', '').replace('-pre', '').split('.')))
    if version >= EXPECTED_NODE_VERSION:
      return True
    logging.warning('node version appears too old (seeing "%s", expected "%s")' % (actual, 'v' + ('.'.join(map(str, EXPECTED_NODE_VERSION)))))
    return False
  except Exception, e:
    logging.warning('cannot check node version: %s',  e)
    return False

# Finds the system temp directory without resorting to using the one configured in .emscripten
def find_temp_directory():
  if WINDOWS:
    if os.getenv('TEMP') and os.path.isdir(os.getenv('TEMP')):
      return os.getenv('TEMP')
    elif os.getenv('TMP') and os.path.isdir(os.getenv('TMP')):
      return os.getenv('TMP')
    elif os.path.isdir('C:\\temp'):
      return os.getenv('C:\\temp')
    else:
      return None # No luck!
  else:
    return '/tmp'

# Check that basic stuff we need (a JS engine to compile, Node.js, and Clang and LLVM)
# exists.
# The test runner always does this check (through |force|). emcc does this less frequently,
# only when ${EM_CONFIG}_sanity does not exist or is older than EM_CONFIG (so,
# we re-check sanity when the settings are changed)
# We also re-check sanity and clear the cache when the version changes

try:
  EMSCRIPTEN_VERSION = open(path_from_root('emscripten-version.txt')).read().strip()
  try:
    parts = map(int, EMSCRIPTEN_VERSION.split('.'))
    EMSCRIPTEN_VERSION_MAJOR = parts[0]
    EMSCRIPTEN_VERSION_MINOR = parts[1]
    EMSCRIPTEN_VERSION_TINY = parts[2]
  except Exception, e:
    logging.warning('emscripten version ' + EMSCRIPTEN_VERSION + ' lacks standard parts')
    EMSCRIPTEN_VERSION_MAJOR = 0
    EMSCRIPTEN_VERSION_MINOR = 0
    EMSCRIPTEN_VERSION_TINY = 0
    raise e
except Exception, e:
  logging.error('cannot find emscripten version ' + str(e))
  EMSCRIPTEN_VERSION = 'unknown'

def generate_sanity():
  return EMSCRIPTEN_VERSION + '|' + get_llvm_target() + '|' + LLVM_ROOT + '|' + get_clang_version()

def check_sanity(force=False):
  try:
    if os.environ.get('EMCC_SKIP_SANITY_CHECK') == '1':
      return
    reason = None
    if not CONFIG_FILE:
      return # config stored directly in EM_CONFIG => skip sanity checks
    else:
      settings_mtime = os.stat(CONFIG_FILE).st_mtime
      sanity_file = CONFIG_FILE + '_sanity'
      if os.path.exists(sanity_file):
        try:
          sanity_mtime = os.stat(sanity_file).st_mtime
          if sanity_mtime <= settings_mtime:
            reason = 'settings file has changed'
          else:
            sanity_data = open(sanity_file).read()
            if sanity_data != generate_sanity():
              reason = 'system change: %s vs %s' % (generate_sanity(), sanity_data)
            else:
              if not force: return # all is well
        except Exception, e:
          reason = 'unknown: ' + str(e)
    if reason:
      logging.warning('(Emscripten: %s, clearing cache)' % reason)
      Cache.erase()
      force = False # the check actually failed, so definitely write out the sanity file, to avoid others later seeing failures too

    # some warning, mostly not fatal checks - do them even if EM_IGNORE_SANITY is on
    check_llvm_version()
    check_node_version()

    if os.environ.get('EMCC_FAST_COMPILER') == '0':
      logging.critical('Non-fastcomp compiler is no longer available, please use fastcomp or an older version of emscripten')
      sys.exit(1)

    fastcomp_ok = check_fastcomp()

    if os.environ.get('EM_IGNORE_SANITY'):
      logging.info('EM_IGNORE_SANITY set, ignoring sanity checks')
      return

    logging.info('(Emscripten: Running sanity checks)')

    if not check_engine(COMPILER_ENGINE):
      logging.critical('The JavaScript shell used for compiling (%s) does not seem to work, check the paths in %s' % (COMPILER_ENGINE, EM_CONFIG))
      sys.exit(1)

    if NODE_JS != COMPILER_ENGINE:
      if not check_engine(NODE_JS):
        logging.critical('Node.js (%s) does not seem to work, check the paths in %s' % (NODE_JS, EM_CONFIG))
        sys.exit(1)

    for cmd in [CLANG, LINK_CMD[0], LLVM_AR, LLVM_OPT, LLVM_AS, LLVM_DIS, LLVM_NM, LLVM_INTERPRETER]:
      if not os.path.exists(cmd) and not os.path.exists(cmd + '.exe'): # .exe extension required for Windows
        logging.critical('Cannot find %s, check the paths in %s' % (cmd, EM_CONFIG))
        sys.exit(1)

    if os.environ.get('EMCC_FAST_COMPILER') != '0':
      if not fastcomp_ok:
        logging.critical('failing sanity checks due to previous fastcomp failure')
        sys.exit(1)

    try:
      subprocess.call([JAVA, '-version'], stdout=PIPE, stderr=PIPE)
    except:
      logging.warning('java does not seem to exist, required for closure compiler, which is optional (define JAVA in ~/.emscripten if you want it)')

    if not os.path.exists(CLOSURE_COMPILER):
     logging.warning('Closure compiler (%s) does not exist, check the paths in %s. -O2 and above will fail' % (CLOSURE_COMPILER, EM_CONFIG))

    # Sanity check passed!

    if not force:
      # Only create/update this file if the sanity check succeeded, i.e., we got here
      f = open(sanity_file, 'w')
      f.write(generate_sanity())
      f.close()

  except Exception, e:
    # Any error here is not worth failing on
    print 'WARNING: sanity check failed to run', e

# Tools/paths

try:
	LLVM_ADD_VERSION
except NameError:
	LLVM_ADD_VERSION = os.getenv('LLVM_ADD_VERSION')

try:
	CLANG_ADD_VERSION
except NameError:
	CLANG_ADD_VERSION = os.getenv('CLANG_ADD_VERSION')

USING_PNACL_TOOLCHAIN = os.path.exists(os.path.join(LLVM_ROOT, 'pnacl-clang'))

def modify_prefix(tool):
  if USING_PNACL_TOOLCHAIN:
    if tool.startswith('llvm-'):
      tool = tool[5:]
    tool = 'pnacl-' + tool
    if WINDOWS:
      tool += '.bat'
  return tool

# Some distributions ship with multiple llvm versions so they add
# the version to the binaries, cope with that
def build_llvm_tool_path(tool):
  tool = modify_prefix(tool)
  if LLVM_ADD_VERSION:
    return os.path.join(LLVM_ROOT, tool + "-" + LLVM_ADD_VERSION)
  else:
    return os.path.join(LLVM_ROOT, tool)

# Some distributions ship with multiple clang versions so they add
# the version to the binaries, cope with that
def build_clang_tool_path(tool):
  tool = modify_prefix(tool)
  if CLANG_ADD_VERSION:
    return os.path.join(LLVM_ROOT, tool + "-" + CLANG_ADD_VERSION)
  else:
    return os.path.join(LLVM_ROOT, tool)

# Whenever building a native executable for OSX, we must provide the OSX SDK version we want to target.
def osx_find_native_sdk_path():
  try:
    sdk_root = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs'
    sdks = os.walk(sdk_root).next()[1]
    sdk_path = os.path.join(sdk_root, sdks[0]) # Just pick first one found, we don't care which one we found.
    logging.debug('Targeting OSX SDK found at ' + sdk_path)
    return sdk_path
  except:
    logging.warning('Could not find native OSX SDK path to target!')
    return None

# These extra args need to be passed to Clang when targeting a native host system executable
CACHED_CLANG_NATIVE_ARGS=None
def get_clang_native_args():
  global CACHED_CLANG_NATIVE_ARGS
  if CACHED_CLANG_NATIVE_ARGS is not None: return CACHED_CLANG_NATIVE_ARGS
  CACHED_CLANG_NATIVE_ARGS = []
  if sys.platform == 'darwin':
    sdk_path = osx_find_native_sdk_path()
    if sdk_path:
      CACHED_CLANG_NATIVE_ARGS = ['-isysroot', osx_find_native_sdk_path()]
  elif os.name == 'nt':
    CACHED_CLANG_NATIVE_ARGS = ['-DWIN32']
    # TODO: If Windows.h et al. are needed, will need to add something like '-isystemC:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Include'.
  return CACHED_CLANG_NATIVE_ARGS

CLANG_CC=os.path.expanduser(build_clang_tool_path('clang'))
CLANG_CPP=os.path.expanduser(build_clang_tool_path('clang++'))
CLANG=CLANG_CPP
if USING_PNACL_TOOLCHAIN:
  # The PNaCl toolchain doesn't have llvm-link, but we can fake it
  LINK_CMD = [build_llvm_tool_path('llvm-ld'), '-nostdlib', '-r']
else:
  LINK_CMD = [build_llvm_tool_path('llvm-link')]
LLVM_AR=build_llvm_tool_path('llvm-ar')
LLVM_OPT=os.path.expanduser(build_llvm_tool_path('opt'))
LLVM_AS=os.path.expanduser(build_llvm_tool_path('llvm-as'))
LLVM_DIS=os.path.expanduser(build_llvm_tool_path('llvm-dis'))
LLVM_NM=os.path.expanduser(build_llvm_tool_path('llvm-nm'))
LLVM_INTERPRETER=os.path.expanduser(build_llvm_tool_path('lli'))
LLVM_COMPILER=os.path.expanduser(build_llvm_tool_path('llc'))

EMSCRIPTEN = path_from_root('emscripten.py')
DEMANGLER = path_from_root('third_party', 'demangler.py')
NAMESPACER = path_from_root('tools', 'namespacer.py')
EMCC = path_from_root('emcc')
EMXX = path_from_root('em++')
EMAR = path_from_root('emar')
EMRANLIB = path_from_root('emranlib')
EMCONFIG = path_from_root('em-config')
EMLINK = path_from_root('emlink.py')
EMMAKEN = path_from_root('tools', 'emmaken.py')
AUTODEBUGGER = path_from_root('tools', 'autodebugger.py')
BINDINGS_GENERATOR = path_from_root('tools', 'bindings_generator.py')
EXEC_LLVM = path_from_root('tools', 'exec_llvm.py')
FILE_PACKAGER = path_from_root('tools', 'file_packager.py')

# Temp dir. Create a random one, unless EMCC_DEBUG is set, in which case use TEMP_DIR/emscripten_temp

def safe_ensure_dirs(dirname):
  try:
    os.makedirs(dirname)
  except os.error, e:
    # Ignore error for already existing dirname
    if e.errno != errno.EEXIST:
      raise e
    # FIXME: Notice that this will result in a false positive,
    # should the dirname be a file! There seems to no way to
    # handle this atomically in Python 2.x.
    # There is an additional option for Python 3.x, though.

# Returns a path to EMSCRIPTEN_TEMP_DIR, creating one if it didn't exist.
def get_emscripten_temp_dir():
  global configuration, EMSCRIPTEN_TEMP_DIR
  if not EMSCRIPTEN_TEMP_DIR:
    EMSCRIPTEN_TEMP_DIR = tempfile.mkdtemp(prefix='emscripten_temp_', dir=configuration.TEMP_DIR)
    def prepare_to_clean_temp(d):
      def clean_temp():
        try_delete(d)
      atexit.register(clean_temp)
    prepare_to_clean_temp(EMSCRIPTEN_TEMP_DIR) # this global var might change later
  return EMSCRIPTEN_TEMP_DIR

class Configuration:
  def __init__(self, environ=os.environ):
    self.DEBUG = environ.get('EMCC_DEBUG')
    if self.DEBUG == "0":
      self.DEBUG = None
    self.DEBUG_CACHE = self.DEBUG and "cache" in self.DEBUG
    self.EMSCRIPTEN_TEMP_DIR = None

    try:
      self.TEMP_DIR = TEMP_DIR
    except NameError:
      self.TEMP_DIR = find_temp_directory()
      if self.TEMP_DIR == None:
        logging.critical('TEMP_DIR not defined in ' + os.path.expanduser('~\\.emscripten') + ", and could not detect a suitable directory! Please configure .emscripten to contain a variable TEMP_DIR='/path/to/temp/dir'.")
      logging.debug('TEMP_DIR not defined in ~/.emscripten, using ' + self.TEMP_DIR)

    if not os.path.isdir(self.TEMP_DIR):
      logging.critical("The temp directory TEMP_DIR='" + self.TEMP_DIR + "' doesn't seem to exist! Please make sure that the path is correct.")

    self.CANONICAL_TEMP_DIR = os.path.join(self.TEMP_DIR, 'emscripten_temp')

    if self.DEBUG:
      try:
        self.EMSCRIPTEN_TEMP_DIR = self.CANONICAL_TEMP_DIR
        safe_ensure_dirs(self.EMSCRIPTEN_TEMP_DIR)
      except Exception, e:
        logging.error(str(e) + 'Could not create canonical temp dir. Check definition of TEMP_DIR in ~/.emscripten')

  def get_temp_files(self):
    return tempfiles.TempFiles(
      tmp=self.TEMP_DIR if not self.DEBUG else get_emscripten_temp_dir(),
      save_debug_files=os.environ.get('EMCC_DEBUG_SAVE'))

def apply_configuration():
  global configuration, DEBUG, EMSCRIPTEN_TEMP_DIR, DEBUG_CACHE, CANONICAL_TEMP_DIR, TEMP_DIR
  configuration = Configuration()
  DEBUG = configuration.DEBUG
  EMSCRIPTEN_TEMP_DIR = configuration.EMSCRIPTEN_TEMP_DIR
  DEBUG_CACHE = configuration.DEBUG_CACHE
  CANONICAL_TEMP_DIR = configuration.CANONICAL_TEMP_DIR
  TEMP_DIR = configuration.TEMP_DIR
apply_configuration()

logging.basicConfig(format='%(levelname)-8s %(name)s: %(message)s') # can add  %(asctime)s  to see timestamps
def set_logging():
  logger = logging.getLogger()
  logger.setLevel(logging.DEBUG if os.environ.get('EMCC_DEBUG') else logging.INFO)
set_logging()

# EM_CONFIG stuff

try:
  JS_ENGINES
except:
  try:
    JS_ENGINES = [JS_ENGINE]
  except Exception, e:
    print 'ERROR: %s does not seem to have JS_ENGINES or JS_ENGINE set up' % EM_CONFIG
    raise

try:
  CLOSURE_COMPILER
except:
  CLOSURE_COMPILER = path_from_root('third_party', 'closure-compiler', 'compiler.jar')

try:
  PYTHON
except:
  logging.debug('PYTHON not defined in ~/.emscripten, using "%s"' % (sys.executable,))
  PYTHON = sys.executable

try:
  JAVA
except:
  logging.debug('JAVA not defined in ~/.emscripten, using "java"')
  JAVA = 'java'

# Additional compiler options

# Target choice. Must be synced with src/settings.js (TARGET_*)
def get_llvm_target():
  if os.environ.get('EMCC_FAST_COMPILER') == '0':
    return 'unavailable-non-fastcomp'
  return os.environ.get('EMCC_LLVM_TARGET') or 'asmjs-unknown-emscripten'
LLVM_TARGET = get_llvm_target()

# COMPILER_OPTS: options passed to clang when generating bitcode for us
try:
  COMPILER_OPTS # Can be set in EM_CONFIG, optionally
except:
  COMPILER_OPTS = []
COMPILER_OPTS = COMPILER_OPTS + [#'-fno-threadsafe-statics', # disabled due to issue 1289
                                 '-target', LLVM_TARGET,
                                 '-D__EMSCRIPTEN_major__=' + str(EMSCRIPTEN_VERSION_MAJOR),
                                 '-D__EMSCRIPTEN_minor__=' + str(EMSCRIPTEN_VERSION_MINOR),
                                 '-D__EMSCRIPTEN_tiny__=' + str(EMSCRIPTEN_VERSION_TINY)]

# COMPILER_STANDARDIZATION_OPTS: Options to correct various predefined macro options.
COMPILER_STANDARDIZATION_OPTS = []

# When we're not using an appropriate target triple, use -m32 to get i386, which we
# can mostly make work.
if LLVM_TARGET != 'asmjs-unknown-emscripten' and LLVM_TARGET != 'le32-unknown-nacl':
  COMPILER_OPTS += ['-m32']
  COMPILER_STANDARDIZATION_OPTS += ['-U__i386__', '-U__i386', '-Ui386',
                                    '-U__SSE__', '-U__SSE_MATH__', '-U__SSE2__', '-U__SSE2_MATH__', '-U__MMX__',
                                    '-U__APPLE__', '-U__linux__']

# With the asmjs-unknown-emscripten target triple, clang sets up language modes
# and predefined macros properly. When using the other targets, we have to set things
# up manually.
if LLVM_TARGET != 'asmjs-unknown-emscripten':
  COMPILER_OPTS += ['-fno-math-errno']
  COMPILER_STANDARDIZATION_OPTS += ['-D__IEEE_LITTLE_ENDIAN']
  COMPILER_OPTS += ['-DEMSCRIPTEN', '-D__EMSCRIPTEN__', '-fno-math-errno',
                    '-U__native_client__', '-U__pnacl__', '-U__ELF__']

# Changes to default clang behavior
if LLVM_TARGET == 'asmjs-unknown-emscripten' or LLVM_TARGET == 'le32-unknown-nacl':
  # Implicit functions can cause horribly confusing asm.js function pointer type errors, see #2175
  # If your codebase really needs them - very unrecommended! - you can disable the error with
  #   -Wno-error=implicit-function-declaration
  # or disable even a warning about it with
  #   -Wno-implicit-function-declaration
  COMPILER_OPTS += ['-Werror=implicit-function-declaration']

USE_EMSDK = not os.environ.get('EMMAKEN_NO_SDK')

if USE_EMSDK:
  # Disable system C and C++ include directories, and add our own (using -idirafter so they are last, like system dirs, which
  # allows projects to override them)
  C_INCLUDE_PATHS = [path_from_root('system', 'local', 'include'),
                     path_from_root('system', 'include', 'compat'),
                     path_from_root('system', 'include'),
                     path_from_root('system', 'include', 'emscripten'),
                     path_from_root('system', 'include', 'libc'),
                     path_from_root('system', 'lib', 'libc', 'musl', 'arch', 'js'),
  ]
  
  CXX_INCLUDE_PATHS = [path_from_root('system', 'include', 'libcxx')
  ]
  
  C_OPTS = ['-nostdinc', '-Xclang', '-nobuiltininc', '-Xclang', '-nostdsysteminc',
  ]
  
  def include_directive(paths):
    result = []
    for path in paths:
      result += ['-Xclang', '-isystem' + path]
    return result
  
  EMSDK_OPTS = C_OPTS + include_directive(C_INCLUDE_PATHS) + include_directive(CXX_INCLUDE_PATHS)

  EMSDK_OPTS += COMPILER_STANDARDIZATION_OPTS
  # For temporary compatibility, treat 'le32-unknown-nacl' as 'asmjs-unknown-emscripten'.
  if LLVM_TARGET != 'asmjs-unknown-emscripten' and \
     LLVM_TARGET != 'le32-unknown-pnacl':
    EMSDK_CXX_OPTS = ['-nostdinc++'] # asmjs-unknown-emscripten target does not need -nostdinc++
  else:
    EMSDK_CXX_OPTS = []
  COMPILER_OPTS += EMSDK_OPTS
else:
  EMSDK_OPTS = []
  EMSDK_CXX_OPTS = []
  COMPILER_OPTS += COMPILER_STANDARDIZATION_OPTS

#print >> sys.stderr, 'SDK opts', ' '.join(EMSDK_OPTS)
#print >> sys.stderr, 'Compiler opts', ' '.join(COMPILER_OPTS)

# Engine tweaks

try:
  if SPIDERMONKEY_ENGINE:
    new_spidermonkey = SPIDERMONKEY_ENGINE
    if '-w' not in str(new_spidermonkey):
      new_spidermonkey += ['-w']
    SPIDERMONKEY_ENGINE = fix_js_engine(SPIDERMONKEY_ENGINE, new_spidermonkey)
except NameError:
  pass

# If we have 'env', we should use that to find python, because |python| may fail while |env python| may work
# (For example, if system python is 3.x while we need 2.x, and env gives 2.x if told to do so.)
ENV_PREFIX = []
if not WINDOWS:
  try:
    assert 'Python' in Popen(['env', 'python', '-V'], stdout=PIPE, stderr=STDOUT).communicate()[0]
    ENV_PREFIX = ['env']
  except:
    pass

# Utilities

def check_engine(engine):
  # TODO: we call this several times, perhaps cache the results?
  try:
    if not CONFIG_FILE:
      return True # config stored directly in EM_CONFIG => skip engine check
    return 'hello, world!' in run_js(path_from_root('src', 'hello_world.js'), engine)
  except Exception, e:
    print 'Checking JS engine %s failed. Check %s. Details: %s' % (str(engine), EM_CONFIG, str(e))
    return False

def make_js_command(filename, engine=None, *args):
  if engine is None:
    engine = JS_ENGINES[0]
  return jsrun.make_command(filename, engine, *args)

def run_js(filename, engine=None, *args, **kw):
  if engine is None:
    engine = JS_ENGINES[0]
  return jsrun.run_js(filename, engine, *args, **kw)

def to_cc(cxx):
  # By default, LLVM_GCC and CLANG are really the C++ versions. This gets an explicit C version
  return cxx.replace('clang++', 'clang').replace('g++', 'gcc')

def line_splitter(data):
  """Silly little tool to split JSON arrays over many lines."""

  out = ''
  counter = 0

  for i in range(len(data)):
    out += data[i]
    if data[i] == ' ' and counter > 60:
      out += '\n'
      counter = 0
    else:
      counter += 1

  return out

def limit_size(string, MAX=800*20):
  if len(string) < MAX: return string
  return string[0:MAX/2] + '\n[..]\n' + string[-MAX/2:]

def read_pgo_data(filename):
  '''
    Reads the output of PGO and generates proper information for CORRECT_* == 2 's *_LINES options
  '''
  signs_lines = []
  overflows_lines = []

  for line in open(filename, 'r'):
    try:
      if line.rstrip() == '': continue
      if '%0 failures' in line: continue
      left, right = line.split(' : ')
      signature = left.split('|')[1]
      if 'Sign' in left:
        signs_lines.append(signature)
      elif 'Overflow' in left:
        overflows_lines.append(signature)
    except:
      pass

  return {
    'signs_lines': signs_lines,
    'overflows_lines': overflows_lines
  }

def unique_ordered(values): # return a list of unique values in an input list, without changing order (list(set(.)) would change order randomly)
  seen = set()
  def check(value):
    if value in seen: return False
    seen.add(value)
    return True
  return filter(check, values)

def expand_response(data):
  if type(data) == str and data[0] == '@':
    return json.loads(open(data[1:]).read())
  return data

# Settings. A global singleton. Not pretty, but nicer than passing |, settings| everywhere

class Settings2(type):
  class __impl:
    attrs = {}

    def __init__(self):
      self.reset()

    @classmethod
    def reset(self):
      self.attrs = { 'QUANTUM_SIZE': 4 }
      self.load()

    # Given some emcc-type args (-O3, -s X=Y, etc.), fill Settings with the right settings
    @classmethod
    def load(self, args=[]):
      # Load the JS defaults into python
      settings = open(path_from_root('src', 'settings.js')).read().replace('//', '#')
      settings = re.sub(r'var ([\w\d]+)', r'self.attrs["\1"]', settings)
      exec settings

      # Apply additional settings. First -O, then -s
      for i in range(len(args)):
        if args[i].startswith('-O'):
          v = args[i][2]
          if v in ['s', 'z']: v = '2'
          level = eval(v)
          self.apply_opt_level(level)
      for i in range(len(args)):
        if args[i] == '-s':
          declare = re.sub(r'([\w\d]+)\s*=\s*(.+)', r'self.attrs["\1"]=\2;', args[i+1])
          exec declare

    # Transforms the Settings information into emcc-compatible args (-s X=Y, etc.). Basically
    # the reverse of load_settings, except for -Ox which is relevant there but not here
    @classmethod
    def serialize(self):
      ret = []
      for key, value in self.attrs.iteritems():
        if key == key.upper(): # this is a hack. all of our settings are ALL_CAPS, python internals are not
          jsoned = json.dumps(value, sort_keys=True)
          ret += ['-s', key + '=' + jsoned]
      return ret

    @classmethod
    def copy(self, values):
      self.attrs = values

    @classmethod
    def apply_opt_level(self, opt_level, noisy=False):
      if opt_level == 0 and os.environ.get('EMCC_FAST_COMPILER') == '0':
        self.attrs['ASM_JS'] = 0 # non-fastcomp has asm off in -O1
      if opt_level >= 1:
        self.attrs['ASM_JS'] = 1
        self.attrs['ASSERTIONS'] = 0
        self.attrs['DISABLE_EXCEPTION_CATCHING'] = 1
        self.attrs['RELOOP'] = 1
        self.attrs['ALIASING_FUNCTION_POINTERS'] = 1

    def __getattr__(self, attr):
      if attr in self.attrs:
        return self.attrs[attr]
      else:
        raise AttributeError

    def __setattr__(self, attr, value):
      if not attr in self.attrs:
        import difflib
        logging.warning('''Assigning a non-existent settings attribute "%s"''' % attr)
        suggestions = ', '.join(difflib.get_close_matches(attr, self.attrs.keys()))
        if suggestions:
          logging.warning(''' - did you mean one of %s?''' % suggestions)
        logging.warning(''' - perhaps a typo in emcc's  -s X=Y  notation?''')
        logging.warning(''' - (see src/settings.js for valid values)''')
      self.attrs[attr] = value

  __instance = None

  @staticmethod
  def instance():
    if Settings2.__instance is None:
      Settings2.__instance = Settings2.__impl()
    return Settings2.__instance

  def __getattr__(self, attr):
    return getattr(self.instance(), attr)

  def __setattr__(self, attr, value):
    return setattr(self.instance(), attr, value)

class Settings(object):
  __metaclass__ = Settings2

# Building

class Building:
  COMPILER = CLANG
  LLVM_OPTS = False
  COMPILER_TEST_OPTS = [] # For use of the test runner
  JS_ENGINE_OVERRIDE = None # Used to pass the JS engine override from runner.py -> test_benchmark.py

  @staticmethod
  def get_building_env(native=False):
    env = os.environ.copy()
    if native:
      env['CC'] = CLANG_CC
      env['CXX'] = CLANG_CPP
      env['LD'] = CLANG
      env['CFLAGS'] = '-O2 -fno-math-errno'
      return env
    env['CC'] = EMCC if not WINDOWS else 'python %r' % EMCC
    env['CXX'] = EMXX if not WINDOWS else 'python %r' % EMXX
    env['AR'] = EMAR if not WINDOWS else 'python %r' % EMAR
    env['LD'] = EMCC if not WINDOWS else 'python %r' % EMCC
    env['LDSHARED'] = EMCC if not WINDOWS else 'python %r' % EMCC
    env['RANLIB'] = EMRANLIB if not WINDOWS else 'python %r' % EMRANLIB
    env['EMMAKEN_COMPILER'] = Building.COMPILER
    env['EMSCRIPTEN_TOOLS'] = path_from_root('tools')
    env['CFLAGS'] = env['EMMAKEN_CFLAGS'] = ' '.join(Building.COMPILER_TEST_OPTS)
    env['HOST_CC'] = CLANG_CC
    env['HOST_CXX'] = CLANG_CPP
    env['HOST_CFLAGS'] = "-W" #if set to nothing, CFLAGS is used, which we don't want
    env['HOST_CXXFLAGS'] = "-W" #if set to nothing, CXXFLAGS is used, which we don't want
    env['PKG_CONFIG_LIBDIR'] = path_from_root('system', 'local', 'lib', 'pkgconfig') + os.path.pathsep + path_from_root('system', 'lib', 'pkgconfig')
    env['PKG_CONFIG_PATH'] = os.environ.get ('EM_PKG_CONFIG_PATH') or ''
    env['EMSCRIPTEN'] = path_from_root()
    env['PATH'] = path_from_root('system', 'bin') + os.pathsep + env['PATH']
    return env

  # Finds the given executable 'program' in PATH. Operates like the Unix tool 'which'.
  @staticmethod
  def which(program):
    import os
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

  # Returns a clone of the given environment with all directories that contain sh.exe removed from the PATH.
  # Used to work around CMake limitation with MinGW Makefiles, where sh.exe is not allowed to be present.
  @staticmethod
  def remove_sh_exe_from_path(env):
    env = env.copy()
    if not WINDOWS: return env
    path = env['PATH'].split(';')
    path = filter(lambda p: not os.path.exists(os.path.join(p, 'sh.exe')), path)
    env['PATH'] = ';'.join(path)
    return env
  
  @staticmethod
  def handle_CMake_toolchain(args, env):

    def has_substr(array, substr):
      for arg in array:
        if substr in arg:
          return True
      return False

    # Append the Emscripten toolchain file if the user didn't specify one.
    if not has_substr(args, '-DCMAKE_TOOLCHAIN_FILE'):
      args.append('-DCMAKE_TOOLCHAIN_FILE=' + path_from_root('cmake', 'Modules', 'Platform', 'Emscripten.cmake'))

    # On Windows specify MinGW Makefiles if we have MinGW and no other toolchain was specified, to avoid CMake
    # pulling in a native Visual Studio, or Unix Makefiles.
    if WINDOWS and not '-G' in args and Building.which('mingw32-make'):
      args += ['-G', 'MinGW Makefiles']

    # CMake has a requirement that it wants sh.exe off PATH if MinGW Makefiles is being used. This happens quite often,
    # so do this automatically on behalf of the user. See http://www.cmake.org/Wiki/CMake_MinGW_Compiler_Issues
    if WINDOWS and 'MinGW Makefiles' in args:
      env = Building.remove_sh_exe_from_path(env)

    return (args, env)

  @staticmethod
  def configure(args, stdout=None, stderr=None, env=None):
    if not args:
      return
    if env is None:
      env = Building.get_building_env()
    if 'cmake' in args[0]:
      # Note: EMMAKEN_JUST_CONFIGURE shall not be enabled when configuring with CMake. This is because CMake
      #       does expect to be able to do config-time builds with emcc.
      args, env = Building.handle_CMake_toolchain(args, env)
    else:
      # When we configure via a ./configure script, don't do config-time compilation with emcc, but instead
      # do builds natively with Clang. This is a heuristic emulation that may or may not work. 
      env['EMMAKEN_JUST_CONFIGURE'] = '1'
    try:
      if EM_BUILD_VERBOSE_LEVEL >= 3: print >> sys.stderr, 'configure: ' + str(args)
      process = Popen(args, stdout=None if EM_BUILD_VERBOSE_LEVEL >= 2 else stdout, stderr=None if EM_BUILD_VERBOSE_LEVEL >= 1 else stderr, env=env)
      process.communicate()
    except Exception, e:
      logging.error('Exception thrown when invoking Popen in configure with args: "%s"!' % ' '.join(args))
      raise
    if 'EMMAKEN_JUST_CONFIGURE' in env: del env['EMMAKEN_JUST_CONFIGURE']
    if process.returncode is not 0:
      logging.error('Configure step failed with non-zero return code ' + str(process.returncode) + '! Command line: ' + str(args) + ' at ' + os.getcwd())
      raise subprocess.CalledProcessError(cmd=args, returncode=process.returncode)

  @staticmethod
  def make(args, stdout=None, stderr=None, env=None):
    if env is None:
      env = Building.get_building_env()
    if not args:
      logging.error('Executable to run not specified.')
      sys.exit(1)
    #args += ['VERBOSE=1']

    # On Windows prefer building with mingw32-make instead of make, if it exists.
    if WINDOWS:
      if args[0] == 'make':
        mingw32_make = Building.which('mingw32-make')
        if mingw32_make:
          args[0] = mingw32_make

      if 'mingw32-make' in args[0]:
        env = Building.remove_sh_exe_from_path(env)

    try:
      # On Windows, run the execution through shell to get PATH expansion and executable extension lookup, e.g. 'sdl2-config' will match with 'sdl2-config.bat' in PATH.
      if EM_BUILD_VERBOSE_LEVEL >= 3: print >> sys.stderr, 'make: ' + str(args)
      process = Popen(args, stdout=None if EM_BUILD_VERBOSE_LEVEL >= 2 else stdout, stderr=None if EM_BUILD_VERBOSE_LEVEL >= 1 else stderr, env=env, shell=WINDOWS)
      process.communicate()
    except Exception, e:
      logging.error('Exception thrown when invoking Popen in make with args: "%s"!' % ' '.join(args))
      raise
    if process.returncode is not 0:
      raise subprocess.CalledProcessError(cmd=args, returncode=process.returncode)


  @staticmethod
  def build_library(name, build_dir, output_dir, generated_libs, configure=['sh', './configure'], configure_args=[], make=['make'], make_args='help', cache=None, cache_name=None, copy_project=False, env_init={}, source_dir=None, native=False):
    ''' Build a library into a .bc file. We build the .bc file once and cache it for all our tests. (We cache in
        memory since the test directory is destroyed and recreated for each test. Note that we cache separately
        for different compilers).
        This cache is just during the test runner. There is a different concept of caching as well, see |Cache|. '''

    if type(generated_libs) is not list: generated_libs = [generated_libs]
    if source_dir is None: source_dir = path_from_root('tests', name.replace('_native', ''))
    if make_args == 'help':
      make_args = ['-j', str(multiprocessing.cpu_count())]

    temp_dir = build_dir
    if copy_project:
      project_dir = os.path.join(temp_dir, name)
      if os.path.exists(project_dir):
        shutil.rmtree(project_dir)
      shutil.copytree(source_dir, project_dir) # Useful in debugging sometimes to comment this out, and two lines above
    else:
      project_dir = build_dir
    try:
      old_dir = os.getcwd()
    except:
      old_dir = None
    os.chdir(project_dir)
    generated_libs = map(lambda lib: os.path.join(project_dir, lib), generated_libs)
    #for lib in generated_libs:
    #  try:
    #    os.unlink(lib) # make sure compilation completed successfully
    #  except:
    #    pass
    env = Building.get_building_env(native)
    for k, v in env_init.iteritems():
      env[k] = v
    if configure: # Useful in debugging sometimes to comment this out (and the lines below up to and including the |link| call)
      try:
        Building.configure(configure + configure_args, env=env, stdout=open(os.path.join(project_dir, 'configure_'), 'w') if EM_BUILD_VERBOSE_LEVEL < 2 else None,
                                                                stderr=open(os.path.join(project_dir, 'configure_err'), 'w') if EM_BUILD_VERBOSE_LEVEL < 1 else None)
      except subprocess.CalledProcessError, e:
        pass # Ignore exit code != 0
    def open_make_out(i, mode='r'):
      return open(os.path.join(project_dir, 'make_' + str(i)), mode)
    
    def open_make_err(i, mode='r'):
      return open(os.path.join(project_dir, 'make_err' + str(i)), mode)

    if EM_BUILD_VERBOSE_LEVEL >= 3:
      make_args += ['VERBOSE=1']

    for i in range(2): # FIXME: Sad workaround for some build systems that need to be run twice to succeed (e.g. poppler)
      with open_make_out(i, 'w') as make_out:
        with open_make_err(i, 'w') as make_err:
          try:
            Building.make(make + make_args, stdout=make_out if EM_BUILD_VERBOSE_LEVEL < 2 else None,
                                            stderr=make_err if EM_BUILD_VERBOSE_LEVEL < 1 else None, env=env)
          except subprocess.CalledProcessError, e:
            pass # Ignore exit code != 0
      try:
        if cache is not None:
          cache[cache_name] = []
          for f in generated_libs:
            basename = os.path.basename(f)
            cache[cache_name].append((basename, open(f, 'rb').read()))
        break
      except Exception, e:
        if i > 0:
          if EM_BUILD_VERBOSE_LEVEL == 0:
            # Due to the ugly hack above our best guess is to output the first run
            with open_make_err(0) as ferr:
              for line in ferr:
                sys.stderr.write(line)
          raise Exception('could not build library ' + name + ' due to exception ' + str(e))
    if old_dir:
      os.chdir(old_dir)
    return generated_libs

  @staticmethod
  def link(files, target, force_archive_contents=False):
    actual_files = []
    # Tracking unresolveds is necessary for .a linking, see below.
    # Specify all possible entry points to seed the linking process.
    # For a simple application, this would just be "main".
    unresolved_symbols = set([func[1:] for func in Settings.EXPORTED_FUNCTIONS])
    resolved_symbols = set()
    temp_dirs = []
    def make_paths_absolute(f):
      if f.startswith('-'): # skip flags
        return f
      else:
        return os.path.abspath(f)
    files = map(make_paths_absolute, files)
    # Paths of already included object files from archives.
    added_contents = set()
    # Map of archive name to list of extracted object file paths.
    ar_contents = {}
    has_ar = False
    for f in files:
      if not f.startswith('-'):
        has_ar = has_ar or Building.is_ar(f)

    # If we have only one archive or the force_archive_contents flag is set,
    # then we will add every object file we see, regardless of whether it
    # resolves any undefined symbols.
    force_add_all = len(files) == 1 or force_archive_contents

    # Considers an object file for inclusion in the link. The object is included
    # if force_add=True or if the object provides a currently undefined symbol.
    # If the object is included, the symbol tables are updated and the function
    # returns True.
    def consider_object(f, force_add=False):
      new_symbols = Building.llvm_nm(f)
      do_add = force_add or not unresolved_symbols.isdisjoint(new_symbols.defs)
      if do_add:
        logging.debug('adding object %s to link' % (f))
        # Update resolved_symbols table with newly resolved symbols
        resolved_symbols.update(new_symbols.defs)
        # Update unresolved_symbols table by adding newly unresolved symbols and
        # removing newly resolved symbols.
        unresolved_symbols.update(new_symbols.undefs.difference(resolved_symbols))
        unresolved_symbols.difference_update(new_symbols.defs)
        actual_files.append(f)
      return do_add

    def get_archive_contents(f):
      if f in ar_contents:
        return ar_contents[f]

      cwd = os.getcwd()
      try:
        emscripten_temp_dir = get_emscripten_temp_dir()
        temp_dir = os.path.join(emscripten_temp_dir, 'ar_output_' + str(os.getpid()) + '_' + str(len(temp_dirs)))
        temp_dirs.append(temp_dir)
        safe_ensure_dirs(temp_dir)
        os.chdir(temp_dir)
        contents = filter(lambda x: len(x) > 0, Popen([LLVM_AR, 't', f], stdout=PIPE).communicate()[0].split('\n'))
        # llvm-ar appears to just use basenames inside archives. as a result, files with the same basename
        # will trample each other when we extract them. to help warn of such situations, we warn if there
        # are duplicate entries in the archive
        if len(contents) != len(set(contents)):
          logging.warning('loading from archive %s, which has duplicate entries (files with identical base names). this is dangerous as only the last will be taken into account, and you may see surprising undefined symbols later. you should rename source files to avoid this problem (or avoid .a archives, and just link bitcode together to form libraries for later linking)' % f)
          warned = set()
          for i in range(len(contents)):
            curr = contents[i]
            if curr not in warned and curr in contents[i+1:]:
              logging.warning('   duplicate: %s' % curr)
              warned.add(curr)
        if len(contents) == 0:
          logging.debug('Archive %s appears to be empty (recommendation: link an .so instead of .a)' % f)
        else:
          for content in contents: # ar will silently fail if the directory for the file does not exist, so make all the necessary directories
            dirname = os.path.dirname(content)
            if dirname:
              safe_ensure_dirs(dirname)
          Popen([LLVM_AR, 'xo', f], stdout=PIPE).communicate() # if absolute paths, files will appear there. otherwise, in this directory
          contents = map(lambda content: os.path.join(temp_dir, content), contents)
          contents = filter(os.path.exists, map(os.path.abspath, contents))
          contents = filter(Building.is_bitcode, contents)
        ar_contents[f] = contents
      finally:
        os.chdir(cwd)

      return contents

    # Traverse a single archive. The object files are repeatedly scanned for
    # newly satisfied symbols until no new symbols are found. Returns true if
    # any object files were added to the link.
    def consider_archive(f):
      added_any_objects = False
      loop_again = True
      logging.debug('considering archive %s' % (f))
      contents = get_archive_contents(f)
      while loop_again: # repeatedly traverse until we have everything we need
        loop_again = False
        for content in contents:
          if content in added_contents: continue
          # Link in the .o if it provides symbols, *or* this is a singleton archive (which is apparently an exception in gcc ld)
          if consider_object(content, force_add=force_add_all):
            added_contents.add(content)
            loop_again = True
            added_any_objects = True
      logging.debug('done running loop of archive %s' % (f))
      return added_any_objects

    current_archive_group = None
    for f in files:
      if f.startswith('-'):
        if f in ['--start-group', '-(']:
          assert current_archive_group is None, 'Nested --start-group, missing --end-group?'
          current_archive_group = []
        elif f in ['--end-group', '-)']:
          assert current_archive_group is not None, '--end-group without --start-group'
          # rescan the archives in the group until we don't find any more
          # objects to link.
          loop_again = True
          logging.debug('starting archive group loop');
          while loop_again:
            loop_again = False
            for archive in current_archive_group:
              if consider_archive(archive):
                loop_again = True
          logging.debug('done with archive group loop');
          current_archive_group = None
        else:
          logging.debug('Ignoring unsupported link flag: %s' % f)
      elif not Building.is_ar(f):
        if Building.is_bitcode(f):
          if has_ar:
            consider_object(f, force_add=True)
          else:
            # If there are no archives then we can simply link all valid bitcode
            # files and skip the symbol table stuff.
            actual_files.append(f)
      else:
        # Extract object files from ar archives, and link according to gnu ld semantics
        # (link in an entire .o from the archive if it supplies symbols still unresolved)
        consider_archive(f)
        # If we're inside a --start-group/--end-group section, add to the list
        # so we can loop back around later.
        if current_archive_group is not None:
          current_archive_group.append(f)
    assert current_archive_group is None, '--start-group without matching --end-group'

    try_delete(target)

    # Finish link
    actual_files = unique_ordered(actual_files) # tolerate people trying to link a.so a.so etc.
    logging.debug('emcc: llvm-linking: %s to %s', actual_files, target)

    # check for too-long command line
    link_cmd = LINK_CMD + actual_files + ['-o', target]
    # 8k is a bit of an arbitrary limit, but a reasonable one
    # for max command line size before we use a respose file
    response_file = None
    if len(' '.join(link_cmd)) > 8192:
      logging.debug('using response file for llvm-link')
      [response_fd, response_file] = mkstemp(suffix='.response', dir=TEMP_DIR)

      link_cmd = LINK_CMD + ["@" + response_file]

      response_fh = os.fdopen(response_fd, 'w')
      for arg in actual_files:
        # we can't put things with spaces in the response file
        if " " in arg:
          link_cmd.append(arg)
        else:
          response_fh.write(arg + "\n")
      response_fh.close()
      link_cmd.append("-o")
      link_cmd.append(target)

      if len(' '.join(link_cmd)) > 8192:
        logging.warning('emcc: link command line is very long, even with response file -- use paths with no spaces')

    output = Popen(link_cmd, stdout=PIPE).communicate()[0]

    if response_file:
      os.unlink(response_file)

    assert os.path.exists(target) and (output is None or 'Could not open input file' not in output), 'Linking error: ' + output
    for temp_dir in temp_dirs:
      try_delete(temp_dir)

  # Emscripten optimizations that we run on the .ll file
  @staticmethod
  def ll_opts(filename):
    ## Remove target info. This helps LLVM opts, if we run them later
    #cleaned = filter(lambda line: not line.startswith('target datalayout = ') and not line.startswith('target triple = '),
    #                 open(filename + '.o.ll', 'r').readlines())
    #os.unlink(filename + '.o.ll')
    #open(filename + '.o.ll.orig', 'w').write(''.join(cleaned))
    pass

  # LLVM optimizations
  # @param opt Either an integer, in which case it is the optimization level (-O1, -O2, etc.), or a list of raw
  #            optimization passes passed to llvm opt
  @staticmethod
  def llvm_opt(filename, opts, out=None):
    if type(opts) is int:
      opts = Building.pick_llvm_opts(opts)
    opts = opts[:]
    #opts += ['-debug-pass=Arguments']
    if get_clang_version() >= '3.4':
      if not Settings.SIMD:
        opts += ['-disable-vectorize']
      else:
        opts += ['-bb-vectorize-vector-bits=128', '-force-vector-width=4']

    logging.debug('emcc: LLVM opts: ' + ' '.join(opts))
    target = out or (filename + '.opt.bc')
    output = Popen([LLVM_OPT, filename] + opts + ['-o', target], stdout=PIPE).communicate()[0]
    assert os.path.exists(target), 'Failed to run llvm optimizations: ' + output
    if not out:
      shutil.move(filename + '.opt.bc', filename)

  @staticmethod
  def llvm_opts(filename): # deprecated version, only for test runner. TODO: remove
    if Building.LLVM_OPTS:
      shutil.move(filename + '.o', filename + '.o.pre')
      output = Popen([LLVM_OPT, filename + '.o.pre'] + Building.LLVM_OPT_OPTS + ['-o', filename + '.o'], stdout=PIPE).communicate()[0]
      assert os.path.exists(filename + '.o'), 'Failed to run llvm optimizations: ' + output

  @staticmethod
  def llvm_dis(input_filename, output_filename=None):
    # LLVM binary ==> LLVM assembly
    if output_filename is None:
      # use test runner conventions
      output_filename = input_filename + '.o.ll'
      input_filename = input_filename + '.o'
    try_delete(output_filename)
    output = Popen([LLVM_DIS, input_filename, '-o', output_filename], stdout=PIPE).communicate()[0]
    assert os.path.exists(output_filename), 'Could not create .ll file: ' + output
    return output_filename

  @staticmethod
  def llvm_as(input_filename, output_filename=None):
    # LLVM assembly ==> LLVM binary
    if output_filename is None:
      # use test runner conventions
      output_filename = input_filename + '.o'
      input_filename = input_filename + '.o.ll'
    try_delete(output_filename)
    output = Popen([LLVM_AS, input_filename, '-o', output_filename], stdout=PIPE).communicate()[0]
    assert os.path.exists(output_filename), 'Could not create bc file: ' + output
    return output_filename

  nm_cache = {} # cache results of nm - it can be slow to run

  @staticmethod
  def llvm_nm(filename, stdout=PIPE, stderr=None):
    if filename in Building.nm_cache:
      #logging.debug('loading nm results for %s from cache' % filename)
      return Building.nm_cache[filename]

    # LLVM binary ==> list of symbols
    output = Popen([LLVM_NM, filename], stdout=stdout, stderr=stderr).communicate()[0]
    class ret:
      defs = []
      undefs = []
      commons = []
    for line in output.split('\n'):
      if len(line) == 0: continue
      parts = filter(lambda seg: len(seg) > 0, line.split(' '))
      # pnacl-nm will print zero offsets for bitcode
      if len(parts) == 3 and parts[0] == "00000000":
        parts.pop(0)
      if len(parts) == 2: # ignore lines with absolute offsets, these are not bitcode anyhow (e.g. |00000630 t d_source_name|)
        status, symbol = parts
        if status == 'U':
          ret.undefs.append(symbol)
        elif status == 'C':
          ret.commons.append(symbol)
        elif status == status.upper(): # all other uppercase statuses ('T', etc.) are normally defined symbols
          ret.defs.append(symbol)
        # otherwise, not something we should notice
    ret.defs = set(ret.defs)
    ret.undefs = set(ret.undefs)
    ret.commons = set(ret.commons)
    Building.nm_cache[filename] = ret
    return ret

  @staticmethod
  def emcc(filename, args=[], output_filename=None, stdout=None, stderr=None, env=None):
    if output_filename is None:
      output_filename = filename + '.o'
    try_delete(output_filename)
    Popen([PYTHON, EMCC, filename] + args + ['-o', output_filename], stdout=stdout, stderr=stderr, env=env).communicate()
    assert os.path.exists(output_filename), 'emcc could not create output file: ' + output_filename

  @staticmethod
  def emar(action, output_filename, filenames, stdout=None, stderr=None, env=None):
    try_delete(output_filename)
    Popen([PYTHON, EMAR, action, output_filename] + filenames, stdout=stdout, stderr=stderr, env=env).communicate()
    if 'c' in action:
      assert os.path.exists(output_filename), 'emar could not create output file: ' + output_filename

  @staticmethod
  def emscripten(filename, append_ext=True, extra_args=[]):
    # Allow usage of emscripten.py without warning
    os.environ['EMSCRIPTEN_SUPPRESS_USAGE_WARNING'] = '1'

    # Run Emscripten
    if not os.path.exists(Settings.RELOOPER):
      Settings.RELOOPER = Cache.get_path('relooper.js')
    settings = Settings.serialize()
    args = settings + extra_args
    if WINDOWS:
      rsp_file = response_file.create_response_file(args, TEMP_DIR)
      args = ['@' + rsp_file]
    cmdline = [PYTHON, EMSCRIPTEN, filename + ('.o.ll' if append_ext else ''), '-o', filename + '.o.js'] + args
    if jsrun.TRACK_PROCESS_SPAWNS:
      logging.info('Executing emscripten.py compiler with cmdline "' + ' '.join(cmdline) + '"')
    jsrun.timeout_run(Popen(cmdline, stdout=PIPE), None, 'Compiling')

    # Clean up .rsp file the compiler used after we are finished.
    if WINDOWS:
      try_delete(rsp_file)

    # Detect compilation crashes and errors
    assert os.path.exists(filename + '.o.js'), 'Emscripten failed to generate .js'

    return filename + '.o.js'

  @staticmethod
  def can_build_standalone():
    return not Settings.BUILD_AS_SHARED_LIB and not Settings.LINKABLE and not Settings.EXPORT_ALL

  @staticmethod
  def can_use_unsafe_opts():
    return Settings.USE_TYPED_ARRAYS == 2

  @staticmethod
  def can_inline():
    return Settings.INLINING_LIMIT == 0

  @staticmethod
  def get_safe_internalize():
    if not Building.can_build_standalone(): return [] # do not internalize anything
    exps = expand_response(Settings.EXPORTED_FUNCTIONS)
    exports = ','.join(map(lambda exp: exp[1:], exps))
    # internalize carefully, llvm 3.2 will remove even main if not told not to
    return ['-internalize', '-internalize-public-api-list=' + exports]

  @staticmethod
  def pick_llvm_opts(optimization_level):
    '''
      It may be safe to use nonportable optimizations (like -OX) if we remove the platform info from the .ll
      (which we do in do_ll_opts) - but even there we have issues (even in TA2) with instruction combining
      into i64s. In any case, the handpicked ones here should be safe and portable. They are also tuned for
      things that look useful.

      An easy way to see LLVM's standard list of passes is

        llvm-as < /dev/null | opt -std-compile-opts -disable-output -debug-pass=Arguments
    '''
    assert 0 <= optimization_level <= 3
    unsafe = Building.can_use_unsafe_opts()
    opts = []
    if optimization_level > 0:
      if unsafe:
        if not Building.can_inline():
          opts.append('-disable-inlining')
        if not Building.can_build_standalone():
          # -O1 does not have -gobaldce, which removes stuff that is needed for libraries and linkables
          optimization_level = min(1, optimization_level)
        opts.append('-O%d' % optimization_level)
        #print '[unsafe: %s]' % ','.join(opts)
      else:
        allow_nonportable = False
        optimize_size = True
        use_aa = False

        # PassManagerBuilder::populateModulePassManager
        if allow_nonportable and use_aa: # ammo.js results indicate this can be nonportable
          opts.append('-tbaa')
          opts.append('-basicaa') # makes fannkuch slow but primes fast

        if Building.can_build_standalone():
          opts += Building.get_safe_internalize()

        opts.append('-globalopt')
        opts.append('-ipsccp')
        opts.append('-deadargelim')
        if allow_nonportable: opts.append('-instcombine')
        opts.append('-simplifycfg')

        opts.append('-prune-eh')
        if Building.can_inline(): opts.append('-inline')
        opts.append('-functionattrs')
        if optimization_level > 2:
          opts.append('-argpromotion')

        # XXX Danger: Can turn a memcpy into something that violates the
        #             load-store consistency hypothesis. See hashnum() in Lua.
        #             Note: this opt is of great importance for raytrace...
        if allow_nonportable: opts.append('-scalarrepl')

        if allow_nonportable: opts.append('-early-cse') # ?
        opts.append('-simplify-libcalls')
        opts.append('-jump-threading')
        if allow_nonportable: opts.append('-correlated-propagation') # ?
        opts.append('-simplifycfg')
        if allow_nonportable: opts.append('-instcombine')

        opts.append('-tailcallelim')
        opts.append('-simplifycfg')
        opts.append('-reassociate')
        opts.append('-loop-rotate')
        opts.append('-licm')
        opts.append('-loop-unswitch') # XXX should depend on optimize_size
        if allow_nonportable: opts.append('-instcombine')
        if Settings.QUANTUM_SIZE == 4: opts.append('-indvars') # XXX this infinite-loops raytrace on q1 (loop in |new node_t[count]| has 68 hardcoded &not fixed)
        if allow_nonportable: opts.append('-loop-idiom') # ?
        opts.append('-loop-deletion')
        opts.append('-loop-unroll')

        ##### not in llvm-3.0. but have |      #addExtensionsToPM(EP_LoopOptimizerEnd, MPM);| if allow_nonportable: opts.append('-instcombine')

        # XXX Danger: Messes up Lua output for unknown reasons
        #             Note: this opt is of minor importance for raytrace...
        if optimization_level > 1 and allow_nonportable: opts.append('-gvn')

        opts.append('-memcpyopt') # Danger?
        opts.append('-sccp')

        if allow_nonportable: opts.append('-instcombine')
        opts.append('-jump-threading')
        opts.append('-correlated-propagation')
        opts.append('-dse')
        #addExtensionsToPM(EP_ScalarOptimizerLate, MPM)

        opts.append('-adce')
        opts.append('-simplifycfg')
        if allow_nonportable: opts.append('-instcombine')

        opts.append('-strip-dead-prototypes')

        if Building.can_build_standalone():
          opts.append('-globaldce')

        if optimization_level > 1: opts.append('-constmerge')

    Building.LLVM_OPT_OPTS = opts
    return opts

  @staticmethod
  def js_optimizer(filename, passes, jcache=False, debug=False, extra_info=None, output_filename=None, just_split=False, just_concat=False):
    ret = js_optimizer.run(filename, passes, NODE_JS, jcache, debug, extra_info, just_split, just_concat)
    if output_filename:
      safe_move(ret, output_filename)
      ret = output_filename
    return ret

  @staticmethod
  def closure_compiler(filename, pretty=True):
    if not os.path.exists(CLOSURE_COMPILER):
      raise Exception('Closure compiler appears to be missing, looked at: ' + str(CLOSURE_COMPILER))

    CLOSURE_EXTERNS = path_from_root('src', 'closure-externs.js')

    # Something like this (adjust memory as needed):
    #   java -Xmx1024m -jar CLOSURE_COMPILER --compilation_level ADVANCED_OPTIMIZATIONS --variable_map_output_file src.cpp.o.js.vars --js src.cpp.o.js --js_output_file src.cpp.o.cc.js
    args = [JAVA,
            '-Xmx' + (os.environ.get('JAVA_HEAP_SIZE') or '1024m'), # if you need a larger Java heap, use this environment variable
            '-jar', CLOSURE_COMPILER,
            '--compilation_level', 'ADVANCED_OPTIMIZATIONS',
            '--language_in', 'ECMASCRIPT5',
            '--externs', CLOSURE_EXTERNS,
            #'--variable_map_output_file', filename + '.vars',
            '--js', filename, '--js_output_file', filename + '.cc.js']
    if pretty: args += ['--formatting', 'PRETTY_PRINT']
    if os.environ.get('EMCC_CLOSURE_ARGS'):
      args += shlex.split(os.environ.get('EMCC_CLOSURE_ARGS'))
    logging.debug('closure compiler: ' + ' '.join(args))
    process = Popen(args, stdout=PIPE, stderr=STDOUT)
    cc_output = process.communicate()[0]
    if process.returncode != 0 or not os.path.exists(filename + '.cc.js'):
      raise Exception('closure compiler error: ' + cc_output + ' (rc: %d)' % process.returncode)

    return filename + '.cc.js'

  _is_ar_cache = {}
  @staticmethod
  def is_ar(filename):
    try:
      if Building._is_ar_cache.get(filename):
        return Building._is_ar_cache[filename]
      b = open(filename, 'r').read(8)
      sigcheck = b[0] == '!' and b[1] == '<' and \
                 b[2] == 'a' and b[3] == 'r' and \
                 b[4] == 'c' and b[5] == 'h' and \
                 b[6] == '>' and ord(b[7]) == 10
      Building._is_ar_cache[filename] = sigcheck
      return sigcheck
    except Exception, e:
      logging.debug('Building.is_ar failed to test whether file \'%s\' is a llvm archive file! Failed on exception: %s' % (filename, e))
      return False

  @staticmethod
  def is_bitcode(filename):
    # look for magic signature
    b = open(filename, 'r').read(4)
    if b[0] == 'B' and b[1] == 'C':
      return True
    # look for ar signature
    elif Building.is_ar(filename):
      return True
    # on OS X, there is a 20-byte prefix
    elif ord(b[0]) == 222 and ord(b[1]) == 192 and ord(b[2]) == 23 and ord(b[3]) == 11:
      b = open(filename, 'r').read(24)
      return b[20] == 'B' and b[21] == 'C'

    return False

  # Make sure the relooper exists. If it does not, check out the relooper code and bootstrap it
  @staticmethod
  def ensure_relooper(relooper):
    if os.path.exists(relooper): return
    if os.environ.get('EMCC_FAST_COMPILER') != '0':
      logging.debug('not building relooper to js, using it in c++ backend')
      return

    Cache.ensure()
    curr = os.getcwd()
    try:
      ok = False
      logging.info('=======================================')
      logging.info('bootstrapping relooper...')
      os.chdir(path_from_root('src'))

      emcc_debug = os.environ.get('EMCC_DEBUG')
      if emcc_debug: del os.environ['EMCC_DEBUG']

      emcc_leave_inputs_raw = os.environ.get('EMCC_LEAVE_INPUTS_RAW')
      if emcc_leave_inputs_raw: del os.environ['EMCC_LEAVE_INPUTS_RAW']

      def make(opt_level, reloop):
        raw = relooper + '.raw.js'
        Building.emcc(os.path.join('relooper', 'Relooper.cpp'), ['-I' + os.path.join('relooper'), '--post-js',
          os.path.join('relooper', 'emscripten', 'glue.js'),
          '--memory-init-file', '0', '-s', 'RELOOP=%d' % reloop,
          '-s', 'EXPORTED_FUNCTIONS=["_rl_set_output_buffer","_rl_make_output_buffer","_rl_new_block","_rl_delete_block","_rl_block_add_branch_to","_rl_new_relooper","_rl_delete_relooper","_rl_relooper_add_block","_rl_relooper_calculate","_rl_relooper_render", "_rl_set_asm_js_mode"]',
          '-s', 'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=["memcpy", "memset", "malloc", "free", "puts"]',
          '-s', 'RELOOPER="' + relooper + '"',
          '-O' + str(opt_level), '--closure', '0'], raw)
        f = open(relooper, 'w')
        f.write("// Relooper, (C) 2012 Alon Zakai, MIT license, https://github.com/kripken/Relooper\n")
        f.write("var Relooper = (function(Module) {\n")
        f.write(open(raw).read())
        f.write('\n  return Module.Relooper;\n')
        f.write('})(RelooperModule);\n')
        f.close()

      # bootstrap phase 1: generate unrelooped relooper, for which we do not need a relooper (so we cannot recurse infinitely in this function)
      logging.info('  bootstrap phase 1')
      make(2, 0)
      # bootstrap phase 2: generate relooped relooper, using the unrelooped relooper (we see relooper.js exists so we cannot recurse infinitely in this function)
      logging.info('  bootstrap phase 2')
      make(2, 1)
      logging.info('bootstrapping relooper succeeded')
      logging.info('=======================================')
      ok = True
    finally:
      os.chdir(curr)
      if emcc_debug: os.environ['EMCC_DEBUG'] = emcc_debug
      if emcc_leave_inputs_raw: os.environ['EMCC_LEAVE_INPUTS_RAW'] = emcc_leave_inputs_raw
      if not ok:
        logging.error('bootstrapping relooper failed. You may need to manually create relooper.js by compiling it, see src/relooper/emscripten')
        try_delete(relooper) # do not leave a phase-1 version if phase 2 broke
        1/0
  
  @staticmethod
  def ensure_struct_info(info_path):
    if os.path.exists(info_path): return
    Cache.ensure()
    
    import gen_struct_info
    gen_struct_info.main(['-qo', info_path, path_from_root('src/struct_info.json')])
  
# compatibility with existing emcc, etc. scripts
Cache = cache.Cache(debug=DEBUG_CACHE)
JCache = cache.JCache(Cache)
chunkify = cache.chunkify

def reconfigure_cache():
  global Cache
  Cache = cache.Cache(debug=DEBUG_CACHE)

class JS:
  memory_initializer_pattern = '/\* memory initializer \*/ allocate\(\[([\d, ]*)\], "i8", ALLOC_NONE, ([\d+Runtime\.GLOBAL_BASEH]+)\);'
  no_memory_initializer_pattern = '/\* no memory initializer \*/'

  memory_staticbump_pattern = 'STATICTOP = STATIC_BASE \+ (\d+);'

  global_initializers_pattern = '/\* global initializers \*/ __ATINIT__.push\((.+)\);'

  @staticmethod
  def to_nice_ident(ident): # limited version of the JS function toNiceIdent
    return ident.replace('%', '$').replace('@', '_').replace('.', '_')

  @staticmethod
  def make_initializer(sig, settings=None):
    settings = settings or Settings
    if sig == 'i':
      return '0'
    elif sig == 'f' and settings.get('PRECISE_F32'):
      return 'Math_fround(0)'
    else:
      return '+0'

  FLOAT_SIGS = ['f', 'd']

  @staticmethod
  def make_coercion(value, sig, settings=None, ffi_arg=False, ffi_result=False, convert_from=None):
    settings = settings or Settings
    if sig == 'i':
      if convert_from in JS.FLOAT_SIGS: value = '(~~' + value + ')'
      return value + '|0'
    if sig in JS.FLOAT_SIGS and convert_from == 'i':
      value = '(' + value + '|0)'
    if sig == 'f' and settings.get('PRECISE_F32'):
      if ffi_arg:
        return '+Math_fround(' + value + ')'
      elif ffi_result:
        return 'Math_fround(+(' + value + '))'
      else:
        return 'Math_fround(' + value + ')'
    elif sig == 'd' or sig == 'f':
      return '+' + value
    else:
      return value

  @staticmethod
  def make_extcall(sig, named=True):
    args = ','.join(['a' + str(i) for i in range(1, len(sig))])
    args = 'index' + (',' if args else '') + args
    # C++ exceptions are numbers, and longjmp is a string 'longjmp'
    ret = '''function%s(%s) {
  %sModule["dynCall_%s"](%s);
}''' % ((' extCall_' + sig) if named else '', args, 'return ' if sig[0] != 'v' else '', sig, args)

    if Settings.DLOPEN_SUPPORT and Settings.ASSERTIONS:
      # guard against cross-module stack leaks
      ret = ret.replace(') {\n', ''') {
  try {
    var preStack = asm.stackSave();
''').replace(';\n}', ''';
  } finally {
    assert(asm.stackSave() == preStack);
  }
}''')
    return ret

  @staticmethod
  def make_invoke(sig, named=True):
    args = ','.join(['a' + str(i) for i in range(1, len(sig))])
    args = 'index' + (',' if args else '') + args
    # C++ exceptions are numbers, and longjmp is a string 'longjmp'
    ret = '''function%s(%s) {
  try {
    %sModule["dynCall_%s"](%s);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}''' % ((' invoke_' + sig) if named else '', args, 'return ' if sig[0] != 'v' else '', sig, args)

    if Settings.DLOPEN_SUPPORT and Settings.ASSERTIONS:
      # guard against cross-module stack leaks
      ret = ret.replace('  try {', '''  var preStack = asm.stackSave();
  try {
''').replace('  }\n}', '''  } finally {
    assert(asm.stackSave() == preStack);
  }
}''')

    return ret

  @staticmethod
  def align(x, by):
    while x % by != 0: x += 1
    return x

  INITIALIZER_CHUNK_SIZE = 10240

  @staticmethod
  def collect_initializers(src):
    ret = []
    max_offset = -1
    for init in re.finditer(JS.memory_initializer_pattern, src):
      contents = init.group(1).split(',')
      offset = sum([int(x) if x[0] != 'R' else 0 for x in init.group(2).split('+')])
      ret.append((offset, contents))
      assert offset > max_offset
      max_offset = offset
    return ret

  @staticmethod
  def split_initializer(contents):
    # given a memory initializer (see memory_initializer_pattern), split it up into multiple initializers to avoid long runs of zeros or a single overly-large allocator
    ret = []
    l = len(contents)
    maxx = JS.INITIALIZER_CHUNK_SIZE
    i = 0
    start = 0
    while 1:
      if i - start >= maxx or (i > start and i == l):
        #print >> sys.stderr, 'new', start, i-start
        ret.append((start, contents[start:i]))
        start = i
      if i == l: break
      if contents[i] != '0':
        i += 1
      else:
        # look for a sequence of zeros
        j = i + 1
        while j < l and contents[j] == '0': j += 1
        if j-i > maxx/10 or j-start >= maxx:
          #print >> sys.stderr, 'skip', start, i-start, j-start
          ret.append((start, contents[start:i])) # skip over the zeros starting at i and ending at j
          start = j
        i = j
    return ret

  @staticmethod
  def replace_initializers(src, inits):
    class State:
      first = True
    def rep(m):
      if not State.first: return ''
      # write out all the new initializers in place of the first old one
      State.first = False
      def gen_init(init):
        offset, contents = init
        return '/* memory initializer */ allocate([%s], "i8", ALLOC_NONE, Runtime.GLOBAL_BASE%s);' % (
          ','.join(contents),
          '' if offset == 0 else ('+%d' % offset)
        )
      return '\n'.join(map(gen_init, inits))
    return re.sub(JS.memory_initializer_pattern, rep, src)

  @staticmethod
  def optimize_initializer(src):
    inits = JS.collect_initializers(src)
    if len(inits) == 0: return None
    assert len(inits) == 1
    init = inits[0]
    offset, contents = init
    assert offset == 0 # offset 0, singleton
    if len(contents) <= JS.INITIALIZER_CHUNK_SIZE: return None
    return JS.replace_initializers(src, JS.split_initializer(contents))

# Compression of code and data for smaller downloads
class Compression:
  on = False

  @staticmethod
  def compressed_name(filename):
    return filename + '.compress'

  @staticmethod
  def compress(filename):
    execute(Compression.encoder, stdin=open(filename, 'rb'), stdout=open(Compression.compressed_name(filename), 'wb'))

  @staticmethod
  def worth_it(original, compressed):
    return compressed < original - 1500 # save at least one TCP packet or so

def execute(cmd, *args, **kw):
  try:
    return Popen(cmd, *args, **kw).communicate() # let compiler frontend print directly, so colors are saved (PIPE kills that)
  except:
    if not isinstance(cmd, str):
      cmd = ' '.join(cmd)
    logging.error('Invoking Process failed: <<< ' + cmd + ' >>>')
    raise

def check_execute(cmd, *args, **kw):
  # TODO: use in more places. execute doesn't actually check that return values
  # are nonzero
  try:
    kw['stderr'] = STDOUT
    subprocess.check_output(cmd, *args, **kw)
    logging.debug("Successfuly executed %s" % " ".join(cmd))
  except subprocess.CalledProcessError as e:
    logging.error("'%s' failed with output:\n%s" % (" ".join(e.cmd), e.output))
    raise

def check_call(cmd, *args, **kw):
  try:
    subprocess.check_call(cmd, *args, **kw)
    logging.debug("Successfuly executed %s" % " ".join(cmd))
  except subprocess.CalledProcessError as e:
    logging.error("'%s' failed" % " ".join(cmd))
    raise

def suffix(name):
  parts = name.split('.')
  if len(parts) > 1:
    return parts[-1]
  else:
    return None

def unsuffixed(name):
  return '.'.join(name.split('.')[:-1])

def unsuffixed_basename(name):
  return os.path.basename(unsuffixed(name))

def safe_move(src, dst):
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  shutil.move(src, dst)

def safe_copy(src, dst):
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  shutil.copyfile(src, dst)

import js_optimizer

