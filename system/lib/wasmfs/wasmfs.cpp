#include "wasmfs.h"

namespace wasmfs {
std::shared_ptr<Directory> WasmFS::getRootDir() {
  static const std::shared_ptr<Directory> rootDirectory = [] {
    std::shared_ptr<Directory> rootDirectory =
      std::make_shared<Directory>(S_IRUGO | S_IXUGO);
    auto devDirectory = std::make_shared<Directory>(S_IRUGO | S_IXUGO);
    rootDirectory->locked().setEntry("dev", devDirectory);

    auto dir = devDirectory->locked();

    dir.setEntry("stdin", StdinFile::getSingleton());
    dir.setEntry("stdout", StdoutFile::getSingleton());
    dir.setEntry("stderr", StderrFile::getSingleton());

    return rootDirectory;
  }();

  return rootDirectory;
}
} // namespace wasmfs
