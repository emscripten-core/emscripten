
#include "simple_ast.h"

// Ref methods

Ref& Ref::operator[](unsigned x) {
  return (*get())[x];
}

Ref& Ref::operator[](IString x) {
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

// Arena

Arena arena;

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
  if (!!node) node->stringify(std::cerr, pretty);
  else std::cerr << "(nullptr)";
  std::cerr << std::endl;
}

