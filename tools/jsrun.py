import time, os, sys, logging
from subprocess import Popen, PIPE, STDOUT

TRACK_PROCESS_SPAWNS = True if (os.getenv('EM_BUILD_VERBOSE') and int(os.getenv('EM_BUILD_VERBOSE')) >= 3) else False

def timeout_run(proc, timeout=None, note='unnamed process', full_output=False):
  start = time.time()
  if timeout is not None:
    while time.time() - start < timeout and proc.poll() is None:
      time.sleep(0.1)
    if proc.poll() is None:
      proc.kill() # XXX bug: killing emscripten.py does not kill it's child process!
      raise Exception("Timed out: " + note)
  out = proc.communicate()
  out = map(lambda o: '' if o is None else o, out)
  if TRACK_PROCESS_SPAWNS:
    logging.info('Process ' + str(proc.pid) + ' finished after ' + str(time.time() - start) + ' seconds. Exit code: ' + str(proc.returncode))
  return '\n'.join(out) if full_output else out[0]

def make_command(filename, engine=None, args=[]):
  if type(engine) is not list:
    engine = [engine]
  return engine + [filename] + (['--'] if 'd8' in engine[0] or 'jsc' in engine[0] else []) + args

def run_js(filename, engine=None, args=[], check_timeout=False, stdin=None, stdout=PIPE, stderr=None, cwd=None, full_output=False, assert_returncode=0, error_limit=-1):
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
  finally:
    if cwd is not None: del os.environ['EMCC_BUILD_DIR']
  timeout = 15*60 if check_timeout else None
  if TRACK_PROCESS_SPAWNS:
    logging.info('Blocking on process ' + str(proc.pid) + ': ' + str(command) + (' for ' + str(timeout) + ' seconds' if timeout else ' until it finishes.'))
  ret = timeout_run(
    proc,
    timeout,
    'Execution',
    full_output=full_output)
  if assert_returncode is not None and proc.returncode is not assert_returncode:
    raise Exception('Expected the command ' + str(command) + ' to finish with return code ' + str(assert_returncode) + ', but it returned with code ' + str(proc.returncode) + ' instead! Output: ' + str(ret)[:error_limit])
  return ret

