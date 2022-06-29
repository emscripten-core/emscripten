// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

/*global Module:true, Runtime*/
/*global HEAP32*/
/*global readLatin1String, UTF8ToString*/

// -- jshint doesn't understand library syntax, so we need to mark the symbols exposed here
/*global _emvalbuilder_finalize*/

mergeInto(LibraryManager.library, {

  _emvalbuilder_finalize__deps: ['$Emval', '$readLatin1String'],
  _emvalbuilder_finalize: function(o, ptr, size) {
    o = Emval.toValue(o);
    var is_array = (o instanceof Array);
    for (var i = 0; i < size; i++) {
        var t = HEAPU8[ptr];
        var val_k = !!HEAPU8[(ptr + 1)];
        var k = HEAP32[(ptr >> 2) + 1];
        ptr += 8;
        var v;
        var skip = false;
        switch (t) {
            case 1:  // INT32
                v = HEAP32[(ptr >> 2)]; break;
            case 2:  // UINT32
                v = HEAPU32[(ptr >> 2)]; break;
            case 3:  // FLOAT32
                v = HEAPF32[(ptr >> 2)]; break;
            case 4:  // FLOAT64
                v = HEAPF64[(ptr >> 3)]; break;
            case 5:  // BOOL
                v = !!HEAPU8[ptr]; break;
            case 6:  // STRING
                v = readLatin1String(HEAP32[(ptr >> 2)]); break;
            case 7:  // U8STRING
                v = UTF8ToString(HEAP32[(ptr >> 2)]); break;
            case 8:  // EMVAL
                v = Emval.toValue(HEAP32[(ptr >> 2)]); break;
            case 9: {  // ARRAY
                var ad = HEAP32[(ptr >> 2)];
                var ty = HEAPU8[(ptr + 4)];
                var con = !!HEAPU8[(ptr + 5)];
                var n = HEAPU16[(ptr >> 1) + 3];
                var heap;
                switch (ty) {
                    case 1: ad >>= 2; heap = HEAP32;  break;  // INT32
                    case 2: ad >>= 2; heap = HEAPU32; break;  // UINT32
                    case 3: ad >>= 2; heap = HEAPF32; break;  // FLOAT32
                    case 4: ad >>= 3; heap = HEAPF64; break;  // FLOAT64
                    case 5:           heap = HEAPU8;  break;  // BOOL
                    default: skip = true; break;
                }
                if (!skip) {
                    v = new Array(n);
                    // Seems using for..loop is faster than slice()!
                    if (ty == 5) {  // BOOL(Byte)
                        for (var j = 0; j < n; j++) v[j] = !!heap[ad+j];
                    } else {
                        for (var j = 0; j < n; j++) v[j] = heap[ad+j];
                    }
                    if (con) {
                        o.concat(v);
                        skip = true;
                    }
                }
                break;
            }
            default: skip = true; break;
        }
        ptr += 8;  // Always advance the ptr
        if (skip) continue;
        if (is_array) {
            o.push(v);
        } else {
            k = val_k ? Emval.toValue(k) : readLatin1String(k);
            o[k] = v;
        }
    }
  },

});
