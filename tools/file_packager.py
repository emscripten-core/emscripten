'''
A tool that generates FS API calls to generate a filesystem, and packages the files
to work with that.

This is called by emcc. You can also call it yourself.

You can split your files into "asset bundles", and create each bundle separately
with this tool. Then just include the generated js for each and they will load
the data and prepare it accordingly. This allows you to share assets and reduce
data downloads.

Usage:

  file_packager.py TARGET [--preload A [B..]] [--embed C [D..]] [--compress COMPRESSION_DATA] [--crunch[=X]] [--js-output=OUTPUT.js] [--no-force] [--use-preload-cache] [--no-heap-copy]

  --crunch=X Will compress dxt files to crn with quality level X. The crunch commandline tool must be present
             and CRUNCH should be defined in ~/.emscripten that points to it. JS crunch decompressing code will
             be added to convert the crn to dds in the browser.
             crunch-worker.js will be generated in the current directory. You should include that file when
             packaging your site.
             DDS files will not be crunched if the .crn is more recent than the .dds. This prevents a lot of
             unneeded computation.

  --js-output=FILE Writes output in FILE, if not specified, standard output is used.

  --no-force Don't create output if no valid input file is specified.

  --use-preload-cache Stores package in IndexedDB so that subsequent loads don't need to do XHR. Checks package version.

  --no-heap-copy If specified, the preloaded filesystem is not copied inside the Emscripten HEAP, but kept in a separate typed array outside it.
                 The default, if this is not specified, is to embed the VFS inside the HEAP, so that mmap()ing files in it is a no-op.
                 Passing this flag optimizes for fread() usage, omitting it optimizes for mmap() usage.

Notes:

  * The file packager generates unix-style file paths. So if you are on windows and a file is accessed at
    subdir\file, in JS it will be subdir/file. For simplicity we treat the web platform as a *NIX.

TODO:        You can also provide .crn files yourself, pre-crunched. With this option, they will be decompressed
             to dds files in the browser, exactly the same as if this tool compressed them.
'''

import os, sys, shutil, random, uuid, ctypes
import posixpath
import shared
from shared import Compression, execute, suffix, unsuffixed
from subprocess import Popen, PIPE, STDOUT

if len(sys.argv) == 1:
  print '''Usage: file_packager.py TARGET [--preload A...] [--embed B...] [--compress COMPRESSION_DATA] [--crunch[=X]] [--js-output=OUTPUT.js] [--no-force] [--use-preload-cache] [--no-heap-copy]
See the source for more details.'''
  sys.exit(0)

DEBUG = os.environ.get('EMCC_DEBUG')

data_target = sys.argv[1]

IMAGE_SUFFIXES = ('.jpg', '.png', '.bmp')
AUDIO_SUFFIXES = ('.ogg', '.wav', '.mp3')
AUDIO_MIMETYPES = { 'ogg': 'audio/ogg', 'wav': 'audio/wav', 'mp3': 'audio/mpeg' }
CRUNCH_INPUT_SUFFIX = '.dds'
CRUNCH_OUTPUT_SUFFIX = '.crn'

DDS_HEADER_SIZE = 128

AV_WORKAROUND = 0 # Set to 1 to randomize file order and add some padding, to work around silly av false positives

data_files = []
in_preload = False
in_embed = False
has_preloaded = False
in_compress = 0
crunch = 0
plugins = []
jsoutput = None
force = True
# If set to True, IndexedDB (IDBFS in library_idbfs.js) is used to locally cache VFS XHR so that subsequent 
# page loads can read the data from the offline cache instead.
use_preload_cache = False
# If set to True, the blob received from XHR is moved to the Emscripten HEAP, optimizing for mmap() performance.
# If set to False, the XHR blob is kept intact, and fread()s etc. are performed directly to that data. This optimizes for minimal memory usage and fread() performance.
no_heap_copy = True

