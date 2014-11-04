/*
 * Extremely minimal JSON, just enough to manipulate an AST in the format the JS optimizer wants: arrays, strings, numbers, bools (no objects, no non-ascii, etc.).
 * Optimized for fast parsing and also manipulation of the AST.
 * Uses shared_ptr for simplicity, basically everywhere. TODO: measure impact
 */

#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <memory>
#include <vector>
#include <ostream>

struct Value;

// Reference to a value. Simple shared_ptr, plus [] operator for convenience - we work on lots of arrays
class Ref : public std::shared_ptr<Value> {
public:
  Ref() {
    reset();
  }
  Ref(Value *v) {
    reset(v);
  }

  Ref& operator[](unsigned x);

  // special convenience for comparison to string, which is by value
  bool operator==(const char *str);
  bool operator!=(const char *str);
};

typedef std::vector<Ref> ArrayStorage;

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

  union { // TODO: optimize
    std::string *str;
    double num;
    ArrayStorage *arr;
    bool boo;
  };

  // constructors all copy their input
  Value() : type(Null), num(0) {}
  explicit Value(const char *s) : type(Null) {
    set(s);
  }
  explicit Value(double n) : type(Null) {
    set(n);
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
    if (type == String) delete str;
    else if (type == Array) delete arr;
    type = Null;
    num = 0;
  }

  Value& set(const char *s) {
    free();
    type = String;
    str = new std::string(s);
    return *this;
  }
  Value& set(double n) {
    free();
    type = Null;
    num = n;
    return *this;
  }
  Value& set(ArrayStorage &a) {
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

  std::string& getString() {
    assert(isString());
    return *str;
  }
  const char* getCString() {
    assert(isString());
    return str->c_str();
  }
  double& getNumber() {
    assert(isNumber());
    return num;
  }
  bool& getBool() {
    assert(isBool());
    return boo;
  }

  Value& operator=(const Value& other) {
    free();
    switch (other.type) {
      case String:
        set(other.str->c_str());
        break;
      case Number:
        set(other.num);
        break;
      case Array:
        set(*other.arr);
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
        return *str == *(other.str);
      case Number:
        return num == other.num;
      case Array:
        if (arr->size() != other.arr->size()) return false;
        for (unsigned i = 0; i < arr->size(); i++) {
          if ((*arr)[i] != (*other.arr)[i]) return false;
        }
        break;
      case Null:
        break;
      case Bool:
        return boo == other.boo;
    }
    return true;
  }

  char* parse(char* curr) {
    #define skip() { while (*curr && isspace(*curr)) curr++; }
    skip();
    if (*curr == '"') {
      // String
      curr++;
      char *close = strchr(curr, '"');
      assert(close);
      *close = 0;
      set(curr);
      *close = '"';
      curr = close+1;
    } else if (*curr == '[') {
      // Array
      curr++;
      skip();
      setArray();
      while (*curr != ']') {
        Ref temp(new Value);
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
      num = strtod(curr, &after);
      curr = after;
    }
    return curr;
  }

  void stringify(std::ostream &os) {
    switch (type) {
      case String:
        os << '"' << *str << '"';
        break;
      case Number:
        os << num;
        break;
      case Array:
        os << '[';
        for (unsigned i = 0; i < arr->size(); i++) {
          if (i > 0) os << ", ";
          (*arr)[i]->stringify(os);
        }
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

  Ref& operator[](unsigned x) {
    assert(isArray());
    return (*arr)[x];
  }

  void push_back(Ref r) {
    assert(isArray());
    arr->push_back(r);
  }

  // Null operations

  // Bool operations
};

// Convenience class to construct arrays
struct ArrayValue : public Value {
  ArrayValue() {
    setArray();
  }
};

// Ref methods

Ref& Ref::operator[](unsigned x) {
  return (*get())[x];
}

bool Ref::operator==(const char *str) {
  return get()->isString() && get()->getString() == str;
}

bool Ref::operator!=(const char *str) {
  return get()->isString() ? get()->getString() != str : true;
}

