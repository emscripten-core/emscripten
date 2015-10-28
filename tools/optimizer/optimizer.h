extern bool preciseF32,
            receiveJSON,
            emitJSON,
            minifyWhitespace,
            last;

extern Ref extraInfo;

void eliminateDeadFuncs(Ref ast);
void eliminate(Ref ast, bool memSafe=false);
void eliminateMemSafe(Ref ast);
void simplifyExpressions(Ref ast);
void optimizeFrounds(Ref ast);
void simplifyIfs(Ref ast);
void registerize(Ref ast);
void registerizeHarder(Ref ast);
void minifyLocals(Ref ast);
void asmLastOpts(Ref ast);

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
  ASM_NONE // number of types
};

struct AsmData;

AsmType detectType(Ref node, AsmData *asmData=nullptr, bool inVarDef=false);

struct AsmData {
  struct Local {
    Local() {}
    Local(AsmType type, bool param) : type(type), param(param) {}
    AsmType type;
    bool param; // false if a var
  };
  typedef std::unordered_map<IString, Local> Locals;

  Locals locals;
  std::vector<IString> params; // in order
  std::vector<IString> vars; // in order
  AsmType ret;

  Ref func;

  AsmType getType(const IString& name) {
    auto ret = locals.find(name);
    if (ret != locals.end()) return ret->second.type;
    return ASM_NONE;
  }
  void setType(const IString& name, AsmType type) {
    locals[name].type = type;
  }

  bool isLocal(const IString& name) {
    return locals.count(name) > 0;
  }
  bool isParam(const IString& name) {
    return isLocal(name) && locals[name].param;
  }
  bool isVar(const IString& name) {
    return isLocal(name) && !locals[name].param;
  }

  AsmData() {} // if you want to fill in the data yourself
  AsmData(Ref f); // if you want to read data from f, and modify it as you go (parallel to denormalize)

  void denormalize();

  void addParam(IString name, AsmType type) {
    locals[name] = Local(type, true);
    params.push_back(name);
  }
  void addVar(IString name, AsmType type) {
    locals[name] = Local(type, false);
    vars.push_back(name);
  }

  void deleteVar(IString name) {
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

extern IString ASM_FLOAT_ZERO;

extern IString SIMD_INT8X16_CHECK,
               SIMD_INT16X8_CHECK,
               SIMD_INT32X4_CHECK,
               SIMD_FLOAT32X4_CHECK,
               SIMD_FLOAT64X2_CHECK;

int parseInt(const char *str);

struct HeapInfo {
  bool valid, unsign, floaty;
  int bits;
  AsmType type;
};

HeapInfo parseHeap(const char *name);