for arg in sys.argv[1:]:
  if arg == '--preload':
    in_preload = True
    in_embed = False
    has_preloaded = True
    in_compress = 0
  elif arg == '--embed':
    in_embed = True
    in_preload = False
    in_compress = 0
  elif arg == '--compress':
    Compression.on = True
    in_compress = 1
    in_preload = False
    in_embed = False
  elif arg == '--no-force':
    force = False
  elif arg == '--use-preload-cache':
    use_preload_cache = True
  elif arg == '--no-heap-copy':
    no_heap_copy = False
  elif arg.startswith('--js-output'):
    jsoutput = arg.split('=')[1] if '=' in arg else None
  elif arg.startswith('--crunch'):
    from shared import CRUNCH
    crunch = arg.split('=')[1] if '=' in arg else '128'
    in_preload = False
    in_embed = False
    in_compress = 0
  elif arg.startswith('--plugin'):
    plugin = open(arg.split('=')[1], 'r').read()
    eval(plugin) # should append itself to plugins
    in_preload = False
    in_embed = False
    in_compress = 0
  elif in_preload or in_embed:
    mode = 'preload'
    if in_embed:
      mode = 'embed'
    if '@' in arg:
      srcpath, dstpath = arg.split('@') # User is specifying destination filename explicitly.
    else:
      srcpath = dstpath = arg # Use source path as destination path.
    if os.path.isfile(srcpath) or os.path.isdir(srcpath):
      data_files.append({ 'srcpath': srcpath, 'dstpath': dstpath, 'mode': mode })
    else:
      print >> sys.stderr, 'Warning: ' + arg + ' does not exist, ignoring.'
  elif in_compress:
    if in_compress == 1:
      Compression.encoder = arg
      in_compress = 2
    elif in_compress == 2:
      Compression.decoder = arg
      in_compress = 3
    elif in_compress == 3:
      Compression.js_name = arg
      in_compress = 0

if (not force) and len(data_files) == 0:
  has_preloaded = False

ret = '''
var Module;
if (typeof Module === 'undefined') Module = eval('(function() { try { return Module || {} } catch(e) { return {} } })()');
if (!Module.expectedDataFileDownloads) {
  Module.expectedDataFileDownloads = 0;
  Module.finishedDataFileDownloads = 0;
}
Module.expectedDataFileDownloads++;
(function() {
'''

code = '''
function assert(check, msg) {
  if (!check) throw msg + new Error().stack;
}
'''

# Win32 code to test whether the given file has the hidden property set.
def has_hidden_attribute(filepath):
  if sys.platform != 'win32':
    return False
    
  try:
    attrs = ctypes.windll.kernel32.GetFileAttributesW(unicode(filepath))
    assert attrs != -1
    result = bool(attrs & 2)
  except (AttributeError, AssertionError):
    result = False
  return result

# The packager should never preload/embed any directories that have a component starting with '.' in them,
# or if the file is hidden (Win32). Note that this filter ONLY applies to directories. Explicitly specified single files
# are always preloaded/embedded, even if they start with a '.'.
def should_ignore(filename):
  if has_hidden_attribute(filename):
    return True
    
  components = filename.replace('\\\\', '/').replace('\\', '/').split('/')
  for c in components:
    if c.startswith('.') and c != '.' and c != '..':
      return True
  return False

# Expand directories into individual files
def add(arg, dirname, names):
  # rootpathsrc: The path name of the root directory on the local FS we are adding to emscripten virtual FS.
  # rootpathdst: The name we want to make the source path available on the emscripten virtual FS.
  mode, rootpathsrc, rootpathdst = arg
  for name in names:
    fullname = os.path.join(dirname, name)
    if not os.path.isdir(fullname):
      if should_ignore(fullname):
        if DEBUG:
          print >> sys.stderr, 'Skipping hidden file "' + fullname + '" from inclusion in the emscripten virtual file system.'
      else:
        dstpath = os.path.join(rootpathdst, os.path.relpath(fullname, rootpathsrc)) # Convert source filename relative to root directory of target FS.
        data_files.append({ 'srcpath': fullname, 'dstpath': dstpath, 'mode': mode })

for file_ in data_files:
  if os.path.isdir(file_['srcpath']):
    os.path.walk(file_['srcpath'], add, [file_['mode'], file_['srcpath'], file_['dstpath']])
data_files = filter(lambda file_: not os.path.isdir(file_['srcpath']), data_files)

