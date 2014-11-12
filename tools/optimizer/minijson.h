/*
 * Extremely minimal JSON, just enough to manipulate an AST in the format the JS optimizer wants: arrays, strings, numbers, bools (no objects, no non-ascii, etc.).
 * Optimized for fast parsing and also manipulation of the AST.
 * Uses shared_ptr for simplicity, basically everywhere. TODO: measure impact
 */

//#define NDEBUG

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#define err(str) fprintf(stderr, str "\n");
#define errv(str, ...) fprintf(stderr, str "\n", __VA_ARGS__);
#define printErr err

struct IString;
class Ref;
struct Value;

void dump(const char *str, Ref node, bool pretty=false);

// Reference to a value, plus some operators for convenience
struct Ref {
  Value* inst;

  Ref(Value *v=nullptr) : inst(v) {}

  Value* get() { return inst; }

  Value& operator*() { return *inst; }
  Value* operator->() { return inst; }
  Ref& operator[](unsigned x);

  // special conveniences
  bool operator==(const char *str); // comparison to string, which is by value
  bool operator!=(const char *str);
  bool operator==(const IString &str);
  bool operator!=(const IString &str);
  bool operator==(double d) { assert(0); return false; } // prevent Ref == number, which is potentially ambiguous; use ->getNumber() == number
  bool operator==(Ref other);
  bool operator!(); // check if null, in effect
};

// Arena allocation, free it all on process exit

struct Arena {
  #define CHUNK_SIZE 1000
  std::vector<Value*> chunks;
  int index; // in last chunk

  Arena() : index(0) {}

  Ref alloc();
};

Arena arena;

// Interned String type, 100% interned on creation. Comparisons are always just a pointer comparison

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
  IString(const char *s) {
    set(s);
  }

  void set(const char *s) {
    auto result = strings.insert(s); // if already present, does nothing
    str = *(result.first);
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

  const char *c_str() const { return str; }

  bool isNull() { return str == nullptr; }
};

IString::StringSet IString::strings;

// Utilities for creating hashmaps/sets over IStrings

namespace std {

  template <> struct hash<IString> : public unary_function<IString, size_t> {
    size_t operator()(const IString& str) const {
      return IString::hash_c(str.c_str());
    }
  };

  template <> struct equal_to<IString> : public binary_function<IString, IString, bool> {
    bool operator()(const IString& x, const IString& y) const {
      return x == y;
    }
  };

}

// Main value type
struct Value {
  enum Type {
    String = 0,
    Number = 1,
    Array = 2,
    Null = 3,
    Bool = 4
  };

  Type type;

  typedef std::vector<Ref> ArrayStorage;

  union { // TODO: optimize
    IString str;
    double num;
    ArrayStorage *arr;
    bool boo;
  };

  // constructors all copy their input
  Value() : type(Null), num(0) {}
  explicit Value(const char *s) : type(Null) {
    setString(s);
  }
  explicit Value(double n) : type(Null) {
    setNumber(n);
  }
  explicit Value(ArrayStorage &a) : type(Null) {
    setArray();
    *arr = a;
  }
  // no bool constructor - would endanger the double one (int might convert the wrong way)

  ~Value() {
    free();
  }

  void free() {
    if (type == Array) delete arr;
    type = Null;
    num = 0;
  }

  Value& setString(const char *s) {
    free();
    type = String;
    str.set(s);
    return *this;
  }
  Value& setString(const IString &s) {
    free();
    type = String;
    str.set(s);
    return *this;
  }
  Value& setNumber(double n) {
    free();
    type = Number;
    num = n;
    return *this;
  }
  Value& setArray(ArrayStorage &a) {
    free();
    type = Array;
    arr = new ArrayStorage();
    *arr = a;
    return *this;
  }
  Value& setArray() {
    free();
    type = Array;
    arr = new ArrayStorage();
    return *this;
  }
  Value& setNull() {
    free();
    type = Null;
    return *this;
  }
  Value& setBool(bool b) { // Bool in the name, as otherwise might overload over int
    free();
    type = Bool;
    boo = b;
    return *this;
  }

  bool isString() { return type == String; }
  bool isNumber() { return type == Number; }
  bool isArray()  { return type == Array; }
  bool isNull()   { return type == Null; }
  bool isBool()   { return type == Bool; }

  bool isBool(bool b) { return type == Bool && b == boo; } // avoid overloading == as it might overload over int

  const char* getCString() {
    assert(isString());
    return str.str;
  }
  IString& getIString() {
    assert(isString());
    return str;
  }
  double& getNumber() {
    assert(isNumber());
    return num;
  }
  ArrayStorage& getArray() {
    assert(isArray());
    return *arr;
  }
  bool& getBool() {
    assert(isBool());
    return boo;
  }

  Value& operator=(const Value& other) {
    free();
    switch (other.type) {
      case String:
        setString(other.str);
        break;
      case Number:
        setNumber(other.num);
        break;
      case Array:
        setArray(*other.arr);
        break;
      case Null:
        setNull();
        break;
      case Bool:
        setBool(other.boo);
        break;
    }
    return *this;
  }

  bool operator==(const Value& other) {
    if (type != other.type) return false;
    switch (other.type) {
      case String:
        return str == other.str;
      case Number:
        return num == other.num;
      case Array:
        return this == &other; // if you want a deep compare, use deepCompare
      case Null:
        break;
      case Bool:
        return boo == other.boo;
    }
    return true;
  }

