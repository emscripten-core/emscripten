// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using std::endl;

//============================================================================
// :: Helpers

namespace
{
  //--------------------------------------------------------------------------
  // Helper to create an empty file with the given path.
  void touch(const std::string& path, const mode_t mode)
  {
    std::cout
      << "Touching file: " << path << " with mode=" << std::oct << mode
      << std::dec <<  endl;

    const int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, mode);
    if (fd == -1) {
      const int error = errno;
      std::cout
        << "Failed to touch file using open: " << path << "; errno=" << error
        << ";" << std::strerror(error) << endl;
    }
    else {
      ::close(fd);
    }
  }

  //--------------------------------------------------------------------------
  // Stats the given path and prints the mode.  Returns true if the path
  // exists; false otherwise.
  bool exists(const std::string& path)
  {
    struct ::stat path_stat;
    if (::lstat(path.c_str(), &path_stat) != 0) {
      const int error = errno;
      if (error == ENOENT) {
        // Only bother logging if something other than the path not existing
        // went wrong.
        std::cout
          << "Failed to lstat path: " << path << "; errno=" << error << "; "
          << std::strerror(error) << endl;
      }
      return false;
    }

    std::cout
      << std::oct << "Mode for path=" << path << ": " << path_stat.st_mode
      << std::dec << endl;
    return true;
  }
}

//============================================================================
// :: Entry Point

int main()
{
  touch("file1", 0667);
  if (not exists("file1")) {
    std::cout << "Failed to create path: file1" << endl;
    return 1;
  }
  if (exists("file1/dir")) {
    std::cout << "Path should not exists: file1/dir" << endl;
    return 1;
  }

  touch("file2", 0676);
  if (not exists("file2")) {
    std::cout << "Failed to create path: file2" << endl;
    return 1;
  }
  if (exists("file2/dir")) {
    std::cout << "Path should not exists: file2/dir" << endl;
    return 1;
  }

  touch("file3", 0766);
  if (not exists("file3")) {
    std::cout << "Failed to create path: file3" << endl;
    return 1;
  }
  if (exists("file3/dir")) {
    std::cout << "Path should not exists: file3/dir" << endl;
    return 1;
  }

  std::cout << "ok." << endl;
  return 0;
}

