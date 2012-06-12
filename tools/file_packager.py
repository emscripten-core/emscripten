'''
A tool that generates FS API calls to generate a filesystem, and packages the files
to work with that.

This is called by emcc. You can also call it yourself.

Usage:

  file_packager.py TARGET [--preload A [B..]] [--embed C [D..]] [--compress COMPRESSION_DATA] [--pre-run]

'''

import os, sys

from shared import Compression, execute

def suffix(name):
  return name.split('.')[-1]

IMAGE_SUFFIXES = ('.jpg', '.png', '.bmp')
AUDIO_SUFFIXES = ('.ogg', '.wav', '.mp3')
AUDIO_MIMETYPES = { 'ogg': 'audio/ogg', 'wav': 'audio/wav', 'mp3': 'audio/mpeg' }

data_files = []
in_preload = False
in_embed = False
has_preloaded = False
in_compress = 0
pre_run = False
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

code = '''
function assert(check, msg) {
  if (!check) throw msg;
}
'''

if has_preloaded:
  code += '''
  var BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : (typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : console.log("warning: cannot build blobs"));
  var URLObject = typeof window != "undefined" ? (window.URL ? window.URL : window.webkitURL) : console.log("warning: cannot create object URLs");
  var hasBlobConstructor;
  try {
    new Blob();
    hasBlobConstructor = true;
  } catch(e) {
    hasBlobConstructor = false;
    console.log("warning: no blob constructor, cannot create blobs with mimetypes");
  }
'''

  code += 'Module["preloadedImages"] = {}; // maps url to image data\n'
  code += 'Module["preloadedAudios"] = {}; // maps url to audio data\n'

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
  file_['name'] = file_['name'].replace(os.path.sep, '/')
  file_['net_name'] = file_['name']

data_target = sys.argv[1]

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
        code += '''Module['FS_createFolder']('/%s', '%s', true, false);\n''' % ('/'.join(parts[:i]), parts[i])
        partial_dirs.append(partial)

if has_preloaded:
  # Bundle all datafiles into one archive. Avoids doing lots of simultaneous XHRs which has overhead.
  data = open(data_target, 'wb')
  start = 0
  for file_ in data_files:
    file_['data_start'] = start
    curr = open(file_['name'], 'rb').read()
    file_['data_end'] = start + len(curr)
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
    code += '''Module['FS_createDataFile']('/', '%s', %s, true, true);\n''' % (os.path.basename(filename), str(map(ord, open(filename, 'rb').read())))
  elif file_['mode'] == 'preload':
    # Preload
    varname = 'filePreload%d' % counter
    counter += 1
    image = filename.endswith(IMAGE_SUFFIXES)
    audio = filename.endswith(AUDIO_SUFFIXES)

    if image:
      finish =  '''
        var bb = new BlobBuilder();
        bb.append(byteArray.buffer);
        var b = bb.getBlob();
        var url = URLObject.createObjectURL(b);
        var img = new Image();
        img.onload = function() {
          assert(img.complete, 'Image %(filename)s could not be decoded');
          var canvas = document.createElement('canvas');
          canvas.width = img.width;
          canvas.height = img.height;
          var ctx = canvas.getContext('2d');
          ctx.drawImage(img, 0, 0);
          Module["preloadedImages"]['%(filename)s'] = canvas;
          URLObject.revokeObjectURL(url);
          Module['removeRunDependency']();
        };
        img.onerror = function(event) {
          console.log('Image %(filename)s could not be decoded');
        };
        img.src = url;
''' % { 'filename': filename }
    elif audio:
      # Need actual blob constructor here, to set the mimetype or else audios fail to decode
      finish =  '''
        if (hasBlobConstructor) {
          var b = new Blob([byteArray.buffer], { type: '%(mimetype)s' });
          var url = URLObject.createObjectURL(b); // XXX we never revoke this!
          var audio = new Audio();
          audio.removedDependency = false;
          audio['oncanplaythrough'] = function() { // XXX string for closure
            audio['oncanplaythrough'] = null;
            Module["preloadedAudios"]['%(filename)s'] = audio;
            if (!audio.removedDependency) {
              Module['removeRunDependency']();
              audio.removedDependency = true;
            }
          };
          audio.onerror = function(event) {
            if (!audio.removedDependency) {
              console.log('Audio %(filename)s could not be decoded or timed out trying to decode');
              Module['removeRunDependency']();
              audio.removedDependency = true;
            }
          };
          setTimeout(audio.onerror, 2000); // workaround for chromium bug 124926 (still no audio with this, but at least we don't hang)
          audio.src = url;
        } else {
          Module["preloadedAudios"]['%(filename)s'] = new Audio(); // empty shim
          Module['removeRunDependency']();
        }
''' % { 'filename': filename, 'mimetype': AUDIO_MIMETYPES[suffix(filename)] }
    else:
      finish = "Module['removeRunDependency']();\n"

    code += '''
    var %(varname)s = new %(request)s();
    %(varname)s.open('GET', '%(netname)s', true);
    %(varname)s.responseType = 'arraybuffer';
    %(varname)s.onload = function() {
      var arrayBuffer = %(varname)s.response;
      assert(arrayBuffer, 'Loading file %(filename)s failed.');
      var byteArray = arrayBuffer.byteLength ? new Uint8Array(arrayBuffer) : arrayBuffer;
      Module['FS_createDataFile']('/%(dirname)s', '%(basename)s', byteArray, true, true);
      %(finish)s
    };
    Module['addRunDependency']();
    %(varname)s.send(null);
''' % {
        'request': 'DataRequest', # In the past we also supported XHRs here
        'varname': varname,
        'filename': filename,
        'netname': file_['net_name'],
        'dirname': os.path.dirname(filename),
        'basename': os.path.basename(filename),
        'finish': finish
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
  use_data += "          Module['removeRunDependency']();\n"

  if Compression.on:
    use_data = '''
      Module["decompress"](byteArray, function(decompressed) {
        byteArray = new Uint8Array(decompressed);
        %s
      });
''' % use_data

  code += '''
    var dataFile = new XMLHttpRequest();
    dataFile.open('GET', '%s', true);
    dataFile.responseType = 'arraybuffer';
    dataFile.onload = function() {
      var arrayBuffer = dataFile.response;
      assert(arrayBuffer, 'Loading data file failed.');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      %s
    };
    Module['addRunDependency']();
    dataFile.send(null);
    if (Module['setStatus']) Module['setStatus']('Downloading...');
  ''' % (Compression.compressed_name(data_target) if Compression.on else data_target, use_data)

if pre_run:
  print 'Module["preRun"] = function() {'

print code

if pre_run:
  print '};'

