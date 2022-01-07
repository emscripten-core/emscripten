/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

requireNodeFS = function() {
  // Use nodePath as the indicator for these not being initialized,
  // since in some environments a global fs may have already been
  // created.
  if (!nodePath) {
    fs = require('fs');
    nodePath = require('path');
  }
}

read_ = function shell_read(filename, binary) {
#if SUPPORT_BASE64_EMBEDDING
  var ret = tryParseAsDataURI(filename);
  if (ret) {
    return binary ? ret : ret.toString();
  }
#endif
  requireNodeFS();
  filename = nodePath['normalize'](filename);
  return fs.readFileSync(filename, binary ? null : 'utf8');
};

readBinary = function readBinary(filename) {
  var ret = read_(filename, true);
  if (!ret.buffer) {
    ret = new Uint8Array(ret);
  }
#if ASSERTIONS
  assert(ret.buffer);
#endif
  return ret;
};

readAsync = function readAsync(filename, onload, onerror) {
#if SUPPORT_BASE64_EMBEDDING
  var ret = tryParseAsDataURI(filename);
  if (ret) {
    onload(ret);
  }
#endif
  requireNodeFS();
  filename = nodePath['normalize'](filename);
  fs.readFile(filename, function(err, data) {
    if (err) onerror(err);
    else onload(data.buffer);
  });
};
