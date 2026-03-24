/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !WASM_EXCEPTIONS

// Base Emscripten EH error class
#if EXCEPTION_STACK_TRACES
class EmscriptenEH extends Error {}
#else
class EmscriptenEH {}
#endif

#if SUPPORT_LONGJMP == 'emscripten'
class EmscriptenSjLj extends EmscriptenEH {}
#endif

#if !DISABLE_EXCEPTION_CATCHING
class CppException extends EmscriptenEH {
  constructor(excPtr) {
#if EXCEPTION_STACK_TRACES
    super(excPtr);
#else
    super();
#endif
    this.excPtr = excPtr;
#if !DISABLE_EXCEPTION_CATCHING && EXCEPTION_STACK_TRACES
    const excInfo = getExceptionMessage(this);
    this.name = excInfo[0];
    this.message = excInfo[1];
#endif
  }
}
#endif

#endif // !WASM_EXCEPTIONS
