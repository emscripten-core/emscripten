#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>

#include <string>
#include <map>
#include <unordered_map>
#include <functional>

#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

//==================
// Globals
//==================

Document doc;

class GlobalStringValue : public Value {
public:
  GlobalStringValue(const char *str) : Value(str, strlen(str)) {}
};

GlobalStringValue RETURN("return"),
                  STAT("stat"),
                  CALL("call"),
                  NAME("name"),
                  NUM("num"),
                  SUB("sub"),
                  ASSIGN("assign"),
                  UNARY_PREFIX("unary-prefix"),
                  BINARY("binary"),
                  CONDITIONAL("conditional"),
                  SEQ("seq"),
                  WHILE("while"),
                  IF("if"),
                  BLOCK("block"),
                  LABEL("label"),
                  TOPLEVEL("toplevel"),
                  DEFUN("defun"),
                  BANG("!"),
                  MATH_FROUND("Math_fround");

//==================
// Infrastructure
//==================

void dump(const char *str, Value &node) {
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  node.Accept(writer);
  fprintf(stderr, "%s: %s\n", str, buffer.GetString());
}

int parseInt(const char *str) {
  int ret = *str - '0';
  while (*(++str)) {
    ret *= 10;
    ret += *str - '0';
  }
  return ret;
}

// Traversals

// Traverses the children of a node.
// visit() receives a reference, so it can modify the value being visited directly.
//       N.B. we allow replacing arrays in place, and consider strings immutable!
// TODO: stoppable version of this
void traverseChildren(Value &node, std::function<void (Value&)> visit) {
  //dump("TC", node);
  if (!node.IsArray()) return;
  int size = node.Size();
  //printf("size: %d\n", size);
  for (int i = 0; i < size; i++) {
    //printf("  %d:\n", i);
    Value &subnode = node[i];
    if (subnode.IsArray() and subnode.Size() > 0) {
      //printf("    go\n");
      visit(subnode);
    }
  }
}

// Traverse, calling visit before the children
void traversePre(Value &node, std::function<void (Value&)> visit) {
  visit(node);
  traverseChildren(node, [&visit](Value& node) {
    traversePre(node, visit);
  });
}

// Traverse, calling visitPre before the children and visitPost after
void traversePrePost(Value &node, std::function<void (Value&)> visitPre, std::function<void (Value&)> visitPost) {
  visitPre(node);
  traverseChildren(node, [&visitPre, &visitPost](Value& node) {
    traversePrePost(node, visitPre, visitPost);
  });
  visitPost(node);
}

// Traverses all the top-level functions in the document
void traverseFunctions(Value &ast, std::function<void (Value&)> visit) {
  if (ast[0] == TOPLEVEL) {
    Value& stats = ast[1];
    for (int i = 0; i < stats.Size(); i++) {
      Value& curr = stats[i];
      if (curr[0] == DEFUN) visit(curr);
    }
  } else if (ast[0] == DEFUN) {
    visit(ast);
  }
}

Value& deStat(Value& node) {
  if (node[0] == STAT) return node[1];
  return node;
}

Value& getStatements(Value& node) {
  if (node[0] == DEFUN) {
    return node[3];
  } else if (node[0] == BLOCK) {
    return node[1];
  } else {
    return Value().SetNull();
  }
}

// Constructions

Value makeName(const char *str) {
  Value ret;
  ret.SetArray();
  ret.PushBack(NAME, doc.GetAllocator());
  ret.PushBack(Value().SetString(str, strlen(str)), doc.GetAllocator());
  return ret;
}

Value makeNum(double x) {
  Value ret;
  ret.SetArray();
  ret.PushBack(NUM, doc.GetAllocator());
  ret.PushBack(Value(x), doc.GetAllocator());
  return ret;
}

Value makeBlock() {
  Value ret;
  ret.SetArray();
  ret.PushBack(BLOCK, doc.GetAllocator());
  ret.PushBack(Value().SetArray(), doc.GetAllocator());
  return ret;
}

Value make2(Value& type, Value&& a, Value&& b) {
  Value ret;
  ret.SetArray();
  ret.PushBack(type, doc.GetAllocator());
  ret.PushBack(a, doc.GetAllocator());
  ret.PushBack(b, doc.GetAllocator());
  return ret;
}
#define mmake2(a, b, c) make2(a, std::move(b), std::move(c))

Value make3(Value& type, Value&& a, Value&& b, Value&& c) {
  Value ret;
  ret.SetArray();
  ret.PushBack(type, doc.GetAllocator());
  ret.PushBack(a, doc.GetAllocator());
  ret.PushBack(b, doc.GetAllocator());
  ret.PushBack(c, doc.GetAllocator());
  return ret;
}
#define mmake3(a, b, c, d) make3(a, std::move(b), std::move(c), std::move(d))

