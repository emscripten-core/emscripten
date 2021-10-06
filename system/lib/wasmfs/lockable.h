// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines a locking object for resources.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

// Locked represents an RAII wrapper object. Access to any template object must go through Locked. A
// Locked<T> holds Lockable's lock for the duration of its lifetime.
template <class T> class Locked {
  T& resource;
  std::unique_lock<std::mutex> lock;

public:
  Locked(T& resource, std::mutex& mutex) : resource(resource), lock(mutex, std::defer_lock) {
#ifdef WASMFS_DEBUG
    EM_ASM({out("Lock acquired")});
#endif
    lock.lock();
  }

  // Default Move Constructor
  Locked(Locked&&) = default;

  ~Locked() {
#ifdef WASMFS_DEBUG
    if (lock) {
      EM_ASM({out("Lock released")});
    }
#endif
  }

  T* operator->() { return &resource; }

  T& operator*() { return resource; }
};

// Lockable represents a wrapper that holds a resource object such as the FileTable. It returns a
// Locked<T> to provide protected access to the resource.
template <typename T> class Lockable {

  T resource;
  std::mutex mutex;

public:
  template <typename... Args> Lockable(Args&&... args) : resource(std::forward<Args>(args)...) {}

  // Acquire a Locked<T> to access resource object.
  Locked<T> get() { return Locked<T>(resource, mutex); }
};