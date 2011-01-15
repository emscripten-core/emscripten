
import shutil, time
from subprocess import Popen, PIPE, STDOUT

CONFIG_FILE = os.path.expanduser('~/.emscripten')
if not os.path.exists(CONFIG_FILE):
  shutil.copy(path_from_root('tests', 'settings.py'), CONFIG_FILE)
exec(open(CONFIG_FILE, 'r').read())

def timeout_run(proc, timeout, note):
  start = time.time()
  if timeout is not None:
    while time.time() - start < timeout and proc.poll() is None:
      time.sleep(0.1)
    if proc.poll() is None:
      proc.kill() # XXX bug: killing emscripten.py does not kill it's child process!
      raise Exception("Timed out: " + note)
  return proc.communicate()[0]

def run_js(engine, filename, args, check_timeout=False):
  return timeout_run(Popen(engine + [filename] + (['--'] if 'v8' in engine[0] else []) + args,
                     stdout=PIPE, stderr=STDOUT), 120 if check_timeout else None, 'Execution')

