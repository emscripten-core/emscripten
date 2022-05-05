// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the standard streams of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"

namespace wasmfs::SpecialFiles {

// /dev/stdin/
std::shared_ptr<DataFile> getStdin();

// /dev/stdout
std::shared_ptr<DataFile> getStdout();

// /dev/stderr
std::shared_ptr<DataFile> getStderr();

} // namespace wasmfs::SpecialFiles
