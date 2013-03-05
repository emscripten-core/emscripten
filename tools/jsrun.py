import time
from subprocess import Popen, PIPE, STDOUT

def timeout_run(proc, timeout, note='unnamed process', full_output=False):
  start = time.time()
  if timeout is not None:
    while time.time() - start < timeout and proc.poll() is None:
      time.sleep(0.1)
    if proc.poll() is None:
      proc.kill() # XXX bug: killing emscripten.py does not kill it's child process!
      raise Exception("Timed out: " + note)
  out = proc.communicate()
  return '\n'.join(out) if full_output else out[0]

def run_js(filename, engine=None, args=[], check_timeout=False, stdout=PIPE, stderr=None, cwd=None, full_output=False):
  if type(engine) is not list:
    engine = [engine]
  command = engine + [filename] + (['--'] if 'd8' in engine[0] else []) + args
  return timeout_run(
    Popen(
      command,
      stdout=stdout,
      stderr=stderr,
      cwd=cwd),
    15*60 if check_timeout else None,
    'Execution',
    full_output=full_output)
