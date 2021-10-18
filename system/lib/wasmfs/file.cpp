// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"

namespace wasmfs {
template<class T> bool File::is() const {
  static_assert(std::is_base_of<File, T>::value,
                "File is not a base of destination type T");
  return int(kind) == int(T::expectedKind);
}

template<class T> T* File::dynCast() {
  static_assert(std::is_base_of<File, T>::value,
                "File is not a base of destination type T");
  return int(kind) == int(T::expectedKind) ? (T*)this : nullptr;
}

template<class T> T* File::cast() {
  static_assert(std::is_base_of<File, T>::value,
                "File is not a base of destination type T");
  assert(int(kind) == int(T::expectedKind));
  return (T*)this;
}

std::shared_ptr<File> Directory::Handle::getEntry(std::string pathName) {
  auto it = getDir().entries.find(pathName);
  if (it == getDir().entries.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

__wasi_errno_t
MemoryFile::write(const uint8_t* buf, __wasi_size_t len, size_t offset) {
  buffer.resize(buffer.size() + len);
  memcpy(&buffer[offset], buf, len * sizeof(uint8_t));

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t
MemoryFile::read(uint8_t* buf, __wasi_size_t len, size_t offset) override {
  std::memcpy(buf, &buffer[offset], len);

  return __WASI_ERRNO_SUCCESS;
};
} // namespace wasmfs