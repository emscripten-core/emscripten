
#include "simple_ast.h"
#include "wasm.h"
#include "optimizer.h"

#include "optimizer-shared.cpp"

using namespace cashew;
using namespace wasm;

// Utilities

IString GLOBAL("global"), NAN_("NaN"), INFINITY_("Infinity"),
        TOPMOST("topmost"),
        INT8ARRAY("Int8Array"),
        INT16ARRAY("Int16Array"),
        INT32ARRAY("Int32Array"),
        UINT8ARRAY("Uint8Array"),
        UINT16ARRAY("Uint16Array"),
        UINT32ARRAY("Uint32Array"),
        FLOAT32ARRAY("Float32Array"),
        FLOAT64ARRAY("Float64Array"),
        IMPOSSIBLE_CONTINUE("impossible-continue"),
        MATH("Math"),
        IMUL("imul");


static void abort_on(std::string why) {
  std::cerr << why << '\n';
  abort();
}
static void abort_on(std::string why, int x) {
  std::cerr << why << ' ' << x << '\n';
  abort();
}
static void abort_on(std::string why, Ref element) {
  std::cerr << why << ' ';
  element->stringify(std::cerr);
  std::cerr << '\n';
  abort();
}
static void abort_on(std::string why, IString element) {
  std::cerr << why << ' ' << element.str << '\n';
  abort();
}

// useful when we need to see our parent, in an expression stack
struct AstStackHelper {
  static std::vector<Ref> astStack;
  AstStackHelper(Ref curr) {
    astStack.push_back(curr);
  }
  ~AstStackHelper() {
    astStack.pop_back();
  }
  Ref getParent() {
    assert(astStack.size() >= 2);
    return astStack[astStack.size()-2];
  }
};

std::vector<Ref> AstStackHelper::astStack;

//
// Asm2WasmModule - converts an asm.js module into WebAssembly
//

class Asm2WasmModule : public wasm::Module {
  wasm::Arena allocator;

  // globals

  unsigned nextGlobal; // next place to put a global
  unsigned maxGlobal; // highest address we can put a global
  struct MappedGlobal {
    unsigned address;
    BasicType type;
    bool import; // if true, this is an import - we should read the value, not just set a zero
    MappedGlobal() : address(0), type(none), import(false) {}
    MappedGlobal(unsigned address, BasicType type, bool import) : address(address), type(type), import(import) {}
  };
  std::map<IString, MappedGlobal> mappedGlobals;

  void allocateGlobal(IString name, BasicType type, bool import) {
    assert(mappedGlobals.find(name) == mappedGlobals.end());
    mappedGlobals.emplace(name, MappedGlobal(nextGlobal, type, import));
    nextGlobal += 8;
    assert(nextGlobal < maxGlobal);
  }

  struct View {
    unsigned bytes;
    bool integer, signed_;
    View() : bytes(0) {}
    View(unsigned bytes, bool integer, bool signed_) : bytes(bytes), integer(integer), signed_(signed_) {}
  };

  std::map<IString, View> views; // name (e.g. HEAP8) => view info
  IString Math_imul; // imported name of Math.imul

  // function types. we fill in this information as we see
  // uses, in the first pass

  std::map<IString, FunctionType> importedFunctionTypes;

  void noteImportedFunctionCall(Ref ast, Ref parent, AsmData *asmData) {
    assert(ast[0] == CALL && ast[1][0] == NAME);
    IString importName = ast[1][1]->getIString();
    FunctionType type;
    type.name = IString((std::string("type$") + importName.str).c_str(), false); // TODO: make a list of such types
    type.result = detectWasmType(parent, asmData);
    Ref args = ast[2];
    for (unsigned i = 0; i < args->size(); i++) {
      type.params.push_back(detectWasmType(args[i], asmData));
    }
    // if we already saw this signature, verify it's the same (or else handle that)
    if (importedFunctionTypes.find(importName) != importedFunctionTypes.end()) {
      FunctionType& previous = importedFunctionTypes[importName];
#if 0
      std::cout << "compare " << importName.str << "\nfirst: ";
      type.print(std::cout, 0);
      std::cout << "\nsecond: ";
      previous.print(std::cout, 0) << ".\n";
#endif
      if (type != previous) {
        // merge it in. we'll add on extra 0 parameters for ones not actually used, etc.
        for (size_t i = 0; i < type.params.size(); i++) {
          if (previous.params.size() > i) {
            if (previous.params[i] == none) {
              previous.params[i] = type.params[i]; // use a more concrete type
            } else {
              previous.params.push_back(type.params[i]); // add a new param
            }
          }
        }
        if (previous.result == none) {
          previous.result = type.result; // use a more concrete type
        }
      }
    } else {
      importedFunctionTypes[importName] = type;
    }
  }

public:
  Asm2WasmModule() : nextGlobal(8), maxGlobal(1000) {}
  void processAsm(Ref ast);

private:
  BasicType asmToWasmType(AsmType asmType) {
    switch (asmType) {
      case ASM_INT: return BasicType::i32;
      case ASM_DOUBLE: return BasicType::f64;
      case ASM_NONE: return BasicType::none;
      default: abort_on("confused detectWasmType", asmType);
    }
  }

