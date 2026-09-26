import source wasmModule from "./foo.wasm";

import source otherModule from "./bar.wasm";

function use() {
  return [ wasmModule, otherModule ];
}

use();