// Types

enum AsmType {
  ASM_INT = 0,
  ASM_DOUBLE = 1,
  ASM_FLOAT = 2,
  ASM_FLOAT32X4 = 3,
  ASM_INT32X4 = 4,
  ASM_NONE = 5
};

struct AsmInfo {
  std::map<std::string, int> params; // name => index
  std::map<std::string, AsmType> types; // name => type

  AsmType getType(std::string name) {
    auto ret = types.find(name);
    if (ret != types.end()) return ret->second;
    return ASM_NONE;
  }
};

struct HeapInfo {
  bool valid, unsign, floaty;
  int bits;
  AsmType type;
};

HeapInfo parseHeap(const char *name) {
  HeapInfo ret;
  if (!strncmp(name, "HEAP", 4)) {
    ret.valid = false;
    return ret;
  }
  ret.valid = true;
  ret.unsign = name[4] == 'U';
  ret.floaty = name[4] == 'F';
  ret.bits = parseInt(name + (ret.unsign || ret.floaty ? 5 : 4));
  ret.type = !ret.floaty ? ASM_INT : (ret.bits == 64 ? ASM_DOUBLE : ASM_FLOAT);
  return ret;
}

AsmType detectType(Value& node, AsmInfo *asmInfo=nullptr, bool inVarDef=false) {
  switch (node[0].GetString()[0]) {
    case 'n': {
      if (node[0] == NUM) {
        if (strchr(node[1].GetString(), '.')) return ASM_DOUBLE;
        return ASM_INT;
      } else if (node[0] == NAME) {
        if (asmInfo) {
          AsmType ret = asmInfo->getType(node[1].GetString());
          if (ret != ASM_NONE) return ret;
        }
        if (!inVarDef) {
          if (node[1] == "inf" || node[1] == "nan") return ASM_DOUBLE;
          if (node[1] == "tempRet0") return ASM_INT;
          return ASM_NONE;
        }
        // We are in a variable definition, where Math_fround(0) optimized into a global constant becomes f0 = Math_fround(0)
        assert(0);
        /*
        if (!ASM_FLOAT_ZERO) ASM_FLOAT_ZERO = node[1];
        else assert(ASM_FLOAT_ZERO == node[1]);
        return ASM_FLOAT;
        */
      }
      break;
    }
    case 'u': {
      if (node[0] == UNARY_PREFIX) {
        switch (node[1].GetString()[0]) {
          case '+': return ASM_DOUBLE;
          case '-': return detectType(node[2], asmInfo, inVarDef);
          case '!': case '~': return ASM_INT;
        }
        break;
      }
      break;
    }
    case 'c': {
      if (node[0] == CALL) {
        if (node[1][0] == NAME) {
          Value& name = node[1][1];
          if (name == "Math_fround") return ASM_FLOAT;
          else if (name == "SIMD_float32x4") return ASM_FLOAT32X4;
          else if (name == "SIMD_int32x4") return ASM_INT32X4;
        }
        return ASM_NONE;
      } else if (node[0] == CONDITIONAL) {
        return detectType(node[2], asmInfo, inVarDef);
      }
      break;
    }
    case 'b': {
      if (node[0] == BINARY) {
        switch (node[1].GetString()[0]) {
          case '+': case '-':
          case '*': case '/': case '%': return detectType(node[2], asmInfo, inVarDef);
          case '|': case '&': case '^': case '<': case '>': // handles <<, >>, >>=, <=, >=
          case '=': case '!': { // handles ==, !=
            return ASM_INT;
          }
        }
      }
      break;
    }
    case 's': {
      if (node[0] == SEQ) {
        return detectType(node[2], asmInfo, inVarDef);
      } else if (node[0] == SUB) {
        assert(node[1][0] == NAME);
        HeapInfo info = parseHeap(node[1][1].GetString());
        if (info.valid) return ASM_NONE;
        return info.floaty ? ASM_DOUBLE : ASM_INT; // XXX ASM_FLOAT?
      }
      break;
    }
  }
  dump("horrible", node);
  assert(0);
}

// Transforms

// We often have branchings that are simplified so one end vanishes, and
// we then get
//   if (!(x < 5))
// or such. Simplifying these saves space and time.
Value simplifyNotCompsDirect(Value node) {
  if (node[0] == UNARY_PREFIX && node[1] == BANG) {
    // de-morgan's laws do not work on floats, due to nans >:(
    if (node[2][0] == BINARY && (detectType(node[2][2]) == ASM_INT && detectType(node[2][3]) == ASM_INT)) {
      Value &op = node[2][1];
      switch(op.GetString()[0]) {
        case '<': {
          if (op == "<")  { op.SetString(">="); break; }
          if (op == "<=") { op.SetString(">"); break; }
          assert(0);
        }
        case '>': {
          if (op == ">")  { op.SetString("<="); break; }
          if (op == ">=") { op.SetString("<"); break; }
          assert(0);
        }
        case '=': {
          if (op == "==") { op.SetString("!="); break; }
          assert(0);
        }
        case '!': {
          if (op == "!=") { op.SetString("=="); break; }
          assert(0);
        }
        default: assert(0);
      }
      return mmake3(BINARY, op, node[2][2], node[2][3]);
    } else if (node[2][0] == UNARY_PREFIX && node[2][1] == '!') {
      return std::move(node[2][2]);
    }
  }
  return std::move(node);
}

