// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Original Source that was modified:
// https://github.com/WebAssembly/binaryen/blob/main/src/tools/fuzzing/random.cpp

#include "random.h"

namespace wasmfs {

int8_t Random::get() {
  if (pos == bytes.size()) {
    // We ran out of input; go back to the start for more.
    finishedInput = true;
    pos = 0;
    xorFactor++;
  }
  return bytes[pos++] ^ xorFactor;
}

int16_t Random::get16() {
  auto temp = uint16_t(get()) << 8;
  return temp | uint16_t(get());
}

int32_t Random::get32() {
  auto temp = uint32_t(get16()) << 16;
  return temp | uint32_t(get16());
}

int64_t Random::get64() {
  auto temp = uint64_t(get32()) << 32;
  return temp | uint64_t(get32());
}

uint32_t Random::upTo(uint32_t x) {
  if (x == 0) {
    return 0;
  }
  uint32_t raw;
  if (x <= 255) {
    raw = get();
  } else if (x <= 65535) {
    raw = get16();
  } else {
    raw = get32();
  }
  auto ret = raw % x;
  // use extra bits as "noise" for later
  xorFactor += raw / x;
  return ret;
}

// Returns a random length string of random characters.
std::string Random::getString(int8_t size) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  std::string ret;
  ret.reserve(size);
  for (int i = 0; i < size; ++i) {
    ret += alphanum[upTo(sizeof(alphanum) - 1)];
  }
  return ret;
}

// Returns a random length string of the same character.
std::string Random::getSingleSymbolString(uint32_t length) {
  return std::string(length, getString(1)[0]);
}

} // namespace wasmfs
