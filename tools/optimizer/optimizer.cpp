#include <cstdint>
#include <cstdio>
#include <cmath>
#include <string>
#include <algorithm>
#include <map>

#include "simple_ast.h"
#include "optimizer.h"

using namespace cashew;

typedef std::vector<IString> StringVec;

//==================
// Globals
//==================

Ref extraInfo;

//==================
// Infrastructure
//==================

template<class T, class V>
int indexOf(T list, V value) {
  for (size_t i = 0; i < list.size(); i++) {
    if (list[i] == value) return i;
  }
  return -1;
}

int jsD2I(double x) {
  return (int)((int64_t)x);
}

char *strdupe(const char *str) {
  char *ret = (char *)malloc(strlen(str)+1); // leaked!
  strcpy(ret, str);
  return ret;
}

IString getHeapStr(int x, bool unsign) {
  switch (x) {
    case 8: return unsign ? HEAPU8 : HEAP8;
    case 16: return unsign ? HEAPU16 : HEAP16;
    case 32: return unsign ? HEAPU32 : HEAP32;
  }
  assert(0);
  return ":(";
}

Ref deStat(Ref node) {
  if (node[0] == STAT) return node[1];
  return node;
}

Ref getStatements(Ref node) {
  if (node[0] == DEFUN) {
    return node[3];
  } else if (node[0] == BLOCK) {
    return node->size() > 1 ? node[1] : nullptr;
  } else {
    return arena.alloc();
  }
}

// Types

AsmType intToAsmType(int type) {
  if (type >= 0 && type <= ASM_NONE) return (AsmType)type;
  else {
    assert(0);
    return ASM_NONE;
  }
}

// forward decls
Ref makeEmpty();
bool isEmpty(Ref node);
Ref makeAsmCoercedZero(AsmType type);
Ref makeArray(int size_hint);
Ref makeBool(bool b);
Ref makeNum(double x);
Ref makeName(IString str);
Ref makeAsmCoercion(Ref node, AsmType type);
Ref make1(IString type, Ref a);
Ref make3(IString type, Ref a, Ref b, Ref c);

