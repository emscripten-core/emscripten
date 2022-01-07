// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines fuzzer workloads which primarily fuzz multi-threaded
// operations.

#include "workload.h"

namespace wasmfs {

bool ReadWrite::isSame(std::vector<char>& target) {
  std::set<char> seen;

  for (auto it = target.begin(); it != target.end() - 1; it++) {
    seen.insert(*it);
    if (seen.size() > 1) {
      printf("Read the following incorrect data: %s\n", target.data());
      return false;
    }
  }
  return true;
}

void ReadWrite::writer() {
  while (!go) {
    std::this_thread::yield();
  }

  while (!stop) {
    if (VERBOSE) {
      std::cout << std::this_thread::get_id() << " is running.\n";
    }
    auto current = rand.upTo(work.size());

    int bytesWritten =
      pwrite(fd, work[current].c_str(), work[current].size(), 0);

    assert(bytesWritten == work[current].size());
  }
}

void ReadWrite::reader() {
  while (!go) {
    std::this_thread::yield();
  }
  if (VERBOSE) {
    std::cout << std::this_thread::get_id() << " is running.\n";
  }

  int size = work.begin()->size();
  // Add room for null terminator.
  std::vector<char> buf(size + 1, 0);

  while (!stop) {
    std::fill(buf.begin(), buf.end(), 0);

    int bytesRead = pread(fd, buf.data(), size, 0);
    if (VERBOSE) {
      printf("Read the following data: %s\n", buf.data());
      printf("Bytes read: %i", bytesRead);
      printf("Expected: %lu", work.begin()->size());
    }
    assert(bytesRead == work.begin()->size());

    assert(isSame(buf));
  }
}

void ReadWrite::execute() {
  // This function should produce a string that consists of 1 character type.
  work.resize(1 + rand.upTo(MAX_WORK));

  for (int i = 0; i < work.size(); i++) {
    work[i] = rand.getSingleSymbolString(FILE_SIZE);
  }
  // Generate a test file to read and write to.
  fd = open("test", O_CREAT | O_RDWR, 0777);
  assert(fd != -1);

  // Populate the file with initial data.
  assert(pwrite(fd, work.begin()->c_str(), work.begin()->size(), 0) != -1);

  // Create writer threads.
  std::vector<std::thread> writers;
  std::vector<std::thread> readers;

  for (int i = 0; i < NUM_WRITERS; i++) {
    std::thread th(&ReadWrite::writer, this);
    writers.emplace_back(std::move(th));
  }

  for (int i = 0; i < NUM_READERS; i++) {
    std::thread th(&ReadWrite::reader, this);
    readers.emplace_back(std::move(th));
  }

  // This atomic bool ensures that all threads have an equal chance of
  // starting first.
  go.store(true);

  std::this_thread::sleep_for(std::chrono::seconds(DURATION));

  // Stop file operations after pre-defined duration.
  stop.store(true);

  for (auto& th : writers) {
    th.join();
  }

  for (auto& th : readers) {
    th.join();
  }

  // Clean up files.
  unlink("test");
  close(fd);
}
} // namespace wasmfs
