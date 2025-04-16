// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// https://github.com/emscripten-core/emscripten/issues/8807

#include <optional>

int main() {
  auto x = std::optional{24};
  return 0;
}
