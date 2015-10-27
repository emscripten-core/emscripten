
#include "simple_ast.h"
#include "wasm.h"

IString GLOBAL("global"), NAN_("NaN"), INFINITY_("Infinity"),
        TOPMOST("topmost"),
        INT8ARRAY("Int8Array"),
        INT16ARRAY("Int16Array"),
        INT32ARRAY("Int32Array"),
        UINT8ARRAY("Uint8Array"),
        UINT16ARRAY("Uint16Array"),
        UINT32ARRAY("Uint32Array"),
        FLOAT32ARRAY("Float32Array"),
        FLOAT64ARRAY("Float64Array");

static void abort_on(std::string why, Ref element) {
  std::cerr << why << ' ';
  element->stringify(std::cerr);
  std::cerr << "\n";
  abort();
}
static void abort_on(std::string why, IString element) {
  std::cerr << why << ' ' << element.str << "\n";
  abort();
}

using namespace cashew;
using namespace wasm;

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

  // function types. we fill in this information as we see
  // uses, in the first pass

  std::map<IString, GeneralType> importedFunctionTypes;

public:
  Asm2WasmModule() : nextGlobal(8), maxGlobal(1000) {}
  void processAsm(Ref ast);

private:
  BasicType detectWasmType(Ref ast) {
    if (ast[0] == BINARY || ast[0] == NUM) return BasicType::i32;
    if (ast[0] == UNARY_PREFIX) return BasicType::f64;
    abort_on("confused detectWasmType", ast);
  }

  bool isIntegerCoercion(Ref ast) {
    if (ast[0] == BINARY && (ast[1] == OR || ast[1] == TRSHIFT)) return true;
    return false;
  }
 
  bool isUnsignedCoercion(Ref ast) {
    if (ast[0] == BINARY && ast[1] == TRSHIFT) return true;
    return false;
  }

  // an asm.js binary op can either be a binary or a relational in wasm
  bool parseAsmBinaryOp(IString op, Ref left, Ref right, BinaryOp &binary, RelationalOp &relational) {
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
    bool isInteger = isIntegerCoercion(left);
    assert(isInteger == isIntegerCoercion(right));
    bool isUnsigned = isUnsignedCoercion(left);
    assert(isUnsigned == isUnsignedCoercion(right));
    if (op == DIV) {
      if (isInteger) {
        { binary = isUnsigned ? BinaryOp::DivU : BinaryOp::DivS; return true; }
      }
      { binary = BinaryOp::Div; return true; }
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

  wasm::Arena tempAllocator;

  std::map<unsigned, Ref> tempNums;

  // given HEAP32[addr >> 2], we need an absolute address, and would like to remove that shift.
  // if there is a shift, we can just look through it, etc.
  Ref unshift(Ref ptr, unsigned bytes) {
    unsigned shifts = bytesToShift(bytes);
    if (ptr[0] == BINARY && ptr[1] == RSHIFT && ptr[3][0] == NUM && ptr[3][1]->getInteger() == shifts) {
      return ptr[2]; // look through it
    } else if (ptr[0] == NUM) {
      // constant, apply a shift (e.g. HEAP32[1] is address 4)
      unsigned addr = ptr[1]->getInteger();
      if (tempNums.find(addr) == tempNums.end()) {
        Ref temp = tempAllocator.alloc<Value>();
        temp->setNumber(addr);
        tempNums[addr] = temp;;
      }
      return tempNums[addr];
    }
    abort();
  }

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
      assert(importedFunctionTypes.find(name) != importedFunctionTypes.end());
      import.type = importedFunctionTypes[name];
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
          assert(value[1]->getInteger() == 0);
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
          // we have to do this later, since we don't know the type yet.
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
          // function table
          Ref contents = value[1];
          for (unsigned k = 0; k < contents->size(); k++) {
            table.vars.push_back(contents[k][1]->getIString());
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

  for (unsigned i = 1; i < body->size(); i++) {
    Ref curr = body[i];
    if (curr[0] == VAR) {
      for (unsigned j = 0; j < curr[1]->size(); j++) {
        Ref pair = curr[1][j];
        IString name = pair[0]->getIString();
        Ref value = pair[1];
        if (value[0] == DOT) {
          // function import
          // we can do this now, after having seen the type based on the use
          addImport(name, value, BasicType::none);
        }
      }
    }
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
    return IString((prefix + std::to_string(nextId++)).c_str(), false);
  };

  IStringSet functionVariables; // params or locals 
  std::vector<IString> breakStack; // where a break will go
  std::vector<IString> continueStack; // where a continue will go

  for (unsigned i = 0; i < params->size(); i++) {
    Ref curr = body[i];
    assert(curr[0] == STAT);
    curr = curr[1];
    assert(curr[0] == ASSIGN && curr[2][0] == NAME);
    IString name = curr[2][1]->getIString();
    function->params.emplace_back(name, detectWasmType(curr[3]));
    functionVariables.insert(name);
  }
  unsigned start = params->size();
  while (start < body->size() && body[start][0] == VAR) {
    Ref curr = body[start];
    for (unsigned j = 0; j < curr[1]->size(); j++) {
      Ref pair = curr[1][j];
      IString name = pair[0]->getIString();
      function->locals.emplace_back(name, detectWasmType(pair[1]));
      functionVariables.insert(name);
    }
    start++;
  }
  function->result = BasicType::none; // updated if we see a return
  Block *topmost = nullptr; // created if we need one for a return
  // processors
  std::function<Expression* (Ref, unsigned)> processStatements;

  bool debug = !!getenv("ASM2WASM_DEBUG");

  std::function<Expression* (Ref)> process = [&](Ref ast) -> Expression* {
    if (debug) {
      std::cout << "at: ";
      ast->stringify(std::cout);
      std::cout << "\n";
    }
    IString what = ast[0]->getIString();
    if (what == STAT) {
      return process(ast[1]); // and drop return value, if any
    } else if (what == ASSIGN) {
      if (ast[2][0] == NAME) {
        auto ret = allocator.alloc<SetLocal>();
        ret->id = ast[2][1]->getIString();
        ret->value = process(ast[3]);
        return ret;
      } else if (ast[2][0] == SUB) {
        Ref target = ast[2];
        assert(target[1][0] == NAME);
        IString heap = target[1][1]->getIString();
        assert(views.find(heap) != views.end());
        View& view = views[heap];
        auto ret = allocator.alloc<Store>();
        ret->bytes = view.bytes;
        ret->offset = 0;
        ret->align = view.bytes;
        ret->ptr = process(unshift(target[2], view.bytes));
        ret->value = process(ast[3]);
        return ret;
      }
      abort_on("confusing assign", ast);
    } else if (what == BINARY) {
      BinaryOp binary;
      RelationalOp relational;
      bool isBinary = parseAsmBinaryOp(ast[1]->getIString(), ast[2], ast[3], binary, relational);
      if (isBinary) {
        auto ret = allocator.alloc<Binary>();
        ret->op = binary;
        ret->left = process(ast[2]);
        ret->right = process(ast[3]);
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
      if (fmod(num, 1) == 0 && double(int(num)) == num) {
        ret->value.type = BasicType::i32;
        ret->value.i32 = num;
      } else {
        ret->value.type = BasicType::f64;
        ret->value.f64 = num;
      }
      return ret;
    } else if (what == NAME) {
      IString name = ast[1]->getIString();
      if (functionVariables.has(name)) {
        // var in scope
        auto ret = allocator.alloc<GetLocal>();
        ret->id = name;
        return ret;
      }
      // global var, do a load from memory
      assert(mappedGlobals.find(name) != mappedGlobals.end());
      MappedGlobal global = mappedGlobals[name];
      auto ret = allocator.alloc<Load>();
      ret->bytes = getBasicTypeSize(global.type);
      ret->signed_ = true; // but doesn't matter
      ret->offset = 0;
      ret->align = ret->bytes;
      auto ptr = allocator.alloc<Const>();
      ptr->value.type = BasicType::i32; // XXX for wasm64, need 64
      ptr->value.i32 = global.address;
      ret->ptr = ptr;
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
      ret->offset = 0;
      ret->align = view.bytes;
      ret->ptr = process(unshift(ast[2], view.bytes));
      return ret;
    } else if (what == UNARY_PREFIX) {
      if (ast[2][0] == NUM) {
        auto ret = allocator.alloc<Const>();
        ret->value.type = BasicType::f64;
        ret->value.f64 = ast[2][1]->getNumber();
        return ret;
      }
      return process(ast[2]); // look through the coercion
    } else if (what == IF) {
      auto ret = allocator.alloc<If>();
      ret->condition = process(ast[1]);
      ret->ifTrue = process(ast[2]);
      ret->ifFalse = !!ast[3] ? process(ast[2]) : nullptr;
      return ret;
    } else if (what == CALL) {
      if (ast[1][0] == NAME) {
        IString name = ast[1][1]->getIString();
        Call* ret;
        if (imports.find(name) != imports.end()) {
          ret = allocator.alloc<CallImport>();
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
      abort(); // TODO: function pointers
    } else if (what == RETURN) {
      // wasm has no return, so we just break on the topmost block
      if (!topmost) {
        topmost = allocator.alloc<Block>();
        topmost->var = TOPMOST;
      }
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
      ret->var = !!ast[1] ? ast[1]->getIString() : breakStack.back();
      ret->condition = nullptr;
      ret->value = nullptr;
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

  processStatements = [&](Ref ast, unsigned from) {
    auto block = allocator.alloc<Block>();
    for (unsigned i = from; i < ast->size(); i++) {
      block->list.push_back(process(ast[i]));
    }
    return block;
  };
  // body
  function->body = processStatements(body, start);
  if (topmost) {
    topmost->list.push_back(function->body);
    function->body = topmost;
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
  //std::cout << "\n";

  printf("wasming...\n");
  Asm2WasmModule wasm;
  wasm.processAsm(asmjs);

  printf("printing...\n");
  wasm.print(std::cout);

  printf("done.\n");
  printf("TODO: get memory for globals, and clear it to zero; and read values for imports\n");
  printf("TODO: assert on no aliasing function pointers\n");
}

