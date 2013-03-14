import os, sys, shutil, random
import argparse

import shared
from shared import Compression, execute, suffix, unsuffixed
from subprocess import Popen, PIPE, STDOUT

IMAGE_SUFFIXES = ('.jpg', '.png', '.bmp')
AUDIO_SUFFIXES = ('.ogg', '.wav', '.mp3')
AUDIO_MIMETYPES = { 'ogg': 'audio/ogg', 'wav': 'audio/wav', 'mp3': 'audio/mpeg' }
CRUNCH_INPUT_SUFFIX = '.dds'
CRUNCH_OUTPUT_SUFFIX = '.crn'

DDS_HEADER_SIZE = 128

AV_WORKAROUND = 0 # Set to 1 to randomize file order and add some padding, to work around silly av false positives

parser = argparse.ArgumentParser(description='''
A tool that packages files and generates Emscripten FS API calls to create a filesystem.
''',
                                 epilog='''
The resulting JS file should be included in your HTML, and should be stored
next to the data files (unless Module['DATA_FILE_PREFIX'] is set before
the script is sourced, in which case it will be appended).
''')

parser.add_argument('output_base', metavar='OUTPUT', type=str,
                    help='Base filename for output.  This tool will create output_base.js, and output_base.NNN.data (where NNN is present if --split is used).')
parser.add_argument('files', metavar='FILE', type=str, nargs='+',
                    help='Files to include in the data package.  Directories will be traversed recrusively.')

parser.add_argument('-s', '--split', dest='split', action='store_true',
                    help='Split the data file into chunks.')
parser.add_argument('--split-size', dest='split_size', type=str, action='store',
                    default='8M',
                    help='Size of split chunks, in bytes.  Must be a multiple of 4.  Defaults to 8M.  Suffixes K, M, and G are supported.')
parser.add_argument('-c', '--crunch', dest='crunch', action='store_true', default=False,
                    help='Will compress dxt files to crn.  CRUNCH in ~/.emscripten must point to the crunch tool binary.  JS crunch decompressing code will be added to convert the crn to dds in the browser.  Additionally, crunch-worker.js will be generated in the output directory.  You should include that file when packaging your site.  DDS files will not be crunched if the .crn is more recent than the .dds.')
parser.add_argument('--crunch-quality', dest='crunch_quality', action='store',
                    type=int,
                    default=128,
                    help='Crunch quality to use.  Defaults to 128.')
parser.add_argument('--align', dest='align', action='store',
                    type=int,
                    default=4,
                    help='Byte alignment of embedded files.  Defaults to 4.')

args = parser.parse_args()

# fix up the size arg
try:
  if args.split_size[-1].lower() == 'k':
    args.split_size = int(args.split_size[:-1]) * 1024
  elif args.split_size[-1].lower() == 'm':
    args.split_size = int(args.split_size[:-1]) * 1024 * 1024
  elif args.split_size[-1].lower() == 'g':
    args.split_size = int(args.split_size[:-1]) * 1024 * 1024 * 1024
  else:
    args.split_size = int(args.split_size)
except:
  print "Invalid split size."
  sys.exit(1)

if not args.split:
  args.split_size = 0

if args.split_size % 4 != 0:
  print "Split size must be a multiple of 4."
  sys.exit(1)

def make_file_list(name_args):
  data_files = []
  def add(orig_base, dirname, names):
    for name in names:
      fullname = os.path.join(dirname, name)
      if not os.path.isdir(fullname):
        n = fullname
        if n.startswith(orig_base):
          n = n[len(orig_base):]
          n = os.path.basename(orig_base) + n
        data_files.append({ 'localfile': fullname, 'name': n })

  for name in name_args:
    if os.path.isdir(name):
      os.path.walk(name, add, name)
    else:
      data_files.append({ 'localfile': name })

  # get rid of the directories
  data_files = filter(lambda file_: not os.path.isdir(file_['localfile']), data_files)

  # do some transforms on the names; 'localfile' is the name
  # in the local filesystem, 'name' is the target name
  for file_ in data_files:
    file_['name'] = file_['name'].replace(os.path.sep, '/')
    file_['size'] = os.stat(file_['localfile']).st_size

  # remove duplicates
  seen = {}
  def was_seen(name):
    if seen.get(name): return True
    seen[name] = 1
    return False
  data_files = filter(lambda file_: not was_seen(file_['name']), data_files)

  if AV_WORKAROUND:
    random.shuffle(data_files)

  # done
  return data_files

