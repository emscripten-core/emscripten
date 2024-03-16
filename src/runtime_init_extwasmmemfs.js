/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if WASMFS
if (!ENVIRONMENT_IS_WORKER) {
  Module["extWasmMemFS"] = {
    "dataFiles": new WebAssembly.Memory({
      initial: 1 << 9, // 32 MB
      maximum: 1 << 16, // 4 GB
      shared: true,
    }),
    "control": new SharedArrayBuffer(64),
    "index": new SharedArrayBuffer(2 * 1024 * 1024 * 8), // 2M files max.
  }

  let __extWasmMem$dataFile = new Uint32Array(Module["extWasmMemFS"]["dataFiles"].buffer);
  __extWasmMem$dataFile[0] = 0; //ensure block 0 is unusable.
  __extWasmMem$dataFile[1] = 0;

  let __extWasmMem$control = new Uint32Array(Module["extWasmMemFS"]["control"]);
  __extWasmMem$control[3] = 0;
  __extWasmMem$control[4] = 8;
}
#endif