AsmData::AsmData(Ref f) {
  func = f;

  // process initial params
  Ref stats = func[3];
  size_t i = 0;
  while (i < stats->size()) {
    Ref node = stats[i];
    if (node[0] != STAT || node[1][0] != ASSIGN || node[1][2][0] != NAME) break;
    node = node[1];
    Ref name = node[2][1];
    int index = func[2]->indexOf(name);
    if (index < 0) break; // not an assign into a parameter, but a global
    IString& str = name->getIString();
    if (locals.count(str) > 0) break; // already done that param, must be starting function body
    locals[str] = Local(detectType(node[3]), true);
    params.push_back(str);
    stats[i] = makeEmpty();
    i++;
  }
  // process initial variable definitions and remove '= 0' etc parts - these
  // are not actually assignments in asm.js
  while (i < stats->size()) {
    Ref node = stats[i];
    if (node[0] != VAR) break;
    for (size_t j = 0; j < node[1]->size(); j++) {
      Ref v = node[1][j];
      IString& name = v[0]->getIString();
      Ref value = v[1];
      if (locals.count(name) == 0) {
        locals[name] = Local(detectType(value, nullptr, true), false);
        vars.push_back(name);
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
    traversePre(stats[i], [&](Ref node) {
      Ref type = node[0];
      if (type == VAR) {
        dump("bad, seeing a var in need of fixing", func);
        abort(); //, 'should be no vars to fix! ' + func[1] + ' : ' + JSON.stringify(node));
      }
    });
    i++;
  }
  // look for final RETURN statement to get return type.
  Ref retStmt = stats->back();
  if (!!retStmt && retStmt[0] == RETURN && !!retStmt[1]) {
    ret = detectType(retStmt[1]);
  } else {
    ret = ASM_NONE;
  }
}

void AsmData::denormalize() {
  Ref stats = func[3];
  // Remove var definitions, if any
  for (size_t i = 0; i < stats->size(); i++) {
    if (stats[i][0] == VAR) {
      stats[i] = makeEmpty();
    } else {
      if (!isEmpty(stats[i])) break;
    }
  }
  // calculate variable definitions
  Ref varDefs = makeArray(vars.size());
  for (auto v : vars) {
    varDefs->push_back(make1(v, makeAsmCoercedZero(locals[v].type)));
  }
  // each param needs a line; reuse emptyNodes as much as we can
  size_t numParams = params.size();
  size_t emptyNodes = 0;
  while (emptyNodes < stats->size()) {
    if (!isEmpty(stats[emptyNodes])) break;
    emptyNodes++;
  }
  size_t neededEmptyNodes = numParams + (varDefs->size() ? 1 : 0); // params plus one big var if there are vars
  if (neededEmptyNodes > emptyNodes) {
    stats->insert(0, neededEmptyNodes - emptyNodes);
  } else if (neededEmptyNodes < emptyNodes) {
    stats->splice(0, emptyNodes - neededEmptyNodes);
  }
  // add param coercions
  int next = 0;
  for (auto param : func[2]->getArray()) {
    IString str = param->getIString();
    assert(locals.count(str) > 0);
    stats[next++] = make1(STAT, make3(ASSIGN, makeBool(true), makeName(str.c_str()), makeAsmCoercion(makeName(str.c_str()), locals[str].type)));
  }
  if (varDefs->size()) {
    stats[next] = make1(VAR, varDefs);
  }
  /*
  if (inlines->size() > 0) {
    var i = 0;
    traverse(func, function(node, type) {
      if (type == CALL && node[1][0] == NAME && node[1][1] == 'inlinejs') {
        node[1] = inlines[i++]; // swap back in the body
      }
    });
  }
  */
  // ensure that there's a final RETURN statement if needed.
  if (ret != ASM_NONE) {
    Ref retStmt = stats->back();
    if (!retStmt || retStmt[0] != RETURN) {
      stats->push_back(make1(RETURN, makeAsmCoercedZero(ret)));
    }
  }
  //printErr('denormalized \n\n' + astToSrc(func) + '\n\n');
}

// Constructions TODO: share common constructions, and assert they remain frozen

Ref makeArray(int size_hint=0) {
  return &arena.alloc()->setArray(size_hint);
}

Ref makeBool(bool b) {
  return &arena.alloc()->setBool(b);
}

Ref makeString(const IString& s) {
  return &arena.alloc()->setString(s);
}

Ref makeEmpty() {
  return ValueBuilder::makeToplevel();
}

Ref makeNum(double x) {
  return ValueBuilder::makeDouble(x);
}

Ref makeName(IString str) {
  return ValueBuilder::makeName(str);
}

Ref makeBlock() {
  return ValueBuilder::makeBlock();
}

Ref make1(IString s1, Ref a) {
  Ref ret(makeArray(2));
  ret->push_back(makeString(s1));
  ret->push_back(a);
  return ret;
}

Ref make2(IString s1, IString s2, Ref a) {
  Ref ret(makeArray(2));
  ret->push_back(makeString(s1));
  ret->push_back(makeString(s2));
  ret->push_back(a);
  return ret;
}

Ref make2(IString s1, Ref a, Ref b) {
  Ref ret(makeArray(3));
  ret->push_back(makeString(s1));
  ret->push_back(a);
  ret->push_back(b);
  return ret;
}

Ref make3(IString type, IString a, Ref b, Ref c) {
  Ref ret(makeArray(4));
  ret->push_back(makeString(type));
  ret->push_back(makeString(a));
  ret->push_back(b);
  ret->push_back(c);
  return ret;
}

Ref make3(IString type, Ref a, Ref b, Ref c) {
  Ref ret(makeArray(4));
  ret->push_back(makeString(type));
  ret->push_back(a);
  ret->push_back(b);
  ret->push_back(c);
  return ret;
}

Ref makeAsmCoercedZero(AsmType type) {
  switch (type) {
    case ASM_INT: return makeNum(0); break;
    case ASM_DOUBLE: return make2(UNARY_PREFIX, PLUS, makeNum(0)); break;
    case ASM_FLOAT: {
      if (!ASM_FLOAT_ZERO.isNull()) {
        return makeName(ASM_FLOAT_ZERO);
      } else {
        return make2(CALL, makeName(MATH_FROUND), &(makeArray(1))->push_back(makeNum(0)));
      }
      break;
    }
    case ASM_FLOAT32X4: {
      return make2(CALL, makeName(SIMD_FLOAT32X4), &(makeArray(4))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_FLOAT64X2: {
      return make2(CALL, makeName(SIMD_FLOAT64X2), &(makeArray(2))->push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_INT8X16: {
      return make2(CALL, makeName(SIMD_INT8X16), &(makeArray(16))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_INT16X8: {
      return make2(CALL, makeName(SIMD_INT16X8), &(makeArray(8))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_INT32X4: {
      return make2(CALL, makeName(SIMD_INT32X4), &(makeArray(4))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_BOOL8X16: {
      return make2(CALL, makeName(SIMD_BOOL8X16), &(makeArray(16))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_BOOL16X8: {
      return make2(CALL, makeName(SIMD_BOOL16X8), &(makeArray(8))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_BOOL32X4: {
      return make2(CALL, makeName(SIMD_BOOL32X4), &(makeArray(4))->push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    case ASM_BOOL64X2: {
      return make2(CALL, makeName(SIMD_BOOL64X2), &(makeArray(2))->push_back(makeNum(0)).push_back(makeNum(0)));
      break;
    }
    default: assert(0);
  }
  abort();
}

Ref makeAsmCoercion(Ref node, AsmType type) {
  switch (type) {
    case ASM_INT: return make3(BINARY, OR, node, makeNum(0));
    case ASM_DOUBLE: return make2(UNARY_PREFIX, PLUS, node);
    case ASM_FLOAT: return make2(CALL, makeName(MATH_FROUND), &(makeArray(1))->push_back(node));
    case ASM_FLOAT32X4: return make2(CALL, makeName(SIMD_FLOAT32X4_CHECK), &(makeArray(1))->push_back(node));
    case ASM_FLOAT64X2: return make2(CALL, makeName(SIMD_FLOAT64X2_CHECK), &(makeArray(1))->push_back(node));
    case ASM_INT8X16: return make2(CALL, makeName(SIMD_INT8X16_CHECK), &(makeArray(1))->push_back(node));
    case ASM_INT16X8: return make2(CALL, makeName(SIMD_INT16X8_CHECK), &(makeArray(1))->push_back(node));
    case ASM_INT32X4: return make2(CALL, makeName(SIMD_INT32X4_CHECK), &(makeArray(1))->push_back(node));
    case ASM_NONE:
    default: return node; // non-validating code, emit nothing XXX this is dangerous, we should only allow this when we know we are not validating
  }
}

// Checks

bool isEmpty(Ref node) {
  return (node->size() == 2 && node[0] == TOPLEVEL && node[1]->size() == 0) ||
          (node->size() > 0 && node[0] == BLOCK && (!node[1] || node[1]->size() == 0));
}

bool commable(Ref node) { // TODO: hashing
  IString type = node[0]->getIString();
  if (type == ASSIGN || type == BINARY || type == UNARY_PREFIX || type == NAME || type == NUM || type == CALL || type == SEQ || type == CONDITIONAL || type == SUB) return true;
  return false;
}

bool isMathFunc(const char *name) {
  static const char *Math_ = "Math_";
  static unsigned size = strlen(Math_);
  return strncmp(name, Math_, size) == 0;
}

bool isMathFunc(Ref value) {
  return value->isString() && isMathFunc(value->getCString());
}

bool callHasSideEffects(Ref node) { // checks if the call itself (not the args) has side effects (or is not statically known)
  return !(node[1][0] == NAME && isMathFunc(node[1][1]));
}

bool hasSideEffects(Ref node) { // this is 99% incomplete!
  IString type = node[0]->getIString();
  switch (type[0]) {
    case 'n':
      if (type == NUM || type == NAME) return false;
      break;
    case 's':
      if (type == STRING) return false;
      if (type == SUB) return hasSideEffects(node[1]) || hasSideEffects(node[2]);
      break;
    case 'u':
      if (type == UNARY_PREFIX) return hasSideEffects(node[2]);
      break;
    case 'b':
      if (type == BINARY) return hasSideEffects(node[2]) || hasSideEffects(node[3]);
      break;
    case 'c':
      if (type == CALL) {
        if (callHasSideEffects(node)) return true;
        // This is a statically known call, with no side effects. only args can side effect us
        for (auto arg : node[2]->getArray()) {
          if (hasSideEffects(arg)) return true;
        }
        return false;
      } else if (type == CONDITIONAL) return hasSideEffects(node[1]) || hasSideEffects(node[2]) || hasSideEffects(node[3]); 
      break;
  }
  return true;
}

// checks if a node has just basic operations, nothing with side effects nor that can notice side effects, which
// implies we can move it around in the code
bool triviallySafeToMove(Ref node, AsmData& asmData) {
  bool ok = true;
  traversePre(node, [&](Ref node) {
    Ref type = node[0];
    if (type == STAT || type == BINARY || type == UNARY_PREFIX || type == ASSIGN || type == NUM) return;
    else if (type == NAME) {
      if (!asmData.isLocal(node[1]->getIString())) ok = false;
    } else if (type == CALL) {
      if (callHasSideEffects(node)) ok = false;
    } else {
      ok = false;
    }  
  });
  return ok;
}

// Transforms

// We often have branchings that are simplified so one end vanishes, and
// we then get
//   if (!(x < 5))
// or such. Simplifying these saves space and time.
Ref simplifyNotCompsDirect(Ref node) {
  if (node[0] == UNARY_PREFIX && node[1] == L_NOT) {
    // de-morgan's laws do not work on floats, due to nans >:(
    if (node[2][0] == BINARY && (detectType(node[2][2]) == ASM_INT && detectType(node[2][3]) == ASM_INT)) {
      Ref op = node[2][1];
      switch(op->getCString()[0]) {
        case '<': {
          if (op == LT)  { op->setString(GE); break; }
          if (op == LE) { op->setString(GT); break; }
          return node;
        }
        case '>': {
          if (op == GT)  { op->setString(LE); break; }
          if (op == GE) { op->setString(LT); break; }
          return node;
        }
        case '=': {
          if (op == EQ) { op->setString(NE); break; }
          return node;
        }
        case '!': {
          if (op == NE) { op->setString(EQ); break; }
          return node;
        }
        default: return node;
      }
      return make3(BINARY, op, node[2][2], node[2][3]);
    } else if (node[2][0] == UNARY_PREFIX && node[2][1] == L_NOT) {
      return node[2][2];
    }
  }
  return node;
}

Ref flipCondition(Ref cond) {
  return simplifyNotCompsDirect(make2(UNARY_PREFIX, L_NOT, cond));
}

void safeCopy(Ref target, Ref source) { // safely copy source onto target, even if source is a subnode of target
  Ref temp = source; // hold on to source
  *target = *temp;
}

void clearEmptyNodes(Ref arr) {
  int skip = 0;
  for (size_t i = 0; i < arr->size(); i++) {
    if (skip) {
      arr[i-skip] = arr[i];
    }
    if (isEmpty(deStat(arr[i]))) {
      skip++;
    }
  }
  if (skip) arr->setSize(arr->size() - skip);
}
void clearUselessNodes(Ref arr) {
  int skip = 0;
  for (size_t i = 0; i < arr->size(); i++) {
    Ref curr = arr[i];
    if (skip) {
      arr[i-skip] = curr;
    }
    if (isEmpty(deStat(curr)) || (curr[0] == STAT && !hasSideEffects(curr[1]))) {
      skip++;
    }
  }
  if (skip) arr->setSize(arr->size() - skip);
}

void removeAllEmptySubNodes(Ref ast) {
  traversePre(ast, [](Ref node) {
    if (node[0] == DEFUN) {
      clearEmptyNodes(node[3]);
    } else if (node[0] == BLOCK && node->size() > 1 && !!node[1]) {
      clearEmptyNodes(node[1]);
    } else if (node[0] == SEQ && isEmpty(node[1])) {
      safeCopy(node, node[2]);
    }
  });
}
void removeAllUselessSubNodes(Ref ast) {
  traversePrePost(ast, [](Ref node) {
    Ref type = node[0];
    if (type == DEFUN) {
      clearUselessNodes(node[3]);
    } else if (type == BLOCK && node->size() > 1 && !!node[1]) {
      clearUselessNodes(node[1]);
    } else if (type == SEQ && isEmpty(node[1])) {
      safeCopy(node, node[2]);
    }
  }, [](Ref node) {
    Ref type = node[0];
    if (type == IF) {
      bool empty2 = isEmpty(node[2]), has3 = node->size() == 4 && !!node[3], empty3 = !has3 || isEmpty(node[3]);
      if (!empty2 && empty3 && has3) { // empty else clauses
        node->setSize(3);
      } else if (empty2 && !empty3) { // empty if blocks
        safeCopy(node, make2(IF, make2(UNARY_PREFIX, L_NOT, node[1]), node[3]));
      } else if (empty2 && empty3) {
        if (hasSideEffects(node[1])) {
          safeCopy(node, make1(STAT, node[1]));
        } else {
          safeCopy(node, makeEmpty());
        }
      }
    }
  });
}

Ref unVarify(Ref vars) { // transform var x=1, y=2 etc. into (x=1, y=2), i.e., the same assigns, but without a var definition
  Ref ret = makeArray(1);
  ret->push_back(makeString(STAT));
  if (vars->size() == 1) {
    ret->push_back(make3(ASSIGN, makeBool(true), makeName(vars[0][0]->getIString()), vars[0][1]));
  } else {
    ret->push_back(makeArray(vars->size()-1));
    Ref curr = ret[1];
    for (size_t i = 0; i+1 < vars->size(); i++) {
      curr->push_back(makeString(SEQ));
      curr->push_back(make3(ASSIGN, makeBool(true), makeName(vars[i][0]->getIString()), vars[i][1]));
      if (i != vars->size()-2) {
        curr->push_back(makeArray());
        curr = curr[2];
      }
    }
    curr->push_back(make3(ASSIGN, makeBool(true), makeName(vars->back()[0]->getIString()), vars->back()[1]));
  }
  return ret;
}

// Calculations

int measureCost(Ref ast) {
  int size = 0;
  traversePre(ast, [&size](Ref node) {
    Ref type = node[0];
    if (type == NUM || type == UNARY_PREFIX) size--;
    else if (type == BINARY) {
      if (node[3][0] == NUM && node[3][1]->getNumber() == 0) size--;
      else if (node[1] == DIV || node[1] == MOD) size += 2;
    }
    else if (type == CALL && !callHasSideEffects(node)) size -= 2;
    else if (type == SUB) size++;
    size++;
  });
  return size;
}

//==================
// Params
//==================

bool preciseF32 = false,
     receiveJSON = false,
     emitJSON = false,
     minifyWhitespace = false,
     last = false;

//=====================
// Optimization passes
//=====================

#define HASES \
  bool has(const IString& str) { \
    return count(str) > 0; \
  } \
  bool has(Ref node) { \
    return node->isString() && count(node->getIString()) > 0; \
  }

class StringSet : public cashew::IStringSet {
public:
  StringSet() {}
  StringSet(const char *str) : IStringSet(str) {}

  HASES

  void dump() {
    err("===");
    for (auto str : *this) {
      errv("%s", str.c_str());
    }
    err("===");
  }
};

StringSet USEFUL_BINARY_OPS("<< >> | & ^"),
          COMPARE_OPS("< <= > >= == == != !=="),
          BITWISE("| & ^"),
          SAFE_BINARY_OPS("+ -"), // division is unsafe as it creates non-ints in JS; mod is unsafe as signs matter so we can't remove |0's; mul does not nest with +,- in asm
          COERCION_REQUIRING_OPS("sub unary-prefix"), // ops that in asm must be coerced right away
          COERCION_REQUIRING_BINARIES("* / %"); // binary ops that in asm must be coerced

StringSet ASSOCIATIVE_BINARIES("+ * | & ^"),
          CONTROL_FLOW("do while for if switch"),
          LOOP("do while for"),
          NAME_OR_NUM("name num"),
          CONDITION_CHECKERS("if do while switch"),
          BOOLEAN_RECEIVERS("if do while conditional"),
          SAFE_TO_DROP_COERCION("unary-prefix name num");

StringSet BREAK_CAPTURERS("do while for switch"),
          CONTINUE_CAPTURERS("do while for"),
          FUNCTIONS_THAT_ALWAYS_THROW("abort ___resumeException ___cxa_throw ___cxa_rethrow");

bool isFunctionTable(const char *name) {
  static const char *functionTable = "FUNCTION_TABLE";
  static unsigned size = strlen(functionTable);
  return strncmp(name, functionTable, size) == 0;
}

bool isFunctionTable(Ref value) {
  return value->isString() && isFunctionTable(value->getCString());
}

// Internal utilities

bool canDropCoercion(Ref node) {
  if (SAFE_TO_DROP_COERCION.has(node[0])) return true;
  if (node[0] == BINARY) {
    switch (node[1]->getCString()[0]) {
      case '>': return node[1] == RSHIFT || node[1] == TRSHIFT;
      case '<': return node[1] == LSHIFT;
      case '|': case '^': case '&': return true;
    }
  }
  return false;
}

Ref simplifyCondition(Ref node) {
  node = simplifyNotCompsDirect(node);
  // on integers, if (x == 0) is the same as if (x), and if (x != 0) as if (!x)
  if (node[0] == BINARY && (node[1] == EQ || node[1] == NE)) {
    Ref target;
    if (detectType(node[2]) == ASM_INT && node[3][0] == NUM && node[3][1]->getNumber() == 0) {
      target = node[2];
    } else if (detectType(node[3]) == ASM_INT && node[2][0] == NUM && node[2][1]->getNumber() == 0) {
      target = node[3];
    }
    if (!!target) {
      if (target[0] == BINARY && (target[1] == OR || target[1] == TRSHIFT) && target[3][0] == NUM && target[3][1]->getNumber() == 0 &&
          canDropCoercion(target[2])) {
        target = target[2]; // drop the coercion, in a condition it is ok to do if (x)
      }
      if (node[1] == EQ) {
        return make2(UNARY_PREFIX, L_NOT, target);
      } else {
        return target;
      }
    }
  }
  return node;
}

// Passes

// Eliminator aka Expressionizer
//
// The goal of this pass is to eliminate unneeded variables (which represent one of the infinite registers in the LLVM
// model) and thus to generate complex expressions where possible, for example
//
//  var x = a(10);
//  var y = HEAP[20];
//  print(x+y);
//
// can be transformed into
//
//  print(a(10)+HEAP[20]);
//
// The basic principle is to scan along the code in the order of parsing/execution, and keep a list of tracked
// variables that are current contenders for elimination. We must untrack when we see something that we cannot
// cross, for example, a write to memory means we must invalidate variables that depend on reading from
// memory, since if we change the order then we do not preserve the computation.
//
// We rely on some assumptions about emscripten-generated code here, which means we can do a lot more than
// a general JS optimization can. For example, we assume that SUB nodes (indexing like HEAP[..]) are
// memory accesses or FUNCTION_TABLE accesses, and in both cases that the symbol cannot be replaced although
// the contents can. So we assume FUNCTION_TABLE might have its contents changed but not be pointed to
// a different object, which allows
//
//  var x = f();
//  FUNCTION_TABLE[x]();
//
// to be optimized (f could replace FUNCTION_TABLE, so in general JS eliminating x is not valid).
//
// In memSafe mode, we are more careful and assume functions can replace HEAP and FUNCTION_TABLE, which
// can happen in ALLOW_MEMORY_GROWTH mode

StringSet ELIMINATION_SAFE_NODES("assign call if toplevel do return label switch binary unary-prefix"); // do is checked carefully, however
StringSet IGNORABLE_ELIMINATOR_SCAN_NODES("num toplevel string break continue dot"); // dot can only be STRING_TABLE.*
StringSet ABORTING_ELIMINATOR_SCAN_NODES("new object function defun for while array throw"); // we could handle some of these, TODO, but nontrivial (e.g. for while, the condition is hit multiple times after the body)
StringSet HEAP_NAMES("HEAP8 HEAP16 HEAP32 HEAPU8 HEAPU16 HEAPU32 HEAPF32 HEAPF64");

bool isTempDoublePtrAccess(Ref node) { // these are used in bitcasts; they are not really affecting memory, and should cause no invalidation
  assert(node[0] == SUB);
  return (node[2][0] == NAME && node[2][1] == TEMP_DOUBLE_PTR) ||
         (node[2][0] == BINARY && ((node[2][2][0] == NAME && node[2][2][1] == TEMP_DOUBLE_PTR) ||
                                     (node[2][3][0] == NAME && node[2][3][1] == TEMP_DOUBLE_PTR)));
}

class StringIntMap : public std::unordered_map<IString, int> {
public:
  HASES
};

class StringStringMap : public std::unordered_map<IString, IString> {
public:
  HASES
};

class StringRefMap : public std::unordered_map<IString, Ref> {
public:
  HASES
};

class StringTypeMap : public std::unordered_map<IString, AsmType> {
public:
  HASES
};

void eliminate(Ref ast, bool memSafe) {
#ifdef PROFILING
  clock_t tasmdata = 0;
  clock_t tfnexamine = 0;
  clock_t tvarcheck = 0;
  clock_t tstmtelim = 0;
  clock_t tstmtscan = 0;
  clock_t tcleanvars = 0;
  clock_t treconstruct = 0;
#endif

  // Find variables that have a single use, and if they can be eliminated, do so
  traverseFunctions(ast, [&](Ref func) {

#ifdef PROFILING
    clock_t start = clock();
#endif

    AsmData asmData(func);

#ifdef PROFILING
    tasmdata += clock() - start;
    start = clock();
#endif

    // First, find the potentially eliminatable functions: that have one definition and one use

    StringIntMap definitions;
    StringIntMap uses;
    StringIntMap namings;
    StringRefMap values;
    StringIntMap varsToRemove; // variables being removed, that we can eliminate all 'var x;' of (this refers to VAR nodes we should remove)
                            // 1 means we should remove it, 2 means we successfully removed it
    StringSet varsToTryToRemove; // variables that have 0 uses, but have side effects - when we scan we can try to remove them

    // examine body and note locals
    traversePre(func, [&](Ref node) {
      Ref type = node[0];
      if (type == NAME) {
        IString& name = node[1]->getIString();
        uses[name]++;
        namings[name]++;
      } else if (type == ASSIGN) {
        Ref target = node[2];
        if (target[0] == NAME) {
          IString& name = target[1]->getIString();
          // values is only used if definitions is 1
          if (definitions[name]++ == 0) {
            values[name] = node[3];
          }
          assert(node[1]->isBool(true)); // not +=, -= etc., just =
          uses[name]--; // because the name node will show up by itself in the previous case
        }
      }
    });

#ifdef PROFILING
    tfnexamine += clock() - start;
    start = clock();
#endif

    StringSet potentials; // local variables with 1 definition and 1 use
    StringSet sideEffectFree; // whether a local variable has no side effects in its definition. Only relevant when there are no uses

    auto unprocessVariable = [&](IString name) {
      potentials.erase(name);
      varsToRemove.erase(name);
      sideEffectFree.erase(name);
      varsToTryToRemove.erase(name);
    };
    std::function<void (IString)> processVariable = [&](IString name) {
      if (definitions[name] == 1 && uses[name] == 1) {
        potentials.insert(name);
      } else if (uses[name] == 0 && definitions[name] <= 1) { // no uses, no def or 1 def (cannot operate on phis, and the llvm optimizer will remove unneeded phis anyhow) (no definition means it is a function parameter, or a local with just |var x;| but no defining assignment)
        bool sideEffects = false;
        auto val = values.find(name);
        Ref value;
        if (val != values.end()) {
          value = val->second;
          // TODO: merge with other side effect code
          // First, pattern-match
          //  (HEAP32[((tempDoublePtr)>>2)]=((HEAP32[(($_sroa_0_0__idx1)>>2)])|0),HEAP32[(((tempDoublePtr)+(4))>>2)]=((HEAP32[((($_sroa_0_0__idx1)+(4))>>2)])|0),(+(HEAPF64[(tempDoublePtr)>>3])))
          // which has no side effects and is the special form of converting double to i64.
          if (!(value[0] == SEQ && value[1][0] == ASSIGN && value[1][2][0] == SUB && value[1][2][2][0] == BINARY && value[1][2][2][1] == RSHIFT &&
                value[1][2][2][2][0] == NAME && value[1][2][2][2][1] == TEMP_DOUBLE_PTR)) {
            // If not that, then traverse and scan normally.
            sideEffects = hasSideEffects(value);
          }
        }
        if (!sideEffects) {
          varsToRemove[name] = !definitions[name] ? 2 : 1; // remove it normally
          sideEffectFree.insert(name);
          // Each time we remove a variable with 0 uses, if its value has no
          // side effects and vanishes too, then we can remove a use from variables
          // appearing in it, and possibly eliminate again
          if (!!value) {
            traversePre(value, [&](Ref node) {
              if (node[0] == NAME) {
                IString name = node[1]->getIString();
                node[1]->setString(EMPTY); // we can remove this - it will never be shown, and should not be left to confuse us as we traverse
                if (asmData.isLocal(name)) {
                  uses[name]--; // cannot be infinite recursion since we descend an energy function
                  assert(uses[name] >= 0);
                  unprocessVariable(name);
                  processVariable(name);
                }
              } else if (node[0] == CALL) {
                // no side effects, so this must be a Math.* call or such. We can just ignore it and all children
                node[0]->setString(NAME);
                node[1]->setString(EMPTY);
              }
            });
          }
        } else {
          varsToTryToRemove.insert(name); // try to remove it later during scanning
        }
      }
    };
    for (auto name : asmData.locals) {
      processVariable(name.first);
    }

#ifdef PROFILING
    tvarcheck += clock() - start;
    start = clock();
#endif

    //printErr('defs: ' + JSON.stringify(definitions));
    //printErr('uses: ' + JSON.stringify(uses));
    //printErr('values: ' + JSON.stringify(values));
    //printErr('locals: ' + JSON.stringify(locals));
    //printErr('varsToRemove: ' + JSON.stringify(varsToRemove));
    //printErr('varsToTryToRemove: ' + JSON.stringify(varsToTryToRemove));
    values.clear();
    //printErr('potentials: ' + JSON.stringify(potentials));
    // We can now proceed through the function. In each list of statements, we try to eliminate
    struct Tracking {
      bool usesGlobals, usesMemory, hasDeps;
      Ref defNode;
      bool doesCall;
    };
    class Tracked : public std::unordered_map<IString, Tracking> {
    public:
      HASES
    };
    Tracked tracked;
    #define dumpTracked() { errv("tracking %d", tracked.size()); for (auto t : tracked) errv("... %s", t.first.c_str()); }
    // Although a set would be more appropriate, it would also be slower
    std::unordered_map<IString, StringVec> depMap;

    bool globalsInvalidated = false; // do not repeat invalidations, until we track something new
    bool memoryInvalidated = false;
    bool callsInvalidated = false;
    auto track = [&](IString name, Ref value, Ref defNode) { // add a potential that has just been defined to the tracked list, we hope to eliminate it
      Tracking& track = tracked[name];
      track.usesGlobals = false;
      track.usesMemory = false;
      track.hasDeps = false;
      track.defNode = defNode;
      track.doesCall = false;
      bool ignoreName = false; // one-time ignorings of names, as first op in sub and call
      traversePre(value, [&](Ref node) {
        Ref type = node[0];
        if (type == NAME) {
          if (!ignoreName) {
            IString depName = node[1]->getIString();
            if (!asmData.isLocal(depName)) {
              track.usesGlobals = true;
            }
            if (!potentials.has(depName)) { // deps do not matter for potentials - they are defined once, so no complexity
              depMap[depName].push_back(name);
              track.hasDeps = true;
            }
          } else {
            ignoreName = false;
          }
        } else if (type == SUB) {
          track.usesMemory = true;
          ignoreName = true;
        } else if (type == CALL) {
          track.usesGlobals = true;
          track.usesMemory = true;
          track.doesCall = true;
          ignoreName = true;
        } else {
          ignoreName = false;
        }
      });
      if (track.usesGlobals) globalsInvalidated = false;
      if (track.usesMemory) memoryInvalidated = false;
      if (track.doesCall) callsInvalidated = false;
    };

    // TODO: invalidate using a sequence number for each type (if you were tracked before the last invalidation, you are cancelled). remove for.in loops
    #define INVALIDATE(what, check) \
    auto invalidate##what = [&]() { \
      std::vector<IString> temp; \
      for (auto t : tracked) { \
        IString name = t.first; \
        Tracking& info = tracked[name]; \
        if (check) { \
          temp.push_back(name); \
        } \
      } \
      for (size_t i = 0; i < temp.size(); i++) { \
        tracked.erase(temp[i]); \
      } \
    };
    INVALIDATE(Globals, info.usesGlobals);
    INVALIDATE(Memory, info.usesMemory);
    INVALIDATE(Calls, info.doesCall);

    auto invalidateByDep = [&](IString dep) {
      for (auto name : depMap[dep]) {
        tracked.erase(name);
      }
      depMap.erase(dep);
    };

    std::function<void (IString name, Ref node)> doEliminate;

    // Generate the sequence of execution. This determines what is executed before what, so we know what can be reordered. Using
    // that, performs invalidations and eliminations
    auto scan = [&](Ref node) {
      bool abort = false;
      bool allowTracking = true; // false inside an if; also prevents recursing in an if
      std::function<void (Ref, bool)> traverseInOrder = [&](Ref node, bool ignoreSub) {
        if (abort) return;
        Ref type = node[0];
        if (type == ASSIGN) {
          Ref target = node[2];
          Ref value = node[3];
          bool nameTarget = target[0] == NAME;
          // If this is an assign to a name, handle it below rather than
          // traversing and treating as a read
          if (!nameTarget) {
            traverseInOrder(target, true); // evaluate left
          }
          traverseInOrder(value,  false); // evaluate right
          // do the actual assignment
          if (nameTarget) {
            IString name = target[1]->getIString();
            if (potentials.has(name) && allowTracking) {
              track(name, node[3], node);
            } else if (varsToTryToRemove.has(name)) {
              // replace it in-place
              safeCopy(node, value);
              varsToRemove[name] = 2;
            } else {
              // expensive check for invalidating specific tracked vars. This list is generally quite short though, because of
              // how we just eliminate in short spans and abort when control flow happens TODO: history numbers instead
              invalidateByDep(name); // can happen more than once per dep..
              if (!asmData.isLocal(name) && !globalsInvalidated) {
                invalidateGlobals();
                globalsInvalidated = true;
              }
              // if we can track this name (that we assign into), and it has 0 uses and we want to remove its VAR
              // definition - then remove it right now, there is no later chance
              if (allowTracking && varsToRemove.has(name) && uses[name] == 0) {
                track(name, node[3], node);
                doEliminate(name, node);
              }
            }
          } else if (target[0] == SUB) {
            if (isTempDoublePtrAccess(target)) {
              if (!globalsInvalidated) {
                invalidateGlobals();
                globalsInvalidated = true;
              }
            } else if (!memoryInvalidated) {
              invalidateMemory();
              memoryInvalidated = true;
            }
          }
        } else if (type == SUB) {
          // Only keep track of the global array names in memsafe mode i.e.
          // when they may change underneath us due to resizing
          if (node[1][0] != NAME || memSafe) {
            traverseInOrder(node[1], false); // evaluate inner
          }
          traverseInOrder(node[2], false); // evaluate outer
          // ignoreSub means we are a write (happening later), not a read
          if (!ignoreSub && !isTempDoublePtrAccess(node)) {
            // do the memory access
            if (!callsInvalidated) {
              invalidateCalls();
              callsInvalidated = true;
            }
          }
        } else if (type == BINARY) {
          bool flipped = false;
          if (ASSOCIATIVE_BINARIES.has(node[1]) && !NAME_OR_NUM.has(node[2][0]) && NAME_OR_NUM.has(node[3][0])) { // TODO recurse here?
            // associatives like + and * can be reordered in the simple case of one of the sides being a name, since we assume they are all just numbers
            Ref temp = node[2];
            node[2] = node[3];
            node[3] = temp;
            flipped = true;
          }
          traverseInOrder(node[2], false);
          traverseInOrder(node[3], false);
          if (flipped && NAME_OR_NUM.has(node[2][0])) { // dunno if we optimized, but safe to flip back - and keeps the code closer to the original and more readable
            Ref temp = node[2];
            node[2] = node[3];
            node[3] = temp;
          }
        } else if (type == NAME) {
          IString name = node[1]->getIString();
          if (tracked.has(name)) {
            doEliminate(name, node);
          } else if (!asmData.isLocal(name) && !callsInvalidated && (memSafe || !HEAP_NAMES.has(name))) { // ignore HEAP8 etc when not memory safe, these are ok to
                                                                                                          // access, e.g. SIMD_Int32x4_load(HEAP8, ...)
            invalidateCalls();
            callsInvalidated = true;
          }
        } else if (type == UNARY_PREFIX || type == UNARY_POSTFIX) {
          traverseInOrder(node[2], false);
        } else if (IGNORABLE_ELIMINATOR_SCAN_NODES.has(type)) {
        } else if (type == CALL) {
          // Named functions never change and are therefore safe to not track
          if (node[1][0] != NAME) {
            traverseInOrder(node[1], false);
          }
          Ref args = node[2];
          for (size_t i = 0; i < args->size(); i++) {
            traverseInOrder(args[i], false);
          }
          if (callHasSideEffects(node)) {
            // these two invalidations will also invalidate calls
            if (!globalsInvalidated) {
              invalidateGlobals();
              globalsInvalidated = true;
            }
            if (!memoryInvalidated) {
              invalidateMemory();
              memoryInvalidated = true;
            }
          }
        } else if (type == IF) {
          if (allowTracking) {
            traverseInOrder(node[1], false); // can eliminate into condition, but nowhere else
            if (!callsInvalidated) { // invalidate calls, since we cannot eliminate them into an if that may not execute!
              invalidateCalls();
              callsInvalidated = true;
            }
            allowTracking = false;
            traverseInOrder(node[2], false); // 2 and 3 could be 'parallel', really..
            if (!!node[3]) traverseInOrder(node[3], false);
            allowTracking = true;
          } else {
            tracked.clear();
          }
        } else if (type == BLOCK) {
          Ref stats = getStatements(node);
          if (!!stats) {
            for (size_t i = 0; i < stats->size(); i++) {
              traverseInOrder(stats[i], false);
            }
          }
        } else if (type == STAT) {
          traverseInOrder(node[1], false);
        } else if (type == LABEL) {
          traverseInOrder(node[2], false);
        } else if (type == SEQ) {
          traverseInOrder(node[1], false);
          traverseInOrder(node[2], false);
        } else if (type == DO) {
          if (node[1][0] == NUM && node[1][1]->getNumber() == 0) { // one-time loop
            traverseInOrder(node[2], false);
          } else {
            tracked.clear();
          }
        } else if (type == RETURN) {
          if (!!node[1]) traverseInOrder(node[1], false);
        } else if (type == CONDITIONAL) {
          if (!callsInvalidated) { // invalidate calls, since we cannot eliminate them into a branch of an LLVM select/JS conditional that does not execute
            invalidateCalls();
            callsInvalidated = true;
          }
          traverseInOrder(node[1], false);
          traverseInOrder(node[2], false);
          traverseInOrder(node[3], false);
        } else if (type == SWITCH) {
          traverseInOrder(node[1], false);
          Tracked originalTracked = tracked;
          Ref cases = node[2];
          for (size_t i = 0; i < cases->size(); i++) {
            Ref c = cases[i];
            assert(c[0]->isNull() || c[0][0] == NUM || (c[0][0] == UNARY_PREFIX && c[0][2][0] == NUM));
            Ref stats = c[1];
            for (size_t j = 0; j < stats->size(); j++) {
              traverseInOrder(stats[j], false);
            }
            // We cannot track from one switch case into another if there are external dependencies, undo all new trackings
            // Otherwise we can track, e.g. a var used in a case before assignment in another case is UB in asm.js, so no need for the assignment
            // TODO: general framework here, use in if-else as well
            std::vector<IString> toDelete;
            for (auto t : tracked) {
              if (!originalTracked.has(t.first)) {
                Tracking& info = tracked[t.first];
                if (info.usesGlobals || info.usesMemory || info.hasDeps) {
                  toDelete.push_back(t.first);
                }
              }
            }
            for (auto t : toDelete) {
              tracked.erase(t);
            }
          }
          tracked.clear(); // do not track from inside the switch to outside
        } else {
          assert(ABORTING_ELIMINATOR_SCAN_NODES.has(type));
          tracked.clear();
          abort = true;
        }
      };
      traverseInOrder(node, false);
    };
    //var eliminationLimit = 0; // used to debugging purposes
    doEliminate = [&](IString name, Ref node) {
      //if (eliminationLimit == 0) return;
      //eliminationLimit--;
      //printErr('elim!!!!! ' + name);
      // yes, eliminate!
      varsToRemove[name] = 2; // both assign and var definitions can have other vars we must clean up
      assert(tracked.has(name));
      Tracking& info = tracked[name];
      Ref defNode = info.defNode;
      assert(!!defNode);
      if (!sideEffectFree.has(name)) {
        assert(defNode[0] != VAR);
        // assign
        Ref value = defNode[3];
        // wipe out the assign
        safeCopy(defNode, makeEmpty());
        // replace this node in-place
        safeCopy(node, value);
      } else {
        // This has no side effects and no uses, empty it out in-place
        safeCopy(node, makeEmpty());
      }
      tracked.erase(name);
    };
    traversePre(func, [&](Ref block) {
      // Look for statements, including while-switch pattern
      Ref stats = getStatements(block);
      if (!stats && (block[0] == WHILE && block[2][0] == SWITCH)) {
        stats = &(makeArray(1)->push_back(block[2]));
      }
      if (!stats) return;
      tracked.clear();
      for (size_t i = 0; i < stats->size(); i++) {
        Ref node = deStat(stats[i]);
        Ref type = node[0];
        if (type == RETURN && i+1 < stats->size()) {
          stats->setSize(i+1); // remove any code after a return
        }
        // Check for things that affect elimination
        if (ELIMINATION_SAFE_NODES.has(type)) {
#ifdef PROFILING
          tstmtelim += clock() - start;
          start = clock();
#endif
          scan(node);
#ifdef PROFILING
          tstmtscan += clock() - start;
          start = clock();
#endif
        } else if (type == VAR) {
          continue; // asm normalisation has reduced 'var' to just the names
        } else {
          tracked.clear(); // not a var or assign, break all potential elimination so far
        }
      }
    });

#ifdef PROFILING
    tstmtelim += clock() - start;
    start = clock();
#endif

    StringIntMap seenUses;
    StringStringMap helperReplacements; // for looper-helper optimization

    // clean up vars, and loop variable elimination
    traversePrePost(func, [&](Ref node) {
      // pre
      Ref type = node[0];
      /*if (type == VAR) {
        node[1] = node[1].filter(function(pair) { return !varsToRemove[pair[0]] });
        if (node[1]->size() == 0) {
          // wipe out an empty |var;|
          node[0] = TOPLEVEL;
          node[1] = [];
        }
      } else */
      if (type == ASSIGN && node[1]->isBool(true) && node[2][0] == NAME && node[3][0] == NAME && node[2][1] == node[3][1]) {
        // elimination led to X = X, which we can just remove
        safeCopy(node, makeEmpty());
      }
    }, [&](Ref node) {
      // post
      Ref type = node[0];
      if (type == NAME) {
        IString name = node[1]->getIString();
        if (helperReplacements.has(name)) {
          node[1]->setString(helperReplacements[name]);
          return; // no need to track this anymore, we can't loop-optimize more than once
        }
        // track how many uses we saw. we need to know when a variable is no longer used (hence we run this in the post)
        seenUses[name]++;
      } else if (type == WHILE) {
        // try to remove loop helper variables specifically
        Ref stats = node[2][1];
        Ref last = stats->back();
        if (!!last && last[0] == IF && last[2][0] == BLOCK && !!last[3] && last[3][0] == BLOCK) {
          Ref ifTrue = last[2];
          Ref ifFalse = last[3];
          clearEmptyNodes(ifTrue[1]);
          clearEmptyNodes(ifFalse[1]);
          bool flip = false;
          if (ifFalse[1]->size() > 0 && !!ifFalse[1][0] && !!ifFalse[1]->back() && ifFalse[1]->back()[0] == BREAK) { // canonicalize break in the if-true
            Ref temp = ifFalse;
            ifFalse = ifTrue;
            ifTrue = temp;
            flip = true;
          }
          if (ifTrue[1]->size() > 0 && !!ifTrue[1][0] && !!ifTrue[1]->back() && ifTrue[1]->back()[0] == BREAK) {
            Ref assigns = ifFalse[1];
            clearEmptyNodes(assigns);
            std::vector<IString> loopers, helpers;
            for (size_t i = 0; i < assigns->size(); i++) {
              if (assigns[i][0] == STAT && assigns[i][1][0] == ASSIGN) {
                Ref assign = assigns[i][1];
                if (assign[1]->isBool(true) && assign[2][0] == NAME && assign[3][0] == NAME) {
                  IString looper = assign[2][1]->getIString();
                  IString helper = assign[3][1]->getIString();
                  if (definitions[helper] == 1 && seenUses[looper] == namings[looper] &&
                      !helperReplacements.has(helper) && !helperReplacements.has(looper)) {
                    loopers.push_back(looper);
                    helpers.push_back(helper);
                  }
                }
              }
            }
            // remove loop vars that are used in the rest of the else
            for (size_t i = 0; i < assigns->size(); i++) {
              if (assigns[i][0] == STAT && assigns[i][1][0] == ASSIGN) {
                Ref assign = assigns[i][1];
                if (!(assign[1]->isBool(true) && assign[2][0] == NAME && assign[3][0] == NAME) || indexOf(loopers, assign[2][1]->getIString()) < 0) {
                  // this is not one of the loop assigns
                  traversePre(assign, [&](Ref node) {
                    if (node[0] == NAME) {
                      int index = indexOf(loopers, node[1]->getIString());
                      if (index < 0) index = indexOf(helpers, node[1]->getIString());
                      if (index >= 0) {
                        loopers.erase(loopers.begin() + index);
                        helpers.erase(helpers.begin() + index);
                      }
                    }
                  });
                }
              }
            }
            // remove loop vars that are used in the if
            traversePre(ifTrue, [&](Ref node) {
              if (node[0] == NAME) {
                int index = indexOf(loopers, node[1]->getIString());
                if (index < 0) index = indexOf(helpers, node[1]->getIString());
                if (index >= 0) {
                  loopers.erase(loopers.begin() + index);
                  helpers.erase(helpers.begin() + index);
                }
              }
            });
            if (loopers.size() == 0) return;
            for (size_t l = 0; l < loopers.size(); l++) {
              IString looper = loopers[l];
              IString helper = helpers[l];
              // the remaining issue is whether loopers are used after the assignment to helper and before the last line (where we assign to it)
              int found = -1;
              for (int i = (int)stats->size()-2; i >= 0; i--) {
                Ref curr = stats[i];
                if (curr[0] == STAT && curr[1][0] == ASSIGN) {
                  Ref currAssign = curr[1];
                  if (currAssign[1]->isBool(true) && currAssign[2][0] == NAME) {
                    IString to = currAssign[2][1]->getIString();
                    if (to == helper) {
                      found = i;
                      break;
                    }
                  }
                }
              }
              if (found < 0) return;
              // if a loop variable is used after we assigned to the helper, we must save its value and use that.
              // (note that this can happen due to elimination, if we eliminate an expression containing the
              // loop var far down, past the assignment!)
              // first, see if the looper and helpers overlap. Note that we check for this looper, compared to
              // *ALL* the helpers. Helpers will be replaced by loopers as we eliminate them, potentially
              // causing conflicts, so any helper is a concern.
              int firstLooperUsage = -1;
              int lastLooperUsage = -1;
              int firstHelperUsage = -1;
              for (int i = found+1; i < (int)stats->size(); i++) {
                Ref curr = i < (int)stats->size()-1 ? stats[i] : last[1]; // on the last line, just look in the condition
                traversePre(curr, [&](Ref node) {
                  if (node[0] == NAME) {
                    if (node[1] == looper) {
                      if (firstLooperUsage < 0) firstLooperUsage = i;
                      lastLooperUsage = i;
                    } else if (indexOf(helpers, node[1]->getIString()) >= 0) {
                      if (firstHelperUsage < 0) firstHelperUsage = i;
                    }
                  }
                });
              }
              if (firstLooperUsage >= 0) {
                // the looper is used, we cannot simply merge the two variables
                if ((firstHelperUsage < 0 || firstHelperUsage > lastLooperUsage) && lastLooperUsage+1 < (int)stats->size() && triviallySafeToMove(stats[found], asmData) &&
                    seenUses[helper] == namings[helper]) {
                  // the helper is not used, or it is used after the last use of the looper, so they do not overlap,
                  // and the last looper usage is not on the last line (where we could not append after it), and the
                  // helper is not used outside of the loop.
                  // just move the looper definition to after the looper's last use
                  stats->insert(lastLooperUsage+1, stats[found]);
                  stats->splice(found, 1);
                } else {
                  // they overlap, we can still proceed with the loop optimization, but we must introduce a
                  // loop temp helper variable
                  IString temp(strdupe((std::string(looper.c_str()) + "$looptemp").c_str()));
                  assert(!asmData.isLocal(temp));
                  for (int i = firstLooperUsage; i <= lastLooperUsage; i++) {
                    Ref curr = i < (int)stats->size()-1 ? stats[i] : last[1]; // on the last line, just look in the condition

                    std::function<bool (Ref)> looperToLooptemp = [&](Ref node) {
                      if (node[0] == NAME) {
                        if (node[1] == looper) {
                          node[1]->setString(temp);
                        }
                      } else if (node[0] == ASSIGN && node[2][0] == NAME) {
                        // do not traverse the assignment target, phi assignments to the loop variable must remain
                        traversePrePostConditional(node[3], looperToLooptemp, [](Ref node){});
                        return false;
                      }
                      return true;
                    };
                    traversePrePostConditional(curr, looperToLooptemp, [](Ref node){});
                  }
                  asmData.addVar(temp, asmData.getType(looper));
                  stats->insert(found, make1(STAT, make3(ASSIGN, makeBool(true), makeName(temp), makeName(looper))));
                }
              }
            }
            for (size_t l = 0; l < helpers.size(); l++) {
              for (size_t k = 0; k < helpers.size(); k++) {
                if (l != k && helpers[l] == helpers[k]) return; // it is complicated to handle a shared helper, abort
              }
            }
            // hurrah! this is safe to do
            for (size_t l = 0; l < loopers.size(); l++) {
              IString looper = loopers[l];
              IString helper = helpers[l];
              varsToRemove[helper] = 2;
              traversePre(node, [&](Ref node) { // replace all appearances of helper with looper
                if (node[0] == NAME && node[1] == helper) node[1]->setString(looper);
              });
              helperReplacements[helper] = looper; // replace all future appearances of helper with looper
              helperReplacements[looper] = looper; // avoid any further attempts to optimize looper in this manner (seenUses is wrong anyhow, too)
            }
            // simplify the if. we remove the if branch, leaving only the else
            if (flip) {
              last[1] = simplifyNotCompsDirect(make2(UNARY_PREFIX, L_NOT, last[1]));
              Ref temp = last[2];
              last[2] = last[3];
              last[3] = temp;
            }
            if (loopers.size() == assigns->size()) {
              last->pop_back();
            } else {
              Ref elseStats = getStatements(last[3]);
              for (size_t i = 0; i < elseStats->size(); i++) {
                Ref stat = deStat(elseStats[i]);
                if (stat[0] == ASSIGN && stat[2][0] == NAME) {
                  if (indexOf(loopers, stat[2][1]->getIString()) >= 0) {
                    elseStats[i] = makeEmpty();
                  }
                }
              }
            }
          }
        }
      }
    });

#ifdef PROFILING
    tcleanvars += clock() - start;
    start = clock();
#endif

    for (auto v : varsToRemove) {
      if (v.second == 2 && asmData.isVar(v.first)) asmData.deleteVar(v.first);
    }

    asmData.denormalize();

#ifdef PROFILING
    treconstruct += clock() - start;
    start = clock();
#endif

  });

  removeAllEmptySubNodes(ast);

#ifdef PROFILING
  errv("    EL stages: a:%li fe:%li vc:%li se:%li (ss:%li) cv:%li r:%li",
    tasmdata, tfnexamine, tvarcheck, tstmtelim, tstmtscan, tcleanvars, treconstruct);
#endif
}

void eliminateMemSafe(Ref ast) {
  eliminate(ast, true);
}

void simplifyExpressions(Ref ast) {
  // Simplify common expressions used to perform integer conversion operations
  // in cases where no conversion is needed.
  auto simplifyIntegerConversions = [](Ref ast) {
    traversePre(ast, [](Ref node) {
      Ref type = node[0];
      if (type == BINARY       && node[1]    == RSHIFT && node[3][0] == NUM &&
          node[2][0] == BINARY && node[2][1] == LSHIFT && node[2][3][0] == NUM && node[3][1]->getNumber() == node[2][3][1]->getNumber()) {
        // Transform (x&A)<<B>>B to X&A.
        Ref innerNode = node[2][2];
        double shifts = node[3][1]->getNumber();
        if (innerNode[0] == BINARY && innerNode[1] == AND && innerNode[3][0] == NUM) {
          double mask = innerNode[3][1]->getNumber();
          if (isInteger32(mask) && isInteger32(shifts) && ((jsD2I(mask) << jsD2I(shifts)) >> jsD2I(shifts)) == jsD2I(mask)) {
            safeCopy(node, innerNode);
            return;
          }
        }
      } else if (type == BINARY && BITWISE.has(node[1])) {
        for (int i = 2; i <= 3; i++) {
          Ref subNode = node[i];
          if (subNode[0] == BINARY && subNode[1] == AND && subNode[3][0] == NUM && subNode[3][1]->getNumber() == 1) {
            // Rewrite (X < Y) & 1 to X < Y , when it is going into a bitwise operator. We could
            // remove even more (just replace &1 with |0, then subsequent passes could remove the |0)
            // but v8 issue #2513 means the code would then run very slowly in chrome.
            Ref input = subNode[2];
            if (input[0] == BINARY && COMPARE_OPS.has(input[1])) {
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
          if (type == BINARY && node[1] == OR) {
            if (node[2][0] == NUM && node[3][0] == NUM) {
              node[2][1]->setNumber(jsD2I(node[2][1]->getNumber()) | jsD2I(node[3][1]->getNumber()));
              stack.push_back(0);
              safeCopy(node, node[2]);
              return;
            }
            bool go = false;
            if (node[2][0] == NUM && node[2][1]->getNumber() == 0) {
              // canonicalize order
              Ref temp = node[3];
              node[3] = node[2];
              node[2] = temp;
              go = true;
            } else if (node[3][0] == NUM && node[3][1]->getNumber() == 0) {
              go = true;
            }
            if (!go) {
              stack.push_back(1);
              return;
            }
            // We might be able to remove this correction
            for (int i = stack.size()-1; i >= 0; i--) {
              if (stack[i] >= 1) {
                if (stack.back() < 2 && node[2][0] == CALL) break; // we can only remove multiple |0s on these
                if (stack.back() < 1 && (COERCION_REQUIRING_OPS.has(node[2][0]) ||
                                                 (node[2][0] == BINARY && COERCION_REQUIRING_BINARIES.has(node[2][1])))) break; // we can remove |0 or >>2
                // we will replace ourselves with the non-zero side. Recursively process that node.
                Ref result = node[2][0] == NUM && node[2][1]->getNumber() == 0 ? node[3] : node[2], other;
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
          } else if (type == BINARY && USEFUL_BINARY_OPS.has(node[1])) {
            stack.push_back(1);
          } else if ((type == BINARY && SAFE_BINARY_OPS.has(node[1])) || type == NUM || type == NAME) {
            stack.push_back(0); // This node is safe in that it does not interfere with this optimization
          } else if (type == UNARY_PREFIX && node[1] == B_NOT) {
            stack.push_back(1);
          } else {
            stack.push_back(-1); // This node is dangerous! Give up if you see this before you see '1'
          }
        };

        traversePrePost(ast, process, [&stack](Ref node) {
          assert(!stack.empty());
          stack.pop_back();
        });
      }
    };

    removeMultipleOrZero();

    // & and heap-related optimizations

    bool hasTempDoublePtr = false, rerunOrZeroPass = false;

    traversePrePostConditional(ast, [](Ref node) {
      // Detect trees which should not
      // be simplified.
      if (node[0] == SUB && node[1][0] == NAME && isFunctionTable(node[1][1])) {
        return false; // do not traverse subchildren here, we should not collapse 55 & 126.
      }
      return true;
    }, [&hasTempDoublePtr, &rerunOrZeroPass](Ref node) {
      // Simplifications are done now so
      // that we simplify a node's operands before the node itself. This allows
      // optimizations to cascade.
      Ref type = node[0];
      if (type == NAME) {
        if (node[1] == TEMP_DOUBLE_PTR) hasTempDoublePtr = true;
      } else if (type == BINARY && node[1] == AND && node[3][0] == NUM) {
        if (node[2][0] == NUM) {
          safeCopy(node, makeNum(jsD2I(node[2][1]->getNumber()) & jsD2I(node[3][1]->getNumber())));
          return;
        }
        Ref input = node[2];
        double amount = node[3][1]->getNumber();
        if (input[0] == BINARY && input[1] == AND && input[3][0] == NUM) {
          // Collapse X & 255 & 1
          node[3][1]->setNumber(jsD2I(amount) & jsD2I(input[3][1]->getNumber()));
          node[2] = input[2];
        } else if (input[0] == SUB && input[1][0] == NAME) {
          // HEAP8[..] & 255 => HEAPU8[..]
          HeapInfo hi = parseHeap(input[1][1]->getCString());
          if (hi.valid) {
            if (isInteger32(amount) && amount == powl(2, hi.bits)-1) {
              if (!hi.unsign) {
                input[1][1]->setString(getHeapStr(hi.bits, true)); // make unsigned
              }
              // we cannot return HEAPU8 without a coercion, but at least we do HEAP8 & 255 => HEAPU8 | 0
              node[1]->setString(OR);
              node[3][1]->setNumber(0);
              return;
            }
          }
        } else if (input[0] == BINARY && input[1] == RSHIFT &&
                   input[2][0] == BINARY && input[2][1] == LSHIFT &&
                   input[2][3][0] == NUM && input[3][0] == NUM &&
                   input[2][3][1]->getInteger() == input[3][1]->getInteger() &&
                   (~(0xFFFFFFFFu >> input[3][1]->getInteger()) & jsD2I(amount)) == 0) {
            // x << 24 >> 24 & 255 => x & 255
            return safeCopy(node, make3(BINARY, AND, input[2][2], node[3]));
        }
      } else if (type == BINARY && node[1] == XOR) {
        // LLVM represents bitwise not as xor with -1. Translate it back to an actual bitwise not.
        if (node[3][0] == UNARY_PREFIX && node[3][1] == MINUS && node[3][2][0] == NUM &&
            node[3][2][1]->getNumber() == 1 &&
            !(node[2][0] == UNARY_PREFIX && node[2][1] == B_NOT)) { // avoid creating ~~~ which is confusing for asm given the role of ~~
          safeCopy(node, make2(UNARY_PREFIX, B_NOT, node[2]));
          return;
        }
      } else if (type       == BINARY && node[1]    == RSHIFT && node[3][0]    == NUM &&
                 node[2][0] == BINARY && node[2][1] == LSHIFT && node[2][3][0] == NUM &&
                 node[2][2][0] == SUB && node[2][2][1][0] == NAME) {
        // collapse HEAPU?8[..] << 24 >> 24 etc. into HEAP8[..] | 0
        double amount = node[3][1]->getNumber();
        if (amount == node[2][3][1]->getNumber()) {
          HeapInfo hi = parseHeap(node[2][2][1][1]->getCString());
          if (hi.valid && hi.bits == 32 - amount) {
            node[2][2][1][1]->setString(getHeapStr(hi.bits, false));
            node[1]->setString(OR);
            node[2] = node[2][2];
            node[3][1]->setNumber(0);
            rerunOrZeroPass = true;
            return;
          }
        }
      } else if (type == ASSIGN) {
        // optimizations for assigning into HEAP32 specifically
        if (node[1]->isBool(true) && node[2][0] == SUB && node[2][1][0] == NAME) {
          if (node[2][1][1] == HEAP32) {
            // HEAP32[..] = x | 0 does not need the | 0 (unless it is a mandatory |0 of a call)
            if (node[3][0] == BINARY && node[3][1] == OR) {
              if (node[3][2][0] == NUM && node[3][2][1]->getNumber() == 0 && node[3][3][0] != CALL) {
                node[3] = node[3][3];
              } else if (node[3][3][0] == NUM && node[3][3][1]->getNumber() == 0 && node[3][2][0] != CALL) {
                node[3] = node[3][2];
              }
            }
          } else if (node[2][1][1] == HEAP8) {
            // HEAP8[..] = x & 0xff does not need the & 0xff
            if (node[3][0] == BINARY && node[3][1] == AND && node[3][3][0] == NUM && node[3][3][1]->getNumber() == 0xff) {
              node[3] = node[3][2];
            }
          } else if (node[2][1][1] == HEAP16) {
            // HEAP16[..] = x & 0xffff does not need the & 0xffff
            if (node[3][0] == BINARY && node[3][1] == AND && node[3][3][0] == NUM && node[3][3][1]->getNumber() == 0xffff) {
              node[3] = node[3][2];
            }
          }
        }
        Ref value = node[3];
        if (value[0] == BINARY && value[1] == OR) {
          // canonicalize order of |0 to end
          if (value[2][0] == NUM && value[2][1]->getNumber() == 0) {
            Ref temp = value[2];
            value[2] = value[3];
            value[3] = temp;
          }
          // if a seq ends in an |0, remove an external |0
          // note that it is only safe to do this in assigns, like we are doing here (return (x, y|0); is not valid)
          if (value[2][0] == SEQ && value[2][2][0] == BINARY && USEFUL_BINARY_OPS.has(value[2][2][1])) {
            node[3] = value[2];
          }
        }
      } else if (type == BINARY && node[1] == RSHIFT && node[2][0] == NUM && node[3][0] == NUM) {
        // optimize num >> num, in asm we need this since we do not optimize shifts in asm.js
        node[0]->setString(NUM);
        node[1]->setNumber(jsD2I(node[2][1]->getNumber()) >> jsD2I(node[3][1]->getNumber()));
        node->setSize(2);
        return;
      } else if (type == BINARY && node[1] == PLUS) {
        // The most common mathop is addition, e.g. in getelementptr done repeatedly. We can join all of those,
        // by doing (num+num) ==> newnum.
        if (node[2][0] == NUM && node[3][0] == NUM) {
          node[2][1]->setNumber(jsD2I(node[2][1]->getNumber()) + jsD2I(node[3][1]->getNumber()));
          safeCopy(node, node[2]);
          return;
        }
      }
    });

    if (rerunOrZeroPass) removeMultipleOrZero();

    if (hasTempDoublePtr) {
      AsmData asmData(ast);
      traversePre(ast, [](Ref node) {
        Ref type = node[0];
        if (type == ASSIGN) {
          if (node[1]->isBool(true) && node[2][0] == SUB && node[2][1][0] == NAME && node[2][1][1] == HEAP32) {
            // remove bitcasts that are now obviously pointless, e.g.
            // HEAP32[$45 >> 2] = HEAPF32[tempDoublePtr >> 2] = ($14 < $28 ? $14 : $28) - $42, HEAP32[tempDoublePtr >> 2] | 0;
            Ref value = node[3];
            if (value[0] == SEQ && value[1][0] == ASSIGN && value[1][2][0] == SUB && value[1][2][1][0] == NAME && value[1][2][1][1] == HEAPF32 &&
                value[1][2][2][0] == BINARY && value[1][2][2][2][0] == NAME && value[1][2][2][2][1] == TEMP_DOUBLE_PTR) {
              // transform to HEAPF32[$45 >> 2] = ($14 < $28 ? $14 : $28) - $42;
              node[2][1][1]->setString(HEAPF32);
              node[3] = value[1][3];
            }
          }
        } else if (type == SEQ) {
          // (HEAP32[tempDoublePtr >> 2] = HEAP32[$37 >> 2], +HEAPF32[tempDoublePtr >> 2])
          //   ==>
          // +HEAPF32[$37 >> 2]
          if (node[0] == SEQ && node[1][0] == ASSIGN && node[1][2][0] == SUB && node[1][2][1][0] == NAME &&
              (node[1][2][1][1] == HEAP32 || node[1][2][1][1] == HEAPF32) &&
              node[1][2][2][0] == BINARY && node[1][2][2][2][0] == NAME && node[1][2][2][2][1] == TEMP_DOUBLE_PTR &&
              node[1][3][0] == SUB && node[1][3][1][0] == NAME && (node[1][3][1][1] == HEAP32 || node[1][3][1][1] == HEAPF32) &&
              node[2][0] != SEQ) { // avoid (x, y, z) which can be used for tempDoublePtr on doubles for alignment fixes
            if (node[1][2][1][1] == HEAP32) {
              node[1][3][1][1]->setString(HEAPF32);
              safeCopy(node, makeAsmCoercion(node[1][3], detectType(node[2])));
              return;
            } else {
              node[1][3][1][1]->setString(HEAP32);
              safeCopy(node, make3(BINARY, OR, node[1][3], makeNum(0)));
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
      std::unordered_map<IString, BitcastData> bitcastVars;
      traversePre(ast, [&bitcastVars](Ref node) {
        if (node[0] == ASSIGN && node[1]->isBool(true) && node[2][0] == NAME) {
          Ref value = node[3];
          if (value[0] == SEQ && value[1][0] == ASSIGN && value[1][2][0] == SUB && value[1][2][1][0] == NAME &&
              (value[1][2][1][1] == HEAP32 || value[1][2][1][1] == HEAPF32) &&
              value[1][2][2][0] == BINARY && value[1][2][2][2][0] == NAME && value[1][2][2][2][1] == TEMP_DOUBLE_PTR) {
            IString name = node[2][1]->getIString();
            IString heap = value[1][2][1][1]->getIString();
            if (heap == HEAP32) {
              bitcastVars[name].define_HEAP32++;
            } else {
              assert(heap == HEAPF32);
              bitcastVars[name].define_HEAPF32++;
            }
            bitcastVars[name].defines.push_back(node);
            bitcastVars[name].ok = true;
          }
        }
      });
      traversePre(ast, [&bitcastVars](Ref node) {
        Ref type = node[0];
        if (type == NAME && bitcastVars[node[1]->getCString()].ok) {
          bitcastVars[node[1]->getCString()].namings++;
        } else if (type == ASSIGN && node[1]->isBool(true)) {
          Ref value = node[3];
          if (value[0] == NAME) {
            IString name = value[1]->getIString();
            if (bitcastVars[name].ok) {
              Ref target = node[2];
              if (target[0] == SUB && target[1][0] == NAME && (target[1][1] == HEAP32 || target[1][1] == HEAPF32)) {
                if (target[1][1] == HEAP32) {
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
        const IString& v = iter.first;
        BitcastData& info = iter.second;
        // good variables define only one type, use only one type, have definitions and uses, and define as a different type than they use
        if (info.define_HEAP32*info.define_HEAPF32 == 0 && info.use_HEAP32*info.use_HEAPF32 == 0 &&
            info.define_HEAP32+info.define_HEAPF32 > 0  && info.use_HEAP32+info.use_HEAPF32 > 0 &&
            info.define_HEAP32*info.use_HEAP32 == 0 && info.define_HEAPF32*info.use_HEAPF32 == 0 &&
            asmData.isLocal(v.c_str()) && info.namings == info.define_HEAP32+info.define_HEAPF32+info.use_HEAP32+info.use_HEAPF32) {
          IString& correct = info.use_HEAP32 ? HEAPF32 : HEAP32;
          for (auto define : info.defines) {
            define[3] = define[3][1][3];
            if (correct == HEAP32) {
              define[3] = make3(BINARY, OR, define[3], makeNum(0));
            } else {
              assert(correct == HEAPF32);
              define[3] = makeAsmCoercion(define[3], preciseF32 ? ASM_FLOAT : ASM_DOUBLE);
            }
            // do we want a simplifybitops on the new values here?
          }
          for (auto use : info.uses) {
            use[2][1][1]->setString(correct.c_str());
          }
          AsmType correctType;
          switch(asmData.getType(v.c_str())) {
            case ASM_INT: correctType = preciseF32 ? ASM_FLOAT : ASM_DOUBLE; break;
            case ASM_FLOAT: case ASM_DOUBLE: correctType = ASM_INT; break;
            default: assert(0);
          }
          asmData.setType(v.c_str(), correctType);
        }
      }
      asmData.denormalize();
    }
  };

  std::function<bool (Ref)> emitsBoolean = [&emitsBoolean](Ref node) {
    Ref type = node[0];
    if (type == NUM) {
      return node[1]->getNumber() == 0 || node[1]->getNumber() == 1;
    }
    if (type == BINARY) return COMPARE_OPS.has(node[1]);
    if (type == UNARY_PREFIX) return node[1] == L_NOT;
    if (type == CONDITIONAL) return emitsBoolean(node[2]) && emitsBoolean(node[3]);
    return false;
  };

  //   expensive | expensive can be turned into expensive ? 1 : expensive, and
  //   expensive | cheap     can be turned into cheap     ? 1 : expensive,
  // so that we can avoid the expensive computation, if it has no side effects.
  auto conditionalize = [&emitsBoolean](Ref ast) {
    traversePre(ast, [&emitsBoolean](Ref node) {
        const int MIN_COST = 7;
        if (node[0] == BINARY && (node[1] == OR || node[1] == AND) && node[3][0] != NUM && node[2][0] != NUM) {
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
        if (node[1] == OR) {
          ret = make3(CONDITIONAL, left, makeNum(1), right);
        } else { // &
          ret = make3(CONDITIONAL, left, right, makeNum(0));
        }
        if (left[0] == UNARY_PREFIX && left[1] == L_NOT) {
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

  auto simplifyNotZero = [](Ref ast) {
    traversePre(ast, [](Ref node) {
      if (BOOLEAN_RECEIVERS.has(node[0])) {
        auto boolean = node[1];
        if (boolean[0] == BINARY && boolean[1] == NE && boolean[3][0] == NUM && boolean[3][1]->getNumber() == 0) {
          node[1] = boolean[2];
        }
      }
    });
  };

  traverseFunctions(ast, [&](Ref func) {
    simplifyIntegerConversions(func);
    simplifyOps(func);
    traversePre(func, [](Ref node) {
      Ref ret = simplifyNotCompsDirect(node);
      if (ret.get() != node.get()) { // if we received a different pointer in return, then we need to copy the new value
        safeCopy(node, ret);
      }
    });
    conditionalize(func);
    simplifyNotZero(func);
  });
}

void simplifyIfs(Ref ast) {
  traverseFunctions(ast, [](Ref func) {
    bool simplifiedAnElse = false;

    traversePre(func, [&simplifiedAnElse](Ref node) {
      // simplify   if (x) { if (y) { .. } }   to   if (x ? y : 0) { .. }
      if (node[0] == IF) {
        Ref body = node[2];
        // recurse to handle chains
        while (body[0] == BLOCK) {
          Ref stats = body[1];
          if (stats->size() == 0) break;
          Ref other = stats->back();
          if (other[0] != IF) {
            // our if block does not end with an if. perhaps if have an else we can flip
            if (node->size() > 3 && !!node[3] && node[3][0] == BLOCK) {
              stats = node[3][1];
              if (stats->size() == 0) break;
              other = stats->back();
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
            for (size_t i = 0; i+1 < stats->size(); i++) {
              Ref curr = deStat(stats[i]);
              if (!commable(curr)) ok = false;
            }
            if (!ok) break;
            for (int i = stats->size()-2; i >= 0; i--) {
              Ref curr = deStat(stats[i]);
              other[1] = make2(SEQ, curr, other[1]);
            }
            Ref temp = makeArray(1);
            temp->push_back(other);
            stats = body[1] = temp;
          }
          if (stats->size() != 1) break;
          if (!!node[3]) simplifiedAnElse = true;
          node[1] = make3(CONDITIONAL, node[1], other[1], makeNum(0));
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
        if (node[0] == ASSIGN && node[2][0] == NAME && node[2][1] == LABEL) {
          if (node[3][0] == NUM) {
            int value = node[3][1]->getInteger();
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
        if (node[0] == BINARY && node[1] == EQ && node[2][0] == BINARY && node[2][1] == OR &&
            node[2][2][0] == NAME && node[2][2][1] == LABEL) {
          if (node[3][0] == NUM) {
            int value = node[3][1]->getInteger();
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
        if (node[0] == WHILE) inLoop++;
        Ref stats = getStatements(node);
        if (!!stats && stats->size() > 0) {
          for (int i = 0; i < (int)stats->size()-1; i++) {
            Ref pre = stats[i];
            Ref post = stats[i+1];
            if (pre[0] == IF && pre->size() > 3 && !!pre[3] && post[0] == IF && (post->size() <= 3 || !post[3])) {
              Ref postCond = post[1];
              if (postCond[0] == BINARY && postCond[1] == EQ &&
                  postCond[2][0] == BINARY && postCond[2][1] == OR &&
                  postCond[2][2][0] == NAME && postCond[2][2][1] == LABEL &&
                  postCond[2][3][0] == NUM && postCond[2][3][1]->getNumber() == 0 &&
                  postCond[3][0] == NUM) {
                int postValue = postCond[3][1]->getInteger();
                Ref preElse = pre[3];
                if (labelAssigns[postValue] == 1 && labelChecks[postValue] == 1 && preElse[0] == BLOCK && preElse->size() >= 2 && preElse[1]->size() == 1) {
                  Ref preStat = preElse[1][0];
                  if (preStat[0] == STAT && preStat[1][0] == ASSIGN &&
                      preStat[1][1]->isBool(true) && preStat[1][2][0] == NAME && preStat[1][2][1] == LABEL &&
                      preStat[1][3][0] == NUM && preStat[1][3][1]->getNumber() == postValue) {
                    // Conditions match, just need to make sure the post clears label
                    if (post[2][0] == BLOCK && post[2]->size() >= 2 && post[2][1]->size() > 0) {
                      Ref postStat = post[2][1][0];
                      bool haveClear =
                        postStat[0] == STAT && postStat[1][0] == ASSIGN &&
                        postStat[1][1]->isBool(true) && postStat[1][2][0] == NAME && postStat[1][2][1] == LABEL &&
                        postStat[1][3][0] == NUM && postStat[1][3][1]->getNumber() == 0;
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
        if (node[0] == WHILE) inLoop--;
      });
      assert(inLoop == 0);
    }
  });
}

void optimizeFrounds(Ref ast) {
  // collapse fround(fround(..)), which can happen due to elimination
  // also emit f0 instead of fround(0) (except in returns)
  int inReturn = 0;
  traversePrePost(ast, [&](Ref node) {
    if (node[0] == RETURN) {
      inReturn++;
    }
  }, [&](Ref node) {
    if (node[0] == RETURN) {
      inReturn--;
    }
    if (node[0] == CALL && node[1][0] == NAME && node[1][1] == MATH_FROUND) {
      Ref arg = node[2][0];
      if (arg[0] == NUM) {
        if (!inReturn && arg[1]->getInteger() == 0) {
          safeCopy(node, makeName(F0));
        }
      } else if (arg[0] == CALL && arg[1][0] == NAME && arg[1][1] == MATH_FROUND) {
        safeCopy(node, arg);
      }
    }
  });
}

// Very simple 'registerization', coalescing of variables into a smaller number.

const char* getRegPrefix(AsmType type) {
  switch (type) {
    case ASM_INT:       return "i"; break;
    case ASM_DOUBLE:    return "d"; break;
    case ASM_FLOAT:     return "f"; break;
    case ASM_FLOAT32X4: return "F4"; break;
    case ASM_FLOAT64X2: return "F2"; break;
    case ASM_INT8X16:   return "I16"; break;
    case ASM_INT16X8:   return "I8"; break;
    case ASM_INT32X4:   return "I4"; break;
    case ASM_BOOL8X16:  return "B16"; break;
    case ASM_BOOL16X8:  return "B8"; break;
    case ASM_BOOL32X4:  return "B4"; break;
    case ASM_BOOL64X2:  return "B2"; break;
    case ASM_NONE:      return "Z"; break;
    default: assert(0); // type doesn't have a name yet
  }
  return nullptr;
}

IString getRegName(AsmType type, int num) {
  const char* str = getRegPrefix(type);
  const int size = 256;
  char temp[size];
  int written = sprintf(temp, "%s%d", str, num);
  assert(written < size);
  temp[written] = 0;
  IString ret;
  ret.set(temp, false);
  return ret;
}

void registerize(Ref ast) {
  traverseFunctions(ast, [](Ref fun) {
    AsmData asmData(fun);
    // Add parameters as a first (fake) var (with assignment), so they get taken into consideration
    // note: params are special, they can never share a register between them (see later)
    Ref fake;
    if (!!fun[2] && fun[2]->size()) {
      Ref assign = makeNum(0);
      // TODO: will be an isEmpty here, can reuse it.
      fun[3]->insert(0, make1(VAR, fun[2]->map([&assign](Ref param) {
        return &(makeArray(2)->push_back(param).push_back(assign));
      })));
    }
    // Replace all var definitions with assignments; we will add var definitions at the top after we registerize
    StringSet allVars;
    traversePre(fun, [&](Ref node) {
      Ref type = node[0];
      if (type == VAR) {
        Ref vars = node[1]->filter([](Ref varr) { return varr->size() > 1; });
        if (vars->size() >= 1) {
          safeCopy(node, unVarify(vars));
        } else {
          safeCopy(node, makeEmpty());
        }
      } else if (type == NAME) {
        allVars.insert(node[1]->getIString());
      }
    });
    removeAllUselessSubNodes(fun); // vacuum?
    StringTypeMap regTypes; // reg name -> type
    auto getNewRegName = [&](int num, IString name) {
      AsmType type = asmData.getType(name);
      IString ret = getRegName(type, num);
      assert(!allVars.has(ret) || asmData.isLocal(ret)); // register must not shadow non-local name
      regTypes[ret] = type;
      return ret;
    };
    // Find the # of uses of each variable.
    // While doing so, check if all a variable's uses are dominated in a simple
    // way by a simple assign, if so, then we can assign its register to it
    // just for its definition to its last use, and not to the entire toplevel loop,
    // we call such variables "optimizable"
    StringIntMap varUses;
    int level = 1;
    std::unordered_map<int, StringSet> levelDominations; // level => set of dominated variables XXX vector?
    StringIntMap varLevels;
    StringSet possibles;
    StringSet unoptimizables;
    auto purgeLevel = [&]() {
      // Invalidate all dominating on this level, further users make it unoptimizable
      for (auto name : levelDominations[level]) {
        varLevels[name] = 0;
      }
      levelDominations[level].clear();
      level--;
    };
    std::function<bool (Ref node)> possibilifier = [&](Ref node) {
      Ref type = node[0];
      if (type == NAME) {
        IString name = node[1]->getIString();
        if (asmData.isLocal(name)) {
          varUses[name]++;
          if (possibles.has(name) && !varLevels[name]) unoptimizables.insert(name); // used outside of simple domination
        }
      } else if (type == ASSIGN && node[1]->isBool(true)) {
        if (!!node[2] && node[2][0] == NAME) {
          IString name = node[2][1]->getIString();
          // if local and not yet used, this might be optimizable if we dominate
          // all other uses
          if (asmData.isLocal(name) && !varUses[name] && !varLevels[name]) {
            possibles.insert(name);
            varLevels[name] = level;
            levelDominations[level].insert(name);
          }
        }
      } else if (CONTROL_FLOW.has(type)) {
        // recurse children, in the context of a loop
        if (type == WHILE || type == DO) {
          traversePrePostConditional(node[1], possibilifier, [](Ref node){});
          level++;
          traversePrePostConditional(node[2], possibilifier, [](Ref node){});
          purgeLevel();
        } else if (type == FOR) {
          traversePrePostConditional(node[1], possibilifier, [](Ref node){});
          for (int i = 2; i <= 4; i++) {
            level++;
            traversePrePostConditional(node[i], possibilifier, [](Ref node){});
            purgeLevel();
          }
        } else if (type == IF) {
          traversePrePostConditional(node[1], possibilifier, [](Ref node){});
          level++;
          traversePrePostConditional(node[2], possibilifier, [](Ref node){});
          purgeLevel();
          if (node->size() > 3 && !!node[3]) {
            level++;
            traversePrePostConditional(node[3], possibilifier, [](Ref node){});
            purgeLevel();
          }
        } else if (type == SWITCH) {
          traversePrePostConditional(node[1], possibilifier, [](Ref node){});
          Ref cases = node[2];
          for (size_t i = 0; i < cases->size(); i++) {
            level++;
            traversePrePostConditional(cases[i][1], possibilifier, [](Ref node){});
            purgeLevel();
          }
        } else assert(0);;
        return false; // prevent recursion into children, which we already did
      }
      return true;
    };
    traversePrePostConditional(fun, possibilifier, [](Ref node){});

    StringSet optimizables;
    for (auto possible : possibles) {
      if (!unoptimizables.has(possible)) optimizables.insert(possible);
    }

    // Go through the function's code, assigning 'registers'.
    // The only tricky bit is to keep variables locked on a register through loops,
    // since they can potentially be returned to. Optimizable variables lock onto
    // loops that they enter, unoptimizable variables lock in a conservative way
    // into the topmost loop.
    // Note that we cannot lock onto a variable in a loop if it was used and free'd
    // before! (then they could overwrite us in the early part of the loop). For now
    // we just use a fresh register to make sure we avoid this, but it could be
    // optimized to check for safe registers (free, and not used in this loop level).
    StringStringMap varRegs; // maps variables to the register they will use all their life
    std::vector<StringVec> freeRegsClasses;
    freeRegsClasses.resize(ASM_NONE);
    int nextReg = 1;
    StringVec fullNames;
    fullNames.push_back(EMPTY); // names start at 1
    std::vector<StringVec> loopRegs; // for each loop nesting level, the list of bound variables
    int loops = 0; // 0 is toplevel, 1 is first loop, etc
    StringSet activeOptimizables;
    StringIntMap optimizableLoops;
    StringSet paramRegs; // true if the register is used by a parameter (and so needs no def at start of function; also cannot
                         // be shared with another param, each needs its own)
    auto decUse = [&](IString name) {
      if (!varUses[name]) return false; // no uses left, or not a relevant variable
      if (optimizables.has(name)) activeOptimizables.insert(name);
      IString reg = varRegs[name];
      assert(asmData.isLocal(name));
      StringVec& freeRegs = freeRegsClasses[asmData.getType(name)];
      if (!reg) {
        // acquire register
        if (optimizables.has(name) && freeRegs.size() > 0 &&
            !(asmData.isParam(name) && paramRegs.has(freeRegs.back()))) { // do not share registers between parameters
          reg = freeRegs.back();
          freeRegs.pop_back();
        } else {
          assert(fullNames.size() == nextReg);
          reg = getNewRegName(nextReg++, name);
          fullNames.push_back(reg);
          if (asmData.isParam(name)) paramRegs.insert(reg);
        }
        varRegs[name] = reg;
      }
      varUses[name]--;
      assert(varUses[name] >= 0);
      if (varUses[name] == 0) {
        if (optimizables.has(name)) activeOptimizables.erase(name);
        // If we are not in a loop, or we are optimizable and not bound to a loop
        // (we might have been in one but left it), we can free the register now.
        if (loops == 0 || (optimizables.has(name) && !optimizableLoops.has(name))) {
          // free register
          freeRegs.push_back(reg);
        } else {
          // when the relevant loop is exited, we will free the register
          int relevantLoop = optimizables.has(name) ? (optimizableLoops[name] ? optimizableLoops[name] : 1) : 1;
          if ((int)loopRegs.size() <= relevantLoop+1) loopRegs.resize(relevantLoop+1);
          loopRegs[relevantLoop].push_back(reg);
        }
      }
      return true;
    };
    traversePrePost(fun, [&](Ref node) { // XXX we rely on traversal order being the same as execution order here
      Ref type = node[0];
      if (type == NAME) {
        IString name = node[1]->getIString();
        if (decUse(name)) {
          node[1]->setString(varRegs[name]);
        }
      } else if (LOOP.has(type)) {
        loops++;
        // Active optimizables lock onto this loop, if not locked onto one that encloses this one
        for (auto name : activeOptimizables) {
          if (!optimizableLoops[name]) {
            optimizableLoops[name] = loops;
          }
        }
      }
    }, [&](Ref node) {
      Ref type = node[0];
      if (LOOP.has(type)) {
        // Free registers that were locked to this loop
        if ((int)loopRegs.size() > loops && loopRegs[loops].size() > 0) {
          for (auto loopReg : loopRegs[loops]) {
            freeRegsClasses[regTypes[loopReg]].push_back(loopReg);
          }
          loopRegs[loops].clear();
        }
        loops--;
      }
    });
    if (!!fun[2] && fun[2]->size()) {
      fun[2]->setSize(0); // clear params, we will fill with registers
      fun[3]->splice(0, 1); // remove fake initial var
    }

    asmData.locals.clear();
    asmData.params.clear();
    asmData.vars.clear();
    for (int i = 1; i < nextReg; i++) {
      IString reg = fullNames[i];
      AsmType type = regTypes[reg];
      if (!paramRegs.has(reg)) {
        asmData.addVar(reg, type);
      } else {
        asmData.addParam(reg, type);
        fun[2]->push_back(makeString(reg));
      }
    }
    asmData.denormalize();
  });
}

// Assign variables to 'registers', coalescing them onto a smaller number of shared
// variables.
//
// This does the same job as 'registerize' above, but burns a lot more cycles trying
// to reduce the total number of register variables.  Key points about the operation:
//
//   * we decompose the AST into a flow graph and perform a full liveness
//     analysis, to determine which variables are live at each point.
//
//   * variables that are live concurrently are assigned to different registers.
//
//   * variables that are linked via 'x=y' style statements are assigned the same
//     register if possible, so that the redundant assignment can be removed.
//     (e.g. assignments used to pass state around through loops).
//
//   * any code that cannot be reached through the flow-graph is removed.
//     (e.g. redundant break statements like 'break L123; break;').
//
//   * any assignments that we can prove are not subsequently used are removed.
//     (e.g. unnecessary assignments to the 'label' variable).
//
void registerizeHarder(Ref ast) {
#ifdef PROFILING
  clock_t tasmdata = 0;
  clock_t tflowgraph = 0;
  clock_t tlabelfix = 0;
  clock_t tbackflow = 0;
  clock_t tjuncvaruniqassign = 0;
  clock_t tjuncvarsort = 0;
  clock_t tregassign = 0;
  clock_t tblockproc = 0;
  clock_t treconstruct = 0;
#endif

  traverseFunctions(ast, [&](Ref fun) {

#ifdef PROFILING
    clock_t start = clock();
#endif

    // Do not try to process non-validating methods, like the heap replacer
    bool abort = false;
    traversePre(fun, [&abort](Ref node) {
      if (node[0] == NEW) abort = true;
    });
    if (abort) return;

    AsmData asmData(fun);

#ifdef PROFILING
    tasmdata += clock() - start;
    start = clock();
#endif

    // Utilities for allocating register variables.
    // We need distinct register pools for each type of variable.

    typedef std::map<int, IString> IntStringMap;
    std::vector<IntStringMap> allRegsByType;
    allRegsByType.resize(ASM_NONE+1);
    int nextReg = 1;

    auto createReg = [&](IString forName) {
      // Create a new register of type suitable for the given variable name.
      AsmType type = asmData.getType(forName);
      IntStringMap& allRegs = allRegsByType[type];
      int reg = nextReg++;
      allRegs[reg] = getRegName(type, reg);
      return reg;
    };

    // Traverse the tree in execution order and synthesize a basic flow-graph.
    // It's convenient to build a kind of "dual" graph where the nodes identify
    // the junctions between blocks at which control-flow may branch, and each
    // basic block is an edge connecting two such junctions.
    // For each junction we store:
    //    * set of blocks that originate at the junction
    //    * set of blocks that terminate at the junction
    // For each block we store:
    //    * a single entry junction
    //    * a single exit junction
    //    * a 'use' and 'kill' set of names for the block
    //    * full sequence of NAME and ASSIGN nodes in the block
    //    * whether each such node appears as part of a larger expression
    //      (and therefore cannot be safely eliminated)
    //    * set of labels that can be used to jump to this block

    struct Junction {
      int id;
      std::set<int> inblocks, outblocks;
      IOrderedStringSet live;
      Junction(int id_) : id(id_) {}
    };
    struct Node {
    };
    struct Block {
      int id, entry, exit;
      std::set<int> labels;
      std::vector<Ref> nodes;
      std::vector<bool> isexpr;
      StringIntMap use;
      StringSet kill;
      StringStringMap link;
      StringIntMap lastUseLoc;
      StringIntMap firstDeadLoc;
      StringIntMap firstKillLoc;
      StringIntMap lastKillLoc;

      Block() : id(-1), entry(-1), exit(-1) {}
    };
    struct ContinueBreak {
      int co, br;
      ContinueBreak() : co(-1), br(-1) {}
      ContinueBreak(int co_, int br_) : co(co_), br(br_) {}
    };
    typedef std::unordered_map<IString, ContinueBreak> LabelState;

    std::vector<Junction> junctions;
    std::vector<Block*> blocks;
    int currEntryJunction = -1;
    Block* nextBasicBlock = nullptr;
    int isInExpr = 0;
    std::vector<LabelState> activeLabels;
    activeLabels.resize(1);
    IString nextLoopLabel;

    const int ENTRY_JUNCTION = 0;
    const int EXIT_JUNCTION = 1;
    const int ENTRY_BLOCK = 0;

    auto addJunction = [&]() {
      // Create a new junction, without inserting it into the graph.
      // This is useful for e.g. pre-allocating an exit node.
      int id = junctions.size();
      junctions.push_back(Junction(id));
      return id;
    };

    std::function<int (int, bool)> joinJunction;

    auto markJunction = [&](int id) {
      // Mark current traversal location as a junction.
      // This makes a new basic block exiting at this position.
      if (id < 0) {
        id = addJunction();
      }
      joinJunction(id, true);
      return id;
    };

    auto setJunction = [&](int id, bool force) {
      // Set the next entry junction to the given id.
      // This can be used to enter at a previously-declared point.
      // You can't return to a junction with no incoming blocks
      // unless the 'force' parameter is specified.
      assert(nextBasicBlock->nodes.size() == 0); // refusing to abandon an in-progress basic block
      if (force || junctions[id].inblocks.size() > 0) {
        currEntryJunction = id;
      } else {
        currEntryJunction = -1;
      }
    };

    joinJunction = [&](int id, bool force) {
      // Complete the pending basic block by exiting at this position.
      // This can be used to exit at a previously-declared point.
      if (currEntryJunction >= 0) {
        assert(nextBasicBlock);
        nextBasicBlock->id = blocks.size();
        nextBasicBlock->entry = currEntryJunction;
        nextBasicBlock->exit = id;
        junctions[currEntryJunction].outblocks.insert(nextBasicBlock->id);
        junctions[id].inblocks.insert(nextBasicBlock->id);
        blocks.push_back(nextBasicBlock);
      } 
      nextBasicBlock = new Block();
      setJunction(id, force);
      return id;
    };

    auto pushActiveLabels = [&](int onContinue, int onBreak) {
      // Push the target junctions for continuing/breaking a loop.
      // This should be called before traversing into a loop.
      assert(activeLabels.size() > 0);
      LabelState& prevLabels = activeLabels.back();
      LabelState newLabels = prevLabels;
      newLabels[EMPTY] = ContinueBreak(onContinue, onBreak);
      if (!!nextLoopLabel) {
        newLabels[nextLoopLabel] = ContinueBreak(onContinue, onBreak);
        nextLoopLabel = EMPTY;
      }
      // An unlabelled CONTINUE should jump to innermost loop,
      // ignoring any nested SWITCH statements.
      if (onContinue < 0 && prevLabels.count(EMPTY) > 0) {
        newLabels[EMPTY].co = prevLabels[EMPTY].co;
      }
      activeLabels.push_back(newLabels);
    };

    auto popActiveLabels = [&]() {
      // Pop the target junctions for continuing/breaking a loop.
      // This should be called after traversing into a loop.
      activeLabels.pop_back();
    };

    auto markNonLocalJump = [&](IString type, IString label) {
      // Complete a block via RETURN, BREAK or CONTINUE.
      // This joins the targetted junction and then sets the current junction to null.
      // Any code traversed before we get back to an existing junction is dead code.
      if (type == RETURN) {
        joinJunction(EXIT_JUNCTION, false);
      } else {
        assert(activeLabels.size() > 0);
        assert(activeLabels.back().count(label) > 0); // 'jump to unknown label');
        auto targets = activeLabels.back()[label];
        if (type == CONTINUE) {
          joinJunction(targets.co, false);
        } else if (type == BREAK) {
          joinJunction(targets.br, false);
        } else {
          assert(0); // 'unknown jump node type');
        }
      }
      currEntryJunction = -1;
    };

    auto addUseNode = [&](Ref node) {
      // Mark a use of the given name node in the current basic block.
      assert(node[0] == NAME); // 'not a use node');
      IString name = node[1]->getIString();
      if (asmData.isLocal(name)) {
        nextBasicBlock->nodes.push_back(node);
        nextBasicBlock->isexpr.push_back(isInExpr != 0);
        if (nextBasicBlock->kill.count(name) == 0) {
          nextBasicBlock->use[name] = 1;
        }
      }
    };

    auto addKillNode = [&](Ref node) {
      // Mark an assignment to the given name node in the current basic block.
      assert(node[0] == ASSIGN); //, 'not a kill node');
      assert(node[1]->isBool(true)); // 'not a kill node');
      assert(node[2][0] == NAME); //, 'not a kill node');
      IString name = node[2][1]->getIString();
      if (asmData.isLocal(name)) {
        nextBasicBlock->nodes.push_back(node);
        nextBasicBlock->isexpr.push_back(isInExpr != 0);
        nextBasicBlock->kill.insert(name);
      }
    };

    std::function<Ref (Ref)> lookThroughCasts = [&](Ref node) {
      // Look through value-preserving casts, like "x | 0" => "x"
      if (node[0] == BINARY && node[1] == OR) {
        if (node[3][0] == NUM && node[3][1]->getNumber() == 0) {
          return lookThroughCasts(node[2]);
        }
      }
      return node;
    };

    auto addBlockLabel = [&](Ref node) {
      assert(nextBasicBlock->nodes.size() == 0); // 'cant add label to an in-progress basic block')
      if (node[0] == NUM) {
        nextBasicBlock->labels.insert(node[1]->getInteger());
      }
    };

    auto isTrueNode = [&](Ref node) {
      // Check if the given node is statically truthy.
      return (node[0] == NUM && node[1]->getNumber() != 0);
    };

    auto isFalseNode = [&](Ref node) {
      // Check if the given node is statically falsy.
      return (node[0] == NUM && node[1]->getNumber() == 0);
    };

    std::function<void (Ref)> buildFlowGraph = [&](Ref node) {
      // Recursive function to build up the flow-graph.
      // It walks the tree in execution order, calling the above state-management
      // functions at appropriate points in the traversal.
      Ref type = node[0];
  
      // Any code traversed without an active entry junction must be dead,
      // as the resulting block could never be entered. Let's remove it.
      if (currEntryJunction < 0 && junctions.size() > 0) {
        safeCopy(node, makeEmpty());
        return;
      }
 
      // Traverse each node type according to its particular control-flow semantics.
      // TODO: switchify this
      if (type == DEFUN) {
        int jEntry = markJunction(-1);
        assert(jEntry == ENTRY_JUNCTION);
        int jExit = addJunction();
        assert(jExit == EXIT_JUNCTION);
        for (size_t i = 0; i < node[3]->size(); i++) {
          buildFlowGraph(node[3][i]);
        }
        joinJunction(jExit, false);
      } else if (type == IF) {
        isInExpr++;
        buildFlowGraph(node[1]);
        isInExpr--;
        int jEnter = markJunction(-1);
        int jExit = addJunction();
        if (!!node[2]) {
          // Detect and mark "if (label == N) { <labelled block> }".
          if (node[1][0] == BINARY && node[1][1] == EQ) {
            Ref lhs = lookThroughCasts(node[1][2]);
            if (lhs[0] == NAME && lhs[1] == LABEL) {
              addBlockLabel(lookThroughCasts(node[1][3]));
            }
          }
          buildFlowGraph(node[2]);
        }
        joinJunction(jExit, false);
        setJunction(jEnter, false);
        if (node->size() > 3 && !!node[3]) {
          buildFlowGraph(node[3]);
        }
        joinJunction(jExit, false);
      } else if (type == CONDITIONAL) {
        isInExpr++;
        // If the conditional has no side-effects, we can treat it as a single
        // block, which might open up opportunities to remove it entirely.
        if (!hasSideEffects(node)) {
          buildFlowGraph(node[1]);
          if (!!node[2]) {
            buildFlowGraph(node[2]);
          }
          if (!!node[3]) {
            buildFlowGraph(node[3]);
          }
        } else {
          buildFlowGraph(node[1]);
          int jEnter = markJunction(-1);
          int jExit = addJunction();
          if (!!node[2]) {
            buildFlowGraph(node[2]);
          }
          joinJunction(jExit, false);
          setJunction(jEnter, false);
          if (!!node[3]) {
            buildFlowGraph(node[3]);
          }
          joinJunction(jExit, false);
        }
        isInExpr--;
      } else if (type == WHILE) {
        // Special-case "while (1) {}" to use fewer junctions,
        // since emscripten generates a lot of these.
        if (isTrueNode(node[1])) {
          int jLoop = markJunction(-1);
          int jExit = addJunction();
          pushActiveLabels(jLoop, jExit);
          buildFlowGraph(node[2]);
          popActiveLabels();
          joinJunction(jLoop, false);
          setJunction(jExit, false);
        } else {
          int jCond = markJunction(-1);
          int jLoop = addJunction();
          int jExit = addJunction();
          isInExpr++;
          buildFlowGraph(node[1]);
          isInExpr--;
          joinJunction(jLoop, false);
          pushActiveLabels(jCond, jExit);
          buildFlowGraph(node[2]);
          popActiveLabels();
          joinJunction(jCond, false);
          // An empty basic-block linking condition exit to loop exit.
          setJunction(jLoop, false);
          joinJunction(jExit, false);
        }
      } else if (type == DO) {
        // Special-case "do {} while (1)" and "do {} while (0)" to use
        // fewer junctions, since emscripten generates a lot of these.
        if (isFalseNode(node[1])) {
          int jExit = addJunction();
          pushActiveLabels(jExit, jExit);
          buildFlowGraph(node[2]);
          popActiveLabels();
          joinJunction(jExit, false);
        } else if (isTrueNode(node[1])) {
          int jLoop = markJunction(-1);
          int jExit = addJunction();
          pushActiveLabels(jLoop, jExit);
          buildFlowGraph(node[2]);
          popActiveLabels();
          joinJunction(jLoop, false);
          setJunction(jExit, false);
        } else {
          int jLoop = markJunction(-1);
          int jCond = addJunction();
          int jCondExit = addJunction();
          int jExit = addJunction();
          pushActiveLabels(jCond, jExit);
          buildFlowGraph(node[2]);
          popActiveLabels();
          joinJunction(jCond, false);
          isInExpr++;
          buildFlowGraph(node[1]);
          isInExpr--;
          joinJunction(jCondExit, false);
          joinJunction(jLoop, false);
          setJunction(jCondExit, false);
          joinJunction(jExit, false);
        }
      } else if (type == FOR) {
        int jTest = addJunction();
        int jBody = addJunction();
        int jStep = addJunction();
        int jExit = addJunction();
        buildFlowGraph(node[1]);
        joinJunction(jTest, false);
        isInExpr++;
        buildFlowGraph(node[2]);
        isInExpr--;
        joinJunction(jBody, false);
        pushActiveLabels(jStep, jExit);
        buildFlowGraph(node[4]);
        popActiveLabels();
        joinJunction(jStep, false);
        buildFlowGraph(node[3]);
        joinJunction(jTest, false);
        setJunction(jBody, false);
        joinJunction(jExit, false);
      } else if (type == LABEL) {
        assert(BREAK_CAPTURERS.has(node[2][0])); // 'label on non-loop, non-switch statement')
        nextLoopLabel = node[1]->getIString();
        buildFlowGraph(node[2]);
      } else if (type == SWITCH) {
        // Emscripten generates switch statements of a very limited
        // form: all case clauses are numeric literals, and all
        // case bodies end with a (maybe implicit) break.  So it's
        // basically equivalent to a multi-way IF statement.
        isInExpr++;
        buildFlowGraph(node[1]);
        isInExpr--;
        Ref condition = lookThroughCasts(node[1]);
        int jCheckExit = markJunction(-1);
        int jExit = addJunction();
        pushActiveLabels(-1, jExit);
        bool hasDefault = false;
        for (size_t i = 0; i < node[2]->size(); i++) {
          setJunction(jCheckExit, false);
          // All case clauses are either 'default' or a numeric literal.
          if (!node[2][i][0]) {
            hasDefault = true;
          } else {
            // Detect switches dispatching to labelled blocks.
            if (condition[0] == NAME && condition[1] == LABEL) {
              addBlockLabel(lookThroughCasts(node[2][i][0]));
            }
          }
          for (size_t j = 0; j < node[2][i][1]->size(); j++) {
            buildFlowGraph(node[2][i][1][j]);
          }
          // Control flow will never actually reach the end of the case body.
          // If there's live code here, assume it jumps to case exit.
          if (currEntryJunction >= 0 && nextBasicBlock->nodes.size() > 0) {
            if (!!node[2][i][0]) {
              markNonLocalJump(RETURN, EMPTY);
            } else {
              joinJunction(jExit, false);
            }
          }
        }
        // If there was no default case, we also need an empty block
        // linking straight from the test evaluation to the exit.
        if (!hasDefault) {
          setJunction(jCheckExit, false);
        }
        joinJunction(jExit, false);
        popActiveLabels();
      } else if (type == RETURN) {
        if (!!node[1]) {
          isInExpr++;
          buildFlowGraph(node[1]);
          isInExpr--;
        }
        markNonLocalJump(type->getIString(), EMPTY);
      } else if (type == BREAK || type == CONTINUE) {
        markNonLocalJump(type->getIString(), !!node[1] ? node[1]->getIString() : EMPTY);
      } else if (type == ASSIGN) {
        isInExpr++;
        buildFlowGraph(node[3]);
        isInExpr--;
        if (node[1]->isBool(true) && node[2][0] == NAME) {
          addKillNode(node);
        } else {
          buildFlowGraph(node[2]);
        }
      } else if (type == NAME) {
        addUseNode(node);
      } else if (type == BLOCK || type == TOPLEVEL) {
        if (!!node[1]) {
          for (size_t i = 0; i < node[1]->size(); i++) {
            buildFlowGraph(node[1][i]);
          }
        }
      } else if (type == STAT) {
        buildFlowGraph(node[1]);
      } else if (type == UNARY_PREFIX || type == UNARY_POSTFIX) {
        isInExpr++;
        buildFlowGraph(node[2]);
        isInExpr--;
      } else if (type == BINARY) {
        isInExpr++;
        buildFlowGraph(node[2]);
        buildFlowGraph(node[3]);
        isInExpr--;
      } else if (type == CALL) {
        isInExpr++;
        buildFlowGraph(node[1]);
        if (!!node[2]) {
          for (size_t i = 0; i < node[2]->size(); i++) {
            buildFlowGraph(node[2][i]);
          }
        }
        isInExpr--;
        // If the call is statically known to throw,
        // treat it as a jump to function exit.
        if (!isInExpr && node[1][0] == NAME) {
          if (FUNCTIONS_THAT_ALWAYS_THROW.has(node[1][1])) {
            markNonLocalJump(RETURN, EMPTY);
          }
        }
      } else if (type == SEQ || type == SUB) {
        isInExpr++;
        buildFlowGraph(node[1]);
        buildFlowGraph(node[2]);
        isInExpr--;
      } else if (type == DOT || type == THROW) {
        isInExpr++;
        buildFlowGraph(node[1]);
        isInExpr--;
      } else if (type == NUM || type == STRING || type == VAR) {
        // nada
      } else {
        assert(0); // 'unsupported node type: ' + type);
      }
    };

    buildFlowGraph(fun);

#ifdef PROFILING
    tflowgraph += clock() - start;
    start = clock();
#endif

    assert(junctions[ENTRY_JUNCTION].inblocks.size() == 0); // 'function entry must have no incoming blocks');
    assert(junctions[EXIT_JUNCTION].outblocks.size() == 0); // 'function exit must have no outgoing blocks');
    assert(blocks[ENTRY_BLOCK]->entry == ENTRY_JUNCTION); //, 'block zero must be the initial block');

    // Fix up implicit jumps done by assigning to the LABEL variable.
    // If a block ends with an assignment to LABEL and there's another block
    // with that value of LABEL as precondition, we tweak the flow graph so
    // that the former jumps straight to the later.

    std::map<int, Block*> labelledBlocks;
    typedef std::pair<Ref, Block*> Jump;
    std::vector<Jump> labelledJumps;

    for (size_t i = 0; i < blocks.size(); i++) {
      Block* block = blocks[i];
      // Does it have any labels as preconditions to its entry?
      for (auto labelVal : block->labels) {
        // If there are multiple blocks with the same label, all bets are off.
        // This seems to happen sometimes for short blocks that end with a return.
        // TODO: it should be safe to merge the duplicates if they're identical.
        if (labelledBlocks.count(labelVal) > 0) {
          labelledBlocks.clear();
          labelledJumps.clear();
          goto AFTER_FINDLABELLEDBLOCKS;
        }
        labelledBlocks[labelVal] = block;
      }
      // Does it assign a specific label value at exit?
      if (block->kill.has(LABEL)) {
        Ref finalNode = block->nodes.back();
        if (finalNode[0] == ASSIGN && finalNode[2][1] == LABEL) {
          // If labels are computed dynamically then all bets are off.
          // This can happen due to indirect branching in llvm output.
          if (finalNode[3][0] != NUM) {
            labelledBlocks.clear();
            labelledJumps.clear();
            goto AFTER_FINDLABELLEDBLOCKS;
          }
          labelledJumps.push_back(Jump(finalNode[3][1], block));
        } else { 
          // If label is assigned a non-zero value elsewhere in the block
          // then all bets are off.  This can happen e.g. due to outlining
          // saving/restoring label to the stack.
          for (size_t j = 0; j < block->nodes.size() - 1; j++) {
            if (block->nodes[j][0] == ASSIGN && block->nodes[j][2][1] == LABEL) {
              if (block->nodes[j][3][0] != NUM || block->nodes[j][3][1]->getNumber() != 0) {
                labelledBlocks.clear();
                labelledJumps.clear();
                goto AFTER_FINDLABELLEDBLOCKS;
              }
            }
          }
        }
      }
    }

    AFTER_FINDLABELLEDBLOCKS:

    for (auto labelVal : labelledBlocks) {
      Block* block = labelVal.second;
      // Disconnect it from the graph, and create a
      // new junction for jumps targetting this label.
      junctions[block->entry].outblocks.erase(block->id);
      block->entry = addJunction();
      junctions[block->entry].outblocks.insert(block->id);
      // Add a fake use of LABEL to keep it alive in predecessor.
      block->use[LABEL] = 1;
      block->nodes.insert(block->nodes.begin(), makeName(LABEL));
      block->isexpr.insert(block->isexpr.begin(), 1);
    }
    for (size_t i = 0; i < labelledJumps.size(); i++) {
      auto labelVal = labelledJumps[i].first;
      auto block = labelledJumps[i].second;
      Block* targetBlock = labelledBlocks[labelVal->getInteger()];
      if (targetBlock) {
        // Redirect its exit to entry of the target block.
        junctions[block->exit].inblocks.erase(block->id);
        block->exit = targetBlock->entry;
        junctions[block->exit].inblocks.insert(block->id);
      }
    }

#ifdef PROFILING
    tlabelfix += clock() - start;
    start = clock();
#endif

    // Do a backwards data-flow analysis to determine the set of live
    // variables at each junction, and to use this information to eliminate
    // any unused assignments.
    // We run two nested phases.  The inner phase builds the live set for each
    // junction.  The outer phase uses this to try to eliminate redundant
    // stores in each basic block, which might in turn affect liveness info.

    auto analyzeJunction = [&](Junction& junc) {
      // Update the live set for this junction.
      IOrderedStringSet live;
      for (auto b : junc.outblocks) {
        Block* block = blocks[b];
        IOrderedStringSet& liveSucc = junctions[block->exit].live;
        for (auto name : liveSucc) {
          if (!block->kill.has(name)) {
            live.insert(name);
          }
        }
        for (auto name : block->use) {
          live.insert(name.first);
        }
      }
      junc.live = live;
    };

    auto analyzeBlock = [&](Block* block) {
      // Update information about the behaviour of the block.
      // This includes the standard 'use' and 'kill' information,
      // plus a 'link' set naming values that flow through from entry
      // to exit, possibly changing names via simple 'x=y' assignments.
      // As we go, we eliminate assignments if the variable is not
      // subsequently used.
      auto live = junctions[block->exit].live;
      StringIntMap use;
      StringSet kill;
      StringStringMap link;
      StringIntMap lastUseLoc;
      StringIntMap firstDeadLoc;
      StringIntMap firstKillLoc;
      StringIntMap lastKillLoc;
      for (auto name : live) {
        link[name] = name;
        lastUseLoc[name] = block->nodes.size();
        firstDeadLoc[name] = block->nodes.size();
      }
      for (int j = block->nodes.size() - 1; j >= 0 ; j--) {
        Ref node = block->nodes[j];
        if (node[0] == NAME) {
          IString name = node[1]->getIString();
          live.insert(name);
          use[name] = j;
          if (lastUseLoc.count(name) == 0) {
            lastUseLoc[name] = j;
            firstDeadLoc[name] = j;
          }
        } else {
          IString name = node[2][1]->getIString();
          // We only keep assignments if they will be subsequently used.
          if (live.has(name)) {
            kill.insert(name);
            use.erase(name);
            live.erase(name);
            firstDeadLoc[name] = j;
            firstKillLoc[name] = j;
            if (lastUseLoc.count(name) == 0) {
              lastUseLoc[name] = j;
            }
            if (lastKillLoc.count(name) == 0) {
              lastKillLoc[name] = j;
            }
            // If it's an "x=y" and "y" is not live, then we can create a
            // flow-through link from "y" to "x".  If not then there's no
            // flow-through link for "x".
            if (link.has(name)) {
              IString oldLink = link[name];
              link.erase(name);
              if (node[3][0] == NAME) {
                if (asmData.isLocal(node[3][1]->getIString())) {
                  link[node[3][1]->getIString()] = oldLink;
                }
              }
            }
          } else {
            // The result of this assignment is never used, so delete it.
            // We may need to keep the RHS for its value or its side-effects.
            auto removeUnusedNodes = [&](int j, int n) {
              for (auto pair : lastUseLoc) {
                pair.second -= n;
              }
              for (auto pair : firstKillLoc) {
                pair.second -= n;
              }
              for (auto pair : lastKillLoc) {
                pair.second -= n;
              }
              for (auto pair : firstDeadLoc) {
                pair.second -= n;
              }
              block->nodes.erase(block->nodes.begin() + j, block->nodes.begin() + j + n);
              block->isexpr.erase(block->isexpr.begin() + j, block->isexpr.begin() + j + n);
            };
            if (block->isexpr[j] || hasSideEffects(node[3])) {
              safeCopy(node, node[3]);
              removeUnusedNodes(j, 1);
            } else {
              int numUsesInExpr = 0;
              traversePre(node[3], [&](Ref node) {
                if (node[0] == NAME && asmData.isLocal(node[1]->getIString())) {
                  numUsesInExpr++;
                }
              });
              safeCopy(node, makeEmpty());
              j = j - numUsesInExpr;
              removeUnusedNodes(j, 1 + numUsesInExpr);
            }
          }
        }
      }
      // XXX efficiency
      block->use = use;
      block->kill = kill;
      block->link = link;
      block->lastUseLoc = lastUseLoc;
      block->firstDeadLoc = firstDeadLoc;
      block->firstKillLoc = firstKillLoc;
      block->lastKillLoc = lastKillLoc;
    };

    // Ordered map to work in approximate reverse order of junction appearance
    std::set<int> jWorkSet;
    std::set<int> bWorkSet;

    // Be sure to visit every junction at least once.
    // This avoids missing some vars because we disconnected them
    // when processing the labelled jumps.
    for (size_t i = EXIT_JUNCTION; i < junctions.size(); i++) {
      jWorkSet.insert(i);
      for (auto b : junctions[i].inblocks) {
        bWorkSet.insert(b);
      }
    }
    // Exit junction never has any live variable changes to propagate
    jWorkSet.erase(EXIT_JUNCTION);

    do {
      // Iterate on just the junctions until we get stable live sets.
      // The first run of this loop will grow the live sets to their maximal size.
      // Subsequent runs will shrink them based on eliminated in-block uses.
      while (jWorkSet.size() > 0) {
        auto last = jWorkSet.end();
        --last;
        Junction& junc = junctions[*last];
        jWorkSet.erase(last);
        IOrderedStringSet oldLive = junc.live; // copy it here, to check for changes later
        analyzeJunction(junc);
        if (oldLive != junc.live) {
          // Live set changed, updated predecessor blocks and junctions.
          for (auto b : junc.inblocks) {
            bWorkSet.insert(b);
            jWorkSet.insert(blocks[b]->entry);
          }
        }
      }
      // Now update the blocks based on the calculated live sets.
      while (bWorkSet.size() > 0) {
        auto last = bWorkSet.end();
        --last;
        Block* block = blocks[*last];
        bWorkSet.erase(last);
        auto oldUse = block->use;
        analyzeBlock(block);
        if (oldUse != block->use) {
          // The use set changed, re-process the entry junction.
          jWorkSet.insert(block->entry);
        }
      }
    } while (jWorkSet.size() > 0);

#ifdef PROFILING
    tbackflow += clock() - start;
    start = clock();
#endif

    // Insist that all function parameters are alive at function entry.
    // This ensures they will be assigned independent registers, even
    // if they happen to be unused.

    for (auto name : asmData.params) {
      junctions[ENTRY_JUNCTION].live.insert(name);
    }

    // For variables that are live at one or more junctions, we assign them
    // a consistent register for the entire scope of the function.  Find pairs
    // of variable that cannot use the same register (the "conflicts") as well
    // as pairs of variables that we'd like to have share the same register
    // (the "links").

    struct JuncVar {
      std::vector<bool> conf;
      IOrderedStringSet link;
      std::unordered_set<int> excl;
      int reg;
      bool used;
      JuncVar() : reg(-1), used(false) {}
    };
    size_t numLocals = asmData.locals.size();
    std::unordered_map<IString, size_t> nameToNum;
    std::vector<IString> numToName;
    nameToNum.reserve(numLocals);
    numToName.reserve(numLocals);
    for (auto kv : asmData.locals) {
      nameToNum[kv.first] = numToName.size();
      numToName.push_back(kv.first);
    }

    std::vector<JuncVar> juncVars(numLocals);
    for (Junction& junc : junctions) {
      for (IString name : junc.live) {
        JuncVar& jVar = juncVars[nameToNum[name]];
        jVar.used = true;
        jVar.conf.assign(numLocals, false);
      }
    }
    std::map<IString, std::vector<Block*>> possibleBlockConflictsMap;
    std::vector<std::pair<size_t, std::vector<Block*>>> possibleBlockConflicts;
    std::unordered_map<IString, std::vector<Block*>> possibleBlockLinks;
    possibleBlockConflicts.reserve(numLocals);
    possibleBlockLinks.reserve(numLocals);

    for (Junction& junc : junctions) {
      // Pre-compute the possible conflicts and links for each block rather
      // than checking potentially impossible options for each var
      possibleBlockConflictsMap.clear();
      possibleBlockConflicts.clear();
      possibleBlockLinks.clear();
      for (auto b : junc.outblocks) {
        Block* block = blocks[b];
        Junction& jSucc = junctions[block->exit];
        for (auto name : jSucc.live) {
          possibleBlockConflictsMap[name].push_back(block);
        }
        for (auto name_linkname : block->link) {
          if (name_linkname.first != name_linkname.second) {
            possibleBlockLinks[name_linkname.first].push_back(block);
          }
        }
      }
      // Find the live variables in this block, mark them as unnecessary to
      // check for conflicts (we mark all live vars as conflicting later)
      std::vector<size_t> liveJVarNums;
      liveJVarNums.reserve(junc.live.size());
      for (auto name : junc.live) {
        size_t jVarNum = nameToNum[name];
        liveJVarNums.push_back(jVarNum);
        possibleBlockConflictsMap.erase(name);
      }
      // Extract just the variables we might want to check for conflicts
      for (auto kv : possibleBlockConflictsMap) {
        possibleBlockConflicts.push_back(std::make_pair(nameToNum[kv.first], kv.second));
      }

      for (size_t jVarNum : liveJVarNums) {
        JuncVar& jvar = juncVars[jVarNum];
        IString name = numToName[jVarNum];
        // It conflicts with all other names live at this junction.
        for (size_t liveJVarNum : liveJVarNums) {
          jvar.conf[liveJVarNum] = true;
        }
        jvar.conf[jVarNum] = false; // except for itself, of course

        // It conflicts with any output vars of successor blocks,
        // if they're assigned before it goes dead in that block.
        for (auto jvarnum_blocks : possibleBlockConflicts) {
          size_t otherJVarNum = jvarnum_blocks.first;
          IString otherName = numToName[otherJVarNum];
          for (auto block : jvarnum_blocks.second) {
            if (block->lastKillLoc[otherName] < block->firstDeadLoc[name]) {
              jvar.conf[otherJVarNum] = true;
              juncVars[otherJVarNum].conf[jVarNum] = true;
              break;
            }
          }
        }

        // It links with any linkages in the outgoing blocks.
        for (auto block: possibleBlockLinks[name]) {
          IString linkName = block->link[name];
          jvar.link.insert(linkName);
          juncVars[nameToNum[linkName]].link.insert(name);
        }
      }
    }

#ifdef PROFILING
    tjuncvaruniqassign += clock() - start;
    start = clock();
#endif

    // Attempt to sort the junction variables to heuristically reduce conflicts.
    // Simple starting point: handle the most-conflicted variables first.
    // This seems to work pretty well.

    std::vector<size_t> sortedJVarNums;
    sortedJVarNums.reserve(juncVars.size());
    std::vector<size_t> jVarConfCounts(numLocals);
    for (size_t jVarNum = 0; jVarNum < juncVars.size(); jVarNum++) {
      JuncVar& jVar = juncVars[jVarNum];
      if (!jVar.used) continue;
      jVarConfCounts[jVarNum] = std::count(jVar.conf.begin(), jVar.conf.end(), true);
      sortedJVarNums.push_back(jVarNum);
    }
    std::sort(sortedJVarNums.begin(), sortedJVarNums.end(), [&](const size_t vi1, const size_t vi2) {
      // sort by # of conflicts
      if (jVarConfCounts[vi1] < jVarConfCounts[vi2]) return true;
      if (jVarConfCounts[vi1] == jVarConfCounts[vi2]) return numToName[vi1] < numToName[vi2];
      return false;
    });

#ifdef PROFILING
    tjuncvarsort += clock() - start;
    start = clock();
#endif

    // We can now assign a register to each junction variable.
    // Process them in order, trying available registers until we find
    // one that works, and propagating the choice to linked/conflicted
    // variables as we go.

    std::function<bool (IString, int)> tryAssignRegister = [&](IString name, int reg) {
      // Try to assign the given register to the given variable,
      // and propagate that choice throughout the graph.
      // Returns true if successful, false if there was a conflict.
      JuncVar& jv = juncVars[nameToNum[name]];
      if (jv.reg > 0) {
        return jv.reg == reg;
      }
      if (jv.excl.count(reg) > 0) {
        return false;
      }
      jv.reg = reg;
      // Exclude use of this register at all conflicting variables.
      for (size_t confNameNum = 0; confNameNum < jv.conf.size(); confNameNum++) {
        if (jv.conf[confNameNum]) {
          juncVars[confNameNum].excl.insert(reg);
        }
      }
      // Try to propagate it into linked variables.
      // It's not an error if we can't.
      for (auto linkName : jv.link) {
        tryAssignRegister(linkName, reg);
      }
      return true;
    };
    for (size_t jVarNum : sortedJVarNums) {
      // It may already be assigned due to linked-variable propagation.
      if (juncVars[jVarNum].reg > 0) {
        continue;
      }
      IString name = numToName[jVarNum];
      // Try to use existing registers first.
      auto& allRegs = allRegsByType[asmData.getType(name)];
      bool moar = false;
      for (auto reg : allRegs) {
        if (tryAssignRegister(name, reg.first)) {
          moar = true;
          break;
        }
      }
      if (moar) continue;
      // They're all taken, create a new one.
      tryAssignRegister(name, createReg(name));
    }

#ifdef PROFILING
    tregassign += clock() - start;
    start = clock();
#endif

    // Each basic block can now be processed in turn.
    // There may be internal-use-only variables that still need a register
    // assigned, but they can be treated just for this block.  We know
    // that all inter-block variables are in a good state thanks to
    // junction variable consistency.

    for (size_t i = 0; i < blocks.size(); i++) {
      Block* block = blocks[i];
      if (block->nodes.size() == 0) continue;
      Junction& jEnter = junctions[block->entry];
      Junction& jExit = junctions[block->exit];
      // Mark the point at which each input reg becomes dead.
      // Variables alive before this point must not be assigned
      // to that register.
      StringSet inputVars;
      std::unordered_map<int, int> inputDeadLoc;
      std::unordered_map<int, IString> inputVarsByReg;
      for (auto name : jExit.live) {
        if (!block->kill.has(name)) {
          inputVars.insert(name);
          int reg = juncVars[nameToNum[name]].reg;
          assert(reg > 0); // 'input variable doesnt have a register');
          inputDeadLoc[reg] = block->firstDeadLoc[name];
          inputVarsByReg[reg] = name;
        }
      }
      for (auto pair : block->use) {
        IString name = pair.first;
        if (!inputVars.has(name)) {
          inputVars.insert(name);
          int reg = juncVars[nameToNum[name]].reg;
          assert(reg > 0); // 'input variable doesnt have a register');
          inputDeadLoc[reg] = block->firstDeadLoc[name];
          inputVarsByReg[reg] = name;
        }
      }
      // TODO assert(setSize(setSub(inputVars, jEnter.live)) == 0);
      // Scan through backwards, allocating registers on demand.
      // Be careful to avoid conflicts with the input registers.
      // We consume free registers in last-used order, which helps to
      // eliminate "x=y" assignments that are the last use of "y".
      StringIntMap assignedRegs;
      auto freeRegsByTypePre = allRegsByType; // XXX copy
      // Begin with all live vars assigned per the exit junction.
      for (auto name : jExit.live) {
        int reg = juncVars[nameToNum[name]].reg;
        assert(reg > 0); // 'output variable doesnt have a register');
        assignedRegs[name] = reg;
        freeRegsByTypePre[asmData.getType(name)].erase(reg); // XXX assert?
      }
      std::vector<std::vector<int>> freeRegsByType;
      freeRegsByType.resize(freeRegsByTypePre.size());
      for (size_t j = 0; j < freeRegsByTypePre.size(); j++) {
        for (auto pair : freeRegsByTypePre[j]) {
          freeRegsByType[j].push_back(pair.first);
        }
      }
      // Scan through the nodes in sequence, modifying each node in-place
      // and grabbing/freeing registers as needed.
      std::vector<std::pair<int, Ref>> maybeRemoveNodes;
      for (int j = block->nodes.size() - 1; j >= 0; j--) {
        Ref node = block->nodes[j];
        IString name = (node[0] == ASSIGN ? node[2][1] : node[1])->getIString();
        IntStringMap& allRegs = allRegsByType[asmData.getType(name)];
        std::vector<int>& freeRegs = freeRegsByType[asmData.getType(name)];
        int reg = assignedRegs[name]; // XXX may insert a zero
        if (node[0] == NAME) {
          // A use.  Grab a register if it doesn't have one.
          if (reg <= 0) {
            if (inputVars.has(name) && j <= block->firstDeadLoc[name]) {
              // Assignment to an input variable, must use pre-assigned reg.
              reg = juncVars[nameToNum[name]].reg;
              assignedRegs[name] = reg;
              for (int k = freeRegs.size() - 1; k >= 0; k--) {
                if (freeRegs[k] == reg) {
                  freeRegs.erase(freeRegs.begin() + k);
                  break;
                }
              }
            } else {
              // Try to use one of the existing free registers.
              // It must not conflict with an input register.
              for (int k = freeRegs.size() - 1; k >= 0; k--) {
                reg = freeRegs[k];
                // Check for conflict with input registers.
                if (inputDeadLoc.count(reg) > 0) {
                  if (block->firstKillLoc[name] <= inputDeadLoc[reg]) {
                    if (name != inputVarsByReg[reg]) {
                      continue;
                    }
                  }
                }
                // Found one!
                assignedRegs[name] = reg;
                assert(reg > 0);
                freeRegs.erase(freeRegs.begin() + k);
                break;
              }
              // If we didn't find a suitable register, create a new one.
              if (assignedRegs[name] <= 0) {
                reg = createReg(name);
                assignedRegs[name] = reg;
              }
            }
          }
          node[1]->setString(allRegs[reg]);
        } else {
          // A kill. This frees the assigned register.
          assert(reg > 0); //, 'live variable doesnt have a reg?')
          node[2][1]->setString(allRegs[reg]);
          freeRegs.push_back(reg);
          assignedRegs.erase(name);
          if (node[3][0] == NAME && asmData.isLocal(node[3][1]->getIString())) {
            maybeRemoveNodes.push_back(std::pair<int, Ref>(j, node));
          }
        }
      }
      // If we managed to create any "x=x" assignments, remove them.
      for (size_t j = 0; j < maybeRemoveNodes.size(); j++) {
        Ref node = maybeRemoveNodes[j].second;
        if (node[2][1] == node[3][1]) {
          if (block->isexpr[maybeRemoveNodes[j].first]) {
            safeCopy(node, node[2]);
          } else {
            safeCopy(node, makeEmpty());
          }
        }
      }
    }

#ifdef PROFILING
    tblockproc += clock() - start;
    start = clock();
#endif

    // Assign registers to function params based on entry junction

    StringSet paramRegs;
    if (!!fun[2]) {
      for (size_t i = 0; i < fun[2]->size(); i++) {
        auto& allRegs = allRegsByType[asmData.getType(fun[2][i]->getIString())];
        fun[2][i]->setString(allRegs[juncVars[nameToNum[fun[2][i]->getIString()]].reg]);
        paramRegs.insert(fun[2][i]->getIString());
      }
    }

    // That's it!
    // Re-construct the function with appropriate variable definitions.

    asmData.locals.clear();
    asmData.params.clear();
    asmData.vars.clear();
    for (int i = 1; i < nextReg; i++) {
      for (size_t type = 0; type < allRegsByType.size(); type++) {
        if (allRegsByType[type].count(i) > 0) {
          IString reg = allRegsByType[type][i];
          if (!paramRegs.has(reg)) {
            asmData.addVar(reg, intToAsmType(type));
          } else {
            asmData.addParam(reg, intToAsmType(type));
          }
          break;
        }
      }
    }
    asmData.denormalize();

    removeAllUselessSubNodes(fun); // XXX vacuum?    vacuum(fun);

#ifdef PROFILING
    treconstruct += clock() - start;
    start = clock();
#endif

  });
#ifdef PROFILING
  errv("    RH stages: a:%li fl:%li lf:%li bf:%li jvua:%li jvs:%li jra:%li bp:%li r:%li",
    tasmdata, tflowgraph, tlabelfix, tbackflow, tjuncvaruniqassign, tjuncvarsort, tregassign, tblockproc, treconstruct);
#endif
}
// end registerizeHarder

// minified names generation
StringSet RESERVED("do if in for new try var env let case else enum this void with");
const char *VALID_MIN_INITS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$";
const char *VALID_MIN_LATERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$0123456789";

StringVec minifiedNames;
std::vector<int> minifiedState;

void ensureMinifiedNames(int n) { // make sure the nth index in minifiedNames exists. done 100% deterministically
  static int VALID_MIN_INITS_LEN = strlen(VALID_MIN_INITS);
  static int VALID_MIN_LATERS_LEN = strlen(VALID_MIN_LATERS);

  while ((int)minifiedNames.size() < n+1) {
    // generate the current name
    std::string name;
    name += VALID_MIN_INITS[minifiedState[0]];
    for (size_t i = 1; i < minifiedState.size(); i++) {
      name += VALID_MIN_LATERS[minifiedState[i]];
    }
    IString str(strdupe(name.c_str())); // leaked!
    if (!RESERVED.has(str)) minifiedNames.push_back(str);
    // increment the state
    size_t i = 0;
    while (1) {
      minifiedState[i]++;
      if (minifiedState[i] < (i == 0 ? VALID_MIN_INITS_LEN : VALID_MIN_LATERS_LEN)) break;
      // overflow
      minifiedState[i] = 0;
      i++;
      if (i == minifiedState.size()) minifiedState.push_back(-1); // will become 0 after increment in next loop head
    }
  }
}

void minifyLocals(Ref ast) {
  assert(!!extraInfo);
  IString GLOBALS("globals");
  assert(extraInfo->has(GLOBALS));
  Ref globals = extraInfo[GLOBALS];

  if (minifiedState.size() == 0) minifiedState.push_back(0);

  traverseFunctions(ast, [&globals](Ref fun) {
    // Analyse the asmjs to figure out local variable names,
    // but operate on the original source tree so that we don't
    // miss any global names in e.g. variable initializers.
    AsmData asmData(fun);
    asmData.denormalize(); // TODO: we can avoid modifying at all here - we just need a list of local vars+params

    StringStringMap newNames;
    StringSet usedNames;

    // Find all the globals that we need to minify using
    // pre-assigned names.  Don't actually minify them yet
    // as that might interfere with local variable names.
    traversePre(fun, [&](Ref node) {
      if (node[0] == NAME) {
        IString name = node[1]->getIString();
        if (!asmData.isLocal(name)) {
          if (globals->has(name)) {
            IString minified = globals[name]->getIString();
            assert(!!minified);
            newNames[name] = minified;
            usedNames.insert(minified);
          }
        }
      }
    });

    // The first time we encounter a local name, we assign it a
    // minified name that's not currently in use.  Allocating on
    // demand means they're processed in a predictable order,
    // which is very handy for testing/debugging purposes.
    int nextMinifiedName = 0;
    auto getNextMinifiedName = [&]() {
      IString minified;
      while (1) {
        ensureMinifiedNames(nextMinifiedName);
        minified = minifiedNames[nextMinifiedName++];
        // TODO: we can probably remove !isLocalName here
        if (!usedNames.has(minified) && !asmData.isLocal(minified)) {
          return minified;
        }
      }
    };

    // We can also minify loop labels, using a separate namespace
    // to the variable declarations.
    StringStringMap newLabels;
    int nextMinifiedLabel = 0;
    auto getNextMinifiedLabel = [&]() {
      ensureMinifiedNames(nextMinifiedLabel);
      return minifiedNames[nextMinifiedLabel++];
    };

    // Traverse and minify all names.
    if (globals->has(fun[1]->getIString())) {
      fun[1]->setString(globals[fun[1]->getIString()]->getIString());
      assert(!!fun[1]);
    }
    if (!!fun[2]) {
      for (size_t i = 0; i < fun[2]->size(); i++) {
        IString minified = getNextMinifiedName();
        newNames[fun[2][i]->getIString()] = minified;
        fun[2][i]->setString(minified);
      }
    }
    traversePre(fun[3], [&](Ref node) {
      Ref type = node[0];
      if (type == NAME) {
        IString name = node[1]->getIString();
        IString minified = newNames[name];
        if (!!minified) {
          node[1]->setString(minified);
        } else if (asmData.isLocal(name)) {
          minified = getNextMinifiedName();
          newNames[name] = minified;
          node[1]->setString(minified);
        }
      } else if (type == VAR) {
        for (size_t i = 0; i < node[1]->size(); i++) {
          Ref defn = node[1][i];
          IString name = defn[0]->getIString();
          if (!(newNames.has(name))) {
            newNames[name] = getNextMinifiedName();
          }
          defn[0]->setString(newNames[name]);
        }
      } else if (type == LABEL) {
        IString name = node[1]->getIString();
        if (!newLabels.has(name)) {
          newLabels[name] = getNextMinifiedLabel();
        }
        node[1]->setString(newLabels[name]);
      } else if (type == BREAK || type == CONTINUE) {
        if (node->size() > 1 && !!node[1]) {
          node[1]->setString(newLabels[node[1]->getIString()]);
        }
      }
    });
  });
}

void asmLastOpts(Ref ast) {
  std::vector<Ref> statsStack;
  traverseFunctions(ast, [&](Ref fun) {
    traversePrePost(fun, [&](Ref node) {
      Ref type = node[0];
      Ref stats = getStatements(node);
      if (!!stats) statsStack.push_back(stats);
      if (CONDITION_CHECKERS.has(type)) {
        node[1] = simplifyCondition(node[1]);
      }
      if (type == WHILE && node[1][0] == NUM && node[1][1]->getNumber() == 1 && node[2][0] == BLOCK && node[2]->size() == 2) {
        // This is at the end of the pipeline, we can assume all other optimizations are done, and we modify loops
        // into shapes that might confuse other passes

        // while (1) { .. if (..) { break } } ==> do { .. } while(..)
        Ref stats = node[2][1];
        Ref last = stats->back();
        if (!!last && last[0] == IF && (last->size() < 4 || !last[3]) && last[2][0] == BLOCK && !!last[2][1][0]) {
          Ref lastStats = last[2][1];
          int lastNum = lastStats->size();
          Ref lastLast = lastStats[lastNum-1];
          if (!(lastLast[0] == BREAK && !lastLast[1])) return;// if not a simple break, dangerous
          for (int i = 0; i < lastNum; i++) {
            if (lastStats[i][0] != STAT && lastStats[i][0] != BREAK) return; // something dangerous
          }
          // ok, a bunch of statements ending in a break
          bool abort = false;
          int stack = 0;
          int breaks = 0;
          traversePrePost(stats, [&](Ref node) {
            Ref type = node[0];
            if (type == CONTINUE) {
              if (stack == 0 || !!node[1]) { // abort if labeled (we do not analyze labels here yet), or a continue directly on us
                abort = true;
              }
            } else if (type == BREAK) {
              if (stack == 0 || !!node[1]) { // relevant if labeled (we do not analyze labels here yet), or a break directly on us
                breaks++;
              }
            } else if (LOOP.has(type)) {
              stack++;
            }
          }, [&](Ref node) {
            if (LOOP.has(node[0])) {
              stack--;
            }
          });
          if (abort) return;
          assert(breaks > 0);
          if (lastStats->size() > 1 && breaks != 1) return; // if we have code aside from the break, we can only move it out if there is just one break
          if (statsStack.size() < 1) return; // no chance we have this stats on hand
          // start to optimize
          if (lastStats->size() > 1) {
            Ref parent = statsStack.back();
            int me = parent->indexOf(node);
            if (me < 0) return; // not always directly on a stats, could be in a label for example
            parent->insert(me+1, lastStats->size()-1);
            for (size_t i = 0; i+1 < lastStats->size(); i++) {
              parent[me+1+i] = lastStats[i];
            }
          }
          Ref conditionToBreak = last[1];
          stats->pop_back();
          node[0]->setString(DO);
          node[1] = simplifyNotCompsDirect(make2(UNARY_PREFIX, L_NOT, conditionToBreak));
        }
      } else if (type == BINARY) {
        if (node[1] == AND) {
          if (node[3][0] == UNARY_PREFIX && node[3][1] == MINUS && node[3][2][0] == NUM && node[3][2][1]->getNumber() == 1) {
            // Change &-1 into |0, at this point the hint is no longer needed
            node[1]->setString(OR);
            node[3] = node[3][2];
            node[3][1]->setNumber(0);
          }
        } else if (node[1] == MINUS && node[3][0] == UNARY_PREFIX) {
          // avoid X - (-Y) because some minifiers buggily emit X--Y which is invalid as -- can be a unary. Transform to
          //        X + Y
          if (node[3][1] == MINUS) { // integer
            node[1]->setString(PLUS);
            node[3] = node[3][2];
          } else if (node[3][1] == PLUS) { // float
            if (node[3][2][0] == UNARY_PREFIX && node[3][2][1] == MINUS) {
              node[1]->setString(PLUS);
              node[3][2] = node[3][2][2];
            }
          }
        }
      }
    }, [&](Ref node) {
      if (statsStack.size() > 0) {
        Ref stats = getStatements(node);
        if (!!stats) statsStack.pop_back();
      }
    });
    // convert  { singleton }  into  singleton
    traversePre(fun, [](Ref node) {
      if (node[0] == BLOCK && !!getStatements(node) && node[1]->size() == 1) {
        safeCopy(node, node[1][0]);
      }
    });
    // convert L: do { .. } while(0) into L: { .. }
    traversePre(fun, [](Ref node) {
      if (node[0] == LABEL && node[1]->isString() /* careful of var label = 5 */ &&
          node[2][0] == DO && node[2][1][0] == NUM && node[2][1][1]->getNumber() == 0 && node[2][2][0] == BLOCK) {
        // there shouldn't be any continues on this, not direct break or continue
        IString label = node[1]->getIString();
        bool abort = false;
        int breakCaptured = 0, continueCaptured = 0;
        traversePrePost(node[2][2], [&](Ref node) {
          if (node[0] == CONTINUE) {
            if (!node[1] && !continueCaptured) {
              abort = true;
            } else if (node[1]->isString() && node[1]->getIString() == label) {
              abort = true;
            }
          }
          if (node[0] == BREAK && !node[1] && !breakCaptured) {
            abort = true;
          }
          if (BREAK_CAPTURERS.has(node[0])) {
            breakCaptured++;
          }
          if (CONTINUE_CAPTURERS.has(node[0])) {
            continueCaptured++;
          }
        }, [&](Ref node) {
          if (BREAK_CAPTURERS.has(node[0])) {
            breakCaptured--;
          }
          if (CONTINUE_CAPTURERS.has(node[0])) {
            continueCaptured--;
          }
        });
        if (abort) return;
        safeCopy(node[2], node[2][2]);
      }
    });
  });
}

// Contrary to the name this does not eliminate actual dead functions, only
// those marked as such with DEAD_FUNCTIONS
void eliminateDeadFuncs(Ref ast) {
  assert(!!extraInfo);
  IString DEAD_FUNCTIONS("dead_functions");
  IString ABORT("abort");
  assert(extraInfo->has(DEAD_FUNCTIONS));
  StringSet deadFunctions;
  for (size_t i = 0; i < extraInfo[DEAD_FUNCTIONS]->size(); i++) {
    deadFunctions.insert(extraInfo[DEAD_FUNCTIONS][i]->getIString());
  }
  traverseFunctions(ast, [&](Ref fun) {
    if (!deadFunctions.has(fun[1].get()->getIString())) {
      return;
    }
    AsmData asmData(fun);
    fun[3]->setSize(1);
    fun[3][0] = make1(STAT, make2(CALL, makeName(ABORT), &(makeArray(1))->push_back(makeNum(-1))));
    asmData.vars.clear();
    asmData.denormalize();
  });
}

