#!/usr/bin/env node --expose-wasm
/* Copyright 2018 Mozilla Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const fs = require("fs");

async function convertDwarfToJSON(wasm, enableXScopes = false) {
    const dwarf_to_json_code = fs.readFileSync(__dirname + "/dwarf_to_json.wasm");
    const {instance: {exports: { alloc_mem, free_mem, convert_dwarf, memory }}} =
        await WebAssembly.instantiate(dwarf_to_json_code);

    const wasmPtr = alloc_mem(wasm.byteLength);
    new Uint8Array(memory.buffer, wasmPtr, wasm.byteLength).set(new Uint8Array(wasm));
    const resultPtr = alloc_mem(12);
    convert_dwarf(wasmPtr, wasm.byteLength, resultPtr, resultPtr + 4, enableXScopes);
    free_mem(wasmPtr);
    const resultView = new DataView(memory.buffer, resultPtr, 12);
    const outputPtr = resultView.getUint32(0, true), outputLen = resultView.getUint32(4, true);
    free_mem(resultPtr);
    const output = Buffer.from(new Uint8Array(memory.buffer, outputPtr, outputLen)).toString("utf8");
    free_mem(outputPtr);
    return output;
}

if (process.argv.length <= 2) {
    console.error("USAGE: dwarf_to_json <input-wasm> <output-map>");
    process.exit(1);
}

const input = process.argv[2];
const output = process.argv[3];

const wasm = fs.readFileSync(input);
convertDwarfToJSON(wasm, false).then(json => {
    if (output) {
        fs.writeFileSync(output, json);
    } else {
        process.stdout.write(json);
    }
});
