/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if EXCEPTION_STACK_TRACES && !WASM_EXCEPTIONS
// Base Emscripten EH error class
class EmscriptenEH extends Error {}

#if SUPPORT_LONGJMP == 'emscripten'
class EmscriptenSjLj extends EmscriptenEH {}
#endif

class CppException extends EmscriptenEH {
  constructor(excPtr) {
    super(excPtr);
    this.excPtr = excPtr;
#if !DISABLE_EXCEPTION_CATCHING
    const excInfo = getExceptionMessage(excPtr);
    this.name = excInfo[0];
    this.message = excInfo[1];
#endif
  }
}
#endif // EXCEPTION_STACK_TRACES && !WASM_EXCEPTIONS
