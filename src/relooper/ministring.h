
// Tiny implementation of strings. Avoids linking in all of std::string

#include <stdlib.h>
#include <string.h>

class ministring {
  int used;
  char *buffer;
  int bufferSize;
public:
  ministring() : used(0), buffer(NULL), bufferSize(0) {}
  ~ministring() { if (buffer) free(buffer); }

  char *c_str() { return buffer; }
  int size() { return used; }

  void clear() {
    used = 0; // keep the buffer alive as an optimization, just resize
  }

  ministring& operator+=(const char *s) {
    int len = strlen(s);
    if (used + len + 2 > bufferSize) {
      // try to avoid frequent reallocations
      bufferSize = 2*(bufferSize + len);
      bufferSize += 1024 - bufferSize % 1024;
      buffer = (char*)(buffer ? realloc(buffer, bufferSize) : malloc(bufferSize));
    }
    strcpy(buffer + used, s);
    used += len;
    return *this;
  }
};

