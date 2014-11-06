#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

#include "minijson.h"

//==================
// Globals
//==================

Ref doc;

//==================
// Infrastructure
//==================

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

std::string getIntStr(int x) {
  switch (x) {
    case 1: return "1";
    case 8: return "8";
    case 16: return "16";
    case 32: return "32";
    case 64: return "64";
  }
  assert(0);
  return ":(";
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
  traverseChildren(node, [&visit](Ref node) {
    traversePre(node, visit);
  });
}

// Traverse, calling visitPre before the children and visitPost after
void traversePrePost(Ref node, std::function<void (Ref)> visitPre, std::function<void (Ref)> visitPost) {
  visitPre(node);
  traverseChildren(node, [&visitPre, &visitPost](Ref node) {
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

Ref deStat(Ref node) {
  if (node[0]->getString() =="stat") return node[1];
  return node;
}

Ref getStatements(Ref node) {
  if (node[0] == "defun") {
    return node[3];
  } else if (node[0] == "block") {
    return node[1];
  } else {
    return new Value();
  }
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

struct AsmData {
  std::map<std::string, int> params; // name => index
  std::map<std::string, AsmType> types; // name => type
  AsmType ret;

  AsmData() : ret(ASM_NONE) {}

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

bool isInteger(double x) {
  return fmod(x, 1) == 0;
}

bool isInteger32(double x) {
  return isInteger(x) && (x == (int32_t)x || x == (uint32_t)x);
}

std::string ASM_FLOAT_ZERO;

AsmType detectType(Ref node, AsmData *asmData=nullptr, bool inVarDef=false) {
  switch (node[0]->getString()[0]) {
    case 'n': {
      if (node[0] == "num") {
        if (!isInteger(node[1]->getNumber())) return ASM_DOUBLE;
        return ASM_INT;
      } else if (node[0] == "name") {
        if (asmData) {
          AsmType ret = asmData->getType(node[1]->getString());
          if (ret != ASM_NONE) return ret;
        }
        if (!inVarDef) {
          if (node[1] == "inf" || node[1] == "nan") return ASM_DOUBLE;
          if (node[1] == "tempRet0") return ASM_INT;
          return ASM_NONE;
        }
        // We are in a variable definition, where Math_fround(0) optimized into a global constant becomes f0 = Math_fround(0)
        if (ASM_FLOAT_ZERO.size() == 0) ASM_FLOAT_ZERO = node[1]->getString();
        else assert(ASM_FLOAT_ZERO == node[1]->getString());
        return ASM_FLOAT;
      }
      break;
    }
    case 'u': {
      if (node[0] == "unary-prefix") {
        switch (node[1]->getString()[0]) {
          case '+': return ASM_DOUBLE;
          case '-': return detectType(node[2], asmData, inVarDef);
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
        return detectType(node[2], asmData, inVarDef);
      }
      break;
    }
    case 'b': {
      if (node[0] == "binary") {
        switch (node[1]->getString()[0]) {
          case '+': case '-':
          case '*': case '/': case '%': return detectType(node[2], asmData, inVarDef);
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
        return detectType(node[2], asmData, inVarDef);
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

// Constructions TODO: share common constructions, and assert they remain frozen

Ref makeEmpty() {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("toplevel"));
  ret->push_back(new ArrayValue());
  return ret;
}

Ref makePair(Ref x, Ref y) {
  Ref ret = new ArrayValue();
  ret->push_back(x);
  ret->push_back(y);
  return ret;
};

Ref makeNum(double x) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("num"));
  ret->push_back(new Value(x));
  return ret;
}

Ref makeName(const char *str) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("name"));
  ret->push_back(new Value(str));
  return ret;
}

Ref makeName(const std::string& str) {
  return makeName(str.c_str());
}

Ref makeBlock() {
  Ref ret(new ArrayValue());
  ret->push_back(new Value("block"));
  ret->push_back(new ArrayValue());
  return ret;
}

Ref make1(const char* type, Ref a) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value(type));
  ret->push_back(a);
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

Ref make3(const char *type, const char *a, Ref b, Ref c) {
  Ref ret(new ArrayValue());
  ret->push_back(new Value(type));
  ret->push_back(new Value(a));
  ret->push_back(b);
  ret->push_back(c);
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

Ref makeAsmVarDef(const std::string& v_, AsmType type) {
  Ref v = new Value(v_);
  Ref val;
  switch (type) {
    case ASM_INT: val = makeNum(0); break;
    case ASM_DOUBLE: val = make2("unary-prefix", "+", makeNum(0)); break;
    case ASM_FLOAT: {
      if (ASM_FLOAT_ZERO.size() > 0) {
        val = makeName(ASM_FLOAT_ZERO.c_str());
      } else {
        return make2("call", makeName("Math_fround"), &(new ArrayValue())->push_back(makeNum(0)));
      }
      break;
    }
    case ASM_FLOAT32X4: {
      val = make2("call", makeName("SIMD_float32x4"), &(new ArrayValue())->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_INT32X4: {
      val = make2("call", makeName("SIMD_int32x4"), &(new ArrayValue())->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    default: assert(0);
  }
  return makePair(v, val);
}

Ref makeAsmCoercion(Ref node, AsmType type) {
  switch (type) {
    case ASM_INT: return make3("binary", "|", node, makeNum(0));
    case ASM_DOUBLE: return make2("unary-prefix", "+", node);
    case ASM_FLOAT: return make2("call", makeName("Math_fround"), &(new ArrayValue())->push_back(node));
    case ASM_FLOAT32X4: return make2("call", makeName("SIMD_float32x4"), &(new ArrayValue())->push_back(node));
    case ASM_INT32X4: return make2("call", makeName("SIMD_int32x4"), &(new ArrayValue())->push_back(node));
    case ASM_NONE:
    default: return node; // non-validating code, emit nothing XXX this is dangerous, we should only allow this when we know we are not validating
  }
}

// Checks

bool isEmpty(Ref node) {
  return node->size() == 2 && node[0] == "toplevel" && node[1]->size() == 0;
}

bool commable(Ref node) { // TODO: hashing
  std::string type = node[0]->getString();
  if (type == "assign" || type == "binary" || type == "unary-prefix" || type == "unary-postfix" || type == "name" || type == "num" || type == "call" || type == "seq" || type == "conditional" || type == "sub") return true;
  return false;
}

bool isMathFunc(const char *name) {
  static const char *Math_ = "Math_";
  static unsigned size = strlen(Math_);
  return strncmp(name, Math_, size) == 0;
}

bool isMathFunc(Ref value) {
  return value->isString() && isMathFunc(value->getString().c_str());
}

bool callHasSideEffects(Ref node) { // checks if the call itself (not the args) has side effects (or is not statically known)
  return !(node[1][0] == "name" && isMathFunc(node[1][1]));
}

bool hasSideEffects(Ref node) { // this is 99% incomplete!
  std::string& type = node[0]->getString();
  switch (type[0]) {
    case 'n':
      if (type == "num" || type == "name") return false;
      break;
    case 's':
      if (type == "string") return false;
      if (type == "sub") return hasSideEffects(node[1]) || hasSideEffects(node[2]);
      break;
    case 'u':
      if (type == "unary-prefix") return hasSideEffects(node[2]);
      break;
    case 'b':
      if (type == "binary") return hasSideEffects(node[2]) || hasSideEffects(node[3]);
      break;
    case 'c':
      if (type == "call") {
        if (callHasSideEffects(node)) return true;
        // This is a statically known call, with no side effects. only args can side effect us
        for (auto arg : node[2]->getArray()) {
          if (hasSideEffects(arg)) return true;
        }
        return false;
      } else if (type == "conditional") return hasSideEffects(node[1]) || hasSideEffects(node[2]) || hasSideEffects(node[3]); 
      break;
  }
  return true;
}

// Transforms
/*
struct AsmData {
  std::map<std::string, int> params; // name => index
  std::map<std::string, AsmType> types; // name => type
  type ret

  AsmType getType(std::string name) {
    auto ret = types.find(name);
    if (ret != types.end()) return ret->second;
    return ASM_NONE;
  }
};
*/
AsmData normalizeAsm(Ref func) {
  AsmData data;
  // process initial params
  Ref stats = func[3];
  int i = 0;
  while (i < stats->size()) {
    Ref node = stats[i];
    if (node[0] != "stat" || node[1][0] != "assign" || node[1][2][0] != "name") break;
    node = node[1];
    Ref name = node[2][1];
    if (!!func[2] && func[2]->indexOf(name) < 0) break; // not an assign into a parameter, but a global
    std::string& str = name->getString();
    if (data.params.count(str) > 0) break; // already done that param, must be starting function body
    data.params[str] = func[2]->indexOf(name);
    data.types[str] = detectType(node[3]);
    stats[i] = makeEmpty();
    i++;
  }
  // process initial variable definitions
  while (i < stats->size()) {
    Ref node = stats[i];
    if (node[0] != "var") break;
    for (int j = 0; j < node[1]->size(); j++) {
      Ref v = node[1][j];
      std::string name = v[0]->getString();
      Ref value = v[1];
      if (!data.types.count(name)) {
        data.types[name] = detectType(value, nullptr, true);
        v->setSize(1); // make an un-assigning var
      } else {
        assert(j == 0); // cannot break in the middle
        goto outside;
      }
    }
    i++;
  }
  outside:
  // look for other var definitions and collect them
  while (i < stats->size()) {
    traversePre(stats[i], [](Ref node) {
      Ref type = node[0];
      if (type == "var") {
        assert(0); //, 'should be no vars to fix! ' + func[1] + ' : ' + JSON.stringify(node));
      }
    });
    i++;
  }
  // look for final "return" statement to get return type.
  Ref retStmt = stats[stats->size() - 1];
  if (!!retStmt && retStmt[0] == "return" && !!retStmt[1]) {
    data.ret = detectType(retStmt[1]);
  }
  return data;
}

void denormalizeAsm(Ref func, AsmData& data) {
  Ref stats = func[3];
  // Remove var definitions, if any
  for (int i = 0; i < stats->size(); i++) {
    if (stats[i][0] == "var") {
      stats[i] = makeEmpty();
    } else {
      if (!isEmpty(stats[i])) break;
    }
  }
  // calculate variable definitions
  Ref varDefs = new ArrayValue();
  for (auto v : data.types) {
    varDefs->push_back(makeAsmVarDef(v.first, v.second));
  }
  // each param needs a line; reuse emptyNodes as much as we can
  int numParams = data.params.size();
  int emptyNodes = 0;
  while (emptyNodes < stats->size()) {
    if (!isEmpty(stats[emptyNodes])) break;
    emptyNodes++;
  }
  int neededEmptyNodes = numParams + (varDefs->size() ? 1 : 0); // params plus one big var if there are vars
  if (neededEmptyNodes > emptyNodes) {
    stats->insert(0, neededEmptyNodes - emptyNodes);
  } else if (neededEmptyNodes < emptyNodes) {
    stats->splice(0, emptyNodes - neededEmptyNodes);
  }
  // add param coercions
  int next = 0;
  for (auto param : func[2]->getArray()) {
    std::string& str = param->getString();
    stats[next++] = make1("stat", make3("assign", &(new Value())->setBool(true), makeName(str), makeAsmCoercion(makeName(str), data.types[str])));
  }
  if (varDefs->size()) {
    stats[next] = make1("var", varDefs);
  }
  /*
  if (data.inlines.length > 0) {
    var i = 0;
    traverse(func, function(node, type) {
      if (type == "call" && node[1][0] == "name" && node[1][1] == 'inlinejs') {
        node[1] = data.inlines[i++]; // swap back in the body
      }
    });
  }
  */
  // ensure that there's a final "return" statement if needed.
  if (data.ret != ASM_NONE) {
    Ref retStmt = stats[stats->size() - 1];
    if (!retStmt || retStmt[0]->getString() != "return") {
      Ref retVal = makeNum(0);
      if (data.ret != ASM_INT) {
        retVal = makeAsmCoercion(retVal, data.ret);
      }
      stats->push_back(make1("return", retVal));
    }
  }
  //printErr('denormalized \n\n' + astToSrc(func) + '\n\n');
}

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
          if (op == "<")  { op->setString(">="); break; }
          if (op == "<=") { op->setString(">"); break; }
          return node;
        }
        case '>': {
          if (op == ">")  { op->setString("<="); break; }
          if (op == ">=") { op->setString("<"); break; }
          return node;
        }
        case '=': {
          if (op == "==") { op->setString("!="); break; }
          return node;
        }
        case '!': {
          if (op == "!=") { op->setString("=="); break; }
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

void safeCopy(Ref target, Ref source) { // safely copy source onto target, even if source is a subnode of target
  Ref temp = source; // hold on to source
  *target = *temp;
}

// Calculations

int measureCost(Ref ast) {
  int size = 0;
  traversePre(ast, [&size](Ref node) {
    Ref type = node[0];
    if (type == "num" || type == "unary-prefix") size--;
    else if (type == "binary") {
      if (node[3][0] == "num" && node[3][1]->getNumber() == 0) size--;
      else if (node[1] == "/" || node[1] == "%") size += 2;
    }
    else if (type == "call" && !callHasSideEffects(node)) size -= 2;
    else if (type == "sub") size++;
    size++;
  });
  return size;
}

//==================
// Params
//==================

bool preciseF32 = false;

//=====================
// Optimization passes
//=====================

class StringSet : public std::unordered_set<std::string> {
public:
  StringSet(const char *init) { // comma-delimited list
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

  bool has(const char *str) {
    return count(str) > 0;
  }
  bool has(Ref node) {
    return has(node->getString().c_str());
  }
};

StringSet USEFUL_BINARY_OPS("<< >> | & ^"),
          COMPARE_OPS("< <= > >= == == != !=="),
          BITWISE("| & ^"),
          SAFE_BINARY_OPS("+ -"), // division is unsafe as it creates non-ints in JS; mod is unsafe as signs matter so we can't remove |0's; mul does not nest with +,- in asm
          COERCION_REQUIRING_OPS("sub unary-prefix"), // ops that in asm must be coerced right away
          COERCION_REQUIRING_BINARIES("* / %"); // binary ops that in asm must be coerced

bool isFunctionTable(const char *name) {
  static const char *functionTable = "FUNCTION_TABLE";
  static unsigned size = strlen(functionTable);
  return strncmp(name, functionTable, size) == 0;
}

bool isFunctionTable(Ref value) {
  return value->isString() && isFunctionTable(value->getString().c_str());
}

void simplifyExpressions(Ref ast) {
  // Simplify common expressions used to perform integer conversion operations
  // in cases where no conversion is needed.
  auto simplifyIntegerConversions = [](Ref ast) {
    traversePre(ast, [](Ref node) {
      Ref type = node[0];
      if (type == "binary"    && node[1]    == ">>" && node[3][0] == "num" &&
          node[2][0] == "binary" && node[2][1] == "<<" && node[2][3][0] == "num" && node[3][1]->getNumber() == node[2][3][1]->getNumber()) {
        // Transform (x&A)<<B>>B to X&A.
        Ref innerNode = node[2][2];
        double shifts = node[3][1]->getNumber();
        if (innerNode[0] == "binary" && innerNode[1] == "&" && innerNode[3][0] == "num") {
          double mask = innerNode[3][1]->getNumber();
          if (isInteger32(mask) && isInteger32(shifts) && ((int(mask) << int(shifts)) >> int(shifts)) == int(mask)) {
            safeCopy(node, innerNode);
            return;
          }
        }
      } else if (type == "binary" && BITWISE.has(node[1])) {
        for (int i = 2; i <= 3; i++) {
          Ref subNode = node[i];
          if (subNode[0] == "binary" && subNode[1] == "&" && subNode[3][0] == "num" && subNode[3][1]->getNumber() == 1) {
            // Rewrite (X < Y) & 1 to X < Y , when it is going into a bitwise operator. We could
            // remove even more (just replace &1 with |0, then subsequent passes could remove the |0)
            // but v8 issue #2513 means the code would then run very slowly in chrome.
            Ref input = subNode[2];
            if (input[0] == "binary" && COMPARE_OPS.has(input[1])) {
              safeCopy(node[i], input);
            }
          }
        }
      }
    });
  };

  // When there is a bunch of math like (((8+5)|0)+12)|0, only the external |0 is needed, one correction is enough.
  // At each node, ((X|0)+Y)|0 can be transformed into (X+Y): The inner corrections are not needed
  // TODO: Is the same is true for 0xff, 0xffff?
  // Likewise, if we have |0 inside a block that will be >>'d, then the |0 is unnecessary because some
  // 'useful' mathops already |0 anyhow.

  auto simplifyOps = [](Ref ast) {
    auto removeMultipleOrZero = [&ast] {
      bool rerun = true;
      while (rerun) {
        rerun = false;
        std::vector<int> stack;
        std::function<void (Ref)> process = [&stack, &rerun, &process, &ast](Ref node) {
          Ref type = node[0];
          if (type == "binary" && node[1] == "|") {
            if (node[2][0] == "num" && node[3][0] == "num") {
              node[2][1]->setNumber(int(node[2][1]->getNumber()) | int(node[3][1]->getNumber()));
              stack.push_back(0);
              safeCopy(node, node[2]);
              return;
            }
            bool go = false;
            if (node[2][0] == "num" && node[2][1]->getNumber() == 0) {
              // canonicalize order
              Ref temp = node[3];
              node[3] = node[2];
              node[2] = temp;
              go = true;
            } else if (node[3][0] == "num" && node[3][1]->getNumber() == 0) {
              go = true;
            }
            if (!go) {
              stack.push_back(1);
              return;
            }
            // We might be able to remove this correction
            for (int i = stack.size()-1; i >= 0; i--) {
              if (stack[i] >= 1) {
                if (stack[stack.size()-1] < 2 && node[2][0] == "call") break; // we can only remove multiple |0s on these
                if (stack[stack.size()-1] < 1 && (COERCION_REQUIRING_OPS.has(node[2][0]) ||
                                                 (node[2][0] == "binary" && COERCION_REQUIRING_BINARIES.has(node[2][1])))) break; // we can remove |0 or >>2
                // we will replace ourselves with the non-zero side. Recursively process that node.
                Ref result = node[2][0] == "num" && node[2][1]->getNumber() == 0 ? node[3] : node[2], other;
                // replace node in-place
                safeCopy(node, result);
                rerun = true;
                process(result);
                return;
              } else if (stack[i] == -1) {
                break; // Too bad, we can't
              }
            }
            stack.push_back(2); // From here on up, no need for this kind of correction, it's done at the top
                           // (Add this at the end, so it is only added if we did not remove it)
          } else if (type == "binary" && USEFUL_BINARY_OPS.has(node[1])) {
            stack.push_back(1);
          } else if ((type == "binary" && SAFE_BINARY_OPS.has(node[1])) || type == "num" || type == "name") {
            stack.push_back(0); // This node is safe in that it does not interfere with this optimization
          } else if (type == "unary-prefix" && node[1] == "~") {
            stack.push_back(1);
          } else {
            stack.push_back(-1); // This node is dangerous! Give up if you see this before you see '1'
          }
        };

        traversePrePost(ast, process, [&stack](Ref Node) {
          stack.pop_back();
        });
      }
    };

    removeMultipleOrZero();
    return;

    // & and heap-related optimizations

    bool hasTempDoublePtr = false, rerunOrZeroPass = false;

    std::function<void (Ref node)> andHeapOpts = [&hasTempDoublePtr, &rerunOrZeroPass, &andHeapOpts](Ref node) {
      // Detect trees which should not
      // be simplified.
      Ref type = node[0];
      if (type == "sub" && node[1][0] == "name" && isFunctionTable(node[1][1])) {
        return; // do not traverse subchildren here, we should not collapse 55 & 126.
      }
      traverseChildren(node, andHeapOpts);
      // Simplifications are done now so
      // that we simplify a node's operands before the node itself. This allows
      // optimizations to cascade.
      if (type == "name") {
        if (node[1] == "tempDoublePtr") hasTempDoublePtr = true;
      } else if (type == "binary" && node[1] == "&" && node[3][0] == "num") {
        if (node[2][0] == "num") {
          safeCopy(node, makeNum(int(node[2][1]->getNumber()) & int(node[3][1]->getNumber())));
          return;
        }
        Ref input = node[2];
        double amount = node[3][1]->getNumber();
        if (input[0] == "binary" && input[1] == "&" && input[3][0] == "num") {
          // Collapse X & 255 & 1
          node[3][1]->setNumber(int(amount) & int(input[3][1]->getNumber()));
          node[2] = input[2];
        } else if (input[0] == "sub" && input[1][0] == "name") {
          // HEAP8[..] & 255 => HEAPU8[..]
          HeapInfo hi = parseHeap(input[1][1]->getCString());
          if (hi.valid) {
            if (isInteger32(amount) && amount == powl(2, hi.bits)-1) {
              if (!hi.unsign) {
                input[1][1]->setString("HEAPU" + getIntStr(hi.bits)); // make unsigned
              }
              // we cannot return HEAPU8 without a coercion, but at least we do HEAP8 & 255 => HEAPU8 | 0
              node[1]->setString("|");
              node[3][1]->setNumber(0);
              return;
            }
          }
        }
      } else if (type == "binary" && node[1] == "^") {
        // LLVM represents bitwise not as xor with -1. Translate it back to an actual bitwise not.
        if (node[3][0] == "unary-prefix" && node[3][1] == "-" && node[3][2][0] == "num" &&
            node[3][2][1]->getNumber() == 1 &&
            !(node[2][0] == "unary-prefix" && node[2][1] == "~")) { // avoid creating ~~~ which is confusing for asm given the role of ~~
          safeCopy(node, make2("unary-prefix", "~", node[2]));
          return;
        }
      } else if (type       == "binary" && node[1]    == ">>" && node[3][0]    == "num" &&
                 node[2][0] == "binary" && node[2][1] == "<<" && node[2][3][0] == "num" &&
                 node[2][2][0] == "sub" && node[2][2][1][0] == "name") {
        // collapse HEAPU?8[..] << 24 >> 24 etc. into HEAP8[..] | 0
        double amount = node[3][1]->getNumber();
        if (amount == node[2][3][1]->getNumber()) {
          HeapInfo hi = parseHeap(node[2][2][1][1]->getCString());
          if (hi.valid && hi.bits == 32 - amount) {
            node[2][2][1][1]->setString("HEAP" + getIntStr(hi.bits));
            node[1]->setString("|");
            node[2] = node[2][2];
            node[3][1]->setNumber(0);
            rerunOrZeroPass = true;
            return;
          }
        }
      } else if (type == "assign") {
        // optimizations for assigning into HEAP32 specifically
        if (node[1]->isBool(true) && node[2][0] == "sub" && node[2][1][0] == "name") {
          if (node[2][1][1] == "HEAP32") {
            // HEAP32[..] = x | 0 does not need the | 0 (unless it is a mandatory |0 of a call)
            if (node[3][0] == "binary" && node[3][1] == "|") {
              if (node[3][2][0] == "num" && node[3][2][1]->getNumber() == 0 && node[3][3][0] != "call") {
                node[3] = node[3][3];
              } else if (node[3][3][0] == "num" && node[3][3][1]->getNumber() == 0 && node[3][2][0] != "call") {
                node[3] = node[3][2];
              }
            }
          } else if (node[2][1][1] == "HEAP8") {
            // HEAP8[..] = x & 0xff does not need the & 0xff
            if (node[3][0] == "binary" && node[3][1] == "&" && node[3][3][0] == "num" && node[3][3][1]->getNumber() == 0xff) {
              node[3] = node[3][2];
            }
          } else if (node[2][1][1] == "HEAP16") {
            // HEAP16[..] = x & 0xffff does not need the & 0xffff
            if (node[3][0] == "binary" && node[3][1] == "&" && node[3][3][0] == "num" && node[3][3][1]->getNumber() == 0xffff) {
              node[3] = node[3][2];
            }
          }
        }
        Ref value = node[3];
        if (value[0] == "binary" && value[1] == "|") {
          // canonicalize order of |0 to end
          if (value[2][0] == "num" && value[2][1]->getNumber() == 0) {
            Ref temp = value[2];
            value[2] = value[3];
            value[3] = temp;
          }
          // if a seq ends in an |0, remove an external |0
          // note that it is only safe to do this in assigns, like we are doing here (return (x, y|0); is not valid)
          if (value[2][0] == "seq" && value[2][2][0] == "binary" && USEFUL_BINARY_OPS.has(value[2][2][1])) {
            node[3] = value[2];
          }
        }
      } else if (type == "binary" && node[1] == ">>" && node[2][0] == "num" && node[3][0] == "num") {
        // optimize num >> num, in asm we need this since we do not run optimizeShifts
        node[0]->setString("num");
        node[1]->setNumber(int(node[2][1]->getNumber()) >> int(node[3][1]->getNumber()));
        node->setSize(2);
        return;
      } else if (type == "binary" && node[1] == "+") {
        // The most common mathop is addition, e.g. in getelementptr done repeatedly. We can join all of those,
        // by doing (num+num) ==> newnum, and (name+num)+num = name+newnum
        if (node[2][0] == "num" && node[3][0] == "num") {
          node[2][1]->setNumber(int(node[2][1]->getNumber()) + int(node[3][1]->getNumber()));
          safeCopy(node, node[2]);
          return;
        }
        for (int i = 2; i <= 3; i++) {
          int ii = 5-i;
          for (int j = 2; j <= 3; j++) {
            if (node[i][0] == "num" && node[ii][0] == "binary" && node[ii][1] == "+" && node[ii][j][0] == "num") {
              node[ii][j][1]->setNumber(int(node[ii][j][1]) + int(node[i][1]->getNumber()));
              safeCopy(node, node[ii]);
              return;
            }
          }
        }
      }
    };
    traverseChildren(ast, andHeapOpts);

    if (rerunOrZeroPass) removeMultipleOrZero();

    if (hasTempDoublePtr) {
      AsmData asmData = normalizeAsm(ast);
      traversePre(ast, [](Ref node) {
        Ref type = node[0];
        if (type == "assign") {
          if (node[1]->isBool(true) && node[2][0] == "sub" && node[2][1][0] == "name" && node[2][1][1] == "HEAP32") {
            // remove bitcasts that are now obviously pointless, e.g.
            // HEAP32[$45 >> 2] = HEAPF32[tempDoublePtr >> 2] = ($14 < $28 ? $14 : $28) - $42, HEAP32[tempDoublePtr >> 2] | 0;
            Ref value = node[3];
            if (value[0] == "seq" && value[1][0] == "assign" && value[1][2][0] == "sub" && value[1][2][1][0] == "name" && value[1][2][1][1] == "HEAPF32" &&
                value[1][2][2][0] == "binary" && value[1][2][2][2][0] == "name" && value[1][2][2][2][1] == "tempDoublePtr") {
              // transform to HEAPF32[$45 >> 2] = ($14 < $28 ? $14 : $28) - $42;
              node[2][1][1]->setString("HEAPF32");
              node[3] = value[1][3];
            }
          }
        } else if (type == "seq") {
          // (HEAP32[tempDoublePtr >> 2] = HEAP32[$37 >> 2], +HEAPF32[tempDoublePtr >> 2])
          //   ==>
          // +HEAPF32[$37 >> 2]
          if (node[0] == "seq" && node[1][0] == "assign" && node[1][2][0] == "sub" && node[1][2][1][0] == "name" &&
              (node[1][2][1][1] == "HEAP32" || node[1][2][1][1] == "HEAPF32") &&
              node[1][2][2][0] == "binary" && node[1][2][2][2][0] == "name" && node[1][2][2][2][1] == "tempDoublePtr" &&
              node[1][3][0] == "sub" && node[1][3][1][0] == "name" && (node[1][3][1][1] == "HEAP32" || node[1][3][1][1] == "HEAPF32") &&
              node[2][0] != "seq") { // avoid (x, y, z) which can be used for tempDoublePtr on doubles for alignment fixes
            if (node[1][2][1][1] == "HEAP32") {
              node[1][3][1][1]->setString("HEAPF32");
              safeCopy(node, makeAsmCoercion(node[1][3], detectType(node[2])));
              return;
            } else {
              node[1][3][1][1]->setString("HEAP32");
              safeCopy(node, make3("binary", "|", node[1][3], makeNum(0)));
              return;
            }
          }
        }
      });

      // finally, wipe out remaining ones by finding cases where all assignments to X are bitcasts, and all uses are writes to
      // the other heap type, then eliminate the bitcast
      struct BitcastData {
        int define_HEAP32, define_HEAPF32, use_HEAP32, use_HEAPF32, namings;
        bool ok;
        std::vector<Ref> defines, uses;

        BitcastData() : define_HEAP32(0), define_HEAPF32(0), use_HEAP32(0), use_HEAPF32(0), namings(0), ok(false) {}
      };
      std::map<std::string, BitcastData> bitcastVars;
      traversePre(ast, [&bitcastVars](Ref node) {
        if (node[0] == "assign" && node[1]->isBool(true) && node[2][0] == "name") {
          Ref value = node[3];
          if (value[0] == "seq" && value[1][0] == "assign" && value[1][2][0] == "sub" && value[1][2][1][0] == "name" &&
              (value[1][2][1][1] == "HEAP32" || value[1][2][1][1] == "HEAPF32") &&
              value[1][2][2][0] == "binary" && value[1][2][2][2][0] == "name" && value[1][2][2][2][1] == "tempDoublePtr") {
            std::string name = node[2][1]->getString();
            std::string heap = value[1][2][1][1]->getString();
            if (heap == "HEAP32") {
              bitcastVars[name].define_HEAP32++;
            } else {
              assert(heap == "HEAPF32");
              bitcastVars[name].define_HEAPF32++;
            }
            bitcastVars[name].defines.push_back(node);
            bitcastVars[name].ok = true;
          }
        }
      });
      traversePre(ast, [&bitcastVars](Ref node) {
        Ref type = node[0];
        if (type == "name" && bitcastVars[node[1]->getString()].ok) {
          bitcastVars[node[1]->getString()].namings++;
        } else if (type == "assign" && node[1]->isBool(true)) {
          Ref value = node[3];
          if (value[0] == "name") {
            std::string name = value[1]->getString();
            if (bitcastVars[name].ok) {
              Ref target = node[2];
              if (target[0] == "sub" && target[1][0] == "name" && (target[1][1] == "HEAP32" || target[1][1] == "HEAPF32")) {
                if (target[1][1] == "HEAP32") {
                  bitcastVars[name].use_HEAP32++;
                } else {
                  bitcastVars[name].use_HEAPF32++;
                }
                bitcastVars[name].uses.push_back(node);
              }
            }
          }
        }
      });
      for (auto iter : bitcastVars) {
        const std::string& v = iter.first;
        BitcastData& info = iter.second;
        // good variables define only one type, use only one type, have definitions and uses, and define as a different type than they use
        if (info.define_HEAP32*info.define_HEAPF32 == 0 && info.use_HEAP32*info.use_HEAPF32 == 0 &&
            info.define_HEAP32+info.define_HEAPF32 > 0  && info.use_HEAP32+info.use_HEAPF32 > 0 &&
            info.define_HEAP32*info.use_HEAP32 == 0 && info.define_HEAPF32*info.use_HEAPF32 == 0 &&
            asmData.types.count(v) && info.namings == info.define_HEAP32+info.define_HEAPF32+info.use_HEAP32+info.use_HEAPF32) {
          std::string correct = info.use_HEAP32 ? "HEAPF32" : "HEAP32";
          for (auto define : info.defines) {
            define[3] = define[3][1][3];
            if (correct == "HEAP32") {
              define[3] = make3("binary", "|", define[3], makeNum(0));
            } else {
              define[3] = makeAsmCoercion(define[3], preciseF32 ? ASM_FLOAT : ASM_DOUBLE);
            }
            // do we want a simplifybitops on the new values here?
          }
          for (auto use : info.uses) {
            use[2][1][1]->setString(correct);
          }
          AsmType correctType;
          switch(asmData.types[v]) {
            case ASM_INT: correctType = preciseF32 ? ASM_FLOAT : ASM_DOUBLE; break;
            case ASM_FLOAT: case ASM_DOUBLE: correctType = ASM_INT; break;
            default: {} // pass
          }
          asmData.types[v] = correctType;
        }
      }
      denormalizeAsm(ast, asmData);
    }
  };

  std::function<bool (Ref)> emitsBoolean = [&emitsBoolean](Ref node) {
    std::string& type = node[0]->getString();
    if (type == "num") {
      return node[1]->getNumber() == 0 || node[1] == 1;
    }
    if (type == "binary") return COMPARE_OPS.has(node[1]);
    if (type == "unary-prefix") return node[1] == "!";
    if (type == "conditional") return emitsBoolean(node[2]) && emitsBoolean(node[3]);
    return false;
  };

  //   expensive | expensive can be turned into expensive ? 1 : expensive, and
  //   expensive | cheap     can be turned into cheap     ? 1 : expensive,
  // so that we can avoid the expensive computation, if it has no side effects.
  auto conditionalize = [&emitsBoolean](Ref ast) {
    const int MIN_COST = 7;
    traversePre(ast, [&emitsBoolean](Ref node) {
      if (node[0] == "binary" && (node[1] == "|" || node[1] == "&") && node[3][0] != "num" && node[2][0] != "num") {
        // logical operator on two non-numerical values
        Ref left = node[2];
        Ref right = node[3];
        if (!emitsBoolean(left) || !emitsBoolean(right)) return;
        bool leftEffects = hasSideEffects(left);
        bool rightEffects = hasSideEffects(right);
        if (leftEffects && rightEffects) return; // both must execute
        // canonicalize with side effects, if any, happening on the left
        if (rightEffects) {
          if (measureCost(left) < MIN_COST) return; // avoidable code is too cheap
          Ref temp = left;
          left = right;
          right = temp;
        } else if (leftEffects) {
          if (measureCost(right) < MIN_COST) return; // avoidable code is too cheap
        } else {
          // no side effects, reorder based on cost estimation
          int leftCost = measureCost(left);
          int rightCost = measureCost(right);
          if (std::max(leftCost, rightCost) < MIN_COST) return; // avoidable code is too cheap
          // canonicalize with expensive code on the right
          if (leftCost > rightCost) {
            Ref temp = left;
            left = right;
            right = temp;
          }
        }
        // worth it, perform conditionalization
        Ref ret;
        if (node[1] == "|") {
          ret = make3("conditional", left, makeNum(1), right);
        } else { // &
          ret = make3("conditional", left, right, makeNum(0));
        }
        if (left[0] == "unary-prefix" && left[1] == "!") {
          ret[1] = flipCondition(left);
          Ref temp = ret[2];
          ret[2] = ret[3];
          ret[3] = temp;
        }
        safeCopy(node, ret);
        return;
      }
    });
  };

  traverseFunctions(ast, [&](Ref func) {
    //simplifyIntegerConversions(func);
    simplifyOps(func);
    //simplifyNotComps(func); // XXX broken in JS, should be a traverse, call that, assign on the node if return is different than itself
    //conditionalize(func);
  });
}

void simplifyIfs(Ref ast) {
  traverseFunctions(ast, [](Ref func) {
    bool simplifiedAnElse = false;

    traversePre(func, [&simplifiedAnElse](Ref node) {
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

      traversePre(func, [&labelAssigns, &abort](Ref node) {
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

      traversePre(func, [&labelChecks, &abort](Ref node) {
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
      traversePrePost(func, [&inLoop, &labelAssigns, &labelChecks](Ref node) {
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
      }, [&inLoop](Ref node) {
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
  std::function<void (Ref)> fix = [&](Ref node) {
    bool ret = node[0] == "return";
    if (ret) inReturn = true;
    traverseChildren(node, fix);
    if (ret) inReturn = false;
    if (node[0] == "call" && node[1][0] == "name" && node[1][1] == "Math_fround") {
      Ref arg = node[2][0];
      if (arg[0] == "num") {
        if (!inReturn && arg[1]->getNumber() == 0) *node = *makeName("f0");
      } else if (arg[0] == "call" && arg[1][0] == "name" && arg[1][1] == "Math_fround") {
        safeCopy(node, arg);
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
    else if (str == "simplifyExpressions") simplifyExpressions(doc);
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

