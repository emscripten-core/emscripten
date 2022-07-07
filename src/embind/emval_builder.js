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

  _emvalbuilder_finalize__sig: 'vppp',
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
            case 1:  // INT8
                v = HEAP8[ptr]; break;
            case 2:  // UINT8
                v = HEAPU8[ptr]; break;
            case 3:  // INT16
                v = HEAP16[(ptr >> 1)]; break;
            case 4:  // UINT16
                v = HEAPU16[(ptr >> 1)]; break;
            case 5:  // INT32
                v = HEAP32[(ptr >> 2)]; break;
            case 6:  // UINT32
                v = HEAPU32[(ptr >> 2)]; break;
            case 7:  // FLOAT32
                v = HEAPF32[(ptr >> 2)]; break;
            case 8:  // FLOAT64
                v = HEAPF64[(ptr >> 3)]; break;
#if WASM_BIGINT
            case 9:  // INT64
                v = HEAP64[(ptr >> 3)]; break;
            case 10:  // UINT64
                v = HEAPU64[(ptr >> 3)]; break;
#endif
            case 11:  // BOOL
                v = !!HEAPU8[ptr]; break;
            case 12:  // STRING
                v = readLatin1String(HEAP32[(ptr >> 2)]); break;
            case 13:  // U8STRING
                v = UTF8ToString(HEAP32[(ptr >> 2)]); break;
            case 14:  // EMVAL
                v = Emval.toValue(HEAP32[(ptr >> 2)]); break;
            case 15: {  // ARRAY
                var ad = HEAP32[(ptr >> 2)];
                var ty = HEAPU8[(ptr + 4)];
                var con = !!HEAPU8[(ptr + 5)];
                var n = HEAPU16[(ptr >> 1) + 3];
                var heap;
                switch (ty) {
                    case 1:           heap = HEAP8;  break;   // INT8
                    case 2:           heap = HEAPU8; break;   // UINT8
                    case 3: ad >>= 1; heap = HEAP16; break;   // INT16
                    case 4: ad >>= 1; heap = HEAPU16; break;  // UINT16
                    case 5: ad >>= 2; heap = HEAP32;  break;  // INT32
                    case 6: ad >>= 2; heap = HEAPU32; break;  // UINT32
                    case 7: ad >>= 2; heap = HEAPF32; break;  // FLOAT32
                    case 8: ad >>= 3; heap = HEAPF64; break;  // FLOAT64
                    case 11:          heap = HEAPU8;  break;  // BOOL
                    default: skip = true; break;
                }
                if (!skip) {
                    v = new Array(n);
                    // Seems using for..loop is faster than slice()!
                    if (ty == 11) {  // BOOL(Byte)
                        for (var j = 0; j < n; j++) v[j] = !!heap[ad + j];
                    } else {
                        for (var j = 0; j < n; j++) v[j] = heap[ad + j];
                    }
                    if (con) {
                        o.push(...v);
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
