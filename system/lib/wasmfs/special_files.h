// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#pragma once

#include "file.h"

namespace wasmfs::SpecialFiles {

// /dev/null
std::shared_ptr<DataFile> getNull();

// /dev/stdin
std::shared_ptr<DataFile> getStdin();

// /dev/stdout
std::shared_ptr<DataFile> getStdout();

// /dev/stderr
std::shared_ptr<DataFile> getStderr();

// /dev/random
std::shared_ptr<DataFile> getRandom();

// /dev/urandom
std::shared_ptr<DataFile> getURandom();

} // namespace wasmfs::SpecialFiles
