
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
  filename = temp_files.get(os.path.basename(filename) + '.jo.js').name
  f = open(filename, 'w')
  f.write(output)
  f.close()
  return filename

def run(filename, passes, js_engine, jcache):
  if jcache: shared.JCache.ensure()

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

  if jcache:
    # load chunks from cache where we can # TODO: ignore small chunks
    cached_outputs = []
    def load_from_cache(chunk):
      keys = [chunk]
      shortkey = shared.JCache.get_shortkey(keys) # TODO: share shortkeys with later code
      out = shared.JCache.get(shortkey, keys)
      if out:
        cached_outputs.append(out)
        return False
      return True
    chunks = filter(load_from_cache, chunks)
    if len(cached_outputs) > 0:
      if out and DEBUG: print >> sys.stderr, '  loading %d funcchunks from jcache' % len(cached_outputs)
    else:
      cached_outputs = []

  if len(chunks) > 1:
    def write_chunk(chunk, i):
      temp_file = temp_files.get('.jsfunc_%d.ll' % i).name
      f = open(temp_file, 'w')
      f.write(chunk)
      if i < len(chunks)-1:
        f.write(suffix) # last already has the suffix
      f.close()
      return temp_file
    filenames = [write_chunk(chunks[i], i) for i in range(len(chunks))]
  else:
    filenames = [filename]

  if len(filenames) > 0:
    # XXX Use '--nocrankshaft' to disable crankshaft to work around v8 bug 1895, needed for older v8/node (node 0.6.8+ should be ok)
    commands = map(lambda filename: [js_engine, JS_OPTIMIZER, filename, 'noPrintMetadata'] + passes, filenames)

    cores = min(multiprocessing.cpu_count(), filenames)
    if len(chunks) > 1 and cores >= 2:
      # We can parallelize
      if DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks, using %d cores  (total: %.2f MB)' % (len(chunks), cores, len(js)/(1024*1024.))
      pool = multiprocessing.Pool(processes=cores)
      filenames = pool.map(run_on_chunk, commands, chunksize=1)
    else:
      # We can't parallize, but still break into chunks to avoid uglify/node memory issues
      if len(chunks) > 1 and DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks' % (len(chunks))
      filenames = [run_on_chunk(command) for command in commands]
  else:
    filenames = []

  filename += '.jo.js'
  f = open(filename, 'w')
  for out_file in filenames:
    f.write(open(out_file).read())
  if jcache:
    for cached in cached_outputs:
      f.write(cached); # TODO: preserve order
  f.write(suffix)
  f.write('\n')
  f.close()

  if jcache:
    # save chunks to cache
    for i in range(len(chunks)):
      chunk = chunks[i]
      keys = [chunk]
      shortkey = shared.JCache.get_shortkey(keys)
      shared.JCache.set(shortkey, keys, outputs[i])
    if out and DEBUG and len(chunks) > 0: print >> sys.stderr, '  saving %d funcchunks to jcache' % len(chunks)

  return filename

