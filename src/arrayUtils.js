if (typeof maybeExport === 'undefined') {
	function maybeExport () {}
}

/** @type {function(string, boolean=, number=)} */
function intArrayFromString(stringy, dontAddNull, length) {
  var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
  var u8array = new Array(len);
  var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
  if (dontAddNull) u8array.length = numBytesWritten;
  return u8array;
}
{{{ maybeExport('intArrayFromString') }}}

function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
#if ASSERTIONS
      assert(false, 'Character code ' + chr + ' (' + String.fromCharCode(chr) + ')  at offset ' + i + ' not in 0x00-0xFF.');
#endif
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}
{{{ maybeExport('intArrayToString') }}}

// Converts a string of base64 into a byte array.
// Throws error on invalid input.
function intArrayFromBase64(s) {
  try {
    var decoded = atob(s);
    var bytes = new Uint8Array(decoded.length);
    for (i = 0 ; i < decoded.length ; ++i) {
      bytes[i] = decoded.charCodeAt(i);
    }
    return bytes;
  } catch (_) {
    throw new Error('Converting base64 string to bytes failed.');
  }
}
{{{ maybeExport('intArrayFromBase64') }}}

// If filename is a base64 data URI, parses and returns data (Buffer on node,
// Uint8Array otherwise). If filename is not a base64 data URI, returns undefined.
function tryParseAsDataURI(filename) {
  var dataURIPrefix = 'data:application/octet-stream;base64,';

  if (!(
    String.prototype.startsWith ?
      filename.startsWith(dataURIPrefix) :
      filename.indexOf(dataURIPrefix) === 0
  )) {
    return;
  }

  var data = filename.slice(dataURIPrefix.length);

  if (typeof Buffer !== 'undefined') {
    return Buffer.from(data, 'base64');
  }

  return intArrayFromBase64(data);
}
{{{ maybeExport('tryParseAsDataURI') }}}
