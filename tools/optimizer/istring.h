// Interned String type, 100% interned on creation. Comparisons are always just a pointer comparison

#include <unordered_set>
#include <unordered_map>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

namespace cashew {

struct IString {
  const char *str;

  static size_t hash_c(const char *str) { // TODO: optimize?
    uint64_t ret = 0;
    while (*str) {
      ret = (ret*6364136223846793005ULL) + *str;
      str++;
    }
    return (size_t)ret;
  }

  class CStringHash : public std::hash<const char *> {
  public:
    size_t operator()(const char *str) const {
      return IString::hash_c(str);
    }
  };
  class CStringEqual : public std::equal_to<const char *> {
  public:
    bool operator()(const char *x, const char *y) const {
      return strcmp(x, y) == 0;
    }
  };
  typedef std::unordered_set<const char *, CStringHash, CStringEqual> StringSet;
  static StringSet strings;

  IString() : str(nullptr) {}
  IString(const char *s, bool reuse=true) { // if reuse=true, then input is assumed to remain alive; not copied
    set(s, reuse);
  }

  void set(const char *s, bool reuse=true) {
    if (reuse) {
      auto result = strings.insert(s); // if already present, does nothing
      str = *(result.first);
    } else {
      auto existing = strings.find(s);
      if (existing == strings.end()) {
        char *copy = (char*)malloc(strlen(s)+1); // XXX leaked
        strcpy(copy, s);
        s = copy;
      } else {
        s = *existing;
      }
      strings.insert(s);
      str = s;
    }
  }

  void set(const IString &s) {
    str = s.str;
  }

  bool operator==(const IString& other) const {
    //assert((str == other.str) == !strcmp(str, other.str));
    return str == other.str; // fast!
  }
  bool operator!=(const IString& other) const {
    //assert((str == other.str) == !strcmp(str, other.str));
    return str != other.str; // fast!
  }

  char operator[](int x) {
    return str[x];
  }

  bool operator!() { // no string, or empty string
    return !str || str[0] == 0;
  }

  const char *c_str() const { return str; }

  bool isNull() { return str == nullptr; }
};

} // namespace cashew

// Utilities for creating hashmaps/sets over IStrings

namespace std {

template <> struct hash<cashew::IString> : public unary_function<cashew::IString, size_t> {
  size_t operator()(const cashew::IString& str) const {
    return cashew::IString::hash_c(str.c_str());
  }
};

template <> struct equal_to<cashew::IString> : public binary_function<cashew::IString, cashew::IString, bool> {
  bool operator()(const cashew::IString& x, const cashew::IString& y) const {
    return x == y;
  }
};

} // namespace std

namespace cashew {

// IStringSet

class IStringSet : public std::unordered_set<IString> {
public:
  IStringSet() {}
  IStringSet(const char *init) { // comma-delimited list
    int size = strlen(init);
    char *curr = (char*)malloc(size+1); // leaked!
    strcpy(curr, init);
    while (1) {
      char *end = strchr(curr, ' ');
      if (end) *end = 0;
      insert(curr);
      if (!end) break;
      curr = end + 1;
    }
  }

  bool has(const IString& str) {
    return count(str) > 0;
  }
};

} // namespace cashew

