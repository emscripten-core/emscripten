
#ifndef __simple_ast_h__
#define __simple_ast_h__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <unordered_map>

#include "parser.h"

#include "snprintf.h"

#define err(str) fprintf(stderr, str "\n");
#define errv(str, ...) fprintf(stderr, str "\n", __VA_ARGS__);
#define printErr err

namespace cashew {

struct Ref;
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
  Ref& operator[](IString x);

  // special conveniences
  bool operator==(const char *str); // comparison to string, which is by value
  bool operator!=(const char *str);
  bool operator==(const IString &str);
  bool operator!=(const IString &str);
  bool operator==(double d) { abort(); return false; } // prevent Ref == number, which is potentially ambiguous; use ->getNumber() == number
  bool operator==(Ref other);
  bool operator!(); // check if null, in effect
};

// Arena allocation, free it all on process exit

typedef std::vector<Ref> ArrayStorage;

struct Arena {
  #define CHUNK_SIZE 1000
  std::vector<Value*> chunks;
  int index; // in last chunk

  std::vector<ArrayStorage*> arr_chunks;
  int arr_index;

  Arena() : index(0), arr_index(0) {}

  Ref alloc();
  ArrayStorage* allocArray();
};

extern Arena arena;

// Main value type
struct Value {
  enum Type {
    String = 0,
    Number = 1,
    Array = 2,
    Null = 3,
    Bool = 4,
    Object = 5
  };

  Type type;

  typedef std::unordered_map<IString, Ref> ObjectStorage;

#ifdef _MSC_VER // MSVC does not allow unrestricted unions: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2544.pdf
  IString str;
#endif
  union { // TODO: optimize
#ifndef _MSC_VER
    IString str;
#endif
    double num;
    ArrayStorage *arr;
    bool boo;
    ObjectStorage *obj;
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
    if (type == Array) { arr->clear(); arr->shrink_to_fit(); }
    else if (type == Object) delete obj;
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
    arr = arena.allocArray();
    *arr = a;
    return *this;
  }
  Value& setArray(int size_hint=0) {
    free();
    type = Array;
    arr = arena.allocArray();
    arr->reserve(size_hint);
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
  Value& setObject() {
    free();
    type = Object;
    obj = new ObjectStorage();
    return *this;
  }

  bool isString() { return type == String; }
  bool isNumber() { return type == Number; }
  bool isArray()  { return type == Array; }
  bool isNull()   { return type == Null; }
  bool isBool()   { return type == Bool; }
  bool isObject()  { return type == Object; }

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

  int getInteger() { // convenience function to get a known integer
    assert(fmod(getNumber(), 1) == 0);
    int ret = int(getNumber());
    assert(double(ret) == getNumber()); // no loss in conversion
    return ret;
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
      case Object:
        abort(); // TODO
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
      case Object:
        return this == &other; // if you want a deep compare, use deepCompare
    }
    return true;
  }

