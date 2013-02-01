
import os, sys, subprocess, multiprocessing, re
import shared

configuration = shared.configuration
temp_files = configuration.get_temp_files()

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

def run_on_js(filename, passes, js_engine, jcache):

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
    # if there is metadata, we will run only on the generated functions. If there isn't, we will run on everything.
    generated = set(eval(suffix[len(suffix_marker)+1:]))

  if not suffix and jcache:
    # JCache cannot be used without metadata, since it might reorder stuff, and that's dangerous since only generated can be reordered
    # This means jcache does not work after closure compiler runs, for example. But you won't get much benefit from jcache with closure
    # anyhow (since closure is likely the longest part of the build).
    if DEBUG: print >>sys.stderr, 'js optimizer: no metadata, so disabling jcache'
    jcache = False

  # If we process only generated code, find that and save the rest on the side
  func_sig = re.compile('( *)function (_[\w$]+)\(')
  if suffix:
    pos = 0
    gen_start = 0
    gen_end = 0
    while 1:
      m = func_sig.search(js, pos)
      if not m: break
      pos = m.end()
      indent = m.group(1)
      ident = m.group(2)
      if ident in generated:
        if not gen_start:
          gen_start = m.start()
          assert gen_start
        gen_end = js.find('\n%s}\n' % indent, m.end()) + (3 + len(indent))
    assert gen_end > gen_start
    pre = js[:gen_start]
    post = js[gen_end:]
    js = js[gen_start:gen_end]
  else:
    pre = ''
    post = ''

  # Pick where to split into chunks, so that (1) they do not oom in node/uglify, and (2) we can run them in parallel
  # If we have metadata, we split only the generated code, and save the pre and post on the side (and do not optimize them)
  parts = map(lambda part: part, js.split('\n}\n'))
  funcs = []
  for i in range(len(parts)):
    func = parts[i]
    if i < len(parts)-1: func += '\n}\n' # last part needs no }
    m = func_sig.search(func)
    if m:
      ident = m.group(1)
    else:
      if suffix: continue # ignore whitespace
      ident = 'anon_%d' % i
    funcs.append((ident, func))
  parts = None
  total_size = len(js)
  js = None

  chunks = shared.chunkify(funcs, BEST_JS_PROCESS_SIZE, jcache.get_cachename('jsopt') if jcache else None)

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
      if DEBUG: print >> sys.stderr, '  loading %d jsfuncchunks from jcache' % len(cached_outputs)
    else:
      cached_outputs = []

  if len(chunks) > 0:
    def write_chunk(chunk, i):
      temp_file = temp_files.get('.jsfunc_%d.js' % i).name
      f = open(temp_file, 'w')
      f.write(chunk)
      f.write(suffix)
      f.close()
      return temp_file
    filenames = [write_chunk(chunks[i], i) for i in range(len(chunks))]
  else:
    filenames = []

  if len(filenames) > 0:
    # XXX Use '--nocrankshaft' to disable crankshaft to work around v8 bug 1895, needed for older v8/node (node 0.6.8+ should be ok)
    commands = map(lambda filename: [js_engine, JS_OPTIMIZER, filename, 'noPrintMetadata'] + passes, filenames)
    #print [' '.join(command) for command in commands]

    cores = min(multiprocessing.cpu_count(), filenames)
    if len(chunks) > 1 and cores >= 2:
      # We can parallelize
      if DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks, using %d cores  (total: %.2f MB)' % (len(chunks), cores, total_size/(1024*1024.))
      pool = multiprocessing.Pool(processes=cores)
      filenames = pool.map(run_on_chunk, commands, chunksize=1)
    else:
      # We can't parallize, but still break into chunks to avoid uglify/node memory issues
      if len(chunks) > 1 and DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks' % (len(chunks))
      filenames = [run_on_chunk(command) for command in commands]
  else:
    filenames = []

  for filename in filenames: temp_files.note(filename)

  filename += '.jo.js'
  f = open(filename, 'w')
  f.write(pre);
  for out_file in filenames:
    f.write(open(out_file).read())
    f.write('\n')
  if jcache:
    for cached in cached_outputs:
      f.write(cached); # TODO: preserve order
      f.write('\n')
  f.write(post);
  # No need to write suffix: if there was one, it is inside post which exists when suffix is there
  f.write('\n')
  f.close()

  if jcache:
    # save chunks to cache
    for i in range(len(chunks)):
      chunk = chunks[i]
      keys = [chunk]
      shortkey = shared.JCache.get_shortkey(keys)
      shared.JCache.set(shortkey, keys, open(filenames[i]).read())
    if DEBUG and len(chunks) > 0: print >> sys.stderr, '  saving %d jsfuncchunks to jcache' % len(chunks)

  return filename

def run(filename, passes, js_engine, jcache):
  return temp_files.run_and_clean(lambda: run_on_js(filename, passes, js_engine, jcache))