# Absolutize paths, and check that they make sense
curr_abspath = os.path.abspath(os.getcwd())
for file_ in data_files:
  if file_['srcpath'] == file_['dstpath']:
    # This file was not defined with src@dst, so we inferred the destination from the source. In that case,
    # we require that the destination not be under the current location
    path = file_['dstpath']
    abspath = os.path.abspath(path)
    if DEBUG: print >> sys.stderr, path, abspath, curr_abspath
    if not abspath.startswith(curr_abspath):
      print >> sys.stderr, 'Error: Embedding "%s" which is below the current directory "%s". This is invalid since the current directory becomes the root that the generated code will see' % (path, curr_abspath)
      sys.exit(1)
    file_['dstpath'] = abspath[len(curr_abspath)+1:]
    if os.path.isabs(path):
      print >> sys.stderr, 'Warning: Embedding an absolute file/directory name "' + path + '" to the virtual filesystem. The file will be made available in the relative path "' + file_['dstpath'] + '". You can use the explicit syntax --preload-file srcpath@dstpath to explicitly specify the target location the absolute source path should be directed to.'

for file_ in data_files:
  file_['dstpath'] = file_['dstpath'].replace(os.path.sep, '/') # name in the filesystem, native and emulated
  if file_['dstpath'].endswith('/'): # If user has submitted a directory name as the destination but omitted the destination filename, use the filename from source file
    file_['dstpath'] = file_['dstpath'] + os.path.basename(file_['srcpath'])
  # make destination path always relative to the root
  file_['dstpath'] = posixpath.normpath(os.path.join('/', file_['dstpath']))
  if DEBUG:
    print >> sys.stderr, 'Packaging file "' + file_['srcpath'] + '" to VFS in path "' + file_['dstpath'] + '".'

# Remove duplicates (can occur naively, for example preload dir/, preload dir/subdir/)
seen = {}
def was_seen(name):
  if seen.get(name): return True
  seen[name] = 1
  return False
data_files = filter(lambda file_: not was_seen(file_['dstpath']), data_files)

if AV_WORKAROUND:
  random.shuffle(data_files)

# Apply plugins
for file_ in data_files:
  for plugin in plugins:
    plugin(file_)

# Crunch files
if crunch:
  shutil.copyfile(shared.path_from_root('tools', 'crunch-worker.js'), 'crunch-worker.js')
  ret += '''
    var decrunchWorker = new Worker('crunch-worker.js');
    var decrunchCallbacks = [];
    decrunchWorker.onmessage = function(msg) {
      decrunchCallbacks[msg.data.callbackID](msg.data.data);
      console.log('decrunched ' + msg.data.filename + ' in ' + msg.data.time + ' ms, ' + msg.data.data.length + ' bytes');
      decrunchCallbacks[msg.data.callbackID] = null;
    };
    function requestDecrunch(filename, data, callback) {
      decrunchWorker.postMessage({
        filename: filename,
        data: new Uint8Array(data),
        callbackID: decrunchCallbacks.length
      });
      decrunchCallbacks.push(callback);
    }
'''

  for file_ in data_files:
    if file_['dstpath'].endswith(CRUNCH_INPUT_SUFFIX):
      src_dds_name = file_['srcpath']
      src_crunch_name = unsuffixed(src_dds_name) + CRUNCH_OUTPUT_SUFFIX

      # Preload/embed the .crn version instead of the .dds version, but use the .dds suffix for the target file in the virtual FS.
      file_['srcpath'] = src_crunch_name

      try:
        # Do not crunch if crunched version exists and is more recent than dds source
        crunch_time = os.stat(src_crunch_name).st_mtime
        dds_time = os.stat(src_dds_name).st_mtime
        if dds_time < crunch_time: continue
      except:
        pass # if one of them does not exist, continue on

      # guess at format. this lets us tell crunch to not try to be clever and use odd formats like DXT5_AGBR
      try:
        format = Popen(['file', file_['srcpath']], stdout=PIPE).communicate()[0]
        if 'DXT5' in format:
          format = ['-dxt5']
        elif 'DXT1' in format:
          format = ['-dxt1']
        else:
          raise Exception('unknown format')
      except:
        format = []
      Popen([CRUNCH, '-outsamedir', '-file', src_dds_name, '-quality', crunch] + format, stdout=sys.stderr).communicate()
      #if not os.path.exists(os.path.basename(crunch_name)):
      #  print >> sys.stderr, 'Failed to crunch, perhaps a weird dxt format? Looking for a source PNG for the DDS'
      #  Popen([CRUNCH, '-file', unsuffixed(file_['srcpath']) + '.png', '-quality', crunch] + format, stdout=sys.stderr).communicate()
      assert os.path.exists(src_crunch_name), 'crunch failed to generate output'
      # prepend the dds header
      crunched = open(src_crunch_name, 'rb').read()
      c = open(src_crunch_name, 'wb')
      c.write(open(src_dds_name, 'rb').read()[:DDS_HEADER_SIZE])
      c.write(crunched)
      c.close()

