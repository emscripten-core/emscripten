// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the Ignore Case Backend of the new file system.
// It is a virtual backend that normalizes all file paths to lower case.

#include "backend.h"
#include "file.h"
#include "memory_backend.h"
#include "wasmfs.h"

namespace {

std::string normalize(const std::string& name) {
  std::string result = name;
  for (auto& ch : result) {
    ch = std::tolower(ch);
  }
  return result;
}
} // namespace

namespace wasmfs {

// Normalizes case and then forwards calls to a directory from underlying
// backend `baseDirectory`. Code is based on `MemoryDirectory`.
class IgnoreCaseDirectory : public Directory {

  // Use a vector instead of a map to save code size.
  struct ChildEntry {
    std::string name; // Normalized name.
    std::string origName;
    std::shared_ptr<File> child;
  };

  std::vector<ChildEntry> entries;
  std::shared_ptr<Directory> baseDirectory;

  std::vector<ChildEntry>::iterator findEntry(const std::string& name) {
    return std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
      return entry.name == name;
    });
  }

  void insertChild(const std::string& originalName,
                   const std::string& normalizedName,
                   std::shared_ptr<File> child) {
    assert(findEntry(normalizedName) == entries.end());
    entries.push_back({normalizedName, originalName, child});
  }

public:
  IgnoreCaseDirectory(std::shared_ptr<Directory> base, backend_t proxyBackend)
    : Directory(base->locked().getMode(), proxyBackend), baseDirectory(base) {}

  std::shared_ptr<File> getChild(const std::string& name) override {
    auto name2 = normalize(name);
    if (auto entry = findEntry(name2); entry != entries.end()) {
      return entry->child;
    }
    return nullptr;
  }

  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode) override {
    auto name2 = normalize(name);
    auto baseDirLocked = baseDirectory->locked();
    auto child = baseDirLocked.insertDataFile(name2, mode);
    if (child) {
      insertChild(name, name2, child);
      // Directory::Handle needs a parent
      child->locked().setParent(cast<Directory>());
    }
    return child;
  }

  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    auto name2 = normalize(name);
    auto baseDirLocked = baseDirectory->locked();
    if (!baseDirLocked.getParent()) {
      // Directory::Handle needs a parent
      assert(parent.lock());
      baseDirLocked.setParent(parent.lock());
    }
    auto baseChild = baseDirLocked.insertDirectory(name2, mode);
    auto child = std::make_shared<IgnoreCaseDirectory>(baseChild, getBackend());
    insertChild(name, name2, child);
    return child;
  }

  std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                         const std::string& target) override {
    auto name2 = normalize(name);
    auto child = baseDirectory->locked().insertSymlink(name2, target);
    if (child) {
      insertChild(name, name2, child);
      // Directory::Handle needs a parent
      child->locked().setParent(cast<Directory>());
    }
    return child;
  }

  int insertMove(const std::string& name, std::shared_ptr<File> file) override {
    auto name2 = normalize(name);
    // Remove entry with the new name (if any) from this directory.
    if (auto err = removeChild(name))
      return err;
    auto oldParent = file->locked().getParent()->locked();
    auto oldName = oldParent.getName(file);
    auto oldName2 = normalize(oldName);
    // Move in underlying directory.
    if (auto err = baseDirectory->locked().insertMove(name2, file))
      return err;
    // Ensure old file was removed.
    if (auto err = oldParent.removeChild(oldName2))
      return err;
    // Cache file with the new name in this directory.
    insertChild(name, name2, file);
    file->locked().setParent(cast<Directory>());
    return 0;
  }

  int removeChild(const std::string& name) override {
    auto name2 = normalize(name);
    auto entry = findEntry(name2);
    if (entry != entries.end()) {
      entry->child->locked().setParent(nullptr);
      entries.erase(entry);
    }
    return baseDirectory->locked().removeChild(name2);
  }

  ssize_t getNumEntries() override {
    return baseDirectory->locked().getNumEntries();
  }

  Directory::MaybeEntries getEntries() override {
    auto result = baseDirectory->locked().getEntries();
    if (result.getError()) {
      return result;
    }
    // Restore original case.
    for (size_t i = 0; i != result->size(); ++i) {
      auto& x = result->at(i);
      auto it = findEntry(normalize(x.name));
      if (it != entries.end()) {
        x.name = it->origName;
      }
    }
    return result;
  }

  std::string getName(std::shared_ptr<File> file) override {
    for (auto&& x : entries) {
      if (x.child == file)
        return x.origName;
    }
    return {};
  }

  // Don't use `dcache` because of underlying backend and case differences.
  bool maintainsFileIdentity() override { return true; }
};

class IgnoreCaseBackend : public Backend {
  backend_t backend;

public:
  IgnoreCaseBackend(std::function<backend_t()> createBackend) {
    backend = createBackend();
  }

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return backend->createFile(mode);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<IgnoreCaseDirectory>(backend->createDirectory(mode),
                                                 this);
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    return backend->createSymlink(normalize(target));
  }
};

// Create an ignore case backend by supplying another backend.
backend_t createIgnoreCaseBackend(std::function<backend_t()> createBackend) {
  return wasmFS.addBackend(std::make_unique<IgnoreCaseBackend>(createBackend));
}

extern "C" {
// C API for creating ignore case backend.
backend_t wasmfs_create_icase_backend(backend_constructor_t create_backend,
                                      void* arg) {
  return createIgnoreCaseBackend(
    [create_backend, arg]() { return create_backend(arg); });
}
}

} // namespace wasmfs
