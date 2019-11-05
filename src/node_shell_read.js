  read_ = function shell_read(filename, binary) {
    var ret;
#if SUPPORT_BASE64_EMBEDDING
    ret = tryParseAsDataURI(filename);
    if (!ret) {
#endif
      if (!nodeFS) nodeFS = require('fs');
      if (!nodePath) nodePath = require('path');
      filename = nodePath['normalize'](filename);
      ret = nodeFS['readFileSync'](filename);
#if SUPPORT_BASE64_EMBEDDING
    }
#endif
    return binary ? ret : ret.toString();
  };

  readBinary = function readBinary(filename) {
    var ret = read_(filename, true);
    if (!ret.buffer) {
      ret = new Uint8Array(ret);
    }
    assert(ret.buffer);
    return ret;
  };