# Set up folders
partial_dirs = []
for file_ in data_files:
  dirname = os.path.dirname(file_['dstpath'])
  dirname = dirname.lstrip('/') # absolute paths start with '/', remove that
  if dirname != '':
    parts = dirname.split('/')
    for i in range(len(parts)):
      partial = '/'.join(parts[:i+1])
      if partial not in partial_dirs:
        code += '''Module['FS_createPath']('/%s', '%s', true, true);\n''' % ('/'.join(parts[:i]), parts[i])
        partial_dirs.append(partial)

if has_preloaded:
  # Bundle all datafiles into one archive. Avoids doing lots of simultaneous XHRs which has overhead.
  data = open(data_target, 'wb')
  start = 0
  for file_ in data_files:
    file_['data_start'] = start
    curr = open(file_['srcpath'], 'rb').read()
    file_['data_end'] = start + len(curr)
    if AV_WORKAROUND: curr += '\x00'
    #print >> sys.stderr, 'bundling', file_['srcpath'], file_['dstpath'], file_['data_start'], file_['data_end']
    start += len(curr)
    data.write(curr)
  data.close()
  # TODO: sha256sum on data_target
  if Compression.on:
    Compression.compress(data_target)

  # Data requests - for getting a block of data out of the big archive - have a similar API to XHRs
  code += '''
    function DataRequest(start, end, crunched, audio) {
      this.start = start;
      this.end = end;
      this.crunched = crunched;
      this.audio = audio;
    }
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.name = name;
        this.requests[name] = this;
        Module['addRunDependency']('fp ' + this.name);
      },
      send: function() {},
      onload: function() {
        var byteArray = this.byteArray.subarray(this.start, this.end);
%s
          this.finish(byteArray);
%s
      },
      finish: function(byteArray) {
        var that = this;
        Module['FS_createPreloadedFile'](this.name, null, byteArray, true, true, function() {
          Module['removeRunDependency']('fp ' + that.name);
        }, function() {
          if (that.audio) {
            Module['removeRunDependency']('fp ' + that.name); // workaround for chromium bug 124926 (still no audio with this, but at least we don't hang)
          } else {
            Module.printErr('Preloading file ' + that.name + ' failed');
          }
        }, false, true); // canOwn this data in the filesystem, it is a slide into the heap that will never change
        this.requests[this.name] = null;
      },
    };
  ''' % ('' if not crunch else '''
        if (this.crunched) {
          var ddsHeader = byteArray.subarray(0, 128);
          var that = this;
          requestDecrunch(this.name, byteArray.subarray(128), function(ddsData) {
            byteArray = new Uint8Array(ddsHeader.length + ddsData.length);
            byteArray.set(ddsHeader, 0);
            byteArray.set(ddsData, 128);
            that.finish(byteArray);
          });
        } else {
''', '' if not crunch else '''
        }
''')

counter = 0
for file_ in data_files:
  filename = file_['dstpath']
  dirname = os.path.dirname(filename)
  basename = os.path.basename(filename)
  if file_['mode'] == 'embed':
    # Embed
    data = map(ord, open(file_['srcpath'], 'rb').read())
    if not data:
      str_data = '[]'
    else:
      str_data = ''
      chunk_size = 10240
      while len(data) > 0:
        chunk = data[:chunk_size]
        data = data[chunk_size:]
        if not str_data:
          str_data = str(chunk)
        else:
          str_data += '.concat(' + str(chunk) + ')'
    code += '''Module['FS_createDataFile']('%s', '%s', %s, true, true);\n''' % (dirname, basename, str_data)
  elif file_['mode'] == 'preload':
    # Preload
    varname = 'filePreload%d' % counter
    counter += 1
    code += '''    new DataRequest(%(start)d, %(end)d, %(crunched)s, %(audio)s).open('GET', '%(filename)s');
''' % {
      'filename': file_['dstpath'],
      'start': file_['data_start'],
      'end': file_['data_end'],
      'crunched': '1' if crunch and filename.endswith(CRUNCH_INPUT_SUFFIX) else '0',
      'audio': '1' if filename[-4:] in AUDIO_SUFFIXES else '0',
    }
  else:
    assert 0

