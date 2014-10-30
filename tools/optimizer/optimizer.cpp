#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>

#include <string>
#include <functional>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

//==================
// Globals
//==================

Document doc;

//==================
// Infrastructure
//==================

// Traverses the children of a node.
// visit() receives a reference, so it can modify the value being visited directly.
// TODO: stoppable version of this
void traverseChildren(Value &node, std::function<void (Value&)> visit) {
  assert(node.IsArray());
  int size = node.Size();
  for (int i = 0; i < size; i++) {
    Value &subnode = node[i];
    if (subnode.IsArray()) {
      visit(subnode);
    }
  }
}

Value makeName(const char *str) {
  Value ret;
  ret.SetArray();
  ret.PushBack(Value().SetString("name", 4), doc.GetAllocator());
  ret.PushBack(Value().SetString(str, strlen(str)), doc.GetAllocator());
  return ret;
}

//==================
// Params
//==================

bool asm_ = false;

//=====================
// Optimization passes
//=====================

void optimizeFrounds(Value &ast) {
  // collapse fround(fround(..)), which can happen due to elimination
  // also emit f0 instead of fround(0) (except in returns)
  bool inReturn = false;
  std::function<void (Value&)> fix = [&](Value& node) {
    bool ret = node[0] == "return";
    if (ret) inReturn = true;
    traverseChildren(node, fix);
    if (ret) inReturn = false;
    if (node[0] == "call" && node[1][0] == "name" && node[1][1] == "Math_fround") {
      Value& arg = node[2][0];
      if (arg[0] == "num") {
        if (!inReturn && arg[1] == 0) node = makeName("f0");
      } else if (arg[0] == "call" && arg[1][0] == "name" && arg[1][1] == "Math_fround") {
        node = arg;
      }
    }
  };
  traverseChildren(ast, fix);
}

//==================
// Main
//==================

int main(int argc, char **argv) {
  // Read input file
  char *input = argv[1];
  struct stat st;
  int result = stat(input, &st);
  assert(result == 0);
  int size = st.st_size;
  char *json = new char[size+1];
  FILE *f = fopen(input, "rb");
  fread(json, 1, size, f);
  fclose(f);
  json[size] = 0;

  // Parse JSON source into a Document
  doc.Parse(json);
  delete[] json;

  // Run passes on the Document
  for (int i = 2; i < argc; i++) {
    std::string str(argv[i]);
    if (str == "asm") asm_ = true;
    else if (str == "optimizeFrounds") optimizeFrounds(doc);
  }

  // Emit JSON of modified Document
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);
  puts(buffer.GetString());

  return 0;
}

