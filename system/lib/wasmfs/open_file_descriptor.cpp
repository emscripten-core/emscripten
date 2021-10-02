// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines an open file descriptor.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "open_file_descriptor.h"

extern "C" {

OpenFileDescriptor::OpenFileDescriptor(uint32_t offset, std::shared_ptr<File> file)
  : offset(offset), file(file) {}

std::shared_ptr<File>& OpenFileDescriptor::getFile() { return file; }
}
