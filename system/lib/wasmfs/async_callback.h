// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "sys/types.h"
#include "wasi/api.h"

// Callbacks for the async API between C and JS. This is declared in a small
// separate header for convenience of gen_struct_info.

// Callbacks take a pointer to a CallbackState structure, which contains both
// the function to call to resume execution, and storage for any out params.
// Basically this stores the state during an async call.
struct CallbackState {
  // The result of the operation, either success or an error code.
  __wasi_errno_t result;

  // Some syscalls return an offset.
  off_t offset;
};
