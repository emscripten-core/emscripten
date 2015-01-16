function test() {
 if ((HEAP32[i5 >> 2] | 0) != 0) {
  if ((_fread(i4 | 0, 1, 257, HEAP32[i5 >> 2] | 0) | 0) != 257) {
   i31 = 9;
  } else {
   if ((HEAPU8[i4 >> 0] | 0 | 0) != (HEAP32[84328] | 0)) {
    i31 = 9;
   } else {
    if ((_memcmp(i4 + 1 | 0, HEAP32[i20 >> 2] | 0, 256) | 0) != 0) {
     i31 = 9;
    } else {
     if ((_fread(HEAP32[i16 >> 2] | 0, 256, 256, HEAP32[i5 >> 2] | 0) | 0) != 256) {
      i31 = 9;
     }
    }
   }
  }
 } else {
  i31 = 9;
 }
 barrier();
 if ((HEAP32[i5 >> 2] | 0) != 0) {
  if ((_fread(i4 | 0, 1, 257, HEAP32[i5 >> 2] | 0) | 0) != 257) {
   if ((HEAPU8[i4 >> 0] | 0 | 0) != (HEAP32[84328] | 0)) {
    if ((_memcmp(i4 + 1 | 0, HEAP32[i20 >> 2] | 0, 256) | 0) != 0) {
     if ((_fread(HEAP32[i16 >> 2] | 0, 256, 256, HEAP32[i5 >> 2] | 0) | 0) != 256) {
      i31 = 9;
     }
    }
   }
  }
 } else {
  i31 = 9;
 }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test"]

