// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Original Source that was modified:
// https://github.com/WebAssembly/binaryen/blob/main/src/tools/fuzzing/random.h

#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace wasmfs {

class Random {
  // The input seed bytes.
  std::vector<char> bytes;
  // The current position in `bytes`.
  size_t pos = 0;
  // Whether we already cycled through all the input (which might mean we should
  // try to finish things off).
  bool finishedInput = false;
  // After we finish the input, we start going through it again, but xoring
  // so it's not identical.
  int xorFactor = 0;

public:
  Random(std::vector<char>&& bytes) : bytes(std::move(bytes)){};

  // Methods for getting random data.
  int8_t get();
  int16_t get16();
  int32_t get32();
  int64_t get64();

  // Returns number in the range of [0,x).
  uint32_t upTo(uint32_t x);

  std::string getString(int8_t size);
  std::string getSingleSymbolString(uint32_t length);
};

} // namespace wasmfs
