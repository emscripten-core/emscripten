// Implementation of libuuid creating RFC4122 version 4 random UUIDs.

mergeInto(LibraryManager.library, {
  // Clear a 'compact' UUID.
  uuid_clear: function(uu) {
    // void uuid_clear(uuid_t uu);
    _memset(uu, 0, 16);
  },

  // Compare whether or not two 'compact' UUIDs are the same.
  // Returns an integer less than, equal to, or greater than zero if uu1  is found, respectively, to be   
  // lexigraphically  less  than,  equal, or greater than uu2.
  uuid_compare__deps: ['memcmp'],
  uuid_compare: function(uu1, uu2) {
    // int uuid_compare(const uuid_t uu1, const uuid_t uu2);
    return _memcmp(uu1, uu2, 16);
  },

  // Copies the 'compact' UUID variable from src to dst.
  uuid_copy: function(dst, src) {
    // void uuid_copy(uuid_t dst, const uuid_t src);
    _memcpy(dst, src, 16);
  },

  // Write a RFC4122 version 4 compliant UUID largely based on the method found in
  // http://stackoverflow.com/questions/105034/how-to-create-a-guid-uuid-in-javascript
  // tweaked slightly in order to use the 'compact' UUID form used by libuuid.
  uuid_generate: function(out) {
    // void uuid_generate(uuid_t out);
    var uuid = null;

    if (ENVIRONMENT_IS_NODE) {
      // If Node.js try to use crypto.randomBytes
      try {
        var rb = require('crypto').randomBytes;
        uuid = rb(16);
      } catch(e) {}
    } else if (ENVIRONMENT_IS_WEB &&
               typeof(window.crypto) !== 'undefined' && 
               typeof(window.crypto.getRandomValues) !== 'undefined') {
      // If crypto.getRandomValues is available try to use it.
      uuid = new Uint8Array(16);
      window.crypto.getRandomValues(uuid);
    }

    // Fall back to Math.random if a higher quality random number generator is not available.
    if (!uuid) {
      uuid = new Array(16);
      var d = new Date().getTime();
      for (var i = 0; i < 16; i++) {
        var r = ((d + Math.random() * 256) % 256)|0;
        d = (d / 256)|0;
        uuid[i] = r;
      }
    }

    uuid[6] = (uuid[6] & 0x0F) | 0x40;
    uuid[8] = (uuid[8] & 0x7F) | 0x80;
    writeArrayToMemory(uuid, out);
  },

  // Compares the value of the supplied 'compact' UUID variable uu to the NULL value.
  // If the value is equal to the NULL UUID, 1 is returned, otherwise 0 is returned.
  uuid_is_null: function(uu) {
    // int uuid_is_null(const uuid_t uu);
    for (var i = 0; i < 4; i++, uu = (uu+4)|0) {
      var val = {{{ makeGetValue('uu', 0, 'i32') }}};
      if (val) {
        return 0;
      }
    }
    return 1;
  },

  // converts the UUID string given by inp into the binary representation. The input UUID is a string of
  // the form "%08x-%04x-%04x-%04x-%012x" 36 bytes plus the trailing '\0'.
  // Upon successfully parsing the input string, 0 is returned, and the UUID is stored in the location
  // pointed to by uu, otherwise -1 is returned.
  uuid_parse: function(inp, uu) {
    // int uuid_parse(const char *in, uuid_t uu);
    var inp = Pointer_stringify(inp);
    if (inp.length === 36) {
      var i = 0;
      var uuid = new Array(16);
      inp.toLowerCase().replace(/[0-9a-f]{2}/g, function(byte) {
        if (i < 16) {
          uuid[i++] = parseInt(byte, 16);
        }
      });

      if (i < 16) {
        return -1;
      } else {
        writeArrayToMemory(uuid, uu);
        return 0;
      }
    } else {
      return -1;
    }
  },
 
  // Convert a 'compact' form UUID to a string, if the upper parameter is supplied make the string upper case.
  uuid_unparse: function(uu, out, upper) {
    // void uuid_unparse(const uuid_t uu, char *out);
    var i = 0;
    var uuid = 'xxxx-xx-xx-xx-xxxxxx'.replace(/[x]/g, function(c) {
      var r = upper ? ({{{ makeGetValue('uu', 'i', 'i8', 0, 1) }}}).toString(16).toUpperCase() :
                      ({{{ makeGetValue('uu', 'i', 'i8', 0, 1) }}}).toString(16);
      r = (r.length === 1) ? '0' + r : r; // Zero pad single digit hex values
      i++;
      return r;
    });
    stringToUTF8(uuid, out, 37); // Always fixed 36 bytes of ASCII characters and a trailing \0.
  },

  // Convert a 'compact' form UUID to a lower case string.
  uuid_unparse_lower__deps: ['uuid_unparse'],
  uuid_unparse_lower: function(uu, out) {
    // void uuid_unparse_lower(const uuid_t uu, char *out);
    _uuid_unparse(uu, out);
  },

  // Convert a 'compact' form UUID to an upper case string.
  uuid_unparse_upper__deps: ['uuid_unparse'],
  uuid_unparse_upper: function(uu, out) {
    // void uuid_unparse_upper(const uuid_t uu, char *out);
    _uuid_unparse(uu, out, true);
  },

  uuid_type: function(uu) {
    // int uuid_type(const uuid_t uu);
    return {{{ cDefine('UUID_TYPE_DCE_RANDOM') }}};
  },

  uuid_variant: function(uu) {
    // int uuid_variant(const uuid_t uu);
    return {{{ cDefine('UUID_VARIANT_DCE') }}};
  }
});

