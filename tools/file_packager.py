'''
A tool that generates FS API calls to generate a filesystem, and packages the files
to work with that.

This is called by emcc. You can also call it yourself.

You can split your files into "asset bundles", and create each bundle separately
with this tool. Then just include the generated js for each and they will load
the data and prepare it accordingly. This allows you to share assets and reduce
data downloads.

Usage:

  file_packager.py TARGET [--preload A [B..]] [--embed C [D..]] [--compress COMPRESSION_DATA] [--pre-run] [--crunch[=X]]

  --pre-run Will generate wrapper code that does preloading in Module.preRun. This is necessary if you add this
            code before the main file has been loading, which includes necessary components like addRunDependency.

  --crunch=X Will compress dxt files to crn with quality level X. The crunch commandline tool must be present
             and CRUNCH should be defined in ~/.emscripten that points to it. JS crunch decompressing code will
             be added to convert the crn to dds in the browser.
             crunch-worker.js will be generated in the current directory. You should include that file when
             packaging your site.
             DDS files will not be crunched if the .crn is more recent than the .dds. This prevents a lot of
             unneeded computation.

Notes:

  * The file packager generates unix-style file paths. So if you are on windows and a file is accessed at
    subdir\file, in JS it will be subdir/file. For simplicity we treat the web platform as a *NIX.

TODO:        You can also provide .crn files yourself, pre-crunched. With this option, they will be decompressed
             to dds files in the browser, exactly the same as if this tool compressed them.
'''

import os, sys, shutil, random

from shared import Compression, execute, suffix, unsuffixed
import shared
from subprocess import Popen, PIPE, STDOUT

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
pre_run = False
crunch = 0
plugins = []

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
  elif arg == '--pre-run':
    pre_run = True
    in_preload = False
    in_embed = False
    in_compress = 0
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
  elif in_preload:
    data_files.append({ 'name': arg, 'mode': 'preload' })
  elif in_embed:
    data_files.append({ 'name': arg, 'mode': 'embed' })
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

print '''
(function() {
'''

code = '''
function assert(check, msg) {
  if (!check) throw msg + new Error().stack;
}
'''

# Expand directories into individual files
def add(mode, dirname, names):
  for name in names:
    fullname = os.path.join(dirname, name)
    if not os.path.isdir(fullname):
      data_files.append({ 'name': fullname, 'mode': mode })

for file_ in data_files:
  if os.path.isdir(file_['name']):
    os.path.walk(file_['name'], add, file_['mode'])
data_files = filter(lambda file_: not os.path.isdir(file_['name']), data_files)

for file_ in data_files:
  file_['name'] = file_['name'].replace(os.path.sep, '/') # name in the filesystem, native and emulated
  file_['localname'] = file_['name'] # name to actually load from local filesystem, after transformations

# Remove duplicates (can occur naively, for example preload dir/, preload dir/subdir/)
seen = {}
def was_seen(name):
  if seen.get(name): return True
  seen[name] = 1
  return False
data_files = filter(lambda file_: not was_seen(file_['name']), data_files)

if AV_WORKAROUND:
  random.shuffle(data_files)

# Apply plugins
for file_ in data_files:
  for plugin in plugins:
    plugin(file_)

# Crunch files
if crunch:
  shutil.copyfile(shared.path_from_root('tools', 'crunch-worker.js'), 'crunch-worker.js')
  print '''
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
        data: data,
        callbackID: decrunchCallbacks.length
      });
      decrunchCallbacks.push(callback);
    }
'''

  for file_ in data_files:
    if file_['name'].endswith(CRUNCH_INPUT_SUFFIX):
      # Do not crunch if crunched version exists and is more recent than dds source
      crunch_name = unsuffixed(file_['name']) + CRUNCH_OUTPUT_SUFFIX
      file_['localname'] = crunch_name
      try:
        crunch_time = os.stat(crunch_name).st_mtime
        dds_time = os.stat(file_['name']).st_mtime
        if dds_time < crunch_time: continue
      except:
        pass # if one of them does not exist, continue on

      # guess at format. this lets us tell crunch to not try to be clever and use odd formats like DXT5_AGBR
      try:
        format = Popen(['file', file_['name']], stdout=PIPE).communicate()[0]
        if 'DXT5' in format:
          format = ['-dxt5']
        elif 'DXT1' in format:
          format = ['-dxt1']
        else:
          raise Exception('unknown format')
      except:
        format = []
      Popen([CRUNCH, '-file', file_['name'], '-quality', crunch] + format, stdout=sys.stderr).communicate()
      #if not os.path.exists(os.path.basename(crunch_name)):
      #  print >> sys.stderr, 'Failed to crunch, perhaps a weird dxt format? Looking for a source PNG for the DDS'
      #  Popen([CRUNCH, '-file', unsuffixed(file_['name']) + '.png', '-quality', crunch] + format, stdout=sys.stderr).communicate()
      assert os.path.exists(os.path.basename(crunch_name)), 'crunch failed to generate output'
      shutil.move(os.path.basename(crunch_name), crunch_name) # crunch places files in the current dir
      # prepend the dds header
      crunched = open(crunch_name, 'rb').read()
      c = open(crunch_name, 'wb')
      c.write(open(file_['name'], 'rb').read()[:DDS_HEADER_SIZE])
      c.write(crunched)
      c.close()

