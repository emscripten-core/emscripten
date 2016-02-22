
#ifndef __optimizer_h__
#define __optimizer_h__

#include "simple_ast.h"

extern bool preciseF32,
            receiveJSON,
            emitJSON,
            minifyWhitespace,
            last;

extern cashew::Ref extraInfo;

void eliminateDeadFuncs(cashew::Ref ast);
void eliminate(cashew::Ref ast, bool memSafe=false);
void eliminateMemSafe(cashew::Ref ast);
void simplifyExpressions(cashew::Ref ast);
void optimizeFrounds(cashew::Ref ast);
void simplifyIfs(cashew::Ref ast);
void registerize(cashew::Ref ast);
void registerizeHarder(cashew::Ref ast);
void minifyLocals(cashew::Ref ast);
void asmLastOpts(cashew::Ref ast);

//

enum AsmType {
  ASM_INT = 0,
  ASM_DOUBLE,
  ASM_FLOAT,
  ASM_FLOAT32X4,
  ASM_FLOAT64X2,
  ASM_INT8X16,
  ASM_INT16X8,
  ASM_INT32X4,
  ASM_BOOL8X16,
  ASM_BOOL16X8,
  ASM_BOOL32X4,
  ASM_BOOL64X2,
  ASM_NONE // number of types
};

struct AsmData;

AsmType detectType(cashew::Ref node, AsmData *asmData=nullptr, bool inVarDef=false);

struct AsmData {
  struct Local {
    Local() {}
    Local(AsmType type, bool param) : type(type), param(param) {}
    AsmType type;
    bool param; // false if a var
  };
  typedef std::unordered_map<cashew::IString, Local> Locals;

  Locals locals;
  std::vector<cashew::IString> params; // in order
  std::vector<cashew::IString> vars; // in order
  AsmType ret;

  cashew::Ref func;

  AsmType getType(const cashew::IString& name) {
    auto ret = locals.find(name);
    if (ret != locals.end()) return ret->second.type;
    return ASM_NONE;
  }
  void setType(const cashew::IString& name, AsmType type) {
    locals[name].type = type;
  }

  bool isLocal(const cashew::IString& name) {
    return locals.count(name) > 0;
  }
  bool isParam(const cashew::IString& name) {
    return isLocal(name) && locals[name].param;
  }
  bool isVar(const cashew::IString& name) {
    return isLocal(name) && !locals[name].param;
  }

  AsmData() {} // if you want to fill in the data yourself
  AsmData(cashew::Ref f); // if you want to read data from f, and modify it as you go (parallel to denormalize)

  void denormalize();

  void addParam(cashew::IString name, AsmType type) {
    locals[name] = Local(type, true);
    params.push_back(name);
  }
  void addVar(cashew::IString name, AsmType type) {
    locals[name] = Local(type, false);
    vars.push_back(name);
  }

  void deleteVar(cashew::IString name) {
    locals.erase(name);
    for (size_t i = 0; i < vars.size(); i++) {
      if (vars[i] == name) {
        vars.erase(vars.begin() + i);
        break;
      }
    }
  }
};

bool isInteger(double x);

bool isInteger32(double x);

extern cashew::IString ASM_FLOAT_ZERO;

extern cashew::IString SIMD_INT8X16_CHECK,
               SIMD_INT16X8_CHECK,
               SIMD_INT32X4_CHECK,
               SIMD_FLOAT32X4_CHECK,
               SIMD_FLOAT64X2_CHECK,
               SIMD_BOOL8X16_CHECK,
               SIMD_BOOL16X8_CHECK,
               SIMD_BOOL32X4_CHECK,
               SIMD_BOOL64X2_CHECK;

int parseInt(const char *str);

struct HeapInfo {
  bool valid, unsign, floaty;
  int bits;
  AsmType type;
};

HeapInfo parseHeap(const char *name);

enum AsmSign {
  ASM_FLEXIBLE = 0, // small constants can be signed or unsigned, variables are also flexible
  ASM_SIGNED = 1,
  ASM_UNSIGNED = 2,
  ASM_NONSIGNED = 3,
};

extern AsmSign detectSign(cashew::Ref node);

#endif // __optimizer_h__