  BasicType detectWasmType(Ref ast, AsmData *data) {
    return asmToWasmType(detectType(ast, data));
  }

  bool isInteger(double num) {
    return fmod(num, 1) == 0 && double(int(num)) == num;
  }
  bool isInteger(Ref ast) {
    return ast[0] == NUM && isInteger(ast[1]->getNumber());
  }

  bool isUnsignedCoercion(Ref ast) { // TODO: use detectSign?
    if (ast[0] == BINARY && ast[1] == TRSHIFT) return true;
    return false;
  }

  // an asm.js binary op can either be a binary or a relational in wasm
  bool parseAsmBinaryOp(IString op, Ref left, Ref right, BinaryOp &binary, RelationalOp &relational, AsmData *asmData) {
    if (op == PLUS) { binary = BinaryOp::Add; return true; }
    if (op == MINUS) { binary = BinaryOp::Sub; return true; }
    if (op == MUL) { binary = BinaryOp::Mul; return true; }
    if (op == AND) { binary = BinaryOp::And; return true; }
    if (op == OR) { binary = BinaryOp::Or; return true; }
    if (op == XOR) { binary = BinaryOp::Xor; return true; }
    if (op == LSHIFT) { binary = BinaryOp::Shl; return true; }
    if (op == RSHIFT) { binary = BinaryOp::ShrS; return true; }
    if (op == TRSHIFT) { binary = BinaryOp::ShrU; return true; }
    if (op == EQ) { relational = RelationalOp::Eq; return false; }
    if (op == NE) { relational = RelationalOp::Ne; return false; }
    BasicType leftType = detectWasmType(left, asmData);
#if 0
    std::cout << "CHECK\n";
    left->stringify(std::cout);
    std::cout << " => ";
    printBasicType(std::cout, leftType);
    std::cout << '\n';
    right->stringify(std::cout);
    std::cout << " => ";
    printBasicType(std::cout, detectWasmType(right, asmData));
#endif
    bool isInteger = leftType == BasicType::i32;
    bool isUnsigned = isUnsignedCoercion(left) || isUnsignedCoercion(right);
    if (op == DIV) {
      if (isInteger) {
        { binary = isUnsigned ? BinaryOp::DivU : BinaryOp::DivS; return true; }
      }
      { binary = BinaryOp::Div; return true; }
    }
    if (op == MOD) {
      if (isInteger) {
        { binary = isUnsigned ? BinaryOp::RemU : BinaryOp::RemS; return true; }
      }
      abort_on("non-integer rem");
    }
    if (op == GE) {
      if (isInteger) {
        { relational = isUnsigned ? RelationalOp::GeU : RelationalOp::GeS; return false; }
      }
      { relational = RelationalOp::Ge; return false; }
    }
    if (op == GT) {
      if (isInteger) {
        { relational = isUnsigned ? RelationalOp::GtU : RelationalOp::GtS; return false; }
      }
      { relational = RelationalOp::Gt; return false; }
    }
    if (op == LE) {
      if (isInteger) {
        { relational = isUnsigned ? RelationalOp::LeU : RelationalOp::LeS; return false; }
      }
      { relational = RelationalOp::Le; return false; }
    }
    if (op == LT) {
      if (isInteger) {
        { relational = isUnsigned ? RelationalOp::LtU : RelationalOp::LtS; return false; }
      }
      { relational = RelationalOp::Lt; return false; }
    }
    abort_on("bad wasm binary op", op);
  }

