#!/usr/bin/env python

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

import os, sys, json, optparse, subprocess, re, time, multiprocessing

if not os.environ.get('EMSCRIPTEN_SUPPRESS_USAGE_WARNING'):
  print >> sys.stderr, '''
==============================================================
WARNING: You should normally never use this! Use emcc instead.
==============================================================
  '''

from tools import shared

DEBUG = os.environ.get('EMCC_DEBUG')

__rootpath__ = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  """Returns the absolute path for which the given path elements are
  relative to the emscripten root.
  """
  return os.path.join(__rootpath__, *pathelems)

temp_files = shared.TempFiles()

compiler_engine = None
jcache = False

def scan(ll, settings):
  # blockaddress(@main, %23)
  blockaddrs = []
  for blockaddr in re.findall('blockaddress\([^)]*\)', ll):
    b = blockaddr.split('(')[1][:-1].split(', ')
    blockaddrs.append(b)
  if len(blockaddrs) > 0:
    settings['NECESSARY_BLOCKADDRS'] = blockaddrs

NUM_CHUNKS_PER_CORE = 5
MIN_CHUNK_SIZE = 1024*1024
MAX_CHUNK_SIZE = float(os.environ.get('EMSCRIPT_MAX_CHUNK_SIZE') or 'inf') # configuring this is just for debugging purposes

def process_funcs(args):
  i, ll, settings_file, compiler, forwarded_file, libraries = args
  funcs_file = temp_files.get('.func_%d.ll' % i).name
  open(funcs_file, 'w').write(ll)
  out = shared.run_js(compiler, compiler_engine, [settings_file, funcs_file, 'funcs', forwarded_file] + libraries, stdout=subprocess.PIPE, cwd=path_from_root('src'))
  shared.try_delete(funcs_file)
  return out.split('//FORWARDED_DATA:')