  bool deepCompare(Ref ref) {
    Value& other = *ref;
    if (*this == other) return true; // either same pointer, or identical value type (string, number, null or bool)
    if (type != other.type) return false;
    if (type == Array) {
      if (arr->size() != other.arr->size()) return false;
      for (unsigned i = 0; i < arr->size(); i++) {
        if (!(*arr)[i]->deepCompare((*other.arr)[i])) return false;
      }
      return true;
    } else if (type == Object) {
      if (obj->size() != other.obj->size()) return false;
      for (auto i : *obj) {
        if (other.obj->count(i.first) == 0) return false;
        if (i.second->deepCompare((*other.obj)[i.first])) return false;
      }
      return true;
    }
    return false;
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
    } else if (*curr == '{') {
      // Object
      curr++;
      skip();
      setObject();
      while (*curr != '}') {
        assert(*curr == '"');
        curr++;
        char *close = strchr(curr, '"');
        assert(close);
        *close = 0; // end this string, and reuse it straight from the input
        IString key(curr);
        curr = close+1;
        skip();
        assert(*curr == ':');
        curr++;
        skip();
        Ref value = arena.alloc();
        curr = value->parse(curr);
        (*obj)[key] = value;
        skip();
        if (*curr == '}') break;
        assert(*curr == ',');
        curr++;
        skip();
      }
      curr++;
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
        if (arr->size() == 0) {
          os << "[]";
          break;
        }
        os << '[';
        if (pretty) {
          os << std::endl;
          indent++;
        }
        for (unsigned i = 0; i < arr->size(); i++) {
          if (i > 0) {
            if (pretty) os << "," << std::endl;
            else os << ", ";
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
      case Object:
        os << '{';
        if (pretty) {
          os << std::endl;
          indent++;
        }
        bool first = true;
        for (auto i : *obj) {
          if (first) {
            first = false;
          } else {
            os << ", ";
            if (pretty) os << std::endl;
          }
          indentify();
          os << '"' << i.first.c_str() << "\": ";
          i.second->stringify(os, pretty);
        }
        if (pretty) {
          os << std::endl;
          indent--;
        }
        indentify();
        os << '}';
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

  Ref& operator[](unsigned x) {
    assert(isArray());
    return arr->at(x);
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

  Ref back() {
    assert(isArray());
    if (arr->size() == 0) return nullptr;
    return arr->back();
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

  Ref map(std::function<Ref (Ref node)> func) {
    assert(isArray());
    Ref ret = arena.alloc();
    ret->setArray();
    for (unsigned i = 0; i < arr->size(); i++) {
      ret->push_back(func((*arr)[i]));
    }
    return ret;
  }

  Ref filter(std::function<bool (Ref node)> func) {
    assert(isArray());
    Ref ret = arena.alloc();
    ret->setArray();
    for (unsigned i = 0; i < arr->size(); i++) {
      Ref curr = (*arr)[i];
      if (func(curr)) ret->push_back(curr);
    }
    return ret;
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

  // Object operations

  Ref& operator[](IString x) {
    assert(isObject());
    return (*obj)[x];
  }

  bool has(IString x) {
    assert(isObject());
    return obj->count(x) > 0;
  }
};

// AST traversals

// Traverse, calling visit before the children
void traversePre(Ref node, std::function<void (Ref)> visit);

// Traverse, calling visitPre before the children and visitPost after
void traversePrePost(Ref node, std::function<void (Ref)> visitPre, std::function<void (Ref)> visitPost);

// Traverse, calling visitPre before the children and visitPost after. If pre returns false, do not traverse children
void traversePrePostConditional(Ref node, std::function<bool (Ref)> visitPre, std::function<void (Ref)> visitPost);

// Traverses all the top-level functions in the document
void traverseFunctions(Ref ast, std::function<void (Ref)> visit);

// JS printer

struct JSPrinter {
  bool pretty, finalize;

  char *buffer;
  int size, used;

  int indent;
  bool possibleSpace; // add a space to separate identifiers

  Ref ast;

  JSPrinter(bool pretty_, bool finalize_, Ref ast_) : pretty(pretty_), finalize(finalize_), buffer(0), size(0), used(0), indent(0), possibleSpace(false), ast(ast_) {}

  void printAst() {
    print(ast);
    buffer[used] = 0;
  }

  // Utils

  void ensure(int safety=100) {
    if (size < used + safety) {
      size = std::max(1024, size*2) + safety;
      if (!buffer) {
        buffer = (char*)malloc(size);
        if (!buffer) {
          printf("Out of memory allocating %d bytes for output buffer!", size);
          abort();
        }
      } else {
        char *buf = (char*)realloc(buffer, size);
        if (!buf) {
          free(buffer);
          printf("Out of memory allocating %d bytes for output buffer!", size);
          abort();
        }
        buffer = buf;
      }
    }
  }

  void emit(char c) {
    maybeSpace(c);
    if (!pretty && c == '}' && buffer[used-1] == ';') used--; // optimize ;} into }, the ; is not separating anything
    ensure(1);
    buffer[used++] = c;
  }

  void emit(const char *s) {
    maybeSpace(*s);
    int len = strlen(s);
    ensure(len+1);
    strcpy(buffer + used, s);
    used += len;
  }

  void newline() {
    if (!pretty) return;
    emit('\n');
    for (int i = 0; i < indent; i++) emit(' ');
  }

  void space() {
    if (pretty) emit(' ');
  }

  void safeSpace() {
    if (pretty) emit(' ');
    else possibleSpace = true;
  }

  void maybeSpace(char s) {
    if (possibleSpace) {
      possibleSpace = false;
      if (isIdentPart(s)) emit(' ');
    }
  }

  void print(Ref node) {
    ensure();
    IString type = node[0]->getIString();
    //fprintf(stderr, "printing %s\n", type.str);
    switch (type.str[0]) {
      case 'a': {
        if (type == ASSIGN) printAssign(node);
        else if (type == ARRAY) printArray(node);
        else abort();
        break;
      }
      case 'b': {
        if (type == BINARY) printBinary(node);
        else if (type == BLOCK) printBlock(node);
        else if (type == BREAK) printBreak(node);
        else abort();
        break;
      }
      case 'c': {
        if (type == CALL) printCall(node);
        else if (type == CONDITIONAL) printConditional(node);
        else if (type == CONTINUE) printContinue(node);
        else abort();
        break;
      }
      case 'd': {
        if (type == DEFUN) printDefun(node);
        else if (type == DO) printDo(node);
        else if (type == DOT) printDot(node);
        else abort();
        break;
      }
      case 'i': {
        if (type == IF) printIf(node);
        else abort();
        break;
      }
      case 'l': {
        if (type == LABEL) printLabel(node);
        else abort();
        break;
      }
      case 'n': {
        if (type == NAME) printName(node);
        else if (type == NUM) printNum(node);
        else if (type == NEW) printNew(node);
        else abort();
        break;
      }
      case 'o': {
        if (type == OBJECT) printObject(node);
        break;
      }
      case 'r': {
        if (type == RETURN) printReturn(node);
        else abort();
        break;
      }
      case 's': {
        if (type == STAT) printStat(node);
        else if (type == SUB) printSub(node);
        else if (type == SEQ) printSeq(node);
        else if (type == SWITCH) printSwitch(node);
        else if (type == STRING) printString(node);
        else abort();
        break;
      }
      case 't': {
        if (type == TOPLEVEL) printToplevel(node);
        else abort();
        break;
      }
      case 'u': {
        if (type == UNARY_PREFIX) printUnaryPrefix(node);
        else abort();
        break;
      }
      case 'v': {
        if (type == VAR) printVar(node);
        else abort();
        break;
      }
      case 'w': {
        if (type == WHILE) printWhile(node);
        else abort();
        break;
      }
      default: {
        printf("cannot yet print %s\n", type.str);
        abort();
      }
    }
  }

  // print a node, and if nothing is emitted, emit something instead
  void print(Ref node, const char *otherwise) {
    int last = used;
    print(node);
    if (used == last) emit(otherwise);
  }

  void printStats(Ref stats) {
    bool first = true;
    for (size_t i = 0; i < stats->size(); i++) {
      Ref curr = stats[i];
      if (!isNothing(curr)) {
        if (first) first = false;
        else newline();
        print(stats[i]);
      }
    }
  }

  void printToplevel(Ref node) {
    if (node[1]->size() > 0) {
      printStats(node[1]);
    }
  }

  void printBlock(Ref node) {
    if (node->size() == 1 || node[1]->size() == 0) {
      emit("{}");
      return;
    }
    emit('{');
    indent++;
    newline();
    printStats(node[1]);
    indent--;
    newline();
    emit('}');
  }

  void printDefun(Ref node) {
    emit("function ");
    emit(node[1]->getCString());
    emit('(');
    Ref args = node[2];
    for (size_t i = 0; i < args->size(); i++) {
      if (i > 0) (pretty ? emit(", ") : emit(','));
      emit(args[i]->getCString());
    }
    emit(')');
    space();
    if (node->size() == 3 || node[3]->size() == 0) {
      emit("{}");
      return;
    }
    emit('{');
    indent++;
    newline();
    printStats(node[3]);
    indent--;
    newline();
    emit('}');
    newline();
  }

  bool isNothing(Ref node) {
    return (node[0] == TOPLEVEL && node[1]->size() == 0) || (node[0] == STAT && isNothing(node[1]));
  }

  void printStat(Ref node) {
    if (!isNothing(node[1])) {
      print(node[1]);
      if (buffer[used-1] != ';') emit(';');
    }
  }

  void printAssign(Ref node) {
    printChild(node[2], node, -1);
    space();
    emit('=');
    space();
    printChild(node[3], node, 1);
  }

  void printName(Ref node) {
    emit(node[1]->getCString());
  }

  static char* numToString(double d, bool finalize=true) {
    bool neg = d < 0;
    if (neg) d = -d;
    // try to emit the fewest necessary characters
    bool integer = fmod(d, 1) == 0;
    #define BUFFERSIZE 1000
    static char full_storage_f[BUFFERSIZE], full_storage_e[BUFFERSIZE]; // f is normal, e is scientific for float, x for integer
    static char *storage_f = full_storage_f + 1, *storage_e = full_storage_e + 1; // full has one more char, for a possible '-'
    double err_f, err_e;
    for (int e = 0; e <= 1; e++) {
      char *buffer = e ? storage_e : storage_f;
      double temp;
      if (!integer) {
        static char format[6];
        for (int i = 0; i <= 18; i++) {
          format[0] = '%';
          format[1] = '.';
          if (i < 10) {
            format[2] = '0' + i;
            format[3] = e ? 'e' : 'f';
            format[4] = 0;
          } else {
            format[2] = '1';
            format[3] = '0' + (i - 10);
            format[4] = e ? 'e' : 'f';
            format[5] = 0;
          }
          snprintf(buffer, BUFFERSIZE-1, format, d);
          sscanf(buffer, "%lf", &temp);
          //errv("%.18f, %.18e   =>   %s   =>   %.18f, %.18e   (%d), ", d, d, buffer, temp, temp, temp == d);
          if (temp == d) break;
        }
      } else {
        // integer
        assert(d >= 0);
        unsigned long long uu = (unsigned long long)d;
        if (uu == d) {
          bool asHex = e && !finalize;
          snprintf(buffer, BUFFERSIZE-1, asHex ? "0x%llx" : "%llu", uu);
          if (asHex) {
            unsigned long long tempULL;
            sscanf(buffer, "%llx", &tempULL);
            temp = (double)tempULL;
          } else {
            sscanf(buffer, "%lf", &temp);
          }
        } else {
          // too large for a machine integer, just use floats
          snprintf(buffer, BUFFERSIZE-1, e ? "%e" : "%.0f", d); // even on integers, e with a dot is useful, e.g. 1.2e+200
          sscanf(buffer, "%lf", &temp);
        }
        //errv("%.18f, %.18e   =>   %s   =>   %.18f, %.18e, %llu   (%d)\n", d, d, buffer, temp, temp, uu, temp == d);
      }
      (e ? err_e : err_f) = fabs(temp - d);
      //errv("current attempt: %.18f  =>  %s", d, buffer);
      //assert(temp == d);
      char *dot = strchr(buffer, '.');
      if (dot) {
        // remove trailing zeros
        char *end = dot+1;
        while (*end >= '0' && *end <= '9') end++;
        end--;
        while (*end == '0') {
          char *copy = end;
          do {
            copy[0] = copy[1];
          } while (*copy++ != 0);
          end--;
        }
        //errv("%.18f  =>   %s", d, buffer);
        // remove preceding zeros
        while (*buffer == '0') {
          char *copy = buffer;
          do {
            copy[0] = copy[1];
          } while (*copy++ != 0);
        }
        //errv("%.18f ===>  %s", d, buffer);
      } else if (!integer || !e) {
        // no dot. try to change 12345000 => 12345e3
        char *end = strchr(buffer, 0);
        end--;
        char *test = end;
        // remove zeros, and also doubles can use at most 24 digits, we can truncate any extras even if not zero
        while ((*test == '0' || test - buffer > 24) && test > buffer) test--;
        int num = end - test;
        if (num >= 3) {
          test++;
          test[0] = 'e';
          if (num < 10) {
            test[1] = '0' + num;
            test[2] = 0;
          } else if (num < 100) {
            test[1] = '0' + (num / 10);
            test[2] = '0' + (num % 10);
            test[3] = 0;
          } else {
            assert(num < 1000);
            test[1] = '0' + (num / 100);
            test[2] = '0' + (num % 100) / 10;
            test[3] = '0' + (num % 10);
            test[4] = 0;
          }
        }
      }
      //errv("..current attempt: %.18f  =>  %s", d, buffer);
    }
    //fprintf(stderr, "options:\n%s\n%s\n (first? %d)\n", storage_e, storage_f, strlen(storage_e) < strlen(storage_f));
    char *ret;
    if (err_e == err_f) {
      ret = strlen(storage_e) < strlen(storage_f) ? storage_e : storage_f;
    } else {
      ret = err_e < err_f ? storage_e : storage_f;
    }
    if (neg) {
      ret--; // safe to go back one, there is one more char in full_*
      *ret = '-';
    }
    return ret;
  }

  void printNum(Ref node) {
    emit(numToString(node[1]->getNumber(), finalize));
  }

  void printString(Ref node) {
    emit('"');
    emit(node[1]->getCString());
    emit('"');
  }

  // Parens optimizing

  bool capturesOperators(Ref node) {
    Ref type = node[0];
    return type == CALL || type == ARRAY || type == OBJECT || type == SEQ;
  }

  int getPrecedence(Ref node, bool parent) {
    Ref type = node[0];
    if (type == BINARY || type == UNARY_PREFIX) {
      return OperatorClass::getPrecedence(type == BINARY ? OperatorClass::Binary : OperatorClass::Prefix, node[1]->getIString());
    } else if (type == SEQ) {
      return OperatorClass::getPrecedence(OperatorClass::Binary, COMMA);
    } else if (type == CALL) {
      return parent ? OperatorClass::getPrecedence(OperatorClass::Binary, COMMA) : -1; // call arguments are split by commas, but call itself is safe
    } else if (type == ASSIGN) {
      return OperatorClass::getPrecedence(OperatorClass::Binary, SET);
    } else if (type == CONDITIONAL) {
      return OperatorClass::getPrecedence(OperatorClass::Tertiary, QUESTION);
    }
    // otherwise, this is something that fixes precedence explicitly, and we can ignore
    return -1; // XXX
  }

  // check whether we need parens for the child, when rendered in the parent
  // @param childPosition -1 means it is printed to the left of parent, 0 means "anywhere", 1 means right
  bool needParens(Ref parent, Ref child, int childPosition) {
    int parentPrecedence = getPrecedence(parent, true);
    int childPrecedence = getPrecedence(child, false);

    if (childPrecedence > parentPrecedence) return true;  // child is definitely a danger
    if (childPrecedence < parentPrecedence) return false; //          definitely cool
    // equal precedence, so associativity (rtl/ltr) is what matters
    // (except for some exceptions, where multiple operators can combine into confusion)
    if (parent[0] == UNARY_PREFIX) {
      assert(child[0] == UNARY_PREFIX);
      if ((parent[1] == PLUS || parent[1] == MINUS) && child[1] == parent[1]) {
        // cannot emit ++x when we mean +(+x)
        return true;
      }
    }
    if (childPosition == 0) return true; // child could be anywhere, so always paren
    if (childPrecedence < 0) return false; // both precedences are safe
    // check if child is on the dangerous side
    if (OperatorClass::getRtl(parentPrecedence)) return childPosition < 0;
    else return childPosition > 0;
  }

  void printChild(Ref child, Ref parent, int childPosition=0) {
    bool parens = needParens(parent, child, childPosition);
    if (parens) emit('(');
    print(child);
    if (parens) emit(')');
  }

  void printBinary(Ref node) {
    printChild(node[2], node, -1);
    space();
    emit(node[1]->getCString());
    space();
    printChild(node[3], node, 1);
  }

  void printUnaryPrefix(Ref node) {
    if (finalize && node[1] == PLUS && (node[2][0] == NUM ||
                                       (node[2][0] == UNARY_PREFIX && node[2][1] == MINUS && node[2][2][0] == NUM))) {
      // emit a finalized number
      int last = used;
      print(node[2]);
      ensure(1); // we temporarily append a 0
      char *curr = buffer + last; // ensure might invalidate
      buffer[used] = 0;
      if (strchr(curr, '.')) return; // already a decimal point, all good
      char *e = strchr(curr, 'e');
      if (!e) {
        emit(".0");
        return;
      }
      ensure(3);
      curr = buffer + last; // ensure might invalidate
      char *end = strchr(curr, 0);
      while (end >= e) {
        end[2] = end[0];
        end--;
      }
      e[0] = '.';
      e[1] = '0';
      used += 2;
      return;
    }
    if ((buffer[used-1] == '-' && node[1] == MINUS) ||
        (buffer[used-1] == '+' && node[1] == PLUS)) {
      emit(' '); // cannot join - and - to --, looks like the -- operator
    }
    emit(node[1]->getCString());
    printChild(node[2], node, 1);
  }

  void printConditional(Ref node) {
    printChild(node[1], node, -1);
    space();
    emit('?');
    space();
    printChild(node[2], node, 0);
    space();
    emit(':');
    space();
    printChild(node[3], node, 1);
  }

  void printCall(Ref node) {
    printChild(node[1], node, 0);
    emit('(');
    Ref args = node[2];
    for (size_t i = 0; i < args->size(); i++) {
      if (i > 0) (pretty ? emit(", ") : emit(','));
      printChild(args[i], node, 0);
    }
    emit(')');
  }

  void printSeq(Ref node) {
    printChild(node[1], node, -1);
    emit(',');
    space();
    printChild(node[2], node, 1);
  }

  void printDot(Ref node) {
    print(node[1]);
    emit('.');
    emit(node[2]->getCString());
  }

  void printSwitch(Ref node) {
    emit("switch");
    space();
    emit('(');
    print(node[1]);
    emit(')');
    space();
    emit('{');
    newline();
    Ref cases = node[2];
    for (size_t i = 0; i < cases->size(); i++) {
      Ref c = cases[i];
      if (!c[0]) {
        emit("default:");
      } else {
        emit("case ");
        print(c[0]);
        emit(':');
      }
      if (c[1]->size() > 0) {
        indent++;
        newline();
        int curr = used;
        printStats(c[1]);
        indent--;
        if (curr != used) newline();
        else used--; // avoid the extra indentation we added tentatively
      } else {
        newline();
      }
    }
    emit('}');
  }

  void printSub(Ref node) {
    printChild(node[1], node, -1);
    emit('[');
    print(node[2]);
    emit(']');
  }

  void printVar(Ref node) {
    emit("var ");
    Ref args = node[1];
    for (size_t i = 0; i < args->size(); i++) {
      if (i > 0) (pretty ? emit(", ") : emit(','));
      emit(args[i][0]->getCString());
      if (args[i]->size() > 1) {
        space();
        emit('=');
        space();
        print(args[i][1]);
      }
    }
    emit(';');
  }

  static bool ifHasElse(Ref node) {
    assert(node[0] == IF);
    return node->size() >= 4 && !!node[3];
  }

  void printIf(Ref node) {
    emit("if");
    safeSpace();
    emit('(');
    print(node[1]);
    emit(')');
    space();
    // special case: we need braces to save us from ambiguity, if () { if () } else. otherwise else binds to inner if
    // also need to recurse for                                if () { if () { } else { if () } else
    // (note that this is only a problem if the if body has a single element in it, not a block or such, as then
    // the block would be braced)
    // this analysis is a little conservative - it assumes any child if could be confused with us, which implies
    // all other braces vanished (the worst case for us, we are not saved by other braces).
    bool needBraces = false;
    bool hasElse = ifHasElse(node);
    if (hasElse) {
      Ref child = node[2];
      while (child[0] == IF) {
        if (!ifHasElse(child)) {
          needBraces = true;
          break;
        }
        child = child[3]; // continue into the else
      }
    }
    if (needBraces) {
      emit('{');
      indent++;
      newline();
      print(node[2]);
      indent--;
      newline();
      emit('}');
    } else {
      print(node[2], "{}");
    }
    if (hasElse) {
      space();
      emit("else");
      safeSpace();
      print(node[3], "{}");
    }
  }

  void printDo(Ref node) {
    emit("do");
    safeSpace();
    print(node[2], "{}");
    space();
    emit("while");
    space();
    emit('(');
    print(node[1]);
    emit(')');
    emit(';');
  }

  void printWhile(Ref node) {
    emit("while");
    space();
    emit('(');
    print(node[1]);
    emit(')');
    space();
    print(node[2], "{}");
  }

  void printLabel(Ref node) {
    emit(node[1]->getCString());
    space();
    emit(':');
    space();
    print(node[2]);
  }

  void printReturn(Ref node) {
    emit("return");
    if (!!node[1]) {
      emit(' ');
      print(node[1]);
    }
    emit(';');
  }

  void printBreak(Ref node) {
    emit("break");
    if (!!node[1]) {
      emit(' ');
      emit(node[1]->getCString());
    }
    emit(';');
  }

  void printContinue(Ref node) {
    emit("continue");
    if (!!node[1]) {
      emit(' ');
      emit(node[1]->getCString());
    }
    emit(';');
  }

  void printNew(Ref node) {
    emit("new ");
    print(node[1]);
  }

  void printArray(Ref node) {
    emit('[');
    Ref args = node[1];
    for (size_t i = 0; i < args->size(); i++) {
      if (i > 0) (pretty ? emit(", ") : emit(','));
      print(args[i]);
    }
    emit(']');
  }

  void printObject(Ref node) {
    emit('{');
    indent++;
    newline();
    Ref args = node[1];
    for (size_t i = 0; i < args->size(); i++) {
      if (i > 0) {
        pretty ? emit(", ") : emit(',');
        newline();
      }
      emit('"');
      emit(args[i][0]->getCString());
      emit("\":");
      space();
      print(args[i][1]);
    }
    indent--;
    newline();
    emit('}');
  }
};

// cashew builder

class ValueBuilder {
  static IStringSet statable;

  static Ref makeRawString(const IString& s) {
    return &arena.alloc()->setString(s);
  }

  static Ref makeRawArray(int size_hint=0) {
    return &arena.alloc()->setArray(size_hint);
  }

  static Ref makeNull() {
    return &arena.alloc()->setNull();
  }

public:
  static Ref makeToplevel() {
    return &makeRawArray(2)->push_back(makeRawString(TOPLEVEL))
                            .push_back(makeRawArray());
  }

  static Ref makeString(IString str) {
    return &makeRawArray(2)->push_back(makeRawString(STRING))
                            .push_back(makeRawString(str));
  }

  static Ref makeBlock() {
    return &makeRawArray(2)->push_back(makeRawString(BLOCK))
                            .push_back(makeRawArray());
  }

  static Ref makeName(IString name) {
    return &makeRawArray(2)->push_back(makeRawString(NAME))
                            .push_back(makeRawString(name));
  }

  static void setBlockContent(Ref target, Ref block) {
    if (target[0] == TOPLEVEL) {
      target[1]->setArray(block[1]->getArray());
    } else if (target[0] == DEFUN) {
      target[3]->setArray(block[1]->getArray());
    } else abort();
  }

  static void appendToBlock(Ref block, Ref element) {
    assert(block[0] == BLOCK);
    block[1]->push_back(element);
  }

  static Ref makeCall(Ref target) {
    return &makeRawArray(3)->push_back(makeRawString(CALL))
                            .push_back(target)
                            .push_back(makeRawArray());
  }

  static void appendToCall(Ref call, Ref element) {
    assert(call[0] == CALL);
    call[2]->push_back(element);
  }

  static Ref makeStatement(Ref contents) {
    if (statable.has(contents[0]->getIString())) {
      return &makeRawArray(2)->push_back(makeRawString(STAT))
                              .push_back(contents);
    } else {
      return contents; // only very specific things actually need to be stat'ed
    }
  }

  static Ref makeDouble(double num) {
    return &makeRawArray(2)->push_back(makeRawString(NUM))
                            .push_back(&arena.alloc()->setNumber(num));
  }
  static Ref makeInt(uint32_t num) {
    return makeDouble(double(num));
  }

  static Ref makeBinary(Ref left, IString op, Ref right) {
    if (op == SET) {
      return &makeRawArray(4)->push_back(makeRawString(ASSIGN))
                              .push_back(&arena.alloc()->setBool(true))
                              .push_back(left)
                              .push_back(right);
    } else if (op == COMMA) {
      return &makeRawArray(3)->push_back(makeRawString(SEQ))
                              .push_back(left)
                              .push_back(right);
    } else {
      return &makeRawArray(4)->push_back(makeRawString(BINARY))
                              .push_back(makeRawString(op))
                              .push_back(left)
                              .push_back(right);
    }
  }

  static Ref makePrefix(IString op, Ref right) {
    return &makeRawArray(3)->push_back(makeRawString(UNARY_PREFIX))
                            .push_back(makeRawString(op))
                            .push_back(right);
  }

  static Ref makeFunction(IString name) {
    return &makeRawArray(4)->push_back(makeRawString(DEFUN))
                            .push_back(makeRawString(name))
                            .push_back(makeRawArray())
                            .push_back(makeRawArray());
  }

  static void appendArgumentToFunction(Ref func, IString arg) {
    assert(func[0] == DEFUN);
    func[2]->push_back(makeRawString(arg));
  }

  static Ref makeVar(bool is_const) {
    return &makeRawArray(2)->push_back(makeRawString(VAR))
                            .push_back(makeRawArray());
  }

  static void appendToVar(Ref var, IString name, Ref value) {
    assert(var[0] == VAR);
    Ref array = &makeRawArray(1)->push_back(makeRawString(name));
    if (!!value) array->push_back(value);
    var[1]->push_back(array);
  }

  static Ref makeReturn(Ref value) {
    return &makeRawArray(2)->push_back(makeRawString(RETURN))
                            .push_back(!!value ? value : makeNull());
  }

  static Ref makeIndexing(Ref target, Ref index) {
    return &makeRawArray(3)->push_back(makeRawString(SUB))
                            .push_back(target)
                            .push_back(index);
  }

  static Ref makeIf(Ref condition, Ref ifTrue, Ref ifFalse) {
    return &makeRawArray(4)->push_back(makeRawString(IF))
                            .push_back(condition)
                            .push_back(ifTrue)
                            .push_back(!!ifFalse ? ifFalse : makeNull());
  }

  static Ref makeConditional(Ref condition, Ref ifTrue, Ref ifFalse) {
    return &makeRawArray(4)->push_back(makeRawString(CONDITIONAL))
                            .push_back(condition)
                            .push_back(ifTrue)
                            .push_back(ifFalse);
  }

  static Ref makeDo(Ref body, Ref condition) {
    return &makeRawArray(3)->push_back(makeRawString(DO))
                            .push_back(condition)
                            .push_back(body);
  }

  static Ref makeWhile(Ref condition, Ref body) {
    return &makeRawArray(3)->push_back(makeRawString(WHILE))
                            .push_back(condition)
                            .push_back(body);
  }

  static Ref makeBreak(IString label) {
    return &makeRawArray(2)->push_back(makeRawString(BREAK))
                            .push_back(!!label ? makeRawString(label) : makeNull());
  }

  static Ref makeContinue(IString label) {
    return &makeRawArray(2)->push_back(makeRawString(CONTINUE))
                            .push_back(!!label ? makeRawString(label) : makeNull());
  }

  static Ref makeLabel(IString name, Ref body) {
    return &makeRawArray(3)->push_back(makeRawString(LABEL))
                            .push_back(makeRawString(name))
                            .push_back(body);
  }

  static Ref makeSwitch(Ref input) {
    return &makeRawArray(3)->push_back(makeRawString(SWITCH))
                            .push_back(input)
                            .push_back(makeRawArray());
  }

  static void appendCaseToSwitch(Ref switch_, Ref arg) {
    assert(switch_[0] == SWITCH);
    switch_[2]->push_back(&makeRawArray(2)->push_back(arg)
                                           .push_back(makeRawArray()));
  }

  static void appendDefaultToSwitch(Ref switch_) {
    assert(switch_[0] == SWITCH);
    switch_[2]->push_back(&makeRawArray(2)->push_back(makeNull())
                                           .push_back(makeRawArray()));
  }

  static void appendCodeToSwitch(Ref switch_, Ref code, bool explicitBlock) {
    assert(switch_[0] == SWITCH);
    assert(code[0] == BLOCK);
    if (!explicitBlock) {
      for (size_t i = 0; i < code[1]->size(); i++) {
        switch_[2]->back()->back()->push_back(code[1][i]);
      }
    } else {
      switch_[2]->back()->back()->push_back(code);
    }
  }

  static Ref makeDot(Ref obj, IString key) {
    return &makeRawArray(3)->push_back(makeRawString(DOT))
                            .push_back(obj)
                            .push_back(makeRawString(key));
  }

  static Ref makeDot(Ref obj, Ref key) {
    assert(key[0] == NAME);
    return makeDot(obj, key[1]->getIString());
  }

  static Ref makeNew(Ref call) {
    return &makeRawArray(2)->push_back(makeRawString(NEW))
                            .push_back(call);
  }

  static Ref makeArray() {
    return &makeRawArray(2)->push_back(makeRawString(ARRAY))
                            .push_back(makeRawArray());
  }

  static void appendToArray(Ref array, Ref element) {
    assert(array[0] == ARRAY);
    array[1]->push_back(element);
  }

  static Ref makeObject() {
    return &makeRawArray(2)->push_back(makeRawString(OBJECT))
                            .push_back(makeRawArray());
  }

  static void appendToObject(Ref array, IString key, Ref value) {
    assert(array[0] == OBJECT);
    array[1]->push_back(&makeRawArray(2)->push_back(makeRawString(key))
                                         .push_back(value));
  }
};

// Tolerates 0.0 in the input; does not trust a +() to be there.
class DotZeroValueBuilder : public ValueBuilder {
public:
  static Ref makeDouble(double num) {
    return makePrefix(PLUS, ValueBuilder::makeDouble(num));
  }
};

} // namespace cashew

#endif // __simple_ast_h__