  unsigned bytesToShift(unsigned bytes) {
    switch (bytes) {
      case 1: return 0;
      case 2: return 1;
      case 4: return 2;
      case 8: return 3;
      default: abort();
    }
  }

  // function table null thunks are always named b\d+
  bool isNullThunk(IString name) {
    const char *str = name.str;
    if (*str != 'b') return false;
    str++;
    while (1) {
      if (*str < '0' || *str > '9') return false;
      str++;
      if (*str == 0) break;
    }
    return true;
  }

  wasm::Arena tempAllocator;

  std::map<unsigned, Ref> tempNums;

  Literal getLiteral(Ref ast) {
    if (ast[0] == NUM) {
      return Literal((int32_t)ast[1]->getInteger());
    } else if (ast[0] == UNARY_PREFIX) {
      assert(ast[1] == MINUS && ast[2][0] == NUM);
      return Literal((int32_t)-ast[2][1]->getInteger());
    }
    abort();
  }

  Function* processFunction(Ref ast);
};

void Asm2WasmModule::processAsm(Ref ast) {
  assert(ast[0] == TOPLEVEL);
  Ref asmFunction = ast[1][0];
  assert(asmFunction[0] == DEFUN);
  Ref body = asmFunction[3];
  assert(body[0][0] == STAT && body[0][1][0] == STRING && body[0][1][1]->getIString() == IString("use asm"));

  auto addImport = [&](IString name, Ref imported, BasicType type) {
    assert(imported[0] == DOT);
    Ref module = imported[1];
    if (module[0] == DOT) {
      // we can have (global.Math).floor; skip the 'Math'
      assert(module[1][0] == NAME);
      if (module[2] == MATH && imported[2] == IMUL) {
        assert(Math_imul.isNull());
        Math_imul = name;
        return;
      }
      module = module[1];
    }
    assert(module[0] == NAME);
    Import import;
    import.name = name;
    import.module = module[1]->getIString();
    import.base = imported[2]->getIString();
    // special-case some asm builtins
    if (import.module == GLOBAL && (import.base == NAN_ || import.base == INFINITY_)) {
      type = BasicType::f64;
    }
    if (type != BasicType::none) {
      // wasm has no imported constants, so allocate a global, and we need to write the value into that
      allocateGlobal(name, type, true);
    } else {
      imports.emplace(name, import);
    }
  };

  // first pass - do almost everything, but function imports

  for (unsigned i = 1; i < body->size(); i++) {
    Ref curr = body[i];
    if (curr[0] == VAR) {
      // import, global, or table
      for (unsigned j = 0; j < curr[1]->size(); j++) {
        Ref pair = curr[1][j];
        IString name = pair[0]->getIString();
        Ref value = pair[1];
        if (value[0] == NUM) {
          // global int
          assert(value[1]->getNumber() == 0);
          allocateGlobal(name, BasicType::i32, false);
        } else if (value[0] == BINARY) {
          // int import
          assert(value[1] == OR && value[3][0] == NUM && value[3][1]->getNumber() == 0);
          Ref import = value[2]; // env.what
          addImport(name, import, BasicType::i32);
        } else if (value[0] == UNARY_PREFIX) {
          // double import or global
          assert(value[1] == PLUS);
          Ref import = value[2];
          if (import[0] == NUM) {
            // global
            assert(import[1]->getNumber() == 0);
            allocateGlobal(name, BasicType::f64, false);
          } else {
            // import
            addImport(name, import, BasicType::f64);
          }
        } else if (value[0] == DOT) {
          // function import
          addImport(name, value, BasicType::none);
        } else if (value[0] == NEW) {
          // ignore imports of typed arrays, but note the names of the arrays
          value = value[1];
          assert(value[0] == CALL);
          Ref constructor = value[1];
          assert(constructor[0] == DOT); // global.*Array
          IString heap = constructor[2]->getIString();
          unsigned bytes;
          bool integer, signed_;
          if (heap == INT8ARRAY) {
            bytes = 1; integer = true; signed_ = true;
          } else if (heap == INT16ARRAY) {
            bytes = 2; integer = true; signed_ = true;
          } else if (heap == INT32ARRAY) {
            bytes = 4; integer = true; signed_ = true;
          } else if (heap == UINT8ARRAY) {
            bytes = 1; integer = true; signed_ = false;
          } else if (heap == UINT16ARRAY) {
            bytes = 2; integer = true; signed_ = false;
          } else if (heap == UINT32ARRAY) {
            bytes = 4; integer = true; signed_ = false;
          } else if (heap == FLOAT32ARRAY) {
            bytes = 4; integer = false; signed_ = true;
          } else if (heap == FLOAT64ARRAY) {
            bytes = 8; integer = false; signed_ = true;
          }
          assert(views.find(name) == views.end());
          views.emplace(name, View(bytes, integer, signed_));
        } else if (value[0] == ARRAY) {
          // function table. we "merge" them, so e.g.   [foo, b1] , [b2, bar]  =>  [foo, bar] , assuming b* are the aborting thunks
          // TODO: we can drop some b*s at the end of the table
          Ref contents = value[1];
          for (unsigned k = 0; k < contents->size(); k++) {
            IString curr = contents[k][1]->getIString();
            if (table.vars.size() <= k) {
              table.vars.push_back(curr);
            } else {
              if (isNullThunk(table.vars[k])) {
                table.vars[k] = curr;
              } else {
                assert(isNullThunk(curr) && "cannot have aliasing function pointers");
              }
            }
          }
        } else {
          abort_on("invalid var element", pair);
        }
      }
    } else if (curr[0] == DEFUN) {
      // function
      functions.push_back(processFunction(curr));
    } else if (curr[0] == RETURN) {
      // exports
      Ref object = curr[1];
      Ref contents = curr[1][1];
      for (unsigned k = 0; k < contents->size(); k++) {
        Ref pair = contents[k];
        IString key = pair[0]->getIString();
        Ref value = pair[1];
        assert(value[0] == NAME);
        Export export_;
        export_.name = key;
        export_.value = value[1]->getIString();
        exports.push_back(export_);
      }
    }
  }

  // second pass - function imports

  std::vector<IString> toErase;

  for (auto& pair : imports) {
    IString name = pair.first;
    Import& import = pair.second;
    if (importedFunctionTypes.find(name) != importedFunctionTypes.end()) {
      import.type = importedFunctionTypes[name];
    } else {
      // never actually used
      toErase.push_back(name);
    }
  }

  for (auto curr : toErase) {
    imports.erase(curr);
  }

  // cleanups

  tempAllocator.clear();
}

