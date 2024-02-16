// EMSCRIPTEN_START_ASM
function instantiate(wasmImports, wasmMemory, wasmTable) {

var helpee = 12;

function helper() {
  return helpee;
}

function asmFunc(global, env, buffer) {
 var memory = env.memory;
 var FUNCTION_TABLE = wasmTable;
 var HEAP8 = new global.Int8Array(buffer);
 var HEAP16 = new global.Int16Array(buffer);
 var HEAP32 = new global.Int32Array(buffer);
 var HEAPU8 = new global.Uint8Array(buffer);
 var HEAPU16 = new global.Uint16Array(buffer);
 var HEAPU32 = new global.Uint32Array(buffer);
 var HEAPF32 = new global.Float32Array(buffer);
 var HEAPF64 = new global.Float64Array(buffer);
 var Math_imul = global.Math.imul;
 var Math_fround = global.Math.fround;
 var Math_abs = global.Math.abs;
 var Math_clz32 = global.Math.clz32;
 var Math_min = global.Math.min;
 var Math_max = global.Math.max;
 var Math_floor = global.Math.floor;
 var Math_ceil = global.Math.ceil;
 var Math_sqrt = global.Math.sqrt;
 var abort = env.abort;
 var nan = global.NaN;
 var infinity = global.Infinity;
 var fimport$0 = env.fd_write;
 var fimport$1 = env.emscripten_memcpy_js;
 var global$0 = 5245632;
 var i64toi32_i32$HIGH_BITS = 0;
 // EMSCRIPTEN_START_FUNCS
;
 function $0($0_1, $1_1) {
  var $2_1 = 0, $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0;
  helper();
  $5_1 = 1024;
  $2_1 = HEAP32[$1_1 + 16 >> 2];
  label$1 : {
   if (!$2_1) {
    if ($1($1_1)) {
     break label$1
    }
    $2_1 = HEAP32[$1_1 + 16 >> 2];
   }
   $4_1 = HEAP32[$1_1 + 20 >> 2];
   if ($2_1 - $4_1 >>> 0 < $0_1 >>> 0) {
    return FUNCTION_TABLE[HEAP32[$1_1 + 36 >> 2]]($1_1, 1024, $0_1) | 0
   }
   label$5 : {
    if (HEAP8[$1_1 + 75 | 0] < 0) {
     break label$5
    }
    $3_1 = $0_1;
    while (1) {
     $2_1 = $3_1;
     if (!$2_1) {
      break label$5
     }
     $3_1 = $2_1 + -1 | 0;
     if (HEAPU8[$3_1 + 1024 | 0] != 10) {
      continue
     }
     break;
    };
    $3_1 = FUNCTION_TABLE[HEAP32[$1_1 + 36 >> 2]]($1_1, 1024, $2_1) | 0;
    if ($3_1 >>> 0 < $2_1 >>> 0) {
     break label$1
    }
    $0_1 = $0_1 - $2_1 | 0;
    $5_1 = $2_1 + 1024 | 0;
    $4_1 = HEAP32[$1_1 + 20 >> 2];
    $6_1 = $2_1;
   }
   $3($4_1, $5_1, $0_1);
   HEAP32[$1_1 + 20 >> 2] = HEAP32[$1_1 + 20 >> 2] + $0_1;
   $3_1 = $0_1 + $6_1 | 0;
  }
  return $3_1;
 }
 
 function $1($0_1) {
  var $1_1 = 0;
  $1_1 = HEAPU8[$0_1 + 74 | 0];
  HEAP8[$0_1 + 74 | 0] = $1_1 + -1 | $1_1;
  $1_1 = HEAP32[$0_1 >> 2];
  if ($1_1 & 8) {
   HEAP32[$0_1 >> 2] = $1_1 | 32;
   return -1;
  }
  HEAP32[$0_1 + 4 >> 2] = 0;
  HEAP32[$0_1 + 8 >> 2] = 0;
  $1_1 = HEAP32[$0_1 + 44 >> 2];
  HEAP32[$0_1 + 28 >> 2] = $1_1;
  HEAP32[$0_1 + 20 >> 2] = $1_1;
  HEAP32[$0_1 + 16 >> 2] = $1_1 + HEAP32[$0_1 + 48 >> 2];
  return 0;
 }
 
 function $2($0_1) {
  if (!$0_1) {
   return 0
  }
  HEAP32[300] = $0_1;
  return -1;
 }
 
 function $3($0_1, $1_1, $2_1) {
  var $3_1 = 0, $4_1 = 0;
  if ($2_1 >>> 0 >= 512) {
   fimport$1($0_1 | 0, $1_1 | 0, $2_1 | 0) | 0;
   return;
  }
  $3_1 = $0_1 + $2_1 | 0;
  label$2 : {
   if (!(($0_1 ^ $1_1) & 3)) {
    label$4 : {
     if (($2_1 | 0) < 1) {
      $2_1 = $0_1;
      break label$4;
     }
     if (!($0_1 & 3)) {
      $2_1 = $0_1;
      break label$4;
     }
     $2_1 = $0_1;
     while (1) {
      HEAP8[$2_1 | 0] = HEAPU8[$1_1 | 0];
      $1_1 = $1_1 + 1 | 0;
      $2_1 = $2_1 + 1 | 0;
      if ($2_1 >>> 0 >= $3_1 >>> 0) {
       break label$4
      }
      if ($2_1 & 3) {
       continue
      }
      break;
     };
    }
    $0_1 = $3_1 & -4;
    label$8 : {
     if ($0_1 >>> 0 < 64) {
      break label$8
     }
     $4_1 = $0_1 + -64 | 0;
     if ($2_1 >>> 0 > $4_1 >>> 0) {
      break label$8
     }
     while (1) {
      HEAP32[$2_1 >> 2] = HEAP32[$1_1 >> 2];
      HEAP32[$2_1 + 4 >> 2] = HEAP32[$1_1 + 4 >> 2];
      HEAP32[$2_1 + 8 >> 2] = HEAP32[$1_1 + 8 >> 2];
      HEAP32[$2_1 + 12 >> 2] = HEAP32[$1_1 + 12 >> 2];
      HEAP32[$2_1 + 16 >> 2] = HEAP32[$1_1 + 16 >> 2];
      HEAP32[$2_1 + 20 >> 2] = HEAP32[$1_1 + 20 >> 2];
      HEAP32[$2_1 + 24 >> 2] = HEAP32[$1_1 + 24 >> 2];
      HEAP32[$2_1 + 28 >> 2] = HEAP32[$1_1 + 28 >> 2];
      HEAP32[$2_1 + 32 >> 2] = HEAP32[$1_1 + 32 >> 2];
      HEAP32[$2_1 + 36 >> 2] = HEAP32[$1_1 + 36 >> 2];
      HEAP32[$2_1 + 40 >> 2] = HEAP32[$1_1 + 40 >> 2];
      HEAP32[$2_1 + 44 >> 2] = HEAP32[$1_1 + 44 >> 2];
      HEAP32[$2_1 + 48 >> 2] = HEAP32[$1_1 + 48 >> 2];
      HEAP32[$2_1 + 52 >> 2] = HEAP32[$1_1 + 52 >> 2];
      HEAP32[$2_1 + 56 >> 2] = HEAP32[$1_1 + 56 >> 2];
      HEAP32[$2_1 + 60 >> 2] = HEAP32[$1_1 + 60 >> 2];
      $1_1 = $1_1 - -64 | 0;
      $2_1 = $2_1 - -64 | 0;
      if ($2_1 >>> 0 <= $4_1 >>> 0) {
       continue
      }
      break;
     };
    }
    if ($2_1 >>> 0 >= $0_1 >>> 0) {
     break label$2
    }
    while (1) {
     HEAP32[$2_1 >> 2] = HEAP32[$1_1 >> 2];
     $1_1 = $1_1 + 4 | 0;
     $2_1 = $2_1 + 4 | 0;
     if ($2_1 >>> 0 < $0_1 >>> 0) {
      continue
     }
     break;
    };
    break label$2;
   }
   if ($3_1 >>> 0 < 4) {
    $2_1 = $0_1;
    break label$2;
   }
   $4_1 = $3_1 + -4 | 0;
   if ($4_1 >>> 0 < $0_1 >>> 0) {
    $2_1 = $0_1;
    break label$2;
   }
   $2_1 = $0_1;
   while (1) {
    HEAP8[$2_1 | 0] = HEAPU8[$1_1 | 0];
    HEAP8[$2_1 + 1 | 0] = HEAPU8[$1_1 + 1 | 0];
    HEAP8[$2_1 + 2 | 0] = HEAPU8[$1_1 + 2 | 0];
    HEAP8[$2_1 + 3 | 0] = HEAPU8[$1_1 + 3 | 0];
    $1_1 = $1_1 + 4 | 0;
    $2_1 = $2_1 + 4 | 0;
    if ($2_1 >>> 0 <= $4_1 >>> 0) {
     continue
    }
    break;
   };
  }
  if ($2_1 >>> 0 < $3_1 >>> 0) {
   while (1) {
    HEAP8[$2_1 | 0] = HEAPU8[$1_1 | 0];
    $1_1 = $1_1 + 1 | 0;
    $2_1 = $2_1 + 1 | 0;
    if (($3_1 | 0) != ($2_1 | 0)) {
     continue
    }
    break;
   }
  }
 }
 
 function $4($0_1, $1_1, $2_1, $3_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  $2_1 = $2_1 | 0;
  $3_1 = $3_1 | 0;
  i64toi32_i32$HIGH_BITS = 0;
  return 0;
 }
 
 function $5($0_1) {
  $0_1 = $0_1 | 0;
  return 0;
 }
 
 function $6($0_1, $1_1, $2_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  $2_1 = $2_1 | 0;
  var $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0, $7_1 = 0, $8_1 = 0;
  $3_1 = global$0 - 32 | 0;
  global$0 = $3_1;
  $4_1 = HEAP32[$0_1 + 28 >> 2];
  HEAP32[$3_1 + 16 >> 2] = $4_1;
  $5_1 = HEAP32[$0_1 + 20 >> 2];
  HEAP32[$3_1 + 28 >> 2] = $2_1;
  HEAP32[$3_1 + 24 >> 2] = $1_1;
  $1_1 = $5_1 - $4_1 | 0;
  HEAP32[$3_1 + 20 >> 2] = $1_1;
  $4_1 = $1_1 + $2_1 | 0;
  $7_1 = 2;
  $1_1 = $3_1 + 16 | 0;
  label$1 : {
   label$2 : {
    label$3 : {
     if (!$2(fimport$0(HEAP32[$0_1 + 60 >> 2], $3_1 + 16 | 0, 2, $3_1 + 12 | 0) | 0)) {
      while (1) {
       $5_1 = HEAP32[$3_1 + 12 >> 2];
       if (($5_1 | 0) == ($4_1 | 0)) {
        break label$3
       }
       if (($5_1 | 0) <= -1) {
        break label$2
       }
       $6_1 = HEAP32[$1_1 + 4 >> 2];
       $8_1 = $5_1 >>> 0 > $6_1 >>> 0;
       $1_1 = $8_1 ? $1_1 + 8 | 0 : $1_1;
       $6_1 = $5_1 - ($8_1 ? $6_1 : 0) | 0;
       HEAP32[$1_1 >> 2] = $6_1 + HEAP32[$1_1 >> 2];
       HEAP32[$1_1 + 4 >> 2] = HEAP32[$1_1 + 4 >> 2] - $6_1;
       $4_1 = $4_1 - $5_1 | 0;
       $7_1 = $7_1 - $8_1 | 0;
       if (!$2(fimport$0(HEAP32[$0_1 + 60 >> 2], $1_1 | 0, $7_1 | 0, $3_1 + 12 | 0) | 0)) {
        continue
       }
       break;
      }
     }
     HEAP32[$3_1 + 12 >> 2] = -1;
     if (($4_1 | 0) != -1) {
      break label$2
     }
    }
    $1_1 = HEAP32[$0_1 + 44 >> 2];
    HEAP32[$0_1 + 28 >> 2] = $1_1;
    HEAP32[$0_1 + 20 >> 2] = $1_1;
    HEAP32[$0_1 + 16 >> 2] = $1_1 + HEAP32[$0_1 + 48 >> 2];
    $0_1 = $2_1;
    break label$1;
   }
   HEAP32[$0_1 + 28 >> 2] = 0;
   HEAP32[$0_1 + 16 >> 2] = 0;
   HEAP32[$0_1 + 20 >> 2] = 0;
   HEAP32[$0_1 >> 2] = HEAP32[$0_1 >> 2] | 32;
   $0_1 = 0;
   if (($7_1 | 0) == 2) {
    break label$1
   }
   $0_1 = $2_1 - HEAP32[$1_1 + 4 >> 2] | 0;
  }
  global$0 = $3_1 + 32 | 0;
  return $0_1 | 0;
 }
 
 function $7($0_1) {
  $0_1 = $0_1 | 0;
  return abort() | 0;
 }
 
 function $8() {
  var $0_1 = 0, $1_1 = 0, $2_1 = 0;
  $1_1 = 1024;
  while (1) {
   $0_1 = $1_1;
   $1_1 = $0_1 + 4 | 0;
   $2_1 = HEAP32[$0_1 >> 2];
   if (!(($2_1 ^ -1) & $2_1 + -16843009 & -2139062144)) {
    continue
   }
   break;
  };
  if (!($2_1 & 255)) {
   return $0_1 - 1024 | 0
  }
  while (1) {
   $2_1 = HEAPU8[$0_1 + 1 | 0];
   $1_1 = $0_1 + 1 | 0;
   $0_1 = $1_1;
   if ($2_1) {
    continue
   }
   break;
  };
  return $1_1 - 1024 | 0;
 }
 
 function $9() {
  var $0_1 = 0, $1_1 = 0;
  $0_1 = HEAP32[260];
  $0_1;
  $1_1 = $8();
  label$3 : {
   if (((($11($1_1, $0_1) | 0) != ($1_1 | 0) ? -1 : 0) | 0) < 0) {
    break label$3
   }
   label$4 : {
    if (HEAPU8[$0_1 + 75 | 0] == 10) {
     break label$4
    }
    $1_1 = HEAP32[$0_1 + 20 >> 2];
    if ($1_1 >>> 0 >= HEAPU32[$0_1 + 16 >> 2]) {
     break label$4
    }
    HEAP32[$0_1 + 20 >> 2] = $1_1 + 1;
    HEAP8[$1_1 | 0] = 10;
    break label$3;
   }
   $10($0_1);
  }
 }
 
 function $10($0_1) {
  var $1_1 = 0, $2_1 = 0, $3_1 = 0;
  $1_1 = global$0 - 16 | 0;
  global$0 = $1_1;
  HEAP8[$1_1 + 15 | 0] = 10;
  $2_1 = HEAP32[$0_1 + 16 >> 2];
  label$1 : {
   if (!$2_1) {
    if ($1($0_1)) {
     break label$1
    }
    $2_1 = HEAP32[$0_1 + 16 >> 2];
   }
   $3_1 = HEAP32[$0_1 + 20 >> 2];
   if (!(HEAP8[$0_1 + 75 | 0] == 10 | $3_1 >>> 0 >= $2_1 >>> 0)) {
    HEAP32[$0_1 + 20 >> 2] = $3_1 + 1;
    HEAP8[$3_1 | 0] = 10;
    break label$1;
   }
   if ((FUNCTION_TABLE[HEAP32[$0_1 + 36 >> 2]]($0_1, $1_1 + 15 | 0, 1) | 0) != 1) {
    break label$1
   }
  }
  global$0 = $1_1 + 16 | 0;
 }
 
 function $11($0_1, $1_1) {
  var $2_1 = 0, $3_1 = 0;
  $2_1 = $0_1;
  $3_1 = $2_1;
  label$1 : {
   if (HEAP32[$1_1 + 76 >> 2] <= -1) {
    $1_1 = $0($2_1, $1_1);
    break label$1;
   }
   $1_1 = $0($2_1, $1_1);
  }
  if (($3_1 | 0) == ($1_1 | 0)) {
   return $0_1
  }
  return $1_1;
 }
 
 function $12($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  $9();
  return 0;
 }
 
 function $13() {
  
 }
 
 // EMSCRIPTEN_END_FUNCS
;
 FUNCTION_TABLE[1] = $5;
 FUNCTION_TABLE[2] = $6;
 FUNCTION_TABLE[3] = $4;
 function __wasm_memory_size() {
  return buffer.byteLength / 65536 | 0;
 }
 
 return {
  "__wasm_call_ctors": $13, 
  "main": $12, 
  "__growWasmMemory": $7
 };
}

return asmFunc({
    'Int8Array': Int8Array,
    'Int16Array': Int16Array,
    'Int32Array': Int32Array,
    'Uint8Array': Uint8Array,
    'Uint16Array': Uint16Array,
    'Uint32Array': Uint32Array,
    'Float32Array': Float32Array,
    'Float64Array': Float64Array,
    'NaN': NaN,
    'Infinity': Infinity,
    'Math': Math
  },
  wasmImports,
  wasmMemory.buffer
)

}
// EMSCRIPTEN_END_ASM


