#if MINIMAL_RUNTIME

#include "runtime_strings_extra.js"

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
  $writeAsciiToMemory: writeAsciiToMemory
});

#endif
