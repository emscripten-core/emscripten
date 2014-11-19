import os.path, sys, shutil, hashlib, cPickle, zlib, time

import tempfiles

# Permanent cache for dlmalloc and stdlibc++
class Cache:
  def __init__(self, dirname=None, debug=False):
    if dirname is None:
      dirname = os.environ.get('EM_CACHE')
    if not dirname:
      dirname = os.path.expanduser(os.path.join('~', '.emscripten_cache'))
    self.dirname = dirname
    self.debug = debug

  def ensure(self):
    shared.safe_ensure_dirs(self.dirname)

  def erase(self):
    tempfiles.try_delete(self.dirname)
    try:
      open(self.dirname + '__last_clear', 'w').write('last clear: ' + time.asctime() + '\n')
    except Exception, e:
      print >> sys.stderr, 'failed to save last clear time: ', e

  def get_path(self, shortname):
    return os.path.join(self.dirname, shortname)

  # Request a cached file. If it isn't in the cache, it will be created with
  # the given creator function
  def get(self, shortname, creator, extension='.bc'):
    if not shortname.endswith(extension): shortname += extension
    cachename = os.path.join(self.dirname, shortname)
    if os.path.exists(cachename):
      return cachename
    self.ensure()
    temp = creator()
    if temp != cachename:
      shutil.copyfile(temp, cachename)
    return cachename

# JS-specific cache. We cache the results of compilation and optimization,
# so that in incremental builds we can just load from cache.
# We cache reasonably-large-sized chunks
class JCache:
  def __init__(self, cache):
    self.cache = cache
    self.dirname = os.path.join(cache.dirname, 'jcache')
    self.debug = cache.debug

  def ensure(self):
    self.cache.ensure()
    shared.safe_ensure_dirs(self.dirname)

  def get_shortkey(self, keys):
    if type(keys) not in [list, tuple]:
      keys = [keys]
    ret = ''
    for key in keys:
      assert type(key) == str
      ret += hashlib.md5(key).hexdigest()
    return ret

  def get_cachename(self, shortkey):
    return os.path.join(self.dirname, shortkey)

  # Returns a cached value, if it exists. Make sure the full key matches
  def get(self, shortkey, keys):
    if self.debug: print >> sys.stderr, 'jcache get?', shortkey
    cachename = self.get_cachename(shortkey)
    if not os.path.exists(cachename):
      if self.debug: print >> sys.stderr, 'jcache none at all'
      return
    try:
      data = cPickle.loads(zlib.decompress(open(cachename).read()))
    except Exception, e:
      if self.debug: print >> sys.stderr, 'jcache decompress/unpickle error:', e
      return
    if len(data) != 2:
      if self.debug: print >> sys.stderr, 'jcache error in get'
      return
    oldkeys = data[0]
    if len(oldkeys) != len(keys):
      if self.debug: print >> sys.stderr, 'jcache collision (a)'
      return
    for i in range(len(oldkeys)):
      if oldkeys[i] != keys[i]:
        if self.debug: print >> sys.stderr, 'jcache collision (b)'
        return
    if self.debug: print >> sys.stderr, 'jcache win'
    return data[1]

  # Sets the cached value for a key (from get_key)
  def set(self, shortkey, keys, value):
    cachename = self.get_cachename(shortkey)
    try:
      f = open(cachename, 'w')
      f.write(zlib.compress(cPickle.dumps([keys, value])))
      f.close()
    except Exception, e:
      if self.debug: print >> sys.stderr, 'jcache compress/pickle error:', e
      return
    #  for i in range(len(keys)):
    #    open(cachename + '.key' + str(i), 'w').write(keys[i])
    #  open(cachename + '.value', 'w').write(value)

# Given a set of functions of form (ident, text), and a preferred chunk size,
# generates a set of chunks for parallel processing and caching.
# It is very important to generate similar chunks in incremental builds, in
# order to maximize the chance of cache hits. To achieve that, we save the
# chunking used in the previous compilation of this phase, and we try to
# generate the same chunks, barring big differences in function sizes that
# violate our chunk size guideline. If caching is not used, chunking_file
# should be None
def chunkify(funcs, chunk_size, chunking_file, DEBUG=False):
  previous_mapping = None
  if chunking_file:
    chunking_file = chunking_file
    if os.path.exists(chunking_file):
      try:
        previous_mapping = cPickle.Unpickler(open(chunking_file, 'rb')).load() # maps a function identifier to the chunk number it will be in
        #if DEBUG: print >> sys.stderr, 'jscache previous mapping', previous_mapping
      except:
        pass
  chunks = []
  if previous_mapping:
    # initialize with previous chunking
    news = []
    for func in funcs:
      ident, data = func
      assert ident, 'need names for jcache chunking'
      if not ident in previous_mapping:
        news.append(func)
      else:
        n = previous_mapping[ident]
        while n >= len(chunks): chunks.append([])
        chunks[n].append(func)
    if DEBUG: print >> sys.stderr, 'jscache not in previous chunking', len(news)
    # add news and adjust for new sizes
    spilled = news
    for i in range(len(chunks)):
      chunk = chunks[i]
      size = sum([len(func[1]) for func in chunk])
      #if DEBUG: print >> sys.stderr, 'need spilling?', i, size, len(chunk), 'vs', chunk_size, 1.5*chunk_size
      while size > 1.5*chunk_size and len(chunk) > 1:
        spill = chunk.pop()
        spilled.append(spill)
        size -= len(spill[1])
    #if DEBUG: print >> sys.stderr, 'jscache new + spilled', len(spilled)
    for chunk in chunks:
      size = sum([len(func[1]) for func in chunk])
      while size < 0.66*chunk_size and len(spilled) > 0:
        spill = spilled.pop()
        chunk.append(spill)
        size += len(spill[1])
    chunks = filter(lambda chunk: len(chunk) > 0, chunks) # might have empty ones, eliminate them
    funcs = spilled # we will allocate these into chunks as if they were normal inputs
    #if DEBUG: print >> sys.stderr, 'leftover spills', len(spilled)
  # initialize reasonably, the rest of the funcs we need to split out
  curr = []
  total_size = 0
  for i in range(len(funcs)):
    func = funcs[i]
    curr_size = len(func[1])
    if total_size + curr_size < chunk_size:
      curr.append(func)
      total_size += curr_size
    else:
      chunks.append(curr)
      curr = [func]
      total_size = curr_size
  if curr:
    chunks.append(curr)
    curr = None
  if chunking_file:
    # sort within each chunk, to keep the order identical
    for chunk in chunks:
      chunk.sort(key=lambda func: func[0])
    # save new mapping info
    new_mapping = {}
    for i in range(len(chunks)):
      chunk = chunks[i]
      for ident, data in chunk:
        assert ident not in new_mapping, 'cannot have duplicate names in jcache chunking'
        new_mapping[ident] = i
    cPickle.Pickler(open(chunking_file, 'wb')).dump(new_mapping)
    #if DEBUG:
    #  for i in range(len(chunks)):
    #    chunk = chunks[i]
    #    print >> sys.stderr, 'final chunk', i, len(chunk)
    #  print >> sys.stderr, 'new mapping:', new_mapping
    #  if previous_mapping:
    #    for ident in set(previous_mapping.keys() + new_mapping.keys()):
    #      if previous_mapping.get(ident) != new_mapping.get(ident):
    #        print >> sys.stderr, 'mapping inconsistency', ident, previous_mapping.get(ident), new_mapping.get(ident)
  return [''.join([func[1] for func in chunk]) for chunk in chunks] # remove function names

import shared