# Set up folders
partial_dirs = []
for file_ in data_files:
  dirname = os.path.dirname(file_['name'])
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
    curr = open(file_['localname'], 'rb').read()
    file_['data_end'] = start + len(curr)
    if AV_WORKAROUND: curr += '\x00'
    print >> sys.stderr, 'bundling', file_['name'], file_['localname'], file_['data_start'], file_['data_end']
    start += len(curr)
    data.write(curr)
  data.close()
  if Compression.on:
    Compression.compress(data_target)

  # Data requests - for getting a block of data out of the big archive - have a similar API to XHRs
  code += '''
    function DataRequest() {}
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.requests[name] = this;
      },
      send: function() {}
    };
  '''

counter = 0
for file_ in data_files:
  filename = file_['name']
  if file_['mode'] == 'embed':
    # Embed
    data = map(ord, open(file_['localname'], 'rb').read())
    str_data = ''
    chunk_size = 10240
    while len(data) > 0:
      chunk = data[:chunk_size]
      data = data[chunk_size:]
      if not str_data:
        str_data = str(chunk)
      else:
        str_data += '.concat(' + str(chunk) + ')'
    code += '''Module['FS_createDataFile']('/%s', '%s', %s, true, true);\n''' % (os.path.dirname(filename), os.path.basename(filename), str_data)
  elif file_['mode'] == 'preload':
    # Preload
    varname = 'filePreload%d' % counter
    counter += 1
    dds = crunch and filename.endswith(CRUNCH_INPUT_SUFFIX)

    prepare = ''
    finish = "Module['removeRunDependency']('fp %s');\n" % filename

    if dds:
      # decompress crunch format into dds
      prepare = '''
        var ddsHeader = byteArray.subarray(0, %(dds_header_size)d);
        requestDecrunch('%(filename)s', byteArray.subarray(%(dds_header_size)d), function(ddsData) {
          byteArray = new Uint8Array(ddsHeader.length + ddsData.length);
          byteArray.set(ddsHeader, 0);
          byteArray.set(ddsData, %(dds_header_size)d);
''' % { 'filename': filename, 'dds_header_size': DDS_HEADER_SIZE }

      finish += '''
        });
'''

    code += '''
    var %(varname)s = new %(request)s();
    %(varname)s.open('GET', '%(filename)s', true);
    %(varname)s.responseType = 'arraybuffer';
    %(varname)s.onload = function() {
      var arrayBuffer = %(varname)s.response;
      assert(arrayBuffer, 'Loading file %(filename)s failed.');
      var byteArray = !arrayBuffer.subarray ? new Uint8Array(arrayBuffer) : arrayBuffer;
      %(prepare)s
      Module['FS_createPreloadedFile']('/%(dirname)s', '%(basename)s', byteArray, true, true, function() {
        %(finish)s
      }%(fail)s);
    };
    Module['addRunDependency']('fp %(filename)s');
    %(varname)s.send(null);
''' % {
        'request': 'DataRequest', # In the past we also supported XHRs here
        'varname': varname,
        'filename': filename,
        'dirname': os.path.dirname(filename),
        'basename': os.path.basename(filename),
        'prepare': prepare,
        'finish': finish,
        'fail': '' if filename[-4:] not in AUDIO_SUFFIXES else ''', function() { Module['removeRunDependency']('fp %s') }''' % filename # workaround for chromium bug 124926 (still no audio with this, but at least we don't hang)
  }
  else:
    assert 0

if has_preloaded:
  # Get the big archive and split it up
  use_data = ''
  for file_ in data_files:
    if file_['mode'] == 'preload':
      use_data += '''
        curr = DataRequest.prototype.requests['%s'];
        curr.response = byteArray.subarray(%d,%d);
        curr.onload();
      ''' % (file_['name'], file_['data_start'], file_['data_end'])
  use_data += "          Module['removeRunDependency']('datafile_%s');\n" % data_target

  if Compression.on:
    use_data = '''
      Module["decompress"](byteArray, function(decompressed) {
        byteArray = new Uint8Array(decompressed);
        %s
      });
''' % use_data

  code += r'''
    if (!Module.expectedDataFileDownloads) {
      Module.expectedDataFileDownloads = 0;
      Module.finishedDataFileDownloads = 0;
    }
    Module.expectedDataFileDownloads++;

    var dataFile = new XMLHttpRequest();
    dataFile.onprogress = function(event) {
      var url = '%s';
      if (event.loaded && event.total) {
        if (!dataFile.addedTotal) {
          dataFile.addedTotal = true;
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
        Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
      } else if (!Module.dataFileDownloads) {
        Module['setStatus']('Downloading data...');
      }
    }
    dataFile.open('GET', '%s', true);
    dataFile.responseType = 'arraybuffer';
    dataFile.onload = function() {
      Module.finishedDataFileDownloads++;
      var arrayBuffer = dataFile.response;
      assert(arrayBuffer, 'Loading data file failed.');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      %s
    };
    Module['addRunDependency']('datafile_%s');
    dataFile.send(null);
    if (Module['setStatus']) Module['setStatus']('Downloading...');
  ''' % (data_target, os.path.basename(Compression.compressed_name(data_target) if Compression.on else data_target), use_data, data_target) # use basename because from the browser's point of view, we need to find the datafile in the same dir as the html file

if pre_run:
  print '''
  if (typeof Module == 'undefined') Module = {};
  if (!Module['preRun']) Module['preRun'] = [];
  Module["preRun"].push(function() {
'''

print code

if pre_run:
  print '  });\n'

if crunch:
  print '''
  if (!Module['postRun']) Module['postRun'] = [];
  Module["postRun"].push(function() {
    decrunchWorker.terminate();
  });
'''

print '''
})();
'''

