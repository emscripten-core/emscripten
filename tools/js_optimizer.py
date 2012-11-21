
import os, sys, subprocess, multiprocessing, re
import shared

temp_files = shared.TempFiles()

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

JS_OPTIMIZER = path_from_root('tools', 'js-optimizer.js')

BEST_JS_PROCESS_SIZE = 1024*1024

WINDOWS = sys.platform.startswith('win')

DEBUG = os.environ.get('EMCC_DEBUG')

def run_on_chunk(command):
  filename = command[2] # XXX hackish
  output = subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0]
  assert len(output) > 0 and not output.startswith('Assertion failed'), 'Error in js optimizer: ' + output
  filename += '.jo.js'
  temp_files.note(filename)
  f = open(filename, 'w')
  f.write(output)
  f.close()
  return filename

def run(filename, passes, js_engine, jcache):
  if type(passes) == str:
    passes = [passes]

  js = open(filename).read()
  if os.linesep != '\n':
    js = js.replace(os.linesep, '\n') # we assume \n in the splitting code

  # Find suffix
  suffix_marker = '// EMSCRIPTEN_GENERATED_FUNCTIONS'
  suffix_start = js.find(suffix_marker)
  suffix = ''
  if suffix_start >= 0:
    suffix = js[suffix_start:js.find('\n', suffix_start)] + '\n'

  # Pick where to split into chunks, so that (1) they do not oom in node/uglify, and (2) we can run them in parallel
  parts = map(lambda part: part, js.split('\n}\n'))
  funcs = []
  buffered = []
  for i in range(len(parts)):
    func = parts[i]
    if i < len(parts)-1: func += '\n}\n' # last part needs no } and already has suffix
    m = re.search('function (_\w+)\(', func)
    if m:
      ident = m.group(1)
      if buffered:
        func = ''.join(buffered) + func
        buffered = []
      funcs.append((ident, func))
    else:
      buffered.append(func)
  if buffered:
    if len(funcs) > 0:
      funcs[-1] = (funcs[-1][0], funcs[-1][1] + ''.join(buffered))
    else:
      funcs.append(('anonymous', ''.join(buffered)))
  parts = None

  chunks = shared.JCache.chunkify(funcs, BEST_JS_PROCESS_SIZE, 'jsopt' if jcache else None)

  if len(chunks) > 1:
    def write_chunk(chunk, i):
      temp_file = temp_files.get('.jsfunc_%d.ll' % i).name
      f = open(temp_file, 'w')
      f.write(chunk)
      if i < len(chunks)-1:
        f.write(suffix) # last already has the suffix
      f.close()
      return temp_file
    chunks = [write_chunk(chunks[i], i) for i in range(len(chunks))]
  else:
    chunks = [filename]

  # XXX Use '--nocrankshaft' to disable crankshaft to work around v8 bug 1895, needed for older v8/node (node 0.6.8+ should be ok)
  commands = map(lambda chunk: [js_engine, JS_OPTIMIZER, chunk] + passes, chunks)

  if len(chunks) > 1:
    # We are splitting into chunks. Hopefully we can do that in parallel
    commands = map(lambda command: command + ['noPrintMetadata'], commands)
    filename += '.jo.js'

    fail = None
    cores = min(multiprocessing.cpu_count(), chunks)
    if cores < 2:
      fail = 'python reports you have %d cores' % cores
    #elif WINDOWS:
    #  fail = 'windows (see issue 663)' # This seems fixed with adding emcc.py that imports this file

    if not fail:
      # We can parallelize
      if DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks, using %d cores  (total: %.2f MB)' % (len(chunks), cores, len(js)/(1024*1024.))
      pool = multiprocessing.Pool(processes=cores)
      filenames = pool.map(run_on_chunk, commands, chunksize=1)
    else:
      # We can't parallize, but still break into chunks to avoid uglify/node memory issues
      if DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks (not in parallel because %s)' % (len(chunks), fail)
      filenames = [run_on_chunk(command) for command in commands]

    f = open(filename, 'w')
    for out_file in filenames:
      f.write(open(out_file).read())
    f.write(suffix)
    f.write('\n')
    f.close()
    return filename
  else:
    # one simple chunk, just do it
    return run_on_chunk(commands[0])

