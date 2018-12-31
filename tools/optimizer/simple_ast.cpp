
#include "simple_ast.h"

namespace cashew {

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
  return get()->isString() ? !!strcmp(get()->str.str, str) : true;
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

ArrayStorage* Arena::allocArray() {
  if (arr_chunks.size() == 0 || arr_index == CHUNK_SIZE) {
    arr_chunks.push_back(new ArrayStorage[CHUNK_SIZE]);
    arr_index = 0;
  }
  return &arr_chunks.back()[arr_index++];
}

// dump

void dump(const char *str, Ref node, bool pretty) {
  std::cerr << str << ": ";
  if (!!node) node->stringify(std::cerr, pretty);
  else std::cerr << "(nullptr)";
  std::cerr << std::endl;
}

// AST traversals

// Traversals

struct TraverseInfo {
  TraverseInfo() {}
  TraverseInfo(Ref node, ArrayStorage* arr) : node(node), arr(arr), index(0) {}
  Ref node;
  ArrayStorage* arr;
  int index;
};

template <class T, int init>
struct StackedStack { // a stack, on the stack
  T stackStorage[init];
  T* storage;
  int used, available; // used amount, available amount
  bool alloced;

  StackedStack() : used(0), available(init), alloced(false) {
    storage = stackStorage;
  }
  ~StackedStack() {
    if (alloced) free(storage);
  }

  int size() { return used; }

  void push_back(const T& t) {
    assert(used <= available);
    if (used == available) {
      available *= 2;
      if (!alloced) {
        T* old = storage;
        storage = (T*)malloc(sizeof(T)*available);
        memcpy(storage, old, sizeof(T)*used);
        alloced = true;
      } else {
        T *newStorage = (T*)realloc(storage, sizeof(T)*available);
        assert(newStorage);
        storage = newStorage;
      }
    }
    assert(used < available);
    assert(storage);
    storage[used++] = t;
  }

  T& back() {
    assert(used > 0);
    return storage[used-1];
  }

  void pop_back() {
    assert(used > 0);
    used--;
  }
};

#define visitable(node) (node->isArray() && node->size() > 0)

#define TRAV_STACK 40

// Traverse, calling visit before the children
void traversePre(Ref node, std::function<void (Ref)> visit) {
  if (!visitable(node)) return;
  visit(node);
  StackedStack<TraverseInfo, TRAV_STACK> stack;
  int index = 0;
  ArrayStorage* arr = &node->getArray();
  int arrsize = (int)arr->size();
  Ref* arrdata = arr->data();
  stack.push_back(TraverseInfo(node, arr));
  while (1) {
    if (index < arrsize) {
      Ref sub = *(arrdata+index);
      index++;
      if (visitable(sub)) {
        stack.back().index = index;
        index = 0;
        visit(sub);
        arr = &sub->getArray();
        arrsize = (int)arr->size();
        arrdata = arr->data();
        stack.push_back(TraverseInfo(sub, arr));
      }
    } else {
      stack.pop_back();
      if (stack.size() == 0) break;
      TraverseInfo& back = stack.back();
      index = back.index;
      arr = back.arr;
      arrsize = (int)arr->size();
      arrdata = arr->data();
    }
  }
}

// Traverse, calling visitPre before the children and visitPost after
void traversePrePost(Ref node, std::function<void (Ref)> visitPre, std::function<void (Ref)> visitPost) {
  if (!visitable(node)) return;
  visitPre(node);
  StackedStack<TraverseInfo, TRAV_STACK> stack;
  int index = 0;
  ArrayStorage* arr = &node->getArray();
  int arrsize = (int)arr->size();
  Ref* arrdata = arr->data();
  stack.push_back(TraverseInfo(node, arr));
  while (1) {
    if (index < arrsize) {
      Ref sub = *(arrdata+index);
      index++;
      if (visitable(sub)) {
        stack.back().index = index;
        index = 0;
        visitPre(sub);
        arr = &sub->getArray();
        arrsize = (int)arr->size();
        arrdata = arr->data();
        stack.push_back(TraverseInfo(sub, arr));
      }
    } else {
      visitPost(stack.back().node);
      stack.pop_back();
      if (stack.size() == 0) break;
      TraverseInfo& back = stack.back();
      index = back.index;
      arr = back.arr;
      arrsize = (int)arr->size();
      arrdata = arr->data();
    }
  }
}

// Traverse, calling visitPre before the children and visitPost after. If pre returns false, do not traverse children
void traversePrePostConditional(Ref node, std::function<bool (Ref)> visitPre, std::function<void (Ref)> visitPost) {
  if (!visitable(node)) return;
  if (!visitPre(node)) return;
  StackedStack<TraverseInfo, TRAV_STACK> stack;
  int index = 0;
  ArrayStorage* arr = &node->getArray();
  int arrsize = (int)arr->size();
  Ref* arrdata = arr->data();
  stack.push_back(TraverseInfo(node, arr));
  while (1) {
    if (index < arrsize) {
      Ref sub = *(arrdata+index);
      index++;
      if (visitable(sub)) {
        if (visitPre(sub)) {
          stack.back().index = index;
          index = 0;
          arr = &sub->getArray();
          arrsize = (int)arr->size();
          arrdata = arr->data();
          stack.push_back(TraverseInfo(sub, arr));
        }
      }
    } else {
      visitPost(stack.back().node);
      stack.pop_back();
      if (stack.size() == 0) break;
      TraverseInfo& back = stack.back();
      index = back.index;
      arr = back.arr;
      arrsize = (int)arr->size();
      arrdata = arr->data();
    }
  }
}

// Traverses all the top-level functions in the document
void traverseFunctions(Ref ast, std::function<void (Ref)> visit) {
  if (!ast || ast->size() == 0) return;
  if (ast[0] == TOPLEVEL) {
    Ref stats = ast[1];
    for (size_t i = 0; i < stats->size(); i++) {
      Ref curr = stats[i];
      if (curr[0] == DEFUN) visit(curr);
    }
  } else if (ast[0] == DEFUN) {
    visit(ast);
  }
}

// ValueBuilder

IStringSet ValueBuilder::statable("assign call binary unary-prefix if name num conditional dot new sub seq string object array");

} // namespace cashew