if has_preloaded:
  # Get the big archive and split it up
  if no_heap_copy:
    use_data = '''
      // copy the entire loaded file into a spot in the heap. Files will refer to slices in that. They cannot be freed though.
      var ptr = Module['_malloc'](byteArray.length);
      Module['HEAPU8'].set(byteArray, ptr);
      DataRequest.prototype.byteArray = Module['HEAPU8'].subarray(ptr, ptr+byteArray.length);
'''
  else:
    use_data = '''
      // Reuse the bytearray from the XHR as the source for file reads.
      DataRequest.prototype.byteArray = byteArray;
'''
  for file_ in data_files:
    if file_['mode'] == 'preload':
      use_data += '          DataRequest.prototype.requests["%s"].onload();\n' % (file_['dstpath'])
  use_data += "          Module['removeRunDependency']('datafile_%s');\n" % data_target

  if Compression.on:
    use_data = '''
      Module["decompress"](byteArray, function(decompressed) {
        byteArray = new Uint8Array(decompressed);
        %s
      });
    ''' % use_data

  package_uuid = uuid.uuid4();
  remote_package_name = os.path.basename(Compression.compressed_name(data_target) if Compression.on else data_target)
  code += r'''
    var PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
    var PACKAGE_NAME = '%s';
    var REMOTE_PACKAGE_NAME = '%s';
    var PACKAGE_UUID = '%s';
  ''' % (data_target, remote_package_name, package_uuid)

  if use_preload_cache:
    code += r'''
      var indexedDB = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
      var IDB_RO = "readonly";
      var IDB_RW = "readwrite";
      var DB_NAME = 'EM_PRELOAD_CACHE';
      var DB_VERSION = 1;
      var METADATA_STORE_NAME = 'METADATA';
      var PACKAGE_STORE_NAME = 'PACKAGES';
      function openDatabase(callback, errback) {
        try {
          var openRequest = indexedDB.open(DB_NAME, DB_VERSION);
        } catch (e) {
          return errback(e);
        }
        openRequest.onupgradeneeded = function(event) {
          var db = event.target.result;

          if(db.objectStoreNames.contains(PACKAGE_STORE_NAME)) {
            db.deleteObjectStore(PACKAGE_STORE_NAME);
          }
          var packages = db.createObjectStore(PACKAGE_STORE_NAME);

          if(db.objectStoreNames.contains(METADATA_STORE_NAME)) {
            db.deleteObjectStore(METADATA_STORE_NAME);
          }
          var metadata = db.createObjectStore(METADATA_STORE_NAME);
        };
        openRequest.onsuccess = function(event) {
          var db = event.target.result;
          callback(db);
        };
        openRequest.onerror = function(error) {
          errback(error);
        };
      };

      /* Check if there's a cached package, and if so whether it's the latest available */
      function checkCachedPackage(db, packageName, callback, errback) {
        var transaction = db.transaction([METADATA_STORE_NAME], IDB_RO);
        var metadata = transaction.objectStore(METADATA_STORE_NAME);

        var getRequest = metadata.get(packageName);
        getRequest.onsuccess = function(event) {
          var result = event.target.result;
          if (!result) {
            return callback(false);
          } else {
            return callback(PACKAGE_UUID === result.uuid);
          }
        };
        getRequest.onerror = function(error) {
          errback(error);
        };
      };

      function fetchCachedPackage(db, packageName, callback, errback) {
        var transaction = db.transaction([PACKAGE_STORE_NAME], IDB_RO);
        var packages = transaction.objectStore(PACKAGE_STORE_NAME);

        var getRequest = packages.get(packageName);
        getRequest.onsuccess = function(event) {
          var result = event.target.result;
          callback(result);
        };
        getRequest.onerror = function(error) {
          errback(error);
        };
      };

      function cacheRemotePackage(db, packageName, packageData, packageMeta, callback, errback) {
        var transaction = db.transaction([PACKAGE_STORE_NAME, METADATA_STORE_NAME], IDB_RW);
        var packages = transaction.objectStore(PACKAGE_STORE_NAME);
        var metadata = transaction.objectStore(METADATA_STORE_NAME);

        var putPackageRequest = packages.put(packageData, packageName);
        putPackageRequest.onsuccess = function(event) {
          var putMetadataRequest = metadata.put(packageMeta, packageName);
          putMetadataRequest.onsuccess = function(event) {
            callback(packageData);
          };
          putMetadataRequest.onerror = function(error) {
            errback(error);
          };
        };
        putPackageRequest.onerror = function(error) {
          errback(error);
        };
      };
    '''

  ret += r'''
    function fetchRemotePackage(packageName, callback, errback) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', packageName, true);
      xhr.responseType = 'arraybuffer';
      xhr.onprogress = function(event) {
        var url = packageName;
        if (event.loaded && event.total) {
          if (!xhr.addedTotal) {
            xhr.addedTotal = true;
            if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
            Module.dataFileDownloads[url] = {
              loaded: event.loaded,
              total: event.total
            };
          } else {
            Module.dataFileDownloads[url].loaded = event.loaded;
          }
          var total = 0;
          var loaded = 0;
          var num = 0;
          for (var download in Module.dataFileDownloads) {
          var data = Module.dataFileDownloads[download];
            total += data.total;
            loaded += data.loaded;
            num++;
          }
          total = Math.ceil(total * Module.expectedDataFileDownloads/num);
          if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
        } else if (!Module.dataFileDownloads) {
          if (Module['setStatus']) Module['setStatus']('Downloading data...');
        }
      };
      xhr.onload = function(event) {
        var packageData = xhr.response;
        callback(packageData);
      };
      xhr.send(null);
    };

    function handleError(error) {
      console.error('package error:', error);
    };
  '''

  code += r'''
    function processPackageData(arrayBuffer) {
      Module.finishedDataFileDownloads++;
      assert(arrayBuffer, 'Loading data file failed.');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      %s
    };
    Module['addRunDependency']('datafile_%s');
  ''' % (use_data, data_target) # use basename because from the browser's point of view, we need to find the datafile in the same dir as the html file

  code += r'''
    if (!Module.preloadResults) Module.preloadResults = {};
  '''

  if use_preload_cache:
    code += r'''
      function preloadFallback(error) {
        console.error(error);
        console.error('falling back to default preload behavior');
        fetchRemotePackage(REMOTE_PACKAGE_NAME, processPackageData, handleError);
      };

      openDatabase(
        function(db) {
          checkCachedPackage(db, PACKAGE_PATH + PACKAGE_NAME,
            function(useCached) {
              Module.preloadResults[PACKAGE_NAME] = {fromCache: useCached};
              if (useCached) {
                console.info('loading ' + PACKAGE_NAME + ' from cache');
                fetchCachedPackage(db, PACKAGE_PATH + PACKAGE_NAME, processPackageData, preloadFallback);
              } else {
                console.info('loading ' + PACKAGE_NAME + ' from remote');
                fetchRemotePackage(REMOTE_PACKAGE_NAME,
                  function(packageData) {
                    cacheRemotePackage(db, PACKAGE_PATH + PACKAGE_NAME, packageData, {uuid:PACKAGE_UUID}, processPackageData,
                      function(error) {
                        console.error(error);
                        processPackageData(packageData);
                      });
                  }
                , preloadFallback);
              }
            }
          , preloadFallback);
        }
      , preloadFallback);

      if (Module['setStatus']) Module['setStatus']('Downloading...');
    '''
  else:
    # Not using preload cache, so we might as well start the xhr ASAP, potentially before JS parsing of the main codebase if it's after us.
    # Only tricky bit is the fetch is async, but also when runWithFS is called is async, so we handle both orderings.
    ret += r'''
      var fetched = null, fetchedCallback = null;
      fetchRemotePackage('%s', function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);
    ''' % os.path.basename(Compression.compressed_name(data_target) if Compression.on else data_target)

    code += r'''
      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }
    '''

ret += '''
  function runWithFS() {
'''
ret += code
ret += '''
  }
  if (Module['calledRun']) {
    runWithFS();
  } else {
    if (!Module['preRun']) Module['preRun'] = [];
    Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
  }
'''

if crunch:
  ret += '''
  if (!Module['postRun']) Module['postRun'] = [];
  Module["postRun"].push(function() {
    decrunchWorker.terminate();
  });
'''

ret += '''
})();
'''
if force or len(data_files) > 0:
  if jsoutput == None:
    print ret
  else:
    f = open(jsoutput, 'w')
    f.write(ret)
