
import os, sys, subprocess, multiprocessing, re, string, json
import shared

configuration = shared.configuration
temp_files = configuration.get_temp_files()

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

JS_OPTIMIZER = path_from_root('tools', 'js-optimizer.js')

NUM_CHUNKS_PER_CORE = 1.5
MIN_CHUNK_SIZE = int(os.environ.get('EMCC_JSOPT_MIN_CHUNK_SIZE') or 1024*1024) # configuring this is just for debugging purposes
MAX_CHUNK_SIZE = 20*1024*1024

WINDOWS = sys.platform.startswith('win')

DEBUG = os.environ.get('EMCC_DEBUG')

func_sig = re.compile('( *)function ([_\w$]+)\(')

class Minifier:
  '''
    asm.js minification support. We calculate possible names and minification of
    globals here, then pass that into the parallel js-optimizer.js runners which
    during registerize perform minification of locals.
  '''

  def __init__(self, js, js_engine):
    self.js = js
    self.js_engine = js_engine

    # Create list of valid short names

    MAX_NAMES = 80000
    INVALID_2 = set(['do', 'if', 'in'])
    INVALID_3 = set(['for', 'new', 'try', 'var', 'env'])

    self.names = []
    init_possibles = string.ascii_letters + '_$'
    later_possibles = init_possibles + string.digits
    for a in init_possibles:
      if len(self.names) >= MAX_NAMES: break
      self.names.append(a)
    for a in init_possibles:
      for b in later_possibles:
        if len(self.names) >= MAX_NAMES: break
        curr = a + b
        if curr not in INVALID_2: self.names.append(curr)
    for a in init_possibles:
      for b in later_possibles:
        for c in later_possibles:
          if len(self.names) >= MAX_NAMES: break
          curr = a + b + c
          if curr not in INVALID_3: self.names.append(curr)
    #print >> sys.stderr, self.names

  def minify_shell(self, shell, compress):
    #print >> sys.stderr, "MINIFY SHELL 1111111111", shell, "\n222222222222222"
    # Run through js-optimizer.js to find and minify the global symbols
    # We send it the globals, which it parses at the proper time. JS decides how
    # to minify all global names, we receive a dictionary back, which is then
    # used by the function processors

    shell = shell.replace('0.0', '13371337') # avoid uglify doing 0.0 => 0

    # Find all globals in the JS functions code
    self.globs = [m.group(2) for m in func_sig.finditer(self.js)]

    temp_file = temp_files.get('.minifyglobals.js').name
    f = open(temp_file, 'w')
    f.write(shell)
    f.write('\n')
    self
    f.write('// MINIFY_INFO:' + self.serialize())
    f.close()

    output = subprocess.Popen(self.js_engine + [JS_OPTIMIZER, temp_file, 'minifyGlobals', 'noPrintMetadata'] + (['compress'] if compress else []), stdout=subprocess.PIPE).communicate()[0]
    assert len(output) > 0 and not output.startswith('Assertion failed'), 'Error in js optimizer: ' + output
    #print >> sys.stderr, "minified SHELL 3333333333333333", output, "\n44444444444444444444"
    code, metadata = output.split('// MINIFY_INFO:')
    self.globs = json.loads(metadata)
    return code.replace('13371337', '0.0')


  def serialize(self):
    return json.dumps({
      'names': self.names,
      'globals': self.globs
    })

def run_on_chunk(command):
  filename = command[2] # XXX hackish
  #print >> sys.stderr, 'running js optimizer command', ' '.join(command), '""""', open(filename).read()
  output = subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0]
  assert len(output) > 0 and not output.startswith('Assertion failed'), 'Error in js optimizer: ' + output
  filename = temp_files.get(os.path.basename(filename) + '.jo.js').name
  f = open(filename, 'w')
  f.write(output)
  f.close()
  if DEBUG and not shared.WINDOWS: print >> sys.stderr, '.' # Skip debug progress indicator on Windows, since it doesn't buffer well with multiple threads printing to console.
  return filename