def emscript(infile, settings, outfile, libraries=[]):
  """Runs the emscripten LLVM-to-JS compiler. We parallelize as much as possible

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted settings that override the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """

  compiler = path_from_root('src', 'compiler.js')

  # Parallelization: We run 3 phases:
  #   1 aka 'pre'  : Process types and metadata and so forth, and generate the preamble.
  #   2 aka 'funcs': Process functions. We can parallelize this, working on each function independently.
  #   3 aka 'post' : Process globals, generate postamble and finishing touches.

  if DEBUG: print >> sys.stderr, 'emscript: ll=>js'

  if jcache: JCache.ensure()

  # Pre-scan ll and alter settings as necessary
  if DEBUG: t = time.time()
  ll = open(infile).read()
  scan(ll, settings)
  total_ll_size = len(ll)
  ll = None # allow collection
  if DEBUG: print >> sys.stderr, '  emscript: scan took %s seconds' % (time.time() - t)

  # Split input into the relevant parts for each phase
  pre = []
  funcs = [] # split up functions here, for parallelism later
  meta = [] # needed by each function XXX

  if DEBUG: t = time.time()
  in_func = False
  ll_lines = open(infile).readlines()
  for line in ll_lines:
    if in_func:
      funcs[-1].append(line)
      if line.startswith('}'):
        in_func = False
        funcs[-1] = ''.join(funcs[-1])
        pre.append(line) # pre needs it to, so we know about all implemented functions
    else:
      if line.startswith('define '):
        in_func = True
        funcs.append([line])
        pre.append(line) # pre needs it to, so we know about all implemented functions
      elif line.find(' = type { ') > 0:
        pre.append(line) # type
      elif line.startswith('!'):
        meta.append(line) # metadata
      else:
        pre.append(line) # pre needs it so we know about globals in pre and funcs. So emit globals there
  ll_lines = None
  meta = ''.join(meta)
  if DEBUG and len(meta) > 1024*1024: print >> sys.stderr, 'emscript warning: large amounts of metadata, will slow things down'
  if DEBUG: print >> sys.stderr, '  emscript: split took %s seconds' % (time.time() - t)

  #if DEBUG:
  #  print >> sys.stderr, '========= pre ================\n'
  #  print >> sys.stderr, ''.join(pre)
  #  print >> sys.stderr, '========== funcs ===============\n'
  #  for func in funcs:
  #    print >> sys.stderr, '\n// ===\n\n', ''.join(func)
  #  print >> sys.stderr, '=========================\n'

  # Save settings to a file to work around v8 issue 1579
  settings_file = temp_files.get('.txt').name
  settings_text = json.dumps(settings)
  s = open(settings_file, 'w')
  s.write(settings_text)
  s.close()

  # Phase 1 - pre
  if DEBUG: t = time.time()
  pre_file = temp_files.get('.pre.ll').name
  pre_input = ''.join(pre) + '\n' + meta
  out = None
  if jcache:
    keys = [pre_input, settings_text, ','.join(libraries)]
    shortkey = JCache.get_key(keys)
    out = JCache.get(shortkey, keys)
  if not out:
    open(pre_file, 'w').write(pre_input)
    out = shared.run_js(compiler, shared.COMPILER_ENGINE, [settings_file, pre_file, 'pre'] + libraries, stdout=subprocess.PIPE, cwd=path_from_root('src'))
    if jcache:
      JCache.set(shortkey, keys, out)
  pre, forwarded_data = out.split('//FORWARDED_DATA:')
  forwarded_file = temp_files.get('.json').name
  open(forwarded_file, 'w').write(forwarded_data)
  if DEBUG: print >> sys.stderr, '  emscript: phase 1 took %s seconds' % (time.time() - t)

  # Phase 2 - func

  cores = multiprocessing.cpu_count()
  assert cores >= 1
  if cores > 1:
    intended_num_chunks = cores * NUM_CHUNKS_PER_CORE
    chunk_size = max(MIN_CHUNK_SIZE, total_ll_size / intended_num_chunks)
    chunk_size += 3*len(meta) # keep ratio of lots of function code to meta (expensive to process, and done in each parallel task)
    chunk_size = min(MAX_CHUNK_SIZE, chunk_size)
  else:
    chunk_size = MAX_CHUNK_SIZE # if 1 core, just use the max chunk size

  if DEBUG: t = time.time()
  forwarded_json = json.loads(forwarded_data)
  indexed_functions = set()
  chunks = [] # bundles of functions
  curr = ''
  for i in range(len(funcs)):
    func = funcs[i]
    if len(curr) + len(func) < chunk_size:
      curr += func
    else:
      chunks.append(curr)
      curr = func
  if curr:
    chunks.append(curr)
    curr = ''
  if cores == 1 and total_ll_size < MAX_CHUNK_SIZE: assert len(chunks) == 1, 'no point in splitting up without multiple cores'
  if DEBUG: print >> sys.stderr, '  emscript: phase 2 working on %d chunks %s (intended chunk size: %.2f MB, meta: %.2f MB, forwarded: %.2f MB, total: %.2f MB)' % (len(chunks), ('using %d cores' % cores) if len(chunks) > 1 else '', chunk_size/(1024*1024.), len(meta)/(1024*1024.), len(forwarded_data)/(1024*1024.), total_ll_size/(1024*1024.))

  commands = [(i, chunks[i] + '\n' + meta, settings_file, compiler, forwarded_file, libraries) for i in range(len(chunks))]

  if len(chunks) > 1:
    pool = multiprocessing.Pool(processes=cores)
    outputs = pool.map(process_funcs, commands, chunksize=1)
  else:
    outputs = [process_funcs(commands[0])]

  funcs_js = ''.join([output[0] for output in outputs])

  for func_js, curr_forwarded_data in outputs:
    # merge forwarded data
    curr_forwarded_json = json.loads(curr_forwarded_data)
    forwarded_json['Types']['preciseI64MathUsed'] = forwarded_json['Types']['preciseI64MathUsed'] or curr_forwarded_json['Types']['preciseI64MathUsed']
    for key, value in curr_forwarded_json['Functions']['blockAddresses'].iteritems():
      forwarded_json['Functions']['blockAddresses'][key] = value
    for key in curr_forwarded_json['Functions']['indexedFunctions'].iterkeys():
      indexed_functions.add(key)
  outputs = None
  if DEBUG: print >> sys.stderr, '  emscript: phase 2 took %s seconds' % (time.time() - t)
  if DEBUG: t = time.time()

  # calculations on merged forwarded data
  forwarded_json['Functions']['indexedFunctions'] = {}
  i = 2
  for indexed in indexed_functions:
    forwarded_json['Functions']['indexedFunctions'][indexed] = i # make sure not to modify this python object later - we use it in indexize
    i += 2
  forwarded_json['Functions']['nextIndex'] = i

  indexing = forwarded_json['Functions']['indexedFunctions']
  def indexize(js):
    return re.sub(r'{{{ FI_([\w\d_$]+) }}}', lambda m: str(indexing[m.groups(0)[0]]), js)

  blockaddrs = forwarded_json['Functions']['blockAddresses']
  def blockaddrsize(js):
    return re.sub(r'{{{ BA_([\w\d_$]+)\|([\w\d_$]+) }}}', lambda m: str(blockaddrs[m.groups(0)[0]][m.groups(0)[1]]), js)

  if DEBUG: outfile.write('// pre\n')
  outfile.write(blockaddrsize(indexize(pre)))
  pre = None

  if DEBUG: outfile.write('// funcs\n')
  outfile.write(blockaddrsize(indexize(funcs_js)))
  funcs_js = None

  # forward
  forwarded_data = json.dumps(forwarded_json)
  forwarded_file = temp_files.get('.2.json').name
  open(forwarded_file, 'w').write(indexize(forwarded_data))
  if DEBUG: print >> sys.stderr, '  emscript: phase 2b took %s seconds' % (time.time() - t)

  # Phase 3 - post
  if DEBUG: t = time.time()
  post_file = temp_files.get('.post.ll').name
  open(post_file, 'w').write('\n') # no input, just processing of forwarded data
  out = shared.run_js(compiler, shared.COMPILER_ENGINE, [settings_file, post_file, 'post', forwarded_file] + libraries, stdout=subprocess.PIPE, cwd=path_from_root('src'))
  if DEBUG: outfile.write('// post\n')
  outfile.write(indexize(out))
  if DEBUG: print >> sys.stderr, '  emscript: phase 3 took %s seconds' % (time.time() - t)

  outfile.close()


