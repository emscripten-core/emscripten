// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines an open file descriptor.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include <mutex>
#include <utility>
#include <wasi/api.h>

#ifdef __cplusplus
extern "C" {
#endif

class OpenFileDescriptor {
  std::shared_ptr<File> file;
  __wasi_filedelta_t offset;
  std::mutex mutex;

public:
  OpenFileDescriptor(uint32_t offset, std::shared_ptr<File> file);

  std::shared_ptr<File>& getFile();
};

#ifdef __cplusplus
}
#endif
