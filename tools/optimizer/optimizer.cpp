#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include <iostream>

#include "minijson.h"

//==================
// Globals
//==================

Ref doc;

//==================
// Infrastructure
//==================

#define err(str) fprintf(stderr, str "\n");
#define errv(str, ...) fprintf(stderr, str "\n", __VA_ARGS__);

void dump(const char *str, Ref node, bool pretty=false) {
  std::cerr << str << ": ";
  node->stringify(std::cerr, pretty);
  std::cerr << std::endl;
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
void traverseChildren(Ref node, std::function<void (Ref)> visit) {
  //dump("TC", node);
  if (!node->isArray()) return;
  int size = node->size();
  //printf("size: %d\n", size);
  for (int i = 0; i < size; i++) {
    //printf("  %d:\n", i);
    Ref subnode = node[i];
    if (subnode->isArray() and subnode->size() > 0) {
      //printf("    go\n");
      visit(subnode);
    }
  }
}

// Traverse, calling visit before the children
void traversePre(Ref node, std::function<void (Ref)> visit) {
  visit(node);
  traverseChildren(node, [&visit](Ref  node) {
    traversePre(node, visit);
  });
}

// Traverse, calling visitPre before the children and visitPost after
void traversePrePost(Ref node, std::function<void (Ref)> visitPre, std::function<void (Ref)> visitPost) {
  visitPre(node);
  traverseChildren(node, [&visitPre, &visitPost](Ref  node) {
    traversePrePost(node, visitPre, visitPost);
  });
  visitPost(node);
}

// Traverses all the top-level functions in the document
void traverseFunctions(Ref ast, std::function<void (Ref)> visit) {
  if (ast[0] == "toplevel") {
    Ref stats = ast[1];
    for (int i = 0; i < stats->size(); i++) {
      Ref curr = stats[i];
      if (curr[0] == "defun") visit(curr);
    }
  } else if (ast[0] == "defun") {
    visit(ast);
  }
}

Ref deStat(Ref  node) {
  if (node[0]->getString() =="stat") return node[1];
  return node;
}

Ref getStatements(Ref  node) {
  if (node[0] == "defun") {
    return node[3];
  } else if (node[0] == "block") {
    return node[1];
  } else {
    return new Value();
  }
}

// Constructions TODO: share common constructions, and assert they remain frozen

Ref makeName(const char *str) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("name"));
  ret->push_back(new Value(str));
  return ret;
}

Ref makeNum(double x) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("num"));
  ret->push_back(new Value(x));
  return ret;
}

Ref makeBlock() {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("block"));
  ret->push_back(new ArrayValue());
  return ret;
}

Ref make2(const char* type, const char *a, Ref b) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value(type));
  ret->push_back(new Value(a));
  ret->push_back(b);
  return ret;
}

Ref make2(const char* type, Ref a, Ref b) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value(type));
  ret->push_back(a);
  ret->push_back(b);
  return ret;
}

Ref make3(const char *type, Ref a, Ref b, Ref c) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value(type));
  ret->push_back(a);
  ret->push_back(b);
  ret->push_back(c);
  return ret;
}

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

