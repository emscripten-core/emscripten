// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// A virtual backend that adapts any underlying backend to be
// case-insensitive. IgnoreCaseDirectory intercepts all directory operations,
// normalizes paths to be lower case, then forwards the operations with the new
// paths to the underlying backend. It stores the original, non-normalized names
// internally so they can be returned later, giving the appearance of a
// case-insensitive but case-preserving file system.
//
// See the comment in virtual.h for an explanation of why DataFiles and Symlinks
// must have no-op wrappers.

#include "backend.h"
#include "file.h"
#include "virtual.h"
#include "wasmfs.h"

namespace {

std::string normalize(const std::string& name) {
  std::string result = name;
  for (auto& ch : result) {
    ch = std::tolower(ch);
  }
  return result;
}

} // anonymous namespace

namespace wasmfs {

class IgnoreCaseDirectory : public VirtualDirectory {

  struct ChildInfo {
    std::string originalName;
    std::shared_ptr<File> child;
  };

  // Map normalized names to virtual files and their non-normalized names.
  std::map<std::string, ChildInfo> children;

public:
  IgnoreCaseDirectory(std::shared_ptr<Directory> real, backend_t backend)
    : VirtualDirectory(real, backend) {}

  std::shared_ptr<File> getChild(const std::string& name) override;
  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode) override;
  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override;
  std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                         const std::string& target) override;
  int insertMove(const std::string& name, std::shared_ptr<File> file) override;
  int removeChild(const std::string& name) override;
  ssize_t getNumEntries() override { return real->locked().getNumEntries(); }
  Directory::MaybeEntries getEntries() override;
  std::string getName(std::shared_ptr<File> file) override;
  bool maintainsFileIdentity() override { return true; }
};

// Wrap a real file in an IgnoreCase virtual file of the same kind.
std::shared_ptr<DataFile> virtualize(std::shared_ptr<DataFile> data,
                                     backend_t backend) {
  return std::make_shared<VirtualDataFile>(data, backend);
}

std::shared_ptr<Directory> virtualize(std::shared_ptr<Directory> dir,
                                      backend_t backend) {
  return std::make_shared<IgnoreCaseDirectory>(dir, backend);
}

std::shared_ptr<Symlink> virtualize(std::shared_ptr<Symlink> link,
                                    backend_t backend) {
  return std::make_shared<VirtualSymlink>(link, backend);
}

std::shared_ptr<File> virtualize(std::shared_ptr<File> file,
                                 backend_t backend) {
  if (auto data = file->dynCast<DataFile>()) {
    return virtualize(data, backend);
  } else if (auto dir = file->dynCast<Directory>()) {
    return virtualize(dir, backend);
  } else if (auto link = file->dynCast<Symlink>()) {
    return virtualize(link, backend);
  }
  WASMFS_UNREACHABLE("unexpected file kind");
}

std::shared_ptr<File> IgnoreCaseDirectory::getChild(const std::string& name) {
  auto normalized = normalize(name);
  if (auto it = children.find(normalized); it != children.end()) {
    return it->second.child;
  }
  auto child = real->locked().getChild(normalized);
  if (!child) {
    return nullptr;
  }
  child = virtualize(child, getBackend());
  children[normalized] = {name, child};
  return child;
}

std::shared_ptr<DataFile>
IgnoreCaseDirectory::insertDataFile(const std::string& name, mode_t mode) {
  auto normalized = normalize(name);
  auto file = real->locked().insertDataFile(normalized, mode);
  if (!file) {
    return nullptr;
  }
  file = virtualize(file, getBackend());
  children[normalized] = {name, file};
  return file;
}

std::shared_ptr<Directory>
IgnoreCaseDirectory::insertDirectory(const std::string& name, mode_t mode) {
  auto normalized = normalize(name);
  auto dir = real->locked().insertDirectory(normalized, mode);
  if (!dir) {
    return nullptr;
  }
  dir = virtualize(dir, getBackend());
  children[normalized] = {name, dir};
  return dir;
}

std::shared_ptr<Symlink>
IgnoreCaseDirectory::insertSymlink(const std::string& name,
                                   const std::string& target) {
  auto normalized = normalize(name);
  auto link = real->locked().insertSymlink(normalized, target);
  if (!link) {
    return nullptr;
  }
  link = virtualize(link, getBackend());
  children[normalized] = {name, link};
  return link;
}

int IgnoreCaseDirectory::insertMove(const std::string& name,
                                    std::shared_ptr<File> file) {
  auto normalized = normalize(name);
  if (auto err = real->locked().insertMove(normalized, devirtualize(file))) {
    return err;
  }
  auto oldParent =
    std::static_pointer_cast<IgnoreCaseDirectory>(file->locked().getParent());
  auto& oldChildren = oldParent->children;
  // Delete the entry in the old parent.
  for (auto it = oldChildren.begin(); it != oldChildren.end(); ++it) {
    if (it->second.child == file) {
      oldChildren.erase(it);
      break;
    }
  }
  // Unlink the overwritten entry if it exists.
  auto [it, inserted] = children.insert({normalized, {name, file}});
  if (!inserted) {
    it->second.child->locked().setParent(nullptr);
    it->second = {name, file};
  }

  return 0;
}

int IgnoreCaseDirectory::removeChild(const std::string& name) {
  auto normalized = normalize(name);
  if (auto err = real->locked().removeChild(normalized)) {
    return err;
  }
  auto it = children.find(normalized);
  assert(it != children.end());
  it->second.child->locked().setParent(nullptr);
  children.erase(it);
  return 0;
}

Directory::MaybeEntries IgnoreCaseDirectory::getEntries() {
  auto entries = real->locked().getEntries();
  if (entries.getError()) {
    return entries;
  }
  for (auto& entry : *entries) {
    if (auto it = children.find(entry.name); it != children.end()) {
      entry.name = it->second.originalName;
    }
  }
  return entries;
}

std::string IgnoreCaseDirectory::getName(std::shared_ptr<File> file) {
  for (auto& [_, info] : children) {
    if (info.child == file) {
      return info.originalName;
    }
  }
  return "";
}

class IgnoreCaseBackend : public Backend {
  backend_t backend;

public:
  IgnoreCaseBackend(std::function<backend_t()> createBackend) {
    backend = createBackend();
  }

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return virtualize(backend->createFile(mode), this);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    auto real = backend->createDirectory(mode);
    // Inserts into the real backing directory won't work if it doesn't appear
    // to be linked, so give it a parent.
    // TODO: Break this reference cycle in a destructor somewhere.
    real->locked().setParent(real);
    auto ret = virtualize(real, this);
    return ret;
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    return virtualize(backend->createSymlink(target), this);
  }
};

// Create an ignore case backend by supplying another backend.
backend_t createIgnoreCaseBackend(std::function<backend_t()> createBackend) {
  return wasmFS.addBackend(std::make_unique<IgnoreCaseBackend>(createBackend));
}

extern "C" {

// C API FOR creating an ignore case backend by supplying a pointer to another
// backend.
backend_t wasmfs_create_icase_backend(backend_t backend) {
  return createIgnoreCaseBackend([backend]() { return backend; });
}

} // extern "C"

} // namespace wasmfs
