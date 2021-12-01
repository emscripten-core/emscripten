// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines fuzzer workloads which primarily fuzz multi-threaded
// operations.

#pragma once

#include <assert.h>
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
protected:
  Random& rand;

public:
  Workload(Random& rand) : rand(rand) {}
  virtual ~Workload() = default;

  // This function should only generate the Random data that seeds a workload.
  // This could be a list of files to rename or random file content to write.
  virtual void generate() = 0;

  // This function defines the actual file system property that should be
  // validated. Any file system syscalls should be called by this function.
  virtual void execute() = 0;
};

// This workload class attempts to fuzz for tearing in reads and writes.
// This should validate that writes are atomic (i.e. that a read interleaved
// between two reads should not read a portion of the first read and a portion
// of the second read). Writer threads should write a uniform string of the same
// character. Reader threads should validate that file content is not intermixed
// (it should check that all characters in the file content are the same).
class ReadWrite : public Workload {
  int fd;
  std::vector<std::string> work;
  std::unordered_map<std::thread::id, std::string> threads;
  std::atomic<bool> go{false};

  bool isSame(std::vector<char>& target) {
    std::set<char> seen;

    for (auto it = target.begin(); it != target.end() - 1; it++) {
      seen.insert(*it);
      if (seen.size() > 1) {
#ifdef FUZZER_DEBUG
        for (int i = 0; i < target.size(); i++) {
          if (target[i] == 0) {
            printf("\\0");
          } else {
            printf("%c", target[i]);
          }
        }
#endif
        return false;
      }
    }
    return true;
  }

  void writer() {
    while (!go) {
    }

    for (int i = 0; i < rand.upTo(10); i++) {
#ifdef FUZZER_DEBUG
      std::cout << std::this_thread::get_id() << " is running.\n";
#endif
      auto current = rand.upTo(work.size());

      int bytesWritten =
        pwrite(fd, work[current].c_str(), work[current].size(), 0);
    }
  }

  void reader() {
    while (!go) {
    }
#ifdef FUZZER_DEBUG
    std::cout << std::this_thread::get_id() << " is running.\n";
#endif

    int size = work.begin()->size();
    // Add room for null terminator.
    std::vector<char> buf(size + 1, 0);

    for (int i = 0; i < rand.upTo(10); i++) {
      std::fill(buf.begin(), buf.end(), 0);

      int bytesRead = pread(fd, buf.data(), size, 0);
#ifdef FUZZER_DEBUG
      printf("Read the following data: %s\n", buf.data());
#endif
      assert(bytesRead == work.begin()->size());

      assert(isSame(buf));
    }
  }

public:
  ReadWrite(Random& rand) : Workload(rand) {}

  void generate() override {
    // TODO: parameterize the number of input strings.
    // This function should produce a string that consists of 1 character type.
    auto size = rand.upTo(10) + 1;
    work.resize(size);

    auto length = rand.getNonZero();

    for (int i = 0; i < work.size(); i++) {
      work[i] = rand.getSingleSymbolString(length);
    }
  }

  void execute() override {
    // Generate a test file to read and write to.
    fd = open("test", O_CREAT | O_RDWR, 0777);

    // Populate the file with initial data.
    assert(pwrite(fd, work[0].c_str(), work[0].size(), 0) != -1);

    // Create writer threads.
    std::vector<std::thread> writers;
    std::vector<std::thread> readers;

    // TODO: Parameterize the number of readers and writers.
    for (int i = 0; i < 3; i++) {
      std::thread th(&ReadWrite::writer, this);
      writers.emplace_back(std::move(th));
    }

    for (int i = 0; i < 3; i++) {
      std::thread th(&ReadWrite::reader, this);
      readers.emplace_back(std::move(th));
    }

    // This atomic bool ensures that all threads have an equal chance of
    // starting first.
    go.store(true);

    for (auto& th : writers) {
      th.join();
    }

    for (auto& th : readers) {
      th.join();
    }
  }
};

} // namespace wasmfs
