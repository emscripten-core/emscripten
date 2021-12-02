// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines global parameters used in the fuzzer.

#pragma once

namespace wasmfs {
//
// ReadWrite Parameters
//
// The number of writer threads.
constexpr uint32_t NUM_WRITERS = 10;

// The number of reader threads.
constexpr uint32_t NUM_READERS = 10;

// The size of the character strings written to a file.
constexpr uint32_t NUM_WORK_SIZE = 21474836;

// The number of strings that are written/read.
constexpr uint32_t NUM_WORK = 10;
} // namespace wasmfs
