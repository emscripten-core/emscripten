from toolchain_profiler import ToolchainProfiler
import time, os, sys, logging
from subprocess import Popen, PIPE, STDOUT

TRACK_PROCESS_SPAWNS = True if (os.getenv('EM_BUILD_VERBOSE') and int(os.getenv('EM_BUILD_VERBOSE')) >= 3) else False
WORKING_ENGINES = {} # Holds all configured engines and whether they work: maps path -> True/False
__rootpath__ = os.path.dirname(os.path.dirname(__file__))

def timeout_run(proc, timeout=None, note='unnamed process', full_output=False, note_args=[], throw_on_failure=True):
  start = time.time()
  if timeout is not None:
    while time.time() - start < timeout and proc.poll() is None:
      time.sleep(0.1)
    if proc.poll() is None:
      proc.kill() # XXX bug: killing emscripten.py does not kill it's child process!
      raise Exception("Timed out: " + note)
  out = proc.communicate()
  out = map(lambda o: '' if o is None else o, out)
  if throw_on_failure and proc.returncode != 0:
    raise Exception('Subprocess "' + ' '.join(note_args) + '" failed with exit code ' + str(proc.returncode) + '!')
  if TRACK_PROCESS_SPAWNS:
    logging.info('Process ' + str(proc.pid) + ' finished after ' + str(time.time() - start) + ' seconds. Exit code: ' + str(proc.returncode))
  return '\n'.join(out) if full_output else out[0]

def make_command(filename, engine=None, args=[]):
  if type(engine) is not list:
    engine = [engine]
  # Emscripten supports multiple javascript runtimes.  The default is nodejs but
  # it can also use d8 (the v8 engine shell) or jsc (JavaScript Core aka
  # Safari).  Both d8 and jsc require a '--' to delimit arguments to be passed
  # to the executed script from d8/jsc options.  Node does not require a
  # delimeter--arguments after the filename are passed to the script.
  #
  # Check only the last part of the engine path to ensure we don't accidentally
  # label a path to nodejs containing a 'd8' as spidermonkey instead.
  jsengine = os.path.split(engine[0])[-1]
  # Use "'d8' in" because the name can vary, e.g. d8_g, d8, etc.
  is_d8 = 'd8' in jsengine
  # Disable true async compilation (async apis will in fact be synchronous) for now
  # due to https://bugs.chromium.org/p/v8/issues/detail?id=6263
  shell_option_flags = ['--no-wasm-async-compilation'] if is_d8 else []
  # Separates engine flags from script flags
  flag_separator = ['--'] if is_d8 or 'jsc' in jsengine else []
  return engine + [filename] + shell_option_flags + flag_separator + args


def check_engine(engine):
  if type(engine) is list:
    engine_path = engine[0]
  else:
    engine_path = engine
  global WORKING_ENGINES
  if engine_path in WORKING_ENGINES:
    return WORKING_ENGINES[engine_path]
  try:
    logging.debug('Checking JS engine %s' % engine)
    if 'hello, world!' in run_js(os.path.join(__rootpath__, 'src', 'hello_world.js'), engine, skip_check=True):
      WORKING_ENGINES[engine_path] = True
  except Exception, e:
    logging.info('Checking JS engine %s failed. Check your config file. Details: %s' % (str(engine), str(e)))
    WORKING_ENGINES[engine_path] = False
  return WORKING_ENGINES[engine_path]


def require_engine(engine):
  engine_path = engine[0]
  if engine_path not in WORKING_ENGINES:
    check_engine(engine)
  if not WORKING_ENGINES[engine_path]:
    logging.critical('The JavaScript shell (%s) does not seem to work, check the paths in the config file' % engine)
    sys.exit(1)


def run_js(filename, engine=None, args=[], check_timeout=False, stdin=None, stdout=PIPE, stderr=None, cwd=None,
           full_output=False, assert_returncode=0, error_limit=-1, skip_check=False):
  #  # code to serialize out the test suite files
  #  # XXX make sure to disable memory init files, and clear out the base_dir. you may also need to manually grab e.g. paper.pdf.js from a run of test_poppler
  #  import shutil, json
  #  base_dir = '/tmp/emscripten_suite'
  #  if not os.path.exists(base_dir):
  #    os.makedirs(base_dir)
  #  commands_file = os.path.join(base_dir, 'commands.txt')
  #  commands = ''
  #  if os.path.exists(commands_file):
  #    commands = open(commands_file).read()
  #  i = 0
  #  while True:
  #    curr = os.path.join(base_dir, str(i) + '.js')
  #    if not os.path.exists(curr): break
  #    i += 1
  #  shutil.copyfile(filename, curr)
  #  commands += os.path.basename(curr) + ',' + json.dumps(args) + '\n'
  #  open(commands_file, 'w').write(commands)

  command = make_command(filename, engine, args)
  try:
    if cwd is not None: os.environ['EMCC_BUILD_DIR'] = os.getcwd()
    proc = Popen(
        command,
        stdin=stdin,
        stdout=stdout,
        stderr=stderr,
        cwd=cwd)
  except Exception, e:
    # the failure may be because the engine is not present. show the proper
    # error in that case
    if not skip_check:
      require_engine(engine)
    # if we got here, then require_engine succeeded, so we can raise the original error
    raise e
  finally:
    if cwd is not None: del os.environ['EMCC_BUILD_DIR']
  timeout = 15*60 if check_timeout else None
  if TRACK_PROCESS_SPAWNS:
    logging.info('Blocking on process ' + str(proc.pid) + ': ' + str(command) + (' for ' + str(timeout) + ' seconds' if timeout else ' until it finishes.'))
  try:
    ret = timeout_run(
      proc,
      timeout,
      'Execution',
      full_output=full_output,
      throw_on_failure=False)
  except Exception, e:
    # the failure may be because the engine does not work. show the proper
    # error in that case
    if not skip_check:
      require_engine(engine)
    # if we got here, then require_engine succeeded, so we can raise the original error
    raise e
  if assert_returncode is not None and proc.returncode is not assert_returncode:
    raise Exception('Expected the command ' + str(command) + ' to finish with return code ' + str(assert_returncode) + ', but it returned with code ' + str(proc.returncode) + ' instead! Output: ' + str(ret)[:error_limit])
  return ret