def main(args):
  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    settings[name] = json.loads(value)

  # Add header defines to settings
  defines = {}
  include_root = path_from_root('system', 'include')
  headers = args.headers[0].split(',') if len(args.headers) > 0 else []
  seen_headers = set()
  while len(headers) > 0:
    header = headers.pop(0)
    if not os.path.isabs(header):
      header = os.path.join(include_root, header)
    seen_headers.add(header)
    for line in open(header, 'r'):
      line = line.replace('\t', ' ')
      m = re.match('^ *# *define +(?P<name>[-\w_.]+) +\(?(?P<value>[-\w_.|]+)\)?.*', line)
      if not m:
        # Catch enum defines of a very limited sort
        m = re.match('^ +(?P<name>[A-Z_\d]+) += +(?P<value>\d+).*', line)
      if m:
        if m.group('name') != m.group('value'):
          defines[m.group('name')] = m.group('value')
        #else:
        #  print 'Warning: %s #defined to itself' % m.group('name') # XXX this can happen if we are set to be equal to an enum (with the same name)
      m = re.match('^ *# *include *["<](?P<name>[\w_.-/]+)[">].*', line)
      if m:
        # Find this file
        found = False
        for w in [w for w in os.walk(include_root)]:
          for f in w[2]:
            curr = os.path.join(w[0], f)
            if curr.endswith(m.group('name')) and curr not in seen_headers:
              headers.append(curr)
              found = True
              break
          if found: break
        #assert found, 'Could not find header: ' + m.group('name')
  if len(defines) > 0:
    def lookup(value):
      try:
        while not unicode(value).isnumeric():
          value = defines[value]
        return value
      except:
        pass
      try: # 0x300 etc.
        value = eval(value)
        return value
      except:
        pass
      try: # CONST1|CONST2
        parts = map(lookup, value.split('|'))
        value = reduce(lambda a, b: a|b, map(eval, parts))
        return value
      except:
        pass
      return None
    for key, value in defines.items():
      value = lookup(value)
      if value is not None:
        defines[key] = str(value)
      else:
        del defines[key]
    #print >> sys.stderr, 'new defs:', str(defines).replace(',', ',\n  '), '\n\n'
    settings.setdefault('C_DEFINES', {}).update(defines)

  # libraries
  libraries = args.libraries[0].split(',') if len(args.libraries) > 0 else []

  # Compile the assembly to Javascript.
  if settings.get('RELOOP'): shared.Building.ensure_relooper()

  emscript(args.infile, settings, args.outfile, libraries)

if __name__ == '__main__':
  parser = optparse.OptionParser(
      usage='usage: %prog [-h] [-H HEADERS] [-o OUTFILE] [-c COMPILER_ENGINE] [-s FOO=BAR]* infile',
      description=('You should normally never use this! Use emcc instead. '
                   'This is a wrapper around the JS compiler, converting .ll to .js.'),
      epilog='')
  parser.add_option('-H', '--headers',
                    default=[],
                    action='append',
                    help='System headers (comma separated) whose #defines should be exposed to the compiled code.')
  parser.add_option('-L', '--libraries',
                    default=[],
                    action='append',
                    help='Library files (comma separated) to use in addition to those in emscripten src/library_*.')
  parser.add_option('-o', '--outfile',
                    default=sys.stdout,
                    help='Where to write the output; defaults to stdout.')
  parser.add_option('-c', '--compiler',
                    default=shared.COMPILER_ENGINE,
                    help='Which JS engine to use to run the compiler; defaults to the one in ~/.emscripten.')
  parser.add_option('-s', '--setting',
                    dest='settings',
                    default=[],
                    action='append',
                    metavar='FOO=BAR',
                    help=('Overrides for settings defined in settings.js. '
                          'May occur multiple times.'))
  parser.add_option('-j', '--jcache',
                    action='store_true',
                    default=False,
                    help=('Enable jcache (ccache-like caching of compilation results, for faster incremental builds).'))

  # Convert to the same format that argparse would have produced.
  keywords, positional = parser.parse_args()
  if len(positional) != 1:
    raise RuntimeError('Must provide exactly one positional argument.')
  keywords.infile = os.path.abspath(positional[0])
  if isinstance(keywords.outfile, basestring):
    keywords.outfile = open(keywords.outfile, 'w')
  compiler_engine = keywords.compiler
  jcache = keywords.jcache

  temp_files.run_and_clean(lambda: main(keywords))