Value flipCondition(Value& cond) {
  return simplifyNotCompsDirect(mmake2(UNARY_PREFIX, BANG, cond));
}

void splice(Value& node, int index) { // removes an element from the middle of an array
  node.Erase(node.Begin() + index);
}

// Checks

bool commable(Value& node) { // TODO: hashing
  std::string type = node[0].GetString();
  if (type == "assign" || type == "binary" || type == "unary-prefix" || type == "unary-postfix" || type == "name" || type == "num" || type == "call" || type == "seq" || type == "conditional" || type == "sub") return true;
  return false;
}

//==================
// Params
//==================

bool preciseF32 = false;

//=====================
// Optimization passes
//=====================

void simplifyIfs(Value& ast) {
  traverseFunctions(ast, [](Value& func) {
    bool simplifiedAnElse = false;

    traversePre(func, [&simplifiedAnElse](Value& node) {
      // simplify   if (x) { if (y) { .. } }   to   if (x ? y : 0) { .. }
      if (node[0] == IF) {
        Value& body = node[2];
        // recurse to handle chains
        while (body[0] == BLOCK) {
          Value& stats = body[1];
          if (stats.Size() == 0) break;
          Value& other = stats[stats.Size()-1];
          if (other[0] != IF) {
            // our if block does not end with an if. perhaps if have an else we can flip
            if (node.Size() >= 4 && node[3][0] == BLOCK) {
              stats = node[3][1];
              if (stats.Size() == 0) break;
              other = stats[stats.Size()-1];
              if (other[0] == IF) {
                // flip node
                node[1] = flipCondition(node[1]);
                node[2] = node[3];
                node[3] = body;
                body = node[2];
              } else break;
            } else break;
          }
          // we can handle elses, but must be fully identical
          if (node.Size() >= 4 || other.Size() >= 4) {
            if (!(node.Size() >= 4)) break;
            if (node[3] != other[3]) {
              // the elses are different, but perhaps if we flipped a condition we can do better
              if (node[3] == other[2]) {
                // flip other. note that other may not have had an else! add one if so; we will eliminate such things later
                if (!(other.Size() >= 4)) other.PushBack(makeBlock(), doc.GetAllocator());
                other[1] = flipCondition(other[1]);
                Value& temp = other[2];
                other[2] = other[3];
                other[3] = temp;
              } else break;
            }
          }
          if (stats.Size() > 1) {
            // try to commaify - turn everything between the ifs into a comma operator inside the second if
            bool ok = true;
            for (int i = 0; i < stats.Size()-1; i++) {
              Value& curr = deStat(stats[i]);
              if (commable(curr)) ok = false;
            }
            if (!ok) break;
            for (int i = stats.Size()-2; i >= 0; i--) {
              Value& curr = deStat(stats[i]);
              other[1] = mmake2(SEQ, curr, other[1]);
            }
            Value temp;
            temp.SetArray();
            temp.PushBack(other, doc.GetAllocator());
            stats = body[1] = temp;
          }
          if (stats.Size() != 1) break;
          if (node.Size() >= 4) simplifiedAnElse = true;
          node[1] = mmake3(CONDITIONAL, node[1], other[1], makeNum(0));
          body = node[2] = other[2]; // XXX
        }
      }
    });

    if (simplifiedAnElse) {
      // there may be fusing opportunities

      // we can only fuse if we remove all uses of the label. if there are
      // other ones - if the label check can be reached from elsewhere -
      // we must leave it
      bool abort = false;

      std::unordered_map<std::string, int> labelAssigns;

      traversePre(func, [&labelAssigns, &abort](Value& node) {
        if (node[0] == ASSIGN && node[2][0] == NAME && node[2][1] == LABEL) {
          if (node[3][0] == NUM) {
            std::string value = node[3][1].GetString();
            labelAssigns[value] = labelAssigns[value] + 1;
          } else {
            // label is assigned a dynamic value (like from indirectbr), we cannot do anything
            abort = true;
          }
        }
      });
      if (abort) return;

      std::unordered_map<std::string, int> labelChecks;

      traversePre(func, [&labelChecks, &abort](Value& node) {
        if (node[0] == BINARY && node[1] == "==" && node[2][0] == BINARY && node[2][1] == '|' &&
            node[2][2][0] == NAME && node[2][2][1] == LABEL) {
          if (node[3][0] == NUM) {
            std::string value = node[3][1].GetString();
            labelChecks[value] = labelChecks[value] + 1;
          } else {
            // label is checked vs a dynamic value (like from indirectbr), we cannot do anything
            abort = true;
          }
        }
      });
      if (abort) return;

      int inLoop = 0; // when in a loop, we do not emit   label = 0;   in the relooper as there is no need
      traversePrePost(func, [&inLoop, &labelAssigns, &labelChecks](Value& node) {
        if (node[0] == WHILE) inLoop++;
        Value& stats = getStatements(node);
        if (!stats.IsNull()) {
          for (int i = 0; i < stats.Size()-1; i++) {
            Value& pre = stats[i];
            Value& post = stats[i+1];
            if (pre[0] == IF && pre.Size() >= 4 && post[0] == IF && post.Size() < 4) {
              Value& postCond = post[1];
              if (postCond[0] == BINARY && postCond[1] == "==" &&
                  postCond[2][0] == BINARY && postCond[2][1] == "|" &&
                  postCond[2][2][0] == NAME && postCond[2][2][1] == LABEL &&
                  postCond[2][3][0] == NUM && postCond[2][3][1] == 0 &&
                  postCond[3][0] == NUM) {
                Value& postValue = postCond[3][1];
                Value& preElse = pre[3];
                if (labelAssigns[postValue.GetString()] == 1 && labelChecks[postValue.GetString()] == 1 && preElse[0] == BLOCK && preElse.Size() >= 2 && preElse[1].Size() == 1) {
                  Value& preStat = preElse[1][0];
                  if (preStat[0] == STAT && preStat[1][0] == ASSIGN &&
                      preStat[1][1] == true && preStat[1][2][0] == NAME && preStat[1][2][1] == LABEL &&
                      preStat[1][3][0] == NUM && preStat[1][3][1] == postValue) {
                    // Conditions match, just need to make sure the post clears label
                    if (post[2][0] == BLOCK && post[2].Size() >= 2 && post[2][1].Size() > 0) {
                      Value& postStat = post[2][1][0];
                      bool haveClear =
                        postStat[0] == STAT && postStat[1][0] == ASSIGN &&
                        postStat[1][1] == true && postStat[1][2][0] == NAME && postStat[1][2][1] == LABEL &&
                        postStat[1][3][0] == NUM && postStat[1][3][1] == 0;
                      if (!inLoop || haveClear) {
                        // Everything lines up, do it
                        pre[3] = post[2];
                        if (haveClear) splice(pre[3][1], 0); // remove the label clearing
                        splice(stats, i+1); // remove the post entirely
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }, [&inLoop](Value& node) {
        if (node[0] == WHILE) inLoop--;
      });
    }
  });
}

void optimizeFrounds(Value &ast) {
  // collapse fround(fround(..)), which can happen due to elimination
  // also emit f0 instead of fround(0) (except in returns)
  bool inReturn = false;
  std::function<void (Value&)> fix = [&](Value& node) {
    bool ret = node[0] == RETURN;
    if (ret) inReturn = true;
    traverseChildren(node, fix);
    if (ret) inReturn = false;
    if (node[0] == CALL && node[1][0] == NAME && node[1][1] == MATH_FROUND) {
      Value& arg = node[2][0];
      if (arg[0] == NUM) {
        if (!inReturn && arg[1] == 0) node = makeName("f0");
      } else if (arg[0] == CALL && arg[1][0] == NAME && arg[1][1] == MATH_FROUND) {
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
  //printf("reading %s, %d bytes\n", input, size);
  char *json = new char[size+1];
  FILE *f = fopen(input, "rb");
  int num = fread(json, 1, size, f);
  assert(num == size);
  fclose(f);
  json[size] = 0;

  char *comment = strstr(json, "//");
  if (comment) *comment = 0; // drop off the comments; TODO: parse extra info

  // Parse JSON source into a Document
  doc.Parse(json);
  assert(!doc.HasParseError());
  delete[] json;

  // Run passes on the Document
  for (int i = 2; i < argc; i++) {
    std::string str(argv[i]);
    if (str == "asm") {} // the default for us
    else if (str == "asmPreciseF32") preciseF32 = true;
    else if (str == "receiveJSON" || str == "emitJSON") {} // the default for us
    else if (str == "optimizeFrounds") optimizeFrounds(doc);
    else if (str == "simplifyIfs") simplifyIfs(doc);
    else {
      fprintf(stderr, "unrecognized argument: %s\n", str.c_str());
      assert(0);
    }
  }

  // Emit JSON of modified Document
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);
  puts(buffer.GetString());

  return 0;
}

