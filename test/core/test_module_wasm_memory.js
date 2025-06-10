#preprocess
/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

Module['wasmMemory'] = new WebAssembly.Memory({
#if MEMORY64 == 1
  'initial': 256n,
  'maximum': 256n,
  'address': 'i64',
#else
  'initial': 256,
  'maximum': 256,
#endif
});
