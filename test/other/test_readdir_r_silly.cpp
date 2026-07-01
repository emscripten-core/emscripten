#include <cassert>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

using std::endl;

// Do a recursive directory listing of the directory whose path is specified
// by \a name.
void ls(const std::string& name, size_t indent = 0) {
  DIR *dir;
  struct dirent *entry;
  if (indent == 0) {
    std::cout << name << endl;
    ++indent;
  }
  // Make sure we can open the directory.  This should also catch cases where
  // the empty string is passed in.
  if (not (dir = opendir(name.c_str()))) {
    const int error = errno;
    std::cout
      << "Failed to open directory: " << name << "; " << error << endl;
    return;
  }
  // Just checking the sanity.
  if (name.empty()) {
    std::cout
      << "Managed to open a directory whose name was the empty string.."
      << endl;
    assert(closedir(dir) != -1);
    return;
  }
  // Iterate over the entries in the directory.
  while ((entry = readdir(dir))) {
    std::string entryName(entry->d_name);
    if (entryName == "." || entryName == "..") {
      // Skip the dot entries.
      continue;
    }
    std::string indentStr(indent * 2, ' ');
    if (entryName.empty()) {
      std::cout
        << indentStr << "\"\": Found empty string as a "
        << (entry->d_type == DT_DIR ? "directory" : "file")
        << " entry!" << endl;
      continue;
    } else {
      std::cout << indentStr << entryName
                << (entry->d_type == DT_DIR ? "/" : "") << endl;
    }
    if (entry->d_type == DT_DIR) {
      // We found a subdirectory; recurse.
      ls(std::string(name + (name == "/" ? "" : "/" ) + entryName),
         indent + 1);
    }
  }
  // Close our handle.
  assert(closedir(dir) != -1);
}

void touch(const char* path) {
  int fd = open(path, O_CREAT | O_TRUNC, 0644);
  assert(fd != -1);
  assert(close(fd) != -1);
}

int main() {
  assert(mkdir("dir", 0755) == 0);
  touch("dir/a");
  touch("dir/b");
  touch("dir/c");
  touch("dir/d");
  touch("dir/e");
  std::cout << "Before:" << endl;
  ls("dir");
  std::cout << endl;
  // Attempt to delete entries as we walk the (single) directory.
  DIR* dir = opendir("dir");
  assert(dir != NULL);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    std::string name(entry->d_name);
    // Skip "." and "..".
    if (name == "." || name == "..") {
      continue;
    }
    // Unlink it.
    std::cout << "Unlinking " << name << endl;
    assert(unlink(("dir/" + name).c_str()) != -1);
  }
  assert(closedir(dir) != -1);
  std::cout << "After:" << endl;
  ls("dir");
  std::cout << "done" << endl;
  return 0;
}