Function* Asm2WasmModule::processFunction(Ref ast) {
  auto function = allocator.alloc<Function>();
  function->name = ast[1]->getIString();
  Ref params = ast[2];
  Ref body = ast[3];

  unsigned nextId = 0;
  auto getNextId = [&nextId](std::string prefix) {
    return IString((prefix + '$' + std::to_string(nextId++)).c_str(), false);
  };

  // given an asm.js label, returns the wasm label for breaks or continues
  auto getBreakLabelName = [](IString label) {
    return IString((std::string("label$break$") + label.str).c_str(), false);
  };
  auto getContinueLabelName = [](IString label) {
    return IString((std::string("label$continue$") + label.str).c_str(), false);
  };

  IStringSet functionVariables; // params or locals 

  IString parentLabel; // set in LABEL, then read in WHILE/DO
  std::vector<IString> breakStack; // where a break will go
  std::vector<IString> continueStack; // where a continue will go

  AsmData asmData; // need to know var and param types, for asm type detection

  for (unsigned i = 0; i < params->size(); i++) {
    Ref curr = body[i];
    assert(curr[0] == STAT);
    curr = curr[1];
    assert(curr[0] == ASSIGN && curr[2][0] == NAME);
    IString name = curr[2][1]->getIString();
    AsmType asmType = detectType(curr[3]);
    function->params.emplace_back(name, asmToWasmType(asmType));
    functionVariables.insert(name);
    asmData.addParam(name, asmType);
  }
  unsigned start = params->size();
  while (start < body->size() && body[start][0] == VAR) {
    Ref curr = body[start];
    for (unsigned j = 0; j < curr[1]->size(); j++) {
      Ref pair = curr[1][j];
      IString name = pair[0]->getIString();
      AsmType asmType = detectType(pair[1], nullptr, true);
      function->locals.emplace_back(name, asmToWasmType(asmType));
      functionVariables.insert(name);
      asmData.addVar(name, asmType);
    }
    start++;
  }

  bool seenReturn = false; // function->result is updated if we see a return
  bool needTopmost = false; // we label the topmost b lock if we need one for a return
  // processors
  std::function<Expression* (Ref, unsigned)> processStatements;
  std::function<Expression* (Ref, unsigned)> processUnshifted;

  bool debug = !!getenv("ASM2WASM_DEBUG") && getenv("ASM2WASM_DEBUG")[0] != '0';

  std::function<Expression* (Ref)> process = [&](Ref ast) -> Expression* {
    AstStackHelper astStackHelper(ast); // TODO: only create one when we need it?
    if (debug) {
      std::cout << "at: ";
      ast->stringify(std::cout);
      std::cout << '\n';
    }
    IString what = ast[0]->getIString();
    if (what == STAT) {
      return process(ast[1]); // and drop return value, if any
    } else if (what == ASSIGN) {
      if (ast[2][0] == NAME) {
        auto ret = allocator.alloc<SetLocal>();
        ret->id = ast[2][1]->getIString();
        ret->value = process(ast[3]);
        ret->type = ret->value->type;
        return ret;
      } else if (ast[2][0] == SUB) {
        Ref target = ast[2];
        assert(target[1][0] == NAME);
        IString heap = target[1][1]->getIString();
        assert(views.find(heap) != views.end());
        View& view = views[heap];
        auto ret = allocator.alloc<Store>();
        ret->bytes = view.bytes;
        ret->float_ = !view.integer;
        ret->offset = 0;
        ret->align = view.bytes;
        ret->ptr = processUnshifted(target[2], view.bytes);
        ret->value = process(ast[3]);
        ret->type = ret->value->type;
        return ret;
      }
      abort_on("confusing assign", ast);
    } else if (what == BINARY) {
      if (ast[1] == OR && ast[3][0] == NUM && ast[3][1]->getNumber() == 0) {
        return process(ast[2]); // just look through the ()|0 coercion
      }
      BinaryOp binary;
      RelationalOp relational;
      bool isBinary = parseAsmBinaryOp(ast[1]->getIString(), ast[2], ast[3], binary, relational, &asmData);
      if (isBinary) {
        auto ret = allocator.alloc<Binary>();
        ret->op = binary;
        ret->left = process(ast[2]);
        ret->right = process(ast[3]);
        ret->type = ret->left->type;
        return ret;
      } else {
        auto ret = allocator.alloc<Compare>();
        ret->op = relational;
        ret->left = process(ast[2]);
        ret->right = process(ast[3]);
        return ret;
      }
    } else if (what == NUM) {
      auto ret = allocator.alloc<Const>();
      double num = ast[1]->getNumber();
      if (isInteger(num)) {
        ret->value.type = BasicType::i32;
        ret->value.i32 = num;
      } else {
        ret->value.type = BasicType::f64;
        ret->value.f64 = num;
      }
      ret->type = ret->value.type;
      return ret;
    } else if (what == NAME) {
      IString name = ast[1]->getIString();
      if (functionVariables.has(name)) {
        // var in scope
        auto ret = allocator.alloc<GetLocal>();
        ret->id = name;
        ret->type = asmToWasmType(asmData.getType(name));
        return ret;
      }
      // global var, do a load from memory
      assert(mappedGlobals.find(name) != mappedGlobals.end());
      MappedGlobal global = mappedGlobals[name];
      auto ret = allocator.alloc<Load>();
      ret->bytes = getBasicTypeSize(global.type);
      ret->signed_ = true; // but doesn't matter
      ret->float_ = isFloat(global.type);
      ret->offset = 0;
      ret->align = ret->bytes;
      auto ptr = allocator.alloc<Const>();
      ptr->value.type = BasicType::i32; // XXX for wasm64, need 64
      ptr->value.i32 = global.address;
      ret->ptr = ptr;
      ret->type = global.type;
      return ret;
    } else if (what == SUB) {
      Ref target = ast[1];
      assert(target[0] == NAME);
      IString heap = target[1]->getIString();
      assert(views.find(heap) != views.end());
      View& view = views[heap];
      auto ret = allocator.alloc<Load>();
      ret->bytes = view.bytes;
      ret->signed_ = view.signed_;
      ret->float_ = !view.integer;
      ret->offset = 0;
      ret->align = view.bytes;
      ret->ptr = processUnshifted(ast[2], view.bytes);
      ret->type = getBasicType(view.bytes, !view.integer);
      return ret;
    } else if (what == UNARY_PREFIX) {
      if (ast[1] == PLUS) {
        if (ast[2][0] == NUM) {
          auto ret = allocator.alloc<Const>();
          ret->value.type = BasicType::f64;
          ret->value.f64 = ast[2][1]->getNumber();
          ret->type = ret->value.type;
          return ret;
        }
        AsmType childType = detectType(ast[2], &asmData);
        if (childType == ASM_INT) {
          auto ret = allocator.alloc<Convert>();
          ret->op = isUnsignedCoercion(ast[2]) ? ConvertUInt32 : ConvertSInt32;
          ret->value = process(ast[2]);
          ret->type = BasicType::i32;
          return ret;
        }
        assert(childType == ASM_NONE); // e.g. a coercion on a call
        return process(ast[2]); // just look through the +() coercion
      } else if (ast[1] == MINUS) {
        if (ast[2][0] == NUM) {
          auto ret = allocator.alloc<Const>();
          ret->value = getLiteral(ast);
          ret->type = ret->value.type;
          return ret;
        }
        AsmType asmType = detectType(ast[2], &asmData);
        if (asmType == ASM_INT) {
          // wasm has no unary negation for int, so do 0-
          auto ret = allocator.alloc<Binary>();
          ret->op = Sub;
          ret->left = allocator.alloc<Const>()->set(Literal((int32_t)0));
          ret->right = process(ast[2]);
          ret->type = BasicType::i32;
          return ret;
        }
        assert(asmType == ASM_DOUBLE);
        auto ret = allocator.alloc<Unary>();
        ret->op = Neg;
        ret->value = process(ast[2]);
        ret->type = BasicType::f64;
        return ret;
      } else if (ast[1] == B_NOT) {
        // ~, might be ~~ as a coercion or just a not
        if (ast[2][0] == UNARY_PREFIX && ast[2][1] == B_NOT) {
          auto ret = allocator.alloc<Convert>();
          ret->op = TruncSFloat64; // equivalent to U, except for error handling, which asm.js doesn't have anyhow
          ret->value = process(ast[2][2]);
          ret->type = BasicType::i32;
          return ret;
        }
        // no bitwise unary not, so do xor with -1
        auto ret = allocator.alloc<Binary>();
        ret->op = Xor;
        ret->left = process(ast[2]);
        ret->right = allocator.alloc<Const>()->set(Literal(int32_t(-1)));
        ret->type = BasicType::i32;
        return ret;
      } else if (ast[1] == L_NOT) {
        // no logical unary not, so do == 0
        auto ret = allocator.alloc<Compare>();
        ret->op = Eq;
        ret->left = process(ast[2]);
        ret->right = allocator.alloc<Const>()->set(Literal(0));
        return ret;
      }
      abort_on("bad unary", ast);
    } else if (what == IF) {
      auto ret = allocator.alloc<If>();
      ret->condition = process(ast[1]);
      ret->ifTrue = process(ast[2]);
      ret->ifFalse = !!ast[3] ? process(ast[3]) : nullptr;
      return ret;
    } else if (what == CALL) {
      if (ast[1][0] == NAME) {
        IString name = ast[1][1]->getIString();
        if (name == Math_imul) {
          auto ret = allocator.alloc<Binary>();
          ret->op = Mul;
          ret->left = process(ast[2][0]);
          ret->right = process(ast[2][1]);
          ret->type = BasicType::i32;
          return ret;
        }
        Call* ret;
        if (imports.find(name) != imports.end()) {
          ret = allocator.alloc<CallImport>();
          noteImportedFunctionCall(ast, astStackHelper.getParent(), &asmData);
        } else {
          ret = allocator.alloc<Call>();
        }
        ret->target = name;
        Ref args = ast[2];
        for (unsigned i = 0; i < args->size(); i++) {
          ret->operands.push_back(process(args[i]));
        }
        return ret;
      }
      // function pointers
      auto ret = allocator.alloc<CallIndirect>();
      Ref target = ast[1];
      assert(target[0] == SUB && target[1][0] == NAME && target[2][0] == BINARY && target[2][1] == AND && target[2][3][0] == NUM); // FUNCTION_TABLE[(expr) & mask]
      ret->target = process(target[2][2]);
      Ref args = ast[2];
      for (unsigned i = 0; i < args->size(); i++) {
        ret->operands.push_back(process(args[i]));
      }
      return ret;
    } else if (what == RETURN) {
      BasicType type = !!ast[1] ? detectWasmType(ast[1], &asmData) : none;
      if (seenReturn) {
        assert(function->result == type);
      } else {
        function->result = type;
      }
      // wasm has no return, so we just break on the topmost block
      needTopmost = true;
      auto ret = allocator.alloc<Break>();
      ret->var = TOPMOST;
      ret->condition = nullptr;
      ret->value = !!ast[1] ? process(ast[1]) : nullptr;
      return ret;
    } else if (what == BLOCK) {
      return processStatements(ast[1], 0);
    } else if (what == BREAK) {
      auto ret = allocator.alloc<Break>();
      assert(breakStack.size() > 0);
      ret->var = !!ast[1] ? getBreakLabelName(ast[1]->getIString()) : breakStack.back();
      ret->condition = nullptr;
      ret->value = nullptr;
      return ret;
    } else if (what == CONTINUE) {
      auto ret = allocator.alloc<Break>();
      assert(continueStack.size() > 0);
      ret->var = !!ast[1] ? getContinueLabelName(ast[1]->getIString()) : continueStack.back();
      ret->condition = nullptr;
      ret->value = nullptr;
      return ret;
    } else if (what == WHILE) {
      bool forever = ast[1][0] == NUM && ast[1][1]->getInteger() == 1;
      auto ret = allocator.alloc<Loop>();
      IString out, in;
      if (!parentLabel.isNull()) {
        out = getBreakLabelName(parentLabel);
        in = getContinueLabelName(parentLabel);
        parentLabel = IString();
      } else {
        out = getNextId("while-out");
        in = getNextId("while-in");
      }
      ret->out = out;
      ret->in = in;
      breakStack.push_back(out);
      continueStack.push_back(in);
      if (forever) {
        ret->body = process(ast[2]);
      } else {
        Break *continueWhile = allocator.alloc<Break>();
        continueWhile->var = in;
        continueWhile->condition = process(ast[1]);
        continueWhile->value = nullptr;
        auto body = allocator.alloc<Block>();
        body->list.push_back(continueWhile);
        body->list.push_back(process(ast[2]));
        ret->body = body;
      }
      continueStack.pop_back();
      breakStack.pop_back();
      return ret;
    } else if (what == DO) {
      if (ast[1][0] == NUM && ast[1][1]->getNumber() == 0) {
        // one-time loop
        auto block = allocator.alloc<Block>();
        IString stop;
        if (!parentLabel.isNull()) {
          stop = getBreakLabelName(parentLabel);
          parentLabel = IString();
        } else {
          stop = getNextId("do-once");
        }
        block->var = stop;
        breakStack.push_back(stop);
        continueStack.push_back(IMPOSSIBLE_CONTINUE);
        block->list.push_back(process(ast[2]));
        continueStack.pop_back();
        breakStack.pop_back();
        return block;
      }
      // general do-while loop
      auto ret = allocator.alloc<Loop>();
      IString out, in;
      if (!parentLabel.isNull()) {
        out = getBreakLabelName(parentLabel);
        in = getContinueLabelName(parentLabel);
        parentLabel = IString();
      } else {
        out = getNextId("do-out");
        in = getNextId("do-in");
      }
      ret->out = out;
      ret->in = in;
      breakStack.push_back(out);
      continueStack.push_back(in);
      ret->body = process(ast[2]);
      continueStack.pop_back();
      breakStack.pop_back();
      Break *continueIf = allocator.alloc<Break>();
      continueIf->var = in;
      continueIf->condition = process(ast[1]);
      continueIf->value = nullptr;
      if (Block *block = dynamic_cast<Block*>(ret->body)) {
        block->list.push_back(continueIf);
      } else {
        auto newBody = allocator.alloc<Block>();
        newBody->list.push_back(ret->body);
        newBody->list.push_back(continueIf);
        ret->body = newBody;
      }
      return ret;
    } else if (what == LABEL) {
      parentLabel = ast[1]->getIString();
      return process(ast[2]);
    } else if (what == CONDITIONAL) {
      auto ret = allocator.alloc<If>();
      ret->condition = process(ast[1]);
      ret->ifTrue = process(ast[2]);
      ret->ifFalse = process(ast[3]);
      return ret;
    } else if (what == SEQ) {
      auto ret = allocator.alloc<Block>();
      ret->list.push_back(process(ast[1]));
      ret->list.push_back(process(ast[2]));
      return ret;
    } else if (what == SWITCH) {
      IString name = getNextId("switch");
      breakStack.push_back(name);
      auto ret = allocator.alloc<Switch>();
      ret->var = name;
      ret->value =  process(ast[1]);
      Ref cases = ast[2];
      for (unsigned i = 0; i < cases->size(); i++) {
        Ref curr = cases[i];
        Ref condition = curr[0];
        Ref body = curr[1];
        if (condition->isNull()) {
          ret->default_ = processStatements(body, 0);
        } else {
          assert(condition[0] == NUM || condition[0] == UNARY_PREFIX);
          Switch::Case case_;
          case_.value = getLiteral(condition);
          case_.body = processStatements(body, 0);
          case_.fallthru = false; // XXX we assume no fallthru, ever
          ret->cases.push_back(case_);
        }
      }
      breakStack.pop_back();
      return ret;
    }
    abort_on("confusing expression", ast);
  };

  // given HEAP32[addr >> 2], we need an absolute address, and would like to remove that shift.
  // if there is a shift, we can just look through it, etc.
  processUnshifted = [&](Ref ptr, unsigned bytes) {
    unsigned shifts = bytesToShift(bytes);
    if (ptr[0] == BINARY && ptr[1] == RSHIFT && ptr[3][0] == NUM && ptr[3][1]->getInteger() == shifts) {
      return process(ptr[2]); // look through it
    } else if (ptr[0] == NUM) {
      // constant, apply a shift (e.g. HEAP32[1] is address 4)
      unsigned addr = ptr[1]->getInteger();
      unsigned shifted = addr << shifts;
      auto ret = allocator.alloc<Const>();
      ret->value.type = BasicType::i32;
      ret->value.i32 = shifted;
      return (Expression*)ret;
    }
    abort_on("bad processUnshifted", ptr);
  };

  processStatements = [&](Ref ast, unsigned from) {
    auto block = allocator.alloc<Block>();
    for (unsigned i = from; i < ast->size(); i++) {
      block->list.push_back(process(ast[i]));
    }
    return block;
  };
  // body
  function->body = processStatements(body, start);
  if (needTopmost) {
    Block* topmost = dynamic_cast<Block*>(function->body);
    assert(topmost);
    topmost->var = TOPMOST;
  }
  // cleanups/checks
  assert(breakStack.size() == 0 && continueStack.size() == 0);

  return function;
}

int main(int argc, char **argv) {
  char *infile = argv[1];

  printf("loading '%s'...\n", infile);
  FILE *f = fopen(argv[1], "r");
  assert(f);
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  char *input = new char[size+1];
  rewind(f);
  int num = fread(input, 1, size, f);
  // On Windows, ftell() gives the byte position (\r\n counts as two bytes), but when
  // reading, fread() returns the number of characters read (\r\n is read as one char \n, and counted as one),
  // so return value of fread can be less than size reported by ftell, and that is normal.
  assert((num > 0 || size == 0) && num <= size);
  fclose(f);
  input[num] = 0;

  /*
  // Separate asm modules look like
  //
  //    Module["asm"] = (function(global, env, buffer) {
  //
  // , we can remove the part until the function.
  */

  printf("parsing...\n");
  cashew::Parser<Ref, ValueBuilder> builder;
  Ref asmjs = builder.parseToplevel(input);

  //asmjs->stringify(std::cout);
  //std::cout << '\n';

  printf("wasming...\n");
  Asm2WasmModule wasm;
  wasm.processAsm(asmjs);

  printf("printing...\n");
  wasm.print(std::cout);

  printf("done.\n");
  printf("TODO: get memory for globals, and clear it to zero; and read values for imports\n");
}