  bool deepCompare(Ref ref) {
    Value& other = *ref;
    if (*this == other) return true; // either same pointer, or identical value type (string, number, null or bool)
    if (type != other.type) return false;
    if (type != Array) return false; // Array is the only one where deep compare differs makes sense, others are shallow and were already tested
    if (arr->size() != other.arr->size()) return false;
    for (unsigned i = 0; i < arr->size(); i++) {
      if (!(*arr)[i]->deepCompare((*other.arr)[i])) return false;
    }
    return true;
  }

  char* parse(char* curr) {
    #define is_json_space(x) (x == 32 || x == 9 || x == 10 || x == 13) /* space, tab, linefeed/newline, or return */
    #define skip() { while (*curr && is_json_space(*curr)) curr++; }
    skip();
    if (*curr == '"') {
      // String
      curr++;
      char *close = strchr(curr, '"');
      assert(close);
      *close = 0; // end this string, and reuse it straight from the input
      setString(curr);
      curr = close+1;
    } else if (*curr == '[') {
      // Array
      curr++;
      skip();
      setArray();
      while (*curr != ']') {
        Ref temp = arena.alloc();
        arr->push_back(temp);
        curr = temp->parse(curr);
        skip();
        if (*curr == ']') break;
        assert(*curr == ',');
        curr++;
        skip();
      }
      curr++;
    } else if (*curr == 'n') {
      // Null
      assert(strncmp(curr, "null", 4) == 0);
      setNull();
      curr += 4;
    } else if (*curr == 't') {
      // Bool true
      assert(strncmp(curr, "true", 4) == 0);
      setBool(true);
      curr += 4;
    } else if (*curr == 'f') {
      // Bool false
      assert(strncmp(curr, "false", 5) == 0);
      setBool(false);
      curr += 5;
    } else {
      // Number
      char *after;
      setNumber(strtod(curr, &after));
      curr = after;
    }
    return curr;
  }

  void stringify(std::ostream &os, bool pretty=false) {
    static int indent = 0;
    #define indentify() { for (int i = 0; i < indent; i++) os << "  "; }
    switch (type) {
      case String:
        os << '"' << str.str << '"';
        break;
      case Number:
        os << std::setprecision(17) << num; // doubles can have 17 digits of precision
        break;
      case Array:
        os << '[';
        if (pretty) {
          os << std::endl;
          indent++;
        }
        for (unsigned i = 0; i < arr->size(); i++) {
          if (i > 0) {
            os << ", ";
            if (pretty) os << std::endl;
          }
          indentify();
          (*arr)[i]->stringify(os, pretty);
        }
        if (pretty) {
          os << std::endl;
          indent--;
        }
        indentify();
        os << ']';
        break;
      case Null:
        os << "null";
        break;
      case Bool:
        os << (boo ? "true" : "false");
        break;
    }
  }

  // String operations

  // Number operations

  // Array operations

  unsigned size() {
    assert(isArray());
    return arr->size();
  }

  void setSize(unsigned size) {
    assert(isArray());
    unsigned old = arr->size();
    if (old != size) arr->resize(size);
    if (old < size) {
      for (unsigned i = old; i < size; i++) {
        (*arr)[i] = arena.alloc();
      }
    }
  }

  Ref& operator[](unsigned x) { // tolerant, returns Null on out of bounds access. makes it convenient to check e.g. [3] on an if node
    static Ref null = arena.alloc(); // TODO: freeze this
    assert(isArray());
    if (x >= arr->size()) return null;
    return (*arr)[x];
  }

  Value& push_back(Ref r) {
    assert(isArray());
    arr->push_back(r);
    return *this;
  }
  Ref pop_back() {
    assert(isArray());
    Ref ret = arr->back();
    arr->pop_back();
    return ret;
  }

  void splice(int x, int num) {
    assert(isArray());
    arr->erase(arr->begin() + x, arr->begin() + x + num);
  }

  void insert(int x, int num) {
    arr->insert(arr->begin() + x, num, Ref());
  }
  void insert(int x, Ref node) {
    arr->insert(arr->begin() + x, 1, node);
  }

  int indexOf(Ref other) {
    assert(isArray());
    for (unsigned i = 0; i < arr->size(); i++) {
      if (other == (*arr)[i]) return i;
    }
    return -1;
  }

  /*
  void forEach(std::function<void (Ref)> func) {
    for (unsigned i = 0; i < arr->size(); i++) {
      func((*arr)[i]);
    }
  }
  */

  // Null operations

  // Bool operations
};

// Ref methods

Ref& Ref::operator[](unsigned x) {
  return (*get())[x];
}

bool Ref::operator==(const char *str) {
  return get()->isString() && !strcmp(get()->str.str, str);
}

bool Ref::operator!=(const char *str) {
  return get()->isString() ? strcmp(get()->str.str, str) : true;
}

bool Ref::operator==(const IString &str) {
  return get()->isString() && get()->str == str;
}

bool Ref::operator!=(const IString &str) {
  return get()->isString() && get()->str != str;
}

bool Ref::operator==(Ref other) {
  return **this == *other;
}

bool Ref::operator!() {
  return !get() || get()->isNull();
}

// Arena methods

Ref Arena::alloc() {
  if (chunks.size() == 0 || index == CHUNK_SIZE) {
    chunks.push_back(new Value[CHUNK_SIZE]);
    index = 0;
  }
  return &chunks.back()[index++];
}

// dump

void dump(const char *str, Ref node, bool pretty) {
  std::cerr << str << ": ";
  node->stringify(std::cerr, pretty);
  std::cerr << std::endl;
}