def run_on_js(filename, passes, js_engine, jcache):
  if isinstance(jcache, bool) and jcache: jcache = shared.JCache
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
    suffix_end = js.find('\n', suffix_start)
    suffix = js[suffix_start:suffix_end] + '\n'
    # if there is metadata, we will run only on the generated functions. If there isn't, we will run on everything.
    generated = set(eval(suffix[len(suffix_marker)+1:]))

  # Find markers
  start_funcs_marker = '// EMSCRIPTEN_START_FUNCS\n'
  end_funcs_marker = '// EMSCRIPTEN_END_FUNCS\n'
  start_funcs = js.find(start_funcs_marker)
  end_funcs = js.rfind(end_funcs_marker)
  #assert (start_funcs >= 0) == (end_funcs >= 0) == (not not suffix)
  asm_registerize = 'asm' in passes and 'registerize' in passes
  if asm_registerize:
    start_asm_marker = '// EMSCRIPTEN_START_ASM\n'
    end_asm_marker = '// EMSCRIPTEN_END_ASM\n'
    start_asm = js.find(start_asm_marker)
    end_asm = js.rfind(end_asm_marker)
    assert (start_asm >= 0) == (end_asm >= 0)

  if not suffix and jcache:
    # JCache cannot be used without metadata, since it might reorder stuff, and that's dangerous since only generated can be reordered
    # This means jcache does not work after closure compiler runs, for example. But you won't get much benefit from jcache with closure
    # anyhow (since closure is likely the longest part of the build).
    if DEBUG: print >>sys.stderr, 'js optimizer: no metadata, so disabling jcache'
    jcache = False

  if suffix:
    if not asm_registerize:
      pre = js[:start_funcs + len(start_funcs_marker)]
      post = js[end_funcs + len(end_funcs_marker):]
      js = js[start_funcs + len(start_funcs_marker):end_funcs]
      if 'asm' not in passes: # can have Module[..] and inlining prevention code, push those to post
        class Finals:
          buf = []
        def process(line):
          if len(line) > 0 and (line.startswith(('Module[', 'if (globalScope)')) or line.endswith('["X"]=1;')):
            Finals.buf.append(line)
            return False
          return True
        js = '\n'.join(filter(process, js.split('\n')))
        post = '\n'.join(Finals.buf) + '\n' + post
      post = end_funcs_marker + post
    else:
      # We need to split out the asm shell as well, for minification
      pre = js[:start_asm + len(start_asm_marker)]
      post = js[end_asm:]
      asm_shell = js[start_asm + len(start_asm_marker):start_funcs + len(start_funcs_marker)] + '''
EMSCRIPTEN_FUNCS();
''' + js[end_funcs + len(end_funcs_marker):end_asm + len(end_asm_marker)]
      js = js[start_funcs + len(start_funcs_marker):end_funcs]

      minifier = Minifier(js, js_engine)
      asm_shell_pre, asm_shell_post = minifier.minify_shell(asm_shell, 'compress' in passes).split('EMSCRIPTEN_FUNCS();');
      asm_shell_post = asm_shell_post.replace('});', '})');
      pre += asm_shell_pre + '\n' + start_funcs_marker
      post = end_funcs_marker + asm_shell_post + post

      minify_info = minifier.serialize()
      #if DEBUG: print >> sys.stderr, 'minify info:', minify_info
    # remove suffix if no longer needed
    if 'last' in passes:
      suffix_start = post.find(suffix_marker)
      suffix_end = post.find('\n', suffix_start)
      post = post[:suffix_start] + post[suffix_end:]

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
      ident = m.group(2)
    else:
      if suffix: continue # ignore whitespace
      ident = 'anon_%d' % i
    assert ident
    funcs.append((ident, func))
  parts = None
  total_size = len(js)
  js = None

  cores = int(os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())
  intended_num_chunks = int(round(cores * NUM_CHUNKS_PER_CORE))
  chunk_size = min(MAX_CHUNK_SIZE, max(MIN_CHUNK_SIZE, total_size / intended_num_chunks))

  chunks = shared.chunkify(funcs, chunk_size, jcache.get_cachename('jsopt') if jcache else None)

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
      f.write(suffix_marker)
      if asm_registerize:
        f.write('\n')
        f.write('// MINIFY_INFO:' + minify_info)
      f.close()
      return temp_file
    filenames = [write_chunk(chunks[i], i) for i in range(len(chunks))]
  else:
    filenames = []

  if len(filenames) > 0:
    # XXX Use '--nocrankshaft' to disable crankshaft to work around v8 bug 1895, needed for older v8/node (node 0.6.8+ should be ok)
    commands = map(lambda filename: js_engine + [JS_OPTIMIZER, filename, 'noPrintMetadata'] + passes, filenames)
    #print [' '.join(command) for command in commands]

    cores = min(cores, filenames)
    if len(chunks) > 1 and cores >= 2:
      # We can parallelize
      if DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks of size %d, using %d cores  (total: %.2f MB)' % (len(chunks), chunk_size, cores, total_size/(1024*1024.))
      pool = multiprocessing.Pool(processes=cores)
      filenames = pool.map(run_on_chunk, commands, chunksize=1)
    else:
      # We can't parallize, but still break into chunks to avoid uglify/node memory issues
      if len(chunks) > 1 and DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks of size %d' % (len(chunks), chunk_size)
      filenames = [run_on_chunk(command) for command in commands]
  else:
    filenames = []

  for filename in filenames: temp_files.note(filename)

  if 'closure' in passes:
    # run closure on the shell code, everything but what we js-optimize
    start_asm = '// EMSCRIPTEN_START_ASM\n'
    end_asm = '// EMSCRIPTEN_END_ASM\n'
    closure_sep = 'wakaUnknownBefore(); var asm=wakaUnknownAfter(global,env,buffer)\n'

    closuree = temp_files.get('.closure.js').name
    c = open(closuree, 'w')
    pre_1, pre_2 = pre.split(start_asm)
    post_1, post_2 = post.split(end_asm)
    c.write(pre_1)
    c.write(closure_sep)
    c.write(post_2)
    c.close()
    closured = shared.Building.closure_compiler(closuree, pretty='compress' not in passes)
    temp_files.note(closured)
    coutput = open(closured).read()
    coutput = coutput.replace('wakaUnknownBefore();', '')
    after = 'wakaUnknownAfter'
    start = coutput.find(after)
    end = coutput.find(')', start)
    pre = coutput[:start] + '(function(global,env,buffer) {\n' + start_asm + pre_2[pre_2.find('{')+1:]
    post = post_1[:post_1.rfind('}')] + '\n' + end_asm + '\n})' + coutput[end+1:]

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