def crunch_files(data_files):
  for file_ in data_files:
    # should we touch it?
    if not file_['name'].lower().endswith(CRUNCH_INPUT_SUFFIX):
      continue
    crunch_name = unsuffixed(file_['name']) + CRUNCH_OUTPUT_SUFFIX
    try:
      crunch_time = os.stat(crunch_name).st_mtime
      dds_time = os.stat(file_['localfile']).st_mtime
      if dds_time < crunch_time:
        # already crunched, skip it
        file_['localfile'] = crunch_name
        continue
    except:
      pass # if one of them does not exist, continue on

          # guess at format. this lets us tell crunch to not try to be clever and use odd formats like DXT5_AGBR
    try:
      # the DXT fourcc will be in the header
      filebits = open(file_['localfile'], 'rb').read(DDS_HEADER_SIZE)
      if 'DXT5' in filebits:
        format = ['-dxt5']
      elif 'DXT1' in filebits:
        format = ['-dxt1']
      else:
        raise Exception('unknown format')
    except:
      format = []
    Popen([CRUNCH, '-file', file_['localfile'], '-quality', crunch] + format, stdout=sys.stderr).communicate()
    #if not os.path.exists(os.path.basename(crunch_name)):
    #  print >> sys.stderr, 'Failed to crunch, perhaps a weird dxt format? Looking for a source PNG for the DDS'
    #  Popen([CRUNCH, '-file', unsuffixed(file_['name']) + '.png', '-quality', crunch] + format, stdout=sys.stderr).communicate()
    assert os.path.exists(os.path.basename(crunch_name)), 'crunch failed to generate output'
    shutil.move(os.path.basename(crunch_name), crunch_name) # crunch places files in the current dir
    # prepend the dds header
    crunched = open(crunch_name, 'rb').read()
    c = open(crunch_name, 'wb')
    c.write(open(file_['localfile'], 'rb').read()[:DDS_HEADER_SIZE])
    c.write(crunched)

    # retarget localname and update size
    file_['localfile'] = crunch_name
    file_['size'] = os.stat(file_['localfile']).st_size

    c.close()

def chunk_filepath(index):
  if args.split:
    return "%s.%03d.data" % (args.output_base, index)
  assert index == 0, "chunk > 0 but not splitting?"
  return args.output_base + ".data"

def chunk_filename(index):
  return os.path.basename(chunk_filepath(index))

#
# takes data_files and puts them into one or more output data files
#
def create_output_files(data_files):
  # seriously hate python sometimes. create a state object.
  state = type("", (), {})()
  state.chunk_index = 0
  state.total_data_size = 0

  state.chunk_out = open(chunk_filepath(0), 'wb')

  def maybe_next_chunk():
    assert state.chunk_out.tell() <= args.split_size, "file position somehow went over split_size?"
    if state.chunk_out.tell() == args.split_size:
      state.chunk_index += 1
      state.chunk_out = open("%s.%03d.data" % (args.output_base, state.chunk_index), 'wb')
  
  def get_padding(size):
    if args.align:
      slop = size & (args.align-1)
      if slop:
        return args.align - slop
    return 0
  
  def write_file(file_):
    size = file_['size']
    padding = get_padding(size)
  
    infp = open(file_['localfile'], 'rb')
  
    file_['data_start'] = args.split_size * state.chunk_index + state.chunk_out.tell()
    file_['data_end'] = file_['data_start'] + size
    state.total_data_size += size + padding
  
    if args.split:
      remaining = size
      while remaining > 0:
        maybe_next_chunk()
        towrite = min(args.split_size - state.chunk_out.tell(), remaining)
        state.chunk_out.write(infp.read(towrite))
        remaining -= towrite
      while padding > 0:
        maybe_next_chunk()
        towrite = min(args.split_size - state.chunk_out.tell(), padding)
        state.chunk_out.write('\x00' * towrite)
        padding -= towrite
    else:
      state.chunk_out.write(infp.read())
      state.chunk_out.write('\x00' * padding)
    infp.close()
  
  # now write everything out
  for file_ in data_files:
    write_file(file_)

  return (state.chunk_index + 1, state.total_data_size)

