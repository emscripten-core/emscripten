// Source-phase imports (https://github.com/tc39/proposal-source-phase-imports).
// The acorn optimizer must parse these via the acorn-import-phases plugin and
// preserve the `source` phase keyword through the terser from_mozilla_ast ->
// print round-trip used at -O2+.
import source wasmModule from './foo.wasm';
import source otherModule from './bar.wasm';

function use() {
  return [wasmModule, otherModule];
}
use();
