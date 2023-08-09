// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines fuzzer workloads which primarily fuzz multi-threaded
// operations.

#pragma once

#include "parameters.h"
#include "random.h"
#include <assert.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>

namespace wasmfs {

// This abstract class defines a fuzzer workload. It sets up a series of file
// system operations and then validates that the expected outcome is observed.
class Workload {
public:
  Workload(Random& rand) : rand(rand) {}
  virtual ~Workload() = default;

  // This function generates a workload based on a Random seed. It will test a
  // file system property, using any required syscalls.
  virtual void execute() = 0;

protected:
  Random& rand;
};

// This workload class attempts to fuzz for tearing in reads and writes.
// This should validate that writes are atomic (i.e. that a read interleaved
// between two writes should not read a portion of the first write and a portion
// of the second write). Writer threads should write a uniform string of the
// same character. Reader threads should validate that file content is not
// intermixed (it should check that all characters in the file content are the
// same).
class ReadWrite : public Workload {

public:
  ReadWrite(Random& rand) : Workload(rand) {}

  void execute() override;

private:
  int fd;
  // Work describes the list of strings being written to a test file.
  std::vector<std::string> work;
  std::atomic<bool> go{false};
  std::atomic<bool> stop{false};

  bool isSame(std::vector<char>& target);
  void reader();
  void writer();
};
} // namespace wasmfs