#
# Make sure the output dir exists
#
destdir = os.path.dirname(args.output_base)
if destdir and not os.path.exists(destdir):
  os.makedirs(destdir)

#
# Create the file list and generate the output data file(s)
#  
data_files = make_file_list(args.files)

if args.crunch:
  precrunch_files(data_files)

(num_chunks, total_data_size) = create_output_files(data_files)

#
# Write the JS file that drives loading
#

js_file = open(args.output_base + '.js', 'w')

# preamble
print >>js_file, '''
/* auto-generated by file_packager2.py */
(function() {
  if (typeof Module == 'undefined') Module = {};
  if (!Module['preRun']) Module['preRun'] = [];
  if (!Module['dataFileDownloads']) Module['dataFileDownloads'] = {};
  if (!Module['finishedDataFileDownloads']) Module['finishedDataFileDownloads'] = 0;
  if (!Module['expectedDataFileDownloads']) Module['expectedDataFileDownloads'] = 0;

  function assert(check, msg) {
    if (!check) throw msg + new Error().stack;
  }
'''

# set up the decrunch worker if we're doing crunch
if args.crunch:
  print >>js_file, '''
  var decrunchWorker = new Worker('crunch-worker.js');
  var decrunchCallbacks = [];
  decrunchWorker.onmessage = function(msg) {
    decrunchCallbacks[msg.data.callbackID](msg.data.data);
    //console.log('decrunched ' + msg.data.filename + ' in ' + msg.data.time + ' ms, ' + msg.data.data.length + ' bytes');
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

print >>js_file, '''
  Module['preRun'].push(function() {
'''

# Set up folders
partial_dirs = []
for file_ in data_files:
  dirname = os.path.dirname(file_['name'])
  dirname = dirname.lstrip('/') # absolute paths start with '/', remove that
  if not dirname:
    continue
  parts = dirname.split('/')
  for i in range(len(parts)):
    partial = '/'.join(parts[:i+1])
    if partial not in partial_dirs:
      print >>js_file, '''    Module['FS_createPath']('/%s', '%s', true, true);''' % ('/'.join(parts[:i]), parts[i])
      partial_dirs.append(partial)

print >>js_file, ''

# Set up chunks
print >>js_file, "    var XHR_COUNT  = Module['XHR_COUNT'] || 2;"
print >>js_file, "    var TOTAL_SIZE = %ld;" % (total_data_size)
print >>js_file, "    var DATA_FILE_PREFIX = Module['DATA_FILE_PREFIX'] || '';"
print >>js_file, "    var DATA_CHUNKS = ["

if args.split:
  for chunk_index in range(num_chunks):
    offset = chunk_index * args.split_size
    print >>js_file, "      [DATA_FILE_PREFIX+'%s', %ld, %ld]," % (chunk_filename(chunk_index), offset, min(args.split_size, total_data_size - offset))
else:
  print >>js_file, "      [DATA_FILE_PREFIX+'%s', %ld, %ld]," % (chunk_filename(0), 0, total_data_size)

print >>js_file, "    ];"
print >>js_file, "    var NUM_DATA_CHUNKS = DATA_CHUNKS.length;"
print >>js_file, ''

# Now write the DataRequests; similar API to XHR
print >>js_file, '''
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
  # Preload
  varname = 'filePreload%d' % counter
  counter += 1
  dds = args.crunch and filename.endswith(CRUNCH_INPUT_SUFFIX)

  prepare = ''
  finish = "Module['removeRunDependency']('fp %s');\n" % filename

  # workaround for chromium bug 124926 (still no audio with this, but at least we don't hang)
  fail = '' if filename[-4:] not in AUDIO_SUFFIXES else ''', function() { Module['removeRunDependency']('fp %s') }''' % filename

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

  print >>js_file, '''
    var %(varname)s = new DataRequest();
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
        'varname': varname,
        'filename': filename,
        'dirname': os.path.dirname(filename),
        'basename': os.path.basename(filename),
        'prepare': prepare,
        'finish': finish,
        'fail': fail
      }


# write the finish-loading functions that will
# split things up
print >>js_file, '''
    function finishedAllLoading(arrayBuffer) {
      assert(arrayBuffer, 'Loading data file failed.');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
'''

for file_ in data_files:
  print >>js_file, '''
      curr = DataRequest.prototype.requests['%s'];
      curr.response = byteArray.subarray(%d,%d);
      curr.onload();
''' % (file_['name'], file_['data_start'], file_['data_end'])

# finish off finishedAllLoading
print >>js_file, '''

      for (var i = 0; i < NUM_DATA_CHUNKS; ++i) {
	Module['removeRunDependency']("__datafile__" + DATA_CHUNKS[i][0]);
      }
    }

'''

# now static postamble that works based on the previous variables that were set up
print >>js_file, '''
    for (var i = 0; i < NUM_DATA_CHUNKS; ++i) {
      assert((DATA_CHUNKS[i] & 3) == 0, "chunks must always start at 4-byte offsets (max chunk size must be a multiple of 4)");
      Module['addRunDependency']("__datafile__" + DATA_CHUNKS[i][0]);
      Module.dataFileDownloads[DATA_CHUNKS[i][0]] = { loaded: 0, total: DATA_CHUNKS[i][2] };
      Module.expectedDataFileDownloads++;
    }

    var resultBuffer = new ArrayBuffer(TOTAL_SIZE);
    var nextDataChunkToLoad = 0;

    var currentDataChunk = 0;
    for (var i = 0; i < XHR_COUNT; ++i) {
      if (i >= NUM_DATA_CHUNKS)
	break;

      var xhr = new XMLHttpRequest();
      var idx = nextDataChunkToLoad++;
      xhr.dataChunkIndex = idx;
      xhr.dataChunkUrl = DATA_CHUNKS[idx][0];
      xhr.onprogress = function(event) {
	//console.log("xhr progress for chunk", this.dataChunkIndex, "loaded: ", event.loaded, "/", event.total);

	Module.dataFileDownloads[this.dataChunkUrl].loaded = event.loaded;

	var total = 0;
	var loaded = 0;
	for (var download in Module.dataFileDownloads) {
	  total += download.total;
	  loaded += download.loaded;
	}

        if (total) {
          Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
        }
      };

      xhr.onload = function() {
	var chunk = DATA_CHUNKS[this.dataChunkIndex];
	var chunklen = chunk[2];
	var chunkBuffer = this.response;

	assert(chunkBuffer.byteLength == chunklen,
	       "got chunk " + this.dataChunkIndex + " whose size didn't match! (got " + chunkBuffer.byteLength + ", expected " + chunklen + ")");

	// copy things into place
	var chunkBuffer32 = new Int32Array(chunkBuffer, 0, chunklen >> 2);
	var destBuffer32 = new Int32Array(resultBuffer, chunk[1], chunklen >> 2);
	destBuffer32.set(chunkBuffer32);

	if ((chunklen & 3) != 0) {
	  // (hopefully) last chunk, we have some trailing bytes
	  var offset = (chunklen >> 2) * 4;
	  var bytes = chunklen & 3;
	  var chunkBuffer8 = new Int8Array(chunkBuffer, offset, bytes);
	  var destBuffer8 = new Int8Array(resultBuffer, chunk[1] + offset, bytes);
	  destBuffer8.set(chunkBuffer8);
	}

	Module.finishedDataFileDownloads++;

	var nextChunk = nextDataChunkToLoad++;
	if (nextChunk == NUM_DATA_CHUNKS) {
	  // for this one, and this one ONLY (not >=, because multiple
	  // XHR's will trigger a >=, but only one will trigger a ==),
	  // trigger the endgame
	  finishedAllLoading(resultBuffer);
	}

	if (nextChunk >= NUM_DATA_CHUNKS) {
	  // but everyone exits
	  return;
	}

	// set us up for the next chunk
	this.dataChunkIndex = nextChunk;
	this.dataChunkUrl = DATA_CHUNKS[nextChunk][0];

	this.open("GET", this.dataChunkUrl, true);
	this.responseType = 'arraybuffer';
	this.send(null);
      };

      // kick it off
      xhr.open("GET", xhr.dataChunkUrl, true);
      xhr.responseType = 'arraybuffer';
      xhr.send(null);
    }
  });
})();
'''

js_file.close()

# Tell user what was done.
if args.split:
  print "Wrote %s and %d chunks, total size = %ld bytes." % (args.output_base + '.js', num_chunks, total_data_size)
else:
  print "Wrote %s, total size = %ld bytes." % (args.output_base + '.js', total_data_size)
