#include <string>
#include <string_view>
#include <vector>

#include <emscripten.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// Splits a path into nonempty path segments. For example, "/path/to/here" will
// be turned into ["path", "to", "here"].
// TODO: move into a shared place, as this might be useful in WasmFS etc.
std::vector<std::string_view> splitPath(std::string_view path) {
  std::vector<std::string_view> ret;
  while (true) {
    // Skip any leading '/' for each segment.
    while (!path.empty() && path.front() == '/') {
      path.remove_prefix(1);
    }

    if (path.empty()) {
      // The path ended in a '/' which we just removed.
      return ret;
    }

    // If this is the leaf segment, return.
    size_t segment_end = path.find_first_of('/');
    if (segment_end == std::string_view::npos) {
      ret.push_back(path);
      return ret;
    }

    // Continue to the child segment.
    auto segment = path.substr(0, segment_end);
    if (!segment.empty()) {
      ret.push_back(segment);
    }
    path.remove_prefix(segment_end);
  }
  return ret;
}

extern "C" {

void emscripten_wget(const char* url, const char* file) {
  // Create the ancestor directories.
  auto segments = splitPath(file);
  if (segments.empty()) {
    return;
  }
  std::string currParent;
  for (size_t i = 0; i < segments.size() - 1; i++) {
    currParent += segments[i];
    currParent += '/';
    int result = mkdir(currParent.c_str(), 0777);
    // Continue while we succeed in creating directories or while we see that
    // they already exist.
    if (result < 0 && errno != EEXIST) {
      return;
    }
  }

  // Fetch the data.
  void* buffer;
  int num;
  int error;
  emscripten_wget_data(url, &buffer, &num, &error);
  if (error) {
    return;
  }

  // Write the data.
  int fd = open(file, O_WRONLY | O_CREAT);
  if (fd >= 0) {
    write(fd, buffer, num);
    close(fd);
  }
  free(buffer);
}

}