AsmType detectType(Ref node, AsmInfo *asmInfo=nullptr, bool inVarDef=false) {
  switch (node[0]->getString()[0]) {
    case 'n': {
      if (node[0] == "num") {
        if (fmod(node[1]->getNumber(), 1) != 0) return ASM_DOUBLE;
        return ASM_INT;
      } else if (node[0] == "name") {
        if (asmInfo) {
          AsmType ret = asmInfo->getType(node[1]->getString());
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
      if (node[0] == "unary-prefix") {
        switch (node[1]->getString()[0]) {
          case '+': return ASM_DOUBLE;
          case '-': return detectType(node[2], asmInfo, inVarDef);
          case '!': case '~': return ASM_INT;
        }
        break;
      }
      break;
    }
    case 'c': {
      if (node[0] == "call") {
        if (node[1][0] == "name") {
          std::string& name = node[1][1]->getString();
          if (name == "Math_fround") return ASM_FLOAT;
          else if (name == "SIMD_float32x4") return ASM_FLOAT32X4;
          else if (name == "SIMD_int32x4") return ASM_INT32X4;
        }
        return ASM_NONE;
      } else if (node[0] == "conditional") {
        return detectType(node[2], asmInfo, inVarDef);
      }
      break;
    }
    case 'b': {
      if (node[0] == "binary") {
        switch (node[1]->getString()[0]) {
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
      if (node[0] == "seq") {
        return detectType(node[2], asmInfo, inVarDef);
      } else if (node[0] == "sub") {
        assert(node[1][0] == "name");
        HeapInfo info = parseHeap(node[1][1]->getCString());
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
Ref simplifyNotCompsDirect(Ref node) {
  if (node[0] == "unary-prefix" && node[1] == "!") {
    // de-morgan's laws do not work on floats, due to nans >:(
    if (node[2][0] == "binary" && (detectType(node[2][2]) == ASM_INT && detectType(node[2][3]) == ASM_INT)) {
      Ref op = node[2][1];
      switch(op->getCString()[0]) {
        case '<': {
          if (op == "<")  { op->set(">="); break; }
          if (op == "<=") { op->set(">"); break; }
          return node;
        }
        case '>': {
          if (op == ">")  { op->set("<="); break; }
          if (op == ">=") { op->set("<"); break; }
          return node;
        }
        case '=': {
          if (op == "==") { op->set("!="); break; }
          return node;
        }
        case '!': {
          if (op == "!=") { op->set("=="); break; }
          return node;
        }
        default: return node;
      }
      return make3("binary", op, node[2][2], node[2][3]);
    } else if (node[2][0] == "unary-prefix" && node[2][1] == "!") {
      return node[2][2];
    }
  }
  return node;
}

Ref flipCondition(Ref cond) {
  return simplifyNotCompsDirect(make2("unary-prefix", "!", cond));
}

// Checks

bool commable(Ref  node) { // TODO: hashing
  std::string type = node[0]->getString();
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

void simplifyIfs(Ref ast) {
  traverseFunctions(ast, [](Ref func) {
    bool simplifiedAnElse = false;

    traversePre(func, [&simplifiedAnElse](Ref  node) {
      // simplify   if (x) { if (y) { .. } }   to   if (x ? y : 0) { .. }
      if (node[0] == "if") {
        Ref body = node[2];
        // recurse to handle chains
        while (body[0] == "block") {
          Ref stats = body[1];
          if (stats->size() == 0) break;
          Ref other = stats[stats->size()-1];
          if (other[0] != "if") {
            // our if block does not end with an if. perhaps if have an else we can flip
            if (!!node[3] && node[3][0] == "block") {
              stats = node[3][1];
              if (stats->size() == 0) break;
              other = stats[stats->size()-1];
              if (other[0] == "if") {
                // flip node
                node[1] = flipCondition(node[1]);
                node[2] = node[3];
                node[3] = body;
                body = node[2];
              } else break;
            } else break;
          }
          // we can handle elses, but must be fully identical
          if (!!node[3] || !!other[3]) {
            if (!node[3]) break;
            if (!node[3]->deepCompare(other[3])) {
              // the elses are different, but perhaps if we flipped a condition we can do better
              if (node[3]->deepCompare(other[2])) {
                // flip other. note that other may not have had an else! add one if so; we will eliminate such things later
                if (!other[3]) other[3] = makeBlock();
                other[1] = flipCondition(other[1]);
                Ref temp = other[2];
                other[2] = other[3];
                other[3] = temp;
              } else break;
            }
          }
          if (stats->size() > 1) {
            // try to commaify - turn everything between the ifs into a comma operator inside the second if
            bool ok = true;
            for (int i = 0; i < stats->size()-1; i++) {
              Ref curr = deStat(stats[i]);
              if (!commable(curr)) ok = false;
            }
            if (!ok) break;
            for (int i = stats->size()-2; i >= 0; i--) {
              Ref curr = deStat(stats[i]);
              other[1] = make2("seq", curr, other[1]);
            }
            Ref temp = new ArrayValue();
            temp->push_back(other);
            stats = body[1] = temp;
          }
          if (stats->size() != 1) break;
          if (!!node[3]) simplifiedAnElse = true;
          node[1] = make3("conditional", node[1], other[1], makeNum(0));
          body = node[2] = other[2];
        }
      }
    });

    if (simplifiedAnElse) {
      // there may be fusing opportunities

      // we can only fuse if we remove all uses of the label. if there are
      // other ones - if the label check can be reached from elsewhere -
      // we must leave it
      bool abort = false;

      std::unordered_map<int, int> labelAssigns;

      traversePre(func, [&labelAssigns, &abort](Ref  node) {
        if (node[0] == "assign" && node[2][0] == "name" && node[2][1] == "label") {
          if (node[3][0] == "num") {
            int value = node[3][1]->getNumber();
            labelAssigns[value] = labelAssigns[value] + 1;
          } else {
            // label is assigned a dynamic value (like from indirectbr), we cannot do anything
            abort = true;
          }
        }
      });
      if (abort) return;

      std::unordered_map<int, int> labelChecks;

      traversePre(func, [&labelChecks, &abort](Ref  node) {
        if (node[0] == "binary" && node[1] == "==" && node[2][0] == "binary" && node[2][1] == "|" &&
            node[2][2][0] == "name" && node[2][2][1] == "label") {
          if (node[3][0] == "num") {
            int value = node[3][1]->getNumber();
            labelChecks[value] = labelChecks[value] + 1;
          } else {
            // label is checked vs a dynamic value (like from indirectbr), we cannot do anything
            abort = true;
          }
        }
      });
      if (abort) return;

      int inLoop = 0; // when in a loop, we do not emit   label = 0;   in the relooper as there is no need
      traversePrePost(func, [&inLoop, &labelAssigns, &labelChecks](Ref  node) {
        if (node[0] == "while") inLoop++;
        Ref stats = getStatements(node);
        if (!stats->isNull() && stats->size() > 0) {
          for (int i = 0; i < stats->size()-1; i++) {
            Ref pre = stats[i];
            Ref post = stats[i+1];
            if (pre[0] == "if" && !!pre[3] && post[0] == "if" && !post[3]) {
              Ref postCond = post[1];
              if (postCond[0] == "binary" && postCond[1] == "==" &&
                  postCond[2][0] == "binary" && postCond[2][1] == "|" &&
                  postCond[2][2][0] == "name" && postCond[2][2][1] == "label" &&
                  postCond[2][3][0] == "num" && postCond[2][3][1]->getNumber() == 0 &&
                  postCond[3][0] == "num") {
                double postValue = postCond[3][1]->getNumber();
                Ref preElse = pre[3];
                if (labelAssigns[postValue] == 1 && labelChecks[postValue] == 1 && preElse[0] == "block" && preElse->size() >= 2 && preElse[1]->size() == 1) {
                  Ref preStat = preElse[1][0];
                  if (preStat[0] == "stat" && preStat[1][0] == "assign" &&
                      preStat[1][1]->isBool(true) && preStat[1][2][0] == "name" && preStat[1][2][1] == "label" &&
                      preStat[1][3][0] == "num" && preStat[1][3][1]->getNumber() == postValue) {
                    // Conditions match, just need to make sure the post clears label
                    if (post[2][0] == "block" && post[2]->size() >= 2 && post[2][1]->size() > 0) {
                      Ref postStat = post[2][1][0];
                      bool haveClear =
                        postStat[0] == "stat" && postStat[1][0] == "assign" &&
                        postStat[1][1]->isBool(true) && postStat[1][2][0] == "name" && postStat[1][2][1] == "label" &&
                        postStat[1][3][0] == "num" && postStat[1][3][1]->getNumber() == 0;
                      if (!inLoop || haveClear) {
                        // Everything lines up, do it
                        pre[3] = post[2];
                        if (haveClear) pre[3][1]->splice(0, 1); // remove the label clearing
                        stats->splice(i+1, 1); // remove the post entirely
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }, [&inLoop](Ref  node) {
        if (node[0] == "while") inLoop--;
      });
      assert(inLoop == 0);
    }
  });
}

void optimizeFrounds(Ref ast) {
  // collapse fround(fround(..)), which can happen due to elimination
  // also emit f0 instead of fround(0) (except in returns)
  bool inReturn = false;
  std::function<void (Ref)> fix = [&](Ref  node) {
    bool ret = node[0] == "return";
    if (ret) inReturn = true;
    traverseChildren(node, fix);
    if (ret) inReturn = false;
    if (node[0] == "call" && node[1][0] == "name" && node[1][1] == "Math_fround") {
      Ref arg = node[2][0];
      if (arg[0] == "num") {
        if (!inReturn && arg[1]->getNumber() == 0) *node = *makeName("f0");
      } else if (arg[0] == "call" && arg[1][0] == "name" && arg[1][1] == "Math_fround") {
        *node = *arg;
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

  // Parse JSON source into the document
  doc = new Value();
  doc->parse(json);
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
  doc->stringify(std::cout);
  std::cout << std::endl;

  return 0;
}

