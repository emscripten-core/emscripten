/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if MINIMAL_RUNTIME

// runtime_strings_extra.js defines a wrapper around TextDecoder, which is added
// in the generated code. The minimal runtime logic here actually runs the
// library code at compile time (as a way to create a library*.js file around
// non-library JS), and so we must define it here as well.
var TextDecoderWrapper = TextDecoder;

// TODO: stop including this in such a manner, and instead make it a normal
// library file in all modes.
#include "runtime_strings_extra.js"
#include "arrayUtils.js"

mergeInto(LibraryManager.library, {
  $AsciiToString: AsciiToString,
  $stringToAscii: stringToAscii,
  $UTF16ToString: UTF16ToString,
  $stringToUTF16: stringToUTF16,
  $lengthBytesUTF16: lengthBytesUTF16,
  $UTF32ToString: UTF32ToString,
  $stringToUTF32: stringToUTF32,
  $lengthBytesUTF32: lengthBytesUTF32,
  $allocateUTF8: allocateUTF8,
  $allocateUTF8OnStack: allocateUTF8OnStack,
  $writeStringToMemory: writeStringToMemory,
  $writeArrayToMemory: writeArrayToMemory,
  $writeAsciiToMemory: writeAsciiToMemory,
  $intArrayFromString: intArrayFromString,
  $intArrayToString: intArrayToString
});

#endif
