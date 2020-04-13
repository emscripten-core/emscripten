/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// These externs are needed for MINIMAL_RUNTIME + USE_PTHREADS + !MODULARIZE
// This file should go away in the future when worker.js is refactored to live inside the JS module.

var ENVIRONMENT_IS_PTHREAD;
/** @suppress {duplicate} */
var wasmMemory;
