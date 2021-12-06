// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines global parameters used in the fuzzer.

#pragma once

namespace wasmfs {
//
// Global var for verbose printing.
//
extern bool VERBOSE;

//
// ReadWrite Parameters
//
// The number of writer threads.
constexpr uint32_t NUM_WRITERS = 1;

// The number of reader threads.
constexpr uint32_t NUM_READERS = 20;

// The size of the character strings written to a file.
constexpr uint32_t FILE_SIZE = 20000000;

// The number of strings that are written/read.
constexpr uint32_t MAX_WORK = 10;

// The time interval that a reader or writer thread is active.
constexpr uint32_t DURATION = 100;
} // namespace wasmfs